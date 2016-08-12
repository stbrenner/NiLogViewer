//-----------------------------------------------------------------------------
//
//    BcFrame.cpp
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "logfiledialog.h"
#include "fastfinddlg.h"
#include "logfolderdlg.h"
#include "cmdlineargs.h"
#include "HyperlinkHandler.h"
#include "LogFolderConn.h"
#include "LogFolderEraser.h"
#include "EmailSender.h"

// Constructor.
BCFrameWnd::BCFrameWnd() :
  m_pSplitter(NULL), m_pSidebarDlg(NULL), m_pFastFindDlg(NULL),
  m_dwSplitterWidth(501), m_bAssociateWithLogExt(FALSE), m_bScrollToEndOnFileLoad(FALSE)
{
}

// Destructor. Destroys the MDI childs which were
// still present in the array.
BCFrameWnd::~BCFrameWnd()
{
	// Delete imagelists.
	if ( m_hToolbar ) ImageList_Destroy( m_hToolbar );

	// Empty child aray.
	for ( int i = 0; i < m_Childs.GetSize(); i++ )
		delete m_Childs[ i ];

  if (m_pSplitter)
    delete m_pSplitter;

  if (m_pSidebarDlg)
    delete m_pSidebarDlg;

  if (m_pFastFindDlg)
    delete m_pFastFindDlg;
}

void BCFrameWnd::SetLogFolder(LPCTSTR lpsz) 
{
  _tcsncpy(m_szLogFolder, lpsz, MAX_PATH); 
  if (m_pSidebarDlg)
    m_pSidebarDlg->SetLogFolder(lpsz);
}

