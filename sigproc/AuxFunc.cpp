// AUXLAB 
//
// Copyright (c) 2009-2019 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.6
// Date: 7/6/2019
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
#include "wavplay.h"
#elif
#include <sys/types.h>
#include <dirent.h>
#endif

#include "aux_classes.h"
#include "sigproc.h"
#ifndef CISIGPROC
#include "psycon.tab.h"
#else
#include "cipsycon.tab.h"
#endif

#include "lame_bj.h"

string CAstSigEnv::AppPath = "";
map<string, Cfunction> dummy_pseudo_vars;
map<string, Cfunction> CAstSigEnv::pseudo_vars = dummy_pseudo_vars;

//#ifdef NO_PLAYSND // for aux_builtin_ext
//CAstSig::play_block_ms = 0;
//CAstSig::record_block_ms = 0;
//CAstSig::record_bytes = 0;
//#endif

map<double, FILE *> file_ids;

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

void EnumAudioVariables(CAstSig *past, vector<string> &var)
{
	var.clear();
	for (map<string, CVar>::iterator what=past->Vars.begin(); what!= past->Vars.end(); what++)
		if (what->second.GetType()==CSIG_AUDIO) var.push_back(what->first);
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

bool CAstSig::IsValidBuiltin(string funcname)
{
	if (pEnv->pseudo_vars.find(funcname) != pEnv->pseudo_vars.end())
		return false;
	return pEnv->builtin.find(funcname) != pEnv->builtin.end();
}

void CAstSig::checkAudioSig(const AstNode *pnode, CVar &checkthis, string addmsg)
{
	if (checkthis.GetType()==CSIG_AUDIO) return;
	if (checkthis.GetType()==CSIG_CELL && ((CSignal)checkthis).GetType()==CSIG_AUDIO) return;
	string msg("requires an audio signal as the base.");
	throw ExceptionMsg(pnode, (msg+addmsg).c_str());
}

void CAstSig::checkTSeq(const AstNode *pnode, CVar &checkthis, string addmsg)
{
	if (checkthis.IsTimeSignal()) return;
	string msg("requires a time_sequence as the base.");
	throw ExceptionMsg(pnode, (msg + addmsg).c_str());
}


void CAstSig::checkComplex (const AstNode *pnode, CVar &checkthis)
{
	if (checkthis.IsComplex()) return;
	string msg("requires a complex vector as the base.");
	throw ExceptionMsg(pnode, msg.c_str());
}

void CAstSig::checkSignal(const AstNode *pnode, CVar &checkthis, string addmsg)
{ // if it is audio or vector --> OK
  // if not (including scalar) --> not OK
	if (checkthis.GetType() == CSIG_VECTOR) return;
	if (checkthis.GetType() == CSIG_AUDIO) return;
	string msg("requires an audio signal or a vector.");
	throw ExceptionMsg(pnode, (msg + addmsg).c_str());
}

void CAstSig::checkVector(const AstNode *pnode, CVar &checkthis, string addmsg)
{
	if (checkthis.GetType() == CSIG_SCALAR) return;
	if (checkthis.GetType() == CSIG_VECTOR) return;
	string msg("requires a non-audio array.");
	throw ExceptionMsg(pnode, (msg+addmsg).c_str());
}

void CAstSig::checkScalar(const AstNode *pnode, CVar &checkthis, string addmsg)
{
	if (checkthis.IsScalar()) return;
	string msg("requires a scalar argument.");
	throw ExceptionMsg(pnode, (msg+addmsg).c_str());
}

void CAstSig::checkString(const AstNode *pnode, CVar &checkthis, string addmsg)
{
	if (checkthis.GetType()==CSIG_STRING) return;
	string msg("requires a string argument.");
	if (checkthis.GetType()==CSIG_CELL && ((CSignal)checkthis).GetType()==CSIG_STRING) return;
	throw ExceptionMsg(pnode, (msg+addmsg).c_str());
}

void CAstSig::blockCell(const AstNode *pnode, CVar &checkthis)
{
	string msg("Not valid with a cell, struct, or point-array variable ");
	if (checkthis.GetFs()==3)
		throw ExceptionMsg(pnode, msg.c_str());
	if (checkthis.GetType() == CSIG_CELL)
		if (((CSignal)checkthis).GetType() == CSIG_EMPTY)
			throw ExceptionMsg(pnode, msg.c_str());
	if (checkthis.GetType() == CSIG_STRUCT)
			throw ExceptionMsg(pnode, msg.c_str());
}

void CAstSig::blockScalar(const AstNode *pnode, CVar &checkthis)
{
	if (checkthis.GetType() != CSIG_SCALAR && !checkthis.IsEmpty()) return;
	string msg("Not valid with a scalar variable ");
	throw ExceptionMsg(pnode, msg.c_str());
}

void CAstSig::blockString(const AstNode *pnode, CVar &checkthis)
{
	if (checkthis.GetType() == CSIG_STRING) {
		string msg("Not valid with a string variable ");
		throw ExceptionMsg(pnode, msg.c_str());
	}
}

void CAstSig::blockComplex(const AstNode *pnode, CVar &checkthis)
{
	if (checkthis.IsComplex()) {
		string msg("Not valid with a complex variable ");
		throw ExceptionMsg(pnode, msg.c_str());
	}
}

void _diff(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkSignal(pnode, past->Sig);
	CSignals sig = past->Sig;
	int order=1;
	if (p) {
		past->Compute(p);
		past->checkScalar(pnode, past->Sig);
		order = (int)round(past->Sig.value());
		if (order < 1) throw past->ExceptionMsg(pnode, "non-negative argument is required");
	}
	sig.Diff(order);
	past->Sig = sig;
}

void _nullin(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	CSignals sig = past->Sig;
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
		past->Sig.SetValue((double)past->toc()); // milliseconds since tic was called.
	}
}

