// AUXLAB 
//
// Copyright (c) 2009-2019Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: graffy
// Graphic Library (Windows only)
// 
// 
// Version: 1.504
// Date: 7/5/2019
// 

/* Note on multiple axes situation,
Check everything with gcf.ax.front() and verify that's what I meant.
When I first used it, it was meant to be the waveform axis (as opposed to spectrum axis), but multiple axes can exist, that shouldn't be assumed.
Replace gcf.ax.front() with the current axis pointer as appropriate.
1/12/2018
*/

/* In OnPaint() gengrids is called and tics1 is set if tics1 is empty. 
This means that if you want tics1 to be reset inside OnPaint, clear tics1 when InvalidateRect is called.
if tics1 is not empty, OnPaint will not set tics1.
5/10/2018
*/

#include "PlotDlg.h"
#include <limits>
#include <mutex>
#include <thread>

#include "wavplay.h"

#define IDM_SPECTRUM_INTERNAL	2222
#define IDC_LEVEL			7000
#define IDC_STATIC_LEVEL	7001
#define MOVE_SPECAX			8560
#define ID_STATUSBAR  1030
#define NO_SELECTION  RANGE_PX(-1,-1)

#define PBPROG_ADVANCE_PIXELS	2

#define MAKE_dB(x) (20*log10((x)) + 3.0103)
#define ROUND(x) (int)((x)+.5)
#define FIX(x) (int)((x))

extern HWND hPlotDlgCurrent;
extern HWND hwnd_AudioCapture;
extern mutex mtx4PlotDlg;
//extern HWND hwnd_AudioCapture;

mutex mtx_OnPaint;

FILE *fpp;

static inline int iabs(int x)
{
	if (x<0)		return -x;
	return x;
}

static inline int LRrange(RECT* rect, int var, char xy)
{
	// 1 for R, -1 for L, 0 for between    (x)
	// 1 for above, -1 for below, 0 for bet (y)
	switch (xy)
	{
	case 'x':
		if (var > rect->right)	return 1;
		else if (var < rect->left)	return -1;
		else return 0;
	case 'y':
		if (var > rect->bottom)	return -1;
		else if (var < rect->top)	return 1;
		else return 0;
	default:
		return -9999;
	}
}

static inline unsigned _int8 GetMousePosReAx(CPoint pt, CAxes* pax)
{// This reveals where pt is located relative to pax
 // If pt is inside the axis rectangle, returns 15 (i.e., 0b1111)
 // If pt is in the ytick area, returns 0b0001
 // If pt is in the xtick area, returns 0b0100
 // If pt is in the xtick and ytick area, returns 0b0101
 // Otherwise, return 0
	if (pax->rct.PtInRect(pt)) return AXCORE;
	unsigned _int8 res(0);
	if (pax->xtick.rt.PtInRect(pt)) res = GRAF_REG1 << 2;
	if (pax->ytick.rt.PtInRect(pt)) res += GRAF_REG1;
	return res;
}

CPlotDlg::CPlotDlg(HINSTANCE hInstance, CGobj *hPar)
:axis_expanding(false), levelView(false), playing(false), paused(false), ClickOn(0), MoveOn(0), devID(0), playLoc(-1), zoom(0), spgram(false), selColor(RGB(150, 180, 155)), gca(NULL), playCursor(-1), inprogress(false)
{
	opacity = 0xff;
	hAudio = 0;
	gcf.m_dlg = this;
	memset(callbackIdentifer, 0, LEN_CALLBACKIDENTIFIER);
	if (hPar)
	{
		gcf.hPar = hPar;
		gcf.hPar->child.push_back(&gcf);
	}

	menu.LoadMenu(IDR_POPMENU);
	subMenu = menu.GetSubMenu(0);
	gcmp=CPoint(-1,-1);
	hInst = hInstance;
	hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR));
	for (int k(0); k<10; k++) hTTtimeax[k]=NULL;
	ttstat.push_back("begin(screen)");
	ttstat.push_back("end(screen)");
	ttstat.push_back("Cursor position");
	ttstat.push_back("");
	ttstat.push_back("begin(selection)");
	ttstat.push_back("end(selection)");
	ttstat.push_back("dBRMS");
	sbinfo.initialshow = false;
	sbinfo.xSelBegin = sbinfo.xSelEnd = 0.;
}

CPlotDlg::~CPlotDlg()
{
	if (HIWORD((INT_PTR)title))
		delete title;
	delete sbar;
}

void CPlotDlg::OnDestroy()
{
	//Don't "delete this" here
	DestroyWindow();
}

void CPlotDlg::OnClose()
{
	// Instead of calling DestroyWindow(), post the message to message loop (either in Auxtra.cpp or plotThread.cpp) and properly delete the figure (and reduce theApp.nFigures by 1)
//	PostMessage(WM_DESTROY);
	StopPlay(hAudio, true);
	PostMessage(WM_QUIT);

	/* axes and texts shouldn't be deleted here. Why?
	1) They are deleted and the vector values are popped during ~CFigure() later anyway
	2) But if they are deleted here, some pointers in GOvars are astray (no longer valid and the application crash 
	during OnPlotDlgDestroyed (which is called from hooking in showvar.cpp). 
	* Note that It might not crash if there's a guarantee that OnPlotDlgDestroyed processes before deleteObj,  
	but I don't control the timing--i.e., OnPlotDlgDestroyed and deleteObj proceed concurrently and there's no timing guarantee.
	9/2/2018 */
	//for (size_t k=0; k<gcf.ax.size(); k++)	deleteObj(gcf.ax[k]); 
	//for (size_t k=0; k<gcf.text.size(); k++)	deleteObj(gcf.text[k]);
}

HACCEL CPlotDlg::GetAccel()
{
	return hAccel;
}

HWND CPlotDlg::CreateTT(HWND hPar, TOOLINFO *tinfo)
{
    HWND TT = ::CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,        
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hPar, NULL, hInst, NULL);

    ::SetWindowPos(hTTscript, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    tinfo->cbSize = sizeof(TOOLINFO);
    tinfo->uFlags = TTF_SUBCLASS;
    tinfo->hwnd = hPar;
    tinfo->hinst = hInst;
    tinfo->lpszText = "";
	tinfo->rect.left=tinfo->rect.right=0;
    
    /* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
    ::SendMessage(TT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) tinfo);	
	::SendMessage(TT, TTM_SETMAXTIPWIDTH, 0, 400);

	return TT;
}
/*
BOOL CPlotDlg::validateScope(bool onoff)
{
	if (!strchr(title, ':')) return 0;
	vector<string> tp;
	str2vect(tp, title, ':');
	if (!onoff)
	{
		unsigned char op = 0x10;
		SetLayeredWindowAttributes(hDlg, 0, op, LWA_ALPHA); // how can I transport transparency from application? Let's think about it tomorrow 1/6/2017 12:19am
		return 1;
	}
	return 0;
}
*/
BOOL CPlotDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SetClassLongPtr(hwnd, GCLP_HICON, (LONG)(LONG_PTR)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, 0));
	mst.clickedOn = false;
	sbar = new CSBAR(this);
	if (lParam == -1) return TRUE; // if lParam is -1, no statusbar
	SetLayeredWindowAttributes(hwnd, 0, opacity, LWA_ALPHA); // how can I transport transparency from application? Let's think about it tomorrow 1/6/2017 12:19am
	hTTscript = CreateTT(hwnd, &ti_script);
    GetClientRect (hwnd, &ti_script.rect);
	ti_script.rect.bottom = ti_script.rect.top + 30;
	::SendMessage(hTTscript, TTM_ACTIVATE, TRUE, 0);	
	::SendMessage(hTTscript, TTM_SETMAXTIPWIDTH, 0, 400);

	sbar->hStatusbar = CreateWindow(STATUSCLASSNAME, "", WS_CHILD | WS_VISIBLE | WS_BORDER | SBS_SIZEGRIP,
		0, 0, 0, 0, hwnd, (HMENU)0, hInst, NULL);
	int width[] = { 40, 110, 2, 40, 40, 160, };
	int sbarWidthArray[8];
	sbarWidthArray[0] = 40;
	for (int k = 1; k < 8; k++)
		sbarWidthArray[k] = sbarWidthArray[k - 1] + width[k - 1];
	sbarWidthArray[7] = -1;
	::SendMessage(sbar->hStatusbar, SB_SETPARTS, 12, (LPARAM)sbarWidthArray);
	// This is necessary to set the showVarDlg window from showvar.cpp as the "anchor" for playback // check if it's still needed 9/4/2019
	SetHWND_WAVPLAY((HWND)lParam); 
	return TRUE;
}

void CPlotDlg::OnCommand(int idc, HWND hwndCtl, UINT event)
{
	int res(0);
	switch (idc)
	{
	case IDM_ADJUSTWINDOWSIZE:
	case IDM_CHANNEL_TOGGLE:
	case IDM_FULLVIEW:
	case IDM_ZOOM_IN:
	case IDM_ZOOM_OUT:
	case IDM_LEFT_STEP:
	case IDM_RIGHT_STEP:
	case IDM_TRANSPARENCY:
#ifndef NO_PLAYSND
	case IDM_PLAY:
	case IDM_STOP:
#endif
#ifndef NO_FFTW
	case IDM_SPECTRUM:
#endif
#ifndef NO_SF
	case IDM_WAVWRITE:
	case IDM_MP3WRITE:
#endif
	case IDM_SPECTROGRAM:
	case IDM_ZOOMSELECT:
		OnMenu(idc);
		break;
	}
}

void CPlotDlg::GetSignalIndicesofInterest(int code, CAxes *pax, int & ind1, int &ind2)
{// This returns the indices of the "main" signal first axis, m_ln[0].sig currently selected, based on xlim, and curRange.
 // if not selected, ind1 and ind2 are the indices of the screen viewing range
 // Assumption: the signal is audio without chain
	
	int fs = pax->m_ln[0]->sig.GetFs();
	if (code) 
	{
		ind1 = (int)(pax->pix2timepoint(curRange.px1)*fs+.5);
		ind2 = (int)(pax->pix2timepoint(curRange.px2)*fs);
	}
	else
	{
		ind1 = (int)(pax->xlim[0]*fs+.5);
		ind2 = (int)(pax->xlim[1]*fs);
	}
}

POINT CPlotDlg::GetIndDisplayed(CAxes *pax)
{ // This returns the indices (x for begin and y for end) of the "main" signal first axis, m_ln[0].sig	currently displayed on the screen (based on current xlim)
	POINT out = {0,0};
	if (gcf.ax.size()==0) return out;
	if (pax->m_ln.size()==0) return out;
	if (pax->xlim[0]>=pax->xlim[1]) return out;
	CTimeSeries *p = &pax->m_ln.front()->sig;
	int fs = p->GetFs();
	if (p->IsTimeSignal())
	{ // Just assume one big chunk (no chain)....
		out.x = (int)(pax->xlim[0]*fs+.5);
		out.y = (int)(pax->xlim[1]*fs+.5);
	}
	else
	{
		if (pax->m_ln.front()->xdata.nSamples == 0) // xdata is just the sample index.
		{
			out.x = (int)ceil(pax->xlim[0])-1; // zero-based index
			out.y = (int)pax->xlim[1];
		}
		else // xdata is specified by the user.
		{ // Let's assume that xdata is monotonously increasing.
			float fxlim0((float)pax->xlim[0]);
			float fxlim1((float)pax->xlim[1]); // why float? the last point was missing (id was less than one than should have been) because xlim[1] was 3.9999999999999991 and xdata was 4
			for (unsigned int id=0; id<p->nSamples; id++)
				if (pax->m_ln.front()->xdata.buf[id]>=fxlim0)
				{ out.x = id; break;}
			for (int id=(int)p->nSamples-1; id>=0; id--)
				if (pax->m_ln.front()->xdata.buf[id]<=fxlim1) 
				{ out.y = id; break;}
		}
	}
	return out;
}

