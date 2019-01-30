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
// Date: 1/30/2019
// 

#include "PlotDlg.h"
#include "msgCrack.h"
#include <string.h>
#include <algorithm>

#pragma data_seg ("shared")
vector<CAstSig*> xcomvecast;
vector<CAstSig*> CAstSig::vecast = xcomvecast;
#pragma data_seg ()

HINSTANCE hInst;
CPlotDlg* childfig;
GRAPHY_EXPORT HWND hPlotDlgCurrent;

#define WM_PLOT_DONE	WM_APP+328


HANDLE mutexPlot;
HANDLE hEvent;
HWND hWndApp(NULL);

void initLineList(); // from Auxtra.cpp

class CGraffyDLL : public CWinApp
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
	HANDLE  openFigure(CRect *rt, const char* caption, HWND hWndAppl, int devID, double blocksize, HANDLE hIcon = NULL);
	HANDLE  openFigure(CRect *rt, HWND hWndAppl, int devID, double blocksize, HANDLE hIcon = NULL);
//	int getselRange(CSignalsGO *hgo, CSignals *out);
	int closeFigure(HANDLE h);
	CGraffyDLL();
	virtual ~CGraffyDLL();
};

CGraffyDLL theApp;

#define THE_CPLOTDLG  static_cast <CPlotDlg*>(theApp.fig[id])

void SetGOProperties(CAstSig *pctx, const char *proptype, CVar RHS); //need to initialize during construction of CGraffyDLL object

int getID4hDlg(HWND hDlg)
{
	/*	FILE *fp=fopen("getID4hDlg.log","at");
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	char buffer[256];
	sprintf(buffer, "[%02d/%02d/%4d, %02d:%02d:%02d] getID4hDlg error\n", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
	fprintf(fp, buffer);
	fclose(fp);*/
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
	//	char buf[32];
	//	sprintf(buf, "%d: ", id);
	//	spyWM(hDlg, umsg, wParam, lParam, "track.txt", wmstr, exc, buf);
	switch (umsg)
	{
		chHANDLE_DLGMSG(hDlg, WM_INITDIALOG, THE_CPLOTDLG->OnInitDialog);
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
		//	chHANDLE_DLGMSG (hDlg, WM_ACTIVATE, THE_CPLOTDLG->OnActivate);
		chHANDLE_DLGMSG(hDlg, WM__AUDIOEVENT, THE_CPLOTDLG->OnSoundEvent);

		//case WM_ACTIVATE:  // x0006
		//	PostMessage(hDlg, WM_FIGURE_CLICKED, (WPARAM)THE_CPLOTDLG->gcf, 0);
		//	return FALSE;

		//	case WM_MOUSEACTIVATE: //0x0021
		//	case WM_NCLBUTTONDOWN: //0x00A1
		//		THE_CPLOTDLG->SetGCF();
		//		return FALSE;
		//		break;

		//	case WM_NCACTIVATE: // x0086
		//		PostMessage(hDlg, WM_FIGURE_CLICKED, (WPARAM)THE_CPLOTDLG->gcf, 0);
		//	res = THE_CPLOTDLG->OnNCActivate((BOOL)(wParam));
		//	SetWindowLong(hDlg, MSGRESULT, (LPARAM)(LRESULT)(res)); // this is where this message is properly returned
		break;

		//case WM_MOUSELEAVE:
		//case WM_NCMOUSELEAVE:
		//	THE_CPLOTDLG->HandleLostFocus(umsg);
		//	return TRUE;

		//case WM_NCHITTEST:
		//	res = DefWindowProc  (hDlg, umsg, wParam, lParam);
		//	SetWindowLong(hDlg, MSGRESULT, (LPARAM)(LRESULT)(res)); // this is where this message is properly returned
		//	if (res!=HTCLIENT)
		//		THE_CPLOTDLG->HandleLostFocus(umsg, res);
		//	break;

	default:
		return FALSE;
	}
	return TRUE;
}


CGraffyDLL::CGraffyDLL()
{
	//	if (!mutexPlot) mutexPlot = CreateMutex(0, 0, 0);
	hEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("graffy"));

	char fullmoduleName[MAX_PATH], AppPath[256], moduleName[MAX_PATH];
	char drive[16], dir[256], ext[8], fname[MAX_PATH];

	GetModuleFileName(NULL, fullmoduleName, MAX_PATH);
	_splitpath(fullmoduleName, drive, dir, fname, ext);
	sprintf(AppPath, "%s%s", drive, dir);
	sprintf(moduleName, "%s%s", fname, ext);

	pglobalEnv = new CAstSigEnv(22050);
	pctx = new CAstSig(pglobalEnv);
	pctx->u.application = "graffy";
	pglobalEnv->InitBuiltInFunctionList();
	pctx->fpmsg.SetGoProperties = SetGOProperties;

	CAstSig::vecast.push_back(pctx);

