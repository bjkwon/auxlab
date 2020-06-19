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

#include <string.h> // aux_file
#include "sigproc.h"
#include "sigproc_internal.h"
#include <algorithm>
#include "bjcommon.h"
#include "audstr.h"
#include "lame_bj.h"

map<double, FILE *> file_ids;

static inline bool isnumeric(const char *buf)
{
	for (size_t k = 0; k < strlen(buf); k++)
	{
		if (buf[k] <= 0 && buf[k] >= 9)
			continue;
		else
		{
			if (buf[k] != '\0' && buf[k] != '\t')
				return false;
		}
	}
	return true;
}

static void EnumAudioVariables(CAstSig *past, vector<string> &var)
{
	var.clear();
	for (map<string, CVar>::iterator what = past->Vars.begin(); what != past->Vars.end(); what++)
		if (what->second.GetType() == CSIG_AUDIO) var.push_back(what->first);
}

void _sprintf(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs);

void _fopen(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	string filename = past->MakeFilename(past->ComputeString(p), "");
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
	if (!past->Sig.IsScalar())
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "First arg must be a file identifider");
	double fl = past->Sig.value();
	FILE *file = file_ids[fl];
	if (!file || fclose(file) == EOF)
	{
		past->Sig.SetValue(-1.);
	}
	else
	{
		past->Sig.SetValue(0);
		file_ids.erase(fl);
	}
}

FILE * __freadwrite(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs, int &bytes, string &prec)
{
	//first arg is always file identifier
	//second arg is the signal to write to file
	//third arg is precision--one of the following: int8 int16 int32 uint8 uint16 uint32 char float double
	FILE *file = nullptr;
	if (past->Sig.IsScalar())
		file = file_ids[past->Sig.value()];
	if (!file)
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "First arg must be either a file identifider");
	CVar second;
	string estr;
	if (!strcmp(pnode->str, "fread"))
	{
		second = past->Compute(p);
		estr = "Second arg must be a string";
	}
	else
	{
		second = past->Compute(p->next);
		estr = "Third arg must be a string";
	}
	if (!past->Sig.IsString())
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, estr.c_str());
	prec = past->Sig.string();
	if (prec == "int8" || prec == "uint8" || prec == "char")
		bytes = 1;
	else if (prec == "int16" || prec == "uint16")
		bytes = 2;
	else if (prec == "int24")
		bytes = 3;
	else if (prec == "float" || prec == "int32" || prec == "uint32")
		bytes = 4;
	else if (prec == "double")
		bytes = 8;
	else
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "Second arg must be either ....");
	if (!strcmp(pnode->str, "fwrite"))
		past->Compute(p->next);
	return file;
}

template<typename T>
size_t fwrite_general(T var, CVar &sig, string prec, FILE * file, int bytes, uint64_t factor)
{
	if (sig.IsAudio())
		sig.MakeChainless();
	else
		factor = 1;
	int k = 0;
	T * pvar = &var;
	if (sig.next)
	{
		double *buf2 = sig.next->buf;
		for_each(sig.buf, sig.buf + sig.nSamples,
			[buf2, pvar, factor, bytes, file, &k](double v) {
				*pvar = (T)(factor * v); fwrite(pvar, bytes, 1, file);
				*pvar = (T)(factor * buf2[k++]); fwrite(pvar, bytes, 1, file); });
	}
	else
		for_each(sig.buf, sig.buf + sig.nSamples,
			[pvar, bytes, factor, file](double v) { *pvar = (T)(factor * v); fwrite(pvar, bytes, 1, file); });
	return sig.nSamples;
}