int CPlotDlg::estimateDrawCounts(const CSignal *p, CAxes *pax, CLine *lyne, RECT paintRC)
{
	//need these two lines because after selection play is done, InvalidateRect might have been called for this part
	if (paintRC.left > pax->rct.right) return 0;
	if (paintRC.right < pax->rct.left) return 0;
	if (paintRC.right > pax->rct.right) paintRC.right = pax->rct.right;
	if (paintRC.left < pax->rct.left) paintRC.left = pax->rct.left;
	if (pax->xlim[0] >= pax->xlim[1]) return -1;
	// if xy plot, return the data count
	if (lyne->xdata.nSamples) return p->nSamples;
	int fs = lyne->sig.GetFs();
	double xPerPixel = (pax->xlim[1] - pax->xlim[0]) / (double)pax->rct.Width(); // How much advance in x-axis per one pixel--time for audio-sig, sample points for nonaudio plot(y), whatever x-axis means for nonaudio play(x,y)
	double nSamplesPerPixel; // how many sample counts of the sig are covered per one pixel. Calculated as a non-integer, rounded-down value is used and every once in a while the remainder is added
	POINT pp = GetIndDisplayed(pax);//The indices of xlim[0] and xlim[1], if p were to be a single chain.
	if (pp.x == pp.y) return -2;
	int ind1 = max(0, pp.x);
	int ind2 = max(0, pp.y);
	bool tseries = ((CTimeSeries*)p)->IsTimeSignal();
	double tpoint1, tpoint2;
	if (tseries)
	{ // if p is one of multiple chains, ind1 and ind2 need to be adjusted to reflect indices of the current p.
		nSamplesPerPixel = xPerPixel * fs;
		tpoint1 = pax->pix2timepoint(paintRC.left);
		tpoint2 = pax->pix2timepoint(paintRC.right);
	}
	else
	{
		nSamplesPerPixel = xPerPixel * (pax->xlim[1] - pax->xlim[0]) / (ind2 - ind1);
		tpoint1 = pax->pix2timepoint(paintRC.left);
		tpoint2 = pax->pix2timepoint(paintRC.right);
	}
	double multiplier = 2.;
	int nPixels = pax->rct.right - pax->rct.left + 1; // pixel count to cover the plot
	int estimatedNSamples = (int)((pax->xlim[1] - pax->xlim[0])*(double)fs); // how many signal sample points are covered in the client axes--not actual sample counts because the signal may exist only in part of range
	int paintAreaWidth = paintRC.right - paintRC.left;
	return paintAreaWidth *2;
}

int CPlotDlg::makeDrawVector(POINT *out, const CSignal *p, CAxes *pax, CLine *lyne, CRect rcPaint)
{
	//need these two lines because after selection play is done, InvalidateRect might have been called for this part
	if (rcPaint.left > pax->rct.right) return 0; 
	if (rcPaint.right < pax->rct.left) return 0;
	if (rcPaint.right > pax->rct.right) rcPaint.right = pax->rct.right;
	if (rcPaint.left < pax->rct.left) rcPaint.left = pax->rct.left;
	POINT pp = GetIndDisplayed(pax);//The indices of xlim[0] and xlim[1], if p were to be a single chain.
	if (pp.x == pp.y) return -2;
	POINT pt = { -91128,-911282 };
	const int fs = lyne->sig.GetFs();
	CRect rcPal = rcPaint; // the area to draw
	rcPal.IntersectRect(rcPal, pax->rct);
	bool tseries = ((CTimeSeries*)p)->IsTimeSignal();
	double x1, y1, x2, y2; // x1 x2: time points of rcPaint relative to rct and xlim
	pax->GetCoordinate(rcPal.TopLeft(), x1, y1);
	pax->GetCoordinate(rcPal.BottomRight(), x2, y2);
	int rightEdge = rcPal.right;
	rightEdge = max(rcPal.left, rightEdge);
	rightEdge = min(rightEdge, pax->rct.right);
	int idBegin, idLast, nSamples2Display;
	//number of samples to display
	int count = 0;
	double xPerPixel = (pax->xlim[1] - pax->xlim[0]) / (double)pax->rct.Width(); // How much advance in x-axis per one pixel--time for audio-sig, sample points for nonaudio plot(y), whatever x-axis means for nonaudio play(x,y)
	double nSamplesPerPixel; // how many sample counts of the sig are covered per one pixel. Calculated as a non-integer, rounded-down value is used and every once in a while the remainder is added
	bool monotonic = true; // actually, monotonically increasing, to be precise
	if (tseries)
	{
		nSamplesPerPixel = xPerPixel * fs;
		double tmarkms = p->tmark / 1000.;
		//index corresponding to x1
		if (x2 < tmarkms) return 0;
		double dur = p->durc().front() / 1000.;
		if (x1 > p->tmark / 1000. + dur) return 0;
		rightEdge = min(rightEdge, pax->double2pixel(p->tmark / 1000.+dur, 'x'));
		//if part selected, it begins greater of begin_of_sel and tmark
		//...... it ends less of end_of_sel and tmark+dur
		if (x1 <= tmarkms) idBegin = 0;
		else
			idBegin = (int)((x1 - tmarkms) * fs);
		idLast = (int)((x2 - tmarkms) * fs);
		idLast = min(idLast, (int)p->nSamples);
		nSamples2Display = idLast - idBegin;
		if (nSamples2Display > (int)p->nSamples - idBegin) nSamples2Display = (int)p->nSamples - idBegin;
	}
	else
	{
		nSamplesPerPixel = xPerPixel * p->nSamples / (pax->xlim[1] - pax->xlim[0]);
		idBegin = max((int)ceil(pax->xlim[0]), (int)x1) - 1;
		idBegin = max(0, idBegin);
		idLast = min((int)ceil(x2), (int)pax->xlim[1]);
		idLast = min(idLast, (int)p->nSamples);
		if (lyne->xdata.nSamples)
		{ // Assuming that lyne->xdata is monotonically increasing
			//first index that is equal or greater than idBegin
			// To do--check if it is not monotonically increasing
			// and call non_monotonic = true here if so.
			double lastval = lyne->xdata.buf[0];
			for (unsigned int k = 1; k < lyne->xdata.nSamples; k++)
			{
				if (lyne->xdata.buf[k] - lastval < 0)
				{
					monotonic = false;
					break;
				}
				lastval = lyne->xdata.buf[k];
			}
		}
		nSamples2Display = !lyne->xdata.IsEmpty() ? lyne->xdata.nSamples : idLast - idBegin;
	}
	//make this conditional prettier when you have time 4/8/2019
	if (nSamplesPerPixel > 2. && monotonic)
	{
		double adder = 0;
		int chunkID0 = idBegin;
		int chunkID1 = idBegin + (int)nSamplesPerPixel;
		const double remnant = nSamplesPerPixel - (int)nSamplesPerPixel;
		pt.x = pax->double2pixel(max(x1, p->tmark/1000.), 'x');
		while (1)
		{
			double dtp;
			if (chunkID0 >= idLast - 1)
				break;
			if (chunkID1 > (int)p->nSamples) chunkID1 = (int)p->nSamples;
			if (chunkID1 - chunkID0 <= 1)
			{
				dtp = pax->rct.bottom - pax->rct.Height() * (p->buf[chunkID0] - pax->ylim[0]) / (pax->ylim[1] - pax->ylim[0]);
				pt.y = min(max((int)(dtp + .5), pax->rct.top), pax->rct.bottom);
				out[count++] = pt;
			}
			else
			{
				int lastpty = std::numeric_limits<int>::infinity();
				const pair<double*, double*> pr = minmax_element(p->buf + chunkID0, p->buf + chunkID1);
				if (*pr.first < pax->ylim[0]) *pr.first = pax->ylim[0];
				dtp = pax->rct.bottom - pax->rct.Height() * (*pr.first - pax->ylim[0]) / (pax->ylim[1] - pax->ylim[0]);
				pt.y = min(max((int)(dtp + .5), pax->rct.top), pax->rct.bottom);
				out[count++] = pt;
				lastpty = pt.y;
				if (*pr.second < pax->ylim[0]) *pr.second = pax->ylim[0];
				dtp = pax->rct.bottom - pax->rct.Height() * (*pr.second - pax->ylim[0]) / (pax->ylim[1] - pax->ylim[0]);
				pt.y = max((int)(dtp + .5), pax->rct.top);
				if (pt.y != lastpty)
					out[count++] = pt;
			}
			adder += remnant;
			chunkID0 = chunkID1;
			chunkID1 += (int)nSamplesPerPixel;
			if (pt.x++ == rightEdge - 1)
				break;
			if (adder > 1) { chunkID1++; adder--; }
		}
	}
	else
	{
		if (lyne->xdata.nSamples) // for xy plot, just make the draw vector here and return. Disregard xlim and ylim
		{
			for (unsigned int k = 0; k < p->nSamples; k++)
			{
				CPoint pt0 = pt;
				pt = pax->double2pixelpt(lyne->xdata.buf[k], p->buf[k], NULL);
				if (pt0 != pt)
					out[count++] = pt;
			}
		}
		else
		{
			for (int k = idBegin; k < idBegin + nSamples2Display; k++)
			{
				if (tseries)
					pt = pax->double2pixelpt(p->tmark / 1000. + (double)k / fs, p->buf[k], NULL);
				else if (lyne->xdata.nSamples > 0)
					pt = pax->double2pixelpt(lyne->xdata.buf[k], p->buf[k], NULL);
				else
					pt = pax->double2pixelpt((double)k + 1, p->buf[k], NULL);
				out[count++] = pt;
			}
		}
	}
	return count;
}

CPen * OnPaint_createpen_with_linestyle(CLine *pln, CDC& dc, CPen **pOldPen)
{
	LOGBRUSH lb;
	lb.lbStyle = BS_SOLID;
	lb.lbColor = pln->color;
	lb.lbHatch = HS_VERTICAL;
	DWORD style = lb.lbStyle;
	int penStyle;
	switch (pln->lineStyle)
	{
	case LineStyle_solid:
		penStyle = PS_SOLID;
		break;
	case LineStyle_dash:
		penStyle = PS_DASH;
		break;
	case LineStyle_dot:
		penStyle = PS_DOT;
		break;
	case LineStyle_dashdot:
		penStyle = PS_DASHDOT;
		break;
	case LineStyle_dashdotdot:
		penStyle = PS_DASHDOTDOT;
		break;
	default:
		penStyle = PS_NULL;
		break;
	}
	CPen *newPen = new CPen(penStyle | PS_GEOMETRIC, pln->lineWidth, &lb, 0, NULL);
	*pOldPen = (CPen*)dc.SelectObject(*newPen);
	return newPen;
}

