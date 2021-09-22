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
#include "..\sigproc_internal.h"

map<double, FILE *> file_ids;

#ifndef NO_FILES

#include <algorithm>
#include <cassert>
#include "bjcommon.h"
#include "audstr.h"
#include "lame_bj.h"
#include "samplerate.h"
#include "sndfile.h"
#include "..\sigplus_internal.h"

void _sprintf(CAstSig* past, const AstNode* pnode);

static inline int _double_to_24bit(double x) // called inside makebuffer
{
	// This maps a double variable raning -1 to 1, to a short variable ranging -16388608 to 16388607.
	return (int)(max(min(x, 1), -1) * MAX_24BIT - .5);
}

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
	for (map<string, CVar>::iterator it = past->Vars.begin(); it != past->Vars.end(); it++)
		if (it->second.GetType() == CSIG_AUDIO) var.push_back(it->first);
}

void _fopen(CAstSig *past, const AstNode *pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	string filename = past->makefullfile(past->ComputeString(p));
	char mode[8];
	strcpy(mode, past->ComputeString(p->next).c_str());
	FILE *fl;
	if (!(fl = fopen(filename.c_str(), mode)))
	{
		if (past->level > 1)
		{ // if fopen is called inside UDF, the file is searched in the same directory as the udf
		  // (which may be part of path or not) unless the path for the file is specified
			char drive[MAX_PATH], dir[MAX_PATH];
			// does p include the path?
			filename = past->ComputeString(p);
			_splitpath(filename.c_str(), drive, dir, NULL, NULL);
			if (!drive[0] && !dir[0]) // no path included
			{
				_splitpath(past->pEnv->udf[past->u.title].fullname.c_str(), drive, dir, NULL, NULL);
				filename = string(drive) + dir + past->ComputeString(p);
			}
			if (!(fl = fopen(filename.c_str(), mode)))
				past->Sig.SetValue(-1.);
			else
			{
				past->Sig.SetValue((double)(INT_PTR)fl);
				file_ids[(double)(INT_PTR)fl] = fl;
			}
		}
		else
			past->Sig.SetValue(-1.);
	}
	else
	{
		past->Sig.SetValue((double)(INT_PTR)fl);
		file_ids[(double)(INT_PTR)fl] = fl;
	}
}

void _fclose(CAstSig *past, const AstNode *pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	if (!past->Sig.IsScalar())
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("First arg must be a file identifider");
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
/* fwrite: audio signal --> rescale from -1 to 1 to each integer range corresponding to the format,
   e.g., -32768 to 32767 for int16, 0 to 65535 for uint16
   if it is stereo, writes the data in an interleaved manner for each channel
   nonaudio signal --> write as is.. don't care whether it is outside of the range.

   fread: reads the data according to the format, e.g., -2^31 to 2^31 for int32, makes a non-audio object
   if the last arg is "a" or "audio," it rescales in the range and makes the object audio (mono)
   if the last arg is "a2" or "audio2," it rescales in the range and makes the object audio (stereo)
*/

FILE * __freadwrite(CAstSig *past, const AstNode *pnode, const AstNode *p, int &bytes, string &prec, char *additional = NULL);

FILE * __freadwrite(CAstSig *past, const AstNode *pnode, const AstNode *p, int &bytes, string &prec, char *additional)
{
	//first arg is always file identifier
	//second arg is the signal to write to file
	//third arg is precision--one of the following: int8 int16 int32 uint8 uint16 uint32 char float double
	FILE *file = nullptr;
	if (past->Sig.IsScalar())
		file = file_ids[past->Sig.value()];
	if (!file)
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("First arg must be either a file identifider");
	CVar second, addition;
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
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc(estr.c_str());
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
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("Second arg must be either ....");
	if (!strcmp(pnode->str, "fread"))
	{
		if (p->next)
		{
			addition = past->Compute(p->next);
			if (!addition.IsString())
			{
				estr = "Third arg must be a string--either \"a\" \"audio\" \"a2\" or \"audio2\" ";
				throw CAstException(FUNC_SYNTAX, *past, pnode).proc(estr.c_str());
			}
			else
				strcpy(additional, addition.string().c_str());
		}
	}
	else
		past->Compute(p->next);
	return file;
}

template<typename T>
size_t fwrite_general_floating(T var, CVar& sig, string prec, FILE* file)
{
	if (sig.IsAudio())
		sig.MakeChainless();
	int k = 0;
	T* pvar = &var;
	if (sig.next)
	{
		double* buf2 = sig.next->buf;
		for_each(sig.buf, sig.buf + sig.nSamples,
			[buf2, pvar, file, &k](double v) {
				*pvar = (T)v; fwrite(pvar, sizeof(T), 1, file);
				*pvar = (T)buf2[k++]; fwrite(pvar, sizeof(T), 1, file); });
	}
	else
	{
		for_each(sig.buf, sig.buf + sig.nSamples,
			[pvar, file](double v) { *pvar = (T)v; fwrite(pvar, sizeof(T), 1, file); });
	}
	return sig.nSamples;
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
				*pvar = (T)(factor * v - .5); fwrite(pvar, bytes, 1, file);
				*pvar = (T)(factor * buf2[k++] - .5); fwrite(pvar, bytes, 1, file); });
	}
	else
	{
		if (sig.IsAudio())
			for_each(sig.buf, sig.buf + sig.nSamples,
				[pvar, bytes, factor, file](double v) { *pvar = (T)(factor * v - .5); fwrite(pvar, bytes, 1, file); });
		else
			for_each(sig.buf, sig.buf + sig.nSamples,
				[pvar, bytes, factor, file](double v) { *pvar = (T)(v - .5); fwrite(pvar, bytes, 1, file); });
	}
	return sig.nSamples;
}

