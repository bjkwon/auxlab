// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: graffy
// Graphic Library (Windows only)
// 
// 
// Version: 1.497
// Date: 12/26/2018
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

FILE *fp;

int iabs(int x)
{
	if (x<0)		return -x;
	return x;
}

int LRrange(RECT* rect, int var, char xy)
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

unsigned _int8 GetMousePosReAx(CPoint pt, CAxes* pax)
{// This reveals where pt is located relative to pax
 // If pt is inside the axis rectangle, returns 15 (i.e., 0b1111)
 // If pt is in the ytick area, returns 0b0001
 // If pt is in the xtick area, returns 0b0100
 // If pt is in the xtick and ytick area, returns 0b0101
 // Otherwise, return 0
	if (pax->axRect.PtInRect(pt)) return AXCORE;
	unsigned _int8 res(0);
	if (pax->xtick.rt.PtInRect(pt)) res = GRAF_REG1 << 2;
	if (pax->ytick.rt.PtInRect(pt)) res += GRAF_REG1;
	return res;
}

CPlotDlg::CPlotDlg()
:axis_expanding(false), levelView(false), playing(false), paused(false), ClickOn(0), MoveOn(0), devID(0), playLoc(-1), zoom(0), spgram(false), selColor(RGB(150, 180, 155)), hStatusbar(NULL)
{ // do not use this.
}

CPlotDlg::CPlotDlg(HINSTANCE hInstance, CGobj *hPar)
:axis_expanding(false), levelView(false), playing(false), paused(false), ClickOn(0), MoveOn(0), devID(0), playLoc(-1), zoom(0), spgram(false), selColor(RGB(150, 180, 155)), hStatusbar(NULL), gca(NULL), playCursor(-1)
{
	opacity = 0xff;
	gcf.m_dlg = this;
	gcf.hPar = hPar;
	gcf.hPar->child.push_back(&gcf);

	menu.LoadMenu(IDR_POPMENU);
	subMenu = menu.GetSubMenu(0);
	gcmp=CPoint(-1,-1);
	z0pt=CPoint(-1,-1);
	hInst = hInstance;
	hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR));
	for (int k(0); k<10; k++) hTTtimeax[k]=NULL;
	ttstat.push_back("begin(screen)");
	ttstat.push_back("end(screen)");
	ttstat.push_back("Cursor position");
	ttstat.push_back("");
	ttstat.push_back("begin(selection)");
	ttstat.push_back("end(selection)");
	ttstat.push_back("Frequency");
	sbinfo.initialshow = false;
	sbinfo.xSelBegin = sbinfo.xSelEnd = 0.;
}

CPlotDlg::~CPlotDlg()
{
	if (HIWORD((INT_PTR)title))
		delete title;
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
	SetLayeredWindowAttributes(hwnd, 0, opacity, LWA_ALPHA); // how can I transport transparency from application? Let's think about it tomorrow 1/6/2017 12:19am
	hTTscript = CreateTT(hwnd, &ti_script);
    GetClientRect (hwnd, &ti_script.rect);
	ti_script.rect.bottom = ti_script.rect.top + 30;
	::SendMessage(hTTscript, TTM_ACTIVATE, TRUE, 0);	
	::SendMessage(hTTscript, TTM_SETMAXTIPWIDTH, 0, 400);
	//These two functions are typically called in pair--so sigproc and graffy can communicate with each other for GUI updates, etc.
	SetClassLongPtr (hwnd, GCLP_HICON, (LONG)(LONG_PTR)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, 0));
	// This is necessary to set the showVarDlg window from showvar.cpp as the "anchor" for playback
	SetHWND_WAVPLAY((HWND)lParam); 
	return TRUE;
}

void CPlotDlg::OnCommand(int idc, HWND hwndCtl, UINT event)
{
	int res(0);
	switch (idc)
	{
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
				if (pax->m_ln.front()->xdata.buf[id]>=fxlim0) { out.x = id; break;}
			for (int id=(int)p->nSamples-1; id>=0; id--)
				if (pax->m_ln.front()->xdata.buf[id]<=fxlim1) { out.y = id; break;}
		}
	}
	return out;
}

vector<POINT> CPlotDlg::makeDrawVector(CSignal *p, CAxes *pax)
{
	unsigned int id(0), beginID, ind1, ind2 ;
	int fs = pax->m_ln.front()->sig.GetFs();
	vector<POINT> draw;
	if (pax->xlim[0]>=pax->xlim[1]) return draw;
	CPoint pt;
	double xPerPixel = (pax->xlim[1]-pax->xlim[0]) / (double)pax->rcAx.Width(); // How much advance in x-axis per one pixel--time for audio-sig, sample points for nonaudio plot(y), whatever x-axis means for nonaudio play(x,y)
	double nSamplesPerPixel; // how many sample counts of the sig are covered per one pixel. Calculated as a non-integer, rounded-down value is used and every once in a while the remainder is added
	POINT pp = GetIndDisplayed(pax);//The indices of xlim[0] and xlim[1], if p were to be a single chain.
	ind1 = max(0, pp.x);
	ind2 = max(0, pp.y); //this is just the right end value, not the y-axis value
	unsigned int inttmark, endttmark;
	CTimeSeries *ppp = (CTimeSeries*)p;
	bool tseries = ppp->IsTimeSignal();
	if (tseries)
	{ // if p is one of multiple chains, ind1 and ind2 need to be adjusted to reflect indices of the current p.
		nSamplesPerPixel = xPerPixel * fs; 
		if (p->tmark>=pax->xlim[1]*1000. || p->endt()<=pax->xlim[0]*1000.) return draw;//return empty
		inttmark = (int)(p->tmark*fs/1000.+.5);
		ind1 = max(ind1, inttmark);
		endttmark = (int)((p->endt()/1000)*fs+.5);
		ind2 = min(ind2, endttmark);
	}
	else
	{
		nSamplesPerPixel = xPerPixel * (pax->xlim[1]-pax->xlim[0]) / (ind2-ind1) ; 
		inttmark = ind1;
		endttmark = ind2;
	}

	double multiplier = 2.;
	double maax, miin;
	int nPixels = pax->rcAx.right-pax->rcAx.left+1; // pixel count to cover the plot

	(p->tmark<=pax->xlim[0]*1000.) ?  beginID = (int)((pax->xlim[0]-p->tmark/1000.)*(double)fs +.5) : beginID = 0;
	int estimatedNSamples = (int)((pax->xlim[1]-pax->xlim[0])*(double)fs);
	if (!pax->hChild && pax->m_ln.front()->sig.GetType()==CSIG_VECTOR)  // For FFT, go to full drawing preemptively...
		estimatedNSamples=0; 
	double remnant(0);
	int adder;
	if (estimatedNSamples>multiplier*nPixels) // Quick drawing // condition: the whole nSamples points are drawn by nPixels points in pax->rcAx
	{
		do {
		remnant += nSamplesPerPixel - (int)(nSamplesPerPixel);
		if (remnant>1) {adder = 1; remnant -= 1.;}
		else adder = 0;
		id = min(beginID+(int)(nSamplesPerPixel)+adder, p->nSamples);
		if (!p->IsLogical())
		{
			body temp(p->buf+beginID, id-beginID);
			miin = temp._min();
			maax = temp._max();
		}
		else
		{
			body temp(p->logbuf+beginID, id-beginID);
			miin = temp._min();
			maax = temp._max();
		}
		if (miin!=1.e100) 
		{
			if (pax->m_ln.front()->xdata.nSamples) // for non-audio, plot(x,y) 
			{ // CHECK HOW THIS IS DIFFERENT FROM LINES 313-318
				pt = pax->double2pixelpt(pax->m_ln.front()->xdata.buf[beginID], miin, NULL);
				pt.y = min(max(pax->rcAx.top, pt.y), pax->rcAx.bottom);
				draw.push_back(pt);
				pt = pax->double2pixelpt(pax->m_ln.front()->xdata.buf[beginID], maax, NULL);
				pt.y = min(max(pax->rcAx.top, pt.y), pax->rcAx.bottom);
			}
			else
			{
				if (tseries)  // for audio, plot(x)
				{
					pt = pax->double2pixelpt((double)beginID/(double)fs+p->tmark/1000., miin, NULL);
					pt.y = min(max(pax->rcAx.top, pt.y), pax->rcAx.bottom);
					draw.push_back(pt);
					pt = pax->double2pixelpt((double)beginID/(double)fs+p->tmark/1000., maax, NULL);
					pt.y = min(max(pax->rcAx.top, pt.y), pax->rcAx.bottom);
				}
				else // for non-audio, plot(x)
				{
					double xval = pax->xlim[0] + (double) (beginID-ind1) / (ind2-ind1) * (pax->xlim[1]-pax->xlim[0]);
					pt = pax->double2pixelpt((double)xval, miin, NULL);
					pt.y = min(max(pax->rcAx.top, pt.y), pax->rcAx.bottom);
					draw.push_back(pt);
					pt = pax->double2pixelpt((double)xval, maax, NULL);
					pt.y = min(max(pax->rcAx.top, pt.y), pax->rcAx.bottom);
				}
			}
			if (p->IsLogical() && maax>0.) 
				pt.y--; // to show inside the axis box
			draw.push_back(pt);
		}
		beginID  = id; 
	} while (id<min(p->nSamples, ind2));
	}
	else // Full drawing 
	{
		if (tseries)
			if (p->IsLogical())
				for (unsigned int k=ind1; k<ind2; k++)
				{
					pt = pax->double2pixelpt((double)k/fs, (double)p->logbuf[k-inttmark], NULL);
					draw.push_back(pt);
				}
			else
				for (unsigned int k=ind1; k<ind2; k++)
				{
					pt = pax->double2pixelpt((double)k/fs, p->buf[k-inttmark], NULL);
					draw.push_back(pt);
				}
		else
		{
			bool wasbull(0);
			if (pax->m_ln.front()->xdata.nSamples==0) // plot(y) 
			{
				wasbull=true;
				pax->m_ln.front()->xdata.UpdateBuffer(p->nSamples);
				for (unsigned int k=0; k<p->nSamples; k++) pax->m_ln.front()->xdata.buf[k] = (double)(k+1);
			}
			else
				ind2++; // The meaning of ind2 is different whether xdata is NULL or not, this line is necessary otherwise the last point is missing
			for (id=ind1; id<ind2 && id-ind1<p->nSamples; id++)
			{
				if (p->bufBlockSize==1)
					pt = pax->double2pixelpt(pax->m_ln.front()->xdata.buf[id], p->logbuf[id], NULL);
				else
					pt = pax->double2pixelpt(pax->m_ln.front()->xdata.buf[id], p->buf[id], NULL);
				if (pt.y > pax->rcAx.bottom || pt.y < pax->rcAx.top)
				{
					if (p->buf[id] == std::numeric_limits<double>::infinity())
						pt.y = pax->rcAx.top - 40;
					else if (p->buf[id] == - std::numeric_limits<double>::infinity())
						pt.y = pax->rcAx.bottom + 40;
					else
						pt.y = min(max(pax->rcAx.top, pt.y), pax->rcAx.bottom);
				}
				draw.push_back(pt);
			}
			//if (wasbull)
			//{
			//	delete[] pax->m_ln.front()->xdata;
			//	pax->m_ln.front()->xdata=NULL;
			//}
		}
	}
	return draw;
}