void _rand(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkScalar(pnode, past->Sig);
	double val = past->Sig.value();
	if (val<0)
		throw past->ExceptionMsg(pnode, fnsigs, "argument must be positive.");
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
		throw past->ExceptionMsg(pnode, fnsigs, "argument must be positive.");
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
		throw past->ExceptionMsg(pnode, fnsigs, "argument must be positive.");
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
				throw past->ExceptionMsg(p, fnsigs, "Third parameter, if used, must be a struct variable.");
		}
		int type = param.GetType();
		if (type!= CSIG_TSERIES && type != CSIG_SCALAR)
			throw past->ExceptionMsg(p, fnsigs, "parameter must be either a scalar or a time sequence.");
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
			throw past->ExceptionMsg(pnode, ("Error in respeed:" + param.string()).c_str());
	}
	catch (const CAstException &e) { throw past->ExceptionMsg(pnode, fnsigs, e.getErrMsg()); }
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
		throw past->ExceptionMsg(pnode, "Internal error! RegExp.Parse( ) failed.");
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
					throw past->ExceptionMsg(pnode, fnsigs, "Not enough arguments.");
				if (nGroupIndex == 2 && *szStart == 's')
					vstring = tast.ComputeString(p);
				else if (tast.Compute(p).IsScalar())
					v = tast.Sig.value();
				else
					throw past->ExceptionMsg(pnode, fnsigs, "Scalar value expected for this argument.");
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

void _fopen(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	string filename = past->MakeFilename(past->ComputeString(p),"");
	char mode[8];
	strcpy(mode, past->ComputeString(p->next).c_str());

	FILE *fl;
	if (!(fl = fopen(filename.c_str(), mode)))
	{
		past->Sig.SetValue(-1.);
	}
	else
	{
		past->Sig.SetValue((double)(INT_PTR)fl);
		file_ids[(double)(INT_PTR)fl] = fl;
	}
}

void _fclose(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->Compute(p);
	if (!past->Sig.IsScalar())
		throw past->ExceptionMsg(pnode, fnsigs, "First arg must be a file identifider");
	double fl = past->Sig.value();
	FILE *file = file_ids[fl];
	if (!file || fclose(file)==EOF)
	{
		past->Sig.SetValue(-1.);
	}
	else
	{
		past->Sig.SetValue(0);
		file_ids.erase(fl);
	}
}

void _fprintf(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	_sprintf(past, pnode, p->next, fnsigs);
	string buffer;
	buffer = past->Sig.string();
	bool openclosehere(1);
	FILE *file;
	//is first argument string?
	past->Compute(p);
	if (past->Sig.IsString())
	{
		string filename = past->MakeFilename(past->ComputeString(p), "txt");
		if (!(file = fopen(filename.c_str(), "a")))
		{
			past->Sig.SetValue(-1.);
			return;
		}
	}
	else
	{
		if (!past->Sig.IsScalar())
		{
			past->Sig.SetValue(-2.);
			return;
		}
		if (past->Sig.value() == 0.)
		{
			printf(buffer.c_str());
			return;
		}
		file = file_ids[past->Sig.value()];
		openclosehere = false;
	}
	if (fprintf(file, buffer.c_str())<0)
		past->Sig.SetValue(-3.);
	else
	{
		if (openclosehere)
		{
			if (fclose(file)==EOF)
			{
				past->Sig.SetValue(-4.);
				return;
			}
		}
		past->Sig.SetValue((double)buffer.length());
	}
}

