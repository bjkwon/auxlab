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
	else if (indsig.IsScalar())
	{
		if (psigBase->GetType() == CSIG_AUDIO)
		{
			if (sec.IsTimeSignal())		// s(repl_RHS) = sound; //insert
				psigBase->Insert(indsig.value(), sec);
			else
			{
				unsigned int id = (unsigned int)round(indsig.value());
				pbase->checkindexrange(pnode, psigBase, id, "LHS##vsdba2");
				if (sec.IsComplex())
				{
					psigBase->SetComplex();
					psigBase->cbuf[id - 1] = sec.cvalue();
				}
				else
					psigBase->buf[id - 1] = sec.value();
			}
		}
		else
		{
			unsigned int id = (unsigned int)round(indsig.value());
			pbase->checkindexrange(pnode, psigBase, id, "LHS");
			if (sec.IsComplex())
			{
				psigBase->SetComplex();
				psigBase->cbuf[id - 1] = sec.cvalue();
			}
			else
				replace(pnode, psigBase, sec, id - 1, id - 1);
		}
	}
	else // not done yet if sec is complex
	{
		//		pnode->type is either N_IXASSIGN (for non-cell LHS)
		//		or cell index for cell LHS,  for example, T_NUMBER for cc{3}
		unsigned int id1, id2;
		AstNode *repl_RHS = pbase->searchtree(pnode->child, T_REPLICA);
		if (psigBase->GetType() == CSIG_AUDIO)
		{
			if (pnode->type == N_ARGS || (pnode->next && pnode->next->type == N_CALL))
			{
				// CAstSig::replace(const AstNode *pnode, CTimeSeries *pobj, body &sec, int id1, int id2)
				// actually works efficiently (without blindly adding to the buffer) when there's no change in nSamples
				// so I'm not doing if (repl_RHS) here.
				// for lines below, it just looks long, but having if (repl_RHS) won't do any harm (maybe tiny bit faster)
				// so I'm not getting rid of it below. 
				// Buf probably I really need to check logical cases instead!!!
				// 11/7/2019
				if (indsig.IsLogical()) // s(conditional_var)
				{
					for (CTimeSeries *p = &sec; p; p = p->chain)
					{
						int id((int)(p->tmark*sec.GetFs() / 1000 + .5));
						for (unsigned int k = 0; k < p->nSamples; k++)
							if (indsig.logbuf[id + k])
								psigBase->buf[id + k] = p->buf[(int)k];
					}
				}
				else // s(id1:id2) or cel{n}(id1:id2)
				{ // x(1200:1201) = zeros(111) FAILED HERE	on memcpy line.... that's wrong. 1/20/2018
					// this must be contiguous
					if (!pbase->isContiguous(indsig, id1, id2))
						replace(pnode, psigBase, sec, indsig); // if sec is a scalar, just assignment call by index 
					else
					{
						pbase->checkindexrange(pnode, psigBase, id1, "LHS");
						pbase->checkindexrange(pnode, psigBase, id2, "LHS");
						if (sec.nSamples != id2 - id1 + 1)
						{
							if (!sec.IsScalar())
								throw pbase->ExceptionMsg(pnode, "to manipulate an audio signal by indices, LHS and RHS must be the same length or RHS is a scalar");
						}
						if (sec.IsComplex() && !psigBase->IsComplex()) psigBase->SetComplex();
						replace(pnode, psigBase, sec, id1 - 1, id2 - 1);
					}
				}
			}
			else if (!pnode->next && !pnode->str) // if s(conditional) is on the LHS, the RHS must be either a scalar, or the replica, i.e., s(conditional)
			{
				if (!repl_RHS && !indsig.IsLogical()) throw pbase->ExceptionMsg(pnode, "Internal logic error (insertreplace:0)--s(conditional?).");
				if (sec.IsScalar())
				{
					double val = sec.value();
					for (CTimeSeries *piece(psigBase), *index(&indsig); piece; piece = piece->chain, index = index->chain)
					{
						for (unsigned int k = 0; k < index->nSamples; k++)
							if (index->logbuf[k]) piece->buf[k] = val;
					}
				}
				else
				{ // RHS is conditional (can be replica)
				  // At this point no need to worry about replacing null with non-null (i.e., signal is always non-null in the signal portions of sec. 
				  //   4/13/2017
					for (CTimeSeries *p = &sec; p; p = p->chain)
					{
						int id = (int)(p->tmark * pbase->GetFs() / 1000 + .5);
						for (unsigned int k = 0; k < p->nSamples; k++)
							psigBase->buf[id + k] = p->buf[k];
					}
				}
			}
			else if (p->type == N_TIME_EXTRACT || (p->next && p->next->type == N_IDLIST))  // s(repl_RHS1~repl_RHS2)   or  cel{n}(repl_RHS1~repl_RHS2)
			{
				if (repl_RHS) //direct update of buf
				{
					id1 = (unsigned int)round(indsig.buf[0] * psigBase->GetFs() / 1000.);
					memcpy(psigBase->logbuf + id1 * psigBase->bufBlockSize, sec.buf, sec.nSamples*sec.bufBlockSize);
				}
				else
					psigBase->ReplaceBetweenTPs(sec, indsig.buf[0], indsig.buf[1]);
			}
			else if (pnode->alt->type == N_HOOK)
			{
				psigBase->ReplaceBetweenTPs(sec, indsig.buf[0], indsig.buf[1]);
			}
			else
				throw pbase->ExceptionMsg(pnode, "Internal logic error (insertreplace:1) --unexpected node type.");
		}
		else
		{
			// v(1:5) or v([contiguous]) = (any array) to replace
			// v(1:2:5) or v([non-contiguous]) = RHS; //LHS and RHS must match length.
			bool contig = pbase->isContiguous(indsig, id1, id2);
			if (!sec.IsEmpty() && !contig && sec.nSamples != 1 && sec.nSamples != indsig.nSamples) throw pbase->ExceptionMsg(pnode, "the number of replaced items must be the same as that of replacing items.");

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
					replace(pnode, psigBase, sec, id1 - 1, id2 - 1);
				else
					replace(pnode, psigBase, sec, indsig);
			}
		}
	}
}

