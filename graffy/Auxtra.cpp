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

#include "graffy.h"
#include "..\sigproc\sigproc_internal.h"
#include <process.h>

#include "wavplay.h"

#define WM_PLOT_DONE	WM_APP+328
#define WM__CLICKED_GCF	WM_APP+329

#define MARKERSTR	"o+*.xsd^v<>ph"
#define COLORSTR	"ymcrgbwk"

extern HWND hWndApp;

//HANDLE hEvent;
uintptr_t hTread;
bool win7;


void addRedrawCue(HWND hDlg, RECT rt);
void ReplaceStr(string &str, const char *from, const char *to) { ReplaceStr(str, string(from), string(to)); }

HMODULE hLib(NULL);

DWORD colordw;
double blocksize;

bool isWin7();

int IsNamedPlot(HWND hwnd)
{ // returns 1 if it is named plot
  // 0 if it is not
  // -1 if hwnd doesn't represent CFigure
	CVar *p = (CVar*)FindFigure(hwnd);
	if (!p) return -1;
	char title[256];
	GetWindowText(hwnd, title, 255);
	if (!strncmp(title, "Figure ", 7))
		return 0;
	return 1;
}

int getmonitorheight (HWND hDlg)
{
	HMONITOR monitor = MonitorFromWindow(hDlg, MONITOR_DEFAULTTONEAREST);
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &info);
	int monitor_width = info.rcMonitor.right - info.rcMonitor.left;
	int monitor_height = info.rcMonitor.bottom - info.rcMonitor.top;
	return monitor_height;
}

void graffytype2string(graffytype type, char *outBuf)
{
	switch(type)
	{
	case 'r':
		strcpy(outBuf, "root");
		break;
	case 'f':
		strcpy(outBuf, "figure");
		break;
	case 'a':
		strcpy(outBuf, "axis");
		break;
	case 't':
		strcpy(outBuf, "text");
		break;
	case 'p':
		strcpy(outBuf, "patch");
		break;
	case 'l':
		strcpy(outBuf, "line");
		break;
	case 'k':
		strcpy(outBuf, "tick");
		break;
	default:
		strcpy(outBuf, "");
	}
}

vector<HANDLE> plotlines;
map<string, LineStyle> linestylelist;
map<char, int> linemarkerlist;
map<char, DWORD> linecolorlist;
multimap<char, string> graffyprop;

void initGraffyProperties()
{
	graffyprop.insert(make_pair('r',string("parent")));
	graffyprop.insert(make_pair('f',string("parent")));
	graffyprop.insert(make_pair('a',string("parent")));
	graffyprop.insert(make_pair('t',string("parent")));
	graffyprop.insert(make_pair('l',string("parent")));
	graffyprop.insert(make_pair('r',string("children")));
	graffyprop.insert(make_pair('f',string("children")));
	graffyprop.insert(make_pair('a',string("children")));
	graffyprop.insert(make_pair('t',string("children")));
	graffyprop.insert(make_pair('l',string("children")));
	graffyprop.insert(make_pair('r',string("pos")));
	graffyprop.insert(make_pair('f',string("pos")));
	graffyprop.insert(make_pair('a',string("pos")));
	graffyprop.insert(make_pair('t',string("pos")));
	graffyprop.insert(make_pair('l',string("pos")));
	graffyprop.insert(make_pair('r',string("color")));
	graffyprop.insert(make_pair('f',string("color")));
	graffyprop.insert(make_pair('a',string("color")));
	graffyprop.insert(make_pair('t',string("color")));
	graffyprop.insert(make_pair('l',string("color")));
	graffyprop.insert(make_pair('r',string("visible")));
	graffyprop.insert(make_pair('f',string("visible")));
	graffyprop.insert(make_pair('a',string("visible")));
	graffyprop.insert(make_pair('t',string("visible")));
	graffyprop.insert(make_pair('l',string("visible")));
	graffyprop.insert(make_pair('r',string("type")));
	graffyprop.insert(make_pair('f',string("type")));
	graffyprop.insert(make_pair('a',string("type")));
	graffyprop.insert(make_pair('t',string("type")));
	graffyprop.insert(make_pair('l',string("type")));

	//add new properties at the end of each type as needed

	graffyprop.insert(make_pair('a',string("axiscolor")));
	graffyprop.insert(make_pair('a',string("xlim")));
	graffyprop.insert(make_pair('a',string("ylim")));

	graffyprop.insert(make_pair('t',string("fontsize")));
	graffyprop.insert(make_pair('t',string("fontname")));
	graffyprop.insert(make_pair('t',string("fontangle")));
	graffyprop.insert(make_pair('t',string("fontweight")));
	graffyprop.insert(make_pair('t',string("string")));
	graffyprop.insert(make_pair('t',string("horizontalalignment")));
	graffyprop.insert(make_pair('t',string("verticalalignment")));

	graffyprop.insert(make_pair('l',string("xdata")));
	graffyprop.insert(make_pair('l',string("ydata")));
	graffyprop.insert(make_pair('l',string("marker")));
	graffyprop.insert(make_pair('l',string("markersize")));
	graffyprop.insert(make_pair('l',string("linestyle")));
	graffyprop.insert(make_pair('l',string("linewidth")));

	win7 = isWin7();
}