void CPlotDlg::OnPaint() 
{
	//return here if hDlg is NULL (rare but it could happe) or coming without ax or text, as in figure function
	if (hDlg == NULL || (gcf.ax.empty() && gcf.text.empty()))  return;
	opacity = gcf.inScope ? 0xff : 0xc0;
	char buf[256];
	PAINTSTRUCT  ps;
	HDC hdc = BeginPaint(hDlg, &ps);
	if (hdc==NULL || gcf.visible != 1) { EndPaint(hDlg, &ps); return; }
	CDC dc(hdc, hDlg);
	CClientDC dc2(hDlg);
	CPoint pt;
	CRect clientRt;
	CAxes* pax;
	SetLayeredWindowAttributes(hDlg, 0, opacity, LWA_ALPHA);
	GetClientRect(hDlg, &clientRt);
	if (clientRt.Height()<15) { EndPaint(hDlg, &ps); return; }
	//sprintf(buf, "OnPaint (%d,%d)-(%d,%d), playCursor = %d, playLock %d\n", ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, playCursor, playLoc);
	//sendtoEventLogger(buf);
	dc.SolidFill(gcf.color, clientRt);
	CRect rt;
	GetClientRect(hDlg, &rt);
	bool paxready(false);
	//Drawing axes
//	for (vector<CAxes*>::iterator itax = gcf.ax.begin(); itax != gcf.ax.end(); )
// Why shouldn't an iterator be used in the for loop?
// axes may be added during the call from another thread
// then itax may become suddenly invalid in the middle of loop and cause crash. 4/8/2019

	// Is dB RMS displayed?
	::SendMessage(sbar->hStatusbar, SB_GETTEXT, 0, (LPARAM)buf);
	if (strlen(buf) == 0)
		SetTimer(FIRST_DISPLAY_STATUSBAR, 200);

	size_t k = 0;
	int temp = 0;
	for (unique_lock<mutex> locker(mtx_OnPaint); k< gcf.ax.size(); temp++)
	{
		if (temp < 1)
		{
			sprintf(buf, "(OnPaint) mtx_OnPaint locked: %d\n", locker.owns_lock());
			sendtoEventLogger(buf);
		}
		// drawing lines
		if (!paxready)
			pax = gcf.ax[k];
		else
			paxready=false;
		//if (find(sbinfo.vax.begin(), sbinfo.vax.end(), pax) == sbinfo.vax.end())
		//	sbinfo.vax.push_back(gcf.ax[k]);
		if (!pax->m_ln.empty()) 
			pax->xTimeScale = pax->m_ln.front()->sig.IsTimeSignal();
		if (!pax->visible) continue;
		if (axis_expanding)
			pax->pos.Set(clientRt, pax->rct); // do this again; what's this? 10/9/2019
		pax->rct=DrawAxis(&dc, &ps, pax);
		//when mouse is moving while clicked
		if (curRange != NO_SELECTION && pax->hPar->type==GRAFFY_figure ) // this applies only to waveform axis (not FFT axis)
		{
			rt = pax->rct;
			rt.left = curRange.px1+1; 
			rt.right = curRange.px2-1; 
			rt.top--;
			rt.bottom++;
			dc.SolidFill(selColor, rt);
		}
		size_t nLines = pax->m_ln.size(); // just FYI
		CPen * ppen = NULL;
		POINT *draw= new POINT [1];
		int nDraws = 0, estCount = 1;
		if (pax->m_ln.empty())
		{ // Need this to bypass axes without line object
			k++;
			continue;
		}
		else
		{
			if (hDlg == hwnd_AudioCapture)
			{
				sendtoEventLogger("(OnPaint) mtx4PlotDlg locking\n");
				unique_lock<mutex> locker(mtx4PlotDlg);
				sendtoEventLogger("(OnPaint) mtx4PlotDlg locked\n");
			}
			for (auto lyne : pax->m_ln)
			{
				CPen *pPenOld = NULL;
				if (!lyne->visible) continue;
				for (CTimeSeries *p = &(lyne->sig); p; p = p->chain)
				{
					auto anSamples = p->nSamples;
					auto atuck = p->nGroups;
					auto atmark = p->tmark;
					BYTE clcode;
					vector<DWORD> kolor;
					clcode = HIBYTE(HIWORD(lyne->color));
					if (clcode == 'L' || clcode == 'R')
						kolor = Colormap(clcode, clcode, 'r', atuck);
					if (clcode == 'l' || clcode == 'r')
						kolor = Colormap(clcode, clcode + 'R' - 'r', 'c', atuck);
					else if (clcode == 'M')
					{
						CVar cmap = lyne->strut["color"];
						for (unsigned int k = 0; k < cmap.nSamples; k += 3)
						{
							DWORD dw = RGB((int)(cmap.buf[k] * 255.), (int)(cmap.buf[k + 1] * 255.), (int)(cmap.buf[k + 2] * 255.));
							kolor.push_back(dw);
						}
					}
					else
						kolor.push_back(lyne->color);
					auto colorIt = kolor.begin();
					for (unsigned int m = 0; m < atuck; m++)
					{
						lyne->color = *colorIt;
						p->nSamples = p->Len();
						p->nGroups = 1;
						memcpy(p->buf, p->buf + m * p->nSamples, p->bufBlockSize*p->nSamples);
						if (p->IsTimeSignal())
							p->tmark += 1000.* m*p->nSamples / p->GetFs();
						if (lyne->lineWidth > 0 || lyne->symbol != 0)
						{
							int tp = estimateDrawCounts(p, pax, lyne, ps.rcPaint);
							if (tp > estCount)
							{
								estCount = tp;
								delete[] draw;
								draw = new POINT[estCount + 1];
							}
							nDraws = makeDrawVector(draw, p, pax, lyne, (CRect)ps.rcPaint);
							if (lyne->sig.nSamples==1)
								lyne->initial = lyne->final = draw[0];
#ifdef _DEBUG
							if (nDraws > estCount)
							{
								sprintf(buf, "left=%d,right=%d,est=%d,actual=%d", ps.rcPaint.left, ps.rcPaint.right, estCount, nDraws);
								::MessageBox(NULL, buf, "exceed", 0);
							}
#endif // _DEBUG
						}
						if (lyne->symbol != 0)
						{
							LineStyle org = lyne->lineStyle;
							lyne->lineStyle = LineStyle_solid;
							if (lyne->lineWidth == 0)
								lyne->lineWidth = 1;
							OnPaint_createpen_with_linestyle(lyne, dc, &pPenOld);
							DrawMarker(dc, lyne, draw, nDraws);
							lyne->lineStyle = org;
						}
						ppen = OnPaint_createpen_with_linestyle(lyne, dc, &pPenOld);
						if (lyne->lineWidth > 0)
						{
							if (p->IsTimeSignal()) {
								if (pt.y < pax->rct.top)  pt.y = pax->rct.top;
								if (pt.y > pax->rct.bottom) pt.y = pax->rct.bottom;
							}
							if (nDraws > 0 && lyne->lineStyle != LineStyle_noline)
							{
								int nOutOfAx = 0;
								for (int k = nDraws - 1; k > 0; k--)
								{
									if (draw[k].x > pax->rct.right) nOutOfAx++;
									else break;
								}
								dc.Polyline(draw, nDraws - nOutOfAx);
							}
							if (kolor.size() > 1)
							{
								colorIt++;
								if (colorIt == kolor.end())		colorIt = kolor.begin();
							}
							p->nGroups = atuck;
							p->tmark = atmark;
							p->nSamples = anSamples;
							if (ppen)
							{
								dc.SelectObject(pPenOld);
								delete ppen;
							}
						}
					}
				}
			}
		}
		if (hDlg == hwnd_AudioCapture)
			sendtoEventLogger("(OnPaint) mtx4PlotDlg unlocked\n");
		// add ticks and ticklabels
		dc.SetBkColor(gcf.color);
		// For the very first call to onPaint, rct is not known so settics is skipped, and need to set it here
		// also when InvalidateRect(NULL) is called, always remake ticks
		if (pax->m_ln.size() > 0)
		{
			//When there's nothing in axes.. bypass this part.. Otherwise,  gengrids will crash!! 10/5/2019
			if (pax->xtick.tics1.empty() && pax->xtick.automatic)
			{
				if (pax->m_ln.front()->sig.IsTimeSignal())
					pax->xtick.tics1 = pax->gengrids('x', -3);
				else
				{
					if (nDraws>2)
						pax->setxticks();
					else
					{
						for (int k = 0; k < nDraws; k++) 
						{
							double x1, y1;
							pax->GetCoordinate(&draw[k], x1, y1);
							pax->xtick.tics1.push_back((int)(x1+.1));
						}
					}
				}
			}
			if (pax->ytick.tics1.empty() && pax->ytick.automatic)
			{
				if (pax->m_ln.front()->sig.bufBlockSize==1)
				{
					vector<double> tics;
					tics.push_back(0); tics.push_back(1);
					pax->ytick.tics1 = tics;
				}
				else
					pax->ytick.tics1 = pax->gengrids('y');
			}
			pax->struts["x"].front()->strut["tick"] = (CSignals)CSignal(pax->xtick.tics1);
			DrawTicks(&dc, pax, 0);

			//x & y labels
			dc.SetTextAlign(TA_RIGHT|TA_BOTTOM);
			if (!gcf.ax.empty() && !gcf.ax.front()->m_ln.empty())
			{
				if (IsSpectrumAxis(pax))
				{
					dc.TextOut(pax->rct.right-3, pax->rct.bottom, "Hz");
					dc.TextOut(pax->rct.left-3, pax->rct.top+1, "dB");
				}
				else if (pax->m_ln.front()->sig.IsTimeSignal())
					dc.SetBkMode(TRANSPARENT), dc.TextOut(pax->rct.right-3, pax->rct.bottom, "sec");
			}
		}
		delete[] draw;
		if (pax->hChild)
		{
			paxready = true;
			pax = (CAxes*)pax->hChild;
			rctHist.clear();
		}
		else
			k++;
		//if (LRrange(&pax->rct, playLoc, 'x') == 0)
		//{
		//	dc.CreatePen(PS_SOLID, 1, RGB(204, 77, 0));
		//	dc.MoveTo(playLoc, pax->rct.bottom);
		//	dc.LineTo(playLoc, pax->rct.top);
		//}
		CAxes *pax0 = gcf.ax.front();
		if (pax0->m_ln.size()>0 || (gcf.ax.size()>1 && gcf.ax[1]->m_ln.size()>0) )
		{
			sbinfo.xBegin = pax0->xlim[0];
			sbinfo.xEnd = pax0->xlim[1];
		}
		if (temp<1)
		{
			sendtoEventLogger("(OnPaint) mtx_OnPaint unlocked.\n");
		}
	} // end of Drawing axes

	  //Drawing texts
	HFONT fontOriginal = (HFONT)dc.SelectObject(GetStockObject(SYSTEM_FONT));
	CSize sz;
	for (vector<CText*>::iterator txit=gcf.text.begin(); txit!=gcf.text.end(); txit++) { 
		if ((*txit)->visible && (*txit)->pos.x0>=0 && (*txit)->pos.y0>=0)
		{
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor((*txit)->color);
			DWORD dw = (*txit)->alignmode;
//			DWORD dw2 = TA_RIGHT | TA_TOP;
//			DWORD dw3 = TA_CENTER | TA_BASELINE;
			dc.SetTextAlign(dw);
			sz = dc.GetTextExtentPoint32((*txit)->str.c_str(), (int)(*txit)->str.length());
			HFONT ft = (HFONT)dc.SelectObject((*txit)->font);
			if (!(*txit)->posmode)
			{
				pt.x = (int)((double)clientRt.right * (*txit)->pos.x0 + .5);
				pt.y = (int)((double)clientRt.bottom * (1 - (*txit)->pos.y0) + .5);
				dc.TextOut(pt.x, pt.y, (*txit)->str.c_str(), (int)(*txit)->str.length());
				(*txit)->textRect = CRect(pt.x, pt.y - sz.cy, pt.x + sz.cx, pt.y);
			}
			else
			{
				GetWindowRect(&rt);
				(*txit)->pos.x0 = (double)(*txit)->textRect.left  / rt.Width();
				(*txit)->pos.y0 = 1. - (double)(*txit)->textRect.top / rt.Height();
				dc.TextOut((*txit)->textRect.left, (*txit)->textRect.top + sz.cy, (*txit)->str.c_str(), (int)(*txit)->str.length());
			}
		}
		else
		{
			::SendMessage(hTTscript, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti_script);
			::SendMessage(hTTscript, TTM_UPDATETIPTEXT, 0,  (LPARAM) (LPTOOLINFO) &ti_script);
		}
	}
	HFONT jj = (HFONT)dc.SelectObject(fontOriginal);
	if (!gcf.inScope)
	{
		dc.SetTextAlign(TA_LEFT | TA_TOP);
		HFONT hF = CreateFont(40, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "SYSTEM_FIXED_FONT");
		fontOriginal = (HFONT)dc.SelectObject(hF);
		BYTE r = GetRValue(gcf.ax.front()->color);
		BYTE g = GetGValue(gcf.ax.front()->color);
		BYTE b = GetBValue(gcf.ax.front()->color);
		dc.SetTextColor(RGB(255 - r, 255 - g, 255 - b));
		vector<string> tp;
		str2vect(tp, title, ':');
		dc.TextOut(pax->rct.left, pax->rct.top, tp[1].c_str());
	}
	EndPaint(hDlg, &ps);
	// vector gcf.ax is pushed in AddAxis() called in showvar.cpp 
	// find out why sometimes gcf.ax[k] is pointing something already deleted and causing a crash here. 8/216/2017 bjk
}

