// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: auxlab
// Main Application. Based on Windows API  
// 
// 
// Version: 1.498
// Date: 2/10/2019


#include "graffy.h" // this should come before the rest because of wxx820
#include <process.h>
#include "showvar.h"
#include "resource1.h"
#include "xcom.h"
#include "histDlg.h"
#include "TabCtrl.h"

#include <cstdlib>

#include "wavplay.h"

HWND hLog;
#define WM__LOG	WM_APP+0x2020

extern xcom mainSpace;

extern CDebugDlg mDebug; // delete?
extern unordered_map<string, CDebugDlg*> dbmap;

extern CWndDlg wnd;
extern CShowvarDlg mShowDlg;
extern char udfpath[4096];
extern void* soundplayPt;
extern double playbackblock;
extern vector<CAstSig*> xcomvecast;

HANDLE hEvent;
extern CHistDlg mHistDlg;
extern CDebugBaseDlg debugBase;
extern CTabControl mTab;

extern vector<UINT> exc;

vector<CWndDlg*> cellviewdlg;
vector<cfigdlg*> plots;

#define ID_HELP_SYSMENU1		33758
#define ID_HELP_SYSMENU2		33759
#define ID_HELP_SYSMENU3		33760
#define ID_HELP_SYSMENU4		33761

#define WM__NEWDEBUGDLG	WM_APP+0x2000

#define WM__AUDIOEVENT	WM_APP + WOM_OPEN
#define PROPCHANGED 0x2020

CAstSig * CDebugDlg::pAstSig = NULL;
int CShowvarDlg::nPlaybackCount = 0;

FILE *fp;

CFileDlg fileDlg;
multimap<string, string> plotsVS;
map<HWND, pair<string, string>> varPlotsHWND;
CFileDlg fileOpenSaveDlg;
char axlfullfname[_MAX_PATH], axlfname[_MAX_FNAME + _MAX_EXT];

HWND CreateTT(HINSTANCE hInst, HWND hParent, RECT rt, char *string, int maxwidth=400);
void closeXcom(const char *AppPath);

BOOL CALLBACK vectorsheetDlg (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam);
BOOL AboutDlg (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK debugDlgProc (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DebugBaseProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#define RMSDB(BUF,FORMAT1,FORMAT2,X) { double rms;	if ((rms=X)==-1.*std::numeric_limits<double>::infinity()) strcpy(BUF, FORMAT1); else sprintf(BUF, FORMAT2, rms); }

CWndDlg * Find_cellviewdlg(const char *name);

uintptr_t hDebugThread(NULL);
uintptr_t hDebugThread2(NULL);

void ValidateFig(const char* scope);

unsigned int WINAPI debugThread2 (PVOID var) ;

int MoveDlgItem(HWND hDlg, int id, CRect rt, BOOL repaint)
{
	HWND h = GetDlgItem(hDlg, id);
	if (!h) return 0;
	return ::MoveWindow(h, rt.left, rt.top, rt.Width(), rt.Height(), repaint);
}

BOOL CALLBACK logProc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	static int lid(0);
	char buf1[4096] = {}, buf2[4096] = {};
	int bytes;
	switch (umsg)
	{
	case WM_INITDIALOG:
		return 1;
	case WM__LOG-1:
		bytes = (int)wParam;
		if (lParam) strcpy(buf2, (char*)lParam);
		else buf2[0] = 0;
		EditPrintf(GetDlgItem(hDlg, IDC_LOG), "(before getline) %d:%d bytes %s\n", lid++, bytes, buf2);
		break;
	case WM__RCI:
		SetDlgItemInt(hDlg, IDC_RCI, (UINT)wParam, 1);
		break;
	case WM__LOG:
		bytes = (int)wParam;
		if (lParam) strcpy(buf2, (char*)lParam);
		else buf2[0] = 0;
		if (bytes>=0)
			EditPrintf(GetDlgItem(hDlg, IDC_LOG), "%d:%d bytes %s\n", lid++, bytes,  buf2);
		else
			EditPrintf(GetDlgItem(hDlg, IDC_LOG), "%d:%s\n", lid++, buf2);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			ShowWindow(hDlg, SW_HIDE);
			break;
		}
		return 1;
	default:
		return FALSE;
	}
	return 1;
}


HWND find_showvarDlg(HWND hplotdlg)
{
	for (vector<CWndDlg*>::iterator it = cellviewdlg.begin(); it != cellviewdlg.end(); it++)
	{
//		if ((*it)->dwUser == 8383)
//			((CVectorsheetDlg*)dlg)->OnClose();
//		else 
		if ((*it)->dwUser == 1010)
		{
			CShowvarDlg *p = (CShowvarDlg *)*it;
			for (vector<HWND>::iterator jt = p->plotDlgList.begin(); jt != p->plotDlgList.end(); jt++)
			{
				if (*jt == hplotdlg)
					return p->hDlg;
			}
		}
	}
	return NULL;
}

LRESULT CALLBACK HookProc2(int code, WPARAM wParam, LPARAM lParam)
{
	static	char varname[256];
	switch(code)
	{
	case HC_ACTION:
		SetEvent(hEvent);
	break;
	}
	return CallNextHookEx(NULL, code, wParam, lParam);
}

void SetGlovar(CVar *cfig)
{
	auto jt = CAstSig::vecast.front()->pEnv->glovar.find("gcf");
	if (jt != CAstSig::vecast.front()->pEnv->glovar.end())
	{
		(*jt).second.clear();
	}
	CAstSig::vecast.front()->pEnv->glovar["gcf"].push_back(cfig);
}

int isSameCSignals_for_GCF_purpose(CSignals *p1, CSignals *p2)
{
	if (p1->GetType()!=p2->GetType()) return 0;
	if (p1->length()!=p2->length()) return 0;
	if (p1->GetType()==CSIG_STRING)
	{
		if (p1->string()!=p2->string()) return 0;
		return 1;
	}
	else if (p1->GetType()==CSIG_SCALAR)
	{
		if (p1->value() != p2->value() )return 0;
		return 1;
	}
	return 0;
}

BOOL FSDlgProc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	int fs;
	int res;
	char errstr[256];
	switch (umsg)
	{
	case WM_INITDIALOG:
		fs = *(int*)lParam;
		res = SetDlgItemInt(hDlg, IDC_FS, fs, 0);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			fs = GetDlgItemInt(hDlg, IDC_FS, NULL, 0);
			if (fs < 500)
			{
				MessageBox(hDlg, "Sampling rate must be greater than 500 Hz.", "AUXLAB: Adjust Sample Rate", 0);
				break;
			}
			else
			{
				CAstSig::vecast.front()->pEnv->Fs = fs; //Sample rate adjusted
				for (map<string, CVar>::iterator what = CAstSig::vecast.front()->Vars.begin(); what != CAstSig::vecast.front()->Vars.end(); what++)
				{
					CSignals tpp = what->second;
					if (tpp.GetType() == CSIG_AUDIO)
					{
						tpp.Resample(fs, errstr); // Sometimes resample generates an array with slightly different length than requested and zeros are padded. If you want to track it down, check errstr here
						what->second = tpp;
					}
				}
				SetDlgItemInt(hDlg, IDC_FS, fs, 0);
			}
			EndDialog(hDlg, 1);
		}
		else if (LOWORD(wParam) == IDCANCEL)
			EndDialog(hDlg, 0);
		break;
	default:
		return FALSE;
	}
	return TRUE;

}

/* showvarplot -- plotting a variable with the Enter key 22/29/2018
1) If there's no precedence, create the figure window, add axes and plot it
2) If there is a figure window,
2-1) if it changes audio, non-audio or tseq to one of the other: make a plot as if it was new
2-2) if it maintains the type, delete line(s) and re-plot it/them, i,e., keep the existing xlim. For non-audio or tseq, keep xlim but re-establish ylim
2-2) audio mono -> audio stereo or vice versa: delete axes and make new plot(s), but keep the old xlim
*/

CFigure * CShowvarDlg::newFigure(CRect rt, string title, const char *varname, GRAFWNDDLGSTRUCT *pin)
{
	pin->hIcon = LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, 0);
	pin->hWndAppl = hDlg;
	pin->block = CAstSig::vecast.front()->audio_block_ms;
	pin->scope = pcast->u.title.empty() ? "base workspace" : pcast->u.title;
	pin->rt = rt;
	pin->caption = title;
	pin->threadCaller = GetCurrentThreadId();
	CFigure *out = (CFigure *)OpenGraffy(*pin);
	//update gcf
	OnPlotDlgCreated(varname, pin);
	out->SetString(varname);
	return out;
}

void On_F2(HWND hDlg, CAstSig f2sig)
{
	try {
		string emsg;
		f2sig.SetNewScript(emsg, "f3_channel_stereo_mono");
		f2sig.Compute();
	}
	catch (const char *_errmsg) {
		bool gotobase = false;
		if (!strncmp(_errmsg, "[GOTO_BASE]", strlen("[GOTO_BASE]")))
			gotobase = true;
		char *errmsg = (char *)_errmsg + (gotobase ? strlen("[GOTO_BASE]") : 0);
		// cleanup_nodes was called with CAstException
		if (strncmp(errmsg, "Invalid", strlen("Invalid")))
			MessageBox(hDlg, errmsg, "ERROR", 0);
		else
			MessageBox(hDlg, errmsg, "Syntax Error", 0);
	}
}

void CShowvarDlg::plotvar(CVar *psig, string title, const char *varname)
{
	static char buf[256];
	vector<HANDLE> plotlines;
	int type = psig->GetType();
	HWND hPlot = varname2HWND(varname);
	CGobj * hobj = (CGobj *)FindFigure(hPlot);
	if (!hobj)
	{
		if (psig->IsGO())
		{

		}
		else if (type == CSIG_AUDIO || type == CSIG_TSERIES || type == CSIG_VECTOR)
		{
			static GRAFWNDDLGSTRUCT in;
			CFigure * cfig = newFigure(CRect(0, 0, 500, 310), title.c_str(), varname, &in);
			plotlines = PlotCSignals(AddAxes(cfig, .08, .18, .86, .72), NULL, psig, -1);
			cfig->m_dlg->GetWindowText(buf, sizeof(buf));
			//For the global variable $gcf, updated whether or not this is named plot.
			SetGlovar((CVar*)cfig);
			if (psig->next)
			{
				On_F2(hDlg, pcast);
			}
			else
			{
				CRect rt;
				plotDlgList.push_back(cfig->m_dlg->hDlg);
				::GetWindowRect(cfig->m_dlg->hDlg, rt);
				rt.MoveToXY(10, 40);
				::MoveWindow(cfig->m_dlg->hDlg, rt.left, rt.top, rt.Width(), rt.Height(), 1);
			}
		}
	}
	else
	{ //essentially the same as CPlotDlg::SetGCF()
		if (hobj->m_dlg->hDlg != GetForegroundWindow())
			SetForegroundWindow(hobj->m_dlg->hDlg);
	}
}

