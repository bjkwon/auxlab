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
#define WM_SHOW_IN_DLG	WM_APP+222
#define WM_SHOW_VARS	WM_APP+223
#define WM_CLOSE_FIG	WM_APP+224
 
//From windowsx.h

#define     SetDlgMsgResult(hwnd, msg, result) (( \
        (msg) == WM_CTLCOLORMSGBOX      || \
        (msg) == WM_CTLCOLOREDIT        || \
        (msg) == WM_CTLCOLORLISTBOX     || \
        (msg) == WM_CTLCOLORBTN         || \
        (msg) == WM_CTLCOLORDLG         || \
        (msg) == WM_CTLCOLORSCROLLBAR   || \
        (msg) == WM_CTLCOLORSTATIC      || \
        (msg) == WM_CTLCOLOREDIT        || \
        (msg) == WM_COMPAREITEM         || \
        (msg) == WM_VKEYTOITEM          || \
        (msg) == WM_CHARTOITEM          || \
        (msg) == WM_QUERYDRAGICON       || \
        (msg) == WM_SYSCOMMAND			|| \
        (msg) == WM_INITDIALOG             \
    ) ? (BOOL)(result) : (SetWindowLongPtr((hwnd), DWLP_MSGRESULT, (LONG)(LPARAM)(LRESULT)(result)), TRUE))

