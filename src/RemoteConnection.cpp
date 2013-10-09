//-----------------------------------------------------------------------------
//
//    RemoteConnection.cpp
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "RemoteConnection.h"

///
/// Creates a remote connection object.
///
CRemoteConnection::CRemoteConnection(IN LPCTSTR lpszPath, IN HWND hWnd)
{
	m_hWnd = hWnd;
	_tcsncpy(m_szPath, lpszPath, MAX_PATH);
}

///
/// Destructor.
///
CRemoteConnection::~CRemoteConnection(void)
{
}

///
/// Opens the connection.
///
DWORD CRemoteConnection::Open()
{
	DWORD dwReturn = NO_ERROR;
	
	dwReturn = CheckAccess();

	if (dwReturn != ERROR_ACCESS_DENIED)
	{
		return dwReturn;
	}

	dwReturn = CancelConnection();

	if (dwReturn != NO_ERROR)
	{
		return dwReturn;
	}

	return AddConnection();
}

///
/// Closes the connection.
///
void CRemoteConnection::Close()
{
}

///
/// Wrapps the WNetAddConnection3 function
///
DWORD CRemoteConnection::AddConnection()
{
	DWORD dwReturn = NO_ERROR;
	
	NETRESOURCE netResource;
	netResource.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
	netResource.dwScope       = RESOURCE_GLOBALNET;
	netResource.dwType        = RESOURCETYPE_ANY;
	netResource.dwUsage       = RESOURCEUSAGE_CONNECTABLE;
	netResource.lpComment     = NULL;
	netResource.lpLocalName   = NULL;
	netResource.lpProvider    = NULL;
	netResource.lpRemoteName  = m_szPath;

	do
	{
		dwReturn = WNetAddConnection3(m_hWnd, &netResource, NULL, NULL, CONNECT_INTERACTIVE | CONNECT_PROMPT);

		if (dwReturn != ERROR_ACCESS_DENIED && dwReturn != ERROR_INVALID_PASSWORD)
		{
			break;
		}

		ReportError(m_hWnd, dwReturn);
	}	while (dwReturn != NO_ERROR);

	return dwReturn;
}

///
/// Wrapps the WNetCancelConnection3 function
///
DWORD CRemoteConnection::CancelConnection()
{
	TCHAR szComputerName[MAX_PATH];
	GetComputerName(szComputerName, MAX_PATH);

	TCHAR szIpcDollarPath[MAX_PATH];
	_stprintf(szIpcDollarPath, _T("\\\\%s\\IPC$"), szComputerName);

	DWORD dwReturn = WNetCancelConnection2(szIpcDollarPath, 0, TRUE);

	return dwReturn == ERROR_NOT_CONNECTED ? NO_ERROR : dwReturn;
}

///
/// Checks if a logon to the folder is required.
///
DWORD CRemoteConnection::CheckAccess()
{
	if (IsIpcDollar())
	{
		return CheckIpcDollarAccess();
	}

	return CheckFileAccess();
}

///
/// Checks, if the path points to IPC$.
///
BOOL CRemoteConnection::IsIpcDollar()
{
	TCHAR szPathLower[MAX_PATH];
	_tcsncpy(szPathLower, m_szPath, MAX_PATH);
	_tcslwr(szPathLower);

	return _tcsstr(szPathLower, _T("ipc$")) != NULL;
}

///
/// Extracts the computer name from the path.
///
void CRemoteConnection::GetComputerName(IN OUT LPTSTR lpszComputerName, IN int iSize)
{
	LPTSTR lpszTempPath = m_szPath;
	tcstrim(&lpszTempPath, _T('\\'));

	LPTSTR lpszEnd = _tcsstr(lpszTempPath, _T("\\"));

	if (lpszEnd == NULL)
	{
		return;
	}

	_tcsncpy_s(lpszComputerName, iSize, lpszTempPath, lpszEnd - lpszTempPath);
}

///
/// Checks if a logon to the file/folder is required.
///
DWORD CRemoteConnection::CheckFileAccess()
{
	// Remove invalid characters
	TCHAR szTempPath[MAX_PATH];
	_tcscpy_s(szTempPath, MAX_PATH, m_szPath);
	tcstrim_right(szTempPath, _T('\\'));

	// Special handling for hidden network folders.
	if (szTempPath[_tcslen(szTempPath)-1] == _T('$'))
	{
		_stprintf(szTempPath, _T("%s\\*.*"), m_szPath);
	}

  // Check, if we can browse through the files of the folder.
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind;
  hFind = FindFirstFile(szTempPath, &FindFileData);

  if (hFind == INVALID_HANDLE_VALUE) 
  {
		return GetLastError();
  } 

	FindClose(hFind);
  return NO_ERROR;
}

///
/// Checks if a logon to IPC$ is required.
///
DWORD CRemoteConnection::CheckIpcDollarAccess()
{
	TCHAR szComputerName[MAX_PATH];
	GetComputerName(szComputerName, MAX_PATH);

	HKEY hRegistry;
	DWORD dwReturn = RegConnectRegistry(szComputerName, HKEY_LOCAL_MACHINE, &hRegistry);

	if (dwReturn != NO_ERROR)
	{
		if (dwReturn == ERROR_BAD_NETPATH)
		{
			return ERROR_REMOTE_REGISTRY_NOT_STARTED;
		}

		return dwReturn;
	}

  HKEY hRegKey;
  dwReturn = RegOpenKeyEx(hRegistry, _T("SOFTWARE"), NULL, KEY_READ, &hRegKey);

	if (dwReturn != NO_ERROR)
	{
		RegCloseKey(hRegistry);
		return dwReturn;
	}

	RegCloseKey(hRegKey);
	RegCloseKey(hRegistry);
	return dwReturn;
}