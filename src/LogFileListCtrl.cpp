#include "stdafx.h"

CLogFileListCtrl::CLogFileListCtrl(void) :
m_pFrame(0),  m_lParamCount(0), m_iCurrKey(0),
m_bSortToBeDone(0), m_iSortColumn(0), m_bSortUp(TRUE), 
m_hListThread(0), m_bStopThread(0)
{
	// Sets the locale to the default, which is the user-default ANSI code page
	// obtained from the operating system
	_tsetlocale( LC_ALL, _T("") );

	m_szLogFolder[0] = 0;
	m_szFilter[0] = 0;

	// Read sort settings from registry
	LONG lRet;
	HKEY hLogVwrReg;
	lRet = RegOpenKeyEx(HKEY_CURRENT_USER, REG_PATH, NULL, KEY_READ, &hLogVwrReg);

	if (lRet == ERROR_SUCCESS)
	{
		DWORD dwType;
		DWORD dwSize = sizeof(m_iSortColumn);
		lRet = RegQueryValueEx(hLogVwrReg, _T("SortColumn"), NULL, &dwType,
			(LPBYTE)&m_iSortColumn, &dwSize);
		dwSize = sizeof(m_bSortUp);
		lRet = RegQueryValueEx(hLogVwrReg, _T("SortUp"), NULL, &dwType,
			(LPBYTE)&m_bSortUp, &dwSize);
		RegCloseKey(hLogVwrReg);
	}

	// Create Events
	m_hFilterChange = CreateEvent(NULL, FALSE, FALSE, _T("FilterChange"));
	m_hLogFolderChange = CreateEvent(NULL, FALSE, FALSE, _T("LogFolderChange"));
	m_hStopThread = CreateEvent(NULL, FALSE, FALSE, _T("StopThread"));
	m_hTriggerSort = CreateEvent(NULL, FALSE, FALSE, _T("TriggerSort"));
	m_hReadAllLogHeaders = CreateEvent(NULL, FALSE, FALSE, _T("ReadAllLogHeaders"));
}

CLogFileListCtrl::~CLogFileListCtrl(void)
{
	// Write sort settings to registry
	HKEY hLogVwrReg;
	DWORD dwDisposition;
	LONG lRet;

	lRet = RegCreateKeyEx(HKEY_CURRENT_USER, REG_PATH, NULL, NULL, 
		REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hLogVwrReg, &dwDisposition);

	if (lRet == ERROR_SUCCESS)
	{
		lRet = RegSetValueEx(hLogVwrReg, _T("SortColumn"), NULL, REG_DWORD, (const BYTE*)&m_iSortColumn, sizeof(m_iSortColumn));
		lRet = RegSetValueEx(hLogVwrReg, _T("SortUp"), NULL, REG_DWORD, (const BYTE*)&m_bSortUp, sizeof(m_bSortUp));
	}

	StopListThread();
}

