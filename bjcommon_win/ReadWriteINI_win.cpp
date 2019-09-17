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
#include "bjcommon.h"
#include "bjcommon_win.h"
#include <stdio.h>
#include <windows.h>

#define MAX_HEADING_LENGTH		128
#define MAX_CHAR 29998

char firstnonwhitechar(char *str)
{
	//Assuming that str is a pointer from other str function (such as strchr), 
	// this retrieves the first non-white character preceding str
	char *pt=str-1;
	while (pt[0]== ' ' || pt[0]== '\t')
		pt -= 1;
	return pt[0];
}

int printfINI (char *errstr, const char *fname, const char *heading, const char * szFormat, ...)
{
	static char szBuffer[MAX_CHAR];
	char *buffer, local_errstr[128], head[MAX_HEADING_LENGTH];
	char *headingPt, *nextHeadingPt, *restBlock;
	size_t len;
	int j ;
	LONG filesize;
	HANDLE		hFile;
	DWORD		dw;
	va_list pArgList ;

	sprintf (head, "[%s]", heading);
	va_start (pArgList, szFormat) ;
	j=vsprintf (szBuffer, szFormat, pArgList) ;
	va_end (pArgList) ;

	if (strstr(szBuffer, "\n[")) {
		memmove(szBuffer+5, szBuffer, strlen(szBuffer)+1);
		strncpy(szBuffer, "//{\r\n", 5);
		strcat(szBuffer, "\r\n//}");
	}
	if (errstr == NULL)
		errstr = local_errstr;
	if ( (hFile = CreateFile (fname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
				OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ))==INVALID_HANDLE_VALUE) {
		strcpy(errstr,"File cannot be opened for writing.");
		return 0;
	}
	filesize = GetFileSize(hFile, &dw);
	buffer = new char[filesize+1];
	if (!ReadFile (hFile, buffer, filesize+1, &dw, NULL))
	{
		sprintf(errstr, "ReadFile() fails, code=%d", GetLastError()); delete[] buffer; 
		CloseHandle(hFile); 
		return 0;
	}
	buffer[dw]=0;
	headingPt = strstr(buffer, head);
	if (headingPt==NULL) // not found.... append the item
	{
		if ((len = strlen(buffer))>0)
		{
			trimRight(buffer, " \t\r\n"); 
			if (strlen(buffer)+2>len)
			{
				char *newbuffer = new char[len+3];
				memcpy((void*)newbuffer, (void*)buffer, strlen(buffer)+1);
				delete[] buffer;
				buffer = newbuffer;
			}
			strcat(buffer, "\r\n");
			if (SetFilePointer(hFile, NULL, NULL, FILE_BEGIN)==INVALID_SET_FILE_POINTER)
			{
				sprintf(errstr, "SetFilePointer() fails, code=%d", GetLastError()); delete[] buffer; 
				CloseHandle(hFile);
				return 0;
			}
			if (!WriteFile (hFile, buffer, (DWORD)strlen(buffer), &dw, NULL))
			{
				sprintf(errstr, "WriteFile() fails, code=%d", GetLastError()); delete[] buffer; 
				CloseHandle(hFile);
				return 0;
			}
		}
		if (!WriteFile (hFile, strcat(head, " "), (DWORD)strlen(head)+1, &dw, NULL))
		{
			sprintf(errstr, "WriteFile() fails, code=%d", GetLastError()); delete[] buffer; 
			CloseHandle(hFile);
			return 0;
		}
		if (!WriteFile (hFile, strcat(szBuffer, "\r\n"), (DWORD)strlen(szBuffer)+2, &dw, NULL))
		{
			sprintf(errstr, "WriteFile() fails, code=%d", GetLastError());  delete[] buffer; 
			CloseHandle(hFile);
			return 0;
		}
	}
	else
	{ 
		if (strstr(headingPt+1, head)!=NULL)
		{
			sprintf(errstr, "The heading %s appears more than once.", head); delete[] buffer; 
			CloseHandle(hFile);
			return 0;
		}
		nextHeadingPt = strchr(headingPt+1, '[');
		if (nextHeadingPt!=NULL)
		{
			char ch = firstnonwhitechar(nextHeadingPt);
			while (ch!='\n' && ch!='\r')	
			{	if ((nextHeadingPt = strchr(nextHeadingPt+1, '['))==NULL) break;
				ch = firstnonwhitechar(nextHeadingPt); }
		}
		if (nextHeadingPt!=NULL)
		{
			restBlock = new char[buffer-nextHeadingPt+filesize+1];
			strcpy(restBlock, nextHeadingPt);
		}
		else
		{
			restBlock = new char[1];
			restBlock[0]=0;
		}
		SetFilePointer(hFile, (LONG)(headingPt-buffer), NULL, FILE_BEGIN);
		
		if (!WriteFile (hFile, strcat(head, " "), (DWORD)strlen(head)+1, &dw, NULL))
		{
			sprintf(errstr, "WriteFile() fails, code=%d", GetLastError()); delete[] buffer; 
			CloseHandle(hFile); delete[] restBlock;
			return 0;
		}
		if (!WriteFile (hFile, szBuffer, (DWORD)strlen(szBuffer), &dw, NULL))
		{
			sprintf(errstr, "WriteFile() fails, code=%d", GetLastError()); delete[] buffer; 
			CloseHandle(hFile); delete[] restBlock;
			return 0;
		}
		WriteFile (hFile, "\r\n", 2, &dw, NULL);
		if (!WriteFile (hFile, restBlock, (DWORD)strlen(restBlock), &dw, NULL))
		{
			sprintf(errstr, "WriteFile() fails, code=%d", GetLastError()); delete[] buffer; 
			CloseHandle(hFile); delete[] restBlock;
			return 0;
		}
		SetEndOfFile(hFile);
		delete[] restBlock;
	}
	CloseHandle(hFile);
	strcpy(errstr, "");
	delete[] buffer; 
	return 1;
}

