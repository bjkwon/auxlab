// AUXLAB 
//
// Copyright (c) 2009-2019 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.7
// Date: 5/24/2020

#include <sstream>
#include <list>
#include <algorithm>
#include <exception>
#include <math.h>
#include <time.h>
#include "aux_classes.h"
#include "sigproc.h"
#include "deque"

#include <algorithm> // for lowercase
#include <assert.h>

#ifndef CISIGPROC
#include "psycon.tab.h"
#else
#include "cipsycon.tab.h"
#endif
#include "sigproc_internal.h"


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

void CNodeProbe::insertreplace(const AstNode *pnode, CVar &sec, CVar &indsig)
{
	bool logicalindex = indsig.IsLogical();
	AstNode *p = pnode->alt;
	if (psigBase->GetType() != CSIG_AUDIO && indsig.IsLogical())
	{ // For non-audio, if isig is the result of logical operation, get the corresponding indices 
		CSignals trueID(1);
		trueID.UpdateBuffer(indsig.nSamples);
		int m = 0;
		for (unsigned int k(0); k < indsig.nSamples; k++)
			if (indsig.logbuf[k])
				trueID.buf[m++] = k + 1; // because aux is one-based index
		trueID.UpdateBuffer(m);
		indsig = trueID;
	}
	if (!indsig.nSamples) return;
	else // not done yet if sec is complex
	{
		//		pnode->type is either N_IXASSIGN (for non-cell LHS)
		//		or cell index for cell LHS,  for example, T_NUMBER for cc{3}
		unsigned int id1, id2;
		AstNode *repl_RHS = pbase->searchtree(pnode->child, T_REPLICA);
		if (psigBase->type() & TYPEBIT_TEMPORAL)
		{
			CVar isig2;
			if (pnode->child->next)
			{
				if (pnode->child->next->type == T_FULLRANGE)
				{
					if (sec.next)
						throw CAstException(USAGE, *pbase, pnode).proc("LHS is mono; RHS should not be stereo.");
					if (indsig.value() == 2.)
					{
						delete psigBase->next;
						psigBase->next = new CSignals(sec.GetFs());
						*(psigBase->next) = sec;
					}
					else
					{
						*psigBase = sec;
					}
					return;
				}
				else
					eval_indexing(pnode->child->next, isig2);
			}
			if (pnode->type == N_ARGS || (pnode->next && pnode->next->type == N_CALL))
			{
				CVar* pid = pnode->child->next ? &isig2 : &indsig;
				// CAstSig::replace(const AstNode *pnode, CTimeSeries *pobj, body &sec, int id1, int id2)
				// actually works efficiently (without blindly adding to the buffer) when there's no change in nSamples
				// so I'm not doing if (repl_RHS) here.
				// for lines below, it just looks long, but having if (repl_RHS) won't do any harm (maybe tiny bit faster)
				// so I'm not getting rid of it below. 
				// Buf probably I really need to check logical cases instead!!!
				// 11/7/2019
				if (pid->IsLogical()) // s(conditional_var)
				{
					for (CTimeSeries* p = &sec; p; p = p->chain)
					{
						int id((int)(p->tmark * sec.GetFs() / 1000 + .5));
						for (unsigned int k = 0; k < p->nSamples; k++)
							if (pid->logbuf[id + k])
								psigBase->buf[id + k] = p->buf[(int)k];
					}
				}
				else // s(id1:id2) or cel{n}(id1:id2)
				{ // x(1200:1201) = zeros(111) FAILED HERE	on memcpy line.... that's wrong. 1/20/2018
					// this must be contiguous
					if (!pbase->isContiguous(*pid, id1, id2))
					{
						if (sec.IsEmpty())
							throw CAstException(USAGE, *pbase, pnode).proc("For non-consecutive indexing, RHS cannot be empty.");
					//	if (sec.nSamples != 1 && sec.nSamples != indsig.nSamples)
					//		throw CAstException(USAGE, *pbase, pnode).proc("For non-consecutive indexing, the length of RHS and LHS must match.");
						vector<unsigned int> ids;
						ids.resize(pid->nSamples);
						unsigned int k = 0;
						if (pnode->child->next)
							for_each(ids.begin(), ids.end(), [isig2, &k](unsigned int& v) { v = (unsigned int)isig2.buf[k++] - 1; });
						else
							for_each(ids.begin(), ids.end(), [indsig, &k](unsigned int& v) { v = (unsigned int)indsig.buf[k++] - 1; });
						if (sec.type() & 1) // scalar
							psigBase->replacebyindex(ids.begin(), ids.end(), sec);
						else
						{
							CTimeSeries* pchain = &sec;
							for (auto itBegin = ids.begin(); pchain; pchain = pchain->chain)
							{
								psigBase->replacebyindex(itBegin, itBegin + pchain->nSamples, *pchain);
								itBegin += pchain->nSamples;
							}
						}
					}
					else
					{
						if (pnode->child->next)
						{
							if (indsig.value()==2.)
								psigBase->next->replacebyindex(id1 - 1, id2 - id1 + 1, sec);
							else
								psigBase->replacebyindex(id1 - 1, id2 - id1 + 1, sec);
						}
						else
							psigBase->replacebyindex(id1 - 1, id2 - id1 + 1, sec);
					}
				}
			}
			else if (!pnode->next && !pnode->str) // if s(conditional) is on the LHS, the RHS must be either a scalar, or the replica, i.e., s(conditional)
			{
				if (!repl_RHS && !indsig.IsLogical())
					throw CAstException(INTERNAL, *pbase, pnode).proc("insertreplace()--s(conditional?)");
				if (sec.IsScalar())
				{
					double val = sec.value();
					for (CTimeSeries* piece(psigBase), *index(&indsig); piece; piece = piece->chain, index = index->chain)
					{
						for (unsigned int k = 0; k < index->nSamples; k++)
							if (index->logbuf[k]) piece->buf[k] = val;
					}
				}
				else
				{ // RHS is conditional (can be replica)
					// At this point no need to worry about replacing null with non-null (i.e., signal is always non-null in the signal portions of sec. 
					//   4/13/2017
					for (CTimeSeries* p = &sec; p; p = p->chain)
					{
						int id = (int)(p->tmark * pbase->GetFs() / 1000 + .5);
						for (unsigned int k = 0; k < p->nSamples; k++)
							psigBase->buf[id + k] = p->buf[k];
					}
				}
			}
			else if ((p->alt && p->alt->type == N_TIME_EXTRACT) || // x{id}(t1~t2) = ...sqrt
				p->type == N_TIME_EXTRACT || (p->next && p->next->type == N_IDLIST))  // s(repl_RHS1~repl_RHS2)   or  cel{n}(repl_RHS1~repl_RHS2)
			{
				if (repl_RHS) //direct update of buf
				{
					id1 = (unsigned int)round(indsig.buf[0] * psigBase->GetFs() / 1000.);
					memcpy(psigBase->logbuf + id1 * psigBase->bufBlockSize, sec.buf, sec.nSamples * sec.bufBlockSize);
					if (psigBase->next)
						memcpy(psigBase->next->logbuf + id1 * psigBase->bufBlockSize, sec.next->buf, sec.nSamples * sec.bufBlockSize);
				}
				else
					psigBase->ReplaceBetweenTPs(sec, indsig.buf[0], indsig.buf[1]);
			}
			else if (pnode->alt->type == N_HOOK)
			{
				psigBase->ReplaceBetweenTPs(sec, indsig.buf[0], indsig.buf[1]);
			}
			else
			{
				ostringstream oss;
				oss << "TYPE=" << pnode->type << " insertreplace()--unexpected node type.";
				throw CAstException(INTERNAL, *pbase, pnode).proc(oss.str().c_str());
			}
		}
		else
		{
			// v(1:5) or v([contiguous]) = (any array) to replace
			// v(1:2:5) or v([non-contiguous]) = RHS; //LHS and RHS must match length.
			bool contig = pbase->isContiguous(indsig, id1, id2);
			if (repl_RHS) //direct update of buf
			{
				if (contig)
					memcpy(psigBase->logbuf + id1 * psigBase->bufBlockSize, sec.buf, sec.nSamples*sec.bufBlockSize);
				else
				{
					for (unsigned int k = 0; k < indsig.nSamples; k++)
					{
						memcpy(psigBase->logbuf + (int)indsig.buf[k] * psigBase->bufBlockSize, sec.buf + k * psigBase->bufBlockSize, psigBase->bufBlockSize);
					}
				}
			}
			else
			{
				if (contig)
				{
					psigBase->replacebyindex(id1 - 1, id2 - id1 + 1, sec);
				}
				else
				{
					if (sec.IsEmpty())
						throw CAstException(USAGE, *pbase, pnode).proc("For non-consecutive indexing, RHS cannot be empty.");
					if (sec.nSamples != 1 && sec.nSamples != indsig.nSamples)
						throw CAstException(USAGE, *pbase, pnode).proc("For non-consecutive indexing, the length of RHS and LHS must match.");
					vector<unsigned int> ids;
					ids.resize(indsig.nSamples);
					unsigned int k = 0;
					for_each(ids.begin(), ids.end(), [indsig, &k](unsigned int& v) { v = (unsigned int)indsig.buf[k++] - 1; });
					psigBase->replacebyindex(ids.begin(), ids.end(), sec);
				}
			}
		}
	}
}