DWORD WINAPI CLogFileListCtrl::ListLogFilesProc(LPVOID data)
{
	CLogFileListCtrl* LogCtrl = (CLogFileListCtrl*)data;

	LogCtrl->ListLogFiles(TRUE);
	__time64_t lLastRefresh, lNow;
	DWORD dwTimeout = INFINITE;
	_time64(&lLastRefresh);

	TCHAR szLogFolder[MAX_PATH];
	LogCtrl->GetLogFolder(szLogFolder, MAX_PATH);
	HANDLE hFileNotification = FindFirstChangeNotification(szLogFolder, 
		FALSE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE);

	for (;;)
	{
		HANDLE handles[5];
		handles[0] = LogCtrl->m_hStopThread;
		handles[1] = LogCtrl->m_hLogFolderChange;
		handles[2] = LogCtrl->m_hFilterChange;
		handles[3] = LogCtrl->m_hTriggerSort;
		handles[4] = LogCtrl->m_hReadAllLogHeaders;
		handles[5] = hFileNotification;

		DWORD dwSignal;
		if (hFileNotification == INVALID_HANDLE_VALUE)   // Maybe the log folder does not exist
			dwSignal = WaitForMultipleObjects(5, handles, FALSE, 5000);
		else
			dwSignal = WaitForMultipleObjects(6, handles, FALSE, dwTimeout);

		// Stop thread signaled
		if (dwSignal == WAIT_OBJECT_0)
			break;

		// Folder change signaled
		if (dwSignal == WAIT_OBJECT_0 + 1)
		{
			LogCtrl->ListLogFiles(TRUE);
			_time64(&lLastRefresh);
			dwTimeout = INFINITE;

			FindCloseChangeNotification(hFileNotification);

			LogCtrl->GetLogFolder(szLogFolder, MAX_PATH);
			hFileNotification = FindFirstChangeNotification(szLogFolder, 
				FALSE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE);
		}

		// Filter change signaled
		if (dwSignal == WAIT_OBJECT_0 + 2)
		{
			LogCtrl->ListLogFiles(TRUE);
			_time64(&lLastRefresh);
			dwTimeout = INFINITE;
		}

		// Timeout signaled
		if (dwSignal == WAIT_TIMEOUT)
		{
			// If the last folder open failed, then retry after 5 seconds
			if (hFileNotification == INVALID_HANDLE_VALUE)
			{
				LogCtrl->GetLogFolder(szLogFolder, MAX_PATH);
				hFileNotification = FindFirstChangeNotification(szLogFolder, 
					FALSE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE);
				if (hFileNotification != INVALID_HANDLE_VALUE)
					LogCtrl->ListLogFiles(TRUE);
			}
			// React on file modifications only once in 2 seconds
			else
			{
				LogCtrl->ListLogFiles(FALSE);
				_time64(&lLastRefresh);
				dwTimeout = INFINITE;
			}
		}

		// Sort log files signaled
		if (dwSignal == WAIT_OBJECT_0 + 3)
			LogCtrl->SortLogFiles();

		// Signaled, that all log headers have to be read
		if (dwSignal == WAIT_OBJECT_0 + 4)
			LogCtrl->ReadAllLogHeaders();

		// Timeout or file modification signaled
		if (dwSignal == WAIT_OBJECT_0 + 5)
		{
			_time64(&lNow);

			if (lNow <= lLastRefresh + 2)   // Maximum refresh rate not more than 2 seconds!
				dwTimeout = (DWORD)((3 - (lNow - lLastRefresh)) * 1000);
			else
			{
				LogCtrl->ListLogFiles(FALSE);
				_time64(&lLastRefresh);
				dwTimeout = INFINITE;
			}

			if (!FindNextChangeNotification(hFileNotification))
			{
				FindCloseChangeNotification(hFileNotification);
				hFileNotification = INVALID_HANDLE_VALUE;
			}
		}
	}

	FindCloseChangeNotification(hFileNotification);
	ExitThread(0);
}

BOOL CLogFileListCtrl::Create(ClsWindow *parent,
							  const ClsRect& rBounds,
							  UINT nID, 
							  UINT nStyle)
{
	BOOL bRet = ClsListView::Create(parent, (ClsRect&)rBounds, nID, nStyle | LVS_REPORT);
	Attach(GetSafeHWND());
	return bRet;
}

BOOL CLogFileListCtrl::Attach(HWND hWnd, BOOL bDestroy)
{
	BOOL bRet = ClsListView::Attach(hWnd, bDestroy);

	ModifyExStyle(0, WS_EX_CLIENTEDGE);
	SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

	// Fill columns
	InsertColumn(0, _T("File"), LVCFMT_LEFT, 180, 0);
	InsertColumn(1, _T("Modified"), LVCFMT_LEFT, 120, 0);
	InsertColumn(2, _T("Size"), LVCFMT_RIGHT, 60, 0);
	InsertColumn(3, _T("Significance"), LVCFMT_RIGHT, 40, 0);
	InsertColumn(4, _T("Errors"), LVCFMT_RIGHT, 40, 0);
	InsertColumn(5, _T("Warnings"), LVCFMT_RIGHT, 40, 0);

	RefreshSortHeader();

	// Start list thread
	if (!m_hListThread)
	{
		DWORD ListThreadId;
		m_hListThread = CreateThread(0, 0, ListLogFilesProc, (void*)this, 0, &ListThreadId);
	}

	return bRet;
}

void CLogFileListCtrl::StopListThread()
{
	if (!m_hListThread)
		return;

	SetEvent(m_hStopThread);
	m_bStopThread = TRUE;

	if (WaitForSingleObject(m_hListThread, 2000) != WAIT_OBJECT_0)
		TerminateThread(m_hListThread, 0); 

	m_hListThread = 0;
}

void CLogFileListCtrl::ReadLogHeader(LPCTSTR lpszFile, DWORD& dwSig, DWORD& dwErr, DWORD& dwWarn)
{
	dwSig = dwErr = dwWarn = 0;

	TCHAR szPath[MAX_PATH];
	_stprintf(szPath, _T("%s\\%s"), m_szLogFolder, lpszFile);

	FILE* stream;
	stream  = _tfopen(szPath, "r");

	if (!stream)
		return;

	// Read file
	TCHAR szData[200];
	_fgetts(szData, 200, stream);
	_fgetts(szData, 200, stream);   // Data is in second line
	fclose(stream);

	TCHAR szTemp[21];
	_stscanf(szData, _T("%20s : %20s [Significance: %i] --- [Errors: %i] --- [Warnings: %i]"), 
		szTemp, szTemp, &dwSig, &dwErr, &dwWarn);
}

