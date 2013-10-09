//-----------------------------------------------------------------------------
//
//    FastFindDlg.h
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------

// Thread messages posted to the main dialog by the
// search thread.
#define THREAD_STOPPED		WM_USER + 3000
#define THREAD_PROCESSING	WM_USER + 3001	// lParam = ( ClsString * )pFileName
#define THREAD_FOUND		WM_USER + 3002  // wParam = nLineNr, lParam = ( ClsString * )pLineStr

class CFindListCtrl : public ClsListView
{
	_NO_COPY(CFindListCtrl);
public:
  CFindListCtrl() {;}
  ~CFindListCtrl() {;}
  inline void SetFrameWindow( BCFrameWnd *pFrame ) { m_pFrame = pFrame; }
  inline void SetFolder( LPCTSTR lpszFolder ) { _tcsncpy(m_szFolder, lpszFolder, MAX_PATH); }

protected:
  virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
  {
    switch ( uMsg )
    {
    case WM_GETDLGCODE:
      if (wParam == 13)   // Return key
        OpenSelectedResult();
      break;
    }

    return ClsListView::WindowProc(uMsg, wParam, lParam);
  }

  virtual LRESULT OnItemActivate( LPNMITEMACTIVATE pItemActive, BOOL& bNotifyParent )
  {
    OpenSelectedResult();
    return ClsListView::OnItemActivate(pItemActive, bNotifyParent);
  }

  void OpenSelectedResult()
  {
    int nSelectionMark = GetSelectionMark();

    // Open document
    TCHAR szFileName[MAX_PATH];
    GetItemText(nSelectionMark, 0, szFileName, MAX_PATH);

    TCHAR szPath[MAX_PATH];
    _stprintf(szPath, _T("%s%s"), m_szFolder, szFileName);
    m_pFrame->AddDocument(szPath);

    // Select line
    TCHAR szLine[20];
    GetItemText(nSelectionMark, 1, szLine, 20);

    int nLine = _ttoi(szLine);

    BCChildWnd* pActive = (BCChildWnd*)m_pFrame->MDIGetActive();
    if (!pActive)
      return;
    
    pActive->m_Brainchild.GotoLine(nLine);
    pActive->m_Brainchild.ExecuteCommand(CID_MARK_LINE);
    pActive->m_Brainchild.GotoLine(nLine);

    // Refresh document window
    m_pFrame->m_Status.SetLineCol(nLine, 1);
    pActive->RedrawWindow();
  }

  BCFrameWnd* m_pFrame;
  TCHAR m_szFolder[MAX_PATH];
};


// ClsDialog derived class which encapsulates the whole
// FastFind functionallity.
class FastFindDlg : public ClsSizeDialog
{
	_NO_COPY( FastFindDlg );
public:
	// Constructor.
	FastFindDlg() {;}

	// Destructor.
	virtual ~FastFindDlg() 
	{ 
		// If the icon exists, destroy it.
	}

  inline void SetFrameWindow( BCFrameWnd *pFrame ) { m_pFrame = pFrame; }

protected:
	// Convert the tabs in the string to spaces.
	// Uses a tab-size of 8.
	static ClsString *ConvertForPrint( LPCTSTR pszSource )
	{
		// Compute the "real" length.
		int nNewLen = 0;
		int i;
		for ( i = 0; i < ( int )_tcslen( pszSource ); i++ )
		{
			// Is it a tab?
			if ( pszSource[ i ] == _T( '\t' ))
				// Add the number of spaces to
				// the next tab stop.
				nNewLen += ( 8 - ( nNewLen % 8 ));
			else
				// One more.
				nNewLen++;
		}

		// Allocate buffer which can hold the
		// "expanded" string.
		ClsString *str = new ClsString();
		str->AllocateBuffer(( nNewLen + 1 ) * sizeof( TCHAR ));
		str->SetStringLength( nNewLen );

		// Expand tabs.
		LPTSTR pszDest = *str;
		int nPos = 0;
		for ( i = 0; i < ( int )_tcslen( pszSource ); i++ )
		{
			// A tab?
			if ( pszSource[ i ] == _T( '\t' ))
			{
				// Compute the number of spaces to the next
				// tab-stop.
				int	nSpace = ( 8 - ( nPos % 8 ));

				// Add the spaces.
				while ( nSpace )
				{
					pszDest[ nPos ] = _T( ' ' );
					nSpace--;
					nPos++;
				}
			}
			else
				// Copy the character.
				pszDest[ nPos++ ] = pszSource[ i ];
		}
		return str;
	}

