#include "sigplus_internal.h"
#include <algorithm>

bool CSignal::overlap(const CSignal &sec)
{
	if (sec.grid().first > grid().second || sec.grid().second < grid().first)
		return false;
	return true;
}

int CSignal::operator_prep(const CSignal& sec, unsigned int &idx4op1, unsigned int &idx4op2, unsigned int &offset)
{//The case of scalar sec is handled separately. Here we only separately allow the case of *this scalar.
	if (nSamples > 1 && fs != sec.fs) throw "lhs and rhs must have the same fs";
	// if this and sec are do not overlap, return here
	if (sec.grid().first > grid().second || sec.grid().second < grid().first)
		return 0;
	auto i1 = grid().first;
	auto i2 = sec.grid().first;
	auto f1 = grid().second;
	auto f2 = sec.grid().second;
	int offset2copy = 0, count2add = 0;
	if (f1 < f2)
		count2add += f2 - f1;
	if (i2 < i1)
	{
		count2add += (offset2copy = i1 - i2);
		tmark = sec.tmark;
	}
	UpdateBuffer(nSamples + count2add, offset2copy);
	nGroups = sec.nGroups;
	if (i1 == 0 && i2 == 0 && f2 == 0)
	{
		double val = buf[0];
		for_each(buf + 1, buf + nSamples, [val](double &v) { v = val; });
	}
	if (i2 < i1)
	{
		memcpy(buf, sec.buf, (i1 - i2)*bufBlockSize);
		offset = (idx4op1 = i1 - i2) ;
	}
	else
	{
		offset = 0;
		idx4op1 = i2 - i1;
	}
	idx4op2 = min(f1, f2) - min(i1, i2) + 1;
	if (f1 < f2)
		memcpy(buf + idx4op2, sec.buf + f1 - i2 + 1, (f2 - f1)*bufBlockSize);
	return 1;
}

void CTimeSeries::sort_by_tmark()
{
	if (!chain) return;
	CTimeSeries temp;
	temp <= *this;
	vector<const CTimeSeries*> chains;
	chains.push_back((const CTimeSeries*)&temp);
	for (const CTimeSeries *p = chain; p; p = p->chain)
		chains.push_back(p);

	std::sort(chains.begin(), chains.end(), [](const CTimeSeries *p, const CTimeSeries *q) 
	{return p->tmark < q->tmark; });

	auto it = chains.begin();
	if (this != *it)
	{
		buf = (*it)->buf;
		nSamples = (*it)->nSamples;
		nGroups = (*it)->nGroups;
		tmark = (*it)->tmark;
	}
	for (CTimeSeries *p = this; p; p = p->chain)
	{
		++it;
		if (it == chains.end())
			p->chain = nullptr;
		else
		{
			if (*it!=&temp)
				p->chain = (CTimeSeries*)*it;
			else
			{
				p->chain->tmark = temp.tmark;
				p->chain->buf = temp.buf;
				p->chain->nSamples = temp.nSamples;
				p->chain->nGroups = temp.nGroups;
			}
		}
	}
}

bool CSignal::operate(const CSignal& sec, char op)
{
	// Exception handling is yet to be done 3/8/2019
	  //Currently only for real arrays. 3/8
	// If fs for one is 1 and for the other is >3 (such as 44100)
	// make fs for this the big number
	if (fs == 1 && sec.fs > 3) fs = sec.fs;
	if (sec.IsScalar())
	{
		double val = sec.value();
		if (op == '+')
			for_each(buf, buf + nSamples, [val](double &v) { v += val; });
		else if (op == '-')
			for_each(buf, buf + nSamples, [val](double &v) { v -= val; });
		else if (op == '*')
			for_each(buf, buf + nSamples, [val](double &v) { v *= val; });
		else if (op == '/')
			for_each(buf, buf + nSamples, [val](double &v) { v /= val; });
	}
	else
	{
		unsigned int offset, idBegin, idEnd;
		if (!operator_prep(sec, idBegin, idEnd, offset)) // if not overlapping, just skip
			return false;
		int k = 0;
		double *secbuffer = sec.buf + offset;
		if (op == '+')
			for_each(buf + idBegin, buf + idEnd, [secbuffer, &k](double &v) { v += secbuffer[k++]; });
		else if (op == '-')
			for_each(buf + idBegin, buf + idEnd, [secbuffer, &k](double &v) { v -= secbuffer[k++]; });
		else if (op == '*')
			for_each(buf + idBegin, buf + idEnd, [secbuffer, &k](double &v) { v *= secbuffer[k++]; });
		else if (op == '/')
			for_each(buf + idBegin, buf + idEnd, [secbuffer, &k](double &v) { v /= secbuffer[k++]; });
	}
	return true;
}

