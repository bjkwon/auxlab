// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.495
// Date: 12/13/2018
// 
#include <sstream>
#include <list>
#include <algorithm>
#include <exception>
#include <math.h>
#include <time.h>
#include "sigproc.h"
#include "bjcommon.h"

#include <algorithm> // for lowercase

#ifndef CISIGPROC
#include "psycon.tab.h"
#else
#include "cipsycon.tab.h"
#endif

/* NOTE 9/23/2017
Don't use------- throw CAstException(p, this, "error message");
when p->type is N_XXXXXX, because it will make an error msg like 

ERROR: N_EXTRACT : variable not available.

which is not helpful to users.
*/
string GetNodeType(int type)
{ // from psycon.yacc.h
	switch (type)
	{
	case N_BLOCK:
		return "N_BLOCK";
	case N_ARGS:
		return "N_ARGS";
	case N_MATRIX:
		return "N_MATRIX";
	case N_VECTOR:
		return "N_VECTOR";
	case N_CALL:
		return "N_CALL";
	case N_CELL:
		return "N_CELL";
	case N_STRUCT:
		return "N_STRUCT";
	case N_IDLIST:
		return "N_IDLIST";
	case N_TIME_EXTRACT:
		return "N_TIME_EXTRACT";
	case N_CELLASSIGN:
		return "N_CELLASSIGN";
	case N_IXASSIGN:
		return "N_IXASSIGN";
	case N_USEC:
		return "N_USEC";
	case N_MSEC:
		return "N_MSEC";
	case N_CIPULSE3:
		return "N_CIPULSE3";
	case N_CIPULSE4:
		return "N_CIPULSE4";
	case N_CIPULSE5:
		return "N_CIPULSE5";
	case N_CIPULSE6:
		return "N_CIPULSE6";
	default:
		return "Unknown NODE";
	}
}


void adjust_AstNode(const AstNode *p)
{
	//if (p)
	//{
	//	AstNode *pp = (AstNode *)p;
	//	pp->line--;
	//	if (p->child) adjust_AstNode(p->child);
	//	if (p->next) adjust_AstNode(p->next);
	//	if (p->alt) adjust_AstNode(p->alt);
	//}
}

// as long as CAstSig *past is used to construct CAstException, cleanup_sons() is called
// and necessary clean up is taken care of, upon exception thrown (error)
// Take care of CAstException constructors not using CAstSig *past... 

CAstException::CAstException(const AstNode *p, CAstSig *pContext, const string s1, const string s2)
: pCtx(pContext), pnode(p), str1(s1), str2(s2), int1(0)
{
	if (pCtx && !strcmp(pCtx->u.application, "auxcon"))
		adjust_AstNode(pnode);
	if (pnode && pnode->str)
	{
		string msg = pnode->str + string(" : ");
		str1.replace(0, 0, msg);
	}
	if (pCtx && pCtx->u.debug.status == typed_line) return;
	str1.insert(0, "[GOTO_BASE]");
	if (!p) pnode = pCtx->pAst;
	makeOutStr();
	if (outstr.empty())
	{ // What is this for? One example: exception thrown in RegisterUDF()
		outstr = str1 + " : "; 
		outstr += str2;
	}
}

CAstException::CAstException(const AstNode *p, CAstSig *past, const string s1, const int x, const string s2)
: pCtx(past), pnode(p), str1(s1), str2(s2), int1(x)
{
	if (past && !strcmp(past->u.application, "auxcon"))
		adjust_AstNode(pnode);
	if (pCtx && pCtx->u.debug.status == typed_line) return;
	str1.insert(0, "[GOTO_BASE]");
	makeOutStr();
}

CAstException::CAstException(const AstNode *p0, CAstSig *pContext, const char* msg)
	: pCtx(pContext), pnode(p0), int1(0)
{
	if (pCtx && !strcmp(pCtx->u.application,"auxcon"))
		adjust_AstNode(pnode);
	char buf[64];
	string unknown;
	if (pnode&&pnode->str)	unknown = pnode->str;
	else {			// needed for calls from CSignals &CAstSig::Compute(const AstNode *pnode)
		if (pnode->type > 256) { sprintf(buf, "%s", GetNodeType(pnode->type).c_str()); unknown = buf; }
		else			unknown = pnode->type;
	}
	str1 = " ";
	str1 += unknown + " : ";
	str1 += msg + string("\n");
	outstr = str1;
	//If this is user-typed lines during debugging (F10, F5,... etc), it should just return without going into what's going on in the current UDF.
	if (pCtx && pCtx->u.debug.status == typed_line) return;
	str1.insert(0, "[GOTO_BASE]");
	if (!pnode) pnode = pCtx->pAst;
	makeOutStr();
}

void CAstException::makeOutStr()
{
	ostringstream oss;
	oss << str1 + ' ';
	if (!str2.empty())
		oss << " \"" << str2 << "\" ";
	if (pCtx)
	{
		if (!pCtx->dad) 
			return;
		vector<int> lines;
		vector<string> strs;
		CAstSig *tp = pCtx;
		char *pstr=NULL;
		while (tp)
		{
			if (!tp->u.base.empty())
			{
				if (tp->u.base == tp->u.title) // base udf (or "other" udf)
					strs.push_back(tp->pEnv->udf[tp->u.base].fullname);
				else
					strs.push_back(string ("function \"") + tp->u.title + "\"");
				if (tp->pLast)
				{
					lines.push_back(tp->pLast->line);
				}
			}
			tp=tp->dad;
		}
		if (!strs.empty())
		{
			vector<string>::iterator it2 = strs.begin();
			//at this point strs can have more items than lines and cols, because son->son is son during CallUDF()
			//so don't use strs iterator for for 
			for (vector<int>::iterator it=lines.begin(); it!=lines.end(); it++, it2++)
			{
				oss << '\n' << "line " << *it;
				if (it == lines.begin()) oss << ", col " << pnode->col;
				oss	<< " in " << *it2;
			}
		}
	}
	else
		oss << "\nIn line " << pnode->line << ", col " << pnode->col;
	if (int1)
		oss << int1;
	outstr = oss.str();
	// if the exception is thrown from auxcon, it will skip
	if (pCtx->dad) // if this call is made for a udf --but if from a local function, or other udf called by that udf, it might be different... think about a better solution. 11/16/2017
		if (pCtx->pAst && pCtx->pAst->type == N_BLOCK) // pCtx->pCtx is checked to avoid crash during wavwrite(undefined_var,"filename")
			CAstSig::cleanup_nodes(pCtx);
}

