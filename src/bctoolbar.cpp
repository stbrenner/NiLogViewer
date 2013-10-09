//
//	bctoolbar.cpp
//
//	By Stephan Brenner.
//
//	NI Log Viewer.
//	This code is public domain, use and abuse as you desire.
//
#include "stdafx.h"

// Construction.
BCToolbar::BCToolbar() 
{;}

// Destruction.
BCToolbar::~BCToolbar() {;}

// Create toolbar.
BOOL BCToolbar::Create( ClsWindow *pParent, ClsMRU *pMRU )
{
	// Save MRU pointer.
	m_pMRU = pMRU;

	// Load the bitmap and map the background color.
	COLORMAP cMap = { RGB( 255, 0, 255 ), ::GetSysColor( COLOR_BTNFACE ) };
	if ( m_Tools.CreateMappedBitmap( IDB_TOOLBAR, 0, &cMap, 1 ))
	{
		// Create the toolbar.
		ClsRect rcEmpty( 0, 0, 0, 0 );
		if ( ClsToolbar::Create( pParent, rcEmpty, 0, WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT ))
		{
			// Set the toolbar.
			SetExtendedStyle( TBSTYLE_EX_DRAWDDARROWS );
			ButtonStructSize( sizeof( TBBUTTON ));
			SetButtonSize( 16, 15 );

			// Add system bitmaps.
			TBADDBITMAP tba;
			tba.hInst = HINST_COMMCTRL;
			tba.nID   = IDB_STD_SMALL_COLOR;
			AddBitmap( 1, &tba );

			tba.hInst = NULL;
			tba.nID   = ( UINT )( HBITMAP )m_Tools;
			int nBase = AddBitmap( 1, &tba );

			// Setup toolbar buttons.
			m_tbTools[ 1 ].iBitmap = nBase + 27; // Reload File
			m_tbTools[ 4 ].iBitmap = nBase + 28; // Find in files
			m_tbTools[ 5 ].iBitmap = nBase + 19; // Find
			m_tbTools[ 7 ].iBitmap = nBase + 18; // Toggle Bookmark
			m_tbTools[ 8 ].iBitmap = nBase + 16; // Next Bookmark
			m_tbTools[ 9 ].iBitmap = nBase + 17; // Prev Bookmark
			m_tbTools[ 10].iBitmap = nBase + 15; // Clear Bookmarks
			m_tbTools[ 13].iBitmap = nBase + 29; // Clear Bookmarks

			// Add the buttons.
			AddButtons( sizeof( m_tbTools ) / sizeof( TBBUTTON ), &m_tbTools[ 0 ] );
			AutoSize();
			return TRUE;
		}
	}
	return FALSE;
}

// Handle TBN_DROPDOWN notification.
LRESULT BCToolbar::OnDropDown( LPNMTOOLBAR pToolbar, BOOL &bNotifyParent )
{
	// We do not need to notify our parent.
	bNotifyParent = FALSE;

	TBBUTTONINFO tbi;
	tbi.cbSize = sizeof( tbi );
	tbi.dwMask = TBIF_LPARAM;
	tbi.lParam = 0xDEAD;
	SetButtonInfo( pToolbar->iItem, tbi );

	// Get "Open" button rectangle.
	ClsRect rcRect( pToolbar->rcButton );
	ClientToScreen( rcRect );

	( ClsMenu::FromHandle( *m_pMRU ))->SetParentRect( rcRect );

	// Make sure the button is updated.
	InvalidateRect( &pToolbar->rcButton, TRUE );
	UpdateWindow();

	// Show the popup menu.
	::TrackPopupMenuEx( *m_pMRU,
			    /*TPM_LEFTALIGN |*/ TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
			    rcRect.Left(),
			    rcRect.Bottom(),
			    GetParent()->GetSafeHWND(),
			    NULL );

	tbi.lParam = 0L;
	SetButtonInfo( pToolbar->iItem, tbi );

	// The notification was handled.
	return TBDDRET_DEFAULT;
}

