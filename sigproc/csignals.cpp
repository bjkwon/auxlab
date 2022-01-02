// AUXLAB 
//
// Copyright (c) 2009-2019 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.7
// Date: 5/24/2020 

// Time sequence with nSamples > 1 must set snap 1
// so the data doesn't go with time but it is about at that particular tmark
// 5/25/2020 

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

#include <complex>

#include <math.h> // ceil

#include "psycon.tab.h"

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

static double quantizetmark(double delta, int fs)
{
	//quantize delta based on the fs grid.  3/31/2016. Rev. 5/19/2018
	if (fs > 1)
	{
		int pt = (int)round(delta / 1000 * fs);
		return 1000. * pt / fs;
	}
	return delta;
}

static void SwapCSignalsPutScalarSecond(CSignals &sig1, CSignals &sig2)
{
	if (!sig1.IsStereo() && sig2.IsStereo())
	{
		CSignals copy(sig1);
		sig1 = sig2, sig2 = copy;
	}
}

/*bufBlockSize is 1 (for logical, string), sizeof(double) (regular), or sizeof(double)*2 (complex).     4/4/2017 bjkwon */
body::body()
	:nSamples(0), bufBlockSize(sizeof(double)), buf(NULL), res(NULL), nGroups(1), ghost(false)
{
}

body::body(double value)
	: nSamples(0), buf(NULL), res(NULL), ghost(false)
{
	SetValue(value);
}

body::body(complex<double> value)
	: nSamples(0), buf(NULL), res(NULL), ghost(false)
{
	cbuf[0] = value;
}

body::body(const body& src)
	: nSamples(0), buf(NULL), res(NULL), ghost(false)
{
	*this = src;
}

body::body(double *y, int len)
	: nSamples(len), bufBlockSize(sizeof(double)), buf(new double[len]), nGroups(1), ghost(false)
{
	memcpy(buf, y, bufBlockSize*len);
}

body::body(bool *y, int len)
	: nSamples(len), bufBlockSize(1), logbuf(new bool[len]), nGroups(1), ghost(false)
{
	memcpy(buf, y, bufBlockSize*len);
}

body::body(const vector<double> & src)
	: nSamples((unsigned int)src.size()), bufBlockSize(sizeof(double)), buf(new double[src.size()]), nGroups(1), ghost(false)
{
	memcpy(buf, src.data(), sizeof(double)*nSamples);
}

body& body::operator=(const vector<double> & rhs)
{
	nSamples = (unsigned int)rhs.size();
	bufBlockSize = 8;
	nGroups = 1;
	ghost = false;
	buf = new double[nSamples];
	return *this;
}

body& body::operator=(const body & rhs)
{
	if (this != &rhs)
	{
		unsigned int currentBufSize = bufBlockSize * nSamples;
		unsigned int reqBufSize = rhs.bufBlockSize * rhs.nSamples;
		nSamples = rhs.nSamples;
		bufBlockSize = rhs.bufBlockSize;
		nGroups = rhs.nGroups;
		if (reqBufSize > currentBufSize)
		{
			if (!ghost && buf)
				delete[] buf;
			if (!rhs.ghost)
				logbuf = new bool[reqBufSize];
		}
		ghost = rhs.ghost;
		if (!rhs.ghost)
			memcpy(buf, rhs.buf, nSamples*bufBlockSize);
		else
			buf = rhs.buf;// shallow (ghost) copy
//		resOutput = move(rhs.resOutput); // Cannot move because it is const... Then how? 11/29/2019
	}
	return *this;
}

body& body::MakeLogical()
{
	if (bufBlockSize == 1) return *this;
	body out;
	out.bufBlockSize = 1; // logical array
	out.UpdateBuffer(nSamples); // This over-reserve the memory, but there's no harm. 4/4/2017 bjk
	for (unsigned int k = 0; k < nSamples; k++)
	{
		if (buf[k] == 0.) out.logbuf[k] = false;
		else			out.logbuf[k] = true;
	}
	out.nGroups = nGroups;
	return (*this = out);
}

CSignal& CSignal::operator=(const body& rhs)
{ // Used only in AuxFunc.cpp 12/30/2017
	if (this != &rhs)
	{
		Reset();
		if (rhs.nSamples > 0)
			body::operator=(rhs);
	}
	return *this;
}

CSignal& CSignal::operator=(const CSignal& rhs)
{   // Make a deep copy only for buf, but not for sc, because sc is not necessarily dynamically allocated.
	// Thus, BE Extra careful when making writing an assignment statement about the scaling..
	if (this != &rhs)
	{
		Reset(rhs.fs);
		if (rhs.fs > 0) // Don't change to the line below... it affects just about everything 12/13/2020
		// if (rhs.type() & TYPEBIT_TSEQ) 
			tmark = quantizetmark(rhs.tmark, rhs.fs);
		else // this way, quantize is skipped for t-seq with relative tmarks.
		{
			tmark = rhs.tmark;
			SetFs(0);
		}
		snap = rhs.snap;
		body::operator=(rhs);
	}
	return *this;
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
		strut = rhs.strut;
		struts = rhs.struts;
	}
	return *this;
}

CVar & CVar::operator=(CVar * rhs)
{
	return *this = *rhs;
}

bool CVar::operator==(std::string rhstr)
{
	return *this == CSignals(rhstr);
}
bool CVar::operator==(double val)
{
	return *this == CSignals(val);
}

bool CSignals::operator==(double rhs)
{
	return *this == CSignals(rhs);
}

bool CSignals::operator==(std::string rhstr)
{
	return *this == CSignals(rhstr);
}

CTimeSeries& CTimeSeries::operator=(const CSignal& rhs)
{
	if (ghost)
		CSignal::operator=(rhs);
	else
		CSignal::operator=(rhs);
	return *this;
}

bool CSignal::operator==(double rhs)
{
	return *this == CSignal(rhs);
}

bool CSignal::operator==(std::string rhstr)
{
	return *this == CSignal(rhstr);
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
			if (!ghost) delete chain;
			chain = NULL;
		}
	}
	return *this;
}

CSignals& CSignals::operator=(const CTimeSeries& rhs)
{ // Don't use this!!!!
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
	}
	return *this;
}

body& body::operator+=(const double con)
{
	unsigned int blockSize = bufBlockSize / sizeof(double);
	for (unsigned int k = 0; k < nSamples; k++) buf[k*blockSize] += con;
	return *this;
}

body& body::operator*=(const double con)
{
	unsigned int blockSize = bufBlockSize / sizeof(double);
	switch (blockSize)
	{
	case 1:
		for (unsigned int k = 0; k < nSamples; k++) buf[k] *= con;
		break;
	case 2:
		for (unsigned int k = 0; k < nSamples; k++) cbuf[k] *= con;
		break;
	}
	return *this;
}

body& body::operator/=(const double con)
{
	return *this *= 1. / con;
}

body::~body()
{
	if (buf) delete[] buf;
	nSamples = 0;
}

void body::SetValue(double v)
{ // why did I change it this way?
 // this way I can just replace a value with the existing value 
 // (i.e., if it was already CSIG_SCALR, just reuse the buffer) 8/15/2018
	if (bufBlockSize != sizeof(double) || nSamples != 1)
	{ 
		bufBlockSize = sizeof(double);
		if (buf) delete[] buf;
		buf = new double[1];
		nSamples = 1;
	}
	nGroups = 1;
	buf[0] = v;
}

void body::SetValue(complex<double> v)
{
	if (bufBlockSize != 2 * sizeof(double) || nSamples != 1)
	{
		bufBlockSize = 2 * sizeof(double);
		if (buf) delete[] buf;
		buf = new double[2];
		nSamples = 1;
	}
	nGroups = 1;
	cbuf[0] = v;
}

body& body::UpdateBuffer(unsigned int length, unsigned int offset)	// Set nSamples. Re-allocate buf if necessary to accommodate new length.
{
	if (!ghost)
	{
		unsigned int currentBufsize = bufBlockSize * nSamples;
		unsigned int reqBufSize = bufBlockSize * length;
		if (length < 0 || currentBufsize == reqBufSize)
			return *this;
		if (length > nSamples) {
			bool *newlogbuf = new bool[reqBufSize];
			if (nSamples > 0)
				memcpy(newlogbuf + offset * bufBlockSize, buf, nSamples*bufBlockSize);
			delete[] buf;
			logbuf = newlogbuf;
		}
		//initializing with zeros for the rest
		if (length > nSamples)
			memset(logbuf + (nSamples + offset) * bufBlockSize, 0, (length - nSamples - offset)*bufBlockSize);
		memset(logbuf, 0, offset * bufBlockSize);
	}
	//For ghost, the rationale for this call is unclear; but just update nSamples with length
	nSamples = length;
	return *this;
}

void body::Reset()
{
	if (!ghost && buf > 0)
		delete[] buf;
	buf = NULL;
	nSamples = 0;
	nGroups = 1;
	ghost = false;
	bufBlockSize = sizeof(double);
}

string body::valuestr(int digits) const
{ //this doesn't throw. Used for showvar.cpp (avoiding unhandled exceptions)
	ostringstream out;
	out.unsetf(ios::floatfield);
	out.precision(digits);
	if (nSamples >= 1)
	{
		if (bufBlockSize == 8)	out << buf[0];
		else if (bufBlockSize == 1)	out << (double)logbuf[0];
		else out << "complex";
	}
	else if (nSamples == 0)
		out << "(emptybuffer)";
	if (nSamples > 1)
		out << "(more)";
	return out.str();
}

double body::value() const
{
	if (nSamples == 1)
	{
		if (bufBlockSize == 8)	return buf[0];
		else if (bufBlockSize == 1)	return (double)logbuf[0];
		else throw "value( ) on complex value. Use cvalue instead.";
	}
	else if (nSamples == 0)
		throw "value( ) on null.";
	else
		throw "value( ) on vector/array.";
}

