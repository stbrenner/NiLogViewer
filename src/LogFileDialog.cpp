//-----------------------------------------------------------------------------
//
//    LogFileDialog.cpp
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include ".\logfiledialog.h"


CLogFileDialog::CLogFileDialog(void)
{
  m_pszFilters = _T("enteo Log Files (*.log*)\0*.log*\0");
  m_nFilterIndex = 0;
}

CLogFileDialog::~CLogFileDialog(void)
{
}
