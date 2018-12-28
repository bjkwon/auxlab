// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: audfret
// Miscellaneous Support Library
// 
// 
// Version: 1.4951
// Date: 12/14/2018
// Change from 1.495: CString, CRect not used-->no need to include wxx_wincore.h
#ifndef BJTOOLS
#define BJTOOLS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

//If this library is linked statically define LINK_STATIC_AUDFRET in the application project.

#define AUD_ERR_FILE_NOT_FOUND			-2
#define AUD_ERR_CANNOT_READ_FILE		-3
#define AUD_ERR_HEADING_NOT_FOUND		-4
#define AUD_ERR_HEADING_FOUND_MULTIPLE	-5

int countDeliminators (const char* buf, const char* deliminators);
char* getfilenameonly(const char* filewpath, char* filename, size_t filenamesize);
int IsBetween (int x, int begin, int ending);
int GetMaxInd4Cut (double criterion, int len, double *x);
char* FulfillFile (char* out, const char *path, const char * file);
void GetPathFileNameFromFull (char *path, char *fname, const char *fullname);

int sprintfFloat(double f, int max_fp, char *strOut, size_t sizeStrOut);
int countMarkers(const char *str, char* marker);
int countstr(const char *str, char* marker);
int countchar(const char *str, char c);

int printf_INI (char *errstr, const char *fname, const char *heading, const char * szFormat, ...);
//int ReadINI (char *errstr, const char *fname, const char *heading, char *strout, size_t strLen);
//int ReadINI(char *errstr, const char *fname, const char *heading, std::string &strOut);
//int sscanfINI (char *errstr, const char *fname, const char *heading, const char * szFormat, ...);
int GetSurroundedBy(char c1, char c2, const char* in, char* out, int iStart);
vector<int> SpreadEvenly(int whole, int nGroups);

/*Do not use %s format for sscanfINI ---it won't work*/
/* sprintfFloat generates a string array from a double value, its floating point varies depending on the value and specified max_fp  */

char *trimLeft (char* string, const char *chars2Trimmed);
char *trimRight (char* string, const char *chars2Trimmed);
int howmanychr(const char* buffer, const char c);
int howmanystr(const char* buffer, const char* str);
short double2short(double x);
double short2double(short x);
int mod (int big, int divider);
int getrand(int x);
double getexp(double dB);

double compFileTime(const char *fname1, const char *fname2);
int isSameFile(const char *fname1, const char *fname2);
int compFileLength(const char *fname1, const char *fname2);

//The functions below are not really for users--only for str2array
// In order to use str2array, make audstr.h and audstr.cpp part of the project with audfretDLL.lib
char * copyItem(char * out, char * in);
int copyItem(int out, char * in);
double copyItem(double out, char * in);
char* recoverNULL(int *dummy, char *dest, const char *src);
char* recoverNULL(float *dummy, char *dest, const char *src);
char* recoverNULL(double *dummy, char *dest, const char *src);

size_t str2vect(std::vector<std::string> &out, std::string input, char delim);
size_t str2vectPlus(std::vector<std::string> &out, std::string input, char delim, char quot);
size_t str2vect(std::vector<std::string> &out, const char* input, const char *delim, int maxSize_x=0);
std::string consoldateStr(std::vector<std::string> &input, char delim);

void trim(string& str, string delim);
void trim(string& str, char delim);
void trim(string& str, char* delim);
int sformat(string& out, const char* format, ...);
int sformat(string& out, size_t nLengthMax, const char* format, ...);

void ReplaceStr(string &str, const string& from, const string& to);
void splitevenindices(vector<unsigned int> &out, unsigned int nTotals, unsigned int nGroups);
void GetLocalTimeStr(string &strOut);
int GetFileText(const char *fname, const char *mod, string &strOut);
int mceil(double x);


#ifndef NO_SOCKET
int TransSocket (const char *ipa, unsigned short port, const char *PipeMsg2Send, char *PipeMsg2Rec, int LenRec);
#endif //NO_SOCKET

//Obsolete....for compatibility only
int str2intarray(int* out, int maxSize_x, const char * str, const char *deliminators);



#endif // BJTOOLS