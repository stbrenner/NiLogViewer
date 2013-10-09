#pragma once

class CFilterEditCtrl : public ClsEdit
{
  _NO_COPY(CFilterEditCtrl);
public:
  CFilterEditCtrl() {;}

protected:
  virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
};

class CSidebarDlg : public ClsDialog
{
	_NO_COPY(CSidebarDlg);
public:
  CSidebarDlg(void);
  ~CSidebarDlg(void);

  inline void SetFrameWindow( BCFrameWnd *pFrame ) { m_pFrame = pFrame; m_LogFileListCtrl.SetFrameWindow(pFrame); }

  void GetLogFolder(LPTSTR lpsz, int count) {m_LogFileListCtrl.GetLogFolder(lpsz, count);}
  void SetLogFolder(LPCTSTR lpsz);

  void GetFilter(LPTSTR lpsz, int count) {m_LogFileListCtrl.GetFilter(lpsz, count);}
  void SetFilter(LPCTSTR lpsz) {
    m_FilterBox.SetWindowText(lpsz);
    m_LogFileListCtrl.SetFilter(lpsz);}

  void GetSelectedLog(LPTSTR lpsz, unsigned int count) {m_LogFileListCtrl.GetSelectedLog(lpsz, count);}

  void SetAnimateDownload(BOOL bRun = TRUE);

protected:
	virtual LRESULT OnInitDialog( PROPSHEETPAGE *p );
  virtual LRESULT OnSize( UINT nSizeType, int nWidth, int nHeight );
  virtual LRESULT OnCommand( UINT nNotifyCode, UINT nCtrlID, HWND hWndCtrl );
  virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
  virtual LRESULT OnClose();

  CLogFileListCtrl m_LogFileListCtrl;
	ClsHyperLink	   m_FolderLink;
  ClsFlatButton    m_SmallButton;
  ClsInfoBar       m_SidebarHeader;
  ClsStatic        m_FolderHeader;
  ClsStatic        m_FilterText;
  CFilterEditCtrl  m_FilterBox;
  ClsButton        m_FilterButton;
  ClsButton        m_ChangeFolderBtn;
  ClsAnimation     m_Animate;

  TCHAR m_szLogFolder[MAX_PATH];
  BCFrameWnd* m_pFrame;
};
