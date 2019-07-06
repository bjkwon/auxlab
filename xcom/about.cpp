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
// Parts of this code are from http://stackoverflow.com/questions/7784975/c-win32-add-a-hyperlink-to-a-dialog Author: SSpoke
// 
#include "bjcommon.h"
#include "bjcommon_win.h"
#include "resource1.h"
#include <commctrl.h>  // includes the common control header
#include "htmlhelp.h"

void CreateHyperLink(HWND hwndControl);

HWND CreateTT(HINSTANCE hInst, HWND hParent, RECT rt, char *string, int maxwidth)
{
	TOOLINFO info;
	HWND TT = ::CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,        
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hParent, NULL, hInst, NULL);
	if (TT==NULL) return TT;
	SetWindowPos(TT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	info.cbSize = sizeof(TOOLINFO);
	info.uFlags = TTF_SUBCLASS;
	info.hwnd = hParent;
	info.hinst = hInst;
	info.lpszText = string;
	info.rect = rt;
    /* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
	SendMessage(TT, TTM_ACTIVATE, TRUE, 0);	
    SendMessage(TT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&info);	
	SendMessage(TT, TTM_SETMAXTIPWIDTH, 0, maxwidth);
	return TT;
}

const wchar_t *makewidechar(const char *c)
{
    const size_t cSize = strlen(c)+1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs (wc, c, cSize);

    return wc;
}

BOOL AboutDlg (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	static HINSTANCE hInst;
	char buf[256], fullmoduleName[MAX_PATH];
 	char drive[16], dir[256], ext[8], buffer[MAX_PATH], AppPath[MAX_PATH];
	wchar_t wbuf[128];
	HWND hPic, hHelp;
	switch (umsg)
	{
	case WM_INITDIALOG:
		hInst = (HINSTANCE) lParam;
 		GetModuleFileName(hInst, fullmoduleName, MAX_PATH);
 		_splitpath(fullmoduleName, drive, dir, buffer, ext);
 		sprintf (AppPath, "%s%s", drive, dir);
		getVersionString(fullmoduleName, buffer, sizeof(buffer));
		swprintf(wbuf, 128, L"AUXLAB %s © 2009--2019", makewidechar(buffer));
		SetDlgItemTextW(hDlg, IDC_VERSTR, wbuf);
		for (int k(IDS_STRING109); k<=IDS_STRING111; k++)
		{
			LoadString (hInst, k, buf, sizeof(buf));
			EditPrintf(GetDlgItem(hDlg, IDC_DISCLAIMER), "%s\n", buf);
			EditPrintf(GetDlgItem(hDlg, IDC_DISCLAIMER), "\n");
		}
		for (int k(IDS_CREDIT1); k<=IDS_CREDIT8; k++)
		{
			LoadString (hInst, k, buf, sizeof(buf));
			EditPrintf(GetDlgItem(hDlg, IDC_CREDITS), "%s\n", buf);
		}
		CreateHyperLink(GetDlgItem(hDlg, IDC_BRMLINK));
		CreateHyperLink(GetDlgItem(hDlg, IDC_WEBLINK));
		CreateHyperLink(GetDlgItem(hDlg, IDC_ONLINE_HELP));
		break;
	case WM_SHOWWINDOW:
		hPic = GetDlgItem(hDlg, IDC_ABOUTICON);
		ShowWindow(hPic,SW_SHOW);
		break;

/* Tried to use Syslink control, but that required common control 6.0, which messed with XP theme style and made listview controls ugly. 
Gave up and used the subclassed static control from SSpoke. 7/7/2016 bjk
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
			case NM_CLICK:
			case NM_RETURN:
			{
				PNMLINK pNMLink = (PNMLINK)lParam;
				LITEM   item    = pNMLink->item;
				if ((((LPNMHDR)lParam)->hwndFrom == hSysLink) && (item.iLink == 0))
				{
					ShellExecuteW(NULL, L"open", item.szUrl, NULL, NULL, SW_SHOW);
				}
 				break;
			}
		}
		break;
*/
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BRMLINK:
			LoadString(hInst, IDS_BRMLINK, buf, sizeof(buf));
			ShellExecute(NULL, "open", buf, NULL, NULL, SW_SHOWNORMAL);
			break;
		case IDC_ONLINE_HELP:
			LoadString(hInst, IDS_ONLINE_HELP, buf, sizeof(buf));
			ShellExecute(NULL, "open", buf, NULL, NULL, SW_SHOWNORMAL);
			break;
		case IDC_WEBLINK:
			LoadString(hInst, IDS_WEBLINK, buf, sizeof(buf));
			ShellExecute(NULL, "open", buf, NULL, NULL, SW_SHOWNORMAL);
			break;
		case IDC_CHM:
			hHelp = HtmlHelp( GetDesktopWindow(), "auxlab.chm", HH_DISPLAY_TOPIC, NULL);
			if (!hHelp)
				MessageBox(hDlg, "auxlab.chm not found", "AUXLAB-About", 0);
			break;
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, 0);
		break;
		}
	default:
		return FALSE;
	}
	return TRUE;
}



/* Subclassing static control to make it act like syslink
   Source: http://stackoverflow.com/questions/7784975/c-win32-add-a-hyperlink-to-a-dialog
   Author: SSpoke 
   Slightly edited by bjkwon to make compatible with WIN64  7/7/2016
   Don't forget to change LONG to LONG_PTR 10/8/2016 
*/