#ifndef WIN64
	sprintf(fname, "%sauxp32.dll", AppPath);
#else
	sprintf(fname, "%sauxp64.dll", AppPath);
#endif
	HANDLE hLib = LoadLibrary(fname); // fix this.... if the path has been changed in the middle, we are no longer in AppPath
	if (!hLib)
		printf("[Warning] Standard private UDF library %s not found\n", fname);
	else
	{
		string res, emsg;
		int id = 100; // the resource ID in auxp begins with 101 (harded-coded)
		while (1)
		{
			res = pctx->LoadPrivateUDF((HMODULE)hLib, ++id, emsg);
			if (res.empty())
				break;
		}
		FreeLibrary((HMODULE)hLib);
	}
}

CGraffyDLL::~CGraffyDLL()
{
	delete pctx;
	for (vector<CPlotDlg*>::iterator it = fig.begin(); it != fig.end(); it++)
		delete *it;
	fig.clear();
	delete pglobalEnv;
	CloseHandle(hEvent);
}

vector<HANDLE> CGraffyDLL::figures()
{
	vector<HANDLE> out;
	for (vector<CPlotDlg*>::iterator it = fig.begin(); it != fig.end(); it++)
		out.push_back(*it);
	return out;
}

HANDLE  CGraffyDLL::findAxes(HANDLE ax)
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

HANDLE CGraffyDLL::findGObj(CSignals *xGO, CGobj *hGOParent)
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


CFigure *CGraffyDLL::findFigure(CSignals *xFig)
{
	for (vector<CPlotDlg*>::iterator it = fig.begin(); it != fig.end(); it++)
		if (*xFig == (*it)->gcf)
			return &(*it)->gcf;
	return NULL;
}

HANDLE CGraffyDLL::openFigure(CRect *rt, HWND hWndAppl, int devID, double blocksize, HANDLE hIcon)
{
	return openFigure(rt, "", hWndAppl, devID, blocksize, hIcon);
}

//int CGraffyDLL::getselRange(CSignalsGO *hgo, CSignals *out)
//{
//	HANDLE h = FindGObj(hgo);
//	if (!h) return 0;
//	CFigure *cfig(NULL);
//	if (h) cfig = static_cast<CFigure *>(h);
//	CPlotDlg *phDlg = (CPlotDlg*)(cfig->m_dlg);
//	CSignals _out;
//	int res = phDlg->GetSelect(&_out);
//	if (res)
//	{
//		out->buf[0] = _out.buf[0];
//		out->buf[1] = _out.buf[1];
//	}
//	return res;
//}

HANDLE CGraffyDLL::openFigure(CRect *rt, const char* caption, HWND hWndAppl, int devID, double blocksize, HANDLE hIcon)
{
	CString s;
	CPlotDlg *newFig;
	fig.push_back(newFig = new CPlotDlg(hInst, &GraffyRoot)); // this needs before CreateDialogParam

	// due to z-order problem in Windows 7, parent is set NULL for win7.
	if (!(newFig->hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_PLOT), isWin7() ? NULL : hWndAppl, (DLGPROC)DlgProc, (LPARAM)hWndAppl)))
	{
		MessageBox(NULL, "Cannot Create graffy dialog box", "", MB_OK);	fig.pop_back(); delete newFig; return NULL;
	}
	ULONG_PTR rrr;
	if (hIcon)
		rrr = SetClassLongPtr(newFig->hDlg, GCLP_HICON, (LONG)(LONG_PTR)hIcon);
	newFig->pctx = pctx;
	newFig->devID = devID;
	newFig->block = blocksize; // this is ignored. 7/15/2016 bjk // revived 7/7/2018
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
		s.Format("Figure %d", hDlg_fig.size());
		newFig->SetWindowText(s);
		newFig->title = (char*)(INT_PTR)hDlg_fig.size();
	}
	else
	{
		vector<string> tp;
		str2vect(tp, caption, ':');
		newFig->SetWindowText(tp[0].c_str());
		newFig->title = new char[strlen(caption) + 1];
		strcpy(newFig->title, caption);
	}
	return &newFig->gcf;
}


