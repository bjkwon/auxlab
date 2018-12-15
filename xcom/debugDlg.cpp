// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: auxlab
// Main Application. Based on Windows API  
// 
// 
// Version: 1.4951
// Date: 12/14/2018
// 
#include "wxx_wincore.h" // Win32++ 8.2. This must be placed prior to <windows.h> to avoid including winsock.h
#include "debugDlg.h"
#include "resource1.h"
#include "audfret.h"
#include "showvar.h"
#include "audstr.h"
#include "Objbase.h"
#include "Shobjidl.h"

#ifndef SIGPROC
#include "sigproc.h"
#endif

#include "xcom.h"
#include "consts.h"
#include "TabCtrl.h"

extern xcom mainSpace;
extern HANDLE hStdin, hStdout; 
CFileDlg fileDlg2;
extern CShowvarDlg mShowDlg;
extern uintptr_t hDebugThread2;
extern char iniFile[256];

CTabControl mTab;

CDebugBaseDlg debugBase;
vector<CDebugDlg *> debugVct; // Debug dialogboxes, which appear on the tab control

unordered_map<string, CDebugDlg*> dbmap;
char fullfname[_MAX_PATH], fname[_MAX_FNAME + _MAX_EXT];

int spyWM(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam, char* const fname, vector<UINT> msg2excl, char* const tagstr);

