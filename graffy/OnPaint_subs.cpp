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
			//{
			//	int nOutOfAx = 0;
			//	for (int k = nDraws - 1; k > 0; k--)
			//	{
			//		if (draw[k].x > pax->rct.right) nOutOfAx++;
			//		else break;
			//	}
			//	dc.Polyline(draw, nDraws - nOutOfAx);
			//}
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
		if (pax->xtick.tics1.empty() && pax->xtick.automatic)
		{
			if (first)
				memcpy(xlim, pax->xlim, sizeof(xlim));
			if (pax->m_ln.front()->sig.IsTimeSignal())
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
			if (pax->m_ln.front()->sig.bufBlockSize == 1)
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