double CShowvarDlg::plotvar_update2(CAxes *pax, CTimeSeries *psig)
{
	//Update sig
	while (!pax->m_ln.empty())
		deleteObj(pax->m_ln.front());
	((CVar*)pax)->struts["children"].clear();
	vector<HANDLE> plotlines = PlotCSignals(pax, NULL, psig, -1);
	double lower = 1.e100;
	for (auto lnObj : plotlines)
	{
		CLine *pp = (CLine *)lnObj;
		lower = min(lower, pp->sig.tmark);
	}
	return lower;
}

double CShowvarDlg::plotvar_update2(CAxes *pax, CSignals *psig)
{
	//Update sig
	while (!pax->m_ln.empty())
		deleteObj(pax->m_ln.front());
	((CVar*)pax)->struts["children"].clear();
	vector<HANDLE> plotlines = PlotCSignals(pax, NULL, psig, -1);
	double lower = 1.e100;
	for (auto lnObj : plotlines)
	{
		CLine *pp = (CLine *)lnObj;
		lower = min(lower, pp->sig.tmark);
	}
	return lower;
}

void CShowvarDlg::plotvar_update(CFigure *cfig, CVar *psig)
{
	// If current xlim can display any part of psig (in either channel)
	// and the variable named  stays mono-to-mono or stereo-to-stereo,
	// keep xlim
	// otherwise

	CTimeSeries *pChan = psig;
	CTimeSeries *pChan2 = psig->next;
	if (pChan2) psig->next = NULL;
	double  lowestTmark = 1.e100;
	vector<CTimeSeries *> input;
	input.push_back(pChan);
	input.push_back(pChan2);
	double xlimOld[2];
	memcpy(xlimOld, cfig->ax.front()->xlim, 2 * sizeof(double));
	if (cfig->ax.size()>1) // for now, assume only ax.size is 2 at the most 2/5/2019
	{
		xlimOld[0] = min(xlimOld[0], cfig->ax[1]->xlim[0]);
		xlimOld[1] = max(xlimOld[1], cfig->ax[1]->xlim[1]);
	}
	auto it = input.begin();
	for (auto ax : cfig->ax)
	{
		if (it == input.end())		break;
		lowestTmark = plotvar_update2(ax, *it);
		it++;
	}	
	double xlim[2] = { 1.e100 , -1.e100, };
	if (pChan && pChan->nSamples > 1)
	{
		xlim[0] = min(xlim[0], pChan->tmark);
		xlim[1] = max(xlim[1], pChan->alldur());
	}
	if (pChan2 && pChan2->nSamples > 1)
	{
		xlim[0] = min(xlim[0], pChan2->tmark);
		xlim[1] = max(xlim[1], pChan2->alldur());
	}
	xlim[0] /= 1.e3; xlim[1] /= 1.e3;
	if (lowestTmark > xlimOld[1] || cfig->ax.front()->xlim[1] < xlimOld[0])
	{ // update xlim
		for (auto ax : cfig->ax)
		{
			ax->xtick.tics1.clear();
			if (psig->GetType() == CSIG_VECTOR) ax->ytick.tics1.clear();
			ax->xlim[1] = xlim[1]; // keep ax->xlim[0]
		}
	}
	else
	{ // keep xlim
		for (auto ax : cfig->ax)
		{
			ax->xtick.tics1.clear();
			if (psig->GetType() == CSIG_VECTOR) ax->ytick.tics1.clear();
			memcpy(ax->xlim, xlimOld, 2 * sizeof(double));
		}
	}
	psig->next = pChan2;
}


LRESULT CALLBACK HookProc(int code, WPARAM wParam, LPARAM lParam)
{
	static char varname[256];
	CVar *pgcfNew, *psig;
	MSG *pmsg = (MSG*)lParam;
	switch (code)
	{
	case HC_ACTION:
		if ((pmsg->message >= WM_NCLBUTTONDOWN && pmsg->message <= WM_NCMBUTTONDBLCLK) || ((pmsg->message >= WM_LBUTTONDOWN && pmsg->message <= WM_MBUTTONDBLCLK)))
		{
			//Things about how to consolidate this with case WM_GCF_UPDATED: in showvarDlg()
			//Seems like this is better approach because it checks the scope and update gcf accordingly... 
			//But in fact gcf should be a global, environmental variable for AUXLAB, so maybe it's better not to think about the scope... 8/17/2017 bjk
			pgcfNew = (CVar*)FindFigure(pmsg->hwnd);
			//what is the current workspace? Let's find out from IDC_DEBUGSCOPE
			LRESULT res = mShowDlg.SendDlgItemMessage(IDC_DEBUGSCOPE, CB_GETCURSEL);
			if (pgcfNew)
			{
				//For the global variable $gcf, updated whether or not this is named plot.
				SetGlovar(pgcfNew);
				if (CAstSig::vecast.at(res)->GetVariable("gcf") != pgcfNew)
				{
					//For the regular variable gcf, updated only if this is not named plot.
					if (pgcfNew->GetFs() != 2)
					{
						CAstSig::vecast.at(res)->SetVar("gcf", pgcfNew);
						mShowDlg.Fillup();
					}
				}
			}
		} 
		else if (pmsg->message == WM_KEYDOWN)
		{
			if (pmsg->wParam == VK_F2)
			{
				map<string, vector<CVar*>>::iterator jt = CAstSig::vecast.front()->pEnv->glovar.find("gcf");
				if (jt != CAstSig::vecast.front()->pEnv->glovar.end())
				{ // ax must be dual; else ax must have two line objects
					auto chvector = (*jt).second.front()->struts["children"];
					size_t nAxes = chvector.size();
					size_t nLines=0;
					bool FFTpaxExists = false;
					for (auto it : chvector.front()->struts["children"])
					{
						if ((*it).strut["type"].string() == "line")
							nLines++;
						else if ((*it).strut["type"].string() == "axes")
							FFTpaxExists = true;
					}
					if (FFTpaxExists) 
						ViewSpectrum((*jt).second.front());
					if (nAxes ==2 ||
						(nAxes == 1 && nLines ==2) )
						On_F2(pmsg->hwnd, CAstSig::vecast.front());
					if (FFTpaxExists) 
						ViewSpectrum((*jt).second.front());
				}
			}
		}
		else if (pmsg->message==WM__VAR_CHANGED)
		{
			cfigdlg *thisDlg = (cfigdlg*)pmsg->wParam;
			auto it = thisDlg->pcast->Vars.find(thisDlg->var);
			if (it == thisDlg->pcast->Vars.end())
			{
				HWND hp = find_showvarDlg(thisDlg->hDlg); // works with win7 and later
				DestroyWindow(thisDlg->hDlg);
				SendMessage(hp, WM__PLOTDLG_DESTROYED, (WPARAM)thisDlg->var.c_str(), (LPARAM)thisDlg->hDlg);
			}
			else
			{
				psig = &thisDlg->pcast->Vars[thisDlg->var];
				int type = psig->GetType();
				if (type == CSIG_AUDIO || type == CSIG_TSERIES || type == CSIG_VECTOR)
				{
					CFigure *cfig = (CFigure *)FindFigure(thisDlg->hDlg);
					CTimeSeries *poldsig = &(cfig->ax.front()->m_ln.front()->sig);
					// cfig->ax must have one or two elements (no zero element)
					if (cfig->ax.size() > 1)
					{ // the old sig is audio, stereo
						if (type == CSIG_TSERIES || type == CSIG_VECTOR)
						{
							//delete both axes and plot afresh
						}
						else if (psig->next)
						{ // new sig is audio, stereo. Update sig in each axes and keep xlim
							mShowDlg.plotvar_update(cfig, psig);

						}
						else
						{ //new sig is audio mono. Delete one axes. Update sig in the remaining axes and keep xlim

						}
					}
					else // previously one axes
					{
						
						switch (type)
						{
						case CSIG_AUDIO:
							if (psig->next)
							{ // previously one, now two axes needed
							  //Update the line objects and call OnF2
								mShowDlg.plotvar_update2(cfig->ax.front(), psig);
	//							On_F2(pmsg->hwnd, mShowDlg.pcast);
							}
							else
							{
								mShowDlg.plotvar_update(cfig, psig);
							}
							break;
						case CSIG_VECTOR:
							//Update sig
							mShowDlg.plotvar_update(cfig, psig);
							//update 
							break;
						case CSIG_TSERIES:
							break;
						}
						// if poldsig->type is the same as type, keep xlim. Otherwise, re-establish xlim.
					}
				}
				else // if the variable is no longer unavable, audio or vector, exit the thread (and delete the fig window)
					PostThreadMessage(GetCurrentThreadId(), WM_QUIT, 0, 0);
			}
			break;
		}
		else if (pmsg->message==WM_QUIT)
		{
			GetWindowText(pmsg->hwnd, varname, sizeof(varname));
			SendMessage(mShowDlg.hDlg, WM__PLOTDLG_DESTROYED, (WPARAM)varname, (LPARAM)pmsg->hwnd);
		}
		break;
	}
	return CallNextHookEx(NULL, code, wParam, lParam);
}

