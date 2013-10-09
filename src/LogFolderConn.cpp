//-----------------------------------------------------------------------------
//
//    LogFolderConn.cpp
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "LogFolderConn.h"
#include "RemoteConnection.h"

static const TCHAR NI_LOGFILE_REGSECTION[] = _T("SOFTWARE\\NetSupport\\NetInstall\\LogFileSettings\\");
static const TCHAR NI_LOGFILE_REGSECTION_WOW6432[] = _T("SOFTWARE\\Wow6432Node\\NetSupport\\NetInstall\\LogFileSettings\\");
static const TCHAR NI_LOGFILE_REGVALUE[]   = _T("ResolvedLogFilePath");
static const TCHAR LANMANAGER_REGSECTION[] = _T("SYSTEM\\CurrentControlSet\\Services\\lanmanserver\\Shares\\");

///
/// Constructor
///
CLogFolderConn::CLogFolderConn(IN HWND hWnd)
{
	m_hWnd = hWnd;
}

///
/// Destructor.
///
CLogFolderConn::~CLogFolderConn(void)
{
}

///
/// Returns the log folder.
///
void CLogFolderConn::GetLogFolder(LPTSTR lpsz, int count)
{
	_tcsncpy(lpsz, m_szLogFolder, count);
}

///
/// Connects to the default log folder on a remote computer.
///
DWORD CLogFolderConn::ConnectToComputer(IN LPCTSTR lpszComputerName)
{
	DWORD dwReturn = NO_ERROR;

	// Remove invalid characters from the computer name.
	LPTSTR lpszComputer = (LPTSTR)lpszComputerName;
	tcstrim(&lpszComputer, _T('\\'));
	tcstrim(&lpszComputer, _T(' '));

	// Read log folder from remote registry.
	TCHAR szLocalFolder[MAX_PATH];
	dwReturn = ReadLogFolderFromRegistry(szLocalFolder, MAX_PATH, lpszComputer);

	if (dwReturn != NO_ERROR)
	{
		return dwReturn;
	}

	// Now convert the local path we got from the registry into a UNC path.
	TCHAR szUncPath[MAX_PATH];
	ConvertLocalFolderToUnc(lpszComputer, szLocalFolder, szUncPath, MAX_PATH);

	if (dwReturn != NO_ERROR)
	{
		return dwReturn;
	}

	return ConnectToFolder(szUncPath);
}

///
/// Converts a local folder path to a UNC path.
///
DWORD CLogFolderConn::ConvertLocalFolderToUnc(IN LPCTSTR lpszComputer, IN LPTSTR lpszLocalFolder, IN OUT LPTSTR lpszUncPath, IN int iSize)
{
	// Open remote registry.
	HKEY hRegistry;
	DWORD dwReturn = RegConnectRegistry(lpszComputer, HKEY_LOCAL_MACHINE, &hRegistry);

	if (dwReturn != ERROR_SUCCESS)
	{
		return dwReturn;
	}

	// Open LAN manager section.
	HKEY hRegSection;
	dwReturn = RegOpenKeyEx(hRegistry, LANMANAGER_REGSECTION, NULL, KEY_READ, &hRegSection);

	if (dwReturn != ERROR_SUCCESS)
	{
		RegCloseKey(hRegistry);
		return dwReturn;
	}

	// Enumerate through the values.
	for (int i = 0; ; i++)
	{
		TCHAR szValueName[MAX_PATH];
		DWORD dwValueName = MAX_PATH;
		DWORD dwType;
		TCHAR szData[1000];
		DWORD dwData = 1000; 

		dwReturn = RegEnumValue(hRegSection, i, szValueName, &dwValueName, NULL, &dwType, (LPBYTE)szData, &dwData);

		if (dwReturn != ERROR_SUCCESS)
		{
			break;   // No more share found!
		}

		// Extract the path from the registry value.
		TCHAR szPath[MAX_PATH];
		LPTSTR lpszCscFlags = szData;
		LPTSTR lpszMaxUses = lpszCscFlags + _tcslen(lpszCscFlags) + 1;
		_stscanf(lpszMaxUses + _tcslen(lpszMaxUses) + 1, _T("Path=%[a-zA-Z:\\ ]"), szPath);

		if (_tcsstr(lpszLocalFolder, szPath) != NULL)
		{
			_stprintf_s(lpszUncPath, iSize, _T("\\\\%s\\%s\\%s"), lpszComputer, szValueName, 
				lpszLocalFolder + _tcslen(szPath));

			RegCloseKey(hRegSection);
			return RegCloseKey(hRegistry);
		}
	}

	RegCloseKey(hRegSection);
	RegCloseKey(hRegistry);

	// Fallback.
	TCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	_tsplitpath(lpszLocalFolder, szDrive, szDir, szFName, szExt);
	szDrive[1] = _T('$');

	_stprintf_s(lpszUncPath, iSize, _T("\\\\%s\\%s%s%s%s"), lpszComputer, szDrive, szDir, szFName, szExt);
	return NO_ERROR;
}