CVar * CNodeProbe::TID_indexing(const AstNode* pnode, AstNode *pLHS, AstNode *pRHS)
{
	CVar isig, isigGhost;
	CVar *tsig = NULL;
	if (psigBase->type() & TYPEBIT_TEMPORAL && pLHS->child->next)
	{
		// 2-D indexing for audio, the first arg must be scalar 1 or two
		// Don't call eval_indexing(). Just Compute();
		isig = pbase->Compute(pLHS->child);
		pbase->checkScalar(pnode, isig, "First arg should be integer.");
		if (isig.value() != 1. && isig.value() != 2.)
			throw CAstException(USAGE, *pbase, pnode).proc("First arg should be either 1 (left chan) or 2 (right chan).");
	}
	else
		eval_indexing(pLHS->child, isig);
	//Check if index is within range 
	if (isig._max() > psigBase->nSamples)
		throw CAstException(RANGE, *pbase, pnode).proc("", varname.c_str(), (int)isig._max(), -1);

	// pRHS should be non-NULL
	tsig = pbase->Compute(pRHS);
	if (lhsref_single)
	{
		if (tsig->nSamples > 1)
			throw CAstException(USAGE, *pbase, pLHS).proc("LHS indicates a scalar; RHS is not.");
		*lhsref = tsig->value();
		return psigBase;
	}
	insertreplace(pLHS, *tsig, isig);
	if (isig.nSamples > 0)
	{
		ostringstream oss;
		oss << "(";
		if (isig.nSamples > 1)  oss << "[";
		if (isig.nSamples < 10)
			for_each(isig.buf, isig.buf + isig.nSamples - 1, [&oss](double v) {oss << v << ' '; });
		else
			oss << (int)isig.buf[0] << ' ' << (int)isig.buf[1] << " ... ";
		if (isig.nSamples > 1)  oss << (int)isig.buf[isig.nSamples - 1] << "]";
		oss << ")";
		varname += oss.str();
	}
	return psigBase;
}

