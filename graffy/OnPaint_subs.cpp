// AUXLAB 
//
// Copyright (c) 2009-2020 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: graffy
// Graphic Library (Windows only)
// 
// Version: 1.7
// Date: 5/24/2020

#include "PlotDlg.h"
#include <limits>
#include <mutex>
#include <thread>
#include <iterator>
#include "wavplay.h"

void CPlotDlg::OnPaintMouseMovingWhileClicked(CAxes* pax, CDC* pdc)
{
	CRect rt;
	if (curRange != NO_SELECTION && pax->hPar->type == GRAFFY_figure) // this applies only to waveform axis (not FFT axis)
	{
		rt = pax->rct;
		rt.left = curRange.px1 + 1;
		rt.right = curRange.px2 - 1;
		rt.top--;
		rt.bottom++;
		pdc->SolidFill(selColor, rt);
	}
}

static inline LONG getpointx(double x, const CRect &rcAx, double xlim[])
{
	double mapx = rcAx.Width() / (xlim[1] - xlim[0]);
	double _x = .5 + rcAx.left + mapx * (x - xlim[0]);
	LONG out = (LONG)_x;
	return out;
}
static inline LONG getpointy(double y, const CRect &rcAx, double ylim[])
{
	double mapy = rcAx.Height() / (ylim[1] - ylim[0]);
	double _y = rcAx.bottom - mapy * (y - ylim[0]);
	LONG out = (LONG)_y;
	return out;
}

static inline POINT getpoint(double x, double y, const CRect &rcAx, double ratiox, double ratioy, double xlim[], double ylim[])
{
	//convert (x,y) in double to point in rcAx
	double _x = .5 + rcAx.left + ratiox * (x - xlim[0]);
	double _y = .5 + rcAx.bottom - ratioy * (y - ylim[0]);
	POINT out = { (LONG)_x, (LONG)_y };
	return out;
}

static pair<map<double, double>::const_iterator, map<double, double>::const_iterator> 
get_inside_xlim(int &count, const map<double, double> &data, double xlim[])
{
	pair<map<double, double>::const_iterator, map<double, double>::const_iterator> out;
	auto it = data.begin();
	//Assumption: xlim is monotonically increasing
	for (; it != data.end() && (*it).first <= xlim[1]; it++)
	{
		if ((*it).first < xlim[0]) continue;
		else
		{
			out.first = it;
			break;
		}
	}
	count = 0;
	for (; it != data.end(); it++)
	{
		if ((*it).first <= xlim[1])
		{
			count++;
			continue;
		}
		else
		{
			out.second = it;
			break;
		}
	}
	if (it==data.end())
		out.second = it;
	return out;
}

// Assumption: buf and xlim are monotonically increasing
rangepair get_inside_xlim(int& count, const vector<double> &buf, double xlim[])
{
	rangepair out;
	//xlim is monotonically increasing
	out.first = lower_bound(buf.begin(), buf.end(), xlim[0]);
	out.second = lower_bound(buf.begin(), buf.end(), xlim[1]);
	count = (int)(out.second - out.first);
	return out;
}

static vector<POINT> data2points(double &xSpacingPP, const vector<double>& xbuf, double *buf, const CRect& rcArea, double xlim[], double ylim[])
{
	// xSpacingPP: [output] the x spacing between data points in pixel
	// grab data in the range of xlim
	// plot them in the coordinate of rcArea
	vector<POINT> out;
	POINT pt;
	// Inspect data and find out where the key is within xlim
	// Assume the key of data is ordered.--> Isn't it what map is about?
	int count;
	rangepair range = get_inside_xlim(count, xbuf, xlim);
	// if xbuf is out of xlim, return empty
	if (count == 0) return out;
	// calculate how many data points one pixel represents 
	LONG px1 = getpointx(*range.first, rcArea, xlim);
	LONG px2 = getpointx(*(range.second-1), rcArea, xlim);
	LONG pxdiff = px2 - px1;
	double dataCount_per_pixel = (double)count / pxdiff;
	xSpacingPP = 1. / dataCount_per_pixel;
	int idataCount_per_pixel = (int)dataCount_per_pixel;
	const double remainder = dataCount_per_pixel - idataCount_per_pixel;
	double leftover = remainder;
	if (dataCount_per_pixel > 4)
	{
		int cnt = 0;
		auto it = range.first;
		double *buf_pos = buf + distance(xbuf.begin(), range.first);
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
			LONG temp = pt.y = getpointy(*minmax.first, rcArea, ylim);
			if (pt.y > rcArea.bottom)
				temp = pt.y = rcArea.bottom;
			out.push_back(pt);
			pt.y = getpointy(*minmax.second, rcArea, ylim);
			pt.y = max(pt.y, rcArea.top);
			if (pt.y!=temp)
				out.push_back(pt);
			advance(it, advance_count);
			buf_pos += advance_count;
			cum += advance_count;
			if (pt.x >= px2 || cum >= count)
				loop = false;
			leftover += remainder;
			pt.x++;
		}
	}
	else
	{
		double *buf_pos = buf + distance(xbuf.begin(), range.first);
		double ratiox = rcArea.Width() / (xlim[1] - xlim[0]);
		double ratioy = rcArea.Height() / (ylim[1] - ylim[0]);
		for (auto it = range.first; it != range.second; it++, buf_pos++)
		{
			// Map data point into point in RECT
			pt = getpoint(*it, *buf_pos, rcArea, ratiox, ratioy, xlim, ylim);
			out.push_back(pt);
		}
	}
	return out;
}