BOOL CALLBACK DebugBaseProc (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK TabPage_DlgProc(HWND hwndDlg, UINT umsg, WPARAM wParam, LPARAM lParam);

#define WM__NEWDEBUGDLG	WM_APP+0x2000
#define WM__UPDATE_UDF_CONTENT	WM_APP+0x2001

//extern vector<UINT> exc; //temp

string cleanupstring(string in)
{
	in.erase(std::remove_if(in.begin(), in.end(),
		[](char c) { return (c >0x7e || c<0x21); }), in.end());
	return in;
}


int getID4hDlg(HWND hDlg)
{
	size_t k(0);
	for (vector<CDebugDlg*>::iterator it=debugVct.begin(); it!=debugVct.end(); it++, k++) 
	{ if (hDlg==(*it)->hDlg)   return (int)k; }
	return -1;
}

#define m_THIS  debugVct[id]

extern vector<UINT> exc;

BOOL CALLBACK debugDlgProc (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	if (exc.size()==0)
	{
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
		exc.push_back(WM_NCMOUSEMOVE);
		exc.push_back(WM_ERASEBKGND);
	}
	
	CDebugDlg *p_db(NULL); 
	int line(-1);
	for (unordered_map<string, CDebugDlg*>::iterator it = dbmap.begin(); it!=dbmap.end(); it++)
		if (hDlg == it->second->hDlg) p_db = it->second;
	if (!p_db && umsg!=WM_INITDIALOG) return FALSE;

	if (umsg==WM_INITDIALOG)
	{
		((CREATE_CDebugDlg*)lParam)->dbDlg->hDlg = hDlg;
		return ((CREATE_CDebugDlg*)lParam)->dbDlg->OnInitDialog((HWND)wParam, lParam);
	}
	switch (umsg)
	{
	chHANDLE_DLGMSG (hDlg, WM_SIZE, p_db->OnSize);
	chHANDLE_DLGMSG (hDlg, WM_CLOSE, p_db->OnClose);
	chHANDLE_DLGMSG (hDlg, WM_DESTROY, p_db->OnDestroy);
	chHANDLE_DLGMSG (hDlg, WM_SHOWWINDOW, p_db->OnShowWindow);
	chHANDLE_DLGMSG (hDlg, WM_COMMAND, p_db->OnCommand);

	case WM_NOTIFY:
		p_db->OnNotify(hDlg, (int)(wParam), lParam);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


unsigned int WINAPI debugThread2 (PVOID var) 
{
	int res;
	CRect *prt = (CRect *)var;
	HWND hBase = CreateDialogParam (GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DEBUG2), mShowDlg.hDlg, (DLGPROC)DebugBaseProc, (LPARAM)&debugBase);
	if (prt)	res=debugBase.MoveWindow(*prt);

	MSG         msg ;
	HACCEL hAccDebug = LoadAccelerators (GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_ACCEL_DEBUG));
	while (GetMessage (&msg, NULL, 0, 0))
	{ 
		if (msg.message==WM__ENDTHREAD) 
			break;
		else if (msg.message==WM__NEWDEBUGDLG)
		{
			if (!msg.wParam)
			{
				fullfname[0] = fname[0] = 0;
				fileDlg2.FileOpenDlg(fullfname, fname, "AUX UDF file (*.AUX)\0*.aux\0", "aux");
		//		debugBase.FileOpen(fullfname);
			}
			else
				strcpy(fullfname, (char*)msg.wParam);
			if (strlen(fullfname)>0)
				debugBase.open_tab_with_UDF(fullfname, -1);
		}
		else
		{
			int res(0);
					if (msg.message==0x100)
					{res++; res--;}
			int iSel = TabCtrl_GetCurSel(debugBase.hTab);
			for (unordered_map<string, CDebugDlg*>::iterator it=dbmap.begin(); it!=dbmap.end(); it++)
			{
				if (it->second->hDlg == mTab.hCur)
				{	
					res = TranslateAccelerator (mTab.hCur, hAccDebug, &msg); 
					if (res)						
					{res++; res--;}
				
					break; }
			}
			if (!res && !IsDialogMessage(msg.hwnd, &msg))
			{
				TranslateMessage (&msg) ;
				DispatchMessage (&msg) ;
			}
		}
	}
	//cleanup -- this may not have enough time during exiting from closeXcom
	for (vector<CDebugDlg *>::iterator it=debugVct.begin(); it!=debugVct.end(); it++)
	{
		DeleteObject((*it)->eFont);
		DeleteObject((*it)->eFont2);
		(*it)->DestroyWindow();
		delete *it;
	}
	debugBase.hDlg = debugBase.hTab = NULL;
	debugBase.udfname.clear();
	debugBase.udf_fullpath.clear();
	debugBase.dlgPage_hwnd.clear();
	dbmap.clear();
	hDebugThread2 = NULL;
	return 0;
}


CDebugDlg::CDebugDlg(void)
{

}

CDebugDlg::~CDebugDlg(void)
{
//	dbmap.erase(udfname);
	vector<string> toDel;
	for (auto play : dbmap)
	{
		auto res = find(mTab.titleStr.begin(), mTab.titleStr.end(), play.first);
		if (res == mTab.titleStr.end()) toDel.push_back(play.first);
	}
}


/* For now (9/12/2017), debug window showing udf files is updated only IF
 the udf is displayed (either click the tab or keyboard input)
 OR
 the udf is called in the command window or called by another function during a udf call

 in CAstSig::SetNewScriptFromFile(), if the file content has been changed, 
 sigproc sendmessage's to showvarDlg with an lParam of fullfilame -->so ShowFileContent is called inside CShowvarDlg::OnDebug()
*/

int CDebugDlg::ShowFileContent(const char* fullfilename)
{
	vector<string> lines;
	size_t count(0);
	FILE *fp = fopen(fullfilename, "rt");
	if (fp!=NULL)
	{
		fseek (fp, 0, SEEK_END);
		int filesize=ftell(fp);
		fseek (fp, 0, SEEK_SET);
		char *buf = new char[filesize+1];
		size_t res = fread(buf, 1, (size_t)filesize, fp);
		buf[res]=0;
		res = fclose(fp);
		// str2vect is not used, because we need to take blank lines as they come
		char *lastpt=buf;
		char *pch=strchr(buf,'\n');
		while (pch)
		{
			*pch = '\0';
			lines.push_back(lastpt);
			lastpt = pch+1;
			pch=strchr(lastpt,'\n');
		}
		// if there's a remainder, take it.
		if (strlen(lastpt)>0) 
			lines.push_back(lastpt);
		delete[] buf;
		FillupContent(lines);
		return 1;
	}
	else
	{
		//Error handle
		return 0;
	}
}

BOOL CDebugDlg::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	CWndDlg::OnInitDialog(hwndFocus, lParam);

	char buf0[256], fname[256];
	CREATE_CDebugDlg *ptransfer = (CREATE_CDebugDlg *)lParam;
	strcpy(buf0, "[Debug] ");
	strcat(buf0, ptransfer->fullfilename);
	::SetWindowText(GetParent(hwndFocus), buf0);
	_splitpath(ptransfer->fullfilename, NULL, NULL, fname, buf0);
	dbmap[fname] = this;
	strcpy(fullUDFpath, ptransfer->fullfilename);
	string temp = fullUDFpath;
	transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
	strcpy(fullUDFpath, temp.c_str());

	udfname = fname;

	CDebugBaseDlg *mPar = (CDebugBaseDlg*)hParent;
	mPar->udfname[hDlg] = udfname;
	mPar->udf_fullpath[udfname] = fullUDFpath;
	mPar->dlgPage_hwnd[udfname] = hDlg;

//	ptransfer->dbDlg->hDlg = GetParent(hwndFocus); // not needed since tabcontrol is used.
	lvInit();

	LvItem.mask=LVIF_TEXT;   // Text Style
	LvItem.cchTextMax = 256; // Max size of text
	LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
	hList = GetDlgItem(IDC_LISTDEBUG);
	LRESULT res = ::SendMessage(hList,LVM_SETEXTENDEDLISTVIEWSTYLE,0, /*LVS_SHOWSELALWAYS | */ LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES);
	
	char buf[256];
	int width[]={50, -1, };
	for(int k = 0; k<2; k++)
	{
		LvCol.cx=width[k];
		LoadString(hInst, IDS_DEBUGBOX1+k, buf, sizeof(buf));
		LvCol.pszText=buf;
		res = SendDlgItemMessage(IDC_LISTDEBUG, LVM_INSERTCOLUMN,k,(LPARAM)&LvCol); 
	}
	ShowFileContent(fullUDFpath);

	fontsize = 10;
	HDC hdc = GetDC(NULL);
	lf.lfHeight = -MulDiv(fontsize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(NULL, hdc);
	strcpy(lf.lfFaceName, "Courier New");
	eFont = CreateFont(lf.lfHeight,0,0,0, FW_MEDIUM, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		FIXED_PITCH | FF_MODERN, lf.lfFaceName);
	eFont2 = CreateFont(lf.lfHeight,0,0,0, FW_BOLD, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		FIXED_PITCH | FF_MODERN, lf.lfFaceName);
	return TRUE;
}

void CDebugDlg::Debug(CAstSig *pastsig, DEBUG_STATUS type, int line)
{
	string fname, filecontent, filecontent2;
	int nLines;
	switch(type)
	{
	case refresh:
		if (pastsig->pEnv->udf[pastsig->u.base].newrecruit)
		{
			//file content changed. Refresh display
			ShowFileContent(pastsig->pEnv->udf[pastsig->u.base].fullname.c_str());
			//Recover debug breakpoints
			for (vector<int>::iterator it = pastsig->pEnv->udf[pastsig->u.base].DebugBreaks.begin(); it != pastsig->pEnv->udf[pastsig->u.base].DebugBreaks.end(); it++)
				ListView_SetCheckState(hList, *it-1, 1);
		}
		pastsig->pEnv->udf[pastsig->u.base].newrecruit = false;
		break;
	case entering:
	case progress:
		SetFocus(hList);
		break;
	case stepping:
		nLines = (int)::SendMessage(hList, LVM_GETITEMCOUNT, 0,0); 
		SetFocus(hList);
		break;
	case purgatory:
		CDebugDlg::pAstSig->u.debug.inPurgatory = true;
		break;
	case exiting:
		break;
	}
	::RedrawWindow(hDlg, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

void CDebugDlg::FillupContent(vector<string> in)
{
	SendDlgItemMessage(IDC_LISTDEBUG,LVM_DELETEALLITEMS,0,0);
	char buf[256];
	LRESULT res;
	size_t ss;
	for (LvItem.iItem=0; LvItem.iItem<(int)in.size(); LvItem.iItem++)
	{
		itoa(LvItem.iItem+1, buf, 10);
		LvItem.iSubItem=0; //First item (InsertITEM)
		LvItem.pszText=buf;
		::SendMessage(hList, LVM_INSERTITEM, 0, (LPARAM)&LvItem);

		LvItem.iSubItem++; //Second column (SetITEM)
		while ( (ss = in[LvItem.iItem].find('\t'))!=string::npos)
			in[LvItem.iItem].replace(ss,1,"  ");
		strcpy(buf,in[LvItem.iItem].c_str());
		res = ::SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem);
	}
//	res = ListView_Scroll(hList,0,ListView_GetItemCount(hList)*14);
	for (vector<int>::iterator it=breakpoint.begin(); it!=breakpoint.end(); it++)
		ListView_SetCheckState(hList, *it-1, 1);
}

void CDebugDlg::lvInit()
{
	LvItem.mask=LVIF_TEXT;   // Text Style
	LvItem.cchTextMax = 256; // Max size of text
	LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
	hList = GetDlgItem(IDC_LISTDEBUG);
	LRESULT res = ::SendMessage(hList,LVM_SETEXTENDEDLISTVIEWSTYLE,0, /*LVS_SHOWSELALWAYS | */ LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES);
	RECT rt;
//	char buf[256];
	::GetClientRect (hList, &rt);
//	LoadString (hInst, IDS_HELP_HISTORY, buf, sizeof(buf));
//	CreateTT(hInst, hList, rt, buf, 270);
}

void CDebugDlg::OnShowWindow(BOOL fShow, UINT status)
{
	int res(0);
	::ShowWindow(hList, fShow);
}

int CDebugDlg::SetSize()
{ // evoked by CDebugBaseDlg::OnSize
 	CRect rt;
	int res = ::GetClientRect (hParent->hDlg, &rt);
	int offset = 25;
	res = MoveWindow(0, offset, rt.right, rt.bottom-offset, 1); // Causing CDebugDlg::OnSize() to run
	return res;
}

void CDebugDlg::OnSize(UINT state, int cx, int cy)
{
	::MoveWindow(hList, 0, 0, cx, cy, 1);
	int res = ListView_SetColumnWidth(hList, 1, cx-53);
}

void CDebugDlg::OnClose()
{
	// Then bring it back ---ShowWindow(SW_SHOW); if it hits the breakpoint.... do this tomorrow 7/3/2017 bjk
}

void CDebugDlg::OnDestroy()
{
//	::SendMessage(GetParent(hDlg), WM_DEBUG_CLOSE, (WPARAM)udfname.c_str(), 0);
	//This gets called only if "DEL" key is pressed.
	//This doesn't get called during closing of the application, so it's OK 
	auto it = dbmap.find(udfname);
	dbmap.erase(it);
}

int CDebugDlg::GetCurrentCursor()
{
	int marked = ListView_GetSelectionMark(hList);
	return marked;
}

void CDebugDlg::OnCommand(int idc, HWND hwndCtl, UINT event)
{
#ifdef _DEBUG
	bool play=true;
#else
	bool play=true;
#endif
	bool debugging = CAstSig::vecast.size()>1;
	char buf[256];
	DWORD dw;
	WORD vcode;
	int res, marked, lineChecked;
	switch(idc)
	{
	case ID_F11: //from accelerator, event should be 1
		strcpy(buf, "#stin ");
		vcode = VK_F11;
		break;
	case ID_F10: //from accelerator, event should be 1
		strcpy(buf, "#step ");
		vcode = VK_F10;
		break;
	case ID_F5: //from accelerator, event should be 1
		strcpy(buf, "#cont ");
		vcode = VK_F5;
		break; 
	case ID_DB_EXIT: //from accelerator, event should be 1
		strcpy(buf, "#abor ");
		vcode = VK_F5;
		break;

	case ID_RUN2CUR:
		strcpy(buf, "#r2cu ");
		vcode = VK_F10;
		for (size_t k=0; k<CAstSig::vecast.size(); k++)
			CAstSig::vecast.at(k)->pEnv->curLine = ListView_GetSelectionMark(hList)+1;
		break;

	case ID_F9: //from accelerator, event should be 1
		marked = ListView_GetSelectionMark(hList);
		lineChecked = ListView_GetCheckState(hList, marked);
		ListView_SetCheckState(hList, marked, lineChecked ? 0:1);
		break;

	case ID_OPEN:
		fullfname[0]=fname[0]=0;
		if (fileDlg2.FileOpenDlg(fullfname, fname, "AUX UDF file (*.AUX)\0*.aux\0", "aux"))
//		if (((CDebugBaseDlg*)hParent)->FileOpen(fullfname))
			((CDebugBaseDlg*)hParent)->open_tab_with_UDF(fullfname, -1);
		break;
	case IDCANCEL:
		break;
	}
	if (debugging && (idc==ID_F10 || idc==ID_F11 || idc==ID_F5 || idc==ID_DB_EXIT || idc==ID_RUN2CUR) )
	{
		//Without this cleanup, previous dwControlKeyState would not be reset, so F5 would be taken as Shift-F5 if Shift-F5 was the previous event. 11/22/2017
		memset(mainSpace.debug_command_frame, 0, sizeof(mainSpace.debug_command_frame));
		for (int k=0; k<2;k++)
		{
			mainSpace.debug_command_frame[k].EventType = KEY_EVENT;
			mainSpace.debug_command_frame[k].Event.KeyEvent.uChar.AsciiChar = 0;
			mainSpace.debug_command_frame[k].Event.KeyEvent.wVirtualKeyCode = vcode;
			if (vcode==VK_F5 && idc==ID_DB_EXIT)
				mainSpace.debug_command_frame[k].Event.KeyEvent.dwControlKeyState = SHIFT_PRESSED;
			else if (vcode==VK_F10 && idc==ID_RUN2CUR) 
				mainSpace.debug_command_frame[k].Event.KeyEvent.dwControlKeyState = RIGHT_CTRL_PRESSED;
		}
		mainSpace.debug_command_frame[0].Event.KeyEvent.bKeyDown = 1;
		mainSpace.debug_command_frame[1].Event.KeyEvent.bKeyDown = 0;
		res = WriteConsoleInput(hStdin, mainSpace.debug_command_frame, 2, &dw);
	}
}

bool CDebugDlg::cannotbeBP(int line)
{
	if (line==0) return true;
	char buffer[512];
	ListView_GetItemText(hList, line, 1, buffer, sizeof(buffer));
	if (!strncmp(buffer, "//", 2)) return true;
	if (strlen(buffer)==0) return true;
	return false;
}

void CDebugDlg::OnNotify(HWND hwnd, int idcc, LPARAM lParam)
{
	static char buf[256];
	string name;
	LPNMHDR pnm = (LPNMHDR)lParam;
	LPNMLISTVIEW pview = (LPNMLISTVIEW)lParam;
	UINT code=pnm->code;
	LPNMITEMACTIVATE lpnmitem;
	vector<int>::iterator it;
	int marked, current, clickedRow;
	int res(0), lineChecked;
	CDebugBaseDlg *mPar = (CDebugBaseDlg*)hParent;
	string full;
	char udf_filename[256];
	switch(code)
	{
	case NM_CLICK:
		break;
	case NM_DBLCLK:
		lpnmitem = (LPNMITEMACTIVATE) lParam;
		clickedRow = lpnmitem->iItem;
		marked = ListView_GetSelectionMark(lpnmitem->hdr.hwndFrom);
		ListView_GetItemText(lpnmitem->hdr.hwndFrom, clickedRow, 0, buf, 256);
		break;

	case LVN_ITEMCHANGED:
		lpnmitem = (LPNMITEMACTIVATE) lParam;
		clickedRow = lpnmitem->iItem;
		if (cannotbeBP(clickedRow)) {ListView_SetCheckState(hList, clickedRow, 0); break;}
		lineChecked = ListView_GetCheckState(hList, clickedRow);
		if (lpnmitem->uNewState>=LVIF_DI_SETITEM && !lpnmitem->uOldState) break;
		if ( (lpnmitem->uNewState & LVIS_SELECTED) && !lineChecked) 	break;
		it = find(breakpoint.begin(), breakpoint.end(), clickedRow+1);
		current = mTab.GetCurrentPageID();
//		udfname = mPar->udfname[mTab.hCur];
		if (lineChecked)
		{
			if (it==breakpoint.end())
			{
				breakpoint.push_back(clickedRow+1);
				sort(breakpoint.begin(), breakpoint.end());
			}
		}
		else
		{
			if (it!=breakpoint.end())
				breakpoint.erase(it);
		}
		_splitpath(fullUDFpath, NULL, NULL, udf_filename, NULL);
		// There are multiple versions of pEnv's in CAstSig::vecast, so update DebugBreaks across all
		if (breakpoint.empty())
			for (auto scope: CAstSig::vecast) scope->pEnv->udf[udf_filename].DebugBreaks.clear();
		else
			for (auto scope : CAstSig::vecast) scope->pEnv->udf[udf_filename].DebugBreaks = breakpoint;
		break;

	//case LVN_KEYDOWN:
	//	lvnkeydown = (LPNMLVKEYDOWN)lParam;
	//	switch(lvnkeydown->wVKey)
	//	{
	//	case VK_RETURN:
	//		lpnmitem = (LPNMITEMACTIVATE) lParam;
	//		marked = ListView_GetSelectionMark(lpnmitem->hdr.hwndFrom);
	//		nItems = ListView_GetSelectedCount(lpnmitem->hdr.hwndFrom);
	//		for (int k=marked; k<marked+nItems; k++)
	//			ListView_GetItemText(lpnmitem->hdr.hwndFrom, k, 0, buf, 256);
	//		SetFocus(GetConsoleWindow());
	//		break;
	//	}
	case NM_CUSTOMDRAW:
  		lpnmitem = (LPNMITEMACTIVATE) lParam;
		clickedRow = lpnmitem->iItem;
      ::SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG)ProcessCustomDraw(pnm));
        return;
	default:
		break;
	}
}

