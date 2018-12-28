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
#include "audfret.h"
#include <stdio.h>
#include <windows.h>

#define MAX_HEADING_LENGTH		128
#define MAX_CHAR 29998

char* deeblank (char* stringarray, char deliminator)
{
	unsigned int i, j, temp=0;
	char *dump;

	size_t len1 = strlen(stringarray);

	dump = (char*)calloc (len1+1,1);

	i=0; // index for dump
	j=0; // moving index for stringarray
	while (j<len1)
	{
		while (stringarray[j]==deliminator)
			j++; // j : the address of the next non-deliminator
		memcpy(dump+i, stringarray+j, 1);
		i++;    j++;
	}
	strcpy (stringarray, dump);
	free(dump);
	return stringarray;
}

int howmanychr(const char* buffer, const char c)
{
	const char *pt;
	int res=0;

	pt = strchr (buffer, (int)c);

	while (pt!=NULL)
	{
		res++;
		pt = strchr (pt+1, (int)c);
	}
	return res;
}

int howmanystr(const char* buffer, const char* str)
{
	const char *pt;
	int res=0;

	pt = strstr (buffer, str);

	while (pt!=NULL)
	{
		res++;
		pt = strstr (pt+strlen(str), str);
	}
	return res;
}

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


int sscanfINI (char *errstr, const char *fname, const char *heading, const char * szFormat, ...)
{
	//This works only for non-string sscanf--for string extraction, use ReadItemFromINI
	// Returns -1 (EOF) for sscanf error, other negative int for other errors. 0 for unsuccessful sscanf conversion (up to you whether to consider it as an error)
	int res;
	DWORD dw;

	HANDLE	hFile = CreateFile (fname, GENERIC_READ, FILE_SHARE_READ, NULL,	OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile==INVALID_HANDLE_VALUE)  { sprintf(errstr,"%s cannot be opened for reading.", fname); return AUD_ERR_FILE_NOT_FOUND; }
	LONG filesize = GetFileSize(hFile, &dw);
	CloseHandle(hFile);
	char *buffer = new char[filesize+10];
	if ((res=ReadINI(errstr, fname, heading, buffer, filesize+9))<=0) {delete[] buffer; return res;}

	va_list pl;
	int *list[36];
	va_start (pl, szFormat) ;
	for (int i=0; i<36; i++)	list[i] = va_arg(pl, int*);
	res = sscanf(buffer, szFormat, list[0], list[1], list[2], list[3], list[4], list[5], list[6],
		list[7], list[8], list[9], list[10], list[11], list[12], list[13], list[14], list[15],
		list[16], list[17], list[18], list[19], list[20], list[21], list[22], list[23], list[24], list[25],
		list[26], list[27], list[28], list[29], list[30], list[31], list[32], list[33], list[34], list[35]);
	va_end (pl) ;
	if (res==EOF)	sprintf(errstr, "sscanf returned EOF for %s", heading);
	delete[] buffer;
	return res;
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

//Obsolete functions... only for backward compatibility purposes

int readINI2Buffer (char *errstr, const char *fname, const char *heading, char * dynamicBuffer, int length)
{
	return ReadINI(errstr, fname, heading, dynamicBuffer, length);
}

int strscanfINI (char *errstr, const char *fname, const char *heading, char *strBuffer, size_t strLen)
{
	return ReadINI(errstr, fname, heading, strBuffer, strLen);
}

int ReadItemFromINI (char *errstr, const char *fname, const char *heading, char *strout, size_t strLen)
{
	return ReadINI(errstr, fname, heading, strout, strLen);
}