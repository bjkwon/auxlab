#include "sigproc.h"

static inline void __noise(double* buf, unsigned int length, int fs)
{
	auto k = (unsigned int)0;
	for_each(buf, buf + length, [&k, fs](double& v)
		{ v = 2. * ((double)rand() / (double)RAND_MAX - .5); });
}

static inline void __gnoise(double* buf, unsigned int length, int fs)
{ //Gaussian noise
	double fac, r, v1, v2, sum(0.);
	for (unsigned int k = 0; k < length; k++)
	{
		do {
			do {
				v1 = (2.0 * (double)rand() / (double)RAND_MAX) - 1.0;
				v2 = (2.0 * (double)rand() / (double)RAND_MAX) - 1.0;
				r = (v1 * v1) + (v2 * v2);
			} while (r >= 1.0);
			fac = sqrt(-2.0 * log(r) / r);
		} while (v2 * fac >= 1.0 || v2 * fac <= -1.0);
		buf[k] = v2 * fac;
		sum += v2 * fac;
	}
}

void _tparamonly(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	CVar dur = past->Sig;
	if (!dur.IsScalar())
		throw CAstException(FUNC_SYNTAX, *past, p).proc("duration must be a scalar.");
	if (dur.value() < 0.)
		throw CAstException(FUNC_SYNTAX, *past, p).proc("duration must be a non-negative number.");
	past->Sig.Reset(past->GetFs());
	unsigned int nSamplesNeeded = (unsigned int)round(dur.value() / 1000. * past->GetFs());
	past->Sig.UpdateBuffer(nSamplesNeeded); //allocate memory if necessary
	if (!strcmp(pnode->str, "noise"))
		__noise(past->Sig.buf, nSamplesNeeded, past->Sig.GetFs());
	else if (!strcmp(pnode->str, "gnoise"))
		__gnoise(past->Sig.buf, nSamplesNeeded, past->Sig.GetFs());
	else if (!strcmp(pnode->str, "silence"))
		memset(past->Sig.buf, 0, sizeof(double) * nSamplesNeeded);
	else if (!strcmp(pnode->str, "dc"))
		for_each(past->Sig.buf, past->Sig.buf + nSamplesNeeded, [](double& v) {v = 1.; });
	else
		throw CAstException(FUNC_SYNTAX, *past, p).proc("Internal error 3426.");
}