BOOL CALLBACK showvarDlgProc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	static int nPlaybackCount(0);
//	spyWM(hDlg, umsg, wParam, lParam, "c:\\temp\\rec", exc, "[showvarDlg]");
	CShowvarDlg *cvDlg(NULL);
	for (vector<CWndDlg*>::iterator it=cellviewdlg.begin(); it!=cellviewdlg.end(); it++)
	{
		// Right after a new instance of CShowvarDlg is created, it is stored in cellviewdlg
		// (*it)->hDlg will be NULL until the message WM_INITDIALOG is returned and can't be matched with cvDlg
		// So if it is NULL, just assume that it is cvDlg, the last created CShowvarDlg * with no hDlg yet available.
		// NOTE 1) If you use this style of message proc, make sure to call cellviewdlg.push_back() right after the instantiation of CShowvarDlg.
		// NOTE 2) When CreateDialog is called, prior to WM_INITDIALOG, several other messages come to proc, such as WM_SETFONT, WM_NOTIFYFORMAT and WM_QUERYUISTATE. 
		//        If the processing of cvDlg with NULL hDlg was not done right, these message wouldn't be processed properly.
		// 3/30/2018		
		if ( !(*it)->hDlg || (*it)->hDlg == hDlg)
		{	cvDlg = (CShowvarDlg *)*it; break;}
	}
	if (!cvDlg)
	{
//		return 0;
//		FILE *fp = fopen("c:\\temp\\rec", "at");
//		fprintf(fp, "NULL----");
//		fclose(fp);
	}
	switch (umsg)
	{
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_ACTIVATE:
		SetForegroundWindow (hDlg);
		if (cvDlg->changed) 
			cvDlg->Fillup();
		break;
	chHANDLE_DLGMSG (hDlg, WM_INITDIALOG, cvDlg->OnInitDialog);
	chHANDLE_DLGMSG (hDlg, WM_SIZE, cvDlg->OnSize);
	chHANDLE_DLGMSG (hDlg, WM_CLOSE, cvDlg->OnClose);
	chHANDLE_DLGMSG (hDlg, WM_SHOWWINDOW, cvDlg->OnShowWindow);
	chHANDLE_DLGMSG (hDlg, WM_COMMAND, cvDlg->OnCommand);
	chHANDLE_DLGMSG (hDlg, WM_SYSCOMMAND, cvDlg->OnSysCommand);
	chHANDLE_DLGMSG (hDlg, WM_CLOSE_FIG, cvDlg->OnCloseFig);
	chHANDLE_DLGMSG (hDlg, WM__AUDIOEVENT, cvDlg->OnSoundEvent);

	case WM_APP + PROPCHANGED:
	{
		CGobj *p = (CGobj *)wParam;
		char *propname = (char*)lParam;

		for (map<string, vector<CVar *>>::iterator it = cvDlg->pGOvars->begin(); it != cvDlg->pGOvars->end(); it++)
		{
			if (p->nSamples && (*it).second.front() == p)
			{
				(*it).second.front()->strut[propname] = p->strut[propname];
				cvDlg->UpdateProp((*it).first, (*it).second.front(), propname);
				break;
			}
		}

	}

		break;

	case WM__PLOTDLG_CREATED: // Posted by plotThread when the plot dlg is displayed.
		//wParam: var name. If Figure __, that means generic figure window, not attached to a variable
		//lParam: pointer to the GRAFWNDDLGSTRUCT structure
		cvDlg->OnPlotDlgCreated((char*)wParam, (GRAFWNDDLGSTRUCT*)lParam);
		break;

	case WM__PLOTDLG_DESTROYED: // Posted by plotThread when the plot dlg is displayed.
		cvDlg->OnPlotDlgDestroyed((char*)wParam, (HWND)lParam);
		break;

	//	chHANDLE_DLGMSG (hDlg, WM_NOTIFY, cvDlg->OnNotify);
	case WM_NOTIFY: //This cannot be through msg cracker... why? NM_CUSTOMDRAW inside WM_NOTIFY needs to call SetWindowLongPtr to have an effect. Msg cracker uses a vanila SetWindowLongPtr for everything without special needs. 5/1/2016 bjkwon
		cvDlg->OnNotify(hDlg, (int)(wParam), lParam);
		break;

	case WM_GCF_UPDATED:
	{
		//CSignals gcf;
		//GetFigID((HANDLE)wParam, gcf);
		//cvDlg->pcast->SetVar("gcf", gcf);
		//cvDlg->OnCommand(IDC_REFRESH, NULL, 0);
		break;
	}
	case WM_DEBUG_CLOSE:
		if (dbmap.find((char*)wParam)!=dbmap.end())
		{
			unordered_map<string, CDebugDlg*>::iterator it = dbmap.find((char*)wParam);
			delete it->second;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

CShowvarDlg::CShowvarDlg(CWndDlg* hPar, map<string, CVar> *pvars)
:changed(false), win7(false), pcastCreated(false), cellview(false), list_head_height(0), idc_stop_height(0), pVars(NULL), pGOvars(NULL), pcast(NULL)
{
	dwUser = 1010;
	hParent = hPar;
	pVars = pvars;
	cellviewdlg.push_back(this);
	memset(&LvItem, 0, sizeof(LvItem));
	memset(&LvCol, 0, sizeof(LvCol));
	LvItem.mask = LVIF_TEXT;   // Text Style
	LvItem.cchTextMax = 256; // Max size of text
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
}

CShowvarDlg::~CShowvarDlg(void)
{
	pVars = NULL;
	pGOvars = NULL;
	lastDebug = NULL;
	for (vector<CWndDlg*>::iterator it = cellviewdlg.begin(); it != cellviewdlg.end(); it++)
	{
		if (this == *it)
		{
			cellviewdlg.erase(it);
			break;
		}
	}
	if (pcastCreated)
		delete pcast;
}

/* Changes made on 10/25/2018
	OnVarChanged(CAstSig *pcast) always scans all figure windows and makes necessary updates 
	which is necessary for during debugging or a N_BLOCK call in non-debugging mode
	To update only a specified variable in the current workspace, which is necessary to limit updating if there are too many variables or anything complicated,
	use void CShowvarDlg::OnVarChanged(const char *varname, CSignals *sig)
*/

void CShowvarDlg::OnVarChanged(const char *varname)
{
	for (auto figdlg : plots)
	{ // update all variable figure windows in the current scope
		string scopename = figdlg->scope;
		if (scopename == "base workspace") scopename.clear();
		if (scopename != pcast->u.title) continue; // skip if not in the same scope
		if (figdlg->var.find("Figure ")==0) continue; //skip numbered window
		figdlg->pcast = pcast; // need to update; figdlg->pcast might be obsolete lingering from the last udf call
		if (!varname || !strlen(varname))
			PostThreadMessage(figdlg->threadID, WM__VAR_CHANGED, (WPARAM)figdlg, 0);
		else if (figdlg->var == varname)
		{
			PostThreadMessage(figdlg->threadID, WM__VAR_CHANGED, (WPARAM)figdlg, 0);
			return;
		}
	}
}

void CShowvarDlg::debug(DEBUG_STATUS status, CAstSig *debugAstSig, int entry)
{
	unordered_map<string, CDebugDlg*>::iterator it;
	LRESULT res;
	CAstSig *lp;
	const char *basename;
	static	string fullname;
	int curId;
	vector<string> list;
	map<string, UDF>::iterator itUDF;
	switch(status)
	{
	case refresh:
		it = dbmap.find(CAstSig::vecast.back()->u.base);
		// if basename is not yet loaded on debug window, it is end
		if (it!=dbmap.end())
			it->second->Debug(debugAstSig, status);
		break;

	case entering: 
		ValidateFig(debugAstSig->u.title.c_str());
		mainSpace.need2validate = true;
		// inspect how many layers of udf's to add to the scope list
		// begin from debugAstSig and check his dad, up and up until NULL
		for (CAstSig *tp = debugAstSig; tp; tp = tp->dad)
			if (!tp->u.title.empty()) list.push_back(tp->u.title);
		for (auto scopeStr = list.rbegin(); scopeStr != list.rend(); scopeStr++)
			SendDlgItemMessage(IDC_DEBUGSCOPE, CB_ADDSTRING, 0, (LPARAM)(*scopeStr).c_str());
//	case progress:
		SendDlgItemMessage(IDC_DEBUGSCOPE, CB_SETCURSEL, SendDlgItemMessage(IDC_DEBUGSCOPE, CB_GETCOUNT)-1);
		pcast = lp = CAstSig::vecast.back(); 
		// is name already open in DebugDlg or not
		// but if name is local udf, it should not be checked with dbmap
		basename = lp->u.base.c_str();
		if (lp->pEnv->udf.find(basename)!=lp->pEnv->udf.end())
		{
			char buf[256], drive[256], folder[256];
			strcpy(buf, lp->pEnv->udf.find(basename)->second.fullname.c_str());
			_splitpath(lp->pEnv->udf.find(basename)->second.fullname.c_str(), drive, folder, NULL, NULL);
			//if this is from auxcon or private udf, skip the rest
			if (strcmp(drive, "a:") && strcmp(drive, "\\module\\") && strcmp(folder, "private://"))
			{
				it = dbmap.find(basename); 
				if (it == dbmap.end())
				{
					debugAstSig->OpenFileInPath(basename, "aux", fullname);
					::SendMessage(mTab.hTab, TCM_GETITEMCOUNT, 0, 0);
					//Faking the "Debug" button
					DWORD Thid = GetThreadId((HANDLE)hDebugThread2);
					PostThreadMessage(Thid, WM__NEWDEBUGDLG, (WPARAM)fullname.c_str(), 0);
					Sleep(200);
					curId = mTab.GetCurrentPageID(basename);
					while ((it = dbmap.find(basename)) == dbmap.end()) { Sleep(50); };
					lp = debugAstSig;
				}
				lastDebug = it->second;
				CDebugDlg::pAstSig = lp;
				lastDebug->Debug(lp, status);
				mTab.SetCurrentTab(basename);
			}
		}
		break;
	case stepping:
		if (CDebugDlg::pAstSig != debugAstSig)
		{
			mTab.SetCurrentTab(debugAstSig->u.base.c_str());
			CDebugDlg::pAstSig = debugAstSig;
			it = dbmap.find(debugAstSig->u.base);
			lastDebug = it->second;
		}
		if (lastDebug)	lastDebug->Debug(NULL, status, entry);
		break;
	case purgatory:
		if (lastDebug) lastDebug->Debug(NULL, status);
		break;
	case exiting:
		//ValidateFig(CAstSig::vecast.front()->u.title.c_str());
		if (debugAstSig) // if called from xcom::cleanup_debug() skip here
		{
			//is there cellviewdlg open for the variable(s) in this scope?
			for (auto var : debugAstSig->Vars)
			{
				CWndDlg *m_cvDlg = Find_cellviewdlg(var.first.c_str());
				if (m_cvDlg)
				{
					//if found, update the title and make a copy of psig
					CVar *copyPsig = new CVar;
					*copyPsig = *(((CVectorsheetDlg*)m_cvDlg)->psig);
					if (((CVectorsheetDlg*)m_cvDlg)->clean_psig)
						delete ((CVectorsheetDlg*)m_cvDlg)->psig;
					((CVectorsheetDlg*)m_cvDlg)->psig = copyPsig;
					((CVectorsheetDlg*)m_cvDlg)->clean_psig = true;
					char title[256];
					sprintf(title, "%s (inactive)", var.first.c_str());
					m_cvDlg->SetWindowTextA(title);
				}
			}
			res = SendDlgItemMessage(IDC_DEBUGSCOPE, CB_GETCOUNT) - 1;
			SendDlgItemMessage(IDC_DEBUGSCOPE, CB_DELETESTRING, res);
			SendDlgItemMessage(IDC_DEBUGSCOPE, CB_SETCURSEL, res - 1);
		}
//		else
		{
			pcast = CAstSig::vecast.front();
			string ss = pcast->u.title;
			ValidateFig(pcast->u.title.c_str());
		}
		if (lastDebug)
		{
			lastDebug->Debug(NULL, status);
			if (debugAstSig && debugAstSig->dad)
			{
				CDebugDlg::pAstSig = debugAstSig = debugAstSig->dad;
				it = dbmap.find(debugAstSig->u.base);
				if (it != dbmap.end())
				{
					lastDebug = it->second;
					if (debugAstSig->u.nextBreakPoint > debugAstSig->u.currentLine)
						mTab.SetCurrentTab(debugAstSig->u.base.c_str());
				}
			}
		}
		//now updated debugAstSig
		if (debugAstSig) // if called from xcom::cleanup_debug() skip here
			if (debugAstSig && debugAstSig->u.debug.status == stepping)
				if (mTab.SetCurrentTab(debugAstSig->Script.c_str()) >= 0)
				{
					it = dbmap.find(debugAstSig->Script);
					if (it != dbmap.end())
						lastDebug = it->second;
				}
		break;
	}
}

void CShowvarDlg::OnPlotDlgCreated(const char *varname, GRAFWNDDLGSTRUCT *pin)
{
	LRESULT res = mShowDlg.SendDlgItemMessage(IDC_DEBUGSCOPE, CB_GETCURSEL);
	SetGlovar(CAstSig::vecast.at(res)->GetVariable("gcf"));
	//It's better to update gcf before WM__PLOTDLG_CREATED is posted
	//that way, environment context can be set properly (e.g., main or inside CallSub)
	//4/24/2017 bjkwon
	//2/1/2019 don't understand the comment
	if (strncmp(varname, "Figure ", 7))
		Fillup();
	HHOOK hh = SetWindowsHookEx (WH_GETMESSAGE, HookProc, NULL, pin->threadPlot);
	HHOOK hh2 = SetWindowsHookEx (WH_KEYBOARD, HookProc2, NULL, pin->threadPlot);
//	plotDlgThread.insert(pair<string, DWORD>(varname, pin->threadPlot));
	cfigdlg *newItem = new cfigdlg(pcast->u.title.c_str());
	newItem->threadID = pin->threadPlot;
	newItem->hDlg = pin->cfig->m_dlg->hDlg;
	newItem->var = varname;
	newItem->pcast = pcast;
	plots.push_back(newItem);
}

void CShowvarDlg::OnPlotDlgDestroyed(const char *varname, HWND hDlgPlot)
{
	vector<string> varname2del;
	CVar *p = (CVar *)FindFigure(hDlgPlot);
	//scan pVars and pGOvars and delete p if found
	CVar dummy;
	for (map<string, vector<CVar*>>::iterator it = pGOvars->begin(); it != pGOvars->end(); )
	{
		if ((*it).second.size() > 1)
		{
			for (vector<CVar*>::iterator jt = (*it).second.begin(); jt != (*it).second.end(); )
			{
				if (Is_A_Ancestor_of_B(p, (*jt)))
					jt = (*it).second.erase(jt);
				else
					jt++;
			}
			if ((*it).second.empty())
			{
				(*pVars)[(*it).first] = dummy;
				it = pGOvars->erase(it);
			}
			else
				it++;
		}
		else
		{
			if (Is_A_Ancestor_of_B(p, (*it).second.front()))
			{
				(*pVars)[(*it).first] = dummy;
				it = pGOvars->erase(it);
			}
			else
				it++;
		}
	}
	Fillup();
	for (auto figdlg=plots.begin(); figdlg!=plots.end(); figdlg++)
	{
		if ((*figdlg)->hDlg == hDlgPlot)
		{
			cfigdlg *toDelete = *figdlg;
			plots.erase(figdlg);
			delete toDelete;
			break;
		}
	}
}

void CShowvarDlg::InitVarShow(int type, const char *name)
{ // Only used for variable content dlg, not the base showdlg
	if (type == CSIG_CELL || type == CSIG_STRUCT || type == CSIG_HANDLE)
	{
		CRect rtDlg;
		mShowDlg.GetWindowRect(&rtDlg);
		::DestroyWindow(GetDlgItem(IDC_STATIC_SCOPE));
		::DestroyWindow(GetDlgItem(IDC_REFRESH));
		::DestroyWindow(GetDlgItem(IDC_SETPATH));
		::DestroyWindow(GetDlgItem(IDC_STATIC_FS));
		::DestroyWindow(GetDlgItem(IDC_FS));
		::DestroyWindow(GetDlgItem(IDC_DEBUG2));
		::DestroyWindow(GetDlgItem(IDC_OPEN));
		::DestroyWindow(GetDlgItem(IDC_SAVE));
		::DestroyWindow(GetDlgItem(IDC_DEBUGSCOPE));
		::DestroyWindow(GetDlgItem(IDC_STOP));
		//Do not set the window position here.. it is to be done during ArrangeInit
		SetWindowText(name);
	}
}

BOOL CShowvarDlg::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	CWndDlg::OnInitDialog(hwndFocus, lParam);

	CRect rtDlg, rt;
	if (this == &mShowDlg)
	{
		mShowDlg.GetWindowRect(&rtDlg);
		fileDlg.InitFileDlg(hDlg, hInst, "");
		fileOpenSaveDlg.InitFileDlg(hDlg, hInst, "");
		SendDlgItemMessage(IDC_DEBUGSCOPE, CB_ADDSTRING, 0, (LPARAM)"base workspace");
		SendDlgItemMessage(IDC_DEBUGSCOPE, CB_SETCURSEL, 0);
		//These two functions are typically called in pair--so sigproc and graffy can communicate with each other for GUI updates, etc.
		SetHWND_WAVPLAY(hDlg);
		SetHWND_GRAFFY(hDlg);

		SendDlgItemMessage(IDC_REFRESH, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP,
			(LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDB_BTM_REFRESH), IMAGE_BITMAP, 25, 25, LR_DEFAULTSIZE));
		SendDlgItemMessage(IDC_STOP, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP,
			(LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDB_BTM_STOP), IMAGE_BITMAP, 25, 25, LR_DEFAULTSIZE));
		SendDlgItemMessage(IDC_DEBUG2, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP,
			(LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDB_BTM_DEBUG), IMAGE_BITMAP, 30, 30, LR_DEFAULTSIZE));
		SendDlgItemMessage(IDC_OPEN, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP,
			(LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDB_BTM_OPEN), IMAGE_BITMAP, 20, 20, LR_DEFAULTSIZE));
		SendDlgItemMessage(IDC_SAVE, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP,
			(LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDB_BTM_SAVE), IMAGE_BITMAP, 20, 20, LR_DEFAULTSIZE));
		::SetWindowText(GetDlgItem(IDC_AUDIO_TITLE), "Audio Variables");
		::SetWindowText(GetDlgItem(IDC_NONAUDIO_TITLE), "Non-Audio Variables");

		hLog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_LOG), GetConsoleWindow(), (DLGPROC)logProc);
	}
	else
	{
		::SetWindowText(GetDlgItem(IDC_AUDIO_TITLE), "Audio Elements");
		::SetWindowText(GetDlgItem(IDC_NONAUDIO_TITLE), "Non-Audio Elements");
	}
	hList1 = GetDlgItem(IDC_LIST1);
	hList2 = GetDlgItem(IDC_LIST2);
	if (!hList1) MessageBox("lvInit");
	LRESULT res = ::SendMessage(hList1, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	::SendMessage(hList2, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

	ArrangeControlsInit();
	char buf[256];
	::GetClientRect(hList1, &rt);
	LoadString(hInst, IDS_HELP_SHOWVARLIST1, buf, sizeof(buf));
	CreateTT(hInst, hList1, rt, buf);
	::GetClientRect(hList2, &rt);
	LoadString(hInst, IDS_HELP_SHOWVARLIST2, buf, sizeof(buf));
	CreateTT(hInst, hList2, rt, buf, 300);
	return TRUE;
}

void CShowvarDlg::OnShowWindow(BOOL fShow, UINT status)
{
	int res;
	HMENU hMenu = ::GetSystemMenu(hDlg, FALSE);
	AppendMenu(hMenu, MF_SEPARATOR, 0, "");
	res = AppendMenu(hMenu, MF_STRING, ID_HELP_SYSMENU3, "Set UDF &Path");
	AppendMenu(hMenu, MF_SEPARATOR, 0, "");
	res = AppendMenu(hMenu, MF_STRING, ID_HELP_SYSMENU2, "Sampling &Rate Adjustment");
	AppendMenu(hMenu, MF_SEPARATOR, 0, "");
	res = AppendMenu(hMenu, MF_STRING, ID_HELP_SYSMENU1, "&About AUX Lab");
	AppendMenu(hMenu, MF_SEPARATOR, 0, "");
	res = AppendMenu(hMenu, MF_STRING, ID_HELP_SYSMENU4, "&Show Key Tracking");
}

void CShowvarDlg::ArrangeControls(map<string, CVar> *Vars, int cx, int cy)
{
	if (Vars == NULL) Vars = &pcast->Vars;
	size_t nVars1(0), nVars2(0);
	for (map<string, CVar>::iterator it = Vars->begin(); it != Vars->end(); it++)
	{
		if ((*it).second.GetType() == CSIG_AUDIO)
			nVars1++;
		else
			nVars2++;
	}
}
void CShowvarDlg::ArrangeControlsInit()
{
	HINSTANCE hModule = GetModuleHandle(NULL);
	char buf[256];
	int res;
	int width[] = { 60, 45, 50, 100, }; // audio
	int width2[] = { 55, 38, 40, 100, }; // non-audio
	for (int k = 0; k<4; k++)
	{
		LvCol.cx = width[k];
		LvCol.pszText = buf;
		LoadString(hModule, IDS_STRING105 + k, buf, sizeof(buf));
		if (k == 0)	strcpy(buf, "Name");
		res = (int)SendDlgItemMessage(IDC_LIST1, LVM_INSERTCOLUMN, k, (LPARAM)&LvCol);
	}
	for (int k = 0; k<4; k++)
	{
		LvCol.cx = width2[k];
		LvCol.pszText = buf;
		LoadString(hModule, IDS_STRING101 + k, buf, sizeof(buf));
		if (k == 0)	strcpy(buf, "Name");
		res = (int)SendDlgItemMessage(IDC_LIST2, LVM_INSERTCOLUMN, k, (LPARAM)&LvCol);
	}

	CRect rt0, rt1, rt2;
	HDC	hdc = GetDC(hDlg);
	GetDlgItemText(IDC_AUDIO_TITLE, buf, sizeof(buf));
	SIZE sz1;
	res = GetTextExtentPoint32(hdc, buf, (int)strlen(buf), &sz1);
	ReleaseDC(NULL, hdc);
	list_head_height = sz1.cy;
	GetClientRect(GetDlgItem(IDC_STOP), rt2);
	idc_stop_height = rt2.bottom;

	int topgap(0);
	if (hDlg == mShowDlg.hDlg)
		topgap = idc_stop_height;

	CRect rt, rtCl;
	::GetClientRect(hDlg, rtCl); // Not GetWindowRect(rt0);
	int gap1 = rtCl.Height() / 100;
	int gap2 = 0;// gap1 / 2;
	rt.top = topgap + gap1;
	rt.bottom = rt.top + list_head_height;
	rt.right = (rt.left = 0) + rtCl.Width();
	res = MoveDlgItem(hDlg, IDC_AUDIO_TITLE, rt, 1);
	int list_Height = (rtCl.Height() - topgap - 2 * gap1 - 2 * gap2 - 2 * list_head_height) / 2;
	rt.top = rt.bottom + gap2;
	rt.bottom = rt.top + list_Height;
	res = MoveDlgItem(hDlg, IDC_LIST1, rt, 1);
	rt.top = rt.bottom + gap1;
	rt.bottom = rt.top + list_head_height;
	res = MoveDlgItem(hDlg, IDC_NONAUDIO_TITLE, rt, 1);
	rt.top = rt.bottom + gap2;
	rt.bottom = rt.top + list_Height;
	res = MoveDlgItem(hDlg, IDC_LIST2, rt, 1);

	listHeight[0] = listHeight[1] = list_Height;

	WORD sss1 = HIWORD(ListView_ApproximateViewRect(hList1, -1, -1, 1));
	WORD sss2 = HIWORD(ListView_ApproximateViewRect(hList1, -1, -1, 2));
	slope = sss2 - sss1;
	offset = sss1 - slope;
}

DWORD CShowvarDlg::calculate_width(int newwidth[], int newwidth2[])
{// Calculates the minimum width accommodating hList1 and hList2, based on the item count
 // Default column widths for listview control 
	int width[] = { 60, 45, 50, 100, }; // audio
	int width2[] = { 55, 38, 40, 100, }; // non-audio
	char buf[256];
	HDC	hdc = GetDC(hList1);
	CSize sz;
	//hList1
	int count1 = ListView_GetItemCount(hList1);
	//p is item, k is subitem
	for (int k = 0; k < 4; k++)
	{
		newwidth[k] = width[k];
		for (int p = 0; p < count1; p++)
		{
			ListView_GetItemText(hList1, p, k, buf, 256);
			GetTextExtentPoint32(hdc, buf, (int)strlen(buf), &sz);
			newwidth[k] = max(sz.cx+5, newwidth[k]);
		}
	}
	//hList2
	int count2 = ListView_GetItemCount(hList2);
	//p is item, k is subitem
	for (int k = 0; k < 4; k++)
	{
		newwidth2[k] = width2[k];
		for (int p = 0; p < count2; p++)
		{
			ListView_GetItemText(hList2, p, k, buf, 256);
			GetTextExtentPoint32(hdc, buf, (int)strlen(buf), &sz);
			newwidth2[k] = max(sz.cx+5, newwidth2[k]);
		}
	}
	ReleaseDC(NULL, hdc);
	int newWidth4Dlg1(0), newWidth4Dlg2(0);
	for (int k = 0; k < 4; k++)	newWidth4Dlg1 += newwidth[k];
	for (int k = 0; k < 4; k++)	newWidth4Dlg2 += newwidth2[k];
	DWORD out = max(newWidth4Dlg1, newWidth4Dlg2);
	return out+24;// margin 24 <-- the width difference between Dlg rect and Dlg client rect
}

void CShowvarDlg::OnSize(UINT state, int cx, int cy)
{
	int topgap(0);
	if (hDlg == mShowDlg.hDlg)
		topgap = idc_stop_height;

	CRect rt, rtCl;
	::GetClientRect(hDlg, rtCl); // Not GetWindowRect(rt0);
	CRect rt0;
	GetWindowRect(rt0);

	int gap1 = rtCl.Height() / 100;
	int gap2 = 0;// gap1 / 2;
	rt.top = topgap + gap1;
	rt.bottom = rt.top + list_head_height;
	rt.right = (rt.left = 0) + rtCl.Width();
	int res = MoveDlgItem(hDlg, IDC_AUDIO_TITLE, rt, 1);
	//Now, depending on how many items are there in list1 and list2, the heights of two hList's are adjusted

	WORD height_aprox1 = HIWORD(ListView_ApproximateViewRect(hList1, -1, -1, -1));
	WORD height_aprox2 = HIWORD(ListView_ApproximateViewRect(hList2, -1, -1, -1));

	//listHeight0: sum of list view heights allowed in current layout
	int listHeight0 = rtCl.Height() - topgap - 2 * gap1 - 2 * gap2 - 2 * list_head_height;
	double req_ratio = (double)height_aprox1 / (height_aprox1+height_aprox2);
	if (height_aprox1 + height_aprox2 < listHeight0 * 2)
	{ // can be accommodated; no need to re-adjust
		listHeight[0] = (int)(listHeight0 * req_ratio + .5);
		listHeight[1] = listHeight0 - listHeight[0];
	}
	else
	{
		//let's not reduce the size further.
		listHeight[0] = listHeight[1] = listHeight0;
		GetWindowRect(rt);
		rt.bottom = rt.top + topgap + 2 * (listHeight0 + gap1 + gap2 + list_head_height) + rt0.Height() - rtCl.Height();
		MoveWindow(rt);
		return;
	}
	rt.top = rt.bottom + gap2;
	rt.bottom = rt.top + listHeight[0];
	res = MoveDlgItem(hDlg, IDC_LIST1, rt, 1);
	rt.top = rt.bottom + gap1;
	rt.bottom = rt.top + list_head_height;
	res = MoveDlgItem(hDlg, IDC_NONAUDIO_TITLE, rt, 1);
	rt.top = rt.bottom + gap2;
	rt.bottom = rt.top + listHeight[1];
	res = MoveDlgItem(hDlg, IDC_LIST2, rt, 1);
	AdjustWidths();
}

void CShowvarDlg::OnClose()
{
	OnDestroy();
}

void CShowvarDlg::AdjustWidths(int redraw)
{ // Adjust the width of hDlg at least to accommodate hList1 and hList2
	int width1[4], width2[4];
	int wid = calculate_width(width1, width2);

	// TASK for 4/19
	// Set maximum width after calculate_width for each column either here or inside of calculate_width
	// based on the current width of dlg box... 

	CRect rt;
	GetWindowRect(rt);
	CRect rtDlg(rt);
	if (rt.Width() < wid || redraw)
	{
		if (this != &mShowDlg)
		{
			rtDlg.right = rtDlg.left + wid;
			MoveWindow(rtDlg);
		}
	}
	//4th column width is set to fit the width of hDlg
	int icum(0);
	for (int k = 0; k < 3; k++)
	{
		ListView_SetColumnWidth(hList1, k, width1[k]);
		icum += width1[k];
	}
	ListView_SetColumnWidth(hList1, 3, rtDlg.Width()-icum-24); // 4th column
	icum = 0;
	for (int k = 0; k < 3; k++)
	{
		ListView_SetColumnWidth(hList2, k, width2[k]);
		icum += width1[k];
	}
	ListView_SetColumnWidth(hList2, 3, rtDlg.Width() - icum-24); // 4th column
}

void CShowvarDlg::OnDestroy()
{
	if (this!=&mShowDlg)
	{
		DestroyWindow();
		delete this;
	}
	else
	{
		if (!cellviewdlg.empty())
		{ // clean up children showvarDlg and CVectorSheetDlg
			for (size_t k = 0; k<cellviewdlg.size(); k++)
			{
				if (cellviewdlg[k] == this) continue;
				delete cellviewdlg[k];
			}
		}
		closeXcom(AppPath);
		exit(0);
	}
}

void CShowvarDlg::OnSysCommand(UINT cmd, int x, int y)
{
	// processing WM_SYSCOMMAND should return 0. How is this done?
	// just add (msg) == WM_SYSCOMMAND	in SetDlgMsgResult in message cracker
	HINSTANCE hInst = GetModuleHandle(NULL);
	INT_PTR res;
	switch(cmd)
	{
	case ID_HELP_SYSMENU1:
		DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ABOUT), GetConsoleWindow(), (DLGPROC)AboutDlg, (LPARAM)hInst);
		break;
	case ID_HELP_SYSMENU2:
		res = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FS), hDlg, (DLGPROC)FSDlgProc, (LPARAM)&CAstSig::vecast.front()->pEnv->Fs);
		break;
	case ID_HELP_SYSMENU3:
		OnCommand(IDC_SETPATH, 0, 0);
		break;
	case ID_HELP_SYSMENU4:
		::ShowWindow(hLog, SW_SHOW);
		break;
	}
}

