// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.498
// Date: 2/4/2019
// 
#ifdef _WINDOWS
#ifndef _MFC_VER // If MFC is used.
#include <windows.h>
#else 
#include "afxwin.h"
#endif 
#endif 
#include <math.h>
#include <limits> 
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <algorithm>

#include "bjcommon.h"
#include "sigplus_internal.h"
#include "samplerate.h"
#include "sndfile.h"

#include <complex>

#include <math.h> // ceil

#ifndef CISIGPROC
#include "psycon.tab.h"
#else
#include "cipsycon.tab.h"
#endif

#include "wavplay.h"


/* To-do
All of these 
	throw "message";
statements should be revised. 
They are const char, not CException; therefore, they are caught by catch (const char *errmsg) in xcom::computeandshow
and such information as line, col, pnode, etc are not processed for display.
A solution might be, whenever a function with the throw statement is used in AstSig.cpp, there should be another try loop
so that this const char * exception can be caught there and the other information is properly handled.
Or is there any other way??? 10/14/2018
*/

#define CRIT  100. // Threshold for computing rms is above 100th of its max

#define RETURN_0_MSG(str) {	strcpy(errstr, str);		return 0;	}

void filter(int nTabs, double *num, double *den, int length, double *in, double *out);
void filter(int nTabs, double *num, int length, double *in, double *out);

#define DOUBLE2SHORT(x)	((short)(max(min((x),1),-1)*(double)0x007fff))

double quantizetmark(double delta, int fs)
{
	//quantize delta based on the fs grid.  3/31/2016. Rev. 5/19/2018
	int pt = (int)round(delta / 1000 * fs);
	double newdelta = 1000. * pt / fs;
	return newdelta;
}

int _double_to_24bit(double x)
{
	// This maps a double variable raning -1 to 1, to a short variable ranging -16388608 to 16388607.
	return (int)(max(min(x,1),-1)*MAX_24BIT-.5);
}

double _24bit_to_double(int x)
{ // converts a short variable into double in a scale of 16388608.
	return ((double)x+.5)/MAX_24BIT;
}

void _short_to_24bit(short* pshort, int* pint, int len)
{ // Assumes that memory blocks have been prepared.
	for (int i=0; i<len; i++) pint[i] = pshort[i]<<8;
}

void _24bit_to_short(int* pint, short* pshort, int len)
{ // Assumes that memory blocks have been prepared.
	for (int i=0; i<len; i++) pshort[i] = (short)(pint[i]>>8);
}

void _double_to_short(double* dint, short* pshort, int len)
{
	for (int i=0; i<len; i++) 
		pshort[i] = (short)(_double_to_24bit(dint[i]) >> 8);
}

double _getdB(double x)
{
	// 3 dB is added to make rms of full scale sinusoid 0 dB
	return 20*log10(x) + 3.0103;
}

void SwapCSignalsPutScalarSecond(CSignals &sig1, CSignals &sig2)
{
	if (!sig1.IsStereo() && sig2.IsStereo())
	{
		CSignals copy(sig1);
		sig1 = sig2, sig2 = copy;
	}
}

/*bufBlockSize is 1 (for logical, string), sizeof(double) (regular), or sizeof(double)*2 (complex).     4/4/2017 bjkwon */
body::body()
:nSamples(0), bufBlockSize(sizeof(double)), buf(new double[0]), nGroups(1), parg(NULL)
{
}

body::body(double value)
:nSamples(1), bufBlockSize(sizeof(double)), buf(new double[1]), nGroups(1), parg(NULL)
{
	SetValue(value);
}

body::body(complex<double> value)
:nSamples(1), bufBlockSize(sizeof(double)*2), buf(new double[2]), nGroups(1), parg(NULL)
{
	cbuf[0]=value; 
}

body::body(const body& src) 
:nSamples(0), bufBlockSize(sizeof(double)), buf(new double[0]), nGroups(1), parg(NULL)
{
	*this = src;
}

body::body(double *y, int len)
:nSamples(len), bufBlockSize(sizeof(double)), buf(new double[len]), nGroups(1), parg(NULL)
{
	memcpy(buf, y, bufBlockSize*len);
}

body::body(bool *y, int len)
:nSamples(len), bufBlockSize(1), logbuf(new bool[len]), nGroups(1), parg(NULL)
{
	memcpy(buf, y, bufBlockSize*len);
}

body& body::operator=(const body& rhs)
{
	if (this != &rhs) 
	{
		unsigned int currentBufSize = bufBlockSize * nSamples; 
		unsigned int reqBufSize = rhs.bufBlockSize * rhs.nSamples;
		if (currentBufSize < reqBufSize) 
		{
			if (nSamples>0) delete[] buf; 
			logbuf = new bool[reqBufSize];
		}
		nSamples = rhs.nSamples;
		bufBlockSize = rhs.bufBlockSize;
		nGroups= rhs.nGroups;
		memcpy(buf, rhs.buf, nSamples*bufBlockSize);
	}
	return *this;
}

body& body::operator+=(const double con)
{
	unsigned int blockSize = bufBlockSize/sizeof(double);
	for (unsigned int k=0; k<nSamples; k++) buf[k*blockSize] += con; 
	return *this;
}

body& body::operator*=(const double con)
{
	unsigned int blockSize = bufBlockSize/sizeof(double);
	switch (blockSize)
	{
	case 1:
		for (unsigned int k=0; k<nSamples; k++) buf[k] *= con;
		break;
	case 2:
		for (unsigned int k=0; k<nSamples; k++) cbuf[k] *= con;
		break;
	}
	return *this;
}

body& body::operator/=(const double con)
{
	return *this *= 1. / con;
}

body& body::operator-(void)
{
	if (bufBlockSize==sizeof(double))
		for (unsigned int k=0; k<nSamples; k++) buf[k] = -buf[k];
	if (bufBlockSize==2*sizeof(double))
		for (unsigned int k=0; k<nSamples; k++) cbuf[k] = -cbuf[k];
	return *this;
}

body::~body()
{
	if (buf) delete[] buf;
}

void body::SetValue(double v) 
{ // why did I change it this way? --- I want to just replace a value with the existing value (i.e., if it was already CSIG_SCALR, I want to reuse the buffer) 8/15/2018
	if (bufBlockSize != sizeof(double))
	{
		bufBlockSize = sizeof(double);
		if (buf) delete[] buf;
		buf = new double[1];
		nSamples = 1;
	}
	if (nSamples != 1)
	{
		if (buf) delete[] buf;
		buf = new double[1];
		nSamples = 1;
	}
	nGroups = 1;
	buf[0]=v; 
}

void body::SetValue(complex<double> v) 
{
	bufBlockSize=2*sizeof(double); 
	if (buf) delete[] buf; 
	buf = new double[2]; 
	nSamples=1; cbuf[0]=v; 
}


body& body::UpdateBuffer(unsigned int length)	// Set nSamples. Re-allocate buf if necessary to accommodate new length.
{
	unsigned int currentBufsize = bufBlockSize * nSamples; 
	unsigned int reqBufSize = bufBlockSize * length; 
	if (length < 0 || currentBufsize == reqBufSize)
		return *this;
	if (length > nSamples) {
		bool *newlogbuf = new bool[reqBufSize];
		if (nSamples>0)
			memcpy(newlogbuf, buf, nSamples*bufBlockSize);
		delete[] buf;
		logbuf = newlogbuf;
	}
	if (length > nSamples)
		memset(logbuf+nSamples*bufBlockSize, 0, (length-nSamples)*bufBlockSize); //initializing with zeros for the rest
	nSamples = length;
	return *this;
}

void body::Reset()
{
	if (buf) 
		delete[] buf;
	buf = NULL;
	nSamples = 0;
	nGroups = 1;
	bufBlockSize = sizeof(double);
}

double body::value() const 
{
	if (nSamples == 1)
	{
		if (bufBlockSize==8)	return buf[0];
		else if (bufBlockSize == 1)	return (double)logbuf[0];
		else throw "value( ) on complex value. Use cvalue instead.";
	}
	else if (nSamples==0) 
		throw "value( ) on null."; 
	else 
		throw "value( ) on vector/array.";
}


complex<double> body::cvalue() const 
{
	if (nSamples==1) 
		return cbuf[0]; 
	else if (nSamples==0) 
		throw "value( ) on null."; 
	else 
		throw "value( ) on vector/array.";
}

//Equivalent to real part
void body::SetReal() 
{ // This converts a complex array into real (decimating the imaginary part).
  // Must not be used when the buf is properly prepared--in that case just bufBlockSize = 1; is sufficient.
	unsigned int k = 0;
	if (IsComplex())
	{
		bufBlockSize = sizeof(double); 
		for (; k<nSamples; k++) buf[k] = buf[2*k];
	}
//if logical or string, it will turn to regular 8 
	else if (bufBlockSize == 1)
	{
		body out;
		out.UpdateBuffer(nSamples);
		for (auto &v : *this) out.buf[k++] = (double)v;
		*this = out;
	}
}

void body::Real()
{
	SetReal();
}

void body::Imag()
{
	if (IsComplex())
	{
		bufBlockSize = sizeof(double);
		for (unsigned int k = 0; k<nSamples; k++) buf[k] = buf[2 * k + 1];
	}
	else
	{
		Reset();
		UpdateBuffer(nSamples);
	}
}


void body::SetComplex() 
{
	if (bufBlockSize != sizeof(double) * 2)
	{
		bufBlockSize = sizeof(double) * 2;
		if (nSamples > 0)
		{
			double *newbuf = new double[2 * nSamples];
			memset(newbuf, 0, sizeof(double) * 2 * nSamples);
			for (unsigned int k = 0; k < nSamples; k++) newbuf[2 * k] = buf[k];
			delete[] buf;
			buf = newbuf;
		}
	}
	else if (bufBlockSize == 1)
	{
		body out;
		out.bufBlockSize = 16;
		out.UpdateBuffer(nSamples);
		for (unsigned int k = 0; k < nSamples; k++) out.buf[2 * k] = buf[k];
		*this = out;
	}
}

void body::SwapContents1node(body &sec)
{	// swaps fs, buf, nSamples & bufBlockSize (rev. 5/19/2018)
	body tmp;
	tmp.buf = buf, tmp.nSamples = nSamples, tmp.bufBlockSize = bufBlockSize, tmp.nGroups = nGroups;
	buf = sec.buf, nSamples = sec.nSamples, bufBlockSize = sec.bufBlockSize, nGroups = sec.nGroups;
	sec.buf = tmp.buf, sec.nSamples = tmp.nSamples, sec.bufBlockSize = tmp.bufBlockSize, sec.nGroups = tmp.nGroups;
	// Mark buf NULL so that destructor won't try to destroy buf which is used by sec
	tmp.buf = NULL;
}

body &body::addmult(char type, body &sec, unsigned int id0, unsigned int len)
{
	if (nSamples / nGroups < sec.nSamples / sec.nGroups) SwapContents1node(sec);
	auto lenL = nSamples / nGroups;
	auto lenR = sec.nSamples / sec.nGroups;
	auto vertID = id0 / lenL;
	auto minlen = min(lenL, lenR);
	//Assume that is this is Complex, sec is also Complex
	if (lenR == 1)
	{
		if (IsComplex())
		{
			auto rhs = sec.nSamples == 1 ? sec.cbuf[0] : sec.cbuf[vertID];
			auto len2 = len == 0 ? lenL : len;
			if (type == '+')
				for (auto k = id0; k<id0 + len2; k++)		cbuf[k] += rhs;
			else
				for (auto k = id0; k<id0 + len2; k++)		cbuf[k] *= rhs;
			len = len2;
		}
		else
		{
			auto rhs = sec.nSamples == 1 ? sec.buf[0] : sec.buf[vertID];
			auto len2 = len == 0 ? lenL : len;
			if (type == '+')
				for (auto k = id0; k < id0 + len2; k++)		buf[k] += rhs;
			else
				for (auto k = id0; k < id0 + len2; k++)		buf[k] *= rhs;
		}
	}
	else
	{
		if (len == 0)	len = minlen;
		else			len = min(len, minlen);
		if (IsComplex())
		{
			if (type == '+')
				for (auto k = id0; k<id0 + len; k++)		cbuf[k] += sec.cbuf[vertID*lenR + k - id0];
			else
				for (auto k = id0; k<id0 + len; k++)		cbuf[k] *= sec.cbuf[vertID*lenR + k - id0];
		}
		else
		{
			if (type == '+')
				for (auto k = id0; k<id0 + len; k++)		buf[k] += sec.buf[vertID*lenR + k - id0];
			else
				for (auto k = id0; k<id0 + len; k++)		buf[k] *= sec.buf[vertID*lenR + k - id0];
		}
	}
	if (IsComplex())
	{
		for (auto k = (unsigned)0; k<len; k++)
			if (imag(cbuf[k]) != 0.) return *this;
		//if it survives this far, it should be set real
		SetReal();
	}
	return *this;
}


body &body::each(double (*fn)(double))
{
	if (bufBlockSize==1)
		for (unsigned int k=0; k<nSamples; ++k)	logbuf[k] = fn(logbuf[k])!=0;
	else
		for (unsigned int k=0; k<nSamples; ++k)	buf[k] = fn(buf[k]);
	return *this;
}

body &body::each(double (*fn)(complex<double>))
{
	double *out = new double[nSamples];
	for (unsigned int k=0; k<nSamples; ++k)	out[k] = fn(cbuf[k]);
	delete[] buf;
	buf = out;
	bufBlockSize=sizeof(double);
	return *this;
}

body &body::each(complex<double>(*fn)(double))
{
	for (unsigned int k = 0; k<nSamples; ++k)	cbuf[k] = fn(buf[k]);
	return *this;
}

body &body::each(complex<double>(*fn)(complex<double>))
{
	for (unsigned int k = 0; k<nSamples; ++k)	cbuf[k] = fn(cbuf[k]);
	return *this;
}

body &body::each(double (*fn)(double, double), const body &arg)
{
	if (arg.nSamples==1) 
	{
		double val = arg.value();
		if (bufBlockSize==1 && arg.bufBlockSize==1)
			for (unsigned int k=0; k<nSamples; k++)	logbuf[k] = fn(logbuf[k],arg.logbuf[0])!=0;
		else if (bufBlockSize==1 && arg.bufBlockSize!=1)
			for (unsigned int k=0; k<nSamples; k++)	logbuf[k] = fn(logbuf[k],val)!=0; 
		else
			for (unsigned int k=0; k<nSamples; k++)	buf[k] = fn(buf[k],val); 
	}
	else if (nSamples==1) 
	{
		double baseval = buf[0];
		UpdateBuffer(arg.nSamples);
		if (bufBlockSize==1 && arg.bufBlockSize==1)
			for (unsigned int k=0; k<arg.nSamples; k++) logbuf[k] = fn(baseval, arg.logbuf[k])!=0;
		else if (bufBlockSize==1 && arg.bufBlockSize!=1)
			for (unsigned int k=0; k<arg.nSamples; k++) logbuf[k] = fn(baseval, arg.buf[k])!=0; 
		else
			for (unsigned int k=0; k<arg.nSamples; k++) buf[k] = fn(baseval, arg.buf[k]); 
	}
	else
	{
		nSamples = min(nSamples, arg.nSamples);
		if (bufBlockSize==1 && arg.bufBlockSize==1)
			for (unsigned int k=0; k<nSamples; k++) logbuf[k] = fn(logbuf[k], arg.logbuf[k])!=0; 
		else if (bufBlockSize==1 && arg.bufBlockSize!=1)
			for (unsigned int k=0; k<nSamples; k++) logbuf[k] = fn(logbuf[k], arg.buf[k])!=0; 
		else
			for (unsigned int k=0; k<nSamples; k++) buf[k] = fn(buf[k], arg.buf[k]); 
	}
	return *this;
}

body &body::each(complex<double> (*fn)(complex<double>, complex<double>), const body &arg)
{
	if (arg.nSamples==1) 
	{
		complex<double> val = arg.cbuf[0];
		for (unsigned int k=0; k<nSamples; k++)	cbuf[k] = fn(cbuf[k],val); 
	}
	else if (nSamples==1) 
	{
		complex<double> val = arg.cbuf[0];
		UpdateBuffer(arg.nSamples);
		for (unsigned int k=0; k<arg.nSamples; k++) cbuf[k] = fn(val, arg.cbuf[k]); 
	}
	else
	{
		nSamples = min(nSamples, arg.nSamples);
		for (unsigned int k=0; k<nSamples; k++) cbuf[k] = fn(cbuf[k], arg.buf[k]); 
	}
	return *this;
}

body &body::mathFunForbidNegative(double(*fn)(double), complex<double>(*cfn)(complex<double>))
{
	void *tp = parg;
	parg = nullptr;
	if (bufBlockSize==16 || _min() < 0)
	{
		SetComplex();
		each(cfn);
	}
	else
		each(fn);
	parg = tp;
	return *this;
}

body &body::mathFunForbidNegative(double(*fn)(double, double), complex<double>(*cfn)(complex<double>, complex<double>), const body &param)
{
	if (cfn && _min() < 0)
	{
		SetComplex();
		body tp(param);
		tp.SetComplex();
		each(cfn, tp);
	}
	else
		each(fn, param);
	return *this;
}

double lt (double a, double b)
{ return a<b;}
double le (double a, double b)
{ return a<=b;}
double gt (double a, double b)
{ return a>b;}
double ge (double a, double b)
{ return a>=b;}
double eq (double a, double b)
{ return a==b;}
double ne (double a, double b)
{ return a!=b;}
double and (double a, double b)
{ return a&&b;}
double or (double a, double b)
{ return a||b;}
double not (double a)
{ if (a==0) return 1.;
else  return 0.;}


body& body::LogOp(body &rhs, int type)
{
	double (*fn)(double, double);
	switch(type)
	{
	case '<':
		fn = lt;
		break;
	case '>':
		fn = gt;
		break;
	case T_LOGIC_LE:
		fn = le;
		break;
	case T_LOGIC_GE:
		fn = ge;
		break;
	case T_LOGIC_EQ:
		fn = eq;
		break;
	case T_LOGIC_NE:
		fn = ne;
		break;
	case T_LOGIC_AND:
		fn = and;
		break;
	case T_LOGIC_OR:
		fn = or;
		break;
	case T_LOGIC_NOT:
		each(not);
		return *this;
	}
	each(fn, rhs);
	MakeLogical();
	return *this;
}


body& body::MakeLogical()
{
	if (bufBlockSize ==1) return *this;
	body out; 
	out.bufBlockSize = 1; // logical array
	out.UpdateBuffer(nSamples); // This over-reserve the memory, but there's no harm. 4/4/2017 bjk
	for (unsigned int k=0; k<nSamples; k++) 
	{
		if (buf[k]==0.) out.logbuf[k] = false;
		else			out.logbuf[k] = true;
	}
	out.nGroups = nGroups;
	return (*this=out);
}