	// Add a backslash to the string.
	static void AddBackslash( ClsString& str )
	{
		// Add a backslash to the string if not yet present.
		if ( str[ str.GetStringLength() - 1 ] != _T( '\\' ))
			str += _T( '\\' );
	}

	// Scan directory. This code get's called on 
	// the spawned thread. It uses recursion to traverse
	// sub-directories when necessary.
	BOOL ScanDirectory( LPCTSTR pszDir )
	{
		ClsString	strPath;
		ClsFindFile	find;
		ClsStdioFile	file;

		// Are we still running?
		if ( m_Thread.Wait() != ClsWorkerThread::wtRunning )
			return FALSE;

		// Get the index of the first delimted part of the string.
    if (m_StrLookIn.CompareNoCase(_T("Log Folder")) == 0)
    {
		  strPath = pszDir;
		  AddBackslash( strPath );
		  strPath += _T("*.log*");
    }
    else if (m_StrLookIn.CompareNoCase(_T("Current File")) == 0)
    {
      strPath = m_pFrame->MDIGetActive()->GetSafeHWND();
      if (strPath.Find(_T(".")) < 0)
      {
        MessageBox(_T("No file selected!"), _T("Failed"));
        return FALSE;
      }
    }
    else
    {
      TCHAR szExt[_MAX_EXT];
      _tsplitpath(m_StrLookIn, NULL, NULL, NULL, szExt);
      if (!_tcsicmp(szExt, _T(".log")) || m_StrLookIn.Find(_T("*")) > -1 || m_StrLookIn.Find(_T("?")) > -1)
        strPath = m_StrLookIn;
      else
      {
		    strPath = m_StrLookIn;
		    AddBackslash( strPath );
		    strPath += _T("*.log*");
      }
    }

    // Give list ctrl the log folder
    TCHAR szLogFolder[MAX_PATH];
    TCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
    _tsplitpath(strPath, szDrive, szDir, szFName, szExt);
    _stprintf(szLogFolder, _T("%s%s"), szDrive, szDir);
    m_List.SetFolder(szLogFolder);

		// Scan the directory.
		if ( find.FindFile( strPath ))
		{
			// Continue until we are done.
			do
			{
				// Still running?
				if ( m_Thread.Wait() != ClsWorkerThread::wtRunning )
				{
					// Close handle and exit.
					find.Close();
					return FALSE;
				}

				// We only show the correct files.
				if ( ! find.IsDots() && ! find.IsDirectory())
				{
					// Get the file name.
					ClsString strName;
					if ( find.GetFilePath( strName ))
					{
						// Show the file we are processing. Simply do a PostMessage()
						// to the dialog which handles the GUI stuff. It will also delete
						// the ClsString we allocate here.
						ClsString *pStr = new ClsString();
            find.GetFileName(*pStr);
						PostMessage( THREAD_PROCESSING, 0, ( LPARAM )pStr );

						// Gracefully handle IO errors.
						try
						{
							// Read the file line-by-line...
							TCHAR szBuf[ 4096 ];
							file.Open( strName, _T( "r" ));
							m_nFiles++;
							int nLine = 0;
							
							while ( file.GetS( szBuf, 4096 ))
							{
								// Still running?
								if ( m_Thread.Wait() != ClsWorkerThread::wtRunning )
								{
									// Close the file and the
									// find-file handles and exit.
									file.Close();
									find.Close();
									return FALSE;
								}

								// Get the base pointer of the buffer.
								TCHAR *ptr = szBuf;
								int    nIdx;

								// Increase line number.
								nLine++;

								// Find all occurences on this line.
								BOOL bAnyFound = FALSE;
								while (( _tcslen( ptr )) && (( nIdx = m_BoyerMoore.FindForward( ptr, ( int )_tcslen( ptr ))) >= 0 ))
								{
									// Are we at the first found entry?
									if ( bAnyFound == FALSE )
									{
										// Strip white spaces from the end of the input buffer.
										while ( _istspace( szBuf[ _tcslen( szBuf ) - 1 ] ))
											szBuf[ _tcslen( szBuf ) - 1 ] = 0;

										// Increase lines-found counter.
										m_nLines++;
									}

									// Found a least one.
									bAnyFound = TRUE;

									// Convert the line for print and post it to the
									// main thread for the GUI stuff.
									ClsString *pStr = ConvertForPrint( szBuf );
									PostMessage( THREAD_FOUND, nLine, ( LPARAM )pStr );
									
									// Increase search pointer so we can search the rest of the line.
									ptr += nIdx + 1;
								}
							}
							// Close file.
							file.Close();
						}
						catch( ClsException& )
						{
							// Continue on IO errors...
							continue;
						}
					}
				}
			} while ( find.FindNextFile());
			// Close the handle.
			find.Close();
		}
		return TRUE;
	}