void GetLineStyle (LineStyle out, string &in)
{
	for(map<string, LineStyle>::iterator it=linestylelist.begin(); it!=linestylelist.end(); it++)
		if (it->second == out) {in = it->first; return;}
}

void EnumGraffyProperties(char type, vector<string> &propname)
{
	propname.clear();
	pair <multimap<char,string>::iterator, multimap<char,string>::iterator> ret = graffyprop.equal_range(type);
	for (multimap<char,string>::iterator it=ret.first; it!=ret.second; ++it)
		propname.push_back(it->second);
}

void EnumGraffyTypesfromProp(string propname, vector<char> &type)
{
	type.clear();
	for(multimap<char,string>::iterator it=graffyprop.begin(); it!=graffyprop.end(); it++)
		if (it->second == propname) type.push_back(it->first);
}

void initLineList()
{
	linestylelist["(none)"] = LineStyle_noline; //no line
	linestylelist["-"] = LineStyle_solid; //solid line
	linestylelist["--"] = LineStyle_dash; //dashed line
	linestylelist[":"] = LineStyle_dot; //dotted line
	linestylelist["-."] = LineStyle_dashdot; //dash-dot line
	linestylelist[".."] = LineStyle_dashdotdot; //dash-dot line

	linemarkerlist[' '] = 0; // no marker
	linemarkerlist['o'] = 'o'; // circle
	linemarkerlist['+'] = '+'; // plus
	linemarkerlist['*'] = '*'; // asterisk
	linemarkerlist['.'] = '.'; // point
	linemarkerlist['x'] = 'x'; // cross
	linemarkerlist['s'] = 's'; // square
	linemarkerlist['d'] = 'd'; // diamond
	linemarkerlist['^'] = '^'; // triangle1
	linemarkerlist['v'] = 'v'; // triangle2
	linemarkerlist['>'] = '>'; // triangle3
	linemarkerlist['<'] = '<'; // triangle
	linemarkerlist['p'] = 'p'; // pentagram
	linemarkerlist['h'] = 'h'; // hexagram

	linecolorlist['y'] = RGB(255,255,0); // yellow
	linecolorlist['m'] = RGB(255,0,255); // magenta
	linecolorlist['c'] = RGB(0,255,255); // cyan
	linecolorlist['r'] = RGB(255,0,0); // red
	linecolorlist['g'] = RGB(0,255,0); // green
	linecolorlist['b'] = RGB(0,0,255); // blue
	linecolorlist['k'] = RGB(0,0,0); // black
	linecolorlist['w'] = RGB(255,255,255); // white 
}
//DO THIS-----if an option is not specified, the default should come in... Now the default seems a bit off. For example, "r" defaults to red, no marker, no line. 8/2/2018
void getLineSpecifier (CAstSig *past, const AstNode *pnode, string input, LineStyle &ls, int &mk, DWORD &col)
{ // markcol is set only if marker is drawn but no line is drawn (
	bool markerspecified(true);
	size_t id;
	string input2(input);
	ReplaceStr(input, "-.", "__");
	if (input.length() > 4)
		throw CAstExceptionInvalidUsage(*past, pnode, "Plot option argument must be 4 characters or less.");
	id = input.find_first_of(COLORSTR);
	if (id!=string::npos) col = linecolorlist[input[id]], input.erase(id,1);
	while ((id=input.find_first_of(COLORSTR))!=string::npos) {
		// what's this? 1/17/2020 
		throw CAstExceptionInvalidUsage(*past, pnode, "more than two characters for line color");
		input.erase(id,1);
		// Do something 1/17/2020
	}
	id = input.find_first_of(MARKERSTR);
	if (id!=string::npos) mk = linemarkerlist[input[id]], input.erase(id,1);
	else				markerspecified = false, mk = 0;
	while ((id=input.find_first_of(MARKERSTR))!=string::npos) {
		// what's this? 1/17/2020 
		throw CAstExceptionInvalidUsage(*past, pnode, "more than two characters for marker");
		input.erase(id,1);
		// Do something 1/17/2020
	}
	ReplaceStr(input, "__", "-.");
	if (input.length() == 0)
	{ // if marker is specified but not linestyle --> treated it as "no line"
		if (markerspecified)	ls = LineStyle_noline;
	  // if color is specified but not linestlye --> leave existing linestyle alone
	}
	else if (input=="-" || input=="--" || input==":" || input=="-.")
		ls = linestylelist[input];
	else
		throw CAstExceptionInvalidUsage(*past, pnode, (input + string(" Invalid line style specifier")).c_str());
	if (input2.find_first_of(MARKERSTR)==string::npos) //marker is not specified but
		if (input.empty()) // linestyle is not 
			ls = linestylelist["-"]; // set it solid 
}

