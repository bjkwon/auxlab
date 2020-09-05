// AUXLAB 
//
// Copyright (c) 2009-2019 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.7
// Date: 5/24/2020

#pragma once

using namespace std;

#include <string>
#include <complex>
#include <vector>
#include <map>
#include <functional>

#define TYPEBIT_NULL		0x0000
#define TYPEBIT_TEMPORAL	0x0004
#define TYPEBIT_SNAP		0x0008
// The difference between TYPEBIT_AUDIO and TYPEBIT_TSEQ: 
// data in TYPEBIT_AUDIO aligned with time points in grid of 1/fs,
// data in TYPEBIT_TSEQ are stacked on the same tmark.
#define TYPEBIT_AUDIO		(TYPEBIT_TEMPORAL + TYPEBIT_NULL)
#define TYPEBIT_TSEQ		(TYPEBIT_TEMPORAL + TYPEBIT_SNAP)
#define TYPEBIT_STRING		0x0020
#define TYPEBIT_LOGICAL		0x0080
#define TYPEBIT_GO			0x0100
#define TYPEBIT_CELL		0x1000
#define TYPEBIT_STRUT		0x2000
#define TYPEBIT_STRUTS		0x4000

#define CSIG_EMPTY		0
#define CSIG_STRING		1
#define CSIG_NULL		2
#define CSIG_SCALAR		4
#define CSIG_CELL		8
#define CSIG_STRUCT		10
#define CSIG_HDLARRAY	12
#define CSIG_HANDLE		15
#define CSIG_VECTOR		17
#define CSIG_MATRIX		19
#define CSIG_AUDIO		65
#define CSIG_TSERIES	81 // Think about this... improve it... 5/24/2018

#ifndef AUX_CLASSES
#define AUX_CLASSES
class body
{
public:
	unsigned int nSamples;
	unsigned int nGroups;
	union
	{
		char *strbuf;
		complex<double> *cbuf;
		double *buf;
		bool *logbuf;
	};
	unsigned char bufBlockSize;
	bool ghost;

	// Constructors
	body();
	body(const body& src);
	body(double value);
	body(complex<double> value);
	body(double *y, int  len);
	body(bool *y, int len);
	body(const vector<double> & src);
	virtual ~body();

	body& operator=(const body& rhs);
	body& operator=(const vector<double> & rhs);
	body& operator<=(const body& rhs);

	body& operator+=(const double con);
	body& operator*=(const double con);
	body& operator/=(const double con);
	bool operator==(const body & rhs);

	body& UpdateBuffer(unsigned int length, unsigned int offset = 0);
	void Reset();

	double value() const;
	string valuestr(int digits = 14) const;
	complex<double> cvalue() const;
	void SetValue(double v);
	void SetValue(complex<double> v);
	void SetComplex();
	void SetReal();
	bool IsComplex() const {return bufBlockSize == 2 * sizeof(double);	};
	bool IsBool() const { return bufBlockSize == 1; };

	void SwapContents1node(body &sec);
	vector<double> ToVector() const;

	body &addmult(char type, body &arg, unsigned int id0 = 0, unsigned int len = 0);

	body &each(double (*fn)(double));
	body &each(double (*fn)(complex<double>));
	body &each(complex<double>(*fn)(double));
	body &each(complex<double> (*fn)(complex<double>));
	body &each(double (*fn)(double, double), const body &arg2); 
	body &each(complex<double> (*fn)(complex<double>, complex<double>), const body &arg2);

	double* begin() { return nSamples>0 ? &buf[0] : nullptr; }
	double* end() { return nSamples>0 ? &buf[nSamples] : nullptr; }

	body & mathFunForbidNegative(double(*fn)(double), complex<double>(*cfn)(complex<double>)); // MFFN
	body & mathFunForbidNegative(double(*fn)(double, double), complex<double>(*cfn)(complex<double>, complex<double>), const body &param); // MFFN

	body &transpose();
	body& MakeLogical();
	body& LogOp(body &rhs, int type);
	body &insert(body &sec, int id);

	body & interp1(body &that, body &qp);
	vector<double> _max(unsigned int id0 = 0, unsigned int len = 0) const;
	vector<double> _min(unsigned int id0 = 0, unsigned int len = 0) const;
	vector<double> sum(unsigned int id0=0, unsigned int len = 0) const;
	vector<double> mean(unsigned int id0 = 0, unsigned int len = 0) const;
	vector<double> stdev(unsigned int id0 = 0, unsigned int len = 0) const;
	bool operator < (const body &rhs) const;

