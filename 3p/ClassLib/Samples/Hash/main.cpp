//
// Hash sample.
//
// Shows the usage of the ClsHashTable class.
//
#include <classes/all.h>
#include <conio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// A simple ClsHashEntry derived class which 
// contains a string.
class keyword : public ClsHashEntry
{
public:
	ClsString	m_str;
};

// A ClsHashTable derived class which has the
// OnGetHashIndex() and OnCompareHashes() overridden.
class keywords : public ClsHashTable<keyword>
{
protected:
	virtual int OnGetHashIndex( keyword *p )
	{
		// We use a simple 16bit CRC checksum as the hash
		// index.
		ClsCrc16	crc;
		int nIndex = crc.Crc(( LPBYTE )(( LPCTSTR )p->m_str ), p->m_str.GetStringLength());

		// Make sure we do not go out of bounds.
		nIndex &= ( m_nSize - 1 );

		// Return the index.
		return nIndex;
	}

	virtual BOOL OnCompareHashes( keyword *p1, keyword *p2 )
	{
		// Compare the two strings.
		return p1->m_str.IsEqual( p2->m_str );
	}
};

// Entry point.
int main( int argc, char **argv )
{
	// Brace the code so that the _CrtDumpMemoryLeaks()
	// call is made after the objects are destroyed.
	{
		// Create a hash table with 256 entries.
		keyword add;
		keywords keys;
		keys.ConstructTable( 256 );

		// Create hash entries.
		_tprintf( _T( "Creating 4096 hashes..." ));
		for ( int i = 0; i < 4096; i++ )
		{
			add.m_str.Format( _T( "This is string %ld" ), i );
			keys.AddEntry( add );
		}
		
		_tprintf( _T( "\nDone!\nPress return to continue." ));
		_gettchar();
		_tprintf( _T( "\n\nComputing spread...\n" ));

		int pos = 0;
		ClsIntArray spread;
		spread.SetSize( keys.GetTableSize(), 0 );
		keyword *p = keys.GetFirstHash( pos );
		if ( p )
		{
			do 
			{
				spread[ pos ]++;
				p = keys.GetNextHash( p, pos );
			}
			while ( p );
		}

		_tprintf( _T( "\nDone!\nPress return to continue." ));
		_gettchar();

		for ( i = 0; i < keys.GetTableSize(); i += 8 )
		{
			for ( int a = 0; a < 8; a++ )
			{
				_tprintf( _T( "%3ld = %3ld " ), i + a, spread[ i + a ] );
			}
			_tprintf( _T( "\n" ));
		}
		_tprintf( _T( "Press return to continue." ));
		_gettchar();
	}
	_CrtDumpMemoryLeaks();
	return 0;
}