#define chHANDLE_DLGMSG(hwnd, message, fn)                 \
   case (message): return (SetDlgMsgResult(hwnd, umsg,     \
      HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

//Modified message handler.

/* void Cls_OnSetFont(HWND hwndCtl, HFONT hfont, BOOL fRedraw) */
#define HANDLE_WM_SETFONT(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HFONT)(wParam), (BOOL)(lParam)), 0L)

#define HANDLE_WM_COMMAND(hwnd, wParam, lParam, fn) \
    ((fn)((int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam)), 0L)
/* void Cls_OnSysCommand(HWND hwnd, UINT cmd, int x, int y) */
#define HANDLE_WM_SYSCOMMAND(hwnd, wParam, lParam, fn) \
    ((fn)((UINT)(wParam), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)
#define HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(BOOL)(fn)((HWND)(wParam), lParam)
#define HANDLE_WM_CTLCOLORSTATIC(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)((HDC)(wParam), (HWND)(lParam), CTLCOLOR_STATIC)
#define HANDLE_WM_CTLCOLOREDIT(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)((HDC)(wParam), (HWND)(lParam), CTLCOLOR_EDIT)
#define HANDLE_WM_CLOSE(hwnd, wParam, lParam, fn) \
    ((fn)(), 0L)
#define HANDLE_WM_SIZE(hwnd, wParam, lParam, fn) \
    ((fn)((UINT)(wParam), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)
#define HANDLE_WM_MOVE(hwnd, wParam, lParam, fn) \
    ((fn)((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)
#define HANDLE_WM_DESTROY(hwnd, wParam, lParam, fn) \
    ((fn)(), 0L)

//#define HANDLE_WM_NOTIFY(hwnd, wParam, lParam, fn) \
//    ((fn)(hwnd, (UINT)(wParam), lParam), 0L)
/* The above is commented out because commctrl.h has the following definition
#define HANDLE_WM_NOTIFY(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), (int)(wParam), (NMHDR *)(lParam))
Here, it's not ((fn)(...), 0L) and it's just (fn)(...)    Therefore, OnNotify function should not return void.
But it's not really useful anyway... WM_NOTIFY shouldn't be used in a cracker... 5/1/2016 bjkwon
*/

/* void OnShowInDlg(CAstSig *asend, const AstNode *p) */
#define HANDLE_WM_SHOW_IN_DLG(hwnd, wParam, lParam, fn) \
    ((fn)((CAstSig*)wParam, (const AstNode *)lParam), 0L)

/* void OnShowVars(CAstSig *asend, const AstNode *p) */
#define HANDLE_WM_SHOW_VARS(hwnd, wParam, lParam, fn) \
    ((fn)((CAstSig*)wParam, (const AstNode *)lParam), 0L)

/* void OnCloseFig(int figID) */
#define HANDLE_WM_CLOSE_FIG(hwnd, wParam, lParam, fn) \
    ((fn)((int)(wParam)), 0L)

/* void OnTimer(UINT id) */
#define HANDLE_WM_TIMER(hwnd, wParam, lParam, fn) \
    ((fn)((UINT)(wParam)), 0L)


/* void OnShowWindow(BOOL fShow, UINT status) */
#define HANDLE_WM_SHOWWINDOW(hwnd, wParam, lParam, fn) \
    ((fn)((BOOL)(wParam), (UINT)(lParam)), 0L)
#define FORWARD_WM_SHOWWINDOW(hwnd, fShow, status, fn) \
    (void)(fn)((hwnd), WM_SHOWWINDOW, (WPARAM)(BOOL)(fShow), (LPARAM)(UINT)(status))


/* void OnGetMinMaxInfo(LPMINMAXINFO lpMinMaxInfo) */
#define HANDLE_WM_GETMINMAXINFO(hwnd, wParam, lParam, fn) \
    ((fn)((LPMINMAXINFO)(lParam)), 0L)


#define FORWARD_WM_KEYDOWN(hwnd, vk, cRepeat, flags, fn) \
    (void)(fn)((hwnd), WM_KEYDOWN, (WPARAM)(UINT)(vk), MAKELPARAM((cRepeat), (flags)))

/* HWND Cls_OnNextDlgCtl(HWND hwnd, HWND hwndSetFocus, BOOL fNext) */
#define HANDLE_WM_NEXTDLGCTL(hwnd, wParam, lParam, fn) \
    (LRESULT)(UINT_PTR)(HWND)(fn)((hwnd), (HWND)(wParam), (BOOL)(lParam))
#define FORWARD_WM_NEXTDLGCTL(hwnd, hwndSetFocus, fNext, fn) \
    (HWND)(UINT_PTR)(fn)((hwnd), WM_NEXTDLGCTL, (WPARAM)(HWND)(hwndSetFocus), (LPARAM)(fNext))

//void OnFlyConnected(HWND hDlg, char *hostname, char* ipa)
#define HANDLE_WM_FL_CONNECTED(hwnd, wParam, lParam, fn) \
    ((fn)((char*)wParam, (char*)lParam), 0L)

//void OnFlyClosed(HWND hDlg)
#define HANDLE_WM_FL_CLOSE(hwnd, wParam, lParam, fn) \
    ((fn)(), 0L)

//void OnFlyArrived(HWND hDlg, WORD code, WORD nBytes, void* inBuffer)
#define HANDLE_WM_FL_ARRIVED(hwnd, wParam, lParam, fn) \
    ((fn)(LOWORD(wParam), HIWORD(wParam), (void*)lParam), 0L)

//void InitDialog2(int fs)
#define HANDLE_WM__INIT(hwnd, wParam, lParam, fn) \
    ((fn)((int)wParam), 0L)


// void OnSoundEvent(CVar *pvar, int code) 
// This is how it's set up in wavplay.cpp
// code: 
//		WOM_OPEN when opening the audio event
//		WOM_CLOSE when closing the audio event
//		-1 if there's an error
// pvar: not used for WOM_OPEN and WOM_CLOSE
//		the pointer to the audio handle for each WOM_DONE message
//		the pointer to the char array if code is -1 ---> Just cast it when using it.

#define HANDLE_WM__AUDIOEVENT(hwnd, wParam, lParam, fn) \
    ((fn)((CVar *)wParam, (int)lParam), 0L)

#define ON_CONTROL(wNotifyCode, id, memberFxn) \
	{ WM_COMMAND, (WORD)wNotifyCode, (WORD)id, (WORD)id, AfxSig_vv, \
		(AFX_PMSG)&memberFxn },

// Static control notification codes
#define ON_STN_CLICKED(id, memberFxn) \
	ON_CONTROL(STN_CLICKED, id, memberFxn)
#define ON_STN_DBLCLK(id, memberFxn) \
	ON_CONTROL(STN_DBLCLK, id, memberFxn)
#define ON_STN_ENABLE(id, memberFxn) \
	ON_CONTROL(STN_ENABLE, id, memberFxn)
#define ON_STN_DISABLE(id, memberFxn) \
	ON_CONTROL(STN_DISABLE, id, memberFxn)


// Edit Control Notification Codes
#define ON_EN_SETFOCUS(id, memberFxn) \
	ON_CONTROL(EN_SETFOCUS, id, memberFxn)
#define ON_EN_KILLFOCUS(id, memberFxn) \
	ON_CONTROL(EN_KILLFOCUS, id, memberFxn)
#define ON_EN_CHANGE(id, memberFxn) \
	ON_CONTROL(EN_CHANGE, id, memberFxn)
#define ON_EN_UPDATE(id, memberFxn) \
	ON_CONTROL(EN_UPDATE, id, memberFxn)
#define ON_EN_ERRSPACE(id, memberFxn) \
	ON_CONTROL(EN_ERRSPACE, id, memberFxn)
#define ON_EN_MAXTEXT(id, memberFxn) \
	ON_CONTROL(EN_MAXTEXT, id, memberFxn)
#define ON_EN_HSCROLL(id, memberFxn) \
	ON_CONTROL(EN_HSCROLL, id, memberFxn)
#define ON_EN_VSCROLL(id, memberFxn) \
	ON_CONTROL(EN_VSCROLL, id, memberFxn)

// User Button Notification Codes
#define ON_BN_CLICKED(id, memberFxn) \
	ON_CONTROL(BN_CLICKED, id, memberFxn)
#define ON_BN_DOUBLECLICKED(id, memberFxn) \
	ON_CONTROL(BN_DOUBLECLICKED, id, memberFxn)
#define ON_BN_SETFOCUS(id, memberFxn) \
	ON_CONTROL(BN_SETFOCUS, id, memberFxn)
#define ON_BN_KILLFOCUS(id, memberFxn) \
	ON_CONTROL(BN_KILLFOCUS, id, memberFxn)

// Listbox Notification Codes
#define ON_LBN_ERRSPACE(id, memberFxn) \
	ON_CONTROL(LBN_ERRSPACE, id, memberFxn)
#define ON_LBN_SELCHANGE(id, memberFxn) \
	ON_CONTROL(LBN_SELCHANGE, id, memberFxn)
#define ON_LBN_DBLCLK(id, memberFxn) \
	ON_CONTROL(LBN_DBLCLK, id, memberFxn)
#define ON_LBN_SELCANCEL(id, memberFxn) \
	ON_CONTROL(LBN_SELCANCEL, id, memberFxn)
#define ON_LBN_SETFOCUS(id, memberFxn) \
	ON_CONTROL(LBN_SETFOCUS, id, memberFxn)
#define ON_LBN_KILLFOCUS(id, memberFxn) \
	ON_CONTROL(LBN_KILLFOCUS, id, memberFxn)

// Check Listbox Notification codes
#define CLBN_CHKCHANGE (40)
#define ON_CLBN_CHKCHANGE(id, memberFxn) \
	ON_CONTROL(CLBN_CHKCHANGE, id, memberFxn)

// Combo Box Notification Codes
#define ON_CBN_ERRSPACE(id, memberFxn) \
	ON_CONTROL(CBN_ERRSPACE, id, memberFxn)
#define ON_CBN_SELCHANGE(id, memberFxn) \
	ON_CONTROL(CBN_SELCHANGE, id, memberFxn)
#define ON_CBN_DBLCLK(id, memberFxn) \
	ON_CONTROL(CBN_DBLCLK, id, memberFxn)
#define ON_CBN_SETFOCUS(id, memberFxn) \
	ON_CONTROL(CBN_SETFOCUS, id, memberFxn)
#define ON_CBN_KILLFOCUS(id, memberFxn) \
	ON_CONTROL(CBN_KILLFOCUS, id, memberFxn)
#define ON_CBN_EDITCHANGE(id, memberFxn) \
	ON_CONTROL(CBN_EDITCHANGE, id, memberFxn)
#define ON_CBN_EDITUPDATE(id, memberFxn) \
	ON_CONTROL(CBN_EDITUPDATE, id, memberFxn)
#define ON_CBN_DROPDOWN(id, memberFxn) \
	ON_CONTROL(CBN_DROPDOWN, id, memberFxn)
#define ON_CBN_CLOSEUP(id, memberFxn)  \
	ON_CONTROL(CBN_CLOSEUP, id, memberFxn)
#define ON_CBN_SELENDOK(id, memberFxn)  \
	ON_CONTROL(CBN_SELENDOK, id, memberFxn)
#define ON_CBN_SELENDCANCEL(id, memberFxn)  \
	ON_CONTROL(CBN_SELENDCANCEL, id, memberFxn)

