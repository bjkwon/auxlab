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

int printf_INI (char *errstr, const char *fname, const char *heading, const char * szFormat, ...)
{
	static char szBuffer[MAX_CHAR];
	char local_errstr[128], head[MAX_HEADING_LENGTH];
	int j ;
	size_t filesize;
	FILE *fp;
	va_list pArgList ;

	sprintf (head, "[%s]", heading);
	va_start (pArgList, szFormat) ;
	j=vsprintf (szBuffer, szFormat, pArgList) ;
	va_end (pArgList) ;
	string newstring = szBuffer;

	if (strstr(szBuffer, "\n[")) {
		memmove(szBuffer+5, szBuffer, strlen(szBuffer)+1);
		strncpy(szBuffer, "//{\r\n", 5);
		strcat(szBuffer, "\r\n//}");
	}
	if (errstr == NULL)
		errstr = local_errstr;
	string readbuffer;
	filesize = GetFileText(fname, "rt", readbuffer);
	if (filesize == 0) {
		strcpy(errstr, "File cannot be inspected for printfINI.");
		return 0;
	}
	size_t headingPos = readbuffer.find(head);
	size_t writePos(-1);
	if (headingPos == string::npos) // not found.... append the item to the end
	{
		readbuffer += head;
	}
	else
	{ // does the head appear again, then return err
		if (readbuffer.find(head, headingPos+1)==string::npos)
		{
			sprintf(errstr, "The heading %s appears more than once.", head);
			return 0;
		}
		// find the next head block
		size_t nextHeadPos = readbuffer.find('[', headingPos + 1);
		if (nextHeadPos == string::npos)
		{ // last head block, just replace the body of the block
			readbuffer.replace(headingPos + strlen(head) + 1, string::npos, newstring);
		}
		else
		{
			size_t len2Discard = nextHeadPos - (headingPos + strlen(head)); //length of string to replace
			readbuffer.replace(headingPos + strlen(head) + 1, len2Discard, newstring);
		}
	}
	fp = fopen(fname, "wt");
	if (!fp)
	{
		strcpy(errstr, "File cannot be opened for printfINI.");
		return 0;
	}
	fprintf(fp, readbuffer.c_str());
	fclose(fp);
	return 1;
}
