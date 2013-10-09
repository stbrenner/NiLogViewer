//-----------------------------------------------------------------------------
// EmailSender.h
//
// Author: Stephan Brenner
// Date:   09/21/2006
//-----------------------------------------------------------------------------

#pragma once

class CEmailSender
{
public:
  CEmailSender(void);
  DWORD Send(IN LPCTSTR lpszLogFilePath);
public:
  ~CEmailSender(void);
};
