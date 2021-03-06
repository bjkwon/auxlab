/// AUXLAB 
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
#include "msgCrack.h"
#include <string.h>
#include <algorithm>
#include "audstr.h"

extern vector<CAstSig*> xscope;
#ifndef GRAFFY_STATIC 
double CAstSig::play_block_ms = 300;
double CAstSig::record_block_ms = 300;
short CAstSig::play_bytes = 2;
short CAstSig::record_bytes = 2;
#endif //GRAFFY_STATIC 

HINSTANCE hInst;
CPlotDlg* childfig;
GRAPHY_EXPORT HWND hPlotDlgCurrent;

#define WM_PLOT_DONE	WM_APP+328

HANDLE mutexPlot;
static HANDLE hEvent1;
HWND hWndApp(NULL); // Settings and variables window
DWORD threadID; // delete this after 7/25/2019

#include <mutex>
#include <thread>
#include <condition_variable>
extern mutex mtx_OnPaint;
extern condition_variable cv_closeFig;

void initLineList(); // from Auxtra.cpp

class CGraffyEnv : public CWinApp
{
public:
	CGobj GraffyRoot;
	CAstSig *pctx;
	unsigned int cumulative;
	vector<CPlotDlg *> fig;
	vector<HWND> hDlg_fig;
	vector<HANDLE> figures();
	CAstSigEnv *pglobalEnv;
	HANDLE  findAxes(HANDLE ax);
	HANDLE findGObj(CSignals *xGO, CGobj *hGOParent = NULL);
	CFigure *findFigure(CSignals *xFig);
	HWND GetFigure(HANDLE h);
	HANDLE  openFigure(CRect *rt, const char* caption, HWND hWndAppl, int devID, double blocksize, const char * callbackID, HANDLE hIcon = NULL);
	HANDLE  openFigure(CRect *rt, HWND hWndAppl, int devID, double blocksize, const char * callbackID, HANDLE hIcon = NULL);
	HANDLE  openChildFigure(CRect *rt, HWND hWndAppl);
	multimap<HWND, RECT> redraw;
	int getselRange(CSignals *hgo, CSignals *out);
	int closeFigure(HANDLE h);
	string iniFilename;
	CRect defFigPos;
	CGraffyEnv();
	virtual ~CGraffyEnv();
private:
	int countFigures();
};

CGraffyEnv theApp;

#define THE_CPLOTDLG  static_cast <CPlotDlg*>(theApp.fig[id])

void CGobj::addRedrawCue(HWND h, RECT rt)
{
	auto it = theApp.redraw.begin();
	while (it != theApp.redraw.end())
	{
		if (it->first == h)
		{
			RECT dummyzero = {};
			// if it->second is already
			if (!memcmp(&dummyzero, &(it->second), sizeof(RECT)))
				return;
			// if rt is all zero, existing redraw is cleared and the 
			if (!memcmp(&dummyzero, &rt, sizeof(RECT)))
			{
				theApp.redraw.clear();
				theApp.redraw.insert(pair<HWND, RECT>(h, rt));
				return;
			}
			CRect crt0(it->second);
			CRect crt(rt);
			crt0.UnionRect(crt0, crt);
			it->second = crt0;
			return;
		}
		if (it++ == theApp.redraw.end())
		{
			theApp.redraw.insert(pair<HWND, RECT>(h, rt));
			return;
		}
	}
	theApp.redraw.insert(pair<HWND, RECT>(h, rt));
}

void CGobj::eraseRedrawCue(HWND hErase)
{
	auto it = theApp.redraw.find(hErase);
	if (it != theApp.redraw.end())
	{
		theApp.redraw.erase(it);
	}
}

void addRedrawCue(HWND hDlg, RECT rt)
{
	theApp.GraffyRoot.addRedrawCue(hDlg, rt);
}

void eraseRedrawCue(HWND hDlg)
{
	theApp.GraffyRoot.eraseRedrawCue(hDlg);
}

vector<CVar*> FindFigurebyvalue(const CVar &vals)
{
	vector<CVar*> out;
	char buf[256], buf2[16];
	//reject if it's a time sequence
	if (vals.IsTimeSignal())
		return out;
	for (unsigned int k = 0; k < vals.nSamples; k++)
	{
		double val = vals.buf[k];
		auto fit = theApp.fig.begin();
		for (; fit != theApp.fig.end(); fit++)
		{
			if ((*fit)->gcf.value() == val)
			{
				out.push_back((CVar*)&(*fit)->gcf);
				break;
			}
		}
	}


	CVar _vals = vals;
	{
		auto f = theApp.fig.begin();
		for (auto h : theApp.hDlg_fig)
		{
			GetWindowText(h, buf, sizeof(buf));
			if (_vals.IsString() && !strcmp(buf, _vals.string().c_str()))
			{
				out.push_back((CVar*)&(*f)->gcf);
				return out;
			}
			// Assume that vals is an integer vector.
			// make a case for vals with HDR_ARRAY
			else
			{ 
				for (unsigned k = 0; k < _vals.nSamples; k++)
				{
					sprintf(buf2, "Figure %d", (int)_vals.buf[k]);
					if (!strcmp(buf, buf2))
					{
						out.push_back((CVar*)&(*f)->gcf);
						break;
					}
				}
			}
		}
	}
	return out;
}

void invalidateRedrawCue()
{
	char buf[512];
	CRect zeros(0, 0, 0, 0);
	for (map<HWND, RECT>::iterator it = theApp.redraw.begin(); it != theApp.redraw.end(); it++)
	{
		GetWindowText(it->first, buf, sizeof(buf));
		sendtoEventLogger("(invalidateRedrawCue) %s\n", buf);
		if (!memcmp(&zeros, &(it->second), sizeof(RECT)))
			InvalidateRect(it->first, NULL, TRUE);
		else
			InvalidateRect(it->first, &it->second, TRUE);
	}
	theApp.redraw.clear();
}

static inline int getID4hDlg(HWND hDlg)
{
	size_t k(0);
	for (vector<HWND>::iterator it = theApp.hDlg_fig.begin(); it != theApp.hDlg_fig.end(); it++, k++)
	{
		if (hDlg == *it)   return (int)k;
	}
	return -1;
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	int id = getID4hDlg(hDlg);
	if (id<0) // This means theApp.hDlg_fig has not gotten hDlg for the created window, i.e., processing early messages prior to WM_INITDIALOG
	{
		//then add hDlg to the vector
		id = (int)theApp.hDlg_fig.size();
		theApp.hDlg_fig.push_back(hDlg);
	}
	switch (umsg)
	{
		if (theApp.hDlg_fig.empty()) break;
		chHANDLE_DLGMSG(hDlg, WM_INITDIALOG, static_cast <CPlotDlg*>(theApp.fig[id])->OnInitDialog);
		chHANDLE_DLGMSG(hDlg, WM_PAINT, THE_CPLOTDLG->OnPaint);
		chHANDLE_DLGMSG(hDlg, WM_SIZE, THE_CPLOTDLG->OnSize);
		chHANDLE_DLGMSG(hDlg, WM_CLOSE, THE_CPLOTDLG->OnClose);
		chHANDLE_DLGMSG(hDlg, WM_DESTROY, THE_CPLOTDLG->OnDestroy);
		chHANDLE_DLGMSG(hDlg, WM_COMMAND, THE_CPLOTDLG->OnCommand);
		chHANDLE_DLGMSG(hDlg, WM_RBUTTONUP, THE_CPLOTDLG->OnRButtonUp);
		chHANDLE_DLGMSG(hDlg, WM_LBUTTONUP, THE_CPLOTDLG->OnLButtonUp);
		chHANDLE_DLGMSG(hDlg, WM_LBUTTONDOWN, THE_CPLOTDLG->OnLButtonDown);
		chHANDLE_DLGMSG(hDlg, WM_LBUTTONDBLCLK, THE_CPLOTDLG->OnLButtonDblClk);
		chHANDLE_DLGMSG(hDlg, WM_MOUSEMOVE, THE_CPLOTDLG->OnMouseMove);
		chHANDLE_DLGMSG(hDlg, WM_MOVE, THE_CPLOTDLG->OnMove);
		chHANDLE_DLGMSG(hDlg, WM_KEYDOWN, THE_CPLOTDLG->OnKeyDown);
		chHANDLE_DLGMSG(hDlg, WM_TIMER, THE_CPLOTDLG->OnTimer);
		chHANDLE_DLGMSG (hDlg, WM_ACTIVATE, THE_CPLOTDLG->OnActivate);
		chHANDLE_DLGMSG(hDlg, WM__AUDIOEVENT, THE_CPLOTDLG->OnSoundEvent);
	default:
		return FALSE;
	}
	return TRUE;
}