int CGraffyDLL::closeFigure(HANDLE h)
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
	if ((in->fig = OpenFigure(&in->rt, in->caption.c_str(), in->hWndAppl, in->devID, in->block, in->hIcon)) == NULL)
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
	PostThreadMessage(in->threadCaller, WM_PLOT_DONE, 0, 0);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_DESTROY || !in->cfig->m_dlg)			
			break;
		if (!TranslateAccelerator(in->cfig->m_dlg->hDlg, in->hAccel, &msg))
		{
			if (msg.message == WM_KEYDOWN)
				if (msg.message == WM_KEYDOWN && msg.wParam == 17 && GetParent(msg.hwnd) == in->cfig->m_dlg->hDlg) // Left control key for window size adjustment
					msg.hwnd = in->cfig->m_dlg->hDlg;
			if (!IsDialogMessage(msg.hwnd, &msg))
			{
				//				SpyGetMessage(msg, "track.txt", wmstr, dum, "Dispatching ");
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			//			else
			//				SpyGetMessage2(msg, "track.txt", wmstr, dum, "Dialog ");
		}
	}
	CloseFigure(in->fig);
	delete in;
}

GRAPHY_EXPORT HANDLE OpenGraffy(GRAFWNDDLGSTRUCT &in)
{ // in in in out
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
	_beginthread(thread4Plot, 0, (void*)pin);
	//	DWORD dw = WaitForSingleObject(hEvent, INFINITE);

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
	return 0;
//	return theApp.getselRange(hgo, out);
}


GRAPHY_EXPORT HANDLE OpenFigure(CRect *rt, HWND hWndAppl, int devID, double block, HANDLE hIcon)
{
	return theApp.openFigure(rt, "", hWndAppl, devID, block, hIcon);
}

GRAPHY_EXPORT HANDLE OpenFigure(CRect *rt, const char *caption, HWND hWndAppl, int devID, double block, HANDLE hIcon)
{
	return theApp.openFigure(rt, caption, hWndAppl, devID, block, hIcon);
}

