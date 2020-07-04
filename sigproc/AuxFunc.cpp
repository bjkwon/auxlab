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
// Date: 5/17/2020
// 
#include <math.h>
#include <stdlib.h>
#include <string.h> // aux_file
#include <time.h>
#include <atlrx.h>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>

#ifdef _WINDOWS
#include <io.h>
#include "bjcommon.h"
#ifndef NO_PLAYSND
#include "wavplay.h"
#endif
#elif
#include <sys/types.h>
#include <dirent.h>
#endif

#include "aux_classes.h"
#include "sigproc.h"
#include "sigproc_internal.h"
#ifndef CISIGPROC
#include "psycon.tab.h"
#else
#include "cipsycon.tab.h"
#endif


#ifndef NO_FILES
//these functions are defined in AuxFunc_file.cpp
void _fopen(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _fclose(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _fprintf(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _fread(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _fwrite(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _write(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _wavwrite(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _wave(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _file(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
#else
void _fopen(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {}
void _fclose(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {}
void _fprintf(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {}
void _fread(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {}
void _fwrite(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {}
void _write(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {}
void _wavwrite(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {}
void _wave(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {}
void _file(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {}

#endif

string CAstSigEnv::AppPath = "";
map<string, Cfunction> dummy_pseudo_vars;
map<string, Cfunction> CAstSigEnv::pseudo_vars = dummy_pseudo_vars;

//#ifdef NO_PLAYSND // for aux_builtin_ext
//CAstSig::play_block_ms = 0;
//CAstSig::record_block_ms = 0;
//CAstSig::record_bytes = 0;
//#endif


void _figure(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _axes(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _text(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _plot(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _line(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _delete_graffy(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _repaint(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _showrms(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
void _replicate(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);

/* 10/10/2018
In all of these pEnv->inFunc, I have been using a temporary variable of CAstSig like
	CAstSig tp(past);
to shield already evaluated past->Sig.
In the past, tp->dad is set to dad, which ends up creating a situation where dad's son is not oneself. 
This is a problem as it causes a crash during the clean up of pAst insie of CallUDF()
Therefore, now, dad of these temporary variables is set to NULL to avoid it.
Also, CAstSig tp(past); in pEnv->inFunc should have a separate, temporary exception catcher, which relays the exception to the main AstSig.
It's a bit complicated solution. If you can handle the overhead of copying, an alternative solution is
not creating CAstSig tp(past); but just make a copied version of past->Sig and proceed with Compute(p)
*/

// Don't do past->Compute(p) inside _function for non-static functions
// Because the first argument has been computed when the function is called. 

int countVectorItems(const AstNode *pnode)
{
	if (pnode->type != N_VECTOR) return 0;
	AstNode *p = ((AstNode*)pnode->str)->alt;
	int res = 0;
	for (; p; p = p->next)
		res++;
	return res;
}

/*
7/24/2018

double (CSignal::*pf_basic)(unsigned int, unsigned int);
CSignal& (CSignal::*pf_basic2)(unsigned int, unsigned int);
CSignal (CSignal::*pf_basic3)(unsigned int, unsigned int);

Notes:
pf_basic and pf_basic3: past->Sig = past->Sig.basic(....)

Examples
pf_basic: dur begint std rms length ...
pf_basic2: envelope hilbert sort ramp
pf_basic3: fft ifft size

Additional parameter is passed through parg, which is a void pointer, mostly used as a pointer to double, but it could be anything else.

*/

/* 11/28/2019
output binding for built-in function:
	Sig has the first (primary) output.
	
*/

bool CAstSig::IsValidBuiltin(string funcname)
{
	if (pEnv->pseudo_vars.find(funcname) != pEnv->pseudo_vars.end())
		return false;
	return pEnv->builtin.find(funcname) != pEnv->builtin.end();
}

void CAstSig::checkAudioSig(const AstNode *pnode, CVar &checkthis, string addmsg)
{
	if (checkthis.type() & TYPEBIT_AUDIO) return;
	if (checkthis.GetType()==CSIG_CELL && ((CSignal)checkthis).GetType()==CSIG_AUDIO) return; // Why is this here? Maybe there is a data type of cell with audio data? 6/29/2020
	string msg("requires an audio signal as the base.");
	throw CAstExceptionInvalidUsage(*this, pnode, (msg+addmsg).c_str());
}

void CAstSig::checkTSeq(const AstNode *pnode, CVar &checkthis, string addmsg)
{
	if (checkthis.IsTimeSignal()) return;
	string msg("requires a time_sequence as the base.");
	throw CAstExceptionInvalidUsage(*this, pnode, (msg + addmsg).c_str());
}


void CAstSig::checkComplex (const AstNode *pnode, CVar &checkthis)
{
	if (checkthis.IsComplex()) return;
	string msg("requires a complex vector as the base.");
	throw CAstExceptionInvalidUsage(*this, pnode, msg.c_str());
}

void CAstSig::checkSignal(const AstNode *pnode, CVar &checkthis, string addmsg)
{ // if it is audio or vector --> OK
  // if not (including scalar) --> not OK
	if (checkthis.GetType() == CSIG_VECTOR) return;
	if (checkthis.GetType() == CSIG_AUDIO) return;
	string msg("requires an audio signal or a vector.");
	throw CAstExceptionInvalidUsage(*this, pnode, (msg + addmsg).c_str());
}

void CAstSig::checkVector(const AstNode *pnode, CVar &checkthis, string addmsg)
{
	if (checkthis.GetType() == CSIG_SCALAR) return;
	if (checkthis.GetType() == CSIG_VECTOR) return;
	string msg("requires a non-audio array.");
	throw CAstExceptionInvalidUsage(*this, pnode, (msg+addmsg).c_str());
}

void CAstSig::checkScalar(const AstNode *pnode, CVar &checkthis, string addmsg)
{
	if (checkthis.IsScalar()) return;
	string msg("requires a scalar argument.");
	throw CAstExceptionInvalidUsage(*this, pnode, (msg+addmsg).c_str());
}

void CAstSig::checkString(const AstNode *pnode, CVar &checkthis, string addmsg)
{
	if (checkthis.GetType()==CSIG_STRING) return;
	string msg("requires a string argument.");
	if (checkthis.GetType()==CSIG_CELL && ((CSignal)checkthis).GetType()==CSIG_STRING) return;
	throw CAstExceptionInvalidUsage(*this, pnode, (msg+addmsg).c_str());
}

void CAstSig::blockCell(const AstNode *pnode, CVar &checkthis)
{
	string msg("Not valid with a cell, struct, or point-array variable ");
	if (checkthis.GetFs()==3)
		throw CAstExceptionInvalidUsage(*this, pnode, msg.c_str());
	if (checkthis.GetType() == CSIG_CELL)
		if (((CSignal)checkthis).GetType() == CSIG_EMPTY)
			throw CAstExceptionInvalidUsage(*this, pnode, msg.c_str());
	if (checkthis.GetType() == CSIG_STRUCT)
			throw CAstExceptionInvalidUsage(*this, pnode, msg.c_str());
}

void CAstSig::blockEmpty(const AstNode *pnode, CVar &checkthis)
{
	if (!checkthis.IsString() && !checkthis.IsEmpty()) return;
	if (checkthis.IsString() && checkthis.nSamples>1) return;
	string msg("Not valid with an empty variable ");
	throw CAstExceptionInvalidUsage(*this, pnode, msg.c_str());
}

void CAstSig::blockScalar(const AstNode *pnode, CVar &checkthis)
{
	if (!(checkthis.type() & 1)) return;
	string msg("Not valid with a scalar variable ");
	throw CAstExceptionInvalidUsage(*this, pnode, msg.c_str());
}

void CAstSig::blockString(const AstNode *pnode, CVar &checkthis)
{
	if (checkthis.GetType() == CSIG_STRING) {
		string msg("Not valid with a string variable ");
		throw CAstExceptionInvalidUsage(*this, pnode, msg.c_str());
	}
}

void CAstSig::blockComplex(const AstNode *pnode, CVar &checkthis)
{
	if (checkthis.IsComplex()) {
		string msg("Not valid with a complex variable ");
		throw CAstExceptionInvalidUsage(*this, pnode, msg.c_str());
	}
}

void _diff(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkSignal(pnode, past->Sig);
	CVar sig = past->Sig;
	int order=1;
	if (p) {
		past->Compute(p);
		past->checkScalar(pnode, past->Sig);
		order = (int)round(past->Sig.value());
		if (order < 1) throw CAstExceptionInvalidUsage(*past, pnode, "non-negative argument is required");
	}
	sig.Diff(order);
	past->Sig = sig;
}

void _nullin(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	CVar sig = past->Sig;
	past->Compute(p);
	past->checkScalar(p, past->Sig);
	sig.NullIn(past->Sig.value());
	past->Sig = sig;
}

void _contig(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->Sig.MakeChainless();
}

void _cumsum(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkVector(pnode, past->Sig);
	past->Sig.Cumsum();
}

void _getfs(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->Sig.SetValue(past->GetFs());
}

void _tictoc(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	string fname = pnode->str;
	if (fname == "tic")
	{
		past->Sig.SetValue((double)past->tic()); // Internal timer value--not really useful for the user other than showing it was working
	}
	else
	{
		past->Sig.SetValue((double)past->toc(pnode)); // milliseconds since tic was called.
	}
}

void _rand(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkScalar(pnode, past->Sig);
	double val = past->Sig.value();
	if (val<0)
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "argument must be positive.");
	int ival = (int)round(val);
	static bool initialized(false);
	if (!initialized)
	{
		srand((unsigned) time(0));
		initialized = true;
	}
	past->Sig.UpdateBuffer(ival);
	for (int k = 0; k < ival; k++)
		past->Sig.buf[k] = (double)rand() / RAND_MAX;
}

void _irand(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkScalar(pnode, past->Sig);
	double val = past->Sig.value();
	if (val<0)
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "argument must be positive.");
	static bool initialized(false);
	if (!initialized)
	{
		srand((unsigned)time(0));
		initialized = true;
	}
	past->Sig.UpdateBuffer(1);
	past->Sig.SetValue((double)ceil((double)rand() / (double)RAND_MAX*val));
}

void _randperm(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkScalar(pnode, past->Sig);
	int ival  = (int)round(past->Sig.value());
	if (ival < 1)
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "argument must be positive.");
	static bool initialized(false);
	if (!initialized)
	{
		srand((unsigned)time(0));
		initialized = true;
	}
	past->Sig.Reset(1);
	past->Sig.UpdateBuffer((size_t)ival);
	int m, n;
	double hold;
	for (int i=0; i<ival; i++)past->Sig.buf[i] = (double)(i+1);
	int repeat = (int)sqrt(ival*100.); // swapping sqrt(ival*100.) times
	for (int i=0; i<repeat; i++)
	{
		m = (int)((double)rand()/(double)RAND_MAX*ival);
		do { n = (int)((double)rand()/(double)RAND_MAX*ival); }
		while (m==n);
		hold = past->Sig.buf[m];
		past->Sig.buf[m] = past->Sig.buf[n];
		past->Sig.buf[n] = hold;
	}
}

void _time_freq_manipulate(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	//check qualifers
	past->checkAudioSig(pnode, past->Sig);
	CVar param, paramopt;
	string fname;
	try {
		CAstSig tp(past);
		param = tp.Compute(p);
		if (p->next)
		{
			paramopt = tp.Compute(p->next);
			if (paramopt.strut.empty())
				throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Third parameter, if used, must be a struct variable.");
		}
		int type = param.GetType();
		if (type!= CSIG_TSERIES && type != CSIG_SCALAR)
			throw CAstInvalidFuncSyntax(*past, p, fnsigs, "parameter must be either a scalar or a time sequence.");
		if (param.GetType() == CSIG_TSERIES)
		{
			double audioDur = past->Sig.dur().front();
			if (param.GetFs() == 0) // relative
				for (CTimeSeries *p = &param; p; p = p->chain)
				{
					p->tmark *= audioDur;
					p->SetFs(past->Sig.GetFs());
				}
			//If the first tmark is not 0, make one with 0 tmark and bring the value at zero
			if (param.tmark != 0.)
			{
				CTimeSeries newParam(past->Sig.GetFs());
				newParam.tmark = 0.;
				newParam.SetValue(param.value());
				newParam.chain = new CTimeSeries;
				*newParam.chain = param; // this way the copied version goes to chain
				param = newParam;
			}
			//If the last tmark is not the end of the signal, make one with 0 tmark and bring the value at zero
			CTimeSeries *pLast;
			for (CTimeSeries *p = &param; p; p = p->chain)
				if (!p->chain) 
					pLast = p;
			if (pLast->tmark != past->Sig.dur().front())
			{
				CTimeSeries newParam(past->Sig.GetFs());
				newParam.tmark = past->Sig.dur().front();
				newParam.SetValue(pLast->value());
				pLast->chain = new CTimeSeries;
				*pLast->chain = newParam; // this way the copied version goes to chain
			}
			//IF tsequence goes beyond the audio duration, cut it out.
			for (CTimeSeries *p = &param; p; p = p->chain)
				if (p->chain && p->chain->tmark > audioDur)
				{
					delete p->chain;
					p->chain = NULL;
				}
		}
		fname = pnode->str;
		for (auto it = paramopt.strut.begin(); it != paramopt.strut.end(); it++)
			param.strut[(*it).first] = (*it).second;
		if (fname == "respeed")
			past->Sig.runFct2modify(&CSignal::resample, &param);
		else if (fname == "movespec")
			past->Sig.runFct2modify(&CSignal::movespec, &param);
		if (param.IsString())
			throw CAstExceptionInvalidUsage(*past, pnode, ("Error in respeed:" + param.string()).c_str());
	}
	catch (const CAstException &e) { throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, e.getErrMsg().c_str()); }
	if (fname == "respeed")
	{ // Take care of overlapping chains after processing
		past->Sig.MergeChains();
	}
}

void processEscapes(string &str)
{
	size_t pos;
	for (size_t start=0; (pos=str.find('\\', start))!=string::npos; start=pos+1)
		switch (str[pos+1]) {
		case 'n':
			str.replace(pos, 2, "\n");
			break;
		case 't':
			str.replace(pos, 2, "\t");
			break;
		default:
			str.erase(pos, 1);
		}
}

void _sprintf(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CAtlRegExp<> regexp;
	REParseError status = regexp.Parse("%([-+ #0]+)?({\\z|\\*})?(\\.{\\z|\\*})?[hlL]?{[cuoxXideEgGfs]}" );
	if (status != REPARSE_ERROR_OK)
		throw CAstExceptionInternal(*past, pnode, "[INTERNAL] _sprintf()--RegExp.Parse( ) failed.");
	past->Sig.Reset(2);	// to get the output string
	CAstSig tast(past);	// to preserve this->Sig
	CAtlREMatchContext<> mcFormat;
	string fmtstring = tast.ComputeString(p);
	processEscapes(fmtstring);
	const char *fmtstr = fmtstring.c_str();
	for (const char *next; fmtstr && regexp.Match(fmtstr, &mcFormat, &next); fmtstr=next) {
		const CAtlREMatchContext<>::RECHAR* szStart = 0;
		const CAtlREMatchContext<>::RECHAR* szEnd = 0;
		string vstring;
		double v;
		string fmt1str(fmtstr, mcFormat.m_Match.szEnd-fmtstr);
		vector<char> outStr;
		outStr.resize(100);
		for (UINT nGroupIndex = 0; nGroupIndex < mcFormat.m_uNumGroups; ++nGroupIndex) {
			mcFormat.GetMatch(nGroupIndex, &szStart, &szEnd);
			if (nGroupIndex == 2 || (nGroupIndex < 2 && szStart && *szStart == '*')) {	// condition for consuming an argument
				if ((p = p->next) == NULL)
					throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "Not enough arguments.");
				if (nGroupIndex == 2 && *szStart == 's')
					vstring = tast.ComputeString(p);
				else if (tast.Compute(p)->IsScalar())
					v = tast.Sig.value();
				else
					throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "Scalar value expected for this argument.");
				if (nGroupIndex != 2) {
					char width[20];
					sprintf(width, "%d", round(v));
					fmt1str.replace(szStart-fmtstr, 1, width);
				}
			}
		}
		switch (*szStart) {
		case 'e': case 'E':
		case 'g': case 'G':
		case 'f':
			sprintf(&outStr[0], fmt1str.c_str(), v);
			break;
		case 'c': case 'o':
		case 'x': case 'X':
		case 'i': case 'u':
		case 'd':
			sprintf(&outStr[0], fmt1str.c_str(), (int)round(v));
			break;
		case 's':
			outStr.resize(vstring.size()+100);
			sprintf(&outStr[0], fmt1str.c_str(), vstring.c_str());
			break;
		}
		unsigned int n = (unsigned int)past->Sig.CSignal::length().front();
		past->Sig.UpdateBuffer(n + (unsigned int)strlen(&outStr[0])+1);
		strcpy(&past->Sig.strbuf[n], &outStr[0]);
	}
	unsigned int n = (unsigned int)past->Sig.CSignal::length().front();
	past->Sig.UpdateBuffer(n + (unsigned int)strlen(fmtstr)+1);
	strcpy(&past->Sig.strbuf[n], &fmtstr[0]);
	past->Sig.bufBlockSize = 1;
}


void _colon(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar first = past->Sig;
	if (!first.IsScalar()) throw CAstExceptionInvalidUsage(*past, pnode, "Colon: All arguments must be scalars (check 1st arg).");
	double val1, val2, step;
	CVar third, second = past->Compute(p->next);
	if (!second.IsScalar()) throw CAstExceptionInvalidUsage(*past, pnode, "Colon: All arguments must be scalars (check 2nd arg).");
	val1 = first.value();
	val2 = second.value();
	if (p->next->next) {
		third = past->Compute(p->next->next);
		if (!third.IsScalar()) throw CAstExceptionInvalidUsage(*past, pnode, "Colon: All arguments must be scalars (check 3rd arg).");
		step = third.value();
	}
	else 
		step = (val1>val2) ? -1. : 1.;
	past->Sig.Reset(1);
	int nItems = max(1, (int)((val2-val1)/step)+1);
	past->Sig.UpdateBuffer(nItems);
	for (int i=0; i<nItems; i++)
		past->Sig.buf[i] = val1 + step*i;
}

void _interp1(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	//Need to add qualifers at some point.
	// Probably allow only vectors... 
	// Assume qp is non-decreasing sequence.
	// 3 / 6 / 2019
	CVar rx = past->Sig;
	CVar ry = past->Compute(p);
	CVar qx = past->Compute(p->next);
	vector<double> rv = rx.ToVector();
	vector<double>::iterator it = rv.begin();
	past->Sig.UpdateBuffer(qx.nSamples);
	int k = 0;
	for (unsigned q=0;q<qx.nSamples;q++)
	{
		it = upper_bound(it, rv.end(), qx.buf[q]); //because qp is sorted, having the iterator (previous searched result, if any) as the first argument will save time.
		ptrdiff_t pos;
		double valueAlready, preVal;
		if (it != rv.end())
		{
			pos = it - rv.begin();
			pos = max(1, pos);
			preVal = *(it - 1);
		}
		else
		{
			pos = ry.nSamples;
			preVal = rv.back();
		}
		valueAlready = ry.buf[pos - 1];
		past->Sig.buf[k++] = valueAlready + (ry.buf[pos] - ry.buf[pos - 1]) / (rx.buf[pos] - rx.buf[pos - 1]) * (qx.buf[q] - preVal);
	}
}

void _fdelete(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	string filename = past->MakeFilename(past->ComputeString(p), "txt");
	int res = remove(filename.c_str());
	if (!res) // success
		past->Sig.SetValue(1);
	else
		past->Sig.SetValue(0);
}

#ifndef NO_SF
// It is better to have setfs as a hook command, because it involves UI and expressions such as setfs(16000)+1 don't make sense.
// Keep this just in case. But this will not update existing variables according to the new sample rate, so its functionality is pretty limited.
// Use an auxlab hook command in this format #setfs 16000
// 11/21/2017
void _setfs(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (!past->Sig.IsScalar()) throw CAstExceptionInvalidUsage(*past, p, "Scalar value should be provided for the sample rate.");
	double fs = past->Sig.value();
	if (fs<500.) throw CAstExceptionInvalidUsage(*past, p, "Sample rate should be at least 500 Hz.");
	past->FsFixed=true;
	past->pEnv->Fs = (int)fs; //Sample rate adjusted
}



#endif // NO_SF

#ifdef _WINDOWS
#ifdef NO_PLAYSND
void _record(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{}
#else
extern HWND hShowDlg;
void _record(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	int nArgs = 0, devID = 0, nChans = 1;
	if (pnode->type != N_STRUCT)
	{ // If not class calling
		if (pnode->alt && pnode->alt->type == N_ARGS)
			p = pnode->alt->child;
	}
	double block = CAstSig::record_block_ms;
	double duration = -1;
	AstNode *cbnode = nullptr;
	if (pnode->type==T_ID && pnode->alt->alt)
	{ // record(....).cbname
		cbnode = pnode->alt->alt;
//		yydeleteAstNode(pnode->alt->alt, 0);
		if (pnode->tail == pnode->alt->alt)
			(AstNode *)pnode->tail = nullptr;
		pnode->alt->alt = nullptr;
	}
	else if (pnode->type == N_STRUCT && pnode->alt)
	{ // v.record.cbname
		cbnode = pnode->alt;
		if (pnode->tail == pnode->alt)
			(AstNode *)pnode->tail = nullptr;
		(AstNode *)pnode->alt = nullptr; // check
	}
	for (const AstNode *cp = p; cp; cp = cp->next)
		++nArgs; 
	switch (nArgs)
	{
	case 4:
		past->Compute(p->next->next->next);
		if (!past->Sig.IsScalar())
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "The fourth argument must be a constant representing the block size for the callback in milliseconds.");
		block = past->Sig.value();
	case 3:
		past->Compute(p->next->next);
		if (!past->Sig.IsScalar())
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "The third argument is either 1 (mono) or 2 (stereo) for recording.");
		nChans = (int)past->Sig.value();
		if (nChans != 1 && nChans != 2)
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "The third argument is either 1 (mono) or 2 (stereo) for recording.");
	case 2:
		past->Compute(p->next);
		if (!past->Sig.IsScalar())
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "The second argument must be a constant representing the duration to record, -1 means indefinite duration until stop is called.");
		duration = past->Sig.value();
	case 1:
		past->Compute(p);
		if (past->Sig.IsScalar())
		{
			devID = (int)past->Sig.value();
			past->Sig.Reset();
		}
		else if (past->Sig.IsStruct() && past->Sig.strut.find("type") != past->Sig.strut.end())
		{
			CVar *pobj = &past->Sig;
			string objname = "devID";
			if (pobj->strut.find(objname) == pobj->strut.end())
				pobj->strut[objname] = CVar((double)devID);
			else
				devID = (int)pobj->strut[objname].value();
			if (pobj->strut.find(objname = "dur") == pobj->strut.end())
				pobj->strut[objname] = CVar((double)duration);
			else
				duration = (int)pobj->strut[objname].value();
			if (pobj->strut.find(objname = "channels") == pobj->strut.end())
				pobj->strut[objname] = CVar(1.);
			else
				nChans = (int)pobj->strut[objname].value();
			if (pobj->strut.find(objname = "block") == pobj->strut.end())
				pobj->strut[objname] = CVar(block);
			else
				block = (int)pobj->strut[objname].value();
		}
		else
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "The first argument must be an audio_recorder object or a constant (integer) representing the device ID.");
		break;
	case 0:
		break;
	}
	srand((unsigned)time(0));
	CVar handle((double)rand());
	handle.strut["dev"] = CVar((double)devID);
	handle.strut["type"] = CVar(string("audio_record"));
	handle.strut["id"] = CVar(handle.value());
	handle.strut["callback"] = "";
	handle.strut["channels"] = CVar((double)nChans);
	handle.strut["durLeft"] = CVar(duration / 1000.);
	handle.strut["durRec"] = CVar(0.);
	handle.strut["block"] = CVar(block);
	handle.strut["active"] = CVar((double)(1 == 0));

	char errstr[256] = {};
	int newfs, recordID = (int)handle.value();
	if ((newfs = Capture(devID, WM__AUDIOEVENT2, hShowDlg, past->pEnv->Fs, nChans, CAstSig::record_bytes, cbnode, duration, block, recordID, errstr)) < 0)
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, errstr);
	handle.strut["active"] = CVar((double)(1 == 1));
	past->Sig.strut["h"] = handle;
	//output binding 
	if (past->lhs)
	{
		if (past->lhs->type == N_VECTOR)
			past->outputbinding(past->lhs);
		else
			past->bind_psig(past->lhs, &past->Sig);
	}
	else
	{ // ans variable
		past->SetVar("ans", &past->Sig);
	}

	// for a statement, y=h.start, y is not from the RHS directly, but is updated laster after the callback
	// so we need to block the RHS from affecting the LHS.. Let's use -1 for suppress (to be used in CDeepProc::TID_tag in AstSig2.cpp)
	past->pAst->suppress = -1;
	if (newfs != past->pEnv->Fs)
	{
		past->pEnv->Fs = newfs;
		sformat(past->statusMsg, "(NOTE)Sample Rate of AUXLAB Environment is adjusted to %d Hz.", past->pEnv->Fs);
	}
	past->Sig.Reset(); // to shield the first LHS variable (callback output) from Sig // ??? 11/29/2019
}
void _play(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar sig = past->Sig;
	//sig must be either an audio signal or an audio handle.
	char errstr[256] = {};
	int nRepeats(1);
	double block = CAstSig::play_block_ms;
	if (sig.GetType() != CSIG_AUDIO)
	{
		if (!sig.IsScalar())
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "The base must be an audio signal or an audio handle (1)");
		if (sig.strut.find("type")== sig.strut.end())
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "The base must be an audio signal or an audio handle (2)");
		CVar type = sig.strut["type"];
		if (type.GetType()!=CSIG_STRING || type.string() != "audio_playback")
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "The base must be an audio signal or an audio handle (3)");
		if (!p)
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "Audio signal not given.");
		past->Compute(p);
		past->checkAudioSig(p, past->Sig);
		CVar audio = past->Sig;
		if (p->next)
		{
			past->Compute(p->next);
			if (!past->Sig.IsScalar())
				throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Argument must be a scalar.");
			nRepeats = (int)past->Sig.value();
			if (nRepeats<1)
				throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Repeat counter must be equal or greater than one.");
		}
		INT_PTR h = PlayCSignals((INT_PTR)sig.value(), audio, 0, WM__AUDIOEVENT1, &block, errstr, nRepeats);
		if (!h)
			past->Sig.SetValue(-1.);
		else
		{
			//Update the handle with the PlayArrayNext info
			//don't update sig, which is only a copy. Instead, go for Vars
			for (map<string, CVar>::iterator it = past->Vars.begin(); it != past->Vars.end(); it++)
			{
				if ((*it).second == sig.value())
				{
					(*it).second.strut["durLeft"].SetValue(sig.strut["durLeft"].value() + audio.alldur()*nRepeats / 1000);
					(*it).second.strut["durTotal"].SetValue(sig.strut["durTotal"].value()+ audio.alldur()*nRepeats / 1000);
					past->Sig = (*it).second;
				}
			}
		}
	}
	else
	{
		if (p)
		{
			past->Compute(p);
			if (!past->Sig.IsScalar())
				throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Argument must be a scalar.");
			nRepeats = (int)past->Sig.value();
			if (nRepeats<1)
				throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Repeat counter must be equal or greater than one.");
		}
		int devID = 0;
		INT_PTR h = PlayCSignals(sig, devID, WM__AUDIOEVENT1, GetHWND_WAVPLAY(), &block, errstr, nRepeats);
		if (!h)
		{ // PlayArray will return 0 if unsuccessful due to waveOutOpen failure. For other reasons.....
			past->Sig.strut.clear();
			//errstr should show the err msg. Use it if necessary 7/23/2018
		}
		else
		{
			double ad = sig.alldur();
			double addtime = ad * nRepeats / 1000.;
			AUD_PLAYBACK * p = (AUD_PLAYBACK*)h;
			p->sig.SetValue((double)(INT_PTR)h);
//			p->sig.strut["data"] = sig; // Let's not do this any more.. no strong need. 9/9/2019
			p->sig.strut.insert(pair<string, CVar>("type", string("audio_playback")));
			p->sig.strut.insert(pair<string, CVar>("devID", CVar((double)devID)));
			p->sig.strut.insert(pair<string, CVar>("durTotal", CVar(addtime)));
			if (p->sig.strut.find("durLeft") == p->sig.strut.end())
				p->sig.strut.insert(pair<string, CVar>("durLeft", CVar(addtime)));
			else
				*(p->sig.strut["durLeft"].buf) += addtime;
			p->sig.strut.insert(pair<string, CVar>("durPlayed", CVar(0.)));
			past->Sig = p->sig; //only to return to xcom
		}
	}
}

void _stop(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	char errstr[256];
	CVar sig = past->Sig;
	if (!sig.IsScalar())
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "Argument must be a scalar.");
	string fname = past->pAst->str;
	if (!p)
		fname = past->pAst->alt->str;
	if (sig.strut["type"].string() == "audio_playback" && (fname == "qstop" || fname == "stop"))
	{
		if (!StopPlay((INT_PTR)sig.value(), fname == "qstop"))
		{
			past->Sig.strut.clear();
			past->Sig.SetValue(-1.);
		}
	}
	else if (sig.strut["type"].string() == "audio_record" && fname == "stop")
	{
		StopRecord((int)sig.value(), errstr);
	}
	else
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "stop() applies only to audio_playback or audio_record.");
}

void _pause_resume(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar sig = past->Sig;
	if (!sig.IsScalar())
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "Argument must be a scalar.");
	string fname = past->pAst->str;
	if (!p)
		fname = past->pAst->alt->str;
	if (sig.strut["type"].string() == "audio_playback")
	{
		if (!PauseResumePlay((INT_PTR)sig.value(), fname == "resume"))
		{
			past->Sig.strut.clear();
			past->Sig.SetValue(-1.);
		}
	}
	else if (sig.strut["type"].string() == "audio_record")
	{

	}
	else
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "pause() or resume() applies only to audio_playback or audio_record.");
}

#endif // NO_PLAYSND


void aux_input(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkString(pnode, past->Sig);
	printf("%s ", past->Sig.string().c_str());
	string user_input;
	cin >> user_input;
	past->Sig.SetString(user_input.c_str());
}

void udf_error(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkString(pnode, past->Sig);
	throw CAstExceptionInvalidUsage(*past, pnode, past->Sig.string().c_str());
}

void udf_warning(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkString(pnode, past->Sig);
	printf("WARNING: %s\n", past->Sig.string().c_str());
}
void udf_rethrow(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkString(pnode, past->Sig);
//
}

#ifdef _WINDOWS
#include "bjcommon_win.h"
#endif

void _inputdlg(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	ostringstream caption;
	if (past->pAst->type == N_BLOCK && !past->Script.empty())
	{
		caption << "[UDF] \"" << past->Script << "\" ";
		caption << "Line " << pnode->line;
	}
	caption << past->Sig.string();
	_sprintf(past, pnode, p, fnsigs);

	char buf[64] = {};
#ifdef _WINDOWS
	INT_PTR res = InputBox(caption.str().c_str(), past->Sig.string().c_str(), buf, sizeof(buf));
	if (res == 1)
		past->Sig.SetString(buf);
	else
		past->Sig.SetString("");
#elif
	printf("[NOTE] inputdlg is in a pre-release condition in non-Windows version.\n%s", past->Sig.string().c_str());
#endif
}

void _msgbox(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	ostringstream caption;
	caption << "Line " << pnode->line;
	if (pnode->type != N_STRUCT)
	{
		p = pnode->alt;
		if (p->type == N_ARGS) p = p->child;
		_sprintf(past, pnode, p, fnsigs);
	}
	past->fpmsg.ShowVariables(past);
#ifdef _WINDOWS
	MessageBox(NULL, past->Sig.string().c_str(), caption.str().c_str(), MB_OK);
	past->Sig.Reset();
#elif
	printf("[NOTE] inputdlg is in a pre-release condition in non-Windows version.\n%s", past->Sig.string().c_str());
#endif
}


#include "audstr.h"


#endif //_WINDOWS this

void _include(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	string dummy;
	string filename = past->ComputeString(p);
	if (FILE *auxfile = past->OpenFileInPath(filename, "", dummy)) {
		try {
			fclose(auxfile);
			CAstSig tast(past->pEnv);
			string body;
			string emsg;
			if (!tast.SetNewScriptFromFile(emsg, filename.c_str(), NULL, body))
				if (!emsg.empty())
					throw emsg.c_str();
			vector<CVar *> res = tast.Compute();
			past->Sig = res.back();
			for (map<string, CVar>::iterator it = tast.Vars.begin(); it != tast.Vars.end(); it++)
				past->Vars[it->first] = it->second;
		} catch (const char *errmsg) {
			fclose(auxfile);
			throw CAstExceptionInvalidUsage(*past, pnode, ("Including "+filename+"\n\nIn the file: \n"+errmsg).c_str());
		}
	} else
		throw CAstExceptionInvalidUsage(*past, pnode, "Cannot read file: ", "", filename);
}

void _eval(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{ // eval() is one of the functions where echoing in the xcom command window doesn't make sense.
  // but the new variables created or modified within the eval call should be transported back to ast
    // As of 5/17/2020, there is no return of eval (null returned if assigned) for when there's no error
	// If there's an error, exception handling (not error handling) is done and it returns the error message
	string str = past->ComputeString(p);
	try {
		CAstSig tast(str.c_str(), past);
		tast.Compute(tast.pAst);
		//transporting variables 
		for (map<string, CVar>::iterator it = tast.Vars.begin(); it != tast.Vars.end(); it++)
			past->SetVar(it->first.c_str(), &it->second);
		past->Sig = tast.Sig; // temp hack; just to port out the last result during the eval call
	} catch (CAstException e) {
		e.line = pnode->line;
		e.col = pnode->col;
		e.pCtx = past;
		throw e;
	}
}

bool isAllNodeT_NUM(const AstNode *p)
{
	if (p->type == T_NUMBER) 
	{ 
		if (p->next)
			return isAllNodeT_NUM(p->next);
		return true;
	}
	return false;
}

void _str2num(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (!past->Sig.IsString())
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "argument must be a text string.");
	string emsg;
	CAstSig tast(past->pEnv);
	if (!tast.SetNewScript(emsg, (string("[") + past->Sig.string() + "]").c_str()))
		tast.SetNewScript(emsg, "[]");
	if (!isAllNodeT_NUM(tast.pAst->child))
		tast.SetNewScript(emsg, "[]");
	vector<CVar *> res = tast.Compute();
	past->Sig = res.back();
}

void _zeros(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (!past->Sig.IsScalar())
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "argument must be a scalar.");
	int n = (int)round(past->Sig.value());
	past->Sig.Reset(1);
	if (n <= 0) return;
	past->Sig.UpdateBuffer(n);
	for (int i=0; i<n; ++i)
		past->Sig.buf[i] = 0;
}

void _ones(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (!past->Sig.IsScalar())
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "argument must be a scalar.");
	int n = (int)round(past->Sig.value());
	past->Sig.Reset(1);
	if (n <= 0) return;
	past->Sig.UpdateBuffer(n);
	for (int i=0; i<n; ++i)
		past->Sig.buf[i] = 1;
}

void _matrix(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->blockCell(pnode, past->Sig);
	CVar second;
	try {
		CAstSig tp(past);
		second = tp.Compute(p);
	}
	catch (const CAstException &e) {
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, e.getErrMsg().c_str());
	}
	if (!second.IsScalar())
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "argument must be a scalar.");
	double val = second.value();
	if (val!=(double)(int)val)
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "argument must be an integer.");
	double nCols = past->Sig.nSamples / val;
	if (past->Sig.GetType() == CSIG_VECTOR)
	{
		if (past->Sig.nSamples / val != (int)nCols)
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "The length of array must be divisible by the requested the row count.");
	}
	else if (past->Sig.GetType() == CSIG_AUDIO)
	{
		if (past->Sig.chain)
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "To make a matrix from an audio signal, null portions should be filled with zeros. Call contig().");
		double rem = fmod((double)past->Sig.nSamples, val);
		if (rem > 0)
		{
			unsigned int nPtsNeeded = (unsigned int)(val - rem);
			past->Sig.UpdateBuffer(past->Sig.nSamples + nPtsNeeded);
		}
	}
	else
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "Vector or audio signals required.");
	past->Sig.nGroups = (int)val;
}

