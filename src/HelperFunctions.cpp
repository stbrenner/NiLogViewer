//-----------------------------------------------------------------------------
//
//    HelperFunctions.cpp
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "HelperFunctions.h"

///
/// Works similar to CString::Trim()
///
void tcstrim(IN OUT LPTSTR* plpsz, IN TCHAR ch)
{
	if (!plpsz || !*plpsz)
		return;

	LPTSTR lpsz = *plpsz;

	while (*lpsz != _T('\0') && *lpsz == ch)
		lpsz++;

	tcstrim_right(lpsz, ch);

	*plpsz = lpsz;
}

///
/// Works similar to CString::TrimRight()
///
void tcstrim_right(IN OUT LPTSTR lpsz, IN TCHAR ch)
{
	if (!lpsz)
		return;

	while (*lpsz != _T('\0') && lpsz[_tcslen(lpsz)-1] == ch)
		lpsz[_tcslen(lpsz)-1] = 0;
}

///
/// Shows a dialog with the error message.
///
/// @return Returns the same error number as supported as input.
DWORD ReportError(IN HWND hWnd, IN DWORD dwValue)
{
	LPTSTR lpszMessage;

	if (::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |	FORMAT_MESSAGE_FROM_SYSTEM,
		  NULL,	dwValue, 0, (LPTSTR)&lpszMessage, 0,	NULL) == 0)
	{
		// Format message failed
		lpszMessage = (LPTSTR)::LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * 200);
		_stprintf(lpszMessage, _T("Unknown error code %08x (%d)"), dwValue, LOWORD(dwValue));
	}
	else
	{
		// Fomat message succeeded
		LPTSTR p = _tcschr(lpszMessage, _T('\r'));
		if(p != NULL)
		{
			*p = _T('\0');
		}
	}

	ErrorMessageBox(hWnd, lpszMessage);
	::LocalFree(lpszMessage);

	return dwValue;
}

///
/// Shows a message with the error message.
///
void ErrorMessageBox(IN HWND hWnd, IN LPCTSTR lpszMessage)
{
	MessageBox(hWnd, lpszMessage, _T("Error"), MB_ICONERROR);
}

///
/// Writes the values from a combo box into the registry.
///
DWORD WriteComboToRegistry(IN HWND hCombo, IN LPCTSTR lpszKey, IN LPCTSTR lpszSubKey, IN int iReservedValues)
{
  TCHAR szValue[MAX_PATH];
  HKEY hLogVwrReg;
  DWORD dwDisposition;

	// Build the registry path and create the corresponding section.
	TCHAR szRegPath[500];
	_stprintf_s(szRegPath, 500, _T("%s%s\\"), lpszKey, lpszSubKey);

  DWORD dwReturn = RegCreateKeyEx(HKEY_CURRENT_USER, szRegPath, NULL, NULL, 
    REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hLogVwrReg, &dwDisposition);

  if (dwReturn != NO_ERROR)
	{
    return dwReturn;
	}

	// At first place we put the current text.
	int iRegCount = 0;
	GetWindowText(hCombo, szValue, MAX_PATH);
	dwReturn = RegSetValueEx(hLogVwrReg, _T("Current"), NULL, REG_SZ, (const BYTE*)szValue, _tcslen(szValue)*sizeof(TCHAR));

	if (::SendMessage(hCombo, CB_FINDSTRING, 0, (LPARAM)szValue) == CB_ERR)
	{
		dwReturn = RegSetValueEx(hLogVwrReg, _T("Value0"), NULL, REG_SZ, (const BYTE*)szValue, _tcslen(szValue)*sizeof(TCHAR));
		iRegCount++;
	}

	// Then we write the list.
	for (int i = iReservedValues; i < ::SendMessage(hCombo, CB_GETCOUNT, 0, 0); i++)
	{
		::SendMessage(hCombo, CB_GETLBTEXT, i, (LPARAM)szValue);

		TCHAR szKey[100];
		_stprintf(szKey, _T("Value%i"), iRegCount++);

		dwReturn = RegSetValueEx(hLogVwrReg, szKey, NULL, REG_SZ, 
			(const BYTE*)szValue, _tcslen(szValue)*sizeof(TCHAR));

		if (dwReturn != NO_ERROR)
		{
			RegCloseKey(hLogVwrReg);
			return dwReturn;
		}
	}

	return RegCloseKey(hLogVwrReg);
}

