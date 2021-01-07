#include "sigproc.h"

void _include(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	string dummy;
	string filename = past->ComputeString(p);
	if (FILE* auxfile = past->fopen_from_path(filename, "", dummy)) {
		try {
			fclose(auxfile);
			CAstSig tast(past->pEnv);
			string body;
			string emsg;
			if (!tast.SetNewScriptFromFile(emsg, filename.c_str(), NULL, body))
				if (!emsg.empty())
					throw emsg.c_str();
			vector<CVar*> res = tast.Compute();
			past->Sig = res.back();
			for (map<string, CVar>::iterator it = tast.Vars.begin(); it != tast.Vars.end(); it++)
				past->Vars[it->first] = it->second;
		}
		catch (const char* errmsg) {
			fclose(auxfile);
			throw CAstException(USAGE, *past, pnode).proc(("Including " + filename + "\n\nIn the file: \n" + errmsg).c_str());
		}
	}
	else
		throw CAstException(USAGE, *past, pnode).proc("Cannot read file: ", "", filename);
}

void _eval(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{ // eval() is one of the functions where echoing in the xcom command window doesn't make sense.
  // but the new variables created or modified within the eval call should be transported back to ast
	// As of 5/17/2020, there is no return of eval (null returned if assigned) for when there's no error
	// If there's an error, exception handling (not error handling) is done and it returns the error message
	string str = past->ComputeString(p);
	try {
		CAstSig tast(str.c_str(), past);
		tast.Compute(tast.pAst);
		//transporting variables 
		for (map<string, CVar>::iterator it = tast.Vars.begin(); it != tast.Vars.end(); it++)
			past->SetVar(it->first.c_str(), &it->second);
		past->Sig = tast.Sig; // temp hack; just to port out the last result during the eval call
	}
	catch (CAstException e) {
		e.line = pnode->line;
		e.col = pnode->col;
		e.pCtx = past;
		throw e;
	}
}