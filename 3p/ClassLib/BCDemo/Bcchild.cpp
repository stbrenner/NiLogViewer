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
		SendMessage( WM_SETICON, 0, ( LPARAM )fi.hFileTypeIconSmall );
		m_pFrame->DrawMenuBar();
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
	// Store maximized state.
	if ( m_pFrame )
		m_pFrame->m_dwChildMax = IsZoomed();

	// Resize Brainchild control.
	m_Brainchild.SetWindowPos( NULL, 0, 0, nWidth, nHeight, SWP_NOZORDER );

	return ClsMDIChildWindow::OnSize( nSizeType, nWidth, nHeight );
}
	
// Handle the WM_CLOSE messages.
LRESULT BCChildWnd::OnClose()
{
	// Modified?
	if ( m_Brainchild.CheckModified() == FALSE )
		return TRUE;
	
	// Destroy the window.
	MDIDestroy();
	return 0;
}

// We are activated...
LRESULT BCChildWnd::OnMDIActivated( ClsWindow *pDeactivated )
{
	// Frame valid?.
	if ( m_pFrame )
	{
		// Enable buttons.
		BCToolbar *pTool = &m_pFrame->m_Toolbar;
		pTool->EnableButton( IDM_SAVE	     );
		pTool->EnableButton( IDM_SAVEALL    );
		pTool->EnableButton( IDM_CLOSE	     );
		pTool->EnableButton( IDM_FIND	     );
		pTool->EnableButton( IDM_REPLACE    );
		pTool->EnableButton( IDM_PRINT	     );
		pTool->EnableButton( IDM_PROPERTIES );
		pTool->EnableButton( IDM_TOGGLEBM   );

		// Update file information.
		UpdateFileInfo();
	}
	return 0;
}

// We are deactivated.
LRESULT BCChildWnd::OnMDIDeactivated( ClsWindow * pActivated )
{
	// Disable buttons.
	if ( m_pFrame )
	{
		BCToolbar *pTool = &m_pFrame->m_Toolbar;
		pTool->EnableButton( IDM_SAVE,		FALSE );
		pTool->EnableButton( IDM_SAVEALL,	FALSE );
		pTool->EnableButton( IDM_CLOSE,	FALSE );
		pTool->EnableButton( IDM_CUT,		FALSE );
		pTool->EnableButton( IDM_COPY,		FALSE );
		pTool->EnableButton( IDM_PASTE,	FALSE );
		pTool->EnableButton( IDM_DELETE,	FALSE );
		pTool->EnableButton( IDM_FIND,		FALSE );
		pTool->EnableButton( IDM_REPLACE,	FALSE );
		pTool->EnableButton( IDM_TOGGLEBM,	FALSE );
		pTool->EnableButton( IDM_NEXTBM,	FALSE );
		pTool->EnableButton( IDM_PREVBM,	FALSE );
		pTool->EnableButton( IDM_CLEARBM,	FALSE );
		pTool->EnableButton( IDM_UNDO,		FALSE );
		pTool->EnableButton( IDM_REDO,		FALSE );
		pTool->EnableButton( IDM_PRINT,	FALSE );
		pTool->EnableButton( IDM_PROPERTIES,	FALSE );
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

		case	WM_DROPFILES:
			// Pass to our owner.
			return m_pFrame->SendMessage( uMsg, wParam, lParam );
	}
	// On to the base class.
	return ClsMDIChildWindow::WindowProc( uMsg, wParam, lParam );
}

LRESULT Brainchild::OnCaretPosition( LPNMCARETPOSITION pCaretPos, BOOL& bNotifyParent )
{
	if ( m_pOwner )
		m_pOwner->m_pFrame->m_Status.SetLineCol( pCaretPos->ptCaretPos.y, pCaretPos->ptCaretPos.x );
	return 0;
}
	
LRESULT Brainchild::OnStatusUpdate( LPNMSTATUSUPDATE pStatusUpd, BOOL& bNotifyParent ) 
{ 
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

		// And the popup menu...
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

		// Show file mode.
		m_pOwner->m_pFrame->m_Status.SetFileMode( pStatusUpd->nFileMode );
	}
	return 0; 
}

LRESULT Brainchild::OnFilenameChanged( LPNMFILENAMECHANGED pFilenameChg, BOOL& bNotifyParent ) { return 0; }

LRESULT Brainchild::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
		case	WM_DROPFILES:
			return m_pOwner->SendMessage( uMsg, wParam, lParam );
	}
	return ClsBrainchild::WindowProc( uMsg, wParam, lParam );
}