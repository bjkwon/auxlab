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
#include <assert.h>
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
	default:
		return "Unknown NODE";
	}
}

CAstException::CAstException(EXCEPTIONTYPE extp, const CAstSig &base, const AstNode *_pnode)
{
	type = extp;
	pCtx = &base;
	pnode = _pnode ? _pnode : base.xtree;
	if (base.inTryCatch)
		findTryLine(base);
	line = pnode->line;
	col = pnode->col;
}

CAstException &CAstException::proc(const char * _basemsg, const char* tidname, string extra)
{ //FUNC_SYNTAX; INTERNAL
	assert(type == FUNC_SYNTAX || type == INTERNAL || type == USAGE || type == UNDEFINED_TID);
	string fnsigs;
	if (pnode->str)
	{
		fnsigs = pnode->str; // 5/23/2021 11:36AM
		auto ft = pCtx->pEnv->builtin.find(pnode->str);
		if (ft != pCtx->pEnv->builtin.end())
			fnsigs += (*ft).second.funcsignature;
	}
	msgonly = basemsg = _basemsg;
	if (type == INTERNAL)
		msgonly = string("[INTERNAL] ") + _basemsg;
	else if (type == UNDEFINED_TID)
		msgonly += string(" ") + (tidstr = tidname);
	else if (!fnsigs.empty())
		msgonly += string("\n  Usage: ") + fnsigs;
	if (!extra.empty())
		msgonly += string("\n") + extra;
	arrayindex = cellindex = -1;
	addLineCol();
	outstr = msgonly + sourceloc;
	return *this;
}

CAstException& CAstException::proc(const string &tidstr, const string &errmsg)
{ //Handling errors after function processing; no need to display the usage help, but only display the error message from the function
	assert(type == ERR_POST_FNC);
	msgonly = errmsg;
	addLineCol();
	ostringstream oss;
	if (msgonly.back()=='\n')
		oss << errmsg << "From: " << tidstr << "(...) ";
	else
		oss << errmsg << endl << "From: " << tidstr << "(...)  ";
	msgonly = oss.str().c_str();
	outstr = msgonly + sourceloc;
	return *this;
}

CAstException &CAstException::proc(const char * _basemsg, const char * varname, int indexSpecified, int indexSpecifiedCell)
{ //range
	assert(type == RANGE);
	msgonly = basemsg = _basemsg;
	if (strlen(varname) > 0)
		msgonly += (tidstr = varname);
	arrayindex = indexSpecified;
	cellindex = indexSpecifiedCell;
	ostringstream oss;
	if (cellindex >= 0) // if cell index is invalid, it doesn't check indexSpecified
		oss << "Out of range : " << msgonly << " cell index " << cellindex;
	else
		oss << "Out of range : " << msgonly << " index " << indexSpecified;
	msgonly = oss.str().c_str();
	addLineCol();
	outstr = msgonly + sourceloc;
	return *this;
}
CAstException &CAstException::proc(const char * _basemsg, const char * funcname, int argIndex)
{ // args
	assert(type == ARGS);
	basemsg = _basemsg;
	ostringstream oss;
	oss << "" << funcname << " : " << "argument " << argIndex << ' ' << _basemsg;
	msgonly = oss.str().c_str();
	arrayindex = argIndex;
	cellindex = -1;
	addLineCol();
	outstr = msgonly + sourceloc;
	return *this;
}

void CAstException::findTryLine(const CAstSig & scope)
{
	//go upstream from pLast until T_TRY is found
	//then update pLast with the catch node
	//begin with scope.xtree which is pnode
	for (const AstNode* p = scope.xtree; p; p = p->next)
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
		if (pCtx->level == pCtx->baselevel.back()) return;
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
