// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.497
// Date: 12/26/2018
// 
#pragma once

#ifndef SIGPROC
#define SIGPROC


#define PI	3.141592
#define LOG10DIV20 0.1151292546497
//#define round(x)	((int)((x)+((x)<0?-.5:.5)))

#define CSIG_EMPTY		0
#define CSIG_STRING		1
#define CSIG_NULL		2
#define CSIG_SCALAR		4
#define CSIG_CELL		8
#define CSIG_STRUCT		10
#define CSIG_HDLARRAY	12
#define CSIG_HANDLE		15
#define CSIG_VECTOR		17
#define CSIG_AUDIO		65
#define CSIG_TSERIES	81 // Think about this... improve it... 5/24/2018

// Used for communication bet sigproc and xcom (i.e., AstSig.cpp and showvar.cpp)
#define WM__DEBUG		WM_APP+4000 
#define ID_DEFAULT	99
#define ID_DEBUG_STEP 108
#define ID_DEBUG_STEPIN 109
#define ID_DEBUG_CONTINUE 110
#define ID_DEBUG_EXIT 111
#define ID_DEBUG 112

enum DEBUG_STATUS
{
    null=-1,
    entering,
    progress,
	stepping,
	stepping_in,
	continuing,
    exiting,
    cleanup,
	aborting,
	purgatory,
	refresh,
	typed_line,
};

//End of Used for communication bet sigproc and xcom (i.e., AstSig.cpp and showvar.cpp)

#ifdef _WINDOWS
#ifndef _MFC_VER // If MFC is NOT used.
#include <windows.h>

#else 
#include "afxwin.h"
#endif 
#endif 

#include <string>
#include <iostream>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <complex>

#ifndef PSYCONYACC
#include "psycon.yacc.h"
#endif

using namespace std;

class CAstSig;

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

	body();
	body(const body& src);
	body(double value);
	body(complex<double> value);
	body(double *y, int  len);
	body(bool *y, int len);
	virtual ~body();

	body& operator=(const body& rhs);
	body& operator+=(const double con);
	body& operator*=(const double con);
	body& operator/=(const double con);
	body& operator-(void);	// Unary minus

	body& UpdateBuffer(unsigned int length);
	void Reset();

	double value() const;
	complex<double> cvalue() const;
	void SetValue(double v);
	void SetValue(complex<double> v);
	void SetComplex();
	void SetReal();
	void Real();
	void Imag();

	bool IsComplex() const  { return (bufBlockSize==2*sizeof(double)); } 
	void SwapContents1node(body &sec);
	vector<double> ToVector();

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

	double _max(unsigned int id0 = 0, unsigned int len = 0);
	double _min(unsigned int id0 = 0, unsigned int len = 0);
	body & interp1(body &that, body &qp);
	double sum(unsigned int id0=0, unsigned int len = 0);
	double mean(unsigned int id0 = 0, unsigned int len = 0) { if (len == 0) len = nSamples; return sum(id0, len) / (double)len; }
	double stdev(unsigned int id0 = 0, unsigned int len = 0);

protected:
	void* parg;
};

class CSignal : public body
{
public:
	double tmark;
	unsigned int Len() { if (fs == 2) return (nSamples-1) / nGroups; else  return nSamples / nGroups; }

	// Signal generation (no stereo handling)
	double * fm(double midFreq, double fmWidth, double fmRate, int  nsamples, double beginFMPhase=0.);
	double * fm(double midFreq, double fmWidth, double fmRate, double dur_ms, double beginFMPhase=0.);
	double * fm2(CSignal flutter, int  multiplier, char *errstr);
	double * Silence(unsigned int nsamples);
	double * Silence(double dur_ms);
	double * DC(double dur_ms);
	double * DC(unsigned int nsamples);
	double * Tone(vector<double> freqs, unsigned int nsamples);
	double * Tone(vector<double> freqs, double dur_ms);
	double * Tone(double freq, unsigned int nsamples, double beginPhase=0.);
	double * Tone(double freq, double dur_ms, double beginPhase=0.);
	double * Noise(double dur_ms);
	double * Noise(unsigned int nsamples);
	double * Noise2(double dur_ms);
	double * Noise2(unsigned int  nsamples);
	double * Truncate(double time_ms1, double time_ms2);
	double * Truncate(int id1, int id2, int code=0);
	CSignal& Modulate(vector<double> &tpoints, vector<double> &tvals);
	CSignal &matrixmult(CSignal *arg);
	CSignal& Diff(int order=1);
	CSignal& Cumsum();

