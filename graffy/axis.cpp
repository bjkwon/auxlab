// AUXLAB
//
// Copyright (c) 2009-2020 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: graffy
// Graphic Library (Windows only)
//
//
// Version: 1.7
// Date: 5/24/2020

#include <math.h>
#include "graffy.h"

#include "PlotDlg.h"

using namespace Win32xx;
#include <iterator>

static map<int, int> rpix, rpix2; // first number: pixel count, second number: number of divisions

void makeRPIX()
{
	rpix.clear();
	rpix2.clear();

	rpix[100] = 2;
	rpix[120] = 3;
	rpix[170] = 4;
	rpix[230] = 5;
	rpix[290] = 6;
	rpix[350] = 7;
	rpix[410] = 8;
	rpix[510] = 9;
	rpix[610] = 10;

	rpix2[40] = 2;
	rpix2[90] = 3;
	rpix2[145] = 4;
	rpix2[200] = 5;
	rpix2[260] = 6;
	rpix2[325] = 7;
	rpix2[375] = 8;
	rpix2[425] = 15;
	rpix2[475] = 19;
	rpix2[525] = 20;
}

double getMin(int len, double *x)
{
	double miin = 1.e100;
	for (int i = 0; i<len; i++)
	{
		if (x[i]<miin)
			if (x[i] != -std::numeric_limits<double>::infinity())
				miin = x[i];
	}
	return miin;
}

double getMax(int len, double *x)
{
	double maax = -1.e100;
	for (int i = 0; i<len; i++)
		if (x[i]>maax)
			if (x[i] != std::numeric_limits<double>::infinity())
				maax = x[i];
	return maax;
}

CAxis::CAxis(CWndDlg * base, CGobj* pParent)
{
	type = GRAFFY_axis;
	hPar = pParent;
	initGO(pParent);
	color = RGB(200, 210, 200); //default
	struts["parent"].push_back(pParent);
	vector<DWORD> cl(1, color);
	strut["color"] = COLORREF2CSignals(cl, CSignals());
	m_dlg = base;
}

CAxis::~CAxis()
{

}

void CAxis::initGO(void * _hpar)
{
	CGobj::initGO(_hpar);
	strut["type"] = std::string("axis");
	strut["xyz"] = std::string("");
	double lim[2] = { 0, -1, };
	strut["lim"] = CSignals(lim, 2);
	strut["fontname"] = std::string("(fontname)");
	strut["fontsize"] = CVar(-1.);
	strut["scale"] = std::string("linear");
	strut["tick"] = CVar(1); // empty with fs=1
	strut["tickdir"] = CVar(1); // empty with fs=1
	strut["ticklabel"] = std::string("");
	bool b = false;
	strut["grid"] = CSignals(&b, 1);
	b = true;
	strut["auto"] = CSignals(&b, 1);
}

CAxis& CAxis::operator=(const CAxis& rhs)
{
	if (this != &rhs)
	{
		CGobj::operator=(rhs);
		strut["xyz"] = ((CVar)rhs).strut["xyz"];
		double lim[2] = { 0, -1, };
		strut["fontname"] = ((CVar)rhs).strut["fontname"];
		strut["fontsize"] = ((CVar)rhs).strut["fontsize"];
		strut["scale"] = ((CVar)rhs).strut["scale"];
		strut["tick"] = ((CVar)rhs).strut["tick"];
		strut["tickdir"] = ((CVar)rhs).strut["tickdir"];
		strut["ticklabel"] = ((CVar)rhs).strut["ticklabel"];
		strut["grid"] = ((CVar)rhs).strut["grid"];
		strut["auto"] = ((CVar)rhs).strut["auto"];
	}
	return *this;
}

CAxes::CAxes()
{ // not used.... shouldn't be used. 12/3
}

GRAPHY_EXPORT CAxes::CAxes(CWndDlg * base, CGobj* pParent /*=NULL*/)
:colorAxis(0), xlimfixed(false), ylimfixed(false), belowMouse(false), limReady(false)
{
	type = GRAFFY_axes;
	initGO(pParent);
	color = DEFAULT_AXES_COLOR;
	ColorFFTAx = DEFAULT_FFT_AXES_COLOR;
	vector<DWORD> cl(1, color);
	strut["color"] = COLORREF2CSignals(cl, CSignals());
	struts["parent"].push_back(pParent);
	m_dlg = base;
	pos.x0=0.;
	pos.y0=0.;
	pos.width=0.;
	pos.height=0.;
	hPar=pParent;
	visible = true;
	xlim[0]=0.;
	ylim[0]=0.;
	xlim[1] = xlim[0]-1.; //reverse the large-small, so indicate uninitialized for lim
	ylim[1] = ylim[0]-1.; //reverse the large-small, so indicate uninitialized for lim
	xtick.hPar = ytick.hPar = this;
	xtick.m_dlg = base;
	ytick.m_dlg = base;
	m_ln.clear();
	hPar->child.push_back(this);
}