body &body::insert(body &sec, int id)
{
	if (sec.nSamples==0) return *this;
	if (id<0) throw "insert index cannot be negative."; 
	if (sec.bufBlockSize!=bufBlockSize) throw "insert must be between the same data structure."; 
	int nToMove = nSamples - id;
	UpdateBuffer(nSamples+sec.nSamples);
	bool *temp = new bool[nToMove*bufBlockSize];
	memcpy(temp, logbuf+id*bufBlockSize, nToMove*bufBlockSize);
	memcpy(logbuf+id*bufBlockSize, sec.buf, sec.nSamples*bufBlockSize);
	memcpy(logbuf+(id+sec.nSamples)*bufBlockSize, temp, nToMove*bufBlockSize);
	delete[] temp;
	return *this;
}

body &body::replace(body &sec, body &index)
{
	//	this is to be used when items are replaced without changing the size.
	// except when sec is empty... in which case the "index"'ed items are deleted.
	// in that case index is assumed to be sorted ascending
	if (index.nSamples == 0) return *this;
	if (sec.bufBlockSize != bufBlockSize)
	{
		if (bufBlockSize == 1 && sec.bufBlockSize != 1) sec.MakeLogical();
		else if (bufBlockSize == 8 && sec.bufBlockSize == 1) sec.SetReal();
		else if (bufBlockSize == 8 && sec.bufBlockSize == 16) SetComplex();
		else if (bufBlockSize == 16 && sec.bufBlockSize == 1) sec.SetComplex();
		else if (bufBlockSize == 16 && sec.bufBlockSize == 8) sec.SetComplex();
	}
	for (unsigned int k = 0; k < index.nSamples; k++)
	{
		if (index.buf[k] < 1.)
			throw "index must be greater than 0";
		unsigned int id = (int)index.buf[k];
		if (id - index.buf[k] > .25 || index.buf[k] - id > .25)
			throw "index must be integer";
		if (id > nSamples) throw "replace index exceeds the range.";
	}
	if (sec.nSamples == 0)
	{
		int trace = (int)(index.buf[0] - 1);
		for (unsigned int k = 0; k < index.nSamples; k++)
		{
			unsigned int diff = (unsigned int) (k < index.nSamples - 1 ? index.buf[k + 1] - index.buf[k] : nSamples + 1 - index.buf[k]);
			diff--;
			memcpy(logbuf + trace*bufBlockSize, logbuf + (int)index.buf[k]*bufBlockSize, diff*bufBlockSize);
			trace += diff;
		}
		nSamples -= index.nSamples;
	}
	else
		for (unsigned int k=0; k<index.nSamples; k++)
		{
			if (index.buf[k]<1.)
				throw "index must be greater than 0";
			unsigned int id = (unsigned int)(index.buf[k]);
			if (id - index.buf[k] > .05 || index.buf[k] - id > .05)
				throw "index must be integer";
			if (id>nSamples) throw "replace index exceeds the range.";
			id--; // zero-based
			if (sec.nSamples == 1) // items from id1 to id2 are to be replaced with sec.value()
				memcpy(logbuf + id*bufBlockSize, sec.logbuf, bufBlockSize);
			else
				memcpy(logbuf +id*bufBlockSize, sec.logbuf+k*bufBlockSize, bufBlockSize);
		}
	return *this;
}

body &body::replace(body &sec, int id1, int id2)
{ // this replaces the data body between id1 and id2 (including edges) with sec
	if (id1<0||id2<0) throw "replace index cannot be negative."; 
	if (sec.bufBlockSize != bufBlockSize)
	{
		if (bufBlockSize == 1 && sec.bufBlockSize != 1) sec.MakeLogical();
		else if (bufBlockSize == 8 && sec.bufBlockSize == 1) sec.SetReal();
		else if (bufBlockSize == 8 && sec.bufBlockSize == 16) SetComplex();
		else if (bufBlockSize == 16 && sec.bufBlockSize == 1) sec.SetComplex();
		else if (bufBlockSize == 16 && sec.bufBlockSize == 8) sec.SetComplex();
	}
	//id1 and id2 are zero-based here.
	if (id1>(int)nSamples-1) throw "replace index1 exceeds the range.";
	if (id2>(int)nSamples-1) throw "replace index2 exceeds the range.";
	unsigned int secnsamples = sec.nSamples;
	bool ch = ((CSignal *)&sec)->bufBlockSize == 1;
	if (ch) secnsamples--;
	if (secnsamples==1) // no change in length--items from id1 to id2 are to be replaced with sec.value()
	{
		if (ch) 
			for (int k=id1; k<=id2; k++) strbuf[k] = sec.strbuf[0];
		else
			for (int k=id1; k<=id2; k++) buf[k] = sec.value();
	}
	else
	{
		unsigned int nAdd = secnsamples;
		unsigned int nSubtr = id2-id1+1;
		unsigned int newLen = nSamples + nAdd - nSubtr;
		unsigned int nToMove = nSamples - id2 - 1;
		if (nAdd>nSubtr) UpdateBuffer(newLen);
		bool *temp = new bool[nToMove*bufBlockSize];
		memcpy(temp, logbuf+(id2+1)*bufBlockSize, nToMove*bufBlockSize);
		memcpy(logbuf+id1*bufBlockSize, sec.buf, secnsamples*bufBlockSize);
		memcpy(logbuf+(id1+secnsamples)*bufBlockSize, temp, nToMove*bufBlockSize);
		delete[] temp;
		nSamples = newLen;
	}
	return *this;
}

double body::sum(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	double out(0.);
	int isum(0);
	if (bufBlockSize==1)
	{
		for (unsigned int k = id0; k<id0+len; k++)		isum += logbuf[k];
		out = (double)isum;
	}
	else
		for (unsigned int k = id0; k<id0 + len; k++)		out += buf[k];
	return out;
}

double body::stdev(unsigned int id0, unsigned int len)
{
	CVar param = *(CVar*)parg;
	double flag = param.value();
	if (len == 0) len = nSamples;
	if (!len) throw "Empty array";
	double sqsum(0.);
	for (unsigned int k = id0; k<id0 + len; k++)		sqsum += buf[k]*buf[k];
	double mm = mean(id0, len);
	if (flag==1.)
	{
		double var = sqsum/len - mm*mm;
		return sqrt(var);
	}
	else
	{
		double var = (sqsum + mm*mm*len - 2*mm*sum(id0,len) ) / (len-1);
		return sqrt(var);
	}
}

int migra(int input, int nGroup, int nSamples)
{
	int x1 = nGroup*(input - 1) + 1;
	int additional(0);
	while (x1 > nSamples)
	{
		x1 -= nSamples;
		additional++;
	}
	return x1 + additional;
}

body &body::transpose()
{
	if (nSamples < 2) return *this;
	double tp1, tp2;
	int next, id;
	unsigned int counter(0);
	unsigned char *check = new unsigned char[nSamples];
	memset(check, 0, nSamples);
	check[1] = 1;
	int lasttrace = migra(2, nGroups, nSamples) ;
	for (unsigned int trace = 2; counter<nSamples-2; trace++)
	{
		if (check[trace-1]) continue;
		id = trace;
		next = migra(id, nGroups, nSamples);
		tp1 = buf[id - 1];
		while ((next = migra(id, nGroups, nSamples)) != trace)
		{
			tp2 = buf[next - 1];
			buf[next - 1] = tp1;
			check[next - 1] = 1;
			counter++;
			tp1 = tp2;
			id = next;
		}
		buf[next - 1] = tp1;
		check[next - 1] = 1;
		counter++;
	}
	nGroups = nSamples / nGroups;
	delete[] check;
	return *this;
}

// DO NOT CALL this function with empty buf
double body::_max(unsigned int id, int unsigned len)
{ // For a complex array, this does not return a complex value (it only returns real portion of the intended max element
 // Therefore, to get the intended max element, you need to get it from the max index through parg.
	unsigned int mid;
	double local = -std::numeric_limits<double>::infinity();
	if (len == 0) len = nSamples;
	if (bufBlockSize == 8)
	{
		for (unsigned int k = id; k < id + len; k++)
		{
			if (buf[k] > local)
				local = buf[k], mid = k;
		}
	}
	else if (bufBlockSize == 16)
	{
		for (unsigned int k = id; k < id + len; k++)
		{
			if (abs(cbuf[k]) >local)
				local = buf[k], mid = k;
		}
	}
	else
	{ // logical
		for (unsigned int k = id; k < id + len; k++)
		{
			if (logbuf[k]) {
				local = 1.; mid = k; break;
			}
		}
		if (local < 0 ) local = 0.;
	}
	if (parg)
		((CVar*)parg)->SetValue((double)mid + 1 - id);
	return local;
}

double body::_min(unsigned int id, unsigned int len)
{
	unsigned int mid;
	double local = std::numeric_limits<double>::infinity();
	if (len == 0) len = nSamples;
	if (bufBlockSize == 8)
	{
		for (unsigned int k = id; k < id + len; k++)
		{
			if (buf[k] < local)
				local = buf[k], mid = k;
		}
	}
	else if (bufBlockSize == 16)
	{
		for (unsigned int k = id; k < id + len; k++)
		{
			if (buf[k] < local)
				local = buf[k], mid = k;
		}
	}
	else
	{ // logical
		for (unsigned int k = id; k < id + len; k++)
		{
			if (!logbuf[k]) {
				local = 0.; mid = k; break;
			}
		}
		if (local > 1) local = 1.;
	}
	if (parg)	
		((CVar*)parg)->SetValue((double)mid + 1 - id);
	return local;
}


CSignal::CSignal()
:fs(1), tmark(0.), pf_basic(NULL), pf_basic2(NULL), pf_basic3(NULL)
{
}

CTimeSeries::CTimeSeries()
:chain(NULL)
{
}

CSignal::CSignal(int sampleRate)
: fs(max(sampleRate,0)), tmark(0.), pf_basic(NULL), pf_basic2(NULL), pf_basic3(NULL)
{
	if (fs==2) bufBlockSize=1;
}

CTimeSeries::CTimeSeries(int sampleRate)
: chain(NULL)
{
	fs = max(sampleRate, 0);
	tmark = 0.;
	if (fs == 2) bufBlockSize = 1;
}

CSignal::CSignal(double value)
:fs(1), tmark(0.), pf_basic(NULL), pf_basic2(NULL), pf_basic3(NULL)
{
	SetValue(value);
}

CTimeSeries::CTimeSeries(double value)
	: chain(NULL)
{
	fs = 1; 
	tmark = 0;
	SetValue(value);
}

CSignal::CSignal(vector<double> vv)
{
	fs = 1;
	tmark = 0;
	UpdateBuffer((unsigned int)vv.size());
	for (unsigned int k = 0; k < (unsigned int)vv.size(); k++)
		buf[k] = vv[k];
}

CSignal::CSignal(std::string str)
{
	SetString(str.c_str());
}

CSignal::CSignal(const CSignal& src)
:fs(1), tmark(0.)
{
	*this = src;
}

CTimeSeries::CTimeSeries(const CSignal& src)
	: chain(NULL) //REQUIRED!!!!
{
	*this = src;
}

CTimeSeries::CTimeSeries(const CTimeSeries& src)
	:chain(NULL) //REQUIRED!!!!  otherwise, copied object return has uninitialized chain and causes a crash later. 5/24/2048
{
	*this = src;
}

CSignal::CSignal(double *y, int len)
:fs(1), tmark(0.), pf_basic(NULL), pf_basic2(NULL), pf_basic3(NULL)
{
	UpdateBuffer(len);
	memcpy((void*)buf, (void*)y, sizeof(double)*len);
}

vector<double> CSignal::ToVector()
{
	vector<double> out;
	out.resize((size_t)nSamples);
	int k = 0;
	for (vector<double>::iterator it = out.begin(); it != out.end(); it++)
		*it = buf[k++];
	return out;
}


CSignal::~CSignal()
{
	if (buf) {
		delete[] buf; 
		buf=NULL;
	}
}

CTimeSeries::~CTimeSeries()
{
	if (chain) {
		delete chain;
		chain = NULL;
	}
}

CSignal CSignal::size(unsigned int id0, unsigned int len)
{ // no longer used. Get rid of other ::size()
	unsigned int ilen = Len();
	UpdateBuffer(2);
	buf[0] = (double)nGroups;
	buf[1] = (double)ilen;
	if (parg)
		((CVar*)parg)->SetValue((double)ilen);
	return *this;
}

CSignal& CSignal::Reset(int fs2set)	// Empty all data fields - sets nSamples to 0.
{
	body::Reset();
	pf_basic = NULL;
	pf_basic2 = NULL;
	if (fs2set)	// if fs2set == 0 (default), keep the current fs.
		fs = max(fs2set,1);	// not to allow minus.
	tmark = 0;
	return *this;
}

CTimeSeries& CTimeSeries::Reset(int fs2set)	// Empty all data fields - sets nSamples to 0.
{
	CSignal::Reset(fs2set);
	if (chain) {
		delete chain;
		chain = NULL;
	}
	return *this;
}

void CTimeSeries::SetChain(CTimeSeries *unit, double time_shifted)
{
	if (unit!=NULL)
	{
		if (nSamples>0)  delete[] buf;
		buf = NULL; 
		chain = unit;
	}
}

void CTimeSeries::SetChain(double time_shifted)
{
	if (nSamples>0)
	{
		tmark = quantizetmark(time_shifted, fs); ;
		chain = this;
		buf = NULL;
	}
}

CSignal& CSignal::operator=(const body& rhs)
{ // Used only in AuxFunc.cpp 12/30/2017
	if (this != &rhs)
	{
		Reset();
		if (rhs.nSamples>0)
			body::operator=(rhs);
	}
	return *this; // return self-reference so cascaded assignment works
}

CSignal& CSignal::operator=(const CSignal& rhs)
{   // Make a deep copy only for buf, but not for sc, because sc is not necessarily dynamically allocated.
	// Thus, BE Extra careful when making writing an assignment statement about the scaling..
	if (this != &rhs) 
	{
		Reset(rhs.fs);
		if (rhs.fs > 0)
			tmark = quantizetmark(rhs.tmark, rhs.fs);
		else // this way, quantize is skipped for t-seq with relative tmarks.
		{
			tmark = rhs.tmark;
			SetFs(0);
		}
		if (rhs.nSamples>0)
			body::operator=(rhs); 
	}
	return *this;
}

CTimeSeries& CTimeSeries::operator=(const CSignal& rhs)
{
	CSignal::operator=(rhs);
	return *this; // return self-reference so cascaded assignment works
}

void CTimeSeries::SwapContents1node(CTimeSeries &sec)
{	// Swaps chain & tmark.	Leaves "next" intact!!!
	CTimeSeries tmp(fs);
	// tmp = *this
	tmp.body::SwapContents1node(*this);
	tmp.chain = chain, tmp.tmark = tmark;
	// *this = sec
	body::SwapContents1node(sec);
	chain = sec.chain, tmark = sec.tmark; fs = sec.fs;
	// sec = tmp
	sec.body::SwapContents1node(tmp);
	sec.chain = tmp.chain, sec.tmark = tmp.tmark; sec.fs = tmp.fs;
	// Mark chain NULL so that destructor won't try to destroy chain which is used by sec
	tmp.chain = NULL;
}

CTimeSeries& CTimeSeries::operator+=(const double con)
{ 
	body::operator+=(con);
	if (chain) *chain += con; 
	return *this; 
} 

CTimeSeries& CTimeSeries::operator*=(const double con)
{ 
	body::operator*=(con);
	if (chain) *chain *= con; 
	return *this; 
} 

CTimeSeries& CTimeSeries::operator-(void)	// Unary minus
{
	body::operator-();
	if (chain)	-*chain;
	return *this;
}

CTimeSeries& CTimeSeries::operator/=(double con)
{
	body::operator/=(con);
	if (chain) *chain /= con;
	return *this;
}

CSignal& CSignal::Modulate(vector<double> &tpoints, vector<double> &tvals)
{
	//assumption: tpoints is increasing 
	auto itval = tvals.begin();
	for (auto it = tpoints.begin(); it != tpoints.end() - 1; it++)
	{
		if (*it > endt()) continue;
		double t1 = *it;
		double t2 = *(it + 1);
		double slope = (*(itval + 1) - *itval) / (t2 - t1);
		double slopePerSample = (*(itval + 1) - *itval) / (t2 - t1) / fs*1000.;
		unsigned int beginID = max(0, (unsigned int)round((t1 - tmark)*fs / 1000.));
		unsigned int endID = min(nSamples, (unsigned int)round((t2 - tmark)*fs / 1000.));
		for (unsigned int k = beginID; k < endID; k++)
			buf[k] *= *itval + slopePerSample * (k - beginID);
		itval++;
	}
	return *this;
}

CTimeSeries& CTimeSeries::operator+=(CTimeSeries &sec)
{
	if (IsEmpty() || GetType() == CSIG_NULL) return (*this = sec);
	if (sec.IsEmpty() || sec.GetType() == CSIG_NULL) return *this;
	if (IsSingle() && !sec.IsSingle())
		SwapContents1node(sec);
	if (sec.IsString() || IsString()) 
		throw "Addition of string is allowed only with a scalar.";
	AddMultChain('+', &sec);
	return *this;
}


CTimeSeries& CTimeSeries::operator*=(CTimeSeries &sec)
{
	if (IsEmpty() || GetType() == CSIG_NULL) return (*this = sec);
	if (sec.IsEmpty() || sec.GetType() == CSIG_NULL) return *this;
	if (IsSingle() && !sec.IsSingle())
		SwapContents1node(sec);
	AddMultChain('*', &sec);
	return *this;
}

CSignal& CSignal::operator+=(CSignal *yy)
{
	if (yy->IsComplex()) SetComplex();
	if (IsComplex()) yy->SetComplex();
	unsigned int nSamples0(nSamples);
	UpdateBuffer(yy->nSamples + nSamples);
	if (GetType() == CSIG_STRING)
		strcat(strbuf, yy->strbuf);
	else
		memcpy(&logbuf[nSamples0*bufBlockSize], yy->buf, bufBlockSize*yy->nSamples);
	return *this;
}

CTimeSeries& CTimeSeries::operator+=(CTimeSeries *yy)
{
	//Do THIS on 5/24/2018......................
	
	// a little jumbled up with jhpark's previous codes and bjkwon's current codes... 7/6/2016
	if (yy==NULL) return *this;
	if (IsEmpty()) return (*this=*yy);
	if (GetType()==CSIG_NULL) return *this=*yy>>=tmark;
	// Concatenation of CTimeSeries
  // yy might lose its value.
	if (GetFs()<2)
		SetFs(yy->GetFs());

	CTimeSeries *ptail = GetDeepestChain();
	bool bb = GetType() == CSIG_AUDIO;
	bb = yy->GetType() == CSIG_AUDIO; 
	bb = (GetType() == CSIG_AUDIO && yy->GetType() == CSIG_AUDIO) && (yy->chain || yy->tmark);
	if ((GetType()==CSIG_AUDIO && yy->GetType()==CSIG_AUDIO) && (yy->chain || yy->tmark) ) {
		// when insertee has chain(s), chain it instead of copying. yy loses its value.
		CTimeSeries *pNew = new CTimeSeries(fs);
		if (chain && !yy->chain) // I don't know what would be the consequences of this leaving out  5/12/2016 ---check later??
			*pNew = *yy;	// make a copy, leaving the original intact for the next channel
		else
			pNew->SwapContents1node(*yy);		// pNew takes the insertee. As yy lost all its pointers, yy can be destroyed later without destroying any data it used to have.
		*pNew >>= ptail->tmark+ptail->_dur();	// to make the insertee the next chain(s) of ptail.
		ptail->chain = pNew;					// link it to *this.
	} else {
		// otherwise, just copy at the end.
		if (yy->nSamples>0)
		{
			int nSamples0 = ptail->nSamples;
			if (yy->IsComplex()) ptail->SetComplex();
			if (IsComplex()) yy->SetComplex();
			ptail->UpdateBuffer(yy->nSamples + nSamples0);
			if (GetType()==CSIG_STRING)
				strcat(ptail->strbuf, yy->strbuf);
			else
				memcpy(&ptail->logbuf[nSamples0*bufBlockSize], yy->buf, bufBlockSize*yy->nSamples);
		}
		if (GetType()!=CSIG_AUDIO && IsComplex())
		{
			for (unsigned int k=0; k<nSamples; k++) 
				if (imag(cbuf[k])!=0.) return *this;
			//if it survives this far, it should be set real
			SetReal();
		}
	}
	return *this;
}

