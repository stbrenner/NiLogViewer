#include "bcdemo.h"

CLogOverview::CLogOverview(void) :
  m_pFrame(NULL)
{
}

CLogOverview::~CLogOverview(void)
{
  DeleteObject(m_hBitmap);
}

// WM_CREATE handler.
LRESULT CLogOverview::OnCreate( LPCREATESTRUCT pCS )
{
	// Pass on to the base class.
	return ClsMDIChildWindow::OnCreate( pCS );
}

// WM_NCCREATE handler. This is called not by the "ClsWindow" message
// dispatcher but by the "ClsMDIChildWindow" message dispatcher.
LRESULT CLogOverview::OnMDINCCreate(LPCREATESTRUCT pCS)
{
	// Make sure the child has a client edge. NOTE: Simply adding this
	// bit to the CREATESTRUCT.dwExStyle will not work...
	//
	// We do it here like this also because the PreCreateWindow() overide
	// will not be called when creating MDI child windows.
	ModifyExStyle( 0, WS_EX_CLIENTEDGE);

  m_HeaderBitmap.Create(this, ClsRect(0, 0, 0, 0), (UINT)-1, _T(""), WS_CHILD | WS_VISIBLE | SS_BITMAP );
  m_hBitmap = ::LoadBitmap(ClsGetInstanceHandle(), MAKEINTRESOURCE(IDB_HEADER) );
  m_HeaderBitmap.SetBitmap(m_hBitmap);

	m_FolderHeader.Create(this, ClsRect(0, 0, 0, 0), (UINT)-1, ISEQ_RIGHT _T("Log File Folder:") );

  TCHAR szFolder[MAX_PATH];
  m_pFrame->GetLogFolder(szFolder, MAX_PATH);
  m_FolderLink.Create(this, ClsRect(0, 0, 0, 0), (UINT)-1, szFolder);
  m_FolderLink.URL() = szFolder;
  m_FolderLink.ShowToolTip();
  m_FolderLink.VisitedColor() = RGB(0, 0, 0);
  m_FolderLink.HotColor() = RGB(0, 0, 255);
  m_FolderLink.NormalColor() = RGB(0, 0, 0);

  m_ChangeButton.Create(this, ClsRect(0, 0, 0, 0), IDB_FOLDERCHANGE, _T("..."));

  m_LogFiles.Create(this, ClsRect(0, 0, 0, 0), (UINT)-1, WS_TABSTOP |
    WS_CHILD | WS_VISIBLE);

  m_RightBorder.Create(this, ClsRect(0, 0, 0, 0), (UINT)-1, NULL);
	m_RightBorder.BackgroundColor() = RGB(0, 0, 128);

  m_LeftBorder.Create(this, ClsRect(0, 0, 0, 0), (UINT)-1, NULL);
	m_LeftBorder.BackgroundColor() = RGB(0, 0, 128);

	return TRUE;
}

// Handle the WM_SIZE messages.
LRESULT CLogOverview::OnSize( UINT nSizeType, int nWidth, int nHeight )
{
	// Resize controls
  static const int nBorder = 10;
  int nW = nWidth > 600 ? 600 : nWidth;
  int nRightBorder = (nWidth - nW)/2;
  int nLeftBorder = nWidth - nW - nRightBorder;
	m_HeaderBitmap.SetWindowPos( NULL, nLeftBorder, 0, 600, 47, SWP_NOZORDER );
	m_FolderHeader.SetWindowPos( NULL, nLeftBorder + nBorder, 60, 80, 20, SWP_NOZORDER );
	m_FolderLink.SetWindowPos( NULL, nLeftBorder + 80 + nBorder, 60, nW - 30 - 90 - nBorder, 20, SWP_NOZORDER );
	m_ChangeButton.SetWindowPos( NULL, nLeftBorder + nW - 30 - nBorder, 60, 30, 20, SWP_NOZORDER );
  if (m_LogFiles.GetSafeHWND())
    m_LogFiles.SetWindowPos( NULL, nLeftBorder + nBorder, 90, nW - nBorder*2, nHeight - 100, SWP_NOZORDER );
  m_RightBorder.SetWindowPos( NULL, nW + nLeftBorder, 0, 1, nHeight, SWP_NOZORDER );
  m_LeftBorder.SetWindowPos( NULL, nLeftBorder - 1, 0, 1, nHeight, SWP_NOZORDER );

	return ClsMDIChildWindow::OnSize( nSizeType, nWidth, nHeight );
}