void _cell(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (!past->Sig.IsScalar())
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "argument must be a scalar.");
	int n = (int)round(past->Sig.value());
	if (n <= 0)
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "argument must be a positive number.");

	past->Sig.Reset();
	CVar tp;
	for (int k = 0; k < n; k++)
		past->Sig.appendcell(tp);
}

void _clear(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->ClearVar((AstNode *)past->pAst, &past->Sig);
	past->Sig.Reset();
}


void _dir(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->Sig.Reset(1);
	past->Compute(p);
	if (!past->Sig.IsString())
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "argument must be a string.");
	string arg = past->Sig.string();
	char drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH], ext[MAX_PATH], pathonly[MAX_PATH] = {};
	_splitpath(arg.c_str(), drive, dir, fname, ext);
	sprintf(pathonly, "%s%s", drive, dir);
	if (strlen(fname)==0 && strlen(ext)==0)
		arg += "\\*.*";
#ifdef _WINDOWS
	WIN32_FIND_DATA ls;
	HANDLE hFind = FindFirstFile(arg.c_str(), &ls);
	if (hFind == INVALID_HANDLE_VALUE)
//	if ((hFile = _findfirst(, &c_file)) == -1L)
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "No files in the specified directory");
	else
	{
		past->Sig.Reset();
		do {
			if (!pathonly[0])
				_splitpath(ls.cFileName, drive, dir, fname, ext);
			else
				_splitpath(ls.cFileName, NULL, NULL, fname, ext);
			char fullname[256];
			CVar tp;
			tp.strut["name"] = string(fname);
			tp.strut["ext"] = string(ext);
			if (fname[0] != '.' || fname[1] != '\0')
			{
				if (pathonly[0])
					tp.strut["path"] = string(pathonly);
				else
				{
					char *pt = strstr(fullname, fname);
					if (pt) *pt = 0;
					tp.strut["path"] = string(fullname);
				}
				tp.strut["bytes"] = CVar((double)(ls.nFileSizeHigh * ((uint64_t)MAXDWORD + 1) + ls.nFileSizeLow));
				FILETIME ft = ls.ftLastWriteTime;
				SYSTEMTIME lt;
				FileTimeToSystemTime(&ft, &lt);
				sprintf(fullname, "%02d/%02d/%4d, %02d:%02d:%02d", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
				tp.strut["date"] = string(fullname);
				CVar b(double(ls.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
				tp.strut["isdir"] = b;
				past->Sig.appendcell(tp);
			}
		} while (FindNextFile(hFind, &ls));
	}
#elif
#endif
}

void _ismember(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (!past->Sig.IsStruct())
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Must be applied to a class/struct object.");
	try {
		CAstSig tp(past);
		CVar param = tp.Compute(p);
		if (!param.IsString())
			throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Argument must be a string.");
		auto it = past->Sig.strut.find(param.string());
		if (it == past->Sig.strut.end())
			past->Sig.SetValue(0.);
		else
			past->Sig.SetValue(1.);
		past->Sig.MakeLogical();
	}
	catch (const CAstException &e) { throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, e.getErrMsg().c_str()); }
}

void _isaudioat(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{ // if the signal is null at specified time_pt
	past->checkAudioSig(pnode, past->Sig);
	CAstSig tp(past);
	tp.Compute(p);
	if (tp.Sig.GetType() != CSIG_SCALAR)
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "argument must be a scalar.");
	double vv = tp.Sig.value();
	past->Sig.SetValue(past->Sig.IsAudioOnAt(vv));
	past->Sig.MakeLogical();
}

// This function datatype will eventually eliminate the use of CSIG_ series constants 
// and all the is____ functions 5/25/2020

int __datatype(const CVar &sig, WORD &out)
{
	if (sig.IsEmpty())				out = 0xffff;
	else if (sig.IsGO())		out = 0x2000;
	else if (sig.GetFs() == 1)		out = 0;
	else if (sig.GetFs() == 2)		out = 0x0010;
	else if (sig.GetFs() > 500)		out = 0x0020;
	else if (sig.IsLogical())		out = 0x0040;
	else if (!sig.cell.empty())		out = 0x1000;
	else if (!sig.strut.empty())		out = 0x2000;
	else
		return -1;
	if (sig.snap) out += 0x0008;
	if (sig.nSamples > 0) out++;
	if (sig.nSamples > 1) out++;
	return 1;
}

void _datatype(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	uint16_t out;
	if ((out=past->Sig.type())==0xffff)
		throw CAstExceptionInternal(*past, pnode, "[INTERNAL] this particular data type has not been ready to handle.");
	if (out & 0x2000)
	{
		past->pgo = NULL;
		past->Sig.Reset();
	}
	past->Sig.SetValue((double)out);
}

void _veq(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CAstSig br(past);
	WORD type1, type2;
	CVar arg1, arg2;
	if (pnode->type == N_STRUCT)
	{ // x.veg(y)
		arg1 = past->Sig;
		arg2 = past->Compute(pnode->alt->child);
	}
	else if (pnode->type == T_ID)
	{ // veq(x,y)
		if (pnode->alt->type == N_ARGS)
		{
			arg1 = past->Compute(pnode->alt->child);
			arg2 = past->Compute(pnode->alt->tail);
		}
		else
			throw CAstExceptionInternal(*past, pnode, "[INTERNAL] this particular data type has not been ready to handle.");
	}
	else
		throw CAstExceptionInternal(*past, pnode, "[INTERNAL] this particular data type has not been ready to handle.");

	if (__datatype(arg1, type1) < 0)
		throw CAstExceptionInternal(*past, pnode, "[INTERNAL] this particular data type has not been ready to handle.");
	if (__datatype(arg2, type2) < 0)
		throw CAstExceptionInternal(*past, pnode, "[INTERNAL] this particular data type has not been ready to handle.");
	try {
		// throw 0 for false
		if (type1 != type2) throw 0;
		else if (arg1.nSamples != arg2.nSamples) throw 0;
		else if (type1 & 0x2000) // GO
		{
			if (arg1.value() != arg2.value()) throw 0;
		}
		else
		{
			if (arg1.bufBlockSize == 8)
				for (unsigned k = 0; k < arg1.nSamples; k++)
				{
					if (arg1.buf[k] != arg2.buf[k]) throw 0;
				}
			else if (arg1.bufBlockSize == 16)
				for (unsigned k = 0; k < arg1.nSamples; k++)
				{
					if (arg1.cbuf[k] != arg2.cbuf[k]) throw 0;
				}
			else
				for (unsigned k = 0; k < arg1.nSamples; k++)
				{
					if (arg1.logbuf[k] != arg2.logbuf[k]) throw 0;
				}
		}
		past->Sig.Reset(1);
		past->Sig.MakeLogical();
		past->Sig.UpdateBuffer(1);
		past->Sig.logbuf[0] = true;
		return;
	}
	catch (int k)
	{
		//k should be 0 and it doesn't matter what k is.
		k = 0; // just to avoid warning C4101
		past->Sig.Reset(1);
		past->Sig.MakeLogical();
		past->Sig.UpdateBuffer(1);
		past->Sig.logbuf[0] = false;
		return;
	}
}

void _varcheck(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	string fname = pnode->str;
	int type = past->Sig.GetType();
	CVar out;
	if (fname == "isempty")			out.SetValue(past->Sig.IsEmpty());
	else if (fname == "isaudio")	out.SetValue(type == CSIG_AUDIO ? 1 : 0);
	else if (fname == "isvector")	out.SetValue(type == CSIG_VECTOR ? 1 : 0);
	else if (fname == "isstring")	out.SetValue(type == CSIG_STRING ? 1 : 0);
	else if (fname == "iscell")		out.SetValue((double)(int)!past->Sig.cell.empty());
	else if (fname == "isclass")	out.SetValue((double)(int)!past->Sig.strut.empty());
	else if (fname == "isbool")		out.SetValue(past->Sig.bufBlockSize == 1 && past->Sig.GetFs() != 2);
	else if (fname == "isstereo")	out.SetValue(past->Sig.next != NULL ? 1 : 0);
	else if (fname == "istseq")		out.SetValue((double)(int)past->Sig.IsTimeSignal()); //check...
	out.MakeLogical();
	past->Sig = out;
	past->pgo = NULL;//must be reset here; otherwise, the GO lingers and interferes with other opereation successively.
}

void _or(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->blockCell(pnode, past->Sig);
	past->blockString(pnode, past->Sig);
	past->blockComplex(pnode, past->Sig);
	if (!p) // single arg
	{
		double res(0.);
		if (past->Sig.IsLogical())
		{
			for (unsigned k = 0; k < past->Sig.nSamples; k++)
				if (past->Sig.logbuf[k]) { res = 1.;		break; }
		}
		else
		{
			for (unsigned k = 0; k < past->Sig.nSamples; k++)
				if (past->Sig.buf[k] != 0.) { res = 1.;	break; }
		}
		past->Sig.SetValue(res);
		past->Sig.MakeLogical();
	}
	else
	{
		CVar x1 = past->Sig;
		CVar x2 = past->Compute(p);
		if (!x2.IsLogical())
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "argument must be a logical variable.");
		past->Sig.Reset(1);
		past->Sig.UpdateBuffer(min(x1.nSamples, x2.nSamples));
		past->Sig.MakeLogical();
		for (unsigned k = 0; k < min(x1.nSamples, x2.nSamples); k++)
			past->Sig.logbuf[k] = x1.logbuf[k] && x2.logbuf[k];
	}
}

