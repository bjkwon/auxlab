// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: auxlab
// Main Application. Based on Windows API  
// 
// 
// Version: 1.4951
// Date: 12/14/2018
// 
#include <windows.h>
#include <commctrl.h>  // includes the common control header
#include "msgCrack.h"
#include "bjcommon.h"
#include "bjcommon_win.h"
#include "consts.h"

class CHistDlg : public CWndDlg
{
public:
	HWND hList;
	LVCOLUMN LvCol;
	LVITEM LvItem;
	char logfilename[256];

	HFONT eFont;
	LONG pixelPerLine;

	CHistDlg(void);
	~CHistDlg(void);

	BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam);
	void OnCommand(int idc, HWND hwndCtl, UINT event);
	void OnTimer(UINT id);
	void OnSize(UINT state, int cx, int cy);
	void OnClose();
	void OnDestroy();
	void OnShowWindow(BOOL fShow, UINT status);
	void OnNotify(HWND hwnd, int idcc, LPARAM lParam);
	LRESULT ProcessCustomDraw (NMHDR *lParam);
	void UpdateSheets();
	void lvInit();
	void FillupHist(vector<string> in);
	void AppendHist(vector<string> input);
};