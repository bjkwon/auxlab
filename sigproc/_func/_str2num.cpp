#include "sigproc.h"
#include "bjcommon.h"
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

void _str2num(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	if (!past->Sig.IsString())
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("argument must be a text string.");
	string emsg;
	CAstSig tast(past->pEnv);
	if (!tast.parse_aux((string("[") + past->Sig.string() + "]").c_str(), emsg))
		tast.parse_aux("[]", emsg);
	if (!isAllNodeT_NUM(tast.xtree->child))
		tast.parse_aux("[]", emsg);
	vector<CVar*> res = tast.Compute();
	past->Sig = res.back();
}

void _esc(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	if (!past->Sig.IsString())
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("argument must be a text string.");
	char estr[256];
	string str = past->Sig.string();
	char* instr = new char[str.size()+1];
	memcpy(instr, str.c_str(), str.size() + 1);
	process_esc_chars(instr, str.size(), estr);
	past->Sig.SetString(instr);
	delete[] instr;
}
