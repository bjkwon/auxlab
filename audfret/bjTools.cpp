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
#include "audfret.h"

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

int countFloatingPoints(double f, int max_fp)
{
	int i, tens;
	double err, errCrit=1.;

	for (i=0; i<max_fp; i++)
		errCrit /= 10.;
	tens = 1;
	for (i=0; i<max_fp; i++)
	{
		err = (f*tens - (double)(__int64)(f*tens)) / tens;
		if (err<errCrit)
			return i;
		tens *= 10;
	}
	return i;
}

AUDFRET_EXP char* RetrieveVersionString(const char* executableName, char* verStrOut, size_t verStrOutLen)
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

AUDFRET_EXP char* getVersionString(const char* executableName, char* verStrOut, size_t verStrOutLen)
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

AUDFRET_EXP char* getVersionStringAndUpdateTitle  (HWND hDlg, const char* executableName, char* verStrOut, size_t verStrOutLen)
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
//
//AUDFRET_EXP int VersionCheck (const char *procName, const char *requiredVersionStr)
//{
//	char ver[64];
//
//	getVersionString (procName, ver, sizeof(ver));
//	if (strlen(ver)==0)		return 0;
//	// The first character of ver is v...
//	if (strcmp(&ver[1],requiredVersionStr)<0)		return 0;
//	else		return 1;
//}

AUDFRET_EXP int countDeliminators (const char* buf, const char* deliminators)
{
	char *token, *newBuffer;
	int i=0;
	newBuffer = new char[strlen(buf)+1];
	strcpy(newBuffer, buf);

	token = strtok( newBuffer, deliminators );
	while( token != NULL )
	{
		i++;
		token = strtok( NULL, deliminators );
	}
	delete[] newBuffer;
	return i; 
}

AUDFRET_EXP char* getfilenameonly(const char* filewpath, char* filename, size_t filenamesize)
{
	char fn[256], et[256];
	char *tp = new char[strlen(fn)+strlen(et)+1];
	strcpy(tp, fn);
	strcat(tp, et);
	strncpy(filename, tp, filenamesize-1);
	filename[filenamesize]=0;
	delete[] tp;
	return filename;
}

AUDFRET_EXP vector<int> SpreadEvenly(int whole, int nGroups)
{
	vector<int> out;
	int perGroup = whole / nGroups;
	out.assign(nGroups, perGroup);
	int remainder = whole - nGroups * perGroup;
	double remainderPerGroup = (double)remainder / nGroups;
	int k = 0;
	double cumRemainder = remainderPerGroup;
	for (auto it = out.begin(); it != out.end(); it++)
	{
		k++;
		if (cumRemainder >= .5)
		{
			(*it)++; cumRemainder--;
		}
		else
			cumRemainder += remainderPerGroup;
	}
	return out;
}

AUDFRET_EXP int countchar(const char *str, char c)
{
	int count(-1);
	const char *ploc(str), *ploc2(str);
	while (ploc2!=NULL)
	{
		ploc2 = strchr(ploc, c);
		if (ploc2!=NULL)	ploc = ploc2+1;
		count++;
	}
	return count;
}

AUDFRET_EXP int countstr(const char *str, char* marker)
{
	int count(-1);
	const char *ploc(str), *ploc2(str);
	while (ploc2!=NULL)
	{
		ploc2 = strstr(ploc, marker);
		if (ploc2!=NULL)	ploc = ploc2+1;
		count++;
	}
	return count;
}

AUDFRET_EXP int countMarkers(const char *str, char* marker)
{
	//HAVEN'T VERIFIED 04/11/2009
	int i,b=1;
	size_t len, lenMarker;
	char *ptFound, *ptCurrent, *newBuffer;
	if (marker==NULL)
		return -1;
	i=0;
	lenMarker = strlen(marker);
	len = strlen(str);
	newBuffer = (char*)calloc(len+1, sizeof(char));
	strcpy(newBuffer, str);
	ptCurrent=newBuffer;
	while (b)
	{
		ptFound = strstr(ptCurrent, marker);
		if (ptFound==ptCurrent)
		{
			ptCurrent = ptFound + lenMarker;
		}
		else 
		{
			if (ptFound==NULL)
			{
				ptFound = ptCurrent + strlen(ptCurrent);
				b=0;
			}
			if (ptCurrent[0]!='\0')
				i++;
			ptCurrent = ptFound + lenMarker;
		}
	}
	free(newBuffer);
	return i;
}

void EditPrintf0 (HWND hwndEdit, char *str)
{
	char buf[MAX_CHAR+1];
	GetWindowText (hwndEdit, buf, sizeof(buf));
	SendMessage (hwndEdit, EM_SETSEL, (WPARAM)strlen(buf), (LPARAM)strlen(buf)) ;
	 SendMessage (hwndEdit, EM_REPLACESEL, FALSE, (LPARAM) str) ;
	 if (GetFocus()!=hwndEdit)	 SendMessage (hwndEdit, EM_SCROLLCARET, 0, 0) ;//don't understand how this works.
}

