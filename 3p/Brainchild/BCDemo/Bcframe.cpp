//
//	bcframe.cpp
//
//	By Jan van den Baard.
//
//	"Brainchild Custom Editor Control" demonstration program.
//	This code is public domain, use and abuse as you desire.
//
#include "bcdemo.h" 

// Constructor.
BCFrameWnd::BCFrameWnd()
{;}

// Destructor. Destroys the MDI childs which were
// still present in the array.
BCFrameWnd::~BCFrameWnd()
{
	// Delete imagelists.
	if ( m_hToolbar ) ImageList_Destroy( m_hToolbar );
	if ( m_hPin     ) ImageList_Destroy( m_hPin );

	// Empty child aray.
	for ( int i = 0; i < m_Childs.GetSize(); i++ )
		delete m_Childs[ i ];
}

// Setup the correct menu.
void BCFrameWnd::SetupMenu()
{
	// Is there a menu set already?
	HMENU hMenu = GetMenu();
	if ( hMenu )
	{
		// Do we have child windows?
		if ( m_Childs.GetSize() && hMenu == m_ChildMenu ) return;
		else if ( ! m_Childs.GetSize() && hMenu == m_FrameMenu ) return;
	}

	// Set the necessary menu.
	BCMenu *pMenu = m_Childs.GetSize() ? &m_ChildMenu : &m_FrameMenu;
	MDISetMenu( pMenu, ( pMenu == &m_ChildMenu ) ? pMenu->GetSubMenu( 2 ) : NULL );

	// Recompute menubar.
	DrawMenuBar();
}

// Add a new document.
void BCFrameWnd::AddDocument( LPCTSTR pszFilename )
{
	// Dynamically create a new child and add it to the
	// storage array.
	try
	{
		BCChildWnd *child = new BCChildWnd;
		int nPos = m_Childs.Add( child );

		// Tell it about us.
		child->SetFrameWindow( this );

		// If we have an input name
		// we convert it to a full
		// path name.
		ClsString path( _T( "Unnamed.txt" ));
		if ( pszFilename )
		{
			// Convert name.
			path.AllocateBuffer( ::GetFullPathName( pszFilename, 0, NULL, NULL ));
			::GetFullPathName( pszFilename, path.GetStringLength(), path, NULL );
		}

		// Does the file exist?
		ClsFile file;
		BOOL bCreate = FALSE;
		if ( pszFilename && _tcslen( pszFilename ))
		{
			try
			{
				// Open the file. Throws an exception uppon an error.
				file.Open( path, ClsFile::fileRead );
			}
			catch( ClsFileException& fe )
			{
				ClsString body;
				// Not found?
				if ( fe.m_nCause == ERROR_FILE_NOT_FOUND )
				{
					// Popup a message box.
					body.Format( _T( "\"%s\"\n\ndoes not exist.\nDo you wish to create a new file?" ), ( LPCTSTR )path );
					if ( MessageBox( body, _T( "BCDemo" ), MB_ICONINFORMATION | MB_YESNO ) == IDNO )
					{
						// Remove it from the MRU list.
						m_MRUFiles.RemoveMRUEntry( path );

						// Remove and destroy the child.
						m_Childs.RemoveAt( nPos, 1 );
						delete child;
						return;
					}
					bCreate = TRUE;
				}
				else
				{
					// Show the error description.
					body.Format( _T( "\"%s\"\n\ncould not be opened.\nReason:\n\n%s" ), ( LPCTSTR )path, ( LPCTSTR )fe.m_sDescription );
					MessageBox( body, _T( "BCDemo" ), MB_ICONERROR | MB_OK );

					// Remove and destroy the child.
					m_Childs.RemoveAt( nPos, 1 );
					delete child;

					// Remove it from the MRU.
					m_MRUFiles.RemoveMRUEntry( path );
					return;
				}
			}
			// Close the file.
			file.Close();
		}

		// Already open in one of the other childs?
		if ( ! AlreadyOpen( path ))
		{
			// Do we have an active window?
			ClsMDIChildWindow *pChild = MDIGetActive();
			if ( pChild ) m_dwChildMax = pChild->IsZoomed();

			// Multipart statusbar.
			m_Status.MultiPart( TRUE );

			// Create window.
			if ( child->Create( path, WS_VISIBLE | ( m_dwChildMax ? WS_MAXIMIZE : 0 ), ClsRect( CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT ), this ))
			{
				// Setup menus.
				SetupMenu();

				// Get the index of the icon.
				int nIndex = -1;
				SHFILEINFO fi;
				if ( ::SHGetFileInfo( path, FILE_ATTRIBUTE_NORMAL, &fi, sizeof( fi ), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES ))
					nIndex = fi.iIcon;

				// Destroy the icon.
				::DestroyIcon( fi.hIcon );

				// Add the tab.
				m_MDITabs.InsertItem( m_MDITabs.GetItemCount(), ::PathFindFileName( path ), nIndex, path, true );
				if ( m_MDITabs.IsWindowVisible() == FALSE )
				{
					m_MDITabs.ShowWindow( SW_SHOW );
					ClsRect rc = GetClientRect();
					OnSize( 0, rc.Width(), rc.Height());
				}

				// Do we have a filename?
				if ( pszFilename )
				{
					// Yes, load the file.
					if ( ! bCreate ) child->m_Brainchild.LoadFile( path );
					else child->m_Brainchild.SetFilename( path );

					// Add it to the MRU list.
					m_MRUFiles.AddMRUEntry( path );
				}
				else
					// Give it a name...
					child->m_Brainchild.SetFilename( path );

				// Update file information.
				child->UpdateFileInfo();
				return;
			}
			else
			{
				// Failed to open the MDI child.
				MessageBox( _T( "Failed to open the document window." ), _T( "BCDemo" ), MB_ICONERROR | MB_OK );

				// Reset statusbar if there are no more
				// documents open.
				if ( m_Children.GetSize() == 0 ) m_Status.MultiPart( FALSE );
			}
		}
	}
	catch( ClsMemoryException& me )
	{
		// Show the error description.
		ClsString body;
		body.Format( _T( "Unable to create document.\nReason:\n\n%s" ), ( LPCTSTR )me.m_sDescription );
		MessageBox( body, _T( "BCDemo" ), MB_ICONERROR | MB_OK );
	}
}