///
/// Connects to a local or remote log folder.
///
DWORD CLogFolderConn::ConnectToFolder(IN LPCTSTR lpszFolder)
{
	DWORD dwReturn = NO_ERROR;

	// Connect to the folder (when necessary with a different user account)
	CRemoteConnection remoteConn(lpszFolder, m_hWnd);
	dwReturn = remoteConn.Open();

	if (dwReturn != NO_ERROR)
	{
		return dwReturn;
	}

	_tcsncpy(m_szLogFolder, lpszFolder, MAX_PATH);
	return NO_ERROR;
}

///
/// Connects to the default local log folder. The default gets read out from the enteo
/// retistry section.
///
DWORD CLogFolderConn::ConnectToLocal()
{
	TCHAR szLogFolder[MAX_PATH];
	DWORD dwReturn = ReadLogFolderFromRegistry(szLogFolder, MAX_PATH);

	if (dwReturn != NO_ERROR)
	{
		return ReportError(m_hWnd, dwReturn);
	}

	return ConnectToFolder(szLogFolder);
}

///
/// Reads the log folder either from the local or from a remote registry.
///
DWORD CLogFolderConn::ReadLogFolderFromRegistry(IN OUT LPTSTR lpszFolder, IN int iMaxSize, IN LPCTSTR lpszComputer)
{
	HKEY hRegistry = HKEY_LOCAL_MACHINE;
	DWORD dwReturn = NO_ERROR;

	if (lpszComputer != NULL && _tcslen(lpszComputer) != 0)
	{
		// Establish a connection to IPC$ - this also gains access to the remote registry.
		TCHAR lpszIpcDollar[MAX_PATH];
		_stprintf(lpszIpcDollar, _T("\\\\%s\\IPC$"), lpszComputer);

		CRemoteConnection remoteConn(lpszIpcDollar, m_hWnd);

		while (true)
		{
			dwReturn = remoteConn.Open();

			if (dwReturn == ERROR_ACCESS_DENIED)
			{
				continue;
			}
			else if (dwReturn == ERROR_SUCCESS)
			{
				break;
			}
			else
			{
				return dwReturn;
			}
		}

		// Open remote registry.
		dwReturn = RegConnectRegistry(lpszComputer, HKEY_LOCAL_MACHINE, &hRegistry);

		if (dwReturn != ERROR_SUCCESS)
		{
			return dwReturn;
		}
	}

	// Open NetInstall section (normal hive)
	HKEY hNiReg;
	dwReturn = RegOpenKeyEx(hRegistry, NI_LOGFILE_REGSECTION, NULL, KEY_READ, &hNiReg);

	if (dwReturn != ERROR_SUCCESS)
	{
		// Fallback: explicitly try 64 bit hive
		dwReturn = RegOpenKeyEx(hRegistry, NI_LOGFILE_REGSECTION, NULL, KEY_READ | KEY_WOW64_64KEY, &hNiReg);
	}

	if (dwReturn != ERROR_SUCCESS)
	{
		// Fallback: explicitly try Wow6432Node hive
		dwReturn = RegOpenKeyEx(hRegistry, NI_LOGFILE_REGSECTION_WOW6432, NULL, KEY_READ, &hNiReg);
	}

	if (dwReturn != ERROR_SUCCESS)
	{
		RegCloseKey(hRegistry);
		return ERROR_NI_REG_SECTION_NOT_FOUND;
	}

	// Read log file folder
	DWORD dwType;
	DWORD dwSize = (DWORD)iMaxSize;
	dwReturn = RegQueryValueEx(hNiReg, NI_LOGFILE_REGVALUE, NULL, &dwType, (LPBYTE)lpszFolder, &dwSize);

	RegCloseKey(hNiReg);
	RegCloseKey(hRegistry);
	return dwReturn;
}