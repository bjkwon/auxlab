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
#define	CUR_MOUSE_POS	3031 // delete
#define	CLEAR_CUR_MOUSE_POS	3030
#define	FIRST_DISPLAY_STATUSBAR	3214
#define	MOUSE_CURSOR_SETTING	3034
#define	WINDOW_SIZE_ADJUSTING	3035
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

class MouseStatus
{
public: 
	MouseStatus() { last_clickPt = CPoint(-1, -1); curPt = CPoint(-1, -1); last_MovPt = CPoint(-1, -1); curAx = NULL; };
	virtual ~MouseStatus() {};
	bool clickedOn;
	CPoint last_clickPt;
	CPoint last_MovPt;
	CPoint curPt;
	CAxes *curAx; // axes being controlled by current mouse movement
};

class CSBAR;

class CPlotDlg : public CWndDlg
{
public:
	vector<CRect> rctHist;
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
	MouseStatus mst;
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
	char callbackIdentifer[LEN_CALLBACKIDENTIFIER];
private:
	bool spgram;
	bool levelView;
	bool playing;
	bool paused;
	bool axis_expanding;
	bool inprogress; // if displayed while the data is incoming, this is set true; necessary to block operations requiring instant computation/updates e.g., OnMouseMove() 
	unsigned short ClickOn;
	unsigned short MoveOn;
	int playingIndex;
	STATUSBARINFO sbinfo;
	CPoint clickedPt;
	CPoint lastPtDrawn;
	CSBAR *sbar;
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
	void ChangeColorSpecAx(CAxes *cax, bool onoff);
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
	void OnLButtonDblClk(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	void DrawTicks(CDC *pDC, CAxes *pax, char xy);
	CRect DrawAxis(CDC *pDC, PAINTSTRUCT *ps, CAxes *pax);
	HACCEL GetAccel();
	void UpdateRects(CAxes *ax);
	unsigned short GetMousePos(CPoint pt);
	void GetAudioSignal(CSignals *pout, CAxes* pax = NULL, bool makechainless = true);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnMenu(UINT nId);
	void OnTimer(UINT id);
	void WindowSizeAdjusting();
	int ViewSpectrum();
	void OnCommand(int idc, HWND hwndCtl, UINT event);
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void OnActivate(UINT state, HWND hwndActDeact, BOOL fMinimized);
	BOOL OnNCActivate(UINT state);
	void OnSoundEvent(CVar *pvar, int code);
	void MouseLeave(UINT umsg);
	int makeDrawVector(POINT *out, const CSignal *p, CAxes *pax, CLine *thisline, CRect rcPaint);
	int estimateDrawCounts(const CSignal *p, CAxes *pax, CLine *thisline, RECT paintRC);
	void DrawMarker(CDC dc, CLine* mline, POINT *draw, int nDraws);
	POINT GetIndDisplayed(CAxes *pax);
	RANGE_ID GetIndSelected(CAxes *pax);
	void HandleLostFocus(UINT umsg, LPARAM lParam=0);
	void ShowStatusBar(SHOWSTATUS status=FULL, const char* msg=NULL); // to be gone
	void dBRMS(SHOWSTATUS st = FULL); // currently not used. just leave it
	void GetSignalIndicesofInterest(int code, CAxes *pax, int & ind1, int &ind2);
	int GetSelect(CSignals *pout);
	void ShowSpectrum(CAxes *pax, CAxes *paxBase);
	void getFFTdata(CTimeSeries *sig_mag, double *fft, int len);
	void setInProg(bool ch) { inprogress = ch; };
	bool getInProg() { return inprogress; };
	void Register(CAxes *pax, bool b);
	void SetAxBelowMouse(CAxes *pax, bool b) { pax->belowMouse = b; };
	void OnGetdefid();
	friend class CSBAR;
};

class CSBAR
{
public:
	CSBAR(CPlotDlg *hPar) {
		base = hPar; 
		cax = nullptr;
	};
	HWND hStatusbar;
	CAxes *cax;
	vector<CAxes *> ax; // registered axes for this sbar
	virtual ~CSBAR() {};
	CPlotDlg *base;
	void showXLIM(CAxes *cax=NULL, CPoint point=CPoint(-1,-1));
	void showXSEL(int ch); // -1 means clear
	void showCursor(CPoint point);
	void dBRMS(SHOWSTATUS st = FULL);
};