// Handle the WM_CLOSE messages.
LRESULT CLogOverview::OnClose()
{
	MDIDestroy();
	return 0;
}

void CLogOverview::MDIDestroy()
{
  m_LogFiles.Destroy();
  ClsMDIChildWindow::MDIDestroy();
}

// Window procedure...
LRESULT CLogOverview::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  // What's up?
  switch ( uMsg )
  {
  case	WM_SETFOCUS:
    if (m_LogFiles.GetSafeHWND())
      m_LogFiles.SetFocus();
    break;
  case  WM_PAINT:   // Avoid flickering
    if (m_LogFiles.GetCurrKey() > 16 && m_LogFiles.GetCurrKey() < 41)   
      ValidateRect(GetClientRect());
    break;
  }
  // On to the base class.
  return ClsMDIChildWindow::WindowProc( uMsg, wParam, lParam );
}

// We are activated...
LRESULT CLogOverview::OnMDIActivated( ClsWindow *pDeactivated )
{
  if (!m_pFrame)
    return 0;

	// Buttons to be disabled
	static UINT ids[] =
	{
    IDM_COPY,
		IDM_FIND,
		IDM_FINDNEXT,
		IDM_FINDPREV,
    IDM_CLEARBM,
    IDM_FIRSTBM,
    IDM_NEXTBM,
    IDM_PREVBM,
    IDM_LASTBM,
    IDM_TOGGLEBM,
    IDM_GOTO,
    IDM_SELECTALL,
    IDM_PRINT,
		IDM_PAGESETUP,
    IDM_RELOAD_FILE,
    IDM_OVERVIEW,
    //IDM_OPEN_SELECTED_PATH,
		0xFFFFFFFF
	};

	for ( int i = 0; ids[ i ] != 0xFFFFFFFF; i++ )
  {
		m_pFrame->m_Toolbar.EnableButton( ids[ i ], FALSE );
    m_pFrame->m_ChildMenu.EnableItem( ids[ i ], MF_BYCOMMAND | MF_GRAYED );
  }

	// Enable buttons
	static UINT ids2[] =
	{
		IDM_CLOSE,
		0xFFFFFFFF
	};

	// Iterate button IDs and enable them.
	for ( int i = 0; ids2[ i ] != 0xFFFFFFFF; i++ )
  {
		m_pFrame->m_Toolbar.EnableButton( ids2[ i ], TRUE );
    m_pFrame->m_ChildMenu.EnableItem( ids2[ i ], MF_BYCOMMAND | MF_ENABLED );
  }

  m_pFrame->m_Status.MultiPart(FALSE);

	return 0;
}

// We are deactivated.
LRESULT CLogOverview::OnMDIDeactivated( ClsWindow * pActivated )
{
	// Buttons.
	static UINT ids[] =
	{
		IDM_CLOSE,
		0xFFFFFFFF
	};

	// Disable buttons.
	if ( m_pFrame )
	{
		// Iterate button IDs and disable them.
		for ( int i = 0; ids[ i ] != 0xFFFFFFFF; i++ )
    {
			m_pFrame->m_Toolbar.EnableButton( ids[ i ], FALSE );
      m_pFrame->m_ChildMenu.EnableItem( ids[ i ], MF_BYCOMMAND | MF_GRAYED );
    }
	}

  m_LogFiles.m_iCurrKey = 0;

	return 0;}

// WM_COMMAND message handler.
LRESULT CLogOverview::OnCommand( UINT nNotifyCode, UINT nCtrlID, HWND hWndCtrl )
{
  switch ( nCtrlID )
	{
  case IDB_FOLDERCHANGE:
    m_pFrame->SendMessage(WM_COMMAND, IDM_CHANGE_LOGFOLDER);
    break;
  }

  // Pass to the base class.
	return ClsMDIChildWindow::OnCommand( nNotifyCode, nCtrlID, hWndCtrl );
}