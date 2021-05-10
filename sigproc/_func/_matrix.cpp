#include "sigproc.h"

void _group(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->blockCell(pnode, past->Sig);
	past->blockString(pnode, past->Sig);
	//	past->blockGO(pnode, past->Sig); // need this 7/11/2020
	CVar second;
	try {
		CAstSig tp(past);
		second = tp.Compute(p);
	}
	catch (const CAstException & e) {
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc(e.getErrMsg().c_str());
	}
	if (!second.IsScalar())
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("argument must be a scalar.");
	double val = second.value();
	if (val != (double)(int)val)
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("argument must be an integer.");
	double nCols = past->Sig.nSamples / val;
	auto type = past->Sig.type();
	if (past->Sig.IsAudio())
	{
		if (past->Sig.chain)
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("To make a matrix from an audio signal, null portions should be filled with zeros. Call contig().");
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
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("The length of array must be divisible by the requested the row count.");
	}
	else
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("Vector or audio signals required.");
	past->Sig.nGroups = (int)val;
	if (!strcmp(pnode->str, "matrix"))
		past->statusMsg = "(NOTE) matrix() is superseded by group() and will be removed in future versions.";
}

void _buffer(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->blockCell(pnode, past->Sig);
	past->blockString(pnode, past->Sig);
	//	past->blockGO(pnode, past->Sig); // need this 7/11/2020
	CVar second, third;
	try {
		CAstSig tp(past);
		second = tp.Compute(p);
		if (p->next) third = tp.Compute(p->next);
	}
	catch (const CAstException& e) {
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc(e.getErrMsg().c_str());
	}
	if (!second.IsScalar())
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("2nd argument must be a scalar.");
	double _overlap, _blocklen = second.value();
	if (_blocklen != (double)(int)_blocklen)
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("2nd argument must be an integer.");
	if (third.type() == TYPEBIT_NULL) _overlap = 0.;
	else
	{
		if (!third.IsScalar())
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("3rd argument must be a scalar.");
		_overlap = third.value();
		if (_overlap != (double)(int)_overlap)
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("3rd argument must be an integer.");
	}
	unsigned int nGroups;
	auto type = past->Sig.type();
	auto blocklen = (unsigned int)_blocklen;
	auto overlap = (unsigned int)_overlap;
	if (past->Sig.IsAudio() || (type & 0x0003) <= 2) // audio, vector or constant
	{
		if (past->Sig.chain)
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("To make a matrix from an audio signal, null portions should be filled with zeros. Call contig().");
		nGroups = (unsigned int)ceil(past->Sig.nSamples / (_blocklen - _overlap));
		unsigned int newlength = nGroups * (unsigned int)blocklen;
		CVar out(past->Sig.GetFs());
		CVar *pout;
		auto nSamples0 = past->Sig.nSamples;
		if (newlength == past->Sig.nSamples)
		{
			past->Sig.nGroups = nGroups;
			return;
		}
		else if (newlength > past->Sig.nSamples)
		{
			out.UpdateBuffer(newlength);
			pout = &out;
		}
		else
		{
			pout = &past->Sig;
			pout->nSamples = newlength;
		}
		unsigned int m = 0;
		for (; m < nGroups-1; m++)
			memcpy(pout->buf + m * blocklen, past->Sig.buf + m * (blocklen - overlap), sizeof(past->Sig.buf) * blocklen);
		auto k = m * blocklen;
		//auto n = k - m * overlap; // the index for past->Sig.buf
		for (; k < nSamples0 + m * overlap; k++)
			pout->buf[k] = past->Sig.buf[k - m * overlap];
		pout->nGroups = nGroups;
		if (newlength > past->Sig.nSamples) 
			past->Sig = out;
	}
	else
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("Vector or audio signals required.");
}

void _ungroup(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->blockCell(pnode, past->Sig);
	past->blockString(pnode, past->Sig);
	//	past->blockGO(pnode, past->Sig); // need this 7/11/2020
	CVar second;
	try {
		CAstSig tp(past);
		second = tp.Compute(p);
	}
	catch (const CAstException& e) {
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc(e.getErrMsg().c_str());
	}
	if (!second.IsScalar())
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("argument must be a scalar.");
	double _overlap = second.value();
	if (_overlap != (double)(int)_overlap)
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("argument must be an integer.");
	auto type = past->Sig.type();
	auto overlap = (int)_overlap;
	if (past->Sig.IsAudio() || (type & 0x0003) <= 2) // audio, vector or constant
	{
		CVar out(past->Sig.GetFs());
		out.UpdateBuffer(past->Sig.nSamples);
		auto blocklen = past->Sig.Len();
		int id = 0;
		int nMoves = blocklen;
		for (unsigned int k = 0, id2 = 0; k < past->Sig.nGroups; k++)
		{
			if (k>0) nMoves = blocklen - overlap;
			memmove(out.buf + id, past->Sig.buf + id2, past->Sig.bufBlockSize * nMoves);
			id += nMoves;
			id2 += nMoves;
			id -= overlap;
			for (int m = 0; m < overlap && id2 < past->Sig.nSamples; m++)
				out.buf[id++] += past->Sig.buf[id2++];
		}
		out.nSamples = id;
		past->Sig = out;
	}
	else
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("Vector or audio signals required.");
}