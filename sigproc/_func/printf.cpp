#include "sigproc.h"
#ifdef _WINDOWS
#include <atlrx.h>
#endif
extern map<double, FILE*> file_ids;

void processEscapes(string& str)
{
	size_t pos;
	for (size_t start = 0; (pos = str.find('\\', start)) != string::npos; start = pos + 1)
		switch (str[pos + 1]) {
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

void _sprintf(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	CAtlRegExp<> regexp;
	REParseError status = regexp.Parse("%([-+ #0]+)?({\\z|\\*})?(\\.{\\z|\\*})?[hlL]?{[cuoxXideEgGfs]}");
	if (status != REPARSE_ERROR_OK)
		throw CAstException(INTERNAL, *past, pnode).proc("_sprintf()--RegExp.Parse( ) failed.");
	past->Sig.Reset(2);	// to get the output string
	CAstSig tast(past);	// to preserve this->Sig
	CAtlREMatchContext<> mcFormat;
	string fmtstring = tast.ComputeString(p);
	processEscapes(fmtstring);
	const char* fmtstr = fmtstring.c_str();
	for (const char* next; fmtstr && regexp.Match(fmtstr, &mcFormat, &next); fmtstr = next) {
		const CAtlREMatchContext<>::RECHAR* szStart = 0;
		const CAtlREMatchContext<>::RECHAR* szEnd = 0;
		string vstring;
		double v;
		string fmt1str(fmtstr, mcFormat.m_Match.szEnd - fmtstr);
		vector<char> outStr;
		outStr.resize(100);
		for (UINT nGroupIndex = 0; nGroupIndex < mcFormat.m_uNumGroups; ++nGroupIndex) {
			mcFormat.GetMatch(nGroupIndex, &szStart, &szEnd);
			if (nGroupIndex == 2 || (nGroupIndex < 2 && szStart && *szStart == '*')) {	// condition for consuming an argument
				if ((p = p->next) == NULL)
					throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, "Not enough arguments.");
				if (nGroupIndex == 2 && *szStart == 's')
					vstring = tast.ComputeString(p);
				else if (tast.Compute(p)->IsScalar())
					v = tast.Sig.value();
				else
					throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, "Scalar value expected for this argument.");
				if (nGroupIndex != 2) {
					char width[20];
					sprintf(width, "%d", round(v));
					fmt1str.replace(szStart - fmtstr, 1, width);
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
			outStr.resize(vstring.size() + 100);
			sprintf(&outStr[0], fmt1str.c_str(), vstring.c_str());
			break;
		}
		unsigned int n = (unsigned int)past->Sig.CSignal::length();
		past->Sig.UpdateBuffer(n + (unsigned int)strlen(&outStr[0]) + 1);
		strcpy(&past->Sig.strbuf[n], &outStr[0]);
	}
	unsigned int n = (unsigned int)past->Sig.CSignal::length();
	past->Sig.UpdateBuffer(n + (unsigned int)strlen(fmtstr) + 1);
	strcpy(&past->Sig.strbuf[n], &fmtstr[0]);
	past->Sig.bufBlockSize = 1;
}

void _fprintf(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	CVar firstarg = past->Sig;
	_sprintf(past, pnode, p, fnsigs);
	string buffer;
	buffer = past->Sig.string();
	bool openclosehere(1);
	FILE* file = nullptr;
	//is first argument string?
//	past->Compute(p);
	if (firstarg.IsString())
	{
		string filename = past->makefullfile(firstarg.string());
		file = fopen(filename.c_str(), "at");
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
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, "First arg must be either a file identifider, filename or 0 (for console)");
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

