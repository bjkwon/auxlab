// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: graffy
// Graphic Library (Windows only)
// 
// 
// Version: 1.497
// Date: 12/26/2018
//
#pragma once

#include "wxx_wincore.h" // Win32++ 8.2. This must be placed prior to <windows.h> to avoid including winsock.h

#ifndef SIGPROC
#include "sigproc.h"
#endif


#ifdef GRAFFY_STATIC //====================== STATIC LIBRARY
#define GRAPHY_EXPORT
#else

#ifdef GRAFFY_INTERNAL //====================== GRAFFY_INTERNAL
#define GRAPHY_EXPORT __declspec (dllexport)
#else
#define GRAPHY_EXPORT __declspec (dllimport)
#endif                //====================== GRAFFY_INTERNAL

#endif               //====================== STATIC LIBRARY


#define GRAPHY_REPLACE_SAME_KEY		(DWORD)0x0000
#define GRAPHY_REPLACE_NO_KEY		(DWORD)0x0001
#define GRAPHY_RETURN_ERR_SAME_KEY	(DWORD)0x0002

#define FONT_STYLE_NORMAL	0
#define FONT_STYLE_ITALIC	1
#define FONT_STYLE_BOLD		2
#define FONT_STYLE_UNDERLINE	4	
#define FONT_STYLE_STRIKEOUT	8	


#define WM_GCF_UPDATED		WM_APP+752
#define WM_PARENT_THREAD	WM_APP+753

#define DEFAULT_AXES_COLOR	RGB(200, 210, 200)
#define DEFAULT_FFT_AXES_COLOR	RGB(220, 220, 150)

#ifndef GRAFFY // ======== CLASS DEFINITIONS FOR graffy
#define GRAFFY
#include "bjcommon_win.h"
#include "bjcommon.h"
#ifndef SIGPROC
#include "sigproc.h"
#endif
//#include "wincore.h" // OLD WIN32+
//#include "winutils.h"  // OLD WIN32+

// CFigure objects are instantiated ("new'ed") during instantiation of CPlotDlg
// CAxes objects are instantiated by CFigure::axes() 

// CFigure, CAxes objects have the proper m_dlg member variable during their instantiation.

extern vector<CAstSig*> xscope;

class CPosition
{
public:
	//all the values in this class is scaled by 1 ("normalized" as in Matlab)
	double x0;
	double y0;
	double width;
	double height;
	GRAPHY_EXPORT CPosition(double x, double y, double w, double h);
	CPosition();
	CRect GetRect(int width, int height);
	void Set(CRect windRect, CRect axRect);
	void AdjustPos2FixLocation(CRect oldrt, CRect newrt);
};

enum graffytype: char
{ 
	GRAFFY_root='r',
	GRAFFY_figure='f',
	GRAFFY_axes = 'a',
	GRAFFY_axis='x',
	GRAFFY_text='t',
	GRAFFY_patch='p',
	GRAFFY_line='l',
	GRAFFY_tick='k',
};


enum LineStyle: unsigned _int8
{ 
	LineStyle_err = 255,
	LineStyle_noline = 0,
	LineStyle_solid, 
	LineStyle_dash, 
	LineStyle_dot, 
	LineStyle_dashdot,
	LineStyle_dashdotdot,
};

class CGobj : public CVar
{
public:
	vector<INT_PTR> geneal; // genealogy of this member.
	COLORREF color;
	CPosition pos;
	CGobj *hPar;
	CGobj *hChild; // this is a linked list, different from vector child, used only for special purpose (for example, to keep a direct genealogy line)
	CWndDlg *m_dlg;
	int visible;
	graffytype type;
	vector<CGobj *> child;
	CGobj();
	virtual ~CGobj();
	// just temporary.. to get rect of the object. Delete once settled 8/31/2019
	virtual POINT GetRef();
	CGobj& operator=(const CGobj& rhs);
	GRAPHY_EXPORT void setPos(double x0, double y0, double width, double height);
	void setPos(CPosition &posIn);
	virtual void initGO(void * _hpar);
	static void addRedrawCue(HWND h, RECT rt);
};