// Handle TBN_GETINFOTIP notification.
LRESULT BCToolbar::OnGetInfoTip( LPNMTBGETINFOTIP pInfoTip, BOOL& bNotifyParent )
{
	// Setup the info struct.
	TBBUTTONINFO	tbbi;
	tbbi.cbSize = sizeof( tbbi );
	tbbi.dwMask = TBIF_LPARAM;

	// Get button info.
	GetButtonInfo( pInfoTip->iItem, tbbi );

	// Any text?
	if ( tbbi.lParam )
	{
		// Setup the text.
		pInfoTip->pszText = ( LPSTR )tbbi.lParam;
		bNotifyParent = FALSE;
		return TRUE;
	}

	// Pass to the base class.
	return ClsToolbar::OnGetInfoTip( pInfoTip, bNotifyParent );
}

LRESULT BCToolbar::OnCustomDraw( LPNMCUSTOMDRAW pCustomDraw, BOOL& bNotifyParent )
{
	// Old style?
	if ( m_bOldStyle ) 
		return CDRF_DODEFAULT;

	// Evaluate draw stage.
	LPNMTBCUSTOMDRAW pCust = reinterpret_cast< LPNMTBCUSTOMDRAW >( pCustomDraw );
	if ( pCust->nmcd.dwDrawStage == CDDS_PREPAINT )
		return CDRF_NOTIFYITEMDRAW;

	// Render item?
	if ( pCust->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		// Obtain information about the button to render.
		TBBUTTONINFO bi;
		TCHAR sz[ 4096 ];
		bi.dwMask  = TBIF_IMAGE | TBIF_STYLE | TBIF_TEXT;
		bi.pszText = sz;
		bi.cchText = 4096;
		bi.cbSize  = sizeof( bi );
		if ( GetButtonInfo( pCust->nmcd.dwItemSpec, bi ) >= 0 )
		{
			// Copy button rectangle and wrap the
			// device context.
			ClsRect rc = pCust->nmcd.rc;
			ClsDC dc( pCust->nmcd.hdc );

			// Draw the special "menu-dropped" style for this
			// button?
			if ( pCust->nmcd.lItemlParam == 0xDEAD )
			{
				// Render the button outline including the button
				// background.
				dc.OutlinedRectangle( rc, XPColors.GetXPColor( ClsXPColors::XPC_IMAGE_DISABLED ), XPColors.GetXPColor( ClsXPColors::XPC_IMAGE_BACKGROUND ));

				// Adjust the rectangle to render the dropdown
				// arrow image.
				ClsRect rc2( rc );
				rc.Right() -= ::GetSystemMetrics( SM_CXMENUCHECK ) - 1;
				rc2.Left() = rc.Right() - 1;
				rc2.Left()--;
				rc2.Top() += 2;

				// Render the dropdown arrow.
				ClsDrawTools::RenderBitmap( dc, ClsGetApp()->GetClsImages(), ClsApp::CII_DROPDOWN_SMALL, rc2, pCust->nmcd.uItemState & CDIS_DISABLED ? ClsDrawTools::CDSF_DISABLED : 0 );
			}
			else
			{
				// Is the button hot and not disabled?
				if ( pCust->nmcd.uItemState & CDIS_HOT && ( ! ( pCust->nmcd.uItemState & CDIS_DISABLED )))
				{
					// Select the colors to use. 
					COLORREF crBg = XPColors.GetXPColor( pCust->nmcd.uItemState & CDIS_SELECTED ? ClsXPColors::XPC_INNER_CHECKED_SELECTED : ClsXPColors::XPC_INNER_SELECTION );
					COLORREF crFg = XPColors.GetXPColor( ClsXPColors::XPC_OUTER_SELECTION );

					// Dropdown button?
					if ( bi.fsStyle & TBSTYLE_DROPDOWN )
					{
						// Make room for the dropdown arrow.
						ClsRect rc2( rc );
						rc.Right() -= ::GetSystemMetrics( SM_CXMENUCHECK ) - 1;
						rc2.Left()  = rc.Right() - 1;

						// Render the button rectangle and the rectangle
						// surrounding the dropdown arrow.
						dc.OutlinedRectangle( rc, crFg, crBg );
						dc.OutlinedRectangle( rc2, crFg, XPColors.GetXPColor( ClsXPColors::XPC_INNER_SELECTION ));

						// Adjust the dropdownrectangle.
						rc2.Left()--;
						rc2.Top() += 2;

						// Render the arrow.
						ClsDrawTools::RenderBitmap( dc, ClsGetApp()->GetClsImages(), ClsApp::CII_DROPDOWN_SMALL, rc2, pCust->nmcd.uItemState & CDIS_DISABLED ? ClsDrawTools::CDSF_DISABLED : 0 );
					}
					else
						// Simply render the button rectangle.
						dc.OutlinedRectangle( rc, crFg, crBg );
				}
				else
				{
					// Erase the baground of the button.
					ClsBrush b( XPColors.GetXPColor( ClsXPColors::XPC_IMAGE_BACKGROUND ));
					dc.FillRect( rc, &b );

					// Drowdown button?
					if ( bi.fsStyle & TBSTYLE_DROPDOWN )
					{
						// Adjust the rectangle for the dropdown image.
						ClsRect rc2( rc );
						rc.Right() -= ::GetSystemMetrics( SM_CXMENUCHECK ) - 1;
						rc2.Left()  = rc.Right() - 2;
						rc2.Top()  += 2;

						// Render the image.
						ClsDrawTools::RenderBitmap( dc, ClsGetApp()->GetClsImages(), ClsApp::CII_DROPDOWN_SMALL, rc2, pCust->nmcd.uItemState & CDIS_DISABLED ? ClsDrawTools::CDSF_DISABLED : 0 );
					}
				}
			}

			// Obtain the toolbar imagelist.
			HIMAGELIST hImage = GetImageList();

			// Is the button disabled?
			if ( pCust->nmcd.uItemState & CDIS_DISABLED )
				// Render the monochrome image for this button.
				ClsDrawTools::RenderMonoBitmap( dc, hImage, bi.iImage , rc, XPColors.GetXPColor( ClsXPColors::XPC_IMAGE_DISABLED ));
			else
			{
				// Hot and not selected?
				if ( pCust->nmcd.uItemState & CDIS_HOT && ( ! ( pCust->nmcd.uItemState & CDIS_SELECTED )))
				{
					// Special "menu-dropped" style?
					if ( pCust->nmcd.lItemlParam != 0xDEAD ) 
					{	
						// Offset image position.
						rc.Offset( 1, 1 );

						// Render the monochrome bitmap for this button.
						ClsDrawTools::RenderMonoBitmap( dc, hImage, bi.iImage , rc, XPColors.GetXPColor( ClsXPColors::XPC_IMAGE_SHADOW ));

						// Offset rectangle again.
						rc.Offset( -2, -2 );
					}
					// Render the normal bitmap image for this button.
					ClsDrawTools::RenderBitmap( dc, hImage, bi.iImage, rc, 0 );
				}
				else
					// Render the normal bitmap image for this button.
					ClsDrawTools::RenderBitmap( dc, hImage, bi.iImage, rc, 0 );
			}
			// Un-wrap the device context handle.
			dc.Detach();
		}
	}
	else
		return CDRF_DODEFAULT;

	// Return flag.
	return CDRF_SKIPDEFAULT;
}

