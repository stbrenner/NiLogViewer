/*
 *	hyperlink.c
 *
 *	(C) Copyright 2005 Jan van den Baard.
 *	    All Rights Reserved.
 */

#include "defs.h"

/*
 *	Possible hyperlinks. Should probably be made
 *	configurable but for now this will do.
 */
static struct 
{
	TCHAR	*pszURL;
	int	 nLength;
} 
Hyper[] = 
{ 
	{ "C:\\Documents and Settings",	25 }, 
	{ "C:\\Program Files (x86)",	22 },
	{ "C:\\Program Files",	16 },
	{ "C:\\Dokumente und Einstellungen",	30 },
	{ "c:\\",	3 },
	{ "d:\\",	3 },
	{ "\\\\",	2 },
	{ "%WINDIR%",	8 },
	{ "%ProgramFilesDir%",	17 },
	{ "%temp%",	6 },

	{ "www.",	4 },
	{ "http:",	5 },
	{ "file://",	7 },
	{ "mailto:",	7 },
	{ "ftp:",	4 },
	{ "https:",	6 },
	{ "gopher:",	7 },
	{ "nntp:",	5 },
	{ "telnet:",	7 },
	{ "news:",	5 },
	{ NULL,		0 }
};

/*
 *	Check if the given position is over
 *	a hyperlink. Also sets up the start
 *	and end points if a hyperlink is
 *	found.
 */
static BOOL CheckForHyperlink( LPCLASSDATA lpcd, LPPOINT lpPos, LPPOINT lpStart, LPPOINT lpEnd, BOOL bQuoted )
{
	LPLINE	lpLine = ( LPLINE )ArrayGetAt( lpcd->lpLines, lpPos->y );
	int	nIndex = lpPos->x, i = 0;

	/*
	 *	Any text and, if so, any text on the
	 *	line?
	 */
	if ( AnyText( lpcd ) == FALSE || lpLine->pcText == NULL )
		return FALSE;

	/*
	 *	Are we in the text of the line?
	 */
	if ( nIndex >= lpLine->nLength )
		return FALSE;

	/*
	 *	Is the given position on a white space and
	 *	are we doing a non-quoted search?
	 */
	if ( ! bQuoted && (_istspace( lpLine->pcText[ nIndex ] ) || lpLine->pcText[ nIndex ] == _T('=')
       || lpLine->pcText[ nIndex ] == _T(';')))   // Modifications by Stephan (2005-05-28)
	{
		/*
		 *	Try a quoted search now.
		 */
		return CheckForHyperlink( lpcd, lpPos, lpStart, lpEnd, TRUE );
	}

	/*
	 *	Store line numbers.
	 */
	lpStart->y = lpEnd->y = lpPos->y;

	/*
	 *	Search for a quoted hyperlink?
	 */
	if ( ! bQuoted )
	{
		/*
		 *	Find the first white space or the 
		 *	beginning of the line.
		 */
		while ( nIndex > 0 && ! _istspace( lpLine->pcText[ nIndex ] ) && lpLine->pcText[ nIndex ] != _T('=') &&
            lpLine->pcText[ nIndex ] != _T(';'))
			nIndex--;

    // Support for certain Windows folders with white spaces
	if (nIndex >= 16 && lpLine->nLength > nIndex + 5 && !_tcsncicmp(lpLine->pcText + nIndex + 1, _T("(x86)"), 5))
		nIndex -= 16;
    else if (nIndex >= 10 && lpLine->nLength > nIndex + 5 && !_tcsncicmp(lpLine->pcText+nIndex+1, _T("Files"), 5))
      nIndex -= 10;
    else if (nIndex >= 16 && lpLine->nLength > nIndex + 8 && !_tcsncicmp(lpLine->pcText+nIndex+1, _T("Settings"), 8))
      nIndex -= 16;
    else if (nIndex >= 16 && lpLine->nLength > nIndex + 13 && !_tcsncicmp(lpLine->pcText+nIndex+1, _T("Einstellungen"), 13))
      nIndex -= 16;
	}
	else
	{
		/*
		 *	Find the first double quote or the 
		 *	beginning of the line.
		 */
		while ( nIndex > 0 && lpLine->pcText[ nIndex ] != _T( '"' ) &&
            lpLine->pcText[ nIndex ] != _T( '\'' ))   // Modifications by Stephan (2005-05-30)
			nIndex--;
	}

	/*
	 *	If were looking for a quoted hyperlink and we
	 *	are not on a double quote we stop here.
	 */
	if ( bQuoted && lpLine->pcText[ nIndex ] != _T( '"' ) && lpLine->pcText[ nIndex ] != _T( '\'' ))   // Modifications by Stephan (2005-05-30)
		return FALSE;
	else if ( _istspace( lpLine->pcText[ nIndex ] ) || lpLine->pcText[ nIndex ] == _T('=') ||
            lpLine->pcText[ nIndex ] == _T(';'))   // Modifications by Stephan (2005-05-28)
		nIndex++;

	/*
	 *	Store the start column.
	 */
	lpStart->x = nIndex;

	/*
	 *	Double or single quoted?
	 */
	if ( lpLine->pcText[ nIndex ] == _T( '"' ) || lpLine->pcText[ nIndex ] == _T( '\'' ))   // Modifications by Stephan (2005-05-30)
		lpStart->x++;

	/*
	 *	Is it a hyperlink?
	 */
	for (i = 0; Hyper[ i ].pszURL; i++ )
	{
		/*
		 *	Check if this hyperlink fits on the line
		 *	from this position.
		 */
		if ( lpLine->nLength - lpStart->x >= Hyper[ i ].nLength )
		{
			/*
			 *	Is it this hyperlink?
			 */
			if ( ! _tcsnicmp( &lpLine->pcText[ lpStart->x ], Hyper[ i ].pszURL, Hyper[ i ].nLength )) 
			{
        // Modified by Stephan (2005-06-12)
        // We only can specify the length of the hyperlink, if we know its identifier.
        // Only this makes it possible to have unquoted hyperlinks with spaces, e.g. c:\program files
	      /*
	      *	Double or single quoted?
	      */
	      if ( lpLine->pcText[ nIndex ] == _T( '"' ) || lpLine->pcText[ nIndex ] == _T( '\'' ))   // Modifications by Stephan (2005-05-30)
	      {
		      /*
		      *	Find the next double quote or the end
		      *	of the line.
		      */
		      nIndex++;
		      while ( nIndex < lpLine->nLength && lpLine->pcText[ nIndex ] != _T( '"' ) &&
                  lpLine->pcText[ nIndex ] != _T( '\'' ))   // Modifications by Stephan (2005-05-30)
			      nIndex++;
	      }
	      else
	      {
          nIndex += Hyper[ i ].nLength;
		      /*
		      *	Find the next white space or the end
		      *	of the line.
		      */
		      while ( nIndex < lpLine->nLength && ! _istspace( lpLine->pcText[ nIndex ] ) &&
                  lpLine->pcText[ nIndex ] != _T('(') && lpLine->pcText[ nIndex ] != _T(',') &&
                  lpLine->pcText[ nIndex ] != _T(';') && lpLine->pcText[ nIndex ] != _T(')') &&
                  lpLine->pcText[ nIndex ] != _T('\''))   // Modified by Stephan (2005-05-28)
			      nIndex++;
	      }

       /*
	      *	Store the end column.
	      */
	      lpEnd->x = nIndex - 1;

				/*
				 *	When we are on a hyperlink we show a hand cursors.
				 */
				SetCursor( lpcd->hHand );
				return TRUE;
			}
		}
	}

	/*
	 *	If we did not find a hyperlink and we are
	 *	not doing a quoted search we start one now.
	 */
	if ( ! bQuoted ) 
		return CheckForHyperlink( lpcd, lpPos, lpStart, lpEnd, TRUE );
	return FALSE;
}

