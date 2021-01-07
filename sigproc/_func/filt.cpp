#include "sigproc.h"

// support.cpp
int countVectorItems(const AstNode* pnode); 
const AstNode* get_line_astnode(const AstNode* root, const AstNode* pnode);

void _filt(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	// CSignal::filter and CSignal::filtfilt
	// pargs 
	past->checkSignal(pnode, past->Sig);
	CVar sig = past->Sig;
	string fname = pnode->str;
	CVar fourth, third, second = past->Compute(p);
	if (p->next) {
		if (p->next->next)
			fourth = past->Compute(p->next->next); // 4 args
		third = past->Compute(p->next); // 3 args
		if (fourth.nSamples >= third.nSamples)
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, "The length of the initial condition vector must be less than the length of denominator coefficients.");
	}
	else {				// 2 args
		if (second.nSamples <= 1)
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, "2nd argument must be a vector(the numerator array for filtering).");
		third.SetValue(1);
	}
	unsigned int len = max(second.nSamples, third.nSamples);
	if (second.IsScalar())
		second.UpdateBuffer(len);
	else if (third.IsScalar())
		third.UpdateBuffer(len);

	if (sig.IsComplex() || second.IsComplex() || third.IsComplex() || fourth.IsComplex())
	{
		vector<complex<double>> initial;
		vector<complex<double>> num(second.cbuf, second.cbuf + second.nSamples);
		vector<complex<double>> den(third.cbuf, third.cbuf + third.nSamples);
		vector<vector<complex<double>>> coeffs;
		if (!second.chain && !third.chain && second.tmark == 0 && third.tmark == 0)
		{
			coeffs.push_back(num);
			coeffs.push_back(den);
			if (fourth.nSamples > 0)
			{
				for (unsigned int k = 0; k < fourth.nSamples; k++) initial.push_back(fourth.buf[k]);
				coeffs.push_back(initial);
			}
			if (fname == "filt")
				sig.fp_mod(&CSignal::filter, &coeffs);
			else if (fname == "filtfilt")
				sig.fp_mod(&CSignal::filtfilt, &coeffs);
			//at this point, coeffs is not the same as before (updated with the final condition)
		}
		else
		{
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, "Internal error--leftover from Dynamic filtering");
		}
		past->Sig = sig;
		auto linehead = get_line_astnode(past->pAst, pnode);
		if (countVectorItems(linehead) > 1)
		{ // in this case coeffs carries the final condition array (for stereo, the size is 2)
			past->Sigs.push_back(move(make_unique<CVar*>(&past->Sig)));
			CVar* newpointer = new CVar(sig.GetFs());
			CSignals finalcondition(coeffs.back().data(), (int)coeffs.back().size()); // final condnition is stored at the last position
			*newpointer = finalcondition;
			unique_ptr<CVar*> pt = make_unique<CVar*>(newpointer);
			past->Sigs.push_back(move(pt));
		}
	}
	else
	{
		vector<double> initial;
		vector<double> num(second.buf, second.buf + second.nSamples);
		vector<double> den(third.buf, third.buf + third.nSamples);
		vector<vector<double>> coeffs;
		if (!second.chain && !third.chain && second.tmark == 0 && third.tmark == 0)
		{
			coeffs.push_back(num);
			coeffs.push_back(den);
			if (fourth.nSamples > 0)
			{
				for (unsigned int k = 0; k < fourth.nSamples; k++) initial.push_back(fourth.buf[k]);
				coeffs.push_back(initial);
			}
			if (fname == "filt")
				sig.fp_mod(&CSignal::filter, &coeffs);
			else if (fname == "filtfilt")
				sig.fp_mod(&CSignal::filtfilt, &coeffs);
			//at this point, coeffs is not the same as before (updated with the final condition)
		}
		else
		{
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, "Internal error--leftover from Dynamic filtering");
		}
		past->Sig = sig;
		auto linehead = get_line_astnode(past->pAst, pnode);
		if (countVectorItems(linehead) > 1)
		{ // in this case coeffs carries the final condition array (for stereo, the size is 2)
			past->Sigs.push_back(move(make_unique<CVar*>(&past->Sig)));
			CVar* newpointer = new CVar(sig.GetFs());
			CSignals finalcondition(coeffs.back().data(), (int)coeffs.back().size()); // final condnition is stored at the last position
			*newpointer = finalcondition;
			unique_ptr<CVar*> pt = make_unique<CVar*>(newpointer);
			past->Sigs.push_back(move(pt));
		}
	}
}