LRESULT CDebugDlg::ProcessCustomDraw (NMHDR *lParam, bool tick)
{
	int line2Highlight = 0;
	DWORD_PTR iRow;
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;
	switch(lplvcd->nmcd.dwDrawStage) 
	{
	case CDDS_PREPAINT : //Before the paint cycle begins
	//request notifications for individual listview items
		return CDRF_NOTIFYITEMDRAW;
            
	case CDDS_ITEMPREPAINT: //Before an item is drawn
		return CDRF_NOTIFYSUBITEMDRAW;

	case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		iRow = lplvcd->nmcd.dwItemSpec;	
		switch(lplvcd->iSubItem)
		{
		case 0:
			lplvcd->clrText   = RGB(50,50,50);
			lplvcd->clrTextBk = RGB(150,150,150);
			SelectObject(lplvcd->nmcd.hdc, eFont);
			break;
		case 1:
			lplvcd->clrText   = RGB(0,0,0);
			lplvcd->clrTextBk = RGB(243,255,193);
			SelectObject(lplvcd->nmcd.hdc, eFont);
			break;
		default:
			return CDRF_DODEFAULT;
		}
		//is this udf tab part of current called udf thread?
		for (auto scope : CAstSig::vecast)
		{
			if (scope->u.title == udfname)
			{
				line2Highlight = scope->u.currentLine;
				break;
			}
		}
		if (line2Highlight && CDebugDlg::pAstSig)
		{
			if (CDebugDlg::pAstSig->u.debug.inPurgatory)
				lplvcd->clrTextBk = RGB(180, 180, 180);
			else if (iRow == line2Highlight - 1)
			{
				if (udfname == CDebugDlg::pAstSig->u.title)
					lplvcd->clrTextBk = RGB(100, 200, 40);
				else
					lplvcd->clrTextBk = RGB(115, 235, 130);
				SelectObject(lplvcd->nmcd.hdc, eFont2);
			}
		}
		return CDRF_NEWFONT;
	}
	return CDRF_DODEFAULT;
}