// Quit...
void BCFrameWnd::Quit()
{
	// Iterate the children.
	for ( int i = 0; i < m_Childs.GetSize(); i++ )
	{
		// Modified?
		if ( m_Childs[ i ]->m_Brainchild.GetSafeHWND() && m_Childs[ i ]->m_Brainchild.CheckModified())
		{
			// Destroy it.
			m_Childs[ i ]->MDIDestroy();

			// Recursively call Quit() until we are done
			// or until we are cancelled.
			Quit();
		}
		else
			// Do not quit...
			return;
	}

	// Exit...
	::PostQuitMessage( 0 );
}

// Window procedure.
LRESULT BCFrameWnd::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// What's up?
	switch ( uMsg )
	{
		case	WM_DROPFILES:
		{
			// Put us upfront.
			SetForegroundWindow();

			// How many files were dropped?
			TCHAR szFileName[ MAX_PATH + 1 ];
			int nNum = DragQueryFile(( HDROP )wParam, ( UINT )-1, NULL, 0 );

			// Iterate the files.
			for ( int i = 0; i < nNum; i++ )
			{
				// Get file name.
				if ( DragQueryFile(( HDROP )wParam, i, szFileName, MAX_PATH ))
					// Add new document.
					AddDocument( szFileName );
			}
                        break;
		}
	}
	// Call the base class.
	return ClsMDIMainWindow::WindowProc( uMsg, wParam, lParam );
}

// Activate a tab.
void BCFrameWnd::ActivateTab( ClsWindow *pWindow )
{
	// Look it up.
	for ( int i = 0; i <= m_Childs.GetSize(); i++ )
	{
		// Is it this one?
		if ( static_cast< ClsWindow * >( m_Childs[ i ] ) == pWindow )
		{
			// Is this one already activated and, even more
			// important, does the tab already exist?
			if ( m_MDITabs.GetCurSel() != i && i < m_MDITabs.GetItemCount())
				m_MDITabs.SetCurSel( i );
			return;
		}
	}
}

// A child was destroyed and removed from the frame.
void BCFrameWnd::OnMDIChildRemoved( ClsMDIChildWindow *pWnd )
{
	// Look it up in our array.
	for ( int i = 0; i < m_Childs.GetSize(); i++ )
	{
		// Is this the one?
		if ( m_Childs[ i ] == pWnd )
		{
			// Yes. Remove and delete the child.
			m_Childs.RemoveAt( i, 1 );
			delete pWnd;

			// Remove the tab. Hide the tabs
			// if there are no more children.
			m_MDITabs.DeleteItem( i );

			// Setup the menu.
			SetupMenu();

			// Any children left? If not we setup the status
			// bar and hide the tabs.
			if ( m_Childs.GetSize() == 0 )
			{
				m_Status.MultiPart( FALSE );
				m_MDITabs.ShowWindow( SW_HIDE );
				ClsRect rc = GetClientRect();
				OnSize( 0, rc.Width(), rc.Height());
			}
			return;
		}
	}
}

