//
//	bcmenu.cpp
//
//	By Stephan Brenner.
//
//	NI Log Viewer.
//	This code is public domain, use and abuse as you desire.
//
#include "stdafx.h"

// Context menu ID->Bitmap index table.
static StrItemBitmap ibContext[] =
{
	6,		IDM_COPY,
	SBM_END,	SBM_END
};

// Frame menu ID->Bitmap index table.
static StrItemBitmap ibFrame[] =
{
	1,		IDM_OPEN,
	SBM_END,	SBM_END
};

// Child menu ID->Bitmap index table.
static StrItemBitmap ibChild[] =
{
	1,		IDM_OPEN,
	4,		IDM_PRINT,
	6,		IDM_COPY,
	15,		IDM_CLEARBM,
	16,		IDM_NEXTBM,
	17,		IDM_PREVBM,
	18,		IDM_TOGGLEBM,
	19,		IDM_FIND,
	20,		IDM_FINDNEXT,
	21,		IDM_FINDPREV,
	22,		IDM_FIRSTBM,
	23,		IDM_LASTBM,
  27,   IDM_RELOAD_FILE,
  29,   IDM_SEND_AS_EMAIL,
	SBM_END,	SBM_END
};

// Construction.
BCMenu::BCMenu()
{;}

// Destruction.
BCMenu::~BCMenu() 
{;}

// Initialize this menu.
BOOL BCMenu::Init( LPCTSTR pszMenuName, HIMAGELIST hImages, BOOL bChildren, BOOL bContext )
{
	// Context menu?
	if ( bContext )
	{
		if ( Create( TRUE ))
		{
			// Setup the imagelist.
			SetImageList( hImages );

			// We are a popup menu...
			IsPopupMenu() = TRUE;

			// Create the context menu items.
			ClsMenuItemInfo mi;

			mi.wID = IDM_COPY;
			mi.fType  = MFT_STRING;
			mi.fMask  = MIIM_TYPE | MIIM_ID;
			mi.dwTypeData = _T( "&Copy\tCtrl+C" );
			mi.cch = _tcslen( mi.dwTypeData );
			InsertItem(( UINT )-1, mi, TRUE );

			mi.wID = IDM_SELECTALL;
			mi.fType  = MFT_STRING;
			mi.fMask  = MIIM_TYPE | MIIM_ID;
			mi.dwTypeData = _T( "Select A&ll\tCtrl+A" );
			mi.cch = _tcslen( mi.dwTypeData );
			InsertItem(( UINT )-1, mi, TRUE );

			/*mi.fType = MFT_SEPARATOR;
			mi.fMask  = MIIM_TYPE;
			InsertItem(( UINT )-1, mi, TRUE );

      mi.wID = IDM_OPEN_SELECTED_PATH;
			mi.fType  = MFT_STRING;
			mi.fMask  = MIIM_TYPE | MIIM_ID;
			mi.dwTypeData = _T( "Open Selected Folder/File\tEnter" );
			mi.cch = _tcslen( mi.dwTypeData );
			InsertItem(( UINT )-1, mi, TRUE );*/

			// Convert...
			return	SetItemBitmap( &ibContext[ 0 ] );
		}
		return FALSE;
	}

	// Loadup the menu.
	if ( Load( pszMenuName ))
	{
		// Setup the imagelist.
		SetImageList( hImages );

		// Is this the frame or the child menu?
		StrItemBitmap *pTable = ( bChildren ? &ibChild[ 0 ] : &ibFrame[ 0 ] );

		// Set the bitmaps for this menu.
		if ( SetItemBitmap( pTable ))
			return TRUE;
	}
	return FALSE;
}
