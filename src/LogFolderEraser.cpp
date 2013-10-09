//-----------------------------------------------------------------------------
//
//    CLogFolderEraser.cpp
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "LogFolderEraser.h"

CLogFolderEraser::CLogFolderEraser(IN LPCTSTR lpszPath)
{
	_tcscpy_s(m_szPath, MAX_PATH, lpszPath);
}

CLogFolderEraser::~CLogFolderEraser(void)
{
}

///
/// Deletes all files from the log folder that are older than a certain timespan.
///
DWORD CLogFolderEraser::Delete(int iSeconds)
{
	// Build a search mask and start search.
	TCHAR szMask[MAX_PATH];
	_stprintf(szMask, _T("%s\\*.log*"), m_szPath);

  WIN32_FIND_DATA FindFileData;
  HANDLE hFind;
  hFind = FindFirstFile(szMask, &FindFileData);

  if (hFind == INVALID_HANDLE_VALUE) 
  {
		return GetLastError();
  } 

	// Enumerate through the files and delete them
	BOOL NoMoreFiles = FALSE;
  while (!NoMoreFiles) 
  {
		TCHAR szFile[MAX_PATH];
		_stprintf(szFile, _T("%s\\%s"), m_szPath, FindFileData.cFileName);
		FILETIME ftLastWriteTime = FindFileData.ftLastWriteTime;

		if (FindNextFile(hFind, &FindFileData) == 0)
		{
			NoMoreFiles = TRUE;
		}

		if (!IsFileOlder(ftLastWriteTime, iSeconds))
		{
			continue;
		}

		// Now delete the file.
		if (DeleteFile(szFile) == 0)
		{
			FindClose(hFind);
			return GetLastError();
		}
	}
    
	FindClose(hFind);
  return NO_ERROR;
}

BOOL CLogFolderEraser::IsFileOlder(IN FILETIME ftLastWriteTime, IN int iSeconds)
{
	// Build the compare value based on the supported seconds input.
	__time64_t timeCurrent;
	_time64(&timeCurrent);
	__time64_t timeCompareTo = timeCurrent - iSeconds;

	// Convert the compare value to a filetime structure.
	struct tm tmCompareTo;
	_gmtime64_s(&tmCompareTo, &timeCompareTo);

	SYSTEMTIME stCompare;
	stCompare.wYear = (WORD)tmCompareTo.tm_year + 1900;
	stCompare.wMonth = (WORD)tmCompareTo.tm_mon + 1;
	stCompare.wDay = (WORD)tmCompareTo.tm_mday;
	stCompare.wHour = (WORD)tmCompareTo.tm_hour;
	stCompare.wMinute = (WORD)tmCompareTo.tm_min;
	stCompare.wSecond = (WORD)tmCompareTo.tm_sec;
	stCompare.wMilliseconds = 0;
	stCompare.wDayOfWeek = (WORD)tmCompareTo.tm_wday;

	FILETIME ftCompare;
	SystemTimeToFileTime(&stCompare, &ftCompare);

	// Now do the compare.
	if (CompareFileTime(&ftLastWriteTime, &ftCompare) >= 0)
	{
		return FALSE;
	}

	return TRUE;
}