// Bye, bye...
LRESULT BCFrameWnd::OnClose()
{
	// Save maximized switches.
	ClsRegKey key;
	if ( key.OpenKey( HKEY_CURRENT_USER, REG_PATH ) == ERROR_SUCCESS )
	{
		// Set values.
		key.SetValue(( DWORD )m_dwChildMax, _T( "ChildMaximized" ));
		key.SetValue(( DWORD )IsZoomed(), _T( "FrameMaximized" ));
		key.CloseKey();
	}

	// Save the MRU and hotlist.
	m_MRUFiles.SaveMRUList();
	m_HotFiles.SaveMRUList();

	// Bye...
	Quit();
	return TRUE;
}

// WM_SIZE message handler.
LRESULT BCFrameWnd::OnSize( UINT nSizeType, int nWidth, int nHeight )
{
	// Pass the message to the statusbar and the toolbar.
	m_Status.SendMessage( WM_SIZE, nSizeType, MAKELPARAM( nWidth, nHeight ));
	m_Toolbar.SendMessage( WM_SIZE, nSizeType, MAKELPARAM( nWidth, nHeight ));

	// Get the client rectangle.
	ClsRect rcClient;
	GetClientRect( rcClient );

	ClsRect rcStat, rcTool;
	m_Status.GetClientRect( rcStat );
	m_Toolbar.GetClientRect( rcTool );

	// Are the tabs visible?
	if ( m_MDITabs.IsWindowVisible())
	{
		// Yes. Resize the tabs and adjust
		// the client rectangle. The height is fixed
		// which will screw up when the icon title
		// font grows to large.
		m_MDITabs.MoveWindow( 0, rcTool.Bottom() + 2, rcClient.Width(), 25, TRUE );
		rcClient.Top() += 27;
	}
	else
		rcClient.Top() += 2;

	// Size the MDI client area so that it will not
	// overlap with the other controls.
	rcClient.Top() += rcTool.Height();
	rcClient.Bottom() -= rcStat.Height();

	// Resize the MDI client.
	GetMDIClient()->MoveWindow( rcClient, TRUE );
	return 0;
}

// Execute a brainchild command.
BOOL BCFrameWnd::ExecuteCommand( UINT nID )
{
	// ID->Command table.
	struct CmdPair
	{
		UINT	nID;
		UINT	nCmd;
	} cmd[] = {
		{ IDM_PAGESETUP,	CID_DIALOG_PAGESETUP },
		{ IDM_PRINT,		CID_FILE_PRINT },
		{ IDM_PROPERTIES,	CID_DIALOG_PROPERTIES },
		{ IDM_UNDO,		CID_EDIT_UNDO},
		{ IDM_REDO,		CID_EDIT_REDO },
		{ IDM_CUT,		CID_CLIP_CUT },
		{ IDM_COPY,		CID_CLIP_COPY },
		{ IDM_PASTE,		CID_CLIP_PASTE },
		{ IDM_DELETE,		CID_CLIP_DELETE },
		{ IDM_SELECTALL,	CID_MARK_ALL },
		{ IDM_INVERTCASE,	CID_EDIT_SWAPCASESELECTION },
		{ IDM_UPPERCASE,	CID_EDIT_UPPERCASESELECTION },
		{ IDM_LOWERCASE,	CID_EDIT_LOWERCASESELECTION },
		{ IDM_FIND,		CID_DIALOG_FIND },
		{ IDM_FINDNEXT,		CID_FIND_NEXT },
		{ IDM_FINDPREV,		CID_FIND_PREV },
		{ IDM_REPLACE,		CID_DIALOG_REPLACE },
		{ IDM_GOTO,		CID_DIALOG_GOTO },
		{ IDM_TOGGLEBM,		CID_BOOKMARK_TOGGLE },
		{ IDM_CLEARBM,		CID_BOOKMARK_CLEARALL },
		{ IDM_FIRSTBM,		CID_BOOKMARK_GOTOFIRST },
		{ IDM_NEXTBM,		CID_BOOKMARK_GOTONEXT },
		{ IDM_PREVBM,		CID_BOOKMARK_GOTOPREV },
		{ IDM_LASTBM,		CID_BOOKMARK_GOTOLAST },
		{ IDM_MATCH,		CID_FIND_MATCHBRACKET },
		{ IDM_ABOUTBRAINCHILD,	CID_DIALOG_ABOUTCONTROL },
		{ 0,			0 }
	};

	// Iterate the command table.
	for ( int i = 0; cmd[ i ].nID != 0 ; i++ )
	{
		// ID match?
		if ( cmd[ i ].nID == nID )
		{
			// Get the active child and execute the command of the
			// matched ID.
			BCChildWnd *pActive = reinterpret_cast< BCChildWnd *>( MDIGetActive());
			if ( pActive )
				pActive->m_Brainchild.ExecuteCommand( cmd[ i ].nCmd );

			// Command found.
			return TRUE;
		}
	}
	// Not found...
	return FALSE;
}