void _fwrite(CAstSig *past, const AstNode *pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	CVar firstarg = past->Sig;
	int bytes;
	string prec;
	size_t res;
	FILE * file = __freadwrite(past, pnode, p, bytes, prec);
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
		//int16_t * pvar = &temp;
		//for (unsigned int k = 0; k < past->Sig.nSamples; k++)
		//{
		//	double v = (past->Sig.buf[k] * 0x8000 - .5);
		//	*pvar = (int16_t)(v);
		//	res = fwrite(pvar, sizeof(temp), 1, file);
		//}

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
		res = fwrite_general_floating(temp, past->Sig, prec, file);
	}
	else if (prec == "double")
	{ // No automatic scaling
		double temp = 0;
		res = fwrite_general_floating(temp, past->Sig, prec, file);
//		res = fwrite(past->Sig.buf, bytes, past->Sig.nSamples, file);
	}
	past->Sig.SetValue((double)res);
}

template<typename T>
void fread_general(T var, CVar &sig, FILE * file, int bytes, char *addarg, uint64_t factor)
{
	T *pvar = &var;
	if (!strcmp(addarg, "audio") || !strcmp(addarg, "a"))
		for_each(sig.buf, sig.buf + sig.nSamples,
			[pvar, bytes, file, factor](double &v) { fread(pvar, bytes, 1, file); v = *pvar / (double)factor; });
	else if (!strcmp(addarg, "audio2") || !strcmp(addarg, "a2"))
	{
		int k = 0;
		CSignals next = CSignal(sig.GetFs(), sig.nSamples);
		sig.SetNextChan(&next);
		double *buf2 = sig.next->buf;
		for_each(sig.buf, sig.buf + sig.nSamples,
			[buf2, pvar, bytes, file, factor, &k](double &v) {
				fread(pvar, bytes, 1, file); v = *pvar / (double)factor;
				fread(pvar, bytes, 1, file); buf2[k++] = *pvar / (double)factor; });
	}
	else
		for_each(sig.buf, sig.buf + sig.nSamples,
			[pvar, bytes, file](double &v) { fread(pvar, bytes, 1, file); v = *pvar; });
}