class CTick : public CGobj
{
public:
	bool automatic; 
	POINT gap4next;
	int size; // in pixel
	int labelPos; // in pixel
	char format[16];
	double mult;
	double offset;
	CFont font;
	CRect rt; // the region where ticks are drawn
	vector<double> tics1;
	//set() generates tic vector and fills tics1
	void set(vector<unsigned int> val, vector<double> xydata, int len); // val.size() is the number of division (i.e., 2 means split by half)
	void extend(bool direction, double xlim);
	void initGO(void * _hpar);
	GRAPHY_EXPORT CTick& operator=(const CTick& rhs);
	CTick(CWndDlg * base=NULL);
	~CTick();
};

class CLine : public CGobj
{ //The class constitutes the line object with the whole duration. id0 and id1 sets the viewing portion.
public:
//	list<map<double,double>> chain; // this is the same meaning as chain in CSignals
	CTimeSeries sig;
	CSignal xdata;
	double t1, t2;
	char symbol; // marker symbol
	unsigned _int8 lineWidth;
	unsigned _int8 markersize;
	COLORREF markerColor;
	LineStyle lineStyle;
	bool filled;

	void initGO(void * _hpar);
	LineStyle GetLineStyle();
	std::string GetLineStyleSymbol();
	GRAPHY_EXPORT CLine& operator=(const CLine& rhs);
	GRAPHY_EXPORT CLine(CWndDlg * base, CGobj* pParent);   // standard constructor
	~CLine();
	// These two members are to be filled out in OnPaint
	POINT initial; // client coordinate for the first point (assuming that the line is not reversing)
	POINT final; // client coordinate for the first point (assuming that the line is not reversing)
	RECT rti, rtf;
	unsigned int orglength() {return xdata.nSamples;};
};

class CText : public CGobj
{
public:
	char fontname[32];
	bool italic;
	bool bold;
	bool underline;
	bool strikeout;
	DWORD alignmode;
	WORD posmode; // 0 for regular pos (from CPosion), 1 for pos determined by user-specified textRect
	int fontsize; // in pixel
	CRect textRect; // in Client coordiate
	CFont font;
	std::string str;
	POINT GetRef();
	GRAPHY_EXPORT HFONT ChangeFont(LPCTSTR fontName, int fontSize=15, DWORD style=0);
	GRAPHY_EXPORT int GetAlignment(std::string &horizontal, std::string &vertical);
	GRAPHY_EXPORT int SetAlignment(const char *alignmodestr);
	void initGO(void * _hpar);
	GRAPHY_EXPORT CText& operator=(const CText& rhs);
	GRAPHY_EXPORT CText(CWndDlg * base, CGobj* pParent = NULL, const char* strInit=NULL, CPosition posInit=CPosition(0,0,0,0));   // standard constructor
	~CText();   
};

class CAxis : public CGobj
{
public:
	void initGO(void * _hpar);
	GRAPHY_EXPORT CAxis& operator=(const CAxis& rhs);
	CAxis(CWndDlg * base, CGobj* pParent);
	~CAxis();
};

class CAxes : public CGobj
{
public:
	COLORREF colorAxis;
	double xlim[2], ylim[2];
	double xlimFull[2], ylimFull[2];
	CTick xtick; // How do I pass the argument for instantiation of these objects?
	CTick ytick; // CTick xtick(this) didn't work..... Maybe during instantiation of CAxes set xtick.m_dlg = this...
	vector<CLine*> m_ln;
//	vector<CPatch*> m_pat;
	bool xlimfixed, ylimfixed;
	bool xTimeScale, belowMouse;
	COLORREF ColorFFTAx;