void CShowvarDlg::OnCommand(int idc, HWND hwndCtl, UINT event)
{
	string addp, str;
	size_t id;
	vector<HANDLE> figs;
	char *longbuffer, errstr[256];
	static char fullfname[256], fname[256];
	switch(idc)
	{
	//case ID_HELP1: 
	//	DialogBoxParam (hInst, MAKEINTRESOURCE(IDD_ABOUT), GetConsoleWindow(), (DLGPROC)AboutDlg, (LPARAM)hInst);
	//	break;
	case IDC_SETPATH: 
		str = udfpath;
		id = str.find_first_not_of(';');
		longbuffer = new char[sizeof(udfpath)*8];
		strcpy(longbuffer,udfpath+id);
		if (InputBox("AUX UDF PATH", "Put path here (Each path is separated by a semicolon) (NOTE: The current application folder is the first priority path by default.)", longbuffer, sizeof(udfpath)*8, false, hDlg)==1)
		{
			strcpy(udfpath,longbuffer);
			addp = AppPath;
			addp += ';';
			addp += longbuffer;
			&CAstSig::vecast.front()->pEnv->SetPath(addp.c_str());
		}
		delete[] longbuffer;
		break;

	case IDC_STOP:
		// TEMPORARY HACK... 32799 is IDM_STOP in PlotDlg.cpp in graffy
		// Not a good way, but other than this, there' no way to make the playback stop
		// that was initiated by a different thread.... That is, call to TerminatePlay() won't work 
		// because threadIDs and len_threadIDs are thread specific 
		// (sort of... these variables are shared by some threads. But not by some other threads.)
		// One bonus feature in going this way is, this would stop ALL playbacks easily that were 
		// going on; otherwise it would have been quite complicated if done individually thread by thread.
		// 7/15/2016 bjk
		figs = graffy_Figures();
		for (vector<HANDLE>::iterator it=figs.begin(); it!=figs.end(); it++)
			::SendMessage(GetHWND_PlotDlg2(*it), WM_COMMAND, 32799, 0); 
		StopPlay(0, false);
		break;

	case IDC_OPEN:
		axlfullfname[0] = axlfname[0] = 0;
		if (fileOpenSaveDlg.FileOpenDlg(axlfullfname, axlfname, "AUXLAB Data file (*.AXL)\0*.axl\0", "axl"))
		{
			string name;
			fp = CAstSig::vecast.front()->OpenFileInPath(axlfullfname, "axl", name);
			if (mainSpace.load_axl(fp, errstr) == 0)
				printf("File %s reading error----%s.\n", axlfullfname, errstr);
			else
			{
				fclose(fp);
				mShowDlg.Fillup();
			}
		}
		break;

	case IDC_SAVE:
		axlfullfname[0] = axlfname[0] = 0;
		if (fileOpenSaveDlg.FileSaveDlg(axlfullfname, axlfname, "AUXLAB Data file (*.AXL)\0*.axl\0", "axl"))
		{
			vector<string> varlist; // empty vector will save all variables.
			if (mainSpace.SAVE_axl(CAstSig::vecast.back(), axlfullfname, varlist, errstr)<=0)
				MessageBox(errstr, "File Save Error");
		}
		break;

	case IDC_DEBUG2:
		if (hDebugThread2)
			PostThreadMessage(GetThreadId ((HANDLE) hDebugThread2), WM__NEWDEBUGDLG, 0, 0);
		else
		{
			if ((hDebugThread2 = _beginthreadex (NULL, 0, debugThread2, NULL, 0, NULL))==-1)
			{MessageBox ("Debug Thread Creation Failed."); break;}
		}
		break;

	case IDCANCEL:
		OnClose();
		break;
	}
}

