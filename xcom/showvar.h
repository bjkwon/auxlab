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
#include "graffy.h"
#ifndef SIGPROC
#include "sigproc.h"
#endif
#include "msgCrack.h"

#include <algorithm> // for remove_if
#include "TabCtrl.h"
#include "debugDlg.h"

#define  SHOWFOREGROUND	989

typedef struct {
	HWND hBase;
	char varname[256];
} VARPLOT;

class CVectorsheetDlg : public CWndDlg
{
public:
	string name;
	CTimeSeries *psig;
	vector<CVar *> *sigvector;
	HWND hList1;
	LVCOLUMN LvCol;
	LVITEM LvItem; 
	char label[256];
	unsigned int id1, id2;
	unsigned int jd1, jd2;
	unsigned int nRows, nCols;
	HFONT eFont;
	bool clean_psig;

	CVectorsheetDlg(CWndDlg* hPar);
	~CVectorsheetDlg(void);

	void Init2(const char *windowtitle);
	BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam);
	void OnCommand(int idc, HWND hwndCtl, UINT event);
	void OnSize(UINT state, int cx, int cy);
	void OnMove(int cx, int cy);
	void OnClose();
	void OnDestroy();
	void OnShowWindow(BOOL fShow, UINT status);
	void OnNotify(HWND hwnd, int idcc, LPARAM lParam);
	LRESULT ProcessCustomDraw (NMHDR *lParam);
	void FillUp();
	void FillUp(vector<CVar *> *_sigvector);
	int SetCell(unsigned int row, unsigned int col, char *buf, bool large=true);
	void SetColumnTitles(vector<size_t> &nChars, HDC hdc, unsigned int id1, unsigned id2, char *title=NULL);
	void AppendRow(char *title, int rowID);
	void ResetDraw(unsigned int p, unsigned int q);
	vector<int> SetColumnWidths(HDC hdc, vector<size_t> &nChars);
private:
	int head_height;
	int offset;
	int slope;
	vector<int> width;
};

class CShowvarDlg : public CWndDlg
{
public:
	static int CShowvarDlg::nPlaybackCount;
	string name;
	CAstSig *pcast;
	map<string, CVar> *pVars;
	map<string, vector<CVar *>> *pGOvars;
	map<string, string> *pGOvarsPar;
	vector<string> scopes;
	CDebugDlg *lastDebug;
	HWND hList1, hList2;
	LVCOLUMN LvCol; // Column struct for ListView
	LVITEM LvItem;  // ListView Item struct
	CVar Sig;
	vector<HWND> plotDlgList;

	HFONT eFont;
	int soundID;
	bool playing;
	bool changed;
	bool win7;
	bool pcastCreated;
	bool cellview;

	CShowvarDlg(CWndDlg* hPar, map<string, CVar> *pvars);
	~CShowvarDlg(void);

	DWORD calculate_width(int newwidth[], int newwidth2[]);
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
	void OnNotify(HWND hwnd, int idcc, LPARAM lParam);
	LRESULT ProcessCustomDraw (NMHDR *lParam);
	void OnSoundEvent(CVar *pvar, int code);
	void OnPlotDlgCreated(const char *varname, GRAFWNDDLGSTRUCT *pin);
	void OnPlotDlgDestroyed(const char *varname, HWND hDlgPlot);
	void OnVarChanged(const char *varname=NULL);
	void OnCloseFig(int figID);
	void Fillup(map<string, CVar> *Tags=NULL, CSignals *cell=NULL);
	void fillrowvar(CVar *pvar, string varname);
	void fillrowvar(vector<CVar *>pvarvect, string varname);
	void updaterow(int row, int col, CVar *pvar);
	void UpdateProp(string varname, CVar *pvar, string propname);

	void showtype(CVar *pvar, char *buf);
	void showsize(CVar *pvar, char *outbuf);
	void showcontent(CVar *pvar, char *outbuf);
	void ArrangeControls(map<string, CVar> *Vars, int cx, int cy);
	void ArrangeControlsInit();
	CWndDlg * DoesThisVarViewExist(string varname);
	void InitVarShow(int type, const char *name);
	void debug(DEBUG_STATUS status, CAstSig *debugAstSig, int entry);
	void AdjustWidths(int redraw=0);
	int ClearVar(const char *var);
	void plotvar(CVar *psig, string title, const char *varname);
	void plotvar_update(CFigure *cfig, CVar *psig);
	double plotvar_update2(CAxes *pax, CSignals *psig);
	CFigure * newFigure(CRect rt, string title, const char *varname, GRAFWNDDLGSTRUCT *pin);
	HWND varname2HWND(const char *varname);

	HACCEL hAccel;
	HANDLE curFig;
	HANDLE *figwnd;
	int nFigureWnd;
	int fs;
	char VerStr[32];
private:
	int list_head_height;
	int idc_stop_height;
	int slope;
	int offset;
	int listHeight[2]; // heights of listview controls (audio, non-audio)
};
