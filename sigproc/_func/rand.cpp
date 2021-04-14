#include "sigproc.h"
#include <time.h>

void _rand(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->checkScalar(pnode, past->Sig);
	double val = past->Sig.value();
	if (val < 0)
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("argument must be positive.");
	int ival = (int)round(val);
	static bool initialized(false);
	if (!initialized)
	{
		srand((unsigned)time(0));
		initialized = true;
	}
	past->Sig.UpdateBuffer(ival);
	for (int k = 0; k < ival; k++)
		past->Sig.buf[k] = (double)rand() / RAND_MAX;
}

void _irand(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->checkScalar(pnode, past->Sig);
	double val = past->Sig.value();
	if (val < 0)
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("argument must be positive.");
	static bool initialized(false);
	if (!initialized)
	{
		srand((unsigned)time(0));
		initialized = true;
	}
	past->Sig.UpdateBuffer(1);
	past->Sig.SetValue((double)ceil((double)rand() / (double)RAND_MAX * val));
}

void _randperm(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->checkScalar(pnode, past->Sig);
	int ival = (int)round(past->Sig.value());
	if (ival < 1)
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("argument must be positive.");
	static bool initialized(false);
	if (!initialized)
	{
		srand((unsigned)time(0));
		initialized = true;
	}
	past->Sig.Reset(1);
	past->Sig.UpdateBuffer((size_t)ival);
	int m, n;
	double hold;
	for (int i = 0; i < ival; i++)past->Sig.buf[i] = (double)(i + 1);
	int repeat = (int)sqrt(ival * 100.); // swapping sqrt(ival*100.) times
	for (int i = 0; i < repeat; i++)
	{
		m = (int)((double)rand() / (double)RAND_MAX * ival);
		do { n = (int)((double)rand() / (double)RAND_MAX * ival); } while (m == n);
		hold = past->Sig.buf[m];
		past->Sig.buf[m] = past->Sig.buf[n];
		past->Sig.buf[n] = hold;
	}
}

