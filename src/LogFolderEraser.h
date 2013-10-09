//-----------------------------------------------------------------------------
//
//    LogFolderEraser.h
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------

#pragma once

class CLogFolderEraser
{
public:
	CLogFolderEraser(IN LPCTSTR lpszPath);
	~CLogFolderEraser(void);

	DWORD Delete(IN int iSeconds);

protected:
	BOOL IsFileOlder(IN FILETIME ftLastWriteTime, IN int iSeconds);

private:
	TCHAR m_szPath[MAX_PATH];
};