// Pop filedialog to select the filename to save under.
void BCFrameWnd::SaveAs( BCChildWnd *pChild )
{
	// Get the active child.
	ClsString title( pChild->GetSafeHWND());

	// Strip modified identifier.
	if ( title.Right(1) == _T( "*" ))
		title.SetStringLength( title.GetStringLength() - 1 );

	// Popup the dialog.
	ClsFileDialog fd;
	fd.CenterOnParent() = TRUE;
	if ( fd.DoModal( this, title, ::PathFindFileName( title ), FALSE ))
	{
		// Obtain the full path name of the selected
		// file.
		ClsString file, path;

		// First let's get the name.
		if ( fd.GetName( 0, file ))
		{
			// Allocate a buffer large enough to hold the complete
			// path name.
			path.AllocateBuffer( ::GetFullPathName( file, 0, NULL, NULL ));

			// Get the full path.
			::GetFullPathName( file, path.GetStringLength(), path, NULL );

			// Save the file...
			pChild->m_Brainchild.SaveFile( path );

			// Append to the MRU.
			m_MRUFiles.AddMRUEntry( path );

			// Rename the tab.
			m_MDITabs.SetRedraw( FALSE );
			m_MDITabs.GetItem( m_MDITabs.GetCurSel())->SetText( ::PathFindFileName( path ));
			m_MDITabs.SetRedraw();
			m_MDITabs.Invalidate();
;
		}
	}
}