int CDebugDlg::GetBPandUpdate()
{
	set<int> out;
	int nLines = ListView_GetItemCount(hList);
	for (int k=0; k<nLines; k++)
	{
		if (ListView_GetCheckState(hList, k))
			out.insert(k);
	}
	return nLines;
}


BOOL CALLBACK DebugBaseProc (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	if (umsg==WM_INITDIALOG)
	{
		((CDebugBaseDlg*)lParam)->hDlg = hDlg;
		return ((CDebugBaseDlg*)lParam)->OnInitDialog((HWND)wParam, lParam);
	}
	switch (umsg)
	{
//	chHANDLE_DLGMSG (hDlg, WM_INITDIALOG, debugBase.OnInitDialog);
	chHANDLE_DLGMSG (hDlg, WM_SIZE, debugBase.OnSize);
	chHANDLE_DLGMSG (hDlg, WM_CLOSE, debugBase.OnClose);
	chHANDLE_DLGMSG (hDlg, WM_DESTROY, debugBase.OnDestroy);
	chHANDLE_DLGMSG (hDlg, WM_SHOWWINDOW, debugBase.OnShowWindow);
	chHANDLE_DLGMSG (hDlg, WM_COMMAND, debugBase.OnCommand);

	case WM_NOTIFY:
		debugBase.OnNotify(hDlg, (int)(wParam), (NMHDR *)lParam);
		return 1; // don't I need this???? Check. 8/4/2017
	default:
		return FALSE;
	}
	return TRUE;
}

