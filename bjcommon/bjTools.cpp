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


int countDeliminators (const char* buf, const char* deliminators)
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

char* getfilenameonly(const char* filewpath, char* filename, size_t filenamesize)
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

vector<int> SpreadEvenly(int whole, int nGroups)
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

int countchar(const char *str, char c)
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

int countstr(const char *str, char* marker)
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

int countMarkers(const char *str, char* marker)
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

int IsBetween (int x, int begin, int ending)
{
	// inclusive the edge points
	if (x>=begin && x<=ending)
		return 1;
	else 
		return 0;
}

char * CutEndSpace(char * str, char c)
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

int sprintfFloat(double f, int max_fp, char *strOut, size_t sizeStrOut)
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

char *trimLeft (char* string, const char *chars2Trimmed)
{
	char *p;
	for (p=string; *p && strchr(chars2Trimmed, *p); p++)
		;
	memmove(string, p, strlen(p)+1);
	return string;
}

char *trimRight (char* string, const char *chars2Trimmed)
{
	char *p;
	for (p=strchr(string, '\0')-1; p>=string && strchr(chars2Trimmed, *p); p--)
		;
	*(p+1) = '\0';
	return string;
}

int IsSemiMonotonic (int len, double *x)
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

int belowabove (double val, int len, double *x)
{
	if (val<x[0]) 
		return -1;
	if (val>=x[len-1]) 
		return len-1;
	return 0;
}

int GetMaxInd4Cut (double criterion, int len, double *x)
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

int mod (int big, int divider)
{
// calculating int remainder
	return (int)fmod((double)big, (double)divider);
}

int getrand(int x)
{// generates a random number between 0 and x-1
	if (x==0) return 0;
	return mod(rand(),x);
}

short double2short(double x)
{
	// This maps a double variable raning -1 to 1, to a short variable ranging -32768 to 32767.
	return (short)(x*32768.-.5);
}

double short2double(short x)
{
	return ((double)x+.5)/32767.5;
}

double getexp(double dB)
{
	return exp( log(10.)/20.*(dB));
}

char * FulfillFile (char *out, const char *path, const char * file)
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

void GetPathFileNameFromFull (char *path, char *fname, const char *fullname)
{
	char drive[MAX_PATH], dir[MAX_PATH], buf[MAX_PATH], ext[32];
 	_splitpath(fullname, drive, dir, buf, ext);
	sprintf(path,"%s%s", drive, dir);
	sprintf(fname,"%s%s", buf, ext);
}

int GetSurroundedBy(char c1, char c2, const char* in, char* out, int iStart)
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

void GetLocalTimeStr(string &strOut)
{
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	sformat(strOut, "[%02d/%02d/%4d, %02d:%02d:%02d]", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
}

int GetFileText(const char *fname, const char *mod, string &strOut)
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

int mceil(double x)
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
		if (ecode<-1) closesocket(sock);
		return -res;
	}
}
#endif //NO_SOCKET