complex<double> body::cvalue() const
{
	if (nSamples == 1)
		return cbuf[0];
	else if (nSamples == 0)
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
		for (; k < nSamples; k++) buf[k] = buf[2 * k];
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
				for (auto k = id0; k < id0 + len2; k++)		cbuf[k] += rhs;
			else
				for (auto k = id0; k < id0 + len2; k++)		cbuf[k] *= rhs;
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
				for (auto k = id0; k < id0 + len; k++)		cbuf[k] += sec.cbuf[vertID*lenR + k - id0];
			else
				for (auto k = id0; k < id0 + len; k++)		cbuf[k] *= sec.cbuf[vertID*lenR + k - id0];
		}
		else
		{
			if (type == '+')
				for (auto k = id0; k < id0 + len; k++)		buf[k] += sec.buf[vertID*lenR + k - id0];
			else
				for (auto k = id0; k < id0 + len; k++)		buf[k] *= sec.buf[vertID*lenR + k - id0];
		}
	}
	if (IsComplex())
	{
		for (auto k = (unsigned)0; k < len; k++)
			if (imag(cbuf[k]) != 0.) return *this;
		//if it survives this far, it should be set real
		SetReal();
	}
	return *this;
}


body &body::each(double(*fn)(double))
{
	if (bufBlockSize == 1)
		for (unsigned int k = 0; k < nSamples; ++k)	logbuf[k] = fn(logbuf[k]) != 0;
	else
		for (unsigned int k = 0; k < nSamples; ++k)	buf[k] = fn(buf[k]);
	return *this;
}

body &body::each(double(*fn)(complex<double>))
{
	double *out = new double[nSamples];
	for (unsigned int k = 0; k < nSamples; ++k)	out[k] = fn(cbuf[k]);
	delete[] buf;
	buf = out;
	bufBlockSize = sizeof(double);
	return *this;
}

body &body::each(complex<double>(*fn)(double))
{
	for (unsigned int k = 0; k < nSamples; ++k)	cbuf[k] = fn(buf[k]);
	return *this;
}

body &body::each(complex<double>(*fn)(complex<double>))
{
	for (unsigned int k = 0; k < nSamples; ++k)	cbuf[k] = fn(cbuf[k]);
	return *this;
}

body &body::each(double(*fn)(double, double), const body &arg)
{
	if (arg.nSamples == 1)
	{
		double val = arg.value();
		if (bufBlockSize == 1 && arg.bufBlockSize == 1)
			for (unsigned int k = 0; k < nSamples; k++)	logbuf[k] = fn(logbuf[k], arg.logbuf[0]) != 0;
		else if (bufBlockSize == 1 && arg.bufBlockSize != 1)
			for (unsigned int k = 0; k < nSamples; k++)	logbuf[k] = fn(logbuf[k], val) != 0;
		else
			for (unsigned int k = 0; k < nSamples; k++)	buf[k] = fn(buf[k], val);
	}
	else if (nSamples == 1)
	{
		double baseval = buf[0];
		UpdateBuffer(arg.nSamples);
		if (bufBlockSize == 1 && arg.bufBlockSize == 1)
			for (unsigned int k = 0; k < arg.nSamples; k++) logbuf[k] = fn(baseval, arg.logbuf[k]) != 0;
		else if (bufBlockSize == 1 && arg.bufBlockSize != 1)
			for (unsigned int k = 0; k < arg.nSamples; k++) logbuf[k] = fn(baseval, arg.buf[k]) != 0;
		else
			for (unsigned int k = 0; k < arg.nSamples; k++) buf[k] = fn(baseval, arg.buf[k]);
	}
	else
	{
		nSamples = min(nSamples, arg.nSamples);
		if (bufBlockSize == 1 && arg.bufBlockSize == 1)
			for (unsigned int k = 0; k < nSamples; k++) logbuf[k] = fn(logbuf[k], arg.logbuf[k]) != 0;
		else if (bufBlockSize == 1 && arg.bufBlockSize != 1)
			for (unsigned int k = 0; k < nSamples; k++) logbuf[k] = fn(logbuf[k], arg.buf[k]) != 0;
		else
			for (unsigned int k = 0; k < nSamples; k++) buf[k] = fn(buf[k], arg.buf[k]);
	}
	return *this;
}

body &body::each(complex<double>(*fn)(complex<double>, complex<double>), const body &arg)
{
	if (arg.nSamples == 1)
	{
		complex<double> val = arg.cbuf[0];
		for (unsigned int k = 0; k < nSamples; k++)	cbuf[k] = fn(cbuf[k], val);
	}
	else if (nSamples == 1)
	{
		complex<double> val = arg.cbuf[0];
		UpdateBuffer(arg.nSamples);
		for (unsigned int k = 0; k < arg.nSamples; k++) cbuf[k] = fn(val, arg.cbuf[k]);
	}
	else
	{
		nSamples = min(nSamples, arg.nSamples);
		for (unsigned int k = 0; k < nSamples; k++) cbuf[k] = fn(cbuf[k], arg.buf[k]);
	}
	return *this;
}

body &body::mathFunForbidNegative(double(*fn)(double), complex<double>(*cfn)(complex<double>))
{
//	void *tp = parg;
//	parg = nullptr;
	if (bufBlockSize == 16 || _min() < 0)
	{
		SetComplex();
		each(cfn);
	}
	else
		each(fn);
//	parg = tp;
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

double lt(double a, double b)
{
	return a < b;
}
double le(double a, double b)
{
	return a <= b;
}
double gt(double a, double b)
{
	return a > b;
}
double ge(double a, double b)
{
	return a >= b;
}
double eq(double a, double b)
{
	return a == b;
}
double ne(double a, double b)
{
	return a != b;
}
double and (double a, double b)
{
	return a && b;
}
double or (double a, double b)
{
	return a || b;
}
double not (double a)
{
	if (a == 0) return 1.;
	else  return 0.;
}


body& body::LogOp(body &rhs, int type)
{
	double(*fn)(double, double);
	switch (type)
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
		fn = or ;
		break;
	case T_LOGIC_NOT:
		each(not);
		return *this;
	default:
		return *this;
	}
	each(fn, rhs);
	MakeLogical();
	return *this;
}


/* If consecutive, RHS with different length can be applied to LHS, updating this length.
If not consecutive, RHS must have the same length as LHS.
11/28/2020
*/

body& body::replacebyindex(vector<unsigned int>::iterator idBegin, vector<unsigned int>::iterator idEnd, const body & RHS)
{ // non-consecutive
	// ASSUME: 1) ind.size -- RHS.length; or RHS is scalar
	// 2) all values in ind must be in the range of this buf
	if (RHS.nSamples == 1)
	{
		if (bufBlockSize == 1)
			for (auto it = idBegin; it!=idEnd; it++) logbuf[*it] = RHS.logbuf[0];
		else if (bufBlockSize == 8)
			for (auto it = idBegin; it!=idEnd; it++) buf[*it] = RHS.buf[0];
		else // if (bufBlockSize == 16)
			for (auto it = idBegin; it!=idEnd; it++) cbuf[*it] = RHS.cbuf[0];
	}
	else
	{
		unsigned int k = 0;
		if (bufBlockSize == 1)
			for (auto it = idBegin; it!=idEnd; it++) logbuf[*it] = RHS.logbuf[k++];
		else if (bufBlockSize == 8)
			for (auto it = idBegin; it!=idEnd; it++) buf[*it] = RHS.buf[k++];
		else // if (bufBlockSize == 16)
			for (auto it = idBegin; it!=idEnd; it++) cbuf[*it] = RHS.cbuf[k++];
	}
	return *this;
}

body& body::replacebyindex(unsigned int id0, unsigned int len, const body & RHS)
{ // consecutive
	// ASSUME: RHS.bufBlockSize is the same as this bufBlockSize
// if RHS.length  > len --> update buffer
// else just update buffer content and update the length of this
	if (RHS.nSamples > len || id0 == nSamples)
	{
		bool* newbuf = new bool[(nSamples + RHS.nSamples - len) * bufBlockSize];
		memcpy(newbuf, logbuf, id0* bufBlockSize);
		memcpy(newbuf + id0 * bufBlockSize, RHS.buf, RHS.nSamples * bufBlockSize);
		memcpy(newbuf + (id0 + RHS.nSamples) * bufBlockSize, logbuf + (id0 + len) * bufBlockSize, (nSamples - id0 - len) * bufBlockSize);
		nSamples += RHS.nSamples - len;
		delete[] buf;
		logbuf = newbuf;
	}
	else
	{
		memmove(logbuf + id0 * bufBlockSize, RHS.buf, RHS.nSamples * bufBlockSize);
		if (RHS.nSamples != len)
			memmove(logbuf + (id0 + RHS.nSamples) * bufBlockSize, logbuf + (id0 + len) * bufBlockSize, (nSamples - id0 - len) * bufBlockSize);
		nSamples += RHS.nSamples - len;
	}
	return *this;
} 

body &body::insert(body &sec, int id)
{
	if (sec.nSamples == 0) return *this;
	if (id < 0) throw "insert index cannot be negative.";
	if (sec.bufBlockSize != bufBlockSize) throw "insert must be between the same data structure.";
	int nToMove = nSamples - id;
	UpdateBuffer(nSamples + sec.nSamples);
	bool *temp = new bool[nToMove*bufBlockSize];
	memcpy(temp, logbuf + id * bufBlockSize, nToMove*bufBlockSize);
	memcpy(logbuf + id * bufBlockSize, sec.buf, sec.nSamples*bufBlockSize);
	memcpy(logbuf + (id + sec.nSamples)*bufBlockSize, temp, nToMove*bufBlockSize);
	delete[] temp;
	return *this;
}