bool CSignal::operator==(const CSignal &rhs)
{
	if (rhs.bufBlockSize != bufBlockSize) return false;
	if (rhs.fs != fs) return false;
	if (rhs.nSamples != nSamples) return false;
	if (bufBlockSize==1)
		for (unsigned int k=0; k < nSamples; k++)
		{
			if (logbuf[k] != rhs.logbuf[k]) return false;
		}
	else if (bufBlockSize == 8)
		for (unsigned int k=0; k < nSamples; k++)
		{
			if (buf[k] != rhs.buf[k]) return false;
		}
	else //if (bufBlockSize == 16)
		for (unsigned int k=0; k < nSamples; k++)
		{
			if (cbuf[k] != rhs.cbuf[k]) return false;
		}
	return true;
}

bool CSignal::operator==(double rhs)
{
	return *this == CSignal(rhs);
}

bool CSignal::operator==(std::string rhstr)
{
	return *this == CSignal(rhstr);
}

double CSignal::dur(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	return 1000. / fs * len;
}

double CSignal::begint(unsigned int id0, unsigned int len)
{
	return tmark + id0 * 1000. / fs;
}

double CSignal::endt(unsigned int id0, unsigned int len)
{
	return begint(id0,len) + dur(id0,len);
}

double CSignal::length(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	if (GetType() == CSIG_STRING) 
		return (double)strlen(strbuf); 
	else 
		return len; 
}

CSignal CTimeSeries::TSeries2CSignal()
{
	if (GetType() != CSIG_TSERIES)
		throw "Internal error---must be called by a member type of CSIG_TSERIES";
	CTimeSeries out;
	unsigned int k(0), count = CountChains();
	out.UpdateBuffer(count);
	for (CTimeSeries *p = this; p; p = p->chain, k++)
		out.buf[k] = p->value();
	return out;
}

CTimeSeries CTimeSeries::basic(double (CSignal::*pf_basic)(unsigned int, unsigned int), void *popt)
{
	parg = popt;
	int _fs = fs == 2 ? 1 : fs;
	CTimeSeries out(_fs);
	if (GetType() == CSIG_TSERIES)
	{
		out.Reset(1); // 1 means new fs
		CSignal tp = TSeries2CSignal();
		out.SetValue((tp.*(this->pf_basic))(0, 0));
	}
	else
	{
		CTimeSeries tp(_fs);
		out.tmark = tmark;
		unsigned int len = Len();
		CVar additional(_fs);
		additional.UpdateBuffer(nGroups);
		additional.nGroups = nGroups;
		if (fs > 2) // if audio
		{
			for (unsigned int k = 0; k < nGroups; k++)
			{
				tp.tmark = tmark+round(1000.*k*Len()/fs);
				tp.SetValue((this->*(this->pf_basic))(k*len, len));
				out.AddChain(tp);
				if (parg)
				{
					((CVar*)parg)->SetFs(_fs);
					additional.buf[k] = ((CVar*)parg)->value(); // fed from individual function
				}
			}
		}
		else
		{
			out.UpdateBuffer(nGroups);
			out.nGroups = nGroups;
			additional.UpdateBuffer(nGroups);
			additional.nGroups = nGroups;
			for (unsigned int k = 0; k < nGroups; k++)
			{
				out.buf[k] = (this->*(this->pf_basic))(k*len, len);
				if (parg)
				{
					// In max() or min(), fs information of parg is wiped out by SetValue(). We need to recoever it here.
					// fs information should be retained because the output is T_SEQ
					// This may not be a permanent solution for other functions. But it's OK for now. 
					// Need to think about it again when more functions are added with multiple output args
					// 9/14/2018s
					((CVar*)parg)->SetFs(_fs);
					additional.buf[k] = ((CVar*)parg)->value(); // fed from individual function
				}
			}
		}
		for (CTimeSeries *p = chain; p; p = p->chain)
		{
			p->parg = popt;
			tp.tmark = p->tmark;
			tp.SetValue((p->*(this->pf_basic))(0,0));
			out.AddChain(tp);
			if (p->parg)
			{
				((CVar*)(p->parg))->tmark = p->tmark;
				additional.AddChain(*(CVar*)p->parg); // fed from individual function
			}
		}
		if (parg)	*(CVar*)parg = additional;
	}
	return out;
}
//For pf_basic2 and pf_basic3, do the same, as above pf_basic, and use parg //9/14/2018

CTimeSeries& CTimeSeries::basic(CSignal& (CSignal::*pf_basic2)(unsigned int, unsigned int), void *popt)
{
	parg = popt;
	if (GetType() == CSIG_TSERIES)
	{
		//out.Reset(1); // 1 means new fs
		//CSignal tp = TSeries2CSignal();
		//out.SetValue((tp.*(this->pf_basic2))(0, 0));
	}
	else
	{
		unsigned int len = Len();
		for (unsigned int k = 0; k < nGroups; k++)
			(this->*(this->pf_basic2))(k*len, len);
		for (CTimeSeries *p = chain; p; p = p->chain)
		{
			p->parg = popt;
			(p->*(this->pf_basic2))(0, p->nSamples);
		}
	}
	return *this;
}

CTimeSeries CTimeSeries::basic(CSignal(CSignal::*pf_basic3)(unsigned int, unsigned int), void *popt)
{
	parg = popt;
	int _fs = fs == 2 ? 1 : fs;
	CTimeSeries out(_fs);
	if (GetType() == CSIG_TSERIES)
	{
		//out.Reset(1); // 1 means new fs
		//CSignal tp = TSeries2CSignal();
		//out.SetValue((tp.*(this->pf_basic2))(0, 0));
	}
	else
	{
		CSignal sbit0, sbit(_fs);
		sbit.tmark = tmark;
		out.tmark = tmark;
		unsigned int len = Len();
		for (unsigned int k = 0; k < nGroups; k++)
		{
			out += &(CTimeSeries)(this->*(this->pf_basic3))(k*len, len);
//			sbit0 += &sbit;
		}
		out.nGroups = nGroups;
		CTimeSeries tp(sbit);
		for (CTimeSeries *p = chain; p; p = p->chain)
		{
			p->parg = popt;
			tp = (p->*(this->pf_basic3))(0, 0);
			tp.tmark = p->tmark;
			out.AddChain(tp);
		}
//		out = tp;
	}
	return out;
}

CSignal &CSignal::matrixmult(CSignal *arg)
{
	unsigned int col = Len();
	unsigned int col2 = arg->Len();
	if (col != arg->nGroups)
		throw "Column count must be the same as row count in the argument.";
	body out;
	out.UpdateBuffer(nGroups * col2);
	out.nGroups = nGroups;
	for (unsigned int m = 0; m < nGroups; m++)
		for (unsigned int n = 0; n < col2; n++)
		{
			double tp = 0;
			for (unsigned int k = 0; k < col; k++)
			{
				tp += buf[m*col + k] * arg->buf[k*col2 + n];
			}
			out.buf[m*col2 + n] = tp;
		}
	return *this = out;
}

void CTimeSeries::AddMultChain(char type, CTimeSeries *sec)
{
	if (fs > 1 && sec->fs > 1 && fs != sec->fs)  throw "The sampling rates of both operands must be the same.";

	CTimeSeries *p;
	if (IsComplex() || sec->IsComplex()) 
	{ SetComplex(); sec->SetComplex(); }
	if (sec->GetType() == CSIG_TSERIES)
	{
		if (GetType() == CSIG_TSERIES) //special case: point by point operation
		{
			if (CountChains() != sec->CountChains())
				throw "Both tseq must be the same length.";
			for (CTimeSeries *p = this, *pp = sec; p; p = p->chain, pp = pp->chain)
			{
				if (p->nSamples != pp->nSamples)
					throw "Both tseq must have the same number of elements.";
			}
			for (CTimeSeries *p = this, *pp = sec; p; p = p->chain, pp = pp->chain)
			{
				if (type=='*')	
					for (unsigned int k = 0; k < p->nSamples; k++)	p->buf[k] *= pp->buf[k];
				else			
					for (unsigned int k = 0; k < p->nSamples; k++)	p->buf[k] += pp->buf[k];
			}
			return;
		}
		else if (type == '+')
			SwapContents1node(*sec); //if sec is tseq and this is not, swap here
	}
	if (GetType() != CSIG_AUDIO || sec->GetType() != CSIG_AUDIO)
	{
		//special case: multiplication after interpolation
		if (type == '*' && (GetType() == CSIG_TSERIES || sec->GetType() == CSIG_TSERIES))
		{
			// need to pass down the case of scalar * tseq
			if ((GetType() == CSIG_AUDIO && sec->GetType() == CSIG_TSERIES) || (sec->GetType() == CSIG_AUDIO && GetType() == CSIG_TSERIES))
			{
				if (GetType() == CSIG_TSERIES)
					SwapContents1node(*sec);
				bool relTime = sec->fs == 0;
				vector<double> tpoints, tvals;
				for (CTimeSeries *p = sec; p; p = p->chain)
				{
					tpoints.push_back(p->tmark);
					tvals.push_back(p->value());
				}
				if (relTime)
				{
					for (auto &tp : tpoints)
						tp *= dur();
				}
				for (CTimeSeries *p = this; p; p = p->chain)
					p->CSignal::Modulate(tpoints, tvals);
				return;
			}
		}
		for (p = this; p; (p = p->chain) && (sec->nSamples>0))
		{
			for (unsigned int k = 0; k < p->nGroups; k++)
				p->addmult(type, *sec, k*p->Len(), p->Len());
			//if (type == '*') // I don't know why this is left.... from v1.45
			//{
			//	if (sec->nSamples >p->nSamples) sec->nSamples -= p->nSamples;
			//	else sec->nSamples = 0;
			//	if (sec->nSamples>0)	sec->buf += sizeof(sec->buf)*p->nSamples;
			//}
		}
		return;
	}

	vector<double> thistmark;
	vector<double> sectmark;
	for (p=this; p; p=p->chain)
	{
		thistmark.push_back(p->tmark);
		thistmark.push_back(p->tmark + p->_dur());
	}
	for (p=sec; p; p=p->chain)
	{
		sectmark.push_back(p->tmark);
		sectmark.push_back(p->tmark + p->_dur());
	}
	unsigned int k, im(0), is(0); // im: index for master (this), is: index for sec
	double anc1, anc0;
	short status(0);
	vector<double> anc; // just for debugging/tracking purposes... OK to delete.
	unsigned int count;
	anc0 = min(thistmark[im],sectmark[is]);
	anc.push_back(anc0);
	CTimeSeries out(sec->GetFs()), part(sec->GetFs());
	if (IsComplex() || sec->IsComplex()) part.SetComplex();
	do{
	if ( im==thistmark.size() && is==sectmark.size() )
		break;
	if ( is==sectmark.size() ) //only thistmark is left
		status += (short)(pow(-1.,(int)im)), im++; 
	else if ( im==thistmark.size() ) //only sectmark is left
		status += (short)(2*pow(-1.,(int)is)), is++;
	else if (thistmark[im]<=sectmark[is]) // both are available. check them
		status += (short)(pow(-1.,(int)im)), im++; 
	else
		status += (short)(2*pow(-1.,(int)is)), is++;
	if (im==thistmark.size() && is==sectmark.size() )
		break;
	else if (im<thistmark.size() && is<sectmark.size() )
		anc1 = min(thistmark[im],sectmark[is]);
	else // this means only one of the two, thistmark and sectmark, is left, just pick the available one.
		if (sectmark.size()-is>0)
			anc1 = sectmark[is];
		else
			anc1 = thistmark[im];
	anc.push_back(anc1);

	if (status>0) // if status is 0, skip this cycle--no need to add chain
	{
		CTimeSeries *pm = this;
		CTimeSeries *ps = sec;
		// status indicates in the time range between the last two values of anc, what signal is present
		// 1 means this, 2 means sec, 3 means both
		count = (unsigned int)round(( anc1-anc0 )/1000.*fs);
		unsigned int blockSize = bufBlockSize/sizeof(double);
		{
			part.UpdateBuffer(count);
			part.tmark = anc0;
			part.nGroups = nGroups;
			if (status&1) 
			{
				for (k=0, p=pm; k<im/2; k++/*, p=p->chain*/)
				{
					if (p->chain!=NULL) p=p->chain;
					else				break;
				}
				int id = (int)round(( anc0-p->tmark )/1000.*fs);
				memcpy(part.buf, p->logbuf+id*bufBlockSize, part.nSamples*bufBlockSize);
			}
			if (status&2) 
			{
				for (k=0, p=ps; k<is/2; k++)
					if (p->chain!=NULL) p=p->chain;
				int id = (int)round(( anc0-p->tmark )/1000.*fs);
				if (status&1) // type=='+' add the two,  type=='*' multiply
				{
					if (type=='+')
					{
						if (part.IsComplex()) for (unsigned int k = 0; k < count; k++)	part.cbuf[k] += p->cbuf[k + id];
						else				 for (unsigned int k = 0; k < count; k++)	part.buf[k] += p->buf[k + id];
					}
					else if (type=='*')
					{
						if (part.IsComplex()) for (unsigned int k=0; k<count; k++)	part.cbuf[k] *= p->cbuf[k+id];
						else				 for (unsigned int k=0; k<count; k++)	part.buf[k] *= p->buf[k+id];
					}
				}
				else 
				{
					memcpy(part.buf, p->logbuf+id*bufBlockSize, part.nSamples*bufBlockSize);
				}
			}
		}
		if (out.tmark==0. && out.nSamples==0) // the very first time
			out = part;
		else if (count>0 && part.nSamples>0) // if count==0, don't add part (which might be leftover from the previous round)
			out.AddChain(part);
	}
	anc0 = anc1;
	} while (im<thistmark.size() || is<sectmark.size() );
	*this = out.ConnectChains();
}

CTimeSeries& CTimeSeries::ConnectChains()
{
	CTimeSeries *p(this), out(fs);
	if (p==NULL || p->chain==NULL) return *this;
	if (tmark>0)
	{
		double shift(tmark);
		for (; p; p=p->chain)	p->tmark -= shift;
		ConnectChains();
		p=this;
		for (; p; p=p->chain)	p->tmark += shift;
		return *this;
	}
	while(1)
	{
		CTimeSeries part(fs);
		if (IsComplex()) part.SetComplex();

		part.tmark = p->tmark;
		unsigned int count(0);
		// Get the count needed to consolidate 
		for (CTimeSeries *tp(p); ;) {
			count += tp->nSamples;
			//at this point tmark should have been adjusted to be very close to align with the fs grid, i.e., tp->chain->tmark/1000.*fs would be very close to an integer
			// if it is an integer, tmark was already aligned with the grid so no need to adjust
			// if it is not an integer (but very close to one), it should be forced to be the one.. now I'm adding .01 
			// this raises a possibility to make tp->chain->tmark/1000.*fs overrun (end up making one greater than it should be) if its decimal is 0.49, but that's not likely...
			// 6/4/2016 bjk
			// if nSamples so far is the same as the following chain tmark, it means continuing (i.e., the chain consolidated)
			// "Being the same" is too restrictive. Allow error margin of 2.
			// 7/18/2016 bjk
			if (tp->chain!=NULL && fabs(tp->chain->tmark/1000.*fs - count ) <=2 )
				tp=tp->chain;
			else
				break;
		}
		// Consolidate 
		part.UpdateBuffer(count);
		int offset(0);
		while (1) {
			memcpy(part.logbuf+offset, p->buf, p->nSamples*bufBlockSize);
			offset += p->nSamples*bufBlockSize;
			if (p->chain!=NULL && offset==(int)(p->chain->tmark/1000.*fs+.1)*bufBlockSize)
				p=p->chain;
			else
				break;
		}
		if (out.tmark==0. && out.nSamples==0) // the very first time
			out = part;
		else
			out.AddChain(part);
		if (p->chain==NULL) break;
		p=p->chain;
	}
	return (*this=out);
}

CSignal& CSignal::Diff(int order)
{
	unsigned int q, len = Len();
	for (unsigned int p = 0; p < nGroups; p++)
	{
		q = p * (len - order);
		for (; q< (p+1)*(len - order); q++)
			buf[q] = buf[q + order + p] - buf[q + p];
	}
	if (nGroups>1)
		UpdateBuffer(nSamples - (len-1)*order);
	else
		UpdateBuffer(nSamples - 1);
	//for (unsigned int k=0; k<nSamples-order; k++)	
	//	buf[k] = buf[k+order]-buf[k];
	return *this;
}

CSignal& CSignal::Cumsum()
{
	unsigned int len = Len();
	for (unsigned int p = 0; p < nGroups; p++)
	{
		unsigned int q = 1 + p * len;
		for (; q < (p+1)*len; q++)
			buf[q] = buf[q - 1] + buf[q];
	}
	return *this;
}

CTimeSeries& CTimeSeries::operator-=(CTimeSeries &sec)
{
	return *this += -sec;
}

CTimeSeries& CTimeSeries::operator/=(CTimeSeries &scaleArray)
{
	return *this *= scaleArray.reciprocal();
}

CSignal& CSignal::reciprocal(void)
{
	for (unsigned int k=0; k<nSamples; k++)
		buf[k] = 1.0/buf[k];
	return *this;
}

CTimeSeries& CTimeSeries::reciprocal(void)
{
	CSignal::reciprocal();
	return *this;
}

CSignal& CSignal::operator>>=(double delta)
{
	if (delta == 0)		return *this;
	double newtmark = quantizetmark(tmark + delta, fs);
	if (newtmark < 0) // cut off negative time domain
	{
		double pts2remove = newtmark / 1000. * fs;
		if (pts2remove>nSamples) {	// nothing left
			Reset();
			return *this;
		}
		tmark = 0;
		nSamples -= (unsigned int)pts2remove;
		for (unsigned int k = 0; k < nSamples; k++)
			buf[k] = buf[k + (unsigned int) pts2remove];
	}
	tmark += delta;
	return *this;
}

