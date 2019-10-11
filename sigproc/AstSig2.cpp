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
			throw ExceptionMsg(pnode, "LHS not ready and cannot evaluate based on the LHS.");
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
					throw ExceptionMsg(pnode, "Only one-level of class member definition allowed");
				else
					throw ExceptionMsg(pnode, "specified class member not available.");
			}
			tstr = pnode->alt->str;
			if (pnode->type == N_HOOK)
			{
				SetGloVar(pnode->alt->str, pgo ? pgo : &Sig, &sigClass);
				tstr0 = pnode->str;
			}
			else
			{
				// SetVar takes a blank sigClass and fills in the strut contents 
				SetVar(pnode->alt->str, pgo ? pgo : &Sig, &sigClass);
				tstr0 = pnode->str;
			}
		}
		else
			tstr = pnode->str;
		if (pnode->type == N_HOOK)
		{
			if (Sig.GetType() == CSIG_HDLARRAY)
				SetGloVar(tstr, &Sig, pnode->alt ? &sigClass : NULL);
			else if (pgo)
				SetGloVar(tstr, pgo, pnode->alt ? &sigClass : NULL);
			else if (!pnode->alt)
				SetGloVar(tstr, &Sig);
		}
		else
		{
			if (Sig.GetType() == CSIG_HDLARRAY)
				SetVar(tstr, &Sig, pnode->alt ? &sigClass : NULL);
			else if (pgo)
				SetVar(tstr, pgo, pnode->alt ? &sigClass : NULL);
			else
				SetVar(tstr, &Sig, pnode->alt ? &sigClass : NULL);
		}
		if (tstr0) // For struct variable, the base name is set here.
		{
			if (pnode->type == N_HOOK)
				SetGloVar(tstr0, &sigClass);
			else
				SetVar(tstr0, &sigClass);
		}
		return Sig;
	}
	throw ExceptionMsg(pnode, "Internal error--define_new_variable()");
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

CVar &CDeepProc::TID_indexing(AstNode *pLHS, AstNode *pRHS, CVar *psig)
{
	// psig is already prepared; i.e., with NULL pRHS, just return *psig. If pRHS exists, use psigBase 
	// pRHS is never NULL... Check if it is the case if it comes from T-Y-2-1 (TID_tags)
//	if so, return *psig;
	CVar isig, tsig;
	pbase->replica = *psig; // whether or not T_REPLICA is in pRHS, just put it here. Not much cost.
	eval_indexing(pLHS->child, isig);
	tsig = pbase->Compute(pRHS);
	//This is where elements of an existing array are changed, upated or replaced by indices.
	pbase->insertreplace(pLHS, level.psigBase, tsig, isig);
	if (isig.nSamples > 0)
	{
		ostringstream oss;
		oss << "(";
		if (isig.nSamples == 1)
		{
			oss << (int)isig.value() << ")";
		}
		else
		{
			oss << "[";
			for (unsigned int k = 0; k < isig.nSamples; k++)
			{
				oss << (int)isig.buf[k];
				if (k == isig.nSamples - 1)
					oss << "])";
				else
					oss << " ";
			}
		}
		level.varname += oss.str();
	}
	return pbase->Sig=tsig;
}