	// Window functions -- signal generation function
	CSignal& Hamming(unsigned int id0 = 0, unsigned int len = 0);
	CSignal& Blackman(unsigned int id0 = 0, unsigned int len = 0);

	// Signal alteration (stereo handling with a clean, inarguable convention)
	virtual void Filter(unsigned int nTabs, double *num, double *den);
	int DecFir(const CSignal& coeff, int offset, int nChan);
#ifndef NO_FFTW
	CSignal& Hilbert(unsigned int id0 = 0, unsigned int len = 0);
	CSignal& HilbertEnv(unsigned int id0 = 0, unsigned int len = 0);
	CSignal& ShiftFreq(unsigned int id0 = 0, unsigned int len = 0);
#endif
	void DownSample(int q);
	void UpSample(int p);
	CSignal& Reset(int  fs2set=0);
#ifndef NO_RESAMPLE
	double * Resample(int newfs, char *errstr);
	double * Resample(vector<unsigned int> newfs, vector<unsigned int> lengths, char *errstr);
#endif //NO_RESAMPLE
	virtual void Dramp(double dur_ms, int beginID=0);
	void ReverseTime();
	CSignal& Interp(const CSignal& gains, const CSignal& tmarks);
	CSignal& reciprocal(void);	// Multiplicative inverse
	CSignal& operator=(const body& rhs);
	CSignal& operator=(const CSignal& rhs);
	bool operator == (const CSignal &rhs);
	bool operator == (double rhs);
	bool operator == (string rhstr);
	CSignal& operator+=(CSignal *yy); // Concatenation
	virtual CSignal& operator>>=(double delta);

	// Signal alteration (stereo handling with an established convention)

	// Signal extraction (stereo handling with a clean, inarguable convention)
	CSignal& Take(CSignal& out, unsigned int id1, unsigned int id2);

	// Retrieve signal characteristics (single channel ONLY)
	int GetType() const; 
	double RMS(unsigned int id0 = 0, unsigned int len = 0);
	int GetFs() const {return fs; };
	void SetFs(int  newfs) {fs = newfs; };
	double* GetBuffer() {return buf;}
	double length(unsigned int id0 = 0, unsigned int len = 0);
	double dur(unsigned int id0 = 0, unsigned int len = 0);
	double begint(unsigned int id0 = 0, unsigned int len = 0);
	double endt(unsigned int id0 = 0, unsigned int len = 0);
	
	CSignal &_atmost(unsigned int id, int unsigned len);
	CSignal &_atleast(unsigned int id, int unsigned len);
	CSignal& sort(unsigned int id0 = 0, unsigned int len = 0);
	CSignal& conv(unsigned int id0, unsigned int len = 0);
	CSignal& dramp(unsigned int id0 = 0, unsigned int len = 0);
	CSignal& timestretch(unsigned int id0 = 0, unsigned int len = 0);
	CSignal& dynafilter(unsigned int id0 = 0, unsigned int len = 0);
	CSignal& filter(unsigned int id0 = 0, unsigned int len = 0);
	CSignal& filtfilt(unsigned int id0 = 0, unsigned int len = 0);
#ifndef NO_IIR
	CSignal& IIR(unsigned int id0 = 0, unsigned int len = 0); 
#endif // NO_IIR


	bool IsScalar() const { return (GetType() == CSIG_SCALAR); }
	bool IsVector() const {return (GetType() == CSIG_SCALAR || GetType() == CSIG_VECTOR);}
	bool IsEmpty() const {return (GetType() == CSIG_EMPTY);}
	bool IsSingle() const;
	bool IsString() const {return (fs == 2 && nSamples>0);}
	bool IsLogical() const {return (bufBlockSize==1 && fs != 2);} // logical can be either audio or non-audio, so GetType() of logical array will not tell you whether that's logical or not.
	bool IsComplex() const  { return (bufBlockSize==2*sizeof(double)); } 
	int IsNull(double timept);
	CSignal& Modulate(double *env, unsigned int lenEnv, unsigned int beginID=0);
	CSignal& SAM(double modRate, double modDepth, double initPhase);

	CSignal(); // default construct
	CSignal(int  sampleRate); // construct with a specified sample rate.
	CSignal(double value); // construct a scala with the specified value
	CSignal(const CSignal& src); // copy constructor
	CSignal(double *y, int  len);
	CSignal(vector<double> vv); 
	CSignal(FILE* fp);
	CSignal(string str); // make a string CSignal