void _fread(CAstSig *past, const AstNode *pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	int bytes;
	string prec;
	char addarg[16] = {};
	FILE * file = __freadwrite(past, pnode, p, bytes, prec, addarg);

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
	if (!strcmp(addarg, "audio2") || !strcmp(addarg, "a2"))
	{
		if (nItems / 2 * 2 != nItems)
			throw CAstException(USAGE, *past, pnode).proc("attempting to read stereo audio data but data count is not even.");
		nItems /= 2;
	}
	past->Sig.UpdateBuffer((unsigned int)nItems);
	if (!strcmp(addarg, "audio") || !strcmp(addarg, "a") || !strcmp(addarg, "audio2") || !strcmp(addarg, "a2"))
		past->Sig.SetFs(past->pEnv->Fs);
	if (prec == "int8" || prec == "uint8")
	{
		int8_t temp = 0;
		fread_general(temp, past->Sig, file, bytes, addarg, 0x80);
	}
	else if (prec == "int16" || prec == "uint16")
	{
		int16_t temp = 0;
		fread_general(temp, past->Sig, file, bytes, addarg, 0x8000);
	}
	else if (prec == "int24")
	{
		int32_t temp = 0; // in24_t doesn't exist
		fread_general(temp, past->Sig, file, bytes, addarg, 0x80000000); // check
	}
	else if (prec == "int32" || prec == "uint32")
	{
		int32_t temp = 0;
		fread_general(temp, past->Sig, file, bytes, addarg, 0x80000000);
	}
	else if (prec == "float")
	{
		float temp = 0.;
		fread_general(temp, past->Sig, file, bytes, addarg, 1);
	}
	else if (prec == "double")
	{
		double temp = 0.;
		fread_general(temp, past->Sig, file, bytes, addarg, 1);
	}
	if (!strcmp(addarg, "audio") || !strcmp(addarg, "a") | !strcmp(addarg, "audio2") || !strcmp(addarg, "a2"))
		past->Sig.SetFs(past->pEnv->Fs);
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

void _wavwrite(CAstSig *past, const AstNode *pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->checkAudioSig(p, past->Sig);
	string option;
	string filename;
	CVar third;
	try {
		CAstSig tp(past);
		tp.Compute(p);
		tp.checkString(p, tp.Sig);
		if (tp.Sig.string().empty())
			throw CAstException(USAGE, tp, p).proc("Empty filename");
		filename = tp.Sig.string();
		if (p->next != NULL)
		{
			third = tp.Compute(p->next);
			tp.checkString(p->next, (CVar)third);
			option = third.string();
		}
	}
	catch (const CAstException &e) { throw CAstException(FUNC_SYNTAX, *past, pnode).proc(e.getErrMsg().c_str()); }

	string fullfilename = past->makefullfile(filename, ".wav");
	char errStr[256];
	if (!past->Sig.Wavwrite(fullfilename.c_str(), errStr, option))
		throw CAstException(USAGE, *past, p).proc(errStr);
}

void _write(CAstSig *past, const AstNode *pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->checkAudioSig(p, past->Sig);
	string option;
	string filename;
	CVar third;
	try {
		CAstSig tp(past);
		tp.Compute(p);
		tp.checkString(p, tp.Sig);
		if (tp.Sig.string().empty())
			throw CAstException(USAGE, tp, p).proc("Empty filename");
		filename = tp.Sig.string();
		if (p->next != NULL)
		{
			third = tp.Compute(p->next);
			tp.checkString(p->next, (CVar)third);
			option = third.string();
		}
	}
	catch (const CAstException &e) { throw CAstException(FUNC_SYNTAX, *past, pnode).proc(e.getErrMsg().c_str()); }
	trim(filename, ' ');
	size_t pdot = filename.rfind('.');
	string extension = filename.substr(pdot + 1);
	if (extension.empty())
		throw CAstException(USAGE, *past, p).proc("The extension must be specified .wav .mp3 or .txt");
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
			throw CAstException(USAGE, *past, p).proc(errStr);
	}
	else if (extension == "wav")
	{
		_wavwrite(past, pnode);
	}
	else if (extension == "txt")
	{
		FILE* fid = fopen(filename.c_str(), "wt");
		if (!fid)
			throw CAstException(USAGE, *past, p).proc("File creation error");
		write2textfile(fid, &past->Sig);
		fclose(fid);
	}
	else
		throw CAstException(USAGE, *past, p).proc("unknown audio file extension. Must be .wav or .mp3");
}