CDebugBaseDlg::CDebugBaseDlg(void)
{
}

CDebugBaseDlg::~CDebugBaseDlg(void)
{ // This does not get called... find out how to clean debugVct 7/31/2017
	CoUninitialize();
	for (size_t k=0; k<debugVct.size(); k++)
	{
		delete debugVct[k];
	}
}

int CDebugBaseDlg::open_tab_with_UDF(const char *fullname, int shownID)
{ // if shownID is -1, it adds the new tab and shows it at the last (most recent) spot.
  // if shownID is -2, it only adds the new tab without showing --use this inside a loop.
	
	// Check if file is there, if not don't bother to open it
	WIN32_FIND_DATA data;
	if (FindFirstFile(fullname, &data) == INVALID_HANDLE_VALUE)
		return 0;
	CREATE_CDebugDlg transfer;
	CDebugDlg *newdbdlg = new CDebugDlg;
	transfer.dbDlg = newdbdlg;
	strcpy(transfer.fullfilename, fullname);
	newdbdlg->hParent = this;
	if (newdbdlg->hDlg = CreateDialogParam (GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_PAGE1), debugBase.hDlg, (DLGPROC)debugDlgProc, (LPARAM)&transfer))
		debugVct.push_back(newdbdlg);
	int res = newdbdlg->SetSize();
	_splitpath(fullname, NULL, NULL, fname, NULL);
	if (shownID==-1)
		shownID = (int)::SendMessage(mTab.hTab, TCM_GETITEMCOUNT, 0, 0);
	mTab.AddPage(newdbdlg->hDlg, fname);
	if (shownID!=-2)
	{
		TabCtrl_SetCurSel(mTab.hTab, shownID);
		TabCtrl_SetCurFocus(mTab.hTab, shownID);

		for (size_t k(0); k<mTab.titleStr.size(); k++)
			::ShowWindow(mTab.page[k], SW_HIDE);
		res = TabCtrl_GetCurSel(mTab.hTab);
		::ShowWindow(mTab.hCur=mTab.page[res], SW_SHOW);
	}
	return 1;
}