BOOL CALLBACK enumproc(HWND hDlg, LPARAM lParam)
{
	char buf[256];
	GetWindowText(hDlg, buf, 256);
	HWND h = hDlg;
	HMODULE hAppInstance = GetModuleHandle(0);
	LONG_PTR out = GetWindowLongPtr(h, GWLP_HINSTANCE);
	if ((HMODULE)out == hAppInstance)
	{
		//Searching for Settings & Variables windows created within the current instance.
		if (strstr(buf, "Settings & Variables"))
			hWndApp = h;
	}
	return 1;
}

void initLineList(); // defined in Auxtra.cpp
void initGraffyProperties(); // defined in Auxtra.cpp


GRAPHY_EXPORT void initGraffy(CAstSig *base)
{
	if (!theApp.pctx)
	{
		theApp.pctx = base;
		theApp.pctx->fpmsg.SetGoProperties = SetGOProperties;
		theApp.pctx->fpmsg.RepaintGO = RepaintGO;
		theApp.pglobalEnv = base->pEnv;
	}
	initGraffyProperties();
	initLineList();
}

CGraffyEnv::CGraffyEnv()
{
	cumulative = 0;
	//	if (!mutexPlot) mutexPlot = CreateMutex(0, 0, 0);
	hEvent1 = CreateEvent(NULL, FALSE, FALSE, TEXT("graffy"));

#ifdef GRAFFY_STATIC 
	hInst = GetModuleHandle(NULL);
#endif

	char fullmoduleName[MAX_PATH], AppPath[256], moduleName[MAX_PATH];
	char drive[16], dir[256], ext[8], fname[MAX_PATH];

	GetModuleFileName(NULL, fullmoduleName, MAX_PATH);
	_splitpath(fullmoduleName, drive, dir, fname, ext);
	sprintf(AppPath, "%s%s", drive, dir);
	sprintf(moduleName, "%s%s", fname, ext);


#ifndef WIN64
	sprintf(fname, "%sauxp32.dll", AppPath);
#else
	sprintf(fname, "%sauxp64.dll", AppPath);
#endif
}

CGraffyEnv::~CGraffyEnv()
{
	for (vector<CPlotDlg*>::iterator it = fig.begin(); it != fig.end(); it++)
		delete *it;
	fig.clear();
	CloseHandle(hEvent1);
}

vector<HANDLE> CGraffyEnv::figures()
{
	vector<HANDLE> out;
	for (vector<CPlotDlg*>::iterator it = fig.begin(); it != fig.end(); it++)
		out.push_back(*it);
	return out;
}

HANDLE  CGraffyEnv::findAxes(HANDLE ax)
{
	for (vector<CPlotDlg*>::iterator it = fig.begin(); it != fig.end(); it++)
	{
		for (vector<CAxes*>::iterator jt = (*it)->gcf.ax.begin(); jt != (*it)->gcf.ax.end(); jt++)
		{
			if (ax == *jt) return ax;
		}
	}
	return ax;
}

HANDLE CGraffyEnv::findGObj(CSignals *xGO, CGobj *hGOParent)
{ 
	if (xGO->nSamples == 0) return NULL; // empty handle 
	if (hGOParent && *hGOParent == *xGO) return hGOParent; // check12/5
	if (hGOParent)
	{
		vector<CGobj*>::iterator it = hGOParent->child.begin();
		for (; it != hGOParent->child.end(); it++)
			if (xGO == (*it))
				return *it;
	}
	else
	{
		for (vector<CPlotDlg*>::iterator it = fig.begin(); it != fig.end(); it++)
		{
			HANDLE res(NULL);
			CGobj *htp = &(*it)->gcf;
			if (htp)
			{
				if (xGO == htp) return htp;
				res = findGObj(xGO, htp);
				if (res) return res;
				for (vector<CGobj*>::iterator jt = htp->child.begin(); jt != htp->child.end(); jt++)
				{
					res = findGObj(xGO, *jt);
					if (res) return res;
					for (vector<CGobj*>::iterator kt = (*jt)->child.begin(); kt != (*jt)->child.end(); kt++)
					{
						res = findGObj(xGO, *jt);
						if (res) return res;
					}
				}
			}
		}
	}
	return NULL;
}


GRAPHY_EXPORT HWND GetFigure(HANDLE h)
{
	return theApp.GetFigure(h);
}

HWND CGraffyEnv::GetFigure(HANDLE h)
{
	CFigure *cfig = (CFigure *)h;
	CVar *pgo = (CVar*)cfig;
	for (vector<CPlotDlg*>::iterator it = fig.begin(); it != fig.end(); it++)
	{
		if (*cfig == (*it)->gcf)
			return (*it)->hDlg;
		return NULL;
	}
	return NULL;
}

CFigure *CGraffyEnv::findFigure(CSignals *xFig)
{
	for (vector<CPlotDlg*>::iterator it = fig.begin(); it != fig.end(); it++)
		if (*xFig == (*it)->gcf)
			return &(*it)->gcf;
	return NULL;
}

HANDLE CGraffyEnv::openFigure(CRect *rt, HWND hWndAppl, int devID, double blocksize, const char * callbackID, HANDLE hIcon)
{
	return openFigure(rt, "", hWndAppl, devID, blocksize, callbackID, hIcon);
}

int CGraffyEnv::getselRange(CSignals *hgo, CSignals *out)
{
	HANDLE h = FindGObj(hgo);
	if (!h) return 0;
	CFigure *cfig(NULL);
	if (h) cfig = static_cast<CFigure *>(h);
	CPlotDlg *phDlg = (CPlotDlg*)(cfig->m_dlg);
	CSignals _out;
	int res = phDlg->GetSelect(&_out);
	if (res)
	{
		out->buf[0] = _out.buf[0];
		out->buf[1] = _out.buf[1];
	}
	return res;
}

HANDLE CGraffyEnv::openChildFigure(CRect *rt, HWND hWndAppl)
{
	CString s;
	CPlotDlg *newFig;
	fig.push_back(newFig = new CPlotDlg(hInst, NULL)); // this needs before CreateDialogParam

	// due to z-order problem in Windows 7, parent is set NULL for win7.
	if (!(newFig->hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_PLOT_CHILD), hWndAppl, (DLGPROC)DlgProc, (LPARAM)-1)))
	{
		MessageBox(NULL, "Cannot Create graffy dialog box", "", MB_OK);	fig.pop_back(); delete newFig; return NULL;
	}
	newFig->pctx = pctx;
	newFig->devID = 0;
	newFig->block = 0; //  7/7/2018
	newFig->title = NULL;
	//Prob. not necessary
	for (size_t k = 0; k < hDlg_fig.size(); k++)
	{
		RECT wndRt;
		GetWindowRect(hDlg_fig[k], &wndRt);
		if (*rt == wndRt) { rt->OffsetRect(20, 32); k = 0; }
	}
	newFig->MoveWindow(rt);
	newFig->gcf.setPos(rt->left, rt->top, rt->Width(), rt->Height());
	addRedrawCue(newFig->hDlg, CRect(0, 0, 0, 0));
	return &newFig->gcf;
}