	virtual ~CSignal();	// standard destructor

	string string();
	char *getString(char *str, const int size);
	CSignal &SetString(const char *str);
	CSignal &SetString(const char c);
	CSignal size(unsigned int id0 = 0, unsigned int len = 0);
	CSignal FFT(unsigned int id0 = 0, unsigned int len = 0);
	CSignal iFFT(unsigned int id0 = 0, unsigned int len = 0);

	double (CSignal::*pf_basic)(unsigned int, unsigned int);
	CSignal& (CSignal::*pf_basic2)(unsigned int, unsigned int);
	CSignal (CSignal::*pf_basic3)(unsigned int, unsigned int);
	CSignal& dynaMA(unsigned int id0, unsigned int len);
	CSignal& dynaAR(unsigned int id0, unsigned int len);

protected:
	double _dur() { return (double)nSamples / fs*1000.; }// for backward compatibility 5/18
	int fs;
private:
	CSignal& _filter(vector<double> num, vector<double> den, unsigned int id0 = 0, unsigned int len = 0);
	CSignal& _dynafilter(CSignal *TSEQ_num, CSignal * TSEQ_den, unsigned int id0 = 0, unsigned int len = 0);
};

class CTimeSeries : public CSignal
{
public:
	CTimeSeries *chain;

	int WriteAXL(FILE* fp);
	int IsTimeSignal();

	CTimeSeries basic(double (CSignal::*pf_basic)(unsigned int, unsigned int), void *popt=NULL);
	CTimeSeries& basic(CSignal& (CSignal::*pf_basic)(unsigned int, unsigned int), void *popt = NULL);
	CTimeSeries basic(CSignal (CSignal::*pf_basic)(unsigned int, unsigned int), void *popt = NULL);

	CTimeSeries Extract(double begin_ms, double end_ms);

	CTimeSeries& Reset(int fs2set = 0);
	void SetChain(CTimeSeries *unit, double time_shifted = 0.);
	void SetChain(double time_shifted);
	CTimeSeries& AddChain(CTimeSeries &sec);
	CTimeSeries * GetDeepestChain();
	CTimeSeries * ExtractDeepestChain(CTimeSeries *deepchain);
	unsigned int CountChains(unsigned int *maxlength=NULL);
	void AddMultChain(char type, CTimeSeries *forthis);
	CTimeSeries * BreakChain(CTimeSeries *chainedout);
	CTimeSeries& MergeChains();
	CTimeSeries& ConnectChains();
	CTimeSeries& MC(CTimeSeries &out, std::vector<double> tmarks, int  id1, int  id2);
	double RMS();  // kept for backward compatibility 5/23/2018
	CTimeSeries& reciprocal(void);	// Multiplicative inverse
	CTimeSeries& timeshift(double tp_ms);
	CTimeSeries& removeafter(double timems);
	CTimeSeries& Squeeze();
	CTimeSeries& Crop(double begin_ms, double end_ms);
	CTimeSeries& Insert(double timept, CTimeSeries &newchunk);
	CTimeSeries& ReplaceBetweenTPs(CTimeSeries &newsig, double t1, double t2);
	CTimeSeries& NullIn(double tpoint);
	CSignal *ChainOrd(unsigned int order);
	CTimeSeries& dynaMA(CTimeSeries &num);
	CTimeSeries& dynaAR(CTimeSeries &den);

	CTimeSeries& operator=(const CSignal& rhs);
	CTimeSeries& operator=(const CTimeSeries& rhs);
	CTimeSeries& operator+=(const double con);
	CTimeSeries& operator*=(const double con);
	CTimeSeries& operator/=(double con);
	CTimeSeries& operator-(void);	// Unary minus
	CTimeSeries& operator*=(CTimeSeries &scaleArray);
	CTimeSeries& operator+=(CTimeSeries &sec);
	CTimeSeries& operator+=(CTimeSeries *yy); // Concatenation
	CTimeSeries& operator-=(CTimeSeries &sec);
	CTimeSeries& operator/=(CTimeSeries &scaleArray);
	CTimeSeries& operator>>=(double delta);
	bool IsNull(double timept);
	int GetType() const;
	void SwapContents1node(CTimeSeries &sec);
	CTimeSeries& LogOp(CTimeSeries &rhs, int type);
	CTimeSeries& Resample(int newfs, char *errstr);
	CTimeSeries& Resample(vector<unsigned int> newfs, vector<unsigned int> lengths, char *errstr);
	void UpSample(int cc);
	void DownSample(int cc);
	void ReverseTime();
	vector<double> tmarks();

