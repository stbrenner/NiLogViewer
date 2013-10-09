//
//	bcstatus.cpp
//
//	By Stephan Brenner.
//
//	NI Log Viewer.
//	This code is public domain, use and abuse as you desire.
//
#include "stdafx.h"

// Constructor.
BCStatus::BCStatus() 
{ 
	// Setup defaults.
	m_bMultiPart  = FALSE;
	m_nTimerTicks = 0;
	m_nTimerID    = 0;
}

// Destructor.
BCStatus::~BCStatus() 
{
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
		for ( int i = 0; i <= ( bMultiPart ? 2 : 1 ); i++ )
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
		SetText( 1, 0, str );
		str.Format( _T( "Col %ld" ), nColumn );
		SetText( 2, 0, str );
		UpdateWindow();
	}
}

// Refresh statusbar panes.
void BCStatus::RefreshParts()
{
	#define MAXPARTS 4
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
	::GetTimeFormat( LOCALE_USER_DEFAULT, TIME_NOSECONDS, NULL, NULL, sz, 64 );
	int nColumn = nFrame + dc.GetTextExtent( _T( "Col 00000000" ), 11 ).CX();
	int nLine   = nFrame + dc.GetTextExtent( _T( "Ln 00000000"), 10 ).CX();
	
	// Multipart
	if ( ! m_bMultiPart )
	{
		// Setup panes.
		nParts[ 0 ] = rcStatus.Right();
	}
	else
	{
		// Setup panes.
		nParts[ 0 ] = rcStatus.Right() - ( nLine + nColumn );
		nParts[ 1 ] = rcStatus.Right() - ( nColumn );
		nParts[ 2 ] = rcStatus.Right();
	}

	// Setup the parts.
	SetParts( m_bMultiPart ? MAXPARTS : 2, nParts );
}

// Window procedure.
LRESULT BCStatus::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// Pass to the base class.
	return ClsStatusbar::WindowProc( uMsg, wParam, lParam );
}


// WM_SIZE message handler.
LRESULT BCStatus::OnSize( UINT nSizeType, int nWidth, int nHeight )
{
	// First the base.
	LRESULT rc =  ClsStatusbar::OnSize( nSizeType, nWidth, nHeight );

	// Update panes.
	RefreshParts();
	return rc;
}

// WM_DESTROY handler.
LRESULT BCStatus::OnDestroy()
{
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