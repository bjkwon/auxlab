// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: audfret
// Miscellaneous Support Library
// 
// 
// Version: 1.4951
// Date: 12/14/2018
// Change from 1.495: CString, CRect not used-->no need to include wxx_wincore.h
#ifndef BJTOOLS_WIN
#define BJTOOLS_WIN

#ifndef _MFC_VER // If MFC is not used.
#include <windows.h>
#else 
#include "afxwin.h"
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

#define AUD_ERR_FILE_NOT_FOUND			-2
#define AUD_ERR_CANNOT_READ_FILE		-3
#define AUD_ERR_HEADING_NOT_FOUND		-4
#define AUD_ERR_HEADING_FOUND_MULTIPLE	-5

bool isWin7();
char* RetrieveVersionString(const char* executableName, char* verStrOut, size_t verStrOutLen);
char* getVersionString (const char* executableName, char* verStrOut, size_t verStrOutLen);
char* getVersionStringAndUpdateTitle (HWND hDlg, const char* executableName, char* verStringOut, size_t verStrOutLen);
void EditPrintf (HWND hwndEdit, const char * szFormat, ...);
void EditPrintfFileBackup (const char * filename);
void ClearEditPrintf (HWND hwndEdit);
void SetDlgItemDouble (HWND hDlg, int id, char *formatstr, double x);
void GetCurrentProcInfo (char *path, char *procName, char *verStr);
int Append2Path(const char *additionalpath, char *errstr);

int EnableDlgItem (HWND hwnd, int id, int onoff);
int ShowDlgItem (HWND hwnd, int id, int nCmdShow);
int printfINI (char *errstr, const char *fname, const char *heading, const char * szFormat, ...);
int ReadINI (char *errstr, const char *fname, const char *heading, char *strout, size_t strLen);
int ReadINI(char *errstr, const char *fname, const char *heading, std::string &strOut);


HANDLE InitPipe (char *PipeName, char *errStr);
int CallPipe (HWND hDlg, char *pipenode, const char *PipeMsg2Send, char *PipeMsg2Rec, int LenRec, char *errstr);
int CallPipe2 (HWND hDlg, char *remotePC, char *pipenode, const char *PipeMsg2Send, char *PipeMsg2Rec, int LenRec, char *errstr);

/*Do not use %s format for sscanfINI ---it won't work*/
/* sprintfFloat generates a string array from a double value, its floating point varies depending on the value and specified max_fp  */

void GetLastErrorStr(DWORD ecode, char *errstr);
void GetLastErrorStr(char *errstr);
double compFileTime(const char *fname1, const char *fname2);
int isSameFile(const char *fname1, const char *fname2);
int compFileLength(const char *fname1, const char *fname2);
int spyWindowMessage(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam, char* const fname, vector<UINT> msg2show, char* const tagstr);
int spyWindowMessageExc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam, char* const fname, vector<UINT> msg2excl, char* const tagstr);
int SpyGetMessage(MSG msg, char* const fname, vector<UINT> msg2show, char* const tagstr);
int SpyGetMessageExc(MSG msg, char* const fname, vector<UINT> msg2excl, char* const tagstr);
void setHWNDEventLogger(HWND hEL);
void sendtoEventLogger(const char* format, ...);
bool IsEventLoggerReady();

#ifdef __cplusplus
double GetDlgItemDouble (HWND hDlg, int id, int* lpTrans=NULL);

#else
double GetDlgItemDouble (HWND hDlg, int id, int* lpTrans);
#endif

INT_PTR InputBox(
    LPCTSTR szTitle, 
    LPCTSTR szPrompt, 
    LPTSTR szResult, 
    DWORD nResultSize,
    bool bMultiLine = false,
    HWND hwndParent = 0);

#ifndef NO_SOCKET
int TransSocket (const char *ipa, unsigned short port, const char *PipeMsg2Send, char *PipeMsg2Rec, int LenRec);
#endif //NO_SOCKET

class CFileDlg
{
public:
	OPENFILENAME ofn;
	void InitFileDlg(HWND hwnd, HINSTANCE hInst, const char *initDir);
	int FileOpenDlg(LPSTR pstrFileName, LPSTR pstrTitleName, LPCSTR szFilter, LPCSTR lpstrDefExt);
	int FileSaveDlg(LPSTR pstrFileName, LPSTR pstrTitleName, LPCSTR szFilter, LPCSTR lpstrDefExt);
	CFileDlg(void);
	char LastPath[MAX_PATH];
	char InitDir[MAX_PATH];
};

/* A problem noted----
Control ID (nIDDlgItem) is not recognized in applications, because resource.h showing those IDs are not available in the scope when this is compiled...
It will not give you an error other than GetDlgItem returns NULL.....
DO something.... 7/29/2016 bjkwon
*/

class CWndDlg
{
public:
	HWND GetDlgItem(int idc);
	void GetLastErrorStr(char *errstr);
	int SetDlgItemInt(int idc, UINT uValue, BOOL bSigned = 0) const;
	int GetDlgItemInt(int idc, BOOL* lpTranslated = NULL, int bSigned = 0)  const;
	int GetDlgItemText(int idc, std::string& strOut) const;
	int SetDlgItemText(int idc, LPCTSTR str) const;
	int GetDlgItemText(int idc, char *strOut, int len) const;
	BOOL SetWindowText(const char *text);
	int GetWindowText(std::string& strOut) const;
	int GetWindowText(char *strOut, int len) const;

	BOOL GetWindowRect(LPRECT lpRect);
	UINT_PTR SetTimer(UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc = NULL);
	BOOL KillTimer(UINT_PTR nIDEvent);
	BOOL MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint);
	BOOL MoveWindow(LPCRECT lpRect, BOOL bRepaint = TRUE);
	BOOL MoveWindow(HWND hCtrl, LPCRECT lpRect, BOOL bRepaint = TRUE);

	LONG_PTR SetWindowLongPtr(int nIndex, LONG dwNewLong);
	LONG_PTR GetWindowLongPtr(int nIndex);
	void OnCommand(int idc, HWND hwndCtl, UINT event);
	void OnClose();
	BOOL ShowWindow(int nCmdShow);
	virtual int OnInitDialog(HWND hwndFocus, LPARAM lParam);
	BOOL InvalidateRect(CONST RECT *lpRect, BOOL bErase = TRUE);
	BOOL DestroyWindow();

	int PostMessage(UINT Msg, WPARAM wParam = NULL, LPARAM lParam = NULL);
	int MessageBox(LPCTSTR lpszText, LPCTSTR lpszCaption = NULL, UINT nType = MB_OK);
	LRESULT SendMessage(UINT Msg, WPARAM wParam = NULL, LPARAM lParam = NULL);
	LRESULT SendDlgItemMessage(int nIDDlgItem, UINT Msg, WPARAM wParam = NULL, LPARAM lParam = NULL);
	HWND ChildWindowFromPoint(POINT Point);
	int GetDlgItemRect(int IDDlgItem, RECT &rc);

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
private:
	POINT clientPt;
};



#endif // BJTOOLS