CTimeSeries& CTimeSeries::operator>>=(double delta)
{
	for (CTimeSeries *p = this; p; p = p->chain)
	{ // CSignal with the face of CTimeSeries. No need to worry about chain processing
		CSignal *pp = (CSignal*)p;
		pp->CSignal::operator>>=(delta);
		if (pp->nSamples == 0)
		{
			Reset();
		}
	}
	//if (tmark+delta < 0) 
	//{
	//	if (p != this) {
	//		pp->chain = NULL;	// to stop destruction at pp. This has to be done before SwapContents1node() for the cases of this==pp. jhpark 5/29/2012.
	//		SwapContents1node(*p);	// *p gets *this
	//		delete p;	// delete from head through pp
	//	}
	//}
	return *this;
}

CTimeSeries& CTimeSeries::operator=(const CTimeSeries& rhs)
{
	if (this != &rhs)
	{
		CSignal::operator=(rhs);
		outarg = rhs.outarg;
		if (rhs.chain)
		{
			chain = new CTimeSeries;
			*chain = *rhs.chain;
		}
		else
		{
			delete chain;
			chain = NULL;
		}
	}
	return *this; // return self-reference so cascaded assignment works
}

CTimeSeries& CTimeSeries::AddChain(CTimeSeries &sec)
{ // MAKE SURE sec is not empty
	if (nSamples == 0)
		return *this = sec;
	if (chain == NULL)
	{
		chain = new CTimeSeries;
		return *chain = sec;
	}
	else
		return chain->AddChain(sec);
}

CTimeSeries * CTimeSeries::ExtractDeepestChain(CTimeSeries *dummy)
{
	// breaks and returns the deepest chain. Without dummy it would try to return local variable
	if (chain==NULL) return this;
	else if (chain->chain==NULL) return BreakChain(dummy);
	else return chain->ExtractDeepestChain(dummy);
}

CTimeSeries * CTimeSeries::BreakChain(CTimeSeries *dummy)
{
	// returns the broken chain
	if (chain==NULL) return NULL;
	dummy = chain;
	chain = NULL;
	return dummy;
}

CTimeSeries * CTimeSeries::GetDeepestChain()
{
	CTimeSeries *p(this);
	for (; p->chain; p=p->chain)
		;
	return p;
}

CSignal *CTimeSeries::ChainOrd(unsigned int order)
{
	if (order == 0) return (CSignal*)this;
	unsigned int k = 0;
	for (CTimeSeries *p = this; k < order && p->chain; p = p->chain, k++)
		if (k == order - 1) return (CSignal*)p->chain;
	return nullptr;
}

unsigned int CTimeSeries::CountChains(unsigned int *maxlength)
{
	// verify again all involved with this 9/18/2018
	unsigned int maxy(-1), res(1);
	CTimeSeries *p;
	for (p=this; p->chain; p=p->chain, res++)
		maxy = max(p->nSamples,maxy);
	if (maxlength) *maxlength = maxy;
	return res;
}

CTimeSeries& CTimeSeries::dynaMA(CTimeSeries &num)
{
	return *this;
}

CTimeSeries& CTimeSeries::dynaAR(CTimeSeries &den)
{
	return *this;
}


double CSignals::alldur()
{
	double out = CTimeSeries::alldur();
	if (next)
		out = max(out, next->alldur());
	return out;
}

double CTimeSeries::alldur()
{
	double out = CSignal::endt();
	CTimeSeries *p(this);
	for (; p; p = p->chain)
		out = p->CSignal::endt();
	return out;
}

double CTimeSeries::RMS()
{ // This does not go into next.... for stereo signals, call RMS specifically, like next->RMS()  bjk 4/23/2016
	double cum(0);
	int count(0);
	CTimeSeries *p(this);
	for (; p; p = p->chain)
	{
		for (unsigned int i = 0; i<p->nSamples; ++i, ++count)
			cum += p->buf[i] * p->buf[i];
	}
	return _getdB(sqrt(cum / count));
}

double CTimeSeries::MakeChainless()
{ //This converts the null intervals of the signal to zero.
	fs = max(fs, 1);
	double newdur = alldur();	// doing this here as fs might have changed.
	if (!tmark && !chain)	// already chainless && no padding required.
		return newdur;

	CTimeSeries nsig(fs);
	nsig.UpdateBuffer((unsigned int)round(newdur/1000.*fs));
	for (CTimeSeries *p=this; p; p=p->chain) {
		if (p->tmark + p->_dur() <= 0)
			continue;
		unsigned int iorg = (unsigned int)( (p->tmark < 0) ? round(-p->tmark/1000.*fs) : 0 );
		unsigned int inew = (unsigned int) ((p->tmark > 0) ? round(p->tmark/1000.*fs) : 0 );
		unsigned int cplen = p->nSamples-iorg;
		if (inew+cplen > nsig.nSamples) {
			if (p->chain == NULL && inew+cplen == nsig.nSamples+1 && newdur/1000.*fs-nSamples > 0.499999)	// last chain, only 1 sample difference, possile rounding error.
				nsig.UpdateBuffer(nsig.nSamples+1);
			else
				throw "Internal error: Buffer overrun detected.";
		}
		memcpy(nsig.buf+inew, p->buf+iorg, cplen*sizeof(*buf));
	}
	SwapContents1node(nsig);	// *this gets the chainless signal, nsig gets the chained signal and will be destroyed soon.
	return newdur;
}


double CSignal::RMS(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	if (len == 0) return std::numeric_limits<double>::infinity();
	double cum(0);
	for (unsigned int k = id0; k<id0 + len; k++)
		cum += buf[k] * buf[k];
	return _getdB(sqrt(cum / len));
}


CSignal& CSignal::Interp(const CSignal& gains, const CSignal& tmarks)
{
	if (gains.nSamples!=tmarks.nSamples)
		throw "The length of both arguments of interp( ) must be the same.";
	int id1(0), id2;
	double gain1(0.), slope;
	int newSampleCount = (int)round(tmarks.buf[tmarks.nSamples-1]/1000.*fs);
	Reset();
	UpdateBuffer(newSampleCount);
	for (unsigned int i=0; i<tmarks.nSamples; i++)
	{
		id2 = (int)round(tmarks.buf[i]/1000.*fs);
		slope = (gains.buf[i]-gain1) / (id2-id1);
		for (int j=id1; j<id2; j++)
			buf[j] = slope * (j-id1) + gain1;
		gain1 = gains.buf[i];
		id1 = id2;
	}
	return *this;
}

CSignal& CSignal::Take(CSignal& out, unsigned int id1, unsigned int id2)
{
	out.Reset();
	unsigned int nSamplesNeeded;
	if (id2<id1)		// Time reversal
	{
		if (id2>nSamples-1) return *this;
		id1 = min (nSamples-1, id1);
		id2 = max (0, id2);
		nSamplesNeeded = id1-id2+1;
		out.UpdateBuffer(nSamplesNeeded);
		for (unsigned int k=0; k<nSamplesNeeded; k++)
			out.buf[k] = buf[id1-k];
	}
	else
	{
		if (id1>nSamples-1) return *this;
		id1 = max (0, id1);
		id2 = min (nSamples-1, id2);
		nSamplesNeeded = id2-id1+1;
		out.UpdateBuffer(nSamplesNeeded);
		memcpy((void*)&out.buf[-min(0,id1)], (void*)&buf[max(0,id1)], sizeof(double)*(id2-max(0,id1)+1));
	}
	return out;
}

CTimeSeries& CTimeSeries::Squeeze()
{
	int nSamplesTotal(0), nSamples0(nSamples);
	for (CTimeSeries* p(this); p; p=p->chain)
		nSamplesTotal += p->nSamples;
	UpdateBuffer(nSamplesTotal);
	nSamplesTotal=nSamples0;
	for (CTimeSeries* p(chain); p; p=p->chain)
		memcpy(&buf[nSamplesTotal], p->buf, p->nSamples*sizeof(p->buf[0])),nSamplesTotal+=p->nSamples;
	delete chain;
	chain = NULL;
	return *this;
}

CTimeSeries& CTimeSeries::MergeChains()
{// This tidy things up by removing unnecessary chains and rearranging them.
	CTimeSeries temp;
	if (nSamples==0 && chain) {temp = *chain; chain=NULL; *this = temp; }

	for (CTimeSeries* p(this); p && p->chain; p=p->chain)
	{
		double et = p->tmark+p->_dur();
		if (et >=p->chain->tmark) // consolidate 
		{
			temp = *p->chain;
			int id1 = (int)round((temp.tmark-p->tmark)/1000.*fs); 
			int common = p->nSamples - id1;
			int id2 = temp.nSamples - common;
			int oldnSamples(p->nSamples); 
			p->UpdateBuffer(p->nSamples+temp.nSamples-common);
			/* overlapping portion */ 
			for (int i=0; i<common; i++) 
				p->buf[oldnSamples-common+i] += temp.buf[i];
			for (unsigned int k=0; k<temp.nSamples-common; k++)
				p->buf[oldnSamples+k] = temp.buf[common+k];
			if (temp.chain==NULL)	p->chain = NULL;
			else *p->chain = *temp.chain; // deep copy is necessary (we are losing p).
		}
	}
	return *this;
}

CTimeSeries& CTimeSeries::MC(CTimeSeries &out, vector<double> tmarks, int id1, int id2)
{
	CTimeSeries parts;
	double tp1 = ((double)id1/fs)*1000.;
	double tp2 = ((double)id2/fs)*1000.;
	for (vector<double>::reverse_iterator iter=tmarks.rbegin(); iter!=tmarks.rend(); iter +=2)
	{
		int idtmark = (int)round((*iter*fs)/1000.);
		int idendt = (int)round((*(iter+1)*fs)/1000.);
		if (id1>idendt) continue;
		vector<double>::reverse_iterator it = iter+2;
		if (it!=tmarks.rend() && tp2 > *it)
		{// id2 over-reaches beyond the block, we need to limit in within a block
			if (out.nSamples==0 && out.tmark==0) 
				MC(out, tmarks, idtmark, idendt);
			else
			{
				MC(out, tmarks, idtmark, idendt);
	//			out.AddChain(out);
			}
		}
		else // if it does not over-reach
		{
			int iBegin = max(idtmark, id1);
			int iEnd = min(id2, idendt)-1;
			if (iEnd<iBegin) 
				return out; // both two points in the null interval
			int count = iEnd - iBegin + 1;
			parts.Reset(fs);
			parts.UpdateBuffer(count);
			memcpy(parts.buf, buf + iBegin, sizeof(double)*count);
			parts.tmark = ((double)iBegin/fs)*1000.;
			if (out.nSamples==0 && out.tmark==0) out = parts;
			else out.AddChain(parts);
			return out;
		}
	}
	return *this = out;
}

CTimeSeries& CTimeSeries::LogOp(CTimeSeries &rhs, int type)
{
	for(CTimeSeries *p = this; p; p=p->chain)
		p->body::LogOp(rhs, type);
	return *this;
}

CTimeSeries& CTimeSeries::removeafter(double timems)
{ // if timems is in the grid, the point is removed (but dur will be until that grid point)
	CTimeSeries *last;
	for (CTimeSeries *p(this); p; p=p->chain)
	{
		if (timems > p->tmark+p->_dur()) {last=p; continue; }
		else if (timems > p->tmark) 
		{
			// no need to worry about p->tmark
			// all points occuring on and after timems will be removed (dur will still be timems)
			// if timems exceeds the grid of p->tmark, it won't be removed, so it is ceil.
			p->nSamples = mceil((timems-p->tmark)*fs/1000.);
		}
		else
		{
			delete[] p;
			last->chain=NULL;
			break;
		}
		last=p;
	}
	return *this;
}

CTimeSeries& CTimeSeries::timeshift(double timems)
{ // if timems is in the grid, the point is kept.
	int chainlevel(0);
	CTimeSeries *p(this);
	for (; p ; p=p->chain)
	{
		if (timems > p->tmark+p->_dur()) { chainlevel++; continue; }
		if (timems < p->tmark) 
			p->tmark -= timems;
		else
		{
			// p->tmark should be zero, except for the small off-granular timing based on the difference between timems and p->tmark
			// the decrease of points will step-up if timems exceeds the grid, so it is ceil.
			int pointsless = mceil((timems-p->tmark)*fs/1000.);
			if (pointsless > 0)
			{
				p->nSamples -= pointsless;
				bool *tbuf = new bool[p->nSamples*bufBlockSize];
				memcpy(tbuf, p->buf + pointsless, p->nSamples*bufBlockSize);
				delete[] p->logbuf;
				p->logbuf = tbuf;
				p->tmark = 0;
			}
		}
	}
	//all chains at and prior to chainlevel are cleared here.
	p = this;
	for (int k(0); k<chainlevel ; k++, p=p->chain)
		delete[] p->buf;
	if (p!=this) // or if chainlevel is non-zero 
	{
		// Make new chain after chainlevel. If p is NULL (make an empty CTimeSeries object to return);
		if (!p) {p = new CTimeSeries;}
		nSamples = p->nSamples;
		tmark = p->tmark;
		buf = p->buf;
		chain = NULL;
	}
	return *this;
}

CTimeSeries CTimeSeries::Extract(double begin_ms, double end_ms)
{
	CTimeSeries out(*this);
	out.removeafter(end_ms);
	out.timeshift(begin_ms);
	return out;
}

CTimeSeries& CTimeSeries::Crop(double begin_ms, double end_ms)
{
	if (begin_ms == end_ms) {Reset(); return *this;}
	vector<double> tmarks;
	if (begin_ms > end_ms) {
		Crop(end_ms, begin_ms);
		ReverseTime();
		return *this;
	}
	removeafter(end_ms);
	CTimeSeries p = timeshift(begin_ms);
	return *this;
}


CSignal &CSignal::_atmost(unsigned int id, int unsigned len)
{
	double limit;
	if (len == 0) len = nSamples;
	CVar param = *(CVar*)parg;
	if (param.IsScalar())
		limit = param.value();
	else
		limit = (id / len<nSamples) ? param.buf[id / len] : std::numeric_limits<double>::infinity();
	for (unsigned int k = id; k < id + len; k++)
		if (buf[k] > limit) buf[k] = limit;
	return *this;
}

CSignal &CSignal::_atleast(unsigned int id, int unsigned len)
{
	double limit;
	if (len == 0) len = nSamples;
	CVar param = *(CVar*)parg;
	if (param.IsScalar())
		limit = param.value();
	else
		limit = (id / len<nSamples) ? param.buf[id / len] : -std::numeric_limits<double>::infinity();
	for (unsigned int k = id; k < id + len; k++)
		if (buf[k] < limit) buf[k] = limit;
	return *this;
}

void CSignal::Dramp(double dur_ms, int beginID)
{
	// Cosine square ramping and damping of the array
	double x, drampFs;
	unsigned int nSamplesNeeded;

	drampFs = 1.e3 / (4.*dur_ms);
	nSamplesNeeded = (unsigned int)round(dur_ms / 1000.*fs);
	nSamplesNeeded = min(nSamples, nSamplesNeeded);
	for (unsigned int i = 0; i<nSamplesNeeded; i++)
	{
		x = sin(2 * PI*drampFs*(double)i / (double)fs);
		buf[beginID + i] = x * x * buf[beginID + i];
		buf[nSamples - i - 1] = x * x * buf[nSamples - i - 1];
	}
}

CSignal& CSignal::dramp(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	double dur_ms = *(double*)parg;
	double drampFs = 1.e3 / (4.*dur_ms);
	unsigned int nSamplesNeeded = (unsigned int)round(dur_ms / 1000.*fs);
	nSamplesNeeded = min(nSamples, nSamplesNeeded);
	for (unsigned int i = 0; i<nSamplesNeeded; i++)
	{
		double x = sin(2 * PI * drampFs * i / fs);
		buf[id0 + i] *= x * x ;
		buf[id0 + len - i - 1] *= x * x;
	}
	return *this;
}

CSignal& CSignal::Hamming(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	for (unsigned int k = 0; k < len; k++)
		buf[id0 + k] *= 0.54 - 0.46*cos(2.0*PI*k / (len - 1.0));
	return *this;
}

CSignal& CSignal::Blackman(unsigned int id0, unsigned int len)
{
	double alpha = *(double*)parg;
	for (unsigned int k = 0; k < len; k++)
		buf[id0 + k] *= (1 - alpha) / 2 - 0.5*cos(2.0*PI*k / (len - 1.0)) + alpha / 2 * cos(4.0*PI*k / (len - 1.0));
	return *this;
}

double * CSignal::Noise(double dur_ms)
{
	unsigned int nSamplesNeeded = (unsigned int)round(dur_ms/1000.*fs);
	double *p = Noise(nSamplesNeeded);
	return p;
}

double * CSignal::Noise(unsigned int nsamples)
{
	Reset();
	UpdateBuffer(nsamples); //allocate memory if necessary
	for (unsigned int i=0; i<nsamples; i++)
	{
		buf[i] = 2.*((double)rand()/(double)RAND_MAX-.5);
//		buf[i] = _double_to_24bit(x);
	}
	return buf;
}

double * CSignal::Noise2 (double dur_ms)
{ //Gaussian noise
	unsigned int nSamplesNeeded = (unsigned int)round(dur_ms/1000.*fs);
	double *p = Noise2(nSamplesNeeded);
	return p;
}

double * CSignal::Noise2 (unsigned int nsamples)
{ //Gaussian noise
	double fac, r, v1, v2, sum(0.);
	Reset();
	UpdateBuffer(nsamples); //allocate memory if necessary
	for (unsigned int i=0; i<nsamples; i++)
	{
		do {
			do {
					v1 = (2.0 * (double)rand()/(double)RAND_MAX ) - 1.0;
					v2 = (2.0 * (double)rand()/(double)RAND_MAX ) - 1.0;
					r = (v1*v1) + (v2*v2);
				} while (r >= 1.0);
			fac = sqrt(-2.0 * log(r) / r);
		} while (v2*fac>=1.0 || v2*fac<=-1.0);
		buf[i] = v2*fac;
		sum += v2*fac;
	}
	return buf;
}

#ifndef NO_RESAMPLE

double * CSignal::fm2(CSignal flutter, int multiplier, char *errstr)
{   // Modulate the entire signal by the array flutter ( > 1, fast-forward, higher-pitched, < 1 slow-down, lower-pitched
	// the length of flutter should be A LOT shorter than nSamples (e.g., 1 tendth of it), but that is checked and exception is handled in AuxFunc.cpp
	flutter *= multiplier;
	for (unsigned int k(0); k<flutter.nSamples; k++) flutter.buf[k] = (double)((int)(flutter.buf[k]+.5));
	flutter /= multiplier;
	CSignals newaudio(*this);
	//First estimate the buffer length at the end of the processing, so we don't call UpdateBuffer multiple times.
	//assume that chain==NULL
	unsigned int nSamples0(nSamples);
	double MeanRate = flutter.mean(0, flutter.nSamples);
	if (MeanRate<1.03) newaudio.UpdateBuffer((unsigned int)(nSamples/MeanRate*1.1)); // if MeanRate is less than 1, buf should be expanded. 1.03 just as a buffer, add 10% as another shock buffer.
	if (Resample(multiplier*fs, errstr)==NULL) return NULL;
	SetFs(fs);
	vector<unsigned int> chunks;
	splitevenindices(chunks, nSamples, flutter.nSamples); // check about here....nSamples0 or nSamples.. not sure....
	int m(0), cum(0);
	unsigned int id(0), idbig(cum);
	for (vector<unsigned int>::iterator chit=chunks.begin(); chit!=chunks.end(); chit++, m++)
	{
		int bigskip((int)(multiplier*flutter.buf[m]));
		for (id=idbig=0; idbig<*chit; id++, idbig += bigskip)
		{
			newaudio.buf[cum+id] = buf[cum+idbig]; // multiplier*flutter.buf[m] should be an integer.
		}
		cum += id;
	}
	newaudio.nSamples = cum;
	*this = newaudio; 
	return buf;
}