#define LOADPF(OUT, DEFTYPE, FUNCSIGNATURE) if ((OUT = (DEFTYPE)GetProcAddress((HMODULE)hLib, FUNCSIGNATURE))==NULL) {char buf[256]; sprintf(buf, "cannot find %s", FUNCSIGNATURE); MessageBox(NULL, "LOADPF error", buf, 0); return 0;}
#define LOADPF2(OUT, DEFTYPE, INDEX, SIMPLESIGNATURE) if ((OUT = (DEFTYPE)GetProcAddress((HMODULE)hLib, (char*)INDEX))==NULL) {char buf[256]; sprintf(buf, "cannot find %s", SIMPLESIGNATURE); MessageBox(NULL, "LOADPF2 error", buf, 0); return 0;}

DWORD double2RGB(double color[3])
{
	WORD r,g,b;
	r = (WORD)(color[0]*255.);
	g = (WORD)(color[1]*255.);
	b = (WORD)(color[2]*255.);
	DWORD out = b*0x10000 + g *0x100 + r;
	return out;
}

/*From now on, Make sure to update this function whenever functions are added/removed in graffy.cpp 
7/15/2016 bjk
*/

vector<CVar*> toDelete(CAstSig *past, CVar *head)
{
	vector<CVar*> out;
	for (auto it = past->GOvars.begin(); it != past->GOvars.end(); it++)
	{
		if ((*it).second.size() > 1)
		{
			for (auto jt = (*it).second.begin(); jt != (*it).second.end(); jt++)
			{
				if (Is_A_Ancestor_of_B(head, (*jt)))
					out.push_back(*jt);
			}
		}
		else
		{
			if (Is_A_Ancestor_of_B(head, (*it).second.front()))
				out.push_back((*it).second.front());
		}
	}
	return out;
}

void delete_toDelete(CAstSig *past, CVar *delThis)
{
	for (auto it = past->GOvars.begin(); it != past->GOvars.end(); )
	{
		if ((*it).second.size() > 1)
		{
			for (auto jt = (*it).second.begin(); jt != (*it).second.end(); )
			{
			if ((*jt) == delThis)
				jt = (*it).second.erase(jt);
			else
				jt++;
			}
			it++;
		}
		else
		{
		if ((*it).second.front() == delThis)
		{
			past->Vars[(*it).first] = CVar(); // do not call SetVars--it doesn't make a new item in Vars because of the item in GOvars with the name (*it).first
			it = past->GOvars.erase(it);
		}
		else
			it++;
		}
	}
}

GRAPHY_EXPORT void _repaint(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (!past->Sig.IsGO())
		throw CAstExceptionInvalidUsage(*past, p, "The argument must be a graphic handle.");
	if (past->Sig.strut["type"].string() == "figure")
	{
		HANDLE h = FindFigure(&past->Sig);
		HWND hh = GetHWND_PlotDlg(h);
		InvalidateRect(hh, NULL, 1);
	}
	else
		throw CAstExceptionInvalidUsage(*past, p, "Only figure handle is supported now.");
}