void _colon(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CSignals first = past->Sig;
	if (!first.IsScalar()) throw past->ExceptionMsg(pnode, "All arguments must be scalars (check 1st arg).");
	double val1, val2, step;
	CSignals third, second = past->Compute(p->next);
	if (!second.IsScalar()) throw past->ExceptionMsg(pnode, "All arguments must be scalars (check 2nd arg)."); 
	val1 = first.value();
	val2 = second.value();
	if (p->next->next) {
		third = past->Compute(p->next->next);
		if (!third.IsScalar()) throw past->ExceptionMsg(pnode, "All arguments must be scalars (check 3rd arg)."); 
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
	CSignals rx = past->Sig;
	CSignals ry = past->Compute(p);
	CSignals qx = past->Compute(p->next);
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


void aux_error(CAstSig *past, const AstNode *pnode, const AstNode *p, int nArgs, string &fnsigs)
{
	string errmsg = past->ComputeString(p);
	throw past->ExceptionMsg(pnode, ("User raised error - " + errmsg).c_str());
}

#ifndef NO_SF
// It is better to have setfs as a hook command, because it involves UI and expressions such as setfs(16000)+1 don't make sense.
// Keep this just in case. But this will not update existing variables according to the new sample rate, so its functionality is pretty limited.
// Use an auxlab hook command in this format #setfs 16000
// 11/21/2017
void _setfs(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (!past->Sig.IsScalar()) throw past->ExceptionMsg(p, "Scalar value should be provided for the sample rate.");
	double fs = past->Sig.value();
	if (fs<500.) throw past->ExceptionMsg(p, "Sample rate should be at least 500 Hz.");
	past->FsFixed=true;
	past->pEnv->Fs = (int)fs; //Sample rate adjusted
}

static void write2textfile(FILE * fid, CVar *psig)
{
	if (psig->bufBlockSize==1)
	{
		for (unsigned int k = 0; k < psig->nSamples; k++)
			fprintf(fid, "%c ", psig->logbuf[k]);
		fprintf(fid, "\n");
	}
	else if (psig->IsAudioObj()) // audio
	{
		for (unsigned int k = 0; k < psig->nSamples; k++)
			fprintf(fid, "%7.4f ", psig->buf[k]);
		if (psig->next)
		{
			fprintf(fid, "\n");
			for (unsigned int k = 0; k < psig->nSamples; k++)
				fprintf(fid, "%7.4f ", psig->next->buf[k]);
		}
		fprintf(fid, "\n");
	}
	else if (!psig->cell.empty())
	{
		for (auto cel : psig->cell)
			write2textfile(fid, &cel);
	}
	else
	{
		for (unsigned int k = 0; k < psig->nSamples; k++)
			fprintf(fid, "%g ", psig->buf[k]);
		fprintf(fid, "\n");
	}
}

void _wavwrite(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkAudioSig(p, past->Sig);
	string option;
	string filename;
	CSignals third;
	try {
		CAstSig tp(past);
		tp.Compute(p);
		tp.checkString(p, tp.Sig);
		if (tp.Sig.string().empty())
			throw tp.ExceptionMsg(p, "Empty filename");
		filename = tp.Sig.string();
		if (p->next!=NULL)
		{
			third = tp.Compute(p->next);
			tp.checkString(p->next, (CVar)third);
			option = third.string();
		}
	}
	catch (const CAstException &e) {	throw past->ExceptionMsg(pnode, fnsigs, e.getErrMsg());	}
	filename = past->MakeFilename(filename, "wav");
	char errStr[256];
	if (!past->Sig.Wavwrite(filename.c_str(), errStr, option))
		throw past->ExceptionMsg(p, errStr);
}

void _write(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkAudioSig(p, past->Sig);
	string option;
	string filename;
	CSignals third;
	try {
		CAstSig tp(past);
		tp.Compute(p);
		tp.checkString(p, tp.Sig);
		if (tp.Sig.string().empty())
			throw tp.ExceptionMsg(p, "Empty filename");
		filename = tp.Sig.string();
		if (p->next != NULL)
		{
			third = tp.Compute(p->next);
			tp.checkString(p->next, (CVar)third);
			option = third.string();
		}
	}
	catch (const CAstException &e) { throw past->ExceptionMsg(pnode, fnsigs, e.getErrMsg()); }
	trim(filename, ' ');
	size_t pdot = filename.rfind('.');
	string extension = filename.substr(pdot + 1);
	if (extension.empty())
		throw past->ExceptionMsg(p, "The extension must be specified .wav .mp3 or .txt");
	else if (extension == "mp3")
	{
		/* For now, MakeChainless makes the whole audio data into one big piece.
		   Compared to wavwrite, short null portions in the middle should not be handled
		   separately, instead save into mp3 as the part of the audio data.
		   If the null portions in the middle are long, it would be better to
		   convert those null portions into separate silent portions and save each piece into 
		   separate mp3 block, but as of today, I'm not sure how to do it.
		   Maybe call lame_encoder_loop in lame_main.c for each block while FILE * outf is open?
		   Let's figure it out later. 10/3/2019
		   */
		past->Sig.MakeChainless();
		char errStr[256] = { 0 };
		int res = write_mp3(past->Sig.nSamples, past->Sig.buf, past->Sig.next ? past->Sig.next->buf : NULL, past->Sig.GetFs(), filename.c_str(), errStr);
		if (!res)
			throw past->ExceptionMsg(p, errStr);
	}
	else if (extension == "wav")
	{
		_wavwrite(past, pnode, p, fnsigs);
	}
	else if (extension == "txt")
	{
		FILE* fid = fopen(filename.c_str(), "wt");
		if (!fid)
			throw past->ExceptionMsg(p, "File creation error");
		write2textfile(fid, &past->Sig);
		fclose(fid);
	}
	else
		throw past->ExceptionMsg(p, "unknown audio file extension. Must be .wav or .mp3");
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
	double block = CAstSig::record_block_ms;
	double duration = -1;
	int nArgs = 0, devID = 0, nChans = 1;
	string callbackname = "default_callback_audio_recording";
	for (const AstNode *cp = p; cp; cp = cp->next)
		++nArgs;
	switch (nArgs)
	{
	case 5:
		past->Compute(p->next->next->next->next);
		if (!past->Sig.IsScalar())
			throw past->ExceptionMsg(pnode, fnsigs, "The fifth argument must be a constant representing the block size for the callback in milliseconds.");
		block = past->Sig.value();
	case 4:
		past->Compute(p->next->next->next);
		if (!past->Sig.IsScalar())
			throw past->ExceptionMsg(pnode, fnsigs, "The fourth argument is either 1 (mono) or 2 (stereo) for recording.");
		nChans = (int)past->Sig.value();
		if (nChans!=1 && nChans!=2)
			throw past->ExceptionMsg(pnode, fnsigs, "The fourth argument is either 1 (mono) or 2 (stereo) for recording.");
	case 3:
		past->Compute(p->next->next);
		if (!past->Sig.IsScalar())
			throw past->ExceptionMsg(pnode, fnsigs, "The fourth argument must be a constant representing the duration to record, -1 means indefinite duration until stop is called.");
		duration = past->Sig.value();
	case 2:
		past->Compute(p->next);
		if (!past->Sig.IsString() && !past->Sig.IsEmpty())
			throw past->ExceptionMsg(pnode, fnsigs, "The second argument must be the file name (may include the path) of the callback function.");
		if (past->Sig.IsEmpty() || past->Sig.string().empty())
			callbackname = "?default_callback_audio_recording";
		else
			callbackname = past->Sig.string();
	case 1:
		past->Compute(p);
		if (!past->Sig.IsScalar())
			throw past->ExceptionMsg(pnode, fnsigs, "The first argument must be a constant (integer) representing the device ID.");
		devID = (int)past->Sig.value();
		break;
	case 0:
		break;
	}
	srand((unsigned)time(0));
	past->Sig.SetValue((double)rand());
	past->Sig.strut["dev"] = CVar((double)devID);
	past->Sig.strut["type"] = CVar(string("audio_record"));
	past->Sig.strut["id"] = CVar(past->Sig.value());
	past->Sig.strut["callback"] = callbackname;
	past->Sig.strut["channels"] = CVar((double)nChans);
	past->Sig.strut["durLeft"] = CVar(duration/1000.);
	past->Sig.strut["durRec"] = CVar(0.);
	past->Sig.strut["block"] = CVar(block);
	past->Sig.strut["active"] = CVar((double)(1==0));

	char errstr[256] = {};
	int newfs, recordID = (int)past->Sig.value();
	if ((newfs = Capture(devID, WM__AUDIOEVENT2, hShowDlg, past->pEnv->Fs, nChans, CAstSig::record_bytes, callbackname.c_str(), duration, block, recordID, errstr)) < 0)
		throw past->ExceptionMsg(pnode, fnsigs, errstr);
	past->Sig.strut["active"] = CVar((double)(1 == 1));
	if (past->lhs && past->lhs->type == N_VECTOR && past->lhs->alt->next)
	{ // [y, h] = record (.....), do output binding here.
		past->SetVar(past->lhs->alt->next->str, &past->Sig);
	}
	// for a statement, y=h.start, y is not from the RHS directly, but is updated laster after the callback
	// so we need to block the RHS from affecting the LHS.. Let's use -1 for suppress (to be used in CDeepProc::TID_tag in AstSig2.cpp)
	past->pAst->suppress = -1;
	if (newfs != past->pEnv->Fs)
	{
		past->pEnv->Fs = newfs;
		sformat(past->statusMsg, "(NOTE)Sample Rate of AUXLAB Environment is adjusted to %d Hz.", past->pEnv->Fs);
	}
	past->Sig.strut["fs"] = CVar((double)newfs);
	past->Sig.Reset(); // to shield the first LHS variable (callback output) from Sig
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
			throw past->ExceptionMsg(pnode, fnsigs, "The base must be an audio signal or an audio handle (1)");
		if (sig.strut.find("type")== sig.strut.end())
			throw past->ExceptionMsg(pnode, fnsigs, "The base must be an audio signal or an audio handle (2)");
		CSignals type = sig.strut["type"];
		if (type.GetType()!=CSIG_STRING || type.string() != "audio_playback")
			throw past->ExceptionMsg(pnode, fnsigs, "The base must be an audio signal or an audio handle (3)");
		if (!p)
			throw past->ExceptionMsg(pnode, fnsigs, "Audio signal not given.");
		past->Compute(p);
		past->checkAudioSig(p, past->Sig);
		CSignals audio = past->Sig;
		if (p->next)
		{
			past->Compute(p->next);
			if (!past->Sig.IsScalar())
				throw past->ExceptionMsg(p, fnsigs, "Argument must be a scalar.");
			nRepeats = (int)past->Sig.value();
			if (nRepeats<1)
				throw past->ExceptionMsg(p, fnsigs, "Repeat counter must be equal or greater than one.");
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
				throw past->ExceptionMsg(p, fnsigs, "Argument must be a scalar.");
			nRepeats = (int)past->Sig.value();
			if (nRepeats<1)
				throw past->ExceptionMsg(p, fnsigs, "Repeat counter must be equal or greater than one.");
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
			p->sig.strut.insert(pair<string, CVar>("devID", CSignals((double)devID)));
			p->sig.strut.insert(pair<string, CVar>("durTotal", CSignals(addtime)));
			if (p->sig.strut.find("durLeft") == p->sig.strut.end())
				p->sig.strut.insert(pair<string, CVar>("durLeft", CSignals(addtime)));
			else
				*(p->sig.strut["durLeft"].buf) += addtime;
			p->sig.strut.insert(pair<string, CVar>("durPlayed", CSignals(0.)));
			past->Sig = p->sig; //only to return to xcom
		}
	}
}

void _stop(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	char errstr[256];
	CVar sig = past->Sig;
	if (!sig.IsScalar())
		throw past->ExceptionMsg(pnode, fnsigs, "Argument must be a scalar.");
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
		throw past->ExceptionMsg(pnode, fnsigs, "stop() applies only to audio_playback or audio_record.");
}

void _pause_resume(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar sig = past->Sig;
	if (!sig.IsScalar())
		throw past->ExceptionMsg(pnode, fnsigs, "Argument must be a scalar.");
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
		throw past->ExceptionMsg(pnode, fnsigs, "pause() or resume() applies only to audio_playback or audio_record.");
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
			vector<CVar> res = tast.Compute();
			past->Sig = res.back();
			for (map<string, CVar>::iterator it = tast.Vars.begin(); it != tast.Vars.end(); it++)
				past->Vars[it->first] = it->second;
		} catch (const char *errmsg) {
			fclose(auxfile);
			throw past->ExceptionMsg(pnode, ("Including "+filename+"\n\nIn the file: \n"+errmsg).c_str());
		}
	} else
		throw past->ExceptionMsg(pnode, "Cannot read file", filename);
}

