#include "stdafx.h"

LRESULT CFilterEditCtrl::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch ( uMsg )
  {
  case WM_GETDLGCODE:
    if (wParam == 13)   // Return key
      GetParent()->SendMessage(WM_COMMAND, IDC_FILTER_BUTTON);
    break;
  }

  // On to the base class.
  return ClsEdit::WindowProc( uMsg, wParam, lParam );
}

CSidebarDlg::CSidebarDlg(void)
{
}

CSidebarDlg::~CSidebarDlg(void)
{
}

void CSidebarDlg::SetLogFolder(LPCTSTR lpsz)
{
  m_LogFileListCtrl.SetLogFolder(lpsz);
  m_FolderLink.URL() = lpsz;
  if (m_FolderLink.GetSafeHWND())
    m_FolderLink.SetWindowText(lpsz);
}

LRESULT CSidebarDlg::OnInitDialog( PROPSHEETPAGE *p )
{
  m_FolderHeader.Attach(GetDlgItemHandle(IDC_FOLDER_HEADER));

  TCHAR szFolder[MAX_PATH];
  GetLogFolder(szFolder, MAX_PATH);
  m_FolderLink.Attach(GetDlgItemHandle( IDC_LOGFOLDER_LINK ));
  m_FolderLink.SetWindowText(szFolder);
  m_FolderLink.ShowToolTip();
  m_FolderLink.VisitedColor() = m_FolderLink.NormalColor();

  m_SmallButton.Attach(GetDlgItemHandle( IDB_SMALL_SIDEBAR ));
  m_SmallButton.SetXPStyle();
  m_SmallButton.Tip() = ClsString(_T("Show/Hide Sidebar (Ctrl+S)"));
  m_SmallButton.ShowToolTip();

  m_SidebarHeader.Attach( GetDlgItemHandle( IDC_SIDEBAR_HEADER ));
	m_SidebarHeader.Gradient() = TRUE;
	m_SidebarHeader.BackgroundColor() = ::GetSysColor( COLOR_INACTIVECAPTION );
	m_SidebarHeader.BackgroundGradient() = ::GetSysColor( COLOR_GRADIENTINACTIVECAPTION );

  m_FilterText.Attach(GetDlgItemHandle(IDC_FILTER_TEXT));
  m_FilterBox.Attach(GetDlgItemHandle(IDC_FILTER_BOX));
  m_FilterButton.Attach(GetDlgItemHandle(IDC_FILTER_BUTTON));

  m_ChangeFolderBtn.Attach(GetDlgItemHandle(IDC_CHANGE_FOLDER_BUTTON));

  m_Animate.Attach(GetDlgItemHandle( IDC_ANIMATE ));
  m_Animate.Open((LPTSTR)IDR_DOWNLOAD_AVI);
  m_Animate.Play();
  
  m_LogFileListCtrl.Attach(GetDlgItemHandle( IDC_LOGFILE_LIST ));

  return ClsDialog::OnInitDialog(p);
}