HANDLE CGraffyEnv::openFigure(CRect *rt, const char* caption, HWND hWndAppl, int devID, double blocksize, const char * callbackID, HANDLE hIcon)
{
	CString s;
	CPlotDlg *newFig;
	EnumWindows(enumproc, 0);
	fig.push_back(newFig = new CPlotDlg(hInst, &GraffyRoot)); // this needs before CreateDialogParam

	// due to z-order problem in Windows 7, parent is set NULL for win7.
	if (!(newFig->hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_PLOT), isWin7() ? NULL : hWndAppl, (DLGPROC)DlgProc, (LPARAM)hWndAppl)))
	{
		MessageBox(NULL, "Cannot Create graffy dialog box", "", MB_OK);	fig.pop_back(); delete newFig; return NULL;
	}
	if (hIcon)
		SetClassLongPtr(newFig->hDlg, GCLP_HICON, (LONG)(LONG_PTR)hIcon);
	CRect RectApplication, rcCon;
	GetWindowRect(hWndAppl, RectApplication);
	int screenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYFULLSCREEN);
	GetWindowRect(GetConsoleWindow(), rcCon);

	newFig->pctx = pctx;
	newFig->devID = devID;
	newFig->block = blocksize; //  7/7/2018
	for (size_t k = 0; k<hDlg_fig.size(); k++)
	{
		RECT wndRt;
		GetWindowRect(hDlg_fig[k], &wndRt);
		if (*rt == wndRt) { rt->OffsetRect(20, 32); k = 0; }
	}
	newFig->MoveWindow(rt);
	newFig->gcf.setPos(rt->left, rt->top, rt->Width(), rt->Height());
	if (strlen(caption) == 0) // if (caption=="")    NOT THE SAME  in WIN64
	{
		s.Format("Figure %d", ++cumulative);
		newFig->SetWindowText(s);
		newFig->title = (char*)(INT_PTR)countFigures();
	}
	else
	{
		vector<string> tp;
		str2vect(tp, caption, ':');
		newFig->SetWindowText(tp[0].c_str());
		newFig->title = new char[strlen(caption) + 1];
		strcpy(newFig->title, caption);
	}
	addRedrawCue(newFig->hDlg, CRect(0,0,0,0));
	if (strlen(callbackID) > 0)
		strcpy(newFig->callbackIdentifer, callbackID);
	return &newFig->gcf;
}

int CGraffyEnv::countFigures()
{
	int out = 0;
	for (auto dlg : fig)
	{
		if (dlg->gcf.hPar)
			out++;
	}
	return out;
}

static int writePos(const char *fname, const RECT &pos, char *errStr)
{
	char buf[256];
	sprintf(buf, "%d %d %d %d", pos.left, pos.top, pos.right, pos.bottom);
	if (!printfINI(errStr, fname, "DEFAULT 1st FIG POS", "%s", buf))  return -1;
	return 0;
}

int CGraffyEnv::closeFigure(HANDLE h)
{
	CRect rt;
	char errstr[256];
	//Returns the number of fig dlg windows remaining.
	if (h == NULL) // delete all
	{
		GetWindowRect(hDlg_fig.back(), rt);
		if (!writePos(theApp.iniFilename.c_str(), rt, errstr))
		{
			//decide what to do 8/15/2020
		}
		fig.back();
		for (vector<CPlotDlg*>::iterator it = fig.begin(); it != fig.end(); it++)
			(*it)->OnClose(); // inside OnClose(), delete *it is called.
		fig.clear();
		hDlg_fig.clear();
	}
	else
	{
		if (hDlg_fig.size() == 1)
		{
			GetWindowRect(hDlg_fig.back(), rt);
			if (!writePos(theApp.iniFilename.c_str(), rt, errstr))
			{
				//decide what to do 8/15/2020
			}
		}
		// When the last remaining figure window is closed, store the position info
		CFigure *cfig = (CFigure*)h;
		for (vector<HWND>::iterator it = hDlg_fig.begin(); it != hDlg_fig.end(); it++)
		{
			if (cfig->m_dlg->hDlg == *it) { hDlg_fig.erase(it); break; }
		}
		for (vector<CPlotDlg*>::iterator it = fig.begin(); it != fig.end(); it++)
		{
			if (cfig->m_dlg == *it) {/*(*it)->OnClose(); */fig.erase(it); break; }
		}
		delete cfig->m_dlg;
	}
	return (int)fig.size();
}

#ifndef GRAFFY_STATIC 
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	hInst = hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//		makewmstr(wmstr);
		initLineList();
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif //GRAFFY_STATIC 

GRAPHY_EXPORT HANDLE FindWithCallbackID(const char *callbackid)
{
	for (auto mdlg : theApp.fig)
	{
		if (!strcmp(mdlg->callbackIdentifer, callbackid))
			return (HANDLE)(&mdlg->gcf);
	}
	return nullptr;
}

GRAPHY_EXPORT HACCEL GetAccel(HANDLE hFig)
{
	for (vector<CPlotDlg*>::iterator it = theApp.fig.begin(); it != theApp.fig.end(); it++)
		if (hFig == &(*it)->gcf)  return (*it)->GetAccel();
	return NULL;
}

GRAPHY_EXPORT HWND GetHWND_PlotDlg(HANDLE hFig)
{
	for (vector<CPlotDlg*>::iterator it = theApp.fig.begin(); it != theApp.fig.end(); it++)
		if (hFig == &(*it)->gcf)  return (*it)->hDlg;
	return NULL;
}

GRAPHY_EXPORT HWND GetHWND_PlotDlg2(HANDLE hFig)
{
	for (size_t i = 0; i<theApp.fig.size(); i++)
		if (hFig == theApp.fig[i])  return theApp.fig[i]->hDlg;
	return NULL;
}


static RECT readFigPos(const char *fname)
{
	char errStr[256];
	string strRead;
	int tar[4];
	RECT out;
	if (ReadINI(errStr, fname, "DEFAULT 1st FIG POS", strRead) >= 0 && str2array(tar, 4, strRead.c_str(), " ") == 4)
	{
		out.left = tar[0];
		out.top = tar[1];
		out.right = tar[2];
		out.bottom = tar[3];
	}
	else
	{
		// Default values if the default was not specified or unavailable
		out.left = 0;
		out.top = 50;
		out.right = 500;
		out.bottom = 310;
	}
	return out;
}