void CShowvarDlg::OnCloseFig(int figID)
{

}

CWndDlg * CShowvarDlg::DoesThisVarViewExist(string varname)
{
	CString str;
	for (auto arrayview : cellviewdlg)
	{
		if (((CVectorsheetDlg*)arrayview)->name == varname)
			return arrayview;
	}
	return NULL;
}

HWND CShowvarDlg::varname2HWND(const char *varname)
{
	// Just a quick patch. varname is a local variable and pcast is not.
	// Not so elegant a way to code. 1/28/2019
	string title;
	if (pcast->u.title.empty())
		title += "base workspace";
	else
		title += pcast->u.title.c_str();
	for (auto figdlg = plots.begin(); figdlg != plots.end(); figdlg++)
	{
		if ((*figdlg)->scope == title && (*figdlg)->var == varname)
		{
			return (*figdlg)->hDlg;
		}
	}
	return nullptr;
}

void CShowvarDlg::OnNotify(HWND hwnd, int idcc, LPARAM lParam)
{
	int res(0);
	string title("");
	static char varname[256]; // keep this in the heap (need to survive during _beginthread)
	char *pvarname;
	char errstr[256];
	static VARPLOT messenger;

	LPNMHDR pnm = (LPNMHDR)lParam;
	LPNMLISTVIEW pview = (LPNMLISTVIEW)lParam;
	LPNMLVKEYDOWN lvnkeydown;
	UINT code=pnm->code;
	LPNMITEMACTIVATE lpnmitem;
	static int clickedRow;
	CWndDlg * arrayview;
	CVar sigVarname;
	CVar *psig(NULL);
	if (changed) Fillup(); 
	int type(0);
	HWND hWndPlot = NULL;
	bool multiGO(false);
	vector<int> selectState;// This is the placeholder for select state used exclusively for mShowDlg, to use to get non-consecutively selected rows. Probably there are easier features already available if I was using .NET or C# 7/11/2016
	map<string, CVar> *pvars(NULL);
	map<string, vector<CVar*>> *pgovars(NULL);
	CGobj * hobj;

	switch(code)
	{
	case NM_CLICK:
		lpnmitem = (LPNMITEMACTIVATE) lParam;
		clickedRow = lpnmitem->iItem;
		break;
	case NM_DBLCLK:
		lpnmitem = (LPNMITEMACTIVATE) lParam;
		if ((clickedRow = lpnmitem->iItem)==-1) break;
		ListView_GetItemText(lpnmitem->hdr.hwndFrom, ListView_GetSelectionMark(lpnmitem->hdr.hwndFrom), 0, varname, 256);
		if (this != &mShowDlg)
			GetWindowText(title);
		if (varname[0] == '.') pvarname = varname + 1;
		else pvarname = varname;
		if (pVars->find(pvarname) != pVars->end())
		{
			psig = &(*pVars)[pvarname];
			type = psig->GetType();
		}
		else if (pGOvars->find(pvarname) != pGOvars->end())
		{
			vector<CVar*> *tp = &(*pGOvars)[pvarname];
			if (tp->size() == 1)
			{
				pvars = &tp->front()->strut;
				pgovars = &tp->front()->struts;
			}
			else
				multiGO = true;
			type = CSIG_STRUCT; // pGOvars already available. No need to worry about psig
		}
		switch(type)
		{
		case CSIG_STRING:
			if (psig->length() == 0) break;
		case CSIG_VECTOR:
		case CSIG_AUDIO:
		case CSIG_TSERIES:
			arrayview = DoesThisVarViewExist(varname);
			if (!arrayview)
			{
				CVectorsheetDlg *cvdlg = new CVectorsheetDlg(this);
				cvdlg->hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DISPVECTOR), hDlg, (DLGPROC)vectorsheetDlg, (LPARAM)varname);
				cvdlg->psig = psig;
				cvdlg->name = title + varname;
				cvdlg->Init2(cvdlg->name.c_str());
				cvdlg->ShowWindow(SW_SHOW);
				if (psig->next)
				{
					char buffer[256];
					cvdlg->GetWindowText(buffer, 256);
					strcat(buffer, " (L)");
					cvdlg->SetWindowTextA(buffer);
					CVectorsheetDlg *cvdlg2 = new CVectorsheetDlg(this);
					cvdlg2->hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DISPVECTOR), hDlg, (DLGPROC)vectorsheetDlg, (LPARAM)varname);
					cvdlg2->psig = psig->next;
					cvdlg2->name = title + varname;
					cvdlg2->name  += " (R)";
					cvdlg2->Init2(cvdlg2->name.c_str());
					cvdlg2->FillUp();
					cvdlg2->ShowWindow(SW_SHOW);
				}
			}
			else
			{
				auto it = find(cellviewdlg.begin(), cellviewdlg.end(), arrayview);
				CVectorsheetDlg *cvdlg = (CVectorsheetDlg*)(*it);
				if (cvdlg->clean_psig)
				{
					delete cvdlg->psig;
					cvdlg->clean_psig = false;
				}
				cvdlg->psig = psig;
				cvdlg->name = title + varname;
				cvdlg->Init2(cvdlg->name.c_str());
				cvdlg->ShowWindow(SW_SHOW);
			}
			break;
		case CSIG_CELL:
		case CSIG_STRUCT:
		case CSIG_HANDLE:
			arrayview = DoesThisVarViewExist(varname);
			if (!arrayview)
			{
				if (multiGO)
				{
					CVectorsheetDlg *cvdlg = new CVectorsheetDlg(this);
					cvdlg->hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DISPVECTOR), hDlg, (DLGPROC)vectorsheetDlg, (LPARAM)varname);
					cvdlg->sigvector = &((*pGOvars)[pvarname]);
					cvdlg->psig = cvdlg->sigvector->front(); // psig itself is not necessary but just need this to get the type in Init2
					cvdlg->Init2((title + varname).c_str());
					cvdlg->name = title + varname;
					cvdlg->FillUp(cvdlg->sigvector);
					cvdlg->ShowWindow(SW_SHOW);
					break;
				}
				else
				{
					//Any CShowvarDlg object created is for either struct or cell
					CShowvarDlg * cvdlg = new CShowvarDlg(this, pvars);
					if (type == CSIG_CELL)
					{
						map<string, CVar> *tp = new map<string, CVar>;
						char idstr[256];
						int k = 1;
						for (vector<CVar>::iterator it = psig->cell.begin(); it != psig->cell.end(); it++, k++)
						{
							sprintf(idstr, "{%d}", k);
							(*tp)[idstr] = *it; // copy of RHS to LHS---not so great way to do... but... 4/9/2018
						}
						cvdlg->pVars = tp;
					}
					else if (!cvdlg->pVars)
						cvdlg->pVars = &(*pVars)[pvarname].strut;
					cvdlg->pcast = pcast;
					if (pgovars)
						cvdlg->pGOvars = pgovars;
					else
						cvdlg->pGOvars = &(*pVars)[pvarname].struts; // For the struct item added with a.item = GO, 9/6/2018 
					cvdlg->pGOvarsPar = NULL;
					cvdlg->hDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SHOWVAR), hDlg, (DLGPROC)showvarDlgProc);
					cvdlg->InitVarShow(type, (title + varname).c_str());
					cvdlg->name = title + varname;
					cvdlg->Fillup();
					cvdlg->ShowWindow(SW_SHOW);
				}
			}
			else
			{
				//Do this later.... depending on multiGO or not, adjust cvdlg.
				auto it = find(cellviewdlg.begin(), cellviewdlg.end(), arrayview);
				CVectorsheetDlg *cvdlg = (CVectorsheetDlg*)(*it);
				cvdlg->ShowWindow(SW_SHOW);
			}
			break;
		case CSIG_SCALAR:
			return;
		}
		break;
	case LVN_KEYDOWN:
		lvnkeydown = (LPNMLVKEYDOWN)lParam;
		ListView_GetItemText(lvnkeydown->hdr.hwndFrom, ListView_GetSelectionMark(lvnkeydown->hdr.hwndFrom), 0, varname, 256);
		if (this != &mShowDlg)
			GetWindowText(title);
		if (strlen(varname)>0)
		{	
			if (varname[0] == '.') pvarname = varname + 1;
			else pvarname = varname;
			if (pVars->find(pvarname) == pVars->end())
			{
				ClearVar(pvarname);
				Fillup(); // why is this needed? 11/23/2018
				break;
			}
			psig = &(*pVars)[pvarname];
			type = psig->GetType();
			hWndPlot = varname2HWND(varname);
			hobj = (CGobj *)FindFigure(hWndPlot);
			switch(lvnkeydown->wVKey)
			{
				CWndDlg *dlg;
			case VK_DELETE: // $$231109 4/25/2017
				if (hobj)
				{
					OnPlotDlgDestroyed(varname, hWndPlot);
					deleteObj(hobj);
				}
				if (this == &mShowDlg)
					ClearVar(pvarname);
				else
				{
					map<string, CVar>::iterator it = pVars->find(pvarname);
					pVars->erase(it);
				}
				dlg = Find_cellviewdlg(pvarname);
				if (dlg)
				{
					if (dlg->dwUser==8383)
						((CVectorsheetDlg*)dlg)->OnClose();
					else if (dlg->dwUser == 1010)
						((CShowvarDlg*)dlg)->OnClose();
				}
				Fillup();
				break;
			case VK_SPACE:
				selectState.resize(ListView_GetItemCount(lvnkeydown->hdr.hwndFrom));
				for (int k(0); k<ListView_GetItemCount(lvnkeydown->hdr.hwndFrom); k++)
				{
					if (ListView_GetItemState(lvnkeydown->hdr.hwndFrom, k, LVIS_SELECTED))
					{
						ListView_GetItemText(lvnkeydown->hdr.hwndFrom, k, 0, varname, 256);
						double block = CAstSig::vecast.front()->audio_block_ms;
						int devID = 0;
						psig = &(*pVars)[varname];
						INT_PTR h = psig->PlayArray(devID, WM__AUDIOEVENT, hDlg, &block, errstr);
						if (!h)
						{ // PlayArray will return 0 if unsuccessful due to waveOutOpen failure. For other reasons.....
							//errstr should show the err msg. Use it if necessary 7/23/2018
						}
						else
						{
							AUD_PLAYBACK * p = (AUD_PLAYBACK*)h;
							CVar *pplayObj = new CVar;
							pplayObj->SetValue((double)(INT_PTR)h);
							pplayObj->strut["data"] = *psig;
							pplayObj->strut["type"] = string("audio_playback");
							pplayObj->strut["devID"] = CSignals((double)devID);
							pplayObj->strut["totalDurMS"] = CSignals(psig->alldur());
							pplayObj->strut["remDurMS"] = CSignals(psig->alldur());
							p->pvar = pplayObj;
						}
					}
				}
				break;
			case VK_RETURN:
				if (hobj)
				{
					::SetFocus(hobj->m_dlg->hDlg);
				}
				else
				{
					title += varname;
					strcpy(varname, title.c_str());
					title += ":";
					if (pcast->u.title.empty())
						title += "base workspace";
					else
						title += pcast->u.title.c_str();
					plotvar(psig, title, varname);
				}
				Fillup();
				break;
			}
		}
		break;

	case NM_CHAR:
		break;
	case NM_CUSTOMDRAW:
        ::SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG)ProcessCustomDraw(pnm));
        return;
	default:
		break;
	}
}