	void* parg;

protected:
		body& operator<=(body * rhs);
};

class CSignal : public body
{
protected:
	int fs;
public:
	double tmark;
	short snap; // 0 for regular; 1 for time seq or an object where data stack up on the same tmark (vertically), like FFT 
	unsigned int Len() { if (fs == 2) return (nSamples-1) / nGroups; else  return nSamples / nGroups; }
	bool operator < (const CSignal &rhs) const;

	// Signal generation (no stereo handling)
	double * fm(double midFreq, double fmWidth, double fmRate, int  nsamples, double beginFMPhase=0.);
	double * fm(double midFreq, double fmWidth, double fmRate, double dur_ms, double beginFMPhase=0.);
	double * Silence(unsigned int nsamples);
	double * Silence(double dur_ms);
	double * DC(double dur_ms);
	double * DC(unsigned int nsamples);
	double * Tone(vector<double> freqs, unsigned int nsamples);
	double * Tone(vector<double> freqs, double dur_ms);
	double * Tone(double freq, unsigned int nsamples, double beginPhase=0.);
	double * Noise(double dur_ms);
	double * Noise(unsigned int nsamples);
	double * Noise2(double dur_ms);
	double * Noise2(unsigned int  nsamples);
	double * Truncate(double time_ms1, double time_ms2);
	double * Truncate(int id1, int id2, int code=0);
	CSignal & Modulate(vector<double> &tpoints, vector<double> &tvals);
	CSignal &matrixmult(CSignal *arg);
	CSignal & Diff(int order=1);
	CSignal & Cumsum();

	// Window functions -- signal generation function
	CSignal & Hamming(unsigned int id0 = 0, unsigned int len = 0);
	CSignal & Blackman(unsigned int id0 = 0, unsigned int len = 0);

	// Signal alteration (stereo handling with a clean, inarguable convention)
	virtual void Filter(unsigned int nTabs, double *num, double *den);
	int DecFir(const CSignal & coeff, int offset, int nChan);
#ifndef NO_FFTW
	CSignal & Hilbert(unsigned int id0 = 0, unsigned int len = 0);
	CSignal & HilbertEnv(unsigned int id0 = 0, unsigned int len = 0);
	CSignal & movespec(unsigned int id0 = 0, unsigned int len = 0);
#endif
	void DownSample(int q);
	void UpSample(int p);
	CSignal & Reset(int  fs2set=0);
	void ReverseTime();
	CSignal & Interp(const CSignal & gains, const CSignal & tmarks);
	CSignal & reciprocal(void);	// Multiplicative inverse
	CSignal & operator=(const body& rhs);
	CSignal & operator=(const CSignal & rhs);
	CSignal & operator<=(const CSignal & rhs);
	bool operator== (const CSignal &rhs);
	bool operator == (double rhs);
	bool operator == (string rhstr);
	CSignal & operator+=(CSignal *yy); // Concatenation
	CSignal & operator>>=(double delta);
	CSignal & operator-(void);	// Unary minus

	// Retrieve signal characteristics (single channel ONLY)
	int GetType() const; 
	int GetFs() const {return fs;};
	void SetFs(int  newfs);
	double* GetBuffer() {return buf;}
	vector<double> length(unsigned int id0 = 0, unsigned int len = 0) const;
	vector<double> dur(unsigned int id0 = 0, unsigned int len = 0) const;
	vector<double> durc(unsigned int id0 = 0, unsigned int len = 0) const;
	vector<double> begint(unsigned int id0 = 0, unsigned int len = 0) const;
	vector<double> endt(unsigned int id0 = 0, unsigned int len = 0) const;
	vector<double> RMS(unsigned int id0 = 0, unsigned int len = 0) const;

	CSignal &_atmost(unsigned int id, int unsigned len);
	CSignal &_atleast(unsigned int id, int unsigned len);
	CSignal & sort(unsigned int id0 = 0, unsigned int len = 0);
	CSignal & conv(unsigned int id0, unsigned int len = 0);
	CSignal & dramp(unsigned int id0 = 0, unsigned int len = 0);
	CSignal & filter(unsigned int id0 = 0, unsigned int len = 0);
	CSignal & filtfilt(unsigned int id0 = 0, unsigned int len = 0);
	CSignal & resample(unsigned int id0 = 0, unsigned int len = 0);
#ifndef NO_IIR
	CSignal & IIR(unsigned int id0 = 0, unsigned int len = 0); 
#endif // NO_IIR