// Add a new document.
void BCFrameWnd::AddDocument( LPCTSTR pszFilename )
{
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
  
  // Already open in one of the other childs?
  if ( AlreadyOpen( path ))
    return;

  // Dynamically create a new child and add it to the
	// storage array.
	try
	{
		BCChildWnd *child = new BCChildWnd;
		int nPos = m_Childs.Add( child );

		// Tell it about us.
		child->SetFrameWindow( this );

		// Does the file exist?
		ClsFile file;
		if ( pszFilename && _tcslen( pszFilename ))
		{
			try
			{
				// Open the file. Throws an exception uppon an error.
				file.Open( path, ClsFile::fileRead | ClsFile::fileShareRead | ClsFile::fileShareWrite );
			}
			catch( ClsFileException& fe )
			{
				ClsString body;
				// Not found?
				if ( fe.m_nCause == ERROR_FILE_NOT_FOUND )
				{
					// Popup a message box.
					body.Format( _T( "\"%s\"\n\ndoes not exist." ), ( LPCTSTR )path );
					MessageBox( body, _T( "NI Log Viewer" ), MB_ICONINFORMATION | MB_OK );

          // Remove it from the MRU list.
					m_MRUFiles.RemoveMRUEntry( path );

					// Remove and destroy the child.
					m_Childs.RemoveAt( nPos, 1 );
					delete child;

					// Remove it from the MRU.
					m_MRUFiles.RemoveMRUEntry( path );
					return;
				}
				else
				{
					// Show the error description.
					body.Format( _T( "\"%s\"\n\ncould not be opened.\nReason:\n\n%s" ), ( LPCTSTR )path, ( LPCTSTR )fe.m_sDescription );
					MessageBox( body, _T( "NI Log Viewer" ), MB_ICONERROR | MB_OK );

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
			// Create window.
			if ( child->Create( path, WS_VISIBLE | WS_MAXIMIZE, ClsRect( CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT ), this ))
			{
				// Multipart statusbar.
				m_Status.MultiPart( TRUE );

				// Get the index of the icon.
				int nIndex = -1;
				SHFILEINFO fi;
				if ( ::SHGetFileInfo( path, FILE_ATTRIBUTE_NORMAL, &fi, sizeof( fi ), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES ))
					nIndex = fi.iIcon;

				// Destroy the icon.
				::DestroyIcon( fi.hIcon );

				// Add the tab.
				m_MDITabs.InsertItem( m_MDITabs.GetItemCount(), ::PathFindFileName( path ), nIndex, path, true );
				if ( m_MDITabs.IsWindowVisible() == FALSE && m_MDITabs.GetItemCount() > 1 )
				{
					m_MDITabs.ShowWindow( SW_SHOW );
					ClsRect rc = GetClientRect();
					OnSize( 0, rc.Width(), rc.Height());
				}
        
        // Do we have a filename?
				if ( pszFilename )
				{
					// Yes, load the file.
					child->m_Brainchild.LoadFile( path );

					// Add it to the MRU list.
					m_MRUFiles.AddMRUEntry( path );
				}
				else
					// Give it a name...
					child->m_Brainchild.SetFilename( path );

				// Update file information.
				child->UpdateFileInfo();
        child->m_Brainchild.SetReadonly();

        // Not more then 10 log windows!
        if (m_Childs.GetSize() > 10)
          m_Childs[0]->MDIDestroy();

        if (m_bScrollToEndOnFileLoad)
        {
    			BCChildWnd *pActive = reinterpret_cast< BCChildWnd * >( MDIGetActive());
          pActive->m_Brainchild.GotoLine(pActive->m_Brainchild.NumberOfLines());
        }

        return;
			}
		}
	}
	catch( ClsMemoryException& me )
	{
		// Show the error description.
		ClsString body;
		body.Format( _T( "Unable to create document.\nReason:\n\n%s" ), ( LPCTSTR )me.m_sDescription );
		MessageBox( body, _T( "NI Log Viewer" ), MB_ICONERROR | MB_OK );
	}
}

// Quit...
void BCFrameWnd::Quit()
{
	// Iterate the children.
	for ( int i = 0; i < m_Childs.GetSize(); i++ )
	{
		// Destroy it.
		m_Childs[ i ]->MDIDestroy();

		// Recursively call Quit() until we are done
		// or until we are cancelled.
		Quit();
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
    case WM_OPEN_LOG_FILE:
    {
      TCHAR szCmdLine[10000];

	    ClsRegKey key;
	    if (key.OpenKey( HKEY_CURRENT_USER, REG_PATH ) == ERROR_SUCCESS )
	    {
        DWORD dwSize = 10000;
        for (int i = 0; i < 3; i++)   // Only for safety reasons, that everything is in the registry
        {
		      key.QueryValue(_T("LastCmdLine"), szCmdLine, &dwSize);
          szCmdLine[9999] = 0;
          if (lParam == (long)_tcslen(szCmdLine))   // lParam contains the string length
            break;
          Sleep(100);
        }
		    key.CloseKey();
	    }

      SetForegroundWindow();
      BringWindowToTop();
      ParseCommandLine(szCmdLine);
      return 1;
    }
    case WM_ANIMATE_DOWNLOAD:
      if (!m_pSidebarDlg)
        break;
      m_pSidebarDlg->SetAnimateDownload(lParam);
      break;
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

      // Any children left?
			if ( m_Childs.GetSize() <= 1 )
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
  if (m_pSplitter)
  {
    m_pSidebarDlg->SendMessage(WM_CLOSE);
    m_pSidebarDlg->Destroy();
    delete m_pSidebarDlg;
    m_pSidebarDlg = NULL;
    if (m_pSplitter->GetSplitterPosition() > MIN_SIDEBAR_SIZE)
      m_dwSplitterWidth = m_pSplitter->GetSplitterPosition();
    m_pSplitter->Destroy();
    delete m_pSplitter;
    m_pSplitter = NULL;
  }

	// Write important values to the registry
	ClsRegKey key;
	if ( key.OpenKey( HKEY_CURRENT_USER, REG_PATH ) == ERROR_SUCCESS )
	{
		// Set values.
		key.SetValue((DWORD)m_dwSplitterWidth, _T("SplitterWidth"));
		key.SetValue((DWORD)IsZoomed(), _T("FrameMaximized"));
		key.SetValue((DWORD)m_bAssociateWithLogExt, _T("AssiciateWithLogExt"));
		key.SetValue((DWORD)m_bScrollToEndOnFileLoad, _T("ScrollToEndOnFileLoad"));
		key.CloseKey();
	}

  WriteFolderToReg();

	// Save the MRU and hotlist.
	m_MRUFiles.SaveMRUList();

	// Bye...
	Quit();

	return TRUE;
}

void BCFrameWnd::ReadFolderFromReg()
{
  TCHAR szLogFolder[MAX_PATH];

  // Fallback option: Hardcoded folder
  TCHAR szProgFilesFolder[MAX_PATH];
  SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, 
    szProgFilesFolder);
  _stprintf(szLogFolder, _T("%s\\NetInst\\NiLogs"), szProgFilesFolder);

  //Read folder from NetInstall's log file settings in the registry
  HKEY hNiReg;
  DWORD lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
    _T("SOFTWARE\\NetSupport\\NetInstall\\LogFileSettings\\"), NULL, KEY_READ, 
    &hNiReg);

  if (lRet == ERROR_SUCCESS)
  {
    DWORD dwType;
    DWORD dwSize = MAX_PATH;
    lRet = RegQueryValueEx(hNiReg, _T("ResolvedLogFilePath"), NULL, &dwType,
      (LPBYTE)szLogFolder, &dwSize);
    RegCloseKey(hNiReg);
  }

  SetLogFolder(szLogFolder);
}

///
/// Writes the currently open log folder to NiLogVwr's registry section
///
void BCFrameWnd::WriteFolderToReg()
{
  HKEY hLogVwrReg;
  DWORD dwDisposition;
  LONG lRet;

  lRet = RegCreateKeyEx(HKEY_CURRENT_USER, REG_PATH, NULL, NULL, 
    REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hLogVwrReg, &dwDisposition);

  if (lRet != ERROR_SUCCESS)
    return;

  lRet = RegSetValueEx(hLogVwrReg, _T("LastLogFolder"), NULL, REG_SZ, 
    (const BYTE*)m_szLogFolder, _tcslen(m_szLogFolder)*sizeof(TCHAR));
  RegCloseKey(hLogVwrReg);
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

  if (m_pSplitter)
  {
    m_pSplitter->SetSplitRect(rcClient);
    m_pSplitter->RedrawPanes();
  }
  else
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
      if (!pActive)
        return 0;
      else
				pActive->MDIDestroy();
			return 0;
		}

		case	IDM_CLOSEALL:
		{
			// Iterate the children.
			while ( m_Childs.GetSize())
			{
				m_Childs[ 0 ]->MDIDestroy();
			}

			return 0;
		}

		case	IDM_NEXTWND:
    {
      int cur = m_MDITabs.GetCurSel();
      cur++;
      if (cur < m_MDITabs.GetItemCount())
        m_MDITabs.SetCurSel(cur);
      else
        m_MDITabs.SetCurSel(0);
			return 0;
    }

    case	IDM_PREVWND:
    {
      int cur = m_MDITabs.GetCurSel();
      cur--;
      if (cur >= 0)
        m_MDITabs.SetCurSel(cur);
      else
        m_MDITabs.SetCurSel(m_MDITabs.GetItemCount()-1);
			return 0;
    }

    case IDM_SEND_AS_EMAIL:
    {
      ClsString strActiveDoc = MDIGetActive()->GetSafeHWND();
      CEmailSender oEmailSender;
      oEmailSender.Send(strActiveDoc);
      break;
    }
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

    case IDM_SHOWSIDEBAR:
      if (m_pSplitter->GetSplitterPosition() > MIN_SIDEBAR_SIZE)
        MinimizeSplitter();
      else
        MaximizeSplitter();
      break;

    case IDM_COMMANDLINEPARAMETERS:
      MessageBox(_T("NiLogVwr.exe [Parameters]\n\n")
        _T("Parameters (also in combinations):\n")
        _T("-computer <ComputerName>\t\tOpens the active log folder on a remote computer\n")
        _T("-folder <LogFolder>\t\tOpens a log folder (UNC or local)\n")
        _T("-filter <FilterString>\t\tFilters the log files after a certain name\n")
        _T("<LogFilePath or LogFolder>\t\tOpens a log file or a log folder\n"), _T("Command Line Parameters"));
      break;

    case IDM_REGISTERFILEEXTENSION:
      SwitchConfigFlag(IDM_REGISTERFILEEXTENSION, m_bAssociateWithLogExt); 
      break;

    case IDM_SCROLL_TO_END_ON_FILE_LOAD:
      SwitchConfigFlag(IDM_SCROLL_TO_END_ON_FILE_LOAD, m_bScrollToEndOnFileLoad); 
      break;

    case	IDM_OPEN:
		{
			// Do we already have an open window?
			ClsMDIChildWindow *pActive = MDIGetActive();

			// get path string.
			ClsString path;
			if ( pActive )
				path = pActive->GetSafeHWND();

			// Popup file dialog.
			CLogFileDialog fd;
			fd.CenterOnParent() = TRUE;
			if ( fd.DoModal( this, path, NULL, 1, OFN_HIDEREADONLY ))
			{
				// Obtain the file name.
				ClsString file;
				if ( fd.GetName( 0, file ))
					// Open the document.
					AddDocument( file );
			}
			return 0;
		}

		case	IDM_EXIT:
			SendMessage( WM_CLOSE );
			return 0;

		case  IDM_RELOAD_FILE:
    {
			BCChildWnd *pActive = reinterpret_cast< BCChildWnd * >( MDIGetActive());

			int iLine = pActive->m_Brainchild.GetLineNumber();
			ClsString strActiveDoc = MDIGetActive()->GetSafeHWND();
      pActive->m_Brainchild.LoadFile(strActiveDoc, TRUE);

      if (m_bScrollToEndOnFileLoad)
        pActive->m_Brainchild.GotoLine(pActive->m_Brainchild.NumberOfLines());
      else
        pActive->m_Brainchild.GotoLine(iLine);

      return 0;
    }

		case	IDM_ABOUT:
		{
      ClsMessageBox mb;
      mb.Icon()    = ClsGetApp()->GetIcon();
      mb.Body()    = ISEQ_LEFT ISEQ_BOLD
        _T( "NI Log Viewer " ) _T(VER_PRODUCT) _T("\n" )
        ISEQ_NORMAL
        _T( "Freeware Event Log Viewer for enteo NetInstall and HEAT DSM\n\n" )
        _T( "Copyright (C) 2005-2016 Stephan Brenner\n\n" )
        _T( "For more information and updates visit " )
        ISEQ_TEXTRGB( _T( "0,0,255" ))
        _T( "http://www.stephan-brenner.com" )
        ISEQ_NORMAL ISEQ_TEXTSYSCOLOR( IG_COLOR_BTNTEXT );
      mb.Title()   = _T( "NI Log Viewer" );
      mb.Buttons() = _T( "*&OK" );
      mb.Flags()   = ClsMessageBox::MBF_ICONCUSTOM;
      mb.MsgBox( GetSafeHWND());
      return 0;
		}

    case IDM_FINDINFILES:
      if (!m_pFastFindDlg)
      {
        m_pFastFindDlg = new FastFindDlg();
        m_pFastFindDlg->SetFrameWindow(this);
        m_pFastFindDlg->Create(*this, MAKEINTRESOURCE(IDD_FIF));
      }
      else
        m_pFastFindDlg->ShowWindow(SW_SHOW);

      return 0;

    case IDM_CHANGE_LOGFOLDER:
    {
      TCHAR szLogFolder[MAX_PATH];
      GetLogFolder(szLogFolder, MAX_PATH);

   		CLogFolderDlg	dlg;
      dlg.SetLogFolder(szLogFolder);
  		if (dlg.DoModal( *this, IDD_LOG_FOLDER ) == IDOK)
      {
        dlg.GetLogFolder(szLogFolder, MAX_PATH);
        SetLogFolder(szLogFolder);
      }
      break;
    }
    case IDM_DELETE_ALL_FILES:
			DeleteLogFiles(0, _T("Do you really want to delete all log files?"));
			break;
    case IDM_DELETE_OLDER_ONE_MINUTE:
			DeleteLogFiles(60, _T("Do you really want to delete all log files that are older than one minute?"));
			break;
    case IDM_DELETE_OLDER_ONE_HOUR:
			DeleteLogFiles(60*60, _T("Do you really want to delete all log files that are older than one hour?"));
			break;
    case IDM_DELETE_OLDER_ONE_DAY:
			DeleteLogFiles(60*60*24, _T("Do you really want to delete all log files that are older than one day?"));
			break;
    case IDM_DELETE_OLDER_ONE_WEEK:
			DeleteLogFiles(60*60*24*7, _T("Do you really want to delete all log files that are older than one week?"));
			break;
    case IDM_DELETE_OLDER_ONE_MONTH:
			DeleteLogFiles(60*60*24*30, _T("Do you really want to delete all log files that are older than one month?"));
			break;
	}

	// Call the base class.
	return ClsMDIMainWindow::OnCommand( nNotifyCode, nCtrlID, hWndCtrl );
}