CSignals CPlotDlg::GetAudioSignal(CAxes* pax, bool makechainless)
{ // if pax is NULL (default), it scans all axes in gcf and returns CSignals
  // it is is not NULL, it checks only pax and returns CTimeSeries (i.e., CSignals with NULL next)
	vector<CAxes*> temp;
	if (pax) temp.push_back(pax);
	else temp = gcf.ax;
	unsigned int count(0);
	double t1, t2, deltat;
	CSignals out;
	CTimeSeries part;
	for (vector<CAxes*>::iterator px= temp.begin(); px != temp.end(); px++)
	{
		for (vector<CLine*>::iterator pline = (*px)->m_ln.begin(); pline != (*px)->m_ln.end(); pline++)
		{
			if ((*pline)->sig.GetType() == CSIG_AUDIO)
			{
				t1 = (*px)->xlim[0] * 1000.;
				t2 = (*px)->xlim[1] * 1000.;
				deltat = t2 - t1;
				if (curRange != NO_SELECTION)
				{
					t1 = (*px)->xlim[0] * 1000. + deltat*(curRange.px1 - (*px)->axRect.left) / (*px)->axRect.Width();
					t2 = (*px)->xlim[0] * 1000. + deltat*(curRange.px2 - (*px)->axRect.left) / (*px)->axRect.Width();
				}
				else if (playCursor > 0)
					t1 = (*px)->xlim[0] * 1000. + deltat*(playCursor - (*px)->axRect.left) / (*px)->axRect.Width();
				count++;
				if (count == 3) break;
				else if (count == 1)
					out = (*pline)->sig.Extract(t1, t2);
				else
				{
					part = (*pline)->sig.Extract(t1, t2);
					out.SetNextChan(&part);
				}
			}
		}
	}
	if (makechainless)
		out.MakeChainless();
	return out;
}


void OnPaint_createpen_with_linestyle(CLine *pln, CDC& dc)
{
	switch(pln->lineStyle)
	{
	case LineStyle_solid:
		dc.CreatePen(PS_SOLID, pln->lineWidth, pln->color);
		break;
	case LineStyle_dash:
		dc.CreatePen(PS_DASH, pln->lineWidth, pln->color);
		break;
	case LineStyle_dot:
		dc.CreatePen(PS_DOT, pln->lineWidth, pln->color);
		break;
	case LineStyle_dashdot:
		dc.CreatePen(PS_DASHDOT, pln->lineWidth, pln->color);
		break;
	default:
		dc.CreatePen(PS_NULL, pln->lineWidth, pln->color);
		break;
	}
}