int CShowvarDlg::ClearVar(const char *var)
{
	map<string, CVar>::iterator it = pVars->find(var);
	if (it != pVars->end())
	{
		pVars->erase(it);
		return 1;
	}
	else
	{
		map<string, vector<CVar*>>::iterator it2 = pGOvars->find(var);
		if (it2 != pGOvars->end())
			pGOvars->erase(it2);
		return 1;
	}
	return 0;
}

void CShowvarDlg::UpdateProp(string varname, CVar *pvar, string propname)
{
	CWndDlg * tp = Find_cellviewdlg(varname.c_str());
	if (tp && ((CShowvarDlg*)tp)->name == varname)
	{
		char buf[256];
		for (int id = 0; ; id++, buf[0] = 0)
		{
			ListView_GetItemText(((CShowvarDlg*)tp)->hList2, id, 0, buf, sizeof(buf));
			if (strlen(buf) == 0) break;
			if (!strcmp(&buf[1], propname.c_str())) 
			{ ((CShowvarDlg*)tp)->updaterow(id, 3, &pvar->strut[propname]); break; }
		}
	}
}

void CShowvarDlg::OnSoundEvent(CVar *pvar, int code)
{
	switch (code)
	{
	case WOM_OPEN:
		EnableDlgItem(hDlg, IDC_STOP, 1);
		nPlaybackCount++;
		break;
	case WOM_CLOSE:
		nPlaybackCount--;
		if (!nPlaybackCount)	EnableDlgItem(hDlg, IDC_STOP, 0);
		for (map<string, CVar>::iterator it = pVars->begin(); it != pVars->end(); it++)
		{
			if ((*it).second == pvar->value())
			{
				(*it).second.strut["type"].SetString((pvar->strut["type"].string() + " (inactive)").c_str());
				UpdateProp((*it).first, &(*it).second, "type");
			}
		}
		break;
	case -1:
		MessageBox((char*)pvar, "audio out error", 0);
		break;
	default: // status updates
		for (map<string, CVar>::iterator it = pVars->begin(); it != pVars->end(); it++)
		{
			if ((*it).second == pvar->value())
			{
				(*it).second.strut["remDurMS"].SetValue(pvar->strut["remDurMS"].value());
				UpdateProp((*it).first, &(*it).second, "remDurMS");
				break;
			}
		}
		break;
	}
}

