// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: auxlab
// Main Application. Based on Windows API  
// 
// 
// Version: 1.499
// Date: 3/11/2019
// 
#include "graffy.h" // this should come before the rest because of wxx820
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <vector>
#include "sigproc.h"
#include "audstr.h"
#include "resource1.h"
#include "showvar.h"
#include "histDlg.h"
#include "consts.h"
#include "xcom.h"

#include "qlparse.h"

char iniFile[256];

double playbackblock(100.);
uintptr_t hShowvarThread(NULL);
uintptr_t hHistoryThread(NULL);
char udfpath[4096];
HANDLE hEventModule;
HANDLE hEventLastKeyStroke2Base;
uintptr_t hAuxconThread;

vector<UINT> exc; // temp

bool moduleLoop(false);

typedef void (*PFUN) (const vector<CAstSig*> &);

xcom mainSpace;
vector<CAstSig*> xcomvecast;
vector<CAstSig*> CAstSig::vecast = xcomvecast;

CWndDlg wnd;
CShowvarDlg mShowDlg(NULL, NULL);
CHistDlg mHistDlg;
void* soundplayPt;
double block;

vector<string> aux_reserves;
void init_aux_reserves()
{
	aux_reserves.push_back("sel");
}

//I could use these typedef for astsig_init--it won't hurt to use them..
typedef void(*port1)(CAstSig *, DEBUG_STATUS);
typedef void(*port2)(CAstSig *, const AstNode *);
typedef bool(*port3)(const char*);

void debug_appl_manager(CAstSig *debugAstSig, DEBUG_STATUS debug_status, int line);
void HoldAtBreakPoint(CAstSig *past, const AstNode *pnode);
bool dbmapfind(const char* udfname);
void Back2BaseScope(int closeauxcon);

void ValidateFig(const char* scope);

extern vector<cfigdlg*> plots;

extern uintptr_t hDebugThread2;
extern unordered_map<string, CDebugDlg*> dbmap;
extern CDebugBaseDlg debugBase;
extern vector<CWndDlg*> cellviewdlg;

unsigned int WINAPI debugThread2 (PVOID var) ;