#endif //NO_RESAMPLE

double * CSignal::fm(double midFreq, double fmWidth, double fmRate, int nsamples, double beginFMPhase)
{   // beginFMPhase is to be set. (beginPhase is zero here ==> Its not so meaningful to set both of them)
	double t;
	Reset();
	UpdateBuffer(nsamples); //allocate memory if necessary
	for (int i=0; i<nsamples; i++)
	{
		t = (double)i/(double)fs;
		buf[i] = sin(2*PI*t*midFreq-fmWidth/fmRate*cos(2*PI*(fmRate*t+beginFMPhase)));
	}
	return buf;
}

double * CSignal::fm(double midFreq, double fmWidth, double fmRate, double dur_ms, double beginFMPhase)
{   
	int nSamplesNeeded = (int)round(dur_ms/1000.*fs); 
	double *p = fm(midFreq, fmWidth, fmRate, nSamplesNeeded, beginFMPhase);
	return p;
}

double * CSignal::Tone(vector<double> freqs, unsigned int len)
{ // freqs: array of desired instantaneous frequencies requested
  // fval: coefficient inside the sine term in each instance
  // for a constant frequency, fval is always the same as specfied frequency, but in general, it is not
  // 7/24/2016 bjk
	Reset();
	UpdateBuffer(len); //allocate memory if necessary
	double t(0), tgrid(1./fs);
	buf[0] = 0;
	if (freqs.size()==2)
	{
		double duration((double)len/(double)fs),  glidelta(freqs[1] - freqs[0]);
		double f1(freqs[0]), f2(freqs[1]), ratio(f2/f1);
		for (unsigned int k(1); k<len; k++)
		{
			t += tgrid;
//			buf[k] = sin(2*PI*t*(f1/log(ratio)*pow(ratio, t/duration)));
			buf[k] = sin(2*PI*t*(freqs[0]+glidelta/2./duration*t));
		}
	}
	else
	{
		double fval(freqs[0]);
		for (unsigned int k(1); k<len; k++)
		{
			fval = (freqs[k]*tgrid + fval*t) / (t + tgrid);
			t += tgrid;
			buf[k] = sin(2*PI*fval*t);
		}
	}
	return buf;
}

double * CSignal::Tone(vector<double> freqs, double dur_ms)
{
	unsigned int nSamplesNeeded = (unsigned int)round(dur_ms/1000.*fs);
	double *p = Tone(freqs, nSamplesNeeded);
	return p;
}

double * CSignal::Tone(double freq, unsigned int nsamples, double beginPhase)
{
	Reset();
	UpdateBuffer(nsamples); //allocate memory if necessary
	for (unsigned int i=0; i<nsamples; i++)
		buf[i] = sin(2*PI*(freq*(double)i/(double)fs+beginPhase));
	return buf;
}

double * CSignal::Tone(double freq, double dur_ms, double beginPhase)
{
	unsigned int nSamplesNeeded = (unsigned int)round(dur_ms/1000.*fs);
	double *p = Tone(freq, nSamplesNeeded, beginPhase);
	return p;
}
#ifndef NO_FFTW

#include "fftw3.h"

