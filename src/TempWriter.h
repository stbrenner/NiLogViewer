#pragma once

class CTempWriter
{
public:
  CTempWriter(void);
  DWORD WriteResourceFile(IN LPCTSTR lpszResourceName, IN LPCTSTR lpszFileName);
  void GetPath(IN LPCTSTR lpszFileName, IN OUT LPTSTR lpszPath);
public:
  ~CTempWriter(void);
};
