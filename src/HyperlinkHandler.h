//-----------------------------------------------------------------------------
//
//    HyperlinkHandler.h
//
//    Author: Stephan Brenner
//    Date:   04/09/2006
//-----------------------------------------------------------------------------

#pragma once

class CHyperlinkHandler
{
public:
	CHyperlinkHandler(IN HWND hWnd, IN LPCTSTR lpszPath);
	~CHyperlinkHandler(void);

	DWORD Execute();
	BOOL HasExtension(IN LPCTSTR lpszExt);
	BOOL HasFolder();

protected:
	DWORD ShellExecuteWrapper(IN LPCTSTR lpszCommand, IN LPCTSTR lpszPath);
	void GetFolder(IN OUT LPTSTR lpszFolder, int size);
	DWORD ShowFolder();

private:
	LPTSTR m_lpszPath;
	HWND m_hWnd;
};