void thread4Plot(PVOID var)
{
	MSG         msg;
	CSignals gcf;
	GRAFWNDDLGSTRUCT *in = (GRAFWNDDLGSTRUCT *)var;
	CRect pos = readFigPos(theApp.iniFilename.c_str());
	if ((in->fig = OpenFigure(&pos, in->caption.c_str(), in->hWndAppl, in->devID, in->block, in->callbackID.c_str(), in->hIcon)) == NULL)
	{
		PostThreadMessage(in->threadCaller, WM_PLOT_DONE, 0, 0);
		return;
	}
	in->cfig = static_cast<CFigure *>(in->fig);
	in->hAccel = GetAccel(in->fig);
	in->threadPlot = GetCurrentThreadId();
	char buf[256];
	in->cfig->m_dlg->GetWindowText(buf, sizeof(buf));
	int figIDint;
	if (!strncmp(buf, "Figure ", 7))
	{
		sscanf(buf + 7, "%d", &figIDint);
		in->cfig->SetValue((double)figIDint);
	}
	else
		in->cfig->SetString(buf);
	sendtoEventLogger("from %d, open %s\n", in->threadCaller, buf);
	PostThreadMessage(in->threadCaller, WM_PLOT_DONE, 0, 0);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_DESTROY || !in->cfig->m_dlg)			
			break;
		if (!TranslateAccelerator(in->cfig->m_dlg->hDlg, in->hAccel, &msg))
		{
			if (!IsDialogMessage(msg.hwnd, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	CFigure *fg = (CFigure*)in->fig;
	CloseFigure(in->fig);
	delete in;
	cv_closeFig.notify_one();
}

GRAPHY_EXPORT HANDLE OpenChildFigure(CRect *rt, HWND hWndAppl)
{
	return theApp.openChildFigure(rt, hWndAppl);
}

void thread4PlotChild(PVOID var)
{
	MSG         msg;
	GRAFWNDDLGCHILDSTRUCT *in = (GRAFWNDDLGCHILDSTRUCT *)var;
	if ((in->fig = OpenChildFigure(&in->rt, in->hWndAppl)) == NULL)
	{
		PostThreadMessage(in->threadCaller, WM_PLOT_DONE, 0, 0);
		return;
	}
	in->threadPlot = GetCurrentThreadId();

	PostThreadMessage(in->threadCaller, WM_PLOT_DONE, 0, 0);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessage(msg.hwnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	delete in;
}

GRAPHY_EXPORT HANDLE OpenChildGraffy(GRAFWNDDLGCHILDSTRUCT &in)
{
	GRAFWNDDLGCHILDSTRUCT* pin = new GRAFWNDDLGCHILDSTRUCT;
	pin->hWndAppl = in.hWndAppl;
	pin->threadCaller = in.threadCaller;
	pin->rt = in.rt;
	_beginthread(thread4PlotChild, 0, (void*)pin);

	MSG         msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_PLOT_DONE)
		{
			in = *pin;
			break;
		}
	}
	return pin->fig;
}

GRAPHY_EXPORT void showvar2graffy(void *p)
{//To be used to send any block of data from showvar to graffy
 // iniFilename is sent for now 8/15/2020
	theApp.iniFilename = (char*)p;
}

GRAPHY_EXPORT HANDLE OpenGraffy(GRAFWNDDLGSTRUCT &in)
{ 
	GRAFWNDDLGSTRUCT* pin = new GRAFWNDDLGSTRUCT;
	pin->fig = NULL;
	pin->block = in.block;
	pin->scope = in.scope;
	pin->devID = 0;
	pin->hIcon = in.hIcon;
	pin->hWndAppl = in.hWndAppl;
	pin->threadCaller = in.threadCaller;
	pin->caption = in.caption;
	pin->rt = in.rt; // probably not necessary any more
	pin->callbackID = in.callbackID;
	_beginthread(thread4Plot, 0, (void*)pin);

	MSG         msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_PLOT_DONE)
		{
			in = *pin;
			break;
		}
	}
	return pin->fig;
}

GRAPHY_EXPORT int GetFigSelRange(CGobj *hgo, CSignals *out)
{
	return theApp.getselRange(hgo, out);
}

GRAPHY_EXPORT HANDLE OpenFigure(CRect *rt, HWND hWndAppl, int devID, double block, const char * callbackID, HANDLE hIcon)
{
	return theApp.openFigure(rt, "", hWndAppl, devID, block, callbackID, hIcon);
}

GRAPHY_EXPORT HANDLE OpenFigure(CRect *rt, const char *caption, HWND hWndAppl, int devID, double block, const char * callbackID, HANDLE hIcon)
{
	return theApp.openFigure(rt, caption, hWndAppl, devID, block, callbackID, hIcon);
}

GRAPHY_EXPORT int CloseFigure(HANDLE h)
{
	return theApp.closeFigure(h);
}

GRAPHY_EXPORT HANDLE FindGObj(CSignals *xGO, CGobj *hGOParent)
{
	return theApp.findGObj(xGO, hGOParent);
}

GRAPHY_EXPORT vector<HANDLE> graffy_Figures()
{
	return theApp.figures();
}

GRAPHY_EXPORT vector<CGobj*> graffy_CFigs()
{
	return theApp.GraffyRoot.child;
}

GRAPHY_EXPORT void graffy_remove_CFigs(CGobj* hRemove)
{


}

/*New 1*/
GRAPHY_EXPORT HANDLE FindFigure(CSignals *pfigsig)
{
	if (!pfigsig) return NULL; // can be NULL while deleting a multi-figure obj.
	// If pfigsig is CGobj, don't call this; Just cast.
	// (It won't be bad even if you call this, but you may just get confused what this is doing. 9/7/2020
	for (auto fig : theApp.fig)
	{
		int fs = fig->gcf.GetFs();
		if (fs == pfigsig->GetFs())
		{
			if (fs == 2 && fig->gcf.string() == pfigsig->string())
				return &fig->gcf;
			if (fs == 1 && fig->gcf.nSamples>0 && fig->gcf.value() == pfigsig->value())
				return &fig->gcf;
		}
	}
	return nullptr;
}

/*New 2*/
GRAPHY_EXPORT HANDLE FindFigure(HWND h)
{
	for (size_t k = 0; k<theApp.fig.size(); k++)
		if (theApp.fig[k]->hDlg == h)
			return &theApp.fig[k]->gcf;
	return NULL;
}

static HANDLE FindFigure(const CVar &sig)
{ // assumption: length of sig is 1
	auto input_type = sig.type();
	if (input_type & TYPEBIT_GO)
	{
		for (auto f : theApp.fig)
			if ((CVar)sig == f->gcf)
				return &f->gcf;
	}
	else if (input_type & TYPEBIT_STRING)
	{
		for (auto f : theApp.fig)
		{
			string title;
			f->GetWindowTextA(title);
			if (title == sig.string())
				return &f->gcf;
		}
	}
	else if (input_type & 1)
	{
		for (auto f : theApp.fig)
		{
			auto type = ((CVar*)&f->gcf)->type();
			// if f is named skip to the next
			if (type & TYPEBIT_STRING) {
				if ((INT_PTR)sig.value() == (INT_PTR)(CVar*)&f->gcf)
					return &f->gcf;
			}
			else if (sig.value() == f->gcf.value())
				return &f->gcf;
			else if (sig.value() == (double)(INT_PTR)&f->gcf)
				return &f->gcf;
		}
	}
	return NULL;
}

GRAPHY_EXPORT vector<HANDLE> FindFigures(const CVar &sig, vector<unsigned int> &idxFig)
{
	vector<HANDLE> out;
	idxFig.clear();
	auto type = sig.type();
	if (sig.nSamples == 0) return out;
	else if (sig.nSamples == 1 || (type & TYPEBIT_GO && sig.GetFs()!=3))
	{
		HANDLE _out = FindFigure(sig);
		if (_out)	out.push_back(_out);
		idxFig.push_back(0);
	}
	else
	{
		for (unsigned int k = 0; k < sig.nSamples; k++)
		{
			HANDLE _out = FindFigure(CSignals(sig.buf[k]));
			if (_out) {
				out.push_back(_out);
				idxFig.push_back(k);
			}
		}
	}
	return out;
}

GRAPHY_EXPORT vector<CVar*> FindNonFigures(const CVar &sig)
{
	vector<CVar*> out;
	auto type = sig.type();
	if (sig.nSamples == 0) return out;
	else if (sig.nSamples == 1 || (type & TYPEBIT_GO && sig.GetFs() != 3))
	{
		HANDLE _out = FindFigure(sig);
		if (!_out)	out.push_back((CVar*)(INT_PTR)sig.value());
	}
	else
	{
		for (unsigned int k = 0; k < sig.nSamples; k++)
		{
			HANDLE _out = FindFigure(CSignals(sig.buf[k]));
			if (!_out) out.push_back((CVar*)(INT_PTR)sig.buf[k]);
		}
	}
	return out;
}

static HANDLE GetGraffyHandle(const CVar &sig)
{
	if (sig.IsGO())
	{
		CGobj *pp = (CGobj*)&sig;
		if (pp->type == GRAFFY_figure)
			return NULL;
		return (HANDLE)&sig;
	}
	else
		return NULL;
}

