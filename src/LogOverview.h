#pragma once
#include "LogFileListCtrl.h"

class ClsStatic;

class CLogOverview :
  public ClsMDIChildWindow
{
	friend class BCFrameWnd;
	_NO_COPY(CLogOverview);

public:
  CLogOverview(void);
  ~CLogOverview(void);

  inline void SetFrameWindow( BCFrameWnd *pFrame ) { m_pFrame = pFrame; m_LogFiles.SetFrameWindow(m_pFrame); }
  void MDIDestroy();
  void SetLogFolder(LPCTSTR lpsz)
  {
    m_LogFiles.SetLogFolder(lpsz); 
    if (m_FolderLink.GetSafeHWND())
    {
      m_FolderLink.SetWindowText(lpsz);
      m_FolderLink.URL() = lpsz;
    }
  }


protected:
  virtual LRESULT OnCreate( LPCREATESTRUCT pCS );
  virtual LRESULT OnMDINCCreate(LPCREATESTRUCT pCS);
  virtual LRESULT OnSize( UINT nSizeType, int nWidth, int nHeight );
  virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
  virtual LRESULT OnMDIActivated( ClsWindow *pDeactivated );
  virtual LRESULT OnMDIDeactivated( ClsWindow *pActivated );
  virtual LRESULT OnCommand( UINT nNotifyCode, UINT nCtrlID, HWND hWndCtrl );
  virtual LRESULT OnClose();

  CLogFileListCtrl  m_LogFiles;
  ClsInfoControl    m_FolderHeader;
	ClsHyperLink	    m_FolderLink;
  ClsButton         m_ChangeButton;
  ClsStatic         m_HeaderBitmap;
  HBITMAP           m_hBitmap;
  ClsInfoControl    m_RightBorder;
  ClsInfoControl    m_LeftBorder;

  BCFrameWnd	*m_pFrame;
};
