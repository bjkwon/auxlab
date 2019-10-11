#include "sigplus_internal.h"

CSignal& CSignal::operator+(const CSignal& sec)
{ // Exception handling is yet to be done 3/8/2019
  //Currently only for real arrays. 3/8
	if (sec.IsScalar())
	{
		double val = sec.value();
		for (unsigned int k = 0; k < nSamples; k++)
			buf[k] += val;
	}
	else
	{
		if (fs != sec.fs) throw "lhs and rhs must have the same fs";
		if (sec.nSamples > nSamples)
		{
			UpdateBuffer(sec.nSamples);
			memcpy(buf + nSamples, sec.buf + nSamples, sizeof(double)*(sec.nSamples - nSamples));
		}
		for (unsigned int k = 0; k < nSamples; k++)
			buf[k] += sec.buf[k];
	}
	return *this;
}

CTimeSeries& CTimeSeries::operator+(CTimeSeries* sec)
{ 
	//if (IsEmpty() || GetType() == CSIG_NULL) return (*this = sec);
	//if (sec.IsEmpty() || sec.GetType() == CSIG_NULL) return *this;
	//if (IsSingle() && !sec.IsSingle())
	//	SwapContents1node(sec);
	//if (sec.IsString() || IsString())
	//	throw "Addition of string is allowed only with a scalar.";
	AddMultChain('+', sec);
	return *this;
}
CSignals & CSignals::operator+(const CSignals& sec)
{
	CTimeSeries::operator+((CTimeSeries*)&sec);
	if (next)
	{
		if (sec.next)
			((CSignals*)next)->CTimeSeries::operator+(sec.next);
		else
			((CSignals*)next)->CTimeSeries::operator+((CTimeSeries*)&sec);
	}
	return *this;
}

CSignal& CSignal::operator-(CSignal& sec)
{
	operator+(sec.operator-());
	return *this;
}
CTimeSeries& CTimeSeries::operator-(CTimeSeries* sec)
{
	AddMultChain('+', &sec->operator-());
	return *this;
}
CSignals& CSignals::operator-(CSignals& sec)
{
	operator+(sec.operator-());
	return *this;
}

CSignal& CSignal::operator-(void)
{
	if (bufBlockSize == sizeof(double))
		for (unsigned int k = 0; k < nSamples; k++) buf[k] = -buf[k];
	if (bufBlockSize == 2 * sizeof(double))
		for (unsigned int k = 0; k < nSamples; k++) cbuf[k] = -cbuf[k];
	return *this;
}

CSignal & CSignal::operator%(double v)
{ // Set the RMS at ____
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
	return *this;
}
CSignals & CSignals::operator%(const CSignals &targetRMS)
{ // PRESUMPTION for targetRMS: nSamples>0, bufBlockSize>=8--reject Empty, reject bool & string
	// for each chain, if chained, or a single chain satisfying that condition
	// if chained, it must be the same time-course; i.e., same tmarks
	// if targetRMS->next, it must have the same structure; if not, apply the same targetRMS for both channel (or one, if this->next is nullptr)
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
