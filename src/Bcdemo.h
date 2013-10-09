#define REG_PATH	_T( "Software\\Stephan Brenner\\NI Log Viewer\\" )

#define IDM_MRU_BASE	( 0xDEAD )
#define IDM_HOT_BASE	( 0xDEAD + 10 )

#include "../libs/classlib/all.h"
#include <io.h>
#include <TIME.H>
#include <locale.h>
#include <math.h>
#include "resource.h"
#include "bcstatus.h"
#include "bctoolbar.h"
#include "bcmenu.h"
#include "bcframe.h"
#include "bcchild.h"
#include "logoverview.h"
#include "logfilelistctrl.h"