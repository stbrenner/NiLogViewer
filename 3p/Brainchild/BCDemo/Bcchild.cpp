//
//	bcchild.cpp
//
//	By Jan van den Baard.
//
//	"Brainchild Custom Editor Control" demonstration program.
//	This code is public domain, use and abuse as you desire.
//
#include "bcdemo.h"

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
		m_pFrame->m_Status.SetIcon( 0, fi.hFileTypeIconSmall );
		m_pFrame->m_Status.SetText( 0, 0, ( LPSTR )fi.lpszFileTypeDesc );

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
	// Do we have a parent and are we the active
	// MDI child?
	if ( m_pFrame && m_pFrame->MDIGetActive() == this )
		// Store maximized state.
		m_pFrame->m_dwChildMax = ( nSizeType & SIZE_MAXIMIZED ? TRUE : FALSE );

	// Resize Brainchild control.
	m_Brainchild.SetWindowPos( NULL, 0, 0, nWidth, nHeight, SWP_NOZORDER );
	return ClsMDIChildWindow::OnSize( nSizeType, nWidth, nHeight );
}
	
// Handle the WM_CLOSE messages.
LRESULT BCChildWnd::OnClose()
{
	// Modified?
	if ( m_Brainchild.GetSafeHWND() && m_Brainchild.CheckModified() == FALSE )
		return TRUE;
	
	// Destroy the window.
	MDIDestroy();
	return 0;
}

// We are activated...
LRESULT BCChildWnd::OnMDIActivated( ClsWindow *pDeactivated )
{
	// Buttons.
	static UINT ids[] = 
	{
		IDM_SAVE,	   
		IDM_SAVEALL,   
		IDM_CLOSE,	   
		IDM_FIND,	   
		IDM_REPLACE,   
		IDM_PRINT,
		IDM_PAGESETUP,
		IDM_PROPERTIES,
		IDM_TOGGLEBM,  
		0xFFFFFFFF
	};

	// Frame valid?.
	if ( m_pFrame )
	{
		// Iterate button IDs and disable them.
		for ( int i = 0; ids[ i ] != 0xFFFFFFFF; i++ )
			m_pFrame->m_Toolbar.EnableButton( ids[ i ] );

		// Update file information.
		UpdateFileInfo();
	}

	// Update tab selection if necessary.
	m_pFrame->ActivateTab( this );
	return 0;
}

// We are deactivated.
LRESULT BCChildWnd::OnMDIDeactivated( ClsWindow * pActivated )
{
	// Buttons.
	static UINT ids[] =
	{
		IDM_SAVE,		
		IDM_SAVEALL,
		IDM_CLOSE,		
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
		0xFFFFFFFF
	};

	// Disable buttons.
	if ( m_pFrame )
	{
		// Iterate button IDs and disable them.
		for ( int i = 0; ids[ i ] != 0xFFFFFFFF; i++ )
			m_pFrame->m_Toolbar.EnableButton( ids[ i ], FALSE );
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
	return 0;
}
	
// Control status change.
LRESULT Brainchild::OnStatusUpdate( LPNMSTATUSUPDATE pStatusUpd, BOOL& bNotifyParent ) 
{ 
	// No need to bother the parent.
	bNotifyParent = FALSE;

	if ( m_pOwner )
	{
		// Set editing mode.
		m_pOwner->m_pFrame->m_Status.SetInsertMode( ! pStatusUpd->bOverwrite );

		// Get child menu.
		BCMenu *pMenu = &m_pOwner->m_pFrame->m_ChildMenu;
		
		// Readonly?
		ClsMenuItemInfo mi;
		mi.fMask  = MIIM_STATE;
		mi.fState = pStatusUpd->bReadOnly ? MFS_CHECKED : MFS_UNCHECKED;
		pMenu->SetItemInfo( IDM_READONLY, &mi );

		// Update menus...
		pMenu->EnableItem( IDM_UNDO,	    pStatusUpd->bCanUndo   ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_REDO,	    pStatusUpd->bCanRedo   ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_CUT,	    pStatusUpd->bCanCut    ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_COPY,	    pStatusUpd->bCanCopy   ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_PASTE,	    pStatusUpd->bCanPaste  ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_DELETE,	    pStatusUpd->bCanDelete ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_INVERTCASE,  pStatusUpd->bCanCopy   ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_UPPERCASE,   pStatusUpd->bCanCopy   ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_LOWERCASE,   pStatusUpd->bCanCopy   ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_CLEARBM,	    pStatusUpd->bBookmarks ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_FIRSTBM,     pStatusUpd->bBookmarks ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_NEXTBM,      pStatusUpd->bBookmarks ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_PREVBM,      pStatusUpd->bBookmarks ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_LASTBM,      pStatusUpd->bBookmarks ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem( IDM_SAVE,	    pStatusUpd->bModified  ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );

		// And the popup menu...
		pMenu = &m_pOwner->m_pFrame->m_ContextMenu;
		pMenu->EnableItem(  IDM_UNDO,	     pStatusUpd->bCanUndo   ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem(  IDM_REDO,	     pStatusUpd->bCanRedo   ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem(  IDM_CUT,	     pStatusUpd->bCanCut    ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem(  IDM_COPY,	     pStatusUpd->bCanCopy   ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem(  IDM_PASTE,	     pStatusUpd->bCanPaste  ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
		pMenu->EnableItem(  IDM_DELETE,	     pStatusUpd->bCanDelete ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );

		// And toolbar buttons.
		BCToolbar *pTool = &m_pOwner->m_pFrame->m_Toolbar;
		pTool->EnableButton( IDM_UNDO,	  pStatusUpd->bCanUndo   );
		pTool->EnableButton( IDM_REDO,	  pStatusUpd->bCanRedo   );
		pTool->EnableButton( IDM_CUT,	  pStatusUpd->bCanCut    );
		pTool->EnableButton( IDM_COPY,	  pStatusUpd->bCanCopy   );
		pTool->EnableButton( IDM_PASTE,	  pStatusUpd->bCanPaste  );
		pTool->EnableButton( IDM_DELETE,  pStatusUpd->bCanDelete );
		pTool->EnableButton( IDM_CLEARBM, pStatusUpd->bBookmarks );
		pTool->EnableButton( IDM_NEXTBM,  pStatusUpd->bBookmarks );
		pTool->EnableButton( IDM_PREVBM,  pStatusUpd->bBookmarks );
		pTool->EnableButton( IDM_SAVE,    pStatusUpd->bModified  );


		// Show file mode.
		m_pOwner->m_pFrame->m_Status.SetFileMode( pStatusUpd->nFileMode );

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