// Message which is posted to the window during the WM_PAINT cycle.
// It will cause the separators to be rendered. This is beginning
// to look like a hack...
#define WM_DRAWSEPARATORS ( WM_APP + 1 )

// Overridable. First let's the toolbar do it's
// thing and then posts the above defined message.
LRESULT BCToolbar::OnPaint( ClsDC *pDC )
{
	// First call the baseclass method.
	LRESULT rc = ClsToolbar::OnPaint( pDC );

	// Post the message which will cause the
	// separators to be rendered. For some reason calling
	// RedrawSeparators() here directly will cause the
	// separators to be overwritten by the separators rendered
	// by the toolbar class.
	if ( ! m_bOldStyle ) 
		PostMessage( WM_DRAWSEPARATORS, 0, 0 );

	// Return the result of the baseclass call.
	return rc;
}

// Catch and process the above defined message.
LRESULT BCToolbar::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == WM_DRAWSEPARATORS )
	{
		// Render the separators.
		RedrawSeparators();
		return 0;
	}

	// Pass the message to the baseclass.
	return ClsToolbar::WindowProc( uMsg, wParam, lParam );
}

// Custom rendering of the separators. It will simply go through the
// buttons of the toolbar and render the separators on the position at
// which they are located.
void BCToolbar::RedrawSeparators()
{
	// Define Gfx objects.
	ClsGetDC dc( this );
	ClsBrush b1( XPColors.GetXPColor( ClsXPColors::XPC_IMAGE_BACKGROUND ));
	ClsBrush b2( XPColors.GetXPColor( ClsXPColors::XPC_IMAGE_DISABLED ));

	// Itterate the toolbar buttons.
	TBBUTTONINFO tbi;
	tbi.cbSize = sizeof( tbi );
	for ( int i = 0; i < ButtonCount(); i++ )
	{
		// We only want to know it's style.
		tbi.dwMask = TBIF_BYINDEX | TBIF_STYLE;
		if ( GetButtonInfo( i, tbi ) > 0 )
		{
			// Is this a separator?
			if ( tbi.fsStyle == BTNS_SEP )
			{
				// Obtain it's rectangle.
				ClsRect rc;
				if ( GetItemRect( i, rc ))
					// Render the separator background.
					dc.FillRect( rc, b1 );

				// Adjust the rectangle for the
				// separator.
				rc.Deflate( 0, 1 );
				rc.Left() += ( rc.Width() / 2 ) - 1;
				rc.Right() = rc.Left() + 1;

				// Render the separator.
				dc.FillRect( rc, b2 );
			}
		}
	}
}