CVar * CNodeProbe::TID_assign(const AstNode *pnode, AstNode *p, AstNode *pRHS)
{
	if (p->alt) // T-Y-2-1 
	{
		if (p->alt->type == N_CELL)
		{ // This applies to a{m} = RHS only. a{m}(n) = RHS  should be taken care of by TID_RHS2LHS::N_ARGS
			if (psigBase)
				*psigBase = pbase->Compute(pRHS); // Updating Vars by directly accessing the pointer attached to the current cell
			else // a{m} = RHS when a is undefined
			{
				ostringstream oss;
				oss << "Variable " << p->str << " not defined";
				throw CAstException(USAGE, *pbase, p).proc(oss.str().c_str());
			}
		}
		else if (p->alt->type == N_ARGS) // regular indexing
		{
			TID_indexing(pnode, p->alt, pRHS);
			if (pbase->pgo)
			{ // need to get the member name right before N_ARGS
				if (p->alt->type==N_ARGS)
					pbase->setgo.type = p->str;
				else
				{ // from previous, untracktable
					auto pp = pbase->findParentNode(pnode, p, true);
					pbase->setgo.type = pp->str;
				}
			}
		}
		else if ( (p->type == T_ID || p->type == N_STRUCT) && p->alt->type == N_STRUCT) // Definition of a struct member variable, but the struct var hasn't beend initialized.
		{ // Initialize the variable and define the member variable from the tip and go reverse
			CVar* psigRHS = pbase->Compute(pRHS);
			deque<string> strchain;
			deque<CVar> cvarchain;
			for (auto q = p->alt; q; q = q->alt)
			{
				strchain.push_back(q->str);
				cvarchain.push_back(CVar());
			}
			auto pvar = psigRHS;
			for (auto rit = cvarchain.end() ; !strchain.empty(); strchain.pop_back())
			{
				rit--;
				auto str = strchain.back();
				(*rit).strut[str] = pvar;
				pvar = &(*rit);
			}
			if (pnode==p)
				pbase->SetVar(p->str, pvar);
			else
			{
				// At this point, pbase must have pnode->str in Vars
				pbase->Vars[pnode->str].strut[p->str] = pvar;
			}
		}
		else
		{
			ostringstream oss;
			oss << "TYPE=" << p->type << " unknwon type in TID_assign()";
			throw CAstException(INTERNAL, *pbase, p).proc(oss.str().c_str());
		}
	}
	else// if (!p->alt) // T-Y-2-2
	{ // no indexing--assign RHS to LHS (if RHS is available) or retrieve LHS
		if (pRHS)
		{
			if (psigBase && psigBase->IsGO())
			{ //reject an attempt to modify unchangeable struts
				if (!strcmp(p->str,"children") || !strcmp(p->str, "parent") || !strcmp(p->str, "gcf") || !strcmp(p->str, "gca"))
					throw CAstException(USAGE, *pbase, p).proc("LHS is unmodifiable.");
			}
			// illegal if p-str is a built-in function name
			if (p->str[0] == '#' || pbase->IsValidBuiltin(p->str))
				throw CAstException(USAGE, *pbase, p).proc("LHS must be l-value; cannot be a built-in function");
			CVar *psigRHS = pbase->Compute(pRHS);
			//This is where an existing variable is changed, upated or replaced by indices.
			if (p->type == N_VECTOR) 
			{
				// if lhs is null, that means outputbinding was done at PrepareAndCallUDF
				if (pbase->lhs)
				{
					pbase->outputbinding(pbase->lhs);
					pbase->Sigs.clear();
					pbase->lhs = nullptr;
				}
			}
			else if (p == pnode)
			{	//top-level: SetVar
				if (p->suppress != -1) // for the case of (recorder).start--we should keep RHS from affecting the LHS
				{
					if (psigRHS->IsGO() && psigRHS->GetFs()!=3)
						pbase->SetVar(p->str, pbase->pgo);
					else
						pbase->SetVar(p->str, psigRHS);
				}
			}
			else
			{ // p->type should be N_STRUCT
				//if a member item p->str is available, psigBase is the pointer to that item
				//if the item is not available, psigBase is the pointer to the base
				if (psigBase->IsStruct())
				{// a.var_not_existing = RHS --> existing Sig is ignored and a new one comes in from Compute(pRHS)
					if (psigRHS->GetFs()!=3)
					{
						if (psigRHS->IsGO())
							psigBase->struts[p->str].push_back(pbase->pgo);
						else
						{
							psigBase->strut[p->str] = psigRHS;
							varname += string(".") + p->str;
						}
					}
					else
						psigBase->struts[pnode->str].push_back(psigRHS); // check
				}
				else
				{
					if (psigRHS->IsGO()) // If the base sig already has corresponding struts, it is a matter of replacing the content, but if the existing member is strut, it is complicated... 9/6/2018
						throw CAstException(USAGE, *pbase, p).proc("You are trying to update a GO member variable. This cannot be done due to a known bug. Clear the member variable and try again. Will be fixed someday. bj kwon 9/6/2018.");
					else
					{	//if psigBase is not strut yet, make it now
						psigBase->strut[p->str] = psigRHS;
						varname += string(".") + p->str;
					}
					if (psigBase->GetFs() == 3) // without this, after gos(1:2).color = [1 1 1], LHS becomes non GO.
						pbase->Sig = *psigBase;
				}
				if (pbase->pgo) 
					pbase->setgo.type = p->str;
			}
		}
		else
		{
			throw CAstException(USAGE, *pbase, p).proc("Not expecting this.. 2.");
		}
	}
	return psigBase;
}