	inline bool IsEmpty() const { return nSamples == 0 && tmark == 0.; }
	inline bool IsScalar() const { return nSamples == 1; }
	inline bool IsVector() const { return nSamples > 1; }
	inline bool IsAudio() const { uint16_t tp = type(); return tp & TYPEBIT_AUDIO  && !(tp & TYPEBIT_SNAP); }
	bool IsString() const { return fs == 2; }

	CSignal & Modulate(double *env, unsigned int lenEnv, unsigned int beginID=0);
	CSignal & SAM(double modRate, double modDepth, double initPhase);

	// Constructors
	CSignal();
	CSignal(int sampleRate); // construct with a specified sample rate.
	CSignal(int sampleRate, unsigned int len); // construct with a specified sample rate and buffer size
	CSignal(double value); // construct a scala with the specified value
	CSignal(const CSignal & src); // copy constructor
	CSignal(double *y, int  len);
	CSignal(vector<double> vv); 
	CSignal(string str); // make a string CSignal

	virtual ~CSignal();	

	string string() const;
	char *getString(char *str, const int size);
	CSignal &SetString(const char *str);
	CSignal &SetString(const char c);
	CSignal FFT(unsigned int id0 = 0, unsigned int len = 0) const;
	CSignal iFFT(unsigned int id0 = 0, unsigned int len = 0) const;

	bool IsSingle() const;
	bool IsLogical() const { return bufBlockSize == 1 && fs != 2; } // this doesn't differentiate "logical" audio

	uint16_t type() const
	{
		uint16_t out = TYPEBIT_NULL;
		if (IsEmpty())			return out;
		else if (fs == 1)		out = 0;
		else if (fs == 2)		out = TYPEBIT_STRING;
		else if (fs == 0 || fs > 500)		out = TYPEBIT_TEMPORAL;
		if (snap) out += TYPEBIT_SNAP;
		if (nSamples > 0) out++;
		if (nSamples > 1) out++;
		if (IsLogical()) out += TYPEBIT_LOGICAL;
		return out;
	};

protected:
	double _dur() { return (double)nSamples / fs*1000.; }// for backward compatibility 5/18. No reason to get rid of it. 10/18/2019
	CSignal & operator<=(CSignal * prhs);
	CSignal & operator%(const CSignal & v); // scale operator (absolute)
	CSignal & operator%(double v); // scale operator (absolute)
	CSignal & operator|(double v); // scale operator (relative)
	CSignal & operator|(const CSignal & RMS2adjust);
	CSignal & operator*(pair<vector<double>, vector<double>> coef);
	pair<unsigned int, unsigned int> grid() const {	return make_pair((unsigned int)round(tmark*fs/1000.), nSamples-1+ (unsigned int)round(tmark*fs / 1000.));	};
	bool overlap(const CSignal &sec);
	function<double(double)> op;
	function<double(double)> op1(double me) { return [me](double you) {return me + you; }; };
	function<double(double)> op2(double me) { return [me](double you) {return me - you; }; };
	function<double(double)> op3(double me) { return [me](double you) {return me * you; }; };
	function<double(double)> op4(double me) { return [me](double you) {return me / you; }; };
	bool operate(const CSignal & sec, char op);

private:
	CSignal & _filter(const vector<double> & num, const vector<double> & den, vector<double> &initialfinal, unsigned int id0 = 0, unsigned int len = 0);
	int operator_prep(const CSignal & sec, unsigned int &idx4op1, unsigned int &idx4op2, unsigned int &offset);

	friend class CSignalExt;
};

class CTimeSeries : public CSignal
{
public:
	CTimeSeries *chain;
	vector<CTimeSeries> outarg;

	int WriteAXL(FILE* fp);
	int IsTimeSignal() const;
	bool operator < (const CTimeSeries & rhs) const;

	CTimeSeries runFct2getvals(vector<double>(CSignal::*)(unsigned int, unsigned int) const, void *popt = NULL);
	CTimeSeries & runFct2modify(CSignal & (CSignal::*)(unsigned int, unsigned int), void *popt = NULL);
	CTimeSeries runFct2getsig(CSignal (CSignal::*)(unsigned int, unsigned int) const, void *popt = NULL);

