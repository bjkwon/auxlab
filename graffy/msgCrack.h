// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: graffy
// Graphic Library (Windows only)
// 
// 
// Version: 1.495
// Date: 12/13/2018
// 

//From windowsx.h
// but with WIn32++, this was modified to resemble the MFC interface.

#define xt2CPoint(xpos,ypos) CPoint((xpos),(ypos))

#ifdef _WIN64
#define MSGRESULT DWLP_MSGRESULT
#else
#define MSGRESULT DWL_MSGRESULT
#endif

#define     SetDlgMsgResult(hwnd, msg, result) (( \
        (msg) == WM_CTLCOLORMSGBOX      || \
        (msg) == WM_CTLCOLOREDIT        || \
        (msg) == WM_CTLCOLORLISTBOX     || \
        (msg) == WM_CTLCOLORBTN         || \
        (msg) == WM_CTLCOLORDLG         || \
        (msg) == WM_CTLCOLORSCROLLBAR   || \
        (msg) == WM_CTLCOLORSTATIC      || \
        (msg) == WM_COMPAREITEM         || \
        (msg) == WM_VKEYTOITEM          || \
        (msg) == WM_CHARTOITEM          || \
        (msg) == WM_QUERYDRAGICON       || \
        (msg) == WM_INITDIALOG             \
    ) ? (BOOL)(result) : (SetWindowLong((hwnd), MSGRESULT, (LPARAM)(LRESULT)(result)), TRUE))

#define chHANDLE_DLGMSG(hwnd, message, fn)                 \
   case (message): return (SetDlgMsgResult(hwnd, umsg,     \
      HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

//Modified message handler.
#define HANDLE_WM_COMMAND(hwnd, wParam, lParam, fn) \
    ((fn)((int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam)), 0L)


/* BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) */
#define HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(BOOL)(fn)((hwnd), (HWND)(wParam), lParam)


#define HANDLE_WM_PAINT(hwnd, wParam, lParam, fn) \
    ((fn)(), 0L)
#define HANDLE_WM_CTLCOLORSTATIC(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(HBRUSH)(fn)((HDC)(wParam), (HWND)(lParam), CTLCOLOR_STATIC)
#define HANDLE_WM_CLOSE(hwnd, wParam, lParam, fn) \
    ((fn)(), 0L)
#define HANDLE_WM_SIZE(hwnd, wParam, lParam, fn) \
    ((fn)((UINT)(wParam), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)
#define HANDLE_WM_DESTROY(hwnd, wParam, lParam, fn) \
    ((fn)(), 0L)

/* void Cls_OnKeyDown(UINT vk, int cRepeat, UINT flags) */
#define HANDLE_WM_KEYDOWN(hwnd, wParam, lParam, fn) \
    ((fn)((UINT)(wParam), (int)(short)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)
/* void Cls_OnKeyUp(UINT vk, int cRepeat, UINT flags) */
#define HANDLE_WM_KEYUP(hwnd, wParam, lParam, fn) \
    ((fn)((UINT)(wParam), (int)(short)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)

/* void Cls_OnMouseMove(UINT keyFlags, CPoint cpt) */
#define HANDLE_WM_MOUSEMOVE(hwnd, wParam, lParam, fn) \
    ((fn)((UINT)(wParam), xt2CPoint((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam))), 0L)

/* void Cls_OnWindowPosChanged(const LPWINDOWPOS lpwpos) */
#define HANDLE_WM_WINDOWPOSCHANGED(hwnd, wParam, lParam, fn) \
    ((fn)((const LPWINDOWPOS)(lParam)), 0L)

/* void Cls_OnMove(int x, int y) */
#define HANDLE_WM_MOVE(hwnd, wParam, lParam, fn) \
    ((fn)((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam)), 0L)


/* void Cls_OnLButtonDown(UINT keyFlags, CPoint cpt) */
#define HANDLE_WM_LBUTTONDOWN(hwnd, wParam, lParam, fn) \
    ((fn)((UINT)(wParam), xt2CPoint((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam))), 0L)

/* void Cls_OnLButtonDown(UINT keyFlags, CPoint cpt) */
#define HANDLE_WM_LBUTTONDBLCLK(hwnd, wParam, lParam, fn) \
    ((fn)((UINT)(wParam), xt2CPoint((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam))), 0L)

/* void Cls_OnLButtonUp(UINT keyFlags, CPoint cpt) */
#define HANDLE_WM_LBUTTONUP(hwnd, wParam, lParam, fn) \
    ((fn)((UINT)(wParam), xt2CPoint((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam))), 0L)

/* void Cls_OnRButtonDown(UINT keyFlags, CPoint cpt) */
#define HANDLE_WM_RBUTTONDOWN(hwnd, wParam, lParam, fn) \
    ((fn)((UINT)(wParam), xt2CPoint((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam))), 0L)

/* void Cls_OnRButtonDown(UINT keyFlags, CPoint cpt) */
#define HANDLE_WM_RBUTTONDBLCLK(hwnd, wParam, lParam, fn) \
    ((fn)((UINT)(wParam), xt2CPoint((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam))), 0L)

/* void Cls_OnRButtonUp(UINT keyFlags, CPoint cpt) */
#define HANDLE_WM_RBUTTONUP(hwnd, wParam, lParam, fn) \
    ((fn)((UINT)(wParam), xt2CPoint((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam))), 0L)

/* void Cls_OnActivate(UINT state, HWND hwndActDeact, BOOL fMinimized) */
#define HANDLE_WM_ACTIVATE(hwnd, wParam, lParam, fn) \
    ((fn)((UINT)LOWORD(wParam), (HWND)(lParam), (BOOL)HIWORD(wParam)), 0L)
/* BOOL Cls_OnNCActivate(BOOL fActive) */
#define HANDLE_WM_NCACTIVATE(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(BOOL)((BOOL)(wParam)

/* void Cls_OnTimer(HWND hwnd, UINT id) */
#define HANDLE_WM_TIMER(hwnd, wParam, lParam, fn) \
    ((fn)((UINT)(wParam)), 0L)


// void OnSoundEvent(int index, int code) // code: 1 for done, -1 for error (for error, index indicates the error code)
#define HANDLE_WM__SOUND_EVENT(hwnd, wParam, lParam, fn) \
    ((fn)((int)lParam, (int)wParam), 0L)


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

#define HANDLE_DM_GETDEFID(hwnd, wParam, lParam, fn) \
    ((fn)(), 0L)