int migra(int input, int nGroup, int nSamples)
{
	int x1 = nGroup * (input - 1) + 1;
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
	int lasttrace = migra(2, nGroups, nSamples);
	for (unsigned int trace = 2; counter < nSamples - 2; trace++)
	{
		if (check[trace - 1]) continue;
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


body & body::interp1(body &that, body &qx)
{
	vector<double> v1 = ToVector();
	vector<double> v2 = that.ToVector();
	vector<double>::iterator it = v1.begin();
	int k = 0;
	body out;
	out.UpdateBuffer(qx.nSamples);
	for (unsigned int q = 0; q < qx.nSamples; q++)
	{
		it = upper_bound(it, v1.end(), qx.buf[q]); //because qx is sorted, having the iterator (previous searched result, if any) as the first argument will save time.
		ptrdiff_t pos;
		double valueAlready, preVal;
		if (it != v1.end())
		{
			pos = it - v1.begin();
			pos = max(1, pos);
			preVal = *(it - 1);
		}
		else
		{
			pos = that.nSamples;
			preVal = v1.back();
		}
		valueAlready = that.buf[pos - 1];
		out.buf[k++] = valueAlready + (that.buf[pos] - that.buf[pos - 1]) / (buf[pos] - buf[pos - 1]) * (qx.buf[q] - preVal);
	}
	return *this = out;
}

CSignal::CSignal()
	:fs(1), tmark(0.), snap(0)
{
}

CTimeSeries::CTimeSeries()
	: chain(NULL)
{
}

CSignal::CSignal(int sampleRate)
	: fs(max(sampleRate, 0)), tmark(0.), snap(0)
{
	if (fs == 2) bufBlockSize = 1;
}

CSignal::CSignal(int sampleRate, unsigned int len)
	: fs(max(sampleRate, 0)), tmark(0.), snap(0)
{
	if (fs == 2) bufBlockSize = 1;
	UpdateBuffer(len);
}

CTimeSeries::CTimeSeries(int sampleRate)
	: chain(NULL)
{
	fs = max(sampleRate, 0);
	tmark = 0.;
	if (fs == 2) bufBlockSize = 1;
}

CTimeSeries::CTimeSeries(int sampleRate, unsigned int len)
	: chain(NULL)
{
	fs = max(sampleRate, 0);
	tmark = 0.;
	if (fs == 2) bufBlockSize = 1;
	UpdateBuffer(len);
}


CSignal::CSignal(double value)
	:fs(1), tmark(0.), snap(0)
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
	snap = 0;
	UpdateBuffer((unsigned int)vv.size());
	for (unsigned int k = 0; k < (unsigned int)vv.size(); k++)
		buf[k] = vv[k];
}

CSignal::CSignal(std::string str)
	:snap(0)
{
	SetString(str.c_str());
}

CSignal::CSignal(const CSignal& src)
{
	*this = src;
}

CTimeSeries::CTimeSeries(const CSignal& src)
	: chain(NULL) //REQUIRED!!!!
{
	*this = src;
}

CTimeSeries::CTimeSeries(const CTimeSeries& src)
	: chain(NULL) //REQUIRED!!!!  otherwise, copied object return has uninitialized chain and causes a crash later. 5/24/2048
{
	*this = src;
}

CSignal::CSignal(double *y, int len)
	: fs(1), tmark(0.), snap(0)
{
	UpdateBuffer(len);
	memcpy((void*)buf, (void*)y, sizeof(double)*len);
}

vector<double> body::ToVector() const
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
	if (buf && !ghost) 
		delete[] buf;
	buf = nullptr;
	nSamples = 0;
}

CTimeSeries::~CTimeSeries()
{
	if (chain) {
//		if (!ghost) 
			delete chain;
		chain = NULL;
	}
}

void CSignal::SetFs(int  newfs)
{  // the old data (the content of buf) should be retained; don't call Reset() carelessly.
	if (bufBlockSize == 1 && newfs != fs)
	{// Trying to convert a data type with a byte size to double size
		double *newbuf = new double[nSamples];
		memset(newbuf, 0, sizeof(double)*nSamples);
		if (IsLogical())
			for (unsigned int k = 0; k < nSamples; k++)
				if (logbuf[k]) newbuf[k] = 1.;
		delete buf;
		buf = newbuf;
		bufBlockSize = sizeof(double);
	}
	fs = newfs; 
}


CSignal& CSignal::Reset(int fs2set)	// Empty all data fields - sets nSamples to 0.
{
	body::Reset();
	if (fs2set)	// if fs2set == 0 (default), keep the current fs.
		fs = max(fs2set, 1);	// not to allow minus.
	tmark = 0;
	snap = 0;
	return *this;
}

CTimeSeries& CTimeSeries::Reset(int fs2set)	// Empty all data fields - sets nSamples to 0.
{
	CSignal::Reset(fs2set);
	if (chain) {
		if (chain->ghost)
		{
			while (chain)
			{
				//clean from the last chain
				CTimeSeries * p = this, *pOneB4Last;
				for (; p->chain; p = p->chain)
				{
					if (p->chain && !p->chain->chain)
						pOneB4Last = p;
				}
				delete p;
				pOneB4Last->chain = NULL;
			}
		}
		else
		{
			delete chain;
			chain = NULL;
		}
	}
	return *this;
}

void CTimeSeries::SetChain(CTimeSeries *unit, double time_shifted)
{
	if (unit != NULL)
	{
		if (nSamples > 0)  delete[] buf;
		buf = NULL;
		chain = unit;
	}
}

