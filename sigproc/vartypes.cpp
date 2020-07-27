#include "sigplus_internal.h"

bool CTimeSeries::IsScalar() const
{
	for (CTimeSeries const *p = this; p; p = p->chain)
		if (!p->CSignal::IsScalar())
			return false;
	return true;
}
bool CSignals::IsScalar() const
{
	bool out = CTimeSeries::IsScalar();
	if (!next) return out;
	if (!out) return out;
	for (CTimeSeries const *p = next; p; p = p->chain)
		if (!p->CSignal::IsScalar())
			return false;
	return true;
}

bool CTimeSeries::IsVector() const
{
	for (CTimeSeries const *p = this; p; p = p->chain)
		if (!p->CSignal::IsVector())
			return false;
	return true;
}
bool CSignals::IsVector() const
{
	bool out = CTimeSeries::IsVector();
	if (!next) return out;
	if (!out) return out;
	for (CTimeSeries const *p = next; p; p = p->chain)
		if (!p->CSignal::IsVector())
			return false;
	return true;
}

bool CTimeSeries::IsAudio() const
{
	for (CTimeSeries const *p = this; p; p = p->chain)
		if (!p->CSignal::IsAudio())
			return false;
	return true;
}
bool CSignals::IsAudio() const
{
	bool out = CTimeSeries::IsAudio();
	if (!next) return out;
	for (CTimeSeries const *p = next; p; p = p->chain)
		if (!p->CSignal::IsAudio())
			return false;
	return true;
}

bool CTimeSeries::IsString() const
{
	for (CTimeSeries const *p = this; p; p = p->chain)
		if (!p->CSignal::IsString())
			return false;
	return true;
}
bool CSignals::IsString() const
{
	bool out = CTimeSeries::IsString();
	if (!next) return out;
	if (!out) return out;
	for (CTimeSeries const *p = next; p; p = p->chain)
		if (!p->CSignal::IsString())
			return false;
	return true;
}

bool CTimeSeries::IsComplex() const
{
	for (CTimeSeries const *p = this; p; p = p->chain)
		if (!p->CSignal::IsComplex())
			return false;
	return true;
}
bool CSignals::IsComplex() const
{
	bool out = CTimeSeries::IsComplex();
	if (!next) return out;
	if (!out) return out;
	for (CTimeSeries const *p = next; p; p = p->chain)
		if (!p->CSignal::IsComplex())
			return false;
	return true;
}

bool CTimeSeries::IsBool() const
{
	for (CTimeSeries const *p = this; p; p = p->chain)
		if (!p->CSignal::IsBool())
			return false;
	return true;
}
bool CSignals::IsBool() const
{
	bool out = CTimeSeries::IsBool();
	if (!next) return out;
	if (!out) return out;
	for (CTimeSeries const *p = next; p; p = p->chain)
		if (!p->CSignal::IsBool())
			return false;
	return true;
}

void CTimeSeries::SetComplex()
{
	for (CTimeSeries *p = this; p; p = p->chain)
		p->CSignal::SetComplex();
}
void CSignals::SetComplex()
{
	CTimeSeries::SetComplex();
	for (CTimeSeries *p = next; p; p = p->chain)
		p->CSignal::SetComplex();
}
void CTimeSeries::SetReal()
{
	for (CTimeSeries *p = this; p; p = p->chain)
		p->CSignal::SetReal();
}
void CSignals::SetReal()
{
	CTimeSeries::SetReal();
	for (CTimeSeries *p = next; p; p = p->chain)
		p->CSignal::SetReal();
}

