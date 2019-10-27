// AUXLAB 
//
// Copyright (c) 2009-2019 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: graffy
// Graphic Library (Windows only)
// 
// 
// Version: 1.5
// Date: 3/30/2019
// 

#include "PlotDlg.h"
#include "msgCrack.h"
#include <string.h>
#include <algorithm>

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
extern mutex mtx_OnPaint;

void initLineList(); // from Auxtra.cpp

class CGraffyEnv : public CWinApp
{
public:
	CGobj GraffyRoot;
	CAstSig *pctx;
	vector<CPlotDlg *> fig;
	vector<HWND> hDlg_fig;
	vector<HANDLE> figures();
	CAstSigEnv *pglobalEnv;
	HANDLE  findAxes(HANDLE ax);
	HANDLE findGObj(CSignals *xGO, CGobj *hGOParent = NULL);
	CFigure *findFigure(CSignals *xFig);
	HANDLE  openFigure(CRect *rt, const char* caption, HWND hWndAppl, int devID, double blocksize, const char * callbackID, HANDLE hIcon = NULL);
	HANDLE  openFigure(CRect *rt, HWND hWndAppl, int devID, double blocksize, const char * callbackID, HANDLE hIcon = NULL);
	HANDLE  openChildFigure(CRect *rt, HWND hWndAppl);
	multimap<HWND, RECT> redraw;
	int getselRange(CSignals *hgo, CSignals *out);
	int closeFigure(HANDLE h);
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
			if (!memcmp(&dummyzero, &(it->second), sizeof(RECT)))
				return;
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
	char buf[512], buf2[256];
	CRect zeros(0, 0, 0, 0);
	for (map<HWND, RECT>::iterator it = theApp.redraw.begin(); it != theApp.redraw.end(); it++)
	{
		GetWindowText(it->first, buf2, sizeof(buf2));
		sprintf(buf, "(invalidateRedrawCue) %s\n", buf2);
		sendtoEventLogger(buf);
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
		s.Format("Figure %d", countFigures());
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


int CGraffyEnv::closeFigure(HANDLE h)
{
	//Returns the number of fig dlg windows remaining.
	if (h == NULL) // delete all
	{
		for (vector<CPlotDlg*>::iterator it = fig.begin(); it != fig.end(); it++)
			(*it)->OnClose(); // inside OnClose(), delete *it is called.
		fig.clear();
		hDlg_fig.clear();
	}
	else
	{
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

void thread4Plot(PVOID var)
{
	MSG         msg;
	CSignals gcf;
	GRAFWNDDLGSTRUCT *in = (GRAFWNDDLGSTRUCT *)var;
	if ((in->fig = OpenFigure(&in->rt, in->caption.c_str(), in->hWndAppl, in->devID, in->block, in->callbackID.c_str(), in->hIcon)) == NULL)
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
	char sendbuffer[512];
	sprintf(sendbuffer, "from %d, open %s\n", in->threadCaller, buf);
	sendtoEventLogger(sendbuffer);
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
	CloseFigure(in->fig);
	delete in;
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
	pin->rt = in.rt;
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

GRAPHY_EXPORT HANDLE GetGraffyHandle(INT_PTR figID)
{
	string str;
	for (size_t k = 0; k<theApp.fig.size(); k++)
	{
		CFigure* cfig = &theApp.fig[k]->gcf;
		if (cfig->nSamples>0 && cfig->GetFs()!=2 && (INT_PTR)cfig->value() == figID) return (HANDLE)cfig;
		for (vector<CAxes*>::iterator paxit = cfig->ax.begin(); paxit != cfig->ax.end(); paxit++)
		{
			if ((INT_PTR)*paxit == figID) return (HANDLE)*paxit;
			for (size_t q = 0; q < (*paxit)->m_ln.size(); q++)
			{
				CLine *ln = (*paxit)->m_ln[q];
				if ((INT_PTR)ln == figID) return (HANDLE)ln;
			}
		}
		for (vector<CText*>::iterator ptxit = cfig->text.begin(); ptxit != cfig->text.end(); ptxit++)
			if ((INT_PTR)*ptxit == figID) return (HANDLE)*ptxit;
	}
	return NULL;
}

GRAPHY_EXPORT bool GetInProg(CVar *xGO)
{
	string type = xGO->strut["type"].string();
	if (type != "figure") return false;
	CPlotDlg *phDlg = (CPlotDlg*)(((CFigure*)xGO)->m_dlg);
	return phDlg->getInProg();
}

GRAPHY_EXPORT void SetInProg(CVar *xGO, bool inprog)
{ // currently applicable only when xGO is CFigure
	string type = xGO->strut["type"].string();
	if (type != "figure") return;
	CPlotDlg *phDlg = (CPlotDlg*)(((CFigure*)xGO)->m_dlg);
	phDlg->setInProg(inprog);
}

GRAPHY_EXPORT bool RegisterAx(CVar *xGO, CAxes *pax, bool b)
{
	string type = xGO->strut["type"].string();
	if (type != "figure") return false;
	CPlotDlg *phDlg = (CPlotDlg*)(((CFigure*)xGO)->m_dlg);
	phDlg->Register(pax, b); // add the case for return false
	return true;
}

void showRMS(CVar *xGO, int code)
{
	string type = xGO->strut["type"].string();
	if (type != "figure") return;
	CPlotDlg *phDlg = (CPlotDlg*)(((CFigure*)xGO)->m_dlg);
	phDlg->dBRMS((SHOWSTATUS)code);
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
	CFigure *fig = static_cast<CFigure *>(_fig);
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

GRAPHY_EXPORT vector<HANDLE> PlotCSignals(HANDLE _ax, double *x, CTimeSeries *pdata, COLORREF col, char cymbol, LineStyle ls)
{
	//ptdata is treated as non-stereo
	CTimeSeries *tp = ((CVar*)pdata)->next;
	((CVar*)pdata)->next = NULL;
	vector<HANDLE> out = PlotCSignals(_ax, x, (CSignals*)pdata, col, cymbol, ls);
	((CVar*)pdata)->next = tp;
	return out;
}

GRAPHY_EXPORT vector<HANDLE> PlotCSignals(HANDLE _ax, double *x, CSignals *pdata, COLORREF col, char cymbol, LineStyle ls)
{
	CLine *lyne;
	CAxes *ax = static_cast<CAxes *>(_ax);
	vector<HANDLE> out;
	if (col == -1)
	{
		col = 0;		*((char*)&col + 3) = 'L';
	} // left channel
	if (pdata->IsComplex())
	{
		CSignals copy(*pdata);
		copy.SetReal();
		out.push_back(ax->plot(x, &copy, col, cymbol, ls));
		pdata->Imag();
		if (HIBYTE(HIWORD(col))) *((char*)&col + 3) = 'l'; // real part in complex input, left channel
		out.push_back(ax->plot(x, pdata, col, cymbol, ls));
	}
	else
		out.push_back(lyne = ax->plot(x, pdata, col, cymbol, ls));
	if (pdata->next)
	{
		*((char*)&col + 3) = 'R';
		if (pdata->next->IsComplex())
		{
			CSignals copy(*pdata->next);
			copy.SetReal();
			out.push_back(ax->plot(x, &copy, col, cymbol, ls));
			pdata->next->Imag();
			if (HIBYTE(HIWORD(col))) *((char*)&col + 3) = 'r'; // real part in complex input, left channel
			out.push_back(ax->plot(x, pdata->next, col, cymbol, ls));
		}
		else
			out.push_back(ax->plot(x, pdata->next, col, cymbol, ls)); // Right channel
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
	addRedrawCue(hPar->m_dlg->hDlg, CRect(0,0,0,0));
	return out;
}

bool Is_A_Ancestor_of_B(vector<INT_PTR> &A, vector<INT_PTR> &B)
{
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
	CGobj* a = (CGobj*)A;
	CGobj* b = (CGobj*)B;
	if (!b->ptarray.empty()) return false;
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
	char buf[512] = {}, buf2[256];
	vector<CGobj*> h = graffy_CFigs();
	if (pctx)
	{
		for (auto fig = h.begin(); fig != h.end(); fig++)
		{
			CFigure * tp = (CFigure *)*fig;
			CVar *pp = (CVar*)tp;
			if (IsEventLoggerReady())
			{
				if (pp->GetFs() == 2)
					sprintf(buf, "(RepaintGO) %s ", pp->string().c_str());
				else if (pp->nSamples > 0)
					sprintf(buf, "(RepaintGO) Figure %d ", (int)pp->value());
				else
					sprintf(buf, "(RepaintGO) Figure (empty)");
			}
			if ( tp->visible == -1 && tp->m_dlg->hDlg)
			{
				tp->visible = 1;
				tp->strut["visible"].SetValue(1.);
				// Don't just assume that tp->m_dlg->hDlg is available. 
				// It is possible  tp->m_dlg was initiated but tp->m_dlg->hDlg is not ready yet.
				// that's why tp->m_dlg->hDlg is checked in the if statement above
				// 8/22/2019
				tp->m_dlg->ShowWindow(SW_SHOW);
				// ShowWindow(SW_SHOW) evokes WM_PAINT, so invalidateRedrawCue() shouldn't include this hDlg
				eraseRedrawCue(tp->m_dlg->hDlg);
				if (IsEventLoggerReady())
				{
					strcpy(buf2, "... made visible");
					strcat(buf, buf2);
					strcat(buf, "\n");
					sendtoEventLogger(buf);
				}
			}
		}
	}
	if (!h.empty())
	{
		unique_lock<mutex> locker(mtx_OnPaint);
		sprintf(buf, "(RepaintGO) mtx_OnPaint locked? %d\n", locker.owns_lock());
		sendtoEventLogger(buf);
		invalidateRedrawCue();
		sendtoEventLogger("(RepaintGO) mtx_OnPaint unlocked.\n");
	}
}

GRAPHY_EXPORT void SetGOProperties(CAstSig *pctx, const char *proptype, CVar RHS)
{
	//put NULL for the first param to invoke InvalidateRect 
	CFigure *cfig;
	HANDLE h;
	if (!pctx->isThisAllowedPropGO(pctx->pgo, proptype, RHS))
		throw CAstException(pctx, "Invalid parameter for the property", proptype);
	string type = pctx->pgo->strut["type"].string();
	CRect rt(0, 0, 0, 0);
	h = FindGObj(pctx->pgo);
	if (type == "figure")
	{
		cfig = static_cast<CFigure *>(h);
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
	}
	else if (type == "axes")
	{
		CAxes *cax = static_cast<CAxes *>(h);
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
	}
	else if (type == "axis")
	{
		CSignals onoff(false);
		h = FindGObj(pctx->pgo->struts["parent"].front());
		CAxes *cax = static_cast<CAxes *>(h);
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
				CSignals onoff(false);
			}
		}
		pctx->pgo->strut["auto"] = onoff;
	}
	else if (type == "line")
	{
		CLine *cline = static_cast<CLine *>(h);
		CAxes *cax = (CAxes *)cline->hPar;
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
				throw CAstException(pctx, "linestyle must be one of the following\nnone - : -- -. ..", "");
			cline->lineStyle = tp;
		}
		else if (!strcmp(proptype, "visible"))
			cline->visible = (int)RHS.value();
		else if (!strcmp(proptype, "xdata"))
		{
			if (RHS.nSamples != pctx->pgo->strut["xdata"].nSamples) 
				throw CAstException(pctx, "RHS elements must be equal to existing points", "");
			cline->xdata = RHS;
			cax->setxlim();
			cax->xtick.tics1.clear();
		}
		else if (!strcmp(proptype, "ydata"))
			cline->sig = RHS;
		rt.UnionRect(cax->rct, cax->xtick.rt);
		//if (invalidateScreen)
		//{
		//	cfig->m_dlg->InvalidateRect(cax->rct); // invalidated rect should also include rects of xtick and ytick .... do it!
		//	cfig->m_dlg->InvalidateRect(cax->xtick.rt); // hhhhm... this is not working.... 8/3/2018 7:40pm
		//}
	}
	else if (type == "text")
	{
		CText *ctxt = static_cast<CText *>(h);
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
