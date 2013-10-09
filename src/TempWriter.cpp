//-----------------------------------------------------------------------------
// TempWriter.cpp
//
// Author: Stephan Brenner
// Date:   09/21/2006
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "TempWriter.h"

CTempWriter::CTempWriter(void)
{
}

CTempWriter::~CTempWriter(void)
{
}

DWORD CTempWriter::WriteResourceFile(IN LPCTSTR lpszResourceName, IN LPCTSTR lpszFileName)
{
  HRSRC hRes = FindResource(NULL, lpszResourceName, _T("RT_FILE"));
  if (!hRes)
    return ERROR;

  DWORD dwFileSize = SizeofResource(NULL, hRes);

  HGLOBAL hResLoad = LoadResource(NULL, hRes); 
  if (!hResLoad)
    return ERROR;

  LPVOID lpResLock = LockResource(hResLoad); 
  if (!lpResLock) 
    return ERROR;

  TCHAR szFilePath[MAX_PATH];
  GetPath(lpszFileName, szFilePath);
  HANDLE hBcDll = CreateFile(szFilePath, GENERIC_WRITE,
    NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);

  DWORD dwBcDllWritten = 0;
  WriteFile(hBcDll, lpResLock, dwFileSize, &dwBcDllWritten, NULL);

  CloseHandle(hBcDll);
  return ERROR_SUCCESS;
}

void CTempWriter::GetPath(IN LPCTSTR lpszFileName, IN OUT LPTSTR lpszPath)
{
  TCHAR szTempPath[MAX_PATH];
  GetTempPath(MAX_PATH, szTempPath);
  _stprintf(lpszPath, _T("%s\\%s"), szTempPath, lpszFileName);
}