CVar * CNodeProbe::TID_indexing(AstNode *pLHS, AstNode *pRHS, CVar *psig)
{
	CVar isig, isigGhost;
	CVar *tsig = NULL;
	eval_indexing(pLHS->child, isig);
	if (pRHS)
	{
		tsig = pbase->Compute(pRHS);
		if (lhsref_single)
		{
			if (tsig->nSamples > 1)
				throw pbase->ExceptionMsg(pLHS, "LHS indicates a scalar; RHS is not.");
			*lhsref = tsig->value();
			return psigBase;
		}
		//This is where elements of an existing array are changed, upated or replaced by indices.
		//if tsig is chainged, call insertreplace for each chain with a moving ghost of isig
		isigGhost <= isig;
		for (CVar *p = tsig; p; p = (CVar*)p->chain)
		{
			int lastIndex = isigGhost.nSamples = p->nSamples;
			insertreplace(pLHS, *p, isigGhost);
			isigGhost.buf += lastIndex;
		}
	}
	else
		insertreplace(pLHS, *psig, isig);
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

CVar * CNodeProbe::TID_tag(const AstNode *pnode, AstNode *p, AstNode *pRHS, CVar *psig)
{
	CVar isig;
	if (p->alt) // T-Y-2-1 
	{
		if (p->alt->type == N_CELL)
		{ // This applies to a{m} = RHS only. a{m}(n) = RHS  should be taken care of by TID_RHS2LHS::N_ARGS
			*psigBase = pbase->Compute(pRHS); // Updating Vars by directly accessing the pointer attached to the current cell
		}
		else if (p->alt->type == N_ARGS) // regular indexing
		{
			TID_indexing(p->alt, pRHS, psig);
			if (pbase->pgo)
			{ // need to get the member name right before N_ARGS
				AstNode *pp = pbase->findParentNode((AstNode*)pnode, p, true);
				pbase->setgo.type = pp->str;
			}
		}
		else 
			throw pbase->ExceptionMsg(p, "Unknown error.");
	}
	else// if (!p->alt) // T-Y-2-2
	{ // no indexing--assign RHS to LHS (if RHS is available) or retrieve LHS
		if (pRHS)
		{
			if (psigBase && psigBase->IsGO())
			{ //reject an attempt to modify unchangeable struts
				if (!strcmp(p->str,"children") || !strcmp(p->str, "parent") || !strcmp(p->str, "gcf") || !strcmp(p->str, "gca"))
					throw pbase->ExceptionMsg(p, "LHS is unmodifiable.");
			}
			// illegal if p-str is a built-in function name
			if (p->str[0] == '#' || pbase->IsValidBuiltin(p->str))
				throw pbase->ExceptionMsg(p, "LHS must be l-value; cannot be a built-in function");
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
					if (psigRHS->IsGO())
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
						throw pbase->ExceptionMsg(p, "You are trying to update a GO member variable. This cannot be done due to a known bug. Clear the member variable and try again. Will be fixed someday. bj kwon 9/6/2018.");
					else
						//if psigBase is not strut yet, make it now
					{
						psigBase->strut[p->str] = psigRHS;
						varname += string(".") + p->str;
					}
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
				return &pbase->Sig;
			if (psig)
				return psig;
			else
				return &pbase->Sig;
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
			throw pbase->ExceptionMsg(p, "Referencing timepoint(s) in an audio variable requires another audio signal on the RHS.");
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
	char estr[256];
	if (!psigBase)
	{
		strcat(estr, " for time extraction ~");
		throw pbase->ExceptionMsg(pnode, estr);
	}
	pbase->checkAudioSig(pnode, *psigBase);

	CTimeSeries *pts = psigBase;
	for (; pts; pts = pts->chain)
		pbase->endpoint = pts->CSignal::endt().front();
	if (psigBase->next)
	{
		pts = psigBase->next;
		pbase->endpoint = max(pbase->endpoint, pts->CSignal::endt().front());
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
			// if diggy.root->child is present, it generally means that RHS exists and should be rejected if LHS is a function call
			// The exception is if LHS has alt node, because it will go down through the next node and it may end up a non-function call.
			if (diggy.root->child) 
				if (!diggy.root->alt)
					throw ExceptionMsg(diggy.root, "LHS must be an l-value. Isn't it a built-in function?");
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
	root = pnode;
	psigBase = psig; // NULL except for T_REPLICA
	lhsref_single = false;
	lhsref = NULL;
}

CVar &CNodeProbe::ExtractByIndex(const AstNode *pnode, AstNode *p)
{ // pnode->type should be N_ARGS
	ostringstream ostream;
	CVar tsig, isig, isig2;
	if (!p->child)	throw pbase->ExceptionMsg(pnode, "A variable index should be provided.");
	eval_indexing(p->child, isig);
	if (isig._max().front() > pbase->Sig.nSamples)
		throw CAstExceptionRange(pbase, pnode, "", varname.c_str(), (int)isig._max().front());
	pbase->Sig = extract(pnode, isig);
	return pbase->Sig;
}

CVar * CNodeProbe::extract(const AstNode *pnode, CTimeSeries &isig)
{
	// Improve--don't use out. Just return psigBase. 11/11/2019

	//pinout comes with input and makes the output
	//At the end, Sig is updated with pinout
	//pinout always has the same or smaller buffer size, so let's not worry about calling UpdateBuffer() here
	//Instead of throw an exception, errstr is copy'ed. Return value should be ignored.
	// For success, errstr stays empty.
	CSignals out((psigBase)->GetFs());
	CTimeSeries *p = &out;
	ostringstream outstream;
	out.UpdateBuffer(isig.nSamples);
	//CSignal::Min() makes a vector
	//body::Min() makes a scalar.
	if (isig._min().front() <= 0.)
	{
		outstream << "Invalid index " << "for " << varname << " : " << (int)isig._min().front() << " (must be positive)";
		throw pbase->ExceptionMsg(pnode, outstream.str().c_str());
	}
	if ((psigBase)->IsComplex())
	{
		out.SetComplex();
		for (unsigned int i = 0; i < isig.nSamples; i++)
			out.cbuf[i] = (psigBase)->cbuf[(int)isig.buf[i] - 1];
	}
	else if ((psigBase)->IsLogical())
	{
		out.MakeLogical();
		for (unsigned int i = 0; i < isig.nSamples; i++)
			out.logbuf[i] = (psigBase)->logbuf[(int)isig.buf[i] - 1];
	}
	else if ((psigBase)->IsString())
	{
		out.UpdateBuffer(isig.nSamples + 1); // make room for null 
		for (unsigned int i = 0; i < isig.nSamples; i++)
			out.strbuf[i] = (psigBase)->strbuf[(int)isig.buf[i] - 1];
		out.strbuf[out.nSamples - 1] = 0;
	}
	else
	{
		if ((psigBase)->IsGO())
		{
			if (isig._max().front() > (psigBase)->nSamples)
			{
				outstream << "Index out of range: " << (int)isig._max().front();
				throw pbase->ExceptionMsg(pnode, outstream.str().c_str());
			}
			if (isig.nSamples == 1)
			{
				size_t did = (size_t)isig.value() - 1;
				CVar *tp = (CVar*)(INT_PTR)(psigBase)->buf[did];
				pbase->pgo = psigBase = tp;
				pbase->Sig = *psigBase;
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
		if (pbase->Sig.GetType() == CSIG_VECTOR)
		{
			int id(0);
			for (unsigned int k = 0; k < isig.nSamples; k++)
				out.buf[id++] = (psigBase)->buf[(int)isig.buf[k] - 1];
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
			char buf[128];
			if (isig2.IsLogical()) pbase->index_array_satisfying_condition(isig2);
			else if (isig2._max().front() > (double)psigBase->Len())
				throw pbase->ExceptionMsg(pInd, "Out of range: 2nd index ", itoa((int)isig2._max().front(), buf, 10));
			pbase->interweave_indices(isig, isig2, psigBase->Len());
		}
	}
	catch (const CAstException &e) {
		throw pbase->ExceptionMsg(pInd, e.getErrMsg().c_str());
	}
	return isig;
}

CVar * CNodeProbe::TID_condition(const AstNode *pnode, AstNode *pLHS, AstNode *pRHS, CVar *psig)
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
					id = (unsigned int)(cts->tmark / 1000. * cts->GetFs());
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
		throw pbase->ExceptionMsg(pRHS, "Invalid RHS; For LHS with conditional indexing, RHS should be either a scalar, empty, or .. (self).");
	pbase->SetVar(pnode->str, psigBase);
	if (isig.nSamples > 0)
	{
		varname = pbase->Script;
	}
	return &pbase->Sig;
}

CVar * CNodeProbe::TID_RHS2LHS(const AstNode *pnode, AstNode *pLHS, AstNode *pRHS, CVar *psig)
{ // Computes pRHS, if available, and assign it to LHS.
	// 2 cases where psig is not Sig: first, a(2), second, a.sqrt
	if (pbase->IsCondition(pLHS))
		return TID_condition(pnode, pLHS, pRHS, psig);
	switch (pLHS->type)
	{
	case N_ARGS:
		TID_indexing(pLHS, pRHS, psig);
		if (pbase->pgo)
		{ // need to get the member name right before N_ARGS
			AstNode *pp = pbase->findParentNode((AstNode*)pnode, pLHS, true);
			pbase->setgo.type = pp->str;
		}
		break;
	case T_ID: // T-Y-2
	case N_STRUCT:
	case N_CALL:
	case N_MATRIX:
	case N_VECTOR:
		TID_tag(pnode, pLHS, pRHS, psig);
		break;
	case N_TIME_EXTRACT: // T-Y-4 
		TID_time_extract(pnode, pLHS, pRHS);
		break;
	case N_HOOK:
		if (pbase->pEnv->pseudo_vars.find(pnode->str) == pbase->pEnv->pseudo_vars.end())
		{
			TID_tag(pnode, pLHS, pRHS, psig);
		}
		else
			throw pbase->ExceptionMsg(pLHS, "Pseudo variable cannot be modified.");
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
			throw pbase->ExceptionMsg(pnode, "index must be greater than 0");
		unsigned int id = (int)index.buf[k];
		if (id - index.buf[k] > .25 || index.buf[k] - id > .25)
			throw pbase->ExceptionMsg(pnode, "index must be integer");
		if (id > pobj->nSamples)
			throw pbase->ExceptionMsg(pnode, "replace index exceeds the range.");
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
				throw pbase->ExceptionMsg(pnode, "index must be greater than 0");
			unsigned int id = (unsigned int)(index.buf[k]);
			if (id - index.buf[k] > .05 || index.buf[k] - id > .05)
				throw pbase->ExceptionMsg(pnode, "index must be integer");
			if (id > pobj->nSamples)
				throw pbase->ExceptionMsg(pnode, "replace index exceeds the range.");
			id--; // zero-based
			if (sec.nSamples == 1) // items from id1 to id2 are to be replaced with sec.value()
				memcpy(pobj->logbuf + id * pobj->bufBlockSize, sec.logbuf, pobj->bufBlockSize);
			else
				memcpy(pobj->logbuf + id * pobj->bufBlockSize, sec.logbuf + k * pobj->bufBlockSize, pobj->bufBlockSize);
		}
	return *pobj;
}

CTimeSeries &CNodeProbe::replace(const AstNode *pnode, CTimeSeries *pobj, body &sec, int id1, int id2)
{ // this replaces the data body between id1 and id2 (including edges) with sec
	if (id1 < 0 || id2 < 0) throw pbase->ExceptionMsg(pnode, "replace index cannot be negative.");
	if (sec.bufBlockSize != pobj->bufBlockSize)
	{
		if (pobj->bufBlockSize == 1 && sec.bufBlockSize != 1) sec.MakeLogical();
		else if (pobj->bufBlockSize == 8 && sec.bufBlockSize == 1) sec.SetReal();
		else if (pobj->bufBlockSize == 8 && sec.bufBlockSize == 16) pobj->SetComplex();
		else if (pobj->bufBlockSize == 16 && sec.bufBlockSize == 1) sec.SetComplex();
		else if (pobj->bufBlockSize == 16 && sec.bufBlockSize == 8) sec.SetComplex();
	}
	//id1 and id2 are zero-based here.
	if (id1 > (int)pobj->nSamples - 1) throw pbase->ExceptionMsg(pnode, "replace index1 exceeds the range.");
	if (id2 > (int)pobj->nSamples - 1) throw pbase->ExceptionMsg(pnode, "replace index2 exceeds the range.");
	unsigned int secnsamples = sec.nSamples;
	bool ch = ((CSignal *)&sec)->bufBlockSize == 1;
	if (ch) secnsamples--;
	if (secnsamples == 1) // no change in length--items from id1 to id2 are to be replaced with sec.value()
	{
		if (ch)
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