void CPlotDlg::DrawTicks(CDC *pDC, CAxes *pax, char xy)
{
	LOGFONT fnt;
	int loc;
	double value;
	int lastpt2Draw(-32767), cum(0);
	double nextpt;
	CSize sz;
	char label[256], tempformat[8];
	HDC hdc = HDC(*pDC);
	vector<double>::iterator it;
	CRect out;
	double step, scale, scalemant;
	strcpy(tempformat,"%d");
	switch(xy)
	{
	case 'x':
		pDC->SetTextAlign (TA_CENTER);
		//find the first point in tics1 in the range and update loc
		for (it=pax->xtick.tics1.begin(); it !=pax->xtick.tics1.end(); it++)
		{
			if (pax->m_ln[0]->sig.GetType()==CSIG_VECTOR && pax->m_ln[0]->xdata.nSamples==0) 
				loc = pax->double2pixel((int)(*it+.5), xy);
			else																	
				loc = pax->double2pixel(*it, xy);
			if (LRrange(&pax->rct, loc, xy)==0) break;
		}
		//iterate within the range
		for (; it !=pax->xtick.tics1.end() && LRrange(&pax->rct, loc, xy)==0 ; it++)
		{
			nextpt = *it;
			if (pax->m_ln[0]->sig.GetType()==CSIG_VECTOR && pax->m_ln[0]->xdata.nSamples == 0)
				loc = pax->double2pixel((int)(nextpt+.5), xy); 
			else											
				loc = pax->double2pixel(nextpt, xy); 
			if (LRrange(&pax->rct, loc, xy)>0) // if the current point is right side of axis
			{
				loc = pax->rct.right;
				value = pax->xtick.mult*pax->GetRangePixel(loc)+pax->xtick.offset;
			}
			else if (LRrange(&pax->rct, loc, xy)<0)  // if the current point is left side of axis, make it forcefully inside.
			{
				loc = pax->rct.left+1;
				value = pax->xtick.mult*pax->GetRangePixel(loc)+pax->xtick.offset;
				//further adjust tics1 according to the adjusted loc
			}
			else
				value = pax->xtick.mult*nextpt+pax->xtick.offset;
			loc = min(pax->rct.right-1, loc);  //loc should not be the same as pax->rct.right (then the ticks would protrude right from the axis rectangle)
			pDC->MoveTo(loc, pax->rct.bottom-1);
			pDC->LineTo(loc, pax->rct.bottom-1 - pax->xtick.size); 
			if (pax->m_ln.front()->sig.IsTimeSignal() && !pax->xtick.format)
				strcpy(pax->xtick.format, "%4.2f"); // This is where two digits under decimal are drawn on the time axis.
			if (pax->xtick.format[0]!=0)
			{
				if (value>=1. || pax->xtick.mult==1.)	sprintf(label, pax->xtick.format, value);
				else			sprintf(label, tempformat, (int)(value/pax->xtick.mult+.5)); // needed for Spectrum axis, when freq is below 1k.
			}
			else
				sprintfFloat(value, 3, label, 256);
			GetTextExtentPoint32(hdc, label, (int)strlen(label), &sz);
			if (iabs(loc-lastpt2Draw)> sz.cx + pax->xtick.gap4next.x) // only if there's enough gap, Textout
			{
				pDC->TextOut(loc, pax->rct.bottom + pax->xtick.labelPos, label);
				lastpt2Draw = loc;
			}
			cum++;
		}
		break;
	case 'y':
		pDC->SetTextAlign (TA_RIGHT);
		fnt = pDC->GetLogFont(); 
		if (pax->ytick.tics1.size() < 2)
			scalemant = 1;
		else
		{
			step = pax->ytick.tics1[1] - pax->ytick.tics1.front();
			scale = pow(10., ceil(log10(step)));
			scalemant = log10(scale);
		}
		//find the first point in tics1 in the range and update loc
		for (it=pax->ytick.tics1.begin(); it !=pax->ytick.tics1.end(); it++)
		{
			loc = pax->double2pixel(*it, xy);
			if (LRrange(&pax->rct, loc, xy)==0) break;
		}
		//iterate within the range
		for (; it !=pax->ytick.tics1.end() && LRrange(&pax->rct, loc, xy)==0 ; it++)
		{
			nextpt = *it;
			loc = min(pax->double2pixel(nextpt, xy), pax->rct.bottom-1); //loc should not be the same as pax->rct.bottom (then the ticks would protrude downward from the axis rectangle)
			if (LRrange(&pax->rct, loc, xy)>0) // if the current point is above of axis
				loc = pax->rct.top;
			else if (LRrange(&pax->rct, loc, xy)<0)  // if the current point is left side of axis, make it forcefully inside.
				loc = pax->rct.bottom-1;
			value = nextpt;
			pDC->MoveTo(pax->rct.left, loc);
			pDC->LineTo(pax->rct.left + pax->ytick.size, loc);
			if (pax->ytick.format[0]!=0)
				sprintf(label, pax->ytick.format, value);
			else
				sprintfFloat(value, max(0,min(3,1-(int)scalemant)), label, 256);
			GetTextExtentPoint32(hdc, label, (int)strlen(label), &sz);
			if (iabs(loc-lastpt2Draw)> sz.cy + pax->xtick.gap4next.y) // only if there's enough gap, Textout
			{
				pDC->TextOut(pax->rct.left - pax->ytick.labelPos, loc-fnt.lfHeight/2, label);
				lastpt2Draw = loc;
			}
			cum++;
		}		
		break;
	default:
		if (pax->m_ln.size()==0) break; // so that it skips when FFT rountine doesn't produce output
		pDC->CreatePen(PS_SOLID, 1, pax->colorAxis);
		DrawTicks(pDC, pax, 'x');
		DrawTicks(pDC, pax, 'y');
		break;
	}
}

CRect CPlotDlg::DrawAxis(CDC *pDC, PAINTSTRUCT *ps, CAxes *pax)
{
	CRect rt;
	GetClientRect(hDlg, &rt);
	unsigned int rr = GetDoubleClickTime();
	char buf1[64];
	if (axis_expanding)
		pDC->CreatePen(PS_DOT, 1, pax->colorAxis), pDC->CreateHatchBrush(HS_BDIAGONAL, RGB(160, 170, 200));
	else
		pDC->CreatePen(PS_SOLID, 1, pax->colorAxis), pDC->CreateSolidBrush(pax->color);
	CRect rt3(pax->pos.GetRect(rt.Width(), rt.Height()));
	pDC->Rectangle(rt3);
	pax->rct = rt3;
	SIZE sz (pDC->GetTextExtentPoint32 ("X", 5));
	int margin = 10;
	pax->ytick.rt = CRect( CPoint(rt3.left-sz.cx, rt3.top), CPoint(rt3.left,rt3.bottom));
	pax->xtick.rt = CRect( CPoint(rt3.left-margin,rt3.bottom), CSize(rt3.right-rt3.left+2*margin,pax->xtick.labelPos+sz.cy));
	if (axis_expanding)
	{
		strcpy(buf1,"Adjust Window Size.");
		pDC->TextOut(50, 5, buf1);
		strcpy(buf1,"Press F3 key again to revert to the normal mode.");
		pDC->TextOut(80, 20, buf1);
	}
	pDC->CreateFont(15, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, 
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
	return rt3;
}

void CPlotDlg::OnSize(UINT nType, int cx, int cy) 
{
	CRect rt;
	GetWindowRect(&rt);
	if (nType == WS_CHILD)
		::MoveWindow(hDlg, 0, 0, cx, cy, 1); // no need to worry about y pos and height
	else
	{
	//	if (title)
			::MoveWindow(sbar->hStatusbar, 0, 0, cx, 0, 1); // no need to worry about y pos and height
		if (curRange != NO_SELECTION && gcf.ax.size() > 0)
		{
			//need to adjust curRange according to the change of size 
			//new pixel pt after size change
			curRange.px1 = gcf.ax.front()->timepoint2pix(selRange.tp1);
			curRange.px2 = gcf.ax.front()->timepoint2pix(selRange.tp2);
		}
		if (hTTtimeax[0] == NULL)
		{
			for (int k = 0; k < 7; k++)
			{
				hTTtimeax[k] = CreateTT(sbar->hStatusbar, &ti_taxis);
				::SendMessage(sbar->hStatusbar, SB_GETRECT, k, (LPARAM)&ti_taxis.rect);
				ti_taxis.lpszText = (LPSTR)ttstat[k].c_str();
				::SendMessage(hTTtimeax[k], TTM_ACTIVATE, TRUE, 0);
				::SendMessage(hTTtimeax[k], TTM_SETMAXTIPWIDTH, 0, 400);
				::SendMessage(hTTtimeax[k], TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti_taxis);
				::SendMessage(hTTtimeax[k], TTM_UPDATETIPTEXT, 0, (LPARAM)(LPTOOLINFO)&ti_taxis);
			}
		}
		::SendMessage(hTTscript, TTM_ACTIVATE, TRUE, 0);
		for (vector<CAxes*>::iterator it = gcf.ax.begin(); it != gcf.ax.end(); it++)
		{
			if ((*it)->ytick.automatic) (*it)->ytick.tics1.clear();
			if ((*it)->xtick.automatic) (*it)->xtick.tics1.clear();
		}
		gcf.strut["pos"].buf[0] = rt.left;
		gcf.strut["pos"].buf[1] = rt.top;
		gcf.strut["pos"].buf[2] = rt.Width();
		gcf.strut["pos"].buf[3] = rt.Height();
		//For showvar.cpp--Update the data content view
		::SendMessage(GetHWND_WAVPLAY(), WM_APP + 0x2020, (WPARAM)&gcf, (LPARAM)"pos");
	}
	InvalidateRect(NULL);
}

void CPlotDlg::OnMove(int x, int y)
{
	CRect rt;
	GetWindowRect(&rt);
	gcf.strut["pos"].buf[0] = rt.left;
	gcf.strut["pos"].buf[1] = rt.top;
	gcf.strut["pos"].buf[2] = rt.Width();
	gcf.strut["pos"].buf[3] = rt.Height();

	//For showvar.cpp--Update the data content view
	::SendMessage(GetHWND_WAVPLAY(), WM_APP + 0x2020, (WPARAM)&gcf, (LPARAM)"pos");
}

//Convention: if a figure handle has two axes, the first axis is the waveform viewer, the second one is for spectrum viewing.
void CPlotDlg::OnRButtonUp(UINT nFlags, CPoint point) 
{
	if (gcf.ax.empty()) return;
	CAxes *pax = CurrentPoint2CurrentAxis(&point);
	if (pax!=NULL)
	{
		int iSel(-1);
		for (size_t i=0; i<gcf.ax.size(); i++) if (pax==gcf.ax[i]) iSel=(int)i;
		//Following the convention
		subMenu.EnableMenuItem(IDM_PLAY, iSel==0? MF_ENABLED : MF_GRAYED);
		subMenu.EnableMenuItem(IDM_WAVWRITE, iSel==0? MF_ENABLED : MF_GRAYED);
		subMenu.EnableMenuItem(IDM_SPECTRUM, iSel==0? MF_ENABLED : MF_GRAYED);
		ClientToScreen(hDlg, &point);    // To convert point to re: the whole screen 
		TrackPopupMenu(subMenu.GetHandle(), TPM_RIGHTBUTTON, point.x, point.y, 0, hDlg, 0);
	}
}

void CPlotDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
}

void CPlotDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
//	char buf[256];
	//sprintf(buf, "OnLButtonDown pt.x=%d, rect.y=%d\n", point.x, point.y);
	//sendtoEventLogger(buf);
	mst.clickedOn = true;
	mst.curPt = mst.last_clickPt = point;
	edge.px1 = edge.px2 = -1;
	gcmp=point;
	CAxes *cax = CurrentPoint2CurrentAxis(&point);
	mst.curAx = cax;
	CSignals temp;
	if (axis_expanding) {ClickOn=0; return;}
	clickedPt = point;
	switch(ClickOn = GetMousePos(point)) // ClickOn indicates where mouse was click.
	{
	case RC_WAVEFORM:  //0x000f
		if (curRange != NO_SELECTION) // if there's previous selection
		{
			sbar->showXSEL(-1);
			for (size_t k=0; k<gcf.ax.size(); k++)
			{
				CRect rt(curRange.px1, gcf.ax[k]->rct.top, curRange.px2+1, gcf.ax[k]->rct.bottom+1);
				InvalidateRect(&rt);
			}
		}
		lbuttondownpoint.x = gcmp.x;
		curRange.reset();
		//sprintf(buf, "ButtonDown pt(%d,%d)\n", point.x, point.y);
		//sendtoEventLogger(buf);

		//temp.ghost = true;
		//GetGhost(&temp, cax, false);
		//if (temp.IsTimeSignal())
		//{
		//	if (playCursor > 0) // if a playCursor was set, then reset here
		//	{
		//		for (size_t k = 0; k < gcf.ax.size(); k++)
		//		{
		//			CRect rt(playCursor - 1, gcf.ax[k]->rct.top, playCursor + 1, gcf.ax[k]->rct.bottom + 1);
		//			InvalidateRect(&rt);
		//		}
		//		playCursor = -1;
		//	}
		//	SetTimer(MOUSE_CURSOR_SETTING, 1000);
		//}
		break;
	default:
		gcmp=CPoint(-1,-1);
		break;
	}
}

void CPlotDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	//	char buf[256];
		//sprintf(buf, "OnLButtonUp pt.x=%d, rect.y=%d\n", point.x, point.y);
		//sendtoEventLogger(buf);
	CRect rt;
	mst.clickedOn = false;
	mst.curPt = point;
	if (axis_expanding | gcf.ax.empty()) { ClickOn = 0; return; }
	// if mst.curAx is one of the FFTAx
	for (auto _ax : sbar->ax)
	{
		if (_ax->hChild == mst.curAx)
		{
			ChangeColorSpecAx(mst.curAx, MoveOn = (bool)0);
			InvalidateRect(mst.curAx->rct);
			mst.curAx = NULL;
			ClickOn = 0;
			gcmp = CPoint(-1, -1);
			mst.last_MovPt = CPoint(-1, -1);
			return;
		}
	}

	CAxes *cax = CurrentPoint2CurrentAxis(&mst.last_clickPt);
	CAxes *pax = gcf.ax.front();
	if (curRange.px2 < 0) // if button up without mouse moving, reset
		curRange.reset();
	CSignals _sig;
	clickedPt.x = -999; clickedPt.y = -999;
	if (cax && curRange.px1 > 0 && curRange.px2 > 0)
	{
		switch (ClickOn)
		{
		case RC_WAVEFORM:  //0x000f
			if (point.x > pax->rct.right || point.x < pax->rct.left) // mouse was up outside axis
				cax = pax;
			rt.top = cax->rct.top;
			rt.bottom = cax->rct.bottom + 1;
			rt.left = point.x - 1; //default
			rt.right = point.x + 1; //default
			if (point.x > cax->rct.right)
			{
				rt.left = curRange.px2 - 1;
				rt.right = curRange.px2 = cax->rct.right;
			}
			else if (point.x < cax->rct.left)
			{
				rt.right = curRange.px1 + 1;
				rt.left = curRange.px1 = cax->rct.left + 1;
			}
			else
			{
				if (point.x > lbuttondownpoint.x) // moving right
				{
					curRange.px2 = point.x;
					if (edge.px1 > curRange.px2) { //moved right and left
						rt.left = curRange.px2;
						rt.right = edge.px1 + 2;
					}
				}
				else
				{
					curRange.px1 = point.x;
					if (edge.px1 > 0 && edge.px1 < curRange.px1) { //moved left and right
						rt.left = edge.px1 - 2;
						rt.right = curRange.px1;
					}
				}
			}
			if (curRange.px2 - curRange.px1 <= 3) curRange.reset();
			InvalidateRect(&rt);
			sbar->dBRMS();
			selRange.tp1 = cax->pix2timepoint(curRange.px1);
			selRange.tp2 = cax->pix2timepoint(curRange.px2);
			if (ClickOn && gcf.ax.front()->hChild)
				OnMenu(IDM_SPECTRUM_INTERNAL);
			break;
		default:
			unsigned short mask = ClickOn & 0xff00;
			if (mask == RC_SPECTRUM)
			{
				ChangeColorSpecAx(cax, MoveOn = (bool)0);
				InvalidateRect(NULL);
			}
			break;
		}
	}
	mst.curAx = NULL;
	ClickOn = 0;
	gcmp=CPoint(-1,-1);
	lbuttondownpoint = CPoint(-1, -1);
}

void CPlotDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	mst.curPt = point;

	static int count(0);
	if (axis_expanding || gcf.ax.empty()) return;
	sbar->showCursor(point);

	// if point is outside of an axes, clear all mst related members

	// if mst.curAx is one of the FFTAx
	if (!mst.clickedOn) return;
	CPoint shift;
	CAxes *cax;
	for (auto _ax : sbar->ax)
	{
		//If FFTAx was clicked and mouse is moving while clicked.
		if (_ax->hChild && _ax->hChild == mst.curAx)
		{
			ChangeColorSpecAx(mst.curAx, true);
			CRect rect2Invalidate = mst.curAx->GetWholeRect();
			rect2Invalidate.top -= 15; // to include the rea of y axis label "dB"
			if (mst.last_MovPt.x==-1 && mst.last_MovPt.y==-1)
				shift = point - mst.last_clickPt;
			else
				shift = point - mst.last_MovPt;
			if (shift.x != 0 || shift.y != 0)
			{
				CRect clientrt;
				GetClientRect(hDlg, &clientrt);
				mst.curAx->rct.MoveToXY(mst.curAx->rct.TopLeft() + shift); // new top left point is shifted
				mst.curAx->pos.Set(clientrt, mst.curAx->rct);
				if (shift.x > 0)
					rect2Invalidate.right += shift.x;
				else
					rect2Invalidate.left += shift.x;
				if (shift.y > 0)
					rect2Invalidate.bottom += shift.y;
				else
					rect2Invalidate.top += shift.y;

				InvalidateRect(rect2Invalidate);
			}
			mst.last_MovPt = point;
		}
		else
		{
			// if the first point clicked is outside the axes, skip
			cax = _ax;
			char buffer[256];
			sprintf(buffer, "OnMouseMove cur(%d,%d), bdp.x=%d ClickOn=%d\n", point.x, point.y, lbuttondownpoint.x, ClickOn);
			sendtoEventLogger(buffer);
			if (lbuttondownpoint.x == -1 && !cax->rct.PtInRect(point))
				continue;
			if (ClickOn & (unsigned short)128)
				lbuttondownpoint.x = cax->rct.left;
			else if (ClickOn & (unsigned short)32)
				lbuttondownpoint.x = cax->rct.right;
			// if clicked point was outside axes but current point goes inside,
			// fake the clicked point as the edge of axes
			if (ClickOn == 0 && cax->rct.PtInRect(point))
			{ // approaching from the right side
				ClickOn = 15;
				lbuttondownpoint.x = cax->rct.right;
			}
			else if (ClickOn == 1 && cax->rct.PtInRect(point))
			{ // approaching from the left side
				ClickOn = 15;
				lbuttondownpoint.x = cax->rct.left;
			}
			// end of faking...
			if (ClickOn)
			{ // lbuttondownpoint.x is the x point when mouse was clicked
				CRect rect2Invalidate;
				rect2Invalidate.top = cax->rct.top + 1;
				rect2Invalidate.bottom = cax->rct.bottom - 1;
				if (point.x > lbuttondownpoint.x) // current position right side of the beginning point
				{
					if (edge.px1 == -1) edge.px1 = lbuttondownpoint.x;
					curRange.px1 = lbuttondownpoint.x;
					curRange.px2 = min(point.x, cax->rct.right);
					edge.px2 = max(lastpoint.x, max(edge.px2, point.x));
					rect2Invalidate.right = edge.px2;

					// If move left-right and passed the beginning point,i.w., lbuttondownpoint, keep the edge.px1 in rt, so it can be properly redrawn
					if ((point.x - lbuttondownpoint.x)*(point.x - edge.px1) > 0)
						rect2Invalidate.left = edge.px1 - 1;
					else
						rect2Invalidate.left = curRange.px1;
					edge.px1 = max(edge.px1, curRange.px2);
				}
				else if (point.x < lbuttondownpoint.x) // moving left
				{
					if (edge.px1 == -1) edge.px1 = lbuttondownpoint.x;
					curRange.px2 = lbuttondownpoint.x;
					curRange.px1 = max(point.x, cax->rct.left);
					if (point.x > lastpoint.x) // moving left but just turned right 
						rect2Invalidate.left = lastpoint.x;
					else
						rect2Invalidate.left = curRange.px1;
					// If move right-left and passed the beginning point, i.e., lbuttondownpoint), keep the edge.px1 in rt, so it can be properly redrawn
					if ((point.x - lbuttondownpoint.x)*(point.x - edge.px1) > 0)
						rect2Invalidate.right = edge.px1 + 1;
					else
						rect2Invalidate.right = curRange.px2;
					edge.px1 = min(edge.px1, curRange.px1);
				}
				else
					curRange.reset();
				CSignals tp(1);
				if (curRange != NO_SELECTION)
				{
					InvalidateRect(&rect2Invalidate);
					tp.UpdateBuffer(2);
					tp.buf[0] = selRange.tp1 = cax->pix2timepoint(curRange.px1);
					tp.buf[1] = selRange.tp2 = cax->pix2timepoint(curRange.px2);
					gcf.strut["sel"] = tp;
					for (auto ax : gcf.ax)
						ax->strut["sel"] = tp;
//					sprintf(buffer, "OnMouseMove cur(%d,%d), bdp.x=%d, edge(%d,%d), rect.left=%d, rect.right=%d\n", point.x, point.y, lbuttondownpoint.x, edge.px1, edge.px2, rect2Invalidate.left, rect2Invalidate.right);
//					sendtoEventLogger(buffer);
					sbar->showXSEL(1);
				}
				lastpoint = point;
			}
			else edge.px1 = -1;
		}
	}
}

void CSBAR::showXLIM(CAxes *pax, CPoint point)
{
	if (!cax)
	{
		if (ax.empty())			return;
		else cax = ax.front();
	}
	if (!pax) pax = cax;
	if (find(ax.begin(), ax.end(), cax)==ax.end()) return;
	char buf[256], buf2[64];
	sprintf(buf, "%.3f", pax->xlim[0]);
	sprintf(buf2, "%.3f", pax->xlim[1]);
	//	sprintf(buf3, "%.3f", sbinfo.xCur);
	if (pax->xTimeScale) // for now, assume that two axes have both same sig type (check only the last one)
	{
		strcat(buf, "s"); strcat(buf2, "s");
	}
	::SendMessage(hStatusbar, SB_SETTEXT, 0, (LPARAM)buf);
	::SendMessage(hStatusbar, SB_SETTEXT, 1, (LPARAM)buf2);
	// Is dB RMS displayed?
	::SendMessage(hStatusbar, SB_GETTEXT, 6, (LPARAM)buf);
	if (strlen(buf) == 0)
		dBRMS();
}
void CSBAR::showXSEL(int ch)
{
	if (ch==-1)
	{
		::SendMessage(hStatusbar, SB_SETTEXT, 4, (LPARAM)"");
		::SendMessage(hStatusbar, SB_SETTEXT, 5, (LPARAM)"");
		return;
	}
	char buf[64], buf2[64];
	const double xSelBegin = base->gcf.ax.back()->pix2timepoint(base->curRange.px1);
	const double xSelEnd = base->gcf.ax.back()->pix2timepoint(base->curRange.px2);
	if (base->curRange == NO_SELECTION)
	{
		buf[0] = 0; buf2[0] = 0;
	}
	else
	{ // for now, use the last one---assume that two axes have both same sig type 
		sprintf(buf, "%.3f", xSelBegin);
		sprintf(buf2, "%.3f", xSelEnd);
		if (cax->xTimeScale) // for now, assume that two axes have both same sig type (check only the last one)
		{
			strcat(buf, "s"); strcat(buf2, "s");
		}
	}
	if (base->playCursor < 0) // if play cursor is set, do not update selection range status bar
	{
		::SendMessage(hStatusbar, SB_SETTEXT, 4, (LPARAM)buf);
		::SendMessage(hStatusbar, SB_SETTEXT, 5, (LPARAM)buf2);
	}
}

