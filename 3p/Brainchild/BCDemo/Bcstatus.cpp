//
//	bcstatus.cpp
//
//	By Jan van den Baard.
//
//	"Brainchild Custom Editor Control" demonstration program.
//	This code is public domain, use and abuse as you desire.
//
#include "bcdemo.h"

// Timer.
#define TIMER_CLOCK	900

// Constructor.
BCStatus::BCStatus() 
{ 
	// Setup defaults.
	m_hClock      = ( HICON )::LoadImage( ClsGetResourceHandle(), MAKEINTRESOURCE( IDI_CLOCK ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
	m_bMultiPart  = FALSE;
	m_nTimerTicks = 0;
	m_nTimerID    = 0;
}

// Destructor.
BCStatus::~BCStatus() 
{
	// Destroy icon if loaded.
	if ( m_hClock ) ::DestroyIcon( m_hClock );
}

// Set/clear multi-part flag.
void BCStatus::MultiPart( BOOL bMultiPart )
{
	// Did it change?
	if ( bMultiPart != m_bMultiPart )
	{
		// Change it.
		m_bMultiPart = bMultiPart;

		// Clear panes.
		for ( int i = 0; i <= ( bMultiPart ? 5 : 1 ); i++ )
		{
			SetText( i, 0, 0 );
			SetIcon( i, NULL );
		}

		// Refresh parts.
		RefreshParts();
	}
}

// Set line/column panes.
void BCStatus::SetLineCol( int nLine, int nColumn )
{
	// Are we multipart?
	if ( m_bMultiPart )
	{
		// Setup panes.
		ClsString str;
		str.Format( _T( "Ln %ld" ), nLine );
		SetText( 3, 0, str );
		str.Format( _T( "Col %ld" ), nColumn );
		SetText( 4, 0, str );
		UpdateWindow();
	}
}

// Set insertmode pane.
void BCStatus::SetInsertMode( BOOL bInsert )
{
	// Are we multipart?
	if ( m_bMultiPart )
		SetText( 1, 0, ( bInsert ? _T( "INS" ) : _T( "OVR" )));
}

// Set filemode pane.
void BCStatus::SetFileMode( int nFileMode )
{
	_ASSERT( nFileMode >= 0 && nFileMode <= 3 ); // Make sure we're in range...

	// Are we multipart?
	if ( m_bMultiPart )
	{
		// Possible modes...
		static LPTSTR pszMode[ 3 ] =
		{
			_T( "MS DOS" ),
			_T( "Unix" ),
			_T( "Macintosh" )
		};

		// Set the mode pane.
		SetText( 2, 0, pszMode[ nFileMode ] );
	}
}

// Update clock part.
void BCStatus::UpdateClock()
{
	TCHAR	szBuffer[ 64 ];

	// Format the time.
	if ( ::GetTimeFormat( LOCALE_USER_DEFAULT, TIME_NOSECONDS, NULL, NULL, szBuffer, 64 ))
		// Update the pane.
		SetText( m_bMultiPart ? 5 : 1, 0, szBuffer );
}

// Refresh statusbar panes.
void BCStatus::RefreshParts()
{
	#define MAXPARTS 6
	int nParts[ MAXPARTS ];

	// Obtain statusbar client rectangle.
	ClsRect rcStatus;
	GetClientRect( rcStatus );

	// Create DC.
	ClsGetDC dc( this );
	ClsSelector sel( &dc, ( HFONT )SendMessage( WM_GETFONT ));

	// Compute maximum pane sizes...
	TCHAR sz[ 64 ];
	int nFrame  = 3 * ::GetSystemMetrics( SM_CXFRAME );
	int nNum    = ::GetTimeFormat( LOCALE_USER_DEFAULT, TIME_NOSECONDS, NULL, NULL, sz, 64 );
	int nClock  = nFrame + dc.GetTextExtent( sz, nNum ).CX() + 32;
	int nColumn = nFrame + dc.GetTextExtent( _T( "Col 00000000" ), 11 ).CX();
	int nLine   = nFrame + dc.GetTextExtent( _T( "Ln 00000000"), 10 ).CX();
	int nType   = nFrame + dc.GetTextExtent( _T( "Macintosh" ), 8 ).CX();
	int nOvr    = nFrame + dc.GetTextExtent( _T( "OVR" ), 3 ).CX();
	
	// Multipart
	if ( ! m_bMultiPart )
	{
		// Setup panes.
		nParts[ 0 ] = rcStatus.Right() - nClock;
		nParts[ 1 ] = -1;
	}
	else
	{
		// Setup panes.
		nParts[ 0 ] = rcStatus.Right() - ( nOvr + nType + nLine + nColumn + nClock );
		nParts[ 1 ] = rcStatus.Right() - ( nType + nLine + nColumn + nClock );
		nParts[ 2 ] = rcStatus.Right() - ( nLine + nColumn + nClock );
		nParts[ 3 ] = rcStatus.Right() - ( nColumn + nClock );
		nParts[ 4 ] = rcStatus.Right() - ( nClock );
		nParts[ 5 ] = rcStatus.Right();//-1;
	}

	// Setup the parts.
	SetParts( m_bMultiPart ? MAXPARTS : 2, nParts );

	// Setup the clock icon.
	if ( m_hClock ) SetIcon( m_bMultiPart ? 5 : 1, m_hClock );

	// Update the clock.
	UpdateClock();
}

// Window procedure.
LRESULT BCStatus::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
		case	WM_TIMER:
			// Update clock pane.
			if ( wParam == TIMER_CLOCK )
			{
				// Refresh parts?
				if ( m_nTimerTicks++ > 10 ) RefreshParts();
				else			    UpdateClock();
				return 0;
			}
			break;
	}
	// Pass to the base class.
	return ClsStatusbar::WindowProc( uMsg, wParam, lParam );
}


// WM_SIZE message handler.
LRESULT BCStatus::OnSize( UINT nSizeType, int nWidth, int nHeight )
{
	// First the base.
	LRESULT rc =  ClsStatusbar::OnSize( nSizeType, nWidth, nHeight );

	// Create the timer if not done already.
	if ( m_nTimerID == 0 )
		m_nTimerID = SetTimer( TIMER_CLOCK, 1000 );

	// Update panes.
	RefreshParts();
	return rc;
}

// WM_DESTROY handler.
LRESULT BCStatus::OnDestroy()
{
	// Kill the timer.
	if ( m_nTimerID ) 
		KillTimer( TIMER_CLOCK );

	// Call the baseclass.
	return ClsStatusbar::OnDestroy();
}

// WM_CREATE message handler.
LRESULT BCStatus::OnCreate( LPCREATESTRUCT pCS )
{
	// First let the baseclass have a go at it.
	LRESULT rc = ClsStatusbar::OnCreate( pCS );

	// OK?
	if ( rc != -1 )
		// Update panes.
		RefreshParts();
	return rc;
}