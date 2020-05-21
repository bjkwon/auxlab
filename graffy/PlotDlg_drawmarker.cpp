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
// Change from 1.495: FileDlg.h dropped 
#include <math.h>
#include "PlotDlg.h"

#define PI 3.141592
#define C30DIV2 .433 // COS 30 degree div 2
#define C30DIV22 .866 // COS 30 degree 

void CPlotDlg::DrawMarker(CDC dc, CLine* mline, POINT * draw, int nDraws)
{
	if (!mline) return;
	int radius(mline->markersize);
	CRect circleRt;
	CPoint mark[16];
	POINT pt;
	int res;
	int q = 0;
	double angle;
	HBRUSH hBr;
	CPen *pen4marker = new CPen;
	HGDIOBJ hOrigBrush(NULL);
	if (mline->markerColor==-1) mline->markerColor = mline->color;
	pen4marker->CreatePen(PS_SOLID, 1, mline->markerColor);
	dc.SelectObject(pen4marker);
	switch (mline->symbol)
	{
	case 'o':
		if (mline->filled)
		{
			hBr = (HBRUSH)::GetStockObject(DC_BRUSH);
			SetDCBrushColor(dc.GetHDC(), mline->markerColor);
		}
		else
			hBr = (HBRUSH)::GetStockObject(NULL_BRUSH);
		hOrigBrush = ::SelectObject(dc.GetHDC(), hBr);
		for (auto k = 0; k < nDraws; k++)
		{
			circleRt.SetRect(CPoint(draw[k]) + CPoint(-radius, -radius), CPoint(draw[k]) + CPoint(radius, radius));
			dc.Ellipse(circleRt);
			if (k == 0)
				mline->rti = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
			if (k == nDraws - 1)
				mline->rtf = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
		}
		if (hOrigBrush) ::SelectObject(dc.GetHDC(), hOrigBrush);
		break;
	case 's':
		radius /= 2;
		if (mline->filled)
		{
			hBr = (HBRUSH)::GetStockObject(DC_BRUSH);
			SetDCBrushColor(dc.GetHDC(), mline->markerColor);
		}
		else
			hBr = (HBRUSH)::GetStockObject(NULL_BRUSH);
		hOrigBrush = ::SelectObject(dc.GetHDC(), hBr);
		for (auto k=0; k<nDraws; k++)
		{
			mark[q++] = CPoint(draw[k].x - radius, draw[k].y - radius);
			mark[q++] = CPoint(draw[k].x + radius, draw[k].y - radius);
			mark[q++] = CPoint(draw[k].x + radius, draw[k].y + radius);
			mark[q++] = CPoint(draw[k].x - radius, draw[k].y + radius);
			mark[q++] = CPoint(draw[k].x - radius, draw[k].y - radius);
			res = dc.Polygon(mark, q);
			q = 0;
			if (k == 0)
				mline->rti = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
			else if (k == nDraws-1)
				mline->rtf = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
		}
		if (hOrigBrush) ::SelectObject(dc.GetHDC(), hOrigBrush);
		break;
	case '.':
		radius = 1;
		for (auto k=0; k<nDraws; k++) 
		{
			circleRt.SetRect(CPoint(draw[k])+CPoint(-1,-radius), CPoint(draw[k])+CPoint(radius,radius));
			dc.Ellipse(circleRt);
			if (k == 0)
				mline->rti = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
			if (k == nDraws - 1)
				mline->rtf = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
		}
		break;	
	case 'x':
		for (auto k=0; k<nDraws; k++) 
		{
			dc.MoveTo(CPoint(draw[k].x - radius, draw[k].y - radius));
			dc.LineTo(CPoint(draw[k].x + radius, draw[k].y + radius));
			dc.MoveTo(CPoint(draw[k].x - radius, draw[k].y + radius));
			dc.LineTo(CPoint(draw[k].x + radius, draw[k].y - radius));
			if (k == 0)
				mline->rti = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
			if (k == nDraws - 1)
				mline->rtf = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
		}
		break;	
	case '+':
		for (auto k=0; k<nDraws; k++) 
		{
			pt.x = draw[k].x - radius;
			pt.y = draw[k].y;
			dc.MoveTo(pt);
			pt.x += radius*2;
			dc.LineTo(pt);
			pt.x = draw[k].x; 
			pt.y = draw[k].y - radius;
			dc.MoveTo(pt);
			pt.y += radius*2;
			dc.LineTo(pt);
			if (k == 0)
				mline->rti = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
			if (k == nDraws - 1)
				mline->rtf = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
		}
		break;	
	case '*':
		for (auto k=0; k<nDraws; k++) 
		{
			pt.x = draw[k].x - radius;
			pt.y = draw[k].y;
			dc.MoveTo(pt);
			pt.x += radius*2;
			dc.LineTo(pt);
			pt.x = draw[k].x; 
			pt.y = draw[k].y - radius;
			dc.MoveTo(pt);
			pt.y += radius*2;
			dc.LineTo(pt);
			pt.x = draw[k].x - radius;
			pt.y = draw[k].y - radius;
			dc.MoveTo(pt);
			pt.x = draw[k].x + radius;
			pt.y = draw[k].y + radius;
			dc.LineTo(pt);
			pt.x = draw[k].x - radius;
			pt.y = draw[k].y + radius;
			dc.MoveTo(pt);
			pt.x = draw[k].x + radius;
			pt.y = draw[k].y - radius;
			dc.LineTo(pt);
			if (k == 0)
				mline->rti = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
			if (k == nDraws - 1)
				mline->rtf = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
		}
		break;	
	case 'd':
		if (mline->filled)
		{
			hBr = (HBRUSH)::GetStockObject(DC_BRUSH);
			SetDCBrushColor(dc.GetHDC(), mline->markerColor);
		}
		else
			hBr = (HBRUSH)::GetStockObject(NULL_BRUSH);
		hOrigBrush = ::SelectObject(dc.GetHDC(), hBr);
		for (auto k=0; k<nDraws; k++)
		{
			mark[q++] = CPoint(draw[k].x, draw[k].y + radius);
			mark[q++] = CPoint(draw[k].x + radius, draw[k].y);
			mark[q++] = CPoint(draw[k].x, draw[k].y - radius);
			mark[q++] = CPoint(draw[k].x - radius, draw[k].y);
			mark[q++] = CPoint(draw[k].x, draw[k].y + radius);
			res = dc.Polygon(mark, q);
			q = 0;
			if (k == 0)
				mline->rti = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
			if (k == nDraws - 1)
				mline->rtf = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
		}
		if (hOrigBrush) ::SelectObject(dc.GetHDC(), hOrigBrush);
		break;
	case '^':
		if (mline->filled)
		{
			hBr = (HBRUSH)::GetStockObject(DC_BRUSH);
			SetDCBrushColor(dc.GetHDC(), mline->markerColor);
		}
		else
			hBr = (HBRUSH)::GetStockObject(NULL_BRUSH);
		hOrigBrush = ::SelectObject(dc.GetHDC(), hBr);
		for (auto k=0; k<nDraws; k++)
		{
			mark[q++] = CPoint(draw[k].x, draw[k].y - (int)(C30DIV22*radius+.5));
			mark[q++] = CPoint(draw[k].x - radius, draw[k].y + (int)(C30DIV2 * radius + .5));
			mark[q++] = CPoint(draw[k].x + radius, draw[k].y + (int)(C30DIV2 * radius + .5));
			res = dc.Polygon(mark, q);
			q = 0;
			if (k == 0)
				mline->rti = CRect(CPoint(draw[k].x - radius, draw[k].y - (int)(C30DIV22*radius + .5)), CPoint(draw[k].x + radius, draw[k].y + (int)(C30DIV2 * radius + .5)));
			if (k == nDraws - 1)
				mline->rtf = CRect(CPoint(draw[k].x - radius, draw[k].y - (int)(C30DIV22*radius + .5)), CPoint(draw[k].x + radius, draw[k].y + (int)(C30DIV2 * radius + .5)));
		}
		if (hOrigBrush) ::SelectObject(dc.GetHDC(), hOrigBrush);
		break;
	case 'v':
		if (mline->filled)
		{
			hBr = (HBRUSH)::GetStockObject(DC_BRUSH);
			SetDCBrushColor(dc.GetHDC(), mline->markerColor);
		}
		else
			hBr = (HBRUSH)::GetStockObject(NULL_BRUSH);
		hOrigBrush = ::SelectObject(dc.GetHDC(), hBr);
		for (auto k = 0; k < nDraws; k++)
		{
			mark[q++] = CPoint(draw[k].x, draw[k].y + (int)(C30DIV22*radius + .5));
			mark[q++] = CPoint(draw[k].x - radius, draw[k].y - (int)(C30DIV2 * radius + .5));
			mark[q++] = CPoint(draw[k].x + radius, draw[k].y - (int)(C30DIV2 * radius + .5));
			res = dc.Polygon(mark, q);
			q = 0;
			if (k == 0)
				mline->rti = CRect(CPoint(draw[k].x - radius, draw[k].y - (int)(C30DIV22*radius + .5)), CPoint(draw[k].x + radius, draw[k].y + (int)(C30DIV2 * radius + .5)));
			if (k == nDraws - 1)
				mline->rtf = CRect(CPoint(draw[k].x - radius, draw[k].y - (int)(C30DIV22*radius + .5)), CPoint(draw[k].x + radius, draw[k].y + (int)(C30DIV2 * radius + .5)));
		}
		if (hOrigBrush) ::SelectObject(dc.GetHDC(), hOrigBrush);
		break;
	case '>':
		if (mline->filled)
		{
			hBr = (HBRUSH)::GetStockObject(DC_BRUSH);
			SetDCBrushColor(dc.GetHDC(), mline->markerColor);
		}
		else
			hBr = (HBRUSH)::GetStockObject(NULL_BRUSH);
		hOrigBrush = ::SelectObject(dc.GetHDC(), hBr);
		for (auto k = 0; k < nDraws; k++)
		{
			mark[q++] = CPoint(draw[k].x + (int)(C30DIV22*radius + .5), draw[k].y);
			mark[q++] = CPoint(draw[k].x - (int)(C30DIV2 * radius + .5), draw[k].y - radius);
			mark[q++] = CPoint(draw[k].x - (int)(C30DIV2 * radius + .5), draw[k].y + radius);
			res = dc.Polygon(mark, q);
			q = 0;
			if (k == 0)
				mline->rti = CRect(CPoint(draw[k].x - (int)(C30DIV22*radius + .5), draw[k].y - radius), CPoint(draw[k].x + (int)(C30DIV22*radius + .5), draw[k].y + radius));
			if (k == nDraws - 1)
				mline->rtf = CRect(CPoint(draw[k].x - (int)(C30DIV22*radius + .5), draw[k].y - radius), CPoint(draw[k].x + (int)(C30DIV22*radius + .5), draw[k].y + radius));
		}
		if (hOrigBrush) ::SelectObject(dc.GetHDC(), hOrigBrush);
		break;
	case '<':
		if (mline->filled)
		{
			hBr = (HBRUSH)::GetStockObject(DC_BRUSH);
			SetDCBrushColor(dc.GetHDC(), mline->markerColor);
		}
		else
			hBr = (HBRUSH)::GetStockObject(NULL_BRUSH);
		hOrigBrush = ::SelectObject(dc.GetHDC(), hBr);
		for (auto k = 0; k < nDraws; k++)
		{
			mark[q++] = CPoint(draw[k].x - (int)(C30DIV22*radius + .5), draw[k].y);
			mark[q++] = CPoint(draw[k].x + (int)(C30DIV2 * radius + .5), draw[k].y - radius);
			mark[q++] = CPoint(draw[k].x + (int)(C30DIV2 * radius + .5), draw[k].y + radius);
			res = dc.Polygon(mark, q);
			q = 0;
			if (k == 0)
				mline->rti = CRect(CPoint(draw[k].x - (int)(C30DIV22*radius + .5), draw[k].y - radius), CPoint(draw[k].x + (int)(C30DIV22*radius + .5), draw[k].y + radius));
			if (k == nDraws - 1)
				mline->rtf = CRect(CPoint(draw[k].x - (int)(C30DIV22*radius + .5), draw[k].y - radius), CPoint(draw[k].x + (int)(C30DIV22*radius + .5), draw[k].y + radius));
		}
		if (hOrigBrush) ::SelectObject(dc.GetHDC(), hOrigBrush);
		break;
	case 'h':
	case 'p':
		if (mline->symbol=='p') angle = 72.;
		else angle = 60.;
		if (mline->filled)
		{
			hBr = (HBRUSH)::GetStockObject(DC_BRUSH);
			SetDCBrushColor(dc.GetHDC(), mline->markerColor);
		}
		else
			hBr = (HBRUSH)::GetStockObject(NULL_BRUSH);
		hOrigBrush = ::SelectObject(dc.GetHDC(), hBr);
		for (auto k=0; k<nDraws; k++) 
		{
			int m = 0;
			for (; m<=5; m++)
			{
				pt.x = draw[k].x + ROUND(radius*sin(2*PI*angle*m/360));
				pt.y = draw[k].y - ROUND(radius*cos(2*PI*angle*m/360));
				mark[m] = pt;
			}
			res = dc.Polygon(mark, m);
			if (k == 0)
				mline->rti = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
			if (k == nDraws - 1)
				mline->rtf = CRect(CPoint(draw[k].x - radius, draw[k].y - radius), CPoint(draw[k].x + radius, draw[k].y + radius));
		}
		if (hOrigBrush) ::SelectObject(dc.GetHDC(), hOrigBrush);
		break;
	}
	delete pen4marker;
}