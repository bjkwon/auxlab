#include <string.h> // aux_file
#include "sigproc.h"
#include "sigproc_internal.h"
#include <algorithm>

map<double, FILE *> file_ids;

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
	past->Compute(p);
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
		second = past->Compute(p->next);
		estr = "Second arg must be a string";
	}
	else
	{
		second = past->Compute(p->next->next);
		estr = "Third arg must be a string";
	}
	if (!past->Sig.IsString())
		throw CAstInvalidFuncSyntax(*past, pnode, fnsigs, estr.c_str());
	prec = past->Sig.string();
	if (prec == "int8" || prec == "uint8" || prec == "char")
		bytes = 1;
	else if (prec == "int16" || prec == "uint16")
		bytes = 2;
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
void fwrite_general(T var, CVar &sig, string prec, FILE * file, int bytes, uint64_t factor)
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
			[pvar, bytes, file](double v) { *pvar = (T)v; fwrite(pvar, bytes, 1, file); });
}

void _fwrite(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	int bytes;
	string prec;
	FILE * file = __freadwrite(past, pnode, p, fnsigs, bytes, prec);
	past->Compute(p->next);
	if (prec == "char")
	{
		if (past->Sig.IsString())
			fwrite(past->Sig.strbuf, 1, past->Sig.nSamples, file);
		else
		{
			char temp = 0;
			fwrite_general(temp, past->Sig, prec, file, bytes, 256);
		}
	}
	else if (prec == "int8")
	{
		int8_t temp = 0;
		fwrite_general(temp, past->Sig, prec, file, bytes, 128);
	}
	else if (prec == "uint8")
	{
		uint8_t temp = 0;
		fwrite_general(temp, past->Sig, prec, file, bytes, 256);
	}
	else if (prec == "int16")
	{
		int16_t temp = 0;
		fwrite_general(temp, past->Sig, prec, file, bytes, 32768);
	}
	else if (prec == "uint16")
	{
		uint16_t temp = 0;
		fwrite_general(temp, past->Sig, prec, file, bytes, 65536);
	}
	else if (prec == "int32")
	{
		int32_t temp = 0;
		fwrite_general(temp, past->Sig, prec, file, bytes, 8388608);
	}
	else if (prec == "uint32")
	{
		uint32_t temp = 0;
		fwrite_general(temp, past->Sig, prec, file, bytes, 16777216);
	}
	else if (prec == "float")
	{ // No automatic scaling
		float temp = 0;
		fwrite_general(temp, past->Sig, prec, file, bytes, 1);
	}
	else if (prec == "double")
	{ // No automatic scaling
		fwrite(past->Sig.buf, bytes, past->Sig.nSamples, file);
	}
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
		past->Sig.UpdateBuffer(nItems);
		fread(past->Sig.strbuf, bytes, nItems, file);
		return;
	}
	past->Sig.Reset(1); // always make it non-audio
	past->Sig.UpdateBuffer(nItems);
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
	_sprintf(past, pnode, p->next, fnsigs);
	string buffer;
	buffer = past->Sig.string();
	bool openclosehere(1);
	FILE *file = nullptr;
	//is first argument string?
	past->Compute(p);
	if (past->Sig.IsString())
	{
		string filename = past->MakeFilename(past->ComputeString(p), "txt");
		file = fopen(filename.c_str(), "a");
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
