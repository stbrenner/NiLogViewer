//
//	bctoolbar.h
//
//	By Jan van den Baard.
//
//	"Brainchild Custom Editor Control" demonstration program.
//	This code is public domain, use and abuse as you desire.
//

class BCToolbar : public ClsToolbar
{
	_NO_COPY( BCToolbar );
public:
	// Construction, destruction...
	BCToolbar();
	virtual ~BCToolbar();

	// Implementation. 
	BOOL Create( ClsWindow *pParent, ClsMRU *pMRU );
	inline BOOL& OldStyle() { return ( BOOL& )m_bOldStyle; }

protected:
	// Overidables.
	virtual LRESULT OnDropDown( LPNMTOOLBAR pToolbar, BOOL &bNotifyParent );
	virtual LRESULT OnGetInfoTip( LPNMTBGETINFOTIP pInfoTip, BOOL& bNotifyParent );
	virtual LRESULT OnCustomDraw( LPNMCUSTOMDRAW pCustomDraw, BOOL& bNotifyParent );
	virtual LRESULT OnPaint( ClsDC *pDC );
	virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

	// Helpers.
	void RedrawSeparators();

	// Data.
	static TBBUTTON		m_tbTools[ 29 ];
	ClsBitmap		m_Tools;
	ClsMRU		       *m_pMRU;
	BOOL			m_bOldStyle;
};
