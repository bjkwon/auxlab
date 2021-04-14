#include "sigproc.h"
#include "..\psycon.tab.h"

// This function datatype will eventually eliminate the use of CSIG_ series constants 
// and all the is____ functions 5/25/2020

int __datatype(const CVar& sig, WORD& out)
{
	if (sig.IsEmpty())				out = 0xffff;
	else if (sig.IsGO())		out = 0x2000;
	else if (sig.GetFs() == 1)		out = 0;
	else if (sig.GetFs() == 2)		out = 0x0010;
	else if (sig.GetFs() > 500)		out = 0x0020;
	else if (sig.IsLogical())		out = 0x0040;
	else if (!sig.cell.empty())		out = 0x1000;
	else if (!sig.strut.empty())		out = 0x2000;
	else
		return -1;
	if (sig.snap) out += 0x0008;
	if (sig.nSamples > 0) out++;
	if (sig.nSamples > 1) out++;
	return 1;
}

void _datatype(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	uint16_t out;
	if ((out = past->Sig.type()) == 0xffff)
		throw CAstException(INTERNAL, *past, pnode).proc("this particular data type has not been ready to handle.");
	if (out & 0x2000)
	{
		past->pgo = NULL;
		past->Sig.Reset();
	}
	past->Sig.SetValue((double)out);
}

void _veq(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	CAstSig br(past);
	WORD type1, type2;
	CVar arg1, arg2;
	if (pnode->type == N_STRUCT)
	{ // x.veg(y)
		arg1 = past->Sig;
		arg2 = past->Compute(pnode->alt->child);
	}
	else if (pnode->type == T_ID)
	{ // veq(x,y)
		if (pnode->alt->type == N_ARGS)
		{
			arg1 = past->Compute(pnode->alt->child);
			arg2 = past->Compute(pnode->alt->tail);
		}
		else
			throw CAstException(INTERNAL, *past, pnode).proc("this particular data type has not been ready to handle.");
	}
	else
		throw CAstException(INTERNAL, *past, pnode).proc("this particular data type has not been ready to handle.");

	if (__datatype(arg1, type1) < 0)
		throw CAstException(INTERNAL, *past, pnode).proc("this particular data type has not been ready to handle.");
	if (__datatype(arg2, type2) < 0)
		throw CAstException(INTERNAL, *past, pnode).proc("this particular data type has not been ready to handle.");
	try {
		// throw 0 for false
		if (type1 != type2) throw 0;
		else if (arg1.nSamples != arg2.nSamples) throw 0;
		else if (type1 & 0x2000) // GO
		{
			if (arg1.value() != arg2.value()) throw 0;
		}
		else
		{
			if (arg1.bufBlockSize == 8)
				for (unsigned k = 0; k < arg1.nSamples; k++)
				{
					if (arg1.buf[k] != arg2.buf[k]) throw 0;
				}
			else if (arg1.bufBlockSize == 16)
				for (unsigned k = 0; k < arg1.nSamples; k++)
				{
					if (arg1.cbuf[k] != arg2.cbuf[k]) throw 0;
				}
			else
				for (unsigned k = 0; k < arg1.nSamples; k++)
				{
					if (arg1.logbuf[k] != arg2.logbuf[k]) throw 0;
				}
		}
		past->Sig.Reset(1);
		past->Sig.MakeLogical();
		past->Sig.UpdateBuffer(1);
		past->Sig.logbuf[0] = true;
		return;
	}
	catch (int k)
	{
		//k should be 0 and it doesn't matter what k is.
		k = 0; // just to avoid warning C4101
		past->Sig.Reset(1);
		past->Sig.MakeLogical();
		past->Sig.UpdateBuffer(1);
		past->Sig.logbuf[0] = false;
		return;
	}
}