///
/// Executes the delete. Prior to that action a message box is displayed.
///
void BCFrameWnd::DeleteLogFiles(IN int iSeconds, IN LPCTSTR lpszMessage)
{
	if (::MessageBox(GetSafeHWND(), lpszMessage, _T("Delete Log Files"), MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	TCHAR szLogFolder[MAX_PATH];
	GetLogFolder(szLogFolder, MAX_PATH);

	CLogFolderEraser oLogFolderEraser(szLogFolder);
	DWORD dwReturn = oLogFolderEraser.Delete(iSeconds);

	if (dwReturn != NO_ERROR)
	{
		ReportError(GetSafeHWND(), dwReturn);
	}
}

/// 
/// Switches a configuration entry in the menu
///
void BCFrameWnd::SwitchConfigFlag(IN DWORD dwMenuId, IN BOOL& bConfigValue)
{
  bConfigValue = !bConfigValue;
  if (bConfigValue)
  {
    m_ChildMenu.CheckItem(dwMenuId, MF_CHECKED);
    RegisterFileExtension();
  }
  else
  {
    m_ChildMenu.CheckItem(dwMenuId, MF_UNCHECKED);
  }
}

void BCFrameWnd::RegisterFileExtension()
{
  HKEY hKey = 0;
  LONG lRet = 0;
  DWORD dwType = 0;
  DWORD dwSize = 0;
  DWORD dwDisposition = 0;
  BOOL bRootLevelCreated = FALSE;
  BOOL bAssociationCreated = FALSE;

  TCHAR szModuleFileName[MAX_PATH];
  GetModuleFileName(NULL, szModuleFileName, MAX_PATH);
  TCHAR szModuleBaseName[MAX_PATH];
  TCHAR szFName[_MAX_FNAME];
  TCHAR szExt[_MAX_EXT];
  _tsplitpath(szModuleFileName, NULL, NULL, szFName, szExt);
  _stprintf(szModuleBaseName, _T("%s%s"), szFName, szExt);

  // Root entry for the application
  TCHAR szKey[MAX_PATH];
  _stprintf(szKey, _T("Applications\\%s\\shell\\open\\command"), szModuleBaseName);
  lRet = RegCreateKeyEx(HKEY_CLASSES_ROOT, 
    szKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE,
    NULL, &hKey, &dwDisposition);

  if (lRet == ERROR_SUCCESS)
  {
    TCHAR szOpenCmd[MAX_PATH];
    _stprintf(szOpenCmd, _T("\"%s\" \"%%1\""), szModuleFileName);
    lRet = RegSetValueEx(hKey, _T(""), NULL, REG_SZ, 
      (const BYTE*)szOpenCmd, _tcslen(szOpenCmd)+1);
    RegCloseKey(hKey);
  }

  if (!m_bAssociateWithLogExt)
    return;

  // User entry for the application
  _stprintf(szKey, _T("Software\\Classes\\Applications\\%s\\shell\\open\\command"), szModuleBaseName);
  lRet = RegCreateKeyEx(HKEY_CURRENT_USER, 
    szKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE,
    NULL, &hKey, &dwDisposition);

  if (lRet == ERROR_SUCCESS)
  {
    TCHAR szOpenCmd[MAX_PATH];
    _stprintf(szOpenCmd, _T("\"%s\" \"%%1\""), szModuleFileName);
    lRet = RegSetValueEx(hKey, _T(""), NULL, REG_SZ, 
      (const BYTE*)szOpenCmd, _tcslen(szOpenCmd)+1);
    RegCloseKey(hKey);
  }

  // Root entry for file type
  lRet = RegCreateKeyEx(HKEY_CLASSES_ROOT, 
    _T("StephanBrenner.LogFile\\shell\\open\\command"), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE,
    NULL, &hKey, &dwDisposition);

  if (lRet == ERROR_SUCCESS)
  {
    TCHAR szOpenCmd[MAX_PATH];
    _stprintf(szOpenCmd, _T("\"%s\" \"%%1\""), szModuleFileName);
    lRet = RegSetValueEx(hKey, _T(""), NULL, REG_SZ, 
      (const BYTE*)szOpenCmd, _tcslen(szOpenCmd)+1);
    RegCloseKey(hKey);
    bRootLevelCreated = TRUE;
  }

  // Root entry for file description
  lRet = RegCreateKeyEx(HKEY_CLASSES_ROOT, 
    _T("StephanBrenner.LogFile"), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE,
    NULL, &hKey, &dwDisposition);

  if (lRet == ERROR_SUCCESS)
  {
    lRet = RegSetValueEx(hKey, _T(""), NULL, REG_SZ, 
      (const BYTE*)_T("Log File"), _tcslen(_T("Log File"))+1);
    RegCloseKey(hKey);
  }

  // Icon on root level
  lRet = RegCreateKeyEx(HKEY_CLASSES_ROOT, _T("StephanBrenner.LogFile\\DefaultIcon"),
    NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, 
    &dwDisposition);

  if (lRet == ERROR_SUCCESS)
  {
    TCHAR szIcon[MAX_PATH];
    _stprintf(szIcon, _T("%s,1"), szModuleFileName);

    lRet = RegSetValueEx(hKey, _T(""), NULL, REG_SZ, 
      (const BYTE*)szIcon, _tcslen(szIcon)+1);
    RegCloseKey(hKey);
  }

  if (!m_bAssociateWithLogExt)
    return;

  // Association between Ext. and Log Viewer on root level
  if (bRootLevelCreated)
  {
    lRet = RegCreateKeyEx(HKEY_CLASSES_ROOT, 
      _T(".log"),
      NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, 
      &dwDisposition);

    if (lRet == ERROR_SUCCESS)
    {
      lRet = RegSetValueEx(hKey, _T(""), NULL, REG_SZ, 
        (const BYTE*)_T("StephanBrenner.LogFile"), _tcslen(_T("StephanBrenner.LogFile"))+1);
      RegCloseKey(hKey);
    }
  }

  // Association between Ext. and Log Viewer on user level
  lRet = RegCreateKeyEx(HKEY_CURRENT_USER, 
    _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.log"),
    NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL, &hKey, 
    &dwDisposition);

  if (lRet == ERROR_SUCCESS)
  {
    dwSize = MAX_PATH;
    TCHAR szTemp[MAX_PATH];
    lRet = RegQueryValueEx(hKey, _T("Application"), NULL, &dwType,
      (LPBYTE)szTemp, &dwSize);
    if (_stricmp(szTemp, szModuleBaseName))
      bAssociationCreated = TRUE;

    lRet = RegSetValueEx(hKey, _T("Application"), NULL, REG_SZ, 
      (const BYTE*)szModuleBaseName, _tcslen(szModuleBaseName)+1);
    RegCloseKey(hKey);
  }

  // Notify shell of change  
  if (bAssociationCreated)
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
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

	// Setup menus.
	if ( m_ChildMenu.Init( MAKEINTRESOURCE( IDR_CHILDMENU ), m_hToolbar, TRUE, FALSE ) == FALSE )
		return -1;
	if ( m_ContextMenu.Init( NULL, m_hToolbar, FALSE, TRUE ) == FALSE )
		return -1;
  MDISetMenu( &m_ChildMenu, NULL );

	// Setup MRU objects.
	if ( m_MRUFiles.SetupMRU( HKEY_CURRENT_USER, REG_PATH _T( "MRU" ), IDM_MRU_BASE ) == FALSE )
		return -1;

	// If more than 256 colors, then enable XP style menues. Not supported by Vista.
  DEVMODE DevMode;
  DevMode.dmSize=sizeof(DEVMODE);
  if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &DevMode) && DevMode.dmBitsPerPel > 8 &&
	  !IsWindowsVista())
  {
  	m_MRUFiles.XPStyle( TRUE ); 
  }
  else
  {
    m_ChildMenu.OldStyle() = TRUE;
    m_ContextMenu.OldStyle() = TRUE;
  }

	// Set recent and hot sub-menus.
	ClsMenuItemInfo mi;
	mi.fMask    = MIIM_SUBMENU;
	mi.hSubMenu = m_MRUFiles;
	m_ChildMenu.SetItemInfo( IDM_RECENTFILES, &mi );

	// Create the statusbar and the toolbar.
	if( m_Status.Create( this, 0 ) == FALSE )
		return -1;
	if ( m_Toolbar.Create( this, &m_MRUFiles ) == FALSE )
		return -1;

	GetKeys();
  RegisterFileExtension();

  if (m_bAssociateWithLogExt)
    m_ChildMenu.CheckItem(IDM_REGISTERFILEEXTENSION, MF_CHECKED);

  if (m_bScrollToEndOnFileLoad)
    m_ChildMenu.CheckItem(IDM_SCROLL_TO_END_ON_FILE_LOAD, MF_CHECKED);

  // Create Sidebar
  m_pSidebarDlg = new CSidebarDlg;
  m_pSidebarDlg->SetFrameWindow(this);
  m_pSidebarDlg->Create(*this, MAKEINTRESOURCE(IDD_SIDEBAR));

  m_pSplitter = new ClsSplitter;
  m_pSplitter->Create( this, 0 );
  m_pSplitter->SetPanes(m_pSidebarDlg->GetSafeHWND(), GetMDIClient()->GetSafeHWND());
  m_pSplitter->SetPaneMinSize( MIN_SIDEBAR_SIZE, 100 );
  MaximizeSplitter();

  ParseCommandLine(ClsGetApp()->GetCmdLine());

  m_pSidebarDlg->SetLogFolder(m_szLogFolder);
	return 0;
}