void CPlotDlg::OnPaint() 
{
	opacity = gcf.inScope ? 0xff : 0xc0;
	SetLayeredWindowAttributes(hDlg, 0, opacity, LWA_ALPHA);
	PAINTSTRUCT  ps;
	if (hDlg==NULL) return;
	HDC hdc = BeginPaint(hDlg, &ps);
	if (hdc==NULL) { EndPaint(hDlg, &ps); return; }
	CDC dc(hdc, hDlg);
	CClientDC dc2(hDlg);
	CPoint pt;
	vector<POINT> draw;
	CRect clientRt;
	CAxes* pax;
	GetClientRect(hDlg, &clientRt);
	if (clientRt.Height()<15) { EndPaint(hDlg, &ps); return; }
	dc.SolidFill(gcf.color, clientRt);
	CRect rt;
	char buf[256];
	GetClientRect(hDlg, &rt);
	if (gcf.ax.size()>0)
	{
		CAxes *pax0 = gcf.ax.front();
		int nax(1);
		// drawing lines
		bool paxready(false);
		for (size_t k=0; k<gcf.ax.size();)
		{
			if (find(sbinfo.vax.begin(), sbinfo.vax.end(), gcf.ax[k]) == sbinfo.vax.end())
				sbinfo.vax.push_back(gcf.ax[k]);
		//the reason I changed from iterator to indexing---
		// now, all axes are not necessarily in the ax vector. some (e.g., FFT axis) are linked from another axis and it's incredibly difficult to make it with an iterator 
		// 8/12/2017 bjk
			if (!paxready) 
				pax = gcf.ax[k];
			else
				paxready=false;
			if (!pax->visible) continue;
			if (!axis_expanding)
				CPosition lastpos(pax->pos);
			else
				pax->pos.Set(clientRt, pax->axRect); // do this again
			pax->rcAx=DrawAxis(&dc, &ps, pax);
			//when mouse is moving while clicked
			if (curRange != NO_SELECTION && pax->hPar->type==GRAFFY_figure ) // this applies only to waveform axis (not FFT axis)
			{
				rt = pax->axRect;
				rt.left = curRange.px1+1; 
				rt.right = curRange.px2-1; 
				rt.top--;
				rt.bottom++;
				dc.SolidFill(selColor, rt);
				if (ClickOn)
				{
					dc.CreatePen(PS_DOT, 1, RGB(255, 100, 0));
					dc.MoveTo(curRange.px1, rt.bottom-2);
					dc.LineTo(curRange.px1, rt.top+1); 
					dc.MoveTo(curRange.px2, rt.bottom-2);
					dc.LineTo(curRange.px2, rt.top+1); 
				}
			}
			size_t nLines = pax->m_ln.size(); // just FYI
			for (auto lyne : pax->m_ln)
			{
				if (!lyne->visible) continue;
				for (CTimeSeries *p = &(lyne->sig); p; p = p->chain)
				{
					auto anSamples = p->nSamples;
					auto atuck = p->nGroups;
					auto atmark = p->tmark;
					double *abuf = new double[p->Len() * sizeof(double)]; // real assumed... check complex 10/30/2018
					memcpy(abuf, p->buf, p->Len() * sizeof(double));
					GetWindowText(buf, sizeof(buf));
					BYTE clcode;
					vector<DWORD> kolor;
					clcode = HIBYTE(HIWORD(lyne->color));
					if (clcode=='L' || clcode=='R') 
						kolor = Colormap(clcode, clcode, 'r', atuck);
					if (clcode == 'l' || clcode == 'r')
						kolor = Colormap(clcode, clcode+'R'-'r', 'c', atuck);
					else if (clcode == 'M')
					{
						CVar cmap = lyne->strut["color"];
						for (unsigned int k=0; k<cmap.nSamples; k+=3)
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
						memcpy(p->buf, p->buf + m*p->nSamples, p->bufBlockSize*p->nSamples);
						if (p->IsTimeSignal())
							p->tmark += 1000.* m*p->nSamples / p->GetFs();
						if (lyne->symbol != 0)
						{
							LineStyle org = lyne->lineStyle;
							lyne->lineStyle = LineStyle_solid;
							if (lyne->lineWidth == 0)
								lyne->lineWidth = 1;
							OnPaint_createpen_with_linestyle(lyne, dc);
							draw = makeDrawVector(p, pax);
							DrawMarker(dc, lyne, draw);
							lyne->lineStyle = org;
						}
						OnPaint_createpen_with_linestyle(lyne, dc);
						if (lyne->lineWidth > 0)
						{
							draw = makeDrawVector(p, pax);
							if (p->IsTimeSignal()) {
								if (pt.y < pax->axRect.top)  pt.y = pax->axRect.top;
								if (pt.y > pax->axRect.bottom) pt.y = pax->axRect.bottom;
							}
							if (draw.size() > 0)
							{
								if (lyne->lineStyle != LineStyle_noline)
									if (draw[draw.size() - 1].x <= pax->axRect.right)
										dc.Polyline(draw.data(), (int)draw.size());
									else
									{
										int shortlen(-1);
										int nDrawPt = (int)draw.size();
										for (int p = nDrawPt - 1; p > 0; p--)
											if (draw[p].x <= pax->axRect.right)
												shortlen = p, p = -1;
										if (shortlen > 0)
											dc.Polyline(draw.data(), shortlen);
									}
							}
							if (kolor.size() > 1)
							{
								colorIt++;
								if (colorIt== kolor.end())		colorIt = kolor.begin();
							}
							p->nGroups = atuck;
							p->tmark = atmark;
							p->nSamples = anSamples;
							memcpy(p->buf, abuf, p->Len() * sizeof(double));
						}
					}
					delete[] abuf;
				}
			} 
//			dc.SetBkColor(pax->color);
			// add ticks and ticklabels
			dc.SetBkColor(gcf.color);
			// For the very first call to onPaint, axRect is not known so settics is skipped, and need to set it here
			// also when InvalidateRect(NULL) is called, always remake ticks
			if (pax->m_ln.size() > 0)
			{
				double xstep = 0;
				if (pax->xtick.tics1.size() > 1) xstep = pax->xtick.tics1.back() - pax->xtick.tics1.front();
				if (pax->xtick.tics1.empty() && pax->xtick.automatic)
				{
					CRect tprt;
					if (!tprt.SubtractRect(&pax->xtick.rt, &ps.rcPaint)) // if xtick rect is part of rcPaint 
						if (pax->m_ln.front()->sig.IsTimeSignal())
							pax->xtick.tics1 = pax->gengrids('x', -3);
						else
						{
							if (draw.size()>2)
								pax->setxticks();
							else// if (draw.size() == 2)
							{
								pax->xtick.tics1.push_back(pax->xlim[0]);
								pax->xtick.tics1.push_back(pax->xlim[1]);
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
				DrawTicks(&dc, pax, 0);

				//x & y labels
				dc.SetTextAlign(TA_RIGHT|TA_BOTTOM);
				if (!gcf.ax.empty() && !gcf.ax.front()->m_ln.empty())
				{
					if (IsSpectrumAxis(pax))
					{
						dc.TextOut(pax->axRect.right-3, pax->axRect.bottom, "Hz");
						dc.TextOut(pax->axRect.left-3, pax->axRect.top-1, "dB");
					}
					else if (pax->m_ln.front()->sig.IsTimeSignal())
						dc.SetBkMode(TRANSPARENT), dc.TextOut(pax->axRect.right-3, pax->axRect.bottom, "sec");
				}
			}
			if (pax->hChild) 	
				paxready=true, pax = (CAxes*)pax->hChild;
			else
				k++;
		}
		CAxes *paxx(gcf.ax.front()); 
		if (playCursor > 0) // if cursor for playback was set
		{
			dc.CreatePen(PS_SOLID, 1, RGB(255, 131, 80));
			dc.MoveTo(playCursor, paxx->axRect.bottom);
			dc.LineTo(playCursor, paxx->axRect.top);
		}
		if (LRrange(&paxx->rcAx, playLoc, 'x')==0)
		{
			dc.CreatePen(PS_SOLID, 1, RGB(204,77,0));
			dc.MoveTo(playLoc, paxx->axRect.bottom);
			dc.LineTo(playLoc, paxx->axRect.top);
		}
		if (pax0->m_ln.size()>0 || (gcf.ax.size()>1 && gcf.ax[1]->m_ln.size()>0) )
		{
			sbinfo.xBegin = pax0->xlim[0];
			sbinfo.xEnd = pax0->xlim[1];
		}
	} // k<gcf.ax.size()
	//Drawing texts
	HFONT fontOriginal = (HFONT)dc.SelectObject(GetStockObject(SYSTEM_FONT));
	for (vector<CText*>::iterator txit=gcf.text.begin(); txit!=gcf.text.end(); txit++) { 
		if ((*txit)->visible && (*txit)->pos.x0>=0 && (*txit)->pos.y0>=0)
		{
			HFONT ft = (HFONT)dc.SelectObject((*txit)->font);
			pt.x = (int)((double)clientRt.right * (*txit)->pos.x0+.5);
			pt.y = (int)((double)clientRt.bottom * (1-(*txit)->pos.y0)+.5);
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor((*txit)->color);
//			dc.DrawText((*txit)->str.c_str(),-1, &(*txit)->textRect, (*txit)->alignmode | DT_NOCLIP);
			DWORD dw = (*txit)->alignmode;
			DWORD dw2 = TA_RIGHT|TA_TOP;
			DWORD dw3 = TA_CENTER|TA_BASELINE;
			dc.SetTextAlign(dw);
			dc.TextOut(pt.x, pt.y, (*txit)->str.c_str(), (int)(*txit)->str.length());
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
		dc.TextOut(pax->axRect.left, pax->axRect.top, tp[1].c_str());
	}
	EndPaint(hDlg, &ps);
	if (!sbinfo.initialshow) 
	{
		sbinfo.initialshow=true; ShowStatusBar();
	//	SetGCF();
	}
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
			if (LRrange(&pax->rcAx, loc, xy)==0) break;
		}
		//iterate within the range
		for (; it !=pax->xtick.tics1.end() && LRrange(&pax->rcAx, loc, xy)==0 ; it++)
		{
			nextpt = *it;
			if (pax->m_ln[0]->sig.GetType()==CSIG_VECTOR && pax->m_ln[0]->xdata.nSamples == 0)
				loc = pax->double2pixel((int)(nextpt+.5), xy); 
			else											
				loc = pax->double2pixel(nextpt, xy); 
			if (LRrange(&pax->rcAx, loc, xy)>0) // if the current point is right side of axis
			{
				loc = pax->rcAx.right;
				value = max(0, pax->xtick.mult*pax->GetRangePixel(loc)+pax->xtick.offset);
			}
			else if (LRrange(&pax->rcAx, loc, xy)<0)  // if the current point is left side of axis, make it forcefully inside.
			{
				loc = pax->rcAx.left+1;
				value = max(0, pax->xtick.mult*pax->GetRangePixel(loc)+pax->xtick.offset);
				//further adjust tics1 according to the adjusted loc
			}
			else
				value = max(0, pax->xtick.mult*nextpt+pax->xtick.offset);
			loc = min(pax->rcAx.right-1, loc);  //loc should not be the same as pax->rcAx.right (then the ticks would protrude right from the axis rectangle)
			pDC->MoveTo(loc, pax->rcAx.bottom-1);
			pDC->LineTo(loc, pax->rcAx.bottom-1 - pax->xtick.size); 
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
				pDC->TextOut(loc, pax->rcAx.bottom + pax->xtick.labelPos, label);
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
			if (LRrange(&pax->rcAx, loc, xy)==0) break;
		}
		//iterate within the range
		for (; it !=pax->ytick.tics1.end() && LRrange(&pax->rcAx, loc, xy)==0 ; it++)
		{
			nextpt = *it;
			loc = min(pax->double2pixel(nextpt, xy), pax->rcAx.bottom-1); //loc should not be the same as pax->rcAx.bottom (then the ticks would protrude downward from the axis rectangle)
			if (LRrange(&pax->rcAx, loc, xy)>0) // if the current point is above of axis
				loc = pax->rcAx.top;
			else if (LRrange(&pax->rcAx, loc, xy)<0)  // if the current point is left side of axis, make it forcefully inside.
				loc = pax->rcAx.bottom-1;
			value = nextpt;
			pDC->MoveTo(pax->rcAx.left, loc);
			pDC->LineTo(pax->rcAx.left + pax->ytick.size, loc);
			if (pax->ytick.format[0]!=0)
				sprintf(label, pax->ytick.format, value);
			else
				sprintfFloat(value, max(0,min(3,1-(int)scalemant)), label, 256);
			GetTextExtentPoint32(hdc, label, (int)strlen(label), &sz);
			if (iabs(loc-lastpt2Draw)> sz.cy + pax->xtick.gap4next.y) // only if there's enough gap, Textout
			{
				pDC->TextOut(pax->rcAx.left - pax->ytick.labelPos, loc-fnt.lfHeight/2, label);
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
	CRect rt3(pax->pos.GetRect(rt));
	LONG temp = rt3.bottom;
	rt3.bottom = rt3.top;
	rt3.top = temp;
	pDC->Rectangle(rt3);
	pax->axRect = rt3;
	SIZE sz (pDC->GetTextExtentPoint32 ("X", 5));
	int margin = 10;
	pax->ytick.rt = CRect( CPoint(rt3.left-sz.cx, rt3.top), CPoint(rt3.left,rt3.bottom));
	pax->xtick.rt = CRect( CPoint(rt3.left-margin,rt3.bottom), CSize(rt3.right-rt3.left+2*margin,pax->xtick.labelPos+sz.cy));
	if (axis_expanding)
	{
		strcpy(buf1,"Adjust Window Size.");
		pDC->TextOut(50, 5, buf1);
		strcpy(buf1,"Press Ctrl key again to revert to the normal mode.");
		pDC->TextOut(80, 20, buf1);
	}

	pDC->CreateFont(15, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, 
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
	return rt3;
}

void CPlotDlg::OnSize(UINT nType, int cx, int cy) 
{
	int new1, new2;
	int sigtype(-1);
	if (gcf.ax.size()>0 && gcf.ax.front()->m_ln.size()>0)
		sigtype = gcf.ax.front()->m_ln.front()->sig.GetType();

	if (hStatusbar==NULL)
	{
		hStatusbar = CreateWindow (STATUSCLASSNAME, "", WS_CHILD|WS_VISIBLE|WS_BORDER|SBS_SIZEGRIP,
			0, 0, 0, 0, hDlg, (HMENU)0, hInst, NULL);
		int width[] = {40, 110, 2, 40, 40, 160,};
		int sbarWidthArray[8];
		sbarWidthArray[0] = 40;
		for (int k=1;k<8;k++) 
			sbarWidthArray[k] = sbarWidthArray[k-1] + width[k-1];
		sbarWidthArray[7]=-1;
		::SendMessage (hStatusbar, SB_SETPARTS, 12, (LPARAM)sbarWidthArray);
		SetHWND_GRAFFY(hDlg);
	}
	else
		::MoveWindow(hStatusbar, 0, 0, cx, 0, 1); // no need to worry about y pos and height
	
	if (curRange != NO_SELECTION && gcf.ax.size()>0)
	{
		//need to adjust curRange according to the change of size 
		//new pixel pt after size change
		new1 = gcf.ax.front()->timepoint2pix(selRange.tp1);
		new2 = gcf.ax.front()->timepoint2pix(selRange.tp2);
		curRange.px1 = new1;
		curRange.px2 = new2;
	}
	if (hTTtimeax[0]==NULL)
	{
		for (int k=0; k<7; k++)
		{
			hTTtimeax[k] = CreateTT(hStatusbar, &ti_taxis);
			::SendMessage (hStatusbar, SB_GETRECT, k, (LPARAM)&ti_taxis.rect);
			ti_taxis.lpszText=(LPSTR)ttstat[k].c_str();
			::SendMessage(hTTtimeax[k], TTM_ACTIVATE, TRUE, 0);	
			::SendMessage(hTTtimeax[k], TTM_SETMAXTIPWIDTH, 0, 400);
			::SendMessage(hTTtimeax[k], TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti_taxis);
			::SendMessage(hTTtimeax[k], TTM_UPDATETIPTEXT, 0,  (LPARAM) (LPTOOLINFO) &ti_taxis);
		}
	}
	::SendMessage(hTTscript, TTM_ACTIVATE, TRUE, 0);
	for (vector<CAxes*>::iterator it = gcf.ax.begin(); it != gcf.ax.end(); it++)
	{
		if ((*it)->ytick.automatic) (*it)->ytick.tics1.clear();
		if ((*it)->xtick.automatic) (*it)->xtick.tics1.clear();
	}
	InvalidateRect	(NULL);

	CRect rt;
	GetWindowRect(&rt);
	gcf.strut["pos"].buf[0] = rt.left;
	gcf.strut["pos"].buf[1] = rt.top;
	gcf.strut["pos"].buf[2] = rt.Width();
	gcf.strut["pos"].buf[3] = rt.Height();
	//For showvar.cpp--Update the data content view
	::SendMessage(GetHWND_WAVPLAY(), WM_APP + 0x2020, (WPARAM)&gcf, (LPARAM)"pos");
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
	SetGCF();
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

void CPlotDlg::SetGCF()
{
	CVar *pgcf = (CVar*)FindFigure(hDlg);
	if (pgcf)
		pctx->SetVar("gcf", pgcf);
}

void CPlotDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetGCF();
	edge.px1 = edge.px2 = -1;
	gcmp=point;
	CAxes *cax = CurrentPoint2CurrentAxis(&point);
//	UpdateRects(cax);
	if (axis_expanding) {ClickOn=0; return;}
	clickedPt = point;
	switch(ClickOn = GetMousePos(point)) // ClickOn indicates where mouse was click.
	{
	case RC_WAVEFORM:  //0x000f
		if (curRange != NO_SELECTION) // if there's previous selection
		{
			ShowStatusBar();
			for (size_t k=0; k<gcf.ax.size(); k++)
			{
				CRect rt(curRange.px1, gcf.ax[k]->axRect.top, curRange.px2+1, gcf.ax[k]->axRect.bottom+1);
				InvalidateRect(&rt);
			}
		}
		lbuttondownpoint.x = gcmp.x;
		curRange.reset();
		{
			CClientDC dc(hDlg);
			dc.CreatePen(PS_DOT, 1, RGB(255, 100, 0));
			dc.MoveTo(gcmp.x, cax->axRect.bottom-1);
			dc.LineTo(gcmp.x, cax->axRect.top+1); 
		}

		if (GetAudioSignal(cax, false).IsTimeSignal())
		{
			if (playCursor > 0) // if a playCursor was set, then reset here
			{
				for (size_t k = 0; k < gcf.ax.size(); k++)
				{
					CRect rt(playCursor - 1, gcf.ax[k]->axRect.top, playCursor + 1, gcf.ax[k]->axRect.bottom + 1);
					InvalidateRect(&rt);
				}
				playCursor = -1;
			}
			SetTimer(MOUSE_CURSOR_SETTING, 1000);
		}
		break;
	default:
		gcmp=CPoint(-1,-1);
		break;
	}
}

void CPlotDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect rt;
	if (axis_expanding | gcf.ax.empty()) {ClickOn=0; return;}
	CAxes *cax = CurrentPoint2CurrentAxis(&point);
	CAxes *pax = gcf.ax.front();
	if (curRange.px2<0) // if button up without mouse moving, reset
		curRange.reset();
	CSignals _sig;
	clickedPt.x=-999; clickedPt.y=-999;
	lastPtDrawn.x=-1; lastPtDrawn.y=-1;
	KillTimer(MOUSE_CURSOR_SETTING);
	switch(ClickOn)
	{
	case RC_WAVEFORM:  //0x000f
		if (point.x > pax->axRect.right || point.x < pax->axRect.left) // mouse was up outside axis
			cax = pax;
		rt.top = cax->axRect.top;
		rt.bottom = cax->axRect.bottom+1;
		rt.left = point.x-1; //default
		rt.right = point.x+1; //default
		if (point.x > cax->axRect.right) 
		{
			rt.left = curRange.px2-1;
			rt.right = curRange.px2 = cax->axRect.right;
		}
		else if (point.x < cax->axRect.left) 
		{
			rt.right = curRange.px1+1;
			rt.left = curRange.px1 = cax->axRect.left+1;
		}
		else
		{
			if (point.x > lbuttondownpoint.x) // moving right
			{
				curRange.px2 = point.x;
				if (edge.px1>curRange.px2) { //moved right and left
					rt.left = curRange.px2;
					rt.right = edge.px1+2; }
			}
			else
			{
				curRange.px1 = point.x;
				if (edge.px1>0 && edge.px1<curRange.px1) { //moved left and right
					rt.left = edge.px1-2;
					rt.right = curRange.px1;
				}
			}
		}
		if (curRange.px2-curRange.px1<=3) curRange.reset();
		InvalidateRect(&rt);
		selRange.tp1 = cax->pix2timepoint(curRange.px1); 
		selRange.tp2 = cax->pix2timepoint(curRange.px2); 
		if (ClickOn && gcf.ax.front()->hChild)
			OnMenu(IDM_SPECTRUM_INTERNAL);
		break;
	default:
		unsigned short mask = ClickOn & 0xff00;
		if ( mask==RC_SPECTRUM_XTICK || mask==RC_SPECTRUM_YTICK )
		{
			if ( (MoveOn & RC_SPECTRUM_XTICK) | (MoveOn & RC_SPECTRUM_YTICK) )	MoveOn = 0;
		}
		else if (mask==RC_SPECTRUM)
			if (1)// (MoveOn & RC_SPECTRUM)
			{
				ChangeColorSpecAx(CRect(((CAxes*)gcf.ax.front()->hChild)->axRect), MoveOn = (bool)0);
				InvalidateRect(NULL);
			}
			else
			{
//				UpdateRects(cax);
				for (int k=0; k<5; k++) InvalidateRect(&roct[k]);
			}
		break;
	}
	ClickOn = 0;
	gcmp=CPoint(-1,-1);

	for (vector<CAxes*>::iterator axt=gcf.ax.begin(); axt!=gcf.ax.end(); axt++) 
	{
		CAxes* paxFFT;
		if (paxFFT = (CAxes*)(*axt)->hChild) ShowSpectrum(paxFFT, *axt);
	}
}

void CPlotDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	static int count(0);
	double x, y;
	if (axis_expanding || gcf.ax.empty()) return;
	CAxes *cax = CurrentPoint2CurrentAxis(&point);
	unsigned short  mousePt = GetMousePos(point);
	CRect rt, rect2Invalidate;
	char buf[64], buf2[64], buf0[64];
	CPoint shift;
	CAxes *paxFFT = (CAxes*)gcf.ax.front()->hChild;
	KillTimer(MOUSE_CURSOR_SETTING);
	switch(mousePt & 0xff00) // spectrum axis has the priority over waveform axis 
	{
	case RC_SPECTRUM:
		sprintf(buf,"%.1fHz",paxFFT->pix2timepoint(point.x));
		paxFFT->GetCoordinate(point, x, y);
		if (gcf.ax.size()>1)
		{
			CAxes *paxFFT2 = (CAxes*)gcf.ax.back()->hChild;
			if (GetMousePosReAx(point, paxFFT2))
			{
				paxFFT2->GetCoordinate(point, x, y);
				paxFFT = paxFFT2;
			}
		}
		sprintf(buf2,"%.2f",y);
		sprintf(buf0, "(%s,%s)", buf, buf2);
		ShowStatusBar(CURSOR_LOCATION_SPECTRUM, buf0);
		KillTimer(CUR_MOUSE_POS);
		SetTimer(CUR_MOUSE_POS, 2000, NULL);
		if (!(ClickOn & RC_SPECTRUM)) break;
		if (lastPtDrawn.x>0 && lastPtDrawn.y>0) // moving while button down 
		{
			shift = point - lastPtDrawn;
			CRect axRectOld(paxFFT->axRect);
			CRect clientrt;
			GetClientRect(hDlg, &clientrt);
			paxFFT->axRect.MoveToXY(paxFFT->axRect.TopLeft()+shift); // new top left point is shifted
			paxFFT->pos.Set(clientrt, paxFFT->axRect);
		}
		rect2Invalidate = paxFFT->GetWholeRect();
		rect2Invalidate.top -= 15;
		rect2Invalidate.right += 20;
		if (shift.x!=0 || shift.y!=0)
			ChangeColorSpecAx(CRect(paxFFT->axRect), true);
		InvalidateRect(rect2Invalidate); 
		lastPtDrawn = point;
		break;
	case RC_SPECTRUM_YTICK:
	// If the y-axis of spectrum needs to be adjusted with mouse-dragging
	// add the code here.
		break;
	case RC_SPECTRUM_XTICK:
		if (!(ClickOn & RC_SPECTRUM_XTICK) & !(ClickOn & RC_SPECTRUM_YTICK)) break; 
		if (lastPtDrawn.x>0 && lastPtDrawn.y>0) // moving while button down 
		{
			shift = point - lastPtDrawn;
			if (shift.x!=0)
			{
				int k(iabs(shift.x));
				if (k>6 && k<15) k=7;
				else if (k>13) k=10;
				if (shift.x>0)	
				{
					for (; k>0; k--)	paxFFT->xlim[1] *= .95 ;
					paxFFT->xlim[1] = max(100, paxFFT->xlim[1]);
				}
				else  /* shift.x<0 */
				{
					for (; k>0; k--)	paxFFT->xlim[1] /= .95 ;
					paxFFT->xlim[1] = min(paxFFT->xlim[1], paxFFT->xlimFull[1]);
				}
				rect2Invalidate = paxFFT->GetWholeRect();
				rect2Invalidate.right += 5 + (int)(rect2Invalidate.Width()/10);
				InvalidateRect(rect2Invalidate); 
				paxFFT->setxticks();
			}
		}
		lastPtDrawn = point;
		break;
	}
	if (mousePt == 0x000f) // RC_WAVEFORM
	{
		for (size_t k=0; k<gcf.ax.size(); k++)
		{
			cax = gcf.ax[k];
			if (ClickOn & (unsigned short)128) lbuttondownpoint.x = cax->axRect.left;
			else if (ClickOn & (unsigned short)32) lbuttondownpoint.x = cax->axRect.right;
			if (ClickOn)
			{ // lbuttondownpoint.x is the x point when mouse was clicked
				rt.top = cax->axRect.top+1;
				rt.bottom = cax->axRect.bottom-1;
				if (point.x > lbuttondownpoint.x) // current position right side of the beginning point
				{
					if (edge.px1==-1) edge.px1 = lbuttondownpoint.x;
					curRange.px1 = lbuttondownpoint.x;
					curRange.px2 = point.x;
					edge.px2 = max(lastpoint.x, max(edge.px2, point.x));
					rt.right = edge.px2;

					// If move left-right and passed the beginning point,i.w., lbuttondownpoint, keep the edge.px1 in rt, so it can be properly redrawn
					if ( (point.x-lbuttondownpoint.x)*(point.x-edge.px1)>0) 
						rt.left = edge.px1-1;
					else
						rt.left = curRange.px1;	
					edge.px1 =  max( edge.px1, curRange.px2);
				}
				else if (point.x < lbuttondownpoint.x) // moving left
				{
					if (edge.px1==-1) edge.px1 = lbuttondownpoint.x;
					curRange.px2 = lbuttondownpoint.x;
					curRange.px1 = point.x;
					if (point.x>lastpoint.x) // moving left but just turned right 
						rt.left = lastpoint.x;
					else
						rt.left = curRange.px1;
					// If move right-left and passed the beginning point, i.e., lbuttondownpoint), keep the edge.px1 in rt, so it can be properly redrawn
					if ( (point.x-lbuttondownpoint.x)*(point.x-edge.px1)>0) 
						rt.right = edge.px1+1;
					else
						rt.right = curRange.px2;
					edge.px1 =  min(edge.px1, curRange.px1);
				}
				else 
					curRange.reset();
				if (curRange != NO_SELECTION)
					InvalidateRect(&rt);
				lastpoint = point;

			}
			else edge.px1 = -1;
			ShowStatusBar(FULL);
		}
		cax = CurrentPoint2CurrentAxis(&point);
		cax->GetCoordinate(point, sbinfo.xCur, sbinfo.yCur);
		sbinfo.xSelBegin = gcf.ax.back()->pix2timepoint(curRange.px1);
		sbinfo.xSelEnd = gcf.ax.back()->pix2timepoint(curRange.px2);
		ShowStatusBar(CURSOR_LOCATION);
//		KillTimer(CUR_MOUSE_POS);
//		SetTimer(CUR_MOUSE_POS, 2000, NULL);
		selRange.tp1 = cax->pix2timepoint(curRange.px1); 
		selRange.tp2 = cax->pix2timepoint(curRange.px2); 
	}
	if (mousePt==0) // outside ANY axis
	{ // clean the status of current location 
		::SendMessage (hStatusbar, SB_SETTEXT, 2, (LPARAM)"");
	}
}

//void CPlotDlg::SetGCF()
//{
//	if (hDlg!=GetForegroundWindow())	SetForegroundWindow(hDlg);
//	::PostMessage(GetHWND_GRAFFY(), WM_GCF_UPDATED, (WPARAM)&gcf, (LPARAM)hDlg);
//}

#define LOG(X) fprintf(fp,(X));

void CPlotDlg::setpbprogln()
{
	for (vector<CAxes*>::iterator axt=gcf.ax.begin(); axt!=gcf.ax.end(); axt++) 
	{
		CAxes *pax = *axt;
		InvalidateRect(CRect(playLoc0-1, pax->axRect.top, playLoc+1, pax->axRect.bottom),0);
	}
}

void CPlotDlg::OnSoundEvent(CVar *pvar, int code)
{//index is the number sent by CWavePlay as each playback block of data is played successfully
 //bufferlocation is current point location of the data block (not really useful on the application side)
 // Both arguments 0 means the playback device is opened and ready to go.
 // index=-1 means playback ended and device is being closed.

	CAxes *pax = gcf.ax.front();
	double dara, gara, tp = pax->pix2timepoint(2);
	switch (code)
	{
	case WOM_OPEN:
		playingIndex = 0;
		playLoc0 = playLoc = (playCursor>1) ? playCursor : (curRange == NO_SELECTION)? pax->axRect.left : curRange.px1;
		setpbprogln();
		::SendMessage(GetHWND_WAVPLAY(), WM__AUDIOEVENT, (WPARAM)pvar, (LPARAM)code); // sending to the dialog in showvar.cpp
		break;
	case WOM_CLOSE:
		playLoc = playLoc0 + pax->timepoint2pix(block);
		setpbprogln();
		playLoc0 = playLoc = -1;
		playing = paused = false;
		::SendMessage(GetHWND_WAVPLAY(), WM__AUDIOEVENT, (WPARAM)pvar, (LPARAM)code); // sending to the dialog in showvar.cpp
		break;
	case -1: // error
		break;
	default: // status updates
		gara = pvar->strut["remDurMS"].value();
		dara = pvar->strut["totalDurMS"].value();
		playingIndex++;
		if (playCursor > 0)
			playLoc = playCursor + pax->timepoint2pix(block*+playingIndex / 1000);
		else if (curRange == NO_SELECTION)
			playLoc = pax->timepoint2pix(pax->xlim[0] + block*+playingIndex / 1000);
		else
		{
			playLoc = pax->timepoint2pix(selRange.tp1 + block*+playingIndex / 1000);
			// if playLoc goes out of range, invalidateRect only to erase playLoc0, i.e., don't update the screen with the newly advanced but out-of-range bar
			if (playLoc > curRange.px2)
			{
				InvalidateRect(CRect(playLoc0 - 1, pax->axRect.top, playLoc0 + 1, pax->axRect.bottom), 0);
				playLoc0 = playLoc = -1;
				playing = paused = false;
				::SendMessage(GetHWND_WAVPLAY(), WM_APP + 0x2300, 0, -1);
				return;
			}
		}
		setpbprogln();
		playLoc0 = playLoc;
	}
}

void CPlotDlg::ChangeColorSpecAx(CRect rt, bool onoff)
{// on: ready to move, off: movind done
	CAxes* paxFFT = (CAxes*)gcf.ax.front()->hChild;
	static COLORREF col1, col2;
	if (!onoff)
	{
		paxFFT->color = col2;
		col1 = col2;
	}
	else if (col1 == col2)
	{
		BYTE r = GetRValue(paxFFT->color);
		BYTE g = GetGValue(paxFFT->color);
		BYTE b = GetBValue(paxFFT->color);
		col2 = paxFFT->color;
		col1 = paxFFT->color = RGB(r*7/8,g*7/8,b*7/8);
	}
	InvalidateRect(&rt);
}

void CPlotDlg::OnTimer(UINT id)
{
	switch (id)
	{
	case MOUSE_CURSOR_SETTING:
		KillTimer(id);
		{
			CAxes *cax = CurrentPoint2CurrentAxis(&gcmp);
			CRect rt(cax->axRect);
			rt.left = rt.right = playCursor = gcmp.x;
			rt.left--; rt.right++;
			InvalidateRect(&rt);
			char buf[16];
			sprintf(buf, "%.3fs", cax->pix2timepoint(playCursor));
			::SendMessage(hStatusbar, SB_SETTEXT, 4, (LPARAM)buf);
		}
		break;

	case CUR_MOUSE_POS:
		::SendMessage (hStatusbar, SB_SETTEXT, 4, (LPARAM)"");
		::SendMessage (hStatusbar, SB_SETTEXT, 5, (LPARAM)"");
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
			CRect rt(ax->axRect.left, ax->axRect.top, ax->axRect.right, ax->axRect.bottom);
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


void CPlotDlg::OnMenu(UINT nID)
{
	if (axis_expanding) return;
	char fullfname[MAX_PATH];
	char fname[MAX_PATH];
	CFileDlg fileDlg;
	CAxes *cax=NULL;
	if (!gcf.ax.empty())
		cax=gcf.ax.front(); // Following the convention
	CAxes *paxFFT;
	CRect rt;
	CSize sz;
	CFont editFont;
	CPosition pos;
	int  len, iMul(1);
	char errstr[256];
	CSignals _sig, _sig2play, chainlessed;
	CSignal dummy;
	char buf[64];
	int k(0), deltapixel;
	vector<CLine*> swappy;
	double shift, newlimit1,  newlimit2, deltaxlim, dval, _block;
	INT_PTR res;

	errstr[0]=0;
	CPosition SpecAxPos(.75, .6, .22, .35);
	switch (nID)
	{
	case IDM_CHANNEL_TOGGLE:
	{
		//moved to HookProc in showvar.cpp 1/31/2019
	}
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
		for (vector<CAxes*>::iterator axt=gcf.ax.begin(); axt!=gcf.ax.end(); axt++)
		{
			cax = *axt;
			memcpy(cax->xlim, cax->xlimFull, sizeof(double)*2);
			cax->xtick.tics1.clear(); cax->xtick.automatic = true;
			cax->ytick.tics1.clear(); cax->ytick.automatic = true;
			cax->struts["x"].front()->strut["auto"] = CSignals(true);
			cax->struts["y"].front()->strut["auto"] = CSignals(true);
			if (paxFFT = (CAxes*)cax->hChild) ShowSpectrum(paxFFT, cax);
			cax->struts["x"].front()->strut["lim"] = (CSignals)CSignal(cax->xlim, 2);
		}
		ShowStatusBar();
		InvalidateRect(NULL);
		return;

	case IDM_ZOOM_IN:
		if (cax->xlim[1] - cax->xlim[0]<0.009) 
			return; // no zoom for 5ms of less
	case IDM_ZOOM_OUT:
		for (vector<CAxes*>::iterator axt=gcf.ax.begin(); axt!=gcf.ax.end(); axt++) 
		{
			cax = *axt;
			if (nID == IDM_ZOOM_OUT && fabs(cax->xlim[1]-cax->xlimFull[1])<1.e-5 && fabs(cax->xlim[0]-cax->xlimFull[0])<1.e-5)
				return; // no unzoom if full length
			if (nID == IDM_ZOOM_IN && cax->m_ln.front()->sig.GetType()==CSIG_VECTOR)
			{
				double percentShown = 1. - ( (cax->xlim[0]-cax->xlimFull[0]) + (cax->xlimFull[1]-cax->xlim[1]) ) / (cax->xlimFull[1]-cax->xlimFull[0]);
				if ((len = (int)(percentShown * cax->m_ln.front()->sig.nSamples+.5))<=3)
					return; // no unzoom if full length
			}
			deltaxlim = cax->xlim[1]-cax->xlim[0];
			if (nID == IDM_ZOOM_IN)
				cax->xlim[0] += deltaxlim/4, 	cax->xlim[1] -= deltaxlim/4;
			else
			{
				cax->xlim[0] -= deltaxlim/2;
				cax->xlim[0] = MAX(cax->xlimFull[0], cax->xlim[0]);
				cax->xlim[1] += deltaxlim/2;
				cax->xlim[1] = MIN(cax->xlim[1], cax->xlimFull[1]);
			}
			if (!cax->m_ln.size()) break;
			cax->xtick.tics1.clear(); cax->xtick.automatic = true;
			cax->ytick.tics1.clear(); cax->ytick.automatic = true;
			cax->struts["x"].front()->strut["auto"] = CSignals(true);
			cax->struts["y"].front()->strut["auto"] = CSignals(true);
			if (paxFFT = (CAxes*)cax->hChild) ShowSpectrum(paxFFT, cax);
			cax->struts["x"].front()->strut["lim"] = (CSignals)CSignal(cax->xlim, 2);
		}
		ShowStatusBar();
		InvalidateRect(NULL);								
		return;
	case IDM_LEFT_STEP:
		iMul *= -1;
	case IDM_RIGHT_STEP:
		for (vector<CAxes*>::iterator axt=gcf.ax.begin(); axt!=gcf.ax.end(); axt++) 
		{
			cax = *axt;
			shift = (cax->xlim[1]-cax->xlim[0]) / 4;
			newlimit1 = cax->xlim[0] + shift*iMul; // only effective for IDM_LEFT_STEP
			if (newlimit1<cax->xlimFull[0]) 
				shift = cax->xlim[0] - cax->xlimFull[0];
			newlimit2 = cax->xlim[1] + shift*iMul; // only effective for IDM_RIGHT_STEP
			if (newlimit2>cax->xlimFull[1]) 
				shift = fabs(cax->xlim[1] - cax->xlimFull[1]);
			if (shift<0.001) return;
			cax->xlim[0] += shift*iMul;
			cax->xlim[1] += shift*iMul;
			// further adjusting lim[0] to xlimFull[0] (i.e., 0 for audio signals) is necessary to avoid in gengrids when re-generating xticks
			if ((cax->xlim[0]-cax->xlimFull[0])<1.e-6) 	cax->xlim[0] = cax->xlimFull[0];
			if ((cax->xlimFull[1]-cax->xlim[1])<1.e-6) 	cax->xlim[1] = cax->xlimFull[1]; // this one may not be necessary.
			//Assuming that the range is determined by the first line
			rt = cax->axRect;
			rt.InflateRect(-10,0,10,30);
			vector<double>::iterator it = cax->xtick.tics1.begin();
			double p, step = *(it + 1) - *it;
			
			cax->xtick.automatic = true;
			cax->struts["x"].front()->strut["auto"] = CSignals(true);
			cax->struts["y"].front()->strut["auto"] = CSignals(true);
			// tick step size is not changed during left/right stepping
			if (nID == IDM_RIGHT_STEP)
			{
				p = cax->xtick.tics1.front();
				cax->xtick.tics1.clear();
				for (; p - cax->xlim[1] < 1.e-6; p += step)
					if (p >= cax->xlim[0])	cax->xtick.tics1.push_back(p);
			}
			else
			{
				p = cax->xtick.tics1.back();
				cax->xtick.tics1.clear();
				vector<double> tp;
				for (; p- cax->xlim[0] > -1.e-6 ; p -= step)
					if (p <= cax->xlim[1])	tp.push_back(p);
				for (vector<double>::reverse_iterator rit = tp.rbegin(); rit != tp.rend(); rit++)
					cax->xtick.tics1.push_back(*rit);
			}
			sbinfo.xBegin = cax->xlim[0];
			sbinfo.xEnd = cax->xlim[1];
			ShowStatusBar();
			InvalidateRect(&rt, FALSE);
			if (paxFFT = (CAxes*)cax->hChild) ShowSpectrum(paxFFT, cax);
			cax->struts["x"].front()->strut["lim"] = (CSignals)CSignal(cax->xlim, 2);
		}
		InvalidateRect(NULL);								
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
		::SendMessage (hStatusbar, SB_SETTEXT, 4, (LPARAM)"");
		::SendMessage (hStatusbar, SB_SETTEXT, 5, (LPARAM)"");
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
						deltapixel = cax->axRect.right - cax->axRect.left;
						deltaxlim = cax->xlim[1] - cax->xlim[0];
					}
					else
					{
						deltapixel = cax->axRect.right - playCursor;
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
				_sig = GetAudioSignal();
				hAudio = _sig.PlayArray(devID, WM__AUDIOEVENT, hDlg, &block, errstr);
				if (!hAudio)
				{
					// PlayArray fails if waveOutOpen fails
				}
				else
				{
					AUD_PLAYBACK * p = (AUD_PLAYBACK*)hAudio;
					CVar *pplayObj = new CVar;
					pplayObj->SetValue((double)(INT_PTR)hAudio);
					pplayObj->strut["data"] = _sig;
					pplayObj->strut["type"] = string("audio_playback");
					pplayObj->strut["devID"] = CSignals((double)devID);
					pplayObj->strut["totalDurMS"] = CSignals(_sig.alldur());
					pplayObj->strut["remDurMS"] = CSignals(_sig.alldur());
					p->pvar = pplayObj;
				}
			}
			else
			{
				PauseResumePlay(hAudio, true);
				paused = false;
			}
			playing = true;
		}
		else // if playing, pause
		{
			PauseResumePlay(hAudio, false);
			paused = true;
			playing = false;
		}
		return;
	case IDM_STOP:
		paused = playing = false;
		res = StopPlay(hAudio, true); // quick stop
		playLoc = -1;
#endif
		return;
	case IDM_SPECTRUM:
		// at this point, sbinfo.vax can be used
		k = 0;
		if (gcf.ax.size()>1) SpecAxPos.height -= .02;
		for (auto Ax : gcf.ax)
		{
			if (Ax->m_ln.size() == 0) continue;
			if (!Ax->m_ln.front()->sig.IsTimeSignal()) break;
			if (!Ax->hChild)
			{
				if (k>0) //if second channel (stereo)
				{ SpecAxPos.y0 = .1; }
				Ax->hChild = paxFFT = Ax->create_child_axis(SpecAxPos);
				paxFFT->color = RGB(220, 220, 150);
				paxFFT->visible = true;
				ShowSpectrum(paxFFT,Ax);
			}
			else
			{
				CRect rt, rt2;
				GetClientRect(hDlg, &rt);
				paxFFT = (CAxes*)Ax->hChild;
				rt2=paxFFT->pos.GetRect(rt);
				rt2 =paxFFT->axRect;
				rt2.InflateRect(40,30);
				InvalidateRect(rt2);
				delete paxFFT; // do not call deleteObj(paxFFT); --> that is to clean up vectors and assumes CAxes object is a child (vector) of CFigure
				Ax->hChild=NULL;
			}
			k++;
		}
		return;
	case IDM_SPECTRUM_INTERNAL:
		return;
	case IDM_WAVWRITE:
#ifndef NO_PLAYSND
		fullfname[0]=0;
		fileDlg.InitFileDlg(hDlg, hInst, "");
		_sig = GetAudioSignal();
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
	}
	InvalidateRect(NULL);
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

void CPlotDlg::getFFTdata(CSignals *psig_mag, CAxes *pax, double *fft, double *freq, int len)
{ // output: psig_mag
	int lenFFT = (len + 1) / 2;
	psig_mag->UpdateBuffer(len / 2 + 1);
	for (int k = 1; k < (len + 1) / 2; ++k)  /* (k < N/2 rounded up) */
		psig_mag->buf[k] = 20.*log10(fft[k] * fft[k] + fft[len - k] * fft[len - k]);
	if (len % 2 == 0) /* N is even */
		psig_mag->buf[len / 2] = 20.*log10(fft[len / 2] * fft[len / 2]);  /* Nyquist freq. */
	double maxmag = psig_mag->_max();
	for (int j = 0; j<lenFFT; j++)	psig_mag->buf[j] -= maxmag;
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
	_sig = GetAudioSignal(paxBase);
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
		dc.TextOut(pax->axRect.left, (pax->axRect.bottom-pax->axRect.top)/2, "Selection too short");
		delete[] dotted;
		return;
	}
	freq = new double[len];
	fft = new double[len];
	for (int k=0; k<len; k++)		freq[k]=(double)k/(double)len*dfs;
	plan = fftw_plan_r2r_1d(len, _sig.buf, fft, FFTW_R2HC, FFTW_ESTIMATE);
	fftw_execute(plan);
	CSignals mag;
	getFFTdata(&mag, pax, fft, freq, len);
	if (_sig.next)
	{//stereo
		plan = fftw_plan_r2r_1d(len, _sig.next->buf, fft, FFTW_R2HC, FFTW_ESTIMATE);
		fftw_execute(plan);
		mag.next = new CTimeSeries; // this will be deleted during the cleanup of mag... really?
		getFFTdata((CSignals*)mag.next, pax, fft, freq, len);
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
	CRect rt;
	GetClientRect(hDlg, &rt);
	pax->axRect = pax->pos.GetRect(rt);
	InvalidateRect(pax->axRect);
#endif
}

int IsInsideRect(RECT* rect, POINT* pt)
{
	return (pt->x >= rect->left && pt->x <= rect->right && pt->y >= rect->top && pt->y <= rect->bottom);
}

CAxes * CPlotDlg::CurrentPoint2CurrentAxis(CPoint *point)
{
	for (int k((int)gcf.ax.size()-1); k>=0; k--) //reason for decreasing: when the spectrum axis is clicked, that should be gca even if that overlaps with signal axis
	{
		if (IsInsideRect(gcf.ax[k]->axRect, point))
		{
			gca = gcf.ax[k];
			return gcf.ax[k];
		}
	}
	return NULL;
}

void CPlotDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	char buf1[64], buf2[64], buf3[64];
	switch(nChar)
	{
	case VK_CONTROL:
		if (ClickOn) ClickOn=0, OnLButtonUp(0, CPoint(-1,-1));
		if (playing) OnMenu(IDM_STOP);
		if (axis_expanding)
			strcpy(buf1,""), strcpy(buf2,""), strcpy(buf3,""); 
		else
			strcpy(buf1,"Adjust"), strcpy(buf2,"Window Size with"), strcpy(buf3,"Mouse"); 
		axis_expanding = !axis_expanding;
		::SendMessage (hStatusbar, SB_SETTEXT, 7, (LPARAM)buf1);
		::SendMessage (hStatusbar, SB_SETTEXT, 8, (LPARAM)buf2);
		::SendMessage (hStatusbar, SB_SETTEXT, 10, (LPARAM)buf3);
		InvalidateRect(NULL);
		break;
	}
}

void CPlotDlg::UpdateRects(CAxes *ax)
{
	if (ax==NULL) return;
	CRect rt0(ax->axRect);

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

void CPlotDlg::OnActivate(UINT state, HWND hwndActDeact, BOOL fMinimized)
{
	hPlotDlgCurrent = hDlg;
	//hQuickSolidBrush = new CBrush[gca->nLines];
	//for (int i=0; i<gca->nLines;i++)
	//	hQuickSolidBrush[i].CreateSolidBrush (gca->m_ln[i]->color);

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
	CRect rt0, rt;
	CAxes *pax;
	char buf[128];
	GetClientRect(hDlg, &rt0);
	//FILE *fp = fopen("WMlog.txt","at");
 //   SYSTEMTIME lt;
 //   GetLocalTime(&lt);	
	if (ClickOn)
	{
		pax = gcf.ax.front();
		rt.top = pax->axRect.top;
		rt.bottom = pax->axRect.bottom+1;
		if (pax->axRect.right - curRange.px2 < curRange.px1 - pax->axRect.left) // toward right
		{
			rt.left = curRange.px2-1;
			rt.right = curRange.px2 = pax->axRect.right;
			sprintf(buf,"%.3f",pax->pix2timepoint(curRange.px2));
			::SendMessage (hStatusbar, SB_SETTEXT, 7, (LPARAM)buf);
		}
		else
		{
			rt.right = curRange.px1+1;
			rt.left = curRange.px1 = pax->axRect.left;
			sprintf(buf,"%.3f",pax->pix2timepoint(curRange.px1));
			::SendMessage (hStatusbar, SB_SETTEXT, 6, (LPARAM)buf);
		}
		InvalidateRect(&rt);
		//fprintf(fp,"[%02d:%02d:%02d:%02d] msg=%x, ClickOn=%d\n", lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds, umsg, ClickOn);
	}
	ClickOn=0;
    //GetLocalTime(&lt);	
	//if (lParam!=0) fprintf(fp,"NCHITTEST res = %d  ", (LRESULT)lParam);
	//fprintf(fp,"[%02d:%02d:%02d:%02d] msg=%x, ClickOn=%d\n", lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds, umsg, ClickOn);
	//fclose(fp);
}

void CPlotDlg::ShowStatusBar(SHOWSTATUS status, const char* msg)
{
	char buf[64]={}, buf2[64];
	string rmstring;

	if (status==CURSOR_LOCATION)
	{
		for (vector<CAxes *>::iterator it = sbinfo.vax.begin(); it != sbinfo.vax.end(); it++)
		{
			if ((*it)->m_ln.size() > 0)
			{
				if ((*it)->m_ln[0]->sig.IsTimeSignal())
					sprintf(buf, "(%.3fs,%.3f)", sbinfo.xCur, sbinfo.yCur);
				else
					sprintf(buf, "(%.3f,%.3f)", sbinfo.xCur, sbinfo.yCur);
				::SendMessage(hStatusbar, SB_SETTEXT, 2, (LPARAM)buf);
				return;
			}
		}
	}
	else if (status==CURSOR_LOCATION_SPECTRUM)
	{
		::SendMessage (hStatusbar, SB_SETTEXT, 2, (LPARAM)msg);
		return;
	}

	char rmstext[64]={};
	CSignals _sig = GetAudioSignal(NULL, false);
	if (_sig.GetType() == CSIG_AUDIO)
	{
		sformat(rmstring, "%.1f dB", _sig.RMS().buf[0]);
		if (_sig.next)
		{
			rmstring.insert(0, "(L)");
			rmstring += " (R)";
			sprintf(buf, "%.1f dB", _sig.next->RMS());
			rmstring += buf;
		}
		strcpy(rmstext, rmstring.c_str());
	}
	if (_sig.IsTimeSignal() && strlen(rmstext) > 0)
		strcat(rmstext, "RMS");
	::SendMessage (hStatusbar, SB_SETTEXT, 6, (LPARAM)rmstext);

	//From OnPaint()---maybe use predefined format for non-audio???
//	char *format = pax0->xtick.format[0]? pax0->xtick.format : "%.2f";
//	sprintf(buf, format, pax0->xlim[0]);
//	sprintf(buf2, format, pax0->xlim[1]);

	if (sbinfo.vax.empty())  return;
	
	sbinfo.xBegin = sbinfo.vax[0]->xlim[0];
	sbinfo.xEnd = sbinfo.vax[0]->xlim[1];
	sprintf(buf, "%.3f", sbinfo.xBegin); 
	sprintf(buf2, "%.3f", sbinfo.xEnd);
//	sprintf(buf3, "%.3f", sbinfo.xCur);
	if (_sig.IsTimeSignal()) // for now, assume that two axes have both same sig type (check only the last one)
	{	strcat(buf, "s"); strcat(buf2, "s");   }
	::SendMessage (hStatusbar, SB_SETTEXT, 0, (LPARAM)buf);
	::SendMessage (hStatusbar, SB_SETTEXT, 1, (LPARAM)buf2);
//	sprintf(buf, "(%s,%.3f)", buf3, sbinfo.yCur);
//	::SendMessage (hStatusbar, SB_SETTEXT, 2, (LPARAM)buf);
	if (curRange==NO_SELECTION)
	{buf[0]=0; buf2[0]=0;}
	else
	{ // for now, use the last one---assume that two axes have both same sig type 
		sprintf(buf, "%.3f", sbinfo.xSelBegin); 
		sprintf(buf2, "%.3f", sbinfo.xSelEnd);
		if (_sig.IsTimeSignal()) // for now, assume that two axes have both same sig type (check only the last one)
		{	strcat(buf, "s"); strcat(buf2, "s");  }
	}
	if (playCursor<0) // if play cursor is set, do not update selection range status bar
	{
		::SendMessage(hStatusbar, SB_SETTEXT, 4, (LPARAM)buf);
		::SendMessage(hStatusbar, SB_SETTEXT, 5, (LPARAM)buf2);
	}
}
/*
#define RMSDB(BUF,FORMAT1,FORMAT2,X) { double rms;	if ((rms=X)==-1.*std::numeric_limits<double>::infinity()) strcpy(BUF, FORMAT1); else sprintf(BUF, FORMAT2, rms); }

void CPlotDlg::ShowStatusSelectionOfRange(CAxes *pax, const char *swich)
{
	if (swich!=NULL && !strcmp(swich,"off")) 
	{
		::SendMessage (hStatusbar, SB_SETTEXT, 4, (LPARAM)"");
		::SendMessage (hStatusbar, SB_SETTEXT, 5, (LPARAM)"");
	}
	else
	{
		char buf[64], buf2[64];
		if (curRange != NO_SELECTION)
		{
			CSignals _sig = pax->m_ln[0]->sig;
//			if (_sig.GetType()==CSIG_AUDIO)
//			{
				if (_sig.IsLogical())	strcpy(buf, "logical");
				else
				{
					_sig = GetAudioSignal();
//					if (_sig.next) RMSDB(buf2, "-Inf dB", "%.1f dB RMS", _sig.next->RMS())

					if (_sig.buf[0] == -1.*std::numeric_limits<double>::infinity()) strcpy(buf, "-Inf dB");
					else sprintf(buf, "%.1f dB RMS", _sig.buf[0]);
				

//					RMSDB(buf, "-Inf dB", "%.1f dB RMS", _sig.RMS())

				}
					
//					if (gcf.ax.size()==1)
//					{
//						RMSDB(RMSselected,"-Inf dB","%.1f dB monoRMS",_sig.RMS())
//					}
//					else // just assume that only stereo (2 channels)
//					{
//						char LR[3];
//						strcpy(LR,"LR");
//						RMSselected[0]=0;
//						for (size_t k=0; k<gcf.ax.size(); k++)
//						{
//							GetCSignalsInRange(1, gcf.ax[k], _sig, 0);
//							if (_sig.nSamples==0) strcpy(buf,"???");
//							else RMSDB(buf,"-Inf dB","%.1f dB RMS",_sig.RMS())
//							sprintf(buf2,"(%c)%s", LR[k], buf);
//							strcat(RMSselected,buf2);
//						}
//					}
					::SendMessage (hStatusbar, SB_SETTEXT, 6, (LPARAM)buf);
				sprintf(buf,"%.3fs",pax->pix2timepoint(curRange.px1));
				sprintf(buf2,"%.3fs",pax->pix2timepoint(curRange.px2));
//			}
//			else
//			{
//				sprintf(buf,"%.2f",pax->pix2timepoint(curRange.px1));
//				sprintf(buf2,"%.2f",pax->pix2timepoint(curRange.px2));
//			}
			::SendMessage (hStatusbar, SB_SETTEXT, 4, (LPARAM)buf);
			::SendMessage (hStatusbar, SB_SETTEXT, 5, (LPARAM)buf2);
		}

	}
}
*/