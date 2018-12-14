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
#include "audfret.h"

#include <algorithm> // for lowercase

#ifndef CISIGPROC
#include "psycon.tab.h"
#else
#include "cipsycon.tab.h"
#endif

CVar &CAstSig::define_new_variable(const AstNode *pnode, AstNode *pRHS)
{
	if (pRHS)
	{ 
// evaluate RHS and setvar RHS 
// if it's top level, Setvar(pnode->str, compute(RHS))
// if it's not, pvarLast[pnode->str] = compute(RHS) .... pvarLast should be either empty or struct'ed with other variable(s).... if not throw exception 
// 
		//Before evaluating RHS, if replica is referred under this node, it should throw, because LHS was unknown and replica is not ready. 9/17/2018
		if (searchtree(pRHS, T_REPLICA))
			throw CAstException(pnode, this, "LHS not ready and cannot evaluate based on the LHS.");
		Compute(pRHS);
		const char *tstr0 = NULL, *tstr = NULL;
		//Top level assignment--acceptable only for T_ID without alt or T_ID---N_STRUCT
		// pnode->type should be T_ID at this point
		CVar sigClass;
		if (pnode->alt)
		{
			if (pnode->alt->alt)
			{
				if (pnode->alt->alt->type == N_STRUCT)
					throw CAstException(pnode, this, "Only one-level of class member definition allowed");
				else
					throw CAstException(pnode, this, "specified class member not available.");
			}
			tstr = pnode->alt->str;
			// SetVar takes a blank sigClass and fills in the strut contents 
			SetVar(pnode->alt->str, pgo ? pgo : &Sig, &sigClass);
			tstr0 = pnode->str;
		}
		else
			tstr = pnode->str;
		if (Sig.GetType()==CSIG_HDLARRAY)
			SetVar(tstr, &Sig, pnode->alt ? &sigClass : NULL);
		else if (pgo)
			SetVar(tstr, pgo, pnode->alt ? &sigClass : NULL);
		else
			SetVar(tstr, &Sig, pnode->alt ? &sigClass : NULL);
		if (tstr0) // For struct variable, the base name is set here.
			SetVar(tstr0, &sigClass);
		return Sig;
	}
	throw CAstException(pnode, this, "Internal error--define_new_variable()");
}

void CAstSig::replica_prep(CVar *psig)
{// For GO's, prepping replica is not simple, because a copy of the psig doesn't represent the GO.
 // Therefore, CSIG_HDLARRAY data type is used to carry it forward,
 // because an CSIG_HDLARRAY object can be copied without worries.
 // Note: in this case CSIG_HDLARRAY with size of one can be created. Keep that in mind.
	if (psig->IsGO())
	{ 
		//if pgo is already a CSIG_HDLARRAY with multiple elements
		if (pgo->GetType() == CSIG_HDLARRAY)
			replica = *pgo;
		else
		{
			vector<CVar *> tp(1, pgo);
			replica = *MakeGOContainer(tp);
		}
	}
	else
		replica = *psig;

}

CVar &CAstSig::TID_indexing(AstNode *pLHS, AstNode *pRHS, CVar *psig, CVar *psigBase)
{
	// psig is already prepared; i.e., with NULL pRHS, just return *psig. If pRHS exists, use psigBase 
	// pRHS is never NULL... Check if it is the case if it comes from T-Y-2-1 (TID_tags)
//	if so, return *psig;
	CVar isig, tsig;
	replica = *psig; // whether or not T_REPLICA is in pRHS, just put it here. Not much cost.
	eval_indexing(pLHS->child, psigBase, isig);
//	prepare_endpoint(pLHS->child , psigBase);
	tsig = Compute(pRHS);
	//This is where elements of an existing array are changed, upated or replaced by indices.
	insertreplace(pLHS, psigBase, tsig, isig);
	return Sig=tsig;
}

