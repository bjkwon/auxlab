#include <algorithm>
#include "sigproc.h"

inline static double _getdB(double x)
{
	// 3 dB is added to make rms of full scale sinusoid 0 dB
	return 20 * log10(x) + 3.0103;
}

void _arraybasic(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	CVar additionalArg(past->Sig.GetFs());
	string fname = pnode->str;
	if (fname == "sum")	past->Sig = past->Sig.fp_getval(&CSignal::sum);
	else if (fname == "mean") past->Sig = past->Sig.fp_getval(&CSignal::mean);
	else if (fname == "length")
	{
		if (past->Sig.next)
		{
			if (past->Sig.next->nSamples != past->Sig.nSamples)
				throw CAstException(USAGE, *past, pnode).proc("A stereo signal with different lengths for L and R.");
		}
		if (past->Sig.IsGO())
		{
			double out = past->Sig.length().buf[0];
			past->Sig.Reset();
			past->pgo = NULL;
			past->Sig.SetValue(out);
		}
		else
		{
			if (!past->Sig.cell.empty())
				past->Sig.SetValue((double)past->Sig.cell.size());
			else if (!past->Sig.chain)
				past->Sig = past->Sig.fp_getval(&CSignal::length);
			else if (past->Sig.IsTimeSignal())
			{
				past->Sig.SetValue((double)(past->Sig.CountChains()));
			}
			else
			{
				throw CAstException(INTERNAL, *past, pnode).proc("_arraybasic--if chained, IsTimeSignal() should return true.");
			}
			past->Sig.SetFs(1);
		}
	}
	else
	{
		if (past->Sig.IsEmpty()) {
			if (fname == "begint" || fname == "endt" || fname == "dur")
				past->Sig = CVar(0.);
			else
				past->Sig = CVar(-std::numeric_limits<double>::infinity());
			return;
		}
		past->checkAudioSig(pnode, past->Sig);
		if (fname == "begint") past->Sig = past->Sig.fp_getval(&CSignal::begint);
		else if (fname == "endt") past->Sig = past->Sig.fp_getval(&CSignal::endt);
		else if (fname == "dur") past->Sig = past->Sig.fp_getval(&CSignal::dur);
		else if (fname == "rms") past->Sig = past->Sig.fp_getval(&CSignal::RMS);
		else if (fname == "rmsall") past->Sig = past->Sig.RMS(); // overall RMS from artificially concatenated chain's 
	}
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}

void _std(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	CVar arg(0.);
	if (p)
	{
		try {
			CAstSig tp(past);
			arg = tp.Compute(p);
			tp.checkScalar(pnode, arg);
			double flag = tp.Sig.value();
			if (flag != 0. && flag != 1.)
				throw CAstException(FUNC_SYNTAX, *past, p).proc("Invalid parameter: should be either 0 (divided by n-1; default) or 1 (divided by n).");
		}
		catch (const CAstException & e) { throw CAstException(FUNC_SYNTAX, *past, pnode).proc(e.getErrMsg().c_str()); }
	}
		past->Sig = past->Sig.fp_getval(&CSignal::stdev, &arg);
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}

void _size(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	CVar arg(0.);
	if (p)
	{
		try {
			CAstSig tp(past);
			arg = tp.Compute(p);
			tp.checkScalar(pnode, arg);
		}
		catch (const CAstException & e) { throw CAstException(FUNC_SYNTAX, *past, pnode).proc(e.getErrMsg().c_str()); }
	}
	double tp1 = past->Sig.nGroups;
	double tp2 = past->Sig.Len();
	if (arg.value() == 0.)
	{
		if (!past->Sig.type())
			tp1 = 0.;
		past->Sig.UpdateBuffer(2);
		past->Sig.buf[0] = tp1;
		past->Sig.buf[1] = tp2;
		past->Sig.nGroups = 1;
	}
	else if (arg.value() == 1.)
		past->Sig.SetValue(tp1);
	else if (arg.value() == 2.)
		past->Sig.SetValue(tp2);
	else
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("Invalid parameter: should be either 1 or 2.");
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}

double body::sum(unsigned int id0, unsigned int len, void *p) const
{
	if (len == 0) len = nSamples;
	double _sum = 0.;
	int isum(0);
	if (bufBlockSize == 1)
	{
		for (unsigned int k = id0; k < id0 + len; k++)		isum += logbuf[k];
		_sum = (double)isum;
	}
	else
		for (unsigned int k = id0; k < id0 + len; k++)		_sum += buf[k];
	return _sum;
}

double body::mean(unsigned int id0, unsigned int len, void *p) const
{
	if (len == 0) len = nSamples;
	return sum(id0, len) / len;
}
double body::stdev(unsigned int id0, unsigned int len, void *p) const
{
	double out;
	double flag = *(double*)p;
	if (len == 0) len = nSamples;
	if (!len) throw "Empty array";
	double sqsum(0.);
	for (unsigned int k = id0; k < id0 + len; k++)		sqsum += buf[k] * buf[k];
	double mm = mean(id0, len);
	if (flag == 1.)
	{
		out = sqsum / len - mm * mm;
	}
	else
	{
		out = (sqsum + mm * mm * len - 2 * mm * sum(id0, len)) / (len - 1);
	}
	return sqrt(out);
}

double CSignal::begint(unsigned int id0, unsigned int len, void *p) const
{
	return tmark + id0 * 1000. / fs;
}

double CSignal::endt(unsigned int id0, unsigned int len, void *p) const
{
	return begint(id0, len) + dur(id0, len);
}

double CSignal::length(unsigned int id0, unsigned int len, void *p) const
{
	if (len == 0) len = nSamples;
	if (GetType() == CSIG_STRING)
		return (double)strlen(strbuf);
	else
		return (double)len;
}

double CSignal::dur(unsigned int id0, unsigned int len, void *p) const
{
	if (len == 0) len = nSamples;
	return 1000. / fs * len;
}

double CSignal::RMS(unsigned int id0, unsigned int len, void *p) const
{
	if (len == 0) len = nSamples;
	if (len == 0) return std::numeric_limits<double>::infinity();
	double val(0);
	for_each(buf + id0, buf + id0 + len, [&val](double& v) {val += v * v; });
	return _getdB(sqrt(val / len));
}

static double RMS_concatenated(const CTimeSeries & sig)
{
	// Compute the "overall" RMS of entire chain as if all chains were concatenated.
	// input sig represents RMS value of each chain (nSamples of each chain is 1)
	// nGroups information is ignored.
	auto cum = 0.;
	unsigned int len = 0;
	for (auto p = &sig; p; p = p->chain)
	{
		cum += pow(10, (p->value() - 3.0103) / 10.) * p->nSamples;
		len += p->nSamples;
	}
	return 10. * log10(cum / len) + 3.0103;
}

CSignals& CSignals::RMS()
{ // calculating the RMS of the entire CSignals as if all chain's were concatenated.
	// CAUTION--This function will replace the existing data with computed RMS.
	CSignals rmsComputed = fp_getval(&CSignal::RMS);
	// at this point rmsComputed is chain'ed with next (also possibly chain'ed) and nSamples = 1 for each of them 
	CSignals out(1);
	double rmsnow, rmsnow2;
	rmsnow = RMS_concatenated(rmsComputed);
	out.SetValue(rmsnow);
	if (rmsComputed.next)
	{
		rmsnow2 = RMS_concatenated(*rmsComputed.next);
		CSignals* tp = new CSignals(rmsnow2);
		out.SetNextChan(tp);
	}
	return *this = out;
}
