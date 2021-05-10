#include "sigproc.h"

void _fdelete(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	char drive[64], dir[MAX_PATH];
	_splitpath(past->ComputeString(p).c_str(), drive, dir, NULL, NULL);
	string filename;
	if (drive[0] == 0 && dir[0] == 0) // no directory info
		filename = past->pEnv->AppPath;
	filename += past->ComputeString(p);
	int res = remove(filename.c_str());
	if (!res) // success
		past->Sig.SetValue(1);
	else
		past->Sig.SetValue(0);
}
