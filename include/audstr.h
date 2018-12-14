#pragma once
#include <string>
#include <sstream>

using namespace std;

void trimr(string& str, string delim);
void triml(string& str, string delim);
void trim(string& str, string delim);
void trimr(string& str, char delim);
void triml(string& str, char delim);
void trim(string& str, char delim);
void trimr(string& str, char* delim);
void triml(string& str, char* delim);
void trim(string& str, char* delim);
int sformat(string& out, size_t nLengthMax, const char* format, ...);

template <class T>
T sumarray(T *in, int len)
{
	T out(0);
	for (int i=0; i<len; i++) out += in[i];
	return out;
}

template <class T1, class T2>
T1 copyItem(T1 out, T2 in)
{
	out = (T1)in;
	return out;
}

template <class T>
char* recoverNULL(T type, char *dest, const char *src)
{
	return strcpy(dest, src);
}

template <class T> 
int str2array(T* out, int maxSize_x, const char *str, const char *deliminators)
{
	char *token, *newBuffer;
	int i(0);
	size_t len = strlen(str);
	size_t len2 = strlen(deliminators);
	char tail;
	unsigned int j, tailtick=0;
	int delim = countDeliminators(str, deliminators);

	if (out==NULL || str==NULL || deliminators==NULL)
		return -1;
	if (maxSize_x<0)
		return -2;

	newBuffer = new char [len+len2+1];
	strcpy(newBuffer, str);
	tail = str[len];
	for (j=0; j<len2; j++)
		if (tail==deliminators[j])
			tailtick=1;
	if (tailtick)
	{
		strncpy(newBuffer+len, deliminators, 1);
		newBuffer[len+1] = '\0';
	}
	int lastLoc(0);
	token = strtok( newBuffer, deliminators );
    while( token != NULL && i<maxSize_x)
    {
	   if (delim >maxSize_x && i+1==maxSize_x)
			recoverNULL(out, newBuffer+lastLoc, str+lastLoc);
	   if (strlen(token)>0)
		   out[i] = copyItem(out[i], token);
	   i++;
	   token = strtok( NULL, deliminators );
	   lastLoc = (int)(token-newBuffer);
	}
	delete[] newBuffer;
	return i;
}

template <class T>
int str2array(T* out, int maxSize_x, string str, const char *deliminators)
{
	char *newBuffer = new char[str.length()+2];
	strcpy (newBuffer, str.c_str());
	int res = str2array(out, maxSize_x, newBuffer, deliminators);
	delete[] newBuffer;
	return res;
}

template <class T>
int array2str(string &strOut, T* arrayIn, int len, const char *marker)
{
	strOut="";
	for (int i=0; i<len; i++) 
	{
		ostringstream tstr;
		tstr << arrayIn[i];
		strOut += tstr.str();
		strOut += marker;
		tstr.clear();
	}
	return (int)strOut.length();
}

template <class T>
int str2arraylast(T* out, string str, const char *deliminators)
{
	int delim = countDeliminators(str.c_str(), deliminators);
	if (delim<=1) 
		out[0] = copyItem(out[0], str.c_str()); 
	else
	{
		T *arr = new T[delim];
		str2array(arr, delim, str.c_str(), deliminators);
		out[0] = copyItem(out[0], arr[delim-1].c_str()); 
		delete[] arr;
	}
	return delim;
}

void triml(string& str, string delim);
void trimr(string& str, string delim);
void trim(string& str, string delim);
void triml(string& str, char delim);
void trimr(string& str, char delim);
void trim(string& str, char delim);
void triml(string& str, char *delim);
void trimr(string& str, char *delim);
void trim(string& str, char *delim);

