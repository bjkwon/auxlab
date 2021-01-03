// AUXLAB 
//
// Copyright (c) 2009-2019 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.704
// Date: 1/1/2021

#include <sstream>
#include <list>
#include <algorithm>
#include <exception>
#include <math.h>
#include <time.h>
#include "aux_classes.h"
#include "sigproc.h"
#include "psycon.tab.h"

AstNode* goto_try_line(const AstNode* pnode, int line)
{
	AstNode* pp, * p = (AstNode*)pnode;
	for (; p; p = p->next)
	{
		if (p->type == T_IF || p->type == T_FOR || p->type == T_WHILE || p->type == T_TRY) // line should be inside of block, i.e., T_FOR T_IF or T_WHILE
		{
			pp = CAstSig::goto_line((const AstNode*)p->child->next, line);
			if (pp) return pp;
		}
		if (p->line == line) return p;
	}
	return p;
}

const AstNode* get_try_node(const AstNode* pnode)
{
	const AstNode* p = pnode;
	// for a base udf, pnode is N_BLOCK; otherwise, it is T_FUNCTION
	if (p->type == T_FUNCTION) p = p->child->next;
	for (; p; p = p->next)
		if (p->type == T_TRY)
			return p;
	return NULL;
}

const AstNode* get_base_node_for_try (const AstNode* pnode, int line)
{// Get the node of try from the t_func node of the base udf
	// First, find out in which (local) function try was called
	const AstNode* p = pnode;
	for (; p; p = p->next)
	{
		if (!p->next)
			return p;
		else if (line < p->next->line)
			return p;
	}
	return NULL;
}

CVar* CAstSig::Try_here(const AstNode* pnode, AstNode* p)
{ // to be used only for udf 
	try {
		Compute(p);
	}
	catch (const CAstException& e) {
		// If an exception is thrown inside of a try in a udf, e carries the information 
		// at the time of exception, including where it occurred, the xtree node info, etc,
		// with inTryCatch set. That information is copied to the variable specified in
		// catch, and the udf proceeds to the bottom of CallUDF(), to the next line
		// i.e., the first line of catch.
		// For other exceptions, this is bypassed and e is further thrown to the user
		// to be captured by xcom 1/1/2021
		if (inTryCatch)
		{ // Make a new struct variable from child of e.pTarget (which is T_CATCH)
			auto baseudf = get_base_node_for_try(e.pCtx->u.t_func_base, e.line);
			auto pnode_try = get_try_node(baseudf);
			const char* name = pnode_try->alt->child->str; // the variable name of catch (as "catchme" in catch "catchme")
			SetVar(name, &CVar()); // new temporary variable; OK this way, the temp. var. is deep-copied
			string emsg = e.outstr;
			size_t id = emsg.find("[GOTO_BASE]");
			if (id != string::npos) emsg = emsg.substr(id + string("[GOTO_BASE]").size());
			CVar msg(emsg);
			SetVar("full", &msg, &Vars[name]);
			msg = e.basemsg;
			SetVar("base", &msg, &Vars[name]);
			emsg = e.msgonly;
			if (id != string::npos) emsg = emsg.substr(id + string("[GOTO_BASE]").size());
			msg = emsg;
			SetVar("body", &msg, &Vars[name]);
			msg = e.sourceloc;
			SetVar("source", &msg, &Vars[name]);
			msg.SetValue(e.line);
			SetVar("errline", &msg, &Vars[name]);
			msg.SetValue(e.col);
			SetVar("errcol", &msg, &Vars[name]);
			Compute(pnode_try->alt->next);
		}
		else
			throw e;
	}
	return &Sig;
}