LRESULT CShowvarDlg::ProcessCustomDraw (NMHDR *lParam)
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;
	switch(lplvcd->nmcd.dwDrawStage) 
	{
	case CDDS_PREPAINT : //Before the paint cycle begins
	//request notifications for individual listview items
		return CDRF_NOTIFYITEMDRAW;
            
	case CDDS_ITEMPREPAINT: //Before an item is drawn
		//if (ListView_GetItemState (lplvcd->nmcd.hdr.hwndFrom,lplvcd->nmcd.dwItemSpec,LVIS_FOCUSED|LVIS_SELECTED) == (LVIS_FOCUSED|LVIS_SELECTED))
		//{
		//	RECT ItemRect;
		//	
		//	ListView_GetItemRect(lplvcd->nmcd.hdr.hwndFrom,lplvcd->nmcd.dwItemSpec,&ItemRect,LVIR_BOUNDS);
		//	HRGN bgRgn = CreateRectRgnIndirect(&ItemRect);
		//	HBRUSH MyBrush = CreateSolidBrush(RGB(150,150,150));
		//	FillRgn(lplvcd->nmcd.hdc, bgRgn, MyBrush);
		//	DeleteObject(MyBrush);
		//	return CDRF_SKIPDEFAULT;
		//}
		return CDRF_NOTIFYSUBITEMDRAW;

	case CDDS_SUBITEM | CDDS_ITEMPREPAINT: //Before a subitem is drawn
		switch(lplvcd->iSubItem)
		{
		case 10:
			lplvcd->clrText   = RGB(255,255,255);
			lplvcd->clrTextBk = RGB(240,55,23);
			return CDRF_NEWFONT;
		case 0:
			lplvcd->clrText   = RGB(255,255,0);
			lplvcd->clrTextBk = RGB(0,0,0);
			return CDRF_NEWFONT;
		case 1:
			lplvcd->clrText   = RGB(20,26,158);
			lplvcd->clrTextBk = RGB(200,200,10);
			return CDRF_NEWFONT;
		case 2:
		case 3:
			lplvcd->clrText   = RGB(12,15,46);
			lplvcd->clrTextBk = RGB(200,200,200);
			return CDRF_NEWFONT;
		case 11:
			lplvcd->clrText   = RGB(120,0,128);
			lplvcd->clrTextBk = RGB(20,200,200);
			return CDRF_NEWFONT;
		case 4:
		case 5:
			lplvcd->clrText   = RGB(255,255,255);
			lplvcd->clrTextBk = RGB(0,0,150);
			return CDRF_NEWFONT;
		case 6:
		case 7:
			lplvcd->clrText   = RGB(42,45,46);
			lplvcd->clrTextBk = RGB(200,200,200);
			return CDRF_NEWFONT;
		}
	}
	return CDRF_DODEFAULT;
}

void SetInsertLV(int type, HWND h1, HWND h2, UINT msg, LPARAM lParam)
{
	if (type==CSIG_AUDIO)
		SendMessage(h1, msg, 0, lParam);
	else
		SendMessage(h2, msg, 0, lParam);
}

char *showcomplex(char *out, complex<double> in)
{
	if (in.imag() == 0.)
		sprintf(out, "%g ", in.real());
	else if (in.imag() == 1.)
		sprintf(out, "%g+i ", in.real());
	else
		sprintf(out,"%g%+gi ", in.real(), in.imag());
	return out;
}

void CShowvarDlg::showtype(CVar *pvar, char *buf)
{
	switch (pvar->GetType())
	{
	case CSIG_STRING:
		strcpy(buf, "TXT");
		break;
	case CSIG_EMPTY:
		strcpy(buf, "NUL");
		break;
	case CSIG_SCALAR:
		strcpy(buf, "CON");
		break;
	case CSIG_VECTOR:
		strcpy(buf, "VCT");
		break;
	case CSIG_TSERIES:
		strcpy(buf, "TSQ");
		break;
	case CSIG_CELL:
		strcpy(buf, "CEL");
		break;
	case CSIG_HANDLE:
		strcpy(buf, "HDL");
		break;
	case CSIG_STRUCT:
		strcpy(buf, "CLAS");
		break;
	}
}