	CTimeSeries& each_sym(double(*fn)(double));
	CTimeSeries& each(double(*fn)(double));
	CTimeSeries& each(double(*fn)(complex<double>));
	CTimeSeries& each(double(*fn)(double, double), body &arg2);
	CTimeSeries& each(complex<double>(*fn)(complex<double>));
	CTimeSeries& each(complex<double>(*fn)(complex<double>, complex<double>), body &arg2);
	CTimeSeries& transpose1();

	CTimeSeries& MFFN(double(*fn)(double), complex<double>(*cfn)(complex<double>));
	CTimeSeries& MFFN(double(*fn)(double, double), complex<double>(*cfn)(complex<double>, complex<double>), const CTimeSeries &param);

	CTimeSeries& Modulate(CTimeSeries env);
	CTimeSeries& SAM(double modRate, double modDepth, double initPhase);

	CTimeSeries * AtTimePoint(double timept);

	map<double, double> showtseries();
	CSignal TSeries2CSignal();

	double MakeChainless();
	CTimeSeries();
	CTimeSeries(int sampleRate); // construct with a specified sample rate.
	CTimeSeries(double value); // construct a scalar with the specified value ---is it necessary? 5/19/2018
	CTimeSeries(const CSignal& src); 
	CTimeSeries(const CTimeSeries& src);
	~CTimeSeries();

	double alldur();

	vector<CTimeSeries> outarg;
};

class CSignals : public CTimeSeries
{
public:
	CTimeSeries *next;

	CSignals basic(double (CSignal::*pf_basic)(unsigned int, unsigned int), void *popt = NULL);
	CSignals& basic(CSignal& (CSignal::*pf_basic)(unsigned int, unsigned int), void *popt = NULL);
	CSignals basic(CSignal(CSignal::*pf_basic)(unsigned int, unsigned int), void *popt = NULL);

	CSignals();
	CSignals(bool b);
	CSignals(int sampleRate);
	CSignals(double value);
	CSignals(double *y, int  len);
	CSignals(const CTimeSeries& src);
	CSignals(const CSignals& src);
	CSignals(std::string str); // make a string CSignals
	~CSignals();
	bool operator == (const CSignals& rhs);
	bool operator == (double rhs);
	bool operator == (std::string rhstr);
	CSignals& operator=(const CTimeSeries& rhs);
	CSignals& operator=(const CSignals& rhs);
	CSignals& operator+=(const double con);
	CSignals& operator+=(CTimeSeries &sec);
	CSignals& operator+=(CSignals &sec);
	const CSignals& operator+=(CSignals *yy);
	const CSignals& operator+=(CSignal *yy);
	CSignals& operator*=(CSignals &sec);
	CSignals& operator*=(const double con);
	CSignals& operator*=(CSignal &sec);
	int IsTimeSignal();
	int IsStereo() { return 0 + (next!=NULL); }

	double MakeChainless();
	void SetValue(double v);
	void SetValue(complex<double> v);	

	int ReadAXL(FILE* fp, bool logical, char *errstr);
	int WriteAXL(FILE* fp);

	int GetType() const;
	int GetTypePlus() const;


	CSignals RMS(); // scalar (mono) or 2-element (stereo) output

	void SetNextChan(CTimeSeries *second);
	CTimeSeries *DetachNextChan() {CTimeSeries *p=next;next=NULL;return p;}
	CSignals& Reset(int fs2set=0);
	CSignals& reciprocal(void);
	CSignals& operator-(void);
	CSignals& operator>>=(const double delta);
	CSignals& Take(CSignals& out, int id1, int id2);
	CSignals& Crop(double begin_ms, double end_ms);
	CSignals& Modulate (CSignals env);
	CSignals& Insert(double timept, CSignals &newchunk);
	CSignals& ReplaceBetweenTPs(CSignals &newsig, double t1, double t2);
	CSignals& LogOp(CSignals &rhs, int type);
	CSignals& SAM(double modRate, double modDepth, double initPhase);
	CSignals& Windowing(std::string type, double opt);

	CSignals& NullIn(double tpoint);

