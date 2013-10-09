#pragma once

class CLogFileListCtrl : public ClsListView
{
public:
  // Constructor/destructor
  CLogFileListCtrl(void);
  ~CLogFileListCtrl(void);

  // Creation
  BOOL Create(ClsWindow *parent, const ClsRect& rBounds, UINT nID, 
    UINT nStyle = WS_CHILD | WS_BORDER | WS_TABSTOP | WS_VISIBLE | LBS_STANDARD);
  BOOL Attach(HWND hWnd, BOOL bDestroy = 0);
  inline void SetFrameWindow( BCFrameWnd *pFrame ) { m_pFrame = pFrame; }

  // Set/get log folder
  void GetLogFolder(LPTSTR lpsz, int count) {_tcsncpy(lpsz, m_szLogFolder, count);}
  void SetLogFolder(LPCTSTR lpsz) {_tcsncpy(m_szLogFolder, lpsz, MAX_PATH); 
                                   SetEvent(m_hLogFolderChange); }

  // Set/get filter
  void GetFilter(LPTSTR lpsz, int count) {_tcsncpy(lpsz, m_szFilter, count);}
  void SetFilter(LPCTSTR lpsz) {_tcsncpy(m_szFilter, lpsz, MAX_PATH); 
                                   SetEvent(m_hFilterChange); }

  // Which log is currently selected?
  void GetSelectedLog(LPTSTR lpsz, unsigned int count);

  // Helps to surpress refresh, when key is pressed (flickering!)
  inline int GetCurrKey() { return m_iCurrKey; }

protected:
  // Helper methods
  void AddLogFile(LPCTSTR lpszFile, UINT dwSize, time_t iModified, BOOL bBatchImport = FALSE);
  void AddLogHeader(int nItem, DWORD& dwSig, DWORD& dwErr, DWORD& dwWarn);
  void RemoveLogFiles(LPCTSTR lpszFiles);
  void ReadLogHeader(LPCTSTR lpszFile, DWORD& dwSig, DWORD& dwErr, DWORD& dwWarn);
  void StopListThread();

  // Callback functions
  static DWORD WINAPI ListLogFilesProc(LPVOID data);
  static int CALLBACK MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

  // Overwritten methods
  virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual LRESULT OnItemActivate( LPNMITEMACTIVATE pItemActive, BOOL& bNotifyParent );
	virtual LRESULT OnColumnClick( LPNMLISTVIEW pListView, BOOL& bNotifyParent );
  virtual LRESULT OnClose();

  void RefreshSortHeader();
  void SetInfoText(LPCTSTR lpsz);

  // Called by list thread
  void ListLogFiles(BOOL bBatchImport = FALSE);
  void SortLogFiles();
  void ReadAllLogHeaders();

  ClsImageList m_ImageList;
  ClsImageList m_ImageListSmall;
  BCFrameWnd* m_pFrame;

  BOOL m_bSortToBeDone;
  int m_iSortColumn;
  BOOL m_bSortUp;

  long m_lParamCount;
  int m_iCurrKey;

  HANDLE m_hListThread;
  BOOL m_bStopThread;

  TCHAR m_szLogFolder[MAX_PATH];
  TCHAR m_szFilter[MAX_PATH];

  // Handles watched by list thread
  HANDLE   m_hLogFolderChange;
  HANDLE   m_hFilterChange;
  HANDLE   m_hStopThread;
  HANDLE   m_hTriggerSort;
  HANDLE   m_hReadAllLogHeaders;
};