// Keep this for the case of plot(x,y) with x not monotonically increasing.
static vector<POINT> data2points(const map<double, double> &data, const CRect & rcArea, double xlim[], double ylim[])
{
	// grab data in the range of xlim
	// plot them in the coordinate of rcArea
	vector<POINT> out;
	POINT pt;
	// Inspect data and find out where the key is within xlim
	// Assume the key of data is ordered.--> Isn't it what map is about?
	int count;
	pair<map<double, double>::const_iterator, map<double, double>::const_iterator> 
		range = get_inside_xlim(count, data, xlim);
	// calculate how many data points one pixel represents 
	double dataCount_per_pixel = (double)count / rcArea.Width();
	if (dataCount_per_pixel > 5)
	{
//		DO THIS AGAIN
	}
	else
	{
		double ratiox = rcArea.Width() / (xlim[1] - xlim[0]);
		double ratioy = rcArea.Height() / (ylim[1] - ylim[0]);
		for (auto it = range.first; it != range.second; it++)
		{
			// Map data point into point in RECT
			pt = getpoint((*it).first, (*it).second, rcArea, ratiox, ratioy, xlim, ylim);
			out.push_back(pt);
		}
	}
	return out;
}

// if it is audio, or null-x, plot, forget about map. Use this.
vector<POINT> CPlotDlg::plotpoints(double &xSpacingPP, CAxes *pax, const vector<double> &xbuf, const CSignal &p, unsigned int begin)
{
	vector<POINT> out;
	int fs = p.GetFs();
	//map<double, double> in;
	//for (unsigned int k = 0; k < p->nSamples; k++)
	//{
	//	double t = (double)k / fs;
	//	in[t] = p->buf[k];
	//}
	//out = data2points(in, pax->rct, pax->xlim, pax->ylim);
	;
//	vector<double> buf(p->buf + begin, p->buf + end);
	vector<double> _xbuf;
	if (xbuf.empty())
	{
		int offset = 1;
		if (p.type() & TYPEBIT_TEMPORAL) offset--;
		for (unsigned int k = offset; k < p.nSamples + offset; k++)
		{
			double t = (double)k / fs + p.tmark / 1000.;
			_xbuf.push_back(t);
		}
		out = data2points(xSpacingPP, _xbuf, p.buf + begin, pax->rct, pax->xlim, pax->ylim);
	}
	else
		out = data2points(xSpacingPP, xbuf, p.buf + begin, pax->rct, pax->xlim, pax->ylim);
	return out;
}

vector<DWORD> color_scheme(COLORREF linecolor, const CVar &linecolorProp, unsigned int nGroups)
{
	vector<DWORD> out;
	BYTE clcode = HIBYTE(HIWORD(linecolor));
	if (clcode == 'L' || clcode == 'R')
		out = Colormap(clcode, clcode, 'r', nGroups);
	if (clcode == 'l' || clcode == 'r')
		out = Colormap(clcode, clcode + 'R' - 'r', 'c', nGroups);
	else if (clcode == 'M')
	{
		for (unsigned int k = 0; k < linecolorProp.nSamples; k += 3)
		{
			DWORD dw = RGB((int)(linecolorProp.buf[k] * 255.), (int)(linecolorProp.buf[k + 1] * 255.), (int)(linecolorProp.buf[k + 2] * 255.));
			out.push_back(dw);
		}
	}
	else
		out.push_back(linecolor);
	return out;
}

