#if !defined(AUX_BUILTIN_EXT)
#define AUX_BUILTIN_EXT 1111

#endif
#include "aux_classes.h"

class CSignalExt : public CSignal
{
public:
	CSignalExt() { fs = 1; tmark = 0.; };
	CSignalExt(const CSignal &base)
	{
		if (base.nSamples > 0)
			body::operator=(base);
		fs = base.fs;
		tmark = base.tmark;
	};
	~CSignalExt();
	
	CSignalExt& operator=(CSignal& rhs);
	CSignal& timestretch(unsigned int id0 = 0, unsigned int len = 0);
	CSignal& pitchscale(unsigned int id0 = 0, unsigned int len = 0);
	CSignal& resample(unsigned int id0 = 0, unsigned int len = 0);

	CSignal& operator=(const CSignalExt& rhs);



	CSignal& (CSignalExt::*pf_exe2)(unsigned int, unsigned int);
};

class CTimeSeriesExt : public CSignalExt
{
public:
	CTimeSeriesExt *chain;
	CTimeSeriesExt() { chain = NULL; };
	~CTimeSeriesExt() {};

	CTimeSeriesExt& basic(CSignal& (CSignalExt::*pf_exe2)(unsigned int, unsigned int), void *popt = NULL);
};

class CSignalsExt : public CTimeSeriesExt
{
public:
	CTimeSeriesExt *next;
	CVar ref;
	CSignalsExt(const CSignals &base)
	{
		fs = base.GetFs();
		tmark = base.tmark;
		next = (CTimeSeriesExt*)base.next;
		chain = (CTimeSeriesExt *)base.chain;
		if (base.nSamples > 0)
			body::operator=(base);
	};
	~CSignalsExt() {};
	CVar &make_CVar();

	CSignalsExt& basic(CSignal& (CSignalExt::*pf_exe2)(unsigned int, unsigned int), void *popt = NULL);
};