GRAPHY_EXPORT HANDLE FindFigure(INT_PTR figID)
{
	string str;
	for (auto f : theApp.fig)
	{
		CFigure* pfig = &f->gcf;
		auto type = ((CVar*)pfig)->type();
		if (!(type & TYPEBIT_STRING) && (INT_PTR)pfig->value() == figID)
			return (HANDLE)pfig;
		for (auto ax : pfig->ax)
		{
			if ((INT_PTR)ax == figID) 
				return (HANDLE)ax;
			for (auto ln : ax->m_ln)
				if ((INT_PTR)ln == figID) return (HANDLE)ln;
		}
		for (auto txt : pfig->text)
		{
			if ((INT_PTR)txt == figID) return (HANDLE)txt;
		}
	}
	return NULL;
}

GRAPHY_EXPORT bool GetInProg(CVar *xGO)
{
	if (GOtype(xGO) != GRAFFY_figure) return false;
	CPlotDlg *phDlg = (CPlotDlg*)(((CFigure*)xGO)->m_dlg);
	return phDlg->getInProg();
}

GRAPHY_EXPORT void SetInProg(CVar *xGO, bool inprog)
{ // currently applicable only when xGO is CFigure
	if (GOtype(xGO) == GRAFFY_figure) {
		CPlotDlg *phDlg = (CPlotDlg*)(((CFigure*)xGO)->m_dlg);
		phDlg->setInProg(inprog);
	}
}

GRAPHY_EXPORT bool RegisterAx(CVar *xGO, CAxes *pax, bool b)
{
	if (GOtype(xGO) != GRAFFY_figure) return false;
	CPlotDlg *phDlg = (CPlotDlg*)(((CFigure*)xGO)->m_dlg);
	phDlg->Register(pax, b); // add the case for return false
	return true;
}

void showRMS(CVar *xGO, int code)
{
	if (GOtype(xGO) == GRAFFY_figure) {
		CPlotDlg *phDlg = (CPlotDlg*)(((CFigure*)xGO)->m_dlg);
		phDlg->dBRMS((SHOWSTATUS)code);
	}
}

GRAPHY_EXPORT void ViewSpectrum(HANDLE _h)
{
	CFigure *pfig = (CFigure *)_h;
	((CPlotDlg *)pfig->m_dlg)->ViewSpectrum();
}

GRAPHY_EXPORT HANDLE  AddAxes(HANDLE _fig, CPosition pos)
{
	CFigure *fig = (CFigure *)_fig;
	CAxes * ax = fig->axes(pos);
	for (size_t k = 0; k<theApp.fig.size(); k++)
	{
		if (&theApp.fig[k]->gcf == _fig)
			return theApp.fig[k]->gca = ax;
	}
	addRedrawCue(fig->m_dlg->hDlg, ax->rct);
	return ax;
}

GRAPHY_EXPORT HANDLE  AddAxes(HANDLE _fig, double x0, double y0, double wid, double hei)
{
	return AddAxes(_fig, CPosition(x0, y0, wid, hei));
}

GRAPHY_EXPORT HANDLE  AddText(HANDLE _fig, const char* text, double x0, double y0, double wid, double hei)
{
	CFigure *fig = (CFigure *)_fig;
	CPosition pos(x0, y0, wid, hei);
	CText *ctxt = fig->AddText(text, pos);
//	addRedrawCue(fig->m_dlg->hDlg, ctxt->textRect);
	return ctxt;
}

GRAPHY_EXPORT void SetRange(HANDLE _ax, const char xy, double x1, double x2)
{
	CAxes *ax = static_cast<CAxes *>(_ax);
	if (xy == 'x')
		ax->xlim[0] = x1, ax->xlim[1] = x2;
	else
		ax->ylim[0] = x1, ax->ylim[1] = x2;
}

GRAPHY_EXPORT void ShowStatusBar(HANDLE _fig)
{
	CFigure *pfig = static_cast<CFigure *>(_fig);
	((CPlotDlg*)pfig->m_dlg)->dBRMS();
}
// if (data.GetType() == CSIG_VECTOR) strcpy(ax->xtick.format, "%.0f"); // for non-audio, plot(x) call, don't bother to show any decimal point on x-axis.

static CTimeSeries Real(const CTimeSeries &x)
{
	CTimeSeries out(x.GetFs(), x.nSamples);
	if (x.IsComplex())
	{
		out.bufBlockSize = sizeof(double);
		for (unsigned int k = 0; k < x.nSamples; k++) out.buf[k] = x.buf[2 * k];
	}
	else
	{
		out = x;
	}
	return out;
}

static CTimeSeries Imag(const CTimeSeries &x)
{
	CTimeSeries out(x.GetFs(), (int)x.nSamples);
	if (x.IsComplex())
	{
		out.bufBlockSize = sizeof(double);
		for (unsigned int k = 0; k < x.nSamples; k++) out.buf[k] = x.buf[2 * k + 1];
	}
	else
	{
		out.UpdateBuffer(x.nSamples);
	}
	return out;
}

GRAPHY_EXPORT vector<HANDLE> PlotCTimeSeries(HANDLE _ax, double *x, const CTimeSeries &data, const std::string& vname, COLORREF col, char cymbol, LineStyle ls)
{
	// mono
	vector<HANDLE> out;
	CLine *lyne;
	CAxes *ax = static_cast<CAxes *>(_ax);
	if (data.IsComplex())
	{
		CTimeSeries real(data);
		real.SetReal();
		out.push_back(ax->plot(x, real, vname, col, cymbol, ls));
		CTimeSeries imag = Imag(data);
		if (HIBYTE(HIWORD(col))) *((char*)&col + 3) = 'l'; // real part in complex input, left channel
		out.push_back(ax->plot(x, imag, vname, col, cymbol, ls));
	}
	else
		out.push_back(lyne = ax->plot(x, data, vname, col, cymbol, ls));
	return out;
}

GRAPHY_EXPORT vector<HANDLE> PlotMultiLines(HANDLE _ax, double* x, const vector<CTimeSeries*>& line, const vector<string>& vnames, vector<COLORREF> cols, const vector<char>& cymbol, const vector<LineStyle>& ls)
{ 
	// if color is not specified, go this way-- b r g y c m
	vector<HANDLE> out;
	if (line.size() == 1)
	{
		CSignals* psig = (CSignals*)line.front();
		cols.push_back(-1);
		// Plotting with the default color scheme for a single csignals
		return PlotCSignals(_ax, x, *psig, vnames.front(), -1);
	}
	auto itcol = cols.begin();
	auto itls = ls.begin();
	auto itsym = cymbol.begin();
	auto itvname = vnames.begin();
	CAxes* ax = (CAxes*)_ax;
	vector<COLORREF> def_cols;
	if (itcol == cols.end())
	{
		def_cols.push_back(RGB(0, 0, 255));
		def_cols.push_back(RGB(255, 0, 0));
		def_cols.push_back(RGB(0, 255, 0));
		def_cols.push_back(RGB(255, 255, 0));
		def_cols.push_back(RGB(0, 255, 255));
		def_cols.push_back(RGB(255, 0, 255));
		cols = def_cols;
		itcol = cols.begin();
	}
	for (auto ln : line)
	{
		COLORREF col = 0;
		LineStyle style = LineStyle_solid;
		char symb = 0;
		if (itcol != cols.end()) col = *itcol, itcol++;
		if (itls != ls.end()) style = *itls, itls++;
		if (itsym != cymbol.end()) symb = *itsym, itsym++;
		out.push_back(ax->plot(x, *ln, (*itvname++), col, symb, style));
		if (((CSignals*)ln)->next)
		{
			itvname--;
			if (itcol != cols.end()) col = *itcol, itcol++;
			if (itls != ls.end()) style = *itls, itls++;
			if (itsym != cymbol.end()) symb = *itsym, itsym++;
			out.push_back(ax->plot(x, *(CTimeSeries*)(((CSignals*)ln)->next), (*itvname++), col, symb, style));
		}
	}
	CAxes* hAx = (CAxes*)ax;
	CGobj* hPar = ((CGobj*)ax)->hPar;
	bool existing = false;
	for (auto it = hPar->struts["children"].begin(); it != hPar->struts["children"].end(); it++)
		if ((*it) == hAx) { existing = true; break; }
	if (!existing)
		hPar->struts["children"].push_back(hAx);
	hPar->struts.erase("gca");
	hPar->struts["gca"].push_back(hAx);

	CAxes* paxFFT;
	if (paxFFT = (CAxes*)ax->hChild)
		ViewSpectrum(paxFFT);
	addRedrawCue(hPar->m_dlg->hDlg, CRect(0, 0, 0, 0));
	return out;
}