void CTimeSeries::SetChain(double time_shifted)
{
	if (nSamples > 0)
	{
		tmark = quantizetmark(time_shifted, fs); ;
		chain = this;
		buf = NULL;
	}
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


CSignal& CSignal::Modulate(vector<double> &tpoints, vector<double> &tvals)
{
	//assumption: tpoints is increasing 
	auto itval = tvals.begin();
	for (auto it = tpoints.begin(); it != tpoints.end() - 1; it++)
	{
		if (*it > endt()) continue;
		double t1 = *it + tmark;
		double t2 = *(it + 1) + tmark;
		double slope = (*(itval + 1) - *itval) / (t2 - t1);
		double slopePerSample = (*(itval + 1) - *itval) / (t2 - t1) / fs * 1000.;
		unsigned int beginID = max(0, (unsigned int)round((t1 - tmark)*fs / 1000.));
		unsigned int endID = min(nSamples, (unsigned int)round((t2 - tmark)*fs / 1000.));
		for (unsigned int k = beginID; k < endID; k++)
			buf[k] *= *itval + slopePerSample * (k - beginID);
		itval++;
	}
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
	if (yy == NULL) return *this;
	if (IsEmpty()) return (*this = *yy);
	if (GetType() == CSIG_NULL) return *this = *yy >>= tmark;
	// Concatenation of CTimeSeries
  // yy might lose its value.
	if (GetFs() < 2)
		SetFs(yy->GetFs());

	CTimeSeries *ptail = GetDeepestChain();
	bool bb = GetType() == CSIG_AUDIO;
	bb = yy->GetType() == CSIG_AUDIO;
	bb = (GetType() == CSIG_AUDIO && yy->GetType() == CSIG_AUDIO) && (yy->chain || yy->tmark);
	if ((GetType() == CSIG_AUDIO && yy->GetType() == CSIG_AUDIO) && (yy->chain || yy->tmark)) {
		// when insertee has chain(s), chain it instead of copying. yy loses its value.
		CTimeSeries *pNew = new CTimeSeries(fs);
		if (chain && !yy->chain) // I don't know what would be the consequences of this leaving out  5/12/2016 ---check later??
			*pNew = *yy;	// make a copy, leaving the original intact for the next channel
		else
			pNew->SwapContents1node(*yy);		// pNew takes the insertee. As yy lost all its pointers, yy can be destroyed later without destroying any data it used to have.
		*pNew >>= ptail->tmark + ptail->_dur();	// to make the insertee the next chain(s) of ptail.
		ptail->chain = pNew;					// link it to *this.
	}
	else {
		// otherwise, just copy at the end.
		if (yy->nSamples > 0)
		{
			int nSamples0 = ptail->nSamples;
			if (yy->IsComplex()) ptail->SetComplex();
			if (IsComplex()) yy->SetComplex();
			ptail->UpdateBuffer(yy->nSamples + nSamples0);
			if (GetType() == CSIG_STRING)
				strcat(ptail->strbuf, yy->strbuf);
			else
				memcpy(&ptail->logbuf[nSamples0*bufBlockSize], yy->buf, bufBlockSize*yy->nSamples);
		}
		if (GetType() != CSIG_AUDIO && IsComplex())
		{
			for (unsigned int k = 0; k < nSamples; k++)
				if (imag(cbuf[k]) != 0.) return *this;
			//if it survives this far, it should be set real
			SetReal();
		}
	}
	return *this;
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

CTimeSeries CTimeSeries::fp_getval(double (CSignal::*fp)(unsigned int, unsigned int, void*) const, void *popt)
{
	// As of 12/19/2020, there's no need to worry about using a function going through fp_getval for string or logical type.
	int _fs = fs == 2 ? 1 : fs;
	CTimeSeries out(_fs);
	// aout output argument from fp_getval function pointer is always a scalar, i.e., double
	double outcarrier; 
	if (GetType() == CSIG_TSERIES)
	{
		out.Reset(1); // 1 means new fs
		CSignal tp = TSeries2CSignal();
		out.SetValue((tp.*fp)(0, 0, popt));
	}
	else
	{
		CTimeSeries tp(_fs);
		out.tmark = tmark;
		unsigned int len = Len();
		CVar aout(_fs); // additional output
		if (popt)
		{
			aout.UpdateBuffer(nGroups);
			aout.nGroups = nGroups;
		}
		// For audio, grouped data output is a chained output
		// For non-audio, grouped data output is a column vector
		if (fs > 2) // if audio
		{
			for (unsigned int k = 0; k < nGroups; k++)
			{
				tp.tmark = tmark + round(1000.*k*Len() / fs);
				if (popt)
				{ // popt is always a pointer to double
					tp.SetValue((this->*fp)(k * len, len, &outcarrier));
					aout.buf[k] = outcarrier; // fed from individual function
				}
				else
					tp.SetValue((this->*fp)(k * len, len, NULL));
				out.AddChain(tp);
			}
			for (CTimeSeries* p = chain; p; p = p->chain)
			{
				for (unsigned int k = 0; k < p->nGroups; k++)
				{
					tp.tmark = p->tmark + round(1000. * k * p->Len() / fs);
					if (popt)
					{ // popt is always a pointer to double
						tp.SetValue((p->*fp)(k * p->Len(), p->Len(), &outcarrier));
						CTimeSeries tp2(_fs);
						tp2.buf[k] = outcarrier;
						aout.AddChain(tp2);
					}
					else
						tp.SetValue((p->*fp)(k * p->Len(), p->Len(), NULL));
					out.AddChain(tp);
				}
			}
		}
		else
		{
			// This assumes that popt from the calling party is a pointer to a CVar object 
			if (popt && ((CVar*)popt)->nSamples	) outcarrier = ((CVar*)popt)->value();
			if (nGroups == nSamples) 
			{ // computing a column vector should yield a scalar. 7/7/2020
				nGroups = 1; 
				len = nSamples;
			}
			out.UpdateBuffer(nGroups);
			out.nGroups = nGroups;
			for (unsigned int k = 0; k < nGroups; k++)
			{
				if (popt)
				{
					out.buf[k] = (this->*fp)(k * len, len, &outcarrier);
					aout.buf[k] = outcarrier; // fed from individual function
				}
				else
					out.buf[k] = (this->*fp)(k * len, len, NULL);
			}
		}
		if (popt)	*(CVar*)popt = aout;
	}
	return out;
}

CTimeSeries& CTimeSeries::fp_mod(CSignal& (CSignal::*fp)(unsigned int, unsigned int, void*), void *popt)
{
	if (GetType() == CSIG_TSERIES)
	{
		//out.Reset(1); // 1 means new fs
		//CSignal tp = TSeries2CSignal();
		//out.SetValue((tp.*fp)(0, 0));
	}
	else
	{ 
		// must re-work with popt to accommodate the output from *fp
		// 12/20/2020
		for (CTimeSeries* p = this; p; p = p->chain)
			for (unsigned int k = 0; k < nGroups; k++)
			{
				auto len = p->Len();
				(p->*fp)(k * len, len, popt);
			}
	}
	return *this;
}

CTimeSeries CTimeSeries::fp_getsig(CSignal(CSignal::*fp)(unsigned int, unsigned int, void*) const, void *popt)
{
//	parg = popt;
	int _fs = fs == 2 ? 1 : fs;
	CTimeSeries out(_fs);
	if (GetType() == CSIG_TSERIES)
	{
		//out.Reset(1); // 1 means new fs
		//CSignal tp = TSeries2CSignal();
		//out.SetValue((tp.*fp)(0, 0));
	}
	else
	{
		CSignal sbit0, sbit(_fs);
		sbit.tmark = tmark;
		out.tmark = tmark;
		unsigned int len = Len();
		for (unsigned int k = 0; k < nGroups; k++)
		{
			out += &(CTimeSeries)(this->*fp)(k*len, len, popt);
			//			sbit0 += &sbit;
		}
		out.nGroups = nGroups;
		CTimeSeries tp(sbit);
		for (CTimeSeries *p = chain; p; p = p->chain)
		{
//			p->parg = popt;
			tp = (p->*fp)(0, 0, popt);
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
	{
		SetComplex(); sec->SetComplex();
	}
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
				if (type == '*')
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
		for (p = this; p; (p = p->chain) && (sec->nSamples > 0))
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
		if (sec->GetType() == CSIG_AUDIO)
			SetFs(sec->fs);
		return;
	}

	vector<double> thistmark;
	vector<double> sectmark;
	for (p = this; p; p = p->chain)
	{
		thistmark.push_back(p->tmark);
		thistmark.push_back(p->tmark + p->_dur());
	}
	for (p = sec; p; p = p->chain)
	{
		sectmark.push_back(p->tmark);
		sectmark.push_back(p->tmark + p->_dur());
	}
	unsigned int k, im(0), is(0); // im: index for master (this), is: index for sec
	double anc1, anc0;
	short status(0);
	vector<double> anc; // just for debugging/tracking purposes... OK to delete.
	unsigned int count;
	anc0 = min(thistmark[im], sectmark[is]);
	anc.push_back(anc0);
	CTimeSeries out(sec->GetFs()), part(sec->GetFs());
	if (IsComplex() || sec->IsComplex()) part.SetComplex();
	do {
		if (im == thistmark.size() && is == sectmark.size())
			break;
		if (is == sectmark.size()) //only thistmark is left
			status += (short)(pow(-1., (int)im)), im++;
		else if (im == thistmark.size()) //only sectmark is left
			status += (short)(2 * pow(-1., (int)is)), is++;
		else if (thistmark[im] <= sectmark[is]) // both are available. check them
			status += (short)(pow(-1., (int)im)), im++;
		else
			status += (short)(2 * pow(-1., (int)is)), is++;
		if (im == thistmark.size() && is == sectmark.size())
			break;
		else if (im < thistmark.size() && is < sectmark.size())
			anc1 = min(thistmark[im], sectmark[is]);
		else // this means only one of the two, thistmark and sectmark, is left, just pick the available one.
			if (sectmark.size() - is > 0)
				anc1 = sectmark[is];
			else
				anc1 = thistmark[im];
		anc.push_back(anc1);

		if (status > 0) // if status is 0, skip this cycle--no need to add chain
		{
			CTimeSeries *pm = this;
			CTimeSeries *ps = sec;
			// status indicates in the time range between the last two values of anc, what signal is present
			// 1 means this, 2 means sec, 3 means both
			count = (unsigned int)round((anc1 - anc0) / 1000.*fs);
			unsigned int blockSize = bufBlockSize / sizeof(double);
			{
				part.UpdateBuffer(count);
				part.tmark = anc0;
				part.nGroups = nGroups;
				if (status & 1)
				{
					for (k = 0, p = pm; k < im / 2; k++/*, p=p->chain*/)
					{
						if (p->chain != NULL) p = p->chain;
						else				break;
					}
					int id = (int)round((anc0 - p->tmark) / 1000.*fs);
					memcpy(part.buf, p->logbuf + id * bufBlockSize, part.nSamples*bufBlockSize);
				}
				if (status & 2)
				{
					for (k = 0, p = ps; k < is / 2; k++)
						if (p->chain != NULL) p = p->chain;
					int id = (int)round((anc0 - p->tmark) / 1000.*fs);
					if (status & 1) // type=='+' add the two,  type=='*' multiply
					{
						if (type == '+')
						{
							if (part.IsComplex()) for (unsigned int k = 0; k < count; k++)	part.cbuf[k] += p->cbuf[k + id];
							else				 for (unsigned int k = 0; k < count; k++)	part.buf[k] += p->buf[k + id];
						}
						else if (type == '*')
						{
							if (part.IsComplex()) for (unsigned int k = 0; k < count; k++)	part.cbuf[k] *= p->cbuf[k + id];
							else				 for (unsigned int k = 0; k < count; k++)	part.buf[k] *= p->buf[k + id];
						}
					}
					else
					{
						memcpy(part.buf, p->logbuf + id * bufBlockSize, part.nSamples*bufBlockSize);
					}
				}
			}
			if (out.tmark == 0. && out.nSamples == 0) // the very first time
				out = part;
			else if (count > 0 && part.nSamples > 0) // if count==0, don't add part (which might be leftover from the previous round)
				out.AddChain(part);
		}
		anc0 = anc1;
	} while (im < thistmark.size() || is < sectmark.size());
	*this = out.ConnectChains();
}

CTimeSeries& CTimeSeries::ConnectChains()
{
	CTimeSeries *p(this), out(fs);
	if (p == NULL || p->chain == NULL) return *this;
	if (tmark > 0)
	{
		double shift(tmark);
		for (; p; p = p->chain)	p->tmark -= shift;
		ConnectChains();
		p = this;
		for (; p; p = p->chain)	p->tmark += shift;
		return *this;
	}
	while (1)
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
			if (tp->chain != NULL && fabs(tp->chain->tmark / 1000.*fs - count) <= 2)
				tp = tp->chain;
			else
				break;
		}
		// Consolidate 
		part.UpdateBuffer(count);
		int offset(0);
		while (1) {
			memcpy(part.logbuf + offset, p->buf, p->nSamples*bufBlockSize);
			offset += p->nSamples*bufBlockSize;
			if (p->chain != NULL && offset == (int)(p->chain->tmark / 1000.*fs + .1)*bufBlockSize)
				p = p->chain;
			else
				break;
		}
		if (out.tmark == 0. && out.nSamples == 0) // the very first time
			out = part;
		else
			out.AddChain(part);
		if (p->chain == NULL) break;
		p = p->chain;
	}
	return (*this = out);
}

CSignal& CSignal::Diff(int order)
{
	unsigned int q, len = Len();
	for (unsigned int p = 0; p < nGroups; p++)
	{
		q = p * (len - order);
		for (; q < (p + 1)*(len - order); q++)
			buf[q] = buf[q + order + p] - buf[q + p];
	}
	if (nGroups > 1)
		UpdateBuffer(nSamples - (len - 1)*order);
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
		for (; q < (p + 1)*len; q++)
			buf[q] = buf[q - 1] + buf[q];
	}
	return *this;
}

CTimeSeries& CTimeSeries::operator/=(CTimeSeries &scaleArray)
{
	operate(scaleArray, '/');
	return *this;
}

CSignal& CSignal::reciprocal(void)
{
	for (unsigned int k = 0; k < nSamples; k++)
		buf[k] = 1.0 / buf[k];
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
		auto remainingDur = quantizetmark(endt() + delta, fs);
		if (remainingDur<=0.)
		{ // nothing left
			Reset(1); // needs to specify 1 to make it NULL with fs=1
			return *this;
		}
		// at this point, pts2remove cannot exceed nSamples
		auto pts2remove = (unsigned int)(nSamples - remainingDur / 1000. * fs + .5);
		tmark = 0;
		nSamples -= pts2remove;
		memmove(logbuf, logbuf + pts2remove * bufBlockSize, nSamples * bufBlockSize);
	}
	else
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
	return *this;
}

