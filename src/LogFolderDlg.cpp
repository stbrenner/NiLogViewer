//-----------------------------------------------------------------------------
//
//    LogFolderDlg.cpp
//
//    Author: Stephan Brenner
//    Date:   03/20/2006
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "LogFolderConn.h"

CLogFolderDlg::CLogFolderDlg(void)
{
}

CLogFolderDlg::~CLogFolderDlg(void)
{
}

///
/// WM_INITDIALOG handler.
///
LRESULT CLogFolderDlg::OnInitDialog( PROPSHEETPAGE *psp )
{
	// Init everything around the "computer" radio button.
	m_ComputerRadio.Attach( GetDlgItemHandle( IDC_COMPUTER_RADIO ));
	m_ComputerRadio.SetCheck(BST_CHECKED);
	m_ComputerCombo.Attach( GetDlgItemHandle( IDC_COMPUTER_COMBO ));
	FillComputerCombo();

	// Init everything around the "path" redio button.
	m_PathRadio.Attach( GetDlgItemHandle( IDC_PATH_RADIO ));
	m_Browser.Attach( GetDlgItemHandle( IDC_FOLDER_COMBO ));
	m_Browser.LoadingTextColor() = RGB( 255, 0, 0 );
	m_Browser.ShowFiles() = FALSE;
	m_Browser.SetWindowText(m_szLogFolder);

	return ClsDialog::OnInitDialog( psp );
}

///
/// Fills the computer combo by reading out the last values from the registry.
///
void CLogFolderDlg::FillComputerCombo()
{
	// "localhost" is a default value.
	m_ComputerCombo.AddString(_T("localhost"));
	m_ComputerCombo.SetWindowText(_T("localhost"));

	// Now add values from registry.
	ReadComboFromRegistry(m_ComputerCombo.GetSafeHWND(), REG_PATH, _T("LastComputers"));
}

///
/// Handle WM_COMMAND messages that are sent by the UI controls.
///
LRESULT CLogFolderDlg::OnCommand( UINT nNotifyCode, UINT nCtrlID, HWND hWndCtrl )
{
	switch ( nCtrlID )
	{
	case IDC_COMPUTER_COMBO:
		if ( nNotifyCode == CBN_EDITCHANGE )
		{
			m_ComputerRadio.SetCheck(BST_CHECKED);
			m_PathRadio.SetCheck(BST_UNCHECKED);
		}
		break;

	case IDC_FOLDER_COMBO:
		if ( nNotifyCode == CBN_EDITCHANGE )
		{
			m_PathRadio.SetCheck(BST_CHECKED);
			m_ComputerRadio.SetCheck(BST_UNCHECKED);
		}
		break;

	case IDC_COMPUTER_RADIO:
		if ( nNotifyCode == BN_CLICKED )
		{
			m_PathRadio.SetCheck(BST_UNCHECKED);
		}
		break;

	case IDC_PATH_RADIO:
		if ( nNotifyCode == BN_CLICKED )
		{
			m_ComputerRadio.SetCheck(BST_UNCHECKED);
		}
		break;
	}

	return ClsDialog::OnCommand( nNotifyCode, nCtrlID, hWndCtrl );
}

///
/// When the user presses OK, try to connect to the log folder.
///
BOOL CLogFolderDlg::OnOK()
{
	DWORD dwResult = NO_ERROR;
	CLogFolderConn oLogFolderConn(GetSafeHWND());
	WriteComboToRegistry(m_ComputerCombo.GetSafeHWND(), REG_PATH, _T("LastComputers"), 1);

	// Depending on the selected radio button, start connecting.
	if (m_ComputerRadio.GetCheck() == BST_CHECKED)
	{
		TCHAR szComputer[MAX_PATH];
		m_ComputerCombo.GetWindowText(szComputer, MAX_PATH);

		if (_tcsicmp(szComputer, _T("localhost")) == 0)
		{
			dwResult = oLogFolderConn.ConnectToLocal();
		}
		else
		{
			dwResult = oLogFolderConn.ConnectToComputer(szComputer);
		}
	}
	else if (m_PathRadio.GetCheck() == BST_CHECKED)
	{
		TCHAR szFolder[MAX_PATH];
		m_Browser.GetWindowText(szFolder, MAX_PATH);

		dwResult = oLogFolderConn.ConnectToFolder(szFolder);
	}

	// Check, if an error occured.
	if (dwResult != NO_ERROR)
	{		
		if (dwResult == ERROR_NI_REG_SECTION_NOT_FOUND)
		{
			ErrorMessageBox(m_hWnd, _T("Cannot open NetInstall section in registry!"));
		}
		if (dwResult == ERROR_REMOTE_REGISTRY_NOT_STARTED)
		{
			ErrorMessageBox(m_hWnd, _T("Please start the remote registry service on the destination computer."));
		}
		else if (dwResult != ERROR_CANCELLED)
		{
			ReportError(m_hWnd, dwResult);
		}

		return FALSE;
	}

	// Everything worked, so take the new folder.
	oLogFolderConn.GetLogFolder(m_szLogFolder, MAX_PATH);
	return TRUE;
}