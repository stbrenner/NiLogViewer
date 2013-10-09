//-----------------------------------------------------------------------------
//
//    RemoteConnection.h
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------

#pragma once

class CRemoteConnection
{
public:
	CRemoteConnection(IN LPCTSTR lpszPath, IN HWND hWnd = NULL);
	~CRemoteConnection(void);

	DWORD Open();
	void Close();

protected:
	DWORD AddConnection();
	DWORD CancelConnection();
	DWORD CheckAccess();
	DWORD CheckFileAccess();
	DWORD CheckIpcDollarAccess();
	BOOL IsIpcDollar();
	void GetComputerName(IN OUT LPTSTR lpszComputerName, IN int iSize);

private:
  HWND  m_hWnd;
  TCHAR m_szPath[MAX_PATH];
};
