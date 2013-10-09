//-----------------------------------------------------------------------------
//
//    HelperFunctions.h
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------

// String helper functions.
void tcstrim(IN OUT LPTSTR* plpsz, IN TCHAR ch);
void tcstrim_right(IN OUT LPTSTR lpsz, IN TCHAR ch);

// Eror handling functions.
DWORD ReportError(IN HWND hWnd, IN DWORD dwValue);
void ErrorMessageBox(IN HWND hWnd, IN LPCTSTR lpszMessage);

// Regisry functions.
DWORD WriteComboToRegistry(IN HWND hCombo, IN LPCTSTR lpszKey, IN LPCTSTR lpszSubKey, IN int iReservedValues);
DWORD ReadComboFromRegistry(IN HWND hCombo, IN LPCTSTR lpszKey, IN LPCTSTR lpszSubKey);
DWORD WriteWindowTextToRegistry(IN HWND hWnd, IN LPCTSTR lpszKey, IN LPCTSTR lpszSubKey);
DWORD ReadWindowTextFromRegistry(IN HWND hWnd, IN LPCTSTR lpszKey, IN LPCTSTR lpszsubKey);