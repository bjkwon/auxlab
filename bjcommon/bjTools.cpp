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

int mod (int big, int divider)
{
// calculating int remainder
	return (int)fmod((double)big, (double)divider);
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