	GRAPHY_EXPORT CAxes *create_child_axis(CPosition pos);
	void GetCoordinate(POINT* pt, double& x, double& y);
	GRAPHY_EXPORT CLine * plot(double *xdata, CTimeSeries *pydata, DWORD colorCode, char cymbol=0, LineStyle ls=LineStyle_solid);
	GRAPHY_EXPORT void setxlim();
	GRAPHY_EXPORT void setylim();
	GRAPHY_EXPORT void DeleteLine(int index);
//	GRAPHY_EXPORT void DeletePatch(int index);
	GRAPHY_EXPORT CRect GetWholeRect(); // Retrieve the whole area including xtick, ytick
	void setxticks();
	POINT GetRef();
	void setRangeFromLines(char xy);
	int GetDivCount(char xy, int dimens);
	POINT double2pixelpt(double x, double y, double *newxlim);
	int double2pixel(double a, char xy);
	double GetRangePixel(int x);
	double pix2timepoint(int pix);
	vector<double> gengrids(char xy, int *pnDv=NULL, int *pdigh=NULL); //pnDv NULL means default grid count. Dont' worry about pdigh when calling from the application.
	vector<double> gengrids(char xy, int pprecision); // to impose the digit precision (such as .001==> -3)
	int timepoint2pix(double timepoint);
	GRAPHY_EXPORT void setRange(const char xy, double x1, double x2);
	GRAPHY_EXPORT void setTick(const char xy, double const x1, double const x2, const double step, char const *format=NULL, double const coeff=1., double const offset=0.);
	GRAPHY_EXPORT CAxes(CWndDlg * base, CGobj* pParent = NULL);   // standard constructor
	void initGO(void * _hpar);
	GRAPHY_EXPORT CAxes& operator=(const CAxes& rhs);
	CAxes();
	~CAxes();
	CRect rct;
};

class CFigure : public CGobj
{
public:
	void DeleteAxis(int index);
	vector<CText*> text;
	vector<CAxes*> ax;
	CText *AddText(const char* string, CPosition pos);
	CAxes *axes(double x0, double y0, double width, double height);
	CAxes *axes(CPosition pos);
	RECT GetRect(CPosition pos);
	GRAPHY_EXPORT void SetXLIM();
	bool inScope;
	void initGO(void * _hpar);
	GRAPHY_EXPORT CFigure& operator=(const CFigure& rhs);
	GRAPHY_EXPORT	CFigure();
	~CFigure();
	POINT GetRef();
};



#endif //GRAFFY


#define WM__VAR_CHANGED			WM_APP+811
#define WM__PLOTDLG_CREATED		WM_APP+823
#define WM__PLOTDLG_DESTROYED	WM_APP+824

struct GRAFWNDDLGSTRUCT
{
	//input parameters
	CRect rt;
	string caption;
	string scope;
	string lineSpecifer;
	string callbackID;
	int devID;
	double block;
	HWND hWndAppl;
	DWORD threadCaller;
	//output parameters
	DWORD threadPlot;
	HANDLE fig;
	HACCEL hAccel;
	CFigure *cfig;
	HANDLE hIcon;
};

struct GRAFWNDDLGCHILDSTRUCT
{
	//input parameters
	CRect rt;
	HWND hWndAppl;
	DWORD threadCaller;
	//output parameters
	DWORD threadPlot;
	HANDLE fig;
	CFigure *cfig;
	HANDLE hIcon;
};

GRAPHY_EXPORT void initGraffy(CAstSig *base);
GRAPHY_EXPORT HANDLE FindGObj(CSignals *xGO, CGobj *hGOParent = NULL);
GRAPHY_EXPORT void SetInProg(CVar *xGO, bool inprog);
GRAPHY_EXPORT bool GetInProg(CVar *xGO);

GRAPHY_EXPORT bool RegisterAx(CVar *xGO, CAxes *pax, bool b);

GRAPHY_EXPORT HANDLE FindFigure(CSignals *figsig);
GRAPHY_EXPORT HANDLE FindFigure(HWND h);