void CAxes::initGO(void * _hpar)
{
	CGobj::initGO(_hpar);
	bool b = true;
	strut["type"] = std::string("axes");
	strut["box"] = CSignals(&b, 1);
	CSignals empty;
	strut["sel"] = empty;
	strut["linewidth"] = CSignals(1.);
	CAxis *mor = new CAxis(m_dlg, this);
	struts["x"].push_back(mor);
	mor->SetValue((double)(INT_PTR)(void*)mor);
	CAxis *mor2 = new CAxis(m_dlg, this);
	struts["y"].push_back(mor2);
	mor2->SetValue((double)(INT_PTR)(void*)mor2);
	strut["nextplot"] = std::string("replace");
}

POINT CAxes::GetRef()
{
	POINT out = { rct.left , rct.top };
	return out;
}

CAxes::~CAxes()
{
	while (!m_ln.empty()) {
		delete m_ln.back();
		m_ln.pop_back();
	}
}

GRAPHY_EXPORT CAxes *CAxes::create_child_axis(CPosition pos)
{
	CAxes *newax = new CAxes(m_dlg, this);
	struts["children"].push_back(newax);
	newax->strut["color"] = strut["color"]; // to inherit the color scheme.
	newax->setPos(pos);
	hChild = newax;
	newax->hPar = this;
	newax->color = ColorFFTAx;
	return newax;
}

void CAxes::GetCoordinate(POINT* pt, double& x, double& y)
{
	int ix = pt->x - rct.left;
	int iy = rct.bottom - pt->y;
	double width = xlim[1]-xlim[0];
	double height = ylim[1]-ylim[0];
	x = ix*width/rct.Width() + xlim[0];
	y = iy*height/rct.Height() + ylim[0];
}

double qut(double lim[2], double d)
{
	double range = lim[1]-lim[0];
	return (d-lim[0])/range;
}

double CAxes::GetRangePixel(int x)
{
	double relativeVal = (double)(x - rct.left)/(double)rct.Width();
	double val = xlim[0] + relativeVal * (xlim[1]-xlim[0]);
	return val;
}

int CAxes::double2pixel(double a, char xy)
{
	double relativeVal;
	int extent; // in pixel
	// rct field, xlim, ylim must have been prepared prior to this call.
	switch(xy)
	{
	case 'x':
		relativeVal = qut(xlim, a);
		extent = rct.Width();
		return rct.left + (int)((double)extent*relativeVal+.5);
	case 'y':
		relativeVal = qut(ylim, a);
		extent = rct.Height();
		return rct.bottom - (int)((double)extent*relativeVal+.5);
	default:
		return -9999;
	}
}

vector<POINT> CAxes::CSignal2pixelPOINT(CSignal* px, const CSignal& y, unsigned int idBegin, unsigned int nSamples2Display)
{
	vector<POINT> out;
	double yval;
	for (unsigned int k = idBegin; k < idBegin + nSamples2Display; k++)
	{
		if (y.IsLogical())
			yval = y.logbuf[k] ? 1.0 : 0;
		else
			yval = y.buf[k];
		if (((CTimeSeries)y).IsTimeSignal())
			out.push_back(double2pixelpt(y.tmark / 1000. + (double)k / y.GetFs(), yval, NULL));
		else if (px)
			out.push_back(double2pixelpt(px->buf[k], yval, NULL));
		else
			out.push_back(double2pixelpt((double)k + 1, yval, NULL));
	}
	return out;
}

POINT CAxes::double2pixelpt(double x, double y, double *newxlim)
{
	// Returns pixel POINT from (x, y) coordinate in double.
	// calculates how many pixels the point advances rct.left and rct.top based on xlim, ylim
	// newxlim is used for audio signal chains (the one with null-signal(s) in the middle)
	POINT pt;
	if (newxlim==NULL) pt.x = double2pixel(x,'x');
	else {
		double ratio = (newxlim[1]-newxlim[0])/(xlim[1]-xlim[0]);
		pt.x = rct.left; // + GetOffsetPixel(x-xlim[0],'x', ratio);  //now,  how to proceed with ratio??
	}
	pt.y = double2pixel(y,'y');
	return pt;
}

GRAPHY_EXPORT CRect CAxes::GetWholeRect()
{ // Output: CRect of the whole axis area including xtick, ytick
	// Add xtick2 and ytick2 on the right and top sides when needed.
	CRect out(rct), outxy;
	outxy.UnionRect(xtick.rt,ytick.rt);
	out.UnionRect(out, outxy);
	return out;
}