/*
 *	Check is the mouse position is located
 *	on a hyperlink.
 */
BOOL MouseOnHyperLink( LPCLASSDATA lpcd )
{
	POINT		ptMousePos;

	/*
	 *	Are we parsing hyperlinks?
	 */
	if ( Parser->bParseHyperLinks )
	{
		/*
		*	Get mouse position and convert
		*	to client coordinates.
		*/
		GetCursorPos( &ptMousePos );
		ScreenToClient( lpcd->hWnd, &ptMousePos );

		/*
		*	Skip selection margin.
		*/
		ptMousePos.x -= ( GetMarginWidth( lpcd ) + GetLineMarginWidth( lpcd ));
		return PointOnHyperlink( lpcd, ptMousePos.x, ptMousePos.y );
	}
	return FALSE;
}

/*
 *	Check is the given position is located
 *	on a hyperlink.
 */
BOOL PointOnHyperlink( LPCLASSDATA lpcd, int x, int y )
{
	POINT ptPos, ptDummy1, ptDummy2;

	/*
	 *	Are we parsing hyperlinks?
	 */
	if ( Parser->bParseHyperLinks )
	{
		/*
		 *	Translate the point to the
		 *	text buffer position.
		 */
		if ( MouseToCaret( lpcd, x, y, &ptPos ))
			/*
			 *	Check if this position is located
			 *	on a hyperlink.
			 */
			return CheckForHyperlink( lpcd, &ptPos, &ptDummy1, &ptDummy2, FALSE );
	}
	return FALSE;
}

/*
 *	Obtain the hyperlink located at the
 *	mouse position. If there is no hyperlink
 *	at the mouse position return NULL.
 */
