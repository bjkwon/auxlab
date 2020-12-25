#ifndef NO_FFTW
#include "fftw3.h"
#include "sigproc.h"

void _envelope(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	past->Sig.fp_mod(&CSignal::HilbertEnv);
}

void _hilbert(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	past->checkAudioSig(pnode, past->Sig);
	past->Sig.fp_mod(&CSignal::Hilbert);
}

CSignal& CSignal::Hilbert(unsigned int id0, unsigned int len, void *p)
{//This calculates the imaginary part of the analytic signal (Hilbert) transform and updates buf with it.
//To get the envelope, get the sqrt of x*x (original signal) plus hilbertx*hilbertx
	if (len == 0) len = nSamples;

	fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * len);
	fftw_complex* mid = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * len);
	fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * len);
	memset(in, 0, len * sizeof(fftw_complex));
	memset(mid, 0, len * sizeof(fftw_complex));
	memset(out, 0, len * sizeof(fftw_complex));

	// FFT
	for (unsigned int k = 0; k < len; k++) in[k][0] = buf[k + id0];
	fftw_plan p1 = fftw_plan_dft_1d(len, in, mid, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_execute(p1);

	memset(in, 0, len * sizeof(fftw_complex));
	// converting halfcomplex array to complex array
	int half = len / 2 + len % 2;
	in[0][0] = mid[0][0];

	for (int k(1); k < half; ++k)
	{
		in[k][0] = 2 * mid[k][0];
		in[k][1] = 2 * mid[k][1];
	}

	if (len % 2 == 0)	// len is even
		in[half][0] = mid[half][0];
	// leave the rest zero

	// iFFT
	fftw_plan p2 = fftw_plan_dft_1d(len, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
	fftw_execute(p2);

	fftw_destroy_plan(p1);
	fftw_destroy_plan(p2);

	for (unsigned int k(0); k < len; ++k)
	{// scale back down since the resulting array is scaled by len.
//		buf[k+id0] = out[k][0] / len;	// This line fills buf with the identical signal with the input
		buf[k + id0] = out[k][1] / len;	// This line is about the imaginary part of the analytic signal.
	}
	fftw_free(in);
	fftw_free(out);
	return *this;
}


CSignal& CSignal::HilbertEnv(unsigned int id0, unsigned int len, void *p)
{
	CSignal copy(fs), out(fs);
	copy.UpdateBuffer(len);
	memcpy(copy.buf, buf + id0, len * bufBlockSize);
	copy.Hilbert(); // making it a phase-shifted version
	SetComplex();
	for (unsigned int k = id0; k < id0 + len; k++) buf[2 * k + 1] = copy.buf[k - id0];
	out.UpdateBuffer(len);
	for (unsigned int k = id0; k < id0 + len; k++) out.buf[k - id0] = abs(cbuf[k]);
	SetReal();
	memcpy(buf + id0, out.buf, len * bufBlockSize);
	return *this;
}

#endif //NO_FFTW
