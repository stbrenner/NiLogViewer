//
//	bcchild.cpp
//
//	By Stephan Brenner.
//
//	NI Log Viewer.
//	This code is public domain, use and abuse as you desire.
//
#include "stdafx.h"

// Construction.
BCChildWnd::BCChildWnd()
{
	// Clear data.
	m_pFrame = NULL;
}

// Destruction.
BCChildWnd::~BCChildWnd()
{;}

// WM_CREATE handler.
LRESULT BCChildWnd::OnCreate( LPCREATESTRUCT pCS )
{
	// Pass on to the base class.
	return ClsMDIChildWindow::OnCreate( pCS );
}

// Set window title.
void BCChildWnd::SetTitle()
{
	// Modified?
	ClsString title;
	title = GetSafeHWND();
	BOOL bSetTitle = FALSE;
	if ( m_Brainchild.Modified())
	{
		// If not done yet we append the "*" character to the
		// title to mark the file as modified.
		if ( title.Right( 1 ) != _T( "*" ))
		{
			title = title + _T( "*" );
			bSetTitle = TRUE;
		}
	}
	else
	{
		// Remove the "*" character if the file is no longer
		// modified.
		if ( title.Right( 1 ) == _T( "*" ))
		{
			title = title.Left( title.GetStringLength() - 1 );
			bSetTitle = TRUE;
		}
	}

	// Update the title.
	if ( bSetTitle ) SetWindowText( title );
}

// Update file information.
void BCChildWnd::UpdateFileInfo()
{
	// Get file information.
	FILEINFO fi;
	if ( m_Brainchild.GetFileInfo( &fi ))
	{
		// Display the file type description
		// and icon in the status bar.
		//m_pFrame->m_Status.SetIcon( 0, fi.hFileTypeIconSmall );
		//m_pFrame->m_Status.SetText( 0, 0, ( LPSTR )fi.lpszFileTypeDesc );

		// Setup MDI window icon.
		SetIcon( fi.hFileTypeIconSmall, FALSE );
		m_pFrame->DrawMenuBar();

		// Setup the title.
		SetTitle();
	}
}

// WM_NCCREATE handler. This is called not by the "ClsWindow" message
// dispatcher but by the "ClsMDIChildWindow" message dispatcher.
LRESULT BCChildWnd::OnMDINCCreate( LPCREATESTRUCT pCS )
{
	// Make sure the child has a client edge. NOTE: Simply adding this
	// bit to the CREATESTRUCT.dwExStyle will not work...
	//
	// We do it here like this also because the PreCreateWindow() overide
	// will not be called when creating MDI child windows.
	ModifyExStyle( 0, WS_EX_CLIENTEDGE );

	// Setup owner.
	m_Brainchild.SetOwner( this );

	// Create brainchild control.
	ClsRect rc( 0, 0, 100, 100 );
	if ( m_Brainchild.Create( this, rc ) == FALSE )
		return FALSE;

	return TRUE;
}

// Handle the WM_SIZE messages.
LRESULT BCChildWnd::OnSize( UINT nSizeType, int nWidth, int nHeight )
{
	// Resize Brainchild control.
	m_Brainchild.SetWindowPos( NULL, 0, 0, nWidth, nHeight, SWP_NOZORDER );
	return ClsMDIChildWindow::OnSize( nSizeType, nWidth, nHeight );
}