GRAPHY_EXPORT vector<HANDLE> PlotCSignals(HANDLE _ax, double *x, const CSignals &data, const std::string& vname, COLORREF col, char cymbol, LineStyle ls)
{
	CAxes *ax = static_cast<CAxes *>(_ax);
	vector<HANDLE> out;
	if (col == -1)
	{
		col = 0;		*((char*)&col + 3) = 'L';
	} // left channel
	out = PlotCTimeSeries(_ax, x, (const CTimeSeries)data, vname, col, cymbol, ls);
	if (data.next)
	{
		*((char*)&col + 3) = 'R';
		vector<HANDLE> out2 = PlotCTimeSeries(_ax, x, (const CTimeSeries)*data.next, vname, col, cymbol, ls);
		out.push_back(out2.front());
	}
	CAxes * hAx = (CAxes *)ax;
	CGobj * hPar = ((CGobj *)ax)->hPar;
	bool existing = false;
	for (auto it = hPar->struts["children"].begin(); it != hPar->struts["children"].end(); it++)
		if ((*it) == hAx) { existing = true; break; }
	if (!existing)
		hPar->struts["children"].push_back(hAx);
	hPar->struts.erase("gca");
	hPar->struts["gca"].push_back(hAx);

	CAxes * paxFFT;
	if (paxFFT = (CAxes*)ax->hChild)
		ViewSpectrum(paxFFT);
	addRedrawCue(hPar->m_dlg->hDlg, hAx->rct);
	addRedrawCue(hPar->m_dlg->hDlg, hAx->xtick.rt);
	addRedrawCue(hPar->m_dlg->hDlg, hAx->ytick.rt);
	return out;
}

bool Is_A_Ancestor_of_B(vector<INT_PTR> &A, vector<INT_PTR> &B)
{
	if (A.empty() || B.empty()) return false;
	vector<INT_PTR>::iterator it, jt;
	for (it = A.begin(), jt = B.begin(); it != A.end() && jt != B.end(); it++, jt++)
	{
		if ((*it) != (*jt)) return false;
	}
	if (it == A.end()) return true;
	return false;
}

GRAPHY_EXPORT bool Is_A_Ancestor_of_B(CSignals *A, CSignals *B)
{
	if (!A || !B) return false;
	if (A == B) return true;
	CGobj* a = (CGobj*)A;
	CGobj* b = (CGobj*)B;
	return Is_A_Ancestor_of_B(a->geneal, b->geneal);
}

void _deleteObj(CFigure *hFig)
{
	vector<HWND>::iterator jt;
	for (vector<CPlotDlg*>::iterator it = theApp.fig.begin(); it != theApp.fig.end(); it++)
		if (hFig == &(*it)->gcf)
		{
			jt = find(theApp.hDlg_fig.begin(), theApp.hDlg_fig.end(), (*it)->gcf.m_dlg->hDlg);
			SendMessage((*it)->gcf.m_dlg->hDlg, WM_DESTROY, 0, 0); // To exit the message loop forcefully, and avoid TranslateAccelerator with a null m_dlg. 7/31/2018
			(*it)->OnClose();
			delete *it; 
			theApp.fig.erase(it); 
			break;
		}
	// If this is done before the message loop closes, hDlg_fig will be push_back'ed again with the now soon-to-be-defunct hDlg and
	// it will cause a crash in CGraffyEnv::closeFigure  7/31/2018
	theApp.hDlg_fig.erase(jt); 
}

static void deep_erase(CVar* Govar, CVar* const del)
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
static void deep_erase(CAstSig* past, CVar* const del)
{
	for (auto gov = past->GOvars.begin(); gov != past->GOvars.end(); gov++)
	{ // gov is either single (you can/should use .front() or multiGO 
		if ((*gov).second.size() == 1)
			deep_erase((*gov).second.front(), del);
		else
		{
			// Taken care of by CAstSig::erase_GO(CVar * obj)
		}
	}
}

static int _delete_graffy_non_figure(CAstSig* past, HANDLE obj)
{
	if (!obj) return 0; // can be NULL while deleting a multi-figure obj.
	CGobj* hobj = (CGobj*)obj;
	CVar* pgo = (CVar*)obj;
	CVar* hPar = ((CFigure*)hobj)->hPar;
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
	InvalidateRect(GetHWND_PlotDlg(hobj), NULL, TRUE);
	return 1;
}

GRAPHY_EXPORT void deleteGObj(CAstSig* past, const CVar& sig)
{
	vector<unsigned int> fids;
	vector<HANDLE> figs2delete = FindFigures(sig, fids);
	vector<CVar*> nonfigs2delete = FindNonFigures(sig);
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
	// non-figures
	for (auto del : nonfigs2delete)
	{
		deep_erase(past, del);
		vector<string> varname = past->erase_GO(del);
		for (auto v : varname)
			var2deleted.push_back(v);
		_delete_graffy_non_figure(past, del);
	}
}

GRAPHY_EXPORT void deleteObj(HANDLE h)
{
	if (!h) return;
	CFigure *hpar;
	CAxes *hpax;
	CGobj* aa = static_cast<CGobj*>(h);
	switch (aa->type)
	{
	case 'f':
		_deleteObj((CFigure *)h);
		break;
	case 't':
	case 'a':
		hpar = (CFigure *)(static_cast<CGobj*>(h)->hPar);
		for (vector<CText*>::iterator it = hpar->text.begin(); it != hpar->text.end(); it++)
		{
			if (h == *it)
			{
				delete *it;
				hpar->text.erase(it);
				return;
			}
		}
		for (vector<CAxes*>::iterator it = hpar->ax.begin(); it != hpar->ax.end(); it++)
			if (h == *it)
			{
				delete *it;
				hpar->ax.erase(it);
				return;
			}
		break;
	case 'l':
		hpax = (CAxes *)(static_cast<CGobj*>(h)->hPar);
		for (vector<CLine*>::iterator it2 = hpax->m_ln.begin(); it2 != hpax->m_ln.end(); it2++)
			if (h == *it2)
			{
				delete *it2;
				hpax->m_ln.erase(it2);
				return;
			}
		break;
	}
}

GRAPHY_EXPORT void SetHWND_GRAFFY(HWND h)
{
	// Register it, so that we can communicate between graffy and the application about the status of gcf.
	// To notify the application of gcf status change, call PostMessage/SendMessage
//	hWndApp = h;
}

GRAPHY_EXPORT HWND GetHWND_GRAFFY()
{
	return nullptr;
//	return hWndApp;
}

GRAPHY_EXPORT DWORD CSignals2COLORREF(CSignals col)
{ // color data in CSignals (scaled between 0 and 1
	if (col.GetType() != CSIG_VECTOR || col.nSamples != 3) return -1;
	return (COLORREF)RGB((LONG)(col.buf[0]*255), (LONG)(col.buf[1] * 255), (LONG)(col.buf[2] * 255));
}