static void resample_if_fs_different(CAstSig* past, const AstNode* p)
{ // call this function only if past->Sig still has the active signal
	vector<string> audiovars;
	EnumAudioVariables(past, audiovars);
	if (past->FsFixed || audiovars.size() > 0)
	{
		if (past->Sig.GetFs() != past->GetFs())
		{
			int oldFs = past->GetFs();
			CVar ratio(1);
			ratio.SetValue(past->Sig.GetFs() / (double)oldFs);
			past->Sig.fp_mod(&CSignal::resample, &ratio);
			if (ratio.IsString()) // this means there was an error during resample
				throw CAstException(FUNC_SYNTAX, *past, p).proc(ratio.string().c_str());
			sformat(past->statusMsg, "(NOTE)File fs=%d Hz. The audio data resampled to %d Hz.", past->Sig.GetFs(), oldFs);
			past->Sig.SetFs(oldFs);
			if (past->Sig.next)
				past->Sig.next->SetFs(oldFs);
		}
	}
	else
	{
		past->pEnv->Fs = past->Sig.GetFs();
		past->statusMsg = "(NOTE)Sample Rate of AUXLAB Environment is now set to ";
		char temp[16];
		sprintf_s(temp,"%d Hz.", past->pEnv->Fs);
		past->statusMsg += temp;
	}
}

void _wave(CAstSig *past, const AstNode *pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	/*! \brief wave(filename)
	*         Open .wav file
	*
	*  Input: filename: string
	*  Output: audio_signal
	*/
	past->checkString(pnode, past->Sig);
	string filename = past->makefullfile(past->Sig.string(), ".wav");
	char errStr[256];
	CVar start2read(0.), dur2read(-1.); // -1. to read the entire duration
	if (p->next) {
		start2read = past->Compute(p->next);
		if (p->next->next) 
			dur2read = past->Compute(p->next->next);
	}
	past->checkScalar(p, start2read);
	
	past->checkScalar(p, dur2read);
	if (!past->Sig.Wavread(filename.c_str(), start2read.value(), dur2read.value(), errStr))
		throw CAstException(USAGE, *past, p).proc(errStr);
	resample_if_fs_different(past, p);
}

