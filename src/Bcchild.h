//
//	bcchild.h
//
//	By Stephan Brenner.
//
//	NI Log Viewer.
//	This code is public domain, use and abuse as you desire.
//

class BCChildWnd;
class BCFrameWnd;

class Brainchild : public ClsBrainchild
{
	friend class BCChildWnd;
	friend class BCFrameWnd;
	
	_NO_COPY( Brainchild );
public:
  Brainchild() : m_iLineNumber(0), m_iColumnNumber(0) {;}
	virtual ~Brainchild() {;}

	// Implementation.
	inline void SetOwner( BCChildWnd *pOwner ) { m_pOwner = pOwner; }
	inline int GetLineNumber() { return m_iLineNumber; }
	inline int GetColumnNumber() { return m_iColumnNumber; }

protected:
	// Overidables...
	virtual LRESULT OnCaretPosition( LPNMCARETPOSITION pCaretPos, BOOL& bNotifyParent );
	virtual LRESULT OnStatusUpdate( LPNMSTATUSUPDATE pStatusUpd, BOOL& bNotifyParent );
	virtual LRESULT OnFilenameChanged( LPNMFILENAMECHANGED pFilenameChg, BOOL& bNotifyParent );
	virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
	
	// Data.
	BCChildWnd	*m_pOwner;
  int          m_iLineNumber;   // Current line
  int          m_iColumnNumber;   // Current column
};

class BCChildWnd : public ClsMDIChildWindow
{
	friend class BCFrameWnd;
	friend class Brainchild;
	friend class CFindListCtrl;

	_NO_COPY( BCChildWnd );
public:
	// Construction/destruction.
	BCChildWnd();
	virtual ~BCChildWnd();

	// Implementation...
	void UpdateFileInfo();
	void SetTitle();
	inline void SetFrameWindow( BCFrameWnd *pFrame ) { m_pFrame = pFrame; }
	inline Brainchild *GetBrainchild() { return &m_Brainchild; }

protected:
	// Overidables.
	virtual LRESULT OnCreate( LPCREATESTRUCT pCS );
	virtual LRESULT OnMDINCCreate( LPCREATESTRUCT pCS );	
	virtual LRESULT OnSize( UINT nSizeType, int nWidth, int nHeight );
	virtual LRESULT OnClose();
	virtual LRESULT OnEraseBkgnd( ClsDC *pDC ) { return 0; }
	virtual LRESULT OnMDIActivated( ClsWindow *pDeactivated );
	virtual LRESULT OnMDIDeactivated( ClsWindow *pActivated );
	virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

	// Data.
	BCFrameWnd	*m_pFrame;	// Frame window.
	Brainchild	 m_Brainchild;	// Brainchild Control.
};
