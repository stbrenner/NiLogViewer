//
//	main.cpp
//
//	By Jan van den Baard.
//
//	"Brainchild Custom Editor Control" demonstration program.
//	This code is public domain, use and abuse as you desire.
//
#include "bcdemo.h"

// Main entry point.
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nShowCmd )
{
	int rc = -1;
	
	InitCommonControls();
	
	// Open brainchild dll.
	ClsModule	mod( _T( "brainchild.dll" ));
	if ( mod.IsValid())
	{
		// Setup app.
		if ( ClsGetApp()->Setup( hInstance, lpCmdLine, nShowCmd, MAKEINTRESOURCE( IDI_ICON )))
		{
			// Check OS version.
			if ( ClsGetApp()->GetPlatformID() != VER_PLATFORM_WIN32_WINDOWS &&
			     ClsGetApp()->GetPlatformID() != VER_PLATFORM_WIN32_NT )
			{
				ClsMessageBox mb;
				mb.Body()    = ISEQ_CENTER ISEQ_BOLD
					       _T( "Brainchild demonstration program\n\n" )
					       ISEQ_NORMAL ISEQ_LEFT
					       _T( "This program requires Windows 95/98/ME/NT4/2000 or XP to run." );
				mb.Title()   = _T( "Brainchild demonstration." );
				mb.Buttons() = _T( "*&OK" );
				mb.Flags()   = ClsMessageBox::MBF_ICONERROR;
				mb.MsgBox( NULL );
				return NULL;
			}

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
			if ( main.Create( WS_EX_ACCEPTFILES, NULL, _T( "Brainchild Demonstration" ), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL ))
			{
				// Maximized?
				main.ShowWindow( dwMaximize ? SW_MAXIMIZE : SW_SHOWNORMAL );

				// Load accelerators...
				if ( ClsGetApp()->LoadAcceleratorTable( main, MAKEINTRESOURCE( IDR_ACCEL )))
					// Messagepump...
					rc = main.HandleMessageTraffic();

				// Destroy the window.
				main.Destroy();
			} 
		}
	}
	return rc;
}