	// Thread which does the actual find-in-files.
	static UINT WINAPI FindThreadProc( LPVOID pParam )
	{
		// The parameter is a pointer to the FastFindDlg object
		// which has initiated this search thread.
		FastFindDlg *pDlg = ( FastFindDlg * )pParam;
    
    // Get log folder
    TCHAR szLogFolder[MAX_PATH];
    pDlg->m_pFrame->GetLogFolder(szLogFolder, MAX_PATH);

		// Scan the directory...
		pDlg->m_Critical.Lock();
		pDlg->ScanDirectory(szLogFolder);
		pDlg->m_Critical.Unlock();

		// Done.
		MessageBeep(( UINT )-1 );

		// Make sure the main thread knows it too.
		pDlg->PostMessage( THREAD_STOPPED );
		return 0;
	}

	// WM_INITDIALOG message handler...
	virtual LRESULT OnInitDialog( PROPSHEETPAGE *p )
	{
		// Create layout engine controls. We do this before "ClsDialog" get's
		// a chance to distribute it's font to it's children so that when it does
		// the layout engine controls will get the font set too.
		m_Master.Create( *this, Offsets( 6, 6, 6, ::GetSystemMetrics( SM_CYHSCROLL )), LAYOUT_Master, TRUE, LAYOUT_Horizontal, FALSE, TAG_END );
		m_Left.Create( *this, Offsets( 0, 0, 0, 0 ), LAYOUT_Horizontal, FALSE, LAYOUT_EqualMinWidth, TRUE, TAG_END );
		m_Right.Create( *this, Offsets( 0, 0, 0, 0 ), LAYOUT_Horizontal, FALSE, TAG_END );
		m_StatBut.Create( *this, Offsets( 0, 0, 0, 0 ), LAYOUT_Horizontal, TRUE, TAG_END );
		m_LeftRight.Create( *this, Offsets( 0, 0, 0, 0 ), LAYOUT_Spacing, 6, LAYOUT_Horizontal, TRUE, TAG_END );

		// Setup infobar control.
		m_Proc.Attach( GetDlgItemHandle( IDC_PROC ));
		m_Proc.CompactAsPath() = TRUE;
    m_Proc.BackgroundColor() = GetSysColor(COLOR_BTNFACE);
    m_Proc.TitleColor() = GetSysColor(COLOR_BTNTEXT);

		// Setup list.
    m_List.SetFrameWindow(m_pFrame);
		m_List.Attach( GetDlgItemHandle( IDC_LIST ));
		m_List.SetExtendedListViewStyle( LVS_EX_FULLROWSELECT );
		m_List.InsertColumn( 0, _T( "File" ), LVCFMT_LEFT, 140);
		m_List.InsertColumn( 1, _T( "Line"), LVCFMT_LEFT, 40);
		m_List.InsertColumn( 2, _T( "Text" ), LVCFMT_LEFT, 600);

		// Setup other controls.
		m_Find.Attach( GetDlgItemHandle( IDC_FIND ));
		m_Case.Attach( GetDlgItemHandle( IDC_CASE ));
		m_LookIn.Attach( GetDlgItemHandle( IDC_FOLDER ));
    m_LookIn.AddString(_T("Current File"));
    m_LookIn.AddString(_T("Log Folder"));
    m_LookIn.SelectString(0, _T("Log Folder"));
		m_Occ.Attach( GetDlgItemHandle( IDC_OCC ));
		m_Go.Attach( GetDlgItemHandle( IDC_GO ));

		// Now add values from registry.
		ReadComboFromRegistry(m_LookIn.GetSafeHWND(), REG_PATH, _T("LastSearchFolders"));
		ReadWindowTextFromRegistry(m_Find.GetSafeHWND(), REG_PATH, _T("LastSearchString"));

		// Call the base class. The base will also distribute the
		// dialog font.
		return ClsSizeDialog::OnInitDialog( p );
	}

