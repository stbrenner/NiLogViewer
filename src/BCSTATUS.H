//
//	bcstatus.h
//
//	By Stephan Brenner.
//
//	NI Log Viewer.
//	This code is public domain, use and abuse as you desire.
//

// A ClsStatusbar derived class which handles the statusbar
// for this application.
class BCStatus : public ClsStatusbar
{
	_NO_COPY( BCStatus );
public:
	// Constructor.
	BCStatus();

	// Destructor.
	virtual ~BCStatus();

	// Implementation.
	void MultiPart( BOOL bMultiPart );
	void SetLineCol( int nLine, int nColumn );

protected:
	// Helpers.
	void RefreshParts();

	// Overidables.
	virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual LRESULT OnSize( UINT nSizeType, int nWidth, int nHeight );
	virtual LRESULT OnDestroy();
	virtual LRESULT OnCreate( LPCREATESTRUCT pCS );

	// Data.
	int	m_nTimerTicks;
	int	m_nTimerID;
	BOOL	m_bMultiPart;
};