GRAPHY_EXPORT void CAxes::DeleteLine(int index)
{
	if (index==-1)
		while(m_ln.size()>0)
			m_ln.pop_back();
	else
		m_ln.erase(m_ln.begin()+index);
	if (m_ln.size()==0)
	{
		xlim[1] = xlim[0]-1.;
		ylim[1] = ylim[0]-1.;
	}
}
//
//GRAPHY_EXPORT void CAxes::DeletePatch(int index)
//{
//	if (index==-1)
//		while(m_pat.size()>0)
//			m_pat.pop_back();
//	else
//		m_pat.erase(m_pat.begin()+index);
//	if (m_pat.size()==0)
//	{
//		xlim[1] = xlim[0]-1.;
//		ylim[1] = ylim[0]-1.;
//	}
//}

int CAxes::GetDivCount(char xy, int dimens)
{
	RECT rt;
	GetClientRect(m_dlg->hDlg, &rt);
	int width(rt.right-rt.left);
	int height(rt.bottom-rt.top);
	if (rpix.empty())	makeRPIX();
	int nTicks(0);
	map<int,int>::key_compare comp;
	map<int,int>::iterator it;
	switch(xy)
	{
	case 'x':
		if (dimens<0) dimens = (int) (width*pos.width+.5);
		it = rpix.begin();	it++;
		if (dimens > rpix.rbegin()->first)
			nTicks = rpix.rbegin()->second + (dimens-rpix.rbegin()->first)/110;
		else if (dimens <= (*it).first)
			nTicks = (*it).second;
		else
		{
			comp = rpix.key_comp();
			for (it = rpix.begin(); comp((*it).first, dimens); it++)
				;
			it--, nTicks = (*it).second;
		}
		break;
	case 'y':
		if (dimens<0) dimens = (int) (height*pos.height+.5); // rct is not updated yet (it is updated inside OnPaint)
		it = rpix2.begin();	it++;
		if (dimens > rpix2.rbegin()->first)
			nTicks = rpix2.rbegin()->second + (dimens-rpix2.rbegin()->first)/70;
		else if (dimens <= (*it).first)
			nTicks = (*it).second;
		else
		{
			comp = rpix2.key_comp();
			for (it = rpix2.begin(); comp((*it).first, dimens); it++)
				;
			it--, nTicks = (*it).second;
		}
	}
	return nTicks;
}

GRAPHY_EXPORT void CAxes::setRange(const char xy, double x1, double x2)
{
	if (xy == 'x')
	{
		xlim[0] = x1;
		xlim[1] = x2;
	}
	else if (xy == 'y')
	{
		ylim[0] = x1;
		ylim[1] = x2;
	}
}

GRAPHY_EXPORT void CAxes::setxlim()
{//to be called after CLine objects are prepared
	if (m_ln.empty()) return;
	CTimeSeries *psig = NULL;
	for (auto lyne : m_ln)
	{
		if (lyne->sig.type() > 0)
		{
			psig = &lyne->sig;
			break;
		}
	}
	if (!psig || !psig->nSamples) return;
	xlim[0] = std::numeric_limits<double>::infinity();
	xlim[1] = -std::numeric_limits<double>::infinity();
	for (auto line : m_ln)
	{
		if (!line->xdata.empty()) // case of no chain.
		{
			xlim[0] = min(*min_element(line->xdata.begin(), line->xdata.end()), xlim[0]) - 1;
			xlim[1] = max(*max_element(line->xdata.begin(), line->xdata.end()), xlim[1]) + 1;
		}
		else
		{
			if (psig->IsTimeSignal())
			{
				xlim[0] = 0;
				xlim[1] = max(line->sig.alldur() / 1000., xlim[1]); // milliseconds
			}
			else
			{
				xlim[1] = max(line->sig.alldur() / 1000. / line->sig.nGroups, xlim[1])+1;
				if (xlim[1] < 10)		xlim[0] = 0.5;
				else if (xlim[1]<100)		xlim[0] = 0.25;
				else 		xlim[0] = 0;
			}
		}
	}
	if (xlim[0] == xlim[1]) { xlim[0] -= .005;	xlim[1] += .005; }
//	memcpy(xlimFull, xlim, sizeof(xlim));
}

