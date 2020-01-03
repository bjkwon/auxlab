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

static HWND hEventLogger;

#define MAX_HEADING_LENGTH		128
#define MAX_CHAR 499

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

#define EDITPRINTFBACKUPFILE "backup.log"
static char backupfile[260] = {};
void Append2Edit (HWND hwndEdit, char *str)
{
	int nCountExisting = GetWindowTextLength(hwndEdit);
	SendMessage (hwndEdit, EM_SETSEL, (WPARAM)nCountExisting, (LPARAM)nCountExisting) ;
	SendMessage (hwndEdit, EM_REPLACESEL, FALSE, (LPARAM) str) ;
	if (GetFocus() != hwndEdit)
		SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
}

void ClearEditPrintf (HWND hwndEdit)
{
	LRESULT i;
     i=SendMessage (hwndEdit, EM_SETSEL, (WPARAM) 0, (LPARAM) -1) ;
     i=SendMessage (hwndEdit, EM_REPLACESEL, FALSE, (LPARAM)"") ;
     i=SendMessage (hwndEdit, EM_SCROLLCARET, 0, 0) ;
}

void EditPrintfFileBackup(const char * filename)
{
	strcpy(backupfile, filename);
}

void EditPrintf (HWND hwndEdit, const char * szFormat, ...)
{
	// If hwndEdit already has illegit CRLF (e.g., \r or \n only)
	// This may display the content looking CRLF jumbled.
	// 9/4/2019
	if (strlen(backupfile)==0)
		strcpy(backupfile, EDITPRINTFBACKUPFILE);
	static char szBuffer[MAX_CHAR+1], prevStr[MAX_CHAR+1];
	va_list pArgList ;

	va_start (pArgList, szFormat) ;
	vsprintf (szBuffer, szFormat, pArgList) ;
	va_end (pArgList) ;

	int limit = (int)SendMessage(hwndEdit, EM_GETLIMITTEXT, (WPARAM)0, (LPARAM)0);
	SendMessage(hwndEdit, EM_SETLIMITTEXT, (WPARAM)limit, (LPARAM)0);
	
	//if \n is found but it is not \r\n, it converts to \r\n
	char *p = strchr(szBuffer, '\n');
	int count = 0;
	char *toadd = szBuffer;
	while (p)
	{
		count++;
		p = strchr(p+1, '\n');
	}
	char *newbuf = NULL;
	if (count > 0)
	{
		newbuf = new char[strlen(szBuffer) + count + 1];
		p = szBuffer;
		char *q = newbuf;
		while (p)
		{
			char *p2 = strchr(p, '\n');
			if (p == szBuffer && p2 == p) continue;
			if (p2[-1] == '\r')  continue;
			memcpy(q, p, p2 - p);
			q[p2 - p] = 0;
			strcat(q, "\r\n");
			q += p2 - p + 2;
			p = strchr(p2 + 1, '\n');
		}
		toadd = newbuf;
	}
	int nCountExisting = GetWindowTextLength(hwndEdit);
	int size2add = (int)strlen(toadd);
	int excess = nCountExisting + size2add - limit;
	if (excess > 0)
	{
		int nChar2Delete = excess + 1; 
		if (size2add > limit)
			toadd += excess;
		SendMessage(hwndEdit, EM_SETSEL, (WPARAM)0, (LPARAM)nChar2Delete);
		SendMessage(hwndEdit, EM_REPLACESEL, FALSE, (LPARAM)"");
		if (backupfile[0])
		{
			FILE *fp = fopen(backupfile, "ab");
			if (fp)
			{
				fwrite(prevStr, 1, nChar2Delete, fp);
				fclose(fp);
			}
		}
	}
	Append2Edit(hwndEdit, toadd);
	if (newbuf) delete[] newbuf;
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

bool IsEventLoggerReady()
{
	return hEventLogger != NULL;
}

void setHWNDEventLogger(HWND hEL)
{
	hEventLogger = hEL;
}

void sendtoEventLogger(char *str, FILETIME * pout)
{
	if (!hEventLogger) return;
	char sendbuffer[4096];
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	sprintf(sendbuffer, "%d[%02d:%02d:%02d:%02d] ", GetCurrentThreadId(), lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
	strcat(sendbuffer, str);
	COPYDATASTRUCT cd;
	cd.lpData = sendbuffer;
	cd.dwData = (ULONG_PTR)&cd;
	cd.cbData = (DWORD)strlen((char*)sendbuffer) + 1;
	SendMessage(hEventLogger, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cd);
	if (pout)
		SystemTimeToFileTime(&lt, pout);
}

void GetLocalTimeStr(string &strOut)
{
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	sformat(strOut, "[%02d/%02d/%4d, %02d:%02d:%02d]", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
}

#ifndef NO_SOCKET
int TransSocket(const char *ipa, unsigned short port, const char *PipeMsg2Send, char *PipeMsg2Rec, int LenRec)
{
	// returns the number of bytes successfully received (including the null char)
	// if error occurs, a negative value returns (error code with a negative sign)
	char RemoteAddress[64];
	int res;
	SOCKET sock;
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.S_un.S_addr = inet_addr(ipa);
	try {
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET) throw - 1;
		if (connect(sock, (SOCKADDR *)&sa, sizeof(sa)))
		{
			HOSTENT *hont = gethostbyname(ipa);
			if (hont == NULL) throw - 2; // HOST_NAME_CANNOT_RESOLVE;
			struct in_addr hostAddress;
			hostAddress.S_un.S_addr = *(u_long*)(hont->h_addr_list[0]);
			strcpy(RemoteAddress, inet_ntoa(hostAddress));
			sa.sin_addr.S_un.S_addr = inet_addr(RemoteAddress);
			if (connect(sock, (SOCKADDR *)&sa, sizeof(sa))) throw - 3;
		}
		if ((res = send(sock, PipeMsg2Send, (int)strlen(PipeMsg2Send) + 1, 0)) == SOCKET_ERROR) throw - 999;
		if ((res = recv(sock, PipeMsg2Rec, LenRec, 0)) == SOCKET_ERROR) throw - 999;
		if ((res = closesocket(sock)) == SOCKET_ERROR) throw - 999;
		return (int)strlen(PipeMsg2Rec) + 1;
	}
	catch (int ecode)
	{
		res = WSAGetLastError();
		if (ecode < -1) closesocket(sock);
		return -res;
	}
}
#endif //NO_SOCKET