BOOL CDebugBaseDlg::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	CWndDlg::OnInitDialog(hwndFocus, lParam);
	fileDlg2.InitFileDlg(hDlg, hInst, "");

	hTab = GetDlgItem(IDC_TAB1);
	mTab.initTabcontrol(hTab);
	mTab.mDad = this;
	mTab.hTab = hTab;

	int firstPage2show(0);
	char errstr[256];
	string strRead;
	if (ReadINI (errstr, iniFile, "DEBUGGING UDFS", strRead)>0)
	{
		vector<string> tar;
		vector<bool> good;
		size_t count = str2vect(tar, strRead.c_str(), "\r\n");

		for (size_t k = 0; k < count; k++)
		{
			string cleaned = cleanupstring(tar[k]);
			if (open_tab_with_UDF(cleaned.c_str(), k == count - 1 ? -1 : -2) == 0)
			{
				//file not found
				string emsg;
				sformat(emsg, "File not found --\n%s", tar[k].c_str());
				good.push_back(false);
				MessageBox(emsg.c_str(), "Check [DEBUG VIEW] in .ini file");
			}
			else
				good.push_back(true);
		}
		string newfiles;
		for (size_t k = 0; k < count; k++)
		{
			if (good[k]) {
				newfiles += tar[k];
				newfiles += "\r\n";
			}
		}
		if (count > 0)
			printfINI(errstr, iniFile, "DEBUGGING UDFS", newfiles.c_str());
	}
	else
	{
		fullfname[0]=fname[0]=0;
		if (fileDlg2.FileOpenDlg(fullfname, fname, "AUX UDF file (*.AUX)\0*.aux\0", "aux"))
//		if (FileOpen(fullfname))
			open_tab_with_UDF(fullfname, 0);
		else
		{
			if (GetLastError()!=0) {GetLastErrorStr(errstr); MessageBox (errstr, "File Open dialog box error"); return 0;}
		}
	}