CVar &CDeepProc::TID_tag(const AstNode *pnode, AstNode *p, AstNode *pRHS, CVar *psig)
{
	CVar isig;
	if (p->alt) // T-Y-2-1 
	{
		if (p->alt->type == N_CELL)
		{ // This applies to a{m} = RHS only. a{m}(n) = RHS  should be taken care of by TID_RHS2LHS::N_ARGS
			pbase->replica = *psig;
			*level.psigBase = pbase->Compute(pRHS); // Updating Vars by directly accessing the pointer attached to the current cell
		}
		else if (p->alt->type == N_ARGS) // regular indexing
		{
			//This may be obsolete and can be removed... check 11/6/2018
			TID_indexing(p->alt, pRHS, psig);
			if (pbase->pgo)
			{ // need to get the member name right before N_ARGS
				AstNode *pp = pbase->findParentNode((AstNode*)pnode, p, true);
				pbase->setgo.type = pp->str;
			}
		}
		else if (p->alt->type == N_STRUCT)
		{
			pbase->define_new_variable(pnode, pRHS);
			level.varname += '.';
			level.varname += p->alt->str;
		}
//			throw pbase->ExceptionMsg(p, "Only one level of class definition allowed.");
		else 
			throw pbase->ExceptionMsg(p, "Unknown error.");
	}
	else// if (!p->alt) // T-Y-2-2
	{ // no indexing--assign RHS to LHS (if RHS is available) or retrieve LHS
		if (pRHS)
		{
			if (level.psigBase && level.psigBase->IsGO())
			{ //reject an attempt to modify unchangeable struts
				if (!strcmp(p->str,"children") || !strcmp(p->str, "parent") || !strcmp(p->str, "gcf") || !strcmp(p->str, "gca"))
					throw pbase->ExceptionMsg(p, "LHS is unmodifiable.");
			}
			// illegal if p-str is a built-in function name
			if (p->str[0] == '#' || pbase->IsValidBuiltin(p->str))
				throw pbase->ExceptionMsg(p, "LHS must be l-value; cannot be a built-in function");
			if (level.psigBase) pbase->replica_prep(level.psigBase);
			CVar *pgo_org = pbase->pgo;
			CVar tsig = pbase->Compute(pRHS);
//			pgo = pgo_org; // if this is uncommented, a = var_go; won't assign the var_go to a if a is already a go. I forgot when/why I had to add this line. 11/29/2018. 
			//I'm trying this to see if it solves the problem. THis may not be bullet-proof. 11/29/2018
			if (!tsig.IsGO()) pbase->pgo = pgo_org;
			//This is where an existing variable is changed, upated or replaced by indices.
			if (p == pnode)
			{	//top-level: SetVar
				if (tsig.GetType()==CSIG_HDLARRAY)
					pbase->SetVar(p->str, &tsig);
				else if (pbase->pgo)
					pbase->SetVar(p->str, pbase->pgo);
				else if (p->suppress!=-1) // for the case of (recorder).start--we should keep RHS from affecting the LHS
					pbase->SetVar(p->str, &tsig);
			}
			else
			{ // p->type should be N_STRUCT
				//if a member item p->str is available, level.psigBase is the pointer to that item
				//if the item is not available, level.psigBase is the pointer to the base
				if (level.psigBase->IsStruct())
				{// a.var_not_existing = RHS --> existing Sig is ignored and a new one comes in from Compute(pRHS)
					if (tsig.GetFs()!=3)
					{
						if (tsig.IsGO())
							level.psigBase->struts[p->str].push_back(pbase->pgo);
						else
						{
							level.psigBase->strut[p->str] = tsig;
							level.varname += string(".") + p->str;
						}
					}
					else
						level.psigBase->struts[pnode->str].push_back(&tsig); // check
				}
				else
				{
					if (tsig.IsGO()) // If the base sig already has corresponding struts, it is a matter of replacing the content, but if the existing member is strut, it is complicated... 9/6/2018
						throw pbase->ExceptionMsg(p, "You are trying to update a GO member variable. This cannot be done due to a known bug. Clear the member variable and try again. Will be fixed someday. bj kwon 9/6/2018.");
					else
						*level.psigBase = tsig;
				}
				if (pbase->pgo) 
					pbase->setgo.type = p->str;
			}
		}
		else
		{
			// if trying to retrieve a.var_not_existing Sig is NULL whereas psig is not
			if (pbase->Sig.nSamples)
				throw pbase->ExceptionMsg(p, "not available member variable or function.");
			if (p->type == N_CALL) 
				return pbase->Sig;
			if (psig)
				return pbase->Sig = *psig;
			else
				return pbase->Sig;
		}
	}
	return pbase->Sig;
}

CVar &CDeepProc::TID_time_extract(const AstNode *pnode, AstNode *p, AstNode *pRHS)
{
	CVar isig;
	if (pRHS)
	{
		isig = pbase->gettimepoints(pnode, p->child);
		if (pbase->searchtree(pRHS, T_REPLICA))
			pbase->replica = TimeExtract(pnode, p->child);
		CVar tsig = pbase->Compute(pRHS); // rhs
		if (tsig.GetType() != CSIG_AUDIO && !tsig.IsEmpty())
			throw pbase->ExceptionMsg(p, "Referencing timepoint(s) in an audio variable requires another audio signal on the RHS.");
		pbase->insertreplace(pnode, level.psigBase, tsig, isig);
		return *level.psigBase;
	}
	else
		return TimeExtract(pnode, p->child);
}