HANDLE hStdin, hStdout; 
BOOL CALLBACK showvarDlgProc (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK historyDlgProc (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam);
BOOL CtrlHandler( DWORD fdwCtrlType );
void nonnulintervals(CTimeSeries *psig, string &out, bool unit, bool clearout=false);
size_t ReadThisLine(string &linebuf, HANDLE hCon, CONSOLE_SCREEN_BUFFER_INFO coninfo0, SHORT thisline, size_t promptoffset);

void nonnulintervals(CTimeSeries *psig, string &out, bool unit, bool clearout)
{
	char buf[128];
	int kk(0), tint(psig->CountChains());
	if (clearout) out.clear();
	for (CTimeSeries *xp=psig;kk<tint;kk++,xp=xp->chain) {
		sprintf(buf, "(%g", xp->tmark);
		if (unit) strcat(buf, "ms");
		out += buf;
		sprintf(buf, "~%g", xp->tmark + 1000.* xp->nSamples/xp->GetFs());
		if (unit) strcat(buf, "ms");
		strcat(buf, ") ");
		out += buf;
	}
}

WORD readINI_pos(const char *fname, CRect *rtMain, CRect *rtShowDlg, CRect *rtHistDlg, CRect *rtDebugDlg)
{
	char errStr[256];
	int tar[4];
	WORD ret(0);
	string strRead;
	CRect crTemp(0, 0, 500, 400);
	if (ReadINI (errStr, fname, "WINDOW POS", strRead)>=0 && str2array (tar, 4, strRead.c_str(), " ")==4)
	{
		if ((tar[2] + tar[0]) < 0) *rtMain = crTemp;
		else
		{
			rtMain->left = tar[0];
			rtMain->top = tar[1];
			rtMain->right = tar[2] + tar[0];
			rtMain->bottom = tar[3] + tar[1];
		}
		ret += 1;
	}
	if (ReadINI (errStr, fname, "VAR VIEW POS", strRead)>=0 && str2array (tar, 4, strRead.c_str(), " ")==4)
	{
		if ((tar[2] + tar[0]) < 0) {
			*rtShowDlg = crTemp; rtShowDlg->MoveToX(500);
		}
		else
		{
			rtShowDlg->left = tar[0];
			rtShowDlg->top = tar[1];
			rtShowDlg->right = tar[2] + tar[0];
			rtShowDlg->bottom = tar[3] + tar[1];
		}
		ret += 2;
	}
	if (ReadINI (errStr, fname, "HIST POS", strRead)>=0 && str2array (tar, 4, strRead.c_str(), " ")==4)
	{
		if ((tar[2] + tar[0]) < 0) {
			*rtHistDlg = crTemp; rtShowDlg->MoveToY(400);
		}
		else
		{
			rtHistDlg->left = tar[0];
			rtHistDlg->top = tar[1];
			rtHistDlg->right = tar[2] + tar[0];
			rtHistDlg->bottom = tar[3] + tar[1];
		}
		ret += 4;
	}
	if (ReadINI (errStr, fname, "DEBUG VIEW POS", strRead)>=0 && str2array (tar, 4, strRead.c_str(), " ")==4)
	{
		if ((tar[2] + tar[0]) < 0) {
			*rtDebugDlg = crTemp; rtShowDlg->MoveToXY(500,400);
		}
		else
		{
			rtDebugDlg->left = tar[0];
			rtDebugDlg->top = tar[1];
			rtDebugDlg->right = tar[2] + tar[0];
			rtDebugDlg->bottom = tar[3] + tar[1];
			ret += 8;
		}
	}
	return ret;
}

int readINIs(const char *fname, char *estr_dummy, int &fs, double &_block, char *path)
{//in, in, out, out, in/out
	string strRead;
	int val;
	double dval;
	int res = ReadINI (estr_dummy, fname, "SAMPLE RATE", strRead);
	if (res>0 && sscanf(strRead.c_str(), "%d", &val)!=EOF && val >10)	
		fs = val;
	else																
		fs = DEFAULT_FS;
	res = ReadINI (estr_dummy, fname, "PLAYBACK BLOCK SIZE MILLISEC", strRead);
	if (res>0 && sscanf(strRead.c_str(), "%lf", &dval)!=EOF && dval >10.)	
		_block = dval;
	else																
		_block = DEFAULT_PLAY_BLOCK_MS;
	if (ReadINI (estr_dummy, fname, "PATH", strRead)>=0)
		strcat(path, strRead.c_str());
	return 1;
}


int writeINIs(const char *fname, char *estr, int fs, double _block, const char *path)
{
	char errStr[256];
	if (!printfINI (errStr, fname, "SAMPLE RATE", "%d", fs)) {strcpy(estr, errStr); 	return 0;}
	if (!printfINI (errStr, fname, "PLAYBACK BLOCK SIZE MILLISEC", "%.1f", _block))	{strcpy(estr, errStr);	return 0;}
	if (!printfINI (errStr, fname, "PATH", "%s", path)) {strcpy(estr, errStr); return 0;}
	return 1;
}

int writeINI_pos(const char *fname, char *estr, CRect rtMain, CRect rtShowDlg, CRect rtHistDlg)
{
	char errStr[256];
	CString str;
	str.Format("%d %d %d %d", rtMain.left, rtMain.top, rtMain.Width(), rtMain.Height());
	if (!printfINI (errStr, fname, "WINDOW POS", "%s", str.c_str())) {strcpy(estr, errStr); return 0;}
	str.Format("%d %d %d %d", rtShowDlg.left, rtShowDlg.top, rtShowDlg.Width(), rtShowDlg.Height());
	if (!printfINI (errStr, fname, "VAR VIEW POS", "%s", str.c_str())) {strcpy(estr, errStr); return 0;}
	str.Format("%d %d %d %d", rtHistDlg.left, rtHistDlg.top, rtHistDlg.Width(), rtHistDlg.Height());
	if (!printfINI (errStr, fname, "HIST POS", "%s", str.c_str())) {strcpy(estr, errStr); return 0;}
	return 1;
}

CWndDlg * Find_cellviewdlg(const char *name)
{ // Currently this finds only CVectorsheetDlg... Expand to search other CShowDlg
	char buf[256];
	if (!name) return NULL;
	vector<CWndDlg*>::iterator it;
	for (it = cellviewdlg.begin(); it != cellviewdlg.end(); it++)
	{
		GetWindowText((*it)->hDlg, buf, sizeof(buf));
		if (!strcmp(buf, name))
			return *it;
	}
	return NULL;
}

void closeXcom(const char *AppPath)
{
	//When CTRL_CLOSE_EVENT is pressed, you have 5 seconds. That's how Console app works in Windows.
	// Finish all cleanup work during this time.

	CAstSig *pcast = CAstSig::vecast.front();
	char estr[256], buffer[256];
	const char *sss = pcast->GetPath();
	const char* pt = strstr(pcast->GetPath(), AppPath); 
	string pathnotapppath;
	size_t loc;
	if (pt!=NULL)
	{
		pathnotapppath.append(pcast->GetPath(),  pcast->GetPath()-pt);
		string  str(pt + strlen(AppPath));
		loc = str.find_first_not_of(';');
		pathnotapppath.append(pt + strlen(AppPath)+loc);
	}
	else
		pathnotapppath.append(pcast->GetPath());
	int res = writeINIs(iniFile, estr, pcast->pEnv->Fs, pcast->audio_block_ms, pathnotapppath.c_str());
	delete pcast->pEnv;

	CRect rt1, rt2, rt3;
	GetWindowRect(GetConsoleWindow(), &rt1);
	mShowDlg.GetWindowRect(rt2);
	mHistDlg.GetWindowRect(rt3);
	res = writeINI_pos(iniFile, estr, rt1,rt2, rt3);

	string debugudfs("");
	for (unordered_map<string, CDebugDlg*>::iterator it=dbmap.begin(); it!=dbmap.end(); it++)
	{
		debugudfs += it->second->fullUDFpath; debugudfs += "\r\n";
	}
	if (debugudfs.size()>0)
	{
		if (!printfINI (estr, iniFile, "DEBUGGING UDFS", "%s", debugudfs.c_str())) strcpy(buffer, estr); 
		unordered_map<string, CDebugDlg*>::iterator it=dbmap.begin(); 
		GetWindowRect(it->second->hParent->hDlg, &rt1);
		CString str;
		str.Format("%d %d %d %d", rt1.left, rt1.top, rt1.Width(), rt1.Height());
		if (!printfINI (estr, iniFile, "DEBUG VIEW POS", "%s", str.c_str())) {strcpy(buffer, estr);/*do something*/ }
	}
	SYSTEMTIME lt;
	GetLocalTime(&lt);	
	vector<string> in;
	sprintf(buffer, "//\t[%02d/%02d/%4d, %02d:%02d:%02d] AUXLAB closes------", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
	in.push_back(buffer);
	mainSpace.LogHistory(in);
	if (hShowvarThread!=NULL) PostThreadMessage(GetWindowThreadProcessId(mShowDlg.hDlg, NULL), WM__ENDTHREAD, 0, 0);
	if (hHistoryThread!=NULL) PostThreadMessage(GetWindowThreadProcessId(mHistDlg.hDlg, NULL), WM__ENDTHREAD, 0, 0);
	int debugview(0);
	if (hDebugThread2)
		debugview = 1;
	else
	{
		unordered_map<string, CDebugDlg*>::iterator it=dbmap.begin();
		if (it!=dbmap.end())
			PostThreadMessage(GetWindowThreadProcessId((it->second)->hDlg, NULL), WM__ENDTHREAD, 0, 0);
	}
	if (!printfINI(estr, iniFile, "DEBUG VIEW", "%d", debugview))
	{	//do something	
	}
	for (vector<CAstSig*>::iterator it = CAstSig::vecast.begin()+1; it != CAstSig::vecast.end(); it++)
		delete *it;
	fclose(stdout);
	fclose(stdin);
}

unsigned int WINAPI histThread (PVOID var) 
{
	WNDCLASS wc;
	int res2 = GetClassInfo(GetModuleHandle(NULL), "#32770", &wc);
	wc.lpszClassName = "SECOND";
	wc.cbWndExtra = DLGWINDOWEXTRA;
	ATOM res6 = RegisterClass(&wc);

	HWND hhh = GetConsoleWindow();
	mHistDlg.hDlg = CreateDialog (NULL, MAKEINTRESOURCE(IDD_HISTORY), GetConsoleWindow(), (DLGPROC)historyDlgProc);
//	if (mHistDlg.hDlg==NULL) {MessageBox(NULL, "History Dlgbox creation failed.","AUXLAB",0); return 0;}

	ShowWindow(mHistDlg.hDlg, SW_SHOW);
	char buf[256];
	GetClassName(mHistDlg.hDlg, buf, sizeof(buf));
	HANDLE h = LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, 0); // success.. why? 12/37am 10/29/2017
	SetClassLongPtr(mHistDlg.hDlg, GCLP_HICON, (LONG)(LONG_PTR)h);

	CRect rt1, rt2, rt3, rt4;
	int res = readINI_pos(iniFile, &rt1, &rt2, &rt3, &rt4);
	if (res & 4)	
		mHistDlg.MoveWindow(rt3);
	else
	{
		RECT rc, rc2;
		int res = GetWindowRect(GetConsoleWindow(), &rc);
		rc2 = rc;
		int width = rc.right-rc.left;
		int height = rc.bottom-rc.top;
		rc2.left = rc.right;
		rc2.right = rc2.left + width/7*4;
		rc2.bottom = rc.bottom;
		rc2.top = rc2.bottom - height/5*4;
		int res2 = MoveWindow(mHistDlg.hDlg, rc2.left, rc2.top, width/7*4, height/5*4, 1);
	}

	SetFocus(GetConsoleWindow()); //This seems to work.
	SetForegroundWindow (GetConsoleWindow());

	MSG         msg ;
	while (GetMessage (&msg, NULL, 0, 0))
	{ 
		if (msg.message==WM__ENDTHREAD) 
			_endthreadex(17);
		TranslateMessage (&msg) ;
		DispatchMessage (&msg) ;
	}
	hHistoryThread=NULL;
	return 0;
}

CWndDlg *GetCWndDlg(HWND hwnd)
{
	for (vector<CWndDlg*>::iterator it = cellviewdlg.begin(); it != cellviewdlg.end(); it++)
	{
		if ((*it)->hDlg == hwnd)
			return *it;
	}
	return NULL;
}

unsigned int WINAPI showvarThread (PVOID var) // Thread for variable show
{
	bool win7 = isWin7() ? true : false;
	HINSTANCE hModule = GetModuleHandle(NULL);
	mShowDlg.hDlg = CreateDialogParam (hModule, MAKEINTRESOURCE(IDD_SHOWVAR), win7 ? NULL : GetConsoleWindow(), (DLGPROC)showvarDlgProc, (LPARAM)&mShowDlg);
	ShowWindow(mHistDlg.hDlg,SW_SHOW);
	mShowDlg.hList1 = GetDlgItem(mShowDlg.hDlg , IDC_LIST1);
	HANDLE h = LoadImage(hModule, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, 0);
	SetClassLongPtr (mShowDlg.hDlg, GCLP_HICON, (LONG)(LONG_PTR)LoadImage(hModule, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, 0));
	mShowDlg.win7 = win7;

	RECT rc;
	CRect rt1, rt2, rt3, rt4;
	int res = readINI_pos(iniFile, &rt1, &rt2, &rt3, &rt4);
	if (res & 2)	
		mShowDlg.MoveWindow(rt2);
	else
	{
		GetWindowRect(GetConsoleWindow(), &rc);
		int width = rc.right-rc.left;
		int height = rc.bottom-rc.top;
		RECT rc2(rc);
		rc2.right = rc.left;
		rc2.left = rc2.right - width/7*4+10;
		rc2.bottom = rc.bottom;
		rc2.top = rc2.bottom - height/5*4;
		if (rc2.left<0) 
		{
			rc2.right -= rc2.left;
			rc2.left = 0;
			MoveWindow(GetConsoleWindow(), rc2.right, rc.top, width, height, 1);
		}
		MoveWindow(mShowDlg.hDlg, rc2.left, rc2.top, width/7*4, height/5*4, 1);
	}
	//begin debug thread only if previously debug files were open
	char errStr[256];
	string strRead;
	if (ReadINI(errStr, iniFile, "DEBUG VIEW", strRead) > 0)
	{
		int ival;
		if (sscanf(strRead.c_str(), "%d", &ival) != EOF)
		{
			if (ival>0 && ReadINI(errStr, iniFile, "DEBUGGING UDFS", strRead) >= 0)
			{
				if ((hDebugThread2 = _beginthreadex(NULL, 0, debugThread2, (void*)&rt4, 0, NULL)) == -1)
					MessageBox(GetConsoleWindow(), "Debug Thread Creation Failed.", 0, 0);
				if (!(res & 8)) // if rt4 is not available from readINI() above, place it bottom right corner of main window
				{
					GetWindowRect(GetConsoleWindow(), &rc);
					rt4 = rc;
					CRect tp(rt4.CenterPoint(), rt4.BottomRight());
					rt4 = tp + CPoint(30, 30);
				}
			}
		}
	}

	char buf[256], newTitle[256];
//	RetrieveVersionString(string(string(mShowDlg.AppPath) + mShowDlg.AppRunName).c_str(), vstr, sizeof(vstr));
	mShowDlg.GetWindowText(buf, sizeof(buf));
	sprintf(newTitle, "AUXLAB %s [%s]", mainSpace.AppVersion, buf);
	mShowDlg.SetWindowText(newTitle);

	SetFocus(GetConsoleWindow()); //This seems to work.
	SetForegroundWindow (GetConsoleWindow());

	MSG msg ;
	HACCEL hAcc = LoadAccelerators (hModule, MAKEINTRESOURCE(IDR_XCOM_ACCEL));

	//FILE *fpp = fopen("c:\\temp\\rec","wt"); 
	//fclose(fpp);

	exc.push_back(WM_NCHITTEST);
	exc.push_back(WM_SETCURSOR);
	exc.push_back(WM_MOUSEMOVE);
	exc.push_back(WM_NCMOUSEMOVE);
	exc.push_back(WM_WINDOWPOSCHANGING);
	exc.push_back(WM_WINDOWPOSCHANGED);
	exc.push_back(WM_CTLCOLORDLG);
	exc.push_back(WM_NCPAINT);
	exc.push_back(WM_GETMINMAXINFO);
	exc.push_back(WM_MOVE);
	exc.push_back(WM_MOVING);
	exc.push_back(WM_PAINT);
	exc.push_back(WM_NCMOUSEMOVE);
	exc.push_back(WM_ERASEBKGND);
	exc.push_back(WM_TIMER);
//	bool printthis;

	while (GetMessage (&msg, NULL, 0, 0))
	{
		if (msg.message==WM__ENDTHREAD) 
			_endthreadex(33);
//		printthis = spyWM(msg.hwnd, msg.message, msg.wParam, msg.lParam, "c:\\temp\\rec", exc, "showvar MessageLoop")>0;
//		if (printthis) fpp=fopen("c:\\temp\\rec","at"); 
		if (!TranslateAccelerator(mShowDlg.hDlg, hAcc, &msg))
		{
	//		if (!IsDialogMessage(msg.hwnd, &msg)) // This should not be used....why?? 4/2/2018
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
//			if (printthis)  fprintf(fpp, "TranslateMessage/DispatchMessage\n"); 
		}
		//else
		//{
		//	if (printthis)  fprintf(fpp, "TranslateAccelerator_mShowDlg.hDlg success.\n"); 
		//}
//		if (printthis) fclose(fpp);
	}
	hShowvarThread=NULL;
	return 0;
}

#define	ISTHISSTR(STR) (!strcmp(pnode->str,STR))

bool need2echo(const AstNode *pnode)
{
	//if there are multiple statements on one line, 

	//if node.str is one of the following, immediately return false
	if (pnode->str)
	{
		if (ISTHISSTR("play")) return false;
		if (ISTHISSTR("wavwrite")) return false;
		if (ISTHISSTR("show")) return false;
		if (ISTHISSTR("fprintf")) return false;
		if (ISTHISSTR("fdelete")) return false;
		if (ISTHISSTR("include")) return false;
		if (ISTHISSTR("eval")) return false;
		if (ISTHISSTR("eval")) return false;
	}
	return true;
}

xcom::xcom()
:nHistFromFile(50), comPrompt(MAIN_PROMPT), need2validate(false)
{
//	comPrompt += (char)175;
}

xcom::~xcom()
{

}

bool isUpperCase(char c)
{
	return (c>64 && c<91);
}
bool isLowerCase(char c)
{
	return (c>96 && c<123);
}

char alphas[]={"QWERTYUIOPASDFGHJKLZXCVBNMqwertyuioplkjhgfdsazxcvbnm"};
char nums[]={"1234567890."};
char alphanums[]={"1234567890.QWERTYUIOPASDFGHJKLZXCVBNMqwertyuioplkjhgfdsazxcvbnm"};

size_t xcom::ctrlshiftleft(const char *buf, DWORD offset)
{
	//RETURNS the position satisfying the condition (zero-based)
	size_t len = strlen(buf);
	char copy[4096];
	strcpy(copy, buf);
	copy[len-offset]=0;
	string str(copy);
	size_t res, res1, res2, res12(0);
	if (isalpha(copy[len-offset-1]))
	{
		if ((res = str.find_last_not_of(alphas))!=string::npos)
			return str.find_first_of(alphas, res);
		else					
			return str.find_first_of(alphas);
	}
	else if (isdigit(copy[len-offset-1]))
	{
		//return the idx of first numeric char
		res = str.find_last_not_of(nums);
			return str.find_first_of(nums, res);
	}
	else
	{
		res1 = str.find_last_of(alphas);
		res2 = str.find_last_of(nums);
		if (res1==string::npos)
		{
			if (res2!=string::npos) 
				res12 = res2;
		}
		else
		{
			if (res2!=string::npos) 
				res12 = max(res1,res2)+1;
			else
				res12 = res1;
		}
		return res12;
	}
	return 0;
}

size_t xcom::ctrlshiftright(const char *buf, DWORD offset)
{
	//RETURNS the position satisfying the condition (zero-based)
	size_t len = strlen(buf);
	string str(buf);
	size_t res, res1, res2, res12(0);
	size_t inspt = len-offset+1;
	if (isalpha(buf[inspt]))
	{
		res = str.find_first_not_of(alphas,len-offset);
		return str.find_last_of(alphas, res);
	}
	else if (isdigit(buf[inspt]))
	{
		//return the idx of first numeric char
		res = str.find_first_not_of(nums,len-offset);
		return str.find_last_of(nums, res);
	}
	else
	{
		res1 = str.find_first_of(alphas,inspt);
		res2 = str.find_first_of(nums,inspt);
		if (res1==string::npos)
		{
			if (res2!=string::npos) 
				res12 = res2;
		}
		else
		{
			if (res2!=string::npos) 
				res12 = min(res1,res2);
			else
				res12 = res1;
		}
		return res12;
	}
	return 0;
}

void xcom::checkdebugkey(INPUT_RECORD *in, int len)
{
	// ReadConsoleInput takes the input event and returns, not exactly totally expected ways.
	// For example, it returns with one event with bKeyDown set and subsequently with the intended virtual keycode
	// returns two events with one bKeyDown up with the intended virtual keycode and another event (e.g., FOCUS_EVENT) 
	// So, the key processing should take place across multiple calls to this function with vcode, vcodeLast as static variables.
	// Hypothetically, if events of multiple keystrokes, for eaxmple, F10 key down, up, down up), come in with one return of ReadConsoleInput, only the first key stroke pair (down/up) will be taken and further processed.
	// But it doesn't always happen that way... in most cases, multiple keystrokes of the same key come in with separate returns of ReadConsoleInput.
	// 10/17/2017 bjk
	if (len==0) 
		return ;
	int countNonKeyEventID(0);
	vector<int> nonKeyEventID;
	static WORD vcode(0), vcodeLast(0);
	WORD vcode0; 
	for (int k=0; k<len; k++)
	{
		if (in[k].EventType !=	KEY_EVENT) 	nonKeyEventID.push_back(k);
	}
	if (nonKeyEventID.size()==len) 
		return ;
	for (int k=0; k<len; k++)
	{
		// Exclude the event if not a keyboard event.
		if (find(nonKeyEventID.begin(), nonKeyEventID.end(), k)==nonKeyEventID.end())
		{
			if (in[k].Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED ||
				in[k].Event.KeyEvent.dwControlKeyState & LEFT_CTRL_PRESSED ||
				in[k].Event.KeyEvent.dwControlKeyState & RIGHT_CTRL_PRESSED)
			{
				// When control or shift key is pressed, only down-key event can lead to the action. Will this be a problem?
				// Maybe. I don't know. 11/23/2017
				vcode0 = in[k].Event.KeyEvent.wVirtualKeyCode;
				if (vcode0 >= VK_F1)
				{
					vcodeLast = vcode = vcode0;
					break;
				}
			}
			if (in[k].Event.KeyEvent.bKeyDown) 
				vcodeLast = in[k].Event.KeyEvent.wVirtualKeyCode;
			else
				vcode = in[k].Event.KeyEvent.wVirtualKeyCode;
			if (vcode==vcodeLast)
				break;
		}
	}
	if (vcode!=vcodeLast) 
		return ;
	vcode0=vcode;
	vcode=vcodeLast=0;
	switch(vcode0)
	{
	case VK_F11:
		throw debug_F11;
	case VK_F5:
		if (in[0].Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)
			throw debug_Shift_F5;
		else
			throw debug_F5;
	case VK_F10:
		if (in[0].Event.KeyEvent.dwControlKeyState & LEFT_CTRL_PRESSED || in[0].Event.KeyEvent.dwControlKeyState & RIGHT_CTRL_PRESSED)
			throw debug_Ctrl_F10;
		else
			throw debug_F10;
	}
}


void xcom::console()
{
	char buf[4096] = {};
	while(1) 
	{
		buf[0] = 0;
		getinput(buf); // this is a holding line.
		SendMessage(hLog, WM__LOG, (WPARAM)strlen(buf), (LPARAM)buf);
		trimLeft(buf,"\xff");
		trimRight(buf,"\r\n\xff");
		if (mainSpace.computeandshow(buf)==-1) break;
	}
}

int xcom::cleanup_debug()
{
	while (mShowDlg.SendDlgItemMessage(IDC_DEBUGSCOPE, CB_DELETESTRING, 1)!=CB_ERR) {}
	mShowDlg.SendDlgItemMessage(IDC_DEBUGSCOPE, CB_SETCURSEL, 0);

	mShowDlg.debug(exiting, NULL, -1);

	// Do memory clean up of sub in CallUDF()
	return 0;
}

ostringstream outstream_complex(complex<double> cval)
{
	ostringstream out;
	double im = imag(cval);
	if (cval == 0.) { out << "0"; return out; }
	if (real(cval) != 0.)	out << real(cval);
	if (im == 0.) return out;
	if (!out.str().empty() && im>0) out << "+";
	if (im != 1.)	out << im;
	out << (char) 161;
	return out;
}


void printf_vector(CVar *pvar, unsigned int id0, int offset, const char *postscript)
{
	cout << xcom::outstream_vector(pvar, id0, offset).str();
	cout << postscript << endl;
}

//void printf_single(CVar *pvar, bool hex = false);

void printf_single(CVar *pvar)
{
	if (pvar->IsComplex())
		cout << outstream_complex(pvar->cvalue()).str();
	else if (pvar->IsLogical())
		cout << pvar->logbuf[0];
	else
	{
		//if (hex)
		//{
		//	int ival = (int)pvar->value();
		//	cout << ival;
		//}
		//else
			cout << pvar->valuestr();
	}
}

ostringstream xcom::outstream_tmarks(CTimeSeries *psig, bool unit)
{
	// unit is to be used in the future 8/15/2018
	ostringstream out;
	streamsize org_precision = out.precision();
	out.setf(ios::fixed);
	out.precision(1);
	int kk(0), tint(psig->CountChains());
	for (CTimeSeries *xp = psig; kk<tint; kk++, xp = xp->chain) {
		out << "(" << xp->tmark;
		if (unit) out << "ms";
		out << "~" << xp->tmark + 1000.* xp->nSamples / xp->GetFs();
		if (unit) out << "ms";
		out << ") ";
	}
	out << endl;
	out.unsetf(ios::floatfield);
	out.precision(org_precision);
	return out;
}

ostringstream xcom::outstream_value(double val, int offset)
{
	ostringstream out;
	if (val > .001 || val < -.001)
		out << val;
	else
	{
		streamsize org_precision = out.precision();
		out.setf(ios::scientific, ios::floatfield);
		out << val;
		out.precision(org_precision);
	}
	return out;
}

ostringstream xcom::outstream_vector(CSignal *pvar, unsigned int id0, int offset)
{
	ostringstream out;
	streamsize org_precision = out.precision();
//	out.setf(ios::fixed);
//	out.precision(1);
	unsigned int k = 0;
	if (pvar->IsComplex())
		for (; k < min(10, pvar->Len()); k++, out << " ")
		{
			for (int m = 0; m < offset; m++) out << " ";
			out << outstream_complex(pvar->cbuf[k + id0]).str();
		}
	else
	{
		for (int m = 0; m < offset; m++) out << " ";
		if (pvar->IsLogical())
		{
			for (; k < min(10, pvar->Len()); k++, out << " ")
				out << pvar->logbuf[k + id0];
		}
		else
			for (; k < min(10, pvar->Len()); k++, out << " ")
				out << pvar->buf[k + id0];
	}
	if (pvar->Len() > 10) // this means nSamples is greater than 10
		out << " ... (length = " << pvar->Len() << ")";
//	out.unsetf(ios::floatfield);
//	out.precision(org_precision);
	return out;
}

ostringstream xcom::outstream_tseq(CTimeSeries *psig, bool unit)
{
	ostringstream out;
	streamsize org_precision = out.precision();
	out.setf(ios::fixed);
	if (unit) out.precision(1);
	else out.precision(2);
	int kk(0), tint(psig->CountChains());
	for (CTimeSeries *xp = psig; kk < tint; kk++, xp = xp->chain)
	{
		out << "(" << xp->tmark;
		if (unit) out << "ms";
		out << ") ";
		out << outstream_vector(xp, 0, 0).str() << endl;
	}
	out.unsetf(ios::floatfield);
	out.precision(org_precision);
	return out;
}

void printf_tseries(CTimeSeries *psig, bool unit)
{
	cout << xcom::outstream_tseq(psig,unit).str();
}

void xcom::echo(const char *varname, CVar *pvar, int offset, const char *postscript)
{
	ios_base::fmtflags org_flags;
	unsigned int j = 1;
	CVar temp;
	streamsize org_precision(-1);
	bool passingdown(false);
	switch (pvar->GetType())
	{
	case CSIG_EMPTY:
		for (int k = 0; k < offset; k++) cout << " ";
		cout << varname << " = ";
		cout << "[]";
		cout << postscript << endl;
		break;
	case CSIG_SCALAR:
		for (int k = 0; k < offset; k++) cout << " ";
		cout << varname << " = ";
		if (strstr(postscript, "[Handle]"))
		{
			org_precision = cout.precision();
			org_flags = cout.flags();
			cout.unsetf(ios::floatfield);
			cout.precision(14);
		}
		printf_single(pvar);
		cout << postscript << endl;
		if (org_precision != -1) {
			cout.precision(org_precision); cout.flags(org_flags);		}
		break;
	case CSIG_STRING:
		for (int k = 0; k < offset; k++) cout << " ";
		cout << varname << " = ";
		cout << "\"" << pvar->string() << "\"" << postscript << endl;
		break;
	case CSIG_TSERIES:
		for (int k = 0; k < offset; k++) cout << " ";
		cout << varname << " = " << endl;
		printf_tseries(pvar, pvar->GetFs()>0);
		break;
	case CSIG_VECTOR:
		for (int p = 0; p < offset; p++) cout << " ";
		cout << varname << " = ";
		if (pvar->IsLogical()) cout << "(logical) ";
		if (pvar->nGroups > 1) cout << endl;
		for (j = 0; j < min(10, pvar->nGroups); j++)
			printf_vector(pvar, pvar->Len()*j, offset+1, postscript);
		if (j==10)
			cout << "\t" << "... (total rows) = " << pvar->nGroups << endl;
		break;
	case CSIG_AUDIO:
		for (int k = 0; k < offset; k++) cout << " ";
		cout << varname << " =" << endl;
		if (pvar->IsStereo())
		{
			for (int k = 0; k < offset+1; k++) cout << " ";
			cout << "audio(L) ";
			cout << xcom::outstream_tmarks(pvar, true).str();
			for (int k = 0; k < offset + 1; k++) cout << " ";
			cout << "audio(R) ";
			cout << xcom::outstream_tmarks(pvar->next, true).str();
		}
		else
		{
			for (int k = 0; k < offset + 1; k++) cout << " ";
			cout << "audio ";
			cout << xcom::outstream_tmarks(pvar, true).str();
		}
		break;
	case CSIG_CELL:
		for (vector<CVar>::iterator it = pvar->cell.begin(); it!= pvar->cell.end(); it++)
		{
			ostringstream _varname;
			_varname << varname << '{' << j++ << '}';
			echo(_varname.str().c_str(), &*it);
		}
		break;
	case CSIG_HDLARRAY:
		for (unsigned int k = 0; k < pvar->nSamples; k++)
		{
			ostringstream varstr;
			varstr << varname << '(' << k+1 << ')';
			CVar *tp = (CVar*)(INT_PTR)pvar->buf[k];
			echo(varstr.str().c_str(), tp, offset);
		}
		break;
	case CSIG_HANDLE:
		for (int k = 0; k < offset; k++) cout << " ";
		temp.UpdateBuffer(1);
		memcpy(temp.buf, pvar->buf, pvar->bufBlockSize);
		if (pvar->bufBlockSize == 1) 	temp.SetFs(2);
		echo(varname, &temp, offset-1, " [Handle]");
		passingdown = true;
	case CSIG_STRUCT:
		if (!passingdown)		cout << varname << " [Structure]" << endl;
		for (map<string, CVar>::iterator it = pvar->strut.begin(); it!= pvar->strut.end(); it++)
		{
			ostringstream var0;
			var0 << '.' << it->first;
			echo(var0.str().c_str(), &it->second, offset);
		}
		for (map<string, vector<CVar*>>::iterator it = pvar->struts.begin(); it != pvar->struts.end(); it++)
		{
			ostringstream var0;
			var0 << '.' << it->first;
			for (vector<CVar*>::iterator jt = (*it).second.begin(); jt != (*it).second.end() && j < 10; jt++)
			{
				if (!CAstSig::HandleSig(&temp, *jt)) temp.SetString("(internal error)");
				echo(var0.str().c_str(), &temp, offset, " [Handle]");
			}
		}
		break;
	default:
		break;
	}
}

void xcom::echo(CAstSig *pctx, const AstNode *pnode, CVar *pvar)
{
	/*
	c=10 '='
	c>1 '>'
	c T_ID
	a.member T_ID
	a.member=0 '='
	c(3)=0 N_IXASSIGN
	v(4) N_CALL
	sqrt(4) N_CALL
	c{1} N_CELL
	c{1}(2) N_CELL
	c{1}=0 N_CELLASSIGN
	c{1}(3)=0 N_CELLASSIGN with 'child'
	*/

	AstNode *p;
;	if (!pnode->suppress)
	{
		if (!pvar)
		{
			string vam;
			pvar = pctx->GetVariable(pnode->str, vam);
			if (!pvar) return; // this filters out a null statement in a block such as a=1; b=100; 500 
		}
		if (CAstSig::IsLooping(pnode)) return; // T_IF, T_FOR, T_WHILE
		if (CAstSig::IsTID(pnode))
		{
			p = pnode->alt;
			if (pnode->child)
				echo(pctx->Script.c_str(), pvar);
			else if ( !pnode->next && (pvar->functionEvalRes) || CAstSig::IsCondition(p))
			{
				pctx->SetVar("ans", pvar);
				echo("ans", pvar);
			}
			else if (p && CAstSig::IsCELL_STRUCT_pnode_TID_ARGS(pnode, p))
			{
				vector<string> parse;
				str2vect(parse, pctx->Script.c_str(), "=+-*/@^#$%");
				echo(parse.front().c_str(), pvar);
			}
			else
				echo(pnode->str, pvar);
		}
		else
		{
			pctx->SetVar("ans", pvar);
			echo("ans", pvar);
		}
	}
}

int xcom::computeandshow(const char *in, CAstSig *pTemp)
{ 
	CAstSig *pContext;
	string emsg;
	bool succ(true);
	if (pTemp == NULL)
		pContext = CAstSig::vecast.back();
	else
		pContext = pTemp;
	bool err(false);
	size_t nItems, k(0);
	string input(in);
	trim(input, " \t\r\n");
	trimr(input, "\r\n");
	if (input.size()>0)
	try { 
		//if the line begins with #, it bypasses the usual parsing
		if (input[0] == '#')
		{
			string tar[2];
			string input1 = input.substr(1);
			nItems = str2array(tar, 2, input1.c_str(), " ");
			if (nItems == 0) return 0;
			if (hook(pContext, tar[0], tar[1].c_str()) == 0)
			{
				ShowWS_CommandPrompt(pContext);
				return pTemp ? 1 : 0;
			}
		}
		if (!pContext->SetNewScript(emsg, input.c_str()))
			throw emsg.c_str();
		pContext->statusMsg.clear();
		pContext->Compute();
		if (pTemp == NULL)
			CDebugDlg::pAstSig = NULL;
		if (CAstSig::IsBLOCK(pContext->pAst))
		{
			for (const AstNode *pp = pContext->pAst->next; pp; pp = pp->next)
			{
				if (pp->next) // 47362898.html
				{
					((AstNode *)pp)->suppress = true; 
					echo(pContext, pp);
				}
				else
					echo(pContext, pp, pContext->Sig.IsGO() ? pContext->pgo : &pContext->Sig); 
			}
		}
		else if (CAstSig::IsVECTOR(pContext->pAst) && pContext->pAst->alt && pContext->pAst->alt->type!=N_STRUCT) // pContext->pAst->alt is necessary to ensure that there's a vector on the LHS 
		{
			for (const AstNode *pp = pContext->pAst->alt; !pContext->pAst->suppress && pp; pp = pp->next)
				echo(pContext, pp);
		}
		else
			echo(pContext, pContext->pAst, pContext->Sig.IsGO() ? pContext->pgo : &pContext->Sig); // fro739222985.html
		//if LHS is x(arg_list) or x(time_extraction), Sig is partial. This needs to the entire portion for plot window
		if (CAstSig::IsTID(pContext->pAst) && pContext->pAst->alt && CAstSig::IsPortion(pContext->pAst->alt))
		{
			string vam;
			CVar *tp = pContext->GetVariable(pContext->pAst->str, vam);
			if (tp) pContext->Sig = *tp;
		}
	}
	catch (const char *_errmsg) {
		bool gotobase = false;
		if (!strncmp(_errmsg, "[GOTO_BASE]", strlen("[GOTO_BASE]")))
			gotobase = true;
		char *errmsg = (char *)_errmsg + (gotobase ? strlen("[GOTO_BASE]") : 0);
		CDebugDlg::pAstSig = NULL;
		// cleanup_nodes was called with CAstException
		if (strncmp(errmsg, "Invalid", strlen("Invalid")))
			cout << "ERROR: " << errmsg << endl;	 
		else
			cout << errmsg << endl;
		//Going back to the base scope only during the debugging (F10, F5,... etc)
		if (gotobase)
			Back2BaseScope(0);
		//		succ = false; // Shouldn't be set here. Must be a leftover fromthe past. 10/11/2018
	}
	catch (CAstSig *ast) 
	{ // this was thrown by aux_HOOK
		if (ast->u.debug.status == aborting)
		{
			CDebugDlg::pAstSig = NULL;
			CAstSig::cleanup_nodes(ast);
			Back2BaseScope(0);
		}
		else
		{
		try {
			string HookName;
			char buf[2048];
			vector<string> tar;
			input = ast->GetScript();
			nItems = str2vect(tar, input.c_str(), " ");
			if (nItems==0) return 0;
			if (tar[0]=="#")
			{
				if (nItems==1) return 0;
				HookName = tar[1];
			}
			else
				HookName = tar[0].substr(1);
			if (nItems==1)
				buf[0]=0;
			else
				strcpy(buf, input.substr(input.find(tar[1])).c_str());
			succ = false;
		if (hook(ast, HookName, buf)==-1)	
				return -1;	
		}
		catch (const char *errmsg)				{
			//		succ = false; // Shouldn't be set here. Must be a leftover fromthe past. 10/11/2018
			cout << "ERROR:" << errmsg << endl;	 }
		}
	}
	ShowWS_CommandPrompt(pContext, succ);
	return pTemp ? 1:0;
}

void xcom::setfs(CAstSig *past, int newfs)
{
	if (past->pEnv->Fs == newfs) return;
	past->pEnv->Fs = newfs; //Sample rate adjusted
	//variables updated
	CSignals ratio(1);
	for (auto &it : past->Vars)
	{
		if (it.second.GetType() == CSIG_AUDIO)
		{
			CSignals level = it.second.RMS();
			ratio.SetValue((double)newfs / it.second.GetFs());
			it.second.basic(it.second.pf_basic2 = &CSignal::resample, &ratio);
			if (ratio.IsString()) // this means there was an error during resample
				MessageBox(mShowDlg.hDlg, ratio.string().c_str(), "Error in setfs", 0);
			it.second.SetFs(newfs);
			CSignals level2 = it.second.RMS();
			it.second *= level2.value() / level.value();
		}
	}
}

int xcom::SAVE_axl(CAstSig *past, const char* filename, vector<string> varlist, char *errstr)
{
	map<string, CVar>::iterator it;
	vector<string>::iterator it2;
	string not_saved_vars;
	int savedCount(0);
	if (varlist.empty())
		it = past->Vars.begin(); 
	else
	{
		it2 = varlist.begin();
		it = past->Vars.find(*it2);
		if (it == past->Vars.end())
		{
			sprintf(errstr, "variable \"%s\" not available.", (*it2).c_str());
			return 0;
		}
	}
	FILE *fp;
	if (fp = fopen(filename, "wb"))
	{
		for (; it != past->Vars.end();)
		{
			if (write_axl_block(fp, it->first, &it->second, errstr) > 0)
				savedCount++;
			else
			{
				not_saved_vars += it->first;
				not_saved_vars += ' ';
			}
			if (varlist.empty())
				it++;
			else
			{
				it2++;
				if (it2 == varlist.end())	break;
				it = past->Vars.find(*it2);
			}
			if (it == past->Vars.end())		break;
		}
	}
	else
		sprintf(errstr, "%s cannot be open for writing.", filename);
	fclose(fp);
	if (!not_saved_vars.empty())
		sprintf(errstr, "Following vars not saved: %s", not_saved_vars.c_str());
	return savedCount;
}

int xcom::hook(CAstSig *ast, string HookName, const char* argstr)
{
	HMODULE hLib ;
	string emsg;
	char errstr[1024], buf[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH], filename[MAX_PATH], ext[MAX_PATH], buffer[MAX_PATH];
	string name, fname;
	size_t k(0), nItems;
	vector<string> varlist, tar;
	if (HookName == "hist")
	{
		ShowWindow(mHistDlg.hDlg, SW_SHOW);
	}
	else if (HookName=="quit")
	{
		return -1;
	} 
	else if (HookName == "setfs")
	{
		if (!argstr[0]) return 0;
		if (str2vect(tar, argstr, " ")>1)
			throw "only one argument allowed";
		if (!ast->SetNewScript(emsg, argstr))
			throw emsg.c_str();
		ast->Compute();
		if (!ast->Sig.IsScalar()) throw "only scalar argument allowed";
		double val = ast->Sig.value();
		if (val<500.) throw "Sampling rate must be greater than 500 Hz.";
		setfs(ast, (int)val);
		mShowDlg.pcast = ast;
		mShowDlg.Fillup();
		ast->FsFixed = true;
	}
	else if (HookName=="load")
	{
		tar = qlparse(argstr, emsg);
		if (tar.size() != 1)
		{
			if (tar.empty())
				sprintf(buf, "Parsing error in qlparse.dll (%s)", emsg.c_str());
			else
				sprintf(buf, "%s requires one argument. %d arguments in %s\n(Did you forget double quotations?)", HookName.c_str(), tar.size(), argstr);
			throw buf;
		}
		FILE *fp = ast->OpenFileInPath(tar[0], "axl", name);
		if (fp)
		{
			if (load_axl(fp, errstr)==0) 
				printf("File %s reading error----%s.\n", argstr, errstr);
			fclose(fp);
			mShowDlg.Fillup(&ast->Vars);
		}
		else
		{
			strcpy(errstr, argstr); strcat(errstr,".axl");
			printf("File %s not found in %s\n", errstr, ast->pEnv->AuxPath.c_str()); 
		}
	}
	else if (HookName=="save")
	{
		string savesuccess("");
		tar = qlparse(argstr, emsg);
		if (tar.size() == 0)
		{
			if (tar.empty())
				sprintf(buf, "Parsing error in qlparse.dll (%s)", emsg.c_str());
			else
				sprintf(buf, "%s requires one argument. %d arguments in %s\n(Did you forget double quotations?)", HookName.c_str(), tar.size(), argstr);
			throw buf;
		}
		// tar[0] must be the file name, the rest are for the variable to save.
		_splitpath(tar[0].c_str(), drive, dir, filename, ext);
		if (strlen(drive) + strlen(dir) == 0)
			FulfillFile(buffer, AppPath, filename), fname = buffer;
		else
			fname = tar[0];
		if (strlen(ext) == 0) fname += ".axl";
		else fname += ext;

		tar.erase(tar.begin());
		if (SAVE_axl(ast, fname.c_str(), tar, errstr)>0)
			printf("Workspace variables saved in %s\n", fname.c_str());
		else
			printf("#save failed---%s\n", errstr);
	}
	else if (HookName == "module")
	{
		tar = qlparse(argstr, emsg);
		if (tar.size() != 1)
		{
			if (tar.empty())
				sprintf(buf, "Parsing error in qlparse.dll (%s)", emsg.c_str());
			else
				sprintf(buf, "%s requires one argument. %d arguments in %s\n(Did you forget double quotations?)", HookName.c_str(), tar.size(), argstr);
			throw buf;
		}
		FILE *fp = ast->OpenFileInPath(tar[0], "dll", emsg);
		if (fp)
		{
			fclose(fp);
			vector<string>::iterator it = find(LoadedModule.begin(), LoadedModule.end(), argstr);
			if (it == LoadedModule.end())
			{
				hLib = LoadLibrary(argstr);
				if (!hLib)
				{
					GetLastErrorStr(errstr);
					emsg = errstr;
					trimr(emsg, "\r\n");
					sprintf(buf, "LoadLibrary error for %s: %s", tar[0].c_str(), emsg.c_str());
					throw buf;
				}
				//All auxlab module (such as auxcon) must have the following gateway function void Open(vector<CAstSig*> *pvecast)
				PFUN pf = (PFUN)GetProcAddress((HMODULE)hLib, (LPCSTR)"?Open@@YAXABV?$vector@PAVCAstSig@@V?$allocator@PAVCAstSig@@@std@@@std@@@Z");
				if (!pf)
				{
					GetLastErrorStr(errstr);
					emsg = errstr;
					trimr(emsg, "\r\n");
					sprintf(buf, "Error accessing for %s module initiation function: %s", tar[0].c_str(), emsg.c_str());
					throw buf;
				}
				pf(CAstSig::vecast); // subclassing so that in the future all the message is routed to ProcessMsg() in the dll.
				LoadedModule.push_back(argstr);
				if (hEventModule) CloseHandle(hEventModule);
				hEventModule = NULL;
				hEventModule = CreateEvent((LPSECURITY_ATTRIBUTES)NULL, 0, 0, "AUXCONScriptEvent");
				WaitForSingleObject(hEventModule, INFINITE);
			}
			else
			{
				sprintf(buf, "Module %s already opened", argstr);
				throw buf;
			}
		}
		else
		{
			if (emsg.empty())
				printf("File %s not found\n", argstr);
			else
				printf("File %s not found in %s\n", argstr, emsg.c_str());
		}
//		h = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON2), IMAGE_ICON, 0, 0, 0);
	}
	else if (HookName == "fs")
	{
		printf("Sampling Rate=%d\n", ast->pEnv->Fs);
	}
	else if (HookName=="clear")
	{
		string savesuccess("");
		nItems = str2vect(tar, argstr, " ");
		int success(0);
		if (nItems>0) savesuccess = "Variable(s) cleared: ";
		for (; k<nItems; k++)
			if (ast->ClearVar(tar[k].c_str()))
				success++, sprintf(buf, "%s ", tar[k].c_str()), savesuccess+= buf;
		if (success) printf("%s\n", savesuccess.c_str()), mShowDlg.Fillup(&ast->Vars);
	}
	else
	{
		//if Hookname is one of the aux-reserves, return -1 so that it continues to parsing 
		nItems = str2vect(tar, HookName.c_str(), ".");
		vector<string>::iterator it = find(aux_reserves.begin(), aux_reserves.end(), HookName);
		if (it != aux_reserves.end()) 	return -1;
		sprintf(errstr, "Undefined HOOK name: %s", HookName.c_str());
		throw errstr;
	}
	return 0;
}

bool dbmapfind(const char* udfname)
{
	unordered_map<string, CDebugDlg*>::iterator it;
	return (it = dbmap.find(udfname))!=dbmap.end(); 
}

void xcom::LogHistory(vector<string> input)
{
	FILE* logFP = fopen(mHistDlg.logfilename,"at"); 
	if (logFP) 
	{
		for (size_t k=0; k<input.size(); k++)
		{
			if (input[k].size()>0)
				fprintf(logFP, "%s\n", input[k].c_str()); 
		}
		fclose(logFP); 
	}
	else
	{
		char temp[256]; 
		sprintf(temp, "fopen error: %s", mHistDlg.logfilename); 
		::MessageBox(mHistDlg.hDlg, temp, "LOGHISTORY", 0); 
	}
}

void AuxconGetInputThread(void *var)
{
	char buf[256] = { 0 };
	CAstSig *pastsig = (CAstSig *)var;
	printf("\n");
	mainSpace.ShowWS_CommandPrompt(pastsig);
	FlushConsoleInputBuffer(hStdin); // with this, any keystroke in the console made before OpDlg was created is deleted.
	CAstSig tempAstSig(pastsig);
	while (moduleLoop)
	{
		mainSpace.getinput(buf);
		mainSpace.computeandshow(buf, mShowDlg.pcast);
	}
	hAuxconThread = NULL;
}

void ShowVariables(CAstSig *pastsig)
{
	CAstSig *pbase = CAstSig::vecast.front();
	if (pastsig->dad == pbase && CAstSig::vecast.size() == 1)
	{
		moduleLoop = true;
		CAstSig::vecast.push_back(pastsig);
		if (!hAuxconThread)
			hAuxconThread = _beginthread(AuxconGetInputThread, 0, (void*)pastsig);
	}
}

void UnloadModule(const char *modulename)
{
	vector<string>::iterator it = find(mainSpace.LoadedModule.begin(), mainSpace.LoadedModule.end(), modulename);
	if (it != mainSpace.LoadedModule.end())
		mainSpace.LoadedModule.erase(it);
}

void Back2BaseScope(int closeauxcon)
{
	while (mShowDlg.SendDlgItemMessage(IDC_DEBUGSCOPE, CB_DELETESTRING, 1) != CB_ERR) {}
	mShowDlg.SendDlgItemMessage(IDC_DEBUGSCOPE, CB_SETCURSEL, 0);
	mShowDlg.debug(exiting, NULL, -1);
	mShowDlg.pVars = &CAstSig::vecast.front()->Vars;
	mShowDlg.pGOvars = &CAstSig::vecast.front()->GOvars;
	mShowDlg.Fillup();
	if (CAstSig::vecast.size() > 1)
	{
		CAstSig::vecast.pop_back();
		FlushConsoleInputBuffer(hStdin);
	}
	if (closeauxcon)
	{
		if (hAuxconThread)
		{
			moduleLoop = false;
			//Emulating "enter" keystroke
			memset(mainSpace.debug_command_frame, 0, sizeof(mainSpace.debug_command_frame));
			for (int k = 0; k < 2; k++)
			{
				mainSpace.debug_command_frame[k].EventType = KEY_EVENT;
				mainSpace.debug_command_frame[k].Event.KeyEvent.uChar.AsciiChar = 0;
				mainSpace.debug_command_frame[k].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
			}
			DWORD dw;
			int res;
			mainSpace.debug_command_frame[0].Event.KeyEvent.bKeyDown = 1;
			mainSpace.debug_command_frame[1].Event.KeyEvent.bKeyDown = 0;
			res = WriteConsoleInput(hStdin, mainSpace.debug_command_frame, 2, &dw);
			if (hEventLastKeyStroke2Base) CloseHandle(hEventLastKeyStroke2Base);
			hEventLastKeyStroke2Base = NULL;
			hEventLastKeyStroke2Base = CreateEvent((LPSECURITY_ATTRIBUTES)NULL, 0, 0, "");
			WaitForSingleObject(hEventLastKeyStroke2Base, INFINITE);
		}
		SetEvent(hEventModule);
	}
}

void HoldAtBreakPoint(CAstSig *pastsig, const AstNode *pnode)
{
	char buf[256];
	bool loop(true);
	vector<string> tar;
	size_t num(0);
	string fname;
	mainSpace.ShowWS_CommandPrompt(pastsig);
	if (!pastsig->u.debug.inPurgatory)
		debug_appl_manager(pastsig, stepping, pastsig->u.currentLine = pnode->line);
	AstNode *p=pastsig->pAst;
	while (p)
	{
		buf[0] = 0;
		DEBUG_KEY ret = mainSpace.getinput(buf);
		switch (ret)
		{
		case debug_F10:
//			if (pastsig->pLast->type == T_IF || pastsig->pLast->type == T_FOR || pastsig->pLast->type == T_WHILE || pnode->next)
				pastsig->u.debug.status = stepping;
			return;
		case debug_F11:
			pastsig->u.debug.status = stepping_in;
			return;
		case debug_Shift_F5:
			pastsig->u.debug.status = aborting;
			mainSpace.need2validate = true;
			ValidateFig(CAstSig::vecast.front()->u.title.c_str());
			throw pastsig;
		case debug_F5:
			pastsig->u.debug.status = continuing;
			fname = pastsig->u.base;
			for (auto ln : pastsig->pEnv->udf[fname].DebugBreaks)
			{
				if (pastsig->u.currentLine >= ln)
				{
					pastsig->u.nextBreakPoint = 0xffff; 
					return;
				}
				pastsig->u.nextBreakPoint = ln;
			}
			return;
		case debug_Ctrl_F10:
			pastsig->u.debug.status = continuing;
			if (pastsig->pEnv->curLine < pastsig->u.nextBreakPoint)
				pastsig->u.nextBreakPoint = pastsig->pEnv->curLine + 1;
			return;
		default: //This is where user-typed lines are processed during debugging.
			//debug.status should be set to null		
			pastsig->u.debug.status = typed_line;
			mainSpace.computeandshow(buf, pastsig);
			break;
		}
	}
	return ;
}

void debug_appl_manager(CAstSig *debugAstSig, DEBUG_STATUS debug_status, int line)
{
	mShowDlg.debug(debug_status, debugAstSig, line);
}

bool xcom::IsNowDebugging(CAstSig *pcast)
{// Check if it is running on debugging mode
 // Assumption: pcast must be a part of vecast vector
 // if pcast is not the base instance, which isn't necessarily CAstSig::vecast.front()--because module such as auxcon may put its own as the front point right behind xcom front
	if (!strcmp(pcast->u.application, "xcom"))
		return (CAstSig::vecast.size() > 1);
	else
		return (CAstSig::vecast.size() > 2);
	return false;
}

void ValidateFig(const char* scope)
{
	if (!strlen(scope)) scope = "base workspace";
	// When a UDF enters debugging, or exits from it, all figure windows are scanned and validated with scope
	for (auto dlg: plots)
	{
		if (dlg->var.find("Figure ") == 0)
			continue;
		HANDLE fig = FindFigure(dlg->hDlg);
		CFigure *cfig = static_cast<CFigure *>(fig);
		cfig->inScope = dlg->scope == scope;
		cfig->m_dlg->InvalidateRect(NULL);
	}
}

void xcom::ShowWS_CommandPrompt(CAstSig *pcast, bool success)
{
	if (success && pcast && pcast->pAst)
	{
		AstNode *p= pcast->pAst;
		if (!pcast->u.base.empty()) // if this is for udf
		{
			//Also, do take care of display vec windows during debugging when debugging is done... what to do...
			int llnn;
			// if it is stepping, it's based on the current line.
			// if it is continuing, it should be nextbreakpoint minus one
			if (pcast->u.debug.status==continuing) llnn = pcast->u.nextBreakPoint - 1;
			else if (pcast->u.debug.status == stepping) llnn = pcast->u.currentLine;
			else
			{// do this later. 10/21/2018
				llnn = 2;
			}
			p = CAstSig::goto_line(pcast->pAst, llnn);
		}
		CWndDlg * dlg;
		if (p && (dlg = Find_cellviewdlg(p->str)))
		{
			if (dlg->dwUser == 8383)
			{
				((CVectorsheetDlg*)dlg)->ResetDraw(pcast->Sig.nGroups, pcast->Sig.Len());
				((CVectorsheetDlg*)dlg)->FillUp();
			}
			else if (dlg->dwUser == 1010)
			{
				((CShowvarDlg*)dlg)->Fillup(); // call with appropriate arguments.... then the auto-update might work
				string rootname(pcast->pAst->str);
				rootname += '.';
				for (vector<CWndDlg*>::iterator it = cellviewdlg.begin(); it != cellviewdlg.end(); it++)
				{
					//if this object's name begins with 
					string ss = ((CShowvarDlg*)*it)->name;
					if (((CShowvarDlg*)*it)->name.find(rootname) == 0 && ((CShowvarDlg*)*it)->name.size() != rootname.size())
					{
						if ((*it)->dwUser == 8383) // check this.
							((CVectorsheetDlg*)*it)->FillUp();
						else
							((CShowvarDlg*)*it)->Fillup();
					}
				}
			}
		}
		//		mShowDlg.pcast = pcast;
		mShowDlg.pVars = &pcast->Vars;
		mShowDlg.pGOvars = &pcast->GOvars;
		mShowDlg.Fillup();
		//if during debugging, redraw all figure windows with the varname in the debugging scope
		if (!pcast->u.title.empty()) // For debugging or N_BLOCK
			mShowDlg.OnVarChanged();
		else if (pcast->pAst->type == N_BLOCK)
		{
			mShowDlg.OnVarChanged();
		}
		else //if (pcast->u.title.empty()) // For non-debugging
		{
			AstNode *p = (pcast->pAst->type==N_BLOCK) ? pcast->pAst->next : pcast->pAst;
			for (; p; p = p->next)
				if (!CAstSig::Var_never_updated(p))
				{
					const char *pt;
					if (CAstSig::IsTID(p))
					{
						pt = p->str;
						if (!pt) continue; // for multiple output, pt is NULL and p->alt->type is N_VECTOR
					}
					else if (CAstSig::IsSTRUCT(p))
						pt = pcast->statusMsg.c_str();
					else
						pt = p->str ? p->str : "ans";
					mShowDlg.OnVarChanged(pt);
				}
		}
		if (pcast->statusMsg.length() > 0 && pcast->statusMsg.substr(0, 6) == "(NOTE)")
		{
			cout << pcast->statusMsg.c_str() << endl;
			pcast->statusMsg.clear();
		}
	}
	if (IsNowDebugging(pcast))
	{
		mainSpace.comPrompt = DEBUG_PROMPT;
		printf(mainSpace.comPrompt.c_str());
	}
	else
	{
		string line;
		CONSOLE_SCREEN_BUFFER_INFO coninfo;
		GetConsoleScreenBufferInfo(hStdout, &coninfo);
		size_t res = ReadThisLine(line, hStdout, coninfo, coninfo.dwCursorPosition.Y, 0);
		if (res>0) printf("\n");
		mainSpace.comPrompt = MAIN_PROMPT;
		printf(mainSpace.comPrompt.c_str());
	}
}

size_t xcom::ReadHist()
{
	DWORD dw;
	HANDLE hFile = CreateFile(mHistDlg.logfilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile==INVALID_HANDLE_VALUE) return 0;
	LARGE_INTEGER fsize;
	if (!GetFileSizeEx(hFile, &fsize)) 
	{ CloseHandle(hFile); MessageBox(mHistDlg.hDlg, "GetFileSizeEx error", "ReadHist()", 0);  return 0;	}

	__int64 size = fsize.QuadPart;
	int sizelimit = 0xffff; //65535;
	if (fsize.HighPart>0)
	{
		LONG high(-fsize.HighPart);
		dw = SetFilePointer (hFile, sizelimit, &high, FILE_END);
	}
	else
		dw = SetFilePointer (hFile, -sizelimit, NULL, FILE_END);
	char *buffer = new char[sizelimit+1];
	BOOL res = ReadFile(hFile, buffer, sizelimit, &dw, NULL);
	buffer[dw]=0;

	string str(buffer), tempstr;
	size_t pos, pos0 = str.find_last_of("\r\n");
	size_t count(0);
	vector<string> _history;
	while (count<nHistFromFile)
	{
		pos = str.find_last_of("\r\n", pos0-2);
		if (pos == string::npos) break;
		tempstr = str.substr(pos+1, pos0-pos-2);
		trim(tempstr, " ");
		if (!tempstr.empty())
		{
			if (tempstr[0]!='/' || tempstr[1]!='/')
				count++, _history.push_back(tempstr);
		}
		pos0 = pos;
	}
	delete[] buffer;
	CloseHandle(hFile); 
	history.reserve(nHistFromFile*10);
	for (vector<string>::reverse_iterator rit=_history.rbegin(); rit!=_history.rend(); rit++)
		history.push_back(*rit);
	return count;
}


LRESULT CALLBACK sysmenuproc (int code, WPARAM wParam, LPARAM lParam)
{
	CWPSTRUCT *param = (CWPSTRUCT*)lParam;
	if (param->message==WM_MENUCOMMAND || param->message==WM_SYSCOMMAND)
	{
		int res=0;
	}
	return CallNextHookEx(NULL, code, wParam, lParam);
}

LRESULT CALLBACK proc (HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	switch (umsg)
	{
	case WM_CREATE:
		return 1;
	case WM_COMMAND:
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc( hwnd, umsg, wParam, lParam );
}


LRESULT CALLBACK wireKeyboardProc(int code, WPARAM wParam,LPARAM lParam) {  
    if (code < 0) {
    	return CallNextHookEx(0, code, wParam, lParam);
    }
    Beep(1000, 20);
    return CallNextHookEx(0, code, wParam, lParam);
}

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	return CallNextHookEx(NULL, code, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	char buf[256];
	int res;
	string addp, emsg;
	vector<string> tar;
	int fs;
	char fullmoduleName[MAX_PATH], moduleName[MAX_PATH];
	char drive[16], dir[256], ext[8], fname[MAX_PATH], buffer[MAX_PATH];

	//	 _set_output_format(_TWO_DIGIT_EXPONENT);

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwICC = ICC_LISTVIEW_CLASSES | ICC_LINK_CLASS; // ICC_LINK_CLASS will not work without common control 6.0 which I opted not to use
	InitCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	BOOL bRet = InitCommonControlsEx(&InitCtrls);
	cellviewdlg.push_back(&mShowDlg);

	AllocConsole();
	AttachConsole(ATTACH_PARENT_PROCESS);
	SetConsoleCP(437);
	SetConsoleOutputCP(437);

	HWND hr = GetConsoleWindow();
	HMENU hMenu = GetSystemMenu(hr, FALSE);
	AppendMenu(hMenu, MF_SEPARATOR, 0, "");
	AppendMenu(hMenu, MF_STRING, 1010, "F1 does not work here. Use it in \"Settings & Variables\"");

	HMODULE h = HMODULE_THIS;
	GetModuleFileName(h, fullmoduleName, MAX_PATH);
	_splitpath(fullmoduleName, drive, dir, fname, ext);
	sprintf(mainSpace.AppPath, "%s%s", drive, dir);
	sprintf(moduleName, "%s%s", fname, ext);
	getVersionString(fullmoduleName, mainSpace.AppVersion, sizeof(mainSpace.AppVersion));
#ifndef WIN64
	sprintf(buf, "AUXLAB %s [AUdio syntaX Console]", mainSpace.AppVersion);
#else
	sprintf(buf, "AUXLAB %s (x64) [AUdio syntaX Console]", mainSpace.AppVersion);
#endif
	SetConsoleTitle(buf);

	RECT rt;
	GetWindowRect(hr, &rt);
	DWORD dw = sizeof(buf);
	GetComputerName(buf, &dw);
	sprintf(iniFile, "%s%s_%s.ini", mainSpace.AppPath, fname, buf);
	double block;
	res = readINIs(iniFile, buf, fs, block, udfpath);
	CAstSigEnv *pglobalEnv = new CAstSigEnv(fs);
	pglobalEnv->InitBuiltInFunctionList();
	CAstSig cast(pglobalEnv);
	if (block > 0) cast.audio_block_ms = block;

	//	cast.Reset(fs,""); //	mainSpace.cast.Sig.Reset(fs); is wrong...
	addp = mainSpace.AppPath;
	if (strlen(udfpath) > 0 && udfpath[0] != ';') addp += ';';
	addp += udfpath;
	pglobalEnv->SetPath(addp.c_str());
	pglobalEnv->AppPath = mainSpace.AppPath;

	if ((hShowvarThread = _beginthreadex(NULL, 0, showvarThread, NULL, 0, NULL)) == -1)
		::MessageBox(hr, "Showvar Thread Creation Failed.", "AUXLAB mainSpace", 0);
	if ((hHistoryThread = _beginthreadex(NULL, 0, histThread, (void*)mainSpace.AppPath, 0, NULL)) == -1)
		::MessageBox(hr, "History Thread Creation Failed.", "AUXLAB mainSpace", 0);

	freopen("CON", "w", stdout);
	freopen("CON", "r", stdin);

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD fdwMode = ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_WINDOW_INPUT | ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS;
	//    DWORD fdwMode = ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_WINDOW_INPUT | ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS | ENABLE_QUICK_EDIT_MODE; 
	SetConsoleMode(hStdin, fdwMode);
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE)) ::MessageBox(hr, "Console control handler error", "AUXLAB", MB_OK);

	fdwMode = ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;
	res = SetConsoleMode(hStdout, fdwMode);

	dw = sizeof(buf);
	GetComputerName(buf, &dw);
	sprintf(mHistDlg.logfilename, "%s%s%s_%s.log", mainSpace.AppPath, fname, HISTORY_FILENAME, buf);
	size_t nHistFromFile = mainSpace.ReadHist();
	mainSpace.comid = nHistFromFile;

#ifndef WIN64
	sprintf(fname, "%sauxp32.dll", mainSpace.AppPath);
#else
	sprintf(fname, "%sauxp64.dll", mainSpace.AppPath);
#endif
	HANDLE hLib = LoadLibrary(fname); // fix this.... if the path has been changed in the middle, we are no longer in AppPath
	if (!hLib)
		printf("[Warning] Private UDF library module %s not found\n", fname);
	else
	{
		string res;
		int id = 100; // the resource ID in auxp begins with 101 (harded-coded)
		while (1)
		{
			res = cast.LoadPrivateUDF((HMODULE)hLib, ++id, emsg);
			if (res.empty())
				break;
		}
		FreeLibrary((HMODULE)hLib);
	}
	WriteConsole(hStdout, mainSpace.comPrompt.c_str(), (DWORD)mainSpace.comPrompt.size(), &dw, NULL);

	while (mShowDlg.hDlg == NULL) {
		Sleep(100);
		mShowDlg.pcast = &cast;
		mShowDlg.pcastCreated = false;
		mShowDlg.name = "#base";
		mShowDlg.Fillup();
		mShowDlg.changed = false;
	}
	mShowDlg.pcast = &cast; // to be deleted...
	mShowDlg.pVars = &cast.Vars; // new 
	mShowDlg.pGOvars = &cast.GOvars; // new 
	ShowWindow(mShowDlg.hDlg, SW_SHOW);
	ShowWindow(mHistDlg.hDlg, SW_SHOW);

	CRect rt1, rt2, rt3, rt4;
	res = readINI_pos(iniFile, &rt1, &rt2, &rt3, &rt4);
	if (res & 1)
		MoveWindow(hr, rt1.left, rt1.top, rt1.Width(), rt1.Height(), TRUE);

	CAstSig::vecast.push_back(&cast);
	cast.u.application = "xcom";
//	mainSpace.RunTest("d:\\temp\\auxlabtest.txt", "", "d:\\temp\\reffile.txt");
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	sprintf(buffer, "//\t[%02d/%02d/%4d, %02d:%02d:%02d] AUXLAB %s begins------", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond, mainSpace.AppVersion);
	vector<string> in;
	in.push_back(buffer);
	mainSpace.LogHistory(in);

	cast.astsig_init(&debug_appl_manager, &HoldAtBreakPoint, &dbmapfind, &ShowVariables, &Back2BaseScope, &ValidateFig, &SetGOProperties, &RepaintGO);
	init_aux_reserves();

	CONSOLE_CURSOR_INFO concurinf;

	concurinf.bVisible = 1;
	concurinf.dwSize = 25;
	SetConsoleCursorInfo(hStdout, &concurinf);

	mainSpace.console();

	closeXcom(mainSpace.AppPath);
	return 1;
}

