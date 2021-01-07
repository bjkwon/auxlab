#include "sigproc.h"
#include "..\psycon.tab.h"

bool isAllNodeT_NUM(const AstNode* p)
{
	if (p->type == T_NUMBER)
	{
		if (p->next)
			return isAllNodeT_NUM(p->next);
		return true;
	}
	return false;
}

void _str2num(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	if (!past->Sig.IsString())
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc(fnsigs, "argument must be a text string.");
	string emsg;
	CAstSig tast(past->pEnv);
	if (!tast.parse_aux((string("[") + past->Sig.string() + "]").c_str(), emsg))
		tast.parse_aux("[]", emsg);
	if (!isAllNodeT_NUM(tast.xtree->child))
		tast.parse_aux("[]", emsg);
	vector<CVar*> res = tast.Compute();
	past->Sig = res.back();
}