GRAPHY_EXPORT int CloseFigure(HANDLE h)
{
	//	delete h;
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
			if (fs == 1 && fig->gcf.value() == pfigsig->value())
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
		if (cfig->GetFs()!=2 && (INT_PTR)cfig->value() == figID) return (HANDLE)cfig;
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



GRAPHY_EXPORT void ShowSpectrum(HANDLE _h)
{
	CFigure *pfig = static_cast<CFigure *>(_h);
	CPlotDlg *tp = (CPlotDlg *)pfig->m_dlg;
	CAxes* paxFFT;
	for (vector<CAxes*>::iterator axt = tp->gcf.ax.begin(); axt != tp->gcf.ax.end(); axt++)
	{
		if (paxFFT = (CAxes*)(*axt)->hChild)
			tp->ShowSpectrum(paxFFT, *axt);
	}
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
	return fig->AddText(text, pos);
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
	((CPlotDlg*)pfig->m_dlg)->ShowStatusBar();
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
		out.push_back(ax->plot(x, pdata, col, cymbol, ls));
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
		ShowSpectrum(paxFFT);
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
	// it will cause a crash in CGraffyDLL::closeFigure  7/31/2018
	theApp.hDlg_fig.erase(jt); 
}

GRAPHY_EXPORT void deleteObj(HANDLE h)
{
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
	hWndApp = h;
}

GRAPHY_EXPORT HWND GetHWND_GRAFFY()
{
	return hWndApp;
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
	sig.UpdateBuffer(3* (unsigned int)col.size());
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

GRAPHY_EXPORT void SetGOProperties(CAstSig *pctx, const char *proptype, CVar RHS)
{
	HANDLE h = FindGObj(pctx->pgo);
	string type = pctx->pgo->strut["type"].string();
	if (type == "figure")
	{
		CFigure *cfig = static_cast<CFigure *>(h);
		if (!strcmp(proptype, "pos"))
		{
			RECT rt;
			rt.left = (LONG)RHS.buf[0];
			rt.top = (LONG)RHS.buf[1];
			rt.right = (LONG)RHS.buf[2] + rt.left;
			rt.bottom = (LONG)RHS.buf[3] + rt.top;
			cfig->m_dlg->MoveWindow(&rt);
		}
		else if (!strcmp(proptype, "color"))
		{
			cfig->color = CSignals2COLORREF(RHS);
			cfig->m_dlg->InvalidateRect(NULL);
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
		CFigure *cfig = (CFigure *)cax->hPar;
		if (!strcmp(proptype, "pos"))
		{
			CVar axpos = pctx->pgo->strut["pos"];
			cax->setPos(axpos.buf[0], axpos.buf[1], axpos.buf[2], axpos.buf[3]);
		}
		else if (!strcmp(proptype, "color"))
		{
			cax->color = CSignals2COLORREF(RHS);
			cfig->m_dlg->InvalidateRect(cax->rcAx);
		}
		else if (!strcmp(proptype, "nextplot"))
		{
			//don't do anything, just return;
			return;
		}
		else if (!strcmp(proptype, "x"))
		{
		}
		CRect rt;
		cfig->m_dlg->GetWindowRect(rt);
		rt.MoveToXY(CPoint(0, 0));
		cfig->m_dlg->InvalidateRect(rt);
	}
	else if (type == "axis")
	{
		CSignals onoff(false);
		h = FindGObj(pctx->pgo->struts["parent"].front());
		CAxes *cax = static_cast<CAxes *>(h);
		CFigure *cfig = (CFigure *)cax->hPar;
		if (!strcmp(proptype, "lim") && RHS.buf)
		{
			onoff.logbuf[0] = true;
			if (pctx->pgo->strut["xyz"].string() == string("x"))
			{
				memcpy(cax->xlim, RHS.buf, 2 * sizeof(double));
				cax->xtick.automatic = true;
			}
			else if (pctx->pgo->strut["xyz"].string() == string("y"))
			{
				memcpy(cax->ylim, RHS.buf, 2 * sizeof(double));
			}
		}
		else if (!strcmp(proptype, "tick"))
		{
			if (pctx->pgo->strut["xyz"].string() == string("x"))
			{
				cax->xtick.automatic = false;
				cax->xtick.tics1 = RHS.ToVector();
			}
			else if (pctx->pgo->strut["xyz"].string() == string("y"))
			{
				cax->ytick.automatic = false;
				cax->ytick.tics1 = RHS.ToVector();
				CSignals onoff(false);
			}
		}
		pctx->pgo->strut["auto"] = onoff;
		cfig->m_dlg->InvalidateRect(NULL);
	}
	else if (type == "line")
	{
		CLine *cline = static_cast<CLine *>(h);
		CAxes *cax = (CAxes *)cline->hPar;
		CFigure *cfig = (CFigure *)cax->hPar;
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
		else if (!strcmp(proptype, "visible"))
			cline->visible = RHS.value();
		else if (!strcmp(proptype, "xdata"))
		{
			if (RHS.nSamples != pctx->pgo->strut["xdata"].nSamples) throw "RHS elements must be equal to existing points.";
			cline->xdata = RHS;
			cax->setxlim();
			cax->xtick.tics1.clear();
		}
		else if (!strcmp(proptype, "ydata"))
			cline->sig = RHS;
		cfig->m_dlg->InvalidateRect(cax->rcAx); // invalidated rect should also include rects of xtick and ytick .... do it!
		cfig->m_dlg->InvalidateRect(cax->xtick.rt); // hhhhm... this is not working.... 8/3/2018 7:40pm
	}
	else if (type == "text")
	{
		CText *ctxt = static_cast<CText *>(h);
		CFigure *cfig = (CFigure *)ctxt->hPar;
		if (!strcmp(proptype, "fontname"))
			ctxt->ChangeFont(RHS.string().c_str(), (int)ctxt->strut["fontsize"].value());
		else if (!strcmp(proptype, "fontsize"))
			ctxt->ChangeFont(ctxt->strut["fontname"].string().c_str(), (int)RHS.value());
		else if (!strcmp(proptype, "pos"))
		{
			ctxt->pos.x0 = RHS.buf[0];
			ctxt->pos.y0 = RHS.buf[1];
		}
		else if (!strcmp(proptype, "color"))
			ctxt->color = CSignals2COLORREF(RHS);
		else if (!strcmp(proptype, "visible"))
			ctxt->visible = RHS.logbuf[0];
		cfig->m_dlg->InvalidateRect(NULL);
	}
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
/*
CFigure *GetCFigure(CSignalsGO *pgo)
{
	if (!pgo || pgo->geneal.empty()) return nullptr;
	return (CFigure*)pgo->geneal.front();
}

CFigure *GetAxes(CSignalsGO *pgo)
{
	if (!pgo || pgo->geneal.size() < 2 ) return nullptr;
	return (CFigure*)(pgo->geneal.front()+1);
}

CGobj * GetGraffyObj(CSignalsGO *pgo)
{
	if (pgo->type == "figure")
	{
		for (auto figDlg : theApp.fig)
		{
			if (GetCFigure(pgo) == &figDlg->gcf)
				return &figDlg->gcf;
		}
		return nullptr;
	}
	else if (pgo->type == "axes")
	{
		for (auto figDlg : theApp.fig)
		{
			if (GetCFigure(pgo) == &figDlg->gcf)
			{
				for (auto figDlg : theApp.fig)
			}
				return &figDlg->gcf;
		}
		return nullptr;

	}
		CFigure* cfig = &figDlg->gcf;
		if (cfig->pgo->GetFs() != 2 && (INT_PTR)cfig->pgo->value() == figID) return (HANDLE)cfig;
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
*/