void CShowvarDlg::showsize(CVar *pvar, char *outbuf)
{
	int xc(0);
	string lenout;
	switch (pvar->GetType())
	{
	case CSIG_AUDIO:
		lenout.clear();
		// next is CSignal, not CSignals (next does not have next)
		xc = 0;
		for (CSignals* pchan = pvar; xc<2 && pchan; xc++, pchan = (CSignals*)pchan->next)
		{
			for (CTimeSeries* p = pchan; p; p = p->chain)
			{
				if (p->nGroups == 1)
					sprintf(outbuf, "%d", p->nSamples);
				else 
					sprintf(outbuf, "%dx%d", p->nGroups, p->Len());
				lenout += outbuf;
				if (p->chain) lenout += ", ";
			}
			if (xc == 0 && pchan->next) lenout += " ; ";
		}
		if (lenout.size()>256) lenout[255] = 0;
		strcpy_s(outbuf, 256, lenout.c_str());
		break;
	case CSIG_SCALAR:
		strcpy(outbuf, "1");
		break;
	case CSIG_NULL:
		strcpy(outbuf, "0");
		break;
	case CSIG_STRING:
	case CSIG_VECTOR:
		if (pvar->nGroups == 1)
			sprintf(outbuf, "%d", (int)pvar->CSignal::length());
		else
			sprintf(outbuf, "%dx%d", pvar->nGroups, pvar->Len());
		break;
	case CSIG_TSERIES:
		sprintf(outbuf, "%d", pvar->CountChains());
		break;
	case CSIG_CELL:
		sprintf(outbuf, "%d", pvar->cell.size());
		break;
	case CSIG_HANDLE:
		sprintf(outbuf, "1");
		break;
	case CSIG_STRUCT:
		sprintf(outbuf, "---");
		break;
	case CSIG_EMPTY:
		sprintf(outbuf, "0");
		break;
	}
}

void CShowvarDlg::showcontent(CVar *pvar, char *outbuf)
{
	string arrout;
	switch (pvar->GetType())
	{
	case CSIG_STRING:
		strcpy(outbuf, pvar->string().c_str());
		break;
	case CSIG_EMPTY:
		sprintf(outbuf, "----");
		break;
	case CSIG_NULL:
		sprintf(outbuf, "NULL(0~%g)", pvar->tmark);
		break;
	case CSIG_VECTOR:
		arrout = "[";
		if (pvar->IsLogical())
		{
			for (unsigned int k = 0; k<min(pvar->nSamples, 10); k++)
			{
				sprintf(outbuf, "%d ", pvar->logbuf[k]); arrout += outbuf;
			}
			if (pvar->nSamples>10) arrout += "...";
		}
		else
		{
			if (pvar->IsComplex())
				for (unsigned int k = 0; k<min(pvar->nSamples, 10); k++)
				{
					showcomplex(outbuf, pvar->cbuf[k]); arrout += outbuf;
				}
			else
				for (unsigned int k = 0; k<min(pvar->nSamples, 10); k++)
				{
					sprintf(outbuf, "%g ", pvar->buf[k]); arrout += outbuf;
				}
			if (pvar->nSamples>10) arrout += "...";
			if (pvar->next)
			{
				arrout += " ; ";
				for (unsigned int k = 0; k<min(pvar->next->nSamples, 10); k++)
				{
					sprintf(outbuf, "%g ", pvar->next->buf[k]); arrout += outbuf;
				}
				if (pvar->next->nSamples>10) arrout += "...";
			}
		}
		strcpy(outbuf, arrout.c_str());
		if (pvar->nSamples <= 10)
			outbuf[strlen(outbuf) - 1] = ']';
		break;
	case CSIG_SCALAR:
		if (pvar->IsLogical())
			sprintf(outbuf, "%d", pvar->logbuf[0]);
		else if (pvar->IsComplex())
			showcomplex(outbuf, pvar->cbuf[0]);
		else
			sprintf(outbuf, "%g", pvar->value());
		break;
	case CSIG_HANDLE:
		if (pvar->IsGO())
		{
			if (pvar->IsString())
				sprintf(outbuf, "\"%s\" [Graphic]", pvar->string().c_str());
			else
				sprintf(outbuf, "%.0lf [Graphic]", pvar->value());
		}
		else if (pvar->IsAudioObj())
			sprintf(outbuf, "[Audioplay]");
		else
			sprintf(outbuf, "Handle");
		break;
	case CSIG_STRUCT:
	case CSIG_CELL:
		sprintf(outbuf, "----");
		break;
	case CSIG_TSERIES:
		sprintf(outbuf, "----");
		break;
	}
}

void CShowvarDlg::updaterow(int row, int col, CVar *pvar)
{
	//currently col is limited to 3 (content)
	char buf[4096];
	LvItem.iItem = row;
	LvItem.iSubItem = col;
	LvItem.pszText = buf;
	int type = pvar->GetType();
	HWND hList;
	(type == CSIG_AUDIO || type == CSIG_NULL) ? hList = GetDlgItem(IDC_LIST1) : hList = GetDlgItem(IDC_LIST2);
	showcontent(pvar, buf);
	::SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem);
}

void CShowvarDlg::fillrowvar(CVar *pvar, string varname)
{ // buf comes with var name and used for other things subsequently.
	HWND hList;
	CSignals tp;
	char buf[4096], buf1[256], buf2[256];
	string out;
	LvItem.iSubItem = 0; //First item (InsertITEM)
	LvItem.pszText = buf;
	if (this==&mShowDlg || varname[0]=='{') // for the main showVar dlg or cell dlg, don't add a dot.
		strcpy(buf, varname.c_str());
	else
		sprintf(buf, ".%s", varname.c_str());
	int type = pvar->GetType();
	(type == CSIG_AUDIO || type == CSIG_NULL) ? hList = GetDlgItem(IDC_LIST1) : hList = GetDlgItem(IDC_LIST2);
	LvItem.iItem = ListView_GetItemCount(hList);
	::SendMessage(hList, LVM_INSERTITEM, 0, (LPARAM)&LvItem);
	LvItem.iSubItem = 1; //second item
	if (type == CSIG_AUDIO)
	{
		if (pvar->IsLogical()) strcpy(buf, "logical");
		else if (!pvar->next)		RMSDB(buf, "-Inf", "%5.1f", pvar->CSignal::RMS())
		else {
			RMSDB(buf1, "-Inf", "%5.1f", pvar->CSignal::RMS())
				RMSDB(buf2, "-Inf", "%5.1f", pvar->next->CSignal::RMS())
				sprintf(buf, "%s,%s", buf1, buf2);
		}
	}
	else
	{
		if (pvar->IsLogical()) 
			sprintf(buf, "LOG");
		else
			showtype(pvar, buf);
	}
	LvItem.pszText = buf;
	::SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem);
	LvItem.iSubItem = 2;
	showsize(pvar, buf); // new buf ready
	LvItem.pszText = buf;
	::SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem);
	LvItem.iSubItem = 3;
	if (type == CSIG_AUDIO)
	{
		out = xcom::outstream_tmarks(pvar, true).str().c_str();
		if (pvar->next) 
		{ 
			out += " ; "; 
			out += xcom::outstream_tmarks(pvar->next, true).str();
		}
		if (out.size()>256) out[255] = 0;
		strcpy_s(buf, 256, out.c_str());
	}
	else
		showcontent(pvar, buf);
	LvItem.pszText = buf;
	::SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem);
}

// Fillup() fills up the listview's from the Vars and govars in the AstSig context.
// mShowDlg::Fillup()  means "fill it up with current Vars and govars
// (other_variable_object)::Fillup(NULL, cellstr, NULL) to view the cell content
// (other_variable_object)::Fillup(vars, cellstr, govar) to view the struct var and govar content

void CShowvarDlg::Fillup(map<string, CVar> *Vars, CSignals *psig)
{
	SendDlgItemMessage(IDC_LIST1,LVM_DELETEALLITEMS,0,0);
	SendDlgItemMessage(IDC_LIST2,LVM_DELETEALLITEMS,0,0);
	changed=false;
	if (pVars)
		for (map<string, CVar>::iterator it = pVars->begin(); it != pVars->end(); it++)
			fillrowvar(&it->second, it->first);
	if (pGOvars)
		for (auto it = pGOvars->begin(); it != pGOvars->end(); it++)
			fillrowvar(it->second, it->first); // 
	AdjustWidths(1); // redraw if this is not the base
}

void CShowvarDlg::fillrowvar(vector<CVar *>gos, string varname)
{ // multiple GOs
	char buf[256];
	LvItem.iSubItem = 0; //First item (InsertITEM)
	LvItem.pszText = buf;
	if (this == &mShowDlg || varname[0] == '{') // for the main showVar dlg or cell dlg, don't add a dot.
		strcpy(buf, varname.c_str());
	else
		sprintf(buf, ".%s", varname.c_str());
	HWND hList = GetDlgItem(IDC_LIST2);
	LvItem.iItem = ListView_GetItemCount(hList);
	::SendMessage(hList, LVM_INSERTITEM, 0, (LPARAM)&LvItem);

	LvItem.iSubItem = 1; //second item
	LvItem.pszText = "HDL";
	::SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem);

	LvItem.iSubItem = 2; //third item
	sprintf(buf, "%d", gos.size());
	LvItem.pszText = buf;
	::SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem);

	LvItem.iSubItem = 3; //fourth item
	unsigned int k(0);
	string strout = "[";
	if (gos.empty())
		strout = "--";
	else
	{
		if (gos.size()>1)
		{
			for (auto psig : gos)
			{
				CAstSig::showGraffyHandle(buf, psig);
				strout += buf;
				if (k == 10) break;
			}
		}
		else
		{
			if (gos.front()->GetType() == CSIG_HDLARRAY)
			{ // This is used when a CSIG_HDLARRAY variable is used (either as a function or made through concatenation or something like that)
				for (unsigned int k = 0; k < gos.front()->nSamples && k < 10; k++)
				{
					CVar *tp = (CVar*)(INT_PTR)gos.front()->buf[k];
					CAstSig::showGraffyHandle(buf, tp);
					strout += buf;
					if (k < gos.front()->nSamples - 1) strout += ' ';
				}
			}
			else
			{
				CAstSig::showGraffyHandle(buf, gos.front());
				strout += buf;
			}
		}
		if (gos.front()->nSamples>10) strout += "...";
		else { trim(strout, ' '); 	strout += ']'; }
	}
	sprintf(buf, "%s", strout.c_str());
	LvItem.pszText = buf;
	::SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem);
}