GRAPHY_EXPORT void CAxes::setylim()
{//to be called after CLine objects are prepared
	ylim[0] = std::numeric_limits<double>::infinity();
	ylim[1] = -std::numeric_limits<double>::infinity();
	bool ylim_adjustmore = false;
	for (size_t k = 0; k < m_ln.size(); k++)
		if (m_ln[k]->sig.GetType() == CSIG_AUDIO)
			ylim[0] = -1, ylim[1] = 1;
		else
		{
			if (m_ln[k]->sig.bufBlockSize == 1) ylim[0] = -.2, ylim[1] = 1.2;
			else
			{
				for (CTimeSeries *p = &m_ln[k]->sig; p; p = p->chain)
				{
					ylim[0] = min(ylim[0], p->_min().front());
					ylim[1] = max(ylim[1], p->_max().front());
				}
				ylim_adjustmore = m_ln[k]->sig.IsTimeSignal();
			}
		}
	if (ylim[0] == ylim[1]) { ylim[0] -= 1.;	ylim[1] += 1.; }
	else if (ylim_adjustmore)
	{
		double yRange = ylim[1] - ylim[0];
		ylim[0] -= yRange / 10;
		ylim[1] += yRange / 10;
	}
	ylimFull[0] = ylim[0]; ylimFull[1] = ylim[1];
}

GRAPHY_EXPORT CLine * CAxes::plot(double *xdata, const CTimeSeries &ydata, const std::string & vname, DWORD col, char cymbol, LineStyle ls)
{
	WORD colorcode = HIWORD(col);
	BYTE lo = LOBYTE(colorcode);
	BYTE hi = HIBYTE(colorcode);
	char buf[64] = {};
	bool complex = ydata.bufBlockSize == 16;
	CLine *in2, *in = new CLine(m_dlg, this);
	in->varname = vname;
	if (complex)
	{
		int fs = ydata.GetFs();
		in2 = new CLine(m_dlg, this);
		for (const CTimeSeries *p = &ydata; p; p = p->chain)
		{
			CTimeSeries tp1(fs), tp2(fs);
			tp1.UpdateBuffer(p->nSamples);
			tp2.UpdateBuffer(p->nSamples);
			tp1.tmark = tp2.tmark = p->tmark;
			for (unsigned int k = 0; k < p->nSamples; k++)
			{
				tp1.buf[k] = real(p->cbuf[k]);
				tp2.buf[k] = imag(p->cbuf[k]);
			}
			in->sig.AddChain(tp1);
			in2->sig.AddChain(tp2);
		}
	}
	else
	{
		in->strut["ydata"] = in->sig = ydata;
	}
	in->symbol = cymbol;
	if (!hi) // color specified
		in->color = col;
	else // color not specified, color code being sent to PlotDlg.cpp
		in->color = col;
	in->lineStyle = ls;
	m_ln.push_back(in);
	if (xdata)
		in->xdata = vector<double>(xdata, xdata + ydata.nSamples);
	setxlim();
	in->strut["xdata"] = CVar(CSignals(CSignal(in->xdata)));
	buf[0] = cymbol;
	in->strut["marker"] = std::string(buf);
	CSignals sig_color;
	vector<DWORD> cmap;
	if (hi) // color not specified, L or R specified.
		cmap = Colormap(hi, hi, 'r', ydata.nGroups);
	else
		cmap.push_back(col);
	in->strut["color"] = COLORREF2CSignals(cmap, sig_color);

	if (complex)
	{
		in2->symbol = cymbol;
		in2->color = RGB(GetBValue(col), GetRValue(col), GetGValue(col));
		in2->lineStyle = ls;
		m_ln.push_back(in2);
	}
	RECT rt;
	GetClientRect(m_dlg->hDlg, &rt);
	int width(rt.right-rt.left);
	int axWidth((int)((double)width*pos.width+.5));
	int height(rt.bottom-rt.top);
	int axHeight((int)((double)height*pos.height+.5));
	if (!ylimfixed)
		setylim();

	in->SetValue((double)(INT_PTR)in);
	in->struts["parent"].push_back(this);
	in->strut["markersize"].SetValue(in->markersize);
	in->strut["linestyle"] = in->GetLineStyleSymbol();
	struts["children"].push_back(in);
	strut["pos"].buf[0] = pos.x0;
	strut["pos"].buf[1] = pos.y0;
	strut["pos"].buf[2] = pos.width;
	strut["pos"].buf[3] = pos.height;
	struts["x"].front()->strut["fontsize"] = CVar(999.); // ???? where can I find this?
	struts["x"].front()->strut["tick"] = CVar(CSignals(CSignal(xtick.tics1)));
	struts["y"].front()->strut["tick"] = CVar(CSignals(CSignal(ytick.tics1)));
	struts["x"].front()->strut["lim"] = CVar(CSignals(xlim, 2));
	struts["y"].front()->strut["lim"] = CVar(CSignals(ylim, 2));
	struts["x"].front()->strut["xyz"] = std::string("x");
	struts["y"].front()->strut["xyz"] = std::string("y");
	return in; //This is the line object that was just created.
}