GRAPHY_EXPORT CSignals &COLORREF2CSignals(vector<double> col, CSignals &sig)
{
	sig.Reset(1);
	sig.UpdateBuffer(3 * (unsigned int)col.size());
	size_t k = 0;
	for (; k<col.size()/3; )
	{
		sig.buf[3 * k] = col[k] / 255;
		sig.buf[3 * k + 1] = col[k+1] / 255;
		sig.buf[3 * k + 2] = col[k+2] / 255;
		k+= 3;
	}
	sig.nGroups = (unsigned int)col.size();
	return sig;
}

GRAPHY_EXPORT CSignals &COLORREF2CSignals(vector<DWORD> col, CSignals &sig)
{
	sig.Reset(1);
	sig.UpdateBuffer(3 * (unsigned int)col.size());
	BYTE r, g, b;
	int k = 0;
	for (auto p : col)
	{
		r = GetRValue(p);
		g = GetGValue(p);
		b = GetBValue(p);
		sig.buf[3 * k] = (double)r / 255;
		sig.buf[3 * k + 1] = (double)g / 255;
		sig.buf[3 * k + 2] = (double)b / 255;
		k++;
	}
	sig.nGroups = (unsigned int)col.size();
	return sig;
}

GRAPHY_EXPORT void RepaintGO(CAstSig *pctx)
{ // make all figures created but unchecked for visible visible
	// to do: this now applies all figures, but should only apply to those created inside pctx
	char buf[512] = {};
	vector<CGobj*> h = graffy_CFigs();
	if (pctx)
	{
		for (auto fig = h.begin(); fig != h.end(); fig++)
		{
			CVar *pvar = (CVar*)*fig;
			CFigure * pfig = (CFigure *)*fig;
			if (pfig->visible == -1 && pfig->m_dlg->hDlg)
			{
				pfig->visible = 1;
				pfig->strut["visible"].SetValue(1.);
				// Don't just assume that tp->m_dlg->hDlg is available. 
				// It is possible  tp->m_dlg was initiated but tp->m_dlg->hDlg is not ready yet.
				// that's why tp->m_dlg->hDlg is checked in the if statement above
				// 8/22/2019
				pfig->m_dlg->ShowWindow(SW_SHOW);
				// ShowWindow(SW_SHOW) evokes WM_PAINT, so invalidateRedrawCue() shouldn't include this hDlg
				eraseRedrawCue(pfig->m_dlg->hDlg);
			}
			if (pctx->son)
			{
				auto hh = pfig->m_dlg->hDlg;
				auto finder = pctx->son->u.rt2validate.find(hh);
				if (finder != pctx->son->u.rt2validate.end())
				{
					InvalidateRect(hh, &finder->second, TRUE);
				}
			}
		}
	}
	if (!h.empty())
	{
		unique_lock<mutex> locker(mtx_OnPaint);
		sendtoEventLogger("(RepaintGO) mtx_OnPaint locked? %d", locker.owns_lock());
		invalidateRedrawCue();
		sendtoEventLogger("(RepaintGO) mtx_OnPaint unlocked.");
	}
}

GRAPHY_EXPORT graffytype GOtype(const CVar & obj)
{
	auto vv = obj.strut.find("type");
	if (vv == obj.strut.end()) return GRAFFY_no_graffy;
	if ((*vv).second.type() & TYPEBIT_STRING)
	{
		string typestr = (*vv).second.string();
		if (typestr == "figure") return GRAFFY_figure;
		if (typestr == "axes") return GRAFFY_axes;
		if (typestr == "line") return GRAFFY_line;
		if (typestr == "tick") return GRAFFY_tick;
		if (typestr == "axis") return GRAFFY_axis;
		if (typestr == "text") return GRAFFY_text;
		if (typestr == "patch") return GRAFFY_patch;
		else return GRAFFY_others;
	}
	else
		return GRAFFY_no_graffy;
}

GRAPHY_EXPORT void SetGOProperties(CAstSig *pctx, const char *proptype, const CVar & RHS)
{
	//put NULL for the first param to invoke InvalidateRect 
	CFigure *cfig;
	CAxes *cax;
	CLine *cline;
	CText *ctxt;
	HANDLE h;
	bool b = false;
	CSignals onoff(&b, 1);
	if (!isThisAllowedPropGO(pctx->pgo, proptype, RHS))
		throw CAstException(USAGE, pctx, NULL).proc("Invalid parameter for the property", proptype);
	CRect rt(0, 0, 0, 0);
	switch (GOtype(pctx->pgo))
	{
	case GRAFFY_figure:
		cfig = static_cast<CFigure *>(pctx->pgo);
		cfig->m_dlg->GetWindowRect(rt);
		if (!strcmp(proptype, "pos"))
		{
			rt.left = (LONG)RHS.buf[0];
			rt.top = (LONG)RHS.buf[1];
			rt.right = (LONG)RHS.buf[2] + rt.left;
			rt.bottom = (LONG)RHS.buf[3] + rt.top;
			cfig->m_dlg->MoveWindow(&rt);
		}
		else if (!strcmp(proptype, "color"))
		{
			cfig->color = CSignals2COLORREF(RHS);
		}
		else if (!strcmp(proptype, "visible"))
		{
			bool onoff = (bool)RHS.value();
			cfig->m_dlg->ShowWindow(onoff ? SW_SHOW : SW_HIDE);
		}
		break;
	case GRAFFY_axes:
		cax = static_cast<CAxes *>(pctx->pgo);
		cfig = (CFigure *)cax->hPar;
		rt = cax->rct;
		if (!strcmp(proptype, "pos"))
		{
			CVar axpos = pctx->pgo->strut["pos"];
			cax->setPos(axpos.buf[0], axpos.buf[1], axpos.buf[2], axpos.buf[3]);
			CRect rt0, rt2;
			cfig->m_dlg->GetWindowRect(rt0);
			rt2.left = (int)(axpos.buf[0] * rt0.Width());
			rt2.top = (int)(axpos.buf[1] * rt0.Height());
			rt2.right = rt2.left + (int)(axpos.buf[2] * rt0.Width());
			rt2.bottom = rt2.top + (int)(axpos.buf[3] * rt0.Height());
			rt.UnionRect(rt, rt2);
		}
		else if (!strcmp(proptype, "color"))
		{
			cax->color = CSignals2COLORREF(RHS);
			rt = cax->rct;
		}
		else if (!strcmp(proptype, "nextplot"))
		{
			//do this later 4/2/2019
			return;
		}
		else
		{
			cfig->m_dlg->GetWindowRect(rt);
			rt.MoveToXY(CPoint(0, 0));
		}
		break;
	case GRAFFY_axis:
		h = pctx->pgo->struts["parent"].front();
		cax = static_cast<CAxes *>(h);
		cfig = (CFigure *)cax->hPar;
		rt = cax->rct;
		rt.UnionRect(rt, cax->xtick.rt);
		rt.UnionRect(rt, cax->ytick.rt);
		rt.InflateRect(5, 5);
		if (!strcmp(proptype, "lim") && RHS.buf)
		{
			onoff.logbuf[0] = true;
			if (pctx->pgo->strut["xyz"].string() == string("x"))
			{
				memcpy(cax->xlim, RHS.buf, 2 * sizeof(double));
				cax->xtick.automatic = true;
				cax->xtick.tics1.clear();
			}
			else if (pctx->pgo->strut["xyz"].string() == string("y"))
			{
				memcpy(cax->ylim, RHS.buf, 2 * sizeof(double));
				cax->ytick.automatic = true;
				cax->ytick.tics1.clear();
			}
		}
		else if (!strcmp(proptype, "tick"))
		{
			if (pctx->pgo->strut["xyz"].string() == string("x"))
			{
				cax->xtick.automatic = false;
				cax->xtick.tics1 = RHS.body::ToVector();
			}
			else if (pctx->pgo->strut["xyz"].string() == string("y"))
			{
				cax->ytick.automatic = false;
				cax->ytick.tics1 = RHS.body::ToVector();
			}
		}
		else if (!strcmp(proptype, "ticklabel"))
		{
			if (pctx->pgo->strut["xyz"].string() == string("x"))
			{
				cax->xtick.ticklabel = RHS.string();
			}
			else if (pctx->pgo->strut["xyz"].string() == string("y"))
			{
				cax->ytick.ticklabel = RHS.string();
			}
		}
		pctx->pgo->strut["auto"] = onoff;
		break;
	case GRAFFY_line:
		cline = static_cast<CLine *>(pctx->pgo);
		cax = (CAxes *)cline->hPar;
		cfig = (CFigure *)cax->hPar;
		if (!strcmp(proptype, "marker"))
		{
			cline->symbol = RHS.string().c_str()[0];
		}
		else if (!strcmp(proptype, "markersize"))
			cline->markersize = (unsigned char)RHS.value();
		else if (!strcmp(proptype, "color"))
		{
			//			pctx->pgo->strut["color"] = COLORREF2CSignals(RHS.ToVector(), CSignals());
			{	cline->color = 0;		*((char*)&cline->color + 3) = 'M'; 	}
		}
		else if (!strcmp(proptype, "width"))
			cline->lineWidth = (unsigned char)RHS.value();
		else if (!strcmp(proptype, "linestyle"))
		{
			LineStyle tp = cline->GetLineStyle();
			if (tp == LineStyle_err)
				throw CAstException(ARGS, pctx, NULL).proc("linestyle must be one of the following\nnone - : -- -. ..", "");
			cline->lineStyle = tp;
		}
		else if (!strcmp(proptype, "visible"))
			cline->visible = (int)RHS.value();
		//else if (!strcmp(proptype, "xdata"))
		//{
		//	if (RHS.nSamples != pctx->pgo->strut["xdata"].nSamples) 
		//		throw CAstException(USAGE, pctx, NULL).proc("RHS elements must be equal to existing points", "");
		//	cline->xdata = RHS;
		//	cax->setxlim();
		//	cax->xtick.tics1.clear();
		//}
		else if (!strcmp(proptype, "ydata"))
			cline->sig = RHS;
		rt.UnionRect(cax->rct, cax->xtick.rt);
		//if (invalidateScreen)
		//{
		//	cfig->m_dlg->InvalidateRect(cax->rct); // invalidated rect should also include rects of xtick and ytick .... do it!
		//	cfig->m_dlg->InvalidateRect(cax->xtick.rt); // hhhhm... this is not working.... 8/3/2018 7:40pm
		//}
		break;
	case GRAFFY_text:
		ctxt = static_cast<CText *>(pctx->pgo);
		cfig = (CFigure *)ctxt->hPar;
		if (!strcmp(proptype, "fontname"))
			ctxt->ChangeFont(RHS.string().c_str(), (int)ctxt->strut["fontsize"].value());
		else if (!strcmp(proptype, "fontsize"))
			ctxt->ChangeFont(ctxt->strut["fontname"].string().c_str(), (int)RHS.value());
		else if (!strcmp(proptype, "pos"))
		{
			ctxt->pos.x0 = RHS.buf[0];
			ctxt->pos.y0 = RHS.buf[1];
		}
		else if (!strcmp(proptype, "string"))
			ctxt->str = RHS.string();
		else if (!strcmp(proptype, "color"))
			ctxt->color = CSignals2COLORREF(RHS);
		else if (!strcmp(proptype, "visible"))
			ctxt->visible = RHS.logbuf[0];
		rt = ctxt->textRect; // no need to worry about old vs new RECT; Re-repaint is done inside of OnPaint if necessary.
		break;
	}
	addRedrawCue(cfig->m_dlg->hDlg, rt);
}