TCHAR *GetHyperlink( LPCLASSDATA lpcd )
{
	POINT ptStart, ptEnd, ptMousePos;

	/*
	 *	Are we parsing hyperlinks?
	 */
	if ( Parser->bParseHyperLinks )
	{
		/*
		 *	Get mouse position and convert
		 *	to client coordinates.
		 */
		GetCursorPos( &ptMousePos );
		ScreenToClient( lpcd->hWnd, &ptMousePos );

		/*
		 *	Skip selection margin.
		 */
		ptMousePos.x -= ( GetMarginWidth( lpcd ) + GetLineMarginWidth( lpcd ));

		/*
		 *	Convert the coordinates to the character
		 *	position.
		 */
		if ( MouseToCaret( lpcd, ptMousePos.x, ptMousePos.y, &ptMousePos ))
		{
			/*
			 *	Check if the character is located inside
			 *	a hyperlink.
			 */
			if ( CheckForHyperlink( lpcd, &ptMousePos, &ptStart, &ptEnd, FALSE ))
			{
				/*
				 *	Allocate memory to store the
				 *	hyperlink text.
				 */
				TCHAR *pszUrl = AllocPooled( lpcd->pMemPool, REAL_SIZE( ptEnd.x - ptStart.x + 2 ));
				if ( pszUrl )
				{
					/*
					 *	Copy the hyperlink text into the
				 	 *	allocated buffer.
					 */
					LPLINE lpLine = ( LPLINE )ArrayGetAt( lpcd->lpLines, ptStart.y );
					memcpy( pszUrl, &lpLine->pcText[ ptStart.x ], ptEnd.x - ptStart.x + 1 );

					/*
					 *	0-terminate to be on the safe side.
					 */
					pszUrl[ ptEnd.x - ptStart.x + 1 ] = 0;
					return pszUrl;
				}
			}
		}
	}
	/*
	 *	No hyperlink or memory failure...
	 */
	return NULL;
}

/*
 *	See if the mouse is located on a hyperlink and,
 *	if so, run it.
 */
BOOL RunHyperlink( LPCLASSDATA lpcd )
{
	/*
	 *	Get the hyperlink at the current mouse
	 *	position.
	 */
	TCHAR *pszLink = GetHyperlink( lpcd );
 
  // Modified by Stephan (2005-05-28)
	if ( pszLink )
	{
    // If it is not a URL
    if (!_tcsnicmp(pszLink, _T("c:\\"), 3) || !_tcsnicmp(pszLink, _T("d:\\"), 3) || 
        !_tcsnicmp(pszLink, _T("\\\\"), 2) || !_tcsnicmp(pszLink, _T("%"), 1))
    {
      TCHAR szAliasFolder[MAX_PATH], szPath[MAX_PATH];
      TCHAR szModuleFileName[MAX_PATH];
      TCHAR szExecParams[MAX_PATH];
      LPTSTR pszLinkNoAlias = pszLink;

      // Resolve aliases
      szAliasFolder[0] = 0;
      if (!_tcsnicmp(pszLink, _T("%WINDIR%"), 8))
      {
        GetWindowsDirectory(szAliasFolder, MAX_PATH);
        pszLinkNoAlias += 8;
      }
	  else if (!_tcsnicmp(pszLink, _T("C:\\Program Files (x86)\\Common"), 29))
	  {
		  _tcscpy(szAliasFolder, _T("c:\\Program Files (x86)\\Common Files"));
		  pszLinkNoAlias += 29;
	  }
	  else if (!_tcsnicmp(pszLink, _T("C:\\Program Files\\Common"), 23))
	  {
		  _tcscpy(szAliasFolder, _T("c:\\Program Files\\Common Files"));
		  pszLinkNoAlias += 23;
	  }
      else if (!_tcsnicmp(pszLink, _T("%ProgramFilesDir%"), 17))
      {
        if (GetSystemDefaultLangID() & 0x07)   // German
          _tcscpy(szAliasFolder, _T("c:\\Programme"));
        else
          _tcscpy(szAliasFolder, _T("c:\\Program Files"));
        pszLinkNoAlias += 17;
      }
      else if (!_tcsnicmp(pszLink, _T("%temp%"), 6))
      {
        GetTempPath(MAX_PATH, szAliasFolder);
        pszLinkNoAlias += 6;
      }

      // Remove :line at the end
      if (_tcschr(pszLinkNoAlias, _T(':')) != _tcsrchr(pszLinkNoAlias, _T(':')))
      {
        LPTSTR lpszSemicolon = _tcsrchr(pszLinkNoAlias, _T(':'));
        if (lpszSemicolon)
          lpszSemicolon[0] = _T('\0');
      }
      

      _stprintf(szPath, _T("%s%s"), szAliasFolder, pszLinkNoAlias);

      GetModuleFileName(NULL, szModuleFileName, MAX_PATH);
      _stprintf(szExecParams, _T("-hyperlink \"%s\""), szPath);
      
	    if (( int )ShellExecute( GetDesktopWindow(), NULL, szModuleFileName, szExecParams, 
			    NULL, SW_SHOWNORMAL ) <= 32 )
			{
		    OkIOError( lpcd, lpcd->hWnd, _T( "%s" ), szPath );
			}
    }
    // Execute URL
    else
    {
		  if (( int )ShellExecute( GetDesktopWindow(), 
				  NULL, 
				  pszLink, 
				  NULL, 
				  NULL, 
				  SW_SHOWNORMAL ) <= 32 )
			  OkIOError( lpcd, lpcd->hWnd, _T( "%s" ), pszLink );
    }

		/*
		 *	Release the url buffer.
		 */
		FreePooled( lpcd->pMemPool, pszLink );
		return TRUE;
	}
	/*
	 *	No hyperlink or memory failure...
	 */
	return FALSE;
}