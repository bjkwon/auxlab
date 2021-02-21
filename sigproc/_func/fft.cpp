#ifndef NO_FFTW
#include "fftw3.h"
#include "sigproc.h"

void _fft(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	past->blockCell(pnode, past->Sig);
	past->blockScalar(pnode, past->Sig);
	past->blockString(pnode, past->Sig);
	CVar param(0.);
	if (p)
	{
		try {
			CAstSig tp(past);
			param = tp.Compute(p);
			past->checkScalar(p, param);
			if (param.value() < 1)
				throw CAstException(USAGE, *past, p).proc("argument must be a positive integer.");
		}
		catch (const CAstException & e) {
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, e.getErrMsg().c_str());
		}
	}
	past->Sig = past->Sig.fp_getsig(&CSignal::FFT, (void*)&param);
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}

void _ifft(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	past->blockCell(pnode, past->Sig);
	past->blockScalar(pnode, past->Sig);
	past->blockString(pnode, past->Sig);
	CVar param(0.);
	if (p)
	{
		try {
			CAstSig tp(past);
			param = tp.Compute(p);
			past->checkScalar(p, param);
			if (param.value() < 1)
				throw CAstException(USAGE, *past, p).proc("argument must be a positive integer.");
		}
		catch (const CAstException & e) {
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, e.getErrMsg().c_str());
		}
	}
	past->Sig = past->Sig.fp_getsig(&CSignal::iFFT, (void*)&param);
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap(0);
}

CSignal CSignal::FFT(unsigned int id0, unsigned int len, void *parg) const
{
	CVar param = *(CVar*)parg;
	if (len == 0) len = nSamples;
	if (len != nSamples)
		if (param.value() != 0)
			throw "The FFT is of a matrix on each group/row is fixed (cannot be changed).";
	int fftsize = param.value() == 0 ? len : (int)param.value();
	int fftRealsize = fftsize / 2 + 1;
	double* in;
	fftw_complex* out;
	fftw_plan p;

	in = (double*)fftw_malloc(sizeof(double) * len);
	out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * fftRealsize);
	memcpy(in, buf + id0, sizeof(double) * len);

	p = fftw_plan_dft_r2c_1d(fftsize, in, out, FFTW_ESTIMATE);
	fftw_execute(p);

	CSignal res(fs);
	res.UpdateBuffer(fftsize);
	res.SetComplex();
	memcpy(res.cbuf, out, sizeof(*cbuf) * fftRealsize);
	complex<double>* tp = reinterpret_cast<complex<double>*> (out);
	for (int k(1); k < fftRealsize; k++)
		res.cbuf[fftsize - k] = conj(tp[k]);

	// For verification
	//	double *out2 = (double*)fftw_malloc(sizeof(double) * len);
	//	fftw_plan p2 = fftw_plan_dft_c2r_1d(len, out, out2, FFTW_ESTIMATE);
	//	fftw_execute(p2);
	//	fftw_destroy_plan(p2);
	//	fftw_free(out2);

	fftw_destroy_plan(p);
	fftw_free(in);
	fftw_free(out);
	return res;
}

CSignal CSignal::iFFT(unsigned int id0, unsigned int len, void *parg) const
{
	CVar param = *(CVar*)parg;
	if (len == 0) len = nSamples;
	if (len != nSamples)
		if (param.value() != 0)
			throw "The iFFT is of a matrix on each group/row is fixed (cannot be changed).";
	int fftsize = param.value() == 0 ? len : (int)param.value();
	int fftRealsize = fftsize / 2 + 1;
	CSignal res(fs);
	res.UpdateBuffer(fftsize);
	fftw_plan p;
	bool hermit(true);
	if (!IsComplex())
	{
		res.SetComplex();
		hermit = false;
	}
	fftw_complex* in;
	//check if it's Hermitian
	for (int k = 1; hermit && k < (fftsize + 1) / 2; k++)
		if (cbuf[k] != conj(cbuf[fftsize - k])) hermit = false;
	if (hermit)
	{
		double* out = (double*)fftw_malloc(sizeof(double) * fftsize);
		in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * fftRealsize);
		memcpy(in, cbuf + id0, sizeof(fftw_complex) * fftsize / 2);
		if (fftsize % 2 == 0)
			memcpy(&in[fftsize / 2], &cbuf[fftsize / 2], sizeof(*cbuf));
		p = fftw_plan_dft_c2r_1d(fftsize, in, out, FFTW_ESTIMATE);
		fftw_execute(p);
		memcpy(res.buf, out, sizeof(double) * fftsize);
		res.bufBlockSize = sizeof(double);
		res /= (double)fftsize;
		fftw_free(out);
	}
	else
	{
		res.SetComplex();
		fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * fftsize);
		in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * fftsize);
		if (IsComplex())
			memcpy(in, cbuf + id0, sizeof(*in) * fftsize);
		else
			for (int k = 0; k < fftsize; k++)
			{
				memcpy(in + k, buf + k, sizeof(*buf));
				memset((char*)(in + k) + sizeof(*buf), 0, sizeof(*buf));
			}
		p = fftw_plan_dft_1d(fftsize, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		memcpy(res.cbuf, out, sizeof(fftw_complex) * fftsize);
		res /= (double)fftsize;
		fftw_free(out);
	}
	fftw_free(in);
	fftw_destroy_plan(p);
	res.snap = 0; // this should be zero, but just to make sure
	return res;
}

#endif //NO_FFTW
