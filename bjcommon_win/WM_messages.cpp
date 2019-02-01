// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: audfret
// Miscellaneous Support Library
// 
// 
// Version: 1.4961
// Date: 12/17/2018
// 
#include "bjcommon.h"
#include "bjcommon_win.h"

#include <map>
#include <vector>
#include <string>

using namespace std;

map<unsigned int, string> wmstr;

void makewmstr()
{
	wmstr.clear();
	wmstr[1] = "WM_CREATE";
	wmstr[2] = "WM_DESTROY";
	wmstr[3] = "WM_MOVE";
	wmstr[5] = "WM_SIZE";
	wmstr[6] = "WM_ACTIVATE";
	wmstr[7] = "WM_SETFOCUS";
	wmstr[8] = "WM_KILLTFOCUS";
	wmstr[0x0A] = "WM_ENABLE";
	wmstr[0x0B] = "WM_SETREDRAW";
	wmstr[0x0C] = "WM_SETTEXT";
	wmstr[0x0D] = "WM_GETTEXT";
	wmstr[0x0F] = "WM_PAINT";
	wmstr[0x10] = "WM_CLOSE";
	wmstr[0x12] = "WM_QUIT";
	wmstr[0x14] = "WM_ERASEBKGND";
	wmstr[0x18] = "WM_SHOWWINDOW";
	wmstr[0x1c] = "WM_ACTIVATEAPP";
	wmstr[0x20] = "WM_SETCURSOR";
	wmstr[0x21] = "WM_MOUSEACTIVATE";
	wmstr[0x24] = "WM_GETMINMAXINFO";
	wmstr[0x30] = "WM_SETFONT";
	wmstr[0x3D] = "WM_GETOBJECT";
	wmstr[0x31f] = "WM_DWMNCRENDERINGCHANGED";
	wmstr[0x4E] = "WM_NOTIFY";
	wmstr[0x55] = "WM_NOTIFYFORMAT";
	wmstr[0x7F] = "WM_GETICON";
	wmstr[0x80] = "WM_SETICON";
	wmstr[0x81] = "WM_NCCREATE";
	wmstr[0x82] = "WM_NCDESTROY";
	wmstr[0x83] = "WM_NCCALCSIZE";
	wmstr[0x84] = "WM_NCHITTEST";
	wmstr[0x85] = "WM_NCPAINT";
	wmstr[0x86] = "WM_NCACTIVATE";
	wmstr[0x87] = "WM_GETDLGCODE";
	wmstr[0x47] = "WM_WINDOWPOSCHANGED";
	wmstr[0x46] = "WM_WINDOWPOSCHANGING";
	wmstr[0xA0] = "WM_NCMOUSEMOVE";
	wmstr[0xA1] = "WM_NCLBUTTONDOWN";
	wmstr[0x100] = "WM_KEYDOWN";
	wmstr[0x101] = "WM_KEYUP";
	wmstr[0x102] = "WM_CHAR";
	wmstr[0x104] = "WM_SYSKEYDOWN";
	wmstr[0x105] = "WM_SYSKEYUP";
	wmstr[0x106] = "WM_SYSCHAR";
	wmstr[0x110] = "WM_INITDIALOG";
	wmstr[0x111] = "WM_COMMAND";
	wmstr[0x113] = "WM_TIMER";
	wmstr[0x127] = "WM_CHANGEUISTATE";
	wmstr[0x128] = "WM_UPDATEUISTATE";
	wmstr[0x129] = "WM_QUERYUISTATE";
	wmstr[0x133] = "WM_CTLCOLOREDIT";
	wmstr[0x135] = "WM_CTLCOLORBTN";
	wmstr[0x136] = "WM_CTLCOLORDLG";
	wmstr[0x138] = "WM_CTLCOLORSTATIC";
	wmstr[0x112] = "WM_SYSCOMMAND";
	wmstr[0xAB] = "WM_NCXBUTTONDOWN";
	wmstr[0xAC] = "WM_NCXBUTTONUP";
	wmstr[0x2A2] = "WM_NCMOUSELEAVE";
	wmstr[0x215] = "WM_CAPTURECHANGED";
	wmstr[0x216] = "WM_MOVING";
	wmstr[0x200] = "WM_MOUSEMOVE";
	wmstr[0x201] = "WM_LBUTTONDOWN";
	wmstr[0x202] = "WM_LBUTTONUP";
	wmstr[0x203] = "WM_LBUTTONDBLCLK";
	wmstr[0x204] = "WM_RBUTTONDOWN";
	wmstr[0x205] = "WM_RBUTTONUP";
	wmstr[0x206] = "WM_RBUTTONDBLCLK";
	wmstr[0x210] = "WM_PARENTNOTIFY";
	wmstr[0x231] = "WM_ENTERSIZEMOVE";
	wmstr[0x281] = "WM_IME_SETCONTEXT";
	wmstr[0x282] = "WM_IME_NOTIFY";
	wmstr[0x400] = "WM_USER";
	wmstr[0x0800] = "WM_APP";
}

int spyWindowMessage(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam, char* const fname, vector<UINT> msg2show, char* const tagstr)
{
	FILE *fp = fopen(fname, "at");
	if (!fp) return 0;
	for (auto it : msg2show)
	{
		if (umsg == it)
		{
			fprintf(fp, "%shDlg %x: msg: 0x%04x %s, wParam=%x, lParam=%x\n", tagstr, (INT_PTR)hDlg, umsg, wmstr[umsg].c_str(), wParam, lParam);
			fclose(fp);
			return 1;
		}
	}
	return -1;
}

int spyWindowMessageExc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam, char* const fname, vector<UINT> msg2excl, char* const tagstr)
{
	if (!wmstr.size()) makewmstr();
	FILE *fp = fopen(fname, "at");
	if (!fp) return 0;
	for (auto it : msg2excl)
		if (umsg == it)	return -1;
	fprintf(fp, "%s %x: msg: 0x%04x %s, wParam=%x, lParam=%x\n", tagstr, (INT_PTR)hDlg, umsg, wmstr[umsg].c_str(), wParam, lParam);
	fclose(fp);
	return 1;
}

int SpyGetMessage(MSG msg, char* const fname, vector<UINT> msg2show, char* const tagstr)
{
	return spyWindowMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam, fname, msg2show, tagstr);
}

int SpyGetMessageExc(MSG msg, char* const fname, vector<UINT> msg2excl, char* const tagstr)
{
	return spyWindowMessageExc(msg.hwnd, msg.message, msg.wParam, msg.lParam, fname, msg2excl, tagstr);
}