void _conv(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	//For only real (double) arrays 3/4/2019
	//p should be non NULL
	CVar sig = past->Sig;
	CVar array2 = past->Compute(p);
	past->Sig = sig.fp_mod(&CSignal::conv, &array2);
}

CSignal& CSignal::_filter(const vector<double>& num, const vector<double>& den, vector<double>& state, unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	if (state.empty()) state.push_back(0.);
	if (IsComplex())
	{
		complex<double>* out = new complex<double>[len];
		for (unsigned int m = id0; m < id0 + len; m++)
		{
			out[m] = num[0] * cbuf[m] + state.front();
			//DO THIS--------------------

			//size of state is always one less than num or den
			//int k = 1;
			//for (auto& v : state)
			//{
			//	v = num[k] * cbuf[m] - den[k] * out[m];
			//	if (k < initial.size())
			//		v += *((&v) + 1);
			//	k++;
			//}
		}
		delete[] cbuf;
		cbuf = out;
	}
	else
	{
		double* out = new double[len];
		for (unsigned int m = id0; m < id0 + len; m++)
		{
			out[m - id0] = num[0] * buf[m] + state.front();
			//size of state is always one less than num or den
			int k = 1;
			for (auto& v : state)
			{
				v = num[k] * buf[m] - den[k] * out[m - id0];
				if (k < state.size())
					v += *((&v) + 1);
				k++;
			}
		}
		memcpy(buf + id0, out, sizeof(double) * len);
		delete[] out;
	}
	return *this;
}

CSignal& CSignal::filter(unsigned int id0, unsigned int len, void *parg)
{
	if (len == 0) len = nSamples;
	vector<double> num, den, initfin;
	vector<vector<double>> coeffs = *(vector<vector<double>>*)parg;
	num = coeffs.front();
	den = *(coeffs.begin() + 1);
	if (coeffs.size() > 2) // initial condition provided
		initfin = coeffs.back();
	_filter(num, den, initfin, id0, len);
	auto vv = (vector<vector<double>>*)parg;
	if (vv->size() == 3)
		vv->back() = initfin; // updating the content of the pointer at the last position of the vector, in parg 
	else // size should be 2 or less
		vv->push_back(initfin);
	return *this;
}

CSignal& CSignal::filtfilt(unsigned int id0, unsigned int len, void *parg)
{
	//Transient edges not handled, only zero-padded edges 
	if (len == 0) len = nSamples;
	vector<double> num, den;
	vector<vector<double>> coeffs = *(vector<vector<double>>*)parg;
	num = coeffs.front();
	den = coeffs.back();

	CSignal temp(fs), out(fs);
	unsigned int nfact = (unsigned int)(3 * (max(num.size(), den.size()) - 1));
//	temp.Silence((unsigned int));
//	temp2.Silence((unsigned int)nfact);
	temp.UpdateBuffer(nfact);
	memset(temp.buf, 0, sizeof(double) * nfact);
	CSignal temp2(temp);
	temp += this;
	temp += &temp2;
	temp.filter(id0, temp.nSamples, parg);
	temp.ReverseTime();
	temp.filter(id0, temp.nSamples);
	temp.ReverseTime();
	out.UpdateBuffer(nSamples);
	memcpy(out.buf, temp.buf + nfact, sizeof(double) * nSamples);
	*this = out;
	return *this;
}