	double alldur();
	double * Mag();
	CSignal Angle();
#ifndef NO_RESAMPLE
	double * Resample(int  newfs, char *errstr);
	double * Resample(vector<unsigned int> newfs, vector<unsigned int> lengths, char *errstr);
	double * fm2(CSignal flutter, int  multiplier, char *errstr);
#endif // NO_RESAMPLE
	void DownSample(int q);
	void UpSample(int p);
#ifndef NO_FFTW

#endif
	CSignals &each_sym(double(*fn)(double));
	CSignals &each(double (*fn)(double));
	CSignals &each(double (*fn)(complex<double>));
	CSignals &each(double (*fn)(double, double), body &arg2);
	CSignals &each(complex<double> (*fn)(complex<double>));
	CSignals &each(complex<double> (*fn)(complex<double>, complex<double>), body &arg2);
	CSignals &transpose1();
	CSignals & MFFN(double(*fn)(double), complex<double>(*cfn)(complex<double>));
	CSignals & MFFN(double(*fn)(double, double), complex<double>(*cfn)(complex<double>, complex<double>), const CSignals &param);

	CSignals* getchildren(vector<CSignals*> *out);
	vector<CSignals*> showallchildren();
	vector<CSignals> outarg2;

#ifdef _WINDOWS
#ifndef NO_PLAYSND
	// Sound Playback functions
	INT_PTR PlayArray(int DevID, UINT userDefinedMsgID, HWND hApplWnd, double *block_dur_ms, char *errstr, int loop=1); // playing with event notification by specified time block
	INT_PTR PlayArray(int DevID, UINT userDefinedMsgID, HWND hApplWnd, int  nProgReport, char *errstr, int  loop=1); // full format
	INT_PTR PlayArrayNext(INT_PTR pWP, int DevID, UINT userDefinedMsgID, double *block_dur_ms, char *errstr, int loop = 1);
	INT_PTR PlayArrayNext(INT_PTR pWP, int DevID, UINT userDefinedMsgID, int  nProgReport, char *errstr, int loop = 1); // full format
	INT_PTR PlayArray(int DevID, char *errstr); // (blocking play)
	INT_PTR PlayArray(char *errstr); //assuming the first device
#endif // NO_PLAYSND

#ifndef NO_SF
	CSignals(const char* wavname);
	int Wavwrite(const char *wavname, char *errstr, std::string wavformat="");
	int Wavread(const char *wavname, char *errstr);
#endif // NO_SF
#endif //_WINDOWS

private:
	short * makebuffer(int  &nChan);
};

class CVar : public CSignals
{
public:
	vector<CVar> cell;
	vector<CVar *> ptarray;
	map<std::string, CVar> strut;
	map<std::string, vector<CVar *>> struts;

	bool functionEvalRes;

	bool IsStruct() const { return (!strut.empty() || !struts.empty()); }
	bool IsEmpty() const { return (CSignal::GetType() == CSIG_EMPTY && strut.empty() && struts.empty()); }
	bool IsAudioObj();
	CVar& Reset(int fs2set = 0);
	int GetType();
	int GetTypePlus();
	bool IsGO();

	CVar& operator=(const CSignals& rhs);
	CVar& operator=(const CVar& rhs);
	CVar& initcell(CVar &sec);
	CVar& appendcell(CVar &sec);
	CVar& setcell(unsigned int id, CVar &sec);
	CVar& length();

	CVar(const CSignals& src);
	CVar(const CVar& src);
	CVar();
	~CVar();
};

class CAstException {
public:
	const AstNode *pnode;
	CAstSig *pCtx; // pointer to the context, AKA AstSig, that threw the exception
	string str1, str2, outstr;
	int int1;

	CAstException(const AstNode *p, CAstSig *pAst, const string s1, const string s2 = "");
	CAstException(const AstNode *p, CAstSig *pAst, const string s1, const int x, const string s2="");
	CAstException(const AstNode *p0, CAstSig *past, const char* msg);
	string getErrMsg() const {return outstr;};
	vector<CSignals *> out;
private:
	void makeOutStr();
};

class UDF
{
public:
	AstNode *pAst;
	string fullname;
	string content;
	vector<int> DebugBreaks;
	map<string, UDF> local;
	bool newrecruit;
	UDF& operator=(const UDF& rhs);
	UDF() {	pAst = NULL; newrecruit = false;	};
	~UDF() {};
};

class CBIF // Built-In Function class; also used for pseudo vars
{
public:
	CBIF() { alwaysstatic = false; };
	virtual ~CBIF() {};
	string name;
	string funcsignature;
	bool alwaysstatic; // true means the function cannot be called as a member function
	int narg1, narg2;
};