int CAxes::timepoint2pix(double timepoint)
{
	double proportion = (timepoint-xlim[0]) / (xlim[1]-xlim[0]);// time proportion re the displayed duration
	int pix_proportion = (int)((double)rct.Width() * proportion);
	return rct.left + pix_proportion;
}

double CAxes::pix2timepoint(int pix)
{
	double proportion; // time proportion re the displayed duration
	int ss=rct.left;
	int ss2=rct.Width();
	proportion = (double) (pix-rct.left) / (double)rct.Width();
	return xlim[0] + (xlim[1]-xlim[0])*proportion;
}

std::vector<double> rule1(double in[], int digone[], int *digHandled = NULL);
std::vector<double> rule2(double in[], int digone[], int *digHandled = NULL);
std::vector<double> MakeGridEdges(double in[2], int *dighand = NULL);
double tenpower(int n);
double granularize(double in);
std::vector<double> makeseq(double lim[], double anch, double step);

vector<double> CAxes::gengrids(char xy, int pprecision)
{
	int nDv = GetDivCount(xy, -1);
	nDv *= -1;
	return gengrids(xy, &nDv, &pprecision);
}

vector<double> CAxes::gengrids(char xy, int *pnDv, int *pdigh)
{	// nDv positive means strict division count; tenpow and width ignored
	// nDv negative means loose division count; based on the grid edges from MakeGridEdges(), it tries to divide into nDv segments,
	// but the priority is given that the step size is granularized by 10%, 20%, 25% or 50% of grid edge width; therefore,
	// it may not end up exactly nDv segments, but it should be close enough with one of those available steps
	std::vector<double> out, grids;
	bool dig2Specified(false);
	int dig2trimThis;
	if (pdigh) dig2Specified = true, dig2trimThis = *pdigh;
	double lim[2];
	if (xy == 'x')
		memcpy(lim, xlim, sizeof(lim));
	else
		memcpy(lim, ylim, sizeof(lim));
	grids = MakeGridEdges(lim, pdigh);
	double delta(grids.back() - grids.front());
	double delta0(lim[1] - lim[0]);
	int nDv;
	if (pnDv)
		nDv = *pnDv;
	else
		nDv = -GetDivCount(xy, -1);
	double step(delta / nDv);
	double step0(delta0 / nDv);
	int copynDV(nDv);
	if (nDv < 0)
	{
		nDv *= -1;
		step0 *= -1;
		step *= -1;
		if (dig2Specified)
		{
			if (tenpower(dig2trimThis) > step)
				step = tenpower(dig2trimThis);
			else
				step = granularize(step);
		}
		else
			step = granularize(step);
	}
	out = makeseq(lim, grids.front(), step);
	while ((double)out.size() / nDv > 1.5)
	{
		double step0Copy = step0;
		if (copynDV < 0)
		{
			if (dig2Specified)
			{
				if (tenpower(dig2trimThis) > step)
					step0 = tenpower(dig2trimThis);
				else
					step0 = granularize(step0);
			}
			else
				step0 = granularize(step0);
			if (step0Copy == step0) break;
		}
		out = makeseq(lim, grids.front(), step0);
	}
	return out;
}

CAxes& CAxes::operator=(const CAxes& rhs)
{
	if (this != &rhs)
	{
		CGobj::operator=(rhs);
		rct = rhs.rct;
		colorAxis = rhs.colorAxis;
		memcpy(xlim, rhs.xlim, sizeof(xlim));
		memcpy(ylim, rhs.ylim, sizeof(ylim));
		xtick = rhs.xtick;
		ytick = rhs.ytick;
		for (auto ln : rhs.m_ln)
		{
			CLine *tp = new CLine(m_dlg, this);
			*tp = *ln;
			m_ln.push_back(tp);
			struts["children"].push_back(tp);
		}
		strut["box"] = ((CVar)rhs).strut["box"];
		strut["linewidth"] = ((CVar)rhs).strut["linewidth"];
		strut["nextplot"] = ((CVar)rhs).strut["nextplot"];
		CAxis *mor = new CAxis(m_dlg, this);
		*mor = *(CAxis*)((CVar)rhs).struts["x"].front();
		struts["x"].push_back(mor);
		mor->SetValue((double)(INT_PTR)(void*)mor);
		CAxis *mor2 = new CAxis(m_dlg, this);
		*mor2 = *(CAxis*)((CVar)rhs).struts["y"].front();
		struts["y"].push_back(mor2);
		mor2->SetValue((double)(INT_PTR)(void*)mor2);
	}
	return *this;
}