CVar * CNodeProbe::TID_time_extract(const AstNode *pnode, AstNode *p, AstNode *pRHS)
{
	if (pRHS)
	{
		vector<double> tpoints = pbase->gettimepoints(pnode, p->child);
		if (pbase->searchtree(pRHS, T_REPLICA))
			pbase->replica = TimeExtract(pnode, p->child);
		CVar tsig = pbase->Compute(pRHS); // rhs
		if (tsig.GetType() != CSIG_AUDIO && !tsig.IsEmpty())
			throw CAstException(USAGE, *pbase, p).proc("Referencing timepoint(s) in an audio variable requires another audio signal on the RHS.");
		CVar isig(1);
		isig.UpdateBuffer(2);
		isig.buf[0] = tpoints[0];
		isig.buf[1] = tpoints[1];
		insertreplace(pnode, tsig, isig);
		ostringstream oss;
		oss << "(" << tpoints[0] << '~' << tpoints[1] << ")";
		varname += oss.str();
		return psigBase;
	}
	else
		return TimeExtract(pnode, p->child);
}

CVar * CNodeProbe::TimeExtract(const AstNode *pnode, AstNode *p)
{
	if (!psigBase)
		throw CAstException(INTERNAL,*pbase, pnode).proc("TimeExtract(): null psigBase");
	pbase->checkAudioSig(pnode, *psigBase);

	CTimeSeries *pts = psigBase;
	for (; pts; pts = pts->chain)
		pbase->endpoint = pts->CSignal::endt();
	if (psigBase->next)
	{
		pts = psigBase->next;
		pbase->endpoint = max(pbase->endpoint, pts->CSignal::endt());
	}
	vector<double> tpoints = pbase->gettimepoints(pnode, p);
	CVar out(*psigBase);
	out.Crop(tpoints[0], tpoints[1]);
	return &(pbase->Sig = out);
}

