#include "sigproc.h"

void _imaginary_unit(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->pseudo_vars.find(pnode->str))).second.alwaysstatic);
	complex<double> x(0, 1);
	past->Sig.SetValue(x);
}

void _pi(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->Sig.SetValue(asin(1) * 2);
}

void _natural_log_base(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->Sig.SetValue(exp(1));
}