AUDFRET_EXP void ClearEditPrintf (HWND hwndEdit)
{
	LRESULT i;
     i=SendMessage (hwndEdit, EM_SETSEL, (WPARAM) 0, (LPARAM) -1) ;
     i=SendMessage (hwndEdit, EM_REPLACESEL, FALSE, (LPARAM)"") ;
     i=SendMessage (hwndEdit, EM_SCROLLCARET, 0, 0) ;
}

AUDFRET_EXP void EditPrintf (HWND hwndEdit, const char * szFormat, ...)
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

AUDFRET_EXP BOOL IsBetween (int x, int begin, int ending)
{
	// inclusive the edge points
	if (x>=begin && x<=ending)
		return 1;
	else 
		return 0;
}

AUDFRET_EXP void SetDlgItemDouble (HWND hDlg, int id, char *formatstr, double x)
{
	char buf[256];
	sprintf (buf, formatstr, x);
	SetDlgItemText (hDlg, id, buf);
}

AUDFRET_EXP double GetDlgItemDouble (HWND hDlg, int id, int* lpTrans)
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

AUDFRET_EXP char * CutEndSpace(char * str, char c)
{
	int i;
	for (i=(int)strlen(str)-1; i>0; i--)
	{
		if (str[i]==c)
			str[i]='\0';
		else
			return str;
	}
	return str;
}

void sprintfFloat(char *strOut, double f, int max_fp)
{
	int fp;
	char *temp, format[64];

	if (f<0)
	{
		sprintfFloat(strOut, -f, max_fp);
		temp = (char*)calloc(strlen(strOut)+2,1);
		strcpy(temp,"-");
		strcat(temp+1, strOut);
		strcpy(strOut, temp);
		free(temp);
		return;
	}

	if ((fp=countFloatingPoints(f, max_fp))==0)
		sprintf (strOut, "%d", (int)f);
	else
	{
		sprintf (format, "%%%d.%df", fp+3, fp);
		sprintf (strOut, format, f);
	}
	trimLeft(strOut, " ");
}

AUDFRET_EXP int sprintfFloat(double f, int max_fp, char *strOut, size_t sizeStrOut)
{
	char buffer[128];
	if (f>1.e10 || f<-1.e10) {	memset((void*)strOut,'*', sizeStrOut-2); strOut[sizeStrOut-1]=0; return 0;}
	sprintfFloat (buffer, f, max_fp);
	if (strlen(buffer)>sizeStrOut-1)
	{
		if (sizeStrOut>1) 
		{	memset((void*)strOut,'*', sizeStrOut-2); strOut[sizeStrOut-1]=0;}
		else if (sizeStrOut==1) 
			strOut[sizeStrOut-1]=0;
		return 0;
	}
	strcpy(strOut, buffer);
	return 1;
}

AUDFRET_EXP void GetCurrentProcInfo (char *path, char *procName, char *verStr)
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

AUDFRET_EXP int EnableDlgItem (HWND hwnd, int id, int onoff)
{
	return EnableWindow(GetDlgItem(hwnd, id), onoff);
}

AUDFRET_EXP int ShowDlgItem (HWND hwnd, int id, int nCmdShow)
{
	return ShowWindow (GetDlgItem(hwnd, id), nCmdShow);
}

AUDFRET_EXP char *trimLeft (char* string, const char *chars2Trimmed)
{
	char *p;
	for (p=string; *p && strchr(chars2Trimmed, *p); p++)
		;
	memmove(string, p, strlen(p)+1);
	return string;
}

AUDFRET_EXP char *trimRight (char* string, const char *chars2Trimmed)
{
	char *p;
	for (p=strchr(string, '\0')-1; p>=string && strchr(chars2Trimmed, *p); p--)
		;
	*(p+1) = '\0';
	return string;
}

AUDFRET_EXP int IsSemiMonotonic (int len, double *x)
{
	int i, updown;
	updown = x[1]>x[0];
	for (i=2; i<len; i++)
	{
		if ( updown != (x[i]>x[i-1]) )
			return 0;
	}
	return 1;
}

AUDFRET_EXP int belowabove (double val, int len, double *x)
{
	if (val<x[0]) 
		return -1;
	if (val>=x[len-1]) 
		return len-1;
	return 0;
}

