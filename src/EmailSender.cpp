//-----------------------------------------------------------------------------
// EmailSender.cpp
//
// Author: Stephan Brenner
// Date:   09/21/2006
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "EmailSender.h"
#include "TempWriter.h"

CEmailSender::CEmailSender(void)
{
}

CEmailSender::~CEmailSender(void)
{
}

DWORD CEmailSender::Send(IN LPCTSTR lpszLogFilePath)
{
  TCHAR szMapiMailPath[MAX_PATH];
  CTempWriter oTempWriter;
  oTempWriter.WriteResourceFile((LPCTSTR)IDR_MAPIMAIL_EXE, "mapimail.exe");
  oTempWriter.GetPath("mapimail.exe", szMapiMailPath);

  TCHAR szArgs[MAX_PATH];
  _stprintf_s(szArgs, MAX_PATH, _T("\"%s\""), lpszLogFilePath);

  ShellExecute(NULL, NULL, szMapiMailPath, szArgs, NULL, SW_HIDE);
  return ERROR_SUCCESS;
}