GRAPHY_EXPORT vector<HANDLE> graffy_Figures();
GRAPHY_EXPORT vector<CGobj*> graffy_CFigs();
GRAPHY_EXPORT HANDLE GetGraffyHandle(INT_PTR figID);
GRAPHY_EXPORT HANDLE GCA(HANDLE _fig);
GRAPHY_EXPORT CVar *GetFigGO(HWND h);
GRAPHY_EXPORT int CloseFigure(HANDLE h);
GRAPHY_EXPORT HANDLE FindWithCallbackID(const char *callbackid);


#ifdef _WIN32XX_WINCORE_H_
#define NO_USING_NAMESPACE
GRAPHY_EXPORT HANDLE OpenGraffy(GRAFWNDDLGSTRUCT &in);
GRAPHY_EXPORT HANDLE OpenChildGraffy(GRAFWNDDLGCHILDSTRUCT &in);

GRAPHY_EXPORT HANDLE  OpenFigure(CRect *rt, HWND hWndAppl, int devID, double block, const char * callbackID, HANDLE hIcon=NULL);
GRAPHY_EXPORT HANDLE  OpenFigure(CRect *rt, const char *caption, HWND hWndAppl, int devID, double block, const char * callbackID, HANDLE hIcon = NULL);
#endif 



GRAPHY_EXPORT HANDLE	AddAxes(HANDLE fig, double x0, double y0, double wid, double hei);
GRAPHY_EXPORT HANDLE  AddAxes(HANDLE _fig, CPosition pos);
GRAPHY_EXPORT HANDLE	AddText (HANDLE fig, const char* text, double x, double y, double wid, double hei);
GRAPHY_EXPORT vector<HANDLE>	PlotCSignals(HANDLE ax, double *x, CSignals *pdata, COLORREF col = 0xff0000, char cymbol = 0, LineStyle ls = LineStyle_solid);
GRAPHY_EXPORT vector<HANDLE>	PlotCSignals(HANDLE ax, double *x, CTimeSeries *pdata, COLORREF col=0xff0000, char cymbol=0, LineStyle ls=LineStyle_solid);
GRAPHY_EXPORT void		SetRange(HANDLE ax, const char xy, double x1, double x2);
GRAPHY_EXPORT int		GetFigSelRange(CSignals *hgo, CSignals *out);
GRAPHY_EXPORT void		ShowStatusBar(HANDLE _fig);
GRAPHY_EXPORT void		ViewSpectrum(HANDLE _fig);
GRAPHY_EXPORT void		showdBRMS(CVar *xGO, int code);  // currently not used. just leave it


GRAPHY_EXPORT HACCEL GetAccel(HANDLE hFig);
GRAPHY_EXPORT HWND GetHWND_PlotDlg(HANDLE hFig);
GRAPHY_EXPORT HWND GetHWND_PlotDlg2(HANDLE hFig);
GRAPHY_EXPORT void SetHWND_GRAFFY(HWND hAppl);
GRAPHY_EXPORT HWND GetHWND_GRAFFY ();
GRAPHY_EXPORT void deleteObj (HANDLE h);
GRAPHY_EXPORT void SetGOProperties(CAstSig *pctx, const char *proptype, CVar RHS);
GRAPHY_EXPORT void RepaintGO(CAstSig *pctx);
GRAPHY_EXPORT bool Is_A_Ancestor_of_B(CSignals *A, CSignals *B);
GRAPHY_EXPORT DWORD CSignals2COLORREF(CSignals col);
GRAPHY_EXPORT CSignals &COLORREF2CSignals(vector<DWORD> col, CSignals &sig);
GRAPHY_EXPORT vector<DWORD> Colormap(BYTE head, char lh, char rc, int nItems);

vector<double> makefixtick(double _x1, double _x2, int count);

int IsNamedPlot(HWND hwnd);

GRAPHY_EXPORT void _figure(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
GRAPHY_EXPORT void _axes(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
GRAPHY_EXPORT void _text(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
GRAPHY_EXPORT void _plot(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
GRAPHY_EXPORT void _line(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
GRAPHY_EXPORT void _delete_graffy(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
GRAPHY_EXPORT void _replicate(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
GRAPHY_EXPORT int _reserve_sel(CAstSig *past, const AstNode *p, CSignals *out);