void _file(CAstSig *past, const AstNode *pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	// Decide the convention when multiple lines input is coming... currently making cell array output... do I want to keep it this way? 2/21/2018 bjk
	past->checkString(pnode, past->Sig);
	string fullpath, content;
	char fname[MAX_PATH], ext[MAX_PATH], errStr[256] = { 0 };
	FILE *fp(NULL);
	int res;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	fp = past->fopen_from_path(past->ComputeString(p), "", fullpath);
	_splitpath(past->ComputeString(p).c_str(), NULL, NULL, fname, ext);
	if (fp)
	{
		fclose(fp);
		if (string(_strlwr(ext)) == ".wav")
		{
#ifndef NO_SF
			_wave(past, pnode);
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
				throw CAstException(FUNC_SYNTAX, *past, p).proc(errStr);
			past->Sig.nSamples = res;
			if (nChans > 1) past->Sig.next->nSamples = res;
			resample_if_fs_different(past, p);
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
				throw CAstException(FUNC_SYNTAX, *past, p).proc(errStr);
			resample_if_fs_different(past, p);
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
		throw CAstException(FUNC_SYNTAX, *past, p).proc("cannot open file");
}

int CSignals::Wavread(const char* wavname, double beginMs, double durMs, char* errstr)
{
	SNDFILE* wavefileID;
	SF_INFO sfinfo;
	sf_count_t count;
	if ((wavefileID = sf_open(wavname, SFM_READ, &sfinfo)) == NULL)
	{
		sprintf(errstr, "Unable to open audio file '%s'\n", wavname);
		sf_close(wavefileID);
		return NULL;
	}
	if (sfinfo.channels > 2) { strcpy(errstr, "Up to 2 channels (L R) are allowed in AUX--we have two ears, dude!"); return NULL; }
	Reset(sfinfo.samplerate);
	SetNextChan(NULL); // Cleans up existing next
	if ((beginMs == 0. && durMs == -1.) || (sfinfo.format & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) 
	{
		if (sfinfo.channels == 1)
		{
			UpdateBuffer((int)sfinfo.frames);
			count = sf_read_double(wavefileID, buf, sfinfo.frames);  // common.h
		}
		else
		{
			double* buffer = new double[(unsigned int)sfinfo.channels * (int)sfinfo.frames];
			count = sf_read_double(wavefileID, buffer, sfinfo.channels * sfinfo.frames);  // common.h
			double(*buf3)[2];
			next = new CSignals(sfinfo.samplerate);
			int m(0);
			buf3 = (double(*)[2]) & buffer[m++];
			UpdateBuffer((int)sfinfo.frames);
			for (unsigned int k = 0; k < sfinfo.frames; k++)			buf[k] = buf3[k][0];
			buf3 = (double(*)[2]) & buffer[m++];
			next->UpdateBuffer((int)sfinfo.frames);
			for (unsigned int k = 0; k < sfinfo.frames; k++)			next->buf[k] = buf3[k][0];
			delete[] buffer;
		}
		sf_close(wavefileID);
	}
	else
	{
		int id1 = (int)(beginMs / 1000. * fs + .5);
		int len = (int)(durMs / 1000. * fs + .5);
		sf_count_t nFrames2Read = sfinfo.frames - id1;
		if (nFrames2Read <= 0) return 1;
		nFrames2Read = min(nFrames2Read, len);
		uint8_t bytes = sfinfo.format & 0x0000000F;
		if (bytes == SF_FORMAT_PCM_U8)
			bytes = 1;
		else if (bytes == SF_FORMAT_FLOAT)
			bytes = 4;
		else if (bytes == SF_FORMAT_DOUBLE)
			bytes = 8;
		size_t memsize = nFrames2Read * sfinfo.channels * bytes;
		void *readBuffer = malloc(memsize);
		FILE *fp = fopen(wavname, "rb"); // sf_open was success above; cannot be null 
		char bufHeader[41];
		size_t count = fread(bufHeader, 1, 40, fp);
		assert(count == 40);
		bufHeader[40] = 0;
		assert(!strcmp(bufHeader + 36, "data"));
//		fseek(fp, 40, SEEK_SET);
		long dataChunkSize;
		count = fread(&dataChunkSize, 1, sizeof(long), fp);
		long skip = (long)(id1 * sfinfo.channels * bytes);
		fseek(fp, skip, SEEK_CUR);
		count = fread(readBuffer, 1, memsize, fp);
		if (sfinfo.channels == 1)
		{
			UpdateBuffer((unsigned int)nFrames2Read);
			switch (sfinfo.format & 0x0000000F)
			{
			case SF_FORMAT_PCM_S8:
				for (int k = 0; k < nFrames2Read; k++)
					buf[k] = ((char*)readBuffer)[k] / 128.;
				break;
			case SF_FORMAT_PCM_16:
				for (int k = 0; k < nFrames2Read; k++)
					buf[k] = ((int16_t*)readBuffer)[k] / 32768.;
				break;
			case SF_FORMAT_PCM_24:
				for (int k = 0; k < nFrames2Read; k++)
					buf[k] = ((int32_t*)readBuffer)[k] / 524288.;
				break;
			case SF_FORMAT_PCM_32:
				for (int k = 0; k < nFrames2Read; k++)
					buf[k] = ((int32_t*)readBuffer)[k] / 2147483648.;
				break;
			case SF_FORMAT_PCM_U8:
				for (int k = 0; k < nFrames2Read; k++)
					buf[k] = ((char*)readBuffer)[k] / 256. - .5;
				break;
			case SF_FORMAT_FLOAT:
				for (int k = 0; k < nFrames2Read; k++)
					buf[k] = ((float*)readBuffer)[k];
				break;
			case SF_FORMAT_DOUBLE:
				for (int k = 0; k < nFrames2Read; k++)
					buf[k] = ((double*)readBuffer)[k];
				break;
			}
		}
		else
		{
			UpdateBuffer((unsigned int)nFrames2Read);
			next = new CSignals(sfinfo.samplerate);
			next->UpdateBuffer((unsigned int)nFrames2Read);
			len = (int)nFrames2Read * sfinfo.channels;
			switch (sfinfo.format & 0x0000000F)
			{
			case SF_FORMAT_PCM_S8:
				for (int k = 0; k < len; k += 2)
				{
					buf[k / 2] = ((char*)readBuffer)[k] / 128.;
					next->buf[k / 2] = ((char*)readBuffer)[k + 1] / 128.;
				}
				break;
			case SF_FORMAT_PCM_16:
				for (int k = 0; k < len; k += 2)
				{
					buf[k / 2] = ((int16_t*)readBuffer)[k] / 32768.;
					next->buf[k / 2] = ((int16_t*)readBuffer)[k + 1] / 32768.;
				}
				break;
			case SF_FORMAT_PCM_24:
				for (int k = 0; k < len; k += 2)
				{
					buf[k / 2] = ((int16_t*)readBuffer)[k] / 524288.;
					next->buf[k / 2] = ((int16_t*)readBuffer)[k + 1] / 524288.;
				}
				break;
			case SF_FORMAT_PCM_32:
				for (int k = 0; k < len; k += 2)
				{
					buf[k / 2] = ((int16_t*)readBuffer)[k] / 524288.;
					next->buf[k / 2] = ((int16_t*)readBuffer)[k + 1] / 2147483648.;
				}
				break;
			case SF_FORMAT_PCM_U8:
				for (int k = 0; k < len; k += 2)
				{
					buf[k / 2] = ((char*)readBuffer)[k] / 256. - .5;
					next->buf[k / 2] = ((char*)readBuffer)[k + 1] / 256. - .5;
				}
				break;
			case SF_FORMAT_FLOAT:
				for (int k = 0; k < len; k += 2)
				{
					buf[k / 2] = ((float*)readBuffer)[k];
					next->buf[k / 2] = ((float*)readBuffer)[k + 1];
				}
				break;
			case SF_FORMAT_DOUBLE:
				for (int k = 0; k < len; k += 2)
				{
					buf[k / 2] = ((double*)readBuffer)[k];
					next->buf[k / 2] = ((double*)readBuffer)[k + 1];
				}
				break;
			}
			free(readBuffer);
		}
	}
	return 1;
}

int CSignals::mp3write(const char* filename, char* errstr, std::string wavformat)
{
	MakeChainless();
	char errStr[256];
	int res = write_mp3(nSamples, buf, next ? next->buf : NULL, fs, filename, errStr);
	if (res == 0)
		sprintf(errstr, "error in write_mp3");
	return res;
}

int CSignals::Wavwrite(const char* wavname, char* errstr, std::string wavformat)
{
	SF_INFO sfinfo;
	SNDFILE* wavefileID;
	sfinfo.channels = (next) ? 2 : 1;
	if (wavformat.length() == 0)
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_PCM_16; // default
	else if (wavformat == "8")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_PCM_U8;
	else if (wavformat == "16")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_PCM_16;
	else if (wavformat == "24")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_PCM_24;
	else if (wavformat == "32")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_PCM_32;
	else if (wavformat == "ulaw")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_ULAW;
	else if (wavformat == "alaw")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_ALAW;
	else if (wavformat == "adpcm1")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_IMA_ADPCM;
	else if (wavformat == "adpcm2")
		sfinfo.format = SF_FORMAT_WAV + SF_FORMAT_MS_ADPCM;
	//	else if (wavformat=="vorbis")
	//		sfinfo.format = SF_FORMAT_OGG + SF_FORMAT_VORBIS; // not available ...  ogg.c requires external lib which I don't have yet. bjkwon 03/19/2016
	else
	{
		sprintf(errstr, "Supported waveformat---8, 16, 24, 32, ulaw, alaw, adpcm1 or adpcm2.\n");
		return 0;
	}
	sfinfo.frames = nSamples;
	sfinfo.samplerate = fs;
	sfinfo.sections = sfinfo.seekable = 1;
	if ((wavefileID = sf_open(wavname, SFM_WRITE, &sfinfo)) == NULL)
	{
		sprintf(errstr, "Unable to open/write audio file to '%s'\n", wavname);
		sf_close(wavefileID);
		return 0;
	}
	double* dbuffer = nullptr;
	int lengthAllocated = -1, length = -1;
	CSignals nextblock, nextblock2;
	nextblock <= *this;
	int nChan = next == NULL ? 1 : 2;
	while (!nextblock.IsEmpty())
	{
		double* buffer;
		double tp1, tp2;
		length = nextblock.getBufferLength(tp1, tp2, CAstSig::play_block_ms);
		if (tp1 == 0. && tp2 == 0. || nChan > 1)
		{
			if (length > lengthAllocated)
			{
				if (dbuffer) delete[] dbuffer;
				dbuffer = new double[length * nChan];
				lengthAllocated = length;
			}
			buffer = dbuffer;
			if (!nextblock.makebuffer<double>(dbuffer, length, tp1, tp2, nextblock2)) // sig is empty
				return 0;
		}
		else
		{
			buffer = nextblock.buf;
			nextblock2 <= nextblock;
			nextblock.nextCSignals(tp1, tp2, nextblock2);
		}
		sf_writef_double(wavefileID, buffer, length);
		nextblock = nextblock2;
	}
	if (dbuffer) delete[] dbuffer;
	sf_close(wavefileID);
	return 1;
}

#endif // NO_FILES