// WM_COMMAND message handler.
LRESULT BCFrameWnd::OnCommand( UINT nNotifyCode, UINT nCtrlID, HWND hWndCtrl )
{
	// Is there a file selected from the MRU list?
	if ( m_MRUFiles.IsMRUSelection( nCtrlID ))
	{
		// Obtain the path of the selection.
		ClsString path;
		m_MRUFiles.GetMRUEntryPath( nCtrlID, path );

		// Open the file.
		AddDocument( path );

		// Move the MRU entry to the top of the
		// list.
		m_MRUFiles.SelectMRUEntry( path );
		return 0;
	}

	// Is there a file selected from the HOT list?
	if ( m_HotFiles.IsMRUSelection( nCtrlID ))
	{
		// Obtain the path of the selection.
		ClsString path;
		m_HotFiles.GetMRUEntryPath( nCtrlID, path );

		// Open the file.
		AddDocument( path );
		
		// Move the MRU entry to the top of the
		// list.
		m_HotFiles.SelectMRUEntry( path );
		return 0;
	}

	// A standard command?
	if ( ExecuteCommand( nCtrlID ))
		return 0;

	// Must be something else...
	switch ( nCtrlID )
	{
		case	IDM_CLOSE:
		{
			// Close the currently active document.
			ClsMDIChildWindow *pActive = MDIGetActive();
			if ( pActive )
				pActive->MDIDestroy();
			return 0;
		}

		case	IDM_CLOSEALL:
		{
			// Iterate the children.
			while ( m_Childs.GetSize())
			{
				// Modified?
				if ( m_Childs[ 0 ]->m_Brainchild.GetSafeHWND() && m_Childs[ 0 ]->m_Brainchild.CheckModified())
					// Destroy it.
					m_Childs[ 0 ]->MDIDestroy();
				else
					// Cancelled...
					break;
			}
			return 0;
		}

		case	IDM_SAVEALL:
		{
			// Iterate all children.
			BCChildWnd *pCurr;
			ClsString title;
			for ( int i = 0; i < m_Childs.GetSize(); i++ )
			{
				// Is this one modified?
				pCurr = m_Childs[ i ];
				if ( pCurr->m_Brainchild.Modified())
				{
					// Get it's title.
					title = pCurr->GetSafeHWND();

					// Strip modified identifier.
					if ( title.Right(1) == _T( "*" ))
						title.SetStringLength( title.GetStringLength() - 1 );

					// If it does not have a filename we popup
					// a file dialog. Otherwise we simply save
					// the file.
					if ( title.CompareNoCase( _T( "Unnamed.txt" )))
						pCurr->m_Brainchild.SaveFile( NULL );
					else
						SaveAs( pCurr );
				}
			}
			return 0;
		}

		case	IDM_SAVE:
		{
			// Get active child.
			BCChildWnd *pActive = reinterpret_cast< BCChildWnd * >( MDIGetActive());
			ClsString title( pActive->GetSafeHWND());

			// Strip modified identifier.
			if ( title.Right(1) == _T( "*" ))
				title.SetStringLength( title.GetStringLength() - 1 );

			// Does it have a filename?
			if ( title.CompareNoCase( _T( "Unnamed.txt" )))
			{
				// Save the file under it's current name.
				pActive->m_Brainchild.SaveFile( NULL );
				return 0;
			}
			// Fall through to SaveAs...
		}

		case	IDM_SAVEAS:
		{
			// Get the active child and popup a file-dialog.
			BCChildWnd *pActive = reinterpret_cast< BCChildWnd * >( MDIGetActive());
			SaveAs( pActive );
			return 0;
		}

		case	IDM_READONLY:
		{
			// Obtain the check state of the menu.
			ClsMenuItemInfo mi;
			mi.fMask = MIIM_STATE;
			if ( ::GetMenuItemInfo( GetMenu(), IDM_READONLY, FALSE, &mi ))
			{
				// Obtain active child.
				BCChildWnd *pActive = reinterpret_cast< BCChildWnd * >( MDIGetActive());

				// Invert the checked bit and set or clear
				// the read-only flag depending on the checked state.
				mi.fState ^= MFS_CHECKED;
				if ( pActive )
					// The menu is updated in the OnStatusUpdate() method
					// in bcchild.cpp
					pActive->GetBrainchild()->SetReadonly(( BOOL )( mi.fState & MFS_CHECKED ));
			}
			return 0;
		}

		case	IDM_NEWONSTART:
			// Change switch menu and registry.
			ChangeSwitch( nCtrlID );
			return 0;

		case	IDM_CASCADE:
			// Cascade children.
			MDICascade();
			return 0;

		case	IDM_TILEHORZ:
			// Tile children horizontally.
			MDITile();
			return 0;

		case	IDM_TILEVERT:
			// Tile children vertically.
			MDITile( MDITILE_VERTICAL | MDITILE_SKIPDISABLED );
			return 0;

		case	IDM_ARRANGE:
			// Arrange iconized children.
			MDIIconArrange();
			return 0;

		case	IDM_OPEN:
		{
			// Do we already have an open window?
			ClsMDIChildWindow *pActive = MDIGetActive();

			// get path string.
			ClsString path;
			if ( pActive )
				path = pActive->GetSafeHWND();

			// Popup file dialog.
			ClsFileDialog fd;
			fd.CenterOnParent() = TRUE;
			if ( fd.DoModal( this, path, NULL ))
			{
				// Obtain the file name.
				ClsString file;
				if ( fd.GetName( 0, file ))
					// Open the document.
					AddDocument( file );
			}
			return 0;
		}

		case	IDM_PASTENEW:
		{
			// Add a new window.
			AddDocument( NULL );

			// Paste the contents of the clipboard
			// to the newly created window.
			BCChildWnd *pActive = reinterpret_cast<BCChildWnd *>( MDIGetActive());
			if ( pActive )
				pActive->GetBrainchild()->SendMessage( WM_PASTE );
			break;
		}

		case	IDM_NEW:
		case	IDM_NEWWINDOW:
			// Add a new, empty, document.
			AddDocument( NULL );
			return 0;

		case	IDM_EXIT:
			// Bye, bye...
			SendMessage( WM_CLOSE );
			return 0;

		case	IDM_ABOUT:
		{
			// Popup a messagebox...
			ClsMessageBox mb;
			mb.Icon()    = ClsGetApp()->GetIcon();
			mb.Body()    = ISEQ_LEFT ISEQ_BOLD ISEQ_UNDERLINE
					_T( "Brainchild Demonstration - by Jan van den Baard\n\n" )
				       ISEQ_NORMAL
					_T( "This program demonstrates the usage of the\n" )
					_T( "Brainchild Custom Editor control.\n\n" )
					_T( "This demonstration program and it's source code are\n" )
					_T( "public domain, use and abuse as you desire.\n\n" )
					ISEQ_BOLD ISEQ_TEXTRGB( _T( "0,0,200" ))
					_T( "The Brainchild Custom Control and the Brainchild\n" )
					_T( "Configuration Tool are not public domain.\n\n" )
					ISEQ_TEXTSYSCOLOR( IG_COLOR_BTNTEXT ) ISEQ_NORMAL
					_T( "They are (C) Copyright 1993-2005 by Jan van den Baard." );
			mb.Title()   = _T( "BCDemo." );
			mb.Buttons() = _T( "*&OK" );
			mb.Flags()   = ClsMessageBox::MBF_ICONCUSTOM;
			mb.MsgBox( GetSafeHWND());
			return 0;
		}

		case	IDM_CONFIG:
			// Popup the brainchild settings dialog.
			OpenSettingsDialog( GetSafeHWND());
			return 0;

		case	IDM_XPSTYLEMENUS:
		{
			// Change the XPStyle switch.
			ChangeSwitch( nCtrlID );
			ClsMenuItemInfo mi;
			mi.fMask = MIIM_STATE;
			if ( ::GetMenuItemInfo( GetMenu(), IDM_XPSTYLEMENUS, FALSE, & mi ))
			{
				// Change the menu styles.
				m_ChildMenu.OldStyle() = ( BOOL )( mi.fState & MFS_CHECKED ? FALSE : TRUE );
				m_FrameMenu.OldStyle() = ( BOOL )( mi.fState & MFS_CHECKED ? FALSE : TRUE );
				m_ContextMenu.OldStyle() = ( BOOL )( mi.fState & MFS_CHECKED ? FALSE : TRUE );

				// And the MRU styles.
				m_MRUFiles.XPStyle( ! m_ChildMenu.OldStyle());
				m_HotFiles.XPStyle( ! m_FrameMenu.OldStyle());

				// And the toolbar...
				m_Toolbar.OldStyle() = ( BOOL )( mi.fState & MFS_CHECKED ? FALSE : TRUE );
				m_Toolbar.Invalidate();

				// Rerender menus.
				DrawMenuBar();
			}
			return 0;
		}

		case	IDM_ADDTOHOT:
		{
			// Do we already have an open window?
			ClsMDIChildWindow *pActive = MDIGetActive();

			// Get path string.
			ClsString path;
			if ( pActive )
			{
				path = pActive->GetSafeHWND();
				if ( path.GetStringLength())
				{
					// Make sure we strip the * character if
					// present.
					if ( path[ path.GetStringLength() - 1 ] == _T( '*' ))
						path.SetStringLength( path.GetStringLength() - 1 );

					// Add it to the hot list.
					m_HotFiles.AddMRUEntry( path );
				}
			}
			return 0;
		}
	}
	// Call the base class.
	return ClsMDIMainWindow::OnCommand( nNotifyCode, nCtrlID, hWndCtrl );
}