// If FileOpen() is used, un-comment this line.
//	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if (udfname.size() > 0)
	{
		ShowWindow(SW_SHOW);
		if (ReadINI(errstr, iniFile, "DEBUG VIEW POS", strRead) > 0)
		{
			int tar[4];
			if (str2array(tar, 4, strRead.c_str(), " ") == 4)
			{
				CRect rtDebugDlg;
				rtDebugDlg.left = tar[0];
				rtDebugDlg.top = tar[1];
				rtDebugDlg.right = tar[2] + tar[0];
				rtDebugDlg.bottom = tar[3] + tar[1];
				MoveWindow(rtDebugDlg);
			}
		}
	}
	else
	{
		DestroyWindow();
		SendMessage(WM__ENDTHREAD); // this will end thread.
	}
	return TRUE;
}

char *UnicodeToAnsi(LPCWSTR s, char *dest)
{
	if (s==NULL) return NULL;
	int cw=lstrlenW(s);
	if (cw==0) {CHAR *psz=new CHAR[1];*psz='\0';return psz;}
	int cc=WideCharToMultiByte(CP_ACP,0,s,cw,NULL,0,NULL,NULL);
	if (cc==0) return NULL;
	cc=WideCharToMultiByte(CP_ACP,0,s,cw,dest,cc,NULL,NULL);
	if (cc==0) {return NULL;}
	dest[cc]='\0';
	return dest;
}

int CDebugBaseDlg::FileOpen(char *fullfilename)
{
	int res(0);
	IFileOpenDialog *pFileOpen;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, 
                IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
	hr = pFileOpen->Show(NULL);
	if (SUCCEEDED(hr))
	{
		IShellItem *pItem;
		hr = pFileOpen->GetResult(&pItem);
		if (SUCCEEDED(hr))
		{
			PWSTR pszFilePath;
			hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
			// Display the file name to the user.
			if (SUCCEEDED(hr))
			{
				if (UnicodeToAnsi(pszFilePath, fullfilename))
					res=1;
				CoTaskMemFree(pszFilePath);
			}
			pItem->Release();
		}
	}
	pFileOpen->Release();
	return res;
}