	CTimeSeries & Reset(int fs2set = 0);
	void SetChain(CTimeSeries *unit, double time_shifted = 0.);
	void SetChain(double time_shifted);
	CTimeSeries & AddChain(const CTimeSeries &sec);
	CTimeSeries * GetDeepestChain();
	CTimeSeries * ExtractDeepestChain(CTimeSeries *deepchain);
	unsigned int CountChains(unsigned int *maxlength=NULL);
	void AddMultChain(char type, CTimeSeries *forthis);
	CTimeSeries * BreakChain(CTimeSeries *chainedout);
	CTimeSeries & MergeChains();
	CTimeSeries & ConnectChains();
	CTimeSeries & MC(CTimeSeries &out, std::vector<double> tmarks, int  id1, int  id2);
	CTimeSeries & reciprocal(void);	// Multiplicative inverse
	CTimeSeries & timeshift(double tp_ms);
	CTimeSeries & removeafter(double timems);
	CTimeSeries & Squeeze();
	CTimeSeries & Crop(double begin_ms, double end_ms);
	CTimeSeries & Insert(double timept, CTimeSeries &newchunk);
	CTimeSeries & ReplaceBetweenTPs(CTimeSeries &newsig, double t1, double t2);
	CTimeSeries & NullIn(double tpoint);
	CSignal *ChainOrd(unsigned int order);

	CTimeSeries & operator=(const CSignal & rhs);
	CTimeSeries & operator=(const CTimeSeries & rhs);
	CTimeSeries & operator<=(const CTimeSeries & rhs);
	CTimeSeries & operator+=(double con);
	CTimeSeries & operator*=(double con);
	CTimeSeries & operator/=(double con);
	CTimeSeries & operator-(void);	// Unary minus
	CTimeSeries & operator*=(CTimeSeries &scaleArray);
	CTimeSeries & operator+=(CTimeSeries *yy); // Concatenation
	CTimeSeries & operator/=(CTimeSeries &scaleArray);
	CTimeSeries & operator>>=(double delta);
	CTimeSeries & operator%(double v);
	CTimeSeries & operator|(double v);
	CTimeSeries & operator|(CTimeSeries * RMS2adjust);
	bool operator==(const CTimeSeries & rhs);

	void setsnap(int set=1) {	for (CTimeSeries*p = this; p; p = p->chain)		p->snap = set;	};
	bool IsAudioOnAt(double timept);
	int GetType() const;
	void SwapContents1node(CTimeSeries &sec);
	CTimeSeries & LogOp(CTimeSeries &rhs, int type);
	void UpSample(int cc);
	void DownSample(int cc);
	void ReverseTime();
	vector<double> tmarks();

	CTimeSeries & each_sym(double(*fn)(double));
	CTimeSeries & each(double(*fn)(double));
	CTimeSeries & each(double(*fn)(complex<double>));
	CTimeSeries & each(double(*fn)(double, double), body &arg2);
	CTimeSeries & each(complex<double>(*fn)(complex<double>));
	CTimeSeries & each(complex<double>(*fn)(complex<double>, complex<double>), body &arg2);
	CTimeSeries & transpose1();

	CTimeSeries & MFFN(double(*fn)(double), complex<double>(*cfn)(complex<double>));
	CTimeSeries & MFFN(double(*fn)(double, double), complex<double>(*cfn)(complex<double>, complex<double>), const CTimeSeries &param);

	CTimeSeries & Modulate(CTimeSeries env);
	CTimeSeries & SAM(double modRate, double modDepth, double initPhase);
	CTimeSeries & GhostCopy(CTimeSeries *pref);

	CTimeSeries * AtTimePoint(double timept);

	map<double, double> showtseries();
	CSignal TSeries2CSignal();

	double MakeChainless();
	
	// Constructors
	CTimeSeries();
	CTimeSeries(int sampleRate); // construct with a specified sample rate.
	CTimeSeries(int sampleRate, unsigned int len); // construct with a specified sample rate and buffer size
	CTimeSeries(double value); // construct a scalar with the specified value ---is it necessary? 5/19/2018
	CTimeSeries(const CSignal & src); 
	CTimeSeries(const CTimeSeries & src);
	virtual ~CTimeSeries();

	double alldur() const;
	bool IsEmpty() const { return chain == nullptr && CSignal::IsEmpty(); }
	bool IsScalar() const;
	bool IsVector() const;
	bool IsAudio() const;
	bool IsString() const;
	bool IsComplex() const;
	bool IsBool() const;

	void SetComplex();
	void SetReal();

