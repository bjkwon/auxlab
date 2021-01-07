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

static pair<vector<double>::const_iterator, vector<double>::const_iterator>
get_inside_xlim(int& count, const vector<double> &buf, double xlim[])
{
	pair<vector<double>::const_iterator, vector<double>::const_iterator> out;
	//Assumption: xlim is monotonically increasing
	auto it = buf.begin();
	for (; it != buf.end() && *it <= xlim[1]; it++)
	{
		if (*it < xlim[0]) continue;
		else
		{
			out.first = it;
			break;
		}
	}
	count = 0;
	for (; it != buf.end(); it++)
	{
		if (*it <= xlim[1])
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
	if (it == buf.end())
		out.second = --it;
	return out;
}



static vector<POINT> data2points(const vector<double>& xbuf, const vector<double> &buf, const CRect& rcArea, double xlim[], double ylim[])
{
	// grab data in the range of xlim
	// plot them in the coordinate of rcArea
	vector<POINT> out;
	POINT pt;
	// Inspect data and find out where the key is within xlim
	// Assume the key of data is ordered.--> Isn't it what map is about?
	int count;
	pair<vector<double>::const_iterator, vector<double>::const_iterator>
		range = get_inside_xlim(count, xbuf, xlim);
	// if xbuf is out of xlim, return empty
	if (count == 0) return out;
	// calculate how many data points one pixel represents 
	LONG px1 = getpointx(*range.first, rcArea, xlim);
	LONG px2 = getpointx(*range.second, rcArea, xlim);
	LONG pxdiff = px2 - px1;
	double dataCount_per_pixel = (double)count / pxdiff;
	int idataCount_per_pixel = (int)dataCount_per_pixel;
	const double remainder = dataCount_per_pixel - idataCount_per_pixel;
	double leftover = remainder;
	if (dataCount_per_pixel > 4)
	{
		int cnt = 0;
		auto it = range.first;
		auto it2 = buf.begin();
		advance(it2, distance(xbuf.begin(), it));
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
			pair<vector<double>::const_iterator, vector<double>::const_iterator>
				ul = minmax_element(it2, it2 + advance_count);
			LONG temp = pt.y = getpointy(*ul.first, rcArea, ylim);
			out.push_back(pt);
			pt.y = getpointy(*ul.second, rcArea, ylim);
			if (pt.y!=temp)
				out.push_back(pt);
			advance(it, advance_count);
			advance(it2, advance_count);
			cum += advance_count;
			if (pt.x >= px2 || cum >= count)
				loop = false;
			leftover += remainder;
			pt.x++;
		}
	}
	else
	{
		auto it2 = buf.begin();
		advance(it2, distance(xbuf.begin(), range.first));
		double ratiox = rcArea.Width() / (xlim[1] - xlim[0]);
		double ratioy = rcArea.Height() / (ylim[1] - ylim[0]);
		for (auto it = range.first; it != range.second; it++, it2++)
		{
			// Map data point into point in RECT
			pt = getpoint(*it, *it2, rcArea, ratiox, ratioy, xlim, ylim);
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
vector<POINT> CPlotDlg::plotpoints(const CSignal *p, CAxes *pax, CLine *lyne, CRect rcPaint)
{
	vector<POINT> out;
	int fs = p->GetFs();
	//map<double, double> in;
	//for (unsigned int k = 0; k < p->nSamples; k++)
	//{
	//	double t = (double)k / fs;
	//	in[t] = p->buf[k];
	//}
	//out = data2points(in, pax->rct, pax->xlim, pax->ylim);
	vector<double> xbuf;
	vector<double> buf(p->buf, p->buf + p->nSamples);
	for (unsigned int k = 0; k < p->nSamples; k++)
	{
		double t = (double)k / fs + p->tmark / 1000.;
		xbuf.push_back(t);
	}
	out = data2points(xbuf, buf, pax->rct, pax->xlim, pax->ylim);
	return out;
}

vector<POINT> CPlotDlg::OnPaint_drawblock(CAxes* pax, CDC &dc, PAINTSTRUCT* pps, CLine *pline, CTimeSeries* block)
{
	// For tseq, if you want to streamline, you can bypass estimateDrawCounts assuming that individual nSample for the block is 1
	// but it may require re-writing code blocks more than you desire... something to think about 5/20/2020

	vector<POINT> drawvector;
	CPoint pt;
	CPen* pPenOld = NULL;
	int nDraws = 0, estCount = 1;
	CPen* ppen = NULL;
	auto anSamples = block->nSamples;
	auto atuck = block->nGroups;
	auto atmark = block->tmark;
	BYTE clcode;
	vector<DWORD> kolor;
	clcode = HIBYTE(HIWORD(pline->color));
	if (clcode == 'L' || clcode == 'R')
		kolor = Colormap(clcode, clcode, 'r', atuck);
	if (clcode == 'l' || clcode == 'r')
		kolor = Colormap(clcode, clcode + 'R' - 'r', 'c', atuck);
	else if (clcode == 'M')
	{
		CVar cmap = pline->strut["color"];
		for (unsigned int k = 0; k < cmap.nSamples; k += 3)
		{
			DWORD dw = RGB((int)(cmap.buf[k] * 255.), (int)(cmap.buf[k + 1] * 255.), (int)(cmap.buf[k + 2] * 255.));
			kolor.push_back(dw);
		}
	}
	else
		kolor.push_back(pline->color);
	auto colorIt = kolor.begin();
	for (unsigned int m = 0; m < atuck; m++)
	{
		pline->color = *colorIt;
		block->nSamples = block->Len();
		block->nGroups = 1;
		memcpy(block->buf, block->buf + m * block->nSamples, block->bufBlockSize * block->nSamples);
		if (block->IsTimeSignal())
			block->tmark += 1000. * m * block->nSamples / block->GetFs();
		if (pline->lineWidth > 0 || pline->symbol != 0)
		{
			drawvector = plotpoints(block, pax, pline, (CRect)pps->rcPaint);
		}
		if (pline->symbol != 0)
		{
			LineStyle org = pline->lineStyle;
			pline->lineStyle = LineStyle_solid;
			if (pline->lineWidth == 0)
				pline->lineWidth = 1;
			OnPaint_createpen_with_linestyle(pline, dc, &pPenOld);
			DrawMarker(dc, pline, drawvector);
			pline->lineStyle = org;
		}
		ppen = OnPaint_createpen_with_linestyle(pline, dc, &pPenOld);
		if (pline->lineWidth > 0)
		{
			if (block->IsTimeSignal()) {
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
			block->nGroups = atuck;
			block->tmark = atmark;
			block->nSamples = anSamples;
			if (ppen)
			{
				dc.SelectObject(pPenOld);
				delete ppen;
			}
		}
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

vector<double> CPlotDlg::OnPaint_make_tics(CDC& dc, CAxes * pax, const vector<POINT> & draw, bool first)
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
			if (first)
				memcpy(xlim, pax->xlim, sizeof(xlim));
			if (psig && psig->IsTimeSignal())
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
			else if (pax->m_ln.front()->sig.IsTimeSignal())
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
