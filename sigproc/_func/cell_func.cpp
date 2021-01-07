#include "sigproc.h"

void _cell(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	if (!past->Sig.IsScalar())
		throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "argument must be a scalar.");
	int n = (int)round(past->Sig.value());
	if (n <= 0)
		throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "argument must be a positive number.");

	past->Sig.Reset();
	CVar tp;
	for (int k = 0; k < n; k++)
		past->Sig.appendcell(tp);
}

void _erase(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	if (!past->Sig.IsStruct())
		throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "Must be applied to a class/struct object.");
	try {
		CAstSig tp(past);
		CVar param = tp.Compute(p);
		if (!param.IsString())
			throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "Argument must be a string indicating the member of the class/struct object.");
		auto it = past->Sig.strut.find(param.string());
		if (it != past->Sig.strut.end())
			past->Sig.strut.erase(it);
		const AstNode* pRoot = past->findParentNode(past->xtree, (AstNode*)pnode, true);
		past->SetVar(pRoot->str, &past->Sig);
	}
	catch (const CAstException & e) { throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, e.getErrMsg().c_str()); }
}

void _head(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	if (!past->Sig.IsStruct())
		throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "Must be applied to a class/struct object.");
	try {
		CAstSig tp(past);
		CVar param = tp.Compute(p);
		past->Sig.set_class_head(param);
	}
	catch (const CAstException & e) { throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, e.getErrMsg().c_str()); }
	const AstNode* pRoot = past->findParentNode(past->xtree, (AstNode*)pnode, true);
	past->SetVar(pRoot->str, &past->Sig);
}