CTimeSeries& CTimeSeries::AddChain(const CTimeSeries &sec)
{ // MAKE SURE sec is not empty
	if (nSamples == 0)
		return *this = sec;
	if (chain == NULL)
		return *(chain = new CTimeSeries(sec));
	else
		return chain->AddChain(sec);
}

CTimeSeries * CTimeSeries::ExtractDeepestChain(CTimeSeries *dummy)
{
	// breaks and returns the deepest chain. Without dummy it would try to return local variable
	if (chain == NULL) return this;
	else if (chain->chain == NULL) return BreakChain(dummy);
	else return chain->ExtractDeepestChain(dummy);
}

CTimeSeries * CTimeSeries::BreakChain(CTimeSeries *dummy)
{
	// returns the broken chain
	if (chain == NULL) return NULL;
	dummy = chain;
	chain = NULL;
	return dummy;
}

CTimeSeries * CTimeSeries::GetDeepestChain()
{
	CTimeSeries *p(this);
	for (; p->chain; p = p->chain)
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

unsigned int CTimeSeries::CountChains(unsigned int *maxlength) const
{
	// verify again all involved with this 9/18/2018
	unsigned int maxy(-1), res(1);
	for (const CTimeSeries* p = this; p->chain; p = p->chain, res++)
		maxy = max(p->nSamples, maxy);
	if (maxlength) *maxlength = maxy;
	return res;
}

double CSignals::alldur() const
{
	double out = CTimeSeries::alldur();
	if (next)
		out = max(out, next->alldur());
	return out;
}

double CTimeSeries::alldur() const
{
	double out;
	for (CTimeSeries *p = (CTimeSeries *)this; p; p = p->chain)
		out = p->CSignal::endt();
	return out;
}

double CTimeSeries::MakeChainless()
{ //This converts the null intervals of the signal to zero.
	fs = max(fs, 1);
	double newdur = alldur();	// doing this here as fs might have changed.
	if (!tmark && !chain)	// already chainless && no padding required.
		return newdur;

	CTimeSeries nsig(fs);
	nsig.UpdateBuffer((unsigned int)round(newdur / 1000.*fs));
	for (CTimeSeries *p = this; p; p = p->chain) {
		if (p->tmark + p->_dur() <= 0)
			continue;
		unsigned int iorg = (unsigned int)((p->tmark < 0) ? round(-p->tmark / 1000.*fs) : 0);
		unsigned int inew = (unsigned int)((p->tmark > 0) ? round(p->tmark / 1000.*fs) : 0);
		unsigned int cplen = p->nSamples - iorg;
		if (inew + cplen > nsig.nSamples) {
			if (p->chain == NULL && inew + cplen == nsig.nSamples + 1 && newdur / 1000.*fs - nSamples > 0.499999)	// last chain, only 1 sample difference, possile rounding error.
				nsig.UpdateBuffer(nsig.nSamples + 1);
			else
				throw "Internal error: Buffer overrun detected.";
		}
		memcpy(nsig.buf + inew, p->buf + iorg, cplen * sizeof(*buf));
	}
	SwapContents1node(nsig);	// *this gets the chainless signal, nsig gets the chained signal and will be destroyed soon.
	return newdur;
}

CSignal& CSignal::Interp(const CSignal& gains, const CSignal& tmarks)
{
	if (gains.nSamples != tmarks.nSamples)
		throw "The length of both arguments of interp( ) must be the same.";
	int id1(0), id2;
	double gain1(0.), slope;
	int newSampleCount = (int)round(tmarks.buf[tmarks.nSamples - 1] / 1000.*fs);
	Reset();
	UpdateBuffer(newSampleCount);
	for (unsigned int i = 0; i < tmarks.nSamples; i++)
	{
		id2 = (int)round(tmarks.buf[i] / 1000.*fs);
		slope = (gains.buf[i] - gain1) / (id2 - id1);
		for (int j = id1; j < id2; j++)
			buf[j] = slope * (j - id1) + gain1;
		gain1 = gains.buf[i];
		id1 = id2;
	}
	return *this;
}

CTimeSeries& CTimeSeries::Squeeze()
{
	int nSamplesTotal(0), nSamples0(nSamples);
	for (CTimeSeries* p(this); p; p = p->chain)
		nSamplesTotal += p->nSamples;
	UpdateBuffer(nSamplesTotal);
	nSamplesTotal = nSamples0;
	for (CTimeSeries* p(chain); p; p = p->chain)
		memcpy(&buf[nSamplesTotal], p->buf, p->nSamples * sizeof(p->buf[0])), nSamplesTotal += p->nSamples;
	delete chain;
	chain = NULL;
	return *this;
}

CTimeSeries& CTimeSeries::MergeChains()
{// This tidy things up by removing unnecessary chains and rearranging them.
	CTimeSeries temp;
	if (nSamples == 0 && chain) { temp = *chain; chain = NULL; *this = temp; }

	for (CTimeSeries* p(this); p && p->chain; p = p->chain)
	{
		double et = p->tmark + p->_dur();
		if (et >= p->chain->tmark) // consolidate 
		{
			temp = *p->chain;
			int id1 = (int)round((temp.tmark - p->tmark) / 1000.*fs);
			int common = p->nSamples - id1;
			int id2 = temp.nSamples - common;
			int oldnSamples(p->nSamples);
			p->UpdateBuffer(p->nSamples + temp.nSamples - common);
			/* overlapping portion */
			for (int i = 0; i < common; i++)
				p->buf[oldnSamples - common + i] += temp.buf[i];
			for (unsigned int k = 0; k < temp.nSamples - common; k++)
				p->buf[oldnSamples + k] = temp.buf[common + k];
			if (temp.chain == NULL)	p->chain = NULL;
			else *p->chain = *temp.chain; // deep copy is necessary (we are losing p).
		}
	}
	return *this;
}

CTimeSeries& CTimeSeries::LogOp(CTimeSeries &rhs, int type)
{
	for (CTimeSeries *p = this; p; p = p->chain)
		p->body::LogOp(rhs, type);
	return *this;
}

CTimeSeries& CTimeSeries::removeafter(double timems)
{ // if timems is in the grid, the point is removed (but dur will be until that grid point)
	CTimeSeries *last = NULL;
	for (CTimeSeries *p(this); p; p = p->chain)
	{
		if (timems > p->tmark + p->_dur()) { last = p; continue; }
		else if (timems > p->tmark)
		{
			// no need to worry about p->tmark
			// all points occuring on and after timems will be removed (dur will still be timems)
			// if timems exceeds the grid of p->tmark, it won't be removed, so it is ceil.
			p->nSamples = mceil((timems - p->tmark)*fs / 1000.);
		}
		else if (last)
		{
			if (!ghost)	delete[] p;
			last->chain = NULL;
			break;
		}
		else
		{ // if p->tmark is less than timems AND last is NULL, which means the very first p happening after timems
			Reset(fs);
			break;
		}
		last = p;
	}
	return *this;
}

CTimeSeries& CTimeSeries::timeshift(double timems)
{ // if timems is in the grid, the point is kept.
	int chainlevel(0);
	CTimeSeries *p(this);
	for (; p; p = p->chain)
	{
		if (timems > p->tmark + p->_dur()) { chainlevel++; continue; }
		if (timems < p->tmark)
			p->tmark -= timems;
		else
		{
			// p->tmark should be zero, except for the small off-granular timing based on the difference between timems and p->tmark
			// the decrease of points will step-up if timems exceeds the grid, so it is ceil.
			int pointsless = mceil((timems - p->tmark)*fs / 1000.);
			if (pointsless > 0)
			{
				p->nSamples -= pointsless;
				if (!ghost)
				{
					bool *tbuf = new bool[p->nSamples*bufBlockSize];
					memcpy(tbuf, p->buf + pointsless, p->nSamples*bufBlockSize);
					delete[] p->logbuf;
					p->logbuf = tbuf;
				}
				else
					p->buf += pointsless;
				p->tmark = 0;
			}
		}
	}
	//all chains at and prior to chainlevel are cleared here.
	p = this;
	for (int k(0); k < chainlevel; k++, p = p->chain)
		if (!ghost) delete[] p->buf;
	if (p != this) // or if chainlevel is non-zero 
	{
		// Make new chain after chainlevel. If p is NULL (make an empty CTimeSeries object to return);
		if (!p) { p = new CTimeSeries; }
		nSamples = p->nSamples;
		tmark = p->tmark;
		buf = p->buf;
		chain = NULL;
	}
	return *this;
}

CTimeSeries& CTimeSeries::Crop(double begin_ms, double end_ms)
{
	if (begin_ms == end_ms) { Reset(); return *this; }
	vector<double> tmarks;
	if (begin_ms > end_ms) {
		Crop(end_ms, begin_ms);
		ReverseTime();
		return *this;
	}
	removeafter(end_ms);
	if (nSamples>0)
		timeshift(begin_ms);
	return *this;
}

#ifndef NO_FFTW

CSignal& CSignal::movespec(unsigned int id0, unsigned int len, void *parg)
{
	if (len == 0) len = nSamples;
	CSignals shift = *(CSignals*)parg;
	CSignals copy(*this);
	Hilbert();
	double t(0), grid(1. / fs);
	const complex<double> j(0.0, 1.0);
	complex<double> datum;
	double val = shift.value();
	for (unsigned int k = 0; k < nSamples; k++)
	{
		datum = (copy.buf[k] + buf[k] * j) * exp(j * val *  2. * PI * t);
		buf[k] = real(datum);
		t += grid;
	}
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
	double samplegrid = 1. / fs;
	double deviationfromgrid = t1 - ((double)(int)(t1*fs)) / fs;
	bool inbet(false); // true means t1 is in between chains.
	for (p = this; p && !inbet; p = p->chain)
	{
		if (t1 > p->tmark + p->_dur() && p->chain && t1 < p->chain->tmark)
			inbet = true;
	}
	if (t1 > 0. && fabs(deviationfromgrid) > 1.e-8 && deviationfromgrid > -samplegrid && deviationfromgrid < samplegrid)
		t1 -= 1000.*samplegrid;  // because its in ms
	Crop(0, t1);
	if (inbet)
		newsig >>= t1 - (tmark + _dur());
	if (newsig.chain) { delete newsig.chain; newsig.chain = NULL; }
	*this += &newsig;
	//if t2 coincides with the sampling grid, don't take that point here (it will be taken twice)
	deviationfromgrid = t2 - ((double)(int)(t2*fs)) / fs;
	// deviationfromgrid of zero can masquerade as a very small number
	if (fabs(deviationfromgrid) > 1.e-8 && deviationfromgrid > -samplegrid && deviationfromgrid < samplegrid)
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
		int count = (int)((tp - p->tmark) * fs / 1000 + .5);
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
	int id;
	// Be careful, this is not a virtual function. So, when called from CSignals, this is a pointer to CSignal
	// casting won't work---i.e., you cannot declare it like this---CSignal *p((CSignal*)this)
	CTimeSeries copy(*this), out(*this);
	CTimeSeries *p(this);
	double insertduration = newchunk.GetDeepestChain()->_dur();
	// Alert---IsAudioOnAt (formerly known as IsNull) was modified... check if the change didn't mess up Insert()
	// 3/10/2019
	if (IsAudioOnAt(timept))
	{
		for (; p; p = p->chain)
		{
			if (timept > p->tmark + p->_dur() && timept < p->chain->tmark)
				break;
		}
		// The line below looks precarious but when IsNull is true
		// if timept > p->endt(), then it has at least one p->chain available
		// so let's not worry about p->chain being NULL
	//at this point, p->chain and all forward need to time-shift right.
		p = p->chain;
		*p >>= insertduration;

		*this = copy;
		//Now, *this is prepared (the new signal buffer can be inserted)
		for (p = &newchunk; p; p = p->chain)
			p->tmark += timept;
		operate(newchunk, '+');
		return *this;
	}
	else
	{
		for (bool done(0); p; p = p->chain)
		{
			if (!done && timept >= p->tmark && timept <= p->tmark + p->_dur())
			{
				CTimeSeries *orgchain(chain);
				CTimeSeries *trailingchain = new CTimeSeries(fs);
				id = (int)((timept - p->tmark) * fs / 1000. + .5);
				int lenTrailingchain(p->nSamples - id);
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
	int id1 = (int)round(time_ms1 / 1000.*fs);
	int id2 = (int)round(time_ms2 / 1000.*fs) - 1;
	return Truncate(id1, id2);
}

double * CSignal::Truncate(int id1, int id2, int code)
{
	if (nSamples == 0) throw "internal error: Truncate()";
	id1 = max(0, id1);
	if (id1 > id2) {
		nSamples = 0;
		return buf;
	}
	if (code)
	{
		// This will not work, if the signal has null portion in the middle and id1 and id2 cover beyond the null portion... 7/15/2016 bjk
		if (tmark > 0)
		{
			CSignal copy(*this);
			double _tmark(copy.tmark);
			copy.tmark = 0;
			id1 -= (int)(_tmark / 1000.*fs + .5);
			id2 -= (int)(_tmark / 1000.*fs + .5);
			copy.Truncate(id1, id2);
			copy.tmark = _tmark;
			*this = copy;
			return buf;
		}
	}
	if (id2 > (int)nSamples - 1) id2 = (int)nSamples - 1;
	nSamples = max(0, id2 - id1 + 1);
	memmove(buf, logbuf + id1 * bufBlockSize, nSamples*bufBlockSize);
	return buf;
}

bool CTimeSeries::IsAudioOnAt(double timept)
{
	if (GetType() != CSIG_AUDIO) return false;
	CTimeSeries *p(this);
	for (; p; p = p->chain)
	{
		if (timept < p->tmark)
			return false;
		if (timept >= p->tmark && timept <= p->tmark + p->dur())
			return true;
	}
	return false;
}

int CSignal::GetType() const
{
	if (nSamples == 0) // empty
	{
		if (tmark == 0.)
			return CSIG_EMPTY;
		else
			return CSIG_NULL;
	}
	else if (fs == 2) // string
		return CSIG_STRING;
	else if (nSamples == 1)
		return CSIG_SCALAR;
	else if (fs > 2) // audio
		return CSIG_AUDIO;
	else
		return CSIG_VECTOR;
}

bool CSignal::IsSingle() const
{
	int type = GetType();
	if (nSamples != 1) return false;
	return (type == CSIG_SCALAR || type == CSIG_STRING || type == CSIG_AUDIO || type == CSIG_TSERIES);
}

int CTimeSeries::GetType() const
{
	// as of 6/12/2018 tseq only supports with scalars
	// still as of 6/20/2018 
	if (tmark > 0 && nSamples == 1) return CSIG_TSERIES;
	if (chain && nSamples == 1)	return CSIG_TSERIES;
	return CSignal::GetType();
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
#ifndef NO_RESAMPLE
static int getSIH(int len, double r1, double r2, int *outlength)
{
	double _r1 = 1. / r1;
	double _r2 = 1. / r2;
	double ratio_mean = 2. / (1. / _r1 + 1. / _r2);
	int N = (int)round(len*ratio_mean);
	*outlength = N;
	double sum = 0;
	double ratio;
	for (int k = 0; k < N; k++)
	{
		ratio = _r1 + (_r2 - _r1)*k / N;
		sum += 1. / ratio;
	}
	double  leftover = len - sum;
	*outlength += (int)round(leftover * ratio);
	return (int)round(leftover);
}

CSignal& CSignal::resample(unsigned int id0, unsigned int len, void *parg)
{
	//This doesn't mean real "resampling" because this does not change fs.
	//pratio < 1 means generate more samples (interpolation)-->longer duration and lower pitch
	//pratio > 1 means downsample--> shorter duration and higher pitch
	//On return, pratio is set with the actual ratio (incoming size / result size) for each chain
	if (len == 0) len = nSamples;
	CSignals *pratio = (CSignals *)parg;
	char errstr[256] = {};
	SRC_DATA conv;
	float *data_out, *data_in = new float[nSamples];
	int errcode;
	SRC_STATE* handle = src_new(SRC_SINC_MEDIUM_QUALITY, 1, &errcode);
	if (errcode)
	{
		pratio->SetString(src_strerror(errcode));
		return *this;
	}
	for (unsigned int k = 0; k < nSamples; k++) data_in[k] = (float)buf[k];
	conv.data_in = data_in;
	if (pratio->GetType() != CSIG_TSERIES)
	{
		conv.src_ratio = 1. / pratio->value();
		conv.input_frames = nSamples;
		conv.output_frames = (long)(nSamples * conv.src_ratio + .5);
		conv.data_out = data_out = new float[conv.output_frames];
		conv.end_of_input = 1;
		errcode = src_process(handle, &conv);
		if (errcode)
		{
			pratio->SetString(src_strerror(errcode));
			delete[] data_in;	delete[] data_out;
			return *this;
		}
		UpdateBuffer(conv.output_frames);
		long k;
		for (k = 0; k < conv.output_frames_gen; k++)
			buf[k] = conv.data_out[k];
		for (k = conv.output_frames_gen; k < conv.output_frames; k++)
			buf[k] = 0;
	}
	else
	{
		int blockCount = 0;
		vector<double> outbuffer;
		//inspect pratio to estimate the output length
		int cum = 0, cumID = 0;
		for (CTimeSeries *p = pratio; p && p->chain; p = p->chain)
			cum += (int)((p->chain->tmark - p->tmark) * fs / 1000 * p->value());
		outbuffer.reserve(cum);
		int lastSize = 1, lastPt = 0;
		data_out = new float[lastSize];
		long inputSamplesLeft = (long)nSamples;
		int orgSampleCounts = 0;
		//assume that pratio time sequence is well prepared--
		for (CTimeSeries *p = pratio; p && p->chain; p = p->chain)
		{
			conv.end_of_input = 0;
			unsigned int i1, i2;
			double ratio_mean;
			int inBuffersize, outBuffersize;
			if (p->value() == p->chain->value())
				src_set_ratio(handle, conv.src_ratio = ratio_mean = 1. / p->value());
			else
			{
				src_set_ratio(handle, 1. / p->value());
				conv.src_ratio = 1. / p->chain->value();
				ratio_mean = (2 * 1. / p->value()*1. / p->chain->value() / (1. / p->value() + 1. / p->chain->value())); // harmonic mean
			}
			//current p covers from p->tmark to p->chain->tmark
			if (!p->chain->chain)
				conv.input_frames = inputSamplesLeft;
			else
			{
				//current p covers from p->tmark to p->chain->tmark
				i1 = (int)(p->tmark * fs / 1000);
				i2 = (int)(p->chain->tmark * fs / 1000);
				conv.input_frames = i2 - i1;
			}
			conv.output_frames = (long)(conv.input_frames * ratio_mean + .5); // when the begining and ending ratio is different, use the harmonic mean for the estimate.
			if (conv.output_frames > lastSize)
			{
				delete[] data_out;
				data_out = new float[lastSize = conv.output_frames + 20000];//reserve the buffer size big enough to avoid memory crash, but find out a better than this.... 3/20/2019
			}
			conv.data_out = data_out;
			int harmean;
			int out2 = getSIH(conv.input_frames, p->value(), p->chain->value(), &harmean);
			int harmean0 = harmean;
			int newlen = harmean + out2 / 2;
			errcode = src_process(handle, &conv);
			inBuffersize = conv.input_frames_used;
			if ( errcode)
			{
				std::string errout;
				sformat(errout, "Error in block %d--%s", blockCount++, src_strerror(errcode));
				pratio->SetString(errout.c_str());
				delete[] data_in;	delete[] data_out;
				return *this;
			}
			outBuffersize = conv.output_frames_gen;
			for (int k = 0; k < conv.output_frames_gen; k++)
				outbuffer.push_back(data_out[k]);
			lastPt += conv.input_frames_used;
			if (p->chain->chain)
			{
				conv.data_in = &data_in[lastPt];
				inputSamplesLeft -= conv.input_frames_used;
			}
			while (conv.input_frames)
			{
				conv.src_ratio = 1. / p->chain->value();
				conv.data_in = &data_in[lastPt];
				conv.input_frames -= conv.input_frames_used;
				conv.end_of_input = conv.input_frames == 0 ? 1 : 0;
				errcode = src_process(handle, &conv);
				inBuffersize += conv.input_frames_used;
				outBuffersize += conv.output_frames_gen;
				for (int k = 0; k < conv.output_frames_gen; k++)
					outbuffer.push_back(data_out[k]);
				lastPt += conv.input_frames_used;
			}
			src_reset(handle);
			p->chain->tmark = p->tmark + 1000. / fs * outBuffersize;
		}
		UpdateBuffer((unsigned int)outbuffer.size());
		memcpy(buf, &outbuffer[0], sizeof(double)*outbuffer.size());
	}
	src_delete(handle);
	delete[] data_in;
	delete[] data_out;
	return *this;
}
#endif //NO_RESAMPLE

void CTimeSeries::UpSample(int cc)
{
	for (CTimeSeries *p = this; p; p = p->chain)
		p->CSignal::UpSample(cc);
}

void CSignal::UpSample(int cc)
{
	if (cc == 1) return;
	int newLength = nSamples * cc;
	CSignal temp(fs*cc);
	temp.UpdateBuffer(newLength);
	memset((void*)temp.buf, 0, sizeof(double)*newLength);
	for (unsigned int i = 0; i < nSamples; i++)
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
	if (cc == 1) return;
	unsigned int newLength = (unsigned int)ceil((double)nSamples / cc);
	CSignal temp(fs / cc);
	temp.UpdateBuffer(newLength);
	unsigned int k;
	for (k = 0; cc*k < nSamples; k++)
		temp.buf[k] = buf[cc*k];
	temp.nSamples = k;
	*this = temp;
}

CSignal& CSignal::conv(unsigned int id0, unsigned int len, void* parg)
{
	if (len == 0) len = nSamples;
	CSignal *parray2 = (CSignal*)parg;
	CSignal out;
	out.UpdateBuffer(nSamples + parray2->nSamples - 1);
	for (unsigned int k = 0; k < out.nSamples; k++)
	{
		double tp = 0.;
		for (int p(0), q(0); p < (int)nSamples; p++)
		{
			if ((q = k - p) < 0) continue;
			if (p < (int)nSamples && q < (int)parray2->nSamples)
				tp += buf[p] * parray2->buf[q];
		}
		out.buf[k] = tp;
	}
	return *this = out;
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
	for (unsigned int i = 0; i < nSamples; i++)
		tempBuf[nSamples - i - 1] = buf[i];
	*this = temp;
}

std::string CSignal::string() const
{
	unsigned int k;
	std::string out;
	out.resize(nSamples);
	for (k = 0; k < nSamples && strbuf[k]; k++)
		out[k] = *(strbuf + k);
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
	UpdateBuffer((int)strlen(str) + 1);
	strcpy(strbuf, str);
	return *this;
}

CSignal &CSignal::SetString(const char c)
{
	Reset(2);
	bufBlockSize = 1;
	if (c == 0) return *this;
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
			for (unsigned int k(0); k < p->nSamples; k++)
			{
				if (p->buf[k] < 0)
					p->buf[k] = -fn(-p->buf[k]);
				else
					p->buf[k] = fn(p->buf[k]);
			}
	}
	else
		throw "each()--only for vector/scalar or audio";
	return *this;
}

CTimeSeries& CTimeSeries::each(double(*fn)(double))
{
	if (GetType() == CSIG_VECTOR || GetType() == CSIG_SCALAR)
		body::each(fn);
	else if (GetType() == CSIG_AUDIO || GetType() == CSIG_TSERIES)
	{
		for (CTimeSeries *p = this; p; p = p->chain)
			for (unsigned int k(0); k < p->nSamples; k++)
				p->buf[k] = fn(p->buf[k]);
	}
	else
		throw "each()--only for vector/scalar or audio";
	return *this;
}

CTimeSeries& CTimeSeries::GhostCopy(CTimeSeries *pref)
{ // This should be cleaned off. Just use the <= operator   10/5/2019
	vector<bool> ghostHistory;
	for (CTimeSeries *p = pref; p; p = p->chain)
	{
		ghostHistory.push_back(p->ghost);
		p->ghost = true;
	}
	*this = *pref;
	auto it = ghostHistory.begin();
	for (CTimeSeries *p = pref; p; p = p->chain)
	{
		p->ghost = *it;
		it++;
	}
	return *this;
}

CTimeSeries & CTimeSeries::MFFN(double(*fn)(double), complex<double>(*cfn)(complex<double>))
{
	if (nSamples == 0) return *this;
	if (GetType() == CSIG_VECTOR || GetType() == CSIG_SCALAR)
		body::mathFunForbidNegative(fn, cfn);
	else if (GetType() == CSIG_AUDIO)
	{
		for (CTimeSeries *p = this; p; p = p->chain)
			for (unsigned int k(0); k < p->nSamples; k++)
			{
				if (p->buf[k] < 0)
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
	if ((GetType() == CSIG_VECTOR || GetType() == CSIG_SCALAR) && (param.GetType() == CSIG_VECTOR || param.GetType() == CSIG_SCALAR))
		body::mathFunForbidNegative(fn, cfn, param);
	else if (GetType() == CSIG_AUDIO)
	{
		if (param.GetType() == CSIG_AUDIO) throw "The operation cannot perform with both audio operands.";
		if (param.GetType() == CSIG_SCALAR || (param.nSamples == 2 && param.buf[0] == param.buf[1]))
		{
			for (CTimeSeries *p = this; p; p = p->chain)
				for (unsigned int k(0); k < p->nSamples; k++)
				{
					if (p->buf[k] < 0)
						p->buf[k] = -fn(-p->buf[k], param.buf[0]);
					else
						p->buf[k] = fn(p->buf[k], param.buf[0]);
				}
		}
		else if (param.GetType() == CSIG_VECTOR)
		{
			if (param.nSamples > 2) throw "The interpolated operand feature has not been implemented in this version. Check back.";
			for (CTimeSeries *p = this; p; p = p->chain)
				for (unsigned int k(0); k < p->nSamples; k++)
				{
					double diff = param.buf[1] - param.buf[0];
					double inc = diff / p->nSamples;
					double val = param.buf[0] + inc * k;
					if (p->buf[k] < 0)
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


CTimeSeries& CTimeSeries::each(complex<double>(*fn)(complex<double>))
{
	if (IsComplex())
		body::each(fn);
	else
		throw "each()--expecting complex number";
	return *this;
}

CTimeSeries& CTimeSeries::each(double(*fn)(complex<double>))
{
	if (IsComplex())
		body::each(fn);
	else
		throw "each()--expecting complex number";
	return *this;
}

CTimeSeries& CTimeSeries::each(double(*fn)(double, double), body &arg2)
{
	if (GetType() == CSIG_VECTOR || GetType() == CSIG_SCALAR)
		body::each(fn, arg2);
	else if (GetType() == CSIG_AUDIO)
	{
		if (arg2.nSamples == 1)
		{
			double val = arg2.value();
			for (CTimeSeries *p = this; p; p = p->chain)
				for (unsigned int k(0); k < p->nSamples; k++)
					p->buf[k] = fn(val, p->buf[k]);
		}
		else
		{
			for (CTimeSeries *p = this; p; p = p->chain)
				for (unsigned int i = 0; i < min(nSamples, arg2.nSamples); ++i)
					p->buf[i] = fn(p->buf[i], arg2.buf[i]);
		}
	}
	else
		throw "each()--only for vector/scalar or audio";
	return *this;
}

CTimeSeries& CTimeSeries::each(complex<double>(*fn)(complex<double>, complex<double>), body &arg2)
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
	res = fwrite((void*)&fs, sizeof(fs), 1, fp);
	res = fwrite((void*)&nChains, sizeof(nChains), 1, fp);
	for (unsigned int k = 0; k < nChains; k++, p = p->chain)
	{
		res = fwrite((void*)&p->nSamples, sizeof(nSamples), 1, fp);
		res = fwrite((void*)&p->tmark, sizeof(tmark), 1, fp);
		res = fwrite((void*)p->buf, p->bufBlockSize, p->nSamples, fp);
	}
	return (int)res;
}

int CTimeSeries::IsTimeSignal() const
{ // Assume that CSignal and next have the same type, except for null or empty
	if (fs == 1 || fs == 2) return 0;
	return 1;
	//	if (GetType() == CSIG_AUDIO || GetType() == CSIG_TSERIES) return 1;
}

CSignals::CSignals()
	: next(NULL)
{
}

CSignals::CSignals(bool *b, unsigned int len)
	: next(NULL)
{
	Reset(1);
	bufBlockSize = 1; // logical array
	UpdateBuffer(len);
	bool bv = *b;
	for_each(logbuf, logbuf + len, [bv](bool v) { v = bv; });
}

CSignals::CSignals(int sampleRate)
	:next(NULL)
{
	SetFs(max(sampleRate, 0));
}

CSignals::CSignals(double value)
	: next(NULL)
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
	: next(NULL)
{
	*this = src;
}

CSignals::CSignals(double* y, int len)
	: next(NULL)
{
	SetFs(1);
	UpdateBuffer(len);
	memcpy(buf, y, sizeof(double) * len);
}

CSignals::CSignals(complex<double> *y, int len)
	: next(NULL)
{
	SetFs(1);
	SetComplex();
	UpdateBuffer(len);
	memcpy(buf, y, sizeof(double) * len);
}

CSignals::~CSignals()
{
	if (ghost) {
		next = nullptr; buf = nullptr; }
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

void CSignals::SetNextChan(CSignals *second, bool need2makeghost)
{
	if (next == second) return;
	if (second && fs != second->GetFs() && second->nSamples > 0 && nSamples > 0)
	{
		// tseq and scalar can be mixed--i.e., if neither of this nor second is tseq throw
		if (!(type()&TYPEBIT_TSEQ) && !(second->type()&TYPEBIT_TSEQ))
		{
			char errstr[256];
			sprintf(errstr, "SetNextChan attempted on different fs: fs1=%d, fs2=%d", GetFs(), second->GetFs());
			throw errstr;
		}
	}
	if (next) {
//		if (!next->ghost) 
			delete next;
		next = NULL;
	}
	if (second) {
		next = new CSignals;
		if (need2makeghost)
			*next <= *second;
		else
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
		if (!next->ghost)
			delete next;
		next = NULL;
	}
	return *this;
}

CSignals CSignals::fp_getval(double (CSignal::*fp)(unsigned int, unsigned int, void *) const, void *popt)
{
	CSignals newout = CTimeSeries::fp_getval(fp, popt);
	if (next)
		newout.SetNextChan(&next->fp_getval(fp, popt));
	return newout;
}

CSignals& CSignals::fp_mod(CSignal& (CSignal::*fp)(unsigned int, unsigned int, void*), void * popt)
{
	CTimeSeries::fp_mod(fp, popt);
	if (next)
		next->fp_mod(fp, popt);
	return *this;
}

CSignals CSignals::fp_getsig(CSignal(CSignal::*fp)(unsigned int, unsigned int, void*) const, void * popt)
{
	CSignals newout = CTimeSeries::fp_getsig(fp, popt);
	if (next)
		newout.SetNextChan(&next->fp_getsig(fp, popt));
//	parg = nullptr;
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


CSignals& CSignals::operator+=(double con)
{
	operate(CSignals(con), '+');
	return *this ;
}
CSignals& CSignals::operator*=(double con)
{
	operate(CSignals(con), '*');
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

int CSignals::IsTimeSignal() const
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
		cell[id - 1] = sec; //replace existing cell 
	else
	{
		CVar tp;
		for (size_t k = cell.size(); k < id - 1; k++)
			cell.push_back(tp);
		appendcell(sec);
	}
	return *this;
}

CVar& CVar::bringnext()
{
	if (!next) Reset();
	else
	{
		CTimeSeries::Reset();
		*this <= next; // ghost copy
		next = NULL;
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
	if (next)	*next >>= delta;
	return *this;
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

CSignal& CSignal::Modulate(double *env, unsigned int lenEnv, unsigned int beginID)
{
	// Modulate this object with env 
	// If lenEnv is greater than nSamples, then the samples in env after nSamples are ignored.
	// beginID indicates the index of buf to start Modulate
	for (unsigned int k = beginID; k < min(nSamples, lenEnv + beginID); k++)
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
				unsigned int countsSkip = (unsigned int)((qq->tmark - t1) * fs / 1000.);
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
	for (unsigned int k(0); k < nSamples; k++)
		out.buf[k] = sqrt(buf[2 * k] * buf[2 * k] + buf[2 * k + 1] * buf[2 * k + 1]);
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
	for (unsigned int i = 0; i < nSamples; i++)
		buf[i] = atan(x2[i] / x1[i]);
	return *this;
}

void CSignals::DownSample(int cc)
{
	CTimeSeries::DownSample(cc);
	if (next != NULL) next->DownSample(cc);
}

void CSignals::UpSample(int cc)
{
	CTimeSeries::UpSample(cc);
	if (next != NULL) next->UpSample(cc);
}

CSignals &CSignals::transpose1()
{
	CTimeSeries::transpose1();
	if (next)
		next->transpose1();
	return *this;
}

CSignals& CSignals::LogOp(CSignals &rhs, int type)
{
	CTimeSeries::LogOp(rhs, type);
	if (next) next->LogOp(rhs, type);
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
	if (next != NULL)
	{
		next->MakeChainless();
		int diff = nSamples - next->nSamples;
		CSignals silenc(fs);
		if (diff > 0) // next is shorter (next needs padding)
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

CSignals &CSignals::each(double(*fn)(double))
{
	CTimeSeries::each(fn);
	if (next) 	next->each(fn);
	return *this;
}

CSignals &CSignals::each(complex<double>(*fn)(complex<double>))
{
	CTimeSeries::each(fn);
	if (next) 	next->each(fn);
	return *this;
}

CSignals &CSignals::each(double(*fn)(complex<double>))
{
	CTimeSeries::each(fn);
	if (next) 	next->each(fn);
	return *this;
}

CSignals &CSignals::each(double(*fn)(double, double), body &arg2)
{
	CTimeSeries::each(fn, arg2);
	if (next) 	next->each(fn, arg2);
	return *this;
}

CSignals &CSignals::each(complex<double>(*fn)(complex<double>, complex<double>), body &arg2)
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

//No error handling--Don't use this constructor unless you are really sure it can't go wrong
CSignals::CSignals(const char* wavname)
	:next(NULL)
{
	char errstr[256];
	Wavread(wavname, 0, -1, errstr);
}

#endif // NO_SF

#ifdef _WINDOWS

#ifndef NO_PLAYSND

#endif // NO_PLAYSND

#endif

#ifndef NO_FFTW

#endif

int CSignals::GetType() const
{
	// just as an exception, if next exists but both nSamples and next->nSamples are 1,
	// treat it as TSERIES
	if (next && nSamples == 1 && next->nSamples == 1)
		return CSIG_TSERIES;
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
	for (unsigned int k = 0; k < nChains; k++)
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
	bool nullchain = nSamples == 0;
	if (nullchain)
		for (CTimeSeries *p = this; p; p = p->chain)
			if (p->nSamples) throw "T_SERIES NULL not consistent.";
			else out[-p->tmark] = 0;
	else
		for (CTimeSeries *p = this; p; p = p->chain)
			if (p->nSamples == 0) throw "T_SERIES NULL not consistent.";
			else out[p->tmark] = p->value();
	return out;
}


bool CVar::IsGO() const
{ //Is this Graphic Object?
	if (fs == 3) return true;
	if (strut.empty()) return false;
	if (strut.find("type") == strut.end()) return false;
	if (strut.find("color") == strut.end()) return false;
	if (strut.find("userdata") == strut.end()) return false;
	if (strut.find("tag") == strut.end()) return false;
	if (strut.find("visible") == strut.end()) return false;
	//if (strut.find("parent") == strut.end()) return false;
	//if (strut.find("children") == strut.end()) return false;

	// Add rejection for other handles (e.g., audio handle)
	return true;
}

CVar&  CVar::length()
{
	if (IsGO())
	{
		if (fs == 3) SetValue((double)nSamples);
		else SetValue(1.);
	}
	else // checkcheckcheck
		fp_getval(&CSignal::length);
	return *this;
}

bool CVar::IsAudioObj() const
{
	if (strut.empty()) return false;
	auto type = strut.find("type");
	if (type == strut.end()) return false;
	if (!(*type).second.IsString()) return false;
	//The text content may be audio_playback or audio_playback (inactive)
	if ((*type).second.string().find("audio_playback") != 0 && (*type).second.string().find("audio_record") != 0) return false;
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
CVar::CVar(CVar * src)
: functionEvalRes(false)
{
	*this = *src;
}

CVar& CVar::Reset(int fs2set)
{ // calling Reset for a CVar object will erase cell, strut and struts members
	CSignals::Reset(fs2set);
	cell.clear();
	strut.clear();
	struts.clear();
	return *this;
}

int CVar::GetType() const
{
	if (fs == 3)
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

void CVar::set_class_head(const CSignals & rhs)
{
	CSignals::operator=(rhs);
}

int CSignals::getBufferLength(double & lasttp, double & lasttp_with_silence, double blockDur) const
{
	double nullportion = tmark;
	if (nullportion >= blockDur) 
	{
		if (!next || next->tmark >= blockDur)
		{
			if (next && next->tmark < nullportion) nullportion = next->tmark;
			lasttp = lasttp_with_silence = 0.;
			return (int)round(nullportion / 1000. * fs);
		}
	}
	const CSignals *p = this;
	multimap<double, int> timepoints;
	for (int k = 0; p && k < 2; k++, p = (CSignals*)p->next)
	{
		for (const CTimeSeries *q = p; q; q = q->chain)
		{
			if (q->IsEmpty()) continue;
			timepoints.insert(pair<double, int>(q->tmark, 1));
			timepoints.insert(pair<double, int>(q->CSignal::endt(), -1));
		}
	}
	auto it = timepoints.begin();
	for (int sum = 0; it != timepoints.end(); it++)
	{
		sum += it->second;
		if (sum == 0)
		{
//			auto jt = it;
//			if (++jt == timepoints.end())
				break;
		}
	}
	lasttp = it->first;
	lasttp_with_silence = lasttp;
	if (++it != timepoints.end())
		lasttp_with_silence = it->first;
	// If the null portion is long enough, treat it as a separate, blank audio block in the subsequent call
	if (lasttp_with_silence - lasttp >= blockDur)
		lasttp_with_silence = lasttp;
	return (int)round(lasttp_with_silence / 1000. * fs);
}

void CSignals::nextCSignals(double lasttp, double lasttp_with_silence, CSignals &ghcopy)
{
	CSignals * p = &ghcopy;
	CTimeSeries *q, *q1 = NULL;
	CSignals *q2 = NULL;
	for (int k = 0; p && k < 2; k++, p = p->next)
	{
		for (q = p; q; q = q->chain)
		{
			if (q->tmark >= lasttp)
			{
				q->tmark -= lasttp_with_silence;
				CSignals *tempNext = p->next;
				if (k == 0)
				{
					p->next = tempNext;
					q1 = q;
				}
				else
				{
					p->next = nullptr;
					q2 = (CSignals*)q;
				}
				break;
			}
		}
	}
	if (q1)
		ghcopy = *q1;
	if (q2) // what's the point of this? q2, if available, should be the same as next at this point. 5/24/2020
		ghcopy.SetNextChan(q2, true);
	if (!q1 && !q2)
		ghcopy.Reset();
}