///
/// Reads combo box values from the registry.
///
DWORD ReadComboFromRegistry(IN HWND hCombo, IN LPCTSTR lpszKey, IN LPCTSTR lpszSubKey)
{
  TCHAR szValue[MAX_PATH];
	DWORD dwType;
  DWORD dwSize = MAX_PATH;
  HKEY hLogVwrReg;

	// Build the registry path and open the corresponding section.
	TCHAR szRegPath[500];
	_stprintf_s(szRegPath, 500, _T("%s%s\\"), lpszKey, lpszSubKey);

  DWORD dwReturn = RegOpenKeyEx(HKEY_CURRENT_USER, szRegPath, NULL, KEY_READ, &hLogVwrReg);

  if (dwReturn != NO_ERROR)
	{
    return dwReturn;
	}

	// Now add values from registry.
	for (int i = 0; i < 10; i++)
	{
		TCHAR szKey[100];
		_stprintf(szKey, _T("Value%i"), i);

		dwSize = MAX_PATH;
    dwReturn = RegQueryValueEx(hLogVwrReg, szKey, NULL, &dwType, (LPBYTE)szValue, &dwSize);

		if (dwReturn != NO_ERROR)
		{
			break;   // Reached last available value!
		}

		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)szValue);
	}

	dwSize = MAX_PATH;
  dwReturn = RegQueryValueEx(hLogVwrReg, _T("Current"), NULL, &dwType, (LPBYTE)szValue, &dwSize);

	if (dwReturn == NO_ERROR)
	{
		SetWindowText(hCombo, szValue);
	}

	return RegCloseKey(hLogVwrReg);
}

///
/// Writes a window text into the registry.
///
DWORD WriteWindowTextToRegistry(IN HWND hWnd, IN LPCTSTR lpszRegPath, IN LPCTSTR lpszKey)
{
  TCHAR szValue[MAX_PATH];
  HKEY hLogVwrReg;
  DWORD dwDisposition;

  DWORD dwReturn = RegCreateKeyEx(HKEY_CURRENT_USER, lpszRegPath, NULL, NULL, 
    REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hLogVwrReg, &dwDisposition);

  if (dwReturn != NO_ERROR)
	{
    return dwReturn;
	}

	GetWindowText(hWnd, szValue, MAX_PATH);
	dwReturn = RegSetValueEx(hLogVwrReg, lpszKey, NULL, REG_SZ, (const BYTE*)szValue, _tcslen(szValue)*sizeof(TCHAR));

	if (dwReturn != NO_ERROR)
	{
		RegCloseKey(hLogVwrReg);
		return dwReturn;
	}

	return RegCloseKey(hLogVwrReg);
}

///
/// Reads a window text from the registry.
///
DWORD ReadWindowTextFromRegistry(IN HWND hWnd, IN LPCTSTR lpszRegPath, IN LPCTSTR lpszKey)
{
  TCHAR szValue[MAX_PATH];
	DWORD dwType;
  DWORD dwSize = MAX_PATH;
  HKEY hLogVwrReg;

	// Now add values from registry.
  DWORD dwReturn = RegOpenKeyEx(HKEY_CURRENT_USER, lpszRegPath, NULL, KEY_READ, &hLogVwrReg);

  if (dwReturn != NO_ERROR)
	{
    return dwReturn;
	}

	dwSize = MAX_PATH;
  dwReturn = RegQueryValueEx(hLogVwrReg, lpszKey, NULL, &dwType, (LPBYTE)szValue, &dwSize);

	if (dwReturn == NO_ERROR)
	{
		SetWindowText(hWnd, szValue);
	}

	return RegCloseKey(hLogVwrReg);
}