void _and(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->blockCell(pnode, past->Sig);
	past->blockString(pnode, past->Sig);
	past->blockComplex(pnode, past->Sig);
	if (!p) // single arg
	{
		double res(1.);
		if (past->Sig.IsLogical())
		{
			for (unsigned k = 0; k < past->Sig.nSamples; k++)
				if (!past->Sig.logbuf[k]) {	res = 0.;		break;	}
		}
		else
		{
			for (unsigned k = 0; k < past->Sig.nSamples; k++)
				if (past->Sig.buf[k] == 0.) {	res = 0.;	break;	}
		}
		past->Sig.SetValue(res);
		past->Sig.MakeLogical();
	}
	else
	{
		CVar x1 = past->Sig;
		CVar x2 = past->Compute(p);
		if (!x2.IsLogical())
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "argument must be a logical variable.");
		past->Sig.Reset(1);
		past->Sig.UpdateBuffer(min(x1.nSamples, x2.nSamples));
		past->Sig.MakeLogical();
		for (unsigned k = 0; k < min(x1.nSamples, x2.nSamples); k++)
			past->Sig.logbuf[k] = x1.logbuf[k] && x2.logbuf[k];
	}
}

#ifndef NO_FFTW

void _fft(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->blockCell(pnode, past->Sig);
	past->blockScalar(pnode, past->Sig);
	past->blockString(pnode, past->Sig);
	CVar param(0.);
	if (p)
	{
		try {
			CAstSig tp(past);
			param = tp.Compute(p);
			past->checkScalar(p, param);
			if (param.value()<1) 
				throw CAstExceptionInvalidUsage(*past, p, "argument must be a positive integer.");
		}
		catch (const CAstException &e) {
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, e.getErrMsg().c_str());
		}
	}
	past->Sig = past->Sig.runFct2getsig(&CSignal::FFT, (void*)&param);
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}

