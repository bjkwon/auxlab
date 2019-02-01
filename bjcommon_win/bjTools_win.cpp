// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: audfret
// Miscellaneous Support Library
// 
// 
// Version: 1.495
// Date: 12/13/2018
// 
#include <math.h>
#include "bjcommon.h"
#include "bjcommon_win.h"

#define MAX_HEADING_LENGTH		128
#define MAX_CHAR 29998

bool isWin7()
{
	char block[4096];
	float val;
	getVersionString("cmd.exe", block, sizeof(block));
	block[4] = 0;
	sscanf(block, "%f", &val);
	if (val<6.2) return true;
	else return false;
}

void getVersionStringFromNumber (const char* verStringIn, char* verStringOut)
{
	int ver[8];
	char buff[32];
	memset(ver, 0, sizeof(ver));
	str2intarray (ver, 4, verStringIn, "., ");
	sprintf(verStringOut, "%d.%d", ver[0], ver[1]);
	if (ver[3] > 0)
	{
		for (int k = 2; k < 4; k++)
			strcat(verStringOut, itoa(ver[k], buff, 10));
	}
	else if (ver[2] > 0)
		strcat(verStringOut, itoa(ver[2], buff, 10));
}

char* RetrieveVersionString(const char* executableName, char* verStrOut, size_t verStrOutLen)
{
	unsigned int len;
	void* lpStr;
	DWORD dwSize, dwReserved;
	if (!(dwSize = GetFileVersionInfoSize(executableName, &dwReserved)))
		return (char*)(verStrOut[0] = '\0');
	void* lpBuffer = calloc(1, dwSize);
	GetFileVersionInfo(executableName, 0, dwSize, lpBuffer);
	if (VerQueryValue(lpBuffer, "\\StringFileInfo\\040904b0\\FileVersion", &lpStr, &len))
		strncpy(verStrOut, (char*)lpStr, verStrOutLen);
	else
		verStrOut[0] = '\0', MessageBox(NULL, "Version information not available", executableName, MB_OK);
	return verStrOut;
}

char* getVersionString(const char* executableName, char* verStrOut, size_t verStrOutLen)
{
	DWORD dwSize, dwReserved;
	unsigned int len;
	void* lpStr;
	if (!(dwSize = GetFileVersionInfoSize(executableName, &dwReserved )))
		return (char*)(verStrOut[0]='\0');
	void* lpBuffer = calloc(1, dwSize);
	GetFileVersionInfo( executableName, 0, dwSize, lpBuffer );
	if (VerQueryValue( lpBuffer, "\\StringFileInfo\\040904b0\\FileVersion", &lpStr, &len ))
		getVersionStringFromNumber ((char*)lpStr, verStrOut);
	else
		verStrOut[0]='\0', MessageBox (NULL, "Version information not available", executableName, MB_OK);
	free(lpBuffer);
	return verStrOut;
}

char* getVersionStringAndUpdateTitle  (HWND hDlg, const char* executableName, char* verStrOut, size_t verStrOutLen)
{
	char zbuf[256];
	getVersionString(executableName, verStrOut, verStrOutLen);
	if (hDlg!=NULL)
	{
		GetWindowText(hDlg, zbuf, sizeof(zbuf));
		strcat (zbuf, " ");
		SetWindowText (hDlg, strcat (zbuf, verStrOut));
	}
	return verStrOut;
}

void EditPrintf0 (HWND hwndEdit, char *str)
{
	char buf[MAX_CHAR+1];
	GetWindowText (hwndEdit, buf, sizeof(buf));
	SendMessage (hwndEdit, EM_SETSEL, (WPARAM)strlen(buf), (LPARAM)strlen(buf)) ;
	 SendMessage (hwndEdit, EM_REPLACESEL, FALSE, (LPARAM) str) ;
	 if (GetFocus()!=hwndEdit)	 SendMessage (hwndEdit, EM_SCROLLCARET, 0, 0) ;//don't understand how this works.
}

void ClearEditPrintf (HWND hwndEdit)
{
	LRESULT i;
     i=SendMessage (hwndEdit, EM_SETSEL, (WPARAM) 0, (LPARAM) -1) ;
     i=SendMessage (hwndEdit, EM_REPLACESEL, FALSE, (LPARAM)"") ;
     i=SendMessage (hwndEdit, EM_SCROLLCARET, 0, 0) ;
}

