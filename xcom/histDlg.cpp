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
#include "histDlg.h"
#include "resource1.h"
#include "bjcommon.h"
#include "bjcommon_win.h"
#include "sigproc.h"
#include "xcom.h"

extern CHistDlg mHistDlg;

#define FILLHIST 100
extern HANDLE hStdin, hStdout; 
extern xcom mainSpace;

HWND CreateTT(HINSTANCE hInst, HWND hParent, RECT rt, char *string, int maxwidth=400);

BOOL CALLBACK historyDlgProc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	switch (umsg)
	{
	chHANDLE_DLGMSG (hDlg, WM_INITDIALOG, mHistDlg.OnInitDialog);
	chHANDLE_DLGMSG (hDlg, WM_SIZE, mHistDlg.OnSize);
	chHANDLE_DLGMSG (hDlg, WM_CLOSE, mHistDlg.OnClose);
	chHANDLE_DLGMSG (hDlg, WM_SHOWWINDOW, mHistDlg.OnShowWindow);
	chHANDLE_DLGMSG (hDlg, WM_COMMAND, mHistDlg.OnCommand);
	chHANDLE_DLGMSG (hDlg, WM_TIMER, mHistDlg.OnTimer);
	case WM_NOTIFY:
		mHistDlg.OnNotify(hDlg, (int)(wParam), lParam);
		break;
	case WM_SETFONT:
		{
			HDC hdc = GetDC(NULL);
			HFONT ft = (HFONT)wParam;
			SIZE  sz;
			GetTextExtentPoint32 (hdc, "X", 1, &sz);
			mHistDlg.pixelPerLine = sz.cy;
			ReleaseDC(NULL, hdc);
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

CHistDlg::CHistDlg(void)
{

}

CHistDlg::~CHistDlg(void)
{

}

BOOL CHistDlg::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	CWndDlg::OnInitDialog(hwndFocus, lParam);
	sendtoEventLogger("CHistDlg::OnInitDialog");
	lvInit();
	SetTimer(FILLHIST, 100, NULL);
	return TRUE;
}

void CHistDlg::lvInit()
{
	LvItem.mask=LVIF_TEXT;   // Text Style
	LvItem.cchTextMax = 256; // Max size of text
	LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
	hList = GetDlgItem(IDC_LISTHIST);
	LRESULT res = ::SendMessage(hList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT);
	RECT rt;
	char buf[256];
	::GetClientRect (hList, &rt);
	LoadString (hInst, IDS_HELP_HISTORY, buf, sizeof(buf));
	CreateTT(hInst, hList, rt, buf, 270);
}

void CHistDlg::OnTimer(UINT id)
{
	KillTimer(id);
	char buf[256];
	DWORD dw = sizeof(buf);
	GetComputerName(buf, &dw);
	sprintf(mHistDlg.logfilename, "%s%s%s_%s.log", AppPath, AppName, HISTORY_FILENAME, buf);
	sendtoEventLogger("CHistDlg::OnTimer begins");

	string strRead;
	auto ress = GetFileText(logfilename, "rt", strRead);
	vector<string> lines;
	size_t res2;
	if (ress>0)
	{
		res2 = str2vect(lines, strRead.c_str(), "\r\n");
	}
	FillupHist(lines);
	sendtoEventLogger("CHistDlg::OnTimer ends");
}

void CHistDlg::OnShowWindow(BOOL fShow, UINT status)
{

}

void CHistDlg::OnSize(UINT state, int cx, int cy)
{
	::MoveWindow(GetDlgItem(IDC_LISTHIST), 0, 0, cx, cy, 1);
	int res = ListView_SetColumnWidth(hList, 3, cx);
}

void CHistDlg::OnClose()
{
	ShowWindow(SW_HIDE);
}

void CHistDlg::OnDestroy()
{

}

void CHistDlg::OnCommand(int idc, HWND hwndCtl, UINT event)
{
}

int char2INPUT_RECORD(const char *buf, INPUT_RECORD ir[])
{
	unsigned int k(0), id(0);
	HKL hkl = GetKeyboardLayout (0);
	SHORT mix;
	for (; id<strlen(buf); k++, id++)
	{
		ir[k].EventType = KEY_EVENT;
		ir[k].Event.KeyEvent.bKeyDown = 1;
		mix = VkKeyScanEx(buf[id], hkl);

		(HIBYTE(mix)==1) ? ir[k].Event.KeyEvent.dwControlKeyState = SHIFT_PRESSED : ir[k].Event.KeyEvent.dwControlKeyState = 0;
		ir[k].Event.KeyEvent.uChar.AsciiChar = buf[id];
		ir[k].Event.KeyEvent.wVirtualKeyCode = LOBYTE(mix);
		ir[k].Event.KeyEvent.wVirtualScanCode = MapVirtualKey (LOBYTE(mix), MAPVK_VK_TO_VSC);
		ir[k].Event.KeyEvent.wRepeatCount = 1;

		if (HIBYTE(mix)==0) {
			k++;
			ir[k] = ir[k-1];
			ir[k].Event.KeyEvent.bKeyDown = 0;
		}
	}
	//This simply marks the end for WriteConsoleInput, not for real "return"
	ir[k].EventType = KEY_EVENT;
	ir[k].Event.KeyEvent.bKeyDown = TRUE;
	ir[k].Event.KeyEvent.dwControlKeyState = 0;
	ir[k].Event.KeyEvent.uChar.AsciiChar = VK_RETURN;
	ir[k].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
	ir[k].Event.KeyEvent.wRepeatCount = 1;
	ir[k].Event.KeyEvent.wVirtualScanCode = MapVirtualKey (VK_RETURN, MAPVK_VK_TO_VSC);
	return k+1;
}

void CHistDlg::OnNotify(HWND hwnd, int idcc, LPARAM lParam)
{
	static int clickedRow;
	int res2, res(0);
	DWORD dw;
	static char buf[256];
	static	string str2conv;
	LPNMHDR pnm = (LPNMHDR)lParam;
	LPNMLISTVIEW pview = (LPNMLISTVIEW)lParam;
	LPNMLVKEYDOWN lvnkeydown;
	UINT code=pnm->code;
	LPNMITEMACTIVATE lpnmitem;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	int marked;
	INPUT_RECORD *ir;
	static FILE *fp;
	switch(code)
	{
	case NM_CLICK:
		lpnmitem = (LPNMITEMACTIVATE) lParam;
		clickedRow = lpnmitem->iItem;
		break;
	case NM_DBLCLK:
		lpnmitem = (LPNMITEMACTIVATE) lParam;
		clickedRow = lpnmitem->iItem;
		marked = ListView_GetSelectionMark(lpnmitem->hdr.hwndFrom);
		ListView_GetItemText(lpnmitem->hdr.hwndFrom, clickedRow, 0, buf, 256);
		//This hopefully takes care of the weird character problem 9/28/2017
		GetConsoleScreenBufferInfo(hStdout, &coninfo);
		coninfo.dwCursorPosition.X = (SHORT)mainSpace.comPrompt.size();
		SetConsoleCursorPosition(hStdout, coninfo.dwCursorPosition);
		ir = new INPUT_RECORD[2*strlen(buf)+3];
		WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), ir, char2INPUT_RECORD(buf, ir), &dw);
		delete[] ir;
		break;
	case LVN_KEYDOWN:
		lvnkeydown = (LPNMLVKEYDOWN)lParam;
		if (lvnkeydown->wVKey==VK_RETURN)
		{
			lpnmitem = (LPNMITEMACTIVATE) lParam;
			marked = ListView_GetSelectionMark(lpnmitem->hdr.hwndFrom);
			int nItems = ListView_GetSelectedCount(lpnmitem->hdr.hwndFrom);
			str2conv="";
			for (int k=marked; k<marked+nItems; k++)
			{
				ListView_GetItemText(lpnmitem->hdr.hwndFrom, k, 0, buf, 256);
				str2conv += buf;
				if (k<marked+nItems-1) str2conv += '\n';
			}
			ir = new INPUT_RECORD[2*str2conv.size() + 3];
			res = char2INPUT_RECORD(str2conv.c_str(), ir);
			//This hopefully takes care of the weird character problem 9/28/2017
			GetConsoleScreenBufferInfo(hStdout, &coninfo);
			coninfo.dwCursorPosition.X = (SHORT)mainSpace.comPrompt.size();
			SetConsoleCursorPosition(hStdout, coninfo.dwCursorPosition);
			res2 = WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), str2conv.c_str(), (DWORD)str2conv.size(), &dw, NULL);
			delete[] ir;
			SetForegroundWindow(GetConsoleWindow());
		}
		break;
	case NM_CUSTOMDRAW:
        ::SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG)ProcessCustomDraw(pnm));
        return;
	default:
		break;
	}
}