int ReadINI (char *errstr, const char *fname, const char *heading, char *strout, size_t strLen)
{
	string tmpstr;
	int res = ReadINI(errstr, fname, heading, tmpstr);
	strncpy(strout, tmpstr.c_str(), strLen);
	strout[strLen-1] = '\0';
	return res;
}

int ReadINI(char *errstr, const char *fname, const char *heading, string &strOut)
{
	char *buffer, head[MAX_HEADING_LENGTH];
	LONG filesize;
	HANDLE		hFile;
	DWORD		dw;
	size_t outputLength;
	char *headingPt, *nextHeadingPt;

	errstr[0]=0;

	sprintf (head, "[%s]", heading);
	if ( (hFile = CreateFile (fname, GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ))==INVALID_HANDLE_VALUE)
	{ sprintf(errstr,"%s not found or error opening.", fname); return AUD_ERR_FILE_NOT_FOUND; }
	filesize = GetFileSize(hFile, &dw);
	buffer = new char[filesize+2];
	if (!ReadFile (hFile, buffer, filesize+1, &dw, NULL))
	{
		sprintf(errstr, "ReadFile() fails, code=%d", GetLastError()); 
		CloseHandle(hFile);
		return AUD_ERR_CANNOT_READ_FILE;
	}
	buffer[dw]=0;
	CloseHandle(hFile);
	headingPt = strstr(buffer, head);
	if (headingPt==NULL) // not found.... 
	{
		sprintf(errstr, "Heading not found: %s", head); delete[] buffer;
		return AUD_ERR_HEADING_NOT_FOUND;
	}
	else
	{
		// trim white spaces from the beginning of data
		for (headingPt+=strlen(head); *headingPt && isspace((unsigned char)*headingPt); headingPt++)
			;
		if (strncmp(headingPt, "//{\r\n", 5) == 0 && (nextHeadingPt=strstr(headingPt, "\r\n//}\r\n"))) 
		{
			headingPt += 5;
			nextHeadingPt += 2;
		} 
		else 
		{
			if (strstr(headingPt, head)!=NULL)
			{
				sprintf(errstr, "The heading %s appears more than once.", head); delete[] buffer; 
				return AUD_ERR_HEADING_FOUND_MULTIPLE;
			}
			nextHeadingPt = strchr(headingPt, '[');
			// if the first non-white space character preceding nextHeadingPt is not CR or LF, keep searching [
			if (nextHeadingPt!=NULL)
			{
				char ch = firstnonwhitechar(nextHeadingPt);
				while (ch!='\n' && ch!='\r')	
				{	if ((nextHeadingPt = strchr(nextHeadingPt+1, '['))==NULL) break;
				ch = firstnonwhitechar(nextHeadingPt); }
			}
		}
		if (nextHeadingPt == NULL)
			nextHeadingPt = headingPt + strlen(headingPt);
		// trim white spaces from the end of data
		for (nextHeadingPt-=1; *nextHeadingPt && isspace((unsigned char)*nextHeadingPt) && nextHeadingPt>=headingPt; nextHeadingPt--)
			;
		if ((outputLength = nextHeadingPt+1-headingPt))
			strOut = string(headingPt, outputLength);
		else {
			strOut.clear();
			strcpy(errstr, "(empty string)");
		}
		delete[] buffer;
		return (int)outputLength;
	}
}
