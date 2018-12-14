// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: auxlab
// Main Application. Based on Windows API  
// 
// 
// Version: 1.495
// Date: 12/13/2018
// 
#ifndef TABCONTROL
#define TABCONTROL

class CTabControl 
{
public:
	CWndDlg *mDad;
	HWND hTab;
	HWND hCur;
	HWND hVisiblePage;
	int tabPageCount;
	BOOL blStretchTabs;
	vector<string> titleStr;
	vector<HWND> page;

	BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam);
	void OnSize(UINT state, int cx, int cy);
	void OnCommand(INT id, HWND hwndCtl, UINT codeNotify);

	void initTabcontrol(HWND);
	int AddPage(HWND hNewpage, string title);
	void DeletePage(int index);
	int GetCurrentPageID();
	int GetCurrentPageID(const char *fname);
	void FirstTabstop_SetFocus(HWND h);
	void TabCtrl_OnKeyDown(LPARAM lParam);
	BOOL TabCtrl_OnSelChanged(int id=-1);
	BOOL OnNotify(LPNMHDR pnm);
	int SetCurrentTab(const char* fname);

 	CTabControl();
	virtual ~CTabControl() {};
private:
	TCITEM tie;
};

#define TabCtrl_SelectTab(hTab,iSel) { \
	TabCtrl_SetCurSel(hTab,iSel); \
	NMHDR nmh = { hTab, GetDlgCtrlID(hTab), TCN_SELCHANGE }; \
	SendMessage(nmh.hwndFrom,WM_NOTIFY,(WPARAM)nmh.idFrom,(LPARAM)&nmh); }

#endif //DEBUG_DLG