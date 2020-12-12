#include "sigproc.h"


void _hamming(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	past->checkSignal(pnode, past->Sig);
	past->Sig.runFct2modify(&CSignal::Hamming);
}

void _blackman(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	past->checkSignal(pnode, past->Sig);
	double alpha;
	string fname = pnode->str;
	if (fname == "blackman")
		alpha = .16;
	else
		alpha = 0;
	if (p) // if hann, p should be NULL, so alpha stays zero
	{
		CAstSig temp(past);
		temp.Compute(p);
		temp.checkScalar(pnode, temp.Sig);
		alpha = temp.Sig.value();
	}
	past->Sig.runFct2modify(&CSignal::Blackman, &alpha);
}

void _ramp(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	CVar param;
	try {
		CAstSig tp(past);
		tp.Compute(p);
		param = tp.Compute(p);
	}
	catch (const CAstException & e) { throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, e.getErrMsg().c_str()); }
	if (!param.IsScalar())
		throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "Ramp_duration must be a scalar.");
	double ramptime = param.value();
	past->Sig.runFct2modify(&CSignal::dramp, &ramptime);
}

void _sam(CAstSig* past, const AstNode* pnode, const AstNode* p, std::string& fnsigs)
{
	double modRate, amDepth(1.), initPhase(0.);
	CVar rate, depth, initphase;
	past->checkAudioSig(pnode, past->Sig);
	try {
		CAstSig tp(past);
		rate = tp.Compute(p);
		if (!rate.IsScalar())
			throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "AM_rate must be a scalar.");
		modRate = rate.value();
		int nArgs(0);
		for (const AstNode* cp = p; cp; cp = cp->next)
			++nArgs;
		if (nArgs >= 2) //either 2 or 3
		{
			depth = tp.Compute(p->next);
			if (!depth.IsScalar())
				throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "AM_depth_m must be a scalar.");
			amDepth = depth.value();
			if (nArgs == 3)
			{
				initphase = tp.Compute(p->next->next);
				if (!initphase.IsScalar())
					throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "initial_phase must be a scalar.");
				initPhase = initphase.value();
			}
		}
	}
	catch (const CAstException & e) { throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, e.getErrMsg().c_str()); }
	past->Sig.SAM(modRate, amDepth, initPhase);
}

CSignal& CSignal::Hamming(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	for (unsigned int k = 0; k < len; k++)
		buf[id0 + k] *= 0.54 - 0.46 * cos(2.0 * PI * k / (len - 1.0));
	return *this;
}

CSignal& CSignal::Blackman(unsigned int id0, unsigned int len)
{
	double alpha = *(double*)parg;
	for (unsigned int k = 0; k < len; k++)
		buf[id0 + k] *= (1 - alpha) / 2 - 0.5 * cos(2.0 * PI * k / (len - 1.0)) + alpha / 2 * cos(4.0 * PI * k / (len - 1.0));
	return *this;
}

CSignal& CSignal::dramp(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	double dur_ms = *(double*)parg;
	double drampFs = 1.e3 / (4. * dur_ms);
	unsigned int nSamplesNeeded = (unsigned int)round(dur_ms / 1000. * fs);
	nSamplesNeeded = min(nSamples, nSamplesNeeded);
	for (unsigned int i = 0; i < nSamplesNeeded; i++)
	{
		double x = sin(2 * PI * drampFs * i / fs);
		buf[id0 + i] *= x * x;
		buf[id0 + len - i - 1] *= x * x;
	}
	return *this;
}

CSignal& CSignal::SAM(double modRate, double modDepth, double initPhase)
{
	double* env = new double[nSamples];
	for (unsigned int k = 0; k < nSamples; k++)
		env[k] = (1. + modDepth * sin(2 * PI * (k * modRate / fs + initPhase - .25))) / (1. + modDepth);
	Modulate(env, nSamples);
	delete[] env;
	return *this;
}

CTimeSeries& CTimeSeries::SAM(double rate, double depth, double phase)
{
	for (CTimeSeries* p = this; p; p = p->chain)
		p->CSignal::SAM(rate, depth, phase);
	return *this;
}

CSignals& CSignals::SAM(double rate, double depth, double phase)
{
	CTimeSeries::SAM(rate, depth, phase);
	if (next)	next->CTimeSeries::SAM(rate, depth, phase);
	return *this;
}