// Handle the WM_CLOSE messages.
LRESULT BCChildWnd::OnClose()
{
	// Destroy the window.
	MDIDestroy();
	return 0;
}
// We are activated...
LRESULT BCChildWnd::OnMDIActivated( ClsWindow *pDeactivated )
{
	static UINT menuIdsToEnable[] =
	{
		IDM_CLOSE,
		IDM_FIND,
		IDM_FINDNEXT,
		IDM_FINDPREV,
    IDM_TOGGLEBM,
    IDM_GOTO,
    IDM_SELECTALL,
    IDM_PRINT,
		IDM_PAGESETUP,
    IDM_RELOAD_FILE,
    IDM_OVERVIEW,
    IDM_SEND_AS_EMAIL,
		0xFFFFFFFF
	};

	for ( int i = 0; menuIdsToEnable[ i ] != 0xFFFFFFFF; i++ )
  {
    m_pFrame->m_ChildMenu.EnableItem( menuIdsToEnable[ i ], MF_BYCOMMAND | MF_ENABLED );
  }

  static UINT toolbarButtonsToEnable[] =
	{
		IDM_FIND,
		IDM_FINDNEXT,
		IDM_FINDPREV,
    IDM_TOGGLEBM,
    IDM_GOTO,
    IDM_SELECTALL,
    IDM_PRINT,
		IDM_PAGESETUP,
    IDM_RELOAD_FILE,
    IDM_OVERVIEW,
    IDM_SEND_AS_EMAIL,
		0xFFFFFFFF
	};

	for ( int i = 0; toolbarButtonsToEnable[ i ] != 0xFFFFFFFF; i++ )
  {
		m_pFrame->m_Toolbar.EnableButton( toolbarButtonsToEnable[ i ], TRUE );
  }


	// Update file information.
	UpdateFileInfo();

  m_pFrame->m_Status.MultiPart(TRUE);
  m_pFrame->m_Status.SetLineCol(m_Brainchild.m_iLineNumber, 
    m_Brainchild.m_iColumnNumber);

	// Update tab selection if necessary.
	m_pFrame->ActivateTab( this );
	return 0;
}

// We are deactivated.
LRESULT BCChildWnd::OnMDIDeactivated( ClsWindow * pActivated )
{
	static UINT menuItemsToDisable[] =
	{
		IDM_CLOSE,
		IDM_FIND,
		IDM_FINDNEXT,
		IDM_FINDPREV,
		IDM_CUT,
		IDM_COPY,
		IDM_PASTE,
		IDM_DELETE,
		IDM_REPLACE,
		IDM_TOGGLEBM,
		IDM_NEXTBM,
		IDM_PREVBM,
		IDM_CLEARBM,
		IDM_UNDO,
		IDM_REDO,
		IDM_PRINT,
		IDM_PAGESETUP,
		IDM_PROPERTIES,
    IDM_RELOAD_FILE,
    IDM_GOTO,
    IDM_SELECTALL,
    IDM_FIRSTBM,
    IDM_LASTBM,
    IDM_SEND_AS_EMAIL,
		0xFFFFFFFF
	};

	// Iterate button IDs and disable them.
	for ( int i = 0; menuItemsToDisable[ i ] != 0xFFFFFFFF; i++ )
  {
    m_pFrame->m_ChildMenu.EnableItem( menuItemsToDisable[ i ], MF_BYCOMMAND | MF_GRAYED );
  }

	static UINT toolbarButtonsToDisable[] =
	{
		IDM_CUT,
		IDM_COPY,
		IDM_PASTE,
		IDM_DELETE,
		IDM_FIND,
		IDM_REPLACE,
		IDM_TOGGLEBM,
		IDM_NEXTBM,
		IDM_PREVBM,
		IDM_CLEARBM,
		IDM_UNDO,
		IDM_REDO,
		IDM_PRINT,
		IDM_PAGESETUP,
		IDM_PROPERTIES,
    IDM_RELOAD_FILE,
    IDM_SEND_AS_EMAIL,
		0xFFFFFFFF
	};

	// Iterate button IDs and disable them.
	for ( int i = 0; toolbarButtonsToDisable[ i ] != 0xFFFFFFFF; i++ )
  {
		m_pFrame->m_Toolbar.EnableButton( toolbarButtonsToDisable[ i ], FALSE );
  }

  return 0;
}


// Window procedure...
LRESULT BCChildWnd::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// What's up?
	switch ( uMsg )
	{
		case	WM_SETFOCUS:
			// Turn focus on to the brainchild control.
			m_Brainchild.SetFocus();
			break;
	}
	// On to the base class.
	return ClsMDIChildWindow::WindowProc( uMsg, wParam, lParam );
}

// Caret position changed.
LRESULT Brainchild::OnCaretPosition( LPNMCARETPOSITION pCaretPos, BOOL& bNotifyParent )
{
	// No need to bother the parent.
	bNotifyParent = FALSE;

	// Update the status bar with the
	// new caret position.
	if ( m_pOwner )
		m_pOwner->m_pFrame->m_Status.SetLineCol( pCaretPos->ptCaretPos.y, pCaretPos->ptCaretPos.x );

  m_iLineNumber = pCaretPos->ptCaretPos.y;
  m_iColumnNumber = pCaretPos->ptCaretPos.x;

	return 0;
}

