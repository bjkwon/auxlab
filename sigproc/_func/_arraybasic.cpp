#include <algorithm>
#include "sigproc.h"

inline static double _getdB(double x)
{
	// 3 dB is added to make rms of full scale sinusoid 0 dB
	return 20 * log10(x) + 3.0103;
}

void _arraybasic(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	CVar additionalArg(past->Sig.GetFs());
	string fname = pnode->str;
	if (fname == "sum")	past->Sig = past->Sig.runFct2getvals(&CSignal::sum);
	else if (fname == "mean") past->Sig = past->Sig.runFct2getvals(&CSignal::mean);
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
				past->Sig = past->Sig.runFct2getvals(&CSignal::length);
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
		if (fname == "begint") past->Sig = past->Sig.runFct2getvals(&CSignal::begint);
		else if (fname == "endt") past->Sig = past->Sig.runFct2getvals(&CSignal::endt);
		else if (fname == "dur") past->Sig = past->Sig.runFct2getvals(&CSignal::dur);
		else if (fname == "rms") past->Sig = past->Sig.runFct2getvals(&CSignal::RMS);
		else if (fname == "rmsall") past->Sig = past->Sig.RMS(); // overall RMS from artificially concatenated chain's 
	}
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}

void _std(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	CVar arg(0.);
	if (p)
	{
		try {
			CAstSig tp(past);
			arg = tp.Compute(p);
			tp.checkScalar(pnode, arg);
			double flag = tp.Sig.value();
			if (flag != 0. && flag != 1.)
				throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "Invalid parameter: should be either 0 (divided by n-1; default) or 1 (divided by n).");
		}
		catch (const CAstException & e) { throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, e.getErrMsg().c_str()); }
	}
	past->Sig = past->Sig.runFct2getvals(&CSignal::stdev, &arg);
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}

void _size(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	CVar arg(0.);
	if (p)
	{
		try {
			CAstSig tp(past);
			arg = tp.Compute(p);
			tp.checkScalar(pnode, arg);
		}
		catch (const CAstException & e) { throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, e.getErrMsg().c_str()); }
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
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, "Invalid parameter: should be either 1 or 2.");
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}

vector<double> body::sum(unsigned int id0, unsigned int len) const
{
	vector<double> out;
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
	out.push_back(_sum);
	return out;
}

vector<double> body::mean(unsigned int id0, unsigned int len) const
{
	if (len == 0) len = nSamples;
	vector<double> out(1, sum(id0, len).front() / len);
	return out;
}
vector<double> body::stdev(unsigned int id0, unsigned int len) const
{
	vector<double> out;
	CVar param = *(CVar*)parg;
	double flag = param.value();
	if (len == 0) len = nSamples;
	if (!len) throw "Empty array";
	double sqsum(0.);
	for (unsigned int k = id0; k < id0 + len; k++)		sqsum += buf[k] * buf[k];
	double mm = mean(id0, len).front();
	if (flag == 1.)
	{
		double var = sqsum / len - mm * mm;
		out.push_back(sqrt(var));
		return out;
	}
	else
	{
		double var = (sqsum + mm * mm * len - 2 * mm * sum(id0, len).front()) / (len - 1);
		out.push_back(sqrt(var));
		return out;
	}
}

vector<double> CSignal::begint(unsigned int id0, unsigned int len) const
{
	vector<double> out(1, tmark + id0 * 1000. / fs);
	return out;
}

vector<double> CSignal::endt(unsigned int id0, unsigned int len) const
{
	vector<double> out(1, begint(id0, len).front() + durc(id0, len).front());
	return out;
}

vector<double> CSignal::length(unsigned int id0, unsigned int len) const
{
	vector<double> out;
	if (len == 0) len = nSamples;
	if (GetType() == CSIG_STRING)
		out.push_back((double)strlen(strbuf));
	else
		out.push_back(len);
	return out;
}

vector<double> CSignal::dur(unsigned int id0, unsigned int len) const
{
	if (len == 0) len = nSamples;
	vector<double> out(1, 1000. / fs * len);
	return out;
}

vector<double> CSignal::RMS(unsigned int id0, unsigned int len) const
{
	if (len == 0) len = nSamples;
	if (len == 0) {
		vector<double> out(1, std::numeric_limits<double>::infinity());
		return out;
	}
	double val(0);
	for_each(buf + id0, buf + id0 + len, [&val](double& v) {val += v * v; });
	//for (unsigned int k = id0; k < id0 + len; k++)
	//	val += buf[k] * buf[k];
	vector<double> out(1, _getdB(sqrt(val / len)));
	return out;
}

CSignals& CSignals::RMS()
{ // calculating the RMS of the entire CSignals as if all chain's were concatenated.
	// CAUTION--This function will replace the existing data with computed RMS.
	CSignals rmsComputed = runFct2getvals(&CSignal::RMS);
	// at this point rmsComputed is chain'ed with next (also possibly chain'ed) and nSamples = 1 for each of them 
	CSignals out(1);
	CSignals* q = &rmsComputed;
	CTimeSeries* pout = &out, * psig = this;
	for (int k = 0; q && k < 2; k++)
	{  // psig is just a copy of the sig, used to get nSamples info
		double cum = 0;
		unsigned int len = 0;
		if (psig->chain)
		{
			for (CTimeSeries* p = q; p; p = p->chain, psig = psig->chain)
			{
				double P = pow(10, (p->value() - 3.0103) / 10.);
				if (psig->chain)
					cum += P * psig->nSamples;
				len += psig->nSamples;
			}
			if (len > 0)
				pout->SetValue(10. * log10(cum / len) + 3.0103);
			else
				pout->SetValue(q->value());
		}
		else
		{ // matrix'ed audio signal (not real chains, just separate rows)
			pout->SetValue(q->value());
		}
		if (k == 0 && (q = (CSignals*)q->next) != nullptr)
		{
			out.SetNextChan(new CSignals(1));
			pout = ((CSignals*)pout)->next;
			psig = next;
		}
	}
	return *this = out;
}