/* Start of HyperLink URL */
#define PROP_ORIGINAL_FONT      TEXT("_Hyperlink_Original_Font_")
#define PROP_ORIGINAL_PROC      TEXT("_Hyperlink_Original_Proc_")
#define PROP_STATIC_HYPERLINK   TEXT("_Hyperlink_From_Static_")
#define PROP_UNDERLINE_FONT     TEXT("_Hyperlink_Underline_Font_")
LRESULT CALLBACK _HyperlinkParentProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK _HyperlinkProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//static void CreateHyperLink(HWND hwndControl);
/* End of HyperLink URL */

static void CreateHyperLink(HWND hwndControl)
{
    // Subclass the parent so we can color the controls as we desire.
    HWND hwndParent = GetParent(hwndControl);
    if (NULL != hwndParent)
    {
        WNDPROC pfnOrigProc = (WNDPROC)GetWindowLongPtr(hwndParent, GWLP_WNDPROC);
        if (pfnOrigProc != _HyperlinkParentProc)
        {
            SetProp(hwndParent, PROP_ORIGINAL_PROC, (HANDLE)pfnOrigProc);
            SetWindowLongPtr(hwndParent, GWLP_WNDPROC, (LONG_PTR)(WNDPROC)_HyperlinkParentProc);
        }
    }

    // Make sure the control will send notifications.
    LONG_PTR dwStyle = GetWindowLongPtr(hwndControl, GWL_STYLE);
    SetWindowLongPtr(hwndControl, GWL_STYLE, dwStyle | SS_NOTIFY);

    // Subclass the existing control.
    WNDPROC pfnOrigProc = (WNDPROC)GetWindowLongPtr(hwndControl, GWLP_WNDPROC);
    SetProp(hwndControl, PROP_ORIGINAL_PROC, (HANDLE)pfnOrigProc);
    SetWindowLongPtr(hwndControl, GWLP_WNDPROC, (LONG_PTR)(WNDPROC)_HyperlinkProc);

    // Create an updated font by adding an underline.
    HFONT hOrigFont = (HFONT)SendMessage(hwndControl, WM_GETFONT, 0, 0);
    SetProp(hwndControl, PROP_ORIGINAL_FONT, (HANDLE)hOrigFont);

    LOGFONT lf;
    GetObject(hOrigFont, sizeof(lf), &lf);
    lf.lfUnderline = TRUE;

    HFONT hFont = CreateFontIndirect(&lf);
    SetProp(hwndControl, PROP_UNDERLINE_FONT, (HANDLE)hFont);

    // Set a flag on the control so we know what color it should be.
    SetProp(hwndControl, PROP_STATIC_HYPERLINK, (HANDLE)1);
}

LRESULT CALLBACK _HyperlinkParentProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WNDPROC pfnOrigProc = (WNDPROC)GetProp(hwnd, PROP_ORIGINAL_PROC);

    switch (message)
    {
    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        HWND hwndCtl = (HWND)lParam;

        BOOL fHyperlink = (NULL != GetProp(hwndCtl, PROP_STATIC_HYPERLINK));
        if (fHyperlink)
        {
            LRESULT lr = CallWindowProc(pfnOrigProc, hwnd, message, wParam, lParam);
            SetTextColor(hdc, RGB(0, 0, 192));
            return lr;
        }

        break;
    }
    case WM_DESTROY:
    {
        SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)pfnOrigProc);
        RemoveProp(hwnd, PROP_ORIGINAL_PROC);
        break;
    }
    }
    return CallWindowProc(pfnOrigProc, hwnd, message, wParam, lParam);
}

LRESULT CALLBACK _HyperlinkProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WNDPROC pfnOrigProc = (WNDPROC)GetProp(hwnd, PROP_ORIGINAL_PROC);

    switch (message)
    {
    case WM_DESTROY:
    {
        SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)pfnOrigProc);
        RemoveProp(hwnd, PROP_ORIGINAL_PROC);

        HFONT hOrigFont = (HFONT)GetProp(hwnd, PROP_ORIGINAL_FONT);
        SendMessage(hwnd, WM_SETFONT, (WPARAM)hOrigFont, 0);
        RemoveProp(hwnd, PROP_ORIGINAL_FONT);

        HFONT hFont = (HFONT)GetProp(hwnd, PROP_UNDERLINE_FONT);
        DeleteObject(hFont);
        RemoveProp(hwnd, PROP_UNDERLINE_FONT);

        RemoveProp(hwnd, PROP_STATIC_HYPERLINK);

        break;
    }
    case WM_MOUSEMOVE:
    {
        if (GetCapture() != hwnd)
        {
            HFONT hFont = (HFONT)GetProp(hwnd, PROP_UNDERLINE_FONT);
            SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, FALSE);
            InvalidateRect(hwnd, NULL, FALSE);
            SetCapture(hwnd);
        }
        else
        {
            RECT rect;
            GetWindowRect(hwnd, &rect);

            POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            ClientToScreen(hwnd, &pt);

            if (!PtInRect(&rect, pt))
            {
                HFONT hFont = (HFONT)GetProp(hwnd, PROP_ORIGINAL_FONT);
                SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, FALSE);
                InvalidateRect(hwnd, NULL, FALSE);
                ReleaseCapture();
            }
        }
        break;
    }
    case WM_SETCURSOR:
    {
        // Since IDC_HAND is not available on all operating systems,
        // we will load the arrow cursor if IDC_HAND is not present.
        HCURSOR hCursor = LoadCursor(NULL, IDC_HAND);
        if (NULL == hCursor)
            hCursor = LoadCursor(NULL, IDC_ARROW);
        SetCursor(hCursor);
        return TRUE;
    }
    }

    return CallWindowProc(pfnOrigProc, hwnd, message, wParam, lParam);
}

/* End of SSpoke's code */