	// Called after ClsDialog has distributed the dialog
	// font to it's children.
	virtual void OnFontDistributed()
	{
		// Setup static controls. Attach them to the objects and
		// add them to the layout engine controls.
		for ( int i = 0; i < 3; i++ )
		{
			m_Statics[ i ].Attach( GetDlgItemHandle( IDC_STATIC_1 + i ));
			m_Left.AddSpacingMember();
			m_Left.AddMember( &m_Statics[ i ], NULL, ATTR_FixMinSize, TRUE, ATTR_RightAlign, TRUE, TAG_END );
			m_Left.AddSpacingMember();
		}

		// Add the controls to the layout engine controls.
		m_Right.AddMember( &m_Find, NULL, TAG_END );
		m_Right.AddMember( &m_LookIn, NULL, TAG_END );
		m_StatBut.AddMember( &m_Occ, NULL, TAG_END );
		m_StatBut.AddMember( &m_Go, NULL, ATTR_UseControlSize, TRUE, ATTR_FixMinWidth, TRUE, TAG_END );
		m_Right.AddMember( &m_StatBut, NULL, TAG_END );

		// Get the minimum size of the "m_Right" layout-engine without
		// the checkboxes.
		ClsSize szRight; m_Right.OnGetMinSize( szRight );

		// Add the checkboxes.
		m_Right.AddMember( &m_Case, NULL, ATTR_FixMinWidth, TRUE, TAG_END );
		m_Right.AddMember( &m_Proc, NULL, ATTR_UseControlSize, TRUE, TAG_END );

		// Combine left and right layout-engine controls.
		m_LeftRight.AddMember( &m_Left, NULL, ATTR_FixMinWidth, TRUE, ATTR_FixHeight, szRight.CY(), TAG_END );
		m_LeftRight.AddMember( &m_Right, NULL, TAG_END );

		// Setup the master layout engine.
		m_Master.AddMember( &m_LeftRight, NULL, ATTR_FixMinHeight, TRUE, TAG_END );
		m_Master.AddMember( &m_List, NULL, TAG_END );

		// Any errors?
		if ( m_Master.Error())
		{
			// Bye...
			EndDialog( TRUE );
			return;
		}

		// Compute the minimum size of the master group.
		if ( m_Master.OnGetMinSize( m_MinSize ) == FALSE )
		{
			// Bye...
			EndDialog( 0 );
			return;
		}

		// Add frame and caption sizes so that we know the minimum
		// size of the dialog.
		m_MinSize.CY() += ( ::GetSystemMetrics( SM_CYFRAME ) * 2 ) + ::GetSystemMetrics( SM_CYCAPTION );
		m_MinSize.CX() += ::GetSystemMetrics( SM_CXFRAME ) * 2;
		
		// Relayout the master layout engine control.
		m_Master.Relayout();

		// No thread running.
		m_bSearchInProgress = FALSE;
		
		// Call the base class.
		ClsSizeDialog::OnFontDistributed();
	}