void CLogFileListCtrl::ListLogFiles(BOOL bBatchImport)
{
	struct _tfinddata_t file;
	intptr_t            hFile;
	TCHAR               szPath[MAX_PATH];
	static TCHAR        szLogFolderOld[MAX_PATH];
	static TCHAR        szFilterOld[MAX_PATH];

	// Get log folder and build search path
	_stprintf(szPath, _T("%s\\*%s*.log*"), m_szLogFolder, m_szFilter);

	// Check if the redraw is better to be done in background
	if (bBatchImport)
	{
		m_pFrame->SendMessage(WM_ANIMATE_DOWNLOAD, 0, TRUE);
		SetRedraw(false);
	}

	_tcscpy(szLogFolderOld, m_szLogFolder);
	_tcscpy(szFilterOld, m_szFilter);

	// Keep book of all files found. This helps to see what files were removed
	int nAllFilesSize = 10000;
	LPTSTR lpszAllFiles = (LPTSTR)malloc(sizeof(TCHAR)*nAllFilesSize);
	_tcscpy(lpszAllFiles, _T(";"));

	// Find first file in the given path
	hFile = _tfindfirst(szPath, &file);

	// Find the rest of the files
	if (hFile != -1L)
	{
		do
		{
			AddLogFile(file.name, file.size, file.time_write, bBatchImport);

			// Add file found to book keeping list
			if (nAllFilesSize - _tcslen(lpszAllFiles) < _tcslen(file.name) + 10)
			{
				nAllFilesSize += 10000;
				LPTSTR lpszOldMem = lpszAllFiles;
				lpszAllFiles = (LPTSTR)malloc(sizeof(TCHAR)*nAllFilesSize);
				_tcscpy(lpszAllFiles, lpszOldMem);
				free(lpszOldMem);
			}
			_tcscat(lpszAllFiles, file.name);
			_tcscat(lpszAllFiles, _T(";"));
		} while (_tfindnext(hFile, &file) == 0 && !m_bStopThread);

		_findclose(hFile);
		SetInfoText(NULL);
		RemoveLogFiles(lpszAllFiles);
		SetEvent(m_hTriggerSort);
	}

	free(lpszAllFiles);

	if (bBatchImport)
		SetRedraw(true);

	// Handle errors
	if (hFile == -1L)
	{
		switch (errno)
		{
		case 2:
			SetInfoText(_T("No log files found"));
			break;
		case 22:
			SetInfoText(_T("Wrong log folder or filter"));
			break;
		default:
			SetInfoText(_tcserror(errno));
		}

		m_pFrame->SendMessage(WM_ANIMATE_DOWNLOAD, 0, FALSE);
		return;
	}

	// If batch import, now it's time to read the log header
	if (bBatchImport)
		SetEvent(m_hReadAllLogHeaders);
}

void CLogFileListCtrl::SetInfoText(LPCTSTR lpsz)
{
	if (lpsz)
	{
		DeleteAllItems();
		EnableWindow(0);

		LV_ITEM lvitem;
		lvitem.iItem = 0;
		lvitem.mask = LVIF_TEXT;
		lvitem.iSubItem = 0;
		lvitem.pszText = (LPSTR)lpsz;
		InsertItem(lvitem);
	}
	else
		EnableWindow(1);
}

