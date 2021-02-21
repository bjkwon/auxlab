#include "sigproc.h"

void _matrix(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	past->blockCell(pnode, past->Sig);
	past->blockString(pnode, past->Sig);
	//	past->blockGO(pnode, past->Sig); // need this 7/11/2020
	CVar second;
	try {
		CAstSig tp(past);
		second = tp.Compute(p);
	}
	catch (const CAstException & e) {
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, e.getErrMsg().c_str());
	}
	if (!second.IsScalar())
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, "argument must be a scalar.");
	double val = second.value();
	if (val != (double)(int)val)
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, "argument must be an integer.");
	double nCols = past->Sig.nSamples / val;
	auto type = past->Sig.type();
	if (past->Sig.IsAudio())
	{
		if (past->Sig.chain)
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, "To make a matrix from an audio signal, null portions should be filled with zeros. Call contig().");
		double rem = fmod((double)past->Sig.nSamples, val);
		if (rem > 0)
		{
			unsigned int nPtsNeeded = (unsigned int)(val - rem);
			past->Sig.UpdateBuffer(past->Sig.nSamples + nPtsNeeded);
		}
	}
	else if ((type & 0x0003) <= 2) // vector or constant
	{
		if (past->Sig.nSamples / val != (int)nCols)
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, "The length of array must be divisible by the requested the row count.");
	}
	else
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, "Vector or audio signals required.");
	past->Sig.nGroups = (int)val;
}