void _ifft(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->blockCell(pnode, past->Sig);
	past->blockScalar(pnode, past->Sig);
	past->blockString(pnode, past->Sig);
	CVar param(0.);
	if (p)
	{
		try {
			CAstSig tp(past);
			param = tp.Compute(p);
			past->checkScalar(p, param);
			if (param.value()<1)
				throw CAstExceptionInvalidUsage(*past, p, "argument must be a positive integer.");
		}
		catch (const CAstException &e) {
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, e.getErrMsg().c_str());
		}
	}
	past->Sig = past->Sig.runFct2getsig(&CSignal::iFFT, (void*)&param);
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap(0);
}

void _envelope(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	past->Sig.runFct2modify(&CSignal::HilbertEnv);
}

void _hilbert(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkAudioSig(pnode,  past->Sig);
	past->Sig.runFct2modify(&CSignal::Hilbert);
}

#endif //NO_FFTW

void _sort(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{ 
	CVar sig = past->Sig;
	past->checkVector(pnode, sig);
	past->blockComplex(pnode, sig);
	double order(1.);
	if (p)
	{
		past->Compute(p);
		if (!past->Sig.IsScalar())
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "2nd argument must be a scalar.");
		if (past->Sig.value() < 0) order = -1.;
	}
	past->Sig = sig.runFct2modify(&CSignal::sort, &order);
}

void _decfir(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar second = past->Compute(p->next);
	CVar third = past->Compute(p->next->next);
	if (!third.IsScalar())
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "3rd argument must be a scalar: offset");
	CVar fourth = past->Compute(p->next->next->next);
	if (!fourth.IsScalar())
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "4th argument must be a scalar: nChan");
	int offset = (int)third.value()-1; // because AUX is one-based
	int nChan = (int)fourth.value();
	if (offset>nChan)
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "nChan must be equal or greater than offset");
	past->Compute(p);
	past->Sig.DecFir(second, offset, nChan);
}

void _conv(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	//For only real (double) arrays 3/4/2019
	//p should be non NULL
	CVar sig = past->Sig;
	CVar array2 = past->Compute(p);
	past->Sig = sig.runFct2modify(&CSignal::conv, &array2);
}

void _filt(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	CVar sig = past->Sig;
	string fname = pnode->str;
	CVar fourth, third, second = past->Compute(p);
	if (p->next) {
		if (p->next->next) 
			fourth = past->Compute(p->next->next); // 4 args
		third = past->Compute(p->next); // 3 args
		if (fourth.nSamples >= third.nSamples)
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "The length of the initial condition vector must be less than the length of denominator coefficients.");
	} else {				// 2 args
		if (second.nSamples <= 1)
			throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "2nd argument must be a vector(the numerator array for filtering).");
		third.SetValue(1);
	}
	unsigned int len = max(second.nSamples, third.nSamples);
	if (second.IsScalar())
		second.UpdateBuffer(len);
	else if (third.IsScalar())
		third.UpdateBuffer(len);

	vector<double> initial;
	vector<double> num(second.buf, second.buf + second.nSamples);
	vector<double> den(third.buf, third.buf + third.nSamples);
	vector<vector<double>> coeffs;
	if (!second.chain && !third.chain && second.tmark==0 && third.tmark==0)
	{
		coeffs.push_back(num);
		coeffs.push_back(den);
		if (fourth.nSamples > 0)
		{
			for (unsigned int k = 0; k < fourth.nSamples; k++) initial.push_back(fourth.buf[k]);
			coeffs.push_back(initial);
		}
		if (fname == "filt")
			sig.runFct2modify(&CSignal::filter, &coeffs);
		else if (fname == "filtfilt")
			sig.runFct2modify(&CSignal::filtfilt, &coeffs);
		//at this point, coeffs is not the same as before (updated with the final condition)
	}
	else
	{
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "Internal error--leftover from Dynamic filtering");
	}
	past->Sig = sig;
	if (countVectorItems(past->pAst) > 1)
	{ // in this case coeffs carries the final condition array (for stereo, the size is 2)
		past->Sigs.push_back(move(make_unique<CVar*>(&past->Sig)));
		CVar *newpointer = new CVar(sig.GetFs());
		CSignals finalcondition(coeffs.back().data(), (int)coeffs.back().size()); // final condnition is stored at the last position
		*newpointer = finalcondition;
		unique_ptr<CVar*> pt = make_unique<CVar*>(newpointer);
		past->Sigs.push_back(move(pt));
	}
}