void EditPrintf (HWND hwndEdit, const char * szFormat, ...)
{
     static char szBuffer[MAX_CHAR+1], prevStr[MAX_CHAR+1];
     va_list pArgList ;
	 size_t size1, size2;
	 int nChar2Delete=0, trimBegin=0;

     va_start (pArgList, szFormat) ;
     vsprintf (szBuffer, szFormat, pArgList) ;
     va_end (pArgList) ;

	 GetWindowText (hwndEdit, prevStr, sizeof(prevStr));
	 size1 = strlen(prevStr);
	 size2 = strlen(szBuffer);
	 if (size2>MAX_CHAR) 
	 {	trimBegin = (int)size2-MAX_CHAR;
		size2 = strlen(szBuffer+trimBegin);
	 }
	 if (size1+size2 > MAX_CHAR) 
	 {	 nChar2Delete = (int)(size1+size2)-MAX_CHAR;
		 ClearEditPrintf (hwndEdit);
		 EditPrintf0 (hwndEdit, &prevStr[nChar2Delete]);
	 }
	 EditPrintf0 (hwndEdit, szBuffer+trimBegin);
}

double GetDlgItemDouble (HWND hDlg, int id, int* lpTrans)
{	
	int res;
	double x;
	char buf[32];
	GetDlgItemText (hDlg, id, buf, sizeof(buf));
	res = sscanf(buf,"%lf", &x);
	if (lpTrans==NULL)
		return x;
	*lpTrans = (res>0);
	return x;
}

void GetCurrentProcInfo (char *path, char *procName, char *verStr)
{
// This was written when I was a novice C programmer. Bad code. 
// Just leave it now. Don't use it until I revise it. 7/7/2016 bjk
	char fullname[512];
 	char drive[16], dir[256], ext[8], buffer[256];
	GetModuleFileName (GetModuleHandle(NULL), fullname, sizeof(fullname));

 	_splitpath(fullname, drive, dir, buffer, ext);
	if (path!=NULL)
		sprintf(path, "%s%s", drive, dir);
	if (procName!=NULL)
		sprintf(procName, "%s%s", buffer, ext);
	if (verStr!=NULL)
		getVersionString(fullname, verStr, sizeof(verStr));
}

int EnableDlgItem (HWND hwnd, int id, int onoff)
{
	return EnableWindow(GetDlgItem(hwnd, id), onoff);
}

int ShowDlgItem (HWND hwnd, int id, int nCmdShow)
{
	return ShowWindow (GetDlgItem(hwnd, id), nCmdShow);
}

int Append2Path(const char *additionalpath, char *errstr)
{
	char *pathstr = new char[MAX_CHAR];
	errstr[0]=0;
	DWORD dwRet = GetEnvironmentVariable("PATH", pathstr, MAX_CHAR);
	if (dwRet==0)
		strcpy(errstr,"environment variable PATH not defined");
	else if(dwRet > MAX_CHAR + strlen(additionalpath)+1 )
	{
		delete[] pathstr;
		pathstr = new char[MAX_CHAR*10];
		dwRet = GetEnvironmentVariable("PATH", pathstr, dwRet);
        if(!dwRet)
        {
            sprintf(errstr, "GetEnvironmentVariable failed (%d)\n", GetLastError());
            return 0;
        }
	}
	if (strstr(pathstr,additionalpath)==NULL)
	{
		strcat(pathstr, ";");
		strcat(pathstr, additionalpath);
		SetEnvironmentVariable("PATH", pathstr);
	}
	delete[] pathstr;
	return 1;
}

void GetLastErrorStr(DWORD ecode, char *errstr)
{
	char  string[256];
	DWORD nchar;

	nchar = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
							NULL, ecode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							(LPTSTR) string, sizeof(string), NULL  );
	if (nchar == 0)
		MessageBox (NULL, "Failed to translate Windows error", "", MB_OK);
	else
	{
		trimRight(string, "\r");
		trimRight(string, "\n");
		strcpy(errstr, string);
	}
	return;
}

void GetLastErrorStr(char *errstr)
{
	GetLastErrorStr(GetLastError(), errstr);
}
