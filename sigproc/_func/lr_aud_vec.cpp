#include "sigproc.h"

void _audio(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->blockString(pnode, past->Sig);
	past->blockCell(pnode, past->Sig);
	switch (past->Sig.nGroups)
	{
	case 1:
		past->Sig.SetFs(past->GetFs());
		break;
	case 2:
	{
		past->Sig.SetFs(past->GetFs());
		past->Sig.nSamples /= 2;
		past->Sig.nGroups = 1;
		CSignals nextgh(past->Sig.GetFs());
		nextgh <= past->Sig;
		nextgh.buf += past->Sig.nSamples;
		past->Sig.SetNextChan(&nextgh);
	}
		break;
	default:
		CAstException(USAGE, *past, p).proc("Cannot apply to a matrix with rows > 2.");
		break;
	}
}

void _vector(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->checkAudioSig(pnode, past->Sig);
	//	past->Sig.MakeChainless(); // if this is on, you can't easily display values from an audio obj 6/29/2020
	past->Sig.SetFs(1);
	past->Sig.snap = false;
	if (past->Sig.next)
	{
		CSignal out = CSignal(1, past->Sig.nSamples * 2);
		memcpy(out.logbuf, past->Sig.logbuf, past->Sig.nSamples * past->Sig.bufBlockSize);
		memcpy(out.logbuf + past->Sig.nSamples * past->Sig.bufBlockSize, past->Sig.next->logbuf, past->Sig.nSamples * past->Sig.bufBlockSize);
		out.nGroups = 2;
		past->Sig = (CVar)(CSignals)out;
	}
}

void _left(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->checkAudioSig(pnode, past->Sig);
	delete past->Sig.next;
	past->Sig.next = NULL;
}

void _right(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->checkAudioSig(pnode, past->Sig);
	past->Sig.bringnext();
}

