//
//	bcframe.h
//
//	By Jan van den Baard.
//
//	"Brainchild Custom Editor Control" demonstration program.
//	This code is public domain, use and abuse as you desire.
//

class BCChildWnd;
class Brainchild;

class BCFrameWnd : public ClsMDIMainWindow
{
	friend class BCChildWnd;
	friend class Brainchild;
	
	_NO_COPY( BCFrameWnd );
public:
	// Constructor/destructor.
	BCFrameWnd();
	virtual ~BCFrameWnd();

	// Setup the correct menu.
	void SetupMenu();
	void AddDocument( LPCTSTR pszFilename );
	void Quit();

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

	// Helpers.
	BOOL GetKeys();
	BOOL ChangeSwitch( UINT nID );
	BOOL AlreadyOpen( LPCTSTR pszFilename );
	BOOL ExecuteCommand( UINT nID );
	void SaveAs( BCChildWnd *pChild );
	void ParseCommandLine();
	void ConvertShortToFull( LPCTSTR pszFileName );
	void ActivateTab( ClsWindow *pWindow );

	// Data.
	ClsSimpleArray<BCChildWnd *>	m_Childs;		// Array to track the created childs.
	BCStatus			m_Status;		// Status bar.
	BCToolbar			m_Toolbar;		// Tool bar.
	BCMenu				m_FrameMenu;		// Frame menu.
	BCMenu				m_ChildMenu;		// Child menu.
	BCMenu				m_ContextMenu;		// Context menu.
	HIMAGELIST			m_hToolbar;		// Toolbar images.
	HIMAGELIST			m_hPin;			// Pin-toolbar.
	ClsMRU				m_MRUFiles;		// Most-Recently-Used files.
	ClsMRU				m_HotFiles;		// Hot files.
	DWORD				m_dwChildMax;		// Children maximized?
	ClsString			m_CurrPath;		// Path of the current window.
	ClsDotNetTabControl		m_MDITabs;		// DotNet style tabs.
};