	bool operate(const CTimeSeries & sec, char op);
protected:
	CTimeSeries & operator<=(CTimeSeries * prhs);
	CTimeSeries & operator%(CTimeSeries * v);
	CTimeSeries & operator+(CTimeSeries * sec);
	CTimeSeries & operator-(CTimeSeries * sec);

private:
	void sort_by_tmark();
};

class CSignals : public CTimeSeries
{
public:
	CSignals *next;

	bool operator < (const CSignals & rhs) const;

	CSignals runFct2getvals(vector<double>(CSignal::*)(unsigned int, unsigned int) const, void *popt = NULL) ;
	CSignals & runFct2modify(CSignal & (CSignal::*)(unsigned int, unsigned int), void *popt = NULL);
	CSignals runFct2getsig(CSignal(CSignal::*)(unsigned int, unsigned int) const, void *popt = NULL);

	// Constructors
	CSignals();
	CSignals(int sampleRate);
	CSignals(double value);
	CSignals(double *y, int len);
	CSignals(const CTimeSeries & src);
	CSignals(const CSignals & src);
	CSignals(std::string str); // make a string CSignals
	CSignals(bool *b, unsigned int len);
	~CSignals();

	bool operator== (const CSignals & rhs);
	bool operator == (double rhs);
	bool operator == (std::string rhstr);
	CSignals & operator=(const CTimeSeries & rhs);
	CSignals & operator=(const CSignals & rhs);
	CSignals & operator+=(double con);
	const CSignals & operator+=(CSignals *yy);
	CSignals & operator*=(double con);
	CSignals & operator%(const CSignals &targetRMS);
	CSignals & operator%(double v);
	CSignals & operator|(double v);
	CSignals & operator|(const CSignals & RMS2adjust);


	int IsTimeSignal() const;
	int IsStereo() { return 0 + (next!=NULL); }

	inline bool IsEmpty() const { return next == nullptr && CTimeSeries::IsEmpty(); }
	bool IsScalar() const;
	bool IsVector() const;
	bool IsAudio() const;
	bool IsString() const;
	bool IsComplex() const;
	bool IsBool() const;

	void SetComplex();
	void SetReal();

	double MakeChainless();
	void SetValue(double v);
	void SetValue(complex<double> v);	

	int ReadAXL(FILE* fp, bool logical, char *errstr);
	int WriteAXL(FILE* fp);

	int GetType() const;
	int GetTypePlus() const;

	CSignals & RMS(); //to calculate the overall RMS; different from CSignal::RMS()

	void SetNextChan(CSignals *second, bool need2makeghost = false);
	CTimeSeries *DetachNextChan() {CTimeSeries *p=next;next=NULL;return p;}
	CSignals & Reset(int fs2set=0);
	CSignals & reciprocal(void);
	CSignals & operator-(void);
	CSignals & operator<=(const CSignals & rhs); // ghost assignment operator2
	CSignals & operator<=(CSignals * prhs); // ghost assignment operator2
	CSignals & operator>>=(const double delta);
	CSignals & Crop(double begin_ms, double end_ms);
	CSignals & Modulate (CSignals env);
	CSignals & Insert(double timept, CSignals &newchunk);
	CSignals & ReplaceBetweenTPs(CSignals &newsig, double t1, double t2);
	CSignals & LogOp(CSignals &rhs, int type);
	CSignals & SAM(double modRate, double modDepth, double initPhase);

	void setsnap(int set=1) {	CTimeSeries::setsnap(set); if (next) next->CTimeSeries::setsnap(set);	};

	CSignals & NullIn(double tpoint);

	double alldur() const;
	double * Mag();
	CSignal Angle();
	void DownSample(int q);
	void UpSample(int p);

