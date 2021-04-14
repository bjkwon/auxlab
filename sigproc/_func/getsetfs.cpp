#include "sigproc.h"

void _getfs(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->Sig.SetValue(past->GetFs());
}

#ifndef NO_SF
// It is better to have setfs as a hook command, because it involves UI and expressions such as setfs(16000)+1 don't make sense.
// Keep this just in case. But this will not update existing variables according to the new sample rate, so its functionality is pretty limited.
// Use an auxlab hook command in this format #setfs 16000
// 11/21/2017
void _setfs(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	if (!past->Sig.IsScalar()) throw CAstException(USAGE, *past, p).proc("Scalar value should be provided for the sample rate.");
	double fs = past->Sig.value();
	if (fs < 500.) throw CAstException(USAGE, *past, p).proc("Sample rate should be at least 500 Hz.");
	past->FsFixed = true;
	past->pEnv->Fs = (int)fs; //Sample rate adjusted
}
#endif // NO_SF
