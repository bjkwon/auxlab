#include "sigproc.h"

int countVectorItems(const AstNode* pnode); // support.cpp

void _minmax(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	if (past->Sig.IsEmpty()) return; //for empty input, empty output
	string fname = pnode->str;
	CVar sig = past->Sig;
	CVar* newpointer = new CVar(sig.GetFs());
	int nOutVars = countVectorItems(past->pAst);
	if (fname == "max") past->Sig = sig.runFct2getvals(&CSignal::_max, newpointer);
	else if (fname == "min") past->Sig = sig.runFct2getvals(&CSignal::_min, newpointer);
	if (past->Sig.IsComplex()) past->Sig.SetValue(-1); // do it again 6/2/2018
	if (nOutVars > 1)
	{
		past->Sigs.push_back(move(make_unique<CVar*>(&past->Sig)));
		unique_ptr<CVar*> pt = make_unique<CVar*>(newpointer);
		past->Sigs.push_back(move(pt));
	}
	else
		delete newpointer;
	if (past->Sig.type() & TYPEBIT_TEMPORAL) past->Sig.setsnap();
}