// Control status change.
LRESULT Brainchild::OnStatusUpdate( LPNMSTATUSUPDATE pStatusUpd, BOOL& bNotifyParent )
{
	// No need to bother the parent.
	bNotifyParent = FALSE;

	if ( m_pOwner )
	{
    // Check, if there is a file to open
    /*BOOL bIsPathSelected = FALSE;
    {
      TCHAR szSelection[MAX_PATH];
      if (GetSelectionLength() && GetSelectionLength() < MAX_PATH+2)
      {
        GetSelection(szSelection);
        szSelection[GetSelectionLength()-2] = 0;
      }
      else
        szSelection[0] = 0;

      if (_tcslen(szSelection) &&
          (_tcsstr(szSelection, _T(":")) || _tcsstr(szSelection, _T("\\")) || _tcsstr(szSelection, _T("."))))
        bIsPathSelected = TRUE;
    }*/

		// Get child menu.
		BCMenu *pMenu = &m_pOwner->m_pFrame->m_ChildMenu;

		// Update menus...
		pMenu->EnableItem( IDM_COPY,	      pStatusUpd->bCanCopy   ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_CLEARBM,	    pStatusUpd->bBookmarks ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_FIRSTBM,     pStatusUpd->bBookmarks ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_NEXTBM,      pStatusUpd->bBookmarks ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_PREVBM,      pStatusUpd->bBookmarks ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_LASTBM,      pStatusUpd->bBookmarks ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		//pMenu->EnableItem(IDM_OPEN_SELECTED_PATH,   bIsPathSelected ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );

		// And the popup menu...
		pMenu = &m_pOwner->m_pFrame->m_ContextMenu;
		pMenu->EnableItem(IDM_COPY,        pStatusUpd->bCanCopy   ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		//pMenu->EnableItem(IDM_OPEN_SELECTED_PATH,   bIsPathSelected ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );

		// And toolbar buttons.
		BCToolbar *pTool = &m_pOwner->m_pFrame->m_Toolbar;
		pTool->EnableButton( IDM_COPY,	  pStatusUpd->bCanCopy   );
		pTool->EnableButton( IDM_CLEARBM, pStatusUpd->bBookmarks );
		pTool->EnableButton( IDM_NEXTBM,  pStatusUpd->bBookmarks );
		pTool->EnableButton( IDM_PREVBM,  pStatusUpd->bBookmarks );

		// Update...
		m_pOwner->SetTitle();
	}
	return 0;
}

// File name is changed.
LRESULT Brainchild::OnFilenameChanged( LPNMFILENAMECHANGED pFilenameChg, BOOL& bNotifyParent )
{
	// No need to bother the parent.
	bNotifyParent = FALSE;
	if ( m_pOwner )
	{
		// First set the title, then evaluate it.
		m_pOwner->SetWindowText( pFilenameChg->lpszFileName );
		m_pOwner->UpdateFileInfo();
	}
	return 0;
}

// Windows procedure override.
LRESULT Brainchild::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  // Evaluate the message...
	switch ( uMsg )
	{
		case	WM_KEYDOWN:
      /*if (wParam == 13)   // Return key
        m_pOwner->m_pFrame->SendMessage(WM_COMMAND, IDM_OPEN_SELECTED_PATH);*/

      // Pass on all keys except the Context menu key.
			if ( wParam != 0x5D )
				break;

		case	WM_RBUTTONUP:
		{
			// Open the context menu at the cursor position.
			ClsPoint pt;
			GetCursorPos( pt );
			TrackPopupMenuEx( m_pOwner->m_pFrame->m_ContextMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.X(), pt.Y(), *m_pOwner->m_pFrame, NULL );

			// If we got here because of a Context menu key
			// we return so that the brainchild control does
			// not get a chance to process it.
			if ( uMsg == WM_KEYDOWN && wParam == 0x5D ) return 0;
			break;
		}
	}
	return ClsBrainchild::WindowProc( uMsg, wParam, lParam );
}