bool CAstSig::builtin_func_call(CNodeProbe &diggy, AstNode *p)
{
	/* p->type :
	N_STRUCT --> struct call --> pvar must be ready
	T_ID --> can be both struct call or not --> let's make sure it is non-struct call
	*/
	if (p->type == T_ID || p->type == N_STRUCT)
	{
		if (p->str[0] == '$' || IsValidBuiltin(p->str)) // hook bypasses IsValidBuiltin
		{
			// if diggy.pnode->child is present, it generally means that RHS exists and should be rejected if LHS is a function call
			// The exception is if LHS has alt node, because it will go down through the next node and it may end up a non-function call.
			if (diggy.root->child)
				if (!diggy.root->alt)
					throw CAstException(USAGE, *this, diggy.root).proc("LHS must be an l-value. Isn't it a built-in function?");
			HandleAuxFunctions(p, diggy.root);
			// while pgo is active, psigBase should point to pgo, not &Sig
			// this causes a crash on the line Sig = *diggy.psigBase in read_node() in AstSig.cpp   4/7/2019
			//
			// pgo changed to diggy.level.pgo because in ax.x.lim = [0 getfs/2] when p is getfs pgo should be cleared, but diggy.pbase (which is pgo) should be untouched.
			// 10/10/2019
//			diggy.psigBase = pgo ? pgo : &Sig;
			return true;
		}
	}
	return false;
}

// To Do.....

char *CAstSig::showGraffyHandle(char *outbuf, CVar *psig)
{ // psig must be a single GO
	if (psig->GetFs() == 2)
		sprintf(outbuf, " \"%s\" ", psig->string().c_str());
	else
		sprintf(outbuf, "%.0lf ", psig->value());
	return outbuf;
}

CNodeProbe::CNodeProbe(CAstSig *past, AstNode *pnode, CVar *psig)
{
	psigBase = NULL;
	pbase = past;
	root = pnode; //what's this?
	psigBase = psig; // NULL except for T_REPLICA
	lhsref_single = false;
	lhsref = NULL;
}

CVar &CNodeProbe::ExtractByIndex(const AstNode *pnode, AstNode *p)
{ // pnode->type should be N_ARGS
	CVar tsig, isig;
	if (!p->child)	throw CAstException(USAGE, *pbase, pnode).proc("A variable index should be provided.");
	if (p->child->type == T_FULLRANGE && (pbase->Sig.type() & TYPEBIT_TEMPORAL))
	{
		// nothing 
	}
	else
	{
		eval_indexing(p->child, isig);
		if (!(isig.type() & 1)) // has more than one element. 
			lhsref_single = false;
		if (isig._max() > pbase->Sig.nSamples) // can be replaced with psigBase->nSamples
			throw CAstException(RANGE, *pbase, pnode).proc("", varname.c_str(), (int)isig._max(), -1);
		pbase->Sig = extract(pnode, isig);
	}
	return pbase->Sig;
}

