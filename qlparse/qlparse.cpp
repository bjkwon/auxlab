// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: qlparse
// Library to parse a directory string
// 
// 
// Version: 1.495
// Date: 12/13/2018
// 
#include "qlparse.h"

#ifndef LQSYNYACC
#include "lqsyn.yacc.h"
#endif

using namespace std;

AstNode *QLSetNewScript(const char *str, string &Errmsg)
{
	AstNode *pAst(NULL);
	int res;
	Errmsg.clear();
	static bool fAllocatedAst(false);
	char *errmsg;
	if (strlen(str) == 0) return pAst;

	if (fAllocatedAst) {
		yydeleteAstNode(pAst, 0);
		fAllocatedAst = false;
	}
	if ((res = yysetNewStringToScan(str)))
	{
		Errmsg = "yysetNewStringToScan() failed!";
	}
	else
	{
		res = yyparse(&pAst, &errmsg);
		fAllocatedAst = pAst ? true : false;
		if (!errmsg && res == 2)
		{
			errmsg = "Out of memory!";
			Errmsg = "Out of memory!";
		}
	}
	if (errmsg || !Errmsg.empty()) {
		if (fAllocatedAst) {
			yydeleteAstNode(pAst, 0);
			fAllocatedAst = false;
		}
		return NULL;
	}
	else if (res)
		return NULL;
	return pAst;
}

__declspec (dllexport) vector<string> qlparse(const char* in, string &errmsg)
{
	vector<string> out;
	AstNode *pParsed = QLSetNewScript(in, errmsg);
	for (AstNode *p = pParsed; p; p = p->next)
		out.push_back(p->str);
	return out;
}

#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}