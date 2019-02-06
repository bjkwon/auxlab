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
#include "graffy.h"	
#include "PlotDlg.h"	
#include "wavplay.h"

void thread4Plot(PVOID var);

CGobj::CGobj()
:m_dlg(nullptr), hPar(nullptr), visible(true), hChild(nullptr)
{
	type = GRAFFY_root;
}

CGobj::~CGobj()
{
	if (hPar && !hPar->child.empty())
		for (vector<CGobj*>::iterator it = hPar->child.begin(); it != hPar->child.end(); it++)
		{
			if (*it == this)
			{
				hPar->child.erase(it);
				return;
			}
		}
}

GRAPHY_EXPORT void CGobj::setPos(double x0, double y0, double width, double height)
{
	pos.x0 = x0;
	pos.y0 = y0;
	pos.width = width;
	pos.height  = height;
	
}

void CGobj::setPos(CPosition &posIn)
{
	pos = posIn;
}

void CGobj::initGO(void * hpar)
{
	// if parent is NULL that means it's a figure window. 
	double buf[4] = { 0 };
	strut["pos"] = CSignals(buf, 4);
	strut["color"] = CSignals(buf, 3);
	strut["userdata"] = CSignals(1); // empty
	strut["tag"] = CSignals(std::string(""));
	strut["visible"] = CSignals(true);
	if (hpar)
	{ // hpar is non-NULL except for CFigure, for which SetValue is done differently (either string or integer value)
		geneal = ((CGobj *)hpar)->geneal;
		SetValue((double)(INT_PTR)hpar);
	}
	geneal.push_back((INT_PTR)this);
}

CGobj& CGobj::operator=(const CGobj& rhs)
{
	if (this != &rhs)
	{
		child = rhs.child;
		color = rhs.color;
		pos = rhs.pos;
		type = rhs.type;
		visible = rhs.visible;
		strut["pos"] = ((CVar)rhs).strut["pos"];
		strut["color"] = ((CVar)rhs).strut["color"];
		strut["userdata"] = ((CVar)rhs).strut["userdata"];
		strut["tag"] = ((CVar)rhs).strut["tag"];
		strut["visible"] = ((CVar)rhs).strut["visible"];
	}
	return *this;
}

CPosition::CPosition()
{

}

CPosition::CPosition(double x, double y, double w, double h)
:x0(x), y0(y), width(w), height(h)
{
}

void CPosition::AdjustPos2FixLocation(CRect oldrt, CRect newrt)
{

}

CRect CPosition::GetRect(CRect windRect)
{ // from pos to RECT --> why not use this for general purposes?  8/14/2017
	POINT org;
	int windWidth, windHeight, axHeight, axWidth;
	windWidth = windRect.right-windRect.left;
	windHeight= windRect.bottom-windRect.top;
	org.x = (int)((double)windRect.left + (double)windRect.Width()*x0+.5);
	org.y = (int)((double)windRect.bottom - (double)windRect.Height()*y0+.5);
	axWidth = (int)((double)windRect.Width()*width+.5);
	axHeight = (int)((double)windRect.Height()*height+.5);
	CRect rctAx(org, CPoint(org.x+axWidth, org.y-axHeight));
	return rctAx;
}

void CPosition::Set(CRect windRect, CRect axRect)
{ // from RECT to pos
	width = (double)axRect.Width()/windRect.Width();
	height = (double)axRect.Height()/windRect.Height();
	x0 = (double)(axRect.left-windRect.left) / windRect.Width();
	y0 = (double)(windRect.bottom-axRect.bottom) / windRect.Height();
}

GRAPHY_EXPORT CFigure::CFigure()
{
	type = GRAFFY_figure;
	inScope = true;
	initGO(NULL);
	vector<DWORD> cl(1, color=RGB(230, 230, 210)); // color can disppear later 12/5
	strut["color"] = COLORREF2CSignals(cl, CSignals());
}

CFigure::~CFigure()
{
	while (!ax.empty()) {
		delete ax.back();
		ax.pop_back();
	}
	while (!text.empty()) {
		delete text.back();
		text.pop_back();
	}
	m_dlg=NULL;
}