// WM_CREATE handler.
LRESULT BCFrameWnd::OnCreate( LPCREATESTRUCT pCS )
{
	// First we let the baseclass do it's thing.
	if ( ClsMDIMainWindow::OnCreate( pCS ) == -1 )
		return -1;

	// Create the tab control.
	if ( m_MDITabs.Create( 0, NULL, NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CTCS_TOOLTIPS | CTCS_DRAGREARRANGE | CTCS_SCROLL | CTCS_BOLDSELECTEDTAB, ClsRect( CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT ), *this, NULL ) == FALSE )
		return -1;

	// Setup tab control.
	m_MDITabs.SetScrollRepeat( ClsDotNetTabControl::ectcScrollRepeat_Slow );
	m_MDITabs.SetSystemImageList( true );

	// Setup icons.
	SendMessage( WM_SETICON, ICON_SMALL, ( LPARAM )ClsGetApp()->GetIcon( TRUE ));
	SendMessage( WM_SETICON, ICON_BIG,   ( LPARAM )ClsGetApp()->GetIcon( FALSE ));

	// Load images.
	if (( m_hToolbar = ImageList_LoadBitmap( ClsGetResourceHandle(), MAKEINTRESOURCE( IDB_TOOLBAR ), 16, 0, RGB( 255, 0, 255 ))) == NULL )
		return -1;
	if (( m_hPin = ImageList_LoadBitmap( ClsGetResourceHandle(), MAKEINTRESOURCE( IDB_PIN ), 16, 0, RGB( 255, 0, 255 ))) == NULL )
		return -1;

	// Setup menus.
	if ( m_FrameMenu.Init( MAKEINTRESOURCE( IDR_FRAMEMENU ), m_hToolbar, FALSE, FALSE ) == FALSE )
		return -1;
	if ( m_ChildMenu.Init( MAKEINTRESOURCE( IDR_CHILDMENU ), m_hToolbar, TRUE, FALSE ) == FALSE )
		return -1;
	if ( m_ContextMenu.Init( NULL, m_hToolbar, FALSE, TRUE ) == FALSE )
		return -1;

	// Setup MRU objects. 
	if ( m_MRUFiles.SetupMRU( HKEY_CURRENT_USER, REG_PATH _T( "MRU" ), IDM_MRU_BASE ) == FALSE )
		return -1;
	if ( m_HotFiles.SetupMRU( HKEY_CURRENT_USER, REG_PATH _T( "HOTLIST" ), IDM_HOT_BASE ) == FALSE )
		return -1;

	// We want to see icons in the MRU and
	// hot file menus.
	m_MRUFiles.XPStyle( TRUE ); m_MRUFiles.UseIcons( TRUE );
	m_HotFiles.XPStyle( TRUE ); m_HotFiles.UseIcons( TRUE );

	// Set recent and hot sub-menus.
	ClsMenuItemInfo mi;
	mi.fMask    = MIIM_SUBMENU;
	mi.hSubMenu = m_MRUFiles;
	m_ChildMenu.SetItemInfo( IDM_RECENTFILES, &mi );
	m_FrameMenu.SetItemInfo( IDM_RECENTFILES, &mi );
	m_ContextMenu.SetItemInfo( IDM_RECENTFILES, &mi );
	mi.hSubMenu = m_HotFiles;
	m_ChildMenu.SetItemInfo( IDM_HOTFILES, &mi );
	m_FrameMenu.SetItemInfo( IDM_HOTFILES, &mi );
	m_ContextMenu.SetItemInfo( IDM_HOTFILES, &mi );

	// Create the statusbar and the toolbar.
	if( m_Status.Create( this, 0 ) == FALSE )
		return -1;
	if ( m_Toolbar.Create( this, &m_MRUFiles ) == FALSE )
		return -1;

	// Load keys.
	GetKeys();

	// Set menubar.
	SetupMenu();

	ParseCommandLine();
	return 0;
}

