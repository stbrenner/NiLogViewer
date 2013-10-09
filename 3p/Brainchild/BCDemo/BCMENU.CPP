//
//	bcmenu.cpp
//
//	By Jan van den Baard.
//
//	"Brainchild Custom Editor Control" demonstration program.
//	This code is public domain, use and abuse as you desire.
//
#include "bcdemo.h"

// Context menu ID->Bitmap index table.
static StrItemBitmap ibContext[] =
{
	24,		IDM_UNDO,
	25,		IDM_REDO,
	5,		IDM_CUT,
	6,		IDM_COPY,
	7,		IDM_PASTE,
	8,		IDM_DELETE,
	SBM_END,	SBM_END
};

// Frame menu ID->Bitmap index table.
static StrItemBitmap ibFrame[] =
{
	0,		IDM_NEW,
	1,		IDM_OPEN,
	7,		IDM_PASTENEW,
	9,		IDM_EXIT,
	10,		IDM_ABOUT,
	11,		IDM_NEWWINDOW,
	SBM_END,	SBM_END
};

// Child menu ID->Bitmap index table.
static StrItemBitmap ibChild[] =
{
	0,		IDM_NEW,
	1,		IDM_OPEN,
	2,		IDM_SAVE,
	3,		IDM_SAVEALL,
	4,		IDM_PRINT,
	5,		IDM_CUT,
	6,		IDM_COPY,
	7,		IDM_PASTE,
	8,		IDM_DELETE,
	9,		IDM_EXIT,
	10,		IDM_ABOUT,
	10,		IDM_ABOUTBRAINCHILD,
	11,		IDM_NEWWINDOW,
	12,		IDM_CASCADE,
	13,		IDM_TILEHORZ,
	14,		IDM_TILEVERT,
	15,		IDM_CLEARBM,
	16,		IDM_NEXTBM,
	17,		IDM_PREVBM,
	18,		IDM_TOGGLEBM,
	19,		IDM_FIND,
	20,		IDM_FINDNEXT,
	21,		IDM_FINDPREV,
	22,		IDM_FIRSTBM,
	23,		IDM_LASTBM,
	24,		IDM_UNDO,
	25,		IDM_REDO,
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
			mi.fMask  = MIIM_TYPE | MIIM_ID;
			mi.fType  = MFT_STRING;

			mi.wID = IDM_UNDO;
			mi.dwTypeData = _T( "&Undo\tCtrl+Z" );
			mi.cch = _tcslen( mi.dwTypeData );
			InsertItem(( UINT )-1, mi, TRUE );

			mi.wID = IDM_REDO;
			mi.dwTypeData = _T( "&Redo\tCtrl+Y" );
			mi.cch = _tcslen( mi.dwTypeData );
			InsertItem(( UINT )-1, mi, TRUE );

			mi.fType = MFT_SEPARATOR;
			InsertItem(( UINT )-1, mi, TRUE );

			mi.fType  = MFT_STRING;
			mi.wID = IDM_CUT;
			mi.dwTypeData = _T( "Cu&t\tCtrl+X" );
			mi.cch = _tcslen( mi.dwTypeData );
			InsertItem(( UINT )-1, mi, TRUE );

			mi.wID = IDM_COPY;
			mi.dwTypeData = _T( "&Copy\tCtrl+C" );
			mi.cch = _tcslen( mi.dwTypeData );
			InsertItem(( UINT )-1, mi, TRUE );

			mi.wID = IDM_PASTE;
			mi.dwTypeData = _T( "&Paste\tCtrl+V" );
			mi.cch = _tcslen( mi.dwTypeData );
			InsertItem(( UINT )-1, mi, TRUE );

			mi.wID = IDM_DELETE;
			mi.dwTypeData = _T( "&Delete\tDel" );
			mi.cch = _tcslen( mi.dwTypeData );
			InsertItem(( UINT )-1, mi, TRUE );

			mi.fType = MFT_SEPARATOR;
			InsertItem(( UINT )-1, mi, TRUE );

			mi.fType  = MFT_STRING;
			mi.wID = IDM_SELECTALL;
			mi.dwTypeData = _T( "Select A&ll\tCtrl+A" );
			mi.cch = _tcslen( mi.dwTypeData );
			InsertItem(( UINT )-1, mi, TRUE );

			mi.fType = MFT_SEPARATOR;
			InsertItem(( UINT )-1, mi, TRUE );

			mi.fType  = MFT_STRING;
			mi.wID = IDM_RECENTFILES;
			mi.dwTypeData = _T( "Recent Files" );
			mi.cch = _tcslen( mi.dwTypeData );
			InsertItem(( UINT )-1, mi, TRUE );

			mi.fType  = MFT_STRING;
			mi.wID = IDM_HOTFILES;
			mi.dwTypeData = _T( "Hot Files" );
			mi.cch = _tcslen( mi.dwTypeData );
			InsertItem(( UINT )-1, mi, TRUE );

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
