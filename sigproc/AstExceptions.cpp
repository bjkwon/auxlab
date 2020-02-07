// AUXLAB 
//
// Copyright (c) 2009-2019 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.5
// Date: 3/30/2019
// 
#include <sstream>
#include <list>
#include <algorithm>
#include <exception>
#include <math.h>
#include <time.h>
#include "aux_classes.h"
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

// The only reason pnode is left is for auxcon stuff, but otherwise they can go 3/15/2019
CAstException CAstSig::ExceptionMsg(const AstNode *pnode, const string s1, const string s2)
{
	return CAstException(pnode, this, s1, s2);
}
CAstException CAstSig::ExceptionMsg(const AstNode *pnode, const char *msg)
{
	return CAstException(pnode, this, msg);
}

CAstException::CAstException(CAstSig *pContext, const string s1, const string s2)
	: pCtx(pContext), str2(s2)
{
#ifdef _WINDOWS
	if (pContext->u.pUDF)
		pnode = pContext->u.pUDF;
	else
#endif
		pnode = pContext->pAst;
	//Use this format when you are not sure what pnode to use
	// Probably it's safe to use this all the time. 
	//Replace the other constructor CAstException(const AstNode *p, CAstSig *pContext, const string s1, const string s2)
	//with this eventually 3/30/2019

#ifdef _WINDOWS
	if (pCtx && !strcmp(pCtx->u.application, "auxcon"))
		adjust_AstNode(pnode);
#endif
	str1 = pnode->str;
	str1 += " : ";
	str1 += s2 + '\n';
	str1 += s1;
	outstr = str1;
#ifdef _WINDOWS
	if (pCtx && pCtx->u.debug.status == typed_line) return;
#endif
	str1.insert(0, "[GOTO_BASE]");
	if (!pnode) pnode = pCtx->pAst;
//	makeOutStr();
	outstr += '\n';
}

CAstException::CAstException(const AstNode *p, CAstSig *pContext, const string s1, const string s2)
: pCtx(pContext), pnode(p), str2(s2)
{
	//Use this format to create an exception message for invalid function definition (and others)
	//p carries the function name
#ifdef _WINDOWS
	if (pCtx && !strcmp(pCtx->u.application, "auxcon"))
		adjust_AstNode(pnode);
#endif
	ostringstream oss;
	if (p->str)
		oss << p->str;
	else if (p->type == T_NUMBER)
		oss << p->dval;
	else
		oss << "unknown type ";
	oss << " : " << s2 << s1;
	outstr = str1 = oss.str().c_str();
#ifdef _WINDOWS
	if (pCtx && pCtx->u.debug.status == typed_line) return;
#endif
	str1.insert(0, "[GOTO_BASE]");
	if (!p) pnode = pCtx->pAst;
//	makeOutStr();
	outstr += '\n';
}

CAstException::CAstException(const AstNode *p, CAstSig *pAst, const string s1)
{
	CAstException(p, pAst, s1.c_str());
}

CAstException::CAstException(const AstNode *p0, CAstSig *pContext, const char* msg)
	: pCtx(pContext), pnode(p0)
{
#ifdef _WINDOWS
	if (pCtx && !strcmp(pCtx->u.application,"auxcon"))
		adjust_AstNode(pnode);
#endif
	outstr = str1 = msg;
	//If this is user-typed lines during debugging (F10, F5,... etc), it should just return without going into what's going on in the current UDF.
#ifdef _WINDOWS
	if (pCtx && pCtx->u.debug.status == typed_line) return;
#endif
	str1.insert(0, "[GOTO_BASE]");
	if (!pnode) pnode = pCtx->pAst;
//	makeOutStr();
	outstr += '\n';
}

void CAstException::findTryLine(const CAstSig & scope)
{
	//go upstream from pLast until T_TRY is found
	//then update pLast with the catch node
	//begin with scope.pAst which is pnode
	for (const AstNode* p = scope.pAst; p; p = p->next)
	{
		if (p->type == T_TRY) pTarget = p->alt;
		if (p == scope.pLast)
		{
			return;
		}
	}
	return;
}

void CAstException::addLineCol()
{
	ostringstream oss;
	if (!pCtx && pnode)
		oss << "\nIn line " << pnode->line << ", col " << pnode->col; // is this necessary? 1/12/2020
	else
	{
		if (!pCtx->dad) return;
		vector<int> lines;
		vector<string> strs;
		const CAstSig *tp = pCtx;
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
					lines.push_back(tp->pLast->line);
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
	sourceloc = oss.str();
}

void CAstException::clean()
{
	// if the exception is thrown from auxcon, it will skip
	if (pCtx && pCtx->dad) // if this call is made for a udf --but if from a local function, or other udf called by that udf, it might be different... think about a better solution. 11/16/2017
		if (pCtx->pAst && pCtx->pAst->type == N_BLOCK) // pCtx->pCtx is checked to avoid crash during wavwrite(undefined_var,"filename")
			CAstSig::cleanup_nodes((CAstSig *)pCtx);
}

