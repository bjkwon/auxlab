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
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>

#ifdef _WINDOWS
#include <io.h>
#include "bjcommon.h"
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

extern HWND hShowDlg;

// 10/1/2020 TO DO-----separate these graffy function somehow to add its own features 
// such as blockNULL etc.

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

void _minmax(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _filt(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _iir(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _tparamonly(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _rand(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _irand(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _randperm(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _sprintf(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _record(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _play(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _pause_resume(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _stop(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _inputdlg(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void aux_input(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void udf_error(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void udf_warning(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void udf_rethrow(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _msgbox(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _dir(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _include(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _eval(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _zeros(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _ones(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _cell(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _matrix(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _interp1(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _fdelete(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _ismember(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _isaudioat(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _fft(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _ifft(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _tone(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _fm(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _tsq_getvalues(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs);
void _tsq_setvalues(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs);
void _tsq_gettimes(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs);
void _tsq_settimes(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs);
void _tsq_isrel(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs);
void _str2num(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs);
void _datatype(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs);
void _veq(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs);
void _varcheck(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _and(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs);
void _or(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs);
void _sort(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs);
void _arraybasic(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs);
void _hamming(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _blackman(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _envelope(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _hilbert(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _sam(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _ramp(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _audio(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _vector(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _left(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _right(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _std(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _size(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _mostleast(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _setnextchan(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _getfs(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _setfs(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _erase(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _head(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _cumsum(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _diff(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _conv(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _imaginary_unit(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _pi(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);
void _natural_log_base(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs);


bool CAstSig::IsValidBuiltin(string funcname)
{
	if (pEnv->pseudo_vars.find(funcname) != pEnv->pseudo_vars.end())
		return false;
	return pEnv->builtin.find(funcname) != pEnv->builtin.end();
}

void CAstSig::checkAudioSig(const AstNode *pnode, const CVar &checkthis, string addmsg)
{
	if (checkthis.type() & TYPEBIT_AUDIO) return;
	if (checkthis.GetType()==CSIG_CELL && ((CSignal)checkthis).GetType()==CSIG_AUDIO) return; // Why is this here? Maybe there is a data type of cell with audio data? 6/29/2020
	string msg("requires an audio signal as the base.");
	throw CAstException(USAGE, *this, pnode).proc((msg+addmsg).c_str());
}

void CAstSig::checkTSeq(const AstNode *pnode, const CVar &checkthis, string addmsg)
{
	if (checkthis.IsTimeSignal()) return;
	string msg("requires a time_sequence as the base.");
	throw CAstException(USAGE, *this, pnode).proc((msg + addmsg).c_str());
}

void CAstSig::checkComplex (const AstNode *pnode, CVar &checkthis)
{
	if (checkthis.IsComplex()) return;
	string msg("requires a complex vector as the base.");
	throw CAstException(USAGE, *this, pnode).proc(msg.c_str());
}

void CAstSig::checkSignal(const AstNode *pnode, const CVar &checkthis, string addmsg)
{ // if it is audio or vector --> OK
  // if not (including scalar) --> not OK
	if (checkthis.GetType() == CSIG_VECTOR) return;
	if (checkthis.GetType() == CSIG_AUDIO) return;
	string msg("requires an audio signal or a vector.");
	throw CAstException(USAGE, *this, pnode).proc((msg + addmsg).c_str());
}

void CAstSig::checkVector(const AstNode *pnode, const CVar &checkthis, string addmsg)
{
	if (checkthis.GetType() == CSIG_SCALAR) return;
	if (checkthis.GetType() == CSIG_VECTOR) return;
	string msg("requires a non-audio array.");
	throw CAstException(USAGE, *this, pnode).proc((msg+addmsg).c_str());
}

void CAstSig::checkScalar(const AstNode *pnode, const CVar &checkthis, string addmsg)
{
	if (checkthis.IsScalar()) return;
	string msg("requires a scalar argument.");
	throw CAstException(USAGE, *this, pnode).proc((msg+addmsg).c_str());
}

void CAstSig::checkString(const AstNode *pnode, const CVar &checkthis, string addmsg)
{
	if (checkthis.GetType()==CSIG_STRING) return;
	string msg("requires a string argument.");
	if (checkthis.GetType()==CSIG_CELL && ((CSignal)checkthis).GetType()==CSIG_STRING) return;
	throw CAstException(USAGE, *this, pnode).proc((msg+addmsg).c_str());
}

bool CAstSig::blockCell_allowGO(const AstNode* pnode, const CVar& checkthis, string addmsg)
{
	if (checkthis.type() & TYPEBIT_GO) return true;
	blockCell(pnode, checkthis, addmsg);
	return false;
}

void CAstSig::blockCell(const AstNode* pnode, const CVar& checkthis, string addmsg)
{
	string msg("Not valid with a cell, struct, or point-array variable; ");
	if (checkthis.GetFs() == 3)
		throw CAstException(USAGE, *this, pnode).proc(msg.c_str());
	if (checkthis.GetType() == CSIG_CELL)
		if (((CSignal)checkthis).GetType() == CSIG_EMPTY)
			throw CAstException(USAGE, *this, pnode).proc((msg + addmsg).c_str());
	if (checkthis.GetType() == CSIG_STRUCT)
		throw CAstException(USAGE, *this, pnode).proc((msg + addmsg).c_str());
}

void CAstSig::blockEmpty(const AstNode* pnode, const CVar &checkthis, string addmsg)
{
	if (!checkthis.IsString() && !checkthis.IsEmpty()) return;
	if (checkthis.IsString() && checkthis.nSamples > 1) return;
	string msg("Not valid with an empty variable; ");
	throw CAstException(USAGE, *this, pnode).proc((msg + addmsg).c_str());
}

void CAstSig::blockScalar(const AstNode* pnode, const CVar &checkthis, string addmsg)
{
	if (!(checkthis.type() & 1)) return;
	string msg("Not valid with a scalar variable; ");
	throw CAstException(USAGE, *this, pnode).proc((msg + addmsg).c_str());
}

void CAstSig::blockString(const AstNode* pnode, const CVar &checkthis, string addmsg)
{
	auto aa = checkthis.type();
	auto bb = TYPEBIT_STRING;
	size_t x = sizeof(aa), y = sizeof(bb);
	bool b = aa == bb;
	if (checkthis.type() == TYPEBIT_STRING) {
		string msg("Not valid with a string variable; ");
		throw CAstException(USAGE, *this, pnode).proc((msg + addmsg).c_str());
	}
}

void CAstSig::blockLogical(const AstNode* pnode, const CVar &checkthis, string addmsg)
{
	if (checkthis.type() == TYPEBIT_LOGICAL) {
		string msg("Not valid with a logical variable; ");
		throw CAstException(USAGE, *this, pnode).proc((msg + addmsg).c_str());
	}
}

void CAstSig::blockTemporal(const AstNode* pnode, const CVar &checkthis, string addmsg)
{
	if (checkthis.type() & TYPEBIT_TEMPORAL) {
		string msg("Not valid with a temporal object; ");
		throw CAstException(USAGE, *this, pnode).proc((msg + addmsg).c_str());
	}
}

void CAstSig::blockComplex(const AstNode* pnode, const CVar &checkthis, string addmsg)
{
	if (checkthis.IsComplex()) {
		string msg("Not valid with a complex variable ");
		throw CAstException(USAGE, *this, pnode).proc((msg + addmsg).c_str());
	}
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
				throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "Third parameter, if used, must be a struct variable.");
		}
		int type = param.GetType();
		if (type!= CSIG_TSERIES && type != CSIG_SCALAR)
			throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "parameter must be either a scalar or a time sequence.");
		if (param.GetType() == CSIG_TSERIES)
		{
			double audioDur = past->Sig.dur();
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
			if (pLast->tmark != past->Sig.dur())
			{
				CTimeSeries newParam(past->Sig.GetFs());
				newParam.tmark = past->Sig.dur();
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
			past->Sig.fp_mod(&CSignal::resample, &param);
		else if (fname == "movespec")
			past->Sig.fp_mod(&CSignal::movespec, &param);
		if (param.IsString())
			throw CAstException(USAGE, *past, pnode).proc(("Error in respeed:" + param.string()).c_str());
	}
	catch (const CAstException &e) { throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, e.getErrMsg().c_str()); }
	if (fname == "respeed")
	{ // Take care of overlapping chains after processing
		past->Sig.MergeChains();
	}
}

void _colon(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar first = past->Sig;
	if (!first.IsScalar()) throw CAstException(USAGE, *past, pnode).proc("Colon: All arguments must be scalars (check 1st arg).");
	double val1, val2, step;
	CVar third, second = past->Compute(p->next);
	if (!second.IsScalar()) throw CAstException(USAGE, *past, pnode).proc("Colon: All arguments must be scalars (check 2nd arg).");
	val1 = first.value();
	val2 = second.value();
	if (p->next->next) {
		third = past->Compute(p->next->next);
		if (!third.IsScalar()) throw CAstException(USAGE, *past, pnode).proc("Colon: All arguments must be scalars (check 3rd arg).");
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

void _clear(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->ClearVar((AstNode *)past->pAst, &past->Sig);
	past->Sig.Reset();
}

void _squeeze(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->Sig.Squeeze();
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
	name = "otype";
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

static inline complex<double> r2c_sqrt(complex<double> x) { return sqrt(x); }
static inline complex<double> r2c_log(complex<double> x) { return log(x); }
static inline complex<double> r2c_log10(complex<double> x) { return log10(x); }
static inline complex<double> r2c_pow(complex<double> x, complex<double> y) { return pow(x, y); }

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
	auto type = Sig.type();
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
			if (p && p->type != N_STRUCT) throw CAstException(USAGE, *this, pnode).proc("Superfluous parameter(s) ", fname.c_str());
		}
		else
		{
			if (!p) throw CAstException(USAGE, *this, pnode).proc("Empty parameter");
			Compute(p);
		}
		if (Sig.IsStruct() || !Sig.cell.empty() || Sig.GetFs() == 3)
		{
			if (structCall)
			{
				string dotnotation = dotstring(pnode, pRoot);
				throw CAstException(USAGE, *this, pnode).proc("Cannot take cell, class or handle object ", dotnotation.c_str());
			}
			else
				throw CAstException(USAGE, *this, p).proc("Cannot take cell, class or handle object ", fname.c_str());
		}
		Sig.MFFN(fn0, cfn1);
		break;
	case 2: // two in args, single out arg
		if (fname == "^" || fname == "pow")	fn2 = pow, cfn2 = r2c_pow;
		else if (fname == "mod")	fn2 = fmod, cfn2 = NULL; // do this
		if (structCall)
		{
			if (!p || p->type == N_STRUCT) throw CAstException(USAGE, *this, pnode).proc("requires second parameter.");
			CVar Sig0 = Sig;
			if (Sig.IsStruct() || !Sig.cell.empty() || Sig.GetFs() == 3)
				throw CAstException(USAGE, *this, pnode).proc("Cannot take cell, class or handle object ", fname.c_str());
			param = Compute(p);
			Sig = Sig0.MFFN(fn2, cfn2, param);
		}
		else
		{
			if (!p->next) throw CAstException(USAGE, *this, pnode).proc("requires second parameter.");
			param = Compute(p->next);
			Compute(p);
			if (Sig.IsStruct() || !Sig.cell.empty() || Sig.GetFs() == 3)
			{
				if (structCall)
				{
					string dotnotation = dotstring(pnode, pRoot);
					throw CAstException(USAGE, *this, pnode).proc("Cannot take cell, class or handle object ", dotnotation.c_str());
				}
				else
					throw CAstException(USAGE, *this, p).proc("Cannot take cell, class or handle object ", fname.c_str());
			}
			Sig.MFFN(fn2, cfn2, param);
		}
		break;
	case 10: // other math function... function pointer ready
		if (!type)
		{
			Sig.functionEvalRes = true;
			Sig.Reset(1);
			return;
		}
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
				throw CAstException(USAGE, *this, pnode).proc("Cannot take cell, class or handle object ", dotnotation.c_str());
			}
			else
				throw CAstException(USAGE, *this, p).proc("Cannot take cell, class or handle object ", fname.c_str());
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
					throw CAstException(USAGE, *this, pnode).proc("Cannot be a member function. ", fname.c_str());
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
			throw CAstException(USAGE, *this, p).proc("HandleAuxFunctions()--Not a built-in function?", fname.c_str());
	}
	Sig.functionEvalRes = true;
	return;
}
