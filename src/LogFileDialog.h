//-----------------------------------------------------------------------------
//
//    LogFileDialog.h
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------

#pragma once
#include "..\3p\classlib\windows\common dialogs\filedialog.h"

class CLogFileDialog : public ClsFileDialog
{
public:
  CLogFileDialog(void);
  ~CLogFileDialog(void);
};
