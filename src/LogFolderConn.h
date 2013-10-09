//-----------------------------------------------------------------------------
//
//    LogFolderConn.h
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------

#pragma once

///
/// Opens a connection to a local or remote log folder.
///
class CLogFolderConn
{
public:
  CLogFolderConn(IN HWND hWnd = NULL);
  ~CLogFolderConn(void);

  DWORD ConnectToComputer(IN LPCTSTR lpszComputerName);
  DWORD ConnectToFolder(IN LPCTSTR lpszFolder);
  DWORD ConnectToLocal();

	void GetLogFolder(LPTSTR lpsz, int count);

protected:
	DWORD ConvertLocalFolderToUnc(IN LPCTSTR lpszComputer, IN LPTSTR lpszLocalFolder, IN OUT LPTSTR lpszUncPath, IN int iSize);
	DWORD ReadLogFolderFromRegistry(IN OUT LPTSTR lpszFolder, IN int iMaxSize, IN LPCTSTR lpszComputer = NULL);

private:
  HWND  m_hWnd;
  TCHAR m_szLogFolder[MAX_PATH];
};