CSignal& CSignal::Hilbert(unsigned int id0, unsigned int len)
{//This calculates the imaginary part of the analytic signal (Hilbert) transform and updates buf with it.
//To get the envelope, get the sqrt of x*x (original signal) plus hilbertx*hilbertx
	if (len == 0) len = nSamples;

	fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * len);
	fftw_complex *mid = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * len);
	fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * len);
	memset(in, 0, len*sizeof(fftw_complex));
	memset(mid, 0, len*sizeof(fftw_complex));
	memset(out, 0, len*sizeof(fftw_complex));

	// FFT
	for (unsigned int k=0; k<len; k++) in[k][0] = buf[k+id0];
	fftw_plan p1 = fftw_plan_dft_1d(len, in, mid, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_execute(p1);

	memset(in, 0, len*sizeof(fftw_complex));
	// converting halfcomplex array to complex array
	int half = len/2 + len%2;
	in[0][0] = mid[0][0];

	for (int k(1); k<half; ++k) 
	{
		in[k][0] = 2 * mid[k][0];
		in[k][1] = 2 * mid[k][1];
	}

	if (len%2 == 0)	// len is even
		in[half][0] = mid[half][0];
	// leave the rest zero

	// iFFT
	fftw_plan p2 = fftw_plan_dft_1d(len, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
	fftw_execute(p2);

	fftw_destroy_plan(p1);
	fftw_destroy_plan(p2);

	for (unsigned int k(0); k<len; ++k)
	{// scale back down since the resulting array is scaled by len.
//		buf[k+id0] = out[k][0] / len;	// This line fills buf with the identical signal with the input
		buf[k + id0] = out[k][1] / len;	// This line is about the imaginary part of the analytic signal.
	}
	fftw_free(in); 
	fftw_free(out);
	return *this;
}

CSignal& CSignal::ShiftFreq(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	double shift = *(double*)parg;
	CSignals copy(*this);
	Hilbert();
	double t(0), grid(1. / fs);
	const complex<double> j(0.0, 1.0);
	complex<double> datum;
	for (unsigned int k = 0; k<nSamples; k++)
	{
		datum = (copy.buf[k] + buf[k] * j) * exp(j * shift *  2. * PI * t);
		buf[k] = real(datum);
		t += grid;
	}
	return *this;
}

CSignal& CSignal::HilbertEnv(unsigned int id0, unsigned int len)
{
	CSignal copy(fs), out(fs);
	copy.UpdateBuffer(len);
	memcpy(copy.buf, buf + id0, len * bufBlockSize);
	copy.Hilbert(); // making it a phase-shifted version
	SetComplex();
	for (unsigned int k = id0; k < id0+len; k++) buf[2*k+1] = copy.buf[k-id0];
	out.UpdateBuffer(len);
	for (unsigned int k = id0; k<id0+len; k++) out.buf[k-id0] = abs(cbuf[k]);
	SetReal();
	memcpy(buf + id0, out.buf, len * bufBlockSize);
	return *this;
}

#endif 
/* This works but is slower.....
double * CSignal::HilbertEnv(int len)
{
	CSignals copy(*this);
	Hilbert(len);
	vector<complex<double>> data;
	data.resize(nSamples);
	int k(0);
	const complex<double> j(0.0,1.0); 
	for (vector<complex<double>>::iterator it=data.begin(); it!=data.end(); it++)
	{
		*it = copy.buf[k] + buf[k] * j;
		buf[k++] = abs(*it);
	}
	return buf;
}
*/

CTimeSeries& CTimeSeries::ReplaceBetweenTPs(CTimeSeries &newsig, double t1, double t2)
{ // signal portion between t1 and t2 is replaced by newsig
 // t1 and t2 are in ms
//	double lastendtofnewsig = newsig.GetDeepestChain()->endt();

	CTimeSeries *p(this);
	CTimeSeries copy(*this);
	double samplegrid = 1./fs;
	double deviationfromgrid = t1-((double)(int)(t1*fs))/fs;
	bool inbet(false); // true means t1 is in between chains.
	for (p=this; p && !inbet; p=p->chain)
	{
		if (t1>p->tmark+p->_dur() && p->chain && t1<p->chain->tmark)
			inbet=true;
	}
	if (t1>0. && fabs(deviationfromgrid)>1.e-8 && deviationfromgrid>-samplegrid && deviationfromgrid < samplegrid) 
		t1 -= 1000.*samplegrid;  // because its in ms
	Crop(0, t1);
	if (inbet)
		newsig >>= t1 - (tmark + _dur());
	if (newsig.chain) {delete newsig.chain; newsig.chain = NULL;}
	*this += &newsig;
	//if t2 coincides with the sampling grid, don't take that point here (it will be taken twice)
	deviationfromgrid = t2-((double)(int)(t2*fs))/fs;
	// deviationfromgrid of zero can masquerade as a very small number
	if (fabs(deviationfromgrid)>1.e-8 && deviationfromgrid>-samplegrid && deviationfromgrid < samplegrid)
		t2 += 1000.*samplegrid;  // because its in ms
	copy.Crop(t2, std::numeric_limits<double>::infinity());
	*this += &copy;
	MergeChains();
	return *this;
}

CTimeSeries& CTimeSeries::NullIn(double tpoint)
{
	double tp = quantizetmark(tpoint, fs);
	for (CTimeSeries *p = this; p; p = p->chain)
	{
		if (p->endt() < tp) continue;
		int count = (int)((tp - p->tmark) * fs / 1000+.5);
		CTimeSeries *temp = p->chain;
		CTimeSeries *newchain = new CTimeSeries(fs);
		newchain->UpdateBuffer(p->nSamples - count);
		memcpy(newchain->buf, p->buf + count, sizeof(double)*count);
		newchain->tmark = tp;
		newchain->chain = temp;
		p->nSamples = count;
		p->chain = newchain;
		return *this;
	}
	return *this;
}


CTimeSeries& CTimeSeries::Insert(double timept, CTimeSeries &newchunk)
{
	int id ;
	// Be careful, this is not a virtual function. So, when called from CSignals, this is a pointer to CSignal
	// casting won't work---i.e., you cannot declare it like this---CSignal *p((CSignal*)this)
	CTimeSeries copy(*this), out(*this);
	CTimeSeries *p(this);
	double insertduration = newchunk.GetDeepestChain()->_dur();
	if (IsNull(timept))
	{
		for (; p; p=p->chain)
		{
			if (timept > p->tmark+p->_dur() && timept < p->chain->tmark)
				break;
		}
			// The line below looks precarious but when IsNull is true
			// if timept > p->endt(), then it has at least one p->chain available
			// so let's not worry about p->chain being NULL
		//at this point, p->chain and all forward need to time-shift right.
		p=p->chain; 
		*p >>= insertduration;

		*this = copy;
		//Now, *this is prepared (the new signal buffer can be inserted)
		for (p=&newchunk;  p; p=p->chain)
			p->tmark += timept;
		*this += newchunk;
		return *this;
	}
	else
	{
		for (bool done(0); p; p=p->chain)
		{
			if (!done && timept >= p->tmark && timept <= p->tmark+p->_dur())
			{
				CTimeSeries *orgchain(chain);
				CTimeSeries *trailingchain = new CTimeSeries(fs);
				id = (int)((timept-p->tmark) * fs/1000. +.5);
				int lenTrailingchain(p->nSamples-id);
				// divide p into the point prior to id and after id.
				trailingchain->UpdateBuffer(lenTrailingchain);
				memcpy(trailingchain->buf, &buf[id], sizeof(double)*lenTrailingchain);
				trailingchain->chain = orgchain;
				p->UpdateBuffer(id);
				*p += &newchunk; //this is concatenation
				*p += trailingchain; // concatenation
				done = true;
				continue;
			}
			if (done) // this means this chain is present after timept and needs time-shift right.
				*p >>= insertduration;
		}
		return *this;
	}
	return *this;
}

double * CSignal::Truncate(double time_ms1, double time_ms2)
{ // Returns integer buffer pointer, to "extract" a signals object, use Take() member function
	int id1 = (int)round(time_ms1/1000.*fs);
	int id2 = (int)round(time_ms2/1000.*fs)-1;
	return Truncate(id1, id2);
}

double * CSignal::Truncate(int id1, int id2, int code)
{
	if (nSamples == 0) throw "internal error: Truncate()";
	id1 = max(0, id1);
	if (id1>id2) {
		nSamples = 0;
		return buf;
	}
	if (code)
	{
		// This will not work, if the signal has null portion in the middle and id1 and id2 cover beyond the null portion... 7/15/2016 bjk
		if (tmark>0)
		{
			CSignal copy(*this);
			double _tmark(copy.tmark);
			copy.tmark = 0;
			id1 -= (int)(_tmark/1000.*fs+.5);
			id2 -= (int)(_tmark/1000.*fs+.5);
			copy.Truncate(id1, id2);
			copy.tmark = _tmark;
			*this = copy;
			return buf;
		}
	}
	if (id2>(int)nSamples-1) id2 = (int)nSamples-1;
	nSamples = max(0,id2-id1+1);
	memmove(buf, logbuf+id1*bufBlockSize, nSamples*bufBlockSize);
	return buf;
}

double * CSignal::Silence(double dur_ms)
{
	unsigned int nSamplesNeeded = (unsigned int)round(dur_ms/1000.*fs);
	double *p = Silence(nSamplesNeeded);
	return p;
}

double * CSignal::Silence(unsigned int nsamples)
{
	Reset();
	UpdateBuffer(nsamples); //allocate memory if necessary
	if (nsamples < 0)
		return NULL;
	memset((void*)buf, 0, nsamples*sizeof(double));
	return buf;
}

double * CSignal::DC(double dur_ms)
{
	unsigned int nSamplesNeeded = (unsigned int)round(dur_ms/1000.*fs);
	double *p = DC(nSamplesNeeded);
	return p;
}

double * CSignal::DC(unsigned int nsamples)
{
	Reset();
	UpdateBuffer(nsamples);
	for (unsigned int i=0; i<nsamples; i++) buf[i] = 1.;
	return buf;
}

bool CTimeSeries::IsNull(double timept)
{
	if (GetType()!=CSIG_AUDIO) return false;
	CTimeSeries *p(this);
	for (; p; p=p->chain)
		if (timept >= p->tmark)
		{
			if (timept > p->tmark+p->_dur()) return false;
			else
				continue;
		}
		else 
			return true;
	return false;
}

int CSignal::GetType() const
{
	if (nSamples==0) // empty
	{
		if (tmark==0.)	
			return CSIG_EMPTY;
		else			
			return CSIG_NULL;	
	}
	else if (fs==2) // string
		return CSIG_STRING;
	else if (nSamples == 1)
		return CSIG_SCALAR;
	else if (fs>2) // audio
		return CSIG_AUDIO;
	else
		return CSIG_VECTOR;
}

bool CSignal::IsSingle() const 
{
	int type = GetType();
	if (nSamples != 1) return false;
	return (type == CSIG_SCALAR || type == CSIG_STRING || type == CSIG_AUDIO || type == CSIG_TSERIES) ;
}

template <class T>
int dcomp(const void * arg1, const void * arg2)
{
	if (*(T*)arg1>*(T*)arg2)	return 1;
	else if (*(T*)arg1 == *(T*)arg2) return 0;
	else	return -1;
}

template <class T>
int dcompR(const void * arg1, const void * arg2)
{
	if (*(T*)arg1<*(T*)arg2)	return 1;
	else if (*(T*)arg1 == *(T*)arg2) return 0;
	else	return -1;
}

CSignal& CSignal::sort(unsigned int id0, unsigned int len)
{
	if (bufBlockSize == 8)
	{
		if (*(double*)parg > 0)
			qsort(logbuf + id0*bufBlockSize, len, bufBlockSize, dcomp<double>);
		else
			qsort(logbuf + id0*bufBlockSize, len, bufBlockSize, dcompR<double > );
	}
	else if (bufBlockSize == 1)
	{
		if (*(double*)parg > 0)
			qsort(logbuf + id0*bufBlockSize, len, bufBlockSize, dcomp<unsigned char>);
		else
			qsort(logbuf + id0*bufBlockSize, len, bufBlockSize, dcompR<unsigned char>);
	}
	else
		throw "internal error--this shouldn't happen. CSignal::sort";
	return *this;
}

CSignal& CSignal::SAM(double modRate, double modDepth, double initPhase)
{
	double *env = new double[nSamples];
	for (unsigned int k = 0; k<nSamples; k++)
		env[k] = (1. + modDepth*sin(2 * PI*(k * modRate / fs + initPhase - .25))) / (1. + modDepth);
	Modulate(env, nSamples);
	delete[] env;
	return *this;
}

CTimeSeries& CTimeSeries::SAM(double rate, double depth, double phase)
{
	for (CTimeSeries *p = this; p; p = p->chain)
		p->CSignal::SAM(rate, depth, phase);
	return *this;
}

CSignals& CSignals::SAM(double rate, double depth, double phase)
{
	CTimeSeries::SAM(rate, depth, phase);
	if (next)	next->CTimeSeries::SAM(rate, depth, phase);
	return *this;
}

int CTimeSeries::GetType() const
{
	// as of 6/12/2018 tseq only supports with scalars
	// still as of 6/20/2018 
	if (tmark>0 && nSamples == 1) return CSIG_TSERIES;
	if (chain && nSamples == 1)	return CSIG_TSERIES;
	return CSignal::GetType();
}

#ifndef NO_RESAMPLE

CTimeSeries& CTimeSeries::Resample(vector<unsigned int> newfs, vector<unsigned int> lengths, char *errstr)
{
	for (CTimeSeries *p=this; p; p=p->chain)
		p->Resample(newfs, lengths, errstr);
	return *this;
}

double * CSignal::Resample(vector<unsigned int> newfs, vector<unsigned int> lengths, char *errstr)
{ // This is not really for resampling but only used for the % operator with a vector (variable_ratio)
	//Therefore, fs is not updated.
	errstr[0]=0;
	SRC_DATA sd;
	vector<float> data_in, data_out;
	int errcode;
	//First estimate the buffer length at the end of the processing, so we don't call UpdateBuffer multiple times.
	double mean(0.);
	int isum(0), id(0);
	for (vector<unsigned int>::iterator it=lengths.begin(); it!=lengths.end(); ++it)
		isum += *it;
	for (vector<unsigned int>::iterator it=newfs.begin(); it!=newfs.end(); ++it, id++)
	{
		double temp = *it*lengths[id]/isum;
		mean += *it*lengths[id]/isum;
	}
	if (mean>fs) UpdateBuffer((int)(nSamples * (mean/fs) + .5));

	SRC_STATE* handle;
	unsigned int cumid2(0);
	for (unsigned int k(0), cumid1(0); k<newfs.size(); k++)
	{
		handle = src_new (SRC_SINC_MEDIUM_QUALITY, 1, &errcode);
		data_in.resize(lengths[k]);
		sd.data_in = &data_in[0]; // data_in[0] doesn't change in the loop, but just keep it here
		for (unsigned int i(0); i<lengths[k]; i++) sd.data_in[i] = (float)buf[cumid1+i];
		cumid1 += lengths[k];
		sd.src_ratio = (double)newfs[k]/fs;
		sd.input_frames = lengths[k];
		sd.output_frames =  (long) (lengths[k] * (sd.src_ratio) + .5) ; // sd.src_ratio+1 should be right, but just in case 
		data_out.resize(sd.output_frames);
		sd.data_out = &data_out[0];
		int res = src_process (handle, &sd);
		if (!res)
		{
			for (int i(0); i<sd.output_frames_gen; i++) 			buf[cumid2+i] = sd.data_out[i];
			cumid2 += sd.output_frames_gen;
			sd.end_of_input = (int)(k==newfs.size()-1);
		}
		else
		{
			strcpy(errstr, src_strerror(res));
			return NULL;
		}
	}
	nSamples = (int)cumid2;
	return buf;
}

CTimeSeries& CTimeSeries::Resample(int newfs, char *errstr) // Revised in Dec09---noted for JHPARK
{
	for (CTimeSeries *p = this; p; p = p->chain)
		p->CSignal::Resample(newfs, errstr);
	return *this;
}

CTimeSeries * CTimeSeries::AtTimePoint(double timept)
{ // This retrieves CSignal at the specified time point. If no CSignal exists, return NULL.
	for (CTimeSeries *p = this; p; p = p->chain)
	{
		if (p->CSignal::endt() < timept) continue;
		if (timept < p->tmark) return NULL;
		return p;
	}
	return NULL;
}

double * CSignal::Resample(int newfs, char *errstr) // Revised in Dec09---noted for JHPARK
{
	errstr[0]=0;
	SRC_DATA conv;
	vector<float> data_in, data_out;
	data_in.resize(nSamples);
	conv.data_in = &data_in[0];
	conv.src_ratio = (double)newfs/(double)fs;
	conv.input_frames = nSamples;
	conv.output_frames =  (long) (nSamples * (conv.src_ratio) + .5) ; // conv.src_ratio+1 should be right, but just in case 
	data_out.resize(conv.output_frames);
	conv.data_out = &data_out[0];
	for (unsigned int i=0; i<nSamples; i++) conv.data_in[i] = (float)buf[i];

	int res = src_simple(&conv, SRC_SINC_BEST_QUALITY, 1);
	if (!res)
	{
		if (conv.output_frames_gen!=newfs) strcpy(errstr, "resampled output generated less points than desired, zeros padded at the end.");

		//update nSamples and fs
		UpdateBuffer(conv.output_frames);
		fs = newfs;
		int i;
		for (i=0; i<conv.output_frames_gen; i++) 			buf[i] = conv.data_out[i];
		for (i=conv.output_frames_gen; i<conv.output_frames; i++)		buf[i] = 0;
	}
	else
	{
		strcpy(errstr, src_strerror(res));
		return NULL;
	}
	return buf;
}

#endif //NO_RESAMPLE

void CTimeSeries::UpSample(int cc)
{
	for (CTimeSeries *p = this; p; p = p->chain)
		p->CSignal::UpSample(cc);
}

void CSignal::UpSample(int cc)
{
	if (cc==1) return;
	int newLength = nSamples * cc;
	CSignal temp(fs*cc);
	temp.UpdateBuffer(newLength);
	memset((void*)temp.buf, 0, sizeof(double)*newLength);
	for (unsigned int i=0; i<nSamples; i++)
		temp.buf[i*cc] = buf[i];
	*this = temp;
}

void CTimeSeries::DownSample(int cc)
{
	for (CTimeSeries *p = this; p; p = p->chain)
		p->CSignal::DownSample(cc);
}

void CSignal::DownSample(int cc)
{
	if (cc==1) return;
	unsigned int newLength = (unsigned int)ceil((double)nSamples / cc);
	CSignal temp(fs/cc);
	temp.UpdateBuffer(newLength);
	unsigned int k;
	for (k=0; cc*k<nSamples; k++)
		temp.buf[k] = buf[cc*k];
	temp.nSamples = k;
	*this = temp;
}

//Check this.....12/25/2017
int CSignal::DecFir(const CSignal& coeff, int offset, int nChan)
{// assumption: nSamples is a multiple of nChan
	unsigned int i, j, nItems = nSamples / nChan; // length of filtering output
	double xx, *out = new double[nItems+1];
	for (i = offset; i<nSamples; i += nChan)
	{
		xx = coeff.buf[0] * buf[i];
		for (j = nChan; j<coeff.nSamples && i >= j; j += nChan)
			xx += coeff.buf[j] * buf[i - j];
		out[(i-offset)/nChan] = xx;
	}
	for (i = offset; i < nSamples; i += nChan)
		buf[i] = out[(i - offset) / nChan];
	delete[] out;
	return 1;
}

CSignal& CSignal::_dynafilter(CSignal *TSEQ_num, CSignal * TSEQ_den, unsigned int id0, unsigned int len)
{
	

	if (len == 0) len = nSamples;
	if (IsComplex())
	{
		//Do this later if someone requests. 10/9/2018
	}
	else
	{
		unsigned int tind, tsInd0;
		double xx, *out = new double[len];
		CTimeSeries *p = (CTimeSeries *)TSEQ_num, *q = (CTimeSeries *)TSEQ_den;
		tsInd0 = (unsigned int)(p->tmark / 1000. * fs);
		while (tsInd0 < id0)
		{
			p = p->chain, q = q->chain;
			if (!p || !q) break;
			tsInd0 = (unsigned int)(p->tmark / 1000. * fs);
		}
		for (tind = id0; tind < tsInd0; tind++)
		{
			xx = p->buf[0] * buf[tind];
			for (unsigned int n = 1; n < p->nSamples && tind >= n; n++)
				xx += p->buf[n] * buf[tind - n];
			for (unsigned int n = 1; n < q->nSamples && tind >= n; n++)
				xx -= q->buf[n] * out[tind - n];
			out[tind] = xx;
		}
		CTimeSeries *pNext = p->chain;
		CTimeSeries *qNext = q->chain;
		unsigned int tsInd;
		vector<double> num0, den0;
		vector<double> num(p->nSamples,0), den(p->nSamples, 0);
		vector<double> num1, den1;
		while (pNext || qNext)
		{
			if (p)	num0 = p->ToVector();
			else { num0.clear();  num0.push_back(1.); }
			if (q)	den0 = q->ToVector();
			else {	den0.clear();  den0.push_back(1.); }
			if (pNext) num1 = pNext->ToVector();
			else { num1.clear();  num1.push_back(1.); }
			if (qNext) den1 = qNext->ToVector();
			else { den1.clear();  den1.push_back(1.); }
			tsInd = (unsigned int)(pNext->tmark / 1000. * fs);
			for (; tind < id0 + len && tind < tsInd; tind++)
			{
				if (pNext)
					for (size_t k = 0; k < num1.size(); k++)
					{
						double slope1 = (num1[k] - num0[k]) / (tsInd - tsInd0);
						num[k] = num0[k] + slope1 * (tind - tsInd0);
					}
				if (qNext)
					for (size_t k = 0; k < den1.size(); k++)
					{
						double slope2 = (den1[k] - den0[k]) / (tsInd - tsInd0);
						den[k] = den0[k] + slope2 * (tind - tsInd0);
					}
				xx = num[0] * buf[tind];
				for (unsigned int n = 1; n < num.size() && tind >= n; n++)
					xx += num[n] * buf[tind - n];
				for (unsigned int n = 1; n < den.size() && tind >= n; n++)
					xx -= den[n] * out[tind - n];
				out[tind] = xx;
			}
			if (pNext)
			{
				p = p->chain;
				pNext = p ? p->chain : NULL;
			}
			if (qNext)
			{
				q = q->chain;
				qNext = q ? q->chain : NULL;
			}
		}
		//at this point pNext is NULL
		CTimeSeries temp(1.);
		CTimeSeries *pLast = p ? p : &temp;
		CTimeSeries *qLast = q ? q : &temp;
		for (; tind < id0 + len; tind++)
		{
			xx = p->buf[0] * buf[tind];
			for (unsigned int n = 1; n < pLast->nSamples && tind >= n; n++)
				xx += pLast->buf[n] * buf[tind - n];
			for (unsigned int n = 1; n < qLast->nSamples && tind >= n; n++)
				xx -= qLast->buf[n] * out[tind - n];
			out[tind] = xx;
		}
		delete[] buf;
		buf = out;
	}
	return *this;
}

CSignal& CSignal::_filter(vector<double> num, vector<double> den, unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	if (IsComplex())
	{
		complex<double> xx, *out = new complex<double>[len];
		for (unsigned int m = id0; m < id0 + len; m++)
		{
			xx = num[0] * cbuf[m];
			for (unsigned int n = 1; n < num.size() && m >= n; n++)
				xx += num[n] * cbuf[m - n];
			for (unsigned int n = 1; n < den.size() && m >= n; n++)
				xx -= den[n] * out[m - n];
			out[m] = xx;
		}
		delete[] cbuf;
		cbuf = out;
	}
	else
	{
		double xx, *out = new double[len];
		for (unsigned int m = id0; m < id0 + len; m++)
		{
			xx = num[0] * buf[m];
			for (unsigned int n = 1; n < num.size() && m >= n; n++)
				xx += num[n] * buf[m - n];
			for (unsigned int n = 1; n < den.size() && m >= n; n++)
				xx -= den[n] * out[m - n];
			out[m] = xx;
		}
		delete[] buf;
		buf = out;
	}
	return *this;
}

CSignal& CSignal::dynaAR(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	vector<vector<double>> pvectors = *(vector<vector<double>>*)parg;
	vector<double> TSden;
	if (pvectors.front().size() == 1)
		TSden.assign(len, pvectors.front().front());
	else
	{
		vector<int> nCounts = SpreadEvenly(len, pvectors.front().size() - 1);
		auto ItCount = nCounts.begin();
		for (auto it = pvectors.front().begin(); it != pvectors.front().end() - 1; it++)
		{
			//make linear interpolated sequence from *it to *(it+1) with a length of ItCount
			for (int k = 0; k < *ItCount; k++)
			{
				double slope = (*(it + 1) - *it) / *ItCount;
				TSden.push_back(*it + slope * k);
			}
			ItCount++;
		}
	}
	//For now, TSden is evenly interpolated across len 10/6/2018
	int order = (int)pvectors.back().front();
	if (order==0)
		throw "AR filter cannot have the coefficient for 0 delay.";
	order *= -1;
	double *out = new double[len];
	memset(out, 0, sizeof(double)*len);
	auto it = TSden.begin();
	for (unsigned int m = id0; m < id0 + order; m++)
		out[m] = buf[m];
	for (unsigned int m = id0 + order; m < id0 + len; m++)
	{
		out[m] = *it * buf[id0 + m - order];
		it++;
	}
	delete[] buf;
	buf = out;
	return *this;
}

CSignal& CSignal::dynaMA(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	vector<vector<double>> pvectors = *(vector<vector<double>>*)parg;
	vector<double> TSnum;
	if (pvectors.front().size() == 1)
		TSnum.assign(len, pvectors.front().front());
	else
	{
		vector<int> nCounts = SpreadEvenly(len, pvectors.front().size() - 1);
		auto ItCount = nCounts.begin();
		for (auto it = pvectors.front().begin(); it != pvectors.front().end() - 1; it++)
		{
			//make linear interpolated sequence from *it to *(it+1) with a length of ItCount
			for (int k = 0; k < *ItCount; k++)
			{
				double slope = (*(it + 1) - *it) / *ItCount;
				TSnum.push_back(*it + slope * k);
			}
			ItCount++;
		}
	}
	//For now, TSnum is evenly interpolated across len 10/6/2018
	int order = (int)pvectors.back().front();
	double *out = new double[len];
	memset(out, 0, sizeof(double)*len);
	auto it = TSnum.begin();
	for (unsigned int m = id0 + order; m < id0 + len; m++)
	{
		out[m] = *it * buf[id0 + m - order];
		it++;
	}
	delete[] buf;
	buf = out;
	return *this;
}

CSignal& CSignal::filter(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	vector<double> num, den;
	vector<vector<double>> coeffs = *(vector<vector<double>>*)parg;
	num = coeffs.front();
	den = coeffs.back();
	_filter(num, den, id0, len);
	return *this;
}

CSignal& CSignal::dynafilter(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	vector<CSignals*> TScoeff = *(vector<CSignals*>*)parg;
	CSignals * TSnum = TScoeff.front();
	CSignals * TSden = TScoeff.back();
	//if TSEQ is a relative time seq, make it to an absolute time seq
	if (TSnum->fs == 0)
	{
		for (CTimeSeries *p = TSnum; p; p = p->chain)
			p->tmark *= (double)(id0 + len) / fs * 1000;
		TSnum->fs = fs;
	}
	if (TSden->fs == 0)
	{
		for (CTimeSeries *p = TSden; p; p = p->chain)
			p->tmark *= (double)(id0 + len) / fs * 1000;
		TSden->fs = fs;
	}
	_dynafilter(TSnum, TSden, id0, len);
	return *this;
}

void CSignal::Filter(unsigned int nTabs, double *num, double *den)
{//used in IIR
	unsigned int i, j;
	if (IsComplex())
	{
		complex<double> xx, *out = new complex<double>[nSamples];
		for (i = 0; i<nSamples; i++)
		{
			xx = num[0] * cbuf[i];
			for (j = 1; j<nTabs && i >= j; j++)
				xx += num[j] * cbuf[i - j];
			for (j = 1; j<nTabs && i >= j; j++)
				xx -= den[j] * out[i - j];
			out[i] = xx;
		}
		if (cbuf) delete[] cbuf;
		cbuf = out;
	}
	else
	{
		if (*den != 1.)
		{
			double tp = *den;
			for (unsigned int k = 0; k < nTabs; k++)
			{
				den[k] /= tp;
				num[k] /= tp;
			}
		}
		double xx, *out = new double[nSamples];
		for (i = 0; i<nSamples; i++)
		{
			xx = num[0] * buf[i];
			for (j = 1; j<nTabs && i >= j; j++)
				xx += num[j] * buf[i - j];
			for (j = 1; j<nTabs && i >= j; j++)
				xx -= den[j] * out[i - j];
			out[i] = xx;
		}
		if (buf) delete[] buf;
		buf = out;
	}
}

CSignal& CSignal::filtfilt(unsigned int id0, unsigned int len)
{
	//Transient edges not handled, only zero-padded edges 
	if (len == 0) len = nSamples;
	vector<double> num, den;
	vector<vector<double>> coeffs = *(vector<vector<double>>*)parg;
	num = coeffs.front();
	den = coeffs.back();

	CSignal temp(fs), temp2(fs), out(fs);
	size_t nfact = 3 * (max(num.size(),den.size()) - 1);
	temp.Silence((unsigned int)nfact);
	temp += this;
	temp2.Silence((unsigned int)nfact);
	temp += &temp2;
	temp.parg = parg;
	temp.filter(id0, temp.nSamples);
	temp.ReverseTime();
	temp.filter(id0, temp.nSamples);
	temp.ReverseTime();
	temp.Take(out, (unsigned int)nfact, (unsigned int)(nfact + nSamples - 1));
	*this = out;
	return *this;
}

void CTimeSeries::ReverseTime()
{
	for (CTimeSeries *p = this; p; p = p->chain)
		p->CSignal::ReverseTime();
}

void CSignal::ReverseTime()
{
	CSignal temp(*this);
	double *tempBuf = temp.GetBuffer();
	for (unsigned int i=0; i<nSamples ; i++)
		tempBuf[nSamples-i-1]=buf[i];
	*this = temp;
}

#ifndef NO_IIR

//void CSignal::IIR(int kind, int type, int order, double *freqs, double passRipple_dB, double stopFreqORAttenDB)
CSignal& CSignal::IIR(unsigned int id0, unsigned int len)
{// kind: 1 butterworth, 2 Chebyshev, 3 Elliptic
	// type: 1 lowpass, 2 bandpass, 3 highpass, 4 bandstop

	if (len == 0) len = nSamples;

	vector<double*> params = *(vector<double*>*)parg;
	vector<double*>::iterator it = params.begin();
	int kind = (int)**it; it++;
	int type = (int)**it; it++;
	int order = (int)**it; it++;
	double *freqs = *it; it++;
	double passRipple_dB = **it; it++;
	double stopFreqORAttenDB = **it; 

	double *den = new double [2*order+1];
	double *num = new double [2*order+1];

	// To obtaine the filter coefficients, the object must be a scalar, the sample rate. 
	// Then on success of this call , this object has the buffer of a and b (den and num) in that order.
	if (IsScalar()) fs = (int)value();
	int res = design_iir(num, den, GetFs(), kind, type, order, freqs, passRipple_dB, stopFreqORAttenDB);
	char errstr[256];
	if (res <= 0) {
		switch (res) {
		case -1:
			strcpy(errstr, "((kind <= 0) || (kind > 3))");
			break;
		case -2:
			strcpy(errstr, "((type <= 0) || (type > 4))");
			break;
		case -3:
			strcpy(errstr, "(n <= 0)");
			break;
		case -4:
			strcpy(errstr, "(fs <= 0)");
			break;
		case -5:
			strcpy(errstr, "Filter frequency is greater than Nyquist rate.");
			break;
		default:
			sprintf(errstr, "Unknown error, code=%d", res);
		}
		throw errstr;
	} else {
		res = 1;
		if (IsScalar()) {
			if (type & 1)
				UpdateBuffer(2*order+2);
			else
				UpdateBuffer(4*order+2);
			memcpy(buf, den, sizeof(*buf)*(nSamples/2));
			memcpy(buf+(nSamples/2), num, sizeof(*buf)*(nSamples/2));
		} else {
			if (type & 1)
				Filter(order+1, num, den);
			else
				Filter(2*order+1, num, den);
		}
	}
	delete[] den; delete[] num;
	return *this;
}
#endif // NO_IIR

std::string CSignal::string()
{
	unsigned int k;
	std::string out;
	out.resize(nSamples);
	for (k=0; k<nSamples && strbuf[k]; k++)
		out[k] = *(strbuf+k);
	out.resize(k);
	return out;
}

char *CSignal::getString(char *str, const int size)
{
	int len = min((int)nSamples, size - 1);
	memcpy(str, strbuf, len);
	str[len] = '\0';
	return str;
}

CSignal &CSignal::SetString(const char *str)
{
	Reset(2);
	bufBlockSize = 1;
	UpdateBuffer((int)strlen(str)+1);
	strcpy(strbuf, str);
	return *this;
}

CSignal &CSignal::SetString(const char c)
{
	Reset(2);
	bufBlockSize = 1;
	if (c==0) return *this;
	UpdateBuffer(2);
	memset(strbuf, 0, 2);
	strbuf[0] = c;
	nSamples = 1;
	return *this;
}

CTimeSeries& CTimeSeries::each_sym(double(*fn)(double))
{
	if (GetType() == CSIG_VECTOR || GetType() == CSIG_SCALAR)
		body::each(fn);
	else if (GetType() == CSIG_AUDIO)
	{
		for (CTimeSeries *p = this; p; p = p->chain)
			for (unsigned int k(0); k<p->nSamples; k++)
			{
				if (p->buf[k]<0)
					p->buf[k] = -fn(-p->buf[k]);
				else
					p->buf[k] = fn(p->buf[k]);
			}
	}
	else
		throw "each()--only for vector/scalar or audio";
	return *this;
}

CTimeSeries& CTimeSeries::each(double (*fn)(double))
{
	if (GetType()==CSIG_VECTOR || GetType()==CSIG_SCALAR)
		body::each(fn); 
	else if (GetType()==CSIG_AUDIO || GetType() == CSIG_TSERIES)
	{
		for (CTimeSeries *p=this; p; p=p->chain)
			for (unsigned int k(0); k<p->nSamples; k++)
				p->buf[k] = fn(p->buf[k]);
	}
	else
		throw "each()--only for vector/scalar or audio"; 
	return *this;
}

CTimeSeries & CTimeSeries::MFFN(double(*fn)(double), complex<double>(*cfn)(complex<double>))
{
	if (nSamples==0) return *this;
	if (GetType() == CSIG_VECTOR || GetType() == CSIG_SCALAR)
		body::mathFunForbidNegative(fn, cfn);
	else if (GetType() == CSIG_AUDIO)
	{
		for (CTimeSeries *p = this; p; p = p->chain)
			for (unsigned int k(0); k<p->nSamples; k++)
			{
				if (p->buf[k]<0)
					p->buf[k] = -fn(-p->buf[k]);
				else
					p->buf[k] = fn(p->buf[k]);
			}
	}
	else
		throw "MFFN()--only for vector/scalar or audio";
	return *this;
}

CTimeSeries & CTimeSeries::MFFN(double(*fn)(double, double), complex<double>(*cfn)(complex<double>, complex<double>), const CTimeSeries &param)
{
	if (nSamples == 0) return *this;
	if ( (GetType() == CSIG_VECTOR || GetType() == CSIG_SCALAR) && (param.GetType() == CSIG_VECTOR || param.GetType() == CSIG_SCALAR))
		body::mathFunForbidNegative(fn, cfn, param);
	else if (GetType() == CSIG_AUDIO)
	{
		if (param.GetType() == CSIG_AUDIO) throw "The operation cannot perform with both audio operands.";
		if (param.GetType() == CSIG_SCALAR || (param.nSamples==2 && param.buf[0] == param.buf[1]))
		{
			for (CTimeSeries *p = this; p; p = p->chain)
				for (unsigned int k(0); k<p->nSamples; k++)
				{
					if (p->buf[k]<0)
						p->buf[k] = -fn(-p->buf[k], param.buf[0]);
					else
						p->buf[k] = fn(p->buf[k], param.buf[0]);
				}
		}
		else if (param.GetType() == CSIG_VECTOR)
		{
			if (param.nSamples>2) throw "The interpolated operand feature has not been implemented in this version. Check back.";
			for (CTimeSeries *p = this; p; p = p->chain)
				for (unsigned int k(0); k<p->nSamples; k++)
				{
					double diff = param.buf[1] - param.buf[0];
					double inc = diff / p->nSamples;
					double val = param.buf[0] + inc *k;
					if (p->buf[k]<0)
						p->buf[k] = -fn(-p->buf[k], val);
					else
						p->buf[k] = fn(p->buf[k], val);
				}
		}
	}
	else
		throw "MFFN()--only for vector/scalar or audio";
	return *this;
}


CTimeSeries& CTimeSeries::each(complex<double> (*fn)(complex<double>))
{
	if (IsComplex())
		body::each(fn); 
	else
		throw "each()--expecting complex number"; 
	return *this;
}

CTimeSeries& CTimeSeries::each(double (*fn)(complex<double>))
{
	if (IsComplex())
		body::each(fn); 
	else
		throw "each()--expecting complex number"; 
	return *this;
}

CTimeSeries& CTimeSeries::each(double (*fn)(double, double), body &arg2)
{
	if (GetType()==CSIG_VECTOR || GetType()==CSIG_SCALAR)
		body::each(fn,arg2); 
	else if (GetType()==CSIG_AUDIO)
	{
		if (arg2.nSamples==1) 
		{
			double val = arg2.value();
			for (CTimeSeries *p=this; p; p=p->chain)
				for (unsigned int k(0); k<p->nSamples; k++)
					p->buf[k] = fn(val, p->buf[k]);
		}
		else
		{
			for (CTimeSeries *p=this; p; p=p->chain)
				for (unsigned int i=0; i<min(nSamples, arg2.nSamples); ++i)
					p->buf[i] = fn(p->buf[i], arg2.buf[i]); 
		}
	}
	else
		throw "each()--only for vector/scalar or audio"; 
	return *this;
}

CTimeSeries& CTimeSeries::each(complex<double> (*fn)(complex<double>, complex<double>), body &arg2)
{
	if (IsComplex())
		body::each(fn, arg2);  
	else
		throw "each()--expecting complex number"; 
	return *this;
}


CTimeSeries& CTimeSeries::transpose1()
{
	if (nSamples == 1)
		return *this;
	CTimeSeries t(buf[0]);
	SwapContents1node(t);
	return *this;
}

int CTimeSeries::WriteAXL(FILE* fp)
{
	CTimeSeries *p(this);
	size_t res;
	unsigned int nChains = CountChains();
	res = fwrite((void*)&fs, sizeof(fs),1,fp);
	res = fwrite((void*)&nChains, sizeof(nChains),1,fp);
	for (unsigned int k=0; k<nChains; k++, p=p->chain)
	{
		res = fwrite((void*)&p->nSamples, sizeof(nSamples),1,fp);
		res = fwrite((void*)&p->tmark, sizeof(tmark),1,fp);
		res = fwrite((void*)p->buf, p->bufBlockSize,p->nSamples,fp);
	}
	return (int)res;
}

int CTimeSeries::IsTimeSignal()
{ // Assume that CSignal and next have the same type, except for null or empty
	if (fs==1 || fs==2) return 0;
	return 1;
//	if (GetType() == CSIG_AUDIO || GetType() == CSIG_TSERIES) return 1;
}

CSignals::CSignals()
: next(NULL)
{
}

CSignals::CSignals(bool b)
: next(NULL)
{
	Reset(1);
	bufBlockSize = 1; // logical array
	UpdateBuffer(1); 
	logbuf[0] = b;
}

CSignals::CSignals(int sampleRate)
:next(NULL)
{
	SetFs(max(sampleRate,0));
}

CSignals::CSignals(double value)
:next(NULL)
{
	SetFs(1);
	SetValue(value);
}

CSignals::CSignals(const CSignals& src)
:next(NULL)
{
	*this = src;
}

CSignals::CSignals(const CTimeSeries& src)
:next(NULL)
{
	*this = src;
}

CSignals::CSignals(double *y, int len)
:next(NULL)
{
	SetFs(1);
	UpdateBuffer(len);
	memcpy(buf, y, sizeof(double)*len);
}



CSignals::~CSignals()
{
	if (next) {
		delete next;
		next = NULL;
	}
}

CSignals::CSignals(std::string str)
:next(NULL)
{
	SetString(str.c_str());
}

void CSignals::SetNextChan(CTimeSeries *second)
{
	if (second && fs != second->GetFs() && second->nSamples > 0 && nSamples > 0)
	{
		char errstr[256];
		sprintf(errstr, "SetNextChan attempted on different fs: fs1=%d, fs2=%d", GetFs(), second->GetFs());
		throw errstr;
	}
	if (next) {
		delete next;
		next = NULL;
	}
	if (second) {
		next = new CTimeSeries;
		*next = *second;
	}
}

void CSignals::SetValue(double v) 
{
	Reset(1);
	body::SetValue(v);
}

void CSignals::SetValue(complex<double> v) 
{
	Reset(1);
	body::SetValue(v);
}

CSignals& CSignals::Reset(int fs2set)	// Empty all data fields - sets nSamples to 0.
{
	CTimeSeries::Reset(fs2set);
	if (next) {
		delete next;
		next = NULL;
	}
	return *this;
}

CSignals CSignals::basic(double (CSignal::*pf_basic)(unsigned int, unsigned int), void *popt)
{
	CSignals newout = CTimeSeries::basic(pf_basic, popt);
	for (vector<CTimeSeries>::iterator it = newout.outarg.begin(); it!=newout.outarg.end(); it++)
		newout.outarg2.push_back(*it);
	if (next != NULL)
	{
		next->pf_basic = pf_basic;
		CSignals nextout = next->basic(pf_basic, popt);
		for (vector<CTimeSeries>::iterator it = nextout.outarg.begin(); it != nextout.outarg.end(); it++)
			nextout.outarg2.push_back(*it);
		newout.SetNextChan(&nextout);
	}
	return newout;
}

CSignals& CSignals::basic(CSignal& (CSignal::*pf_basic2)(unsigned int, unsigned int), void *popt)
{
	CTimeSeries::basic(pf_basic2, popt);
	if (next != NULL)
	{
		next->pf_basic2 = pf_basic2;
		next->basic(pf_basic2, popt);
	}
	return *this;
}

CSignals CSignals::basic(CSignal(CSignal::*pf_basic3)(unsigned int, unsigned int), void *popt)
{
	CSignals newout = CTimeSeries::basic(pf_basic3, popt);
	if (next != NULL)
	{
		next->pf_basic3 = pf_basic3;
		newout.SetNextChan(&next->basic(pf_basic3, popt));
	}
	return newout;
}



CSignals& CSignals::NullIn(double tpoint)
{
	CTimeSeries::NullIn(tpoint);
	if (next != NULL)
	{
		next->CTimeSeries::NullIn(tpoint);
	}
	return *this;
}

CSignals CSignals::RMS()
{
	CSignals newout = CSignal::RMS();
	if (next != NULL)
	{
		newout.UpdateBuffer(2);
		newout.buf[1] = next->CSignal::RMS();
	}
	return newout;
}

CSignals& CSignals::operator=(const CTimeSeries& rhs)
{
	if (this != &rhs)
		CTimeSeries::operator=(rhs);
	return *this;
}

CSignals& CSignals::operator=(const CSignals& rhs)
{   // Make a deep copy only for buf, but not for sc, because sc is not necessarily dynamically allocated.
	// Thus, BE Extra careful when making writing an assignment statement about the scaling..
	if (this != &rhs) 
	{
		CTimeSeries::operator=(rhs);
		SetNextChan(rhs.next);
		outarg = rhs.outarg;
		outarg2 = rhs.outarg2;
	}
	return *this; // return self-reference so cascaded assignment works
}

CSignals& CSignals::operator-(void)	// Unary minus
{
	CTimeSeries::operator-();
	if (next)	-*next;
	return *this;
}

CSignals& CSignals::operator+=(const double con) 
{
	CSignals _sec(con); 
	return (*this+=(_sec)); 
}

CSignals& CSignals::operator+=(CTimeSeries &sec)
{
	CSignals _sec(sec); 
	return (*this+=(_sec)); 
}

CSignals& CSignals::operator+=(CSignals &sec)
{
	SwapCSignalsPutScalarSecond(*this, sec);
	CTimeSeries copy;
	bool stereosec(false);
	if (sec.next) stereosec = true, copy = *sec.next;
	else copy = sec;
	CTimeSeries::operator+=(sec);
	if (next)
		if (stereosec)	*next += *sec.next;
		else			*next += copy;
	return *this;
}

const CSignals& CSignals::operator+=(CSignals *yy)
{
	CTimeSeries::operator+=(yy);
	if (next) {
		if (yy->next)
			*next += yy->next;	// stereo += stereo
		else
			*next += yy;		// stereo += mono
	}
	return *this;
}

CSignals& CSignals::operator*=(const double con) 
{
	CSignals _sec(con); 
	return (*this*=(_sec)); 
}

CSignals& CSignals::operator*=(CSignals &sec)
{
	SwapCSignalsPutScalarSecond(*this, sec);
	CTimeSeries::operator*=(sec);
	if (next)
		if (sec.next)	*next *= *sec.next;
		else			*next *= sec;
	return *this;
}

bool CSignals::operator==(const CSignals &rhs)
{
	if (!CTimeSeries::operator==(rhs)) return false;
	if (next)
		if (rhs.next) return (*next == *rhs.next);
		else return false;
	return true;
}

bool CSignals::operator==(double rhs)
{
	return *this == CSignals(rhs);
}

bool CSignals::operator==(std::string rhstr)
{
	return *this == CSignals(rhstr);
}

int CSignals::IsTimeSignal()
{ // Assume that CTimeSeries and next have the same type, except for null or empty
	if (CTimeSeries::IsTimeSignal()) return 1;
	if (!next) return 0;
	if (next->IsTimeSignal()) return 1;
	return 0;
}

CVar& CVar::initcell(CVar &sec)
{
	Reset();
	cell.push_back(sec);
	return *this;
}

CVar& CVar::appendcell(CVar &sec)
{
	if (GetType() != CSIG_EMPTY && GetType() != CSIG_CELL)
		throw "attempting to add a cell member to a non-cell variable.";
	cell.push_back(sec);
	return *this;
}

CVar& CVar::setcell(unsigned int id, CVar &sec)
{ // id one-based index
	if (GetType() != CSIG_EMPTY && GetType() != CSIG_CELL)
		throw "cannot add a cell member to a non-cell variable.";
	if (id < cell.size() + 1)
		cell[id-1] = sec; //replace existing cell 
	else
	{
		CVar tp;
		for (size_t k = cell.size(); k < id-1; k++)
			cell.push_back(tp);
		appendcell(sec);
	}
	return *this;
}

CSignals& CSignals::reciprocal(void)
{
	CTimeSeries::reciprocal();
	if (next)	next->reciprocal();
	return *this;
}

CSignals& CSignals::operator>>=(const double delta)
{
	CTimeSeries::operator>>=(delta);
	if (next)	*next>>=delta;
	return *this;
}

CSignals& CSignals::Take(CSignals& out, int id1, int id2)
{
	CTimeSeries out2(out.fs);
	CTimeSeries::Take(out, id1, id2);
	if (next != NULL)
	{
		next->Take(out2, id1, id2);
		out.SetNextChan(&out2);
	}
	return out;
}

CSignals& CSignals::Crop(double begin_ms, double end_ms)
{
	CTimeSeries::Crop(begin_ms, end_ms);
	if (next)		next->Crop(begin_ms, end_ms);
	return *this;
}

CSignals& CSignals::ReplaceBetweenTPs(CSignals &newsig, double t1, double t2)
{
	CTimeSeries::ReplaceBetweenTPs(newsig, t1, t2);
	if (next)	next->ReplaceBetweenTPs(newsig, t1, t2);
	return *this;
}

CSignals& CSignals::Insert(double timept, CSignals &newchunk)
{
	CTimeSeries::Insert(timept, newchunk);
	if (next)	next->Insert(timept, newchunk);
	return *this;
}

CSignal& CSignal::Modulate (double *env, unsigned int lenEnv, unsigned int beginID)
{
	// Modulate this object with env 
	// If lenEnv is greater than nSamples, then the samples in env after nSamples are ignored.
	// beginID indicates the index of buf to start Modulate
	for (unsigned int k = beginID; k<min(nSamples,lenEnv+beginID); k++)
		buf[k] *= env[k - beginID];
	return *this;
}

vector<double> CTimeSeries::tmarks()
{
	vector<double> out;
	for (CTimeSeries *p = this; p; p = p->chain)
		out.push_back(p->tmark);
	return out;
}

CTimeSeries& CTimeSeries::Modulate(CTimeSeries env)
{ // VERIFYI THIS..........................5/23/2018
//	map<double, double> et = env.endt().showtseries();
	double t1, t2;
	for (CTimeSeries *p = this; p; p = p->chain)
	{
		CSignal *pp = (CSignal*)p;
		t1 = pp->tmark;
		t2 = pp->endt();
		for (CTimeSeries *q = &env; q; q = q->chain)
		{
			CSignal *qq = (CSignal*)q;
			if (qq->tmark > t2) continue;
			if (qq->endt() < t1) continue;
			if (qq->tmark <= t1)
			{
				unsigned int countsOverlap = (unsigned int)((qq->endt() - t1) * fs / 1000.); // this should be an integer...verify it
				pp->Modulate(qq->buf, countsOverlap); // even if qq ends beyond the endt of pp, that's OK. It won't go further.
			}
			else
			{
				unsigned int countsSkip = (unsigned int)( (qq->tmark-t1) * fs / 1000. );
				pp->Modulate(qq->buf, qq->nSamples, countsSkip + 1);
			}
		}
	}
	return *this;
}

CSignals& CSignals::Modulate(CSignals env)
{
	CTimeSeries::Modulate((CTimeSeries)env);
	if (next)
	{
		if (env.next)
			next->Modulate((CTimeSeries)*env.next);
		else
			next->Modulate((CTimeSeries)env);
	}
	return *this;
}


double * CSignals::Mag()
{ // I don't remember any more when I used this last time. 11/28/2016
	// output of FFT ... if used in other context, note...to generalize // 11/26/2016
	CSignal out(*this);
	if (!IsComplex()) return buf; // if real, return as is.
	out.UpdateBuffer(nSamples);
	for (unsigned int k(0); k<nSamples; k++) 		
		out.buf[k] = sqrt(buf[2*k]*buf[2*k]+buf[2*k+1]*buf[2*k+1]);
	SetReal();
	memcpy(buf, out.buf, sizeof(double)*nSamples);
	return buf;
}

CSignal CSignals::Angle()
{// I don't remember any more when I used this last time. 11/28/2016
	//"imaginary" --- output of FFT
	CSignal out(*this);
	double *x1 = buf;
	double *x2 = next->buf;
	out.UpdateBuffer(nSamples);
	for (unsigned int i=0; i<nSamples; i++)
		buf[i] = atan(x2[i]/x1[i]);
	return *this;
}

void CSignals::DownSample(int cc)
{
	CTimeSeries::DownSample(cc);
	if (next!=NULL) next->DownSample(cc);
}

void CSignals::UpSample(int cc)
{
	CTimeSeries::UpSample(cc);
	if (next!=NULL) next->UpSample(cc);
}

CSignals &CSignals::transpose1()
{ 
	CTimeSeries::transpose1(); 
	if (next) 
		next->transpose1(); 
	return *this; 
}

#ifndef NO_RESAMPLE
double * CSignals::Resample(int newfs, char *errstr)
{
	CTimeSeries::Resample(newfs, errstr);
	if (next!=NULL) next->CTimeSeries::Resample(newfs, errstr);
	return buf;
}

double * CSignals::Resample(vector<unsigned int> newfs, vector<unsigned int> lengths, char *errstr)
{
	CTimeSeries::Resample(newfs, lengths, errstr);
	if (next!=NULL) next->Resample(newfs, lengths, errstr);
	return buf;
}

double * CSignals::fm2(CSignal flutter, int multiplier, char *errstr)
{
	CSignal::fm2(flutter, multiplier, errstr);
	if (next!=NULL) next->fm2(flutter, multiplier, errstr);
	return buf;
}

#endif // NO_RESAMPLE

CSignals& CSignals::LogOp(CSignals &rhs, int type)
{
	CTimeSeries::LogOp(rhs,type);
	if (next) next->LogOp(rhs,type);
	return *this;
}

double CSignals::MakeChainless()
{
	if (next != NULL && nSamples == 0)
	{
		UpdateBuffer(next->nSamples);
		fs = next->GetFs();
	}
	CTimeSeries::MakeChainless();
	if (next!=NULL) 
	{
		next->MakeChainless();
		int diff = nSamples - next->nSamples;
		CSignals silenc(fs);
		if (diff>0) // next is shorter (next needs padding)
			next->UpdateBuffer(nSamples);
		else
			UpdateBuffer(next->nSamples);
	}
	return alldur();
}

CSignals &CSignals::each_sym(double(*fn)(double))
{
	CTimeSeries::each(fn);
	if (next) 	next->each(fn);
	return *this;
}

CSignals &CSignals::each(double (*fn)(double))
{
	CTimeSeries::each(fn);
	if (next) 	next->each(fn);
	return *this;
}

CSignals &CSignals::each(complex<double> (*fn)(complex<double>))
{
	CTimeSeries::each(fn);
	if (next) 	next->each(fn);
	return *this;
}

CSignals &CSignals::each(double (*fn)(complex<double>))
{
	CTimeSeries::each(fn);
	if (next) 	next->each(fn);
	return *this;
}

CSignals &CSignals::each(double (*fn)(double, double), body &arg2)
{
	CTimeSeries::each(fn, arg2);
	if (next) 	next->each(fn, arg2);
	return *this;
}

CSignals &CSignals::each(complex<double> (*fn)(complex<double>, complex<double>), body &arg2)
{
	CTimeSeries::each(fn, arg2);
	if (next) 	next->each(fn, arg2);
	return *this;
}

CSignals & CSignals::MFFN(double(*fn)(double), complex<double>(*cfn)(complex<double>))
{
	CTimeSeries::MFFN(fn, cfn);
	if (next) 	next->MFFN(fn, cfn);
	return *this;
}

CSignals & CSignals::MFFN(double(*fn)(double, double), complex<double>(*cfn)(complex<double>, complex<double>), const CSignals &param)
{
	CTimeSeries::MFFN(fn, cfn, param);
	if (next) 	next->MFFN(fn, cfn, param);
	return *this;
}
#ifndef NO_SF

CSignals::CSignals(const char* wavname)
:next(NULL)
{
	char errstr[256];
	Wavread(wavname, errstr);

}

int CSignals::Wavread(const char *wavname, char *errstr)
{
	SNDFILE *wavefileID;
	SF_INFO sfinfo ;
	sf_count_t count ;
	if ((wavefileID = sf_open (wavname, SFM_READ, &sfinfo)) == NULL)
	{	sprintf(errstr, "Unable to open audio file '%s'\n", wavname);
		sf_close (wavefileID) ;
		return NULL;	}
	if (sfinfo.channels>2) { strcpy(errstr, "Up to 2 channels (L R) are allowed in AUX--we have two ears, dude!"); return NULL;}
	Reset(sfinfo.samplerate);
	SetNextChan(NULL); // Cleans up existing next
	if (sfinfo.channels==1)
	{
		UpdateBuffer((int)sfinfo.frames);
		count = sf_read_double (wavefileID, buf, sfinfo.frames);  // common.h
	}
	else
	{
		double *buffer = new double[(unsigned int)sfinfo.channels*(int)sfinfo.frames];
		count = sf_read_double (wavefileID, buffer, sfinfo.channels*sfinfo.frames);  // common.h
		double (*buf3)[2];
		next = new CTimeSeries(sfinfo.samplerate);
		int m(0);
		buf3 = (double (*)[2])&buffer[m++];
		UpdateBuffer((int)sfinfo.frames);
		for (unsigned int k=0; k<sfinfo.frames; k++)			buf[k] = buf3[k][0];
		buf3 = (double (*)[2])&buffer[m++];
		next->UpdateBuffer((int)sfinfo.frames);
		for (unsigned int k=0; k<sfinfo.frames; k++)			next->buf[k] = buf3[k][0];
		delete[] buffer;
	}
	sf_close (wavefileID) ;
	return 1;
}

int CSignals::Wavwrite(const char *wavname, char *errstr, std::string wavformat)
{
	MakeChainless();
	SF_INFO sfinfo ;
	SNDFILE *wavefileID;
	sfinfo.channels = (next) ? 2 : 1; 
	if (wavformat.length()==0)
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_PCM_16; // default
	else if (wavformat=="8")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_PCM_U8;
	else if (wavformat=="16")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_PCM_16;
	else if (wavformat=="24")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_PCM_24;
	else if (wavformat=="32")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_PCM_32;
	else if (wavformat=="ulaw")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_ULAW;
	else if (wavformat=="alaw")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_ALAW;
	else if (wavformat=="adpcm1")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_IMA_ADPCM;
	else if (wavformat=="adpcm2")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_MS_ADPCM;
//	else if (wavformat=="vorbis")
//		sfinfo.format = SF_FORMAT_OGG + SF_FORMAT_VORBIS; // not available ...  ogg.c requires external lib which I don't have yet. bjkwon 03/19/2016
	else
	{	sprintf(errstr, "Supported waveformat---8, 16, 24, 32, ulaw, alaw, adpcm1 or adpcm2.\n");
		return NULL;	}
	sfinfo.frames = nSamples;
	sfinfo.samplerate = fs;
	sfinfo.sections = sfinfo.seekable = 1;
	if ((wavefileID = sf_open (wavname, SFM_WRITE, &sfinfo)) == NULL)
	{	sprintf(errstr, "Unable to open/write audio file to '%s'\n", wavname);
		sf_close (wavefileID) ;
		return NULL;	}
	if (sfinfo.channels==1)
		sf_writef_double(wavefileID, buf, nSamples);
	else
	{
		double *buffer = new double[(unsigned int)sfinfo.channels*nSamples];
		double (*buf3)[2];
		int m(0);
		// it should not be p && m<2 but just m<2, because p will never be NULL (why? next is CTimeSeries, not CSignals)
		for (CSignals *p = this; p && m<2; p=(CSignals*)p->next, m++)
		{
			buf3 = (double (*)[2])&buffer[m];
			for (unsigned int k=0; k<nSamples; k++)			
				buf3[k][0] = p->buf[k];
		}
		sf_writef_double(wavefileID, buffer, nSamples);
		delete[] buffer;
	}
	sf_close (wavefileID) ;
	return 1;
}
#endif // NO_SF

#ifdef _WINDOWS

#ifndef NO_PLAYSND

INT_PTR CSignals::PlayArray(char *errstr)
{
	return PlayArray(0, errstr);
}

INT_PTR CSignals::PlayArray(int DevID, char *errstr)
{ // returns a negative number if error occurrs
	return PlayArray(DevID, 0, NULL, 2, errstr); 
	// This is how you play in blocking mode (specify 2 for the nProgReport even though you are not utilizing any messaging back to hWnd.. This is just due to the way wavBuffer2snd is written in waves.cpp)
	// Jan 19, 2013. BJ Kwon
}

INT_PTR CSignals::PlayArray(int DevID, UINT userDefinedMsgID, HWND hApplWnd, double *block_dur_ms, char *errstr, int loop)
{// returns a negative number if error occurrs
 // This play the sound by specified block duration, generating event notification in every block
 // block_dur_ms is adjusted by the quantization of fs. Therefore, user should check if it has been adjusted during this call.
 // But block_dur_ms is not adjusted by the playbuffer situation that happens inside PlayBufAsynch16 (wavplay.cpp).
	int nSamples4Block = (int)(*block_dur_ms / (1000. / (double)fs) + .5);
	*block_dur_ms = (double)nSamples4Block *1000. / (double)fs;
	double _nBlocks = (double)nSamples / nSamples4Block;
	int nBlocks = max(2,(int)ceil(_nBlocks));
	return PlayArray(DevID, userDefinedMsgID, hApplWnd, nBlocks, errstr, loop);
}

INT_PTR CSignals::PlayArray(int DevID, UINT userDefinedMsgID, HWND hApplWnd, int nProgReport, char *errstr, int loop)
{// Re-do error treatment 6/1/2016 bjk
	errstr[0] = 0;
	int nChan, ecode(MMSYSERR_NOERROR);
	short *Buffer2Play = makebuffer(nChan);
	return (INT_PTR)PlayBufAsynch16(DevID, Buffer2Play, nSamples, nChan, fs, userDefinedMsgID, hApplWnd, nProgReport, loop, errstr);
}


INT_PTR CSignals::PlayArrayNext(INT_PTR pWP, int DevID, UINT userDefinedMsgID, double *block_dur_ms, char *errstr, int loop)
{// returns a negative number if error occurrs
 // This play the sound by specified block duration, generating event notification in every block
 // block_dur_ms is adjusted by the quantization of fs. Therefore, user should check if it has beend adjusted during this call.
 	int nSamples4Block = (int)(*block_dur_ms/(1000./(double)fs)+.5);
	*block_dur_ms = (double) nSamples4Block *1000. /(double)fs;
	double _nBlocks = (double)nSamples / nSamples4Block;
	int nBlocks = (int)_nBlocks;
	if (_nBlocks - (double)nBlocks > 0.1) nBlocks++;
	return PlayArrayNext(pWP, DevID, userDefinedMsgID, nBlocks, errstr, loop);
}


INT_PTR CSignals::PlayArrayNext(INT_PTR pWP, int DevID, UINT userDefinedMsgID, int nProgReport, char *errstr, int loop)
{
	errstr[0] = 0;
	int nChan, ecode(MMSYSERR_NOERROR);
	short *Buffer2Play = makebuffer(nChan);
	return QueuePlay(pWP, DevID, Buffer2Play, nSamples, nChan, userDefinedMsgID, nProgReport, errstr, loop);
}

short * CSignals::makebuffer(int &nChan)
{	//For now this is only 16-bit playback (Sep 2008)
	short *Buffer2Play;
	MakeChainless();
	if (next!=NULL)
	{
		double *buf2 = next->buf;
		Buffer2Play = new short[nSamples*2];
		for (unsigned int i=0; i<nSamples; ++i) {
			Buffer2Play[i*2] = (short)(_double_to_24bit(buf[i]) >> 8);
			Buffer2Play[i*2+1] = (short)(_double_to_24bit(buf2[i]) >> 8);
		}
		nChan=2;
	}
	else
	{
		Buffer2Play = new short[nSamples];
		_double_to_short(buf, Buffer2Play, nSamples);
		nChan=1;
	}
	return Buffer2Play;
}


#endif // NO_PLAYSND

#endif

#ifndef NO_FFTW

CSignal CSignal::FFT(unsigned int id0, unsigned int len)
{
	CVar param = *(CVar*)parg;
	if (len == 0) len = nSamples;
	if (len != nSamples)
		if (param.value() != 0)
			throw "The FFT is of a matrix on each group/row is fixed (cannot be changed).";
	int fftsize = param.value() == 0 ? len : (int)param.value();
	int fftRealsize = fftsize / 2 + 1;
	double *in;
	fftw_complex *out;
	fftw_plan p;

	in = (double*) fftw_malloc(sizeof(double) * len);
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fftRealsize);
	memcpy(in, buf+id0, sizeof(double)*len);

	p = fftw_plan_dft_r2c_1d(fftsize, in, out, FFTW_ESTIMATE);
	fftw_execute(p);

	CSignal res(fs);
	res.UpdateBuffer(fftsize);
	res.SetComplex();
	memcpy(res.cbuf, out, sizeof(*cbuf)*fftRealsize);
	complex<double> *tp = reinterpret_cast<complex<double> *> (out);
	for (int k(1); k<fftRealsize; k++)
		res.cbuf[fftsize -k] = conj(tp[k]);

// For verification
//	double *out2 = (double*)fftw_malloc(sizeof(double) * len);
//	fftw_plan p2 = fftw_plan_dft_c2r_1d(len, out, out2, FFTW_ESTIMATE);
//	fftw_execute(p2);
//	fftw_destroy_plan(p2);
//	fftw_free(out2);

	fftw_destroy_plan(p);
	fftw_free(in); 
	fftw_free(out);
	return res;
}