vector<POINT> CPlotDlg::drawCLine(CAxes* pax, CDC &dc, CLine *pline)
{
	// For tseq, if you want to streamline, you can bypass estimateDrawCounts assuming that individual nSample for the p is 1
	// but it may require re-writing code ps more than you desire... something to think about 5/20/2020

	vector<POINT> drawvector;
	CPoint pt;
	CPen* pPenOld = NULL;
	int nDraws = 0, estCount = 1;
	double xSpacingPP = (double)pax->rct.Width();
	for (CTimeSeries *p = &(pline->sig); p; p = p->chain)
	{
		// A chain may have multiple groups
		auto nGroups = p->nGroups;
		auto nSamples = p->nSamples;
		auto tmark = p->tmark;
		p->nSamples = p->Len();
		p->nGroups = 1;
		vector<DWORD> kolor = color_scheme(pline->color, pline->strut["color"], nGroups);
		auto colorIt = kolor.begin();
		for (unsigned int m = 0; m < nGroups; m++)
		{
			pline->color = *colorIt;
			if (p->type() & TYPEBIT_TEMPORAL && m > 0) // tmark incremental per Group
				p->tmark += 1000. * p->nSamples / p->GetFs();
			if(pline->sig.nSamples > 0 && (pline->lineWidth > 0 || pline->symbol != 0) )
				drawvector = plotpoints(xSpacingPP, pax, pline->xdata, *p, m * p->nSamples);
			if (drawvector.empty()) {
				if (!kolor.empty()) colorIt++;  
				continue;
			}
			if (pline->symbol != 0 || xSpacingPP > 2.)
			{
				bool tempchange = false;
				auto org_markersize = pline->markersize;
				if (xSpacingPP > 2. && !pline->symbol)
				{ // If zoomed in enough, show symbol always (filled circle)
					pline->symbol = 'o';
					pline->filled = true;
					tempchange = true;
					pline->markersize = 2;
				}
				LineStyle org = pline->lineStyle;
				pline->lineStyle = LineStyle_solid;
				if (pline->lineWidth == 0)
					pline->lineWidth = 1;
				OnPaint_createpen_with_linestyle(pline, dc, &pPenOld);
				DrawMarker(dc, pline, drawvector);
				pline->lineStyle = org;
				if (tempchange)
				{
					pline->symbol = pline->filled = 0;
					pline->markersize = org_markersize;
				}
			}
			CPen * ppen = OnPaint_createpen_with_linestyle(pline, dc, &pPenOld);
			if (pline->lineWidth > 0 && !drawvector.empty())
			{
				if (p->type() & TYPEBIT_TEMPORAL) {
					if (pt.y < pax->rct.top)  pt.y = pax->rct.top;
					if (pt.y > pax->rct.bottom) pt.y = pax->rct.bottom;
				}
				if (pline->lineStyle != LineStyle_noline)
					dc.Polyline(drawvector.data(), (int)drawvector.size());
				if (kolor.size() > 1)
				{
					colorIt++;
					if (colorIt == kolor.end())		colorIt = kolor.begin();
				}
				if (ppen)
				{
					dc.SelectObject(pPenOld);
					delete ppen;
				}
			}
		}
		p->nGroups = nGroups;
		p->tmark = tmark;
		p->nSamples = nSamples;
	}
	return drawvector;
}

CPen * CPlotDlg::OnPaint_createpen_with_linestyle(CLine* pln, CDC& dc, CPen** pOldPen)
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
	CPen* newPen = new CPen(penStyle | PS_GEOMETRIC, pln->lineWidth, &lb, 0, NULL);
	*pOldPen = (CPen*)dc.SelectObject(*newPen);
	return newPen;
}

vector<double> CPlotDlg::OnPaint_make_tics(CDC& dc, CAxes * pax, const vector<POINT> & draw)
{
	if (pax->m_ln.size() > 0)
	{
		//When there's nothing in axes.. bypass this part.. Otherwise,  gengrids will crash!! 10/5/2019
		CTimeSeries *psig = NULL;
		//if pax->m_ln has multiple lines and some of them are empty, it bypasses and continues until 
		// it finds a non-empty one. That first non-empty one is used to make tics
		for (auto lyne : pax->m_ln)
		{
			if (lyne->sig.type() > 0)
			{
				psig = &lyne->sig;
				break;
			}
		}
		if (pax->xtick.tics1.empty() && pax->xtick.automatic)
		{
			if (!pax->limReady)
				pax->limReady=true, memcpy(xlim, pax->xlim, sizeof(xlim));
			if (psig && psig->type() & TYPEBIT_TEMPORAL)
			{
				pax->xtick.tics1 = pax->gengrids('x', -3);
			}
			else
			{
				if (draw.size() > 2)
					pax->setxticks(xlim);
				else
				{
					for (auto v : draw)
					{
						double x1, y1;
						pax->GetCoordinate(&v, x1, y1);
						pax->xtick.tics1.push_back((int)(x1 + .1));
					}
				}
			}
		}
		if (pax->ytick.tics1.empty() && pax->ytick.automatic)
		{
			if (psig && psig->bufBlockSize == 1)
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
		dc.SetTextAlign(TA_RIGHT | TA_BOTTOM);
		if (!gcf.ax.empty() && !gcf.ax.front()->m_ln.empty())
		{
			if (IsSpectrumAxis(pax))
			{
				dc.TextOut(pax->rct.right - 3, pax->rct.bottom, "Hz");
				dc.TextOut(pax->rct.left - 3, pax->rct.top + 1, "dB");
			}
			else if (pax->m_ln.front()->sig.type() & TYPEBIT_TEMPORAL)
				dc.SetBkMode(TRANSPARENT), dc.TextOut(pax->rct.right - 3, pax->rct.bottom, "sec");
		} 
	}
	vector<double> out;
	out.push_back(pax->xlim[0]);
	out.push_back(pax->xlim[1]);
	return out;
}

void CPlotDlg::OnPaint_fill_sbinfo(CAxes* pax)
{
	if (!pax->m_ln.empty() || (!gcf.ax.empty() && !gcf.ax[1]->m_ln.empty() ))
	{
		sbinfo.xBegin = pax->xlim[0];
		sbinfo.xEnd = pax->xlim[1];
	}
}
