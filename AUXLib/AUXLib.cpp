// AUXLAB 
//
// Copyright (c) 2009-2019 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: AUXLib
// dynamic library for sigproc
// 
// Version: 0.1
// Date: 8/29/2021

#include "pch.h"
#include <exception>
#include <vector>
#include <queue>
#include "sigproc.h"
#include "wavplay.h"
#include "AUXLib.h"

#define MAX_AUX_ERR_MSG_LEN	1000

vector<CAstSig *> GAstSigs;
vector<CVar> GSigs;
queue<int> GAstSigRecycle;
CAstSigEnv* pglobalEnv;
char GAstErrMsg[MAX_AUX_ERR_MSG_LEN] = "";

double CAstSig::play_block_ms = 300;
double CAstSig::record_block_ms = 300;
short CAstSig::play_bytes = 2;
short CAstSig::record_bytes = 2;

// These are not used, but need to be declared
#include <condition_variable>
HANDLE hEventRecordingReady;
condition_variable cv_closeFig;
HWND hwnd_AudioCapture = NULL;
mutex mtx4PlotDlg;

AUXLIB_EXP int AUXNew(const int sample_rate, const char *auxpath) // Returns new handle, which is 0 or positive, or -1 on error.
{
	int handle;
	try {
		pglobalEnv = (sample_rate > 1) ? new CAstSigEnv(sample_rate) : new CAstSigEnv();
		if (auxpath)
			pglobalEnv->SetPath(auxpath);
		pglobalEnv->InitBuiltInFunctions();
		CAstSig *pAstSig = new CAstSig(pglobalEnv);
		if (GAstSigRecycle.empty()) {
			handle = (int)GAstSigs.size();
			GAstSigs.push_back(pAstSig);
			GSigs.push_back(CSignals());
		} else {
			handle = GAstSigRecycle.front();
			GAstSigRecycle.pop();
			GAstSigs[handle] = pAstSig;
		}
	} catch (const exception &e) {
		strncpy(GAstErrMsg, e.what(), MAX_AUX_ERR_MSG_LEN);
		return -1;
	}
	return handle;
}


AUXLIB_EXP void AUXDelete(const int hAUX) // Ignores invalid hAUX
{
	if (hAUX<0 || hAUX>(int)GAstSigs.size()-1)
		return;
	if (CAstSig *pAstSig = GAstSigs[hAUX]) {
		delete pAstSig;
		GAstSigs[hAUX] = NULL;
		GAstSigRecycle.push(hAUX);
	}
}


AUXLIB_EXP int AUXEval(const int hAUX, const char *strIn, double **buffer, int *length)
// Returns the number of channels or
//  0 : Empty result
// -1 : Invalid hAUX
// -2 : AUX error
// -3 : Unknown error
//
// Actual size of buffer is (length * channels).
// Data in buffer are valid until next AUXEval() or AUXDelete() with the same hAUX.
{
	try {
		if (hAUX<0 || hAUX>(int)GAstSigs.size()-1) {
			strncpy(GAstErrMsg, "AUXLib error: Invalid handle.", MAX_AUX_ERR_MSG_LEN);
			return -1;
		}
		CAstSig *pAstSig = GAstSigs[hAUX];
		if (!pAstSig) {
			strncpy(GAstErrMsg, "AUXLib error: Invalid handle - already deleted.", MAX_AUX_ERR_MSG_LEN);
			return -1;
		}
		int nChannel;
		char* str_autocorrect = (char*)calloc(strlen(strIn) * 2, 1);
		string emsg;
		if (!(pAstSig->xtree = pAstSig->parse_aux(strIn, emsg, str_autocorrect)))
		{
			delete[] str_autocorrect;
			throw emsg.c_str();
		}
		pAstSig->statusMsg.clear();
		pAstSig->Compute();
		delete[] str_autocorrect;

		GSigs[hAUX] = pAstSig->Sig;
		GSigs[hAUX].MakeChainless();
		if (length)
			*length = GSigs[hAUX].nSamples;
		if (length && *length)
			nChannel = 1;
		else
			return 0;
		if (buffer) 
		{
//			if (GSigs[hAUX].next) 	GSigs[hAUX] += GSigs[hAUX].next;	// concatenation
			*buffer = GSigs[hAUX].buf;
		}
		return nChannel;
	} catch (const char *errmsg) {
		strncpy(GAstErrMsg, errmsg, MAX_AUX_ERR_MSG_LEN);
		return -2;
	} catch (exception &e) {
		strncpy(GAstErrMsg, e.what(), MAX_AUX_ERR_MSG_LEN);
		return -3;
	}
}


AUXLIB_EXP int AUXPlay(const int hAUX, const int DevID)
{
	char errstr[250];
	if (hAUX<0 || hAUX>(int)GAstSigs.size()-1) {
		strncpy(GAstErrMsg, "AUXLib error: Invalid handle.", MAX_AUX_ERR_MSG_LEN);
		return 0;
	}
	CAstSig *pAstSig = GAstSigs[hAUX];
	if (!pAstSig) {
		strncpy(GAstErrMsg, "AUXLib error: Invalid handle - already deleted.", MAX_AUX_ERR_MSG_LEN);
		return 0;
	}
	auto res = PlayCSignals(pAstSig->Sig, 0, 0, NULL, &CAstSig::play_block_ms, errstr, 2);
	if (res==NULL) {
		strncpy(GAstErrMsg, errstr, MAX_AUX_ERR_MSG_LEN);
		return 0;
	} else
		return 1;
}


AUXLIB_EXP int AUXWavwrite(const int hAUX, const char *filename)
{
	char errstr[250];
	if (hAUX<0 || hAUX>(int)GAstSigs.size()-1) {
		strncpy(GAstErrMsg, "AUXLib error: Invalid handle.", MAX_AUX_ERR_MSG_LEN);
		return 0;
	}
	CAstSig *pAstSig = GAstSigs[hAUX];
	if (!pAstSig) {
		strncpy(GAstErrMsg, "AUXLib error: Invalid handle - already deleted.", MAX_AUX_ERR_MSG_LEN);
		return 0;
	}
	if (!pAstSig->Sig.Wavwrite(filename, errstr)) {
		strncpy(GAstErrMsg, errstr, MAX_AUX_ERR_MSG_LEN);
		return 0;
	} else
		return 1;
}


const char *AUXGetErrMsg(void) // Returns error message for the last AUX error.
{
	GAstErrMsg[MAX_AUX_ERR_MSG_LEN-1] = '\0';
	return GAstErrMsg;
}


AUXLIB_EXP int AUXGetInfo(const int hAUX, const char *name, void *output)
{
	if (hAUX<0 || hAUX>(int)GAstSigs.size()-1) {
		strncpy(GAstErrMsg, "AUXLib error: Invalid handle.", MAX_AUX_ERR_MSG_LEN);
		return 0;
	}
	CAstSig *pAstSig = GAstSigs[hAUX];
	if (!pAstSig) {
		strncpy(GAstErrMsg, "AUXLib error: Invalid handle - already deleted.", MAX_AUX_ERR_MSG_LEN);
		return 0;
	}
	if (strcmp(name, "path") == 0) {
		strcpy((char *)output, pglobalEnv->AuxPath.front().c_str());
	} else {
		strncpy(GAstErrMsg, "AUXLib error: Invalid property name was passed to AUXGetInfo().", MAX_AUX_ERR_MSG_LEN);
		return 0;
	}
	return 1;
}