class CAstSigEnv
{
	friend class CAstSig;
public:
	map<string, UDF> udf;
	int Fs;
	int curLine; // used for control F10
	map<string, vector<CVar *>> glovar;
	string AppPath;
	string AuxPath;
	bool shutdown;
	map<string, void(*)(CAstSig *, const AstNode *, const AstNode *, string &)> inFunc;
	void InitBuiltInFunctionList();
	map<string, CBIF> pseudo_vars;
	vector<CBIF> built_in_funcs;
	vector<string> built_in_func_names;

	CAstSigEnv(const int fs = 1);
	~CAstSigEnv();
	CAstSigEnv& operator=(const CAstSigEnv& rhs);
	CAstSigEnv &SetPath(const char *path);
	CAstSigEnv &AddPath(const char *path);
};

class CDebugStatus
{
public:
	DEBUG_STATUS status;
	bool GUI_running;
	bool inPurgatory;
	bool local;
	CDebugStatus() { status = null;  GUI_running = inPurgatory = local = false; };
	virtual ~CDebugStatus() {};
};

class goaction {
public:
	CVar *psig;
	const char *type;
	CVar RHS;
	bool frozen;
	void clear();
	goaction() { frozen = NULL;  psig = NULL;  type = NULL; };
	virtual ~goaction() {};
} ;

class CFuncPointers
{
public:
	void(*UpdateDebuggerGUI)(CAstSig *debugAstSig, DEBUG_STATUS debug_status, int line); // debug_appl_manager in xcom
	void(*HoldAtBreakPoint)(CAstSig *pastsig, const AstNode *pnode);
	bool(*IsCurrentUDFOnDebuggerDeck)(const char*);
	void(*ShowVariables)(CAstSig *pcast);
	void(*Back2BaseScope)(int);
	void(*UnloadModule)(const char*);
	void(*ValidateFig)(const char*);
	void(*SetGoProperties)(CAstSig *past, const char *type, CVar RHS);
	CFuncPointers();
	virtual ~CFuncPointers() {};
	CFuncPointers& operator=(const CFuncPointers& rhs);
};

class NODEDIGGER
{
public:
	NODEDIGGER() { side = ' '; pgo = psigBase = NULL; };
	virtual ~NODEDIGGER() {};
	AstNode *root;
	char side;
	CVar *psigBase;
	CVar Sig;
	CVar *pgo;
};

class CUDF
{
public:
	string title;
	string base;
	AstNode *pUDF; // T_FUNCTION node
	AstNode *pUDF_base; // T_FUNCTION node for base udf
	vector<string> argout; // formal output argument list; to be filled out in PrepareAndCallUDF()
	int nextBreakPoint;
	int currentLine;
	int nargin, nargout;
	CDebugStatus debug;
	const char* application;
	CUDF() {	application = nullptr;  nextBreakPoint = currentLine = -1;		CallbackCIPulse = NULL; CallbackHook = NULL;	};
	virtual ~CUDF() {};
	void(*CallbackCIPulse)(const AstNode *, CAstSig *);
	int(*CallbackHook)(CAstSig *past, const AstNode *pnode, const AstNode *p);
};

class CAstSig
{
public:
#ifndef GRAFFY
	static void cleanup_nodes(CAstSig *beginhere = NULL);
	static void print_links0(FILE *fp, AstNode *p);
	static void print_links(const char *filename, AstNode *pnode);
	static vector<CAstSig*> vecast;
	static bool graffyPrepared;
	static bool IsStatement(const AstNode *p);
	static bool IsCondition(const AstNode *p);
	static bool IsLooping(const AstNode *p);
	static bool IsTID(const AstNode *p);
	static bool IsBLOCK(const AstNode *p);
	static bool IsVECTOR(const AstNode *p);
	static bool IsSTRUCT(const AstNode *p);
	static bool IsPortion(const AstNode *p);
	static bool IsCELL_STRUCT_pnode_TID_ARGS(const AstNode *pnode, const AstNode *p);
	static bool Var_never_updated(const AstNode *p);
	static AstNode *goto_line(const AstNode *pnode, int line);
	static CVar *HandleSig(CVar *ptarget, CVar *pGraffyobj);
	static AstNode *findParentNode(AstNode *p, AstNode *pME, bool altonly = false);
	static char *showGraffyHandle(char *out, CVar *pvar);
#endif //GRAFFY
	map<string, CVar> Vars;
	map<string, vector<CVar *>> GOvars;
	AstNode *pAst;
	CVar Sig;
	CVar *pgo;
	string Script;
	CAstSigEnv *pEnv;
	CUDF u;
	vector<CSignals> outarg;
	AstNode *lhs;
	CAstSig *son;
	CAstSig *dad;
	const AstNode *pLast;
	string statusMsg; // to display any message during processing inside of AstSig.cpp in the application
	unsigned long Tick0, Tick1;
	bool fBreak, fAllocatedAst;
	bool FsFixed;
	CFuncPointers fpmsg; // function pointer "messenger"
	void init();
	static const int DefaultFs = 22050;