static inline LONG getpointx(double x, const CRect& rcAx, double xlim[])
{
	double mapx = rcAx.Width() / (xlim[1] - xlim[0]);
	double _x = .5 + rcAx.left + mapx * (x - xlim[0]);
	LONG out = (LONG)_x;
	return out;
}
static inline LONG getpointy(double y, const CRect& rcAx, double ylim[])
{
	double mapy = rcAx.Height() / (ylim[1] - ylim[0]);
	double _y = rcAx.bottom - mapy * (y - ylim[0]);
	LONG out = (LONG)_y;
	return out;
}

// Map data point into point in RECT
static inline POINT getpoint(double x, double y, const CRect& rcAx, double ratiox, double ratioy, double xlim[], double ylim[])
{
	//convert (x,y) in double to point in rcAx
	double _x = .5 + rcAx.left + ratiox * (x - xlim[0]);
	double _y = .5 + rcAx.bottom - ratioy * (y - ylim[0]);
	POINT out = { (LONG)_x, (LONG)_y };
	return out;
}

// Assumption: buf and xlim are monotonically increasing
rangepair get_inside_xlim_monotonic(int &count, const vector<double>& buf, double xlim[])
{
	rangepair out;
	// if buf is not monotonic, the output is not meaningful, but count is important
	out.first = lower_bound(buf.begin(), buf.end(), xlim[0]);
	out.second = lower_bound(buf.begin(), buf.end(), xlim[1]);
	count = (int)(out.second - out.first);
	return out;
}

pair<double, double> get_inside_xlim_general(int& count, const vector<double>& buf, double xlim[])
{
	vector<double> temp;
	double miny(1.e100), maxy(-1.e100);
	temp.reserve(buf.size());
	count = 0;
	for (auto v : buf)
	{
		if (v >= xlim[0] && v < xlim[1])
		{
			count++;
			miny = min(miny, v);
			maxy = max(maxy, v);
		}
	}
	return make_pair(miny, maxy);
}

// if it is audio, or null-x, plot, forget about map. Use this.
vector<POINT> CAxes::chain_in_CLine_to_POINTs(bool xyplot, const CSignal& p, unsigned int begin, vector<double>& xbuf, double& xSpacingPP, vector<POINT>& out)
{
	if (xbuf.empty())
	{
		int offset = 1;
		if (p.type() & TYPEBIT_TEMPORAL) offset--;
		for (unsigned int k = offset; k < p.nSamples + offset; k++)
		{
			double t = (double)k / p.GetFs() + p.tmark / 1000.;
			xbuf.push_back(t);
		}
	}
	return data2points(xyplot, xbuf, p.buf + begin, xSpacingPP, out);
}

vector<POINT> CAxes::plot_points_compact(int count, LONG px1, LONG px2, vector<double>::const_iterator itx, const vector<double>& xbuf, double* const buf, double nData_p_pixel)
{ // for now, non x-y plot only
	vector<POINT> out;
	POINT pt;
	int idataCount_per_pixel = (int)nData_p_pixel;
	const double remainder = nData_p_pixel - idataCount_per_pixel;
	double leftover = remainder;
	int cnt = 0;
	double* buf_pos = buf + distance(xbuf.begin(), itx);
	size_t cum = 0;
	bool loop = true;
	pt.x = px1;
	while (loop)
	{
		size_t advance_count = idataCount_per_pixel;
		if (leftover > 1)
		{
			advance_count++;
			leftover--;
		}
		if (cum + advance_count > count) advance_count = count - cum;
		auto minmax = minmax_element(buf_pos, buf_pos + advance_count);
		LONG temp = pt.y = getpointy(*minmax.first, rct, ylim);
		if (pt.y > rct.bottom)
			temp = pt.y = rct.bottom;
		out.push_back(pt);
		pt.y = getpointy(*minmax.second, rct, ylim);
		pt.y = max(pt.y, rct.top);
		if (pt.y != temp)
			out.push_back(pt);
		advance(itx, advance_count);
		buf_pos += advance_count;
		cum += advance_count;
		if (pt.x >= px2 || cum >= count)
			loop = false;
		leftover += remainder;
		pt.x++;
	}
	return out;
}

