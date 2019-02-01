// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: graffy
// Graphic Library (Windows only)
// 
// 
// Version: 1.4951
// Date: 12/14/2018
// Change from 1.495: WndDlg0.h dropped 

#include "graffy.h"	
#include "resource.h"	
#include "fftw3.h"
#include <math.h>

#define	CLOSE_FIGURE	1024
#define	CUR_MOUSE_POS	3030
#define	MOUSE_CURSOR_SETTING	3034
#define WM__AUDIOEVENT	WM_APP + WOM_OPEN

#define  RC_SPECTRUM			0x0f00
#define  RC_SPECTRUM_YTICK	0x0100
#define  RC_SPECTRUM_XTICK	0x0400
#define  RC_WAVEFORM		0x000f

#define  FRINGE1		1 // left of axis
#define  FRINGE2		2 // right of axis
#define  FRINGE3		4 // above axis
#define  FRINGE4		8 // below  axis
#define  AXCORE			15

#define  GRAF_REG1		1 // left (or bottom) of region
#define  GRAF_REG2		2 // right (or top) of region
#define  GRAF_REG3		3 // region


#define  XTIC		0x0001
#define  YTIC		0x0002
#define  RMSTIC		0x0004
#define  HZTIC		0x0008
#define  ALLTICS	XTIC + YTIC + RMSTIC + HZTIC

class RANGE_PX
{
public:
    int  px1;
    int  px2;
	RANGE_PX() {px1=-1; px2=-1;}
	RANGE_PX(int x1, int x2) {px1=x1; px2=x2;}
	virtual ~RANGE_PX() {};
	void reset() {px1=px2=-1;}
	bool empty() {if (px1<=px2 || px1<=0 || px2<=0) return true; else return false;}
	bool operator==(const RANGE_PX &sec) const
	{
		if (px1==sec.px1 && px2==sec.px2) return true;
		else return false;
	}
	bool operator!=(const RANGE_PX &sec) const
	{
		if (px1==sec.px1 && px2==sec.px2) return false;
		else return true;
	}
};

typedef struct tagRANGE_ID
{
    int  id1;
    int  id2;
} RANGE_ID;

typedef struct tagRANGE_TP
{
    double tp1;
    double tp2;
} RANGE_TP;

/////////////////////////////////////////////////////////////////////////////
// CPlotDlg dialog

typedef struct tagSTATUSBARINFO
{
	double xBegin;
	double xEnd;
	double xCur;
	double yCur;
	double xSelBegin;
	double xSelEnd;
//	double rms;
	bool initialshow;
	vector<CAxes *> vax;
} STATUSBARINFO;


enum SHOWSTATUS
{
	CLEAR,
	CURSOR_LOCATION,
	CURSOR_LOCATION_SPECTRUM,
	FULL,
};

class CPlotDlg : public CWndDlg
{
public:
	CPoint gcmp; //Current mouse point
	CPoint z0pt; // point where zooming range begins
	CString errStr;
	CAxes * gca;
	CMenu subMenu;
	CMenu menu;
	unsigned char opacity;
	int devID;
	int zoom;
	double block; // Duration in milliseconds of playback buffer. 
	int playLoc; // pixel point (x) corresponding to the wavform that is being played at this very moment
	int playLoc0; // pixel advances of next x point of vertical line showing the progress of playback
	int playCursor; // pixel point (x) corresponding to the wavform set by the user (clicked and held for 1 sec)
	INT_PTR hAudio; // audio handle
	CRect roct[7];
	CRect region[2], fringe[4];
	CFigure gcf;
	RANGE_PX edge;
	RANGE_TP selRange;
	char * title; // for "numbered" CPlotDlg, HIWORD is NULL and LOWORD shows the number; for a string title, title is in const char *, during construction the pointer is set up.
	CAstSig *pctx;
private:
	bool spgram;
	bool levelView;
	bool playing;
	bool paused;
	bool axis_expanding;
	unsigned short ClickOn;
	unsigned short MoveOn;
	int playingIndex;
	STATUSBARINFO sbinfo;
	CPoint clickedPt;
	CPoint lastPtDrawn;
	HWND hStatusbar;
	CPoint mov0; // used for WM_MOUSEMOVE
	CPoint lbuttondownpoint; 
	CPoint lastpoint; // used in WM_MOUSEMOVE
	CRect lastrect; // used for WM_MOUSEMOVE
	double spaxposlimx, spaxposlimy; // used for WM_MOUSEMOVE
	CPosition lastPos;
	HACCEL hAccel;
	COLORREF selColor;
	COLORREF orgColor;
	void setpbprogln();
	void ChangeColorSpecAx(CRect rt, bool onoff);
	int IsSpectrumAxis(CAxes* pax);
	HWND CreateTT(HWND hPar, TOOLINFO *ti);
//	BOOL validateScope(bool onoff);
	HWND hTTscript, hTTtimeax[10];
	TOOLINFO ti_script, ti_taxis;
	CBrush hQuickSolidBrush;
	RANGE_PX curRange;
	vector<string> ttstat;
public:
	CPlotDlg();
	CPlotDlg(HINSTANCE hInstance, CGobj *hPar = NULL);   // standard constructor
	~CPlotDlg();
	CAxes * CurrentPoint2CurrentAxis(Win32xx::CPoint *point);
	void OnPaint();
	void OnClose();
	void OnDestroy();
	void OnSize(UINT nType, int cx, int cy);
	void OnMove(int x, int y);
	void OnRButtonUp(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void SetGCF();
	void OnLButtonDblClk(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	void DrawTicks(CDC *pDC, CAxes *pax, char xy);
	CRect DrawAxis(CDC *pDC, PAINTSTRUCT *ps, CAxes *pax);
	HACCEL GetAccel();
	void UpdateRects(CAxes *ax);
	unsigned short GetMousePos(CPoint pt);
	CSignals GetAudioSignal(CAxes* pax = NULL, bool makechainless = true);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnMenu(UINT nId);
	void OnTimer(UINT id);
	void OnCommand(int idc, HWND hwndCtl, UINT event);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnActivate(UINT state, HWND hwndActDeact, BOOL fMinimized);
	BOOL OnNCActivate(UINT state);
	void OnSoundEvent(CVar *pvar, int code);
	void MouseLeave(UINT umsg);
	vector<POINT> makeDrawVector(CSignal *p, CAxes *pax);
	void DrawMarker(CDC dc, CLine* mline, vector<POINT> draw);
	POINT GetIndDisplayed(CAxes *pax);
	RANGE_ID GetIndSelected(CAxes *pax);
	void HandleLostFocus(UINT umsg, LPARAM lParam=0);
	void ShowStatusBar(SHOWSTATUS status=FULL, const char* msg=NULL);
	void GetSignalIndicesofInterest(int code, CAxes *pax, int & ind1, int &ind2);
	int GetSelect(CSignals *pout);
	void ShowSpectrum(CAxes *pax, CAxes *paxBase);
	void getFFTdata(CSignals *sig_mag, CAxes *pax, double *fft, double *freq, int len);
};
