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
#include <sstream>
#ifdef _WINDOWS
#include <windows.h>
#else
#include <stdarg.h>
#endif
using namespace std;

void triml(string& str, string delim)
{
  string::size_type pos = str.find_first_not_of(delim);
  if(pos != string::npos) str.erase(0, pos);
  else str.erase(str.begin(), str.end());
}

void trimr(string& str, string delim)
{
  string::size_type pos = str.find_last_not_of(delim);
  if(pos != string::npos) str.erase(pos + 1);
  else str.erase(str.begin(), str.end());
}

void trim(string& str, string delim)
{
	triml(str, delim);
	trimr(str, delim);
}

void triml(string& str, char delim)
{
	string _delim(1,delim);
	triml(str, _delim);
}

void trimr(string& str, char delim)
{
	string _delim(1,delim);
	trimr(str, _delim);
}

void trim(string& str, char delim)
{
	string _delim(1,delim);
	trim(str, _delim);
}

void triml(string& str, char *delim)
{
	string _delim(delim);
	triml(str, _delim);
}

void trimr(string& str, char *delim)
{
	string _delim(delim);
	trimr(str, _delim);
}

void trim(string& str, char *delim)
{
	string _delim(delim);
	trim(str, _delim);
}