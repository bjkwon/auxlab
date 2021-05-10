#include "sigproc.h"

void _ismember(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	if (!past->Sig.IsStruct())
		throw CAstException(FUNC_SYNTAX, *past, p).proc("Must be applied to a class/struct object.");
	try {
		CAstSig tp(past);
		CVar param = tp.Compute(p);
		if (!param.IsString())
			throw CAstException(FUNC_SYNTAX, *past, p).proc("Argument must be a string.");
		auto it = past->Sig.strut.find(param.string());
		if (it == past->Sig.strut.end())
			past->Sig.SetValue(0.);
		else
			past->Sig.SetValue(1.);
		past->Sig.MakeLogical();
	}
	catch (const CAstException & e) { throw CAstException(FUNC_SYNTAX, *past, pnode).proc(e.getErrMsg().c_str()); }
}

void _isaudioat(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
    // if the signal is null at specified time_pt
	past->checkAudioSig(pnode, past->Sig);
	CAstSig tp(past);
	tp.Compute(p);
	if (tp.Sig.GetType() != CSIG_SCALAR)
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("argument must be a scalar.");
	double vv = tp.Sig.value();
	past->Sig.SetValue(past->Sig.IsAudioOnAt(vv));
	past->Sig.MakeLogical();
}

void _varcheck(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	string fname = pnode->str;
	int type = past->Sig.GetType();
	CVar out;
	if (fname == "isempty")			out.SetValue(past->Sig.IsEmpty());
	else if (fname == "isaudio")	out.SetValue(type == CSIG_AUDIO ? 1 : 0);
	else if (fname == "isvector")	out.SetValue(type == CSIG_VECTOR ? 1 : 0);
	else if (fname == "isstring")	out.SetValue(type == CSIG_STRING ? 1 : 0);
	else if (fname == "iscell")		out.SetValue((double)(int)!past->Sig.cell.empty());
	else if (fname == "isclass")	out.SetValue((double)(int)!past->Sig.strut.empty());
	else if (fname == "isbool")		out.SetValue(past->Sig.bufBlockSize == 1 && past->Sig.GetFs() != 2);
	else if (fname == "isstereo")	out.SetValue(past->Sig.next != NULL ? 1 : 0);
	else if (fname == "istseq")		out.SetValue((double)(int)past->Sig.IsTimeSignal()); //check...
	out.MakeLogical();
	past->Sig = out;
	past->pgo = NULL;//must be reset here; otherwise, the GO lingers and interferes with other opereation successively.
}

