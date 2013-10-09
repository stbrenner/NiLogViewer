//-----------------------------------------------------------------------------
//
//    bcframe.h
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------

class BCChildWnd;
class Brainchild;
class FastFindDlg;
class CSidebarDlg;
class CLogFileListCtrl;

class BCFrameWnd : public ClsMDIMainWindow
{
	friend class BCChildWnd;
	friend class Brainchild;
  friend class CFindListCtrl;
	
	_NO_COPY( BCFrameWnd );
public:
	// Constructor/destructor.
	BCFrameWnd();
	virtual ~BCFrameWnd();

	void AddDocument(LPCTSTR pszFilename);
  void OpenCurrentLog();
	void Quit();

  void GetLogFolder(LPTSTR lpsz, int count) {_tcsncpy(lpsz, m_szLogFolder, count);}
  void SetLogFolder(LPCTSTR lpsz);

  void MinimizeSplitter()
  {
    m_ChildMenu.CheckItem(IDM_SHOWSIDEBAR, MF_UNCHECKED);
    m_dwSplitterWidth = m_pSplitter->GetSplitterPosition();
    m_pSplitter->SetSplitterPosition(MIN_SIDEBAR_SIZE);
  }
  void MaximizeSplitter()
  {
    m_ChildMenu.CheckItem(IDM_SHOWSIDEBAR, MF_CHECKED);
    m_pSplitter->SetSplitterPosition(m_dwSplitterWidth);
  }

protected:
	// Overidables.
	virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual LRESULT OnEraseBkgnd( ClsDC *pDC ) { return 0; }
	virtual void OnMDIChildRemoved( ClsMDIChildWindow *pWnd );
	virtual LRESULT OnClose();
	virtual LRESULT OnSize( UINT nSizeType, int nWidth, int nHeight );
	virtual LRESULT OnCommand( UINT nNotifyCode, UINT nCtrlID, HWND hWndCtrl );
	virtual LRESULT OnCreate( LPCREATESTRUCT pCS );
  virtual LRESULT OnNotify( LPNMHDR pHdr );

  void ReadFolderFromReg();
  void WriteFolderToReg();
  BOOL BrowseForFolder(LPTSTR szFolder);
	
	// Helpers.
	BOOL GetKeys();
	BOOL AlreadyOpen( LPCTSTR pszFilename );
	BOOL ExecuteCommand( UINT nID );
	void ParseCommandLine(LPCTSTR pszPtr);
	void ConvertShortToFull( LPCTSTR pszFileName );
  void RegisterFileExtension();
	void ActivateTab( ClsWindow *pWindow );
	void DeleteLogFiles(IN int iSeconds, IN LPCTSTR lpszMessage);
  void SwitchConfigFlag(IN DWORD dwMenuId, IN BOOL& bConfigValue);
  BOOL IsWindowsVista();

  // Settings
  DWORD m_dwSplitterWidth;
  BOOL  m_bScrollToEndOnFileLoad;
  BOOL  m_bAssociateWithLogExt;
  TCHAR m_szLogFolder[MAX_PATH];

  // Dialogs
  FastFindDlg* m_pFastFindDlg;
  CSidebarDlg* m_pSidebarDlg;

	ClsSimpleArray<BCChildWnd *>	m_Childs;		// Array to track the created childs.
 	ClsSplitter* m_pSplitter;
  BCStatus			m_Status;		// Status bar.
	BCToolbar			m_Toolbar;		// Tool bar.
	BCMenu				m_ChildMenu;		// Child menu.
	BCMenu				m_ContextMenu;		// Context menu.
	HIMAGELIST			m_hToolbar;		// Toolbar images.
	ClsMRU				m_MRUFiles;		// Most-Recently-Used files.
	ClsDotNetTabControl		m_MDITabs;		// DotNet style tabs.
};