	CVar replica;
	double endpoint;
	bool fExit, fContinue;
	double audio_block_ms;
	goaction setgo;

private:
	void HandleAuxFunctions(const AstNode *pnode, AstNode *pRoot = NULL);
	bool HandlePseudoVar(const AstNode *pnode);
	int HandleMathFunc(bool compl, string &fname, double(**)(double), double(**)(double), double(**)(double, double), double(**)(complex<double>), complex<double>(**)(complex<double>), complex<double>(**)(complex<double>, complex<double>));
	bool IsValidBuiltin(string funcname);
	CAstSig &insertreplace(const AstNode *pnode, CTimeSeries *inout, CVar &sec, CVar &indsig);
	void checkindexrange(const AstNode *pnode, CTimeSeries *inout, unsigned int id, string errstr);
	int checkpositiveinteger(const AstNode *pnode, CVar *id);
	bool isContiguous(body &id, unsigned int &begin,unsigned int &end);
	AstNode *searchtree(AstNode *pp, int type);
	CVar &extract(char *errstr, CVar **pinout, body &isig);
	bool checkcond(const AstNode *p);
	void hold_at_break_point(const AstNode *pnode);
	void prepare_endpoint(const AstNode *p, CVar *pvar);
	bool builtin_func_call(NODEDIGGER &ndog, AstNode *p);
	CVar &TID_RHS2LHS(const AstNode *pnode, AstNode *p, AstNode *pRHS, CVar *psig, CVar *psigBase);
	CVar &TID_indexing(AstNode *p, AstNode *pRHS, CVar *psig, CVar *psigBase);
	CVar &TID_tag(const AstNode *pnode, AstNode *p, AstNode *pRHS, CVar *psig, CVar *psigBase=NULL);
	CVar &TID_cell(AstNode *p, AstNode *pRHS, CVar *psig);
	CVar &TID_time_extract(const AstNode *pnode, AstNode *p, AstNode *pRHS, CVar *psig);
	CVar &TID_condition(const AstNode *pnode, AstNode *pLHS, AstNode *pRHS, CVar *psig, CVar *psigBase);
	AstNode *read_node(NODEDIGGER &ndog, AstNode *pn);
	AstNode *read_nodes(NODEDIGGER &ndog);
	AstNode *read_node_4_clearvar(NODEDIGGER &ndog, AstNode **pn);
	CVar &eval_indexing(const AstNode *pInd, CVar *psig, CVar &indSig);
	void interweave_indices(CVar &isig, CVar &isig2, unsigned int len);
	void index_array_satisfying_condition(CVar &isig);
	void replica_prep(CVar *psig);
	CTimeSeries &replace(const AstNode *pnode, CTimeSeries *pobj, body &sec, int id1, int id2);
	CTimeSeries &replace(const AstNode *pnode, CTimeSeries *pobj, body &sec, body &index);

public:
	CVar &ConditionalOperation(const AstNode *pnode, AstNode *p);
	int updateGO(CVar &ref);
	void ClearVar(AstNode *pnode, CVar *psig);
	CVar &pseudoVar(const AstNode *pnode, AstNode *p, CSignals *pout = NULL);
	CVar &TSeq(const AstNode *pnode, AstNode *p);
	bool isThisAllowedPropGO(CVar *psig, const char *type, CVar &tsig);
	void astsig_init(void(*fp1)(CAstSig *, DEBUG_STATUS, int), void(*fp2)(CAstSig *, const AstNode *), bool(*fp3)(const char *), void(*fp4)(CAstSig *), void(*fp5)(int), void(*fp6)(const char*), void(*fp6a)(const char*), void(*fp7)(CAstSig *, const char *, CVar));
	bool IsThisBreakpoint(const AstNode *pnode);
	void checkAudioSig(const AstNode *pnode, CVar &checkthis, string addmsg="");
	void checkComplex (const AstNode *pnode, CVar &checkthis);
	void checkVector(const AstNode *pnode, CVar &checkthis, string addmsg = "");
	void checkSignal(const AstNode *pnode, CVar &checkthis, string addmsg="");
	void checkScalar(const AstNode *pnode, CVar &checkthis, string addmsg = "");
	void checkString(const AstNode *pnode, CVar &checkthis, string addmsg="");
	void blockCell(const AstNode *pnode, CVar &checkthis);
	void blockScalar(const AstNode *pnode, CVar &checkthis);
	void blockString(const AstNode *pnode, CVar &checkthis);
	void blockComplex(const AstNode *pnode, CVar &checkthis);
	const AstNode *getparentnode(const AstNode *pnode, const AstNode *p);
	bool PrepareAndCallUDF(const AstNode *pnode, CVar *pBase = NULL);
	size_t CallUDF(const AstNode *pnode4UDFcalled, CVar *pBase=NULL);
	string LoadPrivateUDF(HMODULE h, int id, string &emsg);

