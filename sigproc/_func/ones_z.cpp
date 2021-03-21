#include "sigproc.h"

void _zeros(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	if (!past->Sig.IsScalar())
		throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "argument must be a scalar.");
	int n = (int)round(past->Sig.value());
	past->Sig.Reset(1);
	if (n <= 0) return;
	past->Sig.UpdateBuffer(n);
//	memset(past->Sig.buf, 0, n * sizeof(double)); // tried this; not sure if this speeds up. 3/20/2021
	for (int i = 0; i < n; ++i)
		past->Sig.buf[i] = 0;
}

void _ones(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	if (!past->Sig.IsScalar())
		throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "argument must be a scalar.");
	int n = (int)round(past->Sig.value());
	past->Sig.Reset(1);
	if (n <= 0) return;
	past->Sig.UpdateBuffer(n);
	for (int i = 0; i < n; ++i)
		past->Sig.buf[i] = 1;
}

void _cumsum(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	past->checkVector(pnode, past->Sig);
	past->Sig.Cumsum();
}

void _diff(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	past->checkSignal(pnode, past->Sig);
	CVar sig = past->Sig;
	int order = 1;
	if (p) {
		past->Compute(p);
		past->checkScalar(pnode, past->Sig);
		order = (int)round(past->Sig.value());
		if (order < 1) throw CAstException(USAGE, *past, pnode).proc("non-negative argument is required");
	}
	sig.Diff(order);
	past->Sig = sig;
}
