//-----------------------------------------------------------------------------
//
//    HyperlinkHandler.h
//
//    Author: Stephan Brenner
//    Date:   04/09/2006
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "HyperlinkHandler.h"
#include "RemoteConnection.h"

///
/// Constructor.
///
CHyperlinkHandler::CHyperlinkHandler(IN HWND hWnd, IN LPCTSTR lpszPath)
{
	int iSize = _tcslen(lpszPath) + 1;
	m_lpszPath = (LPTSTR)malloc(sizeof(TCHAR) * iSize);
	_tcscpy_s(m_lpszPath, iSize, lpszPath);
	
	m_hWnd = hWnd;
}

///
/// Destructor.
///
CHyperlinkHandler::~CHyperlinkHandler(void)
{
	free(m_lpszPath);
}

///
/// Executes the hyperlink.
///
DWORD CHyperlinkHandler::Execute()
{
	if (HasExtension(_T(".bat")) || HasExtension(_T(".cmd")) || HasExtension(_T(".vbs")))
	{
		return ShellExecuteWrapper(_T("edit"), m_lpszPath);
	}
	else if (HasExtension(_T(".exe")) || HasExtension(_T(".dll")))
	{
		return ShowFolder();
	}

	return ShellExecuteWrapper(NULL, m_lpszPath);
}

///
/// A wrapper around ShellExecute().
///
DWORD CHyperlinkHandler::ShellExecuteWrapper(IN LPCTSTR lpszCommand, IN LPCTSTR lpszPath)
{
	// Logon th remote path.
	CRemoteConnection oRemoteConnection(lpszPath, m_hWnd);
	DWORD dwReturn = oRemoteConnection.Open();

	if (dwReturn != NO_ERROR)
	{
		return dwReturn;
	}

	// Call ShellExecute system API.
  int iReturn = (int)ShellExecute(NULL, lpszCommand, lpszPath, NULL, NULL, SW_SHOWNORMAL);
	
	if (iReturn > 32)
	{
		return NO_ERROR;
	}

	if (iReturn == 0)
	{
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	return iReturn;
}

///
/// Checks, if the hyperlink has a certain file extension (e.g. ".log").
///
BOOL CHyperlinkHandler::HasExtension(IN LPCTSTR lpszExt)
{
	TCHAR szExt[_MAX_EXT];
	_splitpath(m_lpszPath, NULL, NULL, NULL, szExt);

	if (_tcsicmp(szExt, lpszExt))
	{
		return FALSE;
	}

	return TRUE;
}

///
/// Returns the folder, where the file resides.
///
void CHyperlinkHandler::GetFolder(IN OUT LPTSTR lpszFolder, int size)
{
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	_tsplitpath(m_lpszPath, szDrive, szDir, NULL, NULL);

	_stprintf_s(lpszFolder, size, _T("%s%s"), szDrive, szDir);
}

///
/// Shows an explorer window with the folder content.
///
DWORD CHyperlinkHandler::ShowFolder()
{
	TCHAR szFolder[MAX_PATH];
	GetFolder(szFolder, MAX_PATH);

	return ShellExecuteWrapper(NULL, szFolder);
}

///
/// Checks, if the hyperlink consists of a folder part.
///
BOOL CHyperlinkHandler::HasFolder()
{
	TCHAR szFolder[MAX_PATH];
	GetFolder(szFolder, MAX_PATH);
	
	return _tcslen(szFolder) > 0;
}