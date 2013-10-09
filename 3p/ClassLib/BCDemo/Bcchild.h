//
//	bcchild.h
//
//	By Jan van den Baard.
//
//	"Brainchild Custom Editor Control" demonstration program.
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
	Brainchild() {;}
	virtual ~Brainchild() {;}

	// Implementation.
	inline void SetOwner( BCChildWnd *pOwner ) { m_pOwner = pOwner; }

protected:
	// Overidables...
	virtual LRESULT OnCaretPosition( LPNMCARETPOSITION pCaretPos, BOOL& bNotifyParent );
	virtual LRESULT OnStatusUpdate( LPNMSTATUSUPDATE pStatusUpd, BOOL& bNotifyParent );
	virtual LRESULT OnFilenameChanged( LPNMFILENAMECHANGED pFilenameChg, BOOL& bNotifyParent );
	virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
	
	// Data.
	BCChildWnd	*m_pOwner;
};

class BCChildWnd : public ClsMDIChildWindow
{
	friend class BCFrameWnd;
	friend class Brainchild;

	_NO_COPY( BCChildWnd );
public:
	// Construction/destruction.
	BCChildWnd();
	virtual ~BCChildWnd();

	// Implementation...
	void UpdateFileInfo();
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