void _eval(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{ // eval() is one of the functions where echoing in the xcom command window doesn't make sense.
  // but the new variables created or modified within the eval call should be transported back to ast
	string str = past->ComputeString(p);
	try {
		CAstSig tast(str.c_str(), past);
		vector<CVar> res = tast.Compute();
		//transporting variables 
		for (map<string, CVar>::iterator it = tast.Vars.begin(); it != tast.Vars.end(); it++)
			past->SetVar(it->first.c_str(), &it->second);
	} catch (const char *errmsg) {
		throw past->ExceptionMsg(pnode, ("While evaluating\n"+str+"\n"+errmsg).c_str());
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
		throw past->ExceptionMsg(pnode, fnsigs, "argument must be a text string.");
	string emsg;
	CAstSig tast(past->pEnv);
	if (!tast.SetNewScript(emsg, (string("[") + past->Sig.string() + "]").c_str()))
		tast.SetNewScript(emsg, "[]");
	if (!isAllNodeT_NUM(tast.pAst->child))
		tast.SetNewScript(emsg, "[]");
	vector<CVar> res = tast.Compute();
	past->Sig = res.back();
}

void _zeros(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (!past->Sig.IsScalar())
		throw past->ExceptionMsg(p, fnsigs, "argument must be a scalar.");
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
		throw past->ExceptionMsg(p, fnsigs, "argument must be a scalar.");
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
	CSignals second;
	try {
		CAstSig tp(past);
		second = tp.Compute(p);
	}
	catch (const CAstException &e) {
		throw past->ExceptionMsg(pnode, fnsigs, e.getErrMsg());
	}
	if (!second.IsScalar())
		throw past->ExceptionMsg(pnode, fnsigs, "argument must be a scalar.");
	double val = second.value();
	if (val!=(double)(int)val)
		throw past->ExceptionMsg(pnode, fnsigs, "argument must be an integer.");
	double nCols = past->Sig.nSamples / val;
	if (past->Sig.GetType() == CSIG_VECTOR)
	{
		if (past->Sig.nSamples / val != (int)nCols)
			throw past->ExceptionMsg(pnode, fnsigs, "The length of array must be divisible by the requested the row count.");
	}
	else if (past->Sig.GetType() == CSIG_AUDIO)
	{
		if (past->Sig.chain)
			throw past->ExceptionMsg(pnode, fnsigs, "To make a matrix from an audio signal, null portions should be filled with zeros. Call contig().");
		double rem = fmod((double)past->Sig.nSamples, val);
		if (rem > 0)
		{
			unsigned int nPtsNeeded = (unsigned int)(val - rem);
			past->Sig.UpdateBuffer(past->Sig.nSamples + nPtsNeeded);
		}
	}
	else
		throw past->ExceptionMsg(pnode, fnsigs, "Vector or audio signals required.");
	past->Sig.nGroups = (int)val;
}

void _cell(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (!past->Sig.IsScalar())
		throw past->ExceptionMsg(p, fnsigs, "argument must be a scalar.");
	int n = (int)round(past->Sig.value());
	if (n <= 0)
		throw past->ExceptionMsg(p, fnsigs, "argument must be a positive number.");

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
		throw past->ExceptionMsg(p, fnsigs, "argument must be a string.");
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
		throw past->ExceptionMsg(p, fnsigs, "No files in the specified directory");
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
			tp.strut["name"] = CSignals(CSignal(string(fname)));
			tp.strut["ext"] = CSignals(CSignal(string(ext)));
			if (fname[0] != '.' || fname[1] != '\0')
			{
				if (pathonly[0])
					tp.strut["path"] = CSignals(CSignal(string(pathonly)));
				else
				{
					char *pt = strstr(fullname, fname);
					if (pt) *pt = 0;
					tp.strut["path"] = CSignals(CSignal(string(fullname)));
				}
				tp.strut["bytes"] = CSignals((double)((unsigned __int64)(ls.nFileSizeHigh * (MAXDWORD + 1)) + ls.nFileSizeLow));
				FILETIME ft = ls.ftLastWriteTime;
				SYSTEMTIME lt;
				FileTimeToSystemTime(&ft, &lt);
				sprintf(fullname, "%02d/%02d/%4d, %02d:%02d:%02d", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
				tp.strut["date"] = CSignals(CSignal(string(fullname)));
				bool b = ls.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
				tp.strut["isdir"] = CSignals(b);
				past->Sig.appendcell(tp);
			}
		} while (FindNextFile(hFind, &ls));
	}
#elif
#endif
}

void _isaudioat(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{ // if the signal is null at specified time_pt
	past->checkAudioSig(pnode, past->Sig);
	CAstSig tp(past);
	tp.Compute(p);
	if (tp.Sig.GetType() != CSIG_SCALAR)
		throw past->ExceptionMsg(pnode, fnsigs, "argument must be a scalar.");
	double vv = tp.Sig.value();
	past->Sig.SetValue(past->Sig.IsAudioOnAt(vv));
	past->Sig.MakeLogical();
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
		CSignals x1 = past->Sig;
		CSignals x2 = past->Compute(p);
		if (!x2.IsLogical())
			throw past->ExceptionMsg(pnode, fnsigs, "argument must be a logical variable.");
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
		CSignals x1 = past->Sig;
		CSignals x2 = past->Compute(p);
		if (!x2.IsLogical())
			throw past->ExceptionMsg(pnode, fnsigs, "argument must be a logical variable.");
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
				throw past->ExceptionMsg(p, "argument must be a positive integer.");
		}
		catch (const CAstException &e) {
			throw past->ExceptionMsg(pnode, fnsigs, e.getErrMsg());
		}
	}
	past->Sig = past->Sig.runFct2getsig(&CSignal::FFT, (void*)&param);
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
				throw past->ExceptionMsg(p, "argument must be a positive integer.");
		}
		catch (const CAstException &e) {
			throw past->ExceptionMsg(pnode, fnsigs, e.getErrMsg());
		}
	}
	past->Sig = past->Sig.runFct2getsig(&CSignal::iFFT, (void*)&param);
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
			throw past->ExceptionMsg(pnode, fnsigs, "2nd argument must be a scalar.");
		if (past->Sig.value() < 0) order = -1.;
	}
	past->Sig = sig.runFct2modify(&CSignal::sort, &order);
}

