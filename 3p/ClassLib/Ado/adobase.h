#ifdef _IMPORT_ADO_
#ifndef _ADOBASE_H_
#define _ADOBASE_H_
//
// adobase.h
//
// (C) Copyright 2003 Jan van den Baard.
//     All Rights Reserved.
//

#include "../standard.h"
#include "../strings/string.h"
#include "../exceptions/exception.h"
#include "datetime.h"
#include "currency.h"
#include <comutil.h>
#include <comdef.h>
#include <math.h>
#include <icrsint.h>

// Base class for Carlos Antollini's Ado2 and AdoX classes.
class ClsADOBase
{
	_NO_COPY( ClsADOBase );
public:
	// Constructor/destructor, NO-OP.
	ClsADOBase() : m_bThrow( FALSE ) {;}
	virtual ~ClsADOBase() {;}

	// Implementation.
	inline ClsString GetLastErrorString() { return m_strLastError; }
	inline DWORD GetLastError() { return m_dwLastError; }
	inline ClsString GetLastErrorDescription() { return m_strErrorDescription; }
	inline void ThrowComErrors( BOOL bThrow = TRUE ) { m_bThrow = bThrow; }

protected:
	// Test HRESULT return code.
	inline static void CheckHResult( HRESULT hr ) 
	{ 
		// Failure?
		if FAILED( hr )
			// Issue the error
			_com_issue_error( hr ); 
	}

	// Update error statistics.
	void dump_com_error( _com_error &e )
	{
		// Obtain error source and description.
		_bstr_t bstrSource( e.Source());
		_bstr_t bstrDescription( e.Description());
		
		// Format the output.
		m_strLastError.Format( m_strClassName + _T( " Error:\r\n\r\n\tCode = %08lx\r\n\tCode description = %s\r\n\tSource = %s\r\n\tDescription = %s\r\n" ),
				       e.Error(), e.ErrorMessage(), ( LPCSTR )bstrSource, ( LPCSTR )bstrDescription );

		// Setup error information.
		m_strErrorDescription = ( LPCSTR )bstrDescription;
		m_dwLastError         = e.Error();

#ifdef _DEBUG
		// Output trace message.
		XTRACE( m_strLastError );
#endif
		// Throw the error so the application can handle it?
		if ( m_bThrow )
			// Throw...
			throw e;
	}

	// Data.
	ClsString	m_strClassName;
	ClsString	m_strErrorDescription;
	ClsString	m_strLastError;
	DWORD		m_dwLastError;
	BOOL		m_bThrow;
};

#endif // _ADOBASE_H_
#endif // _IMPORT_ADO_
