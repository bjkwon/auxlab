#include "sigproc.h"
#include "bjcommon.h"

void _include(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	string dummy, emsg;
	string filename = past->ComputeString(p);
	if (FILE* auxfile = past->fopen_from_path(filename, "", dummy)) {
		try {
			CAstSig qscope(past);
			string filecontent;
			if (GetFileText(auxfile, filecontent) <= 0)
			{ // File reading error or empty file
				past->statusMsg += "Cannot read specified file: " + filename;
				fclose(auxfile);
				return;
			}
			fclose(auxfile);
			qscope.xtree = qscope.parse_aux(filecontent.c_str(), emsg);
			if (!qscope.xtree)
				throw emsg.c_str();
			vector<CVar*> res = qscope.Compute();
			past->Sig = res.back();
			for (map<string, CVar>::iterator it = qscope.Vars.begin(); it != qscope.Vars.end(); it++)
				past->Vars[it->first] = it->second;
			for (auto it = qscope.GOvars.begin(); it != qscope.GOvars.end(); it++)
				past->GOvars[it->first] = it->second;
		}
		catch (const char* errmsg) {
			fclose(auxfile);
			throw CAstException(USAGE, *past, pnode).proc((string(errmsg) + "while including " + filename + " in the file: ").c_str());
		}
	}
	else
		throw CAstException(USAGE, *past, pnode).proc("Cannot read file: ", "", filename);
}

void _eval(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
    // eval() is one of the functions where echoing in the xcom command window doesn't make sense.
  // but the new variables created or modified within the eval call should be transported back to the calling scope
	// As of 5/17/2020, there is no return of eval (null returned if assigned) for when there's no error
	// If there's an error, exception handling (not error handling) is done and it returns the error message
	string emsg, str = past->ComputeString(p);
	CAstSig qscope(str.c_str(), past);
	try {
		qscope.xtree = qscope.parse_aux(str.c_str(), emsg);
		qscope.Compute(qscope.xtree);
		//transporting variables
		for (map<string, CVar>::iterator it = qscope.Vars.begin(); it != qscope.Vars.end(); it++)
			past->SetVar(it->first.c_str(), &it->second);
		past->Sig = qscope.Sig; // temp hack; just to port out the last result during the eval call
	}
	catch (CAstException e) {
		e.line = pnode->line;
		e.col = pnode->col;
		e.pCtx = past;
		e.pnode = pnode;
		throw e;
	}
}