bool CTimeSeries::operate(const CTimeSeries& sec, char op)
{
	if (sec.nSamples == 1 && !sec.chain)
	{
		for (CTimeSeries *p = this; p; p = p->chain)
			p->CSignal::operate(sec, op);
		return true;
	}
	else if (nSamples == 1 && !chain)
	{
		CTimeSeries tp = *this;
		*this = sec;
		return operate(tp, op);
	}
	vector<const CTimeSeries*> sec_chains;
	for (const CTimeSeries *q = &sec; q; q = q->chain)
		sec_chains.push_back(q);
	for (auto &q : sec_chains)
		((CTimeSeries*)q)->chain = nullptr;
	for (auto it_chain = sec_chains.begin(); it_chain != sec_chains.end(); )
	{
		bool need2add = false;
		bool checker = true;
		for (CTimeSeries *p = this; p && it_chain != sec_chains.end(); p = p->chain)
		{
			if (p->CSignal::operate(**it_chain, op))
			{
				checker &= false;
				it_chain = sec_chains.erase(it_chain);
			}
			else
				checker &= true;
		}
		if (checker)
			it_chain++;
		checker = false;
	}
	for (auto q : sec_chains)
		AddChain(*q);
	// Unite overlapping chains
	for (CTimeSeries *q = this; q; /*q = q->chain; -->shouldn't be here because p can be removed from the chain*/)
	{
		bool autoupdate = true;
		for (CTimeSeries *p = q; p && p->chain; p = p->chain)
		{
			if (q->overlap(*(p->chain)))
			{
				q->CSignal::operate(*(p->chain), op);
				p->chain = p->chain->chain; //			remove p from the chain
				autoupdate = false;
			}
		}
		if (autoupdate)
			q = q->chain;
	}
	//at this point, no overlapping chains
	sort_by_tmark();
	return true;
}
bool CSignals::operate(const CSignals& sec, char op)
{
	CTimeSeries::operate(sec, op);
	if (next)
	{
		if (sec.next)
			((CSignals*)next)->CTimeSeries::operate(*sec.next, op);
		else
			((CSignals*)next)->CTimeSeries::operate(sec, op);
	}
	return true;
}

CVar & CVar::operator+(const CVar & sec)
{
	if (!cell.empty() || !strut.empty() || !struts.empty())
		throw "Invalid operation for a class or cell variable.";
	CSignals::operate(sec, '+');
	return *this;
}
CVar & CVar::operator+=(const CVar & sec)
{
	return operator+(sec);
}
CVar & CVar::operator-()
{
	if (!cell.empty() || !strut.empty() || !struts.empty())
		throw "Invalid operation for a class or cell variable.";
	CSignals::operator-();
	return *this;
}
CVar & CVar::operator-(const CVar & sec)
{
	if (!cell.empty() || !strut.empty() || !struts.empty())
		throw "Invalid operation for a class or cell variable.";
	CSignals::operate(sec, '-');
	return *this;
}
CVar & CVar::operator-=(const CVar & sec)
{
	return operator-(sec);
}
CVar & CVar::operator*(const CVar & sec)
{
	if (!cell.empty() || !strut.empty() || !struts.empty())
		throw "Invalid operation for a class or cell variable.";
	CSignals::operate(sec, '*');
	return *this;
}
CVar & CVar::operator*=(const CVar & sec)
{
	return operator*(sec);
}
CVar & CVar::operator/(const CVar & sec)
{
	if (!cell.empty() || !strut.empty() || !struts.empty())
		throw "Invalid operation for a class or cell variable.";
	CSignals::operate(sec, '/');
	return *this;
}
CVar & CVar::operator/=(const CVar & sec)
{
	return operator/(sec);
}