void _fwrite(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar firstarg = past->Sig;
	int bytes;
	string prec;
	size_t res;
	FILE * file = __freadwrite(past, pnode, p, fnsigs, bytes, prec);
	past->Compute(p);
	if (prec == "char")
	{
		if (past->Sig.IsString())
			res = fwrite(past->Sig.strbuf, 1, past->Sig.nSamples, file);
		else
		{
			char temp = 0;
			res = fwrite_general(temp, past->Sig, prec, file, bytes, 0x100);
		}
	}
	else if (prec == "int8")
	{
		int8_t temp = 0;
		res = fwrite_general(temp, past->Sig, prec, file, bytes, 0x80);
	}
	else if (prec == "uint8")
	{ 
		uint8_t temp = 0;
		res = fwrite_general(temp, past->Sig, prec, file, bytes, 0x100);
	}
	else if (prec == "int16")
	{
		int16_t temp = 0;
		res = fwrite_general(temp, past->Sig, prec, file, bytes, 0x8000);
	}
	else if (prec == "uint16")
	{
		uint16_t temp = 0;
		res = fwrite_general(temp, past->Sig, prec, file, bytes, 0x10000);
	}
	//else if (prec == "int24")
	//{
	//	int32_t temp = 0; // in24_t doesn't exist
	//	res = fwrite_general(temp, past->Sig, prec, file, bytes, 0x800000);
	//}
	else if (prec == "int32")
	{
		int32_t temp = 0;
		res = fwrite_general(temp, past->Sig, prec, file, bytes, 0x80000000);
	}
	else if (prec == "uint32")
	{
		uint32_t temp = 0;
		res = fwrite_general(temp, past->Sig, prec, file, bytes, 0xffffffff);
	}
	else if (prec == "float")
	{ // No automatic scaling
		float temp = 0;
		res = fwrite_general(temp, past->Sig, prec, file, bytes, 1);
	}
	else if (prec == "double")
	{ // No automatic scaling
		res = fwrite(past->Sig.buf, bytes, past->Sig.nSamples, file);
	}
	past->Sig.SetValue((double)res);
}

template<typename T>
void fread_general(T var, CVar &sig, string prec, FILE * file, int bytes)
{
	T *pvar = &var;
	for_each(sig.buf, sig.buf + sig.nSamples,
		[pvar, bytes, file](double &v) { fread(pvar, bytes, 1, file); v = *pvar; });
}

void _fread(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	int bytes;
	string prec;
	FILE * file = __freadwrite(past, pnode, p, fnsigs, bytes, prec);

	fseek(file, 0L, SEEK_END);
	size_t sz = ftell(file);
	fseek(file, 0L, SEEK_SET);

	size_t nItems = sz / bytes;
	if (prec == "char")
	{ // Treat it separately just to make the code neat.
		past->Sig.SetString('\0');
		past->Sig.UpdateBuffer((unsigned int)nItems);
		fread(past->Sig.strbuf, bytes, nItems, file);
		return;
	}
	past->Sig.Reset(1); // always make it non-audio
	past->Sig.UpdateBuffer((unsigned int)nItems);
	if (prec == "int8" || prec == "uint8")
	{
		int8_t temp = 0;
		fread_general(temp, past->Sig, prec, file, bytes);
	}
	else if (prec == "int16" || prec == "uint16")
	{
		int16_t temp = 0;
		fread_general(temp, past->Sig, prec, file, bytes);
	}
	else if (prec == "int24")
	{
		int32_t temp = 0; // in24_t doesn't exist
		fread_general(temp, past->Sig, prec, file, bytes);
	}
	else if (prec == "int32" || prec == "uint32")
	{
		int32_t temp = 0;
		fread_general(temp, past->Sig, prec, file, bytes);
	}
	else if (prec == "float")
	{
		float temp = 0.;
		fread_general(temp, past->Sig, prec, file, bytes);
	}
	else if (prec == "double")
	{
		double temp = 0.;
		fread_general(temp, past->Sig, prec, file, bytes);
	}
}

void _fprintf(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	CVar firstarg = past->Sig;
	_sprintf(past, pnode, p, fnsigs);
	string buffer;
	buffer = past->Sig.string();
	bool openclosehere(1);
	FILE *file = nullptr;
	//is first argument string?
//	past->Compute(p);
	if (firstarg.IsString())
	{
		string filename = past->MakeFilename(past->ComputeString(p), "txt");
		file = fopen(filename.c_str(), "a");
	}
	else
	{
		if (!firstarg.IsScalar())
		{
			past->Sig.SetValue(-2.);
			return;
		}
		if (firstarg.value() == 0.)
		{
			printf(buffer.c_str());
			return;
		}
		file = file_ids[firstarg.value()];
		openclosehere = false;
	}
	if (!file)
	{
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, "First arg must be either a file identifider, filename or 0 (for console)");
	}
	if (fprintf(file, buffer.c_str()) < 0)
		past->Sig.SetValue(-3.);
	else
	{
		if (openclosehere)
		{
			if (fclose(file) == EOF)
			{
				past->Sig.SetValue(-4.);
				return;
			}
		}
		past->Sig.SetValue((double)buffer.length());
	}
}