	// WM_CLOSE handler.
	virtual LRESULT OnClose() 
	{
		// Is the search thread running?
		if ( m_bSearchInProgress ) 
		{ 
			// Stop the thread and delete the ClsWorkerThread object. 
			//
			// At this point there may still be a lot of
			// messages pending from the search thread. 
			// These messages are processed by the main 
			// thread before the window is actually destroyed.
			m_Thread.Stop();
		} 

    ShowWindow(SW_HIDE);

		// Base class.
		return ClsDialog::OnClose(); 
	}

	// This is not allowed to end the dialog.
	virtual BOOL OnOK() { return FALSE; }

	// Escape key pressed.
	virtual BOOL OnCancel() 
	{
		// Search thread running?
		if ( m_bSearchInProgress )
		{
			// Stop the thread and do not
			// close the dialog.
			m_Thread.Stop();
			return FALSE;
		}
    else
      ShowWindow(SW_HIDE);   // Close search dialog

		// Close the dialog.
		return TRUE;
	}

	// WM_SIZE message handler.
	virtual LRESULT OnSize( UINT nSizeType, int nWidth, int nHeight )
	{
		// Tell the master layout engine to layout it's
		// members.
		m_Master.Relayout();

		// Call the base class.
		return ClsSizeDialog::OnSize( nSizeType, nWidth, nHeight );
	}