#ifndef NO_IIR
void _iir(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	int type(0), kind(1), norder(4);
	double freqs[2], rippledB(0.5), stopbandFreqORAttenDB(-40.);
	const AstNode *args = p;
	string fname = pnode->str;
	past->checkAudioSig(pnode, past->Sig);
	CVar sigX(past->Sig);
	string emsg;
	past->Compute(p);
	freqs[0] = past->Sig.value();
	p = p->next;
	if  (fname=="lpf") type=1;
	else if  (fname == "bpf") type=2; 
	else if  (fname == "hpf") type=3;
	else if  (fname == "bsf") type=4; 
	try {
		switch (type)
		{
		case 1:
		case 3:
			if (p) {
				past->Compute(p);		norder = (int)round(past->Sig.value());
				p = p->next;
				if (p) {
					past->Compute(p);	kind = (int)round(past->Sig.value());
					p = p->next;
					if (p) {
						past->Compute(p);		rippledB = past->Sig.value();
						p = p->next;
						if (p) { past->Compute(p);		stopbandFreqORAttenDB = past->Sig.value(); }
					}
				}
			}
			break;
		case 2:
		case 4:
			past->Compute(p);		freqs[1] = past->Sig.value();
			p = p->next;
			if (p) {
				past->Compute(p);	norder = (int)round(past->Sig.value());
				p = p->next;
				if (p) {
					past->Compute(p);		kind = (int)round(past->Sig.value());
					p = p->next;
					if (p) {
						past->Compute(p);		rippledB = past->Sig.value();
						p = p->next;
						if (p)
						{
							past->Compute(p);		stopbandFreqORAttenDB = past->Sig.value();
						}
					}
				}
			}
			break;
		}
		double dkind = kind;
		double dtype = type;
		double dnorder = norder;
		vector<double*> params;
		params.push_back(&dkind);
		params.push_back(&dtype);
		params.push_back(&dnorder);
		params.push_back(freqs);
		params.push_back(&rippledB);
		params.push_back(&stopbandFreqORAttenDB);

		sigX.runFct2modify(&CSignal::IIR, &params);
		past->Sig = sigX;
	}
	catch (const char* estr) 
	{
		sformat(emsg, "Invalid argument---%s\nFor more of ELLF Digital Filter Calculator, check http://www.moshier.net/index.html.", estr);
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, emsg.c_str());
	}
}
#endif // NO_IIR

void _squeeze(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->Sig.Squeeze();
}


void _hamming(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkSignal(pnode, past->Sig);
	past->Sig.runFct2modify(&CSignal::Hamming);
}

void _blackman(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkSignal(pnode, past->Sig);
	double alpha;
	string fname = pnode->str;
	if (fname=="blackman")
		alpha = .16;
	else
		alpha = 0;
	if (p) // if hann, p should be NULL, so alpha stays zero
	{
		CAstSig temp(past);
		temp.Compute(p);
		temp.checkScalar(pnode, temp.Sig);
		alpha = temp.Sig.value();
	}
	past->Sig.runFct2modify(&CSignal::Blackman, &alpha);
}

void _fm(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar fifth, second = past->Compute(p->next);
	CVar third = past->Compute(p->next->next);
	CVar fourth = past->Compute(p->next->next->next);
	if (!second.IsScalar()) throw CAstInvalidFuncSyntax(*past, p, fnsigs, "freq2 must be a scalar.");
	if (!third.IsScalar()) throw CAstInvalidFuncSyntax(*past, p, fnsigs, "mod_rate must be a scalar.");
	if (!fourth.IsScalar()) throw CAstInvalidFuncSyntax(*past, p, fnsigs, "duration must be a scalar.");
	if (fourth.value()<=0) throw CAstInvalidFuncSyntax(*past, p, fnsigs, "duration must be positive.");
	if (p->next->next->next->next) {	// 5 args
		fifth = past->Compute(p->next->next->next->next);
		if (!fifth.IsScalar()) throw CAstInvalidFuncSyntax(*past, p, fnsigs, "init_phase must be a scalar."); }
	else fifth.SetValue(0);

	past->Compute(p);
	double freq1 = past->Sig.value();
	double freq2 = second.value();
	double midFreq = (freq1+freq2)/2.;
	double width = fabs(freq1-freq2)/2.;
	double modRate = third.value();
	double initPhase = fifth.value();
	double dur = fourth.value();
	past->Sig.Reset(past->GetFs());
	if (modRate!=0.)
		past->Sig.fm(midFreq, width, modRate, dur, initPhase-.25);
	else
	{
		vector<double> freqs(2);
		freqs[0] = freq1; freqs[1] = freq2; 
		past->Sig.Tone(freqs, dur);
	}
}

void _std(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar arg(0.);
	if (p)
	{
		try {
			CAstSig tp(past);
			arg = tp.Compute(p);
			tp.checkScalar(pnode, arg);
			double flag = tp.Sig.value();
			if (flag != 0. && flag != 1.)
				throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Invalid parameter: should be either 0 (divided by n-1; default) or 1 (divided by n).");
		}
		catch (const CAstException &e) { throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, e.getErrMsg().c_str()); }
	}
	past->Sig = past->Sig.runFct2getvals(&CSignal::stdev, &arg);
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}

void _size(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar arg(0.);
	if (p)
	{
		try {
			CAstSig tp(past);
			arg = tp.Compute(p);
			tp.checkScalar(pnode, arg);
		}
		catch (const CAstException &e) { throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, e.getErrMsg().c_str()); }
	}
	double tp1 = past->Sig.nGroups;
	double tp2 = past->Sig.Len();
	if (arg.value() == 0.)
	{
		past->Sig.Reset(1);
		past->Sig.UpdateBuffer(2);
		past->Sig.buf[0] = tp1;
		past->Sig.buf[1] = tp2;
		past->Sig.nGroups = 1;
	}
	else if (arg.value() == 1.)
		past->Sig.SetValue(tp1);
	else if (arg.value() == 2.)
		past->Sig.SetValue(tp2);
	else
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "Invalid parameter: should be either 1 or 2.");
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}

void _arraybasic(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar additionalArg(past->Sig.GetFs());
	string fname = pnode->str;
	if (fname == "sum")	past->Sig = past->Sig.runFct2getvals(&CSignal::sum);
	else if (fname == "mean") past->Sig = past->Sig.runFct2getvals(&CSignal::mean);
	else if (fname == "length")
	{
		if (past->Sig.next)
		{
			if (past->Sig.next->nSamples!= past->Sig.nSamples)
				throw CAstExceptionInvalidUsage(*past, pnode, "A stereo signal with different lengths for L and R.");
		}
		if (past->Sig.IsGO())
		{
			double out = past->Sig.length().buf[0];
			past->Sig.Reset();
			past->pgo = NULL;
			past->Sig.SetValue(out);
		}
		else
		{
			if (!past->Sig.cell.empty())
				past->Sig.SetValue((double)past->Sig.cell.size());
			else if (!past->Sig.chain)
				past->Sig = past->Sig.runFct2getvals(&CSignal::length);
			else if (past->Sig.IsTimeSignal())
			{
				past->Sig.SetValue((double)(past->Sig.CountChains()));
			}
			else
			{
				throw CAstExceptionInternal(*past, pnode, "[INTERNAL] _arraybasic--if chained, IsTimeSignal() should return true.");
			}
			past->Sig.SetFs(1);
		}
	}
	else
	{
		if (past->Sig.IsEmpty()) {
			if (fname == "begint" || fname == "endt" || fname == "dur")
				past->Sig = CVar(0.);
			else
				past->Sig = CVar(-std::numeric_limits<double>::infinity());
			return;
		}
		past->checkAudioSig(pnode, past->Sig);
		if (fname == "begint") past->Sig = past->Sig.runFct2getvals(&CSignal::begint);
		else if (fname == "endt") past->Sig = past->Sig.runFct2getvals(&CSignal::endt);
		else if (fname == "dur") past->Sig = past->Sig.runFct2getvals(&CSignal::dur);
		else if (fname == "rms") past->Sig = past->Sig.runFct2getvals(&CSignal::RMS);
		else if (fname == "rmsall") past->Sig = past->Sig.RMS(); // overall RMS from artificially concatenated chain's 
	}
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}

void _mostleast(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (past->Sig.IsEmpty()) return; //for empty input, empty output
	past->checkVector(pnode, past->Sig);
	string func = pnode->str;
	CVar sig = past->Sig;
	CVar param = past->Compute(p);
	if (func == "atmost") past->Sig = sig.runFct2modify(&CSignal::_atmost, &param);
	else if (func == "atleast") past->Sig = sig.runFct2modify(&CSignal::_atleast, &param);
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}

void _minmax(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (past->Sig.IsEmpty()) return; //for empty input, empty output
	string fname = pnode->str;
	CVar sig = past->Sig;
	CVar *newpointer = new CVar(sig.GetFs());
	int nOutVars = countVectorItems(past->pAst);
	if (fname == "max") past->Sig = sig.runFct2getvals(&CSignal::_max, newpointer);
	else if (fname == "min") past->Sig = sig.runFct2getvals(&CSignal::_min, newpointer);
	if (past->Sig.IsComplex()) past->Sig.SetValue(-1); // do it again 6/2/2018
	if (nOutVars > 1)
	{
		past->Sigs.push_back(move(make_unique<CVar*>(&past->Sig)));
		unique_ptr<CVar*> pt = make_unique<CVar*>(newpointer);
		past->Sigs.push_back(move(pt));
	}
	else
		delete newpointer;
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}

void _setnextchan(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{ // current this is used only when a right channel is added to a mono (and it becomes left)
	if (past->Sig.next)
		throw CAstExceptionInvalidUsage(*past, pnode, "This function should be used only for a mono signal.");
	CVar sig = past->Sig;
	CVar param = past->Compute(p);
	if (param.next)
		throw CAstExceptionInvalidUsage(*past, pnode, "This function should be used with a mono signal argument.");
	if ( !(param.type() & TYPEBIT_TEMPORAL) && param.type()!=1)
		throw CAstExceptionInvalidUsage(*past, pnode, "Invalid argument.");
	CVar *second = new CVar;
	*second = param;
	sig.SetNextChan(second);
	past->Sig = sig;
}

AstNode *searchstr(AstNode *p, int findthis)
{ // if there's a node with "type" in the tree, return that node
	if (p)
	{
		if (p->type == findthis) return p;
		if (p->child)
			if (p->child->type == findthis) return p->child;
			else return searchstr(p->child, findthis);
		else if (p->next)
			if (p->next->type == findthis) return p->next;
			else return searchstr(p->next, findthis);
	}
	return NULL;
}

AstNode *searchstr(AstNode *p, const char* pstr)
{ // if there's a node with "type" in the tree, return that node
	if (p)
	{
		if (p->str==pstr) return p;
		if (p->child)
			if (p->child->str==pstr) return p->child;
			else return searchstr(p->child, pstr);
		else if (p->next)
			if (p->next->str==pstr) return p->next;
			else return searchstr(p->next, pstr);
	}
	return NULL;
}

int findcol(AstNode *past, const char* pstr, int line)
{
	for (AstNode *pn=past; pn; pn = pn->next)
	{
		if (pn->line<line) continue;
		AstNode *pm = searchstr(pn->child, pstr);
		if (pm) return pm->col;
		else	return -1;
	}
	return -1;
}

void _audio(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{
	past->blockString(pnode, past->Sig);
	past->blockCell(pnode, past->Sig);
	switch (past->Sig.nGroups)
	{
	case 1:
		past->Sig.SetFs(past->GetFs()); 
		break;
	case 2:
	{
		past->Sig.SetFs(past->GetFs());
		past->Sig.nSamples /= 2;
		CSignals next = CSignal(past->Sig.GetFs(), past->Sig.nSamples);
		memcpy(next.logbuf, past->Sig.logbuf + past->Sig.nSamples*past->Sig.bufBlockSize, past->Sig.nSamples*past->Sig.bufBlockSize);
		past->Sig.SetNextChan(&next);
		past->Sig.nGroups = 1;
	}
		break;
	default:
		CAstExceptionInvalidUsage(*past, p, "Cannot apply to a matrix with rows > 2.");
		break;
	}
}

void _vector(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
//	past->Sig.MakeChainless(); // if this is on, you can't easily display values from an audio obj 6/29/2020
	past->Sig.SetFs(1);
	if (past->Sig.next)
	{
		CSignal out = CSignal(1, past->Sig.nSamples * 2);
		memcpy(out.logbuf, past->Sig.logbuf + past->Sig.nSamples*past->Sig.bufBlockSize, past->Sig.nSamples*past->Sig.bufBlockSize);
		memcpy(out.logbuf + past->Sig.nSamples*past->Sig.bufBlockSize, past->Sig.next->logbuf + past->Sig.nSamples*past->Sig.bufBlockSize, past->Sig.nSamples*past->Sig.bufBlockSize);
		out.nGroups = 2;
		past->Sig = (CVar)(CSignals)out;
	}
}

void _ramp(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	CVar param;
	try {
		CAstSig tp(past);
		tp.Compute(p);
		param = tp.Compute(p);
	}
	catch (const CAstException &e) { throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, e.getErrMsg().c_str()); }
	if (!param.IsScalar())
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Ramp_duration must be a scalar.");
	double ramptime = param.value();
	past->Sig.runFct2modify(&CSignal::dramp, &ramptime);
}

void _sam(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{
	double modRate, amDepth(1.), initPhase(0.);
	CVar rate, depth, initphase;
	past->checkAudioSig(pnode, past->Sig);
	try {
		CAstSig tp(past);
		rate = tp.Compute(p);
		if (!rate.IsScalar())
			throw CAstInvalidFuncSyntax(*past, p, fnsigs, "AM_rate must be a scalar.");
		modRate = rate.value();
		int nArgs(0);
		for (const AstNode *cp = p; cp; cp = cp->next)
			++nArgs;
		if (nArgs >= 2) //either 2 or 3
		{
			depth = tp.Compute(p->next);
			if (!depth.IsScalar())
				throw CAstInvalidFuncSyntax(*past, p, fnsigs, "AM_depth_m must be a scalar.");
			amDepth = depth.value();
			if (nArgs == 3)
			{
				initphase = tp.Compute(p->next->next);
				if (!initphase.IsScalar())
					throw CAstInvalidFuncSyntax(*past, p, fnsigs, "initial_phase must be a scalar.");
				initPhase = initphase.value();
			}
		}
	}
	catch (const CAstException &e) { throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, e.getErrMsg().c_str()); }
	past->Sig.SAM(modRate, amDepth, initPhase);
}

void _left(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	delete past->Sig.next;
	past->Sig.next = NULL;
}

void _right(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	CTimeSeries *right = past->Sig.DetachNextChan();
	if (right == NULL) {
		past->Sig.Reset();
		return;
	}
	past->Sig.SwapContents1node(*right);
	delete right;	// deleting left channel since 'right' now points to the left channel
	delete past->Sig.next;
	past->Sig.next = NULL;
}

void _erase(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (!past->Sig.IsStruct())
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Must be applied to a class/struct object.");
	try {
		CAstSig tp(past);
		CVar param = tp.Compute(p);
		if (!param.IsString())
			throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Argument must be a string indicating the member of the class/struct object.");
		auto it = past->Sig.strut.find(param.string());
		if (it != past->Sig.strut.end())
			past->Sig.strut.erase(it);
		const AstNode *pRoot = past->findParentNode(past->pAst, (AstNode*)pnode, true);
		past->SetVar(pRoot->str, &past->Sig);
	}
	catch (const CAstException &e) { throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, e.getErrMsg().c_str()); }
}