CSignal CSignal::iFFT(unsigned int id0, unsigned int len)
{
	CVar param = *(CVar*)parg;
	if (len == 0) len = nSamples;
	if (len != nSamples)
		if (param.value() != 0)
			throw "The iFFT is of a matrix on each group/row is fixed (cannot be changed).";
	int fftsize = param.value() == 0 ? len : (int)param.value();
	int fftRealsize = fftsize / 2 + 1;
	CSignal res(fs);
	res.UpdateBuffer(fftsize);
	fftw_plan p;
	bool hermit(true);
	if (!IsComplex())
	{
		SetComplex();
		hermit = false;
	}
	fftw_complex *in;
	//check if it's Hermitian
	for (int k = 1; hermit && k < (fftsize + 1) / 2; k++)
		if (cbuf[k] != conj(cbuf[fftsize - k])) hermit = false;
	if (hermit)
	{
		double *out = (double*)fftw_malloc(sizeof(double) * fftsize);
		in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * fftRealsize);
		memcpy(in, cbuf + id0, sizeof(fftw_complex)*fftsize / 2);
		if (fftsize % 2 == 0)
			memcpy(&in[fftsize / 2], &cbuf[fftsize / 2], sizeof(*cbuf));
		p = fftw_plan_dft_c2r_1d(fftsize, in, out, FFTW_ESTIMATE);
		fftw_execute(p);
		memcpy(res.buf, out, sizeof(double)*fftsize);
		res.bufBlockSize = sizeof(double);
		res /= (double)fftsize;
		fftw_free(out);
	}
	else
	{
		res.SetComplex();
		fftw_complex *out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (fftsize+1));
		in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * fftsize);
		memcpy(in, cbuf + id0, sizeof(*cbuf)*fftsize);
		p = fftw_plan_dft_1d(fftsize, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		memcpy(res.cbuf, out, sizeof(fftw_complex)*fftsize);
		res /= (double)fftsize;
		fftw_free(out);
	}
	fftw_free(in);
	fftw_destroy_plan(p);
	return res;
}