void CFigure::initGO(void * _hpar)
{
	CGobj::initGO(_hpar);
	strut["type"] = CSignals(std::string("figure"));
	struts["gca"].push_back(NULL);
	struts["gca"].pop_back();
}

CAxes *CFigure::axes(CPosition pos)
{
	CAxes *in = new CAxes(m_dlg, this);
	in->setPos(pos);
	ax.push_back(in);
	return in;
} 

CAxes *CFigure::axes(double x0, double y0, double width, double height)
{
	CAxes *in = new CAxes(m_dlg, this);
	CSignals childrenPrev = strut["child"];
	childrenPrev += in;
	strut["child"] = childrenPrev;
	in->setPos(x0, y0, width, height);
	ax.push_back(in);
	return in;
}

void CFigure::DeleteAxis(int index)
{
	ax.pop_back();
}

CText *CFigure::AddText(const char* strIn, CPosition pos)
{
	CText *in = new CText(m_dlg, this, strIn, pos);
	text.push_back(in);
	return in;
}

RECT CFigure::GetRect(CPosition pos)
{
	RECT rt, tp;
	HWND h = GetHWND_PlotDlg((HANDLE)this);
	GetClientRect(h,&rt);
	int width = (rt.right-rt.left);
	int height = (rt.bottom-rt.top);
	tp.left = rt.left + (int)(width * pos.x0+.5);
	tp.top = rt.top + (int)(height * pos.y0+.5);
	tp.right = tp.left + (int)(width * pos.width+.5);
	tp.bottom = tp.top+ (int)(height* pos.height+.5);

	return tp;
}

void CFigure::SetXLIM()
{ //This inspects child axes and adjusts xlim of each child axis according to the "common" min and max
	//1) Insect all axes and get the common xlim 
	double xlim[] = { 1.e100, -1.e100, };
	for (vector<CAxes*>::iterator px = ax.begin(); px != ax.end(); px++)
	{
		//Do it if the axis has at least one line
		if (!(*px)->m_ln.empty())
		{
			xlim[0] = min(xlim[0], (*px)->xlim[0]);
			xlim[1] = max(xlim[1], (*px)->xlim[1]);
		}
	}
	//2) Update xlim of each axis with the newly surveyed one
	for (vector<CAxes*>::iterator px = ax.begin(); px != ax.end(); px++)
	{
		memcpy((*px)->xlim, xlim, 2 * sizeof(double));
		memcpy((*px)->xlimFull, xlim, 2 * sizeof(double));
	}
	//3) Invalidate each axis
	for (vector<CAxes*>::iterator px = ax.begin(); px != ax.end(); px++)
	{
		CRect rt((*px)->rcAx);
		rt.InflateRect(5, 0, 10, 30);
		m_dlg->InvalidateRect(rt);
	}
 }

GRAPHY_EXPORT CFigure& CFigure::operator=(const CFigure& rhs)
{
	if (this != &rhs)
	{
		CGobj::operator=(rhs);
		int index = 0;
		auto rvax = ((CVar)rhs).struts["children"];
		map<std::string, vector<CVar*>> aa = ((CVar)rhs).struts;
		bool bb = aa.find("gca") != aa.end();
		if (bb && !((CVar)rhs).struts["gca"].empty())
			for (auto ch : rvax)
			{
				if (((CVar)rhs).struts["gca"].front() == ch)
					break;
				index++;
			}
		rvax = ((CVar)rhs).struts["children"];
		for (auto rax : rvax)
		{
			if (((CGobj*)rax)->type == GRAFFY_axes)
			{
				CAxes * pax = new CAxes(m_dlg, rhs.hPar);
				*pax = *(CAxes*)rax;
				struts["children"].push_back(pax);
				ax.push_back(pax);
			}
			else if (type == GRAFFY_text)
			{
				CText *ptext = new CText(m_dlg, rhs.hPar);
				*ptext = *(CText*)rax;
				struts["children"].push_back(ptext);
				text.push_back(ptext);
			}
			else
				throw "internal error.";
		}
		if (bb && !((CVar)rhs).struts["gca"].empty())
			struts["gca"].push_back(struts["children"][index]);
	}
	return *this;
}