static void write2textfile(FILE * fid, CVar *psig)
{
	if (psig->bufBlockSize == 1)
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
	CVar third;
	try {
		CAstSig tp(past);
		tp.Compute(p);
		tp.checkString(p, tp.Sig);
		if (tp.Sig.string().empty())
			throw CAstExceptionInvalidUsage(tp, p, "Empty filename");
		filename = tp.Sig.string();
		if (p->next != NULL)
		{
			third = tp.Compute(p->next);
			tp.checkString(p->next, (CVar)third);
			option = third.string();
		}
	}
	catch (const CAstException &e) { throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, e.getErrMsg().c_str()); }
	filename = past->MakeFilename(filename, "wav");
	char errStr[256];
	if (!past->Sig.Wavwrite(filename.c_str(), errStr, option))
		throw CAstExceptionInvalidUsage(*past, p, errStr);
}

void _write(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	past->checkAudioSig(p, past->Sig);
	string option;
	string filename;
	CVar third;
	try {
		CAstSig tp(past);
		tp.Compute(p);
		tp.checkString(p, tp.Sig);
		if (tp.Sig.string().empty())
			throw CAstExceptionInvalidUsage(tp, p, "Empty filename");
		filename = tp.Sig.string();
		if (p->next != NULL)
		{
			third = tp.Compute(p->next);
			tp.checkString(p->next, (CVar)third);
			option = third.string();
		}
	}
	catch (const CAstException &e) { throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, e.getErrMsg().c_str()); }
	trim(filename, ' ');
	size_t pdot = filename.rfind('.');
	string extension = filename.substr(pdot + 1);
	if (extension.empty())
		throw CAstExceptionInvalidUsage(*past, p, "The extension must be specified .wav .mp3 or .txt");
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
			throw CAstExceptionInvalidUsage(*past, p, errStr);
	}
	else if (extension == "wav")
	{
		_wavwrite(past, pnode, p, fnsigs);
	}
	else if (extension == "txt")
	{
		FILE* fid = fopen(filename.c_str(), "wt");
		if (!fid)
			throw CAstExceptionInvalidUsage(*past, p, "File creation error");
		write2textfile(fid, &past->Sig);
		fclose(fid);
	}
	else
		throw CAstExceptionInvalidUsage(*past, p, "unknown audio file extension. Must be .wav or .mp3");
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
		throw CAstExceptionInvalidUsage(*past, p, errStr);
	vector<string> audiovars;
	EnumAudioVariables(past, audiovars);
	if (past->FsFixed || audiovars.size() > 0)
	{
		if (past->Sig.GetFs() != past->GetFs())
		{
			int oldFs = past->GetFs();
			CVar ratio(1);
			ratio.SetValue(past->Sig.GetFs() / (double)oldFs);
			past->Sig.runFct2modify(&CSignal::resample, &ratio);
			if (ratio.IsString()) // this means there was an error during resample
				throw CAstInvalidFuncSyntax(*past, p, fnsigs, ratio.string().c_str());
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
			if (len < 0) len = 0xffffffff;
			past->Sig.UpdateBuffer(len);
			if (nChans > 1)
			{
				past->Sig.SetNextChan(new CSignals(ffs));
				past->Sig.next->UpdateBuffer(len);
			}
			int res = read_mp3(&len, past->Sig.buf, (nChans > 1) ? past->Sig.next->buf : NULL, &ffs, fullpath.c_str(), errStr);
			if (!res)
				throw CAstInvalidFuncSyntax(*past, p, fnsigs, errStr);
			past->Sig.nSamples = res;
			if (nChans > 1) past->Sig.next->nSamples = res;
		}
		else if (string(_strlwr(ext)) == ".aiff")
		{
			int len, ffs, nChans;
			read_aiff_header(fullpath.c_str(), &len, &nChans, &ffs, errStr);
			past->Sig.SetFs(ffs);
			past->Sig.UpdateBuffer(len);
			if (nChans > 1)
				past->Sig.SetNextChan(&past->Sig);
			if (!read_mp3(&len, past->Sig.buf, (nChans > 1) ? past->Sig.next->buf : NULL, &ffs, fullpath.c_str(), errStr))
				throw CAstInvalidFuncSyntax(*past, p, fnsigs, errStr);
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
				//if there's at least one non-numeric character except for space and tab, treat the whole line as a string.
				if (!isnumeric(line[k].c_str()))
				{
					past->Sig.appendcell((CVar)line[k]);
				}
				else
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
					if (nLines == 1)
						past->Sig = tp;
					else
						past->Sig.appendcell((CVar)tp);
					delete[] data;
				}
			}
		}
	}
	else
		throw CAstInvalidFuncSyntax(*past, p, fnsigs, "cannot open file");
}

