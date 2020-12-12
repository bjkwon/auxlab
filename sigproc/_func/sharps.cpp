#include "sigproc.h"

void _imaginary_unit(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	complex<double> x(0, 1);
	past->Sig.SetValue(x);
}

void _pi(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	past->Sig.SetValue(asin(1) * 2);
}

void _natural_log_base(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	past->Sig.SetValue(exp(1));
}