AUDFRET_EXP int GetMaxInd4Cut (double criterion, int len, double *x)
{
	// Find the greatest index of the array x, which has the element smaller than (or the same as) val.
	// if it is the first one, returns -1. return value below -1 indicates an error.
	if (len<=0) return -2; // invalid length
	if (!IsSemiMonotonic(len, x)) return -3; // not (semi)monotonic
	if (x[0]>x[1]) return -4; // must not be decreasing.

	int lastID, curId, runlen(len);
	curId=0;
	lastID = belowabove (criterion,len,x);
	if (lastID!=0)
		return lastID;
	while ( runlen>1 && curId<len)
	{
		runlen /= 2;
		if (belowabove (criterion,runlen,x+curId)>0)
			curId += runlen + belowabove (criterion,runlen,x+curId+runlen);
	}
	if (curId>=len) curId = len-1;
	return curId;
}

AUDFRET_EXP int mod (int big, int divider)
{
// calculating int remainder
	return (int)fmod((double)big, (double)divider);
}

AUDFRET_EXP int getrand(int x)
{// generates a random number between 0 and x-1
	if (x==0) return 0;
	return mod(rand(),x);
}

AUDFRET_EXP short double2short(double x)
{
	// This maps a double variable raning -1 to 1, to a short variable ranging -32768 to 32767.
	return (short)(x*32768.-.5);
}

AUDFRET_EXP double short2double(short x)
{
	return ((double)x+.5)/32767.5;
}

AUDFRET_EXP double getexp(double dB)
{
	return exp( log(10.)/20.*(dB));
}

AUDFRET_EXP char * FulfillFile (char *out, const char *path, const char * file)
{
	size_t len = strlen(path);
	if (len==0)		
		strcpy(out, file);
	else if (path[len-1]!='\\')
		sprintf(out, "%s\\%s", path, file);
	else
		sprintf(out, "%s%s", path, file);
	return out;
}

AUDFRET_EXP int Append2Path(const char *additionalpath, char *errstr)
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

AUDFRET_EXP void GetPathFileNameFromFull (char *path, char *fname, const char *fullname)
{
	char drive[MAX_PATH], dir[MAX_PATH], buf[MAX_PATH], ext[32];
 	_splitpath(fullname, drive, dir, buf, ext);
	sprintf(path,"%s%s", drive, dir);
	sprintf(fname,"%s%s", buf, ext);
}

AUDFRET_EXP int GetSurroundedBy(char c1, char c2, const char* in, char* out, int iStart)
{
	// This extracts the string bounded by c1 and c2 (the first boundary)
	// returns the index of the ending border
	// c1 and c2 must be different
	// out must have been allocated by the caller.
	bool inside(false), loop(true);
	int i, cumInt(0);
	if (in==NULL || out==NULL) return -1;
	size_t len = strlen(in);
	out[0]='\0';
	if (len>65535)  return -1;
	for (i=iStart; i<(int)len && loop; i++)
	{
		if (in[i]==c1) { cumInt++; inside=true;}
		else if (in[i]==c2) cumInt--; 
			if (cumInt<0) return -1; // unbalanced parenthesis 
		if (inside && cumInt==0) loop=false;
	}
	if (cumInt!=0 || !inside) return -1;
	const char *firstind_c1 = strchr(in+iStart, c1);
	int first = (int) (firstind_c1 - in);
	int count = i - first-2;
	strncpy(out, firstind_c1+1, count);
	out[count]='\0';
	return i-1;
}

AUDFRET_EXP void GetLastErrorStr(DWORD ecode, char *errstr)
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

AUDFRET_EXP void GetLastErrorStr(char *errstr)
{
	GetLastErrorStr(GetLastError(), errstr);
}

AUDFRET_EXP void GetLocalTimeStr(string &strOut)
{
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	sformat(strOut, "[%02d/%02d/%4d, %02d:%02d:%02d]", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
}

AUDFRET_EXP int GetFileText(const char *fname, const char *mod, string &strOut)
{ // mod is either "rb" or "rt"
	FILE *fp = fopen(fname, mod);
	if (!fp) return -1;
	fseek(fp, 0, SEEK_END);
	int filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *buf = new char[filesize + 1];
	size_t ret = fread(buf, 1, filesize, fp);
	fclose(fp);
	if (ret < 0)
		return -2;
	else
	{
		buf[ret] = 0;
		strOut = buf;
	}
	return (int)ret;
}

AUDFRET_EXP int mceil(double x)
{ // modified ceil with reasonable tolerance
  // This function prevents uncessarily rounding-up
  // if x has long trailing zeros followed by one (due to rounding) such as 513.0000000001 
  // In that case we don't want 514...
  // 5/26/2018
	int implicitFloor = (int)x;
	double ceiled = ceil(x);
	if (ceiled - implicitFloor == 0.)	
		return implicitFloor;
	if (x - implicitFloor < 1.e-6) 
		return implicitFloor;
	else
		return (int)ceiled;
}