	CSignals & each_sym(double(*fn)(double));
	CSignals & each(double (*fn)(double));
	CSignals & each(double (*fn)(complex<double>));
	CSignals & each(double (*fn)(double, double), body &arg2);
	CSignals & each(complex<double> (*fn)(complex<double>));
	CSignals & each(complex<double> (*fn)(complex<double>, complex<double>), body &arg2);
	CSignals & transpose1();
	CSignals & MFFN(double(*fn)(double), complex<double>(*cfn)(complex<double>));
	CSignals & MFFN(double(*fn)(double, double), complex<double>(*cfn)(complex<double>, complex<double>), const CSignals &param);
	int getBufferLength(double & lasttp, double & lasttp_with_silence, double blockDur) const;
	void nextCSignals(double lasttp, double lasttp_with_silence, CSignals &ghcopy);
	template<typename T>
	int makebuffer(T * outbuffer, int length, double lasttp, double lasttp_with_silence, CSignals &ghcopy)
	{
		int nChan = next == nullptr ? 1 : 2;
		memset(outbuffer, 0, sizeof(T) * length * nChan);
		ghcopy <= *this;
		CSignals * p = &ghcopy;
		if (lasttp == 0. && lasttp_with_silence == 0.)
		{ // BEGINNING WITH A NULL PORTION
			// leave outbuffer alone, but update lasttp and lasttp_with_silence
			lasttp = lasttp_with_silence = tmark;
			if (nChan == 2)
			{
				if (next->tmark < lasttp)
					lasttp = lasttp_with_silence = next->tmark;
			}
		}
		else
		{
			for (int ch = 0; p && ch < 2; ch++, p = (CSignals*)p->next)
			{
				for (CTimeSeries *q = p; q; q = q->chain)
				{
					if (q->tmark > lasttp) break;
					int offset = (int)(q->tmark / 1000. * fs + .5) * nChan;
					if (is_same<T, short>::value)
						for (unsigned int m = 0; m < q->nSamples; m++)
							outbuffer[offset + m * nChan + ch] = (short)(_double_to_24bit(q->buf[m]) >> 8);
					else if (is_same<T, double>::value)
						for (unsigned int m = 0; m < q->nSamples; m++)
							outbuffer[offset + m * nChan + ch] = (T)q->buf[m];
				}
			}
		}
		nextCSignals(lasttp, lasttp_with_silence, ghcopy);
		return length;
	}
	vector<CSignals> outarg2;

#if defined(_WINDOWS) || defined(_WINDLL)

#ifndef NO_SF
	CSignals(const char* wavname);
	int Wavwrite(const char *wavname, char *errstr, std::string wavformat = "");
	int mp3write(const char *filename, char *errstr, std::string wavformat = "");
	int Wavread(const char *wavname, char *errstr);
#endif // NO_SF
#endif //_WINDOWS

protected:
	bool operate(const CSignals & sec, char op);
};

class CVar : public CSignals
{
public:
	vector<CVar> cell;
	vector<CVar *> ptarray;
	map<std::string, CVar> strut;
	map<std::string, vector<CVar *>> struts;

	bool functionEvalRes;

	bool operator < (const CVar & rhs) const;
	bool IsStruct() const { return (!strut.empty() || !struts.empty()); }
	bool IsEmpty() const { return (CSignal::GetType() == CSIG_EMPTY && cell.empty() && strut.empty() && struts.empty()); }
	bool IsAudioObj() const;
	CVar& Reset(int fs2set = 0);
	int GetType() const;
	int GetTypePlus();
	bool IsGO() const;
	void set_class_head(const CSignals & rhs);

	CVar & operator=(const CSignals & rhs);
	CVar & operator=(const CVar& rhs);
	CVar & operator=(CVar * rhs);

	bool operator==(const CVar& rhs);
	bool operator==(std::string rhstr);
	bool operator==(double val);

	CVar & initcell(CVar &sec);
	CVar & appendcell(CVar &sec);
	CVar & setcell(unsigned int id, CVar &sec);
	CVar & length();

	// Constructors
	CVar(const CSignals & src);
	CVar(const CVar& src);
	CVar(CVar * src);
	CVar();
	virtual ~CVar();

	CVar & operator+(const CVar & sec);
	CVar & operator-(const CVar & sec);
	CVar & operator*(const CVar & sec);
	CVar & operator/(const CVar & sec);
	CVar & operator-();
	CVar & operator+=(CVar * psec);
	CVar & operator+=(const CVar & sec);
	CVar & operator-=(const CVar & sec);
	CVar & operator*=(const CVar & sec);
	CVar & operator/=(const CVar & sec);

	uint16_t type() const
	{
		uint16_t out = CSignal::type();
		if (out == TYPEBIT_NULL && next) return ((CVar*)next)->CSignal::type();
		if (!cell.empty())		out += TYPEBIT_CELL;
		else if (!strut.empty())
		{
			out += TYPEBIT_STRUT;
			if (!struts.empty()) out += TYPEBIT_STRUTS;
		}
		else if (fs == 3)		out += TYPEBIT_STRUT + TYPEBIT_STRUTS;
		// fs zero means tseq with relative time, treat it as TYPEBIT_AUDIO
		else if (out == 1 && fs == 0)	out += TYPEBIT_AUDIO;
		return out;
	}
};



#endif // AUX_CLASSES