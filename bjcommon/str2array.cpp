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
#pragma once

#include <string>
#include <vector>
using namespace std;
#include "audfret.h"
#include "audstr.h"
#ifdef _WINDOWS
#include <windows.h>
#else
#include <stdarg.h>
#endif

void ReplaceStr(string &str, const string& from, const string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
}

int getQuots(string input, size_t &id1, size_t &id2, char quot)
{

	size_t posQuot1 = input.find(quot);
	if (posQuot1 == string::npos) return 0;
	size_t posQuot2 = input.find(quot, posQuot1+1);
	if (posQuot2 == string::npos) return -1;
	id1 = posQuot1;
	id2 = posQuot2;
	return 1;
}

size_t str2vectPlus(vector<string> &out, string input, char delim, char quot)
{//same as str2vect except if it is enclosed by quot, delim is ignored and untouched.
	out.clear();
	if (delim==quot) return 0; // Invalid. Don't go any further
	size_t posDelim;
	size_t posQuot1;
	size_t posQuot2;
	int res;
	while (!input.empty())
	{
		res=getQuots(input, posQuot1, posQuot2, '\"');
		if (res>0)
		{
			posDelim = input.find(delim);
	
			if (posDelim==string::npos)
			{
				out.push_back(input);
				return out.size();
			}
			else
			{
				if (posDelim>posQuot2 || (posDelim>posQuot1 && posDelim<posQuot2) )
				{
					out.push_back(input.substr(posQuot1+1, posQuot2-1-posQuot1));
					input.erase(0, posQuot2+1);
					if (!input.empty() && input.at(0)==delim) 
						input.erase(0, 1);
				}
				else
				{
					// at this point we should attempt to break it with delim
					posQuot1 = input.find(quot);
					while( (posDelim=input.find(delim))!=string::npos && (posQuot2 = input.find(quot, posQuot1))>posDelim)
					{
						out.push_back(input.substr(0, posDelim));
						input.erase(0, posDelim+1);
						posQuot1 = input.find(quot);
					}
					if (!input.empty() && input.at(0)==delim) 
						input.erase(0, 1);
				}
			}
		}
		else
		{
			int nEst = countchar(input.c_str(), delim);
			size_t pos;
			while( (pos=input.find(delim))!=string::npos)
			{
				out.push_back(input.substr(0, pos));
				input.erase(0, pos+1);
			}
			out.push_back(input);
			return out.size();
		}
//		out.push_back(input);
	}
	return out.size();
}

size_t str2vect(vector<string> &out, string input, char delim)
{
	int nEst = countchar(input.c_str(), delim);
	out.clear();
	out.reserve(nEst+2);
	size_t pos;
	while( (pos=input.find(delim))!=string::npos)
	{
		out.push_back(input.substr(0, pos));
		input.erase(0, pos+1);
	}
	out.push_back(input);
	return out.size();
}

size_t str2vect(vector<string> &_out, const char* input, const char *delim, int maxSize_x)
{ // Bug found--If delim contains repeating character(s), it goes into an infinite loop. Avoid that. 8/4/2017

	size_t pos;
	string str(input);
	string delim2(delim); 
	vector<string> out;
	if (maxSize_x < 0) return 0;
	int nitems = countDeliminators(input, delim);
	out.reserve(nitems);
	trimr(str, delim);
	delim2 = delim2.front();
	for (unsigned int k=1; k<strlen(delim); k++) 
		while( (pos=str.find(delim[k]))!=string::npos)
			str.replace(pos,1,delim2);

	while((pos=str.find(delim[0]))!=string::npos)
	{
		if (maxSize_x > 0)
			if (out.size() == maxSize_x - 1)
				break;
		if (pos==0) pos++;
		else		out.push_back(str.substr(0,pos));
		str.erase(0,pos);
	}
	out.push_back(str);
	_out = out;
	return out.size();
}

string consoldateStr(vector<string> &input, char delim)
{
	string out;
	for (size_t k=0; k<input.size(); k++)
	{
		out += input[k];
		out += delim;
	}
	return out;
}

char* recoverNULL(int *dummy, char *dest, const char *src)
{ return NULL; }
char* recoverNULL(float *dummy, char *dest, const char *src)
{ return NULL; }
char* recoverNULL(double *dummy, char *dest, const char *src)
{ return NULL; }

char * copyItem(char * out, char * in)
{
	return strcpy(out,in);
}

int copyItem(int out, char * in)
{
	int res = sscanf(in, "%d", &out);
	if (res==EOF || res==0) return 0;
	return out;
}

double copyItem(double out, char * in)
{
	int res = sscanf(in, "%lf", &out);
	if (res==EOF || res==0) return 0;
	return out;
}

int str2intarray(int* out, int maxSize_x, const char * str, const char *deliminators)
{
	return str2array(out, maxSize_x, str, deliminators);
}

int str2strarray(char** out, int maxSize_x, const char * str, const char *deliminators)
{
	return str2array(out, maxSize_x, str, deliminators);
}

int str2doublearray(double* out, int maxSize_x, const char * str, const char *deliminators)
{
	return str2array(out, maxSize_x, str, deliminators);
}

void splitevenindices(vector<unsigned int> &out, unsigned int nTotals, unsigned int nGroups)
{
	unsigned int nItems = nTotals / nGroups;
	unsigned int chunkwithnItems = nGroups * nItems;
	int leftover = nTotals - chunkwithnItems;
	out.clear();
	out.assign(nGroups, nItems);

	if (leftover>0)
	{
		unsigned int *holder = new unsigned int[nGroups];
		double split = (double)leftover / (double)nGroups;
		for (unsigned int i=0; i<nGroups; i++)	holder[i] = (int)(split*(double)i+.5);
		for (unsigned int i=1; i<nGroups; i++)	holder[i-1] = holder[i]-holder[i-1];
		unsigned int sum(0);
		for (unsigned int i=0; i<nGroups-1; i++) sum += holder[i];
		holder[nGroups-1] = leftover - sum;
		int k(0);
		for (vector<unsigned int>::iterator it=out.begin(); it!=out.end(); it++, k++)
			*it += holder[k];
		delete[] holder;
	}
}

int sformat(string& out, size_t nLengthMax, const char* format, ...) 
{
	// Obsolete as of 5/2/2017 bjkwon
	char *buffer = new char[nLengthMax];
	va_list vl;
	va_start(vl, format);
	int res = vsnprintf(buffer, nLengthMax, format, vl);
	buffer[nLengthMax-1] =0;
	out = string(buffer);
	delete[] buffer;
	va_end(vl);
	return res;
}


int sformat(string& out, const char* format, ...) 
{
	size_t bufsize(4096);
	bufsize += strlen(format);
	va_list args;
	va_start(args, format);
	char *buffer = new char[bufsize];
	int res = vsnprintf(buffer, bufsize, format, args);
	va_end(args);
	if ((size_t)res>=bufsize) 
	{
		delete[] buffer;
		bufsize = res+1;
		buffer = new char[bufsize];
		va_start(args, format);
		res = vsnprintf(buffer, bufsize, format, args);
		va_end(args);
	}
	out = string(buffer);
	delete[] buffer;
	return res; // This includes the null character
}
