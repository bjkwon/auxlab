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
#ifndef BJTOOLS
#define BJTOOLS

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

//If this library is linked statically define LINK_STATIC_AUDFRET in the application project.
#ifndef LINK_STATIC_AUDFRET
#define AUDFRET_EXP __declspec (dllexport)
#else 
#define AUDFRET_EXP 
#endif

#define AUD_ERR_FILE_NOT_FOUND			-2
#define AUD_ERR_CANNOT_READ_FILE		-3
#define AUD_ERR_HEADING_NOT_FOUND		-4
#define AUD_ERR_HEADING_FOUND_MULTIPLE	-5

AUDFRET_EXP bool isWin7();
AUDFRET_EXP char* RetrieveVersionString(const char* executableName, char* verStrOut, size_t verStrOutLen);
AUDFRET_EXP char* getVersionString (const char* executableName, char* verStrOut, size_t verStrOutLen);
AUDFRET_EXP char* getVersionStringAndUpdateTitle (HWND hDlg, const char* executableName, char* verStringOut, size_t verStrOutLen);
AUDFRET_EXP void EditPrintf (HWND hwndEdit, const char * szFormat, ...);
AUDFRET_EXP void ClearEditPrintf (HWND hwndEdit);
AUDFRET_EXP int countDeliminators (const char* buf, const char* deliminators);
AUDFRET_EXP char* getfilenameonly(const char* filewpath, char* filename, size_t filenamesize);
AUDFRET_EXP BOOL IsBetween (int x, int begin, int ending);
AUDFRET_EXP int GetMaxInd4Cut (double criterion, int len, double *x);
AUDFRET_EXP void SetDlgItemDouble (HWND hDlg, int id, char *formatstr, double x);
AUDFRET_EXP void GetCurrentProcInfo (char *path, char *procName, char *verStr);
AUDFRET_EXP char* FulfillFile (char* out, const char *path, const char * file);
AUDFRET_EXP int Append2Path(const char *additionalpath, char *errstr);
AUDFRET_EXP void GetPathFileNameFromFull (char *path, char *fname, const char *fullname);

AUDFRET_EXP int sprintfFloat(double f, int max_fp, char *strOut, size_t sizeStrOut);
AUDFRET_EXP int countMarkers(const char *str, char* marker);
AUDFRET_EXP int countstr(const char *str, char* marker);
AUDFRET_EXP int countchar(const char *str, char c);
AUDFRET_EXP int EnableDlgItem (HWND hwnd, int id, int onoff);
AUDFRET_EXP int ShowDlgItem (HWND hwnd, int id, int nCmdShow);
//AUDFRET_EXP int VersionCheck (const char *procName, const char *requiredVersionStr);
AUDFRET_EXP int printfINI (char *errstr, const char *fname, const char *heading, const char * szFormat, ...);
AUDFRET_EXP int ReadINI (char *errstr, const char *fname, const char *heading, char *strout, size_t strLen);
AUDFRET_EXP int ReadINI(char *errstr, const char *fname, const char *heading, std::string &strOut);
AUDFRET_EXP int sscanfINI (char *errstr, const char *fname, const char *heading, const char * szFormat, ...);
AUDFRET_EXP int GetSurroundedBy(char c1, char c2, const char* in, char* out, int iStart);
AUDFRET_EXP vector<int> SpreadEvenly(int whole, int nGroups);

AUDFRET_EXP HANDLE InitPipe (char *PipeName, char *errStr);
AUDFRET_EXP int CallPipe (HWND hDlg, char *pipenode, const char *PipeMsg2Send, char *PipeMsg2Rec, int LenRec, char *errstr);
AUDFRET_EXP int CallPipe2 (HWND hDlg, char *remotePC, char *pipenode, const char *PipeMsg2Send, char *PipeMsg2Rec, int LenRec, char *errstr);

/*Do not use %s format for sscanfINI ---it won't work*/
/* sprintfFloat generates a string array from a double value, its floating point varies depending on the value and specified max_fp  */