// WM_NOTIFY handler.
LRESULT BCFrameWnd::OnNotify( LPNMHDR pHdr )
{
	static bool bDragging = false;
	switch ( pHdr->code )
	{
		case	CTCN_BEGINITEMDRAG:
			// Were dragging a tab.
			bDragging = true;
			break;

		case	CTCN_CANCELITEMDRAG:
			// No longer dragging.
			bDragging = false;
			break;

		case	CTCN_ACCEPTITEMDRAG:
		{
			// Drag has been accepted. Remove the child from it's
			// old position and insert it at it's new position.
			LPNMCTC2ITEMS pItems = ( LPNMCTC2ITEMS )pHdr;
			BCChildWnd *pChild = m_Childs[ pItems->iItem1 ];
			m_Childs.RemoveAt( pItems->iItem1, 1 );
			m_Childs.InsertAt( &pChild, pItems->iItem2, 1 );

			// No longer dragging.
			bDragging = false;
			return 0;
		}

		case	CTCN_SELCHANGE:
		{
			// Only when we are not dragging...
			if ( bDragging == false )
			{
				// Activate the new selection.
				LPNMCTC2ITEMS pItems = ( LPNMCTC2ITEMS )pHdr;
				MDIActivate( m_Childs[ pItems->iItem2 ] );
			}
			return 0;
		}
	}
	// Call the baseclass.
	return ClsMDIMainWindow::OnNotify( pHdr );
}

// Load settings.
BOOL BCFrameWnd::GetKeys()
{
	// Open registry key.
	ClsRegKey key;
	if ( key.OpenKey( HKEY_CURRENT_USER, REG_PATH ) == ERROR_SUCCESS )
	{
		// Get registry values.
		DWORD bNew = FALSE, bXPStyle = TRUE;
		key.QueryValue( _T( "NewOnStart" ), bNew );
		key.QueryValue( _T( "XPStyleMenus" ), bXPStyle );

		// New file on startup?.
		ClsMenuItemInfo mi;
		mi.fMask  = MIIM_STATE;
		mi.fState = bNew ? MFS_CHECKED : 0;
		m_ChildMenu.SetItemInfo( IDM_NEWONSTART, &mi );
		m_FrameMenu.SetItemInfo( IDM_NEWONSTART,  &mi );

		// XP style menus?
		mi.fState = bXPStyle ? MFS_CHECKED : 0;
		m_ChildMenu.SetItemInfo( IDM_XPSTYLEMENUS, &mi );
		m_FrameMenu.SetItemInfo( IDM_XPSTYLEMENUS, &mi );
		m_ChildMenu.OldStyle() = ! bXPStyle;
		m_FrameMenu.OldStyle() = ! bXPStyle;
		m_ContextMenu.OldStyle() = ! bXPStyle;
		m_MRUFiles.XPStyle( bXPStyle );
		m_HotFiles.XPStyle( bXPStyle );
		m_Toolbar.OldStyle() = ! bXPStyle;

		// Refresh the menu bar.
		DrawMenuBar();

		// Add a new document if requested.
		if ( bNew )
			AddDocument( NULL );

		// Get maximized switch.
		if ( key.QueryValue( _T( "ChildMaximized" ), m_dwChildMax ) == ERROR_SUCCESS )
			return TRUE;
	}
	return FALSE;
}

