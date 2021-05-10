#include "sigproc.h"

void _setnextchan(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
    // current this is used only when a right channel is added to a mono (and it becomes left)
	if (past->Sig.next)
		throw CAstException(USAGE, *past, pnode).proc("This function should be used only for a mono signal.");
	CVar sig = past->Sig;
	CVar param = past->Compute(p);
	if (param.next)
		throw CAstException(USAGE, *past, pnode).proc("This function should be used with a mono signal argument.");
	if (!(param.type() & TYPEBIT_TEMPORAL) && param.type() != 1)
		throw CAstException(USAGE, *past, pnode).proc("Invalid argument.");
	CVar* second = new CVar;
	*second = param;
	sig.SetNextChan(second);
	past->Sig = sig;
}