AUDFRET_EXP char *trimLeft (char* string, const char *chars2Trimmed);
AUDFRET_EXP char *trimRight (char* string, const char *chars2Trimmed);
AUDFRET_EXP int howmanychr(const char* buffer, const char c);
AUDFRET_EXP int howmanystr(const char* buffer, const char* str);
AUDFRET_EXP short double2short(double x);
AUDFRET_EXP double short2double(short x);
AUDFRET_EXP int mod (int big, int divider);
AUDFRET_EXP int getrand(int x);
AUDFRET_EXP double getexp(double dB);

AUDFRET_EXP void GetLastErrorStr(DWORD ecode, char *errstr);
AUDFRET_EXP void GetLastErrorStr(char *errstr);
AUDFRET_EXP double compFileTime(const char *fname1, const char *fname2);
AUDFRET_EXP int isSameFile(const char *fname1, const char *fname2);
AUDFRET_EXP int compFileLength(const char *fname1, const char *fname2);
AUDFRET_EXP int spyWindowMessage(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam, char* const fname, vector<UINT> msg2show, char* const tagstr);
AUDFRET_EXP int spyWindowMessageExc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam, char* const fname, vector<UINT> msg2excl, char* const tagstr);
AUDFRET_EXP int SpyGetMessage(MSG msg, char* const fname, vector<UINT> msg2show, char* const tagstr);
AUDFRET_EXP int SpyGetMessageExc(MSG msg, char* const fname, vector<UINT> msg2excl, char* const tagstr);


#ifdef __cplusplus
AUDFRET_EXP double GetDlgItemDouble (HWND hDlg, int id, int* lpTrans=NULL);

//The functions below are not really for users--only for str2array
// In order to use str2array, make audstr.h and audstr.cpp part of the project with audfretDLL.lib
AUDFRET_EXP char * copyItem(char * out, char * in);
AUDFRET_EXP int copyItem(int out, char * in);
AUDFRET_EXP double copyItem(double out, char * in);
AUDFRET_EXP char* recoverNULL(int *dummy, char *dest, const char *src);
AUDFRET_EXP char* recoverNULL(float *dummy, char *dest, const char *src);
AUDFRET_EXP char* recoverNULL(double *dummy, char *dest, const char *src);

AUDFRET_EXP size_t str2vect(std::vector<std::string> &out, std::string input, char delim);
AUDFRET_EXP size_t str2vectPlus(std::vector<std::string> &out, std::string input, char delim, char quot);
AUDFRET_EXP size_t str2vect(std::vector<std::string> &out, const char* input, const char *delim, int maxSize_x=0);
AUDFRET_EXP std::string consoldateStr(std::vector<std::string> &input, char delim);

AUDFRET_EXP void trim(string& str, string delim);
AUDFRET_EXP void trim(string& str, char delim);
AUDFRET_EXP void trim(string& str, char* delim);
AUDFRET_EXP int sformat(string& out, const char* format, ...);
AUDFRET_EXP int sformat(string& out, size_t nLengthMax, const char* format, ...);

AUDFRET_EXP void ReplaceStr(string &str, const string& from, const string& to);
AUDFRET_EXP void splitevenindices(vector<unsigned int> &out, unsigned int nTotals, unsigned int nGroups);
AUDFRET_EXP void GetLocalTimeStr(string &strOut);
AUDFRET_EXP int GetFileText(const char *fname, const char *mod, string &strOut);
AUDFRET_EXP int mceil(double x);


#ifndef NO_SOCKET
AUDFRET_EXP int TransSocket (const char *ipa, unsigned short port, const char *PipeMsg2Send, char *PipeMsg2Rec, int LenRec);
#endif //NO_SOCKET

//Obsolete....for compatibility only
AUDFRET_EXP int str2intarray(int* out, int maxSize_x, const char * str, const char *deliminators);


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

class CFileDlg
{
public:
	OPENFILENAME ofn;
	void InitFileDlg(HWND hwnd, HINSTANCE hInst, const char *initDir);
	int FileOpenDlg(LPSTR pstrFileName, LPSTR pstrTitleName, LPSTR szFilter, LPSTR lpstrDefExt);
	int FileSaveDlg(LPSTR pstrFileName, LPSTR pstrTitleName, LPSTR szFilter, LPSTR lpstrDefExt);
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