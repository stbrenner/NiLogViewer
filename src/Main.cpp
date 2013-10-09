//
//	xxmain.cpp
//
//	By Stephan Brenner.
//
//	NI Log Viewer.
//	This code is public domain, use and abuse as you desire.
//
#include "stdafx.h"

// Main entry point.
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nShowCmd )
{
	int rc = -1;
	
	InitCommonControls();

	// Write Command Line to registry
	ClsRegKey key;
	if (lpCmdLine && key.OpenKey( HKEY_CURRENT_USER, REG_PATH ) == ERROR_SUCCESS )
	{
		key.SetValue(lpCmdLine, _T("LastCmdLine"));
		key.CloseKey();
	}

  // Check, if LogViewer is already running
  HWND hLogViewer = FindWindow(_T("ClsWindowClass"), NULL);
  if (hLogViewer && lpCmdLine)
  {
    LRESULT lRet = ::SendMessage(hLogViewer, WM_OPEN_LOG_FILE, 0, _tcslen(lpCmdLine));
    if (lRet == 1)   // 1 = success
      return 0;
  }

  // Create temporary directory
  TCHAR szTempPath[MAX_PATH];
  TCHAR szBrainchildDir[MAX_PATH];

  GetTempPath(MAX_PATH, szTempPath);
  _stprintf(szBrainchildDir, _T("%s\\Brainchild"), szTempPath);
  CreateDirectory(szBrainchildDir, NULL);

  SetCurrentDirectory(szBrainchildDir);
	
  // Extract Brainchild config file
  TCHAR szDefaultsDir[MAX_PATH];
  _stprintf(szDefaultsDir, _T("%s\\Defaults"), szBrainchildDir);
  CreateDirectory(szDefaultsDir, NULL);

  HRSRC hLogBcpRes = FindResource(NULL, (LPCTSTR)IDR_ENTEO_LOG_FILE_BCB, 
    _T("RT_FILE"));
  if (!hLogBcpRes)
    return 1;

  DWORD dwLogBcpSize = SizeofResource(NULL, hLogBcpRes);

  HGLOBAL hLogBcpResLoad = LoadResource(NULL, hLogBcpRes); 
  if (!hLogBcpResLoad)
    return 1;

  LPVOID lpLogBcpResLock = LockResource(hLogBcpResLoad); 
  if (!lpLogBcpResLock) 
    return 1;

  TCHAR szLogBcp[MAX_PATH];
  _stprintf(szLogBcp, _T("%s\\enteo Log File.bcp"), szDefaultsDir);
  HANDLE hLogBcp = CreateFile(szLogBcp, GENERIC_WRITE,
    NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);

  DWORD dwLogBcpWritten = 0;
  WriteFile(hLogBcp, lpLogBcpResLock, dwLogBcpSize,
    &dwLogBcpWritten, NULL);

  CloseHandle(hLogBcp);

  // Extract Brainchild DLL from resource
  HRSRC hBcDllRes = FindResource(NULL, (LPCTSTR)IDR_BRAINCHILD_DLL,
    _T("RT_FILE"));
  if (!hBcDllRes)
    return 1;

  DWORD dwBcDllSize = SizeofResource(NULL, hBcDllRes);

  HGLOBAL hBcDllResLoad = LoadResource(NULL, hBcDllRes); 
  if (!hBcDllResLoad)
    return 1;

  LPVOID lpBcDllResLock = LockResource(hBcDllResLoad); 
  if (!lpBcDllResLock) 
    return 1;

  TCHAR szBrainchildDll[MAX_PATH];
  _stprintf(szBrainchildDll, _T("%s\\brainchild.dll"), szBrainchildDir);
  HANDLE hBcDll = CreateFile(szBrainchildDll, GENERIC_WRITE,
    NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);

  DWORD dwBcDllWritten = 0;
  WriteFile(hBcDll, lpBcDllResLock, dwBcDllSize,
    &dwBcDllWritten, NULL);

  CloseHandle(hBcDll);


  
  // Open Brainchild DLL.
	ClsModule	mod(_T("brainchild.dll"));
	if ( mod.IsValid())
	{
		// Setup app.
		if ( ClsGetApp()->Setup( hInstance, lpCmdLine, nShowCmd, MAKEINTRESOURCE( IDI_ICON ), _T("NI Log Viewer")))
		{
			// Get maximized bit.
			ClsRegKey key;
			DWORD dwMaximize = FALSE;

			// Open key.
			if ( key.OpenKey( HKEY_CURRENT_USER, REG_PATH ) == ERROR_SUCCESS )
			{
				// Read value.
				key.QueryValue( _T( "FrameMaximized" ), dwMaximize );
				key.CloseKey();
			}

			// Create and open the frame window.
			BCFrameWnd main;
			if ( main.Create( WS_EX_ACCEPTFILES, NULL, _T( "NI Log Viewer" ), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL ))
			{
				// Maximized?
				main.ShowWindow( dwMaximize ? SW_MAXIMIZE : SW_SHOWNORMAL );
				main.DrawMenuBar();

				// Load accelerators...
				if ( ClsGetApp()->LoadAcceleratorTable( main, MAKEINTRESOURCE( IDR_ACCEL )))
					// Messagepump...
					rc = main.HandleMessageTraffic();
			}
		}
	}
	return rc;
}