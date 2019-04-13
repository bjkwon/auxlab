// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: graffy
// Graphic Library (Windows only)
// 
// 
// Version: 1.498
// Date: 2/4/2019
// 
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
	strut["type"] = CSignals(std::string("axis"));
	strut["xyz"] = CSignals((std::string)"");
	double lim[2] = { 0, -1, };
	strut["lim"] = lim;
	strut["fontname"] = CSignals((std::string)"(fontname)");
	strut["fontsize"] = CSignals(-1.);
	strut["scale"] = CSignals((std::string)"linear");
	strut["tick"] = CSignals(1); // empty with fs=1
	strut["tickdir"] = CSignals(1); // empty with fs=1
	strut["ticklabel"] = CSignals((std::string)"");
	strut["grid"] = CSignals(false);
	strut["auto"] = CSignals(true);
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
:colorAxis(0)
{ // not used.... shouldn't be used. 12/3
}

GRAPHY_EXPORT CAxes::CAxes(CWndDlg * base, CGobj* pParent /*=NULL*/)
:colorAxis(0)
{
	type = GRAFFY_axes;
	initGO(pParent);
	color = RGB(200, 210, 200); 
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
	strut["type"] = CSignals(std::string("axes"));
	strut["box"] = CSignals(true);
	CSignals empty;
	strut["sel"] = empty;
	strut["linewidth"] = CSignals(1.);
	CAxis *mor = new CAxis(m_dlg, this);
	struts["x"].push_back(mor);
	mor->SetValue((double)(INT_PTR)(void*)mor);
	CAxis *mor2 = new CAxis(m_dlg, this);
	struts["y"].push_back(mor2);
	mor2->SetValue((double)(INT_PTR)(void*)mor2);
	strut["nextplot"] = CSignals(std::string("replace"));
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

void CAxes::setxticks()
{
	int nTicks = GetDivCount('x', -1);
	int beginID, nSamples;
	assert(m_ln.size()>0);
	int fs = m_ln.front()->sig.GetFs();
	vector<unsigned int> out;
	vector<double> vxdata;
	double percentShown;
	if (!xtick.automatic) return;
	//nTicks is the number of divided parts (i.e., 2 means split by half)
	percentShown = 1. - ( (xlim[0]-xlimFull[0]) + (xlimFull[1]-xlim[1]) ) / (xlimFull[1]-xlimFull[0]);
	nSamples = (int)(percentShown * m_ln.front()->sig.nSamples+.5);
	splitevenindices(out, nSamples, nTicks);
	vxdata.reserve(nSamples);
	if (m_ln.front()->xdata.nSamples==0)
	{
		beginID = (int)ceil(xlim[0]*fs);
		for (int k=beginID; k<beginID+nSamples; k++) vxdata.push_back((double)(k)/fs); // no need to check whether it is an audio signal 
	}
	else
	{
		for (beginID=0; beginID<(int)m_ln.front()->sig.nSamples; beginID++)
			if (m_ln.front()->xdata.buf[beginID]>=xlim[0]) 
				break;
		for (int k=beginID; k<beginID+nSamples; k++) 
			vxdata.push_back(m_ln.front()->xdata.buf[k]);
	}
	xtick.set(out, vxdata, nSamples);
}

GRAPHY_EXPORT void CAxes::setxlim()
{//to be called after CLine objects are prepared
	xlim[0] = std::numeric_limits<double>::infinity();
	xlim[1] = -std::numeric_limits<double>::infinity();
	for (auto line : m_ln)
	{
		if (line->xdata.nSamples) // case of no chain.
		{
			xlim[0] = min(line->xdata._min(), xlim[0]) - 1;
			xlim[1] = max(line->xdata._max(), xlim[1]) + 1;
		}
		else
		{
			if (line->sig.IsTimeSignal())
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
	memcpy(xlimFull, xlim, sizeof(xlim));
}

GRAPHY_EXPORT void CAxes::setylim()
{//to be called after CLine objects are prepared
	ylim[0] = std::numeric_limits<double>::infinity();
	ylim[1] = -std::numeric_limits<double>::infinity();
	for (size_t k = 0; k < m_ln.size(); k++)
		if (m_ln[k]->sig.GetType() == CSIG_AUDIO)
			ylim[0] = -1, ylim[1] = 1;
		else
		{
			if (m_ln[k]->sig.bufBlockSize == 1) ylim[0] = -.2, ylim[1] = 1.2;
			else
			{
				ylim[0] = min(ylim[0], getMin(m_ln[k]->sig.nSamples * m_ln[k]->sig.bufBlockSize / 8, m_ln[k]->sig.buf));
				ylim[1] = max(ylim[1], getMax(m_ln[k]->sig.nSamples * m_ln[k]->sig.bufBlockSize / 8, m_ln[k]->sig.buf));
			}
		}
	if (ylim[0] == ylim[1]) { ylim[0] -= 1.;	ylim[1] += 1.; }
	ylimFull[0] = ylim[0]; ylimFull[1] = ylim[1];
}

GRAPHY_EXPORT CLine * CAxes::plot(double *xdata, CTimeSeries *pydata, DWORD col, char cymbol, LineStyle ls)
{
	WORD colorcode = HIWORD(col);
	BYTE lo = LOBYTE(colorcode);
	BYTE hi = HIBYTE(colorcode);
	char buf[64] = {};
	bool complex = pydata->bufBlockSize == 16;
	CLine *in2, *in = new CLine(m_dlg, this);
	if (complex)
	{
		int fs = pydata->GetFs();
		in2 = new CLine(m_dlg, this);
		for (CTimeSeries *p = pydata; p; p = p->chain)
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
		in->strut["ydata"] = in->sig = *pydata;
	}
	in->symbol = cymbol;
	if (!hi) // color specified
		in->color = col;
	else // color not specified, color code being sent to PlotDlg.cpp
		in->color = col;
	in->lineStyle = ls;
	m_ln.push_back(in);
	if (xdata)
		in->xdata = CSignal(xdata, pydata->nSamples);
	setxlim();
	in->strut["xdata"] = CSignals(in->xdata);
	buf[0] = cymbol;
	in->strut["marker"] = CSignals(std::string(buf));
	CSignals sig_color;
	vector<DWORD> cmap;
	if (hi) // color not specified, L or R specified.
		cmap = Colormap(hi, hi, 'r', pydata->nGroups);
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
	xlimFull[0] = xlim[0]; xlimFull[1] = xlim[1];
	RECT rt;
	GetClientRect(m_dlg->hDlg, &rt);
	int width(rt.right-rt.left);
	int axWidth((int)((double)width*pos.width+.5));
	int height(rt.bottom-rt.top);
	int axHeight((int)((double)height*pos.height+.5));
	setylim();

	// parent is Figure object, whose parent is root object whose member is m_dlg
	((CPlotDlg*)m_dlg)->ShowStatusBar();

	in->SetValue((double)(INT_PTR)in);
	in->struts["parent"].push_back(this);
	in->strut["markersize"].SetValue(in->markersize);
	in->strut["linestyle"] = in->GetLineStyleSymbol();
	struts["children"].push_back(in);
	strut["pos"].buf[0] = pos.x0;
	strut["pos"].buf[1] = pos.y0;
	strut["pos"].buf[2] = pos.width;
	strut["pos"].buf[3] = pos.height;
	struts["x"].front()->strut["fontsize"] = CSignals(999.); // ???? where can I find this?
	struts["x"].front()->strut["tick"] = CSignals(CSignal(xtick.tics1));
	struts["y"].front()->strut["tick"] = CSignals(CSignal(ytick.tics1));
	struts["x"].front()->strut["lim"] = CSignals(xlim, 2);
	struts["y"].front()->strut["lim"] = CSignals(ylim, 2);
	struts["x"].front()->strut["xyz"] = CSignals(std::string("x"));
	struts["y"].front()->strut["xyz"] = CSignals(std::string("y"));
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
		memcpy(xlimFull, rhs.xlimFull, sizeof(xlimFull));
		memcpy(ylimFull, rhs.ylimFull, sizeof(ylimFull));
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