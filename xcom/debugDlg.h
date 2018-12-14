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
#ifndef DEBUG_DLG
#define DEBUG_DLG

#include <windows.h>
#include <commctrl.h>  // includes the common control header
#include "WndDlg0.h"
#include "msgCrack.h"
#include <set>

#ifndef SIGPROC
#include "sigproc.h"
#endif

using namespace std;

#define WM_DEBUG_CLOSE	WM_APP+9821

class CDebugDlg : public CWndDlg
{
public:
	static CAstSig *pAstSig;
	string udfname;
	char fullUDFpath[256];
	int lastLine;
	vector<int> breakpoint;
	HWND hList;
	LVCOLUMN LvCol; // Make Coluom struct for ListView
	LVITEM LvItem;  // ListView Item struct

	HFONT eFont, eFont2;
	int fontsize;
	LOGFONT      lf;

	CDebugDlg(void);
	~CDebugDlg(void);

	BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam);
	void OnCommand(int idc, HWND hwndCtl, UINT event);
	void OnSysCommand(UINT cmd, int x, int y);
	void OnSize(UINT state, int cx, int cy);
	void OnClose();
	void OnDestroy();
	void OnShowWindow(BOOL fShow, UINT status);
	HBRUSH OnCtlColorStatic(HDC hdc, HWND hCtrl, int id);
	HBRUSH OnCtlColorEdit(HDC hdc, HWND hCtrl, int id);
	HBRUSH OnCtlColor(HDC hdc, HWND hCtrl, int id);
	int GetCurrentCursor();
	int SetSize();
	void OnNotify(HWND hwnd, int idcc, LPARAM lParam);
	bool cannotbeBP(int line);
	LRESULT ProcessCustomDraw (NMHDR *lParam, bool tick=false);
	void lvInit();
	void FillupContent(vector<string> in);
	int ShowFileContent(const char* fullfilename);
	int GetBPandUpdate();

	void Debug(CAstSig *pastsig, DEBUG_STATUS type, int line=-1);

};

//To create CDebugDlg, make data like the following
struct CREATE_CDebugDlg
{
	CDebugDlg *dbDlg;
	char fullfilename[256];
};


class CDebugBaseDlg : public CWndDlg
{
public:

	HWND hTab;
	map<HWND,string> udfname;
	map<string,string> udf_fullpath;
	map<string,HWND> dlgPage_hwnd;

	CDebugBaseDlg(void);
	~CDebugBaseDlg(void);

	int open_tab_with_UDF(const char *fullname, int shownID);
	BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam);
	void OnCommand(int idc, HWND hwndCtl, UINT event);
	void OnSysCommand(UINT cmd, int x, int y);
	void OnSize(UINT state, int cx, int cy);
	void OnClose();
	void OnDestroy();
	void OnShowWindow(BOOL fShow, UINT status);
	void OnNotify(HWND hwnd, int idcc, NMHDR *pnm);
	int FileOpen(char *fullfilename);
	int GetID(const char *fname);
};

#endif // DEBUG_DLG