CVar * CNodeProbe::extract(const AstNode *pnode, CTimeSeries &isig)
{
	CSignals out((psigBase)->GetFs());
	CTimeSeries *p = &out;
	out.UpdateBuffer(isig.nSamples);
	if (isig._min() <= 0.)
	{
		ostringstream outstream;
		outstream << "Invalid index " << "for " << varname << " : " << (int)isig._min() << " (must be positive)";
		throw CAstException(USAGE, *pbase, pnode).proc(outstream.str().c_str());
	}
	if (psigBase->IsComplex())
	{
		out.SetComplex();
		for (unsigned int i = 0; i < isig.nSamples; i++)
			out.cbuf[i] = (psigBase)->cbuf[(int)isig.buf[i] - 1];
	}
	else if (psigBase->IsLogical())
	{
		out.MakeLogical();
		for (unsigned int i = 0; i < isig.nSamples; i++)
			out.logbuf[i] = (psigBase)->logbuf[(int)isig.buf[i] - 1];
	}
	else if (psigBase->IsString())
	{
		out.UpdateBuffer(isig.nSamples + 1); // make room for null 
		for (unsigned int i = 0; i < isig.nSamples; i++)
			out.strbuf[i] = (psigBase)->strbuf[(int)isig.buf[i] - 1];
		out.strbuf[out.nSamples - 1] = 0;
	}
	else
	{
		if (psigBase->IsGO())
		{
			if (isig._max() > psigBase->nSamples)
				throw CAstException(RANGE, *pbase, pnode).proc("", "", (int)isig._max());
			if (isig.nSamples == 1)
			{
				size_t did = (size_t)isig.value() - 1;
				if (!psigBase->buf[did]) {
					// if Reset() resets a GO into a NULL, the next two lines are not necessary 10/1/2020
					psigBase->strut.clear();
					psigBase->struts.clear();
					psigBase->Reset(1);
				}
				else
				{
					CVar* tp = (CVar*)(INT_PTR)(psigBase)->buf[did];
					pbase->pgo = psigBase = tp;
					pbase->Sig = *psigBase;
				}
				return psigBase;
			}
			else
			{
				vector<INT_PTR> gos;
				for (unsigned int k = 0; k < isig.nSamples; k++)
				{
					size_t did = (size_t)isig.buf[k] - 1;
					CVar *tp = (CVar*)(INT_PTR)(psigBase)->buf[did];
					gos.push_back((INT_PTR)tp);
				}
				psigBase = pbase->MakeGOContainer(gos);
				pbase->Sig = *psigBase;
				return psigBase;
			}
		}
		out.SetReal();
		if (pbase->Sig.type() & TYPEBIT_AUDIO)
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
				id = (int)_pisig->buf[0] - 1;
				p->tmark = (double)id / pbase->Sig.GetFs() * 1000.;
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
					p->buf[cum++] = psigBase->buf[id];
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
		else if (pbase->Sig.type() >= TYPEBIT_GO)
			throw CAstException(USAGE, *pbase, pnode).proc("Invalid object type to extract based on index.");
		else // should be non-audio, non-time sequence data such as vector
		{
			int id = 0;
			for (unsigned int k = 0; k < isig.nSamples; k++)
			{
				auto ind = (unsigned int)isig.buf[k];
				out.buf[id++] = (psigBase)->buf[ind - 1];
			}
		}
	}
	out.nGroups = isig.nGroups;
	pbase->Sig = out;
	return &pbase->Sig;
}

CVar &CNodeProbe::eval_indexing(const AstNode *pInd, CVar &isig)
{
	// input: pInd, psigBase
	// output: isig -- sig holding all indices

	// process the first index
	unsigned int len;
	pbase->prepare_endpoint(pInd, psigBase);
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
			if (psigBase->nGroups > 1 && isig.nSamples > 1)
				isig.nGroups = isig.nSamples;
			AstNode *p = pInd->next;
			CVar isig2;
			if (p->type == T_FULLRANGE)
			{// x(ids,:)
				len = psigBase->Len();
				isig2.UpdateBuffer(len);
				for (unsigned int k = 0; k < len; k++)	isig2.buf[k] = k + 1;
			}
			else // x(ids1,ids2)
			{
				//endpoint for the second arg in 2D is determined here.
				tp.endpoint = (double)psigBase->Len();
				isig2 = tp.Compute(p);
			}
			auto mx = isig2._max();
			auto nx = psigBase->Len();
			if (isig2.IsLogical()) pbase->index_array_satisfying_condition(isig2);
			else if (isig2._max() > (double)psigBase->Len())
				throw CAstException(RANGE, *pbase, pInd).proc("2nd index ", "", psigBase->Len(), (int)isig2._max());
			pbase->interweave_indices(isig, isig2, psigBase->Len());
		}
	}
	catch (const CAstException &e) {
		throw CAstException(USAGE, *pbase, pInd).proc(e.getErrMsg().c_str());
	}
	return isig;
}

CVar * CNodeProbe::TID_condition(const AstNode *pnode, AstNode *pLHS, AstNode *pRHS)
{
	unsigned int id = 0;
	CVar isig;
	eval_indexing(pLHS, isig);
	CVar * prhs = pbase->Compute(pRHS);
	if (prhs->IsScalar())
	{
		//go through chains 6/24/2019
		CTimeSeries *p = psigBase;
		CTimeSeries *pisig = &isig;
		while (p)
		{
			for (unsigned k = 0; k < pisig->nSamples; k++)
				p->buf[(int)pisig->buf[k] - 1] = prhs->value();
			p = p->chain;
			pisig = pisig->chain;
		}
	}
	else if (pbase->searchtree(pRHS, T_REPLICA))
	{
		//go through chains 6/24/2019
		CTimeSeries *p = psigBase;
		CTimeSeries *pisig = &isig;
		while (p)
		{
			if (p->IsTimeSignal()) // Check this part 9/4/2018
			{
				// consolidate chained rhs with lhs
				for (CTimeSeries *cts = prhs; cts; cts = cts->chain)
				{
					// id translated from tmark for each chain
					int fs = cts->GetFs();
					id = (unsigned int)(cts->tmark / 1000. * fs + .5);
					memcpy(p->buf + id, cts->buf, cts->nSamples * sizeof(double));
				}
			}
			else
			{
				for (unsigned k = 0; k < pisig->nSamples; k++)
					p->buf[(int)pisig->buf[k] - 1] = prhs->buf[k];
			}
			p = p->chain;
			if (pisig) pisig = pisig->chain;
			else break;
		}
	}
	else if (prhs->IsEmpty())
	{
		//do we need to go through chains??? 6/24/2019
		insertreplace(pLHS, *prhs, isig);
		pbase->Sig.Reset();
	}
	else
		throw CAstException(USAGE, *pbase, pRHS).proc("Invalid RHS; For LHS with conditional indexing, RHS should be either a scalar, empty, or .. (self).");
	pbase->SetVar(pnode->str, psigBase);
	if (isig.nSamples > 0)
	{
		varname = pbase->Script;
	}
	return &pbase->Sig;
}

