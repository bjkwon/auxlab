// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: graffy
// Graphic Library (Windows only)
// 
// 
// Version: 1.5
// Date: 3/30/2019
// 
#include "graffy.h"

GRAPHY_EXPORT CLine::CLine(CWndDlg * base, CGobj * pParent)
: symbol(0), lineWidth(1), t1(-1), t2(-1), markersize(4), markerColor(-1)
{
	type = GRAFFY_line;
	m_dlg = base;
	filled = false;
	hPar = pParent;
	initGO(pParent);
	strut["markersize"] = CSignals(double(markersize));
	strut["marker"] = CSignals(std::string((char*)&symbol));
	hPar->child.push_back(this);
	strut.erase("pos");
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
	strut["linestyle"] = CSignals(std::string("")); 
}

GRAPHY_EXPORT CLine& CLine::operator=(const CLine& rhs)
{
	if (this != &rhs)
	{
		CGobj::operator=(rhs);
		t1 = rhs.t1;
		t2 = rhs.t2;
		lineStyle = rhs.lineStyle;
		lineWidth = rhs.lineWidth;
		markerColor = rhs.markerColor;
		markersize = rhs.markersize;
		symbol = rhs.symbol;
		sig = rhs.sig;
		xdata = rhs.xdata;
		//copy all strut from RHS
		strut["linewidth"] = ((CVar)rhs).strut["linewidth"];
		strut["linestyle"] = ((CVar)rhs).strut["linestyle"];
		strut["ydata"] = ((CVar)rhs).strut["ydata"];
	}
	return *this;
}

LineStyle CLine::GetLineStyle()
{ // from linestyle struct (i.e., the symbol used in AUXLAB) to LineStyle enum
	// See getLineSpecifier() in Auxtra.cpp for symbols
	std::string str = strut["linestyle"].string();
	if (str == "none")	return LineStyle_noline;
	if (str == "-")	return LineStyle_solid;
	if (str == "--")	return LineStyle_dash;
	if (str == ":")	return LineStyle_dot;
	if (str == "-.")	return LineStyle_dashdot;
	if (str == "..")	return LineStyle_dashdotdot;
	return LineStyle_err;
}
std::string CLine::GetLineStyleSymbol()
{
	//to be done--Clean up the code. Make sure strut["linestyle"] is synch'ed with lineStyle
	switch (lineStyle)
	{
	case LineStyle_noline:
		return "none";
	case LineStyle_solid:
		return "-";
	case LineStyle_dash:
		return "--";
	case LineStyle_dot:
		return ":";
	case LineStyle_dashdot:
		return "-.";
	case LineStyle_dashdotdot:
		return "..";
	default:
		return "";
	}
}
