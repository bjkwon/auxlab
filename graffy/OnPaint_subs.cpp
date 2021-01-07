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

vector<POINT> CPlotDlg::drawCLine(CDC &dc, CLine * const pline, vector<POINT> & out)
{ 
	// For tseq, if you want to streamline, you can bypass estimateDrawCounts assuming that individual nSample for the p is 1
	// but it may require re-writing code ps more than you desire... something to think about 5/20/2020

	vector<POINT> drawvector;
	CPoint pt;
	CAxes* pax = (CAxes*)pline->hPar;
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
			if (pline->sig.nSamples > 0 && (pline->lineWidth > 0 || pline->symbol != 0))
			{
				if (pline->xdata.empty()) pline->xyplot = false;
				drawvector = pax->chain_in_CLine_to_POINTs(pline->xyplot, *p, m * p->nSamples, pline->xdata, xSpacingPP, out);
			}
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
//			if (!pax->limReady)
//				pax->limReady=true, memcpy(xrangeDlg, pax->xlim, sizeof(xlim));
			if (psig->type() & TYPEBIT_TEMPORAL || draw.size() > 2)
				pax->setxticks();
			else
			{
				for (auto v : draw)
				{
					double x1, y1;
					pax->GetCoordinate(&v, x1, y1);
					pax->xtick.tics1.push_back(pax->xlim[0]);
					pax->xtick.tics1.push_back((int)(x1 + .1));
					pax->xtick.tics1.push_back(pax->xlim[1]);
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
	if (!pax->m_ln.empty() || (!gcf.ax.empty() && !gcf.ax[0]->m_ln.empty() ))
	{
		sbinfo.xBegin = pax->xlim[0];
		sbinfo.xEnd = pax->xlim[1];
	}
}
