#include <algorithm>
#include "sigproc.h"

void _interp1(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	//Need to add qualifers at some point.
	// Probably allow only vectors... 
	// Assume qp is non-decreasing sequence.
	// 3 / 6 / 2019
	CVar rx = past->Sig;
	CVar ry = past->Compute(p);
	CVar qx = past->Compute(p->next);
	vector<double> rv = rx.ToVector();
	vector<double>::iterator it = rv.begin();
	past->Sig.UpdateBuffer(qx.nSamples);
	int k = 0;
	for (unsigned q = 0; q < qx.nSamples; q++)
	{
		it = upper_bound(it, rv.end(), qx.buf[q]); //because qp is sorted, having the iterator (previous searched result, if any) as the first argument will save time.
		ptrdiff_t pos;
		double valueAlready, preVal;
		if (it != rv.end())
		{
			pos = it - rv.begin();
			pos = max(1, pos);
			preVal = *(it - 1);
		}
		else
		{
			pos = ry.nSamples;
			preVal = rv.back();
		}
		valueAlready = ry.buf[pos - 1];
		past->Sig.buf[k++] = valueAlready + (ry.buf[pos] - ry.buf[pos - 1]) / (rx.buf[pos] - rx.buf[pos - 1]) * (qx.buf[q] - preVal);
	}
}