void CLogFileListCtrl::AddLogFile(LPCTSTR lpszFile, UINT dwSize, time_t iModified, BOOL bBatchImport)
{
	BOOL bFileChanged = FALSE;

	// Search list, if item already exists
	LVFINDINFO info;
	info.flags = LVFI_STRING;
	info.psz = (LPTSTR)lpszFile;

	int iItem = -1;
	int iItemFind = FindItem(-1, info);
	if (iItemFind == -1)
		iItem = GetItemCount();
	else
		iItem = iItemFind;

	// Insert item if neccessary
	LV_ITEM lvitem;
	lvitem.iItem = iItem;
	if (iItemFind == -1)
	{
		lvitem.mask = LVIF_TEXT | LVIF_PARAM;
		lvitem.iSubItem = 0;
		lvitem.pszText = (LPTSTR)lpszFile;
		lvitem.lParam = m_lParamCount++;
		if (iItem == 0)
		{
			lvitem.mask |= LVIF_STATE;
			lvitem.state = LVIS_SELECTED | LVIS_FOCUSED;
			lvitem.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
		}
		InsertItem(lvitem);
		bFileChanged = TRUE;
		m_bSortToBeDone = TRUE;
	}

	// Set sub-columns
	TCHAR szUpdateTest[MAX_PATH];
	lvitem.mask = LVIF_TEXT;

	TCHAR szTime[50];
	_tcsftime(szTime, 50, _T("%Y-%m-%d %H:%M:%S "), localtime(&iModified));
	GetItemText(iItem, 1, szUpdateTest, MAX_PATH);
	if (_tcscmp(szUpdateTest, szTime))
	{
		lvitem.iSubItem = 1;
		lvitem.pszText = szTime;
		SetItem(lvitem);
		bFileChanged = TRUE;
		if (m_iSortColumn == 1)
			m_bSortToBeDone = TRUE;
	}

	TCHAR szSize[50];
	_stprintf(szSize, _T("%i KB"), dwSize/1024+1);
	GetItemText(iItem, 2, szUpdateTest, MAX_PATH);
	if (_tcscmp(szUpdateTest, szSize))
	{
		lvitem.iSubItem = 2;
		lvitem.pszText = szSize;
		SetItem(lvitem);
		bFileChanged = TRUE;
		if (m_iSortColumn == 2)
			m_bSortToBeDone = TRUE;
	}

	// If it is a batch import or if the file didn't change, then we stop here
	if (bBatchImport || !bFileChanged)
		return;

	// Read and add log header
	DWORD dwSig, dwErr, dwWarn;
	ReadLogHeader(lpszFile, dwSig, dwErr, dwWarn);
	AddLogHeader(iItem, dwSig, dwErr, dwWarn);
}

void CLogFileListCtrl::AddLogHeader(int iItem, DWORD& dwSig, DWORD& dwErr, DWORD& dwWarn)
{
	TCHAR szUpdateTest[MAX_PATH];
	LV_ITEM lvitem;
	lvitem.iItem = iItem;
	lvitem.mask = LVIF_TEXT;

	TCHAR szSig[20];
	if (dwSig > 0)
		_ltot(dwSig, szSig, 10);
	else
		szSig[0] = _T('\0');
	GetItemText(iItem, 3, szUpdateTest, MAX_PATH);
	if (_tcscmp(szUpdateTest, szSig))
	{
		lvitem.iSubItem = 3;
		lvitem.pszText = szSig;
		SetItem(lvitem);
		if (m_iSortColumn == 3)
			m_bSortToBeDone = TRUE;
	}

	TCHAR szErr[20];
	if (dwErr > 0)
		_ltot(dwErr, szErr, 10);
	else
		szErr[0] = _T('\0');
	GetItemText(iItem, 4, szUpdateTest, MAX_PATH);
	if (_tcscmp(szUpdateTest, szErr))
	{
		lvitem.iSubItem = 4;
		lvitem.pszText = szErr;
		SetItem(lvitem);
		if (m_iSortColumn == 4)
			m_bSortToBeDone = TRUE;
	}

	TCHAR szWarn[20];
	if (dwWarn > 0)
		_ltot(dwWarn, szWarn, 10);
	else
		szWarn[0] = _T('\0');
	GetItemText(iItem, 5, szUpdateTest, MAX_PATH);
	if (_tcscmp(szUpdateTest, szWarn))
	{
		lvitem.iSubItem = 5;
		lvitem.pszText = szWarn;
		SetItem(lvitem);
		if (m_iSortColumn == 5)
			m_bSortToBeDone = TRUE;
	}
}

void CLogFileListCtrl::RemoveLogFiles(LPCTSTR lpszFiles)
{
	int nCount = GetItemCount();
	TCHAR szItem[MAX_PATH];
	TCHAR szSearch[MAX_PATH+2];

	for(int nItem = 0; nItem < nCount && !m_bStopThread; nItem++)
	{
		GetItemText(nItem, 0, szItem, MAX_PATH);
		_stprintf(szSearch, _T(";%s;"), szItem);
		if(_tcsstr(lpszFiles, szSearch) == NULL)
		{
			DeleteItem(nItem);
			nItem--;
			nCount--;
		}
	}
}

