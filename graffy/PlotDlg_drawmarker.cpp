/// AUXLAB 
//
// Copyright (c) 2009-2020 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: graffy
// Graphic Library (Windows only)
// 
// Version: 1.7
// Date: 5/21/2020

#include <math.h>
#include "PlotDlg.h"

#define PI 3.141592
#define C30DIV2 .433 // COS 30 degree div 2
#define C30DIV22 .866 // COS 30 degree 

void CPlotDlg::DrawMarker(CDC dc, CLine* mline, const vector<POINT> & draw)
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
	mline->rti = CRect(CPoint(draw.front().x - radius, draw.front().y - radius), CPoint(draw.front().x + radius, draw.front().y + radius));
	mline->rtf = CRect(CPoint(draw.back().x - radius, draw.back().y - radius), CPoint(draw.back().x + radius, draw.back().y + radius));
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
		for (auto v : draw)
		{
			circleRt.SetRect(CPoint(v) + CPoint(-radius, -radius), CPoint(v) + CPoint(radius, radius));
			dc.Ellipse(circleRt);
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
		for (auto v : draw)
		{
			mark[q++] = CPoint(v.x - radius, v.y - radius);
			mark[q++] = CPoint(v.x + radius, v.y - radius);
			mark[q++] = CPoint(v.x + radius, v.y + radius);
			mark[q++] = CPoint(v.x - radius, v.y + radius);
			mark[q++] = CPoint(v.x - radius, v.y - radius);
			res = dc.Polygon(mark, q);
			q = 0;
		}
		if (hOrigBrush) ::SelectObject(dc.GetHDC(), hOrigBrush);
		break;
	case '.':
		radius = 1;
		for (auto v : draw) 
		{
			circleRt.SetRect(CPoint(v)+CPoint(-1,-radius), CPoint(v)+CPoint(radius,radius));
			dc.Ellipse(circleRt);
		}
		break;	
	case 'x':
		for (auto v : draw) 
		{
			dc.MoveTo(CPoint(v.x - radius, v.y - radius));
			dc.LineTo(CPoint(v.x + radius, v.y + radius));
			dc.MoveTo(CPoint(v.x - radius, v.y + radius));
			dc.LineTo(CPoint(v.x + radius, v.y - radius));
		}
		break;	
	case '+':
		for (auto v : draw) 
		{
			pt.x = v.x - radius;
			pt.y = v.y;
			dc.MoveTo(pt);
			pt.x += radius*2;
			dc.LineTo(pt);
			pt.x = v.x; 
			pt.y = v.y - radius;
			dc.MoveTo(pt);
			pt.y += radius*2;
			dc.LineTo(pt);
		}
		break;	
	case '*':
		for (auto v : draw) 
		{
			pt.x = v.x - radius;
			pt.y = v.y;
			dc.MoveTo(pt);
			pt.x += radius*2;
			dc.LineTo(pt);
			pt.x = v.x; 
			pt.y = v.y - radius;
			dc.MoveTo(pt);
			pt.y += radius*2;
			dc.LineTo(pt);
			pt.x = v.x - radius;
			pt.y = v.y - radius;
			dc.MoveTo(pt);
			pt.x = v.x + radius;
			pt.y = v.y + radius;
			dc.LineTo(pt);
			pt.x = v.x - radius;
			pt.y = v.y + radius;
			dc.MoveTo(pt);
			pt.x = v.x + radius;
			pt.y = v.y - radius;
			dc.LineTo(pt);
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
		for (auto v : draw)
		{
			mark[q++] = CPoint(v.x, v.y + radius);
			mark[q++] = CPoint(v.x + radius, v.y);
			mark[q++] = CPoint(v.x, v.y - radius);
			mark[q++] = CPoint(v.x - radius, v.y);
			mark[q++] = CPoint(v.x, v.y + radius);
			res = dc.Polygon(mark, q);
			q = 0;
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
		for (auto v : draw)
		{
			mark[q++] = CPoint(v.x, v.y - (int)(C30DIV22*radius+.5));
			mark[q++] = CPoint(v.x - radius, v.y + (int)(C30DIV2 * radius + .5));
			mark[q++] = CPoint(v.x + radius, v.y + (int)(C30DIV2 * radius + .5));
			res = dc.Polygon(mark, q);
			q = 0;
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
		for (auto v : draw)
		{
			mark[q++] = CPoint(v.x, v.y + (int)(C30DIV22*radius + .5));
			mark[q++] = CPoint(v.x - radius, v.y - (int)(C30DIV2 * radius + .5));
			mark[q++] = CPoint(v.x + radius, v.y - (int)(C30DIV2 * radius + .5));
			res = dc.Polygon(mark, q);
			q = 0;
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
		for (auto v : draw)
		{
			mark[q++] = CPoint(v.x + (int)(C30DIV22*radius + .5), v.y);
			mark[q++] = CPoint(v.x - (int)(C30DIV2 * radius + .5), v.y - radius);
			mark[q++] = CPoint(v.x - (int)(C30DIV2 * radius + .5), v.y + radius);
			res = dc.Polygon(mark, q);
			q = 0;
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
		for (auto v : draw)
		{
			mark[q++] = CPoint(v.x - (int)(C30DIV22*radius + .5), v.y);
			mark[q++] = CPoint(v.x + (int)(C30DIV2 * radius + .5), v.y - radius);
			mark[q++] = CPoint(v.x + (int)(C30DIV2 * radius + .5), v.y + radius);
			res = dc.Polygon(mark, q);
			q = 0;
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
		for (auto v : draw)
		{
			int m = 0;
			for (; m<=5; m++)
			{
				pt.x = v.x + ROUND(radius*sin(2*PI*angle*m/360));
				pt.y = v.y - ROUND(radius*cos(2*PI*angle*m/360));
				mark[m] = pt;
			}
			res = dc.Polygon(mark, m);
		}
		if (hOrigBrush) ::SelectObject(dc.GetHDC(), hOrigBrush);
		break;
	}
	delete pen4marker;
}