void _head(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (!past->Sig.IsStruct())
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Must be applied to a class/struct object.");
	try {
		CAstSig tp(past);
		CVar param = tp.Compute(p);
		past->Sig.set_class_head(param);
	}
	catch (const CAstException &e) { throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, e.getErrMsg().c_str()); }
	const AstNode *pRoot = past->findParentNode(past->pAst, (AstNode*)pnode, true);
	past->SetVar(pRoot->str, &past->Sig);
}

void _tone(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	double initPhase(0.);
	int len, nArgs(0);
	for (const AstNode *cp = p; cp; cp = cp->next)
		++nArgs;
	if (!past->Sig.IsScalar() && (!past->Sig.IsVector() || past->Sig.nSamples>2))
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Frequency must be either a constant or two-element array.");
	body freq = past->Sig; //should be same as tp.Compute(p);
	if (freq._max().front() >= past->GetFs() / 2)
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Frequency exceeds Nyquist frequency.");
	CVar duration = past->Compute(p->next);
	past->checkScalar(pnode, duration);
	if (duration.value() <= 0)
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Duration must be positive.");
	if (nArgs == 3)
	{
		CVar _initph = past->Compute(p->next->next);
		if (!_initph.IsScalar())
			throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Initial_phase must be a scalar.");
		initPhase = _initph.value();
	}
	if ( (len = freq.nSamples )== 1)
	{
		past->Sig.Reset(past->GetFs());
		unsigned int nSamplesNeeded = (unsigned int)round(duration.value() / 1000.*past->GetFs());
		past->Sig.Tone(freq.value(), nSamplesNeeded, initPhase);
	}
	else
	{
		vector<double> freqs(len);
		memcpy((void*)&freqs[0], (void*)freq.buf, len * sizeof(double));
		past->Sig.Reset(past->GetFs());
		past->Sig.Tone(freqs, duration.value()); // For now, initPhase is ignored when a two-element freq is given. 2/21
	}
}