CVar * CNodeProbe::TID_RHS2LHS(const AstNode *pnode, AstNode *pLHS, AstNode *pRHS)
{ // Computes pRHS, if available, and assign it to LHS.
	// 2 cases where psig is not Sig: first, a(2), second, a.sqrt

	// pnode: base ptree
	// p: LHS; same as pnode for top-level LHS
	// pRHS: RHS
	// psig: same as psigBase; NULL if LHS is not yet realized into an object        
	// 
	// TID_assign handles
	// a.val = (RHS) --> pnode: a, p: val, psigBase: pointer to a (directly from the pbase Vars)
	//
	// TID_indexing handles
	// a.val(3) = (RHS) --> pnode: a, p: val, psigBase: pointer to a.val

	if (pbase->IsConditional(pLHS))
		return TID_condition(pnode, pLHS, pRHS);
	switch (pLHS->type)
	{
	case N_ARGS:
		TID_indexing(pnode, pLHS, pRHS);
		if (pbase->pgo)
		{ // need to get the member name right before N_ARGS
			auto pp = pbase->findParentNode(pnode, pLHS, true);
			pbase->setgo.type = pp->str;
		}
		break;
	case T_ID: // T-Y-2
	case N_STRUCT:
	case N_CALL:
	case N_MATRIX:
	case N_VECTOR:
		TID_assign(pnode, pLHS, pRHS);
		break;
	case N_TIME_EXTRACT: // T-Y-4 
		TID_time_extract(pnode, pLHS, pRHS);
		break;
	case N_HOOK:
		if (pbase->pEnv->pseudo_vars.find(pnode->str) == pbase->pEnv->pseudo_vars.end())
		{
			TID_assign(pnode, pLHS, pRHS);
		}
		else
			throw CAstException(USAGE, *pbase, pLHS).proc("Pseudo variable cannot be modified.");
		break;
	default:  // T-Y-5
		return pbase->Compute(pRHS); // check... this may not happen.. if so, inspect.
	}
	return &pbase->Sig;
}

/* These two replace() member functions could be part of body in csignals.cpp for the point of logic,
but better to keep here because of exception handling (i.e., need pnode)
*/
CTimeSeries &CNodeProbe::replace(const AstNode *pnode, CTimeSeries *pobj, body &sec, body &index)
{
	//	this is to be used when items are replaced without changing the size.
	// except when sec is empty... in which case the "index"'ed items are deleted.
	// in that case index is assumed to be sorted ascending
	if (index.nSamples == 0) return *pobj;
	if (sec.bufBlockSize != pobj->bufBlockSize)
	{
		if (pobj->bufBlockSize == 1 && sec.bufBlockSize != 1) sec.MakeLogical();
		else if (pobj->bufBlockSize == 8 && sec.bufBlockSize == 1) sec.SetReal();
		else if (pobj->bufBlockSize == 8 && sec.bufBlockSize == 16) pobj->SetComplex();
		else if (pobj->bufBlockSize == 16 && sec.bufBlockSize == 1) sec.SetComplex();
		else if (pobj->bufBlockSize == 16 && sec.bufBlockSize == 8) sec.SetComplex();
	}
	for (unsigned int k = 0; k < index.nSamples; k++)
	{
		if (index.buf[k] < 1.)
			throw CAstException(RANGE, *pbase, pnode).proc("must be equal or greater than 1", "", (int)index.buf[k]);
		unsigned int id = (int)index.buf[k];
		if (id - index.buf[k] > .25 || index.buf[k] - id > .25)
			throw CAstException(RANGE, *pbase, pnode).proc("must be integer", "", id);
		if (id > pobj->nSamples)
			throw CAstException(RANGE, *pbase, pnode).proc("", "", id);
	}
	if (sec.nSamples == 0)
	{
		int trace = (int)(index.buf[0] - 1);
		for (unsigned int k = 0; k < index.nSamples; k++)
		{
			unsigned int diff = (unsigned int)(k < index.nSamples - 1 ? index.buf[k + 1] - index.buf[k] : pobj->nSamples + 1 - index.buf[k]);
			diff--;
			memcpy(pobj->logbuf + trace * pobj->bufBlockSize, pobj->logbuf + (int)index.buf[k] * pobj->bufBlockSize, diff*pobj->bufBlockSize);
			trace += diff;
		}
		pobj->nSamples -= index.nSamples;
	}
	else
		for (unsigned int k = 0; k < index.nSamples; k++)
		{
			if (index.buf[k] < 1.)
				throw CAstException(RANGE, *pbase, pnode).proc("must be equal or greater than 1", "", (int)index.buf[k]);
			unsigned int id = (unsigned int)(index.buf[k]);
			if (id - index.buf[k] > .05 || index.buf[k] - id > .05)
				throw CAstException(RANGE, *pbase, pnode).proc("must be integer", "", id);
			if (id > pobj->nSamples)
				throw CAstException(RANGE, *pbase, pnode).proc("", "", id);
			id--; // zero-based
			if (sec.nSamples == 1) // items from id1 to id2 are to be replaced with sec.value()
				memcpy(pobj->logbuf + id * pobj->bufBlockSize, sec.logbuf, pobj->bufBlockSize);
			else
				memcpy(pobj->logbuf + id * pobj->bufBlockSize, sec.logbuf + k * pobj->bufBlockSize, pobj->bufBlockSize);
		}
	return *pobj;
}

