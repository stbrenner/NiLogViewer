//
//	bcmenu.h
//
//	By Stephan Brenner.
//
//	NI Log Viewer.
//	This code is public domain, use and abuse as you desire.
//

class BCMenu : public ClsXPMenu
{
	_NO_COPY( BCMenu )
public:
	// Construction/destruction...
	BCMenu();
	virtual ~BCMenu();

	// Implementation.
	BOOL Init( LPCTSTR pszMenuName, HIMAGELIST hImages, BOOL bChildren, BOOL bContext );
};