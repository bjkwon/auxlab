// AUXLAB 
//
// Copyright (c) 2009-2019 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 2
// Date: 7/5/2019
// 


#include "aux_classes.h"
#include "caux.h"

#define PI	3.141592

carray::carray()
{
	pdata = NULL;
}
carray::~carray()
{ 
	if (pdata)
	{
		delete pdata; pdata = NULL;
	}
}
//carray::carray(const carray&copy)
//{
	//switch (copy.type)
	//{
	//case real:
	//	buf = copy.buf;
	//	break;
	//case complex:
	//	cbuf = copy.cbuf;
	//	break;
	//case text:
	//	strbuf = copy.strbuf;
	//	break;
	//case logical:
	//	logbuf = copy.logbuf;
	//	break;
	//}
//}

caudio::caudio()
	:fs(-1)
{
	type = real; pdata = new vector<double>; pdata->reserve(1);
}
caudio::caudio(int _fs)
	:fs(_fs)
{
	type = real; pdata = new vector<double>; pdata->reserve(1);
}
caudio::caudio(int _fs, int len)
	: fs(_fs)
{
	type = real; pdata = new vector<double>; pdata->reserve(len);
}
caudio::~caudio()
{
	if (pdata)
	{
		delete pdata; pdata = NULL;
	}
}

caudio & caudio::tone(double freq, double dur_ms, double beginPhase)
{
	size_t nsamples = (size_t) round(dur_ms / 1000.*fs);
	pdata->reserve(nsamples);
	vector<double> out;

	for (unsigned int k = 0; k < nsamples; k++)
	{
		double val = sin(2 * PI*(freq* k / fs + beginPhase));
		out.push_back(val);
		pdata->push_back(val);
	}
	return *this;
}



ctimesig & ctimesig::makechainless()
{ // must be done for audio signals only--check
	//for (auto player : block)
	//{
	//	player.
	//}
	return *this;
}

ctimesig::ctimesig(int _fs)
{  
	fs = _fs;
//	chain = NULL;
}
ctimesig::~ctimesig()
{
	if (pdata)
	{
		delete pdata; pdata = NULL;
	}
	//for (auto &plock : block)
	//	delete plock;
	//if (chain)
	//	delete chain;
}
