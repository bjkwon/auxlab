// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: graffy
// Graphic Library (Windows only)
// 
// 
// Version: 1.495
// Date: 12/13/2018
// 
#include "graffy.h"

GRAPHY_EXPORT CLine::CLine(CWndDlg * base, CGobj * pParent)
: symbol(0), lineWidth(1), id0(0), id1(0), markersize(4), markerColor(-1)
{
	type = GRAFFY_line;
	m_dlg = base;
	hPar = pParent;
	initGO(pParent);
	strut["markersize"] = CSignals(double(markersize));
	strut["marker"] = CSignals(std::string((char*)&symbol));
	hPar->child.push_back(this);
}

CLine::~CLine() 
{	
}

void CLine::initGO(void * _hpar)
{
	CGobj::initGO(_hpar);
	strut["type"] = CSignals(std::string("line"));
	strut["xdata"] = CSignals(1); // empty with fs=1
	strut["ydata"] = CSignals(1); // empty with fs=1
	strut["width"] = CSignals(1.); // 
}

GRAPHY_EXPORT CLine& CLine::operator=(const CLine& rhs)
{
	if (this != &rhs)
	{
		CGobj::operator=(rhs);
		id0 = rhs.id0;
		id1 = rhs.id1;
		lineStyle = rhs.lineStyle;
		lineWidth = rhs.lineWidth;
		markerColor = rhs.markerColor;
		markersize = rhs.markersize;
		symbol = rhs.symbol;
		sig = rhs.sig;
		xdata = rhs.xdata;
		//copy all strut from RHS
		strut["linewidth"] = ((CVar)rhs).strut["linewidth"];
		strut["ydata"] = ((CVar)rhs).strut["ydata"];
	}
	return *this;
}