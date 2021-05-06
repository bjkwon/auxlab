// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: audfret
// Miscellaneous Support Library
// 
// 
// Version: 1.73
// Date: 11/22/2022
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
		double diff = errCrit - err;
		// Why is this not a simple inequality errCrit > err?
		// with double, err is like 0.09999999999947, whereas errCrit is 0.1
		// There must be a better way than this, but just keep this way for now. 
		// 11/22/2020
		if (diff > 1.e-10) // max_fp capped at 10
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

int sprintfFloat(char *strOut, double f, int max_fp)
{
	int fp;
	char *temp, format[64];

	if (f<0)
	{
		fp = sprintfFloat(strOut, -f, max_fp);
		temp = (char*)calloc(strlen(strOut)+2,1);
		strcpy(temp,"-");
		strcat(temp+1, strOut);
		strcpy(strOut, temp);
		free(temp);
		return -fp;
	}
	if ((fp=countFloatingPoints(f, max_fp))==0)
		sprintf (strOut, "%d", (int)f);
	else
	{
		sprintf (format, "%%%d.%df", fp+3, fp);
		sprintf (strOut, format, f);
	}
	trimLeft(strOut, " ");
	return fp;
}

// Important---as of Nov 2020, max_fp should be capped at 9.
// Look inside countFloatingPoints.
int sprintfFloat(double f, int max_fp, char *strOut, size_t sizeStrOut)
{
	if (max_fp > 9) {
		memset(strOut, 'X', sizeStrOut-1);
		strOut[sizeStrOut - 1] = 0;
		return -1;
	}
	char buffer[256];
	if (f>1.e10 || f<-1.e10) {	memset((void*)strOut,'*', sizeStrOut-2); strOut[sizeStrOut-1]=0; return 0;}
	int res = sprintfFloat (buffer, f, max_fp);
	if (strlen(buffer)>sizeStrOut-1)
	{
		if (sizeStrOut>1) 
		{	memset((void*)strOut,'*', sizeStrOut-2); strOut[sizeStrOut-1]=0;}
		else if (sizeStrOut==1) 
			strOut[sizeStrOut-1]=0;
		return 0;
	}
	if (strchr(buffer, '.'))
	{
		char* pt = buffer + strlen(buffer) - 1; // last character position
		for (; ; pt--)
		{
			if (*pt == '0') *pt = '\0';
			else break;
		}
	}
	strcpy(strOut, buffer);
	return res;
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

int GetFileText(FILE* fp, string& strOut)
{
	if (!fp) return -1;
	fseek(fp, 0, SEEK_END);
	int filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buf = new char[filesize + 1];
	size_t pos = fread(buf, 1, (size_t)filesize, fp);
	// In case logfile contains EOF character in the middle (even accidentially)
	// making sure fread continues beyond the position of EOF
	size_t res = 1;
	while (pos > 0 && pos < filesize && res)
	{
		buf[pos++] = '\n';
		fseek(fp, (long)pos + 1 + 1, SEEK_SET);
		res = fread(buf + pos, 1, (size_t)(filesize-pos), fp);
		pos += res;
	}
	fclose(fp);
	if (pos <= 0)
		return -2;
	else
	{
		buf[pos] = 0;
		strOut = buf;
		delete[] buf;
	}
	return (int)pos;
}

int GetFileText(const char *fname, const char *mod, string &strOut)
{ // mod is either "rb" or "rt"
	FILE *fp = fopen(fname, mod);
	if (!fp) return -1;
	return GetFileText(fp, strOut);
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

size_t process_esc_chars(void* input, size_t len, char * errstr)
{ // assume errstr has been allocated with size > 64
  // returns number of bytes that were successfully converted
  // (not the same as strlen(str) because of a null character
  // returns 0 if an error occurs

	char* buffer = new char[len+1];
	char* str = (char*)input;
	size_t res = 0;
	for (char* pt0 = (char*)str,*pt = strchr((char*)str, '\\'); pt; )
	{
		memcpy(buffer, pt0, pt - pt0);
		switch  (*(pt + 1))
		{
		case 0:
			buffer[pt - pt0] = 0;
			break;
		case 'n':
			buffer[pt - pt0] = '\n';
			break;
		case 'r':
			buffer[pt - pt0] = '\r';
			break;
		case 't':
			buffer[pt - pt0] = '\t';
			break;
		case '"':
			buffer[pt - pt0] = '"';
			break;
		case '\'':
			buffer[pt - pt0] = '\'';
		break;		
		case '\\':
			buffer[pt - pt0] = '\\';
			break;
		}
		res = (int)(pt - pt0);
		char* pt2 = strchr(pt + 1, '\\');
		if (!pt2)
		{
			pt += 2;
			memcpy(buffer + res + 1, pt, strlen(pt));
			res += strlen(pt) + 1; // adding 1 to include the converted character
			break;
		}
		else
			pt = pt2;
	}
	buffer[res] = 0;
	memcpy(input, buffer, res + 1); // adding 1 to include the trailing null character
	delete[] buffer;
	return res;
}
