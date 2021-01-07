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

static void _delete_ans(CAstSig *past)
{
	// Delete the "ans" variable from the main scope if it is a GO.
	// Without this, auxlab may crash if showvar refreshes the variable list and fillup the showvarDlg
	// while plot or line call is in progress (so the new ans variable is not yet ready).
	// Currently used only in plot or _delete_graffy, but may be needed in other Graffy calls. 9/5/2020
	auto ansIt = past->GOvars.find("ans");
	if (ansIt != past->GOvars.end())
		if ((*ansIt).second.front()->IsGO())
			past->GOvars.erase(ansIt);
}

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
		throw CAstException(USAGE, *past, pnode).proc("Plot option argument must be 4 characters or less.");
	id = input.find_first_of(COLORSTR);
	if (id!=string::npos) col = linecolorlist[input[id]], input.erase(id,1);
	while ((id=input.find_first_of(COLORSTR))!=string::npos) {
		// what's this? 1/17/2020
		throw CAstException(USAGE, *past, pnode).proc("more than two characters for line color");
		input.erase(id,1);
		// Do something 1/17/2020
	}
	id = input.find_first_of(MARKERSTR);
	if (id!=string::npos) mk = linemarkerlist[input[id]], input.erase(id,1);
	else				markerspecified = false, mk = 0;
	while ((id=input.find_first_of(MARKERSTR))!=string::npos) {
		// what's this? 1/17/2020
		throw CAstException(USAGE, *past, pnode).proc("more than two characters for marker");
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
		throw CAstException(USAGE, *past, pnode).proc((input + string(" Invalid line style specifier")).c_str());
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

static map<CFigure*, CAxes*> get_gcf_gca(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs, int callingfunction)
{ // callingfunction: 1 for plot or line, 2 for axes, 3 for text, 4 for replicate
	map<CFigure*, CAxes*> out;
	CAxes* pax = NULL;
	CFigure* cfig = NULL;
	if (past->Sig.IsGO()) // when the first arg is specified with a GO
	{
		CVar* pgo = past->pgo; // pgo is the actual go, past->Sig is only the mirroring one.
		if (!pgo)
			throw CAstException(USAGE, *past, p).proc(fnsigs, "A Graffy function called with a NULL base GO.");
		switch (GOtype(*pgo))
		{
		case GRAFFY_figure:
		{
			auto itgca = pgo->struts.find("gca");
			if (itgca == pgo->struts.end() || itgca->second.empty())
			{
				pax = (CAxes*)AddAxes((HANDLE)pgo, .08, .18, .86, .72);
				RegisterAx(pgo, pax, true);
			}
			else
				pax = (CAxes*)pgo->struts["gca"].front();
		}
			break;
		case GRAFFY_axes:
			pax = (CAxes*)pgo;
			pgo = pgo->struts["parent"].front();
			pgo->struts["gca"].clear();
			pgo->struts["gca"].push_back(pax);
			break;
		default:
			throw CAstException(USAGE, *past, p).proc(fnsigs, "A non-graphic object nor a data array is given as the first argument.");
		}
		

		out[(CFigure*)pgo] = pax;
	}
	else
	{
		CVar* pgo = NULL;
		cfig = (CFigure*)past->GetVariable("gcf");
		if (cfig)
		{
			// gcf should not be a named plot
			//if (cfig->struts["gca"].empty()) // no axes present; create one.
			//{
			//	pax = (CAxes*)AddAxes(cfig, .08, .18, .86, .72);
			//	RegisterAx((CVar*)cfig, pax, true);
			//}
			//else 
				if (!cfig->struts["gca"].empty()) // use existing axes
				pax = (CAxes*)cfig->struts["gca"].front();
		}
		out[cfig] = pax;
	}
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
		throw CAstException(USAGE, *past, p).proc("The argument must be a graphic handle.");
	if (GOtype(past->Sig) == GRAFFY_figure)
	{
		HANDLE h = FindFigure(&past->Sig);
		HWND hh = GetHWND_PlotDlg(h);
		InvalidateRect(hh, NULL, 1);
	}
	else
		throw CAstException(USAGE, *past, p).proc("Only figure handle is supported now.");
}

static void _delete_figure(CVar *pgo)
{
	HWND h = GetFigure(pgo);
// 	StopPlay(hAudio, true);
	PostMessage(h, WM_QUIT, 0, 0);
}


static int _delete_graffy_non_figure(CAstSig *past, const AstNode *pnode, HANDLE obj)
{
	if (!obj) return 0; // can be NULL while deleting a multi-figure obj.
	CGobj *hobj = (CGobj *)obj;
	CVar *pgo = (CVar*)obj;
	CVar *hPar = ((CFigure*)hobj)->hPar;
	if (!hobj)
		throw CAstException(USAGE, *past, pnode).proc("1st argument is not a valid graphic object identifier."); //check
	switch (hobj->type)
	{
	case GRAFFY_axes:
		RegisterAx(hPar, (CAxes*)hobj, false);
		hPar->struts["gca"].clear();
		break;
	case GRAFFY_text:
		break;
	case GRAFFY_line:
		break;
	}
	deleteObj(hobj);
	past->pgo = NULL;
	if (past->isthisUDFscope(pnode))
		past->u.rt2validate[GetHWND_PlotDlg(hobj)] = CRect(0, 0, 0, 0);
	else
		InvalidateRect(GetHWND_PlotDlg(hobj), NULL, TRUE);
	return 1;
}

/*
Investigate 10/3/2020, 10/4/2020

a=[4 3 0 8 .5 6.3]
b=[-5 0 3 3 2 -1]
a.plot(b)
figure,plot(b,a)
ly=figure(2).children.children
f1=figure(1);
[f1 ly].delete
===> case 1

a=[4 3 0 8 .5 6.3]
b=[-5 0 3 3 2 -1]
a.plot(b)
figure,plot(b,a)
ly=figure(2).children.children
f2=figure(2);
[f2 ly].delete
==> case 2
Both cases Occassionally timing conflict and hangs
*/

// To do: if Govar is axes, it should also go deeper onto .x or .y 10/4/2020

static void deep_erase(CVar *Govar, CVar* const del)
{
	if (Govar->struts.find("children") == Govar->struts.end() ||
		Govar->struts["children"].empty()) return;
	//See if there's del found at the current layer
	for (auto ch = Govar->struts["children"].begin(); ch != Govar->struts["children"].end(); )
	{
		if (*ch == del)
			ch = Govar->struts["children"].erase(ch);
		else
			ch++;
	}
	//deeper layer
	for (auto ch : Govar->struts["children"])
		deep_erase(ch, del);
}

// Go through every GOvar and its derivatives. If it is same as del, erase from it
// derivatives: children (all types), x or y (axes), userdata
// 
static void deep_erase(CAstSig* past, CVar * const del)
{
	for (auto gov = past->GOvars.begin(); gov != past->GOvars.end(); gov++)
	{ // gov is either single (you can/should use .front() or multiGO 
		if ((*gov).second.size()==1)
			deep_erase((*gov).second.front(), del);
		else
		{
			// Taken care of by CAstSig::erase_GO(CVar * obj)
		}
	}
}


GRAPHY_EXPORT void _delete_graffy(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{ // Not only the current past, but also all past's from xscope should be handlded. Or, the GO deleted in a udf goes astray in the main scope and crashes in xcom when displaying with showvar (FillUp)
//
// The following are OK:
// delete(1:3) --- equivalent to delete(figure(1:3))
// delete(figure(2))
// delete("x") -- instead of delete(figure("x")) ?? Probably better to keep the original form. Just skipping figure call won't do a lot of saving 9/28/2020
// delete(ax)
// delete([gcf figure(3).children])
// delete([gcf figure(3:4)])

// The following are NOT OK:
// delete([gcf 3])
// delete([gcf 3:4])

	// First delete figure objects; then do the rest.
	if (	past->Sig.type() <= 2 || past->Sig.IsGO()) // or a vector
	{
		//figures first
		vector<unsigned int> fids;
		vector<HANDLE> figs2delete = FindFigures(past->Sig, fids);
		vector<CVar*> nonfigs2delete = FindNonFigures(past->Sig);
		if (!figs2delete.empty()) past->wait4cv = true;
		_delete_ans(past);
		vector<string> var2deleted;
		for (auto fig : figs2delete)
		{
			vector<string> varname = past->erase_GO((CVar*)fig);
			CGobj* fobj = (CGobj*)fig;
			// 	StopPlay(hAudio, true);
			PostMessage(fobj->m_dlg->hDlg, WM_QUIT, 0, 0);
			CGobj* hobj = (CGobj*)fig;
			CVar* pgo = (CVar*)fig;
			CVar* hPar = ((CFigure*)hobj)->hPar;
			RegisterAx(hPar, (CAxes*)hobj, false);
			for (auto v : varname)
				var2deleted.push_back(v);
		}

		// NEED TO CALL THIS BLOCK FROM WITHIN CODE (NOT delete AUX command)
		// want to delete children objects of cfig like this... but not directly calling deleteObj()
		//		while (!cfig->ax.empty())
		//			deleteObj(cfig->ax.front());
		// DO IT on 11/26/2020!!!!!!



		// non-figures
		for (auto del : nonfigs2delete)
		{
			deep_erase(past, del);
			vector<string> varname = past->erase_GO(del);
			for (auto v : varname)
				var2deleted.push_back(v);
			_delete_graffy_non_figure(past, pnode, del);
		}



		// if Reset() resets a GO into a NULL, the next two lines are not necessary 10/1/2020
		past->Sig.strut.clear();
		past->Sig.struts.clear();
		past->Sig.SetValue((double)var2deleted.size());
	}
	past->pgo = NULL;
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
					throw CAstException(USAGE, *past, pnode).proc(buf);
					past->pgo = NULL;
					past->Sig = CVar();
					return;
				}
				else
					throw CAstException(USAGE, *past, pnode).proc("Argument must be a blank, figure handle (either integer alias or real handle), or a 4-element vector specifying the figure position (screen coordinate).");

			}
			past->Sig = *static_cast<CFigure *>(h);
			past->pgo = static_cast<CFigure *>(h); // This is how the figure handle (pointer) is sent back to AstSig
			BOOL res = SetForegroundWindow(((CGobj*)past->pgo)->m_dlg->hDlg);
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

GRAPHY_EXPORT void _text(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	CAxes* pax = NULL;
	CFigure* cfig = NULL;
	CText* ctxt = NULL;
	map<CFigure*, CAxes*> objects = get_gcf_gca(past, pnode, p, fnsigs, 2);
	//different cases:
	// 1) text(xpos,ypos,string);  // no gcf available
	// 2) text(xpos,ypos,string);  // gcf available
	// text(GO, xpos,ypos,string); // regardless of gcf availability
	vector<CVar> arg;
	if (past->Sig.IsGO())
	{
		past->Compute(p);
		p = p->next;
	}
	if (past->Sig.type() != 1)
		throw CAstException(USAGE, *past, pnode).proc("1st and 2nd arg must be a scalar: text())");
	arg.push_back(past->Sig);
	past->Compute(p);
	if (past->Sig.type() != 1)
		throw CAstException(USAGE, *past, pnode).proc("1st and 2nd arg should be a scalar: text())");
	arg.push_back(past->Sig);
	p = p->next;
	past->Compute(p);
	if ( !(past->Sig.type() & TYPEBIT_STRING))
		throw CAstException(USAGE, *past, pnode).proc("3rd arg should be a string: text())");
	arg.push_back(past->Sig);
	if ((*objects.begin()).first)
	{
		cfig = (*objects.begin()).first;
		pax = (*objects.begin()).second;
	}
	else
	{
		_figure(past, pnode, NULL, fnsigs);
		cfig = (CFigure*)past->pgo;
	}

	ctxt = static_cast<CText *>(AddText(cfig, arg.back().string().c_str(),
		arg[0].value(), arg[1].value(), 0, 0));
	cfig->struts["children"].push_back(ctxt);
	ctxt->SetValue((double)(INT_PTR)ctxt);
	cfig->struts.erase("gca");
	cfig->struts["gca"].push_back(pax);
	if (!IsNamedPlot(cfig->m_dlg->hDlg))
	{
		past->GOvars["gcf"].clear();
		past->GOvars["gcf"].push_back((CVar*)cfig);
	}
	past->GOvars["?foc"].clear();
	past->GOvars["?foc"].push_back((CVar*)cfig);
	past->Sig = *(past->pgo = ctxt);
	addRedrawCue(cfig->m_dlg->hDlg, CRect(0, 0, 0, 0));
}

CAstSig *mainast;

GRAPHY_EXPORT void _axes(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	mainast = past;
	CAxes* pax = NULL;
	CFigure* cfig = NULL;
	map<CFigure*, CAxes*> objects = get_gcf_gca(past, pnode, p, fnsigs, 2);
	if ((*objects.begin()).first)
	{
		if (p) past->Compute(p);

		cfig = (*objects.begin()).first;
		pax = (*objects.begin()).second;
		if (p && past->Sig.type() != 2)
			throw CAstException(USAGE, *past, pnode).proc("arg should be either a fig or axes handle or a 4-element vector in axes(arg))");
		if (past->Sig.IsGO())
			past->Sig = *(past->pgo = pax);
		else
		{
			CPosition pos(past->Sig.buf[0], past->Sig.buf[1], past->Sig.buf[2], past->Sig.buf[3]);
			pax = (CAxes*)AddAxes(cfig, pos);
		}
	}
	else
	{
		//allow only a 4-element vector
		if (past->Sig.type() != 2)
			throw CAstException(USAGE, *past, pnode).proc("arg should be either a fig or axes handle or a 4-element vector in axes(arg))");
		CPosition pos(past->Sig.buf[0], past->Sig.buf[1], past->Sig.buf[2], past->Sig.buf[3]);
		_figure(past, pnode, NULL, fnsigs);
		cfig = (CFigure*)past->pgo;
		pax = (CAxes*)AddAxes(cfig, pos);
	}

		//throw CAstException(USAGE, *past, pnode).proc("Only figure handle can create axes or handle axes");
	pax->SetValue((double)(INT_PTR)pax);
	pax->strut["pos"].buf[0] = pax->pos.x0;
	pax->strut["pos"].buf[1] = pax->pos.y0;
	pax->strut["pos"].buf[2] = pax->pos.width;
	pax->strut["pos"].buf[3] = pax->pos.height;
	cfig->struts["children"].push_back(pax);
	cfig->struts.erase("gca");
	cfig->struts["gca"].push_back(pax);
	if (!IsNamedPlot(cfig->m_dlg->hDlg))
	{
		past->GOvars["gcf"].clear();
		past->GOvars["gcf"].push_back((CVar*)cfig);
	}
	past->GOvars["?foc"].clear();
	past->GOvars["?foc"].push_back((CVar*)cfig);

	past->Sig = *pax; // Just to show on the screen, not the real output.
	past->pgo = pax; // This is how the figure handle (pointer) is sent back to AstSig
	RegisterAx((CVar*)cfig, pax, true);
//taking care of line graphic handle output
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

#define BLOCKCELLSTRING(AST,SIG) \
{ \
AST->blockCell(pnode, SIG);\
AST->blockString(pnode, SIG);\
}

void __plot(CAxes *pax, CAstSig *past, const AstNode *pnode, const CVar &arg1, const CVar &arg2, const string &plotOptions)
{
	int marker;
	LineStyle linestyle;
	DWORD col(-1);
	getLineSpecifier(past, pnode, plotOptions, linestyle, marker, col); // check if the format is valid and get the plot options if all are good.
	double *xdata = NULL;
	if (arg2.type()) // arg2 not empty; x-y plot
	{
		if (arg1.nSamples != arg2.nSamples)
			throw CAstException(USAGE, *past, pnode).proc("The length of 1st and 2nd arguments must be the same.");
		plotlines = PlotCSignals(pax, arg1.buf, arg2, "", col, marker, linestyle);
	}
	else
		plotlines = PlotCSignals(pax, NULL, arg1, "", col, marker, linestyle);
	pax->xTimeScale = past->Sig.IsTimeSignal();
	pax->limReady = false;
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
	bool newFig = false;
	CAxes *pax = NULL;
	CFigure *cfig = NULL;
	string plotOptions;
	CVar arg1; // x in plot(x,y) or plot(h,x,y), cannot be NULL
	CVar arg2; // y in plot(x,y) or plot(h,x,y)
	CVar arg3; // "o" in plot(x,y,"o") or plot(h,x,y,"o")
	//First, check whether a graphic handle is given as the first param
	map<CFigure*, CAxes*> objects = get_gcf_gca(past, pnode, p, fnsigs, 1);
	if (past->Sig.IsGO()) // update 8/23/2020
	{
		// p should be the CVar object to plot, in plot(h,x,...) and subsequently next'ed.
		// past->Sig has been stored as pgo; no need to shield it with try and CAstSig tp(past)
		if (p) arg1 = past->Compute(p);
		BLOCKCELLSTRING(past, arg1);
		if (p->next)
		{
			if (p->next->next)	{
				arg3 = past->Compute(p->next->next);
				past->checkString(pnode, arg3, "last arg in plot()");
				plotOptions = arg3.string();
			}
			arg2 = past->Compute(p->next);
			if (arg2.type() & TYPEBIT_STRING)
			{
				plotOptions = arg2.string();
				arg2.Reset();
			}
		}
	}
	else
	{
		// past->Sig should be the CVar object to plot, in plot(x,...) and p is the rest of arguments and next'ed.
		try {
			BLOCKCELLSTRING(past, past->Sig);
			arg1 = past->Sig;
			if (p) {
				CAstSig tp(past);
				if (p->next) {
					arg3 = tp.Compute(p->next);
					tp.checkString(pnode, arg3, "last arg in plot()");
					plotOptions = arg3.string();
				}
				arg2 = tp.Compute(p);
				if (arg2.type() & TYPEBIT_STRING)
				{
					plotOptions = arg2.string();
					arg2.Reset();
				}
			}
		}
		catch (const CAstException &e) {
			throw e;
			//			CAstException(USAGE, *past, pnode).proc(e.getErrMsg().c_str());
		}
	}
	if ((*objects.begin()).first)
	{
		cfig = (*objects.begin()).first;
		pax = (*objects.begin()).second;
	}
	else
	{
		CVar temp = past->Sig;
		_figure(past, pnode, NULL, fnsigs);
		cfig = (CFigure *)past->pgo;
	}
	if (!pax) 
	{
		pax = (CAxes*)AddAxes(cfig, .08, .18, .86, .72);
		sendtoEventLogger("(_plot_line) AddAxes called.");
		RegisterAx((CVar*)cfig, pax, true);
	}
	if (strlen(past->callbackIdentifer) > 0) SetInProg(cfig, true);
	// Finally pax and cfig ready. Time to inspect input data
	unique_lock<mutex> locker(mtx_OnPaint);
	sendtoEventLogger("(_plot_line) mtx_OnPaint locked = %d", locker.owns_lock());
	pax->xtick.tics1.clear();
	pax->ytick.tics1.clear();
	if (isPlot)
	{
		//if there's existing line in the specified axes
		_delete_ans(past);
		if (!newFig && pax->strut["nextplot"] == string("replace"))
		{
			pax->ylim[0] = 1; pax->ylim[1] = -1; pax->setylim();
			// line object in pax->child should be removed
			for (auto obj = pax->child.begin(); obj != pax->child.end(); )
			{
				if (GOtype(*obj) == GRAFFY_line)
					obj = pax->child.erase(obj);
				else
					obj++;
			}
			for (auto ch = ((CVar*)pax)->struts["children"].begin(); ch != ((CVar*)pax)->struts["children"].end(); )
			{
				if (GOtype(*ch) == GRAFFY_line)
				{
					deleteObj(*ch);
					ch = ((CVar*)pax)->struts["children"].erase(ch);
					sendtoEventLogger("deleteObj called.");
				}
				else
					ch++;
			}
		}
	}
	if (plotOptions.empty()) plotOptions = "-";
	__plot(pax, past, pnode, arg1, arg2, plotOptions);
	if (isPlot) pax->set_xlim_xrange(); // must be called after PlotCSignals (i.e., line is created)
	if (strlen(past->callbackIdentifer) > 0) SetInProg(cfig, false);
	sendtoEventLogger("(_plot_line) mtx_OnPaint unlocked.");
	if (past->GetVariable("gcf") != cfig)
	{
		past->GOvars["gcf"].clear();
		past->GOvars["gcf"].push_back(cfig);
	}
	BOOL res = SetForegroundWindow(((CGobj*)cfig)->m_dlg->hDlg);

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
		throw CAstException(USAGE, *past, p).proc("The argument must be a graphic handle.");
	if (GOtype(past->Sig) != GRAFFY_figure)
		throw CAstException(USAGE, *past, p).proc("The argument must be a figure handle.");
	CVar *pgo = past->pgo;
	showRMS(pgo, 0);
}


GRAPHY_EXPORT void _replicate(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{ // RegisterAx should be incorporated. 10/11/2019
	CVar out;
	CFigure *cfig;
	CAxes *cax;
	if (!past->Sig.IsGO())
		throw CAstException(USAGE, *past, p).proc("The argument must be a graphic handle.");
	switch (GOtype(past->Sig))
	{
	case GRAFFY_figure:
	{
		static GRAFWNDDLGSTRUCT in;
		in.block = 200; // ((CPlotDlg*)rhs.m_dlg)->block;
		in.lineSpecifer = "";
		((CGobj*)past->pgo)->m_dlg->GetWindowRect(in.rt);
		in.threadCaller = GetCurrentThreadId();
		in.hWndAppl = GetHWND_WAVPLAY();
		cfig = (CFigure *)OpenGraffy(in);
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
	break;
	case GRAFFY_axes:
	{
		cfig = (CFigure *)past->pgo->struts["parent"].front();
		CAxes * pax = new CAxes(((CGobj*)cfig)->m_dlg, (CGobj*)cfig);
		*pax = *(CAxes*)past->pgo;
		((CVar*)cfig)->struts["children"].push_back(pax);
		cfig->ax.push_back(pax);
		pax->SetValue((double)(INT_PTR)(void*)pax);
		// xrange must be copied as well (otherwise, it will hang due to uninitialized xrange).
		memcpy((void*)pax->xrange, ((CAxes*)past->pgo)->xrange, sizeof(double) * 2);
		past->Sig = *(past->pgo = pax);
	}
	break;
	case GRAFFY_text:
	{
		cfig = (CFigure *)past->pgo->struts["parent"].front();
		CText * ptext = new CText(((CGobj*)cfig)->m_dlg, (CGobj*)cfig, NULL, ((CGobj*)past->pgo)->pos);
		*ptext = *(CText*)past->pgo;
		((CVar*)cfig)->struts["children"].push_back(ptext);
		cfig->text.push_back(ptext);
		past->Sig = *(past->pgo = ptext);
	}
	break;
	case GRAFFY_line:
	{
		cax = (CAxes *)past->pgo->struts["parent"].front();
		CLine *tp = new CLine(((CGobj*)cax)->m_dlg, cax);
		*tp = *(CLine *)past->pgo;
		cax->m_ln.push_back(tp);
		cax->struts["children"].push_back(tp);
		past->Sig = *(past->pgo = tp);
		cfig = (CFigure*)cax;
	}
	break;
	}
	if (past->isthisUDFscope(pnode))
		past->u.rt2validate[cfig->m_dlg->hDlg] = CRect(0, 0, 0, 0);
	else
		cfig->m_dlg->InvalidateRect(NULL);
}