GRAPHY_EXPORT vector<DWORD> Colormap(BYTE head, char lh, char rc, int nItems)
{
	// lh : Left or Right
	// rc : Real or Complex
	vector<DWORD> out;
	DWORD dw;
	vector<double> t;
	t.resize(nItems);
	double r, g, b;
	int ir, ig, ib;
	double k = 0;
	if (rc == 'c') k += nItems / 3.;
	for (auto &p : t)
	{
		p = k / nItems;
		k++;
		if (k>nItems) k -= nItems / 3.;
	}
	if (lh == 'R')
		for (auto p : t)
		{
			r = -1.78*p*p*p + 2.78*p*p - 1.94*p + 1;
			g = 1.42*p*p*p - 2.22*p*p + 1.56*p + 1.01;
			b = 2.13*p*p*p - 5.2*p*p + 3.7*p + .15;
			ir = (int)round(r * 255);
			ig = (int)round(g * 255);
			ib = (int)round(b * 255);
			dw = RGB(ir, ig, ib) + (head<<24);
			out.push_back(dw);
		}
	else if (lh == 'L')
		for (auto p : t)
		{
			r = .63*p*p*p + .44*p*p - .35*p + .051;
			g = 1.42*p*p*p - 2.22*p*p + 1.56*p + 1.01;
			b = fabs(.8 - .55*log(4 * p + 1));
			ir = (int)round(r * 255);
			ig = (int)round(g * 255);
			ib = (int)round(b * 255);
			dw = RGB(ir, ig, ib) + (head << 24);
			out.push_back(dw);
		}
	return out;
}

bool isThisAllowedPropGO(CVar *psig, const char *propname, const CVar &tsig)
{
	if (psig->strut.find(propname) == psig->strut.end())
		if (psig->struts.find(propname) == psig->struts.end())
			return false;
	if (!strcmp(propname, "pos")) // 4-element vector
		return (tsig.GetType() == CSIG_VECTOR && tsig.nSamples == 4);
	if (!strcmp(propname, "color")) // 3-element vector
		return tsig.nSamples == 3;
	if (!strcmp(propname, "visible")) // a real constant or bool
		return tsig.nSamples == 1 && tsig.bufBlockSize <= 8;
	if (!strcmp(propname, "nextplot"))
		return (tsig.GetType() == CSIG_STRING);
	if (!strcmp(propname, "tag"))
		return (tsig.GetType() == CSIG_STRING);
	if (!strcmp(propname, "userdata"))
		return true; // everything is allowed for userdata

	switch (GOtype(psig))
	{
	case GRAFFY_axes:
		if (!strcmp(propname, "x") || !strcmp(propname, "y"))
			return true;
		break;
	case GRAFFY_axis:
		if (!strcmp(propname, "lim"))
			return ((tsig.GetType() == CSIG_VECTOR || tsig.GetType() == CSIG_AUDIO) && tsig.nSamples == 2);
		if (!strcmp(propname, "tick"))
			return (tsig.GetType() == CSIG_VECTOR || tsig.GetType() == CSIG_EMPTY);
		if (!strcmp(propname, "ticklabel"))
			return (tsig.GetType() == CSIG_STRING || tsig.GetType() == CSIG_EMPTY);
		break;
	case GRAFFY_line:
		if (!strcmp(propname, "marker"))
		{
			if (tsig.GetType() != CSIG_STRING || tsig.nSamples != 2)
				return false;
			char ch = (char)tsig.logbuf[0];
			return strchr("os.x+*d^v<>ph", ch) != NULL;
		}
		if (!strcmp(propname, "markersize") || !strcmp(propname, "width"))
			return (tsig.GetType() == CSIG_SCALAR);
		if (!strcmp(propname, "xdata") || !strcmp(propname, "ydata"))
			return (tsig.GetType() == CSIG_VECTOR || tsig.GetType() == CSIG_AUDIO);
		if (!strcmp(propname, "linestyle"))
			return true; // check at SetGOProperties() in graffy.cpp
		break;
	case GRAFFY_text:
		if (!strcmp(propname, "fontsize"))
			return (tsig.GetType() == CSIG_SCALAR && tsig.bufBlockSize == 8);
		if (!strcmp(propname, "fontname") || !strcmp(propname, "string"))
			return (tsig.GetType() == CSIG_STRING);
		break;
		/* Do checking differently for different GO--fig, axes, axis, line, text 8/1/2018
		*/
	}
	return false;
}
