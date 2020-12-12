#include "sigproc.h"

int countVectorItems(const AstNode* pnode)
{
	if (pnode->type != N_VECTOR) return 0;
	AstNode* p = ((AstNode*)pnode->str)->alt;
	int res = 0;
	for (; p; p = p->next)
		res++;
	return res;
}

//needed to handle a statment with a function with multiple output vars
const AstNode* get_line_astnode(const AstNode* root, const AstNode* pnode)
{
	for (auto p = root; p; p = p->next)
	{
		if (p->line == pnode->line) return p;
	}
	return NULL;
}

