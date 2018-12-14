// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.495
// Date: 12/13/2018
// 
#include "sigplus_internal.h"

#define RETURN_NULL_MSG(str) {	strcpy(errstr, str);		return NULL; }

int double2int(double xx)
{ return (int) (xx * MAX_24BIT); }
double int2double(int ii)
{ return (double)ii /  MAX_24BIT; }

void filter(int nTabs, double *num, double *den, int length, double *in, double *out)
{
	double xx;
	for (int i=0; i<length; i++)
	{
		xx=num[0]*in[i];
		for (int j=1; j<nTabs && i>=j; j++)
			xx += num[j]*in[i-j];
		for (int j=1; j<nTabs && i>=j; j++)
			xx -= den[j]*out[i-j];
		out[i] = xx;
	}
}

void filter(int nTabs, double *num, double *den, int length, double *data)
{
	double *temp = new double[length];
	filter(nTabs, num, den, length, data, temp);
	memcpy((void*)data, (void*)temp, length*sizeof(double));
	delete[] temp;
}

void filter(int nTabs, double *num, int length, double *in, double *out)
{
	double xx;
	for (int i=0; i<length; i++)
	{
		xx=num[0]*in[i];
		for (int j=1; j<nTabs && i>=j; j++)
			xx += num[j]*in[i-j];
		out[i] = xx;
	}
}

void filter(int nTabs, double *num, int length, double *data)
{
	double *temp = new double[length];
	filter(nTabs, num, length, data, temp);
	memcpy((void*)data, (void*)temp, length*sizeof(double));
	delete[] temp;
}

void filter(int nTabs, double *num, double *den, int length, int *in, int *out)
{ // out need not initialize.
	double *xcl = new double[length];
	double *xout = new double[length];
	for (int i=0; i<length; i++) xcl[i] = int2double(in[i]);
	filter (nTabs, num, den, length, xcl, xout);
	for (int i=0; i<length; i++) out[i] = double2int(xout[i]);
	delete[] xcl;
	delete[] xout;
}

void filter(int nTabs, double *num, int length, int *in, int *out)
{ // out need not initialize.
	//FIR
	double *xcl = new double[length];
	double *xout = new double[length];
	for (int i=0; i<length; i++) xcl[i] = int2double(in[i]);
	filter (nTabs, num, length, xcl, xout);
	for (int i=0; i<length; i++) out[i] = double2int(xout[i]);
	delete[] xcl;
	delete[] xout;
}