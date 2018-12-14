// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: auxlab
// Main Application. Based on Windows API  
// 
// 
// Version: 1.495
// Date: 12/13/2018
// 
// (Parts of this code were adapted from TABCTRL.C by David MacDermot)
//
#include "wxx_wincore.h" // Win32++ 8.2. This must be placed prior to <windows.h> to avoid including winsock.h
#include "showvar.h"

#include <vector>
#include <string>
#include "TabCtrl.h"

using namespace std;

#define PRINTLOG(FNAME,STR) \
{ FILE*__fp=fopen(FNAME,"at"); fprintf(__fp,STR);	fclose(__fp); }

static VOID TabPageMessageLoop(HWND hwnd)
{
	MSG msg;
	int status;
	BOOL handled = FALSE;
	BOOL fFirstStop = FALSE;
	HWND hFirstStop = NULL;

	while ((status = GetMessage(&msg, NULL, 0, 0)))
	{
		if (-1 == status)	// Exception
		{
			return;
		}
		else
		{
			//This message is explicitly posted from TabCtrl_OnSelChanged() to
			// indicate the closing of this page.  Stop the Loop.
			if (WM_SHOWWINDOW == msg.message && FALSE == msg.wParam)
				return;

			//IsDialogMessage() dispatches WM_KEYDOWN to the tab page's child controls
			// so we'll sniff them out before they are translated and dispatched.
			if (WM_KEYDOWN == msg.message && VK_TAB == msg.wParam)
			{
				//Tab each tabstop in a tab page once and then return to to
				// the tabCtl selected tab
				if (!fFirstStop)
				{
					fFirstStop = TRUE;
					hFirstStop = msg.hwnd;
				}
				else if (hFirstStop == msg.hwnd)
				{
					// Tab off the tab page
					HWND hTab = (HWND)GetWindowLongPtr(GetParent(msg.hwnd), GWLP_USERDATA);
					if (NULL == hTab)
						hTab = hTab;
					SetFocus(hTab);
					return;
				}
			}
			// Perform default dialog message processing using IsDialogM. . .
			handled = IsDialogMessage(hwnd, &msg);

			// Non dialog message handled in the standard way.
			if (!handled)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	// If we get here window is closing
	PostQuitMessage(0);
	return;
}

CTabControl::CTabControl()
{
}

void CTabControl::FirstTabstop_SetFocus(HWND h)
{
	FORWARD_WM_NEXTDLGCTL(h, 1, FALSE, SendMessage);
	FORWARD_WM_KEYDOWN(GetFocus(), VK_TAB, 0, 0, PostMessage);
}

void CTabControl::initTabcontrol(HWND h)
{
	hTab = h;
	tie.mask = TCIF_TEXT | TCIF_IMAGE;
	tie.iImage = -1;
}

int CTabControl::AddPage(HWND hNewpage, string title)
{
	char buf[256];
	strcpy(buf, title.c_str());
	char *pt=strchr(buf, '.');
	if (pt) pt[0]=0;
	tie.pszText = buf;
	TabCtrl_InsertItem(hTab, TabCtrl_GetItemCount(hTab), &tie);
	page.push_back(hNewpage);
	titleStr.push_back(buf);
	return 1;
}

void CTabControl::DeletePage(int index)
{
	auto hh = page[index];
	auto res = DestroyWindow(hh);
	page.erase(page.begin() + index);
	titleStr.erase(titleStr.begin() + index);
	TabCtrl_DeleteItem(hTab, index);
	//what if index is 0?
	TabCtrl_SetCurSel(hTab, index-1);
}

BOOL CTabControl::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{

	return 1;
}

int CTabControl::GetCurrentPageID(const char *fname)
{
	CDebugBaseDlg *tp = (CDebugBaseDlg *)mDad;
	map<string, HWND>::iterator it = tp->dlgPage_hwnd.find(string(fname));
	if (it != tp->dlgPage_hwnd.end())
	{
		for (size_t k(0); k<page.size(); k++)
			if (it->second == page[k])
				return (int)k;
	}
	return -1;
}

int CTabControl::GetCurrentPageID()
{
	for (size_t k(0); k<page.size(); k++)
		if (hCur==page[k]) 
			return (int)k;
	return -1;
}

void CTabControl::OnSize(UINT state, int cx, int cy)
{
	RECT wndrct;
	int wndWidth, wndHeight;
	int recWidth, recHeight;
	switch(GetCurrentPageID())
	{
	case 1:
		GetClientRect(hCur, &wndrct);
		wndWidth=wndrct.right-wndrct.left, wndHeight=wndrct.bottom-wndrct.top;
		break;
	case 2: // freq range
		GetClientRect(hCur, &wndrct);
		wndWidth=wndrct.right-wndrct.left;
		wndHeight=wndrct.bottom-wndrct.top;
		recWidth = wndWidth/15;
		recHeight = wndHeight/7;
		CPoint pt1(wndrct.left + recWidth, wndrct.top+recHeight*3);
		CPoint pt2(pt1.x + recWidth, pt1.y+recHeight);
		break;
	}
}


void CTabControl::OnCommand(INT id, HWND hwndCtl, UINT codeNotify)
{
	vector<string> str4pipe;
	string argstr, argstr2;
	int tabID;
	CRect rct;
	switch((tabID=GetCurrentPageID()))
	{
	case 1:
		break;
	case 2:
		break;
	}

	//Forward all commands to the parent proc.
	//Note: We don't use SendMessage because on the receiving end,
	// that is: _OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify),
	// hwnd would = addressee and not the parent of the control.  Intuition
	// favors the notion that hwnd is the parent of hwndCtl.
//	FORWARD_WM_COMMAND(hTab, id, hwndCtl, codeNotify, mappyDlgProc);

	// If this WM_COMMAND message is a notification to parent window
	// ie: EN_SETFOCUS being sent when an edit control is initialized
	// do not engage the Message Loop or send any messages.
	if (codeNotify != 0)
		return;

	// Mouse clicks on a control should engage the Message Loop
	SetFocus(hwndCtl);
	FirstTabstop_SetFocus(hTab);
	TabPageMessageLoop(hTab);
}

BOOL CTabControl::OnNotify(LPNMHDR pnm)
{
	//page[0].OnNotify(pnm); // probably this is not necessary. 7/31/2016 bjk

	switch (pnm->code)
	{
		case TCN_KEYDOWN:
			TabCtrl_OnKeyDown((LPARAM)pnm);

		// fall through to call TabCtrl_OnSelChanged() on each keydown
		case TCN_SELCHANGE:
			return TabCtrl_OnSelChanged();
	}
	return FALSE;
}

int CTabControl::SetCurrentTab(const char* fname)
{
	int curId = GetCurrentPageID(fname);
	if (curId >= 0)
	{
		TabCtrl_OnSelChanged(curId);
		TabCtrl_SetCurSel(hTab, curId);
	}
	return curId;
}

BOOL CTabControl::TabCtrl_OnSelChanged(int id)
{
	for (size_t k(0); k<titleStr.size(); k++)
		ShowWindow(page[k], SW_HIDE);
	if (id < 0)
	{
		int iSel = TabCtrl_GetCurSel(hTab);
		ShowWindow(hCur = page[iSel], SW_SHOW);
	}
	else
	{
		ShowWindow(hCur = page[id], SW_SHOW);
	}

#ifdef _DEBUG
	//char __buf[256];
	//sprintf(__buf, "hCur set 0x%x (TabCtrl_OnSelChanged)\n", hCur);
	//PRINTLOG("c:\\temp\\windows_log.txt", __buf)
#endif //_DEBUG

	return TRUE;
}

void CTabControl::TabCtrl_OnKeyDown(LPARAM lParam)
{
	INT_PTR res;
	TC_KEYDOWN *tk = (TC_KEYDOWN *)lParam;
	int itemCount = TabCtrl_GetItemCount(tk->hdr.hwndFrom);
	int currentSel = TabCtrl_GetCurSel(tk->hdr.hwndFrom);

	if (itemCount <= 1)
		return;	// Ignore if only one TabPage

	BOOL verticalTabs = GetWindowLong(hTab, GWL_STYLE) & TCS_VERTICAL;

	if (verticalTabs)
	{
	}	
	else	// horizontal Tabs
	{
		switch (tk->wVKey)
		{
		case VK_NEXT:	//select the previous page
			if (0 == currentSel)
				return;
			TabCtrl_SetCurSel(tk->hdr.hwndFrom, currentSel - 1);
			TabCtrl_SetCurFocus(tk->hdr.hwndFrom, currentSel - 1);
			break;

		case VK_LEFT:	//select the previous page
			if (0 == currentSel)
				return;
			TabCtrl_SetCurSel(tk->hdr.hwndFrom, currentSel - 1);
			TabCtrl_SetCurFocus(tk->hdr.hwndFrom, currentSel);
			break;

		case VK_PRIOR:	//select the next page
			TabCtrl_SetCurSel(tk->hdr.hwndFrom, currentSel + 1);
			TabCtrl_SetCurFocus(tk->hdr.hwndFrom, currentSel + 1);
			break;

		case VK_RIGHT:	//select the next page
			TabCtrl_SetCurSel(tk->hdr.hwndFrom, currentSel + 1);
			TabCtrl_SetCurFocus(tk->hdr.hwndFrom, currentSel);
			break;

		case VK_UP:	//navagate within selected child tab page
		case VK_DOWN:
			SetFocus(page[currentSel]);
			FirstTabstop_SetFocus(page[currentSel]);
			TabPageMessageLoop(page[currentSel]);
			break;

		case VK_DELETE:
			res = MessageBox(hTab, "Close the tab?", titleStr[currentSel].c_str(), MB_YESNO);
			if (res == IDYES)
				DeletePage(currentSel);
			break;
		default:
			return;
		}
	}
}