void CSBAR::showCursor(CPoint point)
{
	CAxes *pax = base->CurrentPoint2CurrentAxis(&point);
	if (pax)
	{
		cax = pax;
		double xCur, yCur, xSelBegin, xSelEnd;
		pax->GetCoordinate(point, xCur, yCur);
		xSelBegin = base->gcf.ax.back()->pix2timepoint(base->curRange.px1);
		xSelEnd = base->gcf.ax.back()->pix2timepoint(base->curRange.px2);
		if (pax->m_ln.size() > 0)
		{
			char buf[64];
			if (pax->m_ln[0]->sig.IsTimeSignal())
				sprintf(buf, "(%.3fs,%.3f)", xCur, yCur);
			else
				sprintf(buf, "(%.3f,%.3f)", xCur, yCur);
			::SendMessage(hStatusbar, SB_SETTEXT, 2, (LPARAM)buf);
			showXLIM(pax, point);
		}
	}
	else // if cursor is outside an axes, the current cursor info is removed.
	{
		base->KillTimer(CLEAR_CUR_MOUSE_POS);
		base->SetTimer(CLEAR_CUR_MOUSE_POS, 2000, NULL);
	}
}

void CSBAR::dBRMS(SHOWSTATUS st)
{
	//st is no longer used, but keep it for the future 10/4/2019
	CSignals gcopy, res;
	char buf0[32] = {}, buf[256] = {};
	if (ax.empty() || ax.front()->m_ln.empty()) return;
	if (!ax.front()->m_ln.front()->sig.IsAudio()) return;
	sprintf(buf, "[dBRMS] ");
	base->GetGhost(gcopy);
	res = gcopy.RMS(); // Remember, gcopy loses the previous data after this call.
	sprintf(buf0, "%.1f ", res.value());
	strcat(buf, buf0);
	if (res.next)
	{
		sprintf(buf0, "| %.1f ", res.next->value());
		strcat(buf, buf0);
	}
	::SendMessage(hStatusbar, SB_SETTEXT, 6, (LPARAM)buf);
}

#define LOG(X) fprintf(fpp,(X));

void CPlotDlg::setpbprogln()
{
	for (auto ax : gcf.ax)
	{
		if (playLoc0 > ax->rct.right) return;
		if (playLoc > ax->rct.right) playLoc = ax->rct.right;
		InvalidateRect(CRect(playLoc0-1 , ax->rct.top+1, playLoc0+1 , ax->rct.bottom+1), 0);
	//	InvalidateRect(CRect(playLoc-1 , ax->rct.top, playLoc+1 , ax->rct.bottom), 0);
	}
}

void CPlotDlg::OnSoundEvent(CVar *pvar, int code)
{//index is the number sent by CWavePlay as each playback block of data is played successfully
 //bufferlocation is current point location of the data block (not really useful on the application side)
 // Both arguments 0 means the playback device is opened and ready to go.
 // index=-1 means playback ended and device is being closed.

	CAxes *pax = gcf.ax.front();
	double tp = pax->pix2timepoint(2);
	switch (code)
	{
	case WOM_OPEN:
		playingIndex = 0;
		playLoc0 = playLoc = (playCursor>1) ? playCursor : (curRange == NO_SELECTION)? pax->rct.left : curRange.px1;
		setpbprogln();
		::SendMessage(GetHWND_WAVPLAY(), WM__AUDIOEVENT, (WPARAM)pvar, (LPARAM)code); // sending to the dialog in showvar.cpp
		break;
	case WOM_CLOSE:
		playLoc = playLoc0 + 3;
		setpbprogln();
		playLoc0 = playLoc = -1;
		playing = paused = false;
		::SendMessage(GetHWND_WAVPLAY(), WM__AUDIOEVENT, (WPARAM)pvar, (LPARAM)code); // sending to the dialog in showvar.cpp
		break;
	case -1: // error
		break;
	default: // status updates
	{
		//Not used for now, but leaving just for the future use. 2/22/2019
//		double dara, gara;
//		gara = pvar->strut["durLeft"].value();
//		dara = pvar->strut["durTotal"].value();
	}
		playingIndex++;
		if (playCursor > 0)
			playLoc = playCursor + pax->timepoint2pix(block*playingIndex / 1000);
		else if (curRange == NO_SELECTION)
		{
			for (auto _ax : gcf.ax)
			{
				HDC hdc = GetDC(hDlg);
				HPEN hp = CreatePen(PS_SOLID, 1, RGB(255, 131, 80));
				SelectObject(hdc, hp);
				playLoc = _ax->timepoint2pix(pax->xlim[0] + block * playingIndex / 1000);
				if (playLoc >= _ax->rct.right)
				{
					playLoc = min(playLoc, _ax->rct.right - 1);
	//				InvalidateRect(_ax->rct, 0);
				}
				MoveToEx(hdc, playLoc, _ax->rct.bottom - 1, NULL);
				LineTo(hdc, playLoc, _ax->rct.top);
			}
		}
		else
		{
			for (auto _ax : gcf.ax)
			{
				HDC hdc = GetDC(hDlg);
				HPEN hp = CreatePen(PS_SOLID, 1, RGB(255, 131, 80));
				SelectObject(hdc, hp);
				playLoc = _ax->timepoint2pix(selRange.tp1 + block * playingIndex / 1000);
				// if playLoc goes out of range, invalidateRect only to erase playLoc0, i.e., don't update the screen with the newly advanced but out-of-range bar
//				char buf[256];
				//sprintf(buf, "index=%d, playCursor=%d, playLoc=%d\n", playingIndex, playCursor, playLoc);
				//sendtoEventLogger(buf);
				if (playLoc > curRange.px2)
				{
					InvalidateRect(CRect(playLoc0 - 1, _ax->rct.top+5, playLoc0 + 1, _ax->rct.bottom+5), 0);
					playLoc0 = playLoc = -1;
					playing = paused = false;
					::SendMessage(GetHWND_WAVPLAY(), WM_APP + 0x2300, 0, -1);
					return;
				}
				MoveToEx(hdc, playLoc, _ax->rct.bottom - 10, NULL);
				LineTo(hdc, playLoc, _ax->rct.top);
			}
		}
		setpbprogln();
		playLoc0 = playLoc;
		//char buf[256];
		//sprintf(buf, "index=%d\n", playingIndex);
		//sendtoEventLogger(buf);
	}
}

void CPlotDlg::ChangeColorSpecAx(CAxes *cax, bool onoff)
{// on: ready to move, off: movind done
	if (!cax) return;
	if (onoff)
	{
		BYTE r = GetRValue(cax->ColorFFTAx);
		BYTE g = GetGValue(cax->ColorFFTAx);
		BYTE b = GetBValue(cax->ColorFFTAx);
		cax->color = RGB(r * 5 / 8, g * 5 / 8, b * 5 / 8);
	}
	else
		cax->color = cax->ColorFFTAx;
//	InvalidateRect(&rt);
}

void CPlotDlg::OnTimer(UINT id)
{
	switch (id)
	{
	case FIRST_DISPLAY_STATUSBAR:
		KillTimer(id);
		sbar->showXLIM();
		break;
	case MOUSE_CURSOR_SETTING:
		KillTimer(id);
		{
			CAxes *cax = CurrentPoint2CurrentAxis(&gcmp);
			CRect rt(cax->rct);
			rt.left = rt.right = playCursor = gcmp.x;
			rt.left--; rt.right++;
			InvalidateRect(&rt);
			char buf[16];
			sprintf(buf, "%.3fs", cax->pix2timepoint(playCursor));
			::SendMessage(sbar->hStatusbar, SB_SETTEXT, 4, (LPARAM)buf);
		}
		break;

	case CLEAR_CUR_MOUSE_POS:
		::SendMessage (sbar->hStatusbar, SB_SETTEXT, 2, (LPARAM)"");
		KillTimer(id);
		break;
	case MOVE_SPECAX:
		if (ClickOn & RC_SPECTRUM)
		{
			CAxes *ax= (CAxes*)gcf.ax.front()->hChild;
			BYTE r = GetRValue(ax->color);
			BYTE g = GetGValue(ax->color);
			BYTE b = GetBValue(ax->color);
			ax->color = RGB(g,b,r);
			CRect rt(ax->rct.left, ax->rct.top, ax->rct.right, ax->rct.bottom);
			InvalidateRect(&rt);
		}
		KillTimer(id);
		break;
	}
}

int CPlotDlg::IsSpectrumAxis(CAxes* pax)
{ // return true when xtick.tics1 is empty or its full x axis right edge.px1 is half the sample rate of the front signal
	if (pax == gcf.ax.front())	return 0;
	int fs = gcf.ax.front()->m_ln.front()->sig.GetFs();
	if ( abs((int)(pax->xlimFull[1] - fs/2)) <= 2 ) return 1;
	else return 0;
}

void CPlotDlg::WindowSizeAdjusting()
{
	char buf1[256], buf2[256], buf3[256];
	if (ClickOn) ClickOn = 0, OnLButtonUp(0, CPoint(-1, -1));
	if (playing) OnMenu(IDM_STOP);
	if (axis_expanding)
		strcpy(buf1, ""), strcpy(buf2, ""), strcpy(buf3, "");
	else
		strcpy(buf1, "Adjust"), strcpy(buf2, "Window Size with"), strcpy(buf3, "Mouse");
	axis_expanding = !axis_expanding;
	::SendMessage(sbar->hStatusbar, SB_SETTEXT, 7, (LPARAM)buf1);
	::SendMessage(sbar->hStatusbar, SB_SETTEXT, 8, (LPARAM)buf2);
	::SendMessage(sbar->hStatusbar, SB_SETTEXT, 10, (LPARAM)buf3);
	InvalidateRect(NULL);
}

int CPlotDlg::ViewSpectrum()
{
	CAxes *paxFFT;
	CPosition SpecAxPos(.75, .6, .22, .35);
	if (gcf.ax.size() > 1) SpecAxPos.height -= .02;
	int nOuts = 0;
	for (auto Ax : gcf.ax)
	{
		if (Ax->m_ln.size() == 0) continue;
		if (!Ax->m_ln.front()->sig.IsTimeSignal()) break;
		if (Ax->hChild)
		{ // deleting the fft window
			CRect rt2;
			paxFFT = (CAxes*)Ax->hChild;
			rt2 = paxFFT->rct;
			rt2.InflateRect(40, 30);
			InvalidateRect(rt2);
			delete paxFFT; // do not call deleteObj(paxFFT); --> that is to clean up vectors and assumes CAxes object is a child (vector) of CFigure
			auto it = find(Ax->struts["children"].begin(), Ax->struts["children"].end(), paxFFT);
			if (it != Ax->struts["children"].end())
				Ax->struts["children"].erase(it);
			Ax->struts["gca"].clear();
			Ax->hChild = NULL;
		}
		else
		{ // adding the fft window
			if (Ax != gcf.ax.front()) //if second channel (stereo)
				SpecAxPos.y0 = .1;
			Ax->hChild = paxFFT = Ax->create_child_axis(SpecAxPos);
			paxFFT->visible = true;
			ShowSpectrum(paxFFT, Ax);
			nOuts++;
		}
	}
	return nOuts;
}

