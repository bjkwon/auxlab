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

AUDFRET_EXP double compFileTime(const char *fname1, const char *fname2)
{ // positive if fname is newer than fname2
	SYSTEMTIME st1, st2;
	FILETIME ft1, ft2;
	double var1, var2;
	HANDLE hFile1 = CreateFile(fname1, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile1==INVALID_HANDLE_VALUE) return 1.e7;
	HANDLE hFile2 = CreateFile(fname2, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile2==INVALID_HANDLE_VALUE) return 1.e7;

	if (!GetFileTime(hFile1, NULL, NULL, &ft1) || !GetFileTime(hFile2, NULL, NULL, &ft2) ) {
		CloseHandle(hFile1); CloseHandle(hFile2);		
		return 2.e7; }
	CloseHandle(hFile1); CloseHandle(hFile2);
	if (!FileTimeToSystemTime(&ft1, &st1) || !FileTimeToSystemTime(&ft2, &st2) ) return 2.e7;

	var1 = st1.wYear * 10000 + st1.wMonth * 100 + st1.wDay 
			+ (double)st1.wHour/100. + (double)st1.wMinute/10000. + (double)st1.wSecond/1000000.;
	var2 = st2.wYear * 10000 + st2.wMonth * 100 + st2.wDay 
			+ (double)st2.wHour/100. + (double)st2.wMinute/10000. + (double)st2.wSecond/1000000.;
	return var1 - var2;
}

AUDFRET_EXP int compFileLength(const char *fname1, const char *fname2)
{
	HANDLE hFile1 = CreateFile(fname1, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile1==INVALID_HANDLE_VALUE) return 9999;
	HANDLE hFile2 = CreateFile(fname2, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile2==INVALID_HANDLE_VALUE) return 9999;

	DWORD high1, high2;
	DWORD res1, res2;
	if ((res1=GetFileSize(hFile1, &high1))==INVALID_FILE_SIZE  || (res2=GetFileSize(hFile2, &high2)==INVALID_FILE_SIZE))
	{  CloseHandle(hFile1); CloseHandle(hFile2);	return 8888;}
	CloseHandle(hFile1); CloseHandle(hFile2);	
	if (high1*high2)
	{
		if (high1==high2)	{	if (res2==res1) return 0;
								if (res2>res1) return 1;
								if (res2<res1) return -1;	}
		if (high1>high2)	return 1;
		if (high1<high2)	return -1;
	}
	if (high2)		return 1;
	if (high1)		return -1;
	//if it continues to this line, both high1 and high2 are zero
	if (res2==res1) return 0;
	if (res2>res1) return 1;
	else return -1; // res2<res1
}

#define BLOCKSIZE 8192

AUDFRET_EXP int isSameFile(const char *fname1, const char *fname2)
{
	 // if different length, return 0;
	if (!compFileLength(fname1, fname2)) return 0;

	HANDLE hFile1 = CreateFile(fname1, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile1==INVALID_HANDLE_VALUE) return -1;
	HANDLE hFile2 = CreateFile(fname2, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile2==INVALID_HANDLE_VALUE) return -1;
	DWORD high, res;
	DWORD dw, dw1(1), dw2(1);
	char *buf1, *buf2;

	if ((res=GetFileSize(hFile1, &high))==INVALID_FILE_SIZE)
	{  CloseHandle(hFile1); CloseHandle(hFile2);	return -1;}

	__int64 size = high;
	size = size < 32;
	size += res;

	if (high)
		dw = 0xffffffff;
	else
		dw = res;
	buf1 = new char[dw];	buf2 = new char[dw];
	for (unsigned int i=0; i<high; i++)
	{
		if (!ReadFile (hFile1, buf1, dw, &dw1, NULL)) {delete[] buf1; delete[] buf2; CloseHandle(hFile1); CloseHandle(hFile2);	return -1;}
		if (!ReadFile (hFile2, buf2, dw, &dw2, NULL)) {delete[] buf1; delete[] buf2; CloseHandle(hFile1); CloseHandle(hFile2);	return -1;}
		if (memcmp((void*)buf1,(void*)buf2, dw)) return 0;
	}
	dw = res;
	if (!ReadFile (hFile1, buf1, dw, &dw1, NULL)) {delete[] buf1; delete[] buf2; CloseHandle(hFile1); CloseHandle(hFile2);	return -1;}
	if (!ReadFile (hFile2, buf2, dw, &dw2, NULL)) {delete[] buf1; delete[] buf2; CloseHandle(hFile1); CloseHandle(hFile2);	return -1;}
	CloseHandle(hFile1); CloseHandle(hFile2);
	if (memcmp((void*)buf1,(void*)buf2, dw)) return 0;
	else return 1;
}