CTimeSeries &CNodeProbe::replace(const AstNode *pnode, CTimeSeries *pobj, CSignal &sec, int id1, int id2)
{ // this replaces the data body between id1 and id2 (including edges) with sec
	if (id1 < 0) throw CAstException(RANGE, *pbase, pnode).proc("replace index cannot be negative.", "", id1);
	if (id2 < 0) throw CAstException(RANGE, *pbase, pnode).proc("replace index cannot be negative.", "", id2);
	if (sec.bufBlockSize != pobj->bufBlockSize)
	{
		if (pobj->bufBlockSize == 1 && sec.bufBlockSize != 1) sec.MakeLogical();
		else if (pobj->bufBlockSize == 8 && sec.bufBlockSize == 1) sec.SetReal();
		else if (pobj->bufBlockSize == 8 && sec.bufBlockSize == 16) pobj->SetComplex();
		else if (pobj->bufBlockSize == 16 && sec.bufBlockSize == 1) sec.SetComplex();
		else if (pobj->bufBlockSize == 16 && sec.bufBlockSize == 8) sec.SetComplex();
	}
	//id1 and id2 are zero-based here.
	if (id1 > (int)pobj->nSamples - 1) throw CAstException(RANGE, *pbase, pnode).proc("replace index1 ", "", id1);
	if (id2 > (int)pobj->nSamples - 1) throw CAstException(RANGE, *pbase, pnode).proc("replace index2 ", "", id1);
	unsigned int secnsamples = sec.nSamples;
	if (sec.type() & TYPEBIT_STRING) secnsamples--;
	if (secnsamples == 1) // no change in length--items from id1 to id2 are to be replaced with sec.value()
	{
		if (sec.type() & TYPEBIT_LOGICAL)
			for (int k = id1; k <= id2; k++) pobj->strbuf[k] = sec.strbuf[0];
		else if (((CSignal *)&sec)->bufBlockSize == 16)
			for (int k = id1; k <= id2; k++) pobj->cbuf[k] = sec.cvalue();
		else
			for (int k = id1; k <= id2; k++) pobj->buf[k] = sec.value();
	}
	else
	{
		unsigned int nAdd = secnsamples;
		unsigned int nSubtr = id2 - id1 + 1;
		unsigned int newLen = pobj->nSamples + nAdd - nSubtr;
		unsigned int nToMove = pobj->nSamples - id2 - 1;
		if (nAdd > nSubtr) pobj->UpdateBuffer(newLen);
		bool *temp;
		if (nAdd != nSubtr)
		{
			temp = new bool[nToMove*pobj->bufBlockSize];
			memcpy(temp, pobj->logbuf + (id2 + 1)*pobj->bufBlockSize, nToMove*pobj->bufBlockSize);
		}
		memcpy(pobj->logbuf + id1 * pobj->bufBlockSize, sec.buf, secnsamples*pobj->bufBlockSize);
		if (nAdd != nSubtr)
		{
			memcpy(pobj->logbuf + (id1 + secnsamples)*pobj->bufBlockSize, temp, nToMove*pobj->bufBlockSize);
			delete[] temp;
		}
		pobj->nSamples = newLen;
	}
	return *pobj;
}
