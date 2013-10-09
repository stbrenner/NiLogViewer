//-----------------------------------------------------------------------------
//
//    LogFolderDlg.h
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------

#pragma once

class CLogFolderDlg : public ClsDialog
{
	_NO_COPY( CLogFolderDlg );
public:
  CLogFolderDlg(void);
  ~CLogFolderDlg(void);

  void GetLogFolder(LPTSTR lpsz, int count) {_tcsncpy(lpsz, m_szLogFolder, count);}
  void SetLogFolder(LPCTSTR lpsz) {_tcsncpy(m_szLogFolder, lpsz, MAX_PATH);}

protected:
	virtual LRESULT OnCommand( UINT nNotifyCode, UINT nCtrlID, HWND hWndCtrl );
	virtual LRESULT OnInitDialog( PROPSHEETPAGE *psp );
  virtual BOOL OnOK();
	void FillComputerCombo();

private:
  ClsRadioButton    m_ComputerRadio;
	ClsComboBox		    m_ComputerCombo;

	ClsRadioButton    m_PathRadio;
	ClsFileDirBrowser	m_Browser;

  TCHAR m_szLogFolder[MAX_PATH];
};