#ifdef _DEBUG
#include <fstream>

int xcom::RunTestType(string &line)
{
	//get thd first word
	size_t pos = line.find(' ');
	string cmd = line.substr(0, pos);
	line = line.substr(cmd.size());
	if (cmd == "vector")		return CSIG_VECTOR;
	if (cmd == "matrix")		return CSIG_MATRIX;
	if (cmd == "audio")		return CSIG_AUDIO;
	if (cmd == "cell")		return CSIG_CELL;
	if (cmd == "class")		return CSIG_STRUCT;
	if (cmd == "scalar")		return CSIG_SCALAR;
	if (cmd == "string")		return CSIG_STRING;
	if (cmd == "tseq")		return CSIG_TSERIES;
	return -1;
}

int xcom::RunTestCountElement(int type, string &rest, int &col)
{
	istringstream iss(rest);
	col = -1;
	int m(-1);
	switch (type)
	{
	case CSIG_SCALAR:
		return 0;
	case CSIG_VECTOR:
	case CSIG_AUDIO:
	case CSIG_CELL:
		iss >> m;
		return m;
	case CSIG_MATRIX:
		iss >> m >> col;
		return m;
	default:
		return -1;
	}
}
int xcom::RunTestCheckElements(const CVar &generated, const string &expected)
{ // only for CSIG_SCALAR, CSIG_VECTOR, CSIG_MATRIX
	istringstream iss(expected);
	double val;
	for (unsigned int k = 0; k < generated.nSamples; k++)
	{
		iss >> val;
		if (fabs(val - generated.buf[k]) > 1.e-3) return 0;
	}
	return 1;
}
int xcom::RunTest(const char *infile, const char *intended_result_file, const char *reportfile)
{
	ifstream file1, filer;
	ofstream file2;
	ostringstream oss;
	string cmd, ref, out;
	file1.open(infile);
	filer.open(reportfile);
	size_t pos;
	char errstr[256];
	int lineID = 1;
	CAstSig *past = CAstSig::vecast.front();
	int type, nsamples, ncols;
	try {
		while (getline(file1, cmd))
		{
			computeandshow(cmd.c_str());
			while (1)
			{
				getline(filer, ref);
				if (ref.find("//") == string::npos) break;
			}
			type = RunTestType(ref);
			if (type != past->Sig.GetType())
			{
				oss << "Unexpected type error in line " << lineID;
				throw oss.str().c_str();
			}
			if (type == CSIG_SCALAR || type == CSIG_VECTOR || type == CSIG_MATRIX || type == CSIG_CELL)
			{
				nsamples = RunTestCountElement(type, ref, ncols);
				if (nsamples != past->Sig.nSamples)
				{
					oss << "nSamples not the same in line " << lineID;
					throw oss.str().c_str();
				}
			}
			if (type == CSIG_SCALAR || type == CSIG_VECTOR || type == CSIG_MATRIX)
			{
				const string rest = ref.substr(ref.find('\t'));
				pos = RunTestCheckElements(past->Sig, rest);
			}
			else if (type == CSIG_AUDIO)
			{
				past->Sig.PlayArray(errstr);
				CSignals tp;
				if (!tp.Wavread(ref.c_str(), errstr))
				{
					oss << "audio file" << ref << " not found line " << lineID;
					throw oss.str().c_str();
				}
				tp.PlayArray(errstr);
			}
		}
	}
	catch (const char *emsg)
	{
		printf("%s\n", emsg);
	}
	file1.close();
	filer.close();
	return 1;
}
#endif
