//
//	bcstatus.h
//
//	By Jan van den Baard.
//
//	"Brainchild Custom Editor Control" demonstration program.
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
	void SetInsertMode( BOOL bInsert );
	void SetFileMode( int nFileMode );

protected:
	// Helpers.
	void UpdateClock();
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
	HICON	m_hClock;
};