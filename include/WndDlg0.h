// WndDlg0.h: interface for the CWndDlg class.
//
//////////////////////////////////////////////////////////////////////

/* This header file has a problem----
Control ID (nIDDlgItem) is not recognized in real cases, because resource.h showing those IDs are not available in the scope when this is compiled...
It will not give you an error other than GetDlgItem returns NULL.....
DO something.... 7/29/2016 bjk

*/


#ifndef WNDDLG
#define WNDDLG

#include "wxx_wincore.h" // Win32++ 8.2. This must be placed prior to <windows.h> to avoid including winsock.h

#include <windows.h>
#include <stdio.h>

//#include "wincore.h" // for CString // Not needed for wxx820

class CWndDlg  
{
public:

	HWND GetDlgItem(int idc);
	void GetLastErrorStr(char *errstr);
	void GetLastErrorStr(CString &errstr);
	int SetDlgItemInt(int idc, UINT uValue, BOOL bSigned=0) const;
	int GetDlgItemInt(int idc, BOOL* lpTranslated=NULL, int bSigned=0)  const;
	int GetDlgItemText(int idc, CString& strOut) const;
	int GetDlgItemText(int idc, std::string& strOut) const;
	int SetDlgItemText(int idc, LPCTSTR str) const;
	int GetDlgItemText(int idc, char *strOut, int len) const;
	BOOL SetWindowText(const char *text); 
	int GetWindowText(CString& strOut) const;
	int GetWindowText(char *strOut, int len) const;

	BOOL GetWindowRect(LPRECT lpRect);
	UINT_PTR SetTimer(UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc=NULL); 
	BOOL KillTimer(UINT_PTR nIDEvent);
	BOOL MoveWindow (int x, int y, int nWidth, int nHeight, BOOL bRepaint);
	BOOL MoveWindow (LPCRECT lpRect, BOOL bRepaint = TRUE);
	BOOL MoveWindow (HWND hCtrl, LPCRECT lpRect, BOOL bRepaint = TRUE);

	LONG_PTR SetWindowLongPtr(int nIndex, LONG dwNewLong);
	LONG_PTR GetWindowLongPtr(int nIndex);
	void OnCommand(int idc, HWND hwndCtl, UINT event);
	void OnClose();
	BOOL ShowWindow(int nCmdShow);
	virtual int OnInitDialog(HWND hwndFocus, LPARAM lParam);
	BOOL InvalidateRect(CONST RECT *lpRect, BOOL bErase = TRUE);
	BOOL DestroyWindow();

	int PostMessage(UINT Msg, WPARAM wParam = NULL, LPARAM lParam = NULL);
	int MessageBox (LPCTSTR lpszText, LPCTSTR lpszCaption = NULL, UINT nType = MB_OK);
	LRESULT SendMessage(UINT Msg, WPARAM wParam = NULL, LPARAM lParam = NULL);
	LRESULT SendDlgItemMessage(int nIDDlgItem, UINT Msg, WPARAM wParam=NULL, LPARAM lParam=NULL);
	HWND ChildWindowFromPoint(POINT Point);
	int GetDlgItemRect(int IDDlgItem, RECT &rc);
	int MoveDlgItem(int id, CRect rt, BOOL repaint);

	CWndDlg* hParent;
	CWndDlg* hActiveChild;
	HWND hDlg;
	HINSTANCE hInst;
	char AppPath[MAX_PATH];
	char AppName[MAX_PATH]; // without extension
	char AppRunName[MAX_PATH]; // with extension
	DWORD dwUser;
	CWndDlg();
	virtual ~CWndDlg();

	virtual void OnStimBegin() { };
	virtual void OnStimEnd()  { } ;

private:
	POINT clientPt;
};


#endif  // WNDDLG