#endif

int CSignals::GetType() const
{
	if (nSamples > 0 || !next)
		return CTimeSeries::GetType();
	else
		return next->GetType();
}

int CSignals::GetTypePlus() const
{
	int res = GetType();
	if (res == CSIG_VECTOR || res == CSIG_AUDIO)
		return res - (int)(bufBlockSize == 1);
	else
		return res;
}

int CSignals::ReadAXL(FILE* fp, bool logical, char *errstr)
{
	unsigned int nChains, check = sizeof(nSamples);
	size_t res;
	int res2;
	res = fread((void*)&fs, sizeof(fs), 1, fp);
	res2 = ftell(fp);
	Reset(fs);
	res = fread((void*)&nChains, sizeof(nChains), 1, fp);
	res2 = ftell(fp);
	fseek(fp, 0, SEEK_END);
	unsigned int endp = ftell(fp);
	fseek(fp, res2, SEEK_SET);
	for (unsigned int k=0; k < nChains; k++)
	{
		unsigned int cum(0), _nSamples;
		CTimeSeries readsig(fs);
		res = fread((void*)&_nSamples, sizeof(nSamples), 1, fp); // readsig.nSamples shouldn't be directly modified, it should be done inside UpdateBuffer()
		res2 = ftell(fp);
		readsig.UpdateBuffer(_nSamples);
		if (logical) readsig.MakeLogical();
		res = fread((void*)&readsig.tmark, sizeof(tmark), 1, fp);
		res2 = ftell(fp);
		while (cum < readsig.nSamples)
		{
			res = fread((void*)&readsig.logbuf[cum], readsig.bufBlockSize, readsig.nSamples - cum, fp);
			cum += (int)res;
			if (!res)
			{
				res2 = ftell(fp);
				if (res2 == endp)
				{
					sprintf(errstr, "expecting %d bytes, data terminated at %d bytes", readsig.nSamples, cum);
					readsig.nSamples = cum;
					break;
				}
			}
		}
		AddChain(readsig);
		res2 = ftell(fp);
	}
	MergeChains();
	errstr[0] = 0;
	return 1;
}

int CSignals::WriteAXL(FILE* fp)
{
	int res = CTimeSeries::WriteAXL(fp);
	if (next)
		res += next->CTimeSeries::WriteAXL(fp);
	return res;
}

map<double, double> CTimeSeries::showtseries()
{
	map<double, double> out;
	bool nullchain = nSamples==0;
	if (nullchain)
		for (CTimeSeries *p = this; p; p = p->chain)
			if (p->nSamples) throw "T_SERIES NULL not consistent.";
			else out[-p->tmark] = 0;
	else
		for (CTimeSeries *p = this; p; p = p->chain)
			if (p->nSamples==0) throw "T_SERIES NULL not consistent.";
			else out[p->tmark] = p->value();
	return out;
}


bool CVar::IsGO()
{ //Is this Graphic Object?
	if (fs == 3) return true;
	if (strut.empty()) return false;
	if (strut.find("type") == strut.end()) return false;
	if (strut.find("pos") == strut.end()) return false;
	if (strut.find("color") == strut.end()) return false;
	if (strut.find("userdata") == strut.end()) return false;
	if (strut.find("tag") == strut.end()) return false;
	if (strut.find("visible") == strut.end()) return false;
	//if (strut.find("parent") == strut.end()) return false;
	//if (strut.find("children") == strut.end()) return false;
	return true;
}

CVar&  CVar::length()
{ 
	if (IsGO()) 
	{
		if (fs == 3) SetValue((double)nSamples);
		else SetValue(1.);
	} 
	else
		basic(pf_basic = &CSignal::length);
	return *this;
}

bool CVar::IsAudioObj()
{
	if (strut.empty()) return false;
	if (strut.find("type") == strut.end()) return false;
	CSignals type = strut["type"];
	if (type.GetType() != CSIG_STRING) return false;
	//The text content may be audio_playback or audio_playback (inactive)
	if (type.string().find("audio_playback")!=0) return false;
	return true; 
}

CVar::CVar()
{
}

CVar::~CVar()
{
}

CVar::CVar(const CSignals& src)
	:functionEvalRes(false)
{
	*this = src;
}

CVar::CVar(const CVar& src)
	: functionEvalRes(false)
{
	*this = src;
}

CVar& CVar::Reset(int fs2set)
{
	CSignals::Reset(fs2set);
	cell.clear();
	strut.clear();
	struts.clear();
	return *this;
}

int CVar::GetType()
{
	if (fs==3)
		return CSIG_HDLARRAY;
	if (!cell.empty())
		return CSIG_CELL;
	if (strut.size() > 0)
	{
		if (IsGO() || IsAudioObj()) // file handle?
			return CSIG_HANDLE;
		else
			return CSIG_STRUCT;
	}
	if (struts.size() > 0) // this means a.member = GO
		return CSIG_STRUCT;
	else
		return CSignals::GetType();
}

int CVar::GetTypePlus()
{
	int res = GetType();
	if (res == CSIG_VECTOR || res == CSIG_AUDIO)
		return res - (int)(bufBlockSize == 1);
	else
		return res;
}

CVar& CVar::operator=(const CSignals& rhs)
{
	if (this != &rhs)
		CSignals::operator=(rhs);
	return *this;
}

CVar& CVar::operator=(const CVar& rhs)
{   // Make a deep copy only for buf, but not for sc, because sc is not necessarily dynamically allocated.
	// Thus, BE Extra careful when making writing an assignment statement about the scaling..
	if (this != &rhs)
	{
		CSignals::operator=(rhs);
		cell = rhs.cell;
		auto it = strut.find("gc");
		if (it != strut.end())
		{
			auto sss = (*it).second;
			CVar *ptemp = &sss;
		}
		strut = rhs.strut;
		struts = rhs.struts;
	}
	return *this; // return self-reference so cascaded assignment works
}