CTimeSeries& CTimeSeries::operator-(CTimeSeries* sec)
{
	throw "You shouldn't come here---";
	AddMultChain('+', &sec->operator-());
	return *this;
}

CVar & CVar::operator+=(CVar * psec)
{
	if (!cell.empty() || !strut.empty() || !struts.empty())
		throw "Invalid operation for a class or cell variable.";
	CSignals::operator+=(psec);
	return *this;
}

CSignal& CSignal::operator-(void) 	// Unary minus
{
	if (bufBlockSize == sizeof(double))
		for (unsigned int k = 0; k < nSamples; k++) buf[k] = -buf[k];
	if (bufBlockSize == 2 * sizeof(double))
		for (unsigned int k = 0; k < nSamples; k++) cbuf[k] = -cbuf[k];
	return *this;
}
CTimeSeries& CTimeSeries::operator-(void) 	// Unary minus
{
	for (CTimeSeries *p = this; p; p = p->chain)
		p->CSignal::operator-();
	return *this;
}
CSignals& CSignals::operator-(void)	// Unary minus
{
	CTimeSeries::operator-();
	if (next)	-*next;
	return *this;
}

CSignal & CSignal::operator*(pair<vector<double>, vector<double>> coef)
{ // modulate this with coef
	vector<double> tpoints = coef.first;
	vector<double> vals = coef.second;
	//assumption: tpoints is increasing 
	auto itval = vals.begin();
	for (auto it = tpoints.begin(); it != tpoints.end() - 1; it++)
	{
		if (*it > endt().front()) continue;
		double t1 = *it + tmark;
		double t2 = *(it + 1) + tmark;
		double slope = (*(itval + 1) - *itval) / (t2 - t1);
		double slopePerSample = (*(itval + 1) - *itval) / (t2 - t1) / fs * 1000.;
		int beginID = max(0, (int)round((t1 - tmark)*fs / 1000.));
		int endID = min((int)nSamples, (int)round((t2 - tmark)*fs / 1000.));
		for (int k = beginID; k < endID; k++)
			buf[k] *= *itval + slopePerSample * (k - beginID);
		itval++;
	}
	return *this;
}