vector<POINT> CAxes::plot_points_xyplot_beyond_range(const vector<double>& xbuf, double* const buf, double ratiox, double ratioy)
{
	vector<POINT> out;
	auto itx0 = xbuf.begin();
	auto itx1 = xbuf.end();
	double* buf_pos = buf + distance(xbuf.begin(), itx0);
	bool lastin = false;
	for (auto it = itx0; it != itx1; it++, buf_pos++)
	{
		// If the current point is between xlim, push it (CASE 1) 
		// If the current one is in, the next one should be in (CASE 2) because of the current one.
		// If the current one is in, the last one should be in (CASE 3) even if it was not in
		// if the last one and current one flanks xlim, push both current and last (CASE 4) 
		if (*it >= xlim[0] && *it <= xlim[1])
		{
			if (it != itx0 && !lastin)
			{ //(CASE 3) 
				out.push_back(getpoint(*(it - 1), *(buf_pos - 1), rct, ratiox, ratioy, xlim, ylim));
			}
			// (CASE 1) 
			out.push_back(getpoint(*it, *buf_pos, rct, ratiox, ratioy, xlim, ylim));
			lastin = true;
		}
		else if (lastin)
		{ // (CASE 2) 
			out.push_back(getpoint(*it, *buf_pos, rct, ratiox, ratioy, xlim, ylim));
			lastin = false;
		}
		else if (it != itx0 && (*it - xlim[0]) * (*(it - 1) - xlim[0]) < 0)
		{ // (CASE 4) 
			out.push_back(getpoint(*(it - 1), *(buf_pos - 1), rct, ratiox, ratioy, xlim, ylim));
			out.push_back(getpoint(*it, *buf_pos, rct, ratiox, ratioy, xlim, ylim));
			lastin = false;
		}
		else
			lastin = false;
	}
	return out;
}

vector<POINT> CAxes::data2points(bool xyplot, const vector<double>& xbuf, double* const buf, double& xSpacingPP, vector<POINT> &out_cumular)
{
	// xSpacingPP: [output] the x spacing between data points in pixel
	// grab data in the range of xlim
	// plot them in the coordinate of rcArea
	vector<POINT> out;
	// Inspect data and find out where the key is within xlim
	int count;
	double xlim1, xlim2;
	double dataCount_per_pixel = 1;
	LONG px1 = 0, px2 = 0;
	rangepair range;
	if (xyplot)
	{
		auto minmax = get_inside_xlim_general(count, xbuf, xlim);
		xlim1 = minmax.first;
		xlim2 = minmax.second;
		if (count == 0) return out; // if xbuf is out of xlim, return empty
	}
	else
	{
		range = get_inside_xlim_monotonic(count, xbuf, xlim);
		xlim1 = *range.first;
		if (range.first == range.second)
			xlim2 = xlim1;
		else
			xlim2 = *(range.second-1);
		if (count == 0) return out; // if xbuf is out of xlim, return empty
	// calculate how many data points one pixel represents 
		px1 = getpointx(xlim1, rct, xlim);
		px2 = getpointx(xlim2, rct, xlim);
		// if px1==px2 (i.e., there's only one point) dataCount_per_pixel doesn't matter as long as xSpacingPP is big to make a temporary symbol in drawCLine()
		dataCount_per_pixel = (px1 == px2) ? .001 : (double)count / (px2 - px1);
		xSpacingPP = 1. / dataCount_per_pixel;
		if (dataCount_per_pixel > 4)
		{// points are closed to each other enough to treat them as a chunk
			out = plot_points_compact(count, px1, px2, range.first, xbuf, buf, dataCount_per_pixel);
			for (auto p : out)
				out_cumular.push_back(p);
			return out;
		}
	}
	double ratiox = rct.Width() / (xlim[1] - xlim[0]);
	double ratioy = rct.Height() / (ylim[1] - ylim[0]);
	if (xyplot) {
		out = plot_points_xyplot_beyond_range(xbuf, buf, ratiox, ratioy);
	}
	else
	{
		double* buf_pos = buf + distance(xbuf.begin(), range.first);
		for (auto it = range.first; it != range.second; it++, buf_pos++)
			out.push_back(getpoint(*it, *buf_pos, rct, ratiox, ratioy, xlim, ylim));
	}
	for (auto p : out)
		out_cumular.push_back(p);
	return out;
}

static void make(double x1, double x2, uint16_t state, double& div, std::vector<double>& out)
{
	out.clear();
	if (state % 3 < 2)
		div /= 2;
	else
		div /= 2.5;
	double v = (int)ceil(x1 / div) * div;
	if (v != x1)
		out.push_back(x1);
	while (v < x2)
	{
		out.push_back(v);
		v += div;
	}
	out.push_back(x2);
}

std::vector<double> value_grid(double x1, double x2, int nReq)
{ // Generate a sequence between x1 and x2 with the count closest to nReq
  // 0 to 10 is divided with a width of 10 (one), 5 (two), 2 (five), 1 (ten)
  // The return value, the vector, has one of the following size 1, 2, 5, or 10
  // if x1 = 0, x2 = 10.
  // If requested size, nReq, is close to one of those, that's good to return
  // If not, continue with more divisions and try until the error increases in iteration
  // (if the error increases, take the previous case and return that result)

	std::vector<double> out, outlast;
	// divider is 1, 2 or 5 times a power of ten
	// always x2 > x1
	double x = fmax(fabs(x1), fabs(x2));
	double div = pow(10., (double)(int)log10(x) + 1);
	// div/2 is the division value to begin with inside make()
	int prev = nReq;
	for (int factor = 0, b = 1; b > 0; factor++)
	{
		make(x1, x2, factor, div, out);
		int error = abs(nReq - (int)out.size());
		if (error <= 2)
			b = 0;
		else if (error > prev && nReq/2 > prev && !outlast.empty())
		{ // exit check... Make sure this is fail-safe; otherwise an infinite loop
			out = outlast;
			b = 0;
		}
		else
		{
			prev = error;
			outlast = out;
		}
	}
	return out;
}