void _decfir(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CSignals second = past->Compute(p->next);
	CSignals third = past->Compute(p->next->next);
	if (!third.IsScalar())
		throw past->ExceptionMsg(pnode, fnsigs, "3rd argument must be a scalar: offset");
	CSignals fourth = past->Compute(p->next->next->next);
	if (!fourth.IsScalar())
		throw past->ExceptionMsg(pnode, fnsigs, "4th argument must be a scalar: nChan");
	int offset = (int)third.value()-1; // because AUX is one-based
	int nChan = (int)fourth.value();
	if (offset>nChan)
		throw past->ExceptionMsg(pnode, fnsigs, "nChan must be equal or greater than offset");
	past->Compute(p);
	past->Sig.DecFir(second, offset, nChan);
}

void _conv(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	//For only real (double) arrays 3/4/2019
	//p should be non NULL
	CSignals sig = past->Sig;
	CSignals array2 = past->Compute(p);
	past->Sig = sig.runFct2modify(&CSignal::conv, &array2);
}

void _filt(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	CSignals sig = past->Sig;
	string fname = pnode->str;
	CSignals third, second = past->Compute(p);
	if (p->next) {	// 3 args
		third = past->Compute(p->next);
	} else {				// 2 args
		if (second.nSamples <= 1)
			throw past->ExceptionMsg(pnode, fnsigs, "2nd argument must be a vector(the numerator array for filtering).");
		third.SetValue(1);
	}
	unsigned int len = max(second.nSamples, third.nSamples);
	if (second.IsScalar())
		second.UpdateBuffer(len);
	else if (third.IsScalar())
		third.UpdateBuffer(len);

	if (!second.chain && !third.chain && second.tmark==0 && third.tmark==0)
	{
		vector<double> num(second.buf, second.buf + second.nSamples);
		vector<double> den(third.buf, third.buf + third.nSamples);
		vector<vector<double>> coeffs;
		coeffs.push_back(num);
		coeffs.push_back(den);
		if (fname == "filt")
			sig.runFct2modify(&CSignal::filter, &coeffs);
		else if (fname == "filtfilt")
			sig.runFct2modify(&CSignal::filtfilt, &coeffs);
	}
	else
	{
		throw past->ExceptionMsg(pnode, fnsigs, "Internal error--leftover from Dynamic filtering");
	}
	past->Sig = sig;
}

