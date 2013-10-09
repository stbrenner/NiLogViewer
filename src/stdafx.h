//-----------------------------------------------------------------------------
//
//    stdafx.h
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------


#define REG_PATH	_T( "Software\\Stephan Brenner\\NI Log Viewer\\" )

#define IDM_MRU_BASE	( 0xDEAD )
#define IDM_HOT_BASE	( 0xDEAD + 10 )

#define WM_OPEN_LOG_FILE    WM_USER + 3003
#define WM_ANIMATE_DOWNLOAD WM_USER + 3004

#define MIN_SIDEBAR_SIZE 23

#include <vector>
#include <io.h>
#include <TIME.H>
#include <locale.h>
#include <math.h>
#include <errno.h>

#include "../3p/classlib/all.h"
#include "resource.h"
#include "bcstatus.h"
#include "bctoolbar.h"
#include "bcmenu.h"
#include "bcframe.h"
#include "bcchild.h"
#include "logfilelistctrl.h"
#include "logfolderdlg.h"
#include "sidebardlg.h"
#include "HelperFunctions.h"
#include "BuildInfo.h"
#include "Winnetwk.h"
#include "MessageCodes.h"