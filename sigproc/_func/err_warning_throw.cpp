#include "sigproc.h"

void _sprintf(CAstSig* past, const AstNode* pnode);

void aux_input(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->checkString(pnode, past->Sig);
	printf("%s ", past->Sig.string().c_str());
	string user_input;
	cin >> user_input;
	past->Sig.SetString(user_input.c_str());
}

void udf_error(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->checkString(pnode, past->Sig);
	throw CAstException(USAGE, *past, pnode).proc(past->Sig.string().c_str());
}

void udf_warning(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->checkString(pnode, past->Sig);
	printf("WARNING: %s\n", past->Sig.string().c_str());
}
void udf_rethrow(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	past->checkString(pnode, past->Sig);
}

#ifdef _WINDOWS
#include "bjcommon_win.h"
#endif

void _inputdlg(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	ostringstream caption;
	if (past->xtree->type == N_BLOCK && !past->Script.empty())
	{
		caption << "[UDF] \"" << past->Script << "\" ";
		caption << "Line " << pnode->line;
	}
	caption << past->Sig.string();
	_sprintf(past, pnode);

	char buf[64] = {};
#ifdef _WINDOWS
	INT_PTR res = InputBox(caption.str().c_str(), past->Sig.string().c_str(), buf, sizeof(buf));
	if (res == 1)
		past->Sig.SetString(buf);
	else
		past->Sig.SetString("");
#elif
	printf("[NOTE] inputdlg is in a pre-release condition in non-Windows version.\n%s", past->Sig.string().c_str());
#endif
}

void _msgbox(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	ostringstream caption;
	caption << "Line " << pnode->line;
	if (pnode->type != N_STRUCT)
	{
		p = pnode->alt;
		if (p->type == N_ARGS) p = p->child;
		_sprintf(past, pnode);
	}
	past->fpmsg.ShowVariables(past);
#ifdef _WINDOWS
	MessageBox(NULL, past->Sig.string().c_str(), caption.str().c_str(), MB_OK);
	past->Sig.Reset();
#elif
	printf("[NOTE] inputdlg is in a pre-release condition in non-Windows version.\n%s", past->Sig.string().c_str());
#endif
}