void CPlotDlg::OnMenu(UINT nID)
{
	CAxes* paxFFT;
	char fullfname[MAX_PATH];
	char fname[MAX_PATH];
	CFileDlg fileDlg;
	CAxes *cax=NULL;
	if (!gcf.ax.empty())
		cax=gcf.ax.front(); // Following the  ??
	CRect rt;
	CSize sz;
	CFont editFont;
	CPosition pos;
	int  len, iMul(1);
	char errstr[256];
	CSignals _sig;
	CSignal dummy;
	char buf[64];
	int k(0), deltapixel;
	vector<CLine*> swappy;
	double shift, newlimit1,  newlimit2, deltaxlim, dval, _block;
	double xlim[2];
	INT_PTR res;

	errstr[0]=0;
	switch (nID)
	{
	case IDM_ADJUSTWINDOWSIZE:
		WindowSizeAdjusting();
		break;
	case IDM_TRANSPARENCY:
		dval = (double)opacity/255.*100.;
		sprintf(buf,"%d",(int)(dval+.5));
		res = InputBox("Transparency", "transparent(0)--opaque(100)", buf, sizeof(buf));
		if (res==1 && strlen(buf)>0)
		{
			if (res=sscanf(buf,"%lf", &dval))
			{
				if (dval<=100. && dval>=0.)
				{
					opacity = (unsigned char)(dval/100.*255-.5);
					SetLayeredWindowAttributes(hDlg, 0, opacity, LWA_ALPHA); // how can I transport transparency from application? Let's think about it tomorrow 1/6/2017 12:19am
				}
			}
		}
		break;

	case IDM_FULLVIEW:
		zoom=0;
		xlim[0] = 1.e100; xlim[1] = -1.e100;
		for (auto pax : gcf.ax)
		{
			xlim[0] = min(xlim[0], pax->xlimFull[0]);
			xlim[1] = max(xlim[1], pax->xlimFull[1]);
		}
		for (auto pax : gcf.ax)
		{
			memcpy(pax->xlim, xlim, sizeof(double) * 2);
			memcpy(pax->xlimFull, xlim, sizeof(double)*2);
			pax->xtick.tics1.clear(); pax->xtick.automatic = true;
			pax->ytick.tics1.clear(); pax->ytick.automatic = true;
			pax->struts["x"].front()->strut["auto"] = CSignals(true);
			pax->struts["y"].front()->strut["auto"] = CSignals(true);
			if (paxFFT = (CAxes*)pax->hChild) ShowSpectrum(paxFFT, pax);
			pax->struts["x"].front()->strut["lim"] = (CSignals)CSignal(pax->xlim, 2);
			rt = pax->rct;
			rt.InflateRect(10, 10, 10, 30);
			InvalidateRect(&rt, FALSE);
		}
		sbar->showXLIM();
		sbar->dBRMS();
		return;

	case IDM_ZOOM_IN:
		if (cax->xlim[1] - cax->xlim[0]<0.009) 
			return; // no zoom for 5ms of less
	case IDM_ZOOM_OUT:
		for (auto pax : gcf.ax)
		{
			if (nID == IDM_ZOOM_OUT && fabs(pax->xlim[1]-pax->xlimFull[1])<1.e-5 && fabs(pax->xlim[0]-pax->xlimFull[0])<1.e-5)
				return; // no unzoom if full length
			if (nID == IDM_ZOOM_IN && pax->m_ln.front()->sig.GetType()==CSIG_VECTOR)
			{
				double percentShown = 1. - ( (pax->xlim[0]-pax->xlimFull[0]) + (pax->xlimFull[1]-pax->xlim[1]) ) / (pax->xlimFull[1]-pax->xlimFull[0]);
				if ((len = (int)(percentShown * pax->m_ln.front()->sig.nSamples+.5))<=3)
					return; // no unzoom if full length
			}
			deltaxlim = pax->xlim[1]-pax->xlim[0];
			if (nID == IDM_ZOOM_IN)
				pax->xlim[0] += deltaxlim/4, 	pax->xlim[1] -= deltaxlim/4;
			else
			{
				pax->xlim[0] -= deltaxlim/2;
				pax->xlim[0] = MAX(pax->xlimFull[0], pax->xlim[0]);
				pax->xlim[1] += deltaxlim/2;
				pax->xlim[1] = MIN(pax->xlim[1], pax->xlimFull[1]);
			}
			if (!pax->m_ln.size()) break;
			pax->xtick.tics1.clear(); pax->xtick.automatic = true;
			pax->ytick.tics1.clear(); pax->ytick.automatic = true;
			pax->struts["x"].front()->strut["auto"] = CSignals(true);
			pax->struts["y"].front()->strut["auto"] = CSignals(true);
			if (paxFFT = (CAxes*)pax->hChild) ShowSpectrum(paxFFT, pax);
			pax->struts["x"].front()->strut["lim"] = (CSignals)CSignal(pax->xlim, 2);
			rt = pax->rct;
			rt.InflateRect(10, 10, 10, 30);
			InvalidateRect(&rt, FALSE);
		}
		sbar->showXLIM();
		sbar->dBRMS();
		return;
	case IDM_LEFT_STEP:
		iMul *= -1;
	case IDM_RIGHT_STEP:
		for(auto pax : gcf.ax)
		{
			shift = (pax->xlim[1]-pax->xlim[0]) / 4;
			newlimit1 = pax->xlim[0] + shift*iMul; // only effective for IDM_LEFT_STEP
			if (newlimit1<pax->xlimFull[0]) 
				shift = pax->xlim[0] - pax->xlimFull[0];
			newlimit2 = pax->xlim[1] + shift*iMul; // only effective for IDM_RIGHT_STEP
			if (newlimit2>pax->xlimFull[1]) 
				shift = fabs(pax->xlim[1] - pax->xlimFull[1]);
			if (shift<0.001) return;
			if (pax->xtick.tics1.size() == 1)
			{ // if there's only one tick, make sure it has at least two, so that step valid in line+25
				double newxlim[2];
				memcpy(newxlim, pax->xlim, sizeof(newxlim));
				newxlim[0] += shift * iMul;
				newxlim[1] += shift * iMul;
				if (nID == IDM_RIGHT_STEP)
				{
					if (newxlim[1] - pax->xtick.tics1.front() < 1) return;
					pax->xtick.tics1.push_back(pax->xtick.tics1.front() + 1);
				}
				if (nID == IDM_LEFT_STEP)
				{
					if (pax->xtick.tics1.front() - newxlim[0] < 1) return;
					pax->xtick.tics1.insert(pax->xtick.tics1.begin(), pax->xtick.tics1.front() - 1);
				}
			}
			pax->xlim[0] += shift*iMul;
			pax->xlim[1] += shift*iMul;
			// further adjusting lim[0] to xlimFull[0] (i.e., 0 for audio signals) is necessary to avoid in gengrids when re-generating xticks
			if ((pax->xlim[0]-pax->xlimFull[0])<1.e-6) 	pax->xlim[0] = pax->xlimFull[0];
			if ((pax->xlimFull[1]-pax->xlim[1])<1.e-6) 	pax->xlim[1] = pax->xlimFull[1]; // this one may not be necessary.
			//Assuming that the range is determined by the first line
			vector<double>::iterator it = pax->xtick.tics1.begin();
			double p, step = *(it + 1) - *it;
			pax->xtick.automatic = true;
			pax->struts["x"].front()->strut["auto"] = CSignals(true);
			pax->struts["y"].front()->strut["auto"] = CSignals(true);
			// tick step size is not changed during left/right stepping
			if (nID == IDM_RIGHT_STEP)
			{
				p = pax->xtick.tics1.front();
				pax->xtick.tics1.clear();
				for (; p - pax->xlim[1] < 1.e-6; p += step)
					if (p >= pax->xlim[0])	pax->xtick.tics1.push_back(p);
			}
			else
			{
				p = pax->xtick.tics1.back();
				pax->xtick.tics1.clear();
				vector<double> tp;
				for (; p- pax->xlim[0] > -1.e-6 ; p -= step)
					if (p <= pax->xlim[1])	tp.push_back(p);
				for (vector<double>::reverse_iterator rit = tp.rbegin(); rit != tp.rend(); rit++)
					pax->xtick.tics1.push_back(*rit);
			}
			pax->struts["x"].front()->strut["tick"] = (CSignals)CSignal(pax->xtick.tics1);
			sbinfo.xBegin = pax->xlim[0];
			sbinfo.xEnd = pax->xlim[1];
			rt = pax->rct;
			rt.InflateRect(10, 10, 10, 30);
			InvalidateRect(&rt, FALSE);
			if (paxFFT = (CAxes*)pax->hChild) ShowSpectrum(paxFFT, pax);
			pax->struts["x"].front()->strut["lim"] = (CSignals)CSignal(pax->xlim, 2);
		}
		sbar->showXLIM();
		sbar->dBRMS();
		return;

	case IDM_ZOOMSELECT:
		if (!cax->m_ln.size()) break;
		if (curRange != NO_SELECTION)
		{ // need jst, because the moment you update cax->xlim[], you can't call pix2timepoint it any more.
			double jst = cax->pix2timepoint(curRange.px1);
			cax->xlim[1] = cax->pix2timepoint(curRange.px2);
			cax->xlim[0] = jst;
			cax->xtick.tics1.clear();
			cax->ytick.tics1.clear();
			InvalidateRect(NULL);
			if (gcf.ax.front()->hChild) OnMenu(IDM_SPECTRUM_INTERNAL); // do this later.
			cax->struts["x"].front()->strut["lim"] = (CSignals)CSignal(cax->xlim, 2);
		}
		curRange.reset();
		::SendMessage (sbar->hStatusbar, SB_SETTEXT, 4, (LPARAM)"");
		::SendMessage (sbar->hStatusbar, SB_SETTEXT, 5, (LPARAM)"");
		return;

	case IDM_PLAY:
#ifndef NO_PLAYSND
		//DO this 7/11/2018----play pause play pause--then it doesn't pause but playing overlap instead...
		if (!playing)
		{
			if (!paused)
			{
				if (curRange == NO_SELECTION)
				{
					if (playCursor < 0)
					{
						deltapixel = cax->rct.right - cax->rct.left;
						deltaxlim = cax->xlim[1] - cax->xlim[0];
					}
					else
					{
						deltapixel = cax->rct.right - playCursor;
						deltaxlim = cax->xlim[1] - cax->xlim[0]; // do this...
					}
				}
				else
				{
					deltapixel = curRange.px2 - curRange.px1;
					deltaxlim = selRange.tp2 - selRange.tp1; 
				}
				_block = deltaxlim * PBPROG_ADVANCE_PIXELS / deltapixel * 1000.;
				// Below, if this put too low maximum, the progress line may move smoothly, but the playback sound will be choppy.
				block = max(block, _block);
				GetGhost(_sig);
				if (!_sig.IsEmpty() || (_sig.next && !_sig.next->IsEmpty()))
				{
					hAudio = PlayCSignals(_sig, devID, WM__AUDIOEVENT, hDlg, &block, errstr, 1);
					if (!hAudio)
						MessageBox(errstr, "Cannot play the audio"); // PlayArray fails if waveOutOpen fails
					else
					{
						AUD_PLAYBACK * p = (AUD_PLAYBACK*)hAudio;
						p->sig.SetValue((double)(INT_PTR)hAudio);
						p->sig.strut["type"] = string("audio_playback");
						p->sig.strut["devID"] = CSignals((double)devID);
						p->sig.strut["durTotal"] = CSignals(_sig.alldur());
						p->sig.strut["durLeft"] = CSignals(_sig.alldur());
						p->sig.strut["durPlayed"] = CSignals(0.);
						playing = true;
					}
				}
			}
			else
			{
				PauseResumePlay(hAudio, true); // resume
				paused = false;
				playing = true;
			}
		}
		else // if playing, pause
		{
			PauseResumePlay(hAudio, false); // paused
			paused = true;
			playing = false;
		}
		return;
	case IDM_STOP:
		if (hAudio > 0)
		{
			paused = playing = false;
			res = StopPlay(hAudio, true); // quick stop
			playLoc = -1;
			for (auto _ax : sbar->ax)
				InvalidateRect(_ax->rct);

			if (axis_expanding)
				WindowSizeAdjusting();
		}
		return;
#endif
	case IDM_SPECTRUM:
		// at this point, sbinfo.vax can be used
		ViewSpectrum();
		return;
	case IDM_WAVWRITE:
#ifndef NO_PLAYSND
		fullfname[0]=0;
		fileDlg.InitFileDlg(hDlg, hInst, "");
		GetGhost(_sig);
		if (fileDlg.FileSaveDlg(fullfname, fname, "Wav file (*.WAV)\0*.wav\0", "wav"))
		{
			if (!_sig.Wavwrite(fullfname, errstr))	MessageBox (errstr);
		}
		else
		{
			if (GetLastError()!=0) GetLastErrorStr(errstr), MessageBox (errstr, "Filesave dialog box error");
		}
#endif
		return;

	case IDM_MP3WRITE:
#ifndef NO_PLAYSND
		fullfname[0] = 0;
		fileDlg.InitFileDlg(hDlg, hInst, "");
		GetGhost(_sig);
		if (fileDlg.FileSaveDlg(fullfname, fname, "MP3 file (*.MP3)\0*.mp3\0", "mp3"))
		{
			if (!_sig.mp3write(fullfname, errstr))	MessageBox(errstr);
		}
		else
		{
			if (GetLastError() != 0) GetLastErrorStr(errstr), MessageBox(errstr, "Filesave dialog box error");
		}
#endif
		return;
		
	}
	InvalidateRect(NULL);
}


