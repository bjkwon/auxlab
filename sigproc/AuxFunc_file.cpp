#include <string.h> // aux_file
#include "sigproc.h"
#include "sigproc_internal.h"

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
			for (unsigned k = 0; k < past->Sig.nSamples; k++)
			{
				char temp = (char)past->Sig.buf[k];
				fwrite(&temp, 1, 1, file);
			}
	}
	else if (prec == "int8")
	{
		int8_t temp;
		for (unsigned k = 0; k < past->Sig.nSamples; k++)
		{
			temp = (int8_t)past->Sig.buf[k];
			fwrite(&temp, bytes, 1, file);
		}
	}
	else if (prec == "uint8")
	{
		uint8_t temp;
		for (unsigned k = 0; k < past->Sig.nSamples; k++)
		{
			temp = (uint8_t)past->Sig.buf[k];
			fwrite(&temp, bytes, 1, file);
		}
	}
	else if (prec == "int16")
	{
		int16_t temp;
		if (past->Sig.next)
		{
			for (unsigned k = 0; k < past->Sig.nSamples; k++)
			{
				temp = (int16_t)past->Sig.buf[k];
				fwrite(&temp, bytes, 1, file);
				temp = (int16_t)past->Sig.next->buf[k];
				fwrite(&temp, bytes, 1, file);
			}
		}
		else
			for (unsigned k = 0; k < past->Sig.nSamples; k++)
			{
				temp = (int16_t)past->Sig.buf[k];
				fwrite(&temp, bytes, 1, file);
			}
	}
	else if (prec == "uint16")
	{
		uint16_t temp;
		for (unsigned k = 0; k < past->Sig.nSamples; k++)
		{
			temp = (uint16_t)past->Sig.buf[k];
			fwrite(&temp, bytes, 1, file);
		}
	}
	else if (prec == "int32")
	{
		int32_t temp;
		for (unsigned k = 0; k < past->Sig.nSamples; k++)
		{
			temp = (int32_t)past->Sig.buf[k];
			fwrite(&temp, bytes, 1, file);
		}
	}
	else if (prec == "uint32")
	{
		uint32_t temp;
		for (unsigned k = 0; k < past->Sig.nSamples; k++)
		{
			temp = (uint32_t)past->Sig.buf[k];
			fwrite(&temp, bytes, 1, file);
		}
	}
	else if (prec == "float")
	{
		float temp;
		for (unsigned k = 0; k < past->Sig.nSamples; k++)
		{
			temp = (float)past->Sig.buf[k];
			fwrite(&temp, bytes, 1, file);
		}
	}
	else if (prec == "double")
	{
		fwrite(past->Sig.buf, bytes, past->Sig.nSamples, file);
	}
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
	past->Sig.Reset(1); // make it non-audio
	past->Sig.UpdateBuffer(nItems);
	size_t cum = 0, res = 1;
	if (prec == "char")
	{
		past->Sig.SetString('\0');
		while (res)
			res = fread(past->Sig.strbuf + cum++, 1, 1, file);
	}
	else if (prec == "int8" || prec == "uint8")
	{
		int8_t temp;
		while (res)
		{
			res = fread(&temp, bytes, 1, file);
			past->Sig.buf[cum++] = (double)temp;
		}
	}
	else if (prec == "int16" || prec == "uint16")
	{
		int16_t temp;
		while (cum < nItems)
		{
			res = fread(&temp, bytes, 1, file);
			past->Sig.buf[cum++] = (double)temp;
		}
	}
	else if (prec == "int32" || prec == "uint32")
	{
		int32_t temp;
		while (res)
		{
			res = fread(&temp, bytes, 1, file);
			past->Sig.buf[cum++] = (double)temp;
		}
	}
	else if (prec == "float")
	{
		float temp;
		while (res)
		{
			res = fread(&temp, bytes, 1, file);
			past->Sig.buf[cum++] = (double)temp;
		}
	}
	else if (prec == "double")
	{
		double temp;
		while (res)
		{
			res = fread(&temp, bytes, 1, file);
			past->Sig.buf[cum++] = (double)temp;
		}
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