void CLogFileListCtrl::ReadAllLogHeaders()
{
	int nCount = GetItemCount();
	TCHAR szItem[MAX_PATH];
	DWORD dwSig, dwErr, dwWarn;

	for(int nItem = 0; nItem < nCount && !m_bStopThread; nItem++)
	{
		GetItemText(nItem, 0, szItem, MAX_PATH);
		ReadLogHeader(szItem, dwSig, dwErr, dwWarn);
		AddLogHeader(nItem, dwSig, dwErr, dwWarn);
	}

	SetEvent(m_hTriggerSort);
	m_pFrame->SendMessage(WM_ANIMATE_DOWNLOAD, 0, FALSE);
}

LRESULT CLogFileListCtrl::OnItemActivate( LPNMITEMACTIVATE pItemActive, BOOL& bNotifyParent )
{
	if (m_pFrame)
		m_pFrame->OpenCurrentLog();

	return 0;
}

int CALLBACK CLogFileListCtrl::MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CLogFileListCtrl* pListCtrl = (CLogFileListCtrl*) lParamSort;

	LVFINDINFO info;
	info.flags = LVFI_PARAM;

	info.lParam = lParam1;
	int iItem1 = pListCtrl->FindItem(-1, info);
	if (iItem1 == -1)
		return 0;

	info.lParam = lParam2;
	int iItem2 = pListCtrl->FindItem(-1, info);
	if (iItem2 == -1)
		return 0;

	TCHAR szItem1[MAX_PATH], szItem2[MAX_PATH];
	pListCtrl->GetItemText(iItem1, pListCtrl->m_iSortColumn, szItem1, MAX_PATH);
	pListCtrl->GetItemText(iItem2, pListCtrl->m_iSortColumn, szItem2, MAX_PATH);

	if (pListCtrl->m_iSortColumn >= 2)
	{
		long lItem1 = atol(szItem1);
		long lItem2 = atol(szItem2);

		if (pListCtrl->m_bSortUp)
			return lItem2 < lItem1;
		return lItem1 < lItem2;
	}

	if (pListCtrl->m_bSortUp)
		return _tcsicmp(szItem1, szItem2);
	return _tcsicmp(szItem2, szItem1);
}

LRESULT CLogFileListCtrl::OnColumnClick( LPNMLISTVIEW pListView, BOOL& bNotifyParent )
{
	m_bSortUp = m_iSortColumn == pListView->iSubItem ? !m_bSortUp : TRUE;
	m_iSortColumn = pListView->iSubItem;
	RefreshSortHeader();
	m_bSortToBeDone = TRUE;
	SetEvent(m_hTriggerSort);

	return 0;
}

LRESULT CLogFileListCtrl::OnClose()
{
	StopListThread(); 
	return 0;
}

void CLogFileListCtrl::RefreshSortHeader()
{
	// Paint arrow in sort column
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT;
	GetColumn(m_iSortColumn, lvc);
	lvc.fmt |= HDF_SORTUP;
	lvc.fmt |= HDF_SORTDOWN;
	lvc.fmt -= m_bSortUp ? HDF_SORTDOWN : HDF_SORTUP;
	SetColumn(m_iSortColumn, lvc);

	// Set back other columns
	for (int n = 0; n < 6; n++)
	{
		if (n != m_iSortColumn)
		{
			GetColumn(n, lvc);
			lvc.fmt -= lvc.fmt & HDF_SORTDOWN;
			lvc.fmt -= lvc.fmt & HDF_SORTUP;
			SetColumn(n, lvc);
		}
	}
}

void CLogFileListCtrl::SortLogFiles()
{
	if (!m_bSortToBeDone)
		return;

	m_bSortToBeDone = FALSE;
	SortItems(MyCompareProc, (LPARAM)this);
}

// Window procedure...
LRESULT CLogFileListCtrl::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// What's up?
	switch ( uMsg )
	{
	case WM_KEYDOWN:
		m_iCurrKey = wParam;
		break;
	case WM_KEYUP:
		m_iCurrKey = 0;
		break;
	case WM_GETDLGCODE:
		if (wParam == 13)   // Return key
			m_pFrame->OpenCurrentLog();
		break;
	}

	// On to the base class.
	return ClsListView::WindowProc( uMsg, wParam, lParam );
}

void CLogFileListCtrl::GetSelectedLog(LPTSTR lpsz, unsigned int count)
{
	TCHAR szFileName[MAX_PATH];
	int nSelectionMark = GetSelectionMark();
	GetItemText(nSelectionMark, 0, szFileName, MAX_PATH);

	if (_tcslen(szFileName) + _tcslen(m_szLogFolder) + 1 > count)
		return;   // Error - buffer to small

	_stprintf(lpsz, _T("%s\\%s"), m_szLogFolder, szFileName);
}
