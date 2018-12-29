// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.497
// Date: 12/26/2018
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

#include "sigproc.h"
#ifndef CISIGPROC
#include "psycon.tab.h"
#else
#include "cipsycon.tab.h"
//#include "AuxFunc.h"
#endif

#define WM__AUDIOEVENT	WM_APP + WOM_OPEN

map<double, FILE *> file_ids;

#ifdef _WINDOWS
__declspec (dllimport) void _figure(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
__declspec (dllimport) void _axes(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
__declspec (dllimport) void _text(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
__declspec (dllimport) void _plot(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
__declspec (dllimport) void _line(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
__declspec (dllimport) void _delete_graffy(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
__declspec (dllimport) void _replicate(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);
#elif
void _figure(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {};
void _axes(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {};
void _text(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {};
void _plot(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {};
void _line(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {};
void _delete_graffy(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {};
void _replicate(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs) {};
#endif

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
	return pEnv->inFunc.find(funcname) != pEnv->inFunc.end();
}

void CAstSig::checkAudioSig(const AstNode *pnode, CVar &checkthis, string addmsg)
{
	if (checkthis.GetType()==CSIG_AUDIO) return;
	if (checkthis.GetType()==CSIG_CELL && ((CSignal)checkthis).GetType()==CSIG_AUDIO) return;
	string msg("requires an audio signal as the base.");
	throw CAstException(pnode, this, (msg+addmsg).c_str());
}

void CAstSig::checkComplex (const AstNode *pnode, CVar &checkthis)
{
	if (checkthis.IsComplex()) return;
	string msg("requires a complex vector as the base.");
	throw CAstException(pnode, this, msg.c_str());
}

void CAstSig::checkSignal(const AstNode *pnode, CVar &checkthis, string addmsg)
{ // if it is audio or vector --> OK
  // if not (including scalar) --> not OK
	if (checkthis.GetType() == CSIG_VECTOR) return;
	if (checkthis.GetType() == CSIG_AUDIO) return;
	string msg("requires an audio signal or a vector.");
	throw CAstException(pnode, this, (msg + addmsg).c_str());
}

void CAstSig::checkVector(const AstNode *pnode, CVar &checkthis, string addmsg)
{
	if (checkthis.GetType() == CSIG_SCALAR) return;
	if (checkthis.GetType() == CSIG_VECTOR) return;
	string msg("requires a non-audio array.");
	throw CAstException(pnode, this, (msg+addmsg));
}

void CAstSig::checkScalar(const AstNode *pnode, CVar &checkthis, string addmsg)
{
	if (checkthis.IsScalar()) return;
	string msg("requires a scalar argument.");
	throw CAstException(pnode, this, (msg + addmsg));
}

void CAstSig::checkString(const AstNode *pnode, CVar &checkthis, string addmsg)
{
	if (checkthis.GetType()==CSIG_STRING) return;
	string msg("requires a string argument.");
	if (checkthis.GetType()==CSIG_CELL && ((CSignal)checkthis).GetType()==CSIG_STRING) return;
	throw CAstException(pnode, this, (msg+addmsg).c_str());
}

void CAstSig::blockCell(const AstNode *pnode, CVar &checkthis)
{
	string msg("Not valid with a cell, struct, or point-array variable ");
	if (checkthis.GetFs()==3)
		throw CAstException(pnode, this, msg.c_str());
	if (checkthis.GetType() == CSIG_CELL)
		if (((CSignal)checkthis).GetType() == CSIG_EMPTY)
			throw CAstException(pnode, this, msg.c_str());
	if (checkthis.GetType() == CSIG_STRUCT)
			throw CAstException(pnode, this, msg.c_str());
}

void CAstSig::blockScalar(const AstNode *pnode, CVar &checkthis)
{
	if (checkthis.GetType() != CSIG_SCALAR && !checkthis.IsEmpty()) return;
	string msg("Not valid with a scalar variable ");
	throw CAstException(pnode, this, msg.c_str());
}

void CAstSig::blockString(const AstNode *pnode, CVar &checkthis)
{
	if (checkthis.GetType() == CSIG_STRING) {
		string msg("Not valid with a string variable ");
		throw CAstException(pnode, this, msg.c_str());
	}
}

void CAstSig::blockComplex(const AstNode *pnode, CVar &checkthis)
{
	if (checkthis.IsComplex()) {
		string msg("Not valid with a complex variable ");
		throw CAstException(pnode, this, msg.c_str());
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
		if (order < 1) throw CAstException(pnode, past, "non-negative argument is required");
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
		throw CAstException(pnode, past, fnsigs, "argument must be positive.");
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
		throw CAstException(pnode, past, fnsigs, "argument must be positive.");
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
		throw CAstException(pnode, past, fnsigs, "argument must be positive.");
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

#ifndef NO_RESAMPLE

void _caret(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{ // ~ operator
	// The second arg is compress_ratio.
	past->checkAudioSig(pnode, past->Sig);
	CSignals sig = past->Sig;
	char errstr[256] = "";
	past->Compute(p);
	if (past->Sig.IsScalar())
	{
		double param = past->Sig.value();
		int orgfs = sig.GetFs();
		if (!sig.Resample((int)round(orgfs/param), errstr)) throw CAstException(pnode, past, string(errstr));
		sig.SetFs(orgfs);
	}
	else if (past->Sig.GetType()==CSIG_VECTOR)
	{
		CSignal param((CSignal)past->Sig);
		unsigned int len(past->Sig.nSamples);
		vector<unsigned int> fsholder, lengthholder;
		int fs = sig.GetFs();
		for (size_t k(0); k<param.nSamples; k++) fsholder.push_back((int)(fs/param.buf[k]+.5));
		splitevenindices(lengthholder, sig.nSamples, len);
		if (!sig.Resample(fsholder, lengthholder, errstr)) throw CAstException(pnode, past, string(errstr));
	}
	past->Sig = sig;
}

void _fmm(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	CSignals audio = past->Sig;
	char errstr[256];
	int multiplier = 10;
	CVar param = past->Compute(p);
	past->blockCell(pnode, param);
	past->blockString(pnode, param);
	if (param.nSamples >= audio.nSamples/10)
		throw CAstException(pnode, past, fnsigs, "2nd argument must have 1/10 or less the length of the 1st arg.");
	past->Sig = audio;
	if (past->Sig.fm2(param, multiplier, errstr)==NULL)
		throw CAstException(pnode, past, fnsigs, string(errstr));
}
#endif //NO_RESAMPLE

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
		throw CAstException(pnode, past, string("Internal error! RegExp.Parse( ) failed.")); ;
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
					throw CAstException(pnode, past, fnsigs, "Not enough arguments.");
				if (nGroupIndex == 2 && *szStart == 's')
					vstring = tast.ComputeString(p);
				else if (tast.Compute(p).IsScalar())
					v = tast.Sig.value();
				else
					throw CAstException(pnode, past, fnsigs, "Scalar value expected for this argument.");
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
		unsigned int n = (unsigned int)past->Sig.CSignal::length();
		past->Sig.UpdateBuffer(n + (unsigned int)strlen(&outStr[0])+1);
		strcpy(&past->Sig.strbuf[n], &outStr[0]);
	}
	unsigned int n = (unsigned int)past->Sig.CSignal::length();
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
		throw CAstException(pnode, past, fnsigs, "First arg must be a file identifider");
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
	if (!first.IsScalar()) throw CAstException(pnode, past, string("All arguments must be scalars (check 1st arg)."));
	double val1, val2, step;
	CSignals third, second = past->Compute(p->next);
	if (!second.IsScalar()) throw CAstException(pnode, past, string("All arguments must be scalars (check 2nd arg).")); 
	val1 = first.value();
	val2 = second.value();
	if (p->next->next) {
		third = past->Compute(p->next->next);
		if (!third.IsScalar()) throw CAstException(pnode, past, string("All arguments must be scalars (check 3rd arg).")); 
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
	throw CAstException(pnode, past, "User raised error - " + errmsg);
}

#ifndef NO_SF
// It is better to have setfs as a hook command, because it involves UI and expressions such as setfs(16000)+1 don't make sense.
// Keep this just in case. But this will not update existing variables according to the new sample rate, so its functionality is pretty limited.
// Use an auxlab hook command in this format #setfs 16000
// 11/21/2017
void _setfs(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (!past->Sig.IsScalar()) throw CAstException(p, past, string("Scalar value should be provided for the sample rate."));
	double fs = past->Sig.value();
	if (fs<500.) throw CAstException(p, past, "Sample rate should be at least 500 Hz.");
	past->FsFixed=true;
	past->pEnv->Fs = (int)fs; //Sample rate adjusted
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
			throw CAstException(p, &tp, "Empty filename");
		filename = tp.Sig.string();
		if (p->next!=NULL)
		{
			third = tp.Compute(p->next);
			tp.checkString(p->next, (CVar)third);
			option = third.string();
		}
	}
	catch (const CAstException &e) {	throw CAstException(pnode, past, fnsigs, e.getErrMsg());	}
	filename = past->MakeFilename(filename, "wav");
	char errStr[256];
	if (!past->Sig.Wavwrite(filename.c_str(), errStr, option))
		throw CAstException(p, past, string(errStr));
}


#endif // NO_SF

#ifdef _WINDOWS
#ifndef NO_PLAYSND

void _play(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar sig = past->Sig;
	//sig must be either an audio signal or an audio handle.
	char errstr[256] = {};
	int nRepeats(1);
	double block = past->audio_block_ms;
	if (sig.GetType() != CSIG_AUDIO)
	{
		if (!sig.IsScalar())
			throw CAstException(pnode, past, fnsigs, "The base must be an audio signal or an audio handle (1)");
		if (sig.strut.find("type")== sig.strut.end())
			throw CAstException(pnode, past, fnsigs, "The base must be an audio signal or an audio handle (2)");
		CSignals type = sig.strut["type"];
		if (type.GetType()!=CSIG_STRING || type.string() != "audio_playback")
			throw CAstException(pnode, past, fnsigs, "The base must be an audio signal or an audio handle (3)");
		if (!p)
			throw CAstException(pnode, past, fnsigs, "Audio signal not given.");
		past->Compute(p);
		past->checkAudioSig(p, past->Sig);
		CSignals audio = past->Sig;
		if (p->next)
		{
			past->Compute(p->next);
			if (!past->Sig.IsScalar())
				throw CAstException(p, past, fnsigs, "Argument must be a scalar.");
			nRepeats = (int)past->Sig.value();
			if (nRepeats<1)
				throw CAstException(p, past, fnsigs, "Repeat counter must be equal or greater than one.");
		}
		INT_PTR h = audio.PlayArrayNext((INT_PTR)sig.value(), 0, WM__AUDIOEVENT, &block, errstr, nRepeats);
		if (!h)
			past->Sig.SetValue(-1.);
		else
		{
			AUD_PLAYBACK * p = (AUD_PLAYBACK*)h;
			past->Sig = *p->pvar;
		}
	}
	else
	{
		if (p)
		{
			past->Compute(p);
			if (!past->Sig.IsScalar())
				throw CAstException(p, past, fnsigs, "Argument must be a scalar.");
			nRepeats = (int)past->Sig.value();
			if (nRepeats<1)
				throw CAstException(p, past, fnsigs, "Repeat counter must be equal or greater than one.");
		}
		int devID = 0;
		INT_PTR h = sig.PlayArray(devID, WM__AUDIOEVENT, GetHWND_WAVPLAY(), &block, errstr, nRepeats);
		if (!h)
		{ // PlayArray will return 0 if unsuccessful due to waveOutOpen failure. For other reasons.....
			past->Sig.strut.clear();
			//errstr should show the err msg. Use it if necessary 7/23/2018
		}
		else
		{
			AUD_PLAYBACK * p = (AUD_PLAYBACK*)h;
			CVar *pplayObj = new CVar;
			pplayObj->SetValue((double)(INT_PTR)h);
			pplayObj->strut["data"] = sig;
			pplayObj->strut["type"] = string("audio_playback");
			pplayObj->strut["devID"] = CSignals((double)devID);
			pplayObj->strut["totalDurMS"] = CSignals(sig.alldur()*nRepeats);
			pplayObj->strut["remDurMS"] = CSignals(sig.alldur()*nRepeats);
			p->pvar = pplayObj;
			past->Sig = *pplayObj; //only to return to xcom
		}
	}
}

void _playstop(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar sig = past->Sig;
	if (!sig.IsScalar())
		throw CAstException(pnode, past, fnsigs, "Argument must be a scalar.");
	string fname = past->pAst->str;
	if (!p)
		fname = past->pAst->alt->str;
	bool res = StopPlay((INT_PTR)sig.value(), fname == "qstop");
	if (!res)
	{
		past->Sig.strut.clear();
		past->Sig.SetValue(-1.);
	}
	else
	{
		//double remaining = fname == "qstop" ? 0. : 350.;
		//past->Sig.strut["remainingDuration"] = CSignals(remaining);
	}
}

void _playStatus(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{ // To be done 7/9/2018
}

void _pause_resume(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar sig = past->Sig;
	if (!sig.IsScalar())
		throw CAstException(pnode, past, fnsigs, "Argument must be a scalar.");
	string fname = past->pAst->str;
	if (!p)
		fname = past->pAst->alt->str;
	bool res = PauseResumePlay((INT_PTR)sig.value(), fname == "resume");
	if (!res)
	{
		past->Sig.strut.clear();
		past->Sig.SetValue(-1.);
	}
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
			throw CAstException(pnode, past, "Including "+filename+"\n\nIn the file: \n"+errmsg);
		}
	} else
		throw CAstException(pnode, past, "Cannot read file", filename);
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
		throw CAstException(pnode, past, "While evaluating\n"+str+"\n"+errmsg);
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
		throw CAstException(pnode, past, fnsigs, "argument must be a text string.");
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
		throw CAstException(p, past, fnsigs, "argument must be a scalar.");
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
		throw CAstException(p, past, fnsigs, "argument must be a scalar.");
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
		throw CAstException(pnode, past, fnsigs, e.getErrMsg());
	}
	if (!second.IsScalar())
		throw CAstException(pnode, past, fnsigs, "argument must be a scalar.");
	double val = second.value();
	if (val!=(double)(int)val)
		throw CAstException(pnode, past, fnsigs, "argument must be an integer.");
	double nCols = past->Sig.nSamples / val;
	if (past->Sig.GetType() == CSIG_VECTOR)
	{
		if (past->Sig.nSamples / val != (int)nCols)
			throw CAstException(pnode, past, fnsigs, "The length of array must be divisible by the requested the row count.");
	}
	else if (past->Sig.GetType() == CSIG_AUDIO)
	{
		if (past->Sig.chain)
			throw CAstException(pnode, past, fnsigs, "To make a matrix from an audio signal, null portions should be filled with zeros. Call contig().");
		double rem = fmod((double)past->Sig.nSamples, val);
		if (rem > 0)
		{
			unsigned int nPtsNeeded = (unsigned int)(val - rem);
			past->Sig.UpdateBuffer(past->Sig.nSamples + nPtsNeeded);
		}
	}
	else
		throw CAstException(pnode, past, fnsigs, "Vector or audio signals required.");
	past->Sig.nGroups = (int)val;
}

void _cell(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (!past->Sig.IsScalar())
		throw CAstException(p, past, fnsigs, "argument must be a scalar.");
	int n = (int)round(past->Sig.value());
	if (n <= 0)
		throw CAstException(p, past, fnsigs, "argument must be a positive number.");

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
		throw CAstException(p, past, fnsigs, "argument must be a string.");
	string arg = past->Sig.string();
	char drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH], ext[MAX_PATH];
	_splitpath(arg.c_str(), drive, dir, fname, ext);
	if (strlen(fname)==0 && strlen(ext)==0)
		arg += "\\*.*";
#ifdef _WINDOWS
	WIN32_FIND_DATA ls;
	HANDLE hFind = FindFirstFile(arg.c_str(), &ls);
	if (hFind == INVALID_HANDLE_VALUE)
//	if ((hFile = _findfirst(, &c_file)) == -1L)
		throw CAstException(p, past, fnsigs, "No files in the specified directory");
	else
	{
		past->Sig.Reset();
		do {
			_splitpath(ls.cFileName, drive, dir, fname, ext);
			char fullname[256];
			CVar tp;
			tp.strut["name"] = CSignals(CSignal(string(fname)));
			tp.strut["ext"] = CSignals(CSignal(string(ext)));
			char **lppPart = { NULL };
			DWORD dw = GetFullPathName(fname, 256, fullname, lppPart);
			if (dw)
			{
				if (fname[0] != '.' || fname[1] != '\0')
				{
					char *pt = strstr(fullname, fname);
					if (pt) *pt = 0;
					tp.strut["path"] = CSignals(CSignal(string(fullname)));
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
			}
		} while (FindNextFile(hFind, &ls));
	}
#elif
#endif
}

void _isnull(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{ // if the signal is null at specified time_pt
	past->checkAudioSig(pnode, past->Sig);
	CAstSig tp(past);
	tp.Compute(p);
	if (tp.Sig.GetType() != CSIG_SCALAR)
		throw CAstException(pnode, past, fnsigs, "argument must be a scalar.");
	double vv = tp.Sig.value();
	past->checkAudioSig(pnode, past->Sig);
	past->Sig.SetValue(past->Sig.IsNull(vv));//this failed CHECK IT------3/3/2018 
	past->Sig.MakeLogical();
}

void _varcheck(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	string fname = pnode->str;
	int type = past->Sig.GetType();
	if (fname == "isempty")			past->Sig.SetValue(past->Sig.IsEmpty());
	else if (fname == "isaudio")	past->Sig.SetValue(type == CSIG_AUDIO ? 1 : 0);
	else if (fname == "isvector")	past->Sig.SetValue(type == CSIG_VECTOR ? 1 : 0);
	else if (fname == "isstring")	past->Sig.SetValue(type == CSIG_STRING ? 1 : 0);
	else if (fname == "iscell")		past->Sig.SetValue((double)(int)!past->Sig.cell.empty());
	else if (fname == "isclass")	past->Sig.SetValue((double)(int)!past->Sig.strut.empty());
	else if (fname == "isbool")		past->Sig.SetValue(past->Sig.bufBlockSize == 1 && past->Sig.GetFs() != 2);
	else if (fname == "isstereo")	past->Sig.SetValue(past->Sig.next != NULL ? 1 : 0);
	else if (fname == "istseq")		past->Sig.SetValue(type == CSIG_TSERIES ? 1 : 0);
	past->Sig.MakeLogical();
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
			throw CAstException(pnode, past, fnsigs, "argument must be a logical variable.");
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
			throw CAstException(pnode, past, fnsigs, "argument must be a logical variable.");
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
				throw CAstException(p, past, "argument must be a positive integer.");
		}
		catch (const CAstException &e) {
			throw CAstException(pnode, past, fnsigs, e.getErrMsg());
		}
	}
	past->Sig = past->Sig.basic(past->Sig.pf_basic3 = &CSignal::FFT, (void*)&param);
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
				throw CAstException(p, past, "argument must be a positive integer.");
		}
		catch (const CAstException &e) {
			throw CAstException(pnode, past, fnsigs, e.getErrMsg());
		}
	}
	past->Sig = past->Sig.basic(past->Sig.pf_basic3 = &CSignal::iFFT, (void*)&param);
}

void _envelope(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	past->Sig.basic(past->Sig.pf_basic2 = &CSignal::HilbertEnv);
}

void _hilbert(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkAudioSig(pnode,  past->Sig);
	past->Sig.basic(past->Sig.pf_basic2 = &CSignal::Hilbert);
}

void _movespec(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	CAstSig tp(past);
	CSignals param = tp.Compute(p);
	if (!param.IsScalar())
		throw CAstException(p, past, fnsigs, "parameter must be a scalar (frequency to shift).");
	double shift = param.value();
	past->Sig.basic(past->Sig.pf_basic2 = &CSignal::ShiftFreq, &shift); // NOT YET
}

void _tscale(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{ // Currently not used. both tscale and fscale are processed from CallUDF()  v1.47
	past->checkAudioSig(pnode, past->Sig);
	CAstSig tp(past);
	CSignals param = tp.Compute(p);
	if (!param.IsScalar())
		throw CAstException(p, past, fnsigs, "parameter must be a scalar (ratio to compress time).");
	CAstSig tp2(past);
	char str[256];
	string emsg;
	sprintf(str, "tscale(%s,%g)", pnode->str, 1. / param.value());
	tp2.SetNewScript(emsg, str);
	tp2.Compute();
	past->Sig = tp2.Sig;
}

void _fscale(CAstSig *past, const AstNode *pnode, const AstNode *p, std::string &fnsigs)
{ // Currently not used v1.47
	past->checkAudioSig(pnode, past->Sig);
	CAstSig tp(past);
	CSignals param = tp.Compute(p);
	if (!param.IsScalar())
		throw CAstException(p, past, fnsigs, "parameter must be a scalar (amount of pitch lowering in semitones).");
	double fchange = 12. * log(1. / param.value()) / log(2);
	CAstSig tp2(past);
	char str[256];
	string emsg;
	sprintf(str, "fscale(%s,%g)", pnode->str, fchange);
	tp2.SetNewScript(emsg, str);
	tp2.Compute();
	past->Sig = tp2.Sig;
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
			throw CAstException(pnode, past, fnsigs, "2nd argument must be a scalar.");
		if (past->Sig.value() < 0) order = -1.;
	}
	past->Sig = sig.basic(sig.pf_basic2 = &CSignal::sort, &order);
}

void _decfir(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CSignals second = past->Compute(p->next);
	CSignals third = past->Compute(p->next->next);
	if (!third.IsScalar())
		throw CAstException(pnode, past, fnsigs, "3rd argument must be a scalar: offset");
	CSignals fourth = past->Compute(p->next->next->next);
	if (!fourth.IsScalar())
		throw CAstException(pnode, past, fnsigs, "4th argument must be a scalar: nChan");
	int offset = (int)third.value()-1; // because AUX is one-based
	int nChan = (int)fourth.value();
	if (offset>nChan)
		throw CAstException(pnode, past, fnsigs, "nChan must be equal or greater than offset");
	past->Compute(p);
	past->Sig.DecFir(second, offset, nChan);
}

void _filtDynamic(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	CSignals sig = past->Sig;
	string fname = pnode->str;
	CSignals third, second = past->Compute(p); //second must be tseq
	third = past->Compute(p->next); // third must be the order

	//turn tseq to vector
	vector<double> coeff = second.ToVector();
	vector<double> order(1, third.value());
	vector<vector<double>> dynamicfilterparams;
	dynamicfilterparams.push_back(coeff);
	dynamicfilterparams.push_back(order);
	if (third.value()<0)
		sig.basic(sig.pf_basic2 = &CSignal::dynaAR, &dynamicfilterparams);
	else
		sig.basic(sig.pf_basic2 = &CSignal::dynaMA, &dynamicfilterparams);
	past->Sig = sig;
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
			throw CAstException(pnode, past, fnsigs, "2nd argument must be a vector(the numerator array for filtering).");
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
			sig.basic(sig.pf_basic2 = &CSignal::filter, &coeffs);
		else if (fname == "filtfilt")
			sig.basic(sig.pf_basic2 = &CSignal::filtfilt, &coeffs);
	}
	else
	{
		vector<CSignals*> TScoeff;
		TScoeff.push_back(&second);
		TScoeff.push_back(&third);
		if (fname == "filt")
			sig.basic(sig.pf_basic2 = &CSignal::dynafilter, &TScoeff);
		else if (fname == "filtfilt")
			throw CAstException(pnode, past, fnsigs, "Dynamic filtering cannot be applied to filtfilt");
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

		sigX.basic(sigX.pf_basic2 = &CSignal::IIR, &params);
		past->Sig = sigX;
	}
	catch (const char* estr) 
	{
		sformat(emsg, "Invalid argument---%s\nFor more of ELLF Digital Filter Calculator, check http://www.moshier.net/index.html.", estr);
		throw CAstException(pnode, past, fnsigs, emsg);
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
	past->Sig.basic(past->Sig.pf_basic2 = &CSignal::Hamming);
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
	past->Sig.basic(past->Sig.pf_basic2 = &CSignal::Blackman, &alpha);
}

void _fm(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CSignals fifth, second = past->Compute(p->next);
	CSignals third = past->Compute(p->next->next);
	CSignals fourth = past->Compute(p->next->next->next);
	if (!second.IsScalar()) throw CAstException(p, past, fnsigs, "freq2 must be a scalar.");
	if (!third.IsScalar()) throw CAstException(p, past, fnsigs, "mod_rate must be a scalar.");
	if (!fourth.IsScalar()) throw CAstException(p, past, fnsigs, "duration must be a scalar.");
	if (fourth.value()<=0) throw CAstException(p, past, fnsigs, "duration must be positive.");
	if (p->next->next->next->next) {	// 5 args
		fifth = past->Compute(p->next->next->next->next);
		if (!fifth.IsScalar()) throw CAstException(p, past, fnsigs, "init_phase must be a scalar."); }
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
				throw CAstException(p, past, fnsigs, "Invalid parameter: should be either 0 (divided by n-1; default) or 1 (divided by n).");
		}
		catch (const CAstException &e) { throw CAstException(pnode, past, fnsigs, e.getErrMsg()); }
	}
	past->Sig = past->Sig.basic(past->Sig.pf_basic = &CSignal::stdev, &arg);
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
		catch (const CAstException &e) { throw CAstException(pnode, past, fnsigs, e.getErrMsg()); }
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
		throw CAstException(p, past, fnsigs, "Invalid parameter: should be either 1 or 2.");
}

void _arraybasic(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar additionalArg(past->Sig.GetFs());
	string fname = pnode->str;
	if (fname == "sum")	past->Sig = past->Sig.basic(past->Sig.pf_basic = &CSignal::sum);
	else if (fname == "mean") past->Sig = past->Sig.basic(past->Sig.pf_basic = &CSignal::mean);
	else if (fname == "length")
	{
		if (past->Sig.next) 
			throw CAstException(p, past, "Cannot be a stereo audio signal.");
		if (past->Sig.IsGO())
			past->Sig = past->Sig.length();
		else
			past->Sig = past->Sig.basic(past->Sig.pf_basic = &CSignal::length);
	}
	else
	{
		past->checkAudioSig(p, past->Sig);
		if (fname == "begint") past->Sig = past->Sig.basic(past->Sig.pf_basic = &CSignal::begint);
		else if (fname == "endt") past->Sig = past->Sig.basic(past->Sig.pf_basic = &CSignal::endt);
		else if (fname == "dur") past->Sig = past->Sig.basic(past->Sig.pf_basic = &CSignal::dur);
		else if (fname == "rms") past->Sig = past->Sig.basic(past->Sig.pf_basic = &CSignal::RMS);
		else if (fname == "rmsall") past->Sig = past->Sig.RMS(); // overall RMS; scalar or two-element array
	}
	if (past->pAst->type == N_VECTOR)
		if (past->pAst->alt->next)
			past->SetVar(past->pAst->alt->next->str, &additionalArg);
}

void _mostleast(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (past->Sig.IsEmpty()) return; //for empty input, empty output
	past->checkVector(pnode, past->Sig);
	string func = pnode->str;
	CVar sig = past->Sig;
	CVar param = past->Compute(p);
	if (func == "atmost") past->Sig = sig.basic(sig.pf_basic2 = &CSignal::_atmost, &param);
	else if (func == "atleast") past->Sig = sig.basic(sig.pf_basic2 = &CSignal::_atleast, &param);
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
	if (fname == "max") past->Sig = sig.basic(sig.pf_basic = &CSignal::_max, pt);
	else if (fname == "min") past->Sig = sig.basic(sig.pf_basic = &CSignal::_min, pt);
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
	catch (const CAstException &e) { throw CAstException(pnode, past, fnsigs, e.getErrMsg()); }
	if (!param.IsScalar())
		throw CAstException(p, past, fnsigs, "Ramp_duration must be a scalar.");
	double ramptime = param.value();
	past->Sig.basic(past->Sig.pf_basic2 = &CSignal::dramp, &ramptime);
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
			throw CAstException(p, past, fnsigs, "AM_rate must be a scalar.");
		modRate = rate.value();
		int nArgs(0);
		for (const AstNode *cp = p; cp; cp = cp->next)
			++nArgs;
		if (nArgs >= 2) //either 2 or 3
		{
			depth = tp.Compute(p->next);
			if (!depth.IsScalar())
				throw CAstException(p, past, fnsigs, "AM_depth_m must be a scalar.");
			amDepth = depth.value();
			if (nArgs == 3)
			{
				initphase = tp.Compute(p->next->next);
				if (!initphase.IsScalar())
					throw CAstException(p, past, fnsigs, "initial_phase must be a scalar.");
				initPhase = initphase.value();
			}
		}
	}
	catch (const CAstException &e) { throw CAstException(pnode, past, fnsigs, e.getErrMsg()); }
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
	if (!past->Sig.IsVector() || past->Sig.nSamples>2)
		throw CAstException(p, past, fnsigs, "Frequency must be either a constant or two-element array.");
	body freq = past->Sig; //should be same as tp.Compute(p);
	if (freq._max() >= past->GetFs() / 2)
		throw CAstException(p, past, fnsigs, "Frequency exceeds Nyquist frequency.");
	CVar duration = past->Compute(p->next);
	past->checkScalar(pnode, duration);
	if (duration.value() <= 0)
		throw CAstException(p, past, fnsigs, "Duration must be positive.");
	if (nArgs == 3)
	{
		CSignals _initph = past->Compute(p->next->next);
		if (!_initph.IsScalar())
			throw CAstException(p, past, fnsigs, "Initial_phase must be a scalar.");
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
		throw CAstException(p, past, fnsigs, "duration must be a scalar.");
	if (dur.value()<0.)
		throw CAstException(p, past, fnsigs, "duration must be a non-negative number.");
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
		throw CAstException(p, past, fnsigs, "Internal error 3426.");
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
		throw CAstException(p, past, string(errStr));
	vector<string> audiovars;
	EnumAudioVariables(past, audiovars);
	if (past->FsFixed || audiovars.size()>0)
	{
		int old = past->Sig.GetFs();
		if (past->Sig.GetFs() != past->GetFs() && !past->Sig.Resample(past->GetFs(), errStr))
			throw CAstException(p, past, string(errStr) + " " + filename);
		if (old != past->Sig.GetFs())
			sformat(past->statusMsg, "(NOTE)File fs=%d Hz. The audio data resampled to %d Hz.", old, past->Sig.GetFs());
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
	char fname[MAX_PATH], ext[MAX_PATH];
	FILE *fp(NULL);
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
		else if (GetFileText(fullpath.c_str(), "rb", content))
		{
			past->Sig.Reset();
			vector<string> line;
			size_t nLines = str2vect(line, content.c_str(), "\r\n");
			char buf[256];
			int dataSize(16), nItems, res;
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

void _gcf(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	if (CAstSig::vecast.front()->GOvars.find("gcf") != CAstSig::vecast.front()->GOvars.end())
		past->Sig = *(past->pgo = CAstSig::vecast.front()->GOvars["gcf"].front());
	else
		throw CAstException(pnode, past, fnsigs, "CUrrent figure must be set.");
}

void _sel(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{

}

void CAstSigEnv::InitBuiltInFunctionList()
{
	srand((unsigned)time(0) ^ (unsigned int)GetCurrentThreadId());

	CBIF pp;

	pp.name = "tone";
	pp.alwaysstatic = true;
	pp.funcsignature = "(frequency, duration_in_ms [, initial_phase_between_0&1=0])";
	pp.narg1 = 2;	pp.narg2 = 3;
	built_in_func_names.push_back(pp.name);
	inFunc[pp.name] =  &_tone;
	built_in_funcs.push_back(pp);

	pp.name = "sam";
	pp.alwaysstatic = false;
	pp.funcsignature = "(signal, AM_rate[, AM_depth_m, initial_phase_between_0 & 1 = 0])";
	pp.narg1 = 2;	pp.narg2 = 4;
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_sam;
	built_in_funcs.push_back(pp);

	pp.name = "fm";
	pp.alwaysstatic = true;
	pp.funcsignature = "(freq1, freq2, mod_rate, duration, [init_phase=0])";
	pp.narg1 = 4;	pp.narg2 = 5;
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_fm;
	built_in_funcs.push_back(pp);

	pp.funcsignature = "(duration_in_ms)";
	pp.alwaysstatic = true;
	pp.narg1 = 1;	pp.narg2 = 1;
	const char *f0[] = { "noise", "gnoise", "silence", "dc",0 };
	for (int k = 0; f0[k]; k++)
	{
		pp.name = f0[k];
		built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_tparamonly;
		built_in_funcs.push_back(pp);
	}

	pp.narg1 = 2;	pp.narg2 = 3;
	pp.alwaysstatic = true;
	pp.name = ":";
	pp.funcsignature = "(scalar:scalar) or (scalar:scalar:scalar)";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_colon; 
	built_in_funcs.push_back(pp);

	pp.name = "wave";
	pp.alwaysstatic = true;
	pp.funcsignature = "(filename)";
	pp.narg1 = 1;	pp.narg2 = 1;
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_wave;
	built_in_funcs.push_back(pp);

	pp.name = "wavwrite";
	pp.alwaysstatic = false;
	pp.funcsignature = "(audio_signal, filename[, option])";
	pp.narg1 = 2;	pp.narg2 = 3;
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_wavwrite;
	built_in_funcs.push_back(pp);

	pp.name = "setfs"; // check this... is narg1 one correct?
	pp.alwaysstatic = true;
	pp.funcsignature = "(filename)";
	pp.narg1 = 1;	pp.narg2 = 1;
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_setfs;
	built_in_funcs.push_back(pp);

	pp.name = "audio";
	pp.alwaysstatic = false;
	pp.funcsignature = "(non_audio_vector)";
	pp.narg1 = 1;	pp.narg2 = 1;
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_audio;
	built_in_funcs.push_back(pp);

	pp.name = "squeeze"; 
	pp.alwaysstatic = false;
	pp.funcsignature = "() ... to remove the null interval";
	pp.narg1 = 0;	pp.narg2 = 0;
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_squeeze; // do this
	built_in_funcs.push_back(pp);

	pp.name = "decfir";
	pp.alwaysstatic = false;
	pp.funcsignature = "(signal, coeff_vector, offset, nChan)";
	pp.narg1 = 4;	pp.narg2 = 4;
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_decfir; // do this
	built_in_funcs.push_back(pp);

	// begin narg 2 and 2
	pp.alwaysstatic = false;
	pp.narg1 = 2;
	pp.narg2 = 2;

	pp.name = "ramp";
	pp.funcsignature = "(signal, ramp_duration)";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_ramp;
	built_in_funcs.push_back(pp);

	pp.name = "isnull";
	pp.funcsignature = "(audio_signal, time_pt)";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_isnull;
	built_in_funcs.push_back(pp);

	pp.name = "fmm";
	pp.funcsignature = "(audio_signal, freq_variation_array)";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_fmm;
	built_in_funcs.push_back(pp);

	pp.name = "interp";
	pp.funcsignature = "(audio_signal, playback_rate_change_ratio)";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_caret;
	built_in_funcs.push_back(pp);

	// end narg 2 and 2

	pp.narg1 = 1;	pp.narg2 = 1;
	pp.name = "hamming";
	pp.funcsignature = "(audio_signal)";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_hamming;
	built_in_funcs.push_back(pp);
	pp.name = "hann";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_blackman;
	built_in_funcs.push_back(pp);
	pp.narg1 = 1;	pp.narg2 = 2;
	pp.name = "blackman";
	pp.funcsignature = "(audio_signal, [alpha=0.16])";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_blackman;
	built_in_funcs.push_back(pp);

#ifndef NO_IIR
	pp.narg1 = 2;	pp.narg2 = 6;
	const char *f2[] = { "lpf", "hpf", 0 };
	pp.funcsignature = "(signal, freq, [order=4], [kind=1], [dB_passband_ripple=0.5], [dB_stopband_atten=-40])\n  --- kind: 1 for Butterworth, 2 for Chebyshev, 3 for Elliptic";
	for (int k = 0; f2[k]; k++)
	{
		pp.name = f2[k];
		built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_iir;
		built_in_funcs.push_back(pp);
	}
	pp.narg1 = 3;	pp.narg2 = 7;
	const char *f3[] = { "bpf", "bsf", 0 };
	pp.funcsignature = "(signal, freq, [order=4], [kind=1], [dB_passband_ripple=0.5], [dB_stopband_atten=-40])\n  --- kind: 1 for Butterworth, 2 for Chebyshev, 3 for Elliptic";
	for (int k = 0; f3[k]; k++)
	{
		pp.name = f3[k];
		built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_iir;
		built_in_funcs.push_back(pp);
	}
#endif //NO_IIR
	pp.narg1 = 1;	pp.narg2 = 1;
	pp.funcsignature = "(stereo_signal)";
	pp.name = "left";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_left; // check 
	built_in_funcs.push_back(pp);
	pp.name = "right";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_right; // check 
	built_in_funcs.push_back(pp);

	pp.narg1 = 0;	pp.narg2 = 1;
	pp.name = "std";
	pp.funcsignature = "(length, [, bias])";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_std; 
	built_in_funcs.push_back(pp);

	
	pp.narg1 = 2;	pp.narg2 = 2;
	const char *f4[] = { "atleast", "atmost", 0 };
	pp.funcsignature = "(array_or_value, array_or_value_of_limit)";
	for (int k = 0; f4[k]; k++)
	{
		pp.name = f4[k];
		built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_mostleast;
		built_in_funcs.push_back(pp);
	}

	pp.narg1 = 1;	pp.narg2 = 1;
	const char *f5[] = { "min", "max", 0 };
	pp.funcsignature = "(vector) or (scalar1, scalar2, ...)";
	for (int k = 0; f5[k]; k++)
	{
		pp.name = f5[k];
		built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_minmax;
		built_in_funcs.push_back(pp);
	}

	pp.narg1 = 1;	pp.narg2 = 1;
	pp.funcsignature = "(array)";
	const char *f6[] = { "sum", "mean", "length", 0 };
	for (int k = 0; f6[k]; k++)
	{
		pp.name = f6[k];
		built_in_func_names.push_back(pp.name); inFunc[pp.name] = _arraybasic;
		built_in_funcs.push_back(pp);
	}

	pp.narg1 = 1;	pp.narg2 = 2;
	pp.funcsignature = "(array [, dimension_either_1_or_2])";
	pp.name = "size";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_size;
	built_in_funcs.push_back(pp);

	pp.narg1 = 1;	pp.narg2 = 1;
	pp.funcsignature = "(graphic_handle)";
	pp.name = "replicate";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_replicate;
	built_in_funcs.push_back(pp);

	pp.narg1 = 1;	pp.narg2 = 2;
	pp.funcsignature = "(array_or_signal[, order=1])";
	pp.name = "diff";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_diff;
	built_in_funcs.push_back(pp);

	pp.narg1 = 2;	pp.narg2 = 2;
	pp.funcsignature = "(audio_signal)";
	pp.name = "nullin";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_nullin;
	built_in_funcs.push_back(pp);
	pp.narg1 = 1;	pp.narg2 = 1;
	pp.name = "contig";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_contig;
	built_in_funcs.push_back(pp);

	pp.narg1 = 1;	pp.narg2 = 1;
	pp.funcsignature = "(array_or_signal)";
	pp.name = "cumsum";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_cumsum;
	built_in_funcs.push_back(pp);

	pp.funcsignature = "(value)";
	pp.name = "rand";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_rand;
	built_in_funcs.push_back(pp);
	pp.name = "irand";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_irand;
	built_in_funcs.push_back(pp);
	pp.name = "randperm";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_randperm;
	built_in_funcs.push_back(pp);
	pp.name = "zeros";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_zeros;
	built_in_funcs.push_back(pp);
	pp.name = "ones";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_ones;
	built_in_funcs.push_back(pp);
	pp.name = "cell";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_cell;
	built_in_funcs.push_back(pp);
	pp.name = "clear";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_clear;
	built_in_funcs.push_back(pp);

	pp.narg1 = 2;	pp.narg2 = 2;
	pp.name = "matrix";
	pp.funcsignature = "(array, nGroups)";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_matrix;
	built_in_funcs.push_back(pp);

	pp.alwaysstatic = true;
	pp.narg1 = 1;	pp.narg2 = 0;
	pp.funcsignature = "(format_string, ...)";
	pp.name = "sprintf";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_sprintf;
	built_in_funcs.push_back(pp);
	pp.name = "fprintf";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_fprintf;
	built_in_funcs.push_back(pp);

	pp.narg1 = 1;	pp.narg2 = 1;
	pp.funcsignature = "(file_ID)";
	pp.name = "fclose";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_fclose;
	built_in_funcs.push_back(pp);

	pp.narg1 = 2;	pp.narg2 = 2;
	pp.name = "fopen";
	pp.funcsignature = "(filename, mode)";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_fopen; // check 
	built_in_funcs.push_back(pp);

	//where is '#'????

	pp.narg1 = 0;	pp.narg2 = 0;
	pp.name = "getfs";
	pp.funcsignature = "()";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_getfs;
	built_in_funcs.push_back(pp);
	pp.name = "tic";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_tictoc;
	built_in_funcs.push_back(pp);
	pp.name = "toc";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_tictoc;
	built_in_funcs.push_back(pp);

	pp.narg1 = 1;	pp.narg2 = 1;
	pp.funcsignature = "(filename)";
	pp.name = "file";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_file;
	built_in_funcs.push_back(pp);
	pp.name = "include";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_include; // check
	built_in_funcs.push_back(pp);
	pp.name = "fdelete";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_fdelete; // check
	built_in_funcs.push_back(pp); 

	pp.funcsignature = "(directory_name)";
	pp.name = "dir";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_dir;
	built_in_funcs.push_back(pp);

	pp.alwaysstatic = false;
	pp.narg1 = 1;	pp.narg2 = 1;
	pp.funcsignature = "(variable)";
	const char *f7[] = { "isempty", "isaudio", "isvector", "isstring", "isstereo", "isbool", "iscell", "isclass", 0 };
	for (int k = 0; f7[k]; k++)
	{
		pp.name = f7[k];
		built_in_func_names.push_back(pp.name); inFunc[pp.name] = _varcheck;
		built_in_funcs.push_back(pp);
	}

	pp.narg1 = 2;	pp.narg2 = 3;
	pp.funcsignature = "(signal, Numerator_array [, Denominator_array=1 for FIR])";
	const char *f8[] = { "filt", "filtfilt", 0 };
	for (int k = 0; f8[k]; k++)
	{
		pp.name = f8[k];
		built_in_func_names.push_back(pp.name); inFunc[pp.name] = _filt;
		built_in_funcs.push_back(pp);
	}
	//For trial only purposes, let's not "publish" this function as a built-in function.
	pp.narg1 = 2;	pp.narg2 = 3;
	pp.name = "dynfilt";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_filtDynamic;
	built_in_funcs.push_back(pp);

#ifndef NO_FFTW

	pp.narg1 = 1;	pp.narg2 = 2;
	pp.name = "fft";
	pp.funcsignature = "(array, [Num_of_FFT_pts=length_of_the_input])";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_fft; // check
	built_in_funcs.push_back(pp);
	pp.name = "ifft";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_ifft;
	built_in_funcs.push_back(pp);

	pp.narg1 = 1;	pp.narg2 = 1;
	pp.funcsignature = "(signal_or_vector)";
	pp.name = "envelope";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_envelope;
	built_in_funcs.push_back(pp);
	pp.name = "hilbert";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_hilbert;
	built_in_funcs.push_back(pp);

	pp.narg1 = 2;	pp.narg2 = 2;
	pp.name = "movespec";
	pp.funcsignature = "(audio_signal, frequency_to_shift)";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_movespec;
	built_in_funcs.push_back(pp);

	pp.name = "tscale";
	pp.funcsignature = "(audio_signal, compression_ratio)";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_tscale;
	built_in_funcs.push_back(pp);

	pp.name = "fscale";
	pp.funcsignature = "(audio_signal, semitone_to_lower)";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_fscale;
	built_in_funcs.push_back(pp);

#endif

	pp.narg1 = 1;	pp.narg2 = 2;
	pp.funcsignature = "(signal_or_vector [, positive_for_acending_negative_for_descending = 1])";
	pp.name = "sort";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_sort;
	built_in_funcs.push_back(pp);


	pp.alwaysstatic = false;
	pp.narg1 = 1;	pp.narg2 = 1;
	pp.funcsignature = "(audio_handle)";
	pp.name = "pause";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_pause_resume;
	built_in_funcs.push_back(pp);
	pp.name = "resume";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_pause_resume;
	built_in_funcs.push_back(pp);
	pp.name = "stop";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_playstop;
	built_in_funcs.push_back(pp);
	pp.name = "qstop";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_playstop;
	built_in_funcs.push_back(pp);
	pp.name = "status";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_playStatus;
	built_in_funcs.push_back(pp);


	pp.funcsignature = "(audio_signal [, repeat=1]) or (audio_handle, audio_signal [, repeat=1])";
	pp.narg1 = 1;	pp.narg2 = 3;
	pp.name = "play";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_play;
	built_in_funcs.push_back(pp);
	pp.funcsignature = "(audio_signal)";
	pp.narg1 = 1;	pp.narg2 = 1;
	pp.name = "vector";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_vector;
	built_in_funcs.push_back(pp);
	pp.name = "dur";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_arraybasic;
	built_in_funcs.push_back(pp);
	pp.name = "begint";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_arraybasic;
	built_in_funcs.push_back(pp);
	pp.name = "endt";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_arraybasic;
	built_in_funcs.push_back(pp);
	pp.name = "rms";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_arraybasic;
	built_in_funcs.push_back(pp);
	pp.name = "rmsall";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_arraybasic;
	built_in_funcs.push_back(pp);

	pp.alwaysstatic = true;
	pp.name = "msgbox";
	pp.narg1 = 1;	pp.narg2 = 0;
	pp.funcsignature = "(msg_body_with_printf_style)";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_msgbox;
	built_in_funcs.push_back(pp);

	pp.alwaysstatic = false;
	pp.name = "inputdlg";
	pp.narg1 = 2;	pp.narg2 = 99;
	pp.funcsignature = "(title, msg_body)";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_inputdlg;
	built_in_funcs.push_back(pp);

	pp.alwaysstatic = false;
	pp.narg1 = 1;	pp.narg2 = 2;
	pp.funcsignature = "(logical_variable) or (logical_variable1, logical_variable2) ";
	pp.name = "and";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_and;
	built_in_funcs.push_back(pp);
	pp.name = "or";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_or;
	built_in_funcs.push_back(pp);

	pp.alwaysstatic = false;
	pp.narg1 = 1;	pp.narg2 = 1;
	pp.funcsignature = "(string)";
	pp.name = "str2num";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_str2num;
	built_in_funcs.push_back(pp);
	pp.alwaysstatic = true;
	pp.name = "eval";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_eval;
	built_in_funcs.push_back(pp);

	pp.narg1 = 1;	pp.narg2 = 1;
	pp.funcsignature = "(value_or_array)";
	const char *fmath1[] = { "abs", "sqrt", "conj", "real", "imag", "angle", "sign", "sin", "cos", "tan", "asin", "acos", "atan", "log", "log10", "exp", "round", "fix", "ceil", "floor", 0 };
	for (int k = 0; fmath1[k]; k++)
	{
		pp.name = fmath1[k];
		built_in_func_names.push_back(pp.name);inFunc[pp.name] =  NULL;
		built_in_funcs.push_back(pp);
	}
	pp.narg1 = 2;	pp.narg2 = 2;
	pp.funcsignature = "(value_or_array, value_or_array)";
	const char *fmath2[] = { "^", "mod", "pow", 0 };
	for (int k = 0; fmath2[k]; k++)
	{
		pp.name = fmath2[k];
		built_in_func_names.push_back(pp.name);inFunc[pp.name] =  NULL;
		built_in_funcs.push_back(pp);
	}

#ifdef _WINDOWS
	pp.alwaysstatic = true;
	pp.narg1 = 1;	pp.narg2 = 1;
	pp.name = "figure";
	pp.funcsignature = "([position(screen_coordinate)]) or (existing_Figure_ID)";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_figure;
	built_in_funcs.push_back(pp);

	pp.alwaysstatic = false;
	pp.narg1 = 1;	pp.narg2 = 2;
	pp.name = "axes";
	pp.funcsignature = "([position]) or (existing_axes_ID)";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] =  &_axes;
	built_in_funcs.push_back(pp);

	pp.narg1 = 3;	pp.narg2 = 4;
	pp.funcsignature = "([graphic_handle], x, y, string)";
	pp.name = "text";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_text;
	built_in_funcs.push_back(pp);

	pp.narg1 = 1;	pp.narg2 = 4;
	pp.funcsignature = "([graphic_handle], variable [, options]) or (vector_x, vector_y [, options])";
	pp.name = "plot";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_plot;
	built_in_funcs.push_back(pp);
	pp.name = "line";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_line;
	built_in_funcs.push_back(pp);

	pp.name = "delete";
	pp.alwaysstatic = false;
	pp.funcsignature = "(graphic_handle)";
	built_in_func_names.push_back(pp.name); inFunc[pp.name] = &_delete_graffy;
	built_in_funcs.push_back(pp);
#endif // _WINDOWS

	pp.name = "input";
	pp.alwaysstatic = false;
	pp.funcsignature = "(prompt_message)";
	built_in_func_names.push_back(pp.name);inFunc[pp.name] = &aux_input;
	built_in_funcs.push_back(pp);

	pp.name = "i";
	pp.alwaysstatic = false;
	pp.narg1 = pp.narg2 = 0;
	pp.funcsignature = "";
	inFunc[pp.name] = &_imaginary_unit;
	pseudo_vars[pp.name] = pp;
	pp.name = "e";
	inFunc[pp.name] = &_natural_log_base;
	pseudo_vars[pp.name] = pp;
	pp.name = "pi";
	inFunc[pp.name] = &_pi;
	pseudo_vars[pp.name] = pp;
	pp.name = "gcf";
	inFunc[pp.name] = &_gcf;
	pseudo_vars[pp.name] = pp;
	//pp.name = "gca";
	//pseudo_vars[pp.name] = pp;
	//inFunc[pp.name] = &_gca;
	//pp.name = "sel";
	//pseudo_vars[pp.name] = pp;
	//inFunc[pp.name] = &_sel;
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

void CAstSig::HandlePseudoVar(const AstNode *pnode)
{
	string fname = pnode->str;
	string dummy;
	if (pEnv->pseudo_vars.find(fname) != pEnv->pseudo_vars.end())
	{
		pEnv->inFunc[fname](this, pnode, NULL, dummy);
	}
	else
		throw CAstException(pnode, this, fname, "Not a valid pseudo variable.");
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
			if (p && p->type != N_STRUCT) throw CAstException(pnode, this, fname, "Superfluous parameter(s)");
		}
		else
		{
			if (!p) throw CAstException(pnode, this, "", "Empty parameter");
			Compute(p);
		}
		if (Sig.IsStruct() || !Sig.cell.empty() || Sig.GetFs() == 3)
		{
			if (structCall)
			{
				string dotnotation = dotstring(pnode, pRoot);
				throw CAstException(pnode, this, dotnotation, "Cannot take cell, class or handle object");
			}
			else
				throw CAstException(p, this, fname, "Cannot take cell, class or handle object");
		}
		Sig.MFFN(fn0, cfn1);
		break;
	case 2: // two in args, single out arg
		if (fname == "^" || fname == "pow")	fn2 = pow, cfn2 = r2c_pow;
		else if (fname == "mod")	fn2 = fmod, cfn2 = NULL; // do this
		if (structCall)
		{
			if (!p || p->type == N_STRUCT) throw CAstException(pnode, this, "", "requires second parameter.");
			CVar Sig0 = Sig;
			if (Sig.IsStruct() || !Sig.cell.empty() || Sig.GetFs() == 3)
				throw CAstException(pnode, this, fname, "Cannot take cell, class or handle object");
			param = Compute(p);
			Sig = Sig0.MFFN(fn2, cfn2, param);
		}
		else
		{
			if (!p->next) throw CAstException(pnode, this, "", "requires second parameter.");
			param = Compute(p->next);
			Compute(p);
			if (Sig.IsStruct() || !Sig.cell.empty() || Sig.GetFs() == 3)
			{
				if (structCall)
				{
					string dotnotation = dotstring(pnode, pRoot);
					throw CAstException(pnode, this, dotnotation, "Cannot take cell, class or handle object");
				}
				else
					throw CAstException(p, this, fname, "Cannot take cell, class or handle object");
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
				throw CAstException(pnode, this, dotnotation, "Cannot take cell, class or handle object");
			}
			else
				throw CAstException(p, this, fname, "Cannot take cell, class or handle object");
		}
		if (cfn1) Sig.each(cfn1);
		else if (fn1) Sig.each(fn1);
		else if (fn0) Sig.each_sym(fn0);
		else if (cfn0) Sig.each(cfn0);
		break;
	default: // res should be 0
		if (pEnv->inFunc.find(fname) != pEnv->inFunc.end())
		{
			auto pos = find(pEnv->built_in_func_names.begin(), pEnv->built_in_func_names.end(), fname) - pEnv->built_in_func_names.begin();
			fnsigs = fname + pEnv->built_in_funcs[pos].funcsignature;
			if (structCall)
			{
				//if (isgraphicfunc(fname))
				//{
				//	if (!pgo || !pgo->IsGO())
				//		throw CAstException(p, this, fname, "For graphic functions as a member function, a graphic handle should be the base.");
				//}
				if (pEnv->built_in_funcs[pos].alwaysstatic)
					throw CAstException(p, this, fname, "Cannot be a member function.");
				firstparamtrim(fnsigs);
				nArgs = checkNumArgs(pnode, p, fnsigs, pEnv->built_in_funcs[pos].narg1 - 1, pEnv->built_in_funcs[pos].narg2 - 1);
				pEnv->inFunc[fname](this, pnode, p, fnsigs);
			}
			else
			{
				nArgs = checkNumArgs(pnode, p, fnsigs, pEnv->built_in_funcs[pos].narg1, pEnv->built_in_funcs[pos].narg2);
				if (pEnv->built_in_funcs[pos].alwaysstatic)
					pEnv->inFunc[fname](this, pnode, p, fnsigs);
				else
					pEnv->inFunc[fname](this, pnode, p->next, fnsigs);
			}
		}
		else
			throw CAstException(p, this, fname, "Not a built-in function.");
	}
	Sig.functionEvalRes = true;
	return;
}