//	Toolbar buttons. I use the data field
//	to store the button tooltip texts to keep
//	things simple.
#define TBSTYLE_DROPBT		TBSTYLE_BUTTON | TBSTYLE_DROPDOWN
TBBUTTON	BCToolbar::m_tbTools[ 14 ] = 
{
	{ STD_FILEOPEN,		IDM_OPEN,	TBSTATE_ENABLED,  TBSTYLE_DROPBT, { 0, 0 }, ( DWORD )_T( "Open" ),		0 },
	{ 0,      IDM_RELOAD_FILE,	0,		  TBSTYLE_BUTTON, { 0, 0 }, ( DWORD )_T( "Reload" ),		0 },
	{ 0,			0,		TBSTATE_ENABLED,  TBSTYLE_SEP,    { 0, 0 }, 0,					0 },
	{ STD_COPY,		IDM_COPY,	0,		  TBSTYLE_BUTTON, { 0, 0 }, ( DWORD )_T( "Copy" ),		0 },
	{ 0,      IDM_FINDINFILES,	TBSTATE_ENABLED,		  TBSTYLE_BUTTON, { 0, 0 }, ( DWORD )_T( "Find in Files" ),		0 },
	{ 0,      IDM_FIND,	0,		  TBSTYLE_BUTTON, { 0, 0 }, ( DWORD )_T( "Find" ),		0 },
	{ 0,			0,		TBSTATE_ENABLED,  TBSTYLE_SEP,    { 0, 0 }, 0,					0 },
	{ 0,			IDM_TOGGLEBM,	0,		  TBSTYLE_BUTTON, { 0, 0 }, ( DWORD )_T( "Toggle Bookmark" ),	0 },
	{ 0,			IDM_NEXTBM,	0,		  TBSTYLE_BUTTON, { 0, 0 }, ( DWORD )_T( "Go To Next Bookmark" ),0 },
	{ 0,			IDM_PREVBM,	0,		  TBSTYLE_BUTTON, { 0, 0 }, ( DWORD )_T( "Go To Previous Bookmark" ),	0 },
	{ 0,			IDM_CLEARBM,	0,		  TBSTYLE_BUTTON, { 0, 0 }, ( DWORD )_T( "Clear All Bookmarks" ),0 },
	{ 0,			0,		TBSTATE_ENABLED,  TBSTYLE_SEP,    { 0, 0 }, 0,					0 },	
	{ STD_PRINT,		IDM_PRINT,	0,		  TBSTYLE_BUTTON, { 0, 0 }, ( DWORD )_T( "Print" ),		0 },
	{ 0,		  IDM_SEND_AS_EMAIL,	0,		  TBSTYLE_BUTTON, { 0, 0 }, ( DWORD )_T( "Send as Email" ),		0 },
};