CVar &CDeepProc::TimeExtract(const AstNode *pnode, AstNode *p)
{
	char estr[256];
	if (!level.psigBase)
	{
		strcat(estr, " for time extraction ~");
		throw pbase->ExceptionMsg(pnode, estr);
	}
	pbase->checkAudioSig(pnode, *level.psigBase);

	CTimeSeries *pts = level.psigBase;
	for (; pts; pts = pts->chain)
		pbase->endpoint = pts->CSignal::endt().front();
	if (level.psigBase->next)
	{
		pts = level.psigBase->next;
		pbase->endpoint = max(pbase->endpoint, pts->CSignal::endt().front());
	}
	CVar tp = pbase->gettimepoints(pnode, p);
	CVar out(*level.psigBase);
	out.Crop(tp.buf[0], tp.buf[1]);
	return pbase->Sig = out;
}

bool CAstSig::builtin_func_call(CDeepProc &diggy, AstNode *p)
{
	/* p->type :
	N_STRUCT --> struct call --> pvar must be ready
	T_ID --> can be both struct call or not --> let's make sure it is non-struct call
	*/
	if (p->type == T_ID || p->type == N_STRUCT)
	{
		if (p->str[0] == '$' || IsValidBuiltin(p->str)) // hook bypasses IsValidBuiltin
		{
			// if diggy.level.root->child is present, it generally means that RHS exists and should be rejected if LHS is a function call
			// The exception is if LHS has alt node, because it will go down through the next node and it may end up a non-function call.
			if (diggy.level.root->child) 
				if (!diggy.level.root->alt)
					throw ExceptionMsg(diggy.level.root, "LHS must be an l-value. Isn't it a built-in function?");
			HandleAuxFunctions(p, diggy.level.root);
			// while pgo is active, psigBase should point to pgo, not &Sig
			// this causes a crash on the line Sig = *diggy.level.psigBase in read_node() in AstSig.cpp   4/7/2019
			//
			// pgo changed to diggy.level.pgo because in ax.x.lim = [0 getfs/2] when p is getfs pgo should be cleared, but diggy.pbase (which is pgo) should be untouched.
			// 10/10/2019
			diggy.level.psigBase = diggy.level.pgo ? diggy.level.pgo : &Sig;
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

CDeepProc::CDeepProc(CAstSig *past, AstNode *pnode, CVar *psig)
{
	pbase = past;
	level.root = pnode;
	level.psigBase = psig; // NULL except for T_REPLICA
}

CVar &CDeepProc::ExtractByIndex(const AstNode *pnode, AstNode *p)
{ // pnode->type should be N_ARGS
	ostringstream ostream;
	CVar tsig, isig, isig2;
	if (!p->child)	throw pbase->ExceptionMsg(pnode, "A variable index should be provided.");
	eval_indexing(p->child, isig);
	if (isig._max().front() > pbase->Sig.nSamples)
	{
		ostream << "Index " << (int)isig._max().front() << " exceeds the length of " << level.varname;
		throw pbase->ExceptionMsg(pnode, ostream.str().c_str());
	}
	pbase->Sig = extract(pnode, isig);
	return pbase->Sig;
}

CVar &CDeepProc::extract(const AstNode *pnode, CTimeSeries &isig)
{
	//pinout comes with input and makes the output
	//At the end, Sig is updated with pinout
	//pinout always has the same or smaller buffer size, so let's not worry about calling UpdateBuffer() here
	//Instead of throw an exception, errstr is copy'ed. Return value should be ignored.
	// For success, errstr stays empty.
	CSignals out((level.psigBase)->GetFs());
	CTimeSeries *p = &out;
	ostringstream outstream;
	out.UpdateBuffer(isig.nSamples);
	//CSignal::Min() makes a vector
	//body::Min() makes a scalar.
	if (isig._min().front() <= 0.)
	{
		outstream << "Invalid index " << "for " << level.varname << " : " << (int)isig._min().front() << " (must be positive)";
		throw pbase->ExceptionMsg(pnode, outstream.str().c_str());
	}
	if ((level.psigBase)->IsComplex())
	{
		out.SetComplex();
		for (unsigned int i = 0; i < isig.nSamples; i++)
			out.cbuf[i] = (level.psigBase)->cbuf[(int)isig.buf[i] - 1];
	}
	else if ((level.psigBase)->IsLogical())
	{
		out.MakeLogical();
		for (unsigned int i = 0; i < isig.nSamples; i++)
			out.logbuf[i] = (level.psigBase)->logbuf[(int)isig.buf[i] - 1];
	}
	else if ((level.psigBase)->IsString())
	{
		out.UpdateBuffer(isig.nSamples + 1); // make room for null 
		for (unsigned int i = 0; i < isig.nSamples; i++)
			out.strbuf[i] = (level.psigBase)->strbuf[(int)isig.buf[i] - 1];
		out.strbuf[out.nSamples - 1] = 0;
	}
	else
	{
		if ((level.psigBase)->IsGO())
		{
			if (isig._max().front() > (level.psigBase)->nSamples)
			{
				outstream << "Index out of range: " << (int)isig._max().front();
				throw pbase->ExceptionMsg(pnode, outstream.str().c_str());
			}
			if (isig.nSamples == 1)
			{
				size_t did = (size_t)isig.value() - 1;
				CVar *tp = (CVar*)(INT_PTR)(level.psigBase)->buf[did];
				pbase->pgo = level.psigBase = tp;
				return pbase->Sig = *level.psigBase;
			}
			else
			{
				vector<INT_PTR> gos;
				for (unsigned int k = 0; k < isig.nSamples; k++)
				{
					size_t did = (size_t)isig.buf[k] - 1;
					CVar *tp = (CVar*)(INT_PTR)(level.psigBase)->buf[did];
					gos.push_back((INT_PTR)tp);
				}
				level.psigBase = pbase->MakeGOContainer(gos);
				return pbase->Sig = level.psigBase;
			}
		}
		out.SetReal();
		if (pbase->Sig.GetType() == CSIG_VECTOR)
		{
			int id(0);
			for (unsigned int k = 0; k < isig.nSamples; k++)
				out.buf[id++] = (level.psigBase)->buf[(int)isig.buf[k] - 1];
		}
		else if (pbase->Sig.GetType() == CSIG_AUDIO)
		{
			CTimeSeries *_pisig = &isig;
			while (_pisig)
			{
				int cum(0), id(0), lastid = -2;
				vector<int> size2reserve;
				for (unsigned int k = 0; k < _pisig->nSamples; k++)
				{
					id = (int)_pisig->buf[k];
					if (id - lastid > 1)
					{
						if (lastid > 0) size2reserve.push_back(lastid);
						size2reserve.push_back(id);
					}
					lastid = id;
				}
				size2reserve.push_back((int)_pisig->buf[_pisig->nSamples - 1]);
				auto it = size2reserve.begin();
				p->UpdateBuffer(*(it + 1) - *it + 1);
				p->tmark = _pisig->tmark;
				it++; it++;
				lastid = (int)_pisig->buf[0] - 1;
				for (unsigned int i = 0; i < _pisig->nSamples; i++)
				{
					id = (int)_pisig->buf[i] - 1;
					if (id - lastid > 1)
					{
						cum = 0;
						CSignals *pchain = new CSignals(pbase->Sig.GetFs());
						pchain->UpdateBuffer(*(it + 1) - *it + 1);
						pchain->tmark = (double)id / pbase->Sig.GetFs() * 1000.;
						p->chain = pchain;
						p = pchain;
						it++; it++;
					}
					p->buf[cum++] = (level.psigBase)->buf[id];
					lastid = id;
				}
				_pisig = _pisig->chain;
				if (_pisig)
				{
					p->chain = new CTimeSeries;
					p = p->chain;
				}
			}
		}
	}
	out.nGroups = isig.nGroups;
	return (pbase->Sig = out);
}



CVar &CDeepProc::eval_indexing(const AstNode *pInd, CVar &isig)
{
	// input: pInd, level.psigBase
	// output: isig -- sig holding all indices

	// process the first index
	unsigned int len;
	pbase->prepare_endpoint(pInd, level.psigBase);
	try {
		CAstSig tp(pbase);
		if (pInd->type == T_FULLRANGE)
		{ // x(:,ids) or x(:)
			isig.UpdateBuffer((unsigned int)pbase->endpoint);
			for (int k = 0; k < (int)isig.nSamples; k++)	isig.buf[k] = k + 1;
		}
		else
			isig = tp.Compute(pInd);
		if (isig.IsLogical()) pbase->index_array_satisfying_condition(isig);
		// process the second index, if it exists
		if (pInd->next)
		{
			if (level.psigBase->nGroups > 1 && isig.nSamples > 1)
				isig.nGroups = isig.nSamples;
			AstNode *p = pInd->next;
			CVar isig2;
			if (p->type == T_FULLRANGE)
			{// x(ids,:)
				len = level.psigBase->Len();
				isig2.UpdateBuffer(len);
				for (unsigned int k = 0; k < len; k++)	isig2.buf[k] = k + 1;
			}
			else // x(ids1,ids2)
			{
				//endpoint for the second arg in 2D is determined here.
				tp.endpoint = (double)level.psigBase->Len();
				isig2 = tp.Compute(p);
			}
			char buf[128];
			if (isig2.IsLogical()) pbase->index_array_satisfying_condition(isig2);
			else if (isig2._max().front() > (double)level.psigBase->Len())
				throw pbase->ExceptionMsg(pInd, "Out of range: 2nd index ", itoa((int)isig2._max().front(), buf, 10));
			pbase->interweave_indices(isig, isig2, level.psigBase->Len());
		}
	}
	catch (const CAstException &e) {
		throw pbase->ExceptionMsg(pInd, e.getErrMsg().c_str());
	}
	return isig;
}

CVar &CDeepProc::TID_condition(const AstNode *pnode, AstNode *pLHS, AstNode *pRHS, CVar *psig)
{
	unsigned int id = 0;
	CVar isig;
	pbase->replica = pbase->Sig;
	eval_indexing(pLHS, isig);
	CVar rhs = pbase->Compute(pRHS);
	if (rhs.IsScalar())
	{
		//go through chains 6/24/2019
		CTimeSeries *p = level.psigBase;
		CTimeSeries *pisig = &isig;
		while (p)
		{
			for (unsigned k = 0; k < pisig->nSamples; k++)
				p->buf[(int)pisig->buf[k] - 1] = rhs.value();
			p = p->chain;
			pisig = pisig->chain;
		}
	}
	else if (pbase->searchtree(pRHS, T_REPLICA))
	{
		//go through chains 6/24/2019
		CTimeSeries *p = level.psigBase;
		CTimeSeries *pisig = &isig;
		while (p)
		{
			if (p->IsTimeSignal()) // Check this part 9/4/2018
			{
				// consolidate chained rhs with lhs
				for (CTimeSeries *cts = &rhs; cts; cts = cts->chain)
				{
					// id translated from tmark for each chain
					id = (unsigned int)(cts->tmark / 1000. * cts->GetFs());
					memcpy(p->buf + id, cts->buf, cts->nSamples * sizeof(double));
				}
			}
			else
			{
				for (unsigned k = 0; k < pisig->nSamples; k++)
					p->buf[(int)pisig->buf[k] - 1] = rhs.buf[k];
			}
			p = p->chain;
			pisig = pisig->chain;
		}
	}
	else if (rhs.IsEmpty())
	{
		//do we need to go through chains??? 6/24/2019
		pbase->insertreplace(pLHS, level.psigBase, rhs, isig);
		pbase->Sig.Reset();
	}
	else
		throw pbase->ExceptionMsg(pRHS, "Invalid RHS; For LHS with conditional indexing, RHS should be either a scalar, empty, or .. (self).");
	pbase->SetVar(pnode->str, level.psigBase);
	return pbase->Sig;
}

CVar &CDeepProc::TID_RHS2LHS(const AstNode *pnode, AstNode *pLHS, AstNode *pRHS, CVar *psig)
{ // Computes pRHS, if available, and assign it to LHS.
	// 2 cases where psig is not Sig: first, a(2), second, a.sqrt
	CVar tsig;
	if (pbase->IsCondition(pLHS))
		return TID_condition(pnode, pLHS, pRHS, psig);
	switch (pLHS->type)
	{
	case N_ARGS:
		tsig = TID_indexing(pLHS, pRHS, psig);
		if (pbase->pgo)
		{ // need to get the member name right before N_ARGS
			AstNode *pp = pbase->findParentNode((AstNode*)pnode, pLHS, true);
			pbase->setgo.type = pp->str;
		}
		break;
	case T_ID: // T-Y-2
	case N_STRUCT:
	case N_CALL:
		tsig = TID_tag(pnode, pLHS, pRHS, psig);
		break;
	case N_TIME_EXTRACT: // T-Y-4 
		tsig = TID_time_extract(pnode, pLHS, pRHS);
		break;
	case N_HOOK:
		if (pbase->pEnv->pseudo_vars.find(pnode->str) == pbase->pEnv->pseudo_vars.end())
		{
			tsig = TID_tag(pnode, pLHS, pRHS, psig);
		}
		else
			throw pbase->ExceptionMsg(pLHS, "Pseudo variable cannot be modified.");
		break;
	default:  // T-Y-5
		return pbase->Compute(pRHS); // check... this may not happen.. if so, inspect.
	}
	return pbase->Sig;
}