CVar &CAstSig::TID_tag(const AstNode *pnode, AstNode *p, AstNode *pRHS, CVar *psig, CVar *psigBase)
{
	CVar isig;
	if (p->alt) // T-Y-2-1 
	{
		if (p->alt->type == N_CELL)
		{ // This applies to a{m} = RHS only. a{m}(n) = RHS  should be taken care of by TID_RHS2LHS::N_ARGS
			replica = *psig;
			*psigBase = Compute(pRHS); // Updating Vars by directly accessing the pointer attached to the current cell
		}
		else if (p->alt->type == N_ARGS) // regular indexing
		{
			//This may be obsolete and can be removed... check 11/6/2018
			TID_indexing(p->alt, pRHS, psig, psigBase);
			if (pgo)
			{ // need to get the member name right before N_ARGS
				AstNode *pp = findParentNode((AstNode*)pnode, p, true);
				setgo.type = pp->str;
			}
		}
		else if (p->alt->type == N_STRUCT)
			throw CAstException(p, this, "Only one level of class definition allowed.");
		else 
			throw CAstException(p, this, "Unknown error.");
	}
	else// if (!p->alt) // T-Y-2-2
	{ // no indexing--assign RHS to LHS (if RHS is available) or retrieve LHS
		if (pRHS)
		{
			if (psigBase->IsGO())
			{ //reject an attempt to modify unchangeable struts
				if (!strcmp(p->str,"children") || !strcmp(p->str, "parent") || !strcmp(p->str, "gcf") || !strcmp(p->str, "gca"))
					throw CAstException(p, this, "LHS is unmodifiable.");
			}
			// illegal if p-str is a built-in function name
			if (p->str[0] == '#' || IsValidBuiltin(p->str))
				throw CAstException(p, this, "LHS must be l-value; cannot be a built-in function");
			replica_prep(psigBase);
			CVar *pgo_org = pgo;
			CVar tsig = Compute(pRHS);
//			pgo = pgo_org; // if this is uncommented, a = var_go; won't assign the var_go to a if a is already a go. I forgot when/why I had to add this line. 11/29/2018. 
			//I'm trying this to see if it solves the problem. THis may not be bullet-proof. 11/29/2018
			if (!tsig.IsGO()) pgo = pgo_org;
			//This is where an existing variable is changed, upated or replaced by indices.
			if (p == pnode)
			{	//top-level: SetVar
				if (tsig.GetType()==CSIG_HDLARRAY)
					SetVar(p->str, &tsig);
				else if (pgo)
					SetVar(p->str, pgo);
				else
					SetVar(p->str, &tsig);
			}
			else
			{ // p->type should be N_STRUCT
				//if a member item p->str is available, psigBase is the pointer to that item
				//if the item is not available, psigBase is the pointer to the base
				if (psigBase->IsEmpty() || psigBase->IsStruct())
				{// a.var_not_existing = RHS --> existing Sig is ignored and a new one comes in from Compute(pRHS)
					if (tsig.GetFs()!=3)
					{
						if (tsig.IsGO())
							psigBase->struts[p->str].push_back(pgo);
						else
							psigBase->strut[p->str] = tsig;
					}
					else
						psigBase->struts[pnode->str].push_back(&tsig); // check
				}
				else
				{
					if (tsig.IsGO()) // If the base sig already has corresponding struts, it is a matter of replacing the content, but if the existing member is strut, it is complicated... 9/6/2018
						throw CAstException(p, this, "You are trying to update a GO member variable. This cannot be done due to a known bug. Clear the member variable and try again. Will be fixed someday. bj kwon 9/6/2018.");
					else
						*psigBase = tsig;
				}
				if (pgo) setgo.type = p->str;
			}
		}
		else
		{
			// if trying to retrieve a.var_not_existing Sig is NULL whereas psig is not
			if (Sig.nSamples)
				throw CAstException(p, this, "not available member variable or function.");
			if (p->type == N_CALL) 
				return Sig;
			if (psig)
				return Sig = *psig;
			else
				return Sig;
		}
	}
	return Sig;
}

CVar &CAstSig::TID_cell(AstNode *p, AstNode *pRHS, CVar *psig)
{
	CVar isig;
	if (pRHS)
	{
		if (p->alt) // a{2}(3)
		{ // psig is a{2}
			if (pRHS)
			{
				if (searchtree(pRHS, T_REPLICA))
					replica = ExtractByIndex(p, p->alt->child, &psig);
				eval_indexing(p, psig, isig);
				insertreplace(p, psig, Compute(pRHS), isig); // check
			}
			ExtractByIndex(p, p->child, &psig); // Sig is updated with RHS.
		}
		else
		{ // psig is the cell variable p
			isig = Compute(p->child);
			psig->cell[(int)isig.value() - 1] = Compute(pRHS);
		}
	}
	else
	{

	}
	return Sig;
}

CVar &CAstSig::TID_time_extract(const AstNode *pnode, AstNode *p, AstNode *pRHS, CVar *psig)
{
	CVar isig;
	if (pRHS)
	{
		isig = gettimepoints(pnode, p->child);
		if (searchtree(pRHS, T_REPLICA))
			replica = TimeExtract(pnode, p->child, psig);
		CVar tsig = Compute(pRHS); // rhs
		if (tsig.GetType() != CSIG_AUDIO && !tsig.IsEmpty())
			throw CAstException(p, this, "Referencing timepoint(s) in an audio variable requires another audio signal on the RHS.");
		insertreplace(pnode, psig, tsig, isig);
		return *psig;
	}
	else
		return TimeExtract(pnode, p->child, psig);
}

bool CAstSig::builtin_func_call(NODEDIGGER &ndog, AstNode *p)
{
	/* p->type :
	N_STRUCT --> struct call --> pvar must be ready
	T_ID --> can be both struct call or not --> let's make sure it is non-struct call
	*/
	if (p->type == T_ID || p->type == N_STRUCT)
	{
		if (p->str[0] == '$' || IsValidBuiltin(p->str)) // hook bypasses IsValidBuiltin
		{
			if (ndog.root->type==T_ID && ndog.root->child) // if and only if LHS is a function call
				throw CAstException(ndog.root, this, "LHS must be l-value. Isn't it a built-in function?");
			HandleAuxFunctions(p, ndog.root);
			ndog.psigBase = &Sig;
			return true;
		}
	}
	return false;
}

char *CAstSig::showGraffyHandle(char *outbuf, CVar *psig)
{ // psig must be a single GO
	if (psig->GetFs() == 2)
		sprintf(outbuf, " \"%s\" ", psig->string().c_str());
	else
		sprintf(outbuf, "%.0lf ", psig->value());
	return outbuf;
}