BOOL BCFrameWnd::IsWindowsVista()
{
	OSVERSIONINFO osvi;
	bool bIsWindowsXPorLater;

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx (&osvi);
	return osvi.dwMajorVersion >= 6;
}

// Load settings.
BOOL BCFrameWnd::GetKeys()
{
	ClsRegKey key;
	if ( key.OpenKey( HKEY_CURRENT_USER, REG_PATH ) == ERROR_SUCCESS )
	{
		key.QueryValue(_T("SplitterWidth"), m_dwSplitterWidth);
		key.QueryValue(_T("AssiciateWithLogExt"), (DWORD&)m_bAssociateWithLogExt);
		key.QueryValue(_T("ScrollToEndOnFileLoad"), (DWORD&)m_bScrollToEndOnFileLoad);
    key.CloseKey();
	}

  ReadFolderFromReg();

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
				// Yes. Activate it.
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

static const DWORD CMD_LINE_COMPUTER  = 0x01;
static const DWORD CMD_LINE_FOLDER    = 0x02;
static const DWORD CMD_LINE_HYPERLINK = 0x03;
static const DWORD CMD_LINE_FILTER    = 0x04;

// Parse the command line.
void BCFrameWnd::ParseCommandLine(LPCTSTR pszPtr)
{
	DWORD dwReturn = NO_ERROR;
  DWORD dwNextCommand = 0;
  CmdLineArgs args(pszPtr);
  CLogFolderConn lfc(GetSafeHWND());
  
  for (unsigned int argpos = 0; argpos < args.size(); argpos++)
  {
    LPTSTR currarg = args[argpos];

    switch (dwNextCommand)
    {
    case CMD_LINE_COMPUTER:
    {
      LPTSTR lpszCompName = currarg;
      tcstrim(&lpszCompName, _T('"'));   

			dwReturn = lfc.ConnectToComputer(lpszCompName);

			if (dwReturn == ERROR_NI_REG_SECTION_NOT_FOUND)
			{
				ErrorMessageBox(m_hWnd, _T("Cannot open NetInstall section in registry!"));
				continue;
			}
      else if (dwReturn != NO_ERROR)
      {
				ReportError(GetSafeHWND(), dwReturn);
				continue;
			}

      TCHAR szLogFolder[MAX_PATH];
      lfc.GetLogFolder(szLogFolder, MAX_PATH);
      SetLogFolder(szLogFolder);

			dwNextCommand = 0;
      continue;
    }
    case CMD_LINE_FOLDER:
    {
      LPTSTR lpszFolder = currarg;
      tcstrim(&lpszFolder, _T('"'));   
    
			dwReturn = lfc.ConnectToFolder(lpszFolder);

      if (dwReturn != NO_ERROR)
      {
				ReportError(GetSafeHWND(), dwReturn);
				continue;
			}
			
      TCHAR szLogFolder[MAX_PATH];
      lfc.GetLogFolder(szLogFolder, MAX_PATH);
      SetLogFolder(szLogFolder);

			dwNextCommand = 0;
      continue;
    }
    case CMD_LINE_HYPERLINK:
    {
			dwNextCommand = 0;

			LPTSTR lpszHyperlink = currarg;
      tcstrim(&lpszHyperlink, _T('"'));   

			CHyperlinkHandler oHyperlinkHandler(GetSafeHWND(), lpszHyperlink);

			if (oHyperlinkHandler.HasExtension(_T(".log")))
			{
				if (oHyperlinkHandler.HasFolder())
				{
					AddDocument(lpszHyperlink);
					continue;
				}

				TCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
				_tsplitpath(lpszHyperlink, NULL, NULL, szFName, szExt);

				ClsString strActiveDoc = MDIGetActive()->GetSafeHWND();
				_tsplitpath(strActiveDoc, szDrive, szDir, NULL, NULL);

				TCHAR szPath[MAX_PATH];
				_stprintf(szPath, _T("%s%s%s%s"), szDrive, szDir, szFName, szExt);
				AddDocument(szPath);
				continue;
			}
		
			DWORD dwReturn = oHyperlinkHandler.Execute();
			
			if (dwReturn != NO_ERROR)
			{
				ReportError(GetSafeHWND(), dwReturn);
			}

			continue;
    }
    case CMD_LINE_FILTER:
    {
      LPTSTR lpszFilter = currarg;
      tcstrim(&lpszFilter, _T('"'));   
    
      if (m_pSidebarDlg)
        m_pSidebarDlg->SetFilter(lpszFilter);
      dwNextCommand = 0;
      continue;
    }
    default:
      dwNextCommand = 0;
    }

    if (!_tcsicmp(currarg, _T("-computer")) || !_tcsicmp(currarg, _T("/computer")))
    {
      dwNextCommand = CMD_LINE_COMPUTER;
      continue;
    }
    else if (!_tcsicmp(currarg, _T("-folder")) || !_tcsicmp(currarg, _T("/folder")))
    {
      dwNextCommand = CMD_LINE_FOLDER;
      continue;
    }
    else if (!_tcsicmp(currarg, _T("-hyperlink")) || !_tcsicmp(currarg, _T("/hyperlink")))
    {
      dwNextCommand = CMD_LINE_HYPERLINK;
      continue;
    }
    else if (!_tcsicmp(currarg, _T("-filter")) || !_tcsicmp(currarg, _T("/filter")))
    {
      dwNextCommand = CMD_LINE_FILTER;
      continue;
    }
    else if (!_tcsncicmp(currarg, _T("-"), 1) || !_tcsncicmp(currarg, _T("/"), 1))
      continue;

    // Folder passed (without tag -folder)
    TCHAR szExt[_MAX_EXT];
    _tsplitpath(currarg, NULL, NULL, NULL, szExt);

    if(_tcsicmp(szExt, _T(".log")))
    {
      LPTSTR lpszFolder = currarg;
      tcstrim(&lpszFolder, _T('"'));   
    
      CLogFolderConn lfc(GetSafeHWND());
      if (lfc.ConnectToFolder(lpszFolder))
      {
        TCHAR szLogFolder[MAX_PATH];
        lfc.GetLogFolder(szLogFolder, MAX_PATH);
        SetLogFolder(szLogFolder);
      }
      continue;
    }

    // Path to log file is passed
    LPTSTR lpszFileName = currarg;
    tcstrim(&lpszFileName, _T('"'));   
		ConvertShortToFull(lpszFileName);

    if (!MDIGetActive())
      MinimizeSplitter();

		AddDocument(lpszFileName);
  }
}

void BCFrameWnd::OpenCurrentLog()
{
  TCHAR szFile[MAX_PATH];
  if (m_pSidebarDlg)
    m_pSidebarDlg->GetSelectedLog(szFile, MAX_PATH);
  AddDocument(szFile);
}

// Activate a tab.
void BCFrameWnd::ActivateTab( ClsWindow *pWindow )
{
	// Look it up.
	for ( int i = 0; i < m_Childs.GetSize(); i++ )
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
			if ( bDragging == false && m_Childs.GetSize() )
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