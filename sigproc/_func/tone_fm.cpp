#include "sigproc.h"

// DOCUMENT initPhase --- 1 coresponds to PI (180 degrees)
// If you want to make 1 corresponding to 360 degrees.... go ahead, change and document it 12/10/2020
// CHECK!!!!!!!!!!!!!!! 1/22/2021
static inline void __tone(double *buf, double freq, unsigned int length, int fs, double initPhase)
{
	auto k = (unsigned int)0;
	initPhase *= 2 * PI;
	for_each(buf, buf + length, [&k, freq, fs, initPhase](double& v)
		{ v = sin(freq / (double)fs * k++ + initPhase); });
}

static inline void __tone_glide(double* buf, double f1, double f2, unsigned int length, int fs)
{
	auto t(0.), tgrid(1. / fs);
	auto duration = (double)length / fs;
	auto gld = (f2 - f1) / 2. / duration;
	for_each(buf, buf + length, [&t, tgrid, f1, gld](double& v)
		{
			t += tgrid;
			v = sin(2 * PI * t * (f1 + gld * t));
		});
}

static inline void __fm(double* buf, int fs, double midFreq, double fmWidth, double fmRate, unsigned int length, double beginFMPhase)
{   // beginFMPhase is to be set. (But the beginning phase of the sine wave is not meaningful, so skipped)
	auto t(0.), tgrid(1. / fs);
	for_each(buf, buf + length, [&t, tgrid, midFreq, fmWidth, fmRate, beginFMPhase](double& v)
		{
			t += tgrid;
			v = sin(2 * PI * t * midFreq - fmWidth / fmRate * cos(2 * PI * (fmRate * t + beginFMPhase)));
		});
}

void _tone(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	double initPhase(0.);
	int nArgs = 0;
	for (const AstNode* cp = p; cp; cp = cp->next)
		++nArgs;
	if (!past->Sig.IsScalar() && (!past->Sig.IsVector() || past->Sig.nSamples > 2))
		throw CAstException(FUNC_SYNTAX, *past, p).proc("Frequency must be either a constant or two-element array.");
	body freq = past->Sig; //should be same as tp.Compute(p);
	if (freq._max() >= past->GetFs() / 2)
		throw CAstException(FUNC_SYNTAX, *past, p).proc("Frequency exceeds Nyquist frequency.");
	CVar duration = past->Compute(p->next);
	past->checkScalar(pnode, duration);
	if (duration.value() <= 0)
		throw CAstException(FUNC_SYNTAX, *past, p).proc("Duration must be positive.");
	if (nArgs == 3)
	{
		CVar _initph = past->Compute(p->next->next);
		if (!_initph.IsScalar())
			throw CAstException(FUNC_SYNTAX, *past, p).proc("Initial_phase must be a scalar.");
		initPhase = _initph.value();
	}
	past->Sig.Reset(past->GetFs());
	auto nSamplesNeeded = (unsigned int)round(duration.value() / 1000. * past->Sig.GetFs());
	past->Sig.Reset();
	past->Sig.UpdateBuffer(nSamplesNeeded); //allocate memory if necessary
	if (freq.nSamples == 1)
	{
		__tone(past->Sig.buf, 2. * PI * freq.value(), nSamplesNeeded, past->Sig.GetFs(), initPhase);
	}
	else // len == 2
	{
		// For now, initPhase is ignored when a two-element freq is given.
		__tone_glide(past->Sig.buf, freq.buf[0], freq.buf[1], nSamplesNeeded, past->Sig.GetFs());
	}
}

void _fm(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	CVar fifth, second = past->Compute(p->next);
	CVar third = past->Compute(p->next->next);
	CVar fourth = past->Compute(p->next->next->next);
	if (!second.IsScalar()) throw CAstException(FUNC_SYNTAX, *past, p).proc("freq2 must be a scalar.");
	if (!third.IsScalar()) throw CAstException(FUNC_SYNTAX, *past, p).proc("mod_rate must be a scalar.");
	if (!fourth.IsScalar()) throw CAstException(FUNC_SYNTAX, *past, p).proc("duration must be a scalar.");
	if (fourth.value() <= 0) throw CAstException(FUNC_SYNTAX, *past, p).proc("duration must be positive.");
	if (p->next->next->next->next) {	// 5 args
		fifth = past->Compute(p->next->next->next->next);
		if (!fifth.IsScalar()) throw CAstException(FUNC_SYNTAX, *past, p).proc("init_phase must be a scalar.");
	}
	else fifth.SetValue(0);

	past->Compute(p);
	double freq1 = past->Sig.value();
	double freq2 = second.value();
	double midFreq = (freq1 + freq2) / 2.;
	double width = fabs(freq1 - freq2) / 2.;
	double modRate = third.value();
	double initPhase = fifth.value();
	double dur = fourth.value();
	past->Sig.Reset(past->GetFs());
	auto nSamplesNeeded = (int)round(dur / 1000. * past->Sig.GetFs());
	past->Sig.UpdateBuffer(nSamplesNeeded);
	if (modRate != 0.)
	{
		__fm(past->Sig.buf, past->Sig.GetFs(), midFreq, width, modRate, nSamplesNeeded, initPhase - .25);
	}
	else
	{
		__tone_glide(past->Sig.buf, freq1, freq2, nSamplesNeeded, past->Sig.GetFs());
	}
}