    CAstSig(const CAstSig &org);
	CAstSig(const CAstSig *src);
	CAstSig(const char *str, const CAstSig *src);
	CAstSig(AstNode *pNode, const CAstSig *src);
	CAstSig(CAstSigEnv *env);
	CAstSig(const char *str, CAstSigEnv *env);
	CAstSig(AstNode *pNode, CAstSigEnv *env);
	~CAstSig();

	AstNode *SetNewScript(string &emsg, const char *str, const char *premsg = NULL);
	AstNode *SetNewScriptFromFile(string &emsg, const char *full_filename, const char *str, string &filecontent);
	vector<CVar> Compute(void);
	CVar &Compute(const AstNode *pnode);
	CVar &InitCell(const AstNode *pnode, AstNode *p);
	CVar &SetLevel(const AstNode *pnode, AstNode *p);
	CVar &NodeVector(const AstNode *pnode, AstNode *p);
	CVar &NodeMatrix(const AstNode *pnode, AstNode *p);
	CVar &define_new_variable(const AstNode *pnode, AstNode *pRHS);
	CVar *GetGlobalVariable(const AstNode *pnode, const char *varname, CVar *pvar = NULL);
	CVar *GetVariable(const char *varname, CVar *pvar = NULL);
	CVar *GetGOVariable(const char *varname, CVar *pvar = NULL);
	CVar *GetGloGOVariable(const char *varname, CVar *pvar);
	CVar *MakeGOContainer(vector<CVar *> GOs);
	CVar *MakeGOContainer(vector<INT_PTR> GOs);
	CVar &TID(AstNode *pnode, AstNode *p, CVar *psig=NULL);
	CVar &ExtractByIndex(const AstNode *pnode, AstNode *p, CVar **psig = NULL);
	CVar &TimeExtract(const AstNode *pnode, AstNode *p, CVar *psig);
	CVar &Eval(AstNode *pnode);
	CVar &Transpose(const AstNode *pnode, AstNode *p);
	CAstSig &Reset(const int fs = 0, const char* path=NULL);
	CAstSig &SetVar(const char *name, CVar *psig, CVar *pBase = NULL);
	CAstSig &SetGloVar(const char *name, CVar *psig, CVar *pBase = NULL);
	const char *GetPath() {return pEnv->AuxPath.c_str();}
	int GetFs(void) {return pEnv->Fs;}
	string ComputeString(const AstNode *p);
	string GetScript() {return Script;}
	void set_interrupt(bool ch);
	bool isInterrupted(void);
	CVar &gettimepoints(const AstNode *pnode, AstNode *p);
	unsigned long tic();
	unsigned long toc();

	int isthislocaludf(void);
	int isthislocaludf(string fname);

	int ClearVar(const char *var, CVar *psigBase=NULL);
	void EnumVar(vector<string> &var);
	CVar *GetSig(const char *var);

	string MakeFilename(string fname, const string ext);
	FILE *OpenFileInPath(string fname, string ext, string &fullfilename);
	AstNode *ReadUDF(string &emsg, const char *udf_filename, const char *internaltransport=NULL);
	AstNode *RegisterUDF(const AstNode *pnode, const char *udf_filename, string &filecontent);
	int checkNumArgs(const AstNode *pnode, const AstNode *p, std::string &FuncSigs, int minArgs, int maxArgs);
	int checkNumArgs(const AstNode *pnode, const AstNode *p, std::string &FuncSigs, int *args);
};




#endif // SIGPROC