CSignal & CSignal::operator%(double v)
{ // "at" operator; set the RMS at ____
	double rms = RMS().front();
	double factor = v - rms;
	*this *= pow(10, factor / 20.);
	return *this;
}
CTimeSeries & CTimeSeries::operator%(double v)
{
	CSignal::operator%(v);
	for (CTimeSeries *p = this; p; p = p->chain)
		p->CSignal::operator%(v);
	return *this;
}
CSignals & CSignals::operator%(double v)
{
	CTimeSeries::operator%(v);
	if (next)
		next->operator%(v);
	return *this;
}
//No exception handling when "PRESUMPTION" is noted.
CSignal & CSignal::operator%(const CSignal &targetRMS)
{ // PRESUMPTION for targetRMS: nSamples>0, bufBlockSize>=8--reject Empty, reject bool & string
	operator%(targetRMS.buf[0]);
	return *this;
}
CTimeSeries & CTimeSeries::operator%(CTimeSeries * targetRMS)
{ // PRESUMPTION for targetRMS: nSamples>0, bufBlockSize>=8--reject Empty, reject bool & string
	// for each chain, if chained, or a single chain satisfying that condition
	// if chained, it must be the same time-course; i.e., same tmarks
	// Or, scalar time sequence, where nSamples=1 for all chains
	bool timescalar = targetRMS->nSamples==1;
	for (CTimeSeries *p = targetRMS->chain; p; p = p->chain)
	{
		if (timescalar && p->nSamples > 1)
			throw "all operand chains must be scalar.";
		if (!timescalar && p->nSamples <= 1)
			throw "all operand chains must be vector or audio.";
	}
	if (timescalar)
	{ // time scalars 
		vector<double> _tpoints, vals;
		bool relTime = targetRMS->fs == 0;
		for (CTimeSeries *p = targetRMS; p; p = p->chain)
		{
			_tpoints.push_back(p->tmark);
			vals.push_back(p->value());
		}
		vector<double> tpoints(_tpoints);
		if (relTime)
		{
			for (auto &tp : tpoints)
				tp *= _dur();
		}
		CSignal::operator*(make_pair(tpoints, vals));
		for (CTimeSeries *p = chain, *q = targetRMS->chain; p; p = p->chain)
		{
			tpoints = _tpoints;
			if (relTime)
			{
				for (auto &tp : tpoints)
					tp *= p->_dur();
			}
			p->CSignal::operator*(make_pair(tpoints, vals));
		}
	}
	else
	{
		double v = targetRMS->buf[0];
		CSignal::operator%(v);
		for (CTimeSeries *p = chain, *q = targetRMS->chain; p; p = p->chain)
		{
			if (targetRMS->chain)
			{
				v = q->buf[0];
				q = q->chain;
			}
			p->CSignal::operator%(v);
		}
	}
	return *this;
}
CSignals & CSignals::operator%(const CSignals &targetRMS)
{ // PRESUMPTION for targetRMS: 
	// nSamples>0, bufBlockSize>=8--reject Empty, reject bool & string
	// for each chain, if chained, or a single chain satisfying that condition
	// if chained, it must be the same time-course; i.e., same tmarks
	// if targetRMS->next, it must have the same structure; if not, apply the same targetRMS for both channel (or one, if this->next is nullptr)
	//
	// OR
	// scalar time sequence, where nSamples=1 for all chains
	CTimeSeries::operator%((CTimeSeries*)&targetRMS);
	if (next)
	{
		if (targetRMS.next)
			((CSignals*)next)->CTimeSeries::operator%(targetRMS.next);
		else
			((CSignals*)next)->CTimeSeries::operator%((CTimeSeries *)&targetRMS);
	}
	return *this;
}

CSignal & CSignal::operator|(double v)
{ // Set the RMS at ____ (go up or down)
	*this *= pow(10, v / 20.);
	return *this;
}
CTimeSeries & CTimeSeries::operator|(double v)
{
	CSignal::operator|(v);
	for (CTimeSeries *p = this; p; p = p->chain)
		p->CSignal::operator|(v);
	return *this;
}
CSignals & CSignals::operator|(double v)
{
	CTimeSeries::operator|(v);
	if (next)
		next->operator|(v);
	return *this;
}

CSignal & CSignal::operator|(const CSignal & RMS2adjust)
{ // PRESUMPTION for RMS2adjust: nSamples>0, bufBlockSize>=8--reject Empty, reject bool & string
	operator|(RMS2adjust.buf[0]);
	return *this;
}
CTimeSeries & CTimeSeries::operator|(CTimeSeries * RMS2adjust)
{
	double v = RMS2adjust->buf[0];
	CSignal::operator|(v);
	for (CTimeSeries *p = chain, *q = RMS2adjust->chain; p; p = p->chain)
	{
		if (RMS2adjust->chain)
		{
			v = q->buf[0];
			q = q->chain;
		}
		p->CSignal::operator|(v);
	}
	return *this;
}
CSignals & CSignals::operator|(const CSignals & RMS2adjust)
{
	CTimeSeries::operator|((CTimeSeries*)&RMS2adjust);
	if (next)
	{
		if (RMS2adjust.next)
			((CSignals*)next)->CTimeSeries::operator|(RMS2adjust.next);
		else
			((CSignals*)next)->CTimeSeries::operator|((CTimeSeries *)&RMS2adjust);
	}
	return *this;
}
CTimeSeries& CTimeSeries::operator+(CTimeSeries* sec)
{
	AddMultChain('+', sec);
	return *this;
}