	// Window procedure override.
	virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		// What's up?
		switch ( uMsg )
		{
			case	WM_GETMINMAXINFO:
			{
				// Fill in the minimum size of the dialog.
				LPMINMAXINFO pmmi = ( LPMINMAXINFO )lParam;
				pmmi->ptMinTrackSize.x = m_MinSize.CX();
				pmmi->ptMinTrackSize.y = m_MinSize.CY();
				return 0;
			}

			case	THREAD_STOPPED:
			{
				// Destroy thread.
				m_Thread.Destroy();

				// Enable GUI.
				m_Case.EnableWindow();
				m_Find.EnableWindow();
				m_LookIn.EnableWindow();
				m_Go.SetWindowText( "Start" );
				m_Proc.SetWindowText( NULL );
				m_bSearchInProgress = FALSE;
        m_Go.EnableWindow(FALSE);
        m_List.SetFocus();

				// Setup columns.
				m_List.SetColumnWidth( 1, LVSCW_AUTOSIZE_USEHEADER );
				m_List.SetColumnWidth( 2, LVSCW_AUTOSIZE_USEHEADER );

				// Final report.
				ClsString str;
				m_Critical.Lock();
				str.Format( _T( "%ld on %ld lines (Searched: %ld files)." ), m_nItem, m_nLines, m_nFiles );
				m_Critical.Unlock();
				m_Occ.SetWindowText( str );
				return 0;
			}
			
			case	THREAD_PROCESSING:
				// Only update GUI when the thread is still alive.
				if ( m_Thread.IsRunning())
					// Setup info-bar.
					m_Proc.SetWindowText( *(( ClsString * )lParam ));

				// We always must delete the string otherwise
				// we can have a giant memory leak.
				delete ( ClsString * )lParam;
				return 0;

			case	THREAD_FOUND:
			{
				// Only update GUI when the thread is still alive.
				ClsString *str = ( ClsString *)lParam, tmp;
				if ( m_Thread.IsRunning())
				{
					// Pause the thread so we do not pile up to many
					// message during this action.
					m_Thread.Pause();

					// Insert the information into the listview.
					LVITEM it;
					tmp = m_Proc;
					it.mask = LVIF_TEXT;
					it.iItem = m_nItem;
          if (m_nItem == 0)
          {
            it.mask |= LVIF_STATE;
            it.state = LVIS_SELECTED | LVIS_FOCUSED;
            it.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
          }
					it.iSubItem = 0;
					it.pszText = tmp;
					m_List.InsertItem( it );
					tmp = ( UINT )wParam;
					m_List.SetItemText( m_nItem, 1, tmp );
					m_List.SetItemText( m_nItem, 2, *str );

					// Update the occurences control.
					tmp = m_nItem++;
					m_Occ.SetWindowText( tmp );

					// Go on.
					m_Thread.Run();
				}

				// Delete the string.
				delete str;
				break;
			}
		}
		// Base class.
		return ClsSizeDialog::WindowProc( uMsg, wParam, lParam );
	}

	// WM_COMMAND message handler.
	virtual LRESULT OnCommand( UINT nNotifyCode, UINT nCtrlID, HWND hWndCtrl )
	{
		switch ( nCtrlID )
		{
			case	IDC_FIND:
			{
				// Contents changed?
				if ( nNotifyCode == EN_CHANGE )
					// Enable start button.
					m_Go.EnableWindow(( BOOL )( m_Find.GetWindowTextLength() && m_LookIn.GetWindowTextLength()));
				break;
			}

      case  IDC_CASE:
			case	IDC_FOLDER:
			{
				// Enable start button.
				m_Go.EnableWindow(( BOOL )( m_Find.GetWindowTextLength() && m_LookIn.GetWindowTextLength()));
				break;
			}

			case	IDC_GO:
				// Search thread running?
				if ( m_bSearchInProgress )
					// Stop it.
					m_Thread.Stop();
				else
				{
					// Write values to registry.
					WriteComboToRegistry(m_LookIn.GetSafeHWND(), REG_PATH, _T("LastSearchFolders"), 2);
					WriteWindowTextToRegistry(m_Find.GetSafeHWND(), REG_PATH, _T("LastSearchString"));

					// Clear brainchild control.
					m_List.DeleteAllItems();
					m_Occ.SetWindowText( _T( "0" ));
          
					// Setup data for the search thread.
					m_Critical.Lock();
					m_nItem = m_nLines = m_nFiles = 0;
					m_bCase = m_Case.IsChecked();
					m_StrFind = m_Find;
					m_StrLookIn = m_LookIn;

					// Setup Boyer-Moore object.
					m_BoyerMoore.SetSearchString( m_StrFind );
					m_BoyerMoore.SetCaseMode( m_bCase );
					m_Critical.Unlock();

					// Create thread.
					if ( m_Thread.Start( FindThreadProc, this ))
					{
						// Thread running.
						m_bSearchInProgress = TRUE;

						// Disable GUI.
						m_Case.EnableWindow( FALSE );
						m_Find.EnableWindow( FALSE );
						m_LookIn.EnableWindow( FALSE );

						// Change button.
						m_Go.SetWindowText( "Stop" );
						m_Go.SetFocus();
					}
				}
				break;
		}
		// Base class.
		return ClsSizeDialog::OnCommand( nNotifyCode, nCtrlID, hWndCtrl );
	}

	// Control objects.
	ClsLayoutEngine		m_Master, m_Left, m_Right, m_StatBut, m_LeftRight;
	ClsLStatic		m_Statics[ 7 ];
	CFindListCtrl		m_List;
	ClsLPushButton		m_Go;
	ClsLCheckBox		m_Case;
	ClsLEdit		m_Find;
	ClsComboBox	m_LookIn;
	ClsInfoBar		m_Proc;
	ClsLStatic		m_Occ;

	// Misc data.
	ClsSize			m_MinSize;
	int			m_nOccurences, m_nItem;
  BCFrameWnd* m_pFrame;

	// Thread related data.
	ClsWorkerThread	        m_Thread;
	BOOL			m_bRunning, m_bSearchInProgress, m_bCase;
	ClsBoyerMoore		m_BoyerMoore;
	ClsString		m_StrFind, m_StrLookIn;
	int			m_nLines, m_nFiles;
	ClsCriticalSection	m_Critical;
};