/* In an axes, all lines must be either non-temporal or all-temporal

When multiple lines exist, 
if it is non-temporal and non xyplot, all lines begin at 0 (or at the same point) but it may end at a different point.
--> Try to make xtick at the point where the x point the data exists.
If any one line is xyplot, each line may begin and end any time.
--> Let's not try aligning xtick with data point.

If it is temporal, any line can begin and end at any point.
--> No need to align xtick with data point.

*/
void CAxes::setxticks()
{ // xtics1 begins with xlim[0] and ends with xlim[1] and filled with the actual ticks
  // If the first or last actual ticks item overlaps with xlim[0] or xlim[1], do not duplicate
  // This rule is in effect in DrawTicks() in PlotDlg.cpp
	if (!xtick.automatic) return;
	int nTicks = GetDivCount('x', -1);
	assert(m_ln.size() > 0);
	int fs = m_ln.front()->sig.GetFs();
	vector<double> out;

	// Is multiple lines and any line is xyplot -- condition A
	bool xyp = false;
	for (auto v : m_ln)
		if (v->xyplot) {
			xyp = true;
			break;
		}
	if (m_ln.size()>1)
	{
		if (xyp)// condition A
			out = value_grid(xlim[0], xlim[1], nTicks);
		else // condition B: multiple lines, but all have the same begin point
		{
			out = value_grid(xlim[0], xlim[1], nTicks);
		}
	}
	else
	{
		out = value_grid(xlim[0], xlim[1], nTicks);
	}
	xtick.tics1 = out;
}

int headint(double x)
{
	if (x == 0) return 0;
	double lg = log10(x);
	if (x >= 1.)
		return (int)pow(10., lg - (int)lg);
	return (int)headint(pow(10., lg + 1 - ceil(lg)));
}

double remove_head(double v, int& lasthead)
{ // this removes the head. If it is <1, shift the head into the digit above decimal point.
	lasthead = 0;
	if (v == 0.) return 0.;
	if (v < 0) return remove_head(-v, lasthead);
	lasthead = headint(v);
	if (v > 1)
		return v - lasthead * pow(10., (int)log10(v));
	else
		return v - lasthead * pow(10., (int)log10(v) - 1);
}

int diff_digit(double v1, double v2)
{ // returns the digit (below the decimal point) where v1 and v2 differ
  // if v1 == v2, return -1
  // if v1 and v2 are in different digits, return -1
  // if it differs on a digit above the decimal point, return -1
	if (v1 == v2) return -1;
	if (v1 * v2 == 0.) return -1;
	if (v1 < 0) v1 = -v1;
	if (v2 < 0) v2 = -v2;
	int cum = (int)log10(v1);
	double mag1 = log10(v1);
	double mag2 = log10(v2);
	if (mag1 * mag2 <= 0.) return -1;
	if ((int)mag1 != (int)mag2) return -1;
	int head1 = 0;
	int head2 = 0;
	if (v1 >= 1.)
	{
		head1 = headint(v1);
		head2 = headint(v2);
	}
	else
		cum--;
	while (head1 == head2)
	{
		// Remove the head digit
		v1 = remove_head(v1, head1);
		v2 = remove_head(v2, head2);
		if (head1 != head2) break;
		cum--;
		if (head1 + head2 == 0) break;
	}
	return cum;
}

int CAxes::get_prec_xtick()
{
	if (xtick.tics1.size() < 2) return 0;
	int prec1 = 0x7FFFFFFF; // 32-bit MAX_MAX
	int prec2 = 0x80000000; // 32-bit MAX_MIN
	// if tics1 has more than two elements with greater than 1 and if they are 
	// analyze tics1 separately for > 1 and < 1
	for (auto it = xtick.tics1.rbegin() + 1; it != xtick.tics1.rend(); it++)
	{
		if (*it < 1) break;
		prec2 = max(prec2, diff_digit(*(it - 1), *it));
	}
	if (prec2 > 0) return prec2;
	for (auto it = xtick.tics1.begin() + 1; it != xtick.tics1.end(); it++)
	{
		prec1 = min(prec1, diff_digit(*(it - 1), *it));
	}
	return -prec1;
}