// Change setting.
BOOL BCFrameWnd::ChangeSwitch( UINT nID )
{
	// Get item state of the menu currently
	// active on the frame.
	ClsMenuItemInfo mi;
	mi.fMask = MIIM_STATE;
	if ( ::GetMenuItemInfo( GetMenu(), nID, FALSE, &mi ))
	{
		// Flip checked bit.
		mi.fState ^= MFS_CHECKED;

		// Update both the frame and the child menu.
		if ( m_ChildMenu.SetItemInfo( nID, &mi ) && m_FrameMenu.SetItemInfo( nID, &mi ))
		{
			// Open the registry key.
			ClsRegKey key;
			if ( key.OpenKey( HKEY_CURRENT_USER, REG_PATH ) == ERROR_SUCCESS )
			{
				// Determine the switch which need to be changed.
				ClsString val;
				switch ( nID )
				{
					case	IDM_NEWONSTART:
						val = _T( "NewOnStart" );
						break;

					case	IDM_XPSTYLEMENUS:
						val = _T( "XPStyleMenus" );
						break;
				}

				// Set the new value.
				key.SetValue(( DWORD )( mi.fState & MFS_CHECKED ? TRUE : FALSE ), val );
				key.CloseKey();
				return TRUE;
			}
		}
	}
	return FALSE;
}

// File already opened?
BOOL BCFrameWnd::AlreadyOpen( LPCTSTR pszFilename )
{
	// Iterate children...
	ClsString name;
	for ( int i = 0; i < m_Childs.GetSize(); i++ )
	{
		// Valid window?
		if ( m_Childs[ i ]->GetSafeHWND())
		{
			// Obtain the title of the child window. This is the
			// full path of the file.
			name = m_Childs[ i ]->GetSafeHWND();

			// If it is modified we remove the "*" character
			// from the name.
			if ( name[ name.GetStringLength() - 1 ] == _T( '*' ))
				name.SetStringLength( name.GetStringLength() - 1 );

			// Found a match?
			if ( name.CompareNoCase( pszFilename ) == 0 )
			{
				// Yes. Activate it by activating
				// it's tab.
				m_MDITabs.SetCurSel( i );
				return TRUE;
			}
		}
	}
	// Not found.
	return FALSE;
}

// Convert a 8.3 name to the full name.
//
// Does not convert the directory name,
// only the file name.
void BCFrameWnd::ConvertShortToFull( LPCTSTR pszFileName )
{
	// Find the file, this will actually convert
	// a 8.3 file name to a full name.
	ClsFindFile find;
	if ( find.FindFile( pszFileName ))
	{
		// Find the filename part of the path.
		LPTSTR pszPtr = ::PathFindFileName( pszFileName );
		if ( pszPtr )
		{
			// Get the converted filename.
			ClsString file;
			if ( find.GetFileName( file ))
				// Overwrite the original filename.
				_tcscpy( pszPtr, file );
		}
		// Close the handle.
		find.Close();
	}
}

// Parse the command line.
void BCFrameWnd::ParseCommandLine()
{
	LPTSTR	        pszPtr = ClsGetApp()->GetCmdLine(), pszPtr1;
	TCHAR		szFileName[ MAX_PATH ];
	BOOL		bQuoted;
	int		nCount;

	// Iterate command line.
	for( ; ; )
	{
		// Skip blanks.
		while ( isspace( *pszPtr ) && *pszPtr != '\0' ) pszPtr++;
		if ( *pszPtr == '\0' ) return;

		// Reset counter.
		nCount = 1;

		// Quoted argument? Dropping files
		// on the program icon is done differently
		// on Window 95/98 and Windows 2000.
		//
		// On Windows 95/98 the path of the dropped
		// files is on 8.3 form. This means that there
		// are no spaces in the path and the files
		// are seperated by a space.
		//
		// On Windows 2000 the full path of the dropped
		// files is used and the names are enclosed
		// in double-quotes.
		if ( *pszPtr == '"' )
		{
			// Yes...
			bQuoted = TRUE;
			pszPtr++;
		}
		else
			// Not a quoted argument.
			bQuoted = FALSE;

		// Copy string base.
		pszPtr1 = pszPtr;

		// Count next argument length...
		while ( *pszPtr && nCount < MAX_PATH )
		{
			// Qouted arg?
			if ( bQuoted )
			{
				// Is it a quote?
				if ( *pszPtr == '"' )
				{
					// Skip quote and
					// continue.
					pszPtr++;
					break;
				}
			}
			else
			{
				// space?
				if ( isspace( *pszPtr ))
					break;
			}

			// Adjust counters.
			pszPtr++;
			nCount++;
		}

		// Copy the pathname and convert
		// it's filename part to a long
		// file name.
		ZeroMemory( szFileName, MAX_PATH * sizeof( TCHAR ));
#ifndef _UNICODE
		// Straight character copy.
		strncpy( szFileName, pszPtr1, nCount );
#else
		// Wide character conversion.
		MultiByteToWideChar( CP_ACP, 0, pszPtr1, nCount, szFileName, MAX_PATH );
#endif
		// Perform 8.3 to fullname conversion if necessary.
		ConvertShortToFull( szFileName );

		// Clear quoted boolean.
		bQuoted = FALSE;

		// Open the file.
		AddDocument( szFileName );

		// Done?
		if ( *pszPtr == '\0' )
			break;
	}
}