GRAPHY_EXPORT void _delete_graffy(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{ // Not only the current past, but also all past's from xscope should be handlded. Or, the GO deleted in a udf goes astray in the main scope and crashes in xcom when displaying with showvar (FillUp)
// 
	if (!past->pgo) // if argument is scalar or vector with integer, delete(1:5), figure windows are deleted
	{ // this is not in compliance with AUX syntax philosophy but recognized for the sake of convenience. 10/24/2019
		CVar tsig = past->Sig;
		vector<CVar*> figs = FindFigurebyvalue(tsig);
		while (!figs.empty())
		{
			auto f = figs.front();
			past->pgo = f;
			_delete_graffy(past, pnode, p, fnsigs);
			figs = FindFigurebyvalue(tsig);
		}
		past->pgo = nullptr;
		return;
	}

	// To delete multiple GO's, delete one by one
	if (past->pgo->GetType() == CSIG_HDLARRAY)
	{
		CVar temp = *past->pgo;
		for (unsigned int k = 0; k < temp.nSamples; k++)
		{
			past->pgo = (CVar*)(INT_PTR)temp.buf[k];
			_delete_graffy(past, pnode, p, fnsigs);
		}
		return;
	}

	CGobj *hobj = (CGobj *)past->pgo;
	if (!hobj)
		throw CAstExceptionInvalidUsage(*past, pnode, "1st argument is not a valid graphic object identifier."); //check
	//remove from registered ax list
	if (past->pgo->strut["type"] == string("axes"))
	{
		CFigure *cfg = (CFigure *)((CFigure*)hobj)->hPar;
		RegisterAx((CVar*)cfg, (CAxes*)hobj, false);
	}

	map<CAstSig *, vector<CVar *>> scope_delList;
	CAstSig *tp = past->dad;
	while (tp)
	{
		scope_delList[tp] = toDelete(tp, past->pgo);
		tp = tp->dad;
	}

	// Clear the content of the corresponding variable from GOvar
	for (map<string, vector<CVar*>>::iterator it = past->GOvars.begin(); it != past->GOvars.end(); )
	{
		if ((*it).second.size() > 1)
		{
			for (vector<CVar*>::iterator jt = (*it).second.begin(); jt != (*it).second.end(); )
			{
				if (Is_A_Ancestor_of_B(past->pgo, (*jt)))
					jt = (*it).second.erase(jt);
				else
					jt++;
			}
			it++;
		}
		else
		{
			if (Is_A_Ancestor_of_B(past->pgo, (*it).second.front()))
			{
				past->Vars[(*it).first] = CVar(); // do not call SetVars--it doesn't make a new item in Vars because of the item in GOvars with the name (*it).first
				it = past->GOvars.erase(it);
			}
			else
				it++;
		}
	}
	// Clean the other way
	for (auto it = past->GOvars.begin(); it != past->GOvars.end(); it++)
	{
		for (auto jt = (*it).second.begin(); jt != (*it).second.end(); jt++)
		{
			for (auto kt = (*jt)->struts["children"].begin(); kt != (*jt)->struts["children"].end(); )
			{
				if (*kt == past->pgo)
					kt = (*jt)->struts["children"].erase(kt);
				else
					kt++;
			}
		}
	}

	deleteObj(hobj);
	past->Sig = CVar();
	past->pgo = NULL;
	tp = past->dad;
	while (tp)
	{
		for (auto it = scope_delList[tp].begin(); it != scope_delList[tp].end(); it++)
			delete_toDelete(tp, *it);
		tp = tp->dad;
	}
	if (past->isthisUDFscope(pnode))
		past->u.rt2validate[GetHWND_PlotDlg(hobj)] = CRect(0, 0, 0, 0);
	else
		InvalidateRect(GetHWND_PlotDlg(hobj), NULL, TRUE);
}

GRAPHY_EXPORT void _figure(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	static GRAFWNDDLGSTRUCT in;
	CRect rt(0, 0, 500, 310);
	int nArgs(0);
	const AstNode *pp(p);
	while (pp)
	{
		++nArgs;
		pp = pp->next;
	}
	if (nArgs == 1)
	{
		CVar param = past->Compute(p);
		if (param.IsVector() && param.nSamples == 4)
		{
			rt.left = (LONG)param.buf[0];
			rt.top = (LONG)param.buf[1];
			rt.right = (LONG)(rt.left + param.buf[2]);
			rt.bottom = (LONG)(rt.top + param.buf[3]);
		}
		else
		{
			HANDLE h = FindFigure(&param);
			if (!h)
			{
				if (param.IsString())
				{
					string temp;
					sformat(temp, "Figure with the specified title not found: \"%s\"", param.string().c_str());
					//throw CAstException(p, past, temp.c_str());
					past->pgo = NULL;
					past->Sig = CVar();
					return;
				}
				else if (param.IsScalar())
				{
					char buf[256];
					sprintf(buf, "Figure with the specified handle not found: %lf", param.value());
					//throw CAstException(p, past, buf);
					past->pgo = NULL;
					past->Sig = CVar();
					return;
				}
				else
					throw CAstExceptionInvalidUsage(*past, pnode, "Argument must be a blank, figure handle (either integer alias or real handle), or a 4-element vector specifying the figure position (screen coordinate).");

			}
			past->Sig = *static_cast<CFigure *>(h);
			past->pgo = static_cast<CFigure *>(h); // This is how the figure handle (pointer) is sent back to AstSig
			return;
		}
	}
	past->Sig.strut.clear();
	in.block = CAstSig::play_block_ms;
	in.rt = rt;
	in.threadCaller = GetCurrentThreadId();
	in.hWndAppl = hWndApp;
	in.callbackID = past->callbackIdentifer;
	CFigure * cfig = (CFigure *)OpenGraffy(in);
	past->SetVar("?foc", cfig);
	if (!IsNamedPlot(cfig->m_dlg->hDlg))
		past->SetVar("gcf", cfig);
	static char buf[64];
	cfig->m_dlg->GetWindowText(buf, sizeof(buf));
	PostMessage(hWndApp, WM__PLOTDLG_CREATED, (WPARAM)buf, (LPARAM)&in);
	cfig->strut["name"] = string(buf);
	BYTE r = GetRValue(cfig->color);
	BYTE b = GetBValue(cfig->color);
	BYTE g = GetGValue(cfig->color);
	cfig->strut["color"].buf[0] = (double)r / 256;
	cfig->strut["color"].buf[1] = (double)g / 256;
	cfig->strut["color"].buf[2] = (double)b / 256;
	cfig->strut["pos"].buf[0] = rt.left;
	cfig->strut["pos"].buf[1] = rt.top;
	cfig->strut["pos"].buf[2] = rt.Width();
	cfig->strut["pos"].buf[3] = rt.Height();
	past->Sig = *(past->pgo = cfig); 
	addRedrawCue(cfig->m_dlg->hDlg, CRect(0, 0, 0, 0));
	//if called by a callback
	if (strlen(past->callbackIdentifer)>0) SetInProg(cfig, true);

}

GRAPHY_EXPORT void _text(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CSignals *pGO(NULL);
	vector<CVar *> args;
	if (past->pgo)
		pGO = past->pgo;
	else if (past->Sig.IsGO())
		pGO = &past->Sig;
	else
		args.push_back(&past->Sig);
	if (pGO)
		args.push_back(past->Compute(p));
	for (const AstNode *pp(p); pp; pp = pp->next)
		args.push_back(past->Compute(pp));
	int count = 0;
	vector<CVar *>::reverse_iterator rit = args.rbegin();
	if ((**rit).GetType() != CSIG_STRING) throw CAstExceptionInvalidUsage(*past, pnode, "The last argument must be string.");
	for (rit++; count<2; rit++, count++)
	{
		if ((**rit).GetType() != CSIG_SCALAR) 
			throw CAstExceptionInvalidUsage(*past, pnode, "X- and Y- positions must be scalar.");
	}
	CSignals *pgcf = past->GetVariable("gcf");
	if (!pGO)
	{
		if (pgcf == NULL || pgcf->IsEmpty())
		{
			_figure(past, pnode, NULL, fnsigs);
			pgcf = past->GetVariable("gcf");
		}
	}
	else
		pgcf = pGO;
	CFigure *cfig = (CFigure *)pgcf;
	CText *ctxt = static_cast<CText *>(AddText(cfig, args.back()->string().c_str(), 
		(*(args.end()-3))->value(), (*(args.end() - 2))->value(), 0, 0));
	cfig->struts["children"].push_back(ctxt);
	ctxt->SetValue((double)(INT_PTR)ctxt);
	past->Sig = *(past->pgo = ctxt); 
	addRedrawCue(cfig->m_dlg->hDlg, CRect(0, 0, 0, 0));
}

CAstSig *mainast;

GRAPHY_EXPORT void _axes(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	mainast = past;
	CVar *pGO(NULL);
	// What is past->pgo doing here? There must have been a reason but I'm not sure any more now.
    // I just removed past->pgo in _plot(). Don't know what the implications would be. Just keep it like this now. 8/2/2018
	if (past->pgo)
		pGO = past->pgo;
	else if (past->Sig.IsGO())
		pGO = &past->Sig;
	if (p) past->Compute(p);
	else if(pGO)
		throw CAstExceptionInvalidUsage(*past, pnode, "axes position is required.");
	CPosition pos(past->Sig.buf[0], past->Sig.buf[1], past->Sig.buf[2], past->Sig.buf[3]);
	CSignals *pgcf;
	if (!pGO || pGO->IsEmpty())
	{
		pgcf = past->GetVariable("gcf");
		if (!pgcf || pgcf->IsEmpty())
		{
			_figure(past, pnode, NULL, fnsigs);
			pgcf = past->GetVariable("gcf");
		}
	}
	else
	{
		if (pGO->strut["type"].string() == "figure")
			pgcf = pGO;
		else 
			throw CAstExceptionInvalidUsage(*past, pnode, "Only figure handle can create axes or handle axes");
	}
	CFigure *cfig = (CFigure *)FindFigure(pgcf);
	CAxes * cax;
	cax = static_cast<CAxes *>(AddAxes(cfig, pos));
	RegisterAx((CVar*)cfig, cax, true);

	//taking care of line graphic handle output
	cax->SetValue((double)(INT_PTR)cax);
	cax->strut["pos"].buf[0] = cax->pos.x0;
	cax->strut["pos"].buf[1] = cax->pos.y0;
	cax->strut["pos"].buf[2] = cax->pos.width;
	cax->strut["pos"].buf[3] = cax->pos.height;
	cfig->struts["children"].push_back(cax);
	cfig->struts.erase("gca");
	cfig->struts["gca"].push_back(cax);
	past->Sig = *cax; // Just to show on the screen, not the real output.
	past->pgo = cax; // This is how the figure handle (pointer) is sent back to AstSig
	addRedrawCue(cfig->m_dlg->hDlg, CRect(0, 0, 0, 0));
}

GRAPHY_EXPORT int _reserve_sel(CAstSig *past, const AstNode *p, CSignals *out)
{
	//CSignals *pgcf = past->GetVariable("gcf"); 
	//if (!pgcf) return -1;
	//out->Reset();
	//out->UpdateBuffer(2);
	//int res = GetFigSelRange(pgcf, out);
	//if (res == 0)
	//	throw CAstException(p, past, "#sel called without selecting a range in a figure window", p->str);
	//if (res == -1)
	//	throw CAstException(p, past, "#sel current figure window handle not available", p->str);
	//*out *= 1000.;
	//return res;
	return 0;
}

void __plot(CAxes *pax, CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs, string plotOptions, int nArgs)
{
	int marker;
	LineStyle linestyle;
	DWORD col(-1);
	getLineSpecifier(past, pnode, plotOptions, linestyle, marker, col); // check if the format is valid and get the plot options if all are good.
	double *xdata = NULL;
	int xdataLen=-1;
	int count = 1;
	bool ignoreLast = false;
	for (const AstNode *pp = p; ; count++)
	{
		if (count == 1)
		{
			if (past->pgo)		continue;
		}
		else
		{
			if (p)
			{
				CAstSig tp(past);
				if (!tp.Compute(pp)->IsString())
					past->Compute(pp);
				pp = p->next;
			}
		}
		past->blockCell(pnode, past->Sig);
		past->blockString(pnode, past->Sig);
		past->blockScalar(pnode, past->Sig);
		if (pp)
		{//check whether pp is processed into a string...We need to know whether this is x-ploy or xy-plot
		 //it should have survived error handling in _plot_line
			CAstSig tp(past);
			if (tp.Compute(pp)->IsString())
				ignoreLast = true;
		}
		if ((!pp || ignoreLast) && !xdata) // NULL p: the last; NULL xdata: not xy plot
		{
			plotlines = PlotCSignals(pax, NULL, &past->Sig, col, marker, linestyle);
			break;
		}
		else
		{
			if (xdata)
			{ //xy-plot
				if (past->Sig.nSamples != xdataLen)
					throw CAstExceptionInvalidUsage(*past, pnode, "The length of 1st and 2nd arguments must be the same.");
				plotlines = PlotCSignals(pax, xdata, &past->Sig, col, marker, linestyle);
				delete[] xdata;
				break;
			}
			else
			{
				xdata = new double[xdataLen = past->Sig.nSamples];
				memcpy(xdata, past->Sig.buf, past->Sig.nSamples*past->Sig.bufBlockSize);
			}
			pax->xTimeScale = past->Sig.IsTimeSignal();
		}
	}
}

#include <mutex>
#include <thread>
extern mutex mtx_OnPaint;


void _plot_line(bool isPlot, CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	/* a plot call is one of the following---
	plot(handle, x)
	plot(handle, x, options)
	plot(handle, x, y)
	plot(handle, x, y, options)
	plot(x)
	plot(x, options)
	plot(x, y)
	plot(x, y, options)
	handle is a handle to a figure or an axes object

	The line function is the same except it always adds the line object.
	*/
	//First, check whether a graphic handle is given as the first param
	CVar *pgo = past->pgo;
	if (!past->Sig.IsGO())	pgo = NULL; // first param is not a graphic handle
	static vector<CVar> args;
	static GRAFWNDDLGSTRUCT in;
	//args is the argument list not including the graphic handle.
	mainast = past;
	CAxes *cax = NULL;
	CFigure *cfig = NULL;
	try {
		CAstSig tp(past);
		//Find gcf if pgo is NULL; if no gcf. create a figure
		string plotOptions;
		int nArgs = 1;
		for (const AstNode *pp(p); pp; pp = pp->next)
		{
			nArgs++;
			if (!pp->next) // the last one
			{
				tp.Compute(pp);
				if (tp.Sig.GetType() == CSIG_STRING)
					plotOptions = tp.Sig.string();
			}
		}
		bool newFig = false;
		if (!pgo)
		{
			cfig = (CFigure *)past->GetVariable("gcf");
			if (cfig)
			{ 
				// gcf should not be a named plot
				if (cfig->struts["gca"].empty()) // no axes present; create one.
				{
					cax = (CAxes *)AddAxes(cfig, .08, .18, .86, .72);
					RegisterAx((CVar*)cfig, cax, true);
				}
				else // use existing axes 
					cax = (CAxes *)cfig->struts["gca"].front();
			}
			else
				newFig = true;
		}
		else
		{
			if (pgo->strut["type"].string() == "figure")
			{ // pgo is figure; check if it is named plot, if so mark it and go down; otherwise, set cax
				if (pgo->GetFs() == 2)
					newFig = true;
				else
				{
					auto itgca = pgo->struts.find("gca");
					if (itgca == pgo->struts.end() || itgca->second.empty())
					{
						cax = (CAxes *)AddAxes(FindFigure(pgo), .08, .18, .86, .72);
						RegisterAx((CVar*)FindFigure(pgo), cax, true);
					}
					else
						cax = (CAxes *)pgo->struts["gca"].front();
				}
			}
			else if (pgo->strut["type"].string() == "axes")
				cax = (CAxes *)pgo;
			else
				throw CAstInvalidFuncSyntax(*past, p, fnsigs, "A non-graphic object nor a data array is given as the first argument.");
		}
		if (newFig)
		{
			CVar temp = past->Sig;
			_figure(past, pnode, NULL, fnsigs);
			cfig = (CFigure *)past->pgo;
			cax = (CAxes *)AddAxes(cfig, .08, .18, .86, .72);
			sendtoEventLogger("(_plot_line) AddAxes called.");
			past->Sig = temp;
			past->pgo = NULL;
			RegisterAx((CVar*)cfig, cax, true);
		}
		//Finally cax and cfig ready. Time to inspect input data
		if (tp.Sig.GetType() == CSIG_STRING)
			nArgs = -1;
		plotOptions = (tp.Sig.GetType() == CSIG_STRING) ? tp.Sig.string() : "-";
		if (!cfig) cfig = (CFigure*)((CVar*)cax)->struts["parent"].front();
		if (strlen(past->callbackIdentifer) > 0) SetInProg(cfig, true);
		unique_lock<mutex> locker(mtx_OnPaint);
		sendtoEventLogger("(_plot_line) mtx_OnPaint locked = %d", locker.owns_lock());
		if (isPlot)
		{
			//if there's existing line in the specified axes
			if (!newFig && cax->strut["nextplot"] == string("replace"))
			{
				cax->xlim[0] = 1; cax->xlim[1] = -1; cax->setxlim();
				cax->ylim[0] = 1; cax->ylim[1] = -1; cax->setylim();
				cax->xtick.tics1.clear();
				cax->ytick.tics1.clear();
				// line object in cax->child should be removed
				for (auto obj = cax->child.begin(); obj != cax->child.end(); )
				{
					if ((*obj)->strut["type"].string() == "line")
						obj = cax->child.erase(obj);
					else
						obj++;
				}
				for (auto ch = ((CVar*)cax)->struts["children"].begin(); ch != ((CVar*)cax)->struts["children"].end(); )
				{
					if ((*ch)->strut["type"].string() == "line")
					{
						deleteObj(*ch);
						ch = ((CVar*)cax)->struts["children"].erase(ch);
						sendtoEventLogger("deleteObj called.");
					}
					else
						ch++;
				}
			}
		}
		__plot(cax, past, pnode, p, fnsigs, plotOptions, nArgs);

		if (strlen(past->callbackIdentifer) > 0) SetInProg(cfig, false);
		if (!newFig)
			cfig = (CFigure*)cax->struts["parent"].front();
		sendtoEventLogger("(_plot_line) mtx_OnPaint unlocked.");
	}
	catch (const CAstException &e) { 
		throw CAstExceptionInvalidUsage(*past, pnode, e.getErrMsg().c_str()); }

	//past->pgo carries the pointer, past->Sig is sent only for console display
	switch (plotlines.size())
	{
	case 1:
		past->Sig = *(past->pgo = (CVar*)plotlines.front());
		break;
	case 2:
		vector<INT_PTR> temp;
		for (auto item : plotlines)
			temp.push_back((INT_PTR)item);
		past->Sig = *(past->pgo = past->MakeGOContainer(temp)); // This is how the figure handle (pointer) is sent back to AstSig
		break;
	}
	addRedrawCue(cfig->m_dlg->hDlg, CRect(0, 0, 0, 0));

	//x.plot(___) ==> x needs updating here (a break from the convention where aux calls don't update the variable when applied to it) 3/13/2018
	//Update gcf if it is not showvar-enter figure handle
	//allow pgo NULL (then go with gcf)

	// plot(audio), plot(x,y), plot(x,y,"specifiers") such as plot(x,y,"co:") (cyan, marker o, dotted line)
	// plot(nonaudio, "*") : x axis is just the index of the array


//	if (mutex==NULL) mutex = CreateMutex(0, 0, 0);
//	if (hEvent==NULL) 	hEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("AUXCONScriptEvent")); 
}

GRAPHY_EXPORT void _line(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	_plot_line(0, past, pnode, p, fnsigs);
}

GRAPHY_EXPORT void _plot(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	_plot_line(1, past, pnode, p, fnsigs);
}

GRAPHY_EXPORT void _showrms(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (!past->Sig.IsGO())
		throw CAstExceptionInvalidUsage(*past, p, "The argument must be a graphic handle.");
	if (past->Sig.strut["type"].string() != "figure")
		throw CAstExceptionInvalidUsage(*past, p, "The argument must be a figure handle.");
	CVar *pgo = past->pgo;
	showRMS(pgo, 0);
}


GRAPHY_EXPORT void _replicate(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{ // RegisterAx should be incorporated. 10/11/2019
	CVar out;
	CFigure *cfig;
	CAxes *cax;
	if (!past->Sig.IsGO())
		throw CAstExceptionInvalidUsage(*past, p, "The argument must be a graphic handle.");
	if (past->Sig.strut["type"].string() == "figure")
	{
		static GRAFWNDDLGSTRUCT in;
		in.block = 200; // ((CPlotDlg*)rhs.m_dlg)->block;
		in.lineSpecifer = "";
		((CGobj*)past->pgo)->m_dlg->GetWindowRect(in.rt);
		in.threadCaller = GetCurrentThreadId();
		in.hWndAppl = GetHWND_WAVPLAY();
		CFigure *cfig = (CFigure *)OpenGraffy(in);
		static char buf[64];
		cfig->m_dlg->GetWindowText(buf, sizeof(buf));
		cfig->strut["name"] = string(buf);
		cfig->m_dlg->GetWindowText(buf, sizeof(buf));
		if (!strncmp(buf, "Figure ", 7))
		{
			int figIDint;
			sscanf(buf + 7, "%d", &figIDint);
			((CVar*)cfig)->SetValue((double)figIDint);
		}
		else
		{
			std::string newstr = "*";
			newstr += string();
			((CVar*)cfig)->SetString(newstr.c_str());
		}
		*cfig = *(CFigure*)(past->pgo);
		in.cfig = cfig;
		PostMessage(GetHWND_WAVPLAY(), WM__PLOTDLG_CREATED, (WPARAM)buf, (LPARAM)&in);
		past->Sig = *(past->pgo = cfig);
	}
	else if (past->Sig.strut["type"].string() == "axes")
	{
		cfig = (CFigure *)past->pgo->struts["parent"].front();
		CAxes * pax = new CAxes(((CGobj*)cfig)->m_dlg, (CGobj*)cfig);
		*pax = *(CAxes*)past->pgo;
		((CVar*)cfig)->struts["children"].push_back(pax);
		cfig->ax.push_back(pax);
		past->Sig = *(past->pgo = pax);
	}
	else if (past->Sig.strut["type"].string() == "text")
	{
		cfig = (CFigure *)past->pgo->struts["parent"].front();
		CText * ptext = new CText(((CGobj*)cfig)->m_dlg, (CGobj*)cfig, NULL, ((CGobj*)past->pgo)->pos);
		*ptext = *(CText*)past->pgo;
		((CVar*)cfig)->struts["children"].push_back(ptext);
		cfig->text.push_back(ptext);
		past->Sig = *(past->pgo = ptext);
	}
	else if (past->Sig.strut["type"].string() == "line")
	{
		cax = (CAxes *)past->pgo->struts["parent"].front();
		CLine *tp = new CLine(((CGobj*)cax)->m_dlg, cax);
		*tp = *(CLine *)past->pgo;
		cax->m_ln.push_back(tp);
		cax->struts["children"].push_back(tp);
		past->Sig = *(past->pgo = tp);
		cfig = (CFigure*)cax;
	}
	if (past->isthisUDFscope(pnode))
		past->u.rt2validate[cfig->m_dlg->hDlg] = CRect(0, 0, 0, 0);
	else
		cfig->m_dlg->InvalidateRect(NULL);
}
