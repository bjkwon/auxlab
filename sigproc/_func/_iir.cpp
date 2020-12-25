#include "sigproc.h"
#include "bjcommon.h" // sformat

//from ellf.c
extern "C" int design_iir(double* num, double* den, int fs, int kind, int type, int n, double* freqs, double dbr /*rippledB*/, double dbd /*stopEdgeFreqORattenDB*/);

void _iir(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	int type(0), kind(1), norder(4);
	double freqs[2], rippledB(0.5), stopbandFreqORAttenDB(-40.);
	const AstNode* args = p;
	string fname = pnode->str;
	past->checkAudioSig(pnode, past->Sig);
	CVar sigX(past->Sig);
	string emsg;
	past->Compute(p);
	freqs[0] = past->Sig.value();
	p = p->next;
	if (fname == "lpf") type = 1;
	else if (fname == "bpf") type = 2;
	else if (fname == "hpf") type = 3;
	else if (fname == "bsf") type = 4;
	try {
		switch (type)
		{
		case 1:
		case 3:
			if (p) {
				past->Compute(p);		norder = (int)round(past->Sig.value());
				p = p->next;
				if (p) {
					past->Compute(p);	kind = (int)round(past->Sig.value());
					p = p->next;
					if (p) {
						past->Compute(p);		rippledB = past->Sig.value();
						p = p->next;
						if (p) { past->Compute(p);		stopbandFreqORAttenDB = past->Sig.value(); }
					}
				}
			}
			break;
		case 2:
		case 4:
			past->Compute(p);		freqs[1] = past->Sig.value();
			p = p->next;
			if (p) {
				past->Compute(p);	norder = (int)round(past->Sig.value());
				p = p->next;
				if (p) {
					past->Compute(p);		kind = (int)round(past->Sig.value());
					p = p->next;
					if (p) {
						past->Compute(p);		rippledB = past->Sig.value();
						p = p->next;
						if (p)
						{
							past->Compute(p);		stopbandFreqORAttenDB = past->Sig.value();
						}
					}
				}
			}
			break;
		}
		double dkind = kind;
		double dtype = type;
		double dnorder = norder;
		vector<double*> params;
		params.push_back(&dkind);
		params.push_back(&dtype);
		params.push_back(&dnorder);
		params.push_back(freqs);
		params.push_back(&rippledB);
		params.push_back(&stopbandFreqORAttenDB);

		sigX.fp_mod(&CSignal::IIR, &params);
		past->Sig = sigX;
	}
	catch (const char* estr)
	{
		sformat(emsg, "Invalid argument---%s\nFor more of ELLF Digital Filter Calculator, check http://www.moshier.net/index.html.", estr);
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, emsg.c_str());
	}
}

//void CSignal::IIR(int kind, int type, int order, double *freqs, double passRipple_dB, double stopFreqORAttenDB)
CSignal& CSignal::IIR(unsigned int id0, unsigned int len, void *p)
{// kind: 1 butterworth, 2 Chebyshev, 3 Elliptic
	// type: 1 lowpass, 2 bandpass, 3 highpass, 4 bandstop

	if (len == 0) len = nSamples;

	vector<double*> params = *(vector<double*>*)p;
	vector<double*>::iterator it = params.begin();
	int kind = (int)**it; it++;
	int type = (int)**it; it++;
	int order = (int)**it; it++;
	double* freqs = *it; it++;
	double passRipple_dB = **it; it++;
	double stopFreqORAttenDB = **it;

	double* den = new double[2 * order + 1];
	double* num = new double[2 * order + 1];

	// To obtaine the filter coefficients, the object must be a scalar, the sample rate. 
	// Then on success of this call , this object has the buffer of a and b (den and num) in that order.
	if (IsScalar()) fs = (int)value();
	int res = design_iir(num, den, GetFs(), kind, type, order, freqs, passRipple_dB, stopFreqORAttenDB);
	char errstr[256];
	if (res <= 0) {
		switch (res) {
		case -1:
			strcpy(errstr, "((kind <= 0) || (kind > 3))");
			break;
		case -2:
			strcpy(errstr, "((type <= 0) || (type > 4))");
			break;
		case -3:
			strcpy(errstr, "(n <= 0)");
			break;
		case -4:
			strcpy(errstr, "(fs <= 0)");
			break;
		case -5:
			strcpy(errstr, "Filter frequency is greater than Nyquist rate.");
			break;
		default:
			sprintf(errstr, "Unknown error, code=%d", res);
		}
		throw errstr;
	}
	else {
		res = 1;
		if (IsScalar()) {
			if (type & 1)
				UpdateBuffer(2 * order + 2);
			else
				UpdateBuffer(4 * order + 2);
			memcpy(buf, den, sizeof(*buf) * (nSamples / 2));
			memcpy(buf + (nSamples / 2), num, sizeof(*buf) * (nSamples / 2));
		}
		else {
			if (type & 1)
				Filter(order + 1, num, den);
			else
				Filter(2 * order + 1, num, den);
		}
	}
	delete[] den; delete[] num;
	return *this;
}

void CSignal::Filter(unsigned int nTabs, double* num, double* den)
{//used in IIR
	unsigned int i, j;
	if (IsComplex())
	{
		complex<double> xx, * out = new complex<double>[nSamples];
		for (i = 0; i < nSamples; i++)
		{
			xx = num[0] * cbuf[i];
			for (j = 1; j < nTabs && i >= j; j++)
				xx += num[j] * cbuf[i - j];
			for (j = 1; j < nTabs && i >= j; j++)
				xx -= den[j] * out[i - j];
			out[i] = xx;
		}
		if (cbuf) delete[] cbuf;
		cbuf = out;
	}
	else
	{
		if (*den != 1.)
		{
			double tp = *den;
			for (unsigned int k = 0; k < nTabs; k++)
			{
				den[k] /= tp;
				num[k] /= tp;
			}
		}
		double xx, * out = new double[nSamples];
		for (i = 0; i < nSamples; i++)
		{
			xx = num[0] * buf[i];
			for (j = 1; j < nTabs && i >= j; j++)
				xx += num[j] * buf[i - j];
			for (j = 1; j < nTabs && i >= j; j++)
				xx -= den[j] * out[i - j];
			out[i] = xx;
		}
		if (buf) delete[] buf;
		buf = out;
	}
}