LRESULT CHistDlg::ProcessCustomDraw (NMHDR *lParam)
{
	return CDRF_DODEFAULT;
}

void CHistDlg::FillupHist(const vector<string> &in)
{
	int width[]={400, };
	LRESULT res;
	int cum = 0;
	for(int k = 0; k<1; k++)
	{
		LvCol.cx=width[k];
		res = ::SendMessage(hList, LVM_INSERTCOLUMN,k,(LPARAM)&LvCol); 
	}
	LvItem.iItem=0;
	LvItem.iSubItem=0;
	//display up to 2048 previous commands
	for (auto r = in.rbegin(); r<in.rend() && cum<2048; r++)
	{
		if ((*r).find("//")!=0)
		{
			LvItem.pszText=(char*)(*r).c_str();
			res = SendDlgItemMessage(IDC_LISTHIST,LVM_INSERTITEM,0,(LPARAM)&LvItem);
		}
	}
	res = ListView_Scroll(hList,0,ListView_GetItemCount(hList)*14);
}
void CHistDlg::AppendHist(vector<string> input)
{
	for (size_t k=0; k<input.size(); k++)
	{
		LvItem.pszText=(char*)input[k].c_str();
		LvItem.iItem=ListView_GetItemCount(hList)+1;
		SendDlgItemMessage(IDC_LISTHIST,LVM_INSERTITEM,0,(LPARAM)&LvItem);
	}
	//This advances history view by one line.
	SendDlgItemMessage(IDC_LISTHIST,LVM_SCROLL ,0,(LPARAM)pixelPerLine);
}