#ifndef NO_IIR
void _iir(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	int type(0), kind(1), norder(4);
	double freqs[2], rippledB(0.5), stopbandFreqORAttenDB(-40.);
	const AstNode *args = p;
	string fname = pnode->str;
	past->checkAudioSig(pnode, past->Sig);
	CSignals sigX(past->Sig);
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
		throw past->ExceptionMsg(pnode, fnsigs, emsg);
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
	CSignals fifth, second = past->Compute(p->next);
	CSignals third = past->Compute(p->next->next);
	CSignals fourth = past->Compute(p->next->next->next);
	if (!second.IsScalar()) throw past->ExceptionMsg(p, fnsigs, "freq2 must be a scalar.");
	if (!third.IsScalar()) throw past->ExceptionMsg(p, fnsigs, "mod_rate must be a scalar.");
	if (!fourth.IsScalar()) throw past->ExceptionMsg(p, fnsigs, "duration must be a scalar.");
	if (fourth.value()<=0) throw past->ExceptionMsg(p, fnsigs, "duration must be positive.");
	if (p->next->next->next->next) {	// 5 args
		fifth = past->Compute(p->next->next->next->next);
		if (!fifth.IsScalar()) throw past->ExceptionMsg(p, fnsigs, "init_phase must be a scalar."); }
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
				throw past->ExceptionMsg(p, fnsigs, "Invalid parameter: should be either 0 (divided by n-1; default) or 1 (divided by n).");
		}
		catch (const CAstException &e) { throw past->ExceptionMsg(pnode, fnsigs, e.getErrMsg()); }
	}
	past->Sig = past->Sig.runFct2getvals(&CSignal::stdev, &arg);
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
		catch (const CAstException &e) { throw past->ExceptionMsg(pnode, fnsigs, e.getErrMsg()); }
	}
	double tp1 = past->Sig.nGroups;
	double tp2 = past->Sig.Len();
	if (arg.value() == 0.)
	{
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
		throw past->ExceptionMsg(pnode, fnsigs, "Invalid parameter: should be either 1 or 2.");
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
				throw past->ExceptionMsg(pnode, "A stereo signal with different lengths for L and R.");
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
			past->Sig.SetFs(1); // Don't call Reset because we need to leave the data buffer as is.
			if (!past->Sig.cell.empty())
				past->Sig.SetValue((double)past->Sig.cell.size());
			else if (past->Sig.IsTimeSignal())
			{
				past->Sig.SetValue((double)(past->Sig.CountChains()));
			}
			else
			{
				past->Sig = past->Sig.runFct2getvals(&CSignal::length);
			}
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
	//commenting out 3/6/2019--don't know what this was for. Now causing crash.
//	if (past->pAst->type == N_VECTOR)
//		if (past->pAst->alt->next)
//			past->SetVar(past->pAst->alt->next->str, &additionalArg);
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
}

void _minmax(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (past->Sig.IsEmpty()) return; //for empty input, empty output
	string fname = pnode->str;
	CSignals sig = past->Sig;
	CVar additionalArg(sig.GetFs());
	CVar *pt(NULL);
	if (past->pAst->type == N_VECTOR && past->pAst->alt->next) pt =  &additionalArg;
	if (past->pLast && past->pLast->type==N_VECTOR) pt = &additionalArg;
	if (fname == "max") past->Sig = sig.runFct2getvals(&CSignal::_max, pt);
	else if (fname == "min") past->Sig = sig.runFct2getvals(&CSignal::_min, pt);
	if (past->Sig.IsComplex()) past->Sig.SetValue(-1); // do it again 6/2/2018
	if (past->pAst->type == N_VECTOR)
	{
		if (past->pAst->alt->next)
			past->SetVar(past->pAst->alt->next->str, &additionalArg);
	}
	if (past->pLast && past->pLast->type == N_VECTOR)
		if (past->pLast->alt->next)
			past->SetVar(past->pLast->alt->next->str, &additionalArg);
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
	past->Sig.SetFs(past->GetFs()); // CHECK IT!!!!!!!!
}

