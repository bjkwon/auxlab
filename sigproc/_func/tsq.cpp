#include "sigproc.h"

void _tsq_isrel(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs)
{
	int type = past->Sig.GetType();
	bool res = (type == CSIG_TSERIES) && (past->Sig.GetFs() == 0);
	double dres = res ? 1. : 0.;
	past->Sig.SetValue(dres);
}

void _tsq_gettimes(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs)
{
	past->checkTSeq(pnode, past->Sig);
	//get the item count; i.e., the number of chains
	int nItems = past->Sig.CountChains();
	double* dbuf = new double[nItems];
	int k = 0;
	int nChains = 1;
	bool relative = past->Sig.GetFs() == 0;
	for (CTimeSeries* q = &past->Sig; q; q = q->chain)
		dbuf[k++] = q->tmark;
	past->Sig.Reset(1);
	past->Sig.UpdateBuffer(nItems * nChains);
	past->Sig.nGroups = 1;// nChains;
	memcpy(past->Sig.buf, dbuf, sizeof(double) * past->Sig.nSamples);
	delete[] dbuf;
}

void _tsq_settimes(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs)
{
	past->checkTSeq(pnode, past->Sig);
	int nItems = past->Sig.CountChains();
	try {
		CAstSig tp(past);
		CVar newtime = tp.Compute(p);
		if (newtime.GetType() != CSIG_VECTOR)
			throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "Argument must be a vector of time points.");
		if (newtime.nSamples != nItems)
			throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "Argument vector must have the same number of elements as the TSEQ.");
		int id = 0;
		for (CTimeSeries* p = &past->Sig; p; p = p->chain)
		{
			p->tmark = newtime.buf[id++];
			if (newtime.GetFs() == 0) p->SetFs(0);
		}
	}
	catch (const CAstException & e) { throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, e.getErrMsg().c_str()); }
	const AstNode* pRoot = past->findParentNode(past->xtree, (AstNode*)pnode, true);
	past->SetVar(pRoot->str, &past->Sig);
}

void _tsq_getvalues(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs)
{
	past->checkTSeq(pnode, past->Sig);
	int k = 0, nItems = past->Sig.CountChains();
	CTimeSeries out(1);
	out.UpdateBuffer(nItems * past->Sig.nSamples);
	//	out.nGroups = nItems;
	for (CTimeSeries* p = &past->Sig; p; p = p->chain)
		memcpy(out.buf + k++ * p->nSamples, p->buf, sizeof(double) * p->nSamples); // assuming that p->nSamples is always the same
	past->Sig = out;
}

void _tsq_setvalues(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs)
{
	past->checkTSeq(pnode, past->Sig);
	int nItems = past->Sig.CountChains();
	try {
		CAstSig tp(past);
		CVar newvalues = tp.Compute(p);
		if (newvalues.GetType() != CSIG_VECTOR)
			throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "Argument must be a vector.");
		if (newvalues.nGroups != nItems)
			throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "Argument vector must have the same number of groups as the TSEQ length.");
		int id = 0;
		for (CTimeSeries* p = &past->Sig; p; p = p->chain)
		{
			p->UpdateBuffer(newvalues.Len());
			memcpy(p->buf, newvalues.buf + id++ * newvalues.Len(), sizeof(double) * newvalues.Len());
		}
	}
	catch (const CAstException & e) { throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, e.getErrMsg().c_str()); }
	const AstNode* pRoot = past->findParentNode(past->xtree, (AstNode*)pnode, true);
	past->SetVar(pRoot->str, &past->Sig);
}