void CDebugBaseDlg::OnShowWindow(BOOL fShow, UINT status)
{
 	RECT rt;
	::GetClientRect (hDlg, &rt);
	RECT rt2;
	int res2 = ::GetClientRect (mTab.hTab, &rt2);
//	::MoveWindow(hTab, 0, 0,  rt.right, rt.bottom, 1);
	int res = SetWindowPos(hTab, HWND_BOTTOM, 0, 0, rt.right-rt.left, rt.bottom-rt.top, SWP_SHOWWINDOW );
	res2 = ::GetClientRect (mTab.hTab, &rt2);
//	That is NOT the same as ::MoveWindow(hTab, 0, 0,  rt.right, rt.bottom, 1); ==> This changes the z-order and puts hTab on TOP
}

void CDebugBaseDlg::OnSize(UINT state, int cx, int cy)
{ // This is only in response to user re-sizing of debug window
	::MoveWindow(hTab, 0, 0, cx, 50, 1);
  //  SetWindowPos(hTab, HWND_TOPMOST, 0, 0, cx, cy, SWP_SHOWWINDOW);
// 	RECT rt;
//	::GetClientRect (hDlg, &rt);
//	int iSel = TabCtrl_GetCurSel(hTab);
	for (unordered_map<string, CDebugDlg*>::iterator it=dbmap.begin(); it!=dbmap.end(); it++)
	{
		it->second->SetSize();

	}
//	::ShowWindow(mTab.hCur, SW_SHOW);
	
}

void CDebugBaseDlg::OnClose()
{
	if (MessageBox("If you close this, all udfs opened will be closed.", "Are you sure?", MB_YESNO)==IDYES)
		OnDestroy();
}

void CDebugBaseDlg::OnDestroy()
{
//	::SendMessage(GetParent(hDlg), WM_DEBUG_CLOSE, 0, 0);
	PostThreadMessage(GetWindowThreadProcessId(hDlg, NULL), WM__ENDTHREAD, 0, 0);
}

void CDebugBaseDlg::OnCommand(int idc, HWND hwndCtl, UINT event)
{
	switch(idc)
	{
	case IDCANCEL:
		OnClose();
		break;
	}
}

void CDebugBaseDlg::OnNotify(HWND hwnd, int idcc, NMHDR *pnm)
{
	int res(0), iPage;
	LPNMLVKEYDOWN lvnkeydown = (LPNMLVKEYDOWN)pnm;
	if (idcc==IDC_TAB1) 
	{
		mTab.OnNotify(pnm);
	}
	switch(pnm->code)
	{
	case TCN_SELCHANGING:
        ::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG)FALSE); // TO RETURN FALSE
        return;
	case TCN_SELCHANGE:
		iPage = TabCtrl_GetCurSel(hTab); 
		break;
	case NM_DBLCLK:
		break;
	case LVN_ITEMCHANGED:
		break;
	case LVN_KEYDOWN:
		switch(lvnkeydown->wVKey)
		{
		case VK_SPACE:
			break;
		case VK_DELETE:
			break;
		}
		break;
	}
}

int CDebugBaseDlg::GetID(const char *fname)
{
	int res(-1);
	HWND hTarget;
	for (map<HWND, string>::iterator it = udfname.begin(); it != udfname.end(); it++)
	{
		if (it->second == string(fname))
		{
			hTarget = it->first;
			break;
		}
	}
	LRESULT count = ::SendMessage(hTab, TCM_GETITEMCOUNT, 0, 0);
	for (int k = 0; k < count; k++)
	{
//		if ()
	}
	return 1;


}
BOOL CALLBACK TabPage_DlgProc(HWND hwndDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	static int mouseclicked(0);
	mTab.hCur = hwndDlg;
	switch (umsg)
	{
		chHANDLE_DLGMSG(hwndDlg, WM_INITDIALOG, mTab.OnInitDialog);
		chHANDLE_DLGMSG(hwndDlg, WM_SIZE, mTab.OnSize);
		chHANDLE_DLGMSG(hwndDlg, WM_COMMAND, mTab.OnCommand);
		//// TODO: Add TabPage dialog message crackers here...

		//default:
		//{
		//	m_lptc->ParentProc(hwndDlg, msg, wParam, lParam);
		//	return FALSE;
		//}
	}
	return 0;
}