void _vector(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{
	past->Sig.MakeChainless();
	past->Sig.SetFs(1);
}

void _ramp(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	CSignals param;
	try {
		CAstSig tp(past);
		tp.Compute(p);
		param = tp.Compute(p);
	}
	catch (const CAstException &e) { throw past->ExceptionMsg(pnode, fnsigs, e.getErrMsg()); }
	if (!param.IsScalar())
		throw past->ExceptionMsg(p, fnsigs, "Ramp_duration must be a scalar.");
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
			throw past->ExceptionMsg(p, fnsigs, "AM_rate must be a scalar.");
		modRate = rate.value();
		int nArgs(0);
		for (const AstNode *cp = p; cp; cp = cp->next)
			++nArgs;
		if (nArgs >= 2) //either 2 or 3
		{
			depth = tp.Compute(p->next);
			if (!depth.IsScalar())
				throw past->ExceptionMsg(p, fnsigs, "AM_depth_m must be a scalar.");
			amDepth = depth.value();
			if (nArgs == 3)
			{
				initphase = tp.Compute(p->next->next);
				if (!initphase.IsScalar())
					throw past->ExceptionMsg(p, fnsigs, "initial_phase must be a scalar.");
				initPhase = initphase.value();
			}
		}
	}
	catch (const CAstException &e) { throw past->ExceptionMsg(pnode, fnsigs, e.getErrMsg()); }
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


void _tone(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	double initPhase(0.);
	int len, nArgs(0);
	for (const AstNode *cp = p; cp; cp = cp->next)
		++nArgs;
	if (!past->Sig.IsScalar() && (!past->Sig.IsVector() || past->Sig.nSamples>2))
		throw past->ExceptionMsg(p, fnsigs, "Frequency must be either a constant or two-element array.");
	body freq = past->Sig; //should be same as tp.Compute(p);
	if (freq._max().front() >= past->GetFs() / 2)
		throw past->ExceptionMsg(p, fnsigs, "Frequency exceeds Nyquist frequency.");
	CVar duration = past->Compute(p->next);
	past->checkScalar(pnode, duration);
	if (duration.value() <= 0)
		throw past->ExceptionMsg(p, fnsigs, "Duration must be positive.");
	if (nArgs == 3)
	{
		CSignals _initph = past->Compute(p->next->next);
		if (!_initph.IsScalar())
			throw past->ExceptionMsg(p, fnsigs, "Initial_phase must be a scalar.");
		initPhase = _initph.value();
	}
	if ( (len = freq.nSamples )== 1)
	{
		past->Sig.Reset(past->GetFs());
		past->Sig.Tone(freq.value(), duration.value(), initPhase);
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
	CSignals dur = past->Sig;
	if (!dur.IsScalar())
		throw past->ExceptionMsg(p, fnsigs, "duration must be a scalar.");
	if (dur.value()<0.)
		throw past->ExceptionMsg(p, fnsigs, "duration must be a non-negative number.");
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
		throw past->ExceptionMsg(p, fnsigs, "Internal error 3426.");
}


void _wave(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	/*! \brief wave(filename)
	*         Open .wav file
	*
	*  Input: filename: string
	*  Output: audio_signal
	*/
	past->checkString(pnode, past->Sig);
	string filename = past->MakeFilename(past->Sig.string(), "wav");
	char errStr[256];
	if (!past->Sig.Wavread(filename.c_str(), errStr))
		throw past->ExceptionMsg(p, errStr);
	vector<string> audiovars;
	EnumAudioVariables(past, audiovars);
	if (past->FsFixed || audiovars.size()>0)
	{
		if (past->Sig.GetFs() != past->GetFs())
		{
			int oldFs = past->GetFs();
			CSignals ratio(1);
			ratio.SetValue(past->Sig.GetFs() / (double)oldFs);
			past->Sig.runFct2modify(&CSignal::resample, &ratio);
			if (ratio.IsString()) // this means there was an error during resample
				throw past->ExceptionMsg(p, fnsigs, ratio.string().c_str());
			sformat(past->statusMsg, "(NOTE)File fs=%d Hz. The audio data resampled to %d Hz.", past->Sig.GetFs(), oldFs);
			past->Sig.SetFs(oldFs);
			if (past->Sig.next)
				past->Sig.next->SetFs(oldFs);
		}
	}
	else
	{
		past->pEnv->Fs = past->Sig.GetFs();
		sformat(past->statusMsg, "(NOTE)Sample Rate of AUXLAB Environment is now set to %d Hz.", past->pEnv->Fs);
	}
}

void _file(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{ // Decide the convention when multiple lines input is coming... currently making cell array output... do I want to keep it this way? 2/21/2018 bjk
	past->checkString(pnode, past->Sig);
	string fullpath, content;
	char fname[MAX_PATH], ext[MAX_PATH], errStr[256] = { 0 };
	FILE *fp(NULL);
	int res;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	fp = past->OpenFileInPath(past->ComputeString(p), "", fullpath);
	_splitpath(past->ComputeString(p).c_str(), NULL, NULL, fname, ext);
	if (fp)
	{
		fclose(fp);
		if (string(_strlwr(ext)) == ".wav")
		{
#ifndef NO_SF
			_wave(past, pnode, p, fnsigs);
#endif // NO_SF
		}
		else if (string(_strlwr(ext)) == ".mp3")
		{
			int len, ffs, nChans;
			read_mp3_header(fullpath.c_str(), &len, &nChans, &ffs, errStr);
			past->Sig.SetFs(ffs);
			past->Sig.UpdateBuffer(len);
			if (nChans>1)
				past->Sig.SetNextChan((CTimeSeries *)&past->Sig);
			if (!read_mp3(&len, past->Sig.buf, (nChans > 1) ? past->Sig.next->buf : NULL, &ffs, fullpath.c_str(), errStr))
				throw past->ExceptionMsg(p, fnsigs, errStr);
		}
		else if (string(_strlwr(ext)) == ".aiff")
		{
			int len, ffs, nChans;
			read_aiff_header(fullpath.c_str(), &len, &nChans, &ffs, errStr);
			past->Sig.SetFs(ffs);
			past->Sig.UpdateBuffer(len);
			if (nChans > 1)
				past->Sig.SetNextChan((CTimeSeries *)&past->Sig);
			if (!read_mp3(&len, past->Sig.buf, (nChans > 1) ? past->Sig.next->buf : NULL, &ffs, fullpath.c_str(), errStr))
				throw past->ExceptionMsg(p, fnsigs, errStr);
		}
		else if (GetFileText(fullpath.c_str(), "rb", content))
		{
			past->Sig.Reset();
			vector<string> line;
			size_t nLines = str2vect(line, content.c_str(), "\r\n");
			char buf[256];
			int dataSize(16), nItems;
			for (size_t k = 0; k < nLines; k++)
			{
				nItems = countDeliminators(line[k].c_str(), " \t");
				double *data = new double[nItems];
				res = str2array(data, nItems, line[k].c_str(), " \t");
				if (res < nItems)
				{
					//This was added to catch invalid characters in the file, but it doesn't work for now. Do something else if this is important. 12/18/2017
					sprintf(buf, "Invalid format while file() on Line %d (ignore Line 1 below)", k + 1);
					delete[] data;
					throw buf;
				}
				CSignals tp(data, nItems);
				past->Sig.appendcell((CVar)tp);
				delete[] data;
			}
		}
	}
	else
		throw past->ExceptionMsg(p, fnsigs, "cannot open file");
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
	for (CTimeSeries *p = &past->Sig; p; p = p->chain)
		dbuf[k++] = p->tmark;
	if (p)
	{
		past->Compute(p);
		past->checkAudioSig(pnode, past->Sig);
		if (past->Sig.next)
			throw past->ExceptionMsg(pnode, "Cannot be a stereo audio signal--just for now...");
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
	else
		past->Sig.SetFs(0); // setting fs=0 to indicate relative time points... this is a temporary hack. 3/10/2019
	past->Sig.UpdateBuffer(nItems*nChains);
	past->Sig.nGroups = nChains;
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
			throw past->ExceptionMsg(p, fnsigs, "Argument must be a vector of time points.");
		if (newtime.nSamples!=nItems)
			throw past->ExceptionMsg(p, fnsigs, "Argument vector must have the same number of elements as the TSEQ.");
		int id = 0;
		for (CTimeSeries *p = &past->Sig; p; p = p->chain)
		{
			p->tmark = newtime.buf[id++];
			if (newtime.GetFs() == 0) p->SetFs(0);
		}
	}
	catch (const CAstException &e) { throw past->ExceptionMsg(pnode, fnsigs, e.getErrMsg()); }
	const AstNode *pRoot = past->findParentNode(past->pAst, (AstNode*)pnode, true);
	past->SetVar(pRoot->str, &past->Sig);
}

void _tsq_getvalues(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{
	past->checkTSeq(pnode, past->Sig);
	int k=0, nItems = past->Sig.CountChains();
	CTimeSeries out(1);
	out.UpdateBuffer(nItems * past->Sig.nSamples);
	out.nGroups = nItems;
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
			throw past->ExceptionMsg(p, fnsigs, "Argument must be a vector.");
		if (newvalues.nGroups!= nItems)
			throw past->ExceptionMsg(p, fnsigs, "Argument vector must have the same number of groups as the TSEQ length.");
		int id = 0;
		for (CTimeSeries *p = &past->Sig; p; p = p->chain)
		{
			p->UpdateBuffer(newvalues.Len());
			memcpy(p->buf, newvalues.buf + id++ * newvalues.Len(), sizeof(double)*newvalues.Len());
		}
	}
	catch (const CAstException &e) { throw past->ExceptionMsg(pnode, fnsigs, e.getErrMsg()); }
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

	ft.narg1 = 0;	ft.narg2 = 1;
	name = "std";
	ft.funcsignature = "(length, [, bias])";
	ft.func =  &_std; 
	builtin[name] = ft;

	
	ft.narg1 = 2;	ft.narg2 = 2;
	const char *f4[] = { "atleast", "atmost", 0 };
	ft.funcsignature = "(array_or_value, array_or_value_of_limit)";
	for (int k = 0; f4[k]; k++)
	{
		name = f4[k];
		ft.func = &_mostleast;
	builtin[name] = ft;
	}

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
	ft.narg1 = 1;	ft.narg2 = 0;
	ft.funcsignature = "(format_string, ...)";
	name = "sprintf";
	ft.func =  &_sprintf;
	builtin[name] = ft;
	name = "fprintf";
	ft.func =  &_fprintf;
	builtin[name] = ft;

	ft.narg1 = 1;	ft.narg2 = 1;
	ft.funcsignature = "(file_ID)";
	name = "fclose";
	ft.func =  &_fclose;
	builtin[name] = ft;

	ft.narg1 = 2;	ft.narg2 = 2;
	name = "fopen";
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
	ft.funcsignature = "(variable)";
	const char *f7[] = { "isempty", "isaudio", "isvector", "isstring", "isstereo", "isbool", "iscell", "isclass", "istseq", 0 };
	for (int k = 0; f7[k]; k++)
	{
		name = f7[k];
		ft.func = _varcheck;
		builtin[name] = ft;
	}

	ft.narg1 = 2;	ft.narg2 = 3;
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
	ft.funcsignature = "(devID [=0], callbackfunction_text [=""], recording_duration [=-1;indefinite], mono1_or_stereo2 [=1], callback_duration_ms [=setting in ini])";
	ft.alwaysstatic = true;
	ft.narg1 = 0;	ft.narg2 = 5;
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

	CSignals param, arg;
	// if Sig is real, negative forbid functions are checked here and proper function pointers are set... really??? 8/21/2018
	switch (res)
	{
	case 1: //single inarg, single outarg
		if (fname == "sqrt")		fn0 = sqrt, cfn1 = r2c_sqrt;
		else if (fname == "log")	fn0 = log, cfn1 = r2c_log;
		else if (fname == "log10")	fn0 = log10, cfn1 = r2c_log10;
		if (structCall)
		{
			if (p && p->type != N_STRUCT) throw ExceptionMsg(pnode, fname, "Superfluous parameter(s)");
		}
		else
		{
			if (!p) throw ExceptionMsg(pnode, "", "Empty parameter");
			Compute(p);
		}
		if (Sig.IsStruct() || !Sig.cell.empty() || Sig.GetFs() == 3)
		{
			if (structCall)
			{
				string dotnotation = dotstring(pnode, pRoot);
				throw ExceptionMsg(pnode, dotnotation, "Cannot take cell, class or handle object");
			}
			else
				throw ExceptionMsg(p, fname, "Cannot take cell, class or handle object");
		}
		Sig.MFFN(fn0, cfn1);
		break;
	case 2: // two in args, single out arg
		if (fname == "^" || fname == "pow")	fn2 = pow, cfn2 = r2c_pow;
		else if (fname == "mod")	fn2 = fmod, cfn2 = NULL; // do this
		if (structCall)
		{
			if (!p || p->type == N_STRUCT) throw ExceptionMsg(pnode, "", "requires second parameter.");
			CVar Sig0 = Sig;
			if (Sig.IsStruct() || !Sig.cell.empty() || Sig.GetFs() == 3)
				throw ExceptionMsg(pnode, fname, "Cannot take cell, class or handle object");
			param = Compute(p);
			Sig = Sig0.MFFN(fn2, cfn2, param);
		}
		else
		{
			if (!p->next) throw ExceptionMsg(pnode, "", "requires second parameter.");
			param = Compute(p->next);
			Compute(p);
			if (Sig.IsStruct() || !Sig.cell.empty() || Sig.GetFs() == 3)
			{
				if (structCall)
				{
					string dotnotation = dotstring(pnode, pRoot);
					throw ExceptionMsg(pnode, dotnotation, "Cannot take cell, class or handle object");
				}
				else
					throw ExceptionMsg(p, fname, "Cannot take cell, class or handle object");
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
				throw ExceptionMsg(pnode, dotnotation, "Cannot take cell, class or handle object");
			}
			else
				throw ExceptionMsg(p, fname, "Cannot take cell, class or handle object");
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
					throw ExceptionMsg(p, fname, "Cannot be a member function.");
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
			throw ExceptionMsg(p, fname, "Internal Error (users shouldn't see this error)--Not a built-in function");
	}
	Sig.functionEvalRes = true;
	return;
}
