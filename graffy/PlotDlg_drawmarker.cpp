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

#define ROUND(dub) (int)(dub+.5)
#define PI 3.141592

void CPlotDlg::DrawMarker(CDC dc, CLine* mline, vector<POINT> draw)
{
	if (!mline) return;
	int radius(mline->markersize);
	CRect circleRt;
	vector<POINT> mark;
	POINT pt;
	int res;
	double angle;
	HBRUSH hBr;
	CPen *pen4marker = new CPen;
	HGDIOBJ hOrigBrush(NULL);
	if (mline->markerColor==-1) mline->markerColor = mline->color;
	pen4marker->CreatePen(PS_SOLID, 1, mline->markerColor);
	dc.SelectObject(pen4marker);
	switch(mline->symbol)
	{
	case 'o':
		hBr = (HBRUSH)::GetStockObject(NULL_BRUSH);
		hOrigBrush = ::SelectObject(dc.GetHDC(),hBr);
		for (size_t k=0; k<draw.size(); k++) 
		{
			circleRt.SetRect(CPoint(draw[k])+CPoint(-radius,-radius), CPoint(draw[k])+CPoint(radius,radius));
			dc.Ellipse(circleRt);
		}
		if (hOrigBrush) ::SelectObject(dc.GetHDC(),hOrigBrush);
		break;
	case 's':
		for (size_t k=0; k<draw.size(); k++) 
		{
			mark.clear();
			pt.x = draw[k].x - radius;
			pt.y = draw[k].y - radius;
			mark.push_back(pt);
			pt.x = draw[k].x + radius;
			pt.y = draw[k].y - radius;
			mark.push_back(pt);
			pt.x = draw[k].x + radius;
			pt.y = draw[k].y + radius;
			mark.push_back(pt);
			pt.x = draw[k].x - radius;
			pt.y = draw[k].y + radius;
			mark.push_back(pt);
			pt.x = draw[k].x - radius;
			pt.y = draw[k].y - radius;
			mark.push_back(pt);
			res = dc.Polyline(mark.data(), (int)mark.size());
		}
		break;
	case '.':
		radius = 1;
		for (size_t k=0; k<draw.size(); k++) 
		{
			circleRt.SetRect(CPoint(draw[k])+CPoint(-1,-radius), CPoint(draw[k])+CPoint(radius,radius));
			dc.Ellipse(circleRt);
		}
		break;	
	case 'x':
		for (size_t k=0; k<draw.size(); k++) 
		{
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
		}
		break;	
	case '+':
		for (size_t k=0; k<draw.size(); k++) 
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
		}
		break;	
	case '*':
		for (size_t k=0; k<draw.size(); k++) 
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
		}
		break;	
	case 'd':
		for (size_t k=0; k<draw.size(); k++) 
		{
			pt.x = draw[k].x; 
			pt.y = draw[k].y + radius;
			dc.MoveTo(pt);
			pt.x += radius/2;
			pt.y -= radius;
			dc.LineTo(pt);
			pt.x -= radius/2;
			pt.y -= radius;
			dc.LineTo(pt);
			pt.x -= radius/2;
			pt.y += radius;
			dc.LineTo(pt);
			pt.x += radius/2;
			pt.y += radius;
			dc.LineTo(pt);
		}
		break;		
	case '^':
		for (size_t k=0; k<draw.size(); k++) 
		{
			pt.x = draw[k].x; 
			pt.y = draw[k].y - radius;
			dc.MoveTo(pt);
			pt.x += (int)((double)radius*1.7321/2.+.5);
			pt.y += (int)(radius*3./2+.5);
			dc.LineTo(pt);
			pt.x = draw[k].x - (int)((double)radius*1.7321/2.+.5);
			dc.LineTo(pt);
			pt.x = draw[k].x; 
			pt.y = draw[k].y - radius;
			dc.LineTo(pt);
		}
		break;		
	case 'v':
		for (size_t k=0; k<draw.size(); k++) 
		{
			pt.x = draw[k].x; 
			pt.y = draw[k].y + radius;
			dc.MoveTo(pt);
			pt.x += (int)((double)radius*1.7321/2.+.5);
			pt.y -= (int)(radius*3./2+.5);
			dc.LineTo(pt);
			pt.x = draw[k].x - (int)((double)radius*1.7321/2.+.5);
			dc.LineTo(pt);
			pt.x = draw[k].x; 
			pt.y = draw[k].y + radius;
			dc.LineTo(pt);
		}
		break;		
	case '<':
		for (size_t k=0; k<draw.size(); k++) 
		{
			pt.y = draw[k].y; 
			pt.x = draw[k].x - radius;
			dc.MoveTo(pt);
			pt.y += (int)((double)radius*1.7321/2.+.5);
			pt.x += (int)(radius*3./2+.5);
			dc.LineTo(pt);
			pt.y = draw[k].y - (int)((double)radius*1.7321/2.+.5);
			dc.LineTo(pt);
			pt.y = draw[k].y; 
			pt.x = draw[k].x - radius;
			dc.LineTo(pt);
		}
		break;		
	case '>':
		for (size_t k=0; k<draw.size(); k++) 
		{
			pt.y = draw[k].y; 
			pt.x = draw[k].x + radius;
			dc.MoveTo(pt);
			pt.y += ROUND(radius*1.7321/2.);
			pt.x -= ROUND(radius*3./2);
			dc.LineTo(pt);
			pt.y = draw[k].y - ROUND(radius*1.7321/2.);
			dc.LineTo(pt);
			pt.y = draw[k].y; 
			pt.x = draw[k].x + radius;
			dc.LineTo(pt);
		}
		break;		
	case 'p':
		angle = 72.;
		for (size_t k=0; k<draw.size(); k++) 
		{
			pt.x = draw[k].x; 
			pt.y = draw[k].y - radius;
			dc.MoveTo(pt);
			for (int m=1; m<=5; m++)
			{
				pt.x = draw[k].x + ROUND(radius*sin(2*PI*angle*m/360));
				pt.y = draw[k].y - ROUND(radius*cos(2*PI*angle*m/360));
				dc.LineTo(pt);
			}
		}
		break;		
	case 'h':
		angle = 60.;
		for (size_t k=0; k<draw.size(); k++) 
		{
			pt.y = draw[k].y; 
			pt.x = draw[k].x - radius;
			dc.MoveTo(pt);
			for (int m=1; m<=6; m++)
			{
				pt.y = draw[k].y + ROUND(radius*sin(2*PI*angle*m/360));
				pt.x = draw[k].x - ROUND(radius*cos(2*PI*angle*m/360));
				dc.LineTo(pt);
			}
		}
		break;		
	}
	delete pen4marker;
}