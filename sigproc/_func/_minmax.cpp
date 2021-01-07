#include "sigproc.h"

int countVectorItems(const AstNode* pnode); // support.cpp

void _minmax(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	if (past->Sig.IsEmpty()) return; //for empty input, empty output
	past->blockLogical(pnode, past->Sig, "max() or min()");
	past->blockString(pnode, past->Sig, "max() or min()");
	string fname = pnode->str;
	CVar sig = past->Sig;
	//we need tree pointer to the one calling the current statement
	auto ptree_thisline = CAstSig::goto_line(past->xtree, pnode->line);
	// To do...check the logic flow----12/29/2020
	// ptree_thisline should be one of the following:
	// 1) no child 
	// 2) child 
	// 3) N_BLOCK
	// 4) T_IF, T_FOR, T_WHILE, or T_SWITCH
	int nOutVars;
	if (pnode->type == N_STRUCT)
	{
		auto ss = CAstSig::findDadNode(ptree_thisline, pnode);
		auto ssp = CAstSig::findDadNode(ptree_thisline, ss);
		if (ssp->type == N_ARGS)
			nOutVars = 0;
		else if (ssp->type == N_VECTOR)
			nOutVars = countVectorItems(ssp);
		else
			throw "unchecked logic flow";
	}
	else
	{
		auto ss = CAstSig::findDadNode(ptree_thisline, pnode);
		if (ss->type == N_ARGS)
			nOutVars = 0;
		else if (ss->type == N_VECTOR)
			nOutVars = countVectorItems(ss);
		else
			throw "unchecked logic flow";
	}
	
	CVar* popt = NULL;
	if (nOutVars > 1)
		popt = new CVar(sig.GetFs());
	auto out = max_element(sig.strbuf, sig.strbuf + sig.nSamples);

	if (fname == "max") past->Sig = sig.fp_getval(&CSignal::_max, popt);
	else if (fname == "min") past->Sig = sig.fp_getval(&CSignal::_min, popt);
	if (nOutVars > 1)
	{
		past->Sigs.push_back(move(make_unique<CVar*>(&past->Sig)));
		unique_ptr<CVar*> pt = make_unique<CVar*>(popt); // popt carries maximum/minimum indices
		past->Sigs.push_back(move(pt));
	}
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}

// DO NOT CALL this function with empty buf
double body::_max(unsigned int id, int unsigned len, void* p) const
{ // p is an output carrier, a pointer to a double object
	// For a complex array, this does not return a complex value (it only returns real portion of the intended max element
 // Therefore, to get the intended max element, you need to get it from the max index through parg.
	unsigned int mid;
	double out = -std::numeric_limits<double>::infinity();
	if (len == 0) len = nSamples;
	if (bufBlockSize == 8)
	{
		for (unsigned int k = id; k < id + len; k++)
		{
			if (buf[k] > out)
				out = buf[k], mid = k;
		}
	}
	else if (bufBlockSize == 16)
	{
		for (unsigned int k = id; k < id + len; k++)
		{
			if (abs(cbuf[k]) > out)
				out = buf[k], mid = k;
		}
	}
	else
		throw "Invalid bufBlockSize";
	if (p) *(double*)p = (double)mid + 1 - id;
	return out;
}

double body::_min(unsigned int id, unsigned int len, void* p) const
{
	unsigned int mid;
	double out = std::numeric_limits<double>::infinity();
	if (len == 0) len = nSamples;
	if (bufBlockSize == 8)
	{
		for (unsigned int k = id; k < id + len; k++)
		{
			if (buf[k] < out)
				out = buf[k], mid = k;
		}
	}
	else if (bufBlockSize == 16)
	{
		for (unsigned int k = id; k < id + len; k++)
		{
			if (buf[k] < out)
				out = buf[k], mid = k;
		}
	}
	else
		throw "Invalid bufBlockSize";
	if (p) *(double*)p = (double)mid + 1 - id;
	return out;
}
