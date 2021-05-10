#include "sigproc.h"


void _and(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->blockCell(pnode, past->Sig);
	past->blockString(pnode, past->Sig);
	past->blockComplex(pnode, past->Sig);
	if (!p) // single arg
	{
		double res(1.);
		if (past->Sig.IsLogical())
		{
			for (unsigned k = 0; k < past->Sig.nSamples; k++)
				if (!past->Sig.logbuf[k]) { res = 0.;		break; }
		}
		else
		{
			for (unsigned k = 0; k < past->Sig.nSamples; k++)
				if (past->Sig.buf[k] == 0.) { res = 0.;	break; }
		}
		past->Sig.SetValue(res);
		past->Sig.MakeLogical();
	}
	else
	{
		CVar x1 = past->Sig;
		CVar x2 = past->Compute(p);
		if (!x2.IsLogical())
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("argument must be a logical variable.");
		past->Sig.Reset(1);
		past->Sig.UpdateBuffer(min(x1.nSamples, x2.nSamples));
		past->Sig.MakeLogical();
		for (unsigned k = 0; k < min(x1.nSamples, x2.nSamples); k++)
			past->Sig.logbuf[k] = x1.logbuf[k] && x2.logbuf[k];
	}
}

void _or(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->blockCell(pnode, past->Sig);
	past->blockString(pnode, past->Sig);
	past->blockComplex(pnode, past->Sig);
	if (!p) // single arg
	{
		double res(0.);
		if (past->Sig.IsLogical())
		{
			for (unsigned k = 0; k < past->Sig.nSamples; k++)
				if (past->Sig.logbuf[k]) { res = 1.;		break; }
		}
		else
		{
			for (unsigned k = 0; k < past->Sig.nSamples; k++)
				if (past->Sig.buf[k] != 0.) { res = 1.;	break; }
		}
		past->Sig.SetValue(res);
		past->Sig.MakeLogical();
	}
	else
	{
		CVar x1 = past->Sig;
		CVar x2 = past->Compute(p);
		if (!x2.IsLogical())
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("argument must be a logical variable.");
		past->Sig.Reset(1);
		past->Sig.UpdateBuffer(min(x1.nSamples, x2.nSamples));
		past->Sig.MakeLogical();
		for (unsigned k = 0; k < min(x1.nSamples, x2.nSamples); k++)
			past->Sig.logbuf[k] = x1.logbuf[k] && x2.logbuf[k];
	}
}

void _sort(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	CVar sig = past->Sig;
	past->checkVector(pnode, sig, "sort() ");
	past->blockComplex(pnode, sig);
	double order(1.);
	if (p)
	{
		past->Compute(p);
		past->checkScalar(pnode, past->Sig, "sort() arg2 ");
		past->blockComplex(pnode, past->Sig, "sort() arg2 ");
		if (past->Sig.value() < 0) order = -1.;
	}
	past->Sig = sig.fp_mod(&CSignal::sort, &order);
}

void _mostleast(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	if (past->Sig.IsEmpty()) return; //for empty input, empty output
	past->checkVector(pnode, past->Sig);
	string func = pnode->str;
	CVar sig = past->Sig;
	CVar param = past->Compute(p);
	if (func == "atmost") past->Sig = sig.fp_mod(&CSignal::_atmost, &param);
	else if (func == "atleast") past->Sig = sig.fp_mod(&CSignal::_atleast, &param);
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}

template <class T>
int dcomp(const void* arg1, const void* arg2)
{
	if (*(T*)arg1 > * (T*)arg2)	return 1;
	else if (*(T*)arg1 == *(T*)arg2) return 0;
	else	return -1;
}

template <class T>
int dcompR(const void* arg1, const void* arg2)
{
	if (*(T*)arg1 < *(T*)arg2)	return 1;
	else if (*(T*)arg1 == *(T*)arg2) return 0;
	else	return -1;
}

CSignal& CSignal::sort(unsigned int id0, unsigned int len, void *parg)
{
	if (bufBlockSize == 8)
	{
		if (*(double*)parg > 0)
			qsort(logbuf + id0 * bufBlockSize, len, bufBlockSize, dcomp<double>);
		else
			qsort(logbuf + id0 * bufBlockSize, len, bufBlockSize, dcompR<double >);
	}
	else if (bufBlockSize == 1)
	{
		if (*(double*)parg > 0)
			qsort(logbuf + id0 * bufBlockSize, len, bufBlockSize, dcomp<unsigned char>);
		else
			qsort(logbuf + id0 * bufBlockSize, len, bufBlockSize, dcompR<unsigned char>);
	}
	else
		throw "internal error--this shouldn't happen. CSignal::sort";
	return *this;
}

CSignal& CSignal::_atmost(unsigned int id, int unsigned len, void *parg)
{
	double limit;
	if (len == 0) len = nSamples;
	CVar param = *(CVar*)parg;
	if (param.IsScalar())
		limit = param.value();
	else
		limit = (id / len < nSamples) ? param.buf[id / len] : std::numeric_limits<double>::infinity();
	for (unsigned int k = id; k < id + len; k++)
		if (buf[k] > limit) buf[k] = limit;
	return *this;
}

CSignal& CSignal::_atleast(unsigned int id, int unsigned len, void *parg)
{
	double limit;
	if (len == 0) len = nSamples;
	CVar param = *(CVar*)parg;
	if (param.IsScalar())
		limit = param.value();
	else
		limit = (id / len < nSamples) ? param.buf[id / len] : -std::numeric_limits<double>::infinity();
	for (unsigned int k = id; k < id + len; k++)
		if (buf[k] < limit) buf[k] = limit;
	return *this;
}