LRESULT CSidebarDlg::OnSize( UINT nSizeType, int nWidth, int nHeight )
{
  TCHAR szSmallBtnText[10];
  m_SmallButton.GetWindowText(szSmallBtnText, 10);

  // Sidebar has normal size
  if (nWidth > MIN_SIDEBAR_SIZE)
  {
    if (_tcsicmp(szSmallBtnText, _T("<<")))
      m_SmallButton.SetWindowText(_T("<<"));

    m_SidebarHeader.SetWindowPos(NULL, 0, 0, nWidth - 23, 20, SWP_NOZORDER);
    m_SidebarHeader.Vertical() = FALSE;
    m_SmallButton.SetWindowPos(NULL, nWidth-22, 0, 22, 20, SWP_NOZORDER);

    m_FolderHeader.SetWindowPos(NULL, 5, 28, 35, 17, SWP_NOZORDER);
    m_FolderLink.SetWindowPos(NULL, 40, 25, nWidth-40, 20, SWP_NOZORDER);

    m_FilterText.SetWindowPos(NULL, 5, 53, 30, 20, SWP_NOZORDER);
    m_FilterBox.SetWindowPos(NULL, 35, 50, 80, 20, SWP_NOZORDER);
    m_FilterButton.SetWindowPos(NULL, 118, 50, 30, 20, SWP_NOZORDER);

    if (nWidth < 270)
    {
      m_Animate.SetWindowPos(NULL, 150, 50, 20, 20, SWP_NOZORDER);
      m_ChangeFolderBtn.SetWindowPos(NULL, 170, 50, 100, 20, SWP_NOZORDER);
    }
    else
    {
      m_Animate.SetWindowPos(NULL, nWidth/2+15, 50, 20, 20, SWP_NOZORDER);
      m_ChangeFolderBtn.SetWindowPos(NULL, nWidth-100, 50, 100, 20, SWP_NOZORDER);
    }

    m_LogFileListCtrl.SetWindowPos(NULL, 0, 75, nWidth, nHeight-75, SWP_NOZORDER);
  }

  // Sidebar is minimized
  else if (nWidth <= MIN_SIDEBAR_SIZE )
  {
    if (_tcsicmp(szSmallBtnText, _T(">>")))
      m_SmallButton.SetWindowText(_T(">>"));

    m_SidebarHeader.SetWindowPos(NULL, 1, 21, 22, 140, SWP_NOZORDER);
    m_SidebarHeader.Vertical() = TRUE;
    m_SmallButton.SetWindowPos(NULL, 1, 0, 22, 20, SWP_NOZORDER);

    m_FolderHeader.SetWindowPos(NULL, MIN_SIDEBAR_SIZE + 10, 0, 0, 0, SWP_NOZORDER);
    m_FolderLink.SetWindowPos(NULL, MIN_SIDEBAR_SIZE + 10, 0, 0, 0, SWP_NOZORDER);

    m_FilterText.SetWindowPos(NULL, MIN_SIDEBAR_SIZE + 10, 0, 0, 0, SWP_NOZORDER);
    m_FilterBox.SetWindowPos(NULL, MIN_SIDEBAR_SIZE + 10, 0, 0, 0, SWP_NOZORDER);
    m_FilterButton.SetWindowPos(NULL, MIN_SIDEBAR_SIZE + 10, 0, 0, 0, SWP_NOZORDER);

    m_Animate.SetWindowPos(NULL, MIN_SIDEBAR_SIZE + 10, 0, 0, 0, SWP_NOZORDER);
    m_ChangeFolderBtn.SetWindowPos(NULL, MIN_SIDEBAR_SIZE + 10, 0, 0, 0, SWP_NOZORDER);

    m_LogFileListCtrl.SetWindowPos(NULL, MIN_SIDEBAR_SIZE + 10, 0, 0, 0, SWP_NOZORDER);  
  }

	return ClsDialog::OnSize( nSizeType, nWidth, nHeight );
}

LRESULT CSidebarDlg::OnCommand( UINT nNotifyCode, UINT nCtrlID, HWND hWndCtrl )
{
 	switch ( nCtrlID )
	{
    case IDB_SMALL_SIDEBAR:
    {
      TCHAR szText[10];
      m_SmallButton.GetWindowText(szText, 10);
      if (!_tcsicmp(szText, _T("<<")))
      {
        m_pFrame->MinimizeSplitter();
        m_SmallButton.SetWindowText(_T(">>"));
      }
      else
      {
        m_pFrame->MaximizeSplitter();
        m_SmallButton.SetWindowText(_T("<<"));
      }
      m_LogFileListCtrl.SetFocus();
      break;
    }
    case IDC_CHANGE_FOLDER_BUTTON:
      m_pFrame->SendMessage(WM_COMMAND, IDM_CHANGE_LOGFOLDER);
      break;
    case IDC_FILTER_BUTTON:
    {
      TCHAR szFilter[MAX_PATH];
      m_FilterBox.GetWindowText(szFilter, MAX_PATH);
      m_LogFileListCtrl.SetFilter(szFilter);
      break;
    }
  }

	return ClsDialog::OnCommand( nNotifyCode, nCtrlID, hWndCtrl );
}

LRESULT CSidebarDlg::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  // What's up?
  switch ( uMsg )
  {
  case  WM_PAINT:   // Avoid flickering
    if (m_LogFileListCtrl.GetCurrKey() > 17 && m_LogFileListCtrl.GetCurrKey() < 41)   
      ValidateRect(GetClientRect());
    break;
  }

  // On to the base class.
  return ClsDialog::WindowProc( uMsg, wParam, lParam );
}

void CSidebarDlg::SetAnimateDownload(BOOL bRun)
{
  if (!m_Animate.GetSafeHWND())
    return;

  if (bRun)
    m_Animate.ShowWindow(SW_SHOW);
  else
    m_Animate.ShowWindow(SW_HIDE);
}

LRESULT CSidebarDlg::OnClose()
{
  m_LogFileListCtrl.SendMessage(WM_CLOSE);
  return 0;
}