void CPlotDlg::GetGhost(CSignals &out, CAxes* pax)
{ 
	// If there's one registered axes, take the first two for stere (one for mono)
	// If there are two or more registered axes, take one lyne from each of the first two axes 
	//     (if there's no lyne in an axes, make an empty (or null) signal for that channel)
	// Therefore, two (or more) axes --> stereo output
	//            one axes --> mono (if there's one lyne); stereo (if there are multiple lyne's)
	// 9/17/2019
	vector<CAxes*> aa;
	if (!pax)
		aa = sbar->ax;
	else
		aa.push_back(sbar->ax.front());
	int count = min((int)aa.size(), 2);
	if (count == 1)
	{
		for (auto lyne : aa.front()->m_ln)
		{
			CTimeSeries *p = &lyne->sig;
			if (lyne->sig.GetType() == CSIG_AUDIO)
				count++;
		}
	}
	double t1, t2; // t1, t2 is x limit, either screen limit or selection range
	int _count = 0;
	CSignals *q = &out;
	for (auto _ax : aa)
	{
		if (curRange == NO_SELECTION)
		{
			t1 = _ax->xlim[0] * 1000.;
			t2 = _ax->xlim[1] * 1000.;
		}
		else
		{
			t1 = _ax->pix2timepoint(curRange.px1) * 1000.;
			t2 = _ax->pix2timepoint(curRange.px2) * 1000.;
		}
		for (auto lyne : _ax->m_ln)
		{
			CTimeSeries *p = &lyne->sig;
			if (count > 1 && _count == 1)
			{
				q->next = new CTimeSeries;
				q = (CSignals*) q->next;
				q->next = NULL;
			}
			//don't use the <= operator here, because p is not CSignals *
			*q = ((CTimeSeries*)q)->GhostCopy(p);
			if (lyne == _ax->m_ln.front())
				out.SetFs(lyne->sig.GetFs());
			if (lyne->sig.IsEmpty() && lyne == _ax->m_ln.back() && _count == 0)
			{
				_count++;
				continue;
			}
			if (lyne->sig.GetType() != CSIG_AUDIO) continue;
			for (CTimeSeries *pp = p; pp; pp = pp->chain)
				pp->ghost = true;
			q->Crop(t1, t2);
			for (CTimeSeries *pp = p; pp; pp = pp->chain)
				pp->ghost = false;
			if (++_count == 2) return;
		}
	}
}


int CPlotDlg::GetSelect(CSignals *pout)
{
	CAxes *cax(gcf.ax.front()); // Fix this..... 3/24/2018
	if (curRange != NO_SELECTION)
	{
		double d1 = cax->pix2timepoint(curRange.px1);
		double d2 = cax->pix2timepoint(curRange.px2);
		pout->UpdateBuffer(2);
		pout->buf[0] = d1;
		pout->buf[1] = d2;
		return 1;
	}
	return 0;
}

void CPlotDlg::getFFTdata(CTimeSeries *psig_mag, double *fft, int len)
{ // output: psig_mag
	int lenFFT = (len + 1) / 2;
	psig_mag->UpdateBuffer(len / 2 + 1);
	for (int k = 1; k < (len + 1) / 2; ++k)  /* (k < N/2 rounded up) */
		psig_mag->buf[k] = 20.*log10(fft[k] * fft[k] + fft[len - k] * fft[len - k]);
	/* DC and Nyquist components are not displayed
	psig_mag->buf[0] = 20.*log10(fft[0] * fft[0]); // DC component
	if (len % 2 == 0) // N is even 
		psig_mag->buf[len / 2] = 20.*log10(fft[len / 2] * fft[len / 2]);  // Nyquist freq. 
	*/
	if (len % 2 == 0) psig_mag->nSamples--; // Nyquist component excluded
	double maxmag = psig_mag->_max().front();
	*psig_mag += -maxmag;
}

void CPlotDlg::ShowSpectrum(CAxes *pax, CAxes *paxBase)
{
	bool stereo;
	int len;
	fftw_plan plan;
	double dfs, *freq, *fft;
	CSignals _sig;
	double lastxlim[2];
	CTick lastxtick;

#ifndef NO_FFTW
	stereo = pax->m_ln.size()>1 ? true : false;
	if ( gcf.ax.size()==0) return;

	// It gets Chainless inside GetSignalofInterest in this call
	dfs = (double)paxBase->m_ln.front()->sig.GetFs();
	pax->xlim[0] = 0;  pax->xlim[1] = dfs / 2;
	//Right now _sig is a chainless'ed version... do it later to keep the chain
	GetGhost(_sig);
	//if _sig is stereo, each is the two registered signals
	//if paxBase has only one m_ln, spectrum should show only the one corresponding one.
	if (paxBase->m_ln.size() == 1 && _sig.next)
	{ // which, _sig or _sig.next, is the same as paxBase->m_ln[0]?
		if (_sig.buf != paxBase->m_ln.front()->sig.buf)
			_sig = *_sig.next;
		_sig.next = nullptr;
	}
	if (pax->m_ln.empty()) lastxlim[0]=1.,lastxlim[1]=-1.;
	for (; pax->m_ln.size()>0;)	
	{
		lastxtick = pax->xtick;
		memcpy((void*)lastxlim, (void*)pax->xlim, sizeof(pax->xlim));
		deleteObj(pax->m_ln[0]);
	}
	if ((len=_sig.nSamples)<20) 
	{
		CClientDC dc(hDlg);
		CPen *dotted = new CPen;
		COLORREF dd = RGB(255, 0, 0);
		dotted->CreatePen(PS_DOT, 1, dd);
		dc.SelectObject(dotted);
		dc.TextOut(pax->rct.left, (pax->rct.bottom-pax->rct.top)/2, "Selection too short");
		delete[] dotted;
		return;
	}
	freq = new double[len/2+1];
	fft = new double[len];
	for (int k=0; k<len/2+1; k++)		
		freq[k] = dfs / len * k ;
	plan = fftw_plan_r2r_1d(len, _sig.buf, fft, FFTW_R2HC, FFTW_ESTIMATE);
	fftw_execute(plan);
	CSignals mag;
	getFFTdata(&mag, fft, len);
	if (_sig.next)
	{
		int len2 = _sig.next->nSamples;
		if (len2 > len)
		{
			delete[] freq;
			delete[] fft;
			freq = new double[len2 / 2 + 1];
			fft = new double[len2];
		}
		for (int k = 0; k < len2 / 2 + 1; k++)
			freq[k] = dfs / len2 * k;
		plan = fftw_plan_r2r_1d(len2, _sig.next->buf, fft, FFTW_R2HC, FFTW_ESTIMATE);
		fftw_execute(plan);
		mag.next = new CTimeSeries; // this will be deleted during the cleanup of mag... really?
		getFFTdata(mag.next, fft, len2);
	}
	PlotCSignals(pax, freq, &mag, 0, 0, LineStyle_solid); // inherited color scheme, no marker and solid line style
	strcpy(pax->xtick.format,"%.2gk"); // pax->xtick.format is called in anticipation of drawticks. i.e., format is used in drawticks
	if (lastxlim[0]>lastxlim[1])
	{
		pax->xtick.tics1 = pax->gengrids('x');
		pax->xtick.mult = 0.001;		
	}
	else
	{
		pax->xtick = lastxtick;
		memcpy((void*)pax->xlim, (void*)lastxlim, sizeof(pax->xlim));
	}
	pax->ylim[0]=-110; pax->ylim[1] = 0;
	pax->ytick.tics1 = pax->gengrids('y');
	pax->m_ln.front()->color = gcf.ax.front()->m_ln.front()->color;
	delete[] freq;
	delete[] fft;

	CRect rt, rt2;
	GetClientRect(hDlg, &rt);
	rt2 = pax->rct = pax->pos.GetRect(rt.Width(), rt.Height());
	rt2.InflateRect(40, 30);
	InvalidateRect(rt2);
#endif
}

int IsInsideRect(RECT* rect, POINT* pt)
{
	return (pt->x >= rect->left && pt->x <= rect->right && pt->y >= rect->top && pt->y <= rect->bottom);
}

CAxes * CPlotDlg::CurrentPoint2CurrentAxis(CPoint *point)
{ // search only among registered axes
	for (auto _ax : sbar->ax)
	{
		auto __ax = _ax;
		bool fftAxPresent = _ax->hChild != NULL;
		//if _ax has FFTAx
		if (fftAxPresent)
		{
			__ax = (CAxes*)_ax->hChild;
			if (IsInsideRect(__ax->rct, point))
				return gca = __ax;
		}
		if (IsInsideRect(_ax->rct, point))
			return gca = _ax;
	}
	return NULL;
}

void CPlotDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
}

void CPlotDlg::UpdateRects(CAxes *ax)
{
	if (ax==NULL) return;
	CRect rt0(ax->rct);

	roct[0] = CRect(rt0.left, gcmp.y-2, gcmp.x+2,gcmp.y+2);
	roct[1] = CRect(gcmp.x-2, gcmp.y-2, gcmp.x+2, rt0.bottom);
	roct[2] = CRect(CPoint(gcmp.x-40,rt0.bottom+6), CSize(80,18));
	roct[3] = CRect(CPoint(rt0.left,gcmp.y-20), CSize(40,18));
	roct[4] = CRect(CPoint(gcmp.x-50,gcmp.y-40), CSize(100,40));
}

unsigned short CPlotDlg::GetMousePos(CPoint pt)
{ // revise to accommodate stereo
	unsigned short res(0);
	static int count(0);
	if (gcf.ax.empty()) return 0;
	//process only the first two axes
	CAxes* paxFFT2, *paxFFT = (CAxes*)gcf.ax.front()->hChild;
	switch(gcf.ax.size())
	{
	case 1: // mono
		if (paxFFT)
			return MAKEWORD(GetMousePosReAx(pt, gcf.ax.front()),  GetMousePosReAx(pt, paxFFT)); 
		else
			return MAKEWORD(GetMousePosReAx(pt, gcf.ax.front()),  0); 
	case 2: //stereo
		paxFFT2 = (CAxes*)gcf.ax.back()->hChild;
		if (paxFFT2)
		{
			unsigned char a = GetMousePosReAx(pt, gcf.ax.front());
			unsigned char b = GetMousePosReAx(pt, gcf.ax.back());
			unsigned char c = GetMousePosReAx(pt, paxFFT);
			unsigned char d = GetMousePosReAx(pt, paxFFT2);
			return MAKEWORD(a|b,c|d); 
		}
		else
		{
			unsigned char a = GetMousePosReAx(pt, gcf.ax.front());
			unsigned char b = GetMousePosReAx(pt, gcf.ax.back());
			return MAKEWORD(a|b,0); 
		}
	}
	return 0;
}

void CPlotDlg::Register(CAxes *pax, bool b)
{
	if (b)
	{
		if (find(sbar->ax.begin(), sbar->ax.end(), pax) == sbar->ax.end())
			sbar->ax.push_back(pax);
	}
	else
	{
		auto it = find(sbar->ax.begin(), sbar->ax.end(), pax);
		if (it != sbar->ax.end())
			sbar->ax.erase(it);
	}
}

void CPlotDlg::OnActivate(UINT state, HWND hwndActDeact, BOOL fMinimized)
{
	hPlotDlgCurrent = hDlg;
	HandleLostFocus(WM_ACTIVATE);
}

BOOL CPlotDlg::OnNCActivate(UINT state)
{
	HandleLostFocus(WM_NCACTIVATE);

	if (!state) return TRUE;
	else		return FALSE;

}

void CPlotDlg::MouseLeave(UINT umsg)
{

}

void CPlotDlg::HandleLostFocus(UINT umsg, LPARAM lParam)
{
	if (mst.clickedOn)
	{
		OnLButtonUp(0, mst.curPt);
	}
}

void CPlotDlg::dBRMS(SHOWSTATUS st)
{
	sbar->dBRMS(st);
}