void _tparamonly(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar dur = past->Sig;
	if (!dur.IsScalar())
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "duration must be a scalar.");
	if (dur.value()<0.)
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "duration must be a non-negative number.");
	past->Sig.SetFs(past->GetFs());
	if (!strcmp(pnode->str,"noise"))
		past->Sig.Noise(dur.value());
	else if (!strcmp(pnode->str, "gnoise"))
		past->Sig.Noise2(dur.value());
	else if (!strcmp(pnode->str, "silence"))
		past->Sig.Silence(dur.value());
	else if (!strcmp(pnode->str, "dc"))
		past->Sig.DC(dur.value());
	else
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Internal error 3426.");
}
void _tsq_isrel(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{
	int type = past->Sig.GetType();
	bool res = (type == CSIG_TSERIES) && (past->Sig.GetFs() == 0);
	double dres = res ? 1. : 0.;
	past->Sig.SetValue(dres);
}

void _tsq_gettimes(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{
	past->checkTSeq(pnode, past->Sig);
	//get the item count; i.e., the number of chains
	int nItems = past->Sig.CountChains();
	double *dbuf = new double[nItems];
	int k = 0;
	int nChains=1;
	bool relative = past->Sig.GetFs() == 0;
	for (CTimeSeries *q = &past->Sig; q; q = q->chain)
		dbuf[k++] = q->tmark;
	if (p)
	{
		past->Compute(p);
		past->checkAudioSig(pnode, past->Sig);
		if (past->Sig.next)
			throw CAstExceptionInvalidUsage(*past, pnode, "Cannot be a stereo audio signal--just for now...");
		if (relative)
		{
			nChains = past->Sig.CountChains();
			double *newdbuf = new double[nItems*nChains];
			for (int k = 0; k < nChains; k++)
				memcpy(newdbuf + k * nItems, dbuf, sizeof(double)*nItems);
			delete[] dbuf;
			dbuf = newdbuf;
			int id = 0;
			for (CTimeSeries *p = &past->Sig; p; p = p->chain)
			{
				double durseg = p->dur().front();
				for (int k = 0; k < nItems; k++)
				{
					dbuf[id*nItems + k] *= durseg;
					dbuf[id*nItems + k] += p->tmark;
				}
				id++;
			}
		}
		else
		{
			//Now, check the audio is on at each time point.. if not remove the time point
			double *newdbuf = new double[nItems*nChains];
			int count = 0;
			for (int k = 0; k < nItems; k++)
			{
				if (past->Sig.IsAudioOnAt(dbuf[k]))
					newdbuf[count++] = dbuf[k];
			}
			delete[] dbuf;
			dbuf = newdbuf;
			nItems = count;
		}
		past->Sig.Reset(1);
	}
//	else
//		past->Sig.SetFs(0); // setting fs=0 to indicate relative time points... this is a temporary hack. 3/10/2019
	past->Sig.Reset(1);
	past->Sig.UpdateBuffer(nItems*nChains);
	past->Sig.nGroups = 1;// nChains;
	memcpy(past->Sig.buf, dbuf, sizeof(double)*past->Sig.nSamples);
	delete[] dbuf;
}

void _tsq_settimes(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{
	past->checkTSeq(pnode, past->Sig);
	int nItems = past->Sig.CountChains();
	try {
		CAstSig tp(past);
		CVar newtime = tp.Compute(p);
		if (newtime.GetType()!=CSIG_VECTOR)
			throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Argument must be a vector of time points.");
		if (newtime.nSamples!=nItems)
			throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Argument vector must have the same number of elements as the TSEQ.");
		int id = 0;
		for (CTimeSeries *p = &past->Sig; p; p = p->chain)
		{
			p->tmark = newtime.buf[id++];
			if (newtime.GetFs() == 0) p->SetFs(0);
		}
	}
	catch (const CAstException &e) { throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, e.getErrMsg().c_str()); }
	const AstNode *pRoot = past->findParentNode(past->pAst, (AstNode*)pnode, true);
	past->SetVar(pRoot->str, &past->Sig);
}

void _tsq_getvalues(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{
	past->checkTSeq(pnode, past->Sig);
	int k=0, nItems = past->Sig.CountChains();
	CTimeSeries out(1);
	out.UpdateBuffer(nItems * past->Sig.nSamples);
//	out.nGroups = nItems;
	for (CTimeSeries *p = &past->Sig; p; p = p->chain)
		memcpy(out.buf + k++ * p->nSamples, p->buf, sizeof(double)*p->nSamples); // assuming that p->nSamples is always the same
	past->Sig = out;
}

void _tsq_setvalues(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{
	past->checkTSeq(pnode, past->Sig);
	int nItems = past->Sig.CountChains();
	try {
		CAstSig tp(past);
		CVar newvalues = tp.Compute(p);
		if (newvalues.GetType() != CSIG_VECTOR)
			throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Argument must be a vector.");
		if (newvalues.nGroups!= nItems)
			throw CAstInvalidFuncSyntax(*past, p, fnsigs, "Argument vector must have the same number of groups as the TSEQ length.");
		int id = 0;
		for (CTimeSeries *p = &past->Sig; p; p = p->chain)
		{
			p->UpdateBuffer(newvalues.Len());
			memcpy(p->buf, newvalues.buf + id++ * newvalues.Len(), sizeof(double)*newvalues.Len());
		}
	}
	catch (const CAstException &e) { throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, e.getErrMsg().c_str()); }
	const AstNode *pRoot = past->findParentNode(past->pAst, (AstNode*)pnode, true);
	past->SetVar(pRoot->str, &past->Sig);
}

void _imaginary_unit(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	complex<double> x(0, 1);
	past->Sig.SetValue(x);
}

void _pi(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->Sig.SetValue(asin(1)*2);
}

void _natural_log_base(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->Sig.SetValue(exp(1));
}

typedef  map<string, Cfunction>(_cdecl  *PF) ();

void CAstSigEnv::InitBuiltInFunctionsExt(const char *dllname)
{
	HANDLE hLib = LoadLibrary(dllname);
	if (hLib)
	{
		PF pt = (PF)GetProcAddress((HMODULE)hLib, (LPCSTR)MAKELONG(1, 0)); // Init()
		map<string, Cfunction> res = pt();
		for (auto it = res.begin(); it != res.end(); it++)
		{
			builtin[(*it).first] = (*it).second;
		}
	}
}
void CAstSigEnv::InitBuiltInFunctions(HWND h)
{
#ifndef NO_PLAYSND
	hShowDlg = h;
#endif
	srand((unsigned)time(0) ^ (unsigned int)GetCurrentThreadId());

	string name;
	Cfunction ft;

	name = "tone";
	ft.alwaysstatic = true;
	ft.funcsignature = "(frequency, duration_in_ms [, initial_phase_between_0&1=0])";
	ft.narg1 = 2;	ft.narg2 = 3;
	ft.func = &_tone;
	builtin[name] = ft;

	name = "sam";
	ft.alwaysstatic = false;
	ft.funcsignature = "(signal, AM_rate[, AM_depth_m = 1, initial_phase_between_0 & 1 = 0])";
	ft.narg1 = 2;	ft.narg2 = 4;
	ft.func =  &_sam;
	builtin[name] = ft;

	name = "fm";
	ft.alwaysstatic = true;
	ft.funcsignature = "(freq1, freq2, mod_rate, duration, [init_phase=0])";
	ft.narg1 = 4;	ft.narg2 = 5;
	ft.func =  &_fm;
	builtin[name] = ft;

	ft.funcsignature = "(duration_in_ms)";
	ft.alwaysstatic = true;
	ft.narg1 = 1;	ft.narg2 = 1;
	const char *f0[] = { "noise", "gnoise", "silence", "dc",0 };
	for (int k = 0; f0[k]; k++)
	{
		name = f0[k];
		ft.func =  &_tparamonly;
		builtin[name] = ft;
	}

	ft.narg1 = 2;	ft.narg2 = 3;
	ft.alwaysstatic = true;
	name = ":";
	ft.funcsignature = "(scalar:scalar) or (scalar:scalar:scalar)";
	ft.func =  &_colon; 
	builtin[name] = ft;

	name = "wave";
	ft.alwaysstatic = true;
	ft.funcsignature = "(filename)";
	ft.narg1 = 1;	ft.narg2 = 1;
	ft.func =  &_wave;
	builtin[name] = ft;

	name = "wavwrite";
	ft.alwaysstatic = false;
	ft.funcsignature = "(audio_signal, filename[, option])";
	ft.narg1 = 2;	ft.narg2 = 3;
	ft.func =  &_wavwrite;
	builtin[name] = ft;

	name = "write";
	ft.alwaysstatic = false;
	ft.funcsignature = "(audio_signal, filename[, option])";
	ft.narg1 = 2;	ft.narg2 = 3;
	ft.func = &_write;
	builtin[name] = ft;

	name = "setfs"; // check this... is narg1 one correct?
	ft.alwaysstatic = true;
	ft.funcsignature = "(filename)";
	ft.narg1 = 1;	ft.narg2 = 1;
	ft.func =  &_setfs;
	builtin[name] = ft;

	name = "audio";
	ft.alwaysstatic = false;
	ft.funcsignature = "(non_audio_vector)";
	ft.narg1 = 1;	ft.narg2 = 1;
	ft.func =  &_audio;
	builtin[name] = ft;

	name = "squeeze"; 
	ft.alwaysstatic = false;
	ft.funcsignature = "() ... to remove the null interval";
	ft.narg1 = 0;	ft.narg2 = 0;
	ft.func =  &_squeeze; // do this
	builtin[name] = ft;

	// begin narg 2 and 2
	ft.alwaysstatic = false;
	ft.narg1 = 2;
	ft.narg2 = 2;

	name = "ramp";
	ft.funcsignature = "(signal, ramp_duration)";
	ft.func =  &_ramp;
	builtin[name] = ft;

	name = "isaudioat";
	ft.funcsignature = "(audio_signal, time_pt)";
	ft.func =  &_isaudioat;
	builtin[name] = ft;

	// end narg 2 and 2

	ft.narg1 = 1;	ft.narg2 = 1;
	name = "hamming";
	ft.funcsignature = "(audio_signal)";
	ft.func = &_hamming;
	builtin[name] = ft;
	name = "hann";
	ft.func = &_blackman;
	builtin[name] = ft;
	ft.narg1 = 1;	ft.narg2 = 2;
	name = "blackman";
	ft.funcsignature = "(audio_signal, [alpha=0.16])";
	ft.func = &_blackman;
	builtin[name] = ft;

#ifndef NO_IIR
	ft.narg1 = 2;	ft.narg2 = 6;
	const char *f2[] = { "lpf", "hpf", 0 };
	ft.funcsignature = "(signal, freq, [order=4], [kind=1], [dB_passband_ripple=0.5], [dB_stopband_atten=-40])\n  --- kind: 1 for Butterworth, 2 for Chebyshev, 3 for Elliptic";
	for (int k = 0; f2[k]; k++)
	{
		name = f2[k];
		ft.func =  &_iir;
	builtin[name] = ft;
	}
	ft.narg1 = 3;	ft.narg2 = 7;
	const char *f3[] = { "bpf", "bsf", 0 };
	ft.funcsignature = "(signal, freq, [order=4], [kind=1], [dB_passband_ripple=0.5], [dB_stopband_atten=-40])\n  --- kind: 1 for Butterworth, 2 for Chebyshev, 3 for Elliptic";
	for (int k = 0; f3[k]; k++)
	{
		name = f3[k];
		ft.func =  &_iir;
	builtin[name] = ft;
	}
#endif //NO_IIR
	ft.narg1 = 1;	ft.narg2 = 1;
	ft.funcsignature = "(stereo_signal)";
	name = "left";
	ft.func =  &_left; // check 
	builtin[name] = ft;
	name = "right";
	ft.func =  &_right; // check 
	builtin[name] = ft;

	ft.narg1 = 1;	ft.narg2 = 2;
	name = "std";
	ft.funcsignature = "(length, [, bias])";
	ft.func =  &_std; 
	builtin[name] = ft;
	
	ft.narg1 = 2;	ft.narg2 = 2;
	name = "setnextchan";
	ft.func = &_setnextchan;
	builtin[name] = ft;
	const char *f4[] = { "atleast", "atmost", 0 };
	ft.funcsignature = "(array_or_value, array_or_value_of_limit)";
	for (int k = 0; f4[k]; k++)
	{
		name = f4[k];
		ft.func = &_mostleast;
	builtin[name] = ft;
	}
	ft.funcsignature = "(obj, member_variable_string)";
	name = "erase";
	ft.func = &_erase;
	builtin[name] = ft;
	ft.funcsignature = "(obj, member_variable_string)";
	name = "ismember";
	ft.func = &_ismember;
	builtin[name] = ft;
	ft.funcsignature = "(obj, any_non-cell_non-class_expression)";
	name = "head";
	ft.func = &_head;
	builtin[name] = ft;

	ft.narg1 = 1;	ft.narg2 = 1;
	const char *f5[] = { "min", "max", 0 };
	ft.funcsignature = "(vector) or (scalar1, scalar2, ...)";
	for (int k = 0; f5[k]; k++)
	{
		name = f5[k];
		ft.func =  &_minmax;
	builtin[name] = ft;
	}

	ft.narg1 = 1;	ft.narg2 = 1;
	ft.funcsignature = "(array)";
	const char *f6[] = { "sum", "mean", "length", 0 };
	for (int k = 0; f6[k]; k++)
	{
		name = f6[k];
		ft.func = _arraybasic;
	builtin[name] = ft;
	}

	ft.narg1 = 1;	ft.narg2 = 2;
	ft.funcsignature = "(array [, dimension_either_1_or_2])";
	name = "size";
	ft.func = &_size;
	builtin[name] = ft;

	ft.narg1 = 1;	ft.narg2 = 1;
	ft.funcsignature = "(graphic_handle)";
	name = "replicate";
	ft.func = &_replicate;
	builtin[name] = ft;

	ft.narg1 = 1;	ft.narg2 = 2;
	ft.funcsignature = "(array_or_signal[, order=1])";
	name = "diff";
	ft.func =  &_diff;
	builtin[name] = ft;

	ft.narg1 = 2;	ft.narg2 = 2;
	ft.funcsignature = "(audio_signal)";
	name = "nullin";
	ft.func = &_nullin;
	builtin[name] = ft;
	ft.narg1 = 1;	ft.narg2 = 1;
	name = "contig";
	ft.func = &_contig;
	builtin[name] = ft;

	ft.narg1 = 1;	ft.narg2 = 1;
	ft.funcsignature = "(array_or_signal)";
	name = "cumsum";
	ft.func = &_cumsum;
	builtin[name] = ft;

	ft.funcsignature = "(value)";
	name = "rand";
	ft.func =  &_rand;
	builtin[name] = ft;
	name = "irand";
	ft.func =  &_irand;
	builtin[name] = ft;
	name = "randperm";
	ft.func =  &_randperm;
	builtin[name] = ft;
	name = "zeros";
	ft.func =  &_zeros;
	builtin[name] = ft;
	name = "ones";
	ft.func =  &_ones;
	builtin[name] = ft;
	name = "cell";
	ft.func =  &_cell;
	builtin[name] = ft;
	name = "clear";
	ft.func = &_clear;
	builtin[name] = ft;

	ft.narg1 = 2;	ft.narg2 = 2;
	name = "matrix";
	ft.funcsignature = "(array, nGroups)";
	ft.func =  &_matrix;
	builtin[name] = ft;

	ft.narg1 = 2;	ft.narg2 = 2;
	name = "respeed";
	ft.funcsignature = "(audio_signal, playback_rate_change_ratio)";
	ft.func = &_time_freq_manipulate;
	builtin[name] = ft;

	ft.narg1 = 3;	ft.narg2 = 3;
	name = "interp";
	ft.funcsignature = "(refX, refY, query_x_points)";
	ft.func = &_interp1;
	builtin[name] = ft;

	ft.alwaysstatic = true;
	ft.narg1 = 2;	ft.narg2 = 0;
	ft.funcsignature = "(format_string, ...)";
	name = "sprintf";
	ft.func =  &_sprintf;
	builtin[name] = ft;
	name = "fprintf";
	ft.alwaysstatic = false;
	ft.func =  &_fprintf;
	builtin[name] = ft;

	ft.narg1 = 2;	ft.narg2 = 3;
	name = "fread";
	ft.func = &_fread;
	builtin[name] = ft;
	ft.narg1 = 3;	ft.narg2 = 3;
	name = "fwrite";
	ft.func = &_fwrite;
	builtin[name] = ft;

	ft.narg1 = 1;	ft.narg2 = 1;
	ft.funcsignature = "(file_ID)";
	name = "fclose";
	ft.func =  &_fclose;
	builtin[name] = ft;

	ft.narg1 = 2;	ft.narg2 = 2;
	name = "fopen";
	ft.alwaysstatic = true;
	ft.funcsignature = "(filename, mode)";
	ft.func = &_fopen; // check 
	builtin[name] = ft;

	ft.narg1 = 0;	ft.narg2 = 0;
	name = "getfs";
	ft.funcsignature = "()";
	ft.func =  &_getfs;
	builtin[name] = ft;
	name = "tic";
	ft.func = &_tictoc;
	builtin[name] = ft;
	name = "toc";
	ft.func = &_tictoc;
	builtin[name] = ft;

	ft.narg1 = 1;	ft.narg2 = 1;
	ft.funcsignature = "(filename)";
	name = "file";
	ft.func =  &_file;
	builtin[name] = ft;
	name = "include";
	ft.func =  &_include; // check
	builtin[name] = ft;
	name = "fdelete";
	ft.func =  &_fdelete; // check
	builtin[name] = ft;

	ft.funcsignature = "(directory_name)";
	name = "dir";
	ft.func =  &_dir;
	builtin[name] = ft;

	ft.alwaysstatic = false;
	ft.narg1 = 1;	ft.narg2 = 1;
	ft.funcsignature = "(object)";
	const char *f7[] = { "isempty", "isaudio", "isvector", "isstring", "isstereo", "isbool", "iscell", "isclass", "istseq", 0 };
	for (int k = 0; f7[k]; k++)
	{
		name = f7[k];
		ft.func = _varcheck;
		builtin[name] = ft;
	}
	ft.narg1 = 1;	ft.narg2 = 1;
	name = "dtype";
	ft.func = _datatype;
	builtin[name] = ft;
	ft.alwaysstatic = true;
	ft.narg1 = 2;	ft.narg2 = 2;
	name = "issame";
	ft.func = _veq;
	builtin[name] = ft;

	ft.alwaysstatic = false;
	ft.narg1 = 2;	ft.narg2 = 4;
	ft.funcsignature = "(signal, Numerator_array [, Denominator_array=1 for FIR])";
	const char *f8[] = { "filt", "filtfilt", 0 };
	for (int k = 0; f8[k]; k++)
	{
		name = f8[k];
		ft.func = _filt;
		builtin[name] = ft;
	}
#ifndef NO_FFTW

	ft.narg1 = 1;	ft.narg2 = 2;
	name = "fft";
	ft.funcsignature = "(array, [Num_of_FFT_pts=length_of_the_input])";
	ft.func =  &_fft; // check
	builtin[name] = ft;
	name = "ifft";
	ft.func = &_ifft;
	builtin[name] = ft;

	ft.narg1 = 1;	ft.narg2 = 1;
	ft.funcsignature = "(signal_or_vector)";
	name = "envelope";
	ft.func =  &_envelope;
	builtin[name] = ft;
	name = "hilbert";
	ft.func =  &_hilbert;
	builtin[name] = ft;

	ft.narg1 = 2;	ft.narg2 = 2;
	name = "movespec";
	ft.funcsignature = "(audio_signal, frequency_to_shift)";
	ft.func = &_time_freq_manipulate;
	builtin[name] = ft;

	name = "conv";
	ft.funcsignature = "(array1, array2)";
	ft.func = &_conv;
	builtin[name] = ft;
#endif

	ft.narg1 = 1;	ft.narg2 = 2;
	ft.funcsignature = "(signal_or_vector [, positive_for_acending_negative_for_descending = 1])";
	name = "sort";
	ft.func =  &_sort;
	builtin[name] = ft;

#ifndef NO_PLAYSND
	ft.alwaysstatic = false;
	ft.narg1 = 1;	ft.narg2 = 1;
	ft.funcsignature = "(audio_handle)";
	name = "pause";
	ft.func = &_pause_resume;
	builtin[name] = ft;
	name = "resume";
	ft.func = &_pause_resume;
	builtin[name] = ft;
	name = "stop";
	ft.func = &_stop;
	builtin[name] = ft;
	name = "qstop";
	ft.func = &_stop;
	builtin[name] = ft;

	ft.funcsignature = "(audio_signal [, repeat=1]) or (audio_handle, audio_signal [, repeat=1])";
	ft.narg1 = 1;	ft.narg2 = 3;
	name = "play";
	ft.func =  &_play;
	builtin[name] = ft;
	ft.funcsignature = "(deviceID, recording_duration [=-1;indefinite], mono1_or_stereo2 [=1], callback_duration_ms [=setting in ini])";
	ft.narg1 = 1;	ft.narg2 = 4;
	name = "record";
	ft.func = &_record;
#endif
	builtin[name] = ft;
	ft.funcsignature = "(audio_signal)";
	ft.alwaysstatic = false;
	ft.narg1 = 1;	ft.narg2 = 1;
	name = "vector";
	ft.func =  &_vector;
	builtin[name] = ft;
	name = "dur";
	ft.func = &_arraybasic;
	builtin[name] = ft;
	name = "begint";
	ft.func = &_arraybasic;
	builtin[name] = ft;
	name = "endt";
	ft.func = &_arraybasic;
	builtin[name] = ft;
	name = "rms";
	ft.func = &_arraybasic;
	builtin[name] = ft;
	name = "rmsall";
	ft.func = &_arraybasic;
	builtin[name] = ft;

	ft.alwaysstatic = false;
	ft.narg1 = 1;	ft.narg2 = 1;
	name = "tsq_isrel";
	ft.funcsignature = "(tseq)";
	ft.func = &_tsq_isrel;
	builtin[name] = ft;

	ft.narg1 = 1;	ft.narg2 = 2;
	name = "tsq_gettimes";
	ft.funcsignature = "(tseq [, audio_signal])";
	ft.func = &_tsq_gettimes;//
	builtin[name] = ft;

	ft.narg1 = 2;	ft.narg2 = 2;
	name = "tsq_settimes";
	ft.funcsignature = "(tseq, times_vector_or_tseq)";
	ft.func = &_tsq_settimes;
	builtin[name] = ft;

	ft.narg1 = 1;	ft.narg2 = 1;
	name = "tsq_getvalues";
	ft.funcsignature = "(tseq)";
	ft.func = &_tsq_getvalues;
	builtin[name] = ft;

	ft.narg1 = 2;	ft.narg2 = 2;
	name = "tsq_setvalues";
	ft.funcsignature = "(tseq, values_matrix)";
	ft.func = &_tsq_setvalues;
	builtin[name] = ft;

	ft.alwaysstatic = true;
	name = "msgbox";
	ft.narg1 = 1;	ft.narg2 = 0;
	ft.funcsignature = "(msg_body_with_printf_style)";
	ft.func =  &_msgbox;
	builtin[name] = ft;

	ft.alwaysstatic = false;
	name = "inputdlg";
	ft.narg1 = 2;	ft.narg2 = 99;
	ft.funcsignature = "(title, msg_body)";
	ft.func =  &_inputdlg;
	builtin[name] = ft;

	ft.alwaysstatic = false;
	ft.narg1 = 1;	ft.narg2 = 2;
	ft.funcsignature = "(logical_variable) or (logical_variable1, logical_variable2) ";
	name = "and";
	ft.func =  &_and;
	builtin[name] = ft;
	name = "or";
	ft.func =  &_or;
	builtin[name] = ft;

	ft.alwaysstatic = false;
	ft.narg1 = 1;	ft.narg2 = 1;
	ft.funcsignature = "(string)";
	name = "str2num";
	ft.func =  &_str2num;
	builtin[name] = ft;
	ft.alwaysstatic = true;
	name = "eval";
	ft.func =  &_eval;
	builtin[name] = ft;

	ft.narg1 = 1;	ft.narg2 = 1;
	ft.funcsignature = "(value_or_array)";
	const char *fmath1[] = { "abs", "sqrt", "conj", "real", "imag", "angle", "sign", "sin", "cos", "tan", "asin", "acos", "atan", "log", "log10", "exp", "round", "fix", "ceil", "floor", 0 };
	for (int k = 0; fmath1[k]; k++)
	{
		name = fmath1[k];
		ft.func =  NULL;
		builtin[name] = ft;
	}
	ft.narg1 = 2;	ft.narg2 = 2;
	ft.funcsignature = "(value_or_array, value_or_array)";
	const char *fmath2[] = { "^", "mod", "pow", 0 };
	for (int k = 0; fmath2[k]; k++)
	{
		name = fmath2[k];
		ft.func =  NULL;
		builtin[name] = ft;
	}

#ifdef _WINDOWS
	ft.alwaysstatic = true;
	ft.narg1 = 0;	ft.narg2 = 1;
	name = "figure";
	ft.funcsignature = "([position(screen_coordinate)]) or (existing_Figure_ID)";
	ft.func =  &_figure;
	builtin[name] = ft;

	ft.alwaysstatic = false;
	ft.narg1 = 1;	ft.narg2 = 2;
	name = "axes";
	ft.funcsignature = "([position]) or (existing_axes_ID)";
	ft.func =  &_axes;
	builtin[name] = ft;

	ft.narg1 = 3;	ft.narg2 = 4;
	ft.funcsignature = "([graphic_handle], x, y, string)";
	name = "text";
	ft.func = &_text;
	builtin[name] = ft;

	ft.narg1 = 1;	ft.narg2 = 4;
	ft.funcsignature = "([graphic_handle], variable [, options]) or (vector_x, vector_y [, options])";
	name = "plot";
	ft.func = &_plot;
	builtin[name] = ft;
	name = "line";
	ft.func = &_line;
	builtin[name] = ft;

	name = "delete";
	ft.narg1 = 1;	ft.narg2 = 1;
	ft.alwaysstatic = false;
	ft.funcsignature = "(graphic_handle)";
	ft.func = &_delete_graffy;
	builtin[name] = ft;	
	name = "repaint";
	ft.funcsignature = "(graphic_handle)";
	ft.func = &_repaint;
	builtin[name] = ft;
	name = "showrms";
	ft.funcsignature = "(graphic_handle)";
	ft.func = &_showrms;
	builtin[name] = ft;
#endif // _WINDOWS

	name = "input";
	ft.alwaysstatic = false;
	ft.funcsignature = "(prompt_message)";
	ft.func = &aux_input;
	builtin[name] = ft;

	name = "error";
	ft.alwaysstatic = false;
	ft.narg1 = ft.narg2 = 1;
	ft.funcsignature = "(message)";
	ft.func = &udf_error;
	builtin[name] = ft;
	name = "warning";
	ft.func = &udf_warning;
	builtin[name] = ft;
	name = "throw";
	ft.func = &udf_rethrow;
	builtin[name] = ft;


	name = "i";
	ft.alwaysstatic = false;
	ft.narg1 = ft.narg2 = 0;
	ft.funcsignature = "";
	ft.func = &_imaginary_unit;
	pseudo_vars[name] = ft;
	name = "e";
	ft.func = &_natural_log_base;
	pseudo_vars[name] = ft;
	name = "pi";
	ft.func = &_pi;
	pseudo_vars[name] = ft;

	//name = "gca";
	//pseudo_vars[name] = pp;
	//inFunc[name] = &_gca;
	//name = "sel";
	//pseudo_vars[name] = pp;
	//inFunc[name] = &_sel;
}

void firstparamtrim(string &str)
{
	size_t parenth1 = str.find('(');
	size_t parenth2 = str.find(')');
	size_t comma = str.find(',', parenth1);
	string tocopy(")");
	if (comma != string::npos)
		tocopy = str.substr(comma + 2);
	str.replace(str.begin() + parenth1 + 1, str.end(), tocopy);
	str.replace(str.begin() + 1, str.end(), str);
	str[0] = '.';
}

complex<double> r2c_sqrt(complex<double> x) { return sqrt(x); }
complex<double> r2c_log(complex<double> x) { return log(x); }
complex<double> r2c_log10(complex<double> x) { return log10(x); }
complex<double> r2c_pow(complex<double> x, complex<double> y) { return pow(x, y); }

void aux_HOOK(CAstSig *past, const AstNode *pnode, const AstNode *p, int nArgs, string &fnsigs);

bool isgraphicfunc(string fname)
{
	if (fname == "figure") return true;
	if (fname == "text") return true;
	if (fname == "delete") return true;
	if (fname == "plot") return true;
	if (fname == "axes") return true;
	return false;
}

bool CAstSig::HandlePseudoVar(const AstNode *pnode)
{
	string fname = pnode->str;
	string dummy;
	auto it = pEnv->pseudo_vars.find(fname);
	if (it != pEnv->pseudo_vars.end())
		(*it).second.func(this, pnode, NULL, dummy);
	else
		return false;
	return true;
}

string dotstring(const AstNode *pnode, AstNode *pRoot)
{
	string out;
	AstNode *p = pRoot;
	while (p)
	{
		if (!out.empty()) out += '.';
		out += p->str;
		p = p->alt;
		if (p == pnode) break;
	}
	return out;
}

void CAstSig::HandleAuxFunctions(const AstNode *pnode, AstNode *pRoot)
{
	string fnsigs;
	int res, nArgs;
	double(*fn0)(double) = NULL;
	double(*fn1)(double) = NULL;
	double(*fn2)(double, double) = NULL;
	double(*cfn0)(complex<double>) = NULL;
	complex<double>(*cfn1)(complex<double>) = NULL;
	complex<double>(*cfn2)(complex<double>, complex<double>) = NULL;
	string fname = pnode->str;
	auto ft = pEnv->builtin.find(fname);
	bool structCall = pnode->type == N_STRUCT;
	AstNode * p(NULL);
	if (pnode->alt)
	{
		if (pnode->alt->type == N_ARGS)
			p = pnode->alt->child;
		else if (pnode->alt->type == N_HOOK)
			p = pnode->alt;
	}
	bool compl;
	if (structCall)
		compl = Sig.IsComplex(); // Sig was prepared prior to this call.
	else
	{
		Compute(p);
		compl = Sig.IsComplex();
	}
	res = HandleMathFunc(compl, fname, &fn0, &fn1, &fn2, &cfn0, &cfn1, &cfn2);
	//HandleMathFunc returns 1 for sqrt, log and log10, 2 for ^ and mode, -1 for functions for complex numbers (fn2 available), 10 for any other math functions; 0 otherwise (not a math function on my list)
	
	// To do-----6/3/2018
	//1) Groupizing this part
	//2) _minmax return complex if complex...

	CVar param, arg;
	// if Sig is real, negative forbid functions are checked here and proper function pointers are set... really??? 8/21/2018
	switch (res)
	{
	case 1: //single inarg, single outarg
		if (fname == "sqrt")		fn0 = sqrt, cfn1 = r2c_sqrt;
		else if (fname == "log")	fn0 = log, cfn1 = r2c_log;
		else if (fname == "log10")	fn0 = log10, cfn1 = r2c_log10;
		if (structCall)
		{
			if (p && p->type != N_STRUCT) throw CAstExceptionInvalidUsage(*this, pnode, "Superfluous parameter(s) ", fname.c_str());
		}
		else
		{
			if (!p) throw CAstExceptionInvalidUsage(*this, pnode, "Empty parameter");
			Compute(p);
		}
		if (Sig.IsStruct() || !Sig.cell.empty() || Sig.GetFs() == 3)
		{
			if (structCall)
			{
				string dotnotation = dotstring(pnode, pRoot);
				throw CAstExceptionInvalidUsage(*this, pnode, "Cannot take cell, class or handle object ", dotnotation.c_str());
			}
			else
				throw CAstExceptionInvalidUsage(*this, p, "Cannot take cell, class or handle object ", fname.c_str());
		}
		Sig.MFFN(fn0, cfn1);
		break;
	case 2: // two in args, single out arg
		if (fname == "^" || fname == "pow")	fn2 = pow, cfn2 = r2c_pow;
		else if (fname == "mod")	fn2 = fmod, cfn2 = NULL; // do this
		if (structCall)
		{
			if (!p || p->type == N_STRUCT) throw CAstExceptionInvalidUsage(*this, pnode, "requires second parameter.");
			CVar Sig0 = Sig;
			if (Sig.IsStruct() || !Sig.cell.empty() || Sig.GetFs() == 3)
				throw CAstExceptionInvalidUsage(*this, pnode, "Cannot take cell, class or handle object ", fname.c_str());
			param = Compute(p);
			Sig = Sig0.MFFN(fn2, cfn2, param);
		}
		else
		{
			if (!p->next) throw CAstExceptionInvalidUsage(*this, pnode, "requires second parameter.");
			param = Compute(p->next);
			Compute(p);
			if (Sig.IsStruct() || !Sig.cell.empty() || Sig.GetFs() == 3)
			{
				if (structCall)
				{
					string dotnotation = dotstring(pnode, pRoot);
					throw CAstExceptionInvalidUsage(*this, pnode, "Cannot take cell, class or handle object ", dotnotation.c_str());
				}
				else
					throw CAstExceptionInvalidUsage(*this, p, "Cannot take cell, class or handle object ", fname.c_str());
			}
			Sig.MFFN(fn2, cfn2, param);
		}
		break;
	case 10: // other math function... function pointer ready
		fnsigs = fname + "(scalar or vector)";
		if (p && p->type != N_STRUCT)
			if (structCall)
			{
				firstparamtrim(fnsigs);
				checkNumArgs(pnode, p, fnsigs, 0, 0);
			}
			else
				checkNumArgs(pnode, p, fnsigs, 1, 1);
		if (Sig.IsStruct() || !Sig.cell.empty() || Sig.GetFs() == 3)
		{
			if (structCall)
			{
				string dotnotation = dotstring(pnode, pRoot);
				throw CAstExceptionInvalidUsage(*this, pnode, "Cannot take cell, class or handle object ", dotnotation.c_str());
			}
			else
				throw CAstExceptionInvalidUsage(*this, p, "Cannot take cell, class or handle object ", fname.c_str());
		}
		if (cfn1) Sig.each(cfn1);
		else if (fn1) Sig.each(fn1);
		else if (fn0) Sig.each_sym(fn0);
		else if (cfn0) Sig.each(cfn0);
		break;
	default: // res should be 0
		if (ft != pEnv->builtin.end())
		{
			fnsigs = fname + (*ft).second.funcsignature;
			if (structCall)
			{
				if ((*ft).second.alwaysstatic)
					throw CAstExceptionInvalidUsage(*this, pnode, "Cannot be a member function. ", fname.c_str());
				firstparamtrim(fnsigs);
				nArgs = checkNumArgs(pnode, p, fnsigs, (*ft).second.narg1 - 1, (*ft).second.narg2 - 1);
				(*ft).second.func(this, pnode, p, fnsigs);
			}
			else
			{
				nArgs = checkNumArgs(pnode, p, fnsigs, (*ft).second.narg1, (*ft).second.narg2);
				if ((*ft).second.alwaysstatic)
					(*ft).second.func(this, pnode, p, fnsigs);
				else
					(*ft).second.func(this, pnode, p->next, fnsigs);
			}
		}
		else
			throw CAstExceptionInvalidUsage(*this, p, "[INTERNAL] HandleAuxFunctions()--Not a built-in function?", fname.c_str());
	}
	Sig.functionEvalRes = true;
	return;
}
