// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.495
// Date: 12/13/2018
// 
#include <sstream>
#include <list>
#include <algorithm>
#include <exception>
#include <math.h>
#include <time.h>
#include "sigproc.h"
#include "audfret.h"

#include <algorithm> // for lowercase

#ifndef CISIGPROC
#include "psycon.tab.h"
#else
#include "cipsycon.tab.h"
#endif

/* NOTE 9/23/2017
Don't use------- throw CAstException(p, this, "error message");
when p->type is N_XXXXXX, because it will make an error msg like 

ERROR: N_EXTRACT : variable not available.

which is not helpful to users.
*/

#define PRINTLOG(FNAME,STR) \
{ FILE*__fp=fopen(FNAME,"at"); fprintf(__fp,STR);	fclose(__fp); }

void UpdateGO(CAstSig &ast, CVar &Sig);

#ifndef AUX_NO_EXTRA
int _reserve_sel(CAstSig *past, const AstNode *p, CSignals *out);
#else
int _reserve_sel(CAstSig *past, const AstNode *p, CSignals *out)
{
	return 1;
}
#endif // AUX_NO_EXTRA


void dummy_fun1(CAstSig *a, DEBUG_STATUS b, int line)
{
}
void dummy_fun2(CAstSig *a, const AstNode * b)
{
}
bool dummy_fun3(const char * a)
{
	return false;
}
void dummy_fun4(CAstSig *a)
{
}
void dummy_fun5(int a)
{
}
void dummy_fun6(const char* pt)
{
}

void dummy_fun6a(const char* pt)
{
}

void dummy_fun7(CAstSig *a, const char *b, CVar c)
{
}


AstNode *get_parent_node(AstNode *root, AstNode *pChild)
{
	if (root == pChild) return NULL;
	AstNode *p = root;
	if (p == pChild) return p;
	//first locate the base node for the line of pChild
	while (p)
	{
		AstNode *res;
		if (p->line == pChild->line) break;
		if (p->type == T_IF)
		{ // if ... else ... end is done
		//do if ... elseif ... else ... end
			// do while ... end
			//do for ... end
			res = get_parent_node(p->child->next, pChild);
			if (!res)
				res = get_parent_node(p->child->alt, pChild);
			if (res) return res;
		}
		p = p->next;
	}
	//Now, p is the base node for the line
	return CAstSig::findParentNode(p, pChild);
}


#ifndef LINK_STATIC_SIGPROC
HMODULE hDllModule;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	hDllModule = hModule;
	if (ul_reason_for_call == 1 || ul_reason_for_call == 2)
		srand((unsigned int)time(NULL) ^ (unsigned int)GetCurrentThreadId());
    return TRUE;
}
#endif

//DO NOT USE THIS MACRO for a negative input*********
#define INT(X) (int)(X+.5)

#ifdef _WINDOWS
#define GetTickCount0 GetTickCount
#else if
unsigned long GetTickCount0()
{
	return 1;
}
#endif

#ifndef GRAFFY
void CAstSig::cleanup_nodes(CAstSig *beginhere)
{
	size_t nLayersBase(1);
	if (!beginhere) // if called from auxcon, beginhere is NULL, then beginhere should be set to point the last astsig 
	{
		nLayersBase++;
		for (beginhere = vecast.front(); beginhere->son; )
			beginhere = beginhere->son;
	}
	for (CAstSig *pst = beginhere; vecast.size() > nLayersBase; )
	{
		vecast.pop_back();
		CAstSig *up = pst->dad;
		delete pst;
		up->son = NULL;
		pst = up;
	}
}

AstNode *CAstSig::findParentNode(AstNode *p, AstNode *pME, bool altonly)
{
	if (p)
	{
		if (!p->child && !p->alt) return NULL;
		if (!altonly && p->child == pME) return p;
		if (p->alt == pME) return p;
		if (!altonly && p->child)
			return findParentNode(p->child, pME, altonly);
		else
			return findParentNode(p->alt, pME, altonly);
	}
	return NULL;
}

void CAstSig::print_links0(FILE *fp, AstNode *p)
{
	while (p)
	{
		switch (p->type)
		{
		case 285:// T_num
			fprintf(fp, "0x%x line=%d, type=%d, %f\n", (INT_PTR)p, p->line, p->type, p->dval);
			break;
		case 286:// T_num
			fprintf(fp, "0x%x line=%d, type=%d, %s\n", (INT_PTR)p, p->line, p->type, p->str);
			break;
		case 264:// T_WHILE
		case 265:// T_FOR
		case 10004:// 
		case '=':// 
			fprintf(fp, "0x%x line=%d, type=%d, %s\n", (INT_PTR)p, p->line, p->type, p->str);
			print_links0(fp, p->child);
			break;
		default:
			fprintf(fp, "0x%x line=%d, type=%d, %s\n", (INT_PTR)p, p->line, p->type, p->str);
			if (p->child)
			{
				if (p->child->type == 285)//T_NUMBER
					fprintf(fp, "CHILD 0x%x type=%d, %f\n", (INT_PTR)p->child, p->child->type, p->child->dval);
				else if (p->child->type == 286)//T_NUMBER
					fprintf(fp, "CHILD 0x%x type=%d, %s\n", (INT_PTR)p->child, p->child->type, p->child->str);
			}
			break;
		}
		p = p->next;
	}
}

void CAstSig::print_links(const char *filename, AstNode *pnode)
{
	FILE *fp = fopen(filename, "wt");
	fprintf(fp, "0x%x type=%d, %s\n", (INT_PTR)pnode, pnode->type, pnode->str);
	print_links0(fp, pnode);
	fclose(fp);
}
#endif //GRAFFY

CAstSig::CAstSig(const CAstSig &org)
{
	throw "Internal error: Copy constructor is prohibited. You probably meant to call CAstSig(&env)." ;
}

//RECOMMENDED CONSTRUCTOR 1  7/8/2017
CAstSig::CAstSig(CAstSigEnv *env) // Use this constructor for auxlab. env has been defined prior to this.
{
	init();
	pEnv = env;

	//FILE *fp = fopen("ast_mem_track.txt", "at");
	//string timeStr;
	//GetLocalTimeStr(timeStr);
	//fprintf(fp, "%s CAstSig created 0x%x from pEnv= 0x%x\n", timeStr.c_str(), this, env);
	//fclose(fp);

}
//RECOMMENDED CONSTRUCTOR 2  7/8/2017
//Now (10/10/2018), if this is used not for the temporary variable during func in AuxFunc.cpp, then the validity of commenting out dad = src->dad; should be checked.
CAstSig::CAstSig(const CAstSig *src)
{
	init();
	Vars = src->Vars;
	GOvars = src->GOvars;
	u = src->u;
	fpmsg = src->fpmsg;
	if (src) {
		pEnv = src->pEnv;
	} else
		pEnv = new CAstSigEnv(DefaultFs);
	pAst = src->pAst;
	endpoint = src->endpoint;
	pgo = src->pgo;
	Script = src->Script;
	pLast = src->pLast;
}
// Used in auxfunc.cpp and psyntegDlgNIC.cpp
CAstSig::CAstSig(const char *str, const CAstSig *src) 
{
	init();
	if (src) 
	{
		pEnv = src->pEnv;
		Vars = src->Vars;
		u = src->u;
		fpmsg = src->fpmsg;
		endpoint = src->endpoint;
	}
	else
	{
		pEnv = new CAstSigEnv(DefaultFs);
	}
	string emsg;
	SetNewScript(emsg, str);

	if (src)
	{
		pgo = src->pgo;
		Script = src->Script;
		dad = src->dad;
		pLast = src->pLast;
	}
}
// Used in dancer
CAstSig::CAstSig(const char *str, CAstSigEnv *env)
{
	init();
	pEnv = env;
	string emsg;
	SetNewScript(emsg, str);
}

CAstSig::CAstSig(AstNode *pnode, const CAstSig *src)
{
	init();
	Vars = src->Vars;
	u = src->u;
	fpmsg = src->fpmsg;
	pAst = pnode;
	if (src) {
		pEnv = src->pEnv;
	}
	else
		pEnv = new CAstSigEnv(DefaultFs);
	endpoint = src->endpoint;
	pgo = src->pgo;
	Script = src->Script;
	dad = src->dad;
	pLast = src->pLast;
}

CAstSig::CAstSig(AstNode *pnode, CAstSigEnv *env)
{
	init();
	pAst = pnode;
	pEnv = env;
}

void CAstSig::init()
{
	pAst = NULL;
	Script = "";
	statusMsg = "";
	fAllocatedAst = false;
	fBreak = false;
	fExit = false;
	pLast = NULL;
	son = NULL;
	dad = NULL;

	FsFixed = false;
	audio_block_ms = 300;
	pgo = NULL;
	Tick0 = 1;
}

CAstSig::~CAstSig()
{
//	FILE *fp = fopen("ast_mem_track.txt", "at");
//	string timeStr;
//	GetLocalTimeStr(timeStr);
//	fprintf(fp, "%s 0x%x out, pAst=0x%x, %s, fAllocatedAst=%d\n", timeStr.c_str(), this, pAst, Script.c_str(), (int)fAllocatedAst);
//	fclose(fp);
	if (fAllocatedAst)
		yydeleteAstNode(pAst, 0);
}

unsigned long CAstSig::tic()
{
	return Tick0 = GetTickCount0();
}

unsigned long CAstSig::toc()
{
	if (Tick0==1)
		throw CAstException(pAst, this, "toc called without tic");
	return Tick1 = (GetTickCount0() - Tick0);
}

AstNode *CAstSig::SetNewScript(string &emsg, const char *str, const char *premsg)
{
	// New rules when using SetNewScript 12/19/2017
	// when it fails, it returns NULL and errstr should carry the message, so that the caller can throw an exception
	// when it suceeds or just needs to return NULL, errstr is empty.
	// errstr shall be allocated by the caller
	emsg.clear();
	int res;
	char *errmsg;
	if (strlen(str)==0) return pAst;
	if (pAst && Script == str)
		return pAst;
	if (fAllocatedAst) {
		yydeleteAstNode(pAst, 0);
		fAllocatedAst = false;
	}
	pAst = NULL;
	if ((res = yysetNewStringToScan(str)))
	{
		emsg = "yysetNewStringToScan() failed!";
		return NULL;
	}
 	res = yyparse(&pAst, &errmsg);
	fAllocatedAst = pAst ? true : false;
	if (!errmsg && res == 2)
	{
		emsg = "Out of memory!";
		return NULL;
	}
	if (errmsg) {
		if (fAllocatedAst) {
			yydeleteAstNode(pAst, 0);
			fAllocatedAst = false;
			pAst = NULL;
		}
		if (premsg)
		{
			char errStr[256];
			strcpy(errStr, premsg);
			strcat(errStr, "\n");
			strcat(errStr, errmsg);
			errmsg = errStr;
		}
		emsg = errmsg;
		return NULL;
	}
	Script = str;
	return pAst;
}

#ifdef NO_PLAYSND // just for psynteg 11.17.2017
HWND GetHWND_WAVPLAY()
{return NULL;}
#endif



bool GfInterrupted;	// needs to be global to interrupt even inside UDF
void CAstSig::set_interrupt(bool ch)
{
	GfInterrupted = ch;
}
bool CAstSig::isInterrupted(void)
{
	return GfInterrupted;
}


CVar &CAstSig::Eval(AstNode *pnode)
{
	try {
		if (!pnode)
			return Sig;
		return Compute(pnode); 
	} catch (const CAstException &e) {
		char errmsg[500];
		strncpy(errmsg, e.getErrMsg().c_str(), sizeof(errmsg)/sizeof(*errmsg));
		errmsg[sizeof(errmsg)/sizeof(*errmsg)-1] = '\0';
		throw errmsg;
	}
}

AstNode *CAstSig::goto_line(const AstNode *pnode, int line)
{
	AstNode *pp, *p = (AstNode *)pnode;
	for (; p; p = p->next)
	{
		if (p->type==T_IF || p->type == T_FOR || p->type == T_WHILE) // line should be inside of block, i.e., T_FOR T_IF or T_WHILE
		{
			pp = CAstSig::goto_line((const AstNode*)p->child->next, line);
			if (pp) return pp;
		}
		if (p->line == line) return p;
	}
	return p;
}


int CAstSig::isthislocaludf(void)
{// check if this AstSig has been created to process a local udf
// assume: Script has been set.
	map<string, UDF>::iterator ref = pEnv->udf.find(Script);
	if (ref!=pEnv->udf.end()) return 0; // a UDF found with the name Script
	for (map<string, UDF>::iterator it=pEnv->udf.begin(); it!=pEnv->udf.end(); it++)
	{
		if (it->first!=Script)
		{
			for (AstNode *p=it->second.pAst; p; p=p->next)
				if (p==ref->second.pAst) return 1;
		}
	}
	return 0;
}

bool CAstSig::IsThisBreakpoint(const AstNode *pnode)
{
	CAstSig *tp = son ? son : this;
	if (!tp) return false;
	if (tp->pEnv->curLine == pnode->line) return true;
	try {
		for (auto line: pEnv->udf[u.base].DebugBreaks)
		{	if (pnode->line == line) return true;	}
		return false;
	}
	catch (out_of_range oor)
	{
		return false;
	}
}

void CAstSig::hold_at_break_point(const AstNode *pnode)
{
	//either returns immediately or calls xcom::HoldAtBreakPoint to hold
	// stepping: always hold
	// continuing: hold if current line is checked in DebugBreaks
	// Shift_F5: always call xcom::HoldAtBreakPoint and let it do the rest
	// Ctrl_F10: hold if current line is "clicked" line 
	// stepping_in shouldn't appear here (it would have become stepping)
	switch (u.debug.status)
	{
	case null:
		return;
	case stepping:
		fpmsg.HoldAtBreakPoint(this, pnode);
		break;
	case progress:
	case continuing:
		if (IsThisBreakpoint(pnode))
			fpmsg.HoldAtBreakPoint(this, pnode);
		break;
	}
}

void CAstSig::astsig_init(void(*fp1)(CAstSig *, DEBUG_STATUS, int), void(*fp2)(CAstSig *, const AstNode *), bool(*fp3)(const char *), void(*fp4)(CAstSig *), void(*fp5)(int), void(*fp6)(const char*), void(*fp6a)(const char*), void(*fp7)(CAstSig *, const char *, CVar))
{
	fpmsg.UpdateDebuggerGUI = fp1;
	fpmsg.HoldAtBreakPoint = fp2;
	fpmsg.IsCurrentUDFOnDebuggerDeck = fp3;
	fpmsg.ShowVariables = fp4;
	fpmsg.Back2BaseScope = fp5;
	fpmsg.UnloadModule = fp6;
	fpmsg.ValidateFig = fp6a;
	fpmsg.SetGoProperties = fp7;
}

CFuncPointers::CFuncPointers()
{
	UpdateDebuggerGUI = dummy_fun1;
	HoldAtBreakPoint = dummy_fun2;
	IsCurrentUDFOnDebuggerDeck = dummy_fun3;
	ShowVariables = dummy_fun4;
	Back2BaseScope = dummy_fun5;
	UnloadModule = dummy_fun6;
	ValidateFig = dummy_fun6a;
	SetGoProperties = dummy_fun7;
}

CFuncPointers& CFuncPointers::operator=(const CFuncPointers& rhs)
{
	UpdateDebuggerGUI = rhs.UpdateDebuggerGUI;
	HoldAtBreakPoint = rhs.HoldAtBreakPoint;
	IsCurrentUDFOnDebuggerDeck = rhs.IsCurrentUDFOnDebuggerDeck;
	ShowVariables = rhs.ShowVariables;
	Back2BaseScope = rhs.Back2BaseScope;
	UnloadModule = rhs.UnloadModule;
	SetGoProperties = rhs.SetGoProperties;
	return *this;
}


bool CAstSig::PrepareAndCallUDF(const AstNode *pCalling, CVar *pBase)
{
	if (!pCalling->str)
		throw CAstException(pCalling, this, "Internal error! CheckPrepareCallUDF()");
	u.currentLine = pCalling->line; // ? 10/18/2018
	// Check if the same udf is called during debugging... in that case Script shoudl be checked and handled...

	CAstSigEnv tempEnv(*pEnv);
	son = new CAstSig(&tempEnv);
	son->u = u;
	son->u.title = pCalling->str;
	son->u.debug.status = null;
	son->lhs = lhs;
	son->dad = this; // necessary when debugging exists with stepping (F10), the stepping can continue in tbe calling scope without breakpoints. --=>check 7/25
	son->fpmsg = fpmsg;
	auto itUDF = pEnv->udf.find(pCalling->str);
	if (itUDF != pEnv->udf.end())
	{
		son->u.pUDF = (*itUDF).second.pAst;
		son->u.pUDF_base = son->u.pUDF;
		son->u.base = son->u.pUDF->str;
	}
	else
	{
		auto jtUDF = pEnv->udf.find(u.base); // if this is to be a local udf, base should be ready through a previous iteration.
		if (jtUDF == pEnv->udf.end())
			throw CAstException(pCalling, this, "Internal error! CheckPrepareCallUDF()", "supposed to be a local udf, but AstNode with that name not prepared");
		son->u.pUDF_base = (*jtUDF).second.pAst;
		son->u.base = u.base; // this way, base can maintain through iteration.
		son->u.pUDF = (*pEnv->udf.find(u.base)).second.local[pCalling->str].pAst;
	}
	son->pAst = son->u.pUDF_base->child->next;
	//output argument string list
	son->u.argout.clear();
	AstNode *pOutParam = son->u.pUDF->alt;
	if (pOutParam) {
		if (pOutParam->type == N_VECTOR)
			for (AstNode *pf = pOutParam->child; pf; pf = pf->next)
				son->u.argout.push_back(pf->str);
		else
			throw CAstException(pOutParam, this, "Internal error! UDF output should be alt and N_VECTOR", pOutParam->type);
	}
	AstNode *pf, *pa;	 // formal & actual parameter; pa is for this, pf is for son
	//Checking requested output arguments vs formal output arguments
	if (lhs)
	{
		ostringstream oss;
		AstNode *p = lhs->type == N_VECTOR ? lhs->alt : lhs;
		for (son->u.nargout = 0; p && p->line == lhs->line; p = p->next, son->u.nargout++) {}
		if (son->u.nargout > (int)son->u.argout.size()) {
			oss << "Maximum number of return arguments for function '" << u.pUDF->str << "' is " << son->u.argout.size() << ".";
			throw CAstException(u.pUDF, this, oss.str());
		}
	}
	else
		son->u.nargout = (int) son->u.argout.size(); // probably not correct
	// input parameter binding
	pa = pCalling->alt;
	//If the line invoking the udf res = udf(arg1, arg2...), pa points to arg1 and so on
	if (pa && pa->type == N_ARGS)
		pa = pa->child;
	//If the line invoking the udf res = var.udf(arg1, arg2...), binding of the first arg, var, is done separately via pBase. The rest, arg1, arg2, ..., are done below with pf->next
	pf = son->u.pUDF->child->child;
	if (pBase) { son->SetVar(pf->str, pBase); pf = pf->next; }
	//if this is for udf object function call, put that psigBase for pf->str and the rest from pa
	for (son->u.nargin = 0; pa && pf; pa = pa->next, pf = pf->next, son->u.nargin++)
	{
		CVar tsig = Compute(pa);
		if (tsig.IsGO())
			son->SetVar(pf->str, pgo); // variable pf->str is created in the context of son
		else
			son->SetVar(pf->str, &tsig); // variable pf->str is created in the context of son
	}
	if (u.debug.status == stepping_in) son->u.debug.status = stepping;
	CAstSig::vecast.push_back(son);
	size_t nArgout = son->CallUDF(pCalling, pBase);
	// output parameter binding
	vector<CVar *> holder;
	size_t cnt = 0;
	// output argument transfer from son to this
	if (!lhs)	// no output parameter specified. --> first formal output arg goes to ans
	{
		if (son->u.argout.empty())
			Sig.Reset();
		else
		{
			if (son->Vars.find(son->u.argout.front())!= son->Vars.end())
				Sig = son->Vars[son->u.argout.front()];
			else
			{ 
				if (son->GOvars[son->u.argout.front()].size() > 1)
					Sig = *(pgo = MakeGOContainer(son->GOvars[son->u.argout.front()]));
				else if (son->GOvars[son->u.argout.front()].size() == 1)
					Sig = *(pgo = son->GOvars[son->u.argout.front()].front());
				else
					Sig = CVar();
			}
		}
	}
	else
	{
		auto count = 0;
		AstNode *pp = lhs;
		if (lhs && lhs->type == N_VECTOR) pp = lhs->alt;
		for (auto arg : son->u.argout)
		{
			if (son->Vars.find(arg) != son->Vars.end())
			{
				SetVar(pp->str, &son->Vars[arg]);
				if (count++ == 0) 
					Sig = son->Vars[arg]; //why is this needed?
				if (--nArgout == 0) break;
			}
			else if (son->GOvars.find(arg) != son->GOvars.end())
			{
				if (son->GOvars[arg].size() > 1)
				{ // need to make an CSIG_HDLARRAY object
					pgo = MakeGOContainer(son->GOvars[arg]);
				}
				else
					pgo = son->GOvars[arg].front();
				if (count++ == 0)
					Sig = *pgo; //Ghost output to console
				if (--nArgout == 0) break;
			}
			pp = pp->next;
		}
	}
	if ((son->u.debug.status == stepping || son->u.debug.status == continuing) && u.debug.status == null)
	{ // no b.p set in the main udf, but in these conditions, as the local udf is finishing, the stepping should continue in the main udf, or debug.status should be set progress, so that the debugger would be properly exiting as it finishes up in CallUDF()
		u.debug.GUI_running = true;
		if (son->u.debug.status == stepping) // b.p. set in a local udf
			u.debug.status = stepping; 
		else // b.p. set in other udf 
			u.debug.status = progress;
	}
	delete son;
	son = NULL;
	if (pgo) pgo->functionEvalRes = true;
	Sig.functionEvalRes = true;
	CAstSig::vecast.pop_back(); // move here????? to make purgatory work...
	return true;
}
//FYI--hangs at hold_at_break_point and ...
//FYI--Go to HoldAtBreakPoint in xcom if you want to see how things are handled during debugging when the execution hangs 

size_t CAstSig::CallUDF(const AstNode *pnode4UDFcalled, CVar *pBase)
{	
	// pUDF: the T_FUNCTION node pointer for the current UDF call, created after ReadUDF ("formal" context--i.e., how the udf file was read with variables used in the file)
	// pOutParam: AstNode for formal output variable (or LHS), just used inside of this function.
	// Output parameter dispatching (sending the output back to the calling worksapce) is done with pOutParam and lhs at the bottom.

	// u.debug.status is set when debug key is pressed (F5, F10, F11), prior to this call.
	// For an initial entry UDF, u.debug.status should be null
	SetVar("nargin", &CVar((double)u.nargin));
	SetVar("nargout", &CVar((double)u.nargout)); // probably not correct
	// If the udf has multiple statements, p->type is N_BLOCK), then go deeper
	// If it has a single statement, take it from there.
	AstNode *pFirst = u.pUDF->child->next;
	if (pFirst->type == N_BLOCK)	pFirst = pFirst->next;
	//Get the range of lines for the current udf
	u.currentLine = pFirst->line;
	AstNode *p;
	int line2;
	for (p = pFirst; p; p = p->next)
	{
		line2 = p->line;
		if (!p->next) // if the node is T_FOR, T_WHILE or T_IF, p-next is NULL is it should continue through p->child
		{
			if (p->type == T_FOR || p->type == T_WHILE)
				p = p->alt;
			else if (p->type == T_IF)
			{
				if (p->alt)
					p = p->alt;
				else
					p = p->child;
			}
		}
	}

	/* When this is called by auxcon, the memory for CAstSig::vecast is different from when called by xcom, so as a temporary measure,
	fpmsg.ShowVariables now includes CAstSig::vecast.push_back(son) and a duplicate vecast is managed on the xcom side.
	-- temporary solution xcom 1.31 12/9/2017 */
	if (strcmp(u.application, "xcom"))
		fpmsg.ShowVariables(this);
	//probably needed to enter a new, external udf (if not, may skip)
	if (pEnv->udf[u.base].newrecruit)
		fpmsg.UpdateDebuggerGUI(this, refresh, -1); // shouldn't this be entering instead of refresh? It seems that way at least to F11 an not-yet-opened udf 10/16/2018. But.. it crashes. It must not have been worked on thoroughly...
														//if this is auxconscript front astsig, enter call fpmsg.UpdateDebuggerGUI()
	if (Script == "auxconscript01927362()")
		/*u.debug.GUI_running = true, */ fpmsg.UpdateDebuggerGUI(this, entering, -1); // the rest of if's will be skipped
																						 //check if this is subject to debugging.
	if (u.debug.status == stepping)
		/*u.debug.GUI_running = true, */ fpmsg.UpdateDebuggerGUI(this, entering, -1);
	else
	{ // probably entrance udf... First, check if current udfname (i.e., Script) is found in DebugBreaks
	  // if so, mark u.debug.status as progress and set next breakpoint
	  // and call debug_GUI 
		vector<int> breakpoint = pEnv->udf[u.base].DebugBreaks;
		for (vector<int>::iterator it = breakpoint.begin(); it != breakpoint.end(); it++)
		{
			if (*it < u.currentLine) continue;
			if (*it <= line2) {
				u.debug.status = progress; u.nextBreakPoint = *it;
				u.debug.GUI_running = true, fpmsg.UpdateDebuggerGUI(this, entering, -1);
				break;
			}
		}
	}

#ifdef _DEBUG
	Beep(1000, 50);
#endif

//	CAstSig::print_links("aux2_son_inCallUDF.txt", son->pAst->next);
	p = pFirst;
	while (p)
	{
		pLast=p;
		// T_IF, T_WHILE, T_FOR are checked here to break right at the beginning of the loop
		if (p->type==T_ID || p->type == T_FOR || p->type == T_IF || p->type == T_WHILE || p->type == N_IDLIST || p->type == N_VECTOR)
			hold_at_break_point(p);
		Compute(p);
		if (fExit) break;
		p=p->next;
	}
	if (!strcmp(u.application, "auxcon") && Script == "auxconscript01927362()")
		fpmsg.Back2BaseScope(0);
	if (u.debug.status!=null)
	{
//		currentLine = -1; // to be used in CDebugDlg::ProcessCustomDraw() (re-drawing with default background color)... not necessary for u.debug.status==stepping (because currentLine has been updated stepping) but won't hurt
		if (u.debug.GUI_running == true)
		{
			// send to purgatory and standby for another debugging key action, if dad is the base scope
			if (dad == CAstSig::vecast.front() && u.debug.status == stepping) 
			{
				fpmsg.UpdateDebuggerGUI(this, purgatory, -1);
				fpmsg.HoldAtBreakPoint(this, pLast);
			}
			u.currentLine = -1;
			u.debug.inPurgatory = false; // necessary to reset the color of debug window listview.
			//when exiting from a inside udf (whether local or not) to a calling udf with F10 or F11, the calling udf now should have stepping.
			fpmsg.UpdateDebuggerGUI(this, exiting, -1);
		}
		if (CAstSig::vecast.size()>2) // pvevast hasn't popped yet... This means son is secondary udf (either a local udf or other udf called by the primary udf)
		{//why is this necessary? 10/19/2018
			if (u.debug.status==stepping && fpmsg.IsCurrentUDFOnDebuggerDeck && !fpmsg.IsCurrentUDFOnDebuggerDeck(Script.c_str()))
				fpmsg.UpdateDebuggerGUI(this, entering, -1);
		}
	}
	return u.nargout;
}

void AddConditionMeetingBlockAsChain(CVar *Sig, CVar *psig, unsigned int iBegin, unsigned int iNow, CVar &part)
{
	part.UpdateBuffer(iNow-iBegin);
	memcpy(part.buf, (void*)(psig->buf+iBegin), (iNow-iBegin)*sizeof(double));
	part.tmark = 1.e3*iBegin/psig->GetFs();
	if (Sig->nSamples==0)	*Sig = part;
	else					Sig->AddChain(part);
}


const AstNode *CAstSig::getparentnode(const AstNode *ptree, const AstNode *p)
{ // This finds the parent tree of p in the ptree structure
	if (ptree->child == p) return ptree;
	if (ptree->alt == p) return ptree;
	if (ptree->next== p) return ptree;
	const AstNode *pp = ptree->child;
	while (pp)
	{
		if (getparentnode(pp, p)) return pp;
		pp = pp->child;
	}
	pp = ptree->alt;
	while (pp)
	{
		if (getparentnode(pp, p)) return pp;
		pp = pp->alt;
	}
	pp = ptree->next;
	while (pp)
	{
		if (getparentnode(pp, p)) return pp;
		pp = pp->next;
	}
	return NULL;
}

AstNode *CAstSig::searchtree(AstNode *p, int type)
{ // if there's a node with "type" in the tree, return that node
	if (p)
	{
		if (p->type==type) return p;
		if (p->child)
			if (p->child->type == type) return p->child;
			else
			{
				if (searchtree(p->child, type)) return p->child;
				if (searchtree(p->alt, type)) return p->alt;
			}
		if (p->alt)
			if (p->alt->type == type) return p->alt;
			else
			{
				if (searchtree(p->alt, type)) return p->alt;
				if (searchtree(p->child, type)) return p->child;
			}
		if (p->next)
			if (p->next->type==type) return p->next;
			else return searchtree(p->next, type);
	}
	return NULL;
}

bool CAstSig::isContiguous(body &ind, unsigned int &begin, unsigned int &end)
{ // in out out
	bool contig(true);
	double id, id0(ind.buf[0]);
	double diff = ind.buf[1] - id0;
	for (unsigned int k=1; k<ind.nSamples && contig; k++)
	{
		id = ind.buf[(int)k];
		if (id-id0 != diff || diff!=1. && diff!=-1.)
			contig = false;
		id0 = id;
	}
	if (diff>0) begin =  INT(ind.buf[0]), end =  INT(ind.buf[ind.nSamples-1]);
	else	end =  INT(ind.buf[0]), begin =  INT(ind.buf[ind.nSamples-1]);
	return contig;
}

CVar &CAstSig::extract(char *errstr, CVar **pinout, body &isig)
{   
	//pinout comes with input and makes the output
	//At the end, Sig is updated with pinout
	//pinout always has the same or smaller buffer size, so let's not worry about calling UpdateBuffer() here
	//Instead of throw an exception, errstr is copy'ed. Return value should be ignored.
	// For success, errstr stays empty.
	errstr[0] = 0;
	CSignals out((*pinout)->GetFs());
	CSignals *p = &out;
	out.UpdateBuffer(isig.nSamples);
	//CSignal::Min() makes a vector
	//body::Min() makes a scalar.
	if (isig._min() <= 0. )
	{
		strcpy(errstr, "Index must be positive.");
		return Sig;
	}
	if ((*pinout)->IsComplex())
	{
		out.SetComplex();
		for (unsigned int i=0; i<isig.nSamples; i++)
			out.cbuf[i] = (*pinout)->cbuf[(int)isig.buf[i]-1];
	}
	else if ((*pinout)->IsLogical())
	{
		out.MakeLogical();
		for (unsigned int i=0; i<isig.nSamples; i++)
			out.logbuf[i] = (*pinout)->logbuf[(int)isig.buf[i]-1];
	}
	else if ((*pinout)->IsString())
	{
		out.UpdateBuffer(isig.nSamples+1); // make room for null 
		for (unsigned int i=0; i<isig.nSamples; i++)
			out.strbuf[i] = (*pinout)->strbuf[(int)isig.buf[i]-1];
		out.strbuf[out.nSamples-1]=0;
	}
	else
	{
		if ((*pinout)->IsGO())
		{
			if (isig._max()>(*pinout)->nSamples)
			{
				sprintf(errstr, "Index out of range: %d", (int)isig._max());
				return Sig;
			}
			if (isig.nSamples == 1)
			{
				size_t did = (size_t)isig.value() - 1;
				CVar *tp = (CVar*)(INT_PTR)(*pinout)->buf[did];
				pgo = *pinout = tp;
				return Sig = **pinout;
			}
			else
			{
				vector<INT_PTR> gos;
				for (unsigned int k = 0; k < isig.nSamples; k++)
				{
					size_t did = (size_t)isig.buf[k] - 1;
					CVar *tp = (CVar*)(INT_PTR)(*pinout)->buf[did];
					gos.push_back((INT_PTR)tp);
				}
				*pinout = MakeGOContainer(gos);
				return Sig = *pinout;
			}
		}
		out.SetReal();
		if (Sig.GetType() == CSIG_VECTOR)
		{
			int id(0);
			for (unsigned int i = 0; i < isig.nSamples; i++)
				out.buf[id++] = (*pinout)->buf[(int)isig.buf[i] - 1];
		}
		else if (Sig.GetType() == CSIG_AUDIO)
		{ // 
			int cum(0), id(0), lastid = -2;
			vector<int> size2reserve;
			for (unsigned int k = 0; k < isig.nSamples; k++)
			{
				id = (int)isig.buf[k];
				if (id - lastid > 1)
				{
					if (lastid>0) size2reserve.push_back(lastid);
					size2reserve.push_back(id);
				}
				lastid = id;
			}
			size2reserve.push_back((int)isig.buf[isig.nSamples - 1]);
			auto it = size2reserve.begin();
			p->UpdateBuffer(*(it + 1) - *it + 1);
			p->tmark = (double)(isig.buf[0] - 1) / Sig.GetFs() * 1000.;
			it++; it++;
			lastid = (int)isig.buf[0] - 1;
			for (unsigned int i = 0; i < isig.nSamples; i++)
			{
				id = (int)isig.buf[i] - 1;
				if (id - lastid > 1)
				{
					cum = 0;
					CSignals *pchain = new CSignals(Sig.GetFs());
					pchain->UpdateBuffer(*(it + 1) - *it + 1);
					pchain->tmark = (double)id / Sig.GetFs() * 1000.;
					p->chain = pchain;
					p = pchain;
					it++; it++;
				}
				p->buf[cum++] = (*pinout)->buf[id];
				lastid = id;
			}
		}
	}
	out.nGroups = isig.nGroups;
	return (Sig=out);
}

bool CAstSig::checkcond(const AstNode *p)
{
	ConditionalOperation(p, p->child);
	if (!Sig.IsScalar())	throw CAstException(p, this, "--conditional op requires a scalar.");
	if (Sig.IsLogical()) 
		return Sig.logbuf[0];
	else				
		return Sig.value()!=0.;
}

void CAstSig::checkindexrange(const AstNode *pnode, CTimeSeries *inout, unsigned int id, string errstr)
{
	if (id>inout->nSamples) 
	{
		string out(errstr);
		out += " index ";
		char buf[64];
		sprintf(buf, "%d out of range", id);
		out += buf;
		throw CAstException(pnode, this, out);
	}
}

int CAstSig::checkpositiveinteger(const AstNode *pnode, CVar *id)
{
	// This must be called after id is scalar.
	double did = id->value();
	if (did<1.)
		throw CAstException(pnode, this, "index must be greater than 0");
	if ((double)(int)did - did > .05 || did- (double)(int)did>.05)
		throw CAstException(pnode, this, "index must be integer");
	return (int)did;
}

CAstSig &CAstSig::insertreplace(const AstNode *pnode, CTimeSeries *inout, CVar &sec, CVar &indsig)
{
	bool logicalindex = indsig.IsLogical();
	AstNode *p = pnode->alt; 
	if (inout->GetType()!= CSIG_AUDIO && indsig.IsLogical())
	{ // For non-audio, if isig is the result of logical operation, get the corresponding indices 
		CSignals trueID(1);
		trueID.UpdateBuffer(indsig.nSamples);
		int m=0;
		for (unsigned int k(0); k<indsig.nSamples; k++)
			if (indsig.logbuf[(int)k]) 
				trueID.buf[m++]=k+1; // because aux is one-based index
		trueID.UpdateBuffer(m);
		indsig = trueID;
	}
	if (!indsig.nSamples) return *this;
	else if (indsig.IsScalar())
	{
		if (inout->GetType() == CSIG_AUDIO)
		{	
			if (sec.IsTimeSignal())		// s(tp) = sound; //insert
				inout->Insert(indsig.value(), sec);
			else
			{
				unsigned int id = (unsigned int)round(indsig.value());
				checkindexrange(pnode, inout, id, "LHS##vsdba2");
				inout->buf[id - 1] = sec.value();
			}
		}
		else
		{
//			if (sec.length()!=1) throw CAstException(pnode, this, "RHS must be a scalar");
			unsigned int id = (unsigned int)round(indsig.value());
			checkindexrange(pnode, inout, id, "LHS");
			if (sec.IsComplex())
			{
				inout->SetComplex();
				inout->cbuf[id - 1] = sec.cvalue();
			}
			else
				inout->replace(sec, id - 1, id - 1);
		}
	}
	else // not done yet if sec is complex
	{
//		pnode->type is either N_IXASSIGN (for non-cell LHS)
//		or cell index for cell LHS,  for example, T_NUMBER for cc{3}
		unsigned int id1, id2;
		if (inout->GetType()==CSIG_AUDIO) 
		{
			AstNode *tp = searchtree(pnode->child, T_REPLICA);
			if (pnode->type==N_ARGS || (pnode->next && pnode->next->type==N_CALL) )
			{
				if (indsig.IsLogical()) // s(conditional_var)
				{
					for (CTimeSeries *p=&sec; p; p = p->chain)
					{
						int id((int)(p->tmark*GetFs()/1000+.5)); 
						for (unsigned int k=0; k<p->nSamples; k++)
							if (indsig.logbuf[id+k]) 
								inout->buf[id+k] = p->buf[(int)k];
					}
				}
				else // s(id1:id2) or cel{n}(id1:id2)
				{ // x(1200:1201) = zeros(111) FAILED HERE	on memcpy line.... that's wrong. 1/20/2018
					// this must be contiguous
					if (!isContiguous(indsig, id1, id2))
					{
	//					if (!sec.IsScalar())
	//						throw CAstException(pnode, this, "to replace audio signal with a non-scalar object, the indices must be contiguous.");
						inout->replace(sec, indsig); // if sec is a scalar, just assignment call by index 
					}
					else
					{
						checkindexrange(pnode, inout, id1, "LHS");
						checkindexrange(pnode, inout, id2, "LHS");
						if (sec.nSamples != id2 - id1 + 1)
						{
							if (!sec.IsScalar())
								throw CAstException(pnode, this, "to manipulate an audio signal by indices, LHS and RHS must be the same length or RHS is a scalar");
						}
						if (sec.IsComplex() && !inout->IsComplex()) inout->SetComplex();
						inout->replace(sec, id1 - 1, id2 - 1);
					}
				}
			}
			else if (!pnode->next && !pnode->str) // if s(conditional) is on the LHS, the RHS must be either a scalar, or the replica, i.e., s(conditional)
			{
				if (!tp && !indsig.IsLogical()) throw CAstException(pnode, this, "Internal logic error (insertreplace:0)--s(conditional?).");
				if (sec.IsScalar())
				{
					double val = sec.value();
					for (CTimeSeries *piece(inout), *index(&indsig); piece; piece = piece->chain, index = index->chain)
					{
						for (unsigned int k = 0; k<index->nSamples; k++)
							if (index->logbuf[(int)k]) piece->buf[(int)k] = val;
					}
				}
				else
				{ // RHS is conditional (can be replica)
				  // At this point no need to worry about replacing null with non-null (i.e., signal is always non-null in the signal portions of sec. 
				  //   4/13/2017
					for (CTimeSeries *p = &sec; p; p = p->chain)
					{
						int id((int)(p->tmark*GetFs() / 1000 + .5));
						for (unsigned int k = 0; k<p->nSamples; k++)
							inout->buf[id + k] = p->buf[(int)k];
					}
				}
			}
			else if (p->type == N_TIME_EXTRACT || (p->next && p->next->type == N_IDLIST))  // s(tp1~tp2)   or  cel{n}(tp1~tp2)
				inout->ReplaceBetweenTPs(sec, indsig.buf[0], indsig.buf[1]);
			else if (pnode->alt->type == N_HOOK) 
			{
				inout->ReplaceBetweenTPs(sec, indsig.buf[0], indsig.buf[1]);
			}
			else
				throw CAstException(pnode, this, "Internal logic error (insertreplace:1) --unexpected node type.");
		}
		else
		{
			// v(1:5) or v([contiguous]) = (any array) to replace
			// v(1:2:5) or v([non-contiguous]) = RHS; //LHS and RHS must match length.
			bool contig = isContiguous(indsig, id1, id2);
			if (!sec.IsEmpty() && !contig && sec.nSamples!=1 && sec.nSamples!=indsig.nSamples) throw CAstException(pnode, this, "the number of replaced items must be the same as that of replacing items.");
			if (contig)
				inout->replace(sec, id1-1, id2-1);
			else 
				inout->replace(sec, indsig);
		}
	}
	return *this;
}

AstNode *CAstSig::SetNewScriptFromFile(string &emsg, const char *full_filename, const char *udf_filename, string &filecontent)
{
	// if filecontent is not empty, it doesn't check whether the file exits and checks only if the content is the same
	if (filecontent.empty())
	{
		if (GetFileText(full_filename, "rt", filecontent) < 0)
		{
			emsg = "SetNewScriptFromFile: GetFileText errror.";
			return NULL;
		}
	}
	// if udf_filename is NULL, this was called by aux_include (a script, not a udf)
	if (!udf_filename)
	{
		if (!SetNewScript(emsg, filecontent.c_str()))
			return NULL;
		}
	else
	{ // file exists. let's see if the udf was already registered.
		if (pEnv->udf.find(udf_filename) != pEnv->udf.end())
		{// Check if the content of the udf file is the same as filecontent; then return NULL
			if (pEnv->udf[udf_filename].content == filecontent.c_str())
			{
				emsg.clear();
				return NULL;
			}
		}
		Script = udf_filename; // will not have any lingering effects. The context stays within CAstSig aux(pEnv) in ReadUDF()
		pEnv->udf[udf_filename].newrecruit = true;
		if (!SetNewScript(emsg, filecontent.c_str()))
			return NULL; // syntax error in auxcon script is caught here
	}
	return pAst;
}

typedef  int(_cdecl  *PF) (int, std::string &);

string CAstSig::LoadPrivateUDF(HMODULE h, int id, string &emsg)
{
	PF pt = (PF)GetProcAddress(h, (LPCSTR)MAKELONG(1, 0)); // ReadAUXP
	string read;
	size_t res = pt(id, read); 
	CAstSig aux(pEnv);
	AstNode *tempAst = aux.SetNewScript(emsg, read.c_str());
	size_t wh1, wh2;
	string fname, out;
	if (tempAst) {
		wh1 = read.find('=');
		wh2 = read.find('(');
		out = fname = read.substr(wh1 + 1, wh2 - wh1 - 1);
		trim(fname, ' ');
		fname.insert(0, "private://");
		RegisterUDF(aux.pAst, fname.c_str(), read);
		aux.pAst = NULL;// To keep aux.pAst from deallocating when this function returns. This will be cleaned during the app shutdown. 9/19/2018
		return out;
	}
	return "";
}

AstNode *CAstSig::ReadUDF(string &emsg, const char *udf_filename, const char *internaltransport)
{
	/* internaltransport carries the content of AUXCON script when ReadUDF() is called in auxconDlg.cpp
	*/
	if (!udf_filename) return NULL;
	emsg.clear();
	string fullpath, filecontent("");
	AstNode *pout;
	if (pEnv->udf.empty())
	{ // Read aux private functions
	}
	if (!internaltransport) // not from AUXCON
	{
		//local functions have precedence; i.e., if fun1() has myloc() in it, the file myloc.aux will not be searched.
		map<string, UDF>::iterator it = pEnv->udf.find(u.base);
		if (it != pEnv->udf.end())
		{
			map<string, UDF>::iterator jt = it->second.local.find(udf_filename);
			if (jt != it->second.local.end())
				return jt->second.pAst;
		}
		//Search for the file 
		FILE *auxfile = OpenFileInPath(udf_filename, "aux", fullpath);
		if (!auxfile)
		{ //if not found, it shoulbe be a built-in or local function, then return
			if (pEnv->udf.find(udf_filename) == pEnv->udf.end())
				return NULL;
			else // file not found but it is not a local function.... what can it be?
				return pEnv->udf[udf_filename].pAst; // check
		}
		//file found
		fclose(auxfile);	
		transform(fullpath.begin(), fullpath.end(), fullpath.begin(), ::tolower);
	}
	else
	{ //from AUXCON
		filecontent = internaltransport;
		fullpath = string("9:\\module\\") + udf_filename;
	}

	CAstSig aux(pEnv);
	//read the file;  if new, parse the string and set new pAst. If the udf was already registered, it returns NULL.
	AstNode *tempAst = aux.SetNewScriptFromFile(emsg, fullpath.c_str(), udf_filename, filecontent);
	if (tempAst) {
		aux.pAst = tempAst;
		pout = RegisterUDF(aux.pAst, fullpath.c_str(), filecontent);
		// The following should be after all the throws. Otherwise, the UDF AST will be dangling.
		// To prevent de-allocation of the AST of the UDF when the aux is destroyed.
		if (aux.pAst->type == N_BLOCK)
			aux.pAst->next = NULL;
		else
			aux.pAst = NULL;
	}
	else
	{
		if (emsg.empty())
			pout = pEnv->udf[udf_filename].pAst;
		else
		{ // parsing error in udf file
			char buf[256];
			sprintf(buf, " in %s", fullpath.c_str());
			emsg += buf;
			return NULL;
		}
	}
	//pout->child should be ID_LIST
	if (!pout->child->child) // function takes no input argument
		pout->suppress = 3;
	return pout;
}

AstNode *CAstSig::RegisterUDF(const AstNode *p, const char *fullfilename, string &filecontent)
{ 
	//Deregistering takes place during cleaning out of pEnv i.e., ~CAstSigEnv()
	char udf_filename[256];
	_splitpath(fullfilename, NULL, NULL, udf_filename, NULL);
	AstNode *pnode4Func = (AstNode *)((p->type == N_BLOCK) ? p->next : p);
	//for (AstNode *pp = pnode4Func; pp; pp = pp->next)
	//	assert(pp->type != T_FUNCTION); 	//if not,	throw CAstException(pp, this, "All codes in AUX file must be inside function definitions.");
	if (strcmp(udf_filename, pnode4Func->str)) // pnode4Func is NULL for local function call and it will crash.....8/1/
		throw CAstException(p, this, "inconsistent function name", string(string(string(udf_filename) + " vs ") + pnode4Func->str).c_str());

	pEnv->udf[udf_filename].pAst = pnode4Func;	
	pEnv->udf[udf_filename].fullname = fullfilename;
	pEnv->udf[udf_filename].content = filecontent.c_str();
	for (AstNode *pp = pnode4Func->next; pp; pp = pp->next)
	{// if one or more local functions exists
		UDF loc;
		loc.pAst = pp;
		loc.fullname = fullfilename;
		loc.content = "see base function for content";
		pEnv->udf[udf_filename].local[pp->str] = loc;
	}
	return pnode4Func;
}

CVar &CAstSig::SetLevel(const AstNode *pnode, AstNode *p)
{ // CHECK this..............5/25/2018
	bool trinary(false);
	CSignals tsig, isig,  rms, refRMS;
	tsig = Compute(p->next); // p->next points to the last operand.
	bool tseq = (tsig.nSamples == 1 && tsig.chain);
	if (!tseq && tsig.GetType()!=CSIG_SCALAR && tsig.GetType() != CSIG_VECTOR)
		throw CAstException(pnode, this, "The last operand should be scalar or 2-element vector.");
	if (p->type == '@') 
	{ // trinary
		CSignals second = Compute(p->child->next);
		if (second.GetType()==CSIG_AUDIO)
		{	
			trinary = true;
			double srms = second.CSignal::RMS();
			if (srms == -std::numeric_limits<double>::infinity())
				throw CAstException(pnode, this, "x @ ref @ value ---- Invalid: RMS of ref is infinity.");
			if (isnan(srms))
				throw CAstException(pnode, this, "x @ ref @ value ---- Invalid: Left chan of ref is NULL.");
			refRMS.SetValue(srms);
			if (second.next && second.next->nSamples)
			{
				isig.SetValue(second.next->RMS());
				refRMS.SetNextChan(&isig);
			}
			Compute(p->child);	// Sig has the first operand
			if (!Sig.IsStereo() && second.IsStereo())
				throw CAstException(pnode, this, "x @ ref @ value ---- if x is mono, ref must be mono.");
		}
		else
			throw CAstException(pnode, this, "A @ B @ C ---- B must be an audio signal.");
	}
	else
	{ // binary
		refRMS.SetValue(-0.000262);	// the calculated rms value of a full scale sinusoid (necessary to avoid the clipping of rms adjusted full scale sinusoid)
		Compute(p); // Sig has the first operand
	}
	checkAudioSig(pnode,  Sig);
	if (!Sig.IsStereo() && !tsig.IsScalar()) throw CAstException(p->next, this, "Mono signal should be scaled with a scalar.");
	double mrms = Sig.CSignal::RMS();
	if (mrms == -std::numeric_limits<double>::infinity())
		throw CAstException(pnode, this, "Invalid: The signal is all-zero (-infinity RMS).");
	rms.SetValue(-mrms);
	if (Sig.IsStereo() && Sig.next->nSamples)
	{
		if (tsig.nSamples>2)
			throw CAstException(pnode, this, "For a stereo signal, the last operand should be a vector of two or less elements.");
		double msrms = Sig.next->CSignal::RMS();
		if (msrms == -std::numeric_limits<double>::infinity())
			throw CAstException(pnode, this, "Invalid: The signal (2nd Chan) is all-zero (-infinity RMS).");
		isig.SetValue(-msrms);
		rms.SetNextChan(&isig);
		if (tsig.nSamples==2)
		{
			double tp = tsig.buf[1];
			tsig.SetValue(tsig.buf[0]);
			isig.SetValue(tp);
			tsig.SetNextChan(&isig);
		}
	}
	rms += refRMS;
	rms += tsig;
	rms *= LOG10DIV20;
	if (rms.GetType()==CSIG_AUDIO || rms.GetType()==CSIG_VECTOR)
		Sig *= rms;
	else
		Sig *= rms.each(exp).transpose1();
	return Sig;
}


void CAstSig::prepare_endpoint(const AstNode *p, CVar *pvar)
{  // p is the node the indexing starts (e.g., child of N_ARGS... wait is it also child of conditional p?
	if (p->next) // first index in 2D 
		endpoint = (double)pvar->nGroups;
	else
		endpoint = (double)pvar->nSamples;
	//else
	//	endpoint = (double)pvar->Len();
}

void CAstSig::interweave_indices(CVar &isig, CVar &isig2, unsigned int len)
{
	CVar out;
	out.UpdateBuffer(isig.nSamples * isig2.nSamples);
	out.nGroups = isig.nGroups;
	for (size_t p=0; p < isig.nSamples; p++)
		for (unsigned int q = 0; q < isig2.nSamples; q++)
		{
			out.buf[p*isig2.nSamples + q] = (isig.buf[p]-1) * len + isig2.buf[q];
		}
	isig = out;
}

void CAstSig::index_array_satisfying_condition(CVar &isig)
{ // input: isig, logical array
  // output: isig, a new array of indices satisfying the condition
	int count = 0;
	for (unsigned int k = 0; k < isig.nSamples; k++)
		if (isig.logbuf[k]) count++;
	CVar out;
	out.UpdateBuffer(count);
	out.nGroups = isig.nGroups;
	count = 0;
	for (unsigned int k = 0; k < isig.nSamples; k++)
		if (isig.logbuf[k]) out.buf[count++] = k + 1;
	isig = out;
}

CVar &CAstSig::eval_indexing(const AstNode *pInd, CVar *psig, CVar &isig)
{
	// input: pInd, psig
	// output: isig -- sig holding all indices

	// process the first index
	unsigned int len;
	prepare_endpoint(pInd, psig);
	try {
		CAstSig tp(this);
		if (pInd->type == T_FULLRANGE)
		{ // x(:,ids) or x(:)
			isig.UpdateBuffer((unsigned int)endpoint);
			for (int k = 0; k <(int)isig.nSamples; k++)	isig.buf[k] = k + 1;
		}
		else
			isig = tp.Compute(pInd);
		if (isig.IsLogical()) index_array_satisfying_condition(isig);
		// process the second index, if it exists
		if (pInd->next)
		{
			if (psig->nGroups > 1 && isig.nSamples > 1)
				isig.nGroups = isig.nSamples;
			AstNode *p = pInd->next;
			CVar isig2;
			if (p->type == T_FULLRANGE)
			{// x(ids,:)
				len = psig->Len();
				isig2.UpdateBuffer(len);
				for (unsigned int k = 0; k < len; k++)	isig2.buf[k] = k + 1;
			}
			else // x(ids1,ids2)
			{
				//endpoint for the second arg in 2D is determined here.
				tp.endpoint = (double)psig->Len();
				isig2 = tp.Compute(p);
			}
			char buf[128];
			if (isig2.IsLogical()) index_array_satisfying_condition(isig2);
			else if (isig2._max()>(double)psig->Len())
				throw CAstException(pInd, this, "Out of range: 2nd index ", itoa((int)isig2._max(), buf, 10));
			interweave_indices(isig, isig2, psig->Len());
		}
	}
	catch (const CAstException &e) {
		throw CAstException(pInd, this, e.getErrMsg());
	}
	return isig;
}

CVar &CAstSig::ExtractByIndex(const AstNode *pnode, AstNode *p, CVar **psig)
{ // pnode->type should be N_ARGS
	char ebuf[256];
	CVar tsig, isig, isig2;
	if (!p)	throw CAstException(pnode, this, "A variable index should be provided.");
	if ((*psig)->GetType() == CSIG_CELL) throw CAstException(p, this, "A cell array cannot be accessed with ( ).");
	eval_indexing(pnode->child, *psig, isig);
	Sig = extract(ebuf, psig, isig);
	if (ebuf[0])
	{
//		AstNode *pp = get_parent_node(pAst, (AstNode *)pnode, 2);
		throw CAstException(pnode, this, ebuf);
	}
	return Sig;
}

CVar &CAstSig::TimeExtract(const AstNode *pnode, AstNode *p, CVar *psig)
{
	char estr[256];
	if (!psig)
	{
		strcat(estr, " for time extraction ~");
		throw CAstException(pnode, this, estr);
	}
	checkAudioSig(pnode,  *psig);

	CTimeSeries *pts = psig;
	for (; pts; pts = pts->chain)
		endpoint = pts->CSignal::endt();
	if (psig->next)
	{
		pts = psig->next;
		endpoint = max(endpoint, pts->CSignal::endt());
	}
	CVar tp = gettimepoints(pnode, p);
	CVar out(*psig);
	out.Crop(tp.buf[0], tp.buf[1]);
	return Sig = out;
}

CVar &CAstSig::TSeq(const AstNode *pnode, AstNode *p)
{ 
	//For now (6/12/2018) only [vector1][vector2] where two vectors have the same length.
	CVar tsig2, tsig = Compute(p);
	int type1 = tsig.GetType();
	if (type1 != CSIG_SCALAR && type1 != CSIG_VECTOR)
		strcpy(pnode->str, "TSEQ"),	throw CAstException(pnode, this, "Invalid tmark array");
	if (pnode->child->next) {
		//[tvalues][val] pnode->child->next is N_VECTOR for val
		//[tvalues][val;] pnode->child->next is N_MATRIX for val
		tsig2 = Compute(pnode->child->next);
		checkVector(pnode, tsig2);
		int type2 = tsig2.GetType();
		if (type2 != CSIG_SCALAR && type2 != CSIG_VECTOR)
			strcpy(pnode->str, "TSEQ"), throw CAstException(pnode, this, "Invalid t-sequence value array");
		if (tsig2.nGroups == 1)
		{
			//if 
			if (pnode->child->next->type == N_VECTOR)
			{
				if (tsig2.nSamples != tsig.nSamples)
					strcpy(pnode->str, "TSEQ"), throw CAstException(pnode, this, "Time point and value arrays must have the same length.");
			}
			else //pnode->child->next->type == N_MATRIX
			{
				//ok
			}
		}
		else
		{
			if (tsig2.nGroups != tsig.nSamples)
				strcpy(pnode->str, "TSEQ"), throw CAstException(pnode, this, "The number of time points and time sequences must have the same.");
		}
	}
	Sig.Reset(tsig.GetFs());
	CTimeSeries tp, *run(&Sig);
	if (pnode->str && pnode->str[0] == 'R') //Relative tmarks
	{
		tp.SetFs(0); // new fs is 0.
		Sig.SetFs(0);
	}
	else // if it is not relative, fs is set from the current system default.
	tp.SetFs(GetFs());
	for (unsigned int k = 0; k < tsig.nSamples; k++)
	{
		tp.tmark = tsig.buf[k];
		if (tsig2.nSamples > 0)
		{
			if (tsig2.nGroups==1 && pnode->child->next->type == N_VECTOR)
				tp.SetValue(tsig2.buf[k]);
			else
			{
				unsigned int len = tsig2.Len();
				tp.UpdateBuffer(len);
				memcpy(tp.buf, tsig2.buf + len*k, len*sizeof(double));
			}
		}
		if (k == 0) *run = tp;
		else
		{
			run->chain = new CTimeSeries;
			run = run->chain;
			*run = tp;
		}
	}
	return Sig;
}

void aux_HOOK(CAstSig *past, const AstNode *pnode, const AstNode *p, int nArgs, string &fnsigs);

CVar &CAstSig::pseudoVar(const AstNode *pnode, AstNode *p, CSignals *pout)
{
	int res;
	string dummy;
	if (!pout) pout = &Sig;
	if (pnode->type == N_HOOK)
		HandlePseudoVar(pnode);
	else
	{

		//{
		//	aux_HOOK(this, pnode, p, 0, dummy);
		//	return Sig;
		//}
		if (p->type == N_ARGS) p = p->child;
		if (p->type == N_HOOK) p = p->child;
		if (!strcmp(p->str, "sel"))
		{
			res = _reserve_sel(this, p, pout);
			if (!p->alt)
			{
				//pvar = RetrieveVar(pnode, estr);
				//if (!pvar)
				//	throw CAstException(p, this, "context is required.", p->str);
				//Sig = *pvar;
				//Sig.Crop(out.buf[0], out.buf[1]);
			}
			else
			{
				double tp[2];
				memcpy(tp, pout->buf, sizeof(double) * 2);
				pout->Reset();
				if (!strcmp(p->alt->str, "t1"))
					pout->SetValue(tp[0]);
				else if (!strcmp(p->alt->str, "t2"))
					pout->SetValue(tp[1]);
			}
		}
	}
	return Sig; // nominal return value 
}

CVar &CAstSig::NodeMatrix(const AstNode *pnode, AstNode *p)
{ //[x1; x2]  if for a stereo audio signal, both x1 and x2 must be audio
	// if none of these elements are audio, it can have multiple rows [x1; x2; x3; .... xn]. But these elements must be the same length.
	CVar esig, tsig = Compute(p);
	blockCell(pnode,  tsig);
	int audio(0);
	if (tsig.GetType() == CSIG_AUDIO ) audio = 1;
	else if (tsig.GetType() == CSIG_TSERIES) audio = 10;
	else if (tsig.GetType() == CSIG_TSERIES) audio = 10;
	else if (tsig.GetType() == CSIG_STRING || tsig.GetType() == CSIG_SCALAR || tsig.GetType() == CSIG_VECTOR) audio = -1;
	if (audio == 0 && !p->next)
		return Sig.Reset();
	CVar *psig = &tsig;
	unsigned int k(1);
	AstNode *pp = pnode->tail; // temporary holder for the matrix-wide dot operation
	if (audio >= 0)
	{
		if (!p->next) // must be audio > 0
		{
			CTimeSeries *nullnext = new CTimeSeries(tsig.GetFs());
			tsig.SetNextChan(nullnext);
		}
		else
		{
			if (p->next->alt && p->next->alt->type == N_STRUCT && p->next->alt == pnode->tail) // in the last loop, if p->alt is structure and the same as pnode->tail; i.e., the "dot" operation applies to the matrix, not the vector, so, Compute(p), which tries to apply the dot opeartor to the vector through N_VECTOR, shouldn't be called. The matrix-wide dot operation should be handled at the bottom. 4/17/2018
				p->next->alt = NULL;
			esig = Compute(p->next);
			blockCell(pnode, esig);
			if (audio == 0 && (esig.GetType() == CSIG_AUDIO || esig.GetType() == CSIG_TSERIES))
			{
				if (p->next->next)	throw CAstException(pnode, this, "Currently two channels or less for audio signals or t-series are allowed.");
				tsig.SetNextChan(&esig);
			}
			else if (audio > 0)
			{
				if (p->next->next)	throw CAstException(pnode, this, "Currently two channels or less for audio signals or t-series are allowed.");
				if (esig.GetType() != tsig.GetType())
					throw CAstException(pnode, this, "Signal type, Audio (or t-series), must be the same in both channels.");
				tsig.SetNextChan(&esig);
			}
		}
	}
	else
	{
		for (p = p->next; p; p = p->next, k++) {
			if (!p->next && p->alt && p->alt->type == N_STRUCT && p->alt == pnode->tail) // in the last loop, if p->alt is structure and the same as pnode->tail; i.e., the "dot" operation applies to the matrix, not the vector, so, Compute(p), which tries to apply the dot opeartor to the vector through N_VECTOR, shouldn't be called. The matrix-wide dot operation should be handled at the bottom. 4/17/2018
				p->alt = NULL;
			esig = Compute(p);
//			if (esig.GetType() == CSIG_AUDIO || esig.GetType() == CSIG_TSERIES)
//				throw CAstException(pnode, this, "Audio signal and non-audio signal cannot be in a matrix.");
			if (!esig.IsEmpty() && tsig.Len() != esig.Len()) throw CAstException(pnode, this, "All groups must have the same length for 2-D data. (All rows must be the same length in the matrix)");
			tsig += &esig;
			tsig.nGroups += esig.nGroups;
		}
	}
	Sig = tsig;
	//Finally the matrix-wide dot operation
	/*if (pp && pp->type == N_STRUCT)
		NodeCall(pp, pp->child, NULL, true);*/
	return Sig;
}

CVar &CAstSig::NodeVector(const AstNode *pnode, AstNode *p)
{
	unsigned int len;
	vector<double> databuf;
	vector<complex<double>> cdatabuf;
	//First it checks whether every item has the same nGroups
	//Also checks if it's complex
	CVar tsig = Compute(p);
	if (tsig.GetType()==CSIG_AUDIO) return Sig = tsig;
	bool thisisGO = false;
	bool beginswithempty = tsig.IsEmpty();
	thisisGO = tsig.GetType() == CSIG_HDLARRAY || tsig.IsGO();
	unsigned int ngroups = tsig.nGroups; // final number of rows 
	unsigned int totalLen = tsig.Len(); // final number of cols
	bool compl=tsig.IsComplex();
	for (p = p->next; p; p = p->next)
	{
		tsig = Compute(p);
		if (thisisGO && (tsig.GetType() != CSIG_HDLARRAY && !tsig.IsGO()))
			throw CAstException(pnode, this, "Graphic Handle and non-Graphic handle cannot be in an array");
		if ((!thisisGO && !beginswithempty) && (tsig.GetType() == CSIG_HDLARRAY || tsig.IsGO()))
			throw CAstException(pnode, this, "Graphic Handle and non-Graphic handle cannot be in an array");
		if (ngroups != tsig.nGroups)
			throw CAstException(pnode, this, "Every item in the brackets must have the same nGroups.");
		totalLen += Sig.Len();
		if (!compl) compl = tsig.IsComplex();
		thisisGO = tsig.GetType() == CSIG_HDLARRAY || tsig.IsGO();
	}
	CVar out;
	if (thisisGO)
	{
		out.SetFs(3);
		for (p = pnode->child; p; p = p->next)
		{
			tsig = Compute(p);
			if (tsig.GetType() == CSIG_HDLARRAY)
			{
				unsigned int beginID = out.nSamples;
				out.UpdateBuffer(out.nSamples + tsig.nSamples);
				for (unsigned int k = 0; k < tsig.nSamples; k++)
					out.buf[beginID+k] = (double)(INT_PTR)tsig.buf[k];
			}
			else
			{ // tsig must be a copy of single GO--> get it from pgo
				out.UpdateBuffer(out.nSamples + 1);
				out.buf[out.nSamples-1] = (double)(INT_PTR)pgo;
			}
		}
		return Sig = out;
	}
	out.UpdateBuffer(ngroups*totalLen);
	out.nGroups = ngroups;
	if (compl) out.SetComplex();
	int col; // column locator
	for (col = 0, p=pnode->child; p; p=p->next)
	{
		Compute(p);
		out.SetFs(Sig.GetFs());
		out.tmark = Sig.tmark;
		if (compl && !Sig.IsComplex()) Sig.SetComplex();
		len = Sig.Len();
		unsigned int bufsize = Sig.bufBlockSize;
		for (unsigned int k = 0; k < ngroups; k++)
			memcpy(out.logbuf+(k*totalLen+col)*bufsize, Sig.logbuf+(k*len)*bufsize, bufsize*len);
		col += len;
	}
	out.bufBlockSize = Sig.bufBlockSize;
	return Sig = out;
}

CVar &CAstSig::gettimepoints(const AstNode *pnode, AstNode *p)
{ // assume: pnode type is N_TIME_EXTRACT
	CVar tsig1 = Compute(p);
	if (!tsig1.IsScalar())
		throw CAstException(pnode, this, "Time marker1 should be scalar");
	CSignals tsig2 = Compute(p->next);
	if (!tsig2.IsScalar())
		throw CAstException(pnode, this, "Time marker2 should be scalar");
	tsig1 += &tsig2;
	return Sig = tsig1;
}

bool CAstSig::isThisAllowedPropGO(CVar *psig, const char *propname, CVar &tsig)
{
	if (psig->strut.find(propname) == psig->strut.end())
		if (psig->struts.find(propname) == psig->struts.end())
			return false;
	if (!strcmp(propname, "pos")) // 4-element vector
		return (tsig.GetType() == CSIG_VECTOR && tsig.nSamples == 4);
	if (!strcmp(propname, "color")) // 3-element vector
		return tsig.nSamples == 3;
	if (!strcmp(propname, "visible")) // a real constant or bool
		return tsig.nSamples == 1 && tsig.bufBlockSize<=8;
	if (!strcmp(propname, "nextplot"))
		return (tsig.GetType() == CSIG_STRING);
	if (!strcmp(propname, "tag"))
		return (tsig.GetType() == CSIG_STRING);
	if (!strcmp(propname, "userdata"))
		return true; // everything is allowed for userdata

	if (psig->strut["type"] == string("axes"))
	{
		if (!strcmp(propname, "x") || !strcmp(propname, "y"))
			return true;
	}
	else if (psig->strut["type"] == string("axis"))
	{ 
		if (!strcmp(propname, "lim"))
			return (tsig.GetType() == CSIG_VECTOR && tsig.nSamples == 2);
		if (!strcmp(propname, "tick"))
			return (tsig.GetType() == CSIG_VECTOR);
	}
	else if (psig->strut["type"] == string("line"))
	{
		if (!strcmp(propname, "marker"))
			return (tsig.GetType() == CSIG_STRING && tsig.length() == 1);
		if (!strcmp(propname, "markersize") || !strcmp(propname, "width"))
			return (tsig.GetType() == CSIG_SCALAR);
		if (!strcmp(propname, "xdata") || !strcmp(propname, "ydata"))
			return (tsig.GetType() == CSIG_VECTOR || tsig.GetType() == CSIG_AUDIO);
	}
	else if (psig->strut["type"] == string("text"))
	{
		if (!strcmp(propname, "fontsize"))
			return (tsig.GetType() == CSIG_SCALAR && tsig.bufBlockSize == 8);
		if (!strcmp(propname, "fontname"))
			return (tsig.GetType() == CSIG_STRING);
	}
	/* Do checking differently for different GO--fig, axes, axis, line, text 8/1/2018
	*/
	return false;
}

vector<AstNode *> copy_AstNode(const AstNode *psrc, AstNode *ptarget)
{ // copy everything except for alt and tail
	vector<AstNode *> out;
	ptarget->line = psrc->line;
	ptarget->type = psrc->type;
	ptarget->col = psrc->col;
	ptarget->dval = psrc->dval;
	ptarget->suppress = psrc->suppress;
	if (psrc->str) strcpy(ptarget->str, psrc->str);
	else ptarget->str = NULL;
	if (psrc->child)
	{
		AstNode *tp = new AstNode;
		copy_AstNode(psrc->child, tp);
		out.push_back(tp);
	}
	else
		ptarget->child = NULL;
	if (psrc->next)
	{
		AstNode *tp = new AstNode;
		copy_AstNode(psrc->next, tp);
		out.push_back(tp);
	}
	else
		ptarget->next = NULL;
	ptarget->tail = NULL;
	ptarget->alt= NULL;
	return out;
}

vector<CVar> CAstSig::Compute(void)
{ 
	// There are many reasons to use this function as a Gateway function in the application, avoiding calling Compute(pAst) directly.
	// Call Compute(pAst) only if you know exactly what's going on. 11/8/2017 bjk
	vector<CVar> res;
	Sig.cell.clear();
	Sig.strut.clear();
	Sig.struts.clear();
	Sig.outarg.clear();
	Sig.outarg2.clear();
	Sig.SetNextChan(NULL);
	Sig.functionEvalRes = false;
	pgo = NULL;
	lhs = NULL;
	try {
		if (!pAst) {
			res.push_back(Sig);
			return res;
		}
		fBreak = false;
		GfInterrupted = false;
//		Tick0 = GetTickCount0();
//		printf("begin: %d\n", Tick0);
		if (pAst->type == N_BLOCK && u.application &&  !strcmp(u.application,"xcom")) {
			AstNode *p = pAst->next;
			while (p)
			{
				res.push_back(Compute(p));
				p = p->next;
			}
		}
		else
		{

			res.push_back(Compute(pAst));
		}
		Tick1 = GetTickCount0();
//		printf("end:   %d\n", Tick1);
		return res;
	}
	catch (const CAstException &e) {
		if (!e.out.empty())
		{
			for (size_t k=0; k<e.out.size(); k++)
			{
				res.push_back(*e.out[(int)k]);
			}
			return res;
		}
		else
		{
			char errmsg[2048];
			strncpy(errmsg, e.getErrMsg().c_str(), sizeof(errmsg) / sizeof(*errmsg));
			errmsg[sizeof(errmsg) / sizeof(*errmsg) - 1] = '\0';
			throw errmsg;
		}
	}
}

CVar *CAstSig::MakeGOContainer(vector<INT_PTR> GOs)
{
	CVar *pout(NULL);
	//Create a container
	pout = new CVar(3);
	pout->UpdateBuffer((unsigned int)GOs.size());
	unsigned int k = 0;
	for (auto gos : GOs)
		pout->buf[k++] = (double)gos;
	return pout;
}

CVar *CAstSig::MakeGOContainer(vector<CVar *> GOs)
{
	CVar *pout(NULL);
	//Create a container
	pout = new CVar(3);
	pout->UpdateBuffer((unsigned int)GOs.size());
	unsigned int k = 0;
	for (auto gos : GOs)
		pout->buf[k++] = (double)(INT_PTR)gos;
	return pout;
}

CVar *CAstSig::GetGOVariable(const char *varname, CVar *pvar)
{ // To retrieve a GO variable. 
  // For a single element, returns its pointer
  // For a array GO, create a container showing the pointers of the elements and return its pointer
	try {
		CVar *pout(NULL);
		vector<CVar *> GOs;
		if (pvar)
			GOs = pvar->struts.at(varname);
		else
			GOs = GOvars.at(varname);
		// If the retrieved GOs is a size of 1, return the front element pointer 
		// OK to return it even if the retrieved GOs is a GO container
		if (GOs.size() == 1)
			return GOs.front();
		//return the newly created container for multiple GOs
		return MakeGOContainer(GOs);
	}
	catch (out_of_range oor)
	{
		throw CAstException(pAst, this, "Internal error--GetGOVariable() should be called when varname is sure to exist in GOvars");
	}
}

CVar *CAstSig::GetVariable(const char *varname, CVar *pvar)
{ //To retrive a variable from a workspace pvar is NULL (default)
  //To retrive a member variable, specify pvar as the base variable 
 // For multiple GO's, calls GetGOVariable()
	CVar *pout(NULL);
	if (!varname)
		throw CAstException(pAst, this, "Internal error--varname not specified in GetVariable()");
	if (pvar)
	{
		if (pvar->strut.find(varname) != pvar->strut.end())
			pout = &pvar->strut.at(varname);
		else if (pvar->struts.find(varname) == pvar->struts.end())
			return NULL;
	}
	else
	{
		if (Vars.find(varname) != Vars.end())
			pout = &Vars.at(varname);
		else if (GOvars.find(varname) == GOvars.end())
			return NULL;
	}
	if (pout) return pout;
	return GetGOVariable(varname, pvar);
}

CVar &CAstSig::TID_condition(const AstNode *pnode, AstNode *pLHS, AstNode *pRHS, CVar *psig, CVar *psigBase)
{
	unsigned int id = 0;
	CVar isig;
	replica = Sig;
	eval_indexing(pLHS, psigBase, isig);
	CVar rhs = Compute(pRHS);
	if (rhs.IsScalar())
	{
		for (unsigned k = 0; k < isig.nSamples; k++)
			psigBase->buf[(int)isig.buf[k]-1] = rhs.value();
	}
	else if (searchtree(pRHS, T_REPLICA))
	{ 
		if (psigBase->IsTimeSignal()) // Check this part 9/4/2018
		{
			// consolidate chained rhs with lhs
			for (CTimeSeries *cts = &rhs; cts; cts = cts->chain)
			{
				// id translated from tmark for each chain
				id = (unsigned int)(cts->tmark / 1000. * cts->GetFs());
				memcpy(psigBase->buf + id, cts->buf, cts->nSamples * sizeof(double));
			}
		}
		else
		{
			for (unsigned k = 0; k < isig.nSamples; k++)
					psigBase->buf[(int)isig.buf[k]-1] = rhs.buf[k];
		}
	}
	else if (rhs.IsEmpty())
	{
		insertreplace(pLHS, psigBase, rhs, isig);
		Sig.Reset();
	}
	else
		throw CAstException(pRHS, this, "Invalid RHS; For LHS with conditional indexing, RHS should be either a scalar, empty, or .. (self).");
	SetVar(pnode->str, psigBase);
	return Sig;
}

CVar &CAstSig::TID_RHS2LHS(const AstNode *pnode, AstNode *pLHS, AstNode *pRHS, CVar *psig, CVar *psigBase)
{ // Computes pRHS, if available, and assign it to LHS. If pRHS is NULL, evaluates the single side (pLHS) and puts in to Sig and the application may echo as needed.
	// 2 cases where psig is not Sig: first, a(2), second, a.sqrt
	CVar tsig;
	if (IsCondition(pLHS))
		return TID_condition(pnode, pLHS, pRHS, psig, psigBase);
	switch (pLHS->type)
	{
	case N_ARGS: 
		tsig = TID_indexing(pLHS, pRHS, psig, psigBase);
		if (pgo)
		{ // need to get the member name right before N_ARGS
			AstNode *pp = findParentNode((AstNode*)pnode, pLHS, true);
			setgo.type = pp->str;
		}
		break;
	case T_ID: // T-Y-2
	case N_STRUCT:
	case N_CALL:
		tsig = TID_tag(pnode, pLHS, pRHS, psig, psigBase);
		break;
	case N_TIME_EXTRACT: // T-Y-4 
		tsig = TID_time_extract(pnode, pLHS, pRHS, psigBase);
		break;
	default:  // T-Y-5
		return Compute(pRHS); // check... this may not happen.. if so, inspect.
	}
	return Sig;
}

AstNode *get_next_parsible_node(AstNode *pn)
{
	if (pn->alt && pn->alt->type == N_CELL) pn = pn->alt;
	return pn->alt;
}

void CAstSig::ClearVar(AstNode *pnode, CVar *psig)
{
	NODEDIGGER ndog;
	ndog.root = pnode;
	AstNode *pn = ndog.root;
	while (pn)
	{
		AstNode *p = read_node_4_clearvar(ndog, &pn);
		if (!p) 
		{
				ClearVar(pn->str, ndog.psigBase);
				return;
		}
		pn = p;
	}
}
AstNode *CAstSig::read_node_4_clearvar(NODEDIGGER &ndog, AstNode **pn)
{ // This returns the next node. pn gets updated as well.
  // pn->type must always be T_ID or N_STRUCT
	if ((*pn)->type!=T_ID && (*pn)->type!=N_STRUCT)
		throw CAstException(*pn, this, "Only a variable or member variable can be cleared.");
	if ((*pn)->str[0] == '#' || IsValidBuiltin((*pn)->str))
		throw CAstException(*pn, this, "Function cannot be cleared.");
	CVar *pres;
	if (!(pres = GetVariable((*pn)->str, ndog.psigBase)))
		throw CAstException(*pn, this, "Variable not found.");
	AstNode * res = get_next_parsible_node(*pn);
	if (res && res->type == N_STRUCT && !strcmp(res->str, "clear"))
	{
		return NULL;
	}
	ndog.psigBase = pres;
	return res;
}

AstNode *CAstSig::read_node(NODEDIGGER &ndog, AstNode *pn)
{
	if (pn->type == T_OP_CONCAT || pn->type == '+' || pn->type == '-' || pn->type == T_TRANSPOSE || pn->type == T_MATRIXMULT
		|| pn->type == '*' || pn->type == '/' || pn->type == T_OP_SHIFT || pn->type == T_NEGATIVE || (pn==ndog.root && IsCondition(pn)))
	{ //No further actions
		return get_next_parsible_node(pn);
	}
	string emsg;
	char estr[256];
	AstNode *p = pn;
	int cellind, ind(0);
	AstNode *pUDF;
	if ((pUDF=ReadUDF(emsg, pn->str)))
	{
		if (pn->child)
		{
			throw CAstException(pn, this, "Name conflict between the LHS variable and a user-defined function.");
		}
		AstNode *higher = get_parent_node(pAst, pn);
		if (higher)
		{
			// See aux-indexing-parsing-plan.doc for more info. The parent node is lhs only if...
//			if ((higher->child == pn || higher->alt == pn) && (higher->type==T_ID || higher->type == N_CELL || higher->type == N_VECTOR))
//				lhs = higher;
			// Otherwise, the parent node is not necessarily the LHS. 10/14/2018
		}
		// if static function, ndog.psigBasemust be NULL
		if (pUDF->suppress==3 && ndog.psigBase)
			throw CAstException(pUDF, this, "Function declared as static cannot be called as a member function.");
		if (PrepareAndCallUDF(pn, ndog.psigBase)) // this probably won't return false
		{// if a function call follows N_ARGS, skip it for next_parsible_node
			ndog.psigBase = &Sig;
			if (pn->alt && pn->alt->type == N_ARGS)
				pn = pn->alt; // to skip N_ARGS
		}
	}
	else
	{
		if (!emsg.empty())	throw CAstException(pn, this, emsg);
		if (builtin_func_call(ndog, pn))
		{ // if a function call follows N_ARGS, skip it for next_parsible_node
			if (pn->alt)
			{
				if (pn->alt->type == N_ARGS || pn->alt->type == N_HOOK)
					pn = pn->alt; // to skip N_ARGS
			}
		}
		else
		{
			if (pn->type == N_ARGS)
			{
				if (ndog.psigBase->IsGO() && ndog.psigBase->GetFs() != 3)
				{
					CVar tp = Compute(pn->child);
					if (tp.GetType()!=CSIG_SCALAR || tp.value()!=1.)
						throw CAstException(pn, this, "Invalid index of a graphic object arrary.");
				}
				else
					ExtractByIndex(pn, pn->child, &ndog.psigBase); //Sig updated. No change in psig
				if (ndog.psigBase->IsGO())
					pgo = ndog.psigBase;
			}
			else if (IsCondition(pn))
			{
				if (ndog.psigBase->GetType() == CSIG_CELL) throw CAstException(pn, this, "A cell a454rray cannot be accessed with ( ).");
				CVar isig, isig2;
				eval_indexing(pn, ndog.psigBase, isig);
				Sig = extract(estr, &ndog.psigBase, isig);
				if (estr[0]) throw CAstException(pn, this, estr);
				if (!pn->next) // 1D indexing, unGroup it
					Sig.nGroups = 1;
			}
			else if (pn->type == N_TIME_EXTRACT)
				TimeExtract(pn, pn->child, ndog.psigBase);
			else if (pn->type == T_REPLICA || pn->type == T_ENDPOINT)
				Sig = *ndog.psigBase;
			else
			{
				CVar *pres;
				//Need to scan the whole statement whether this is a pgo statement requiring an update of GO
				if (!(pres = GetVariable(pn->str, ndog.psigBase)) || pres->IsEmpty())
				{
					if (ndog.root->child && (pn->type == N_STRUCT || pn->type == T_ID))
					{
						if (!pn->alt) return NULL; // if pn is the last node, no exception and continue to check RHS
						if (pn->alt->type == N_STRUCT) return NULL; // (something_not_existing).var = RHS
					}
					throw CAstException(pn, this, "Variable or function not available.");
				}
				if (pres->IsGO())
				{ // the variable pn->str is a GO
					Sig = *(ndog.psigBase = pgo = pres);
					if (pn->child) setgo.type = pn->str;
				}
				/* I had thought this would be necessary, but it works without this... what's going on? 11/6/2018*/
				// If pgo is not NULL but pres is not a GO, that means a struct member of a GO, such as fig.pos, then check RHS (child of root. if it is NULL, reset pgo, so that the non-GO result is displayed)
				else if (pgo && !ndog.root->child && !setgo.frozen)
					pgo = NULL;
				if (pn->alt)
				{
					p = pn->alt;
					if (p->type == N_HOOK)
						pseudoVar(pn, p);
					else if (p->type == N_CELL)
						//either x{2} or x{2}(3). x or x(2) doesn't come here.
						// I.E., p->child should be non-NULL
					{
						cellind = (int)Compute(p->child).value(); // check the validity of ind...probably it will be longer than this.
						Sig = *(ndog.psigBase = &pres->cell.at(cellind - 1)); // check the validity
					}
					else if (p->type == N_STRUCT)
					{
						if (!pres->cell.empty() || (!pres->IsStruct() && !pres->IsEmpty()))
						{
							if (p->str[0] != '#' && !IsValidBuiltin(p->str))
								if (!ReadUDF(emsg, p->str))
									if (!emsg.empty())
										throw CAstException(NULL, this, emsg);
									else
										throw CAstException(pn, this, "Unknown variable, function or keyword identifier.", p->str);
						}
						Sig = *(ndog.psigBase = pres);
					}
					else
						Sig = *(ndog.psigBase = pres);
				}
				else
					Sig = *(ndog.psigBase = pres);
			}
		}
	}
	return get_next_parsible_node(pn);
}

AstNode *CAstSig::read_nodes(NODEDIGGER &ndog)
{
	AstNode *pn = ndog.root;
	AstNode *p;
	CVar pvar;
	while (pn)
	{
		p = read_node(ndog, pn);
		if (!p) return pn;
		pn = p;
	}
	return NULL; // shouldn't come thru here; only for the formality
}

CVar &CAstSig::TID(AstNode *pnode, AstNode *pRHS, CVar *psig)
{
	if (pnode)
	{
		setgo.clear();
		NODEDIGGER ndog;
		ndog.root = pnode;
		ndog.psigBase = psig; // NULL except for T_REPLICA

		char ebuf[256] = {};
		// a = sqrt(2) --> inside psigAtNode, without knowing whether this call is part of RHS handling or not, there's no way to know whether a string is something new to fill in later or just throw an exception.
		// by default the 3rd arg of psigAtNode is false, but if this call is made during RHS handling (where pRHS is NULL), it tells psigAtNode to try builtin_func_call first. 8/28/2018

		AstNode *pLast = read_nodes(ndog); // that's all about LHS.
		if (!ndog.psigBase)
		{
			ndog.side = 'L';
			return define_new_variable(pnode, pRHS);
		}
		// At this point, Sig should be it
		// psig : the base content of Sig 
		// pLast: the node corresponding to psig
		if (!pRHS)
		{
			if (ndog.psigBase->IsGO()) return *pgo;
			else	return Sig;
		}
		setgo.frozen = true;
		ndog.side = 'R';
		lhs = pLast;
		CVar res = TID_RHS2LHS(pnode, pLast, pRHS, &Sig, ndog.psigBase);
		// Do this again....  f.pos(2) = 200
		// Need to send the whole content of f.pos, not just f.pos(2), to SetGoProperties
		// 11/4/2018
		if (setgo.type)
			fpmsg.SetGoProperties(this, setgo.type, *ndog.psigBase);
		ndog.psigBase = NULL;
	}
	return Sig;
}

CVar &CAstSig::ConditionalOperation(const AstNode *pnode, AstNode *p)
{
//	if (pnode->type==T_ID) return Compute(pnode);
	CVar rsig;
	switch (pnode->type)
	{
	case '<':
	case '>':
	case T_LOGIC_LE:
	case T_LOGIC_GE:
	case T_LOGIC_EQ:
	case T_LOGIC_NE:
		rsig = Compute(p->next);
		blockCell(pnode, rsig);
		Compute(p);
		blockCell(pnode, Sig);
		Sig.LogOp(rsig, pnode->type);
		if (Sig.IsString())
		{
			Sig.SetFs(1); // body::MakeLogical (which is part of LogOp) doesn't turn a string into a logical
			Sig.nSamples--; // get rid of the null space.
		}
		break;
	case T_LOGIC_NOT:
		blockCell(pnode, Sig);
		rsig.Reset();
		rsig.MakeLogical();
		Sig = Compute(p);
		if (!Sig.IsLogical() || !rsig.IsLogical())
			throw CAstException(p, this, "Logical operation is only for logical arrays.");
		Sig.LogOp(rsig, pnode->type); // rsig is a dummy for func signature.
		break;
	case T_LOGIC_OR:
	case T_LOGIC_AND:
		rsig = ConditionalOperation(p, p->child);
		ConditionalOperation(p->next, p->next->child);
		Sig.LogOp(rsig, pnode->type);
		break;
	case N_VECTOR:
	case N_MATRIX:
		//Coming here: if [1 1].and statement; end
		return Compute(pnode);
		break;
	default:
		//Coming here: if and([1 1]) statement; end
		return TID((AstNode*)pnode, NULL);
//		Compute(pnode);
		break;
	}
//	return Sig;
	//Need this instead of return Sig to cover
	// if (a==[1 4 5]).and statement; end
	return TID(pnode->alt, NULL, &Sig);
}

CVar &CAstSig::Compute(const AstNode *pnode)
{
	CVar tsig, isig, lsig, rsig;
	bool trinary(false);
	if (!pnode) 
	{	Sig.Reset(1); return Sig; }
	AstNode *p = pnode->child;
try {
	if (GfInterrupted)
		throw CAstException(pnode, this, "Script execution has been interrupted.");
	switch (pnode->type) {
	case T_ID:
		return TID((AstNode*)pnode, p);
	case N_HOOK:
		return pseudoVar(pnode, p);
	case N_TSEQ:
		return TSeq(pnode, p);
	case N_IDLIST:
		tsig = Compute(p);
		if (p && pnode->alt && !pnode->tail && !pnode->str)
		{    // LHS is x(tp1~tp2)
			Compute(pnode->alt);
			Sig += &tsig;
		}
		return Sig;
	case T_NUMBER:
		Sig.Reset();
		Sig.SetValue(pnode->dval);
		return TID(pnode->alt, NULL, &Sig);
		break;
	case T_STRING:
		Sig.Reset();
		Sig.SetString(pnode->str);
		return TID(pnode->alt, NULL, &Sig);
		break;
	case N_MATRIX:
		Sig.Reset();
		if (p) 	NodeMatrix(pnode, p);
		else	Sig.Reset(1);
		return TID(pnode->alt, NULL, &Sig);
	case N_VECTOR:
		if (!p) Sig.Reset();
		else if (p->next)
		{
			Sig.Reset();
			if (p) 	NodeVector(pnode, p);
			else	Sig.Reset(1);
		}
		else
		{
			Compute(p);
			if (pnode->alt)
			{
				// Top-level function call. Vector is on the LHS: e.g., [a b]=max(a_matrix)
				// Get RHS. The results should be assigned to variables in the 
				// To-do: Check arg_list inside the vector and see if they are valid
				SetVar(pnode->alt->str, &Sig);
				break;
			}
		}
		return TID(pnode->alt, NULL, &Sig);
		break;
	case T_REPLICA:
		return TID((AstNode*)pnode, NULL, &replica); //Make sure replica has been prepared prior to this
	case T_ENDPOINT:
		tsig.SetValue(endpoint);
		return TID((AstNode*)pnode, NULL, &tsig); //Make sure endpoint has been prepared prior to this
	case '+':
	case '-':
		tsig = Compute(p);
		blockCell(pnode,  tsig);
		if (pnode->type=='+')	Compute(p->next);
		else					-Compute(p->next);
		blockCell(pnode,  Sig);
		Sig += tsig;
		return TID((AstNode*)pnode, NULL, &Sig);
	case '*':
	case '/':
	case T_MATRIXMULT: // "**"
		tsig = Compute(p);
		blockCell(pnode,  Sig);
		blockString(pnode,  Sig);
		if (pnode->type=='*')	Compute(p->next);
		else if (pnode->type == '/')	Compute(p->next).reciprocal();
		else
		{
			checkVector(pnode, tsig);
			Compute(p->next);
			checkVector(pnode, Sig);
			Sig = (CSignals)tsig.matrixmult(&Sig);
			return TID((AstNode*)pnode, NULL, &Sig);
		}
		blockString(pnode,  Sig);
		Sig *= tsig;
		return TID((AstNode*)pnode, NULL, &Sig);
	case '%':
		//only in the format of A %= B
		((AstNode*)pnode)->type = N_CALL;
		((AstNode*)pnode)->str = strdup("mod");
		((AstNode*)pnode)->alt = p->next;
		p->next = NULL;
		Sig = replica;
		HandleAuxFunctions(pnode); // Assuming that current body content (Sig) is already prepared...is it true? 8/23/2018
		break;
	case T_TRANSPOSE:
		Transpose(pnode, p);
		return TID((AstNode*)pnode, NULL, &Sig);
	case T_NEGATIVE:
		-Compute(p);
		blockString(pnode,  Sig);
		return TID((AstNode*)pnode, NULL, &Sig);
	case T_OP_SHIFT:
		tsig = Compute(p->next);
		blockCell(pnode,  Sig);
		if (!tsig.IsScalar())
			throw CAstException(p->next, this, "Second operand of '>>' must be a scalar.");
		Compute(p);
		checkAudioSig(pnode,  Sig);
		Sig >>= tsig.value();
		Sig.nGroups = tsig.nGroups;
		return TID((AstNode*)pnode, NULL, &Sig);
	case T_OP_CONCAT:
		tsig = Compute(p->next);
		if (pgo)
		{ //special treatment needed to multiple GO's
			vector<CVar*> tp;
			tp.push_back(pgo);
			Compute(p);
			if (Sig.IsEmpty()) return Sig = *pgo;
			//Now, Sig can be CSIG_HDLARRAY, then use it as is.
			if (!pgo)
				throw CAstException(p, this, "RHS is a graphic handle. LHS is not. Can't concatenate.");
			if (Sig.GetType() == CSIG_HDLARRAY)
			{
				Sig.UpdateBuffer(Sig.nSamples + 1);
				Sig.buf[Sig.nSamples-1] = (double)(INT_PTR)tp.front();
			}
			//else
			//{
			//	tp.insert(tp.begin(), pgo);
			//	pgo = MakeGOContainer(tp);
			//}
			//else
			//	*pgo += tp.front();
			return Sig;
		}
		Compute(p);
		if (pgo)
			throw CAstException(p, this, "LHS is a graphic handle. RHS is not. Can't concatenate.");
		if (Sig.nGroups > 1)
			if (tsig.nSamples > 0 && tsig.nGroups != Sig.nGroups)
				throw CAstException(p->next, this, "To concatenate, the second operand must be either empty, the two operands must have the same number of each group (i.e., same row count).");
		if ( (tsig.GetType()==CSIG_CELL) && (Sig.GetType()!=CSIG_CELL) )
			throw CAstException(p->next, this, "A cell variable cannot be appended to a non-cell variable.");
		if (Sig.GetType()==CSIG_CELL)
		{
			if (tsig.GetType() == CSIG_CELL)
			{
				for (size_t k=0; k<tsig.cell.size(); k++)
					Sig.cell.push_back(tsig.cell[(int)k]);
			}
			else
				Sig.cell.push_back(tsig);
		}
		else
		{
			//For matrix, Group-wise (i.e., row-wise) concatenation
			if (Sig.nGroups > 1)
			{
				unsigned int len0 = Sig.Len();
				Sig.UpdateBuffer(Sig.nSamples+tsig.nSamples);
				unsigned int len1 = Sig.Len();
				unsigned int lent = tsig.Len();
				for (unsigned int k, kk = 0; kk < Sig.nGroups; kk++)
				{
					k = Sig.nGroups - kk - 1;
					memcpy(Sig.buf + len1*k, Sig.buf+len0*k, sizeof(double)*len0);
					memcpy(Sig.buf + len1*k + len0, tsig.buf + lent*k, sizeof(double)*lent);
				}
			}
			else
			{
				Sig += &tsig;
				Sig.MergeChains();
			}
		}
		return TID((AstNode*)pnode, NULL, &Sig);
	case T_LOGIC_OR:
	case T_LOGIC_AND:
	case '<':
	case '>':
	case T_LOGIC_LE:
	case T_LOGIC_GE:
	case T_LOGIC_EQ:
	case T_LOGIC_NE:
	case T_LOGIC_NOT:
		ConditionalOperation(pnode, p);
		return TID((AstNode*)pnode, NULL, &Sig);
	case '@':
		SetLevel(pnode, p);
		return TID((AstNode*)pnode->alt, NULL, &Sig);
	case N_INITCELL:
		return InitCell(pnode, p);
	case N_BLOCK:
		for (p = pnode->next; p && !fExit && !fBreak; p = p->next)
		{
			pLast = p;
			hold_at_break_point(p);
			Compute(p);
		}
		break;
	case T_IF:
		if (!p) break;
		if (checkcond(p))
			Compute(p->next);
		else if (pnode->alt) 
			Compute(pnode->alt);
		break;
	case T_SWITCH:
		//tsig = Compute(p);
		//for (p=p->next; p && p->next; p=p->next->next)	// p is a case exp, pcase->next is the code block.
		//	if (p->type == N_ARGS) {
		//		for (AstNode *pa=p->child; pa; pa=pa->next)
		//			if (tsig == Compute(pa))
		//				return Compute(p->next);	// no further processing of this 'switch' statement.
		//	} else if (tsig == Compute(p))
		//		return Compute(p->next);	// no further processing of this 'switch' statement.
		//// now p is at the end of 'case' list, without executing any conditional code.
		//if (p)	// if not null, it's the 'otherwise' code block
		//	Compute(p);
		break;
	case T_WHILE:
		fExit=fBreak=false;
		while (checkcond(p) && !fExit && !fBreak)
			Compute(pnode->alt);
		fBreak = false;
		break;
	case T_FOR:
		fExit=fBreak=false;
		isig = Compute(p->child);
		for (unsigned int i=0; i<isig.nSamples && !fExit && !fBreak; i++) 
		{
			SetVar(p->str, &CVar(isig.buf[i]));
			Compute(pnode->alt);
		}
		fBreak = false;
		break;
	case T_BREAK:
		fBreak = true;
		break;
	case T_CONTINUE:
		fContinue = true;
		break;
	case T_SIGMA:
		try {
			//int n;
			//tsig = Compute(p->child);
			//blockCell(pnode,  Sig);
			//n = 0;
			//for (CAstSig SubAstSig(p->next, this); n<tsig.nSamples; n++) {
			//	SubAstSig.SetVar(p->str, tsig.buf[n]);
			//	if (n)
			//		Sig += SubAstSig.Compute();
			//	else
			//		Sig = SubAstSig.Compute();
			//}
		} catch (const char *errmsg) {
			throw CAstException(pnode, this, "Calling sigma( )\n\nIn sigma expression:\n"+string(errmsg));
		}
		break;
	case T_RETURN:
		fExit = true;
		break;
	case N_CIPULSE3:
	case N_CIPULSE4:
	case N_CIPULSE5:
	case N_CIPULSE6:
		if (u.CallbackCIPulse)
			u.CallbackCIPulse(pnode, this);
		else
			throw CAstException(pnode, this, "Internal error! CI Pulse without handler!");
		break;
	case T_FUNCTION:
		// should have already been removed by Compute(void)
		throw CAstException(pnode, this, "Internal error! Unexpected node - T_FUNCTION");
		break;
	default:
		throw CAstException(pnode, this, "Internal error! Unknown node type - ", pnode->type);
	}
} catch (const exception &e) {
	throw CAstException(pnode, this, string("Internal error! ") + e.what());
} 
return Sig;
}

CVar *CAstSig::HandleSig(CVar *ptarget, CVar *pGraffyobj)
{
	//pGraffyobj must be a graffy object. Make a better way to verify that
	if (pGraffyobj->struts.empty()) return NULL;
	auto endy = pGraffyobj->struts.end();
	auto endy0 = pGraffyobj->strut.end();
	if (pGraffyobj->strut.find("type") == endy0) return NULL;
	if (pGraffyobj->struts.find("parent") == endy && pGraffyobj->struts.find("children") == endy) return NULL;
	if (pGraffyobj->strut["type"].string() == "figure")
	{
		if (pGraffyobj->GetFs()==2)
			ptarget->SetString(pGraffyobj->string().c_str());
		else
			ptarget->SetValue(pGraffyobj->buf[0]);
	}
	else
	{
		ptarget->SetValue(pGraffyobj->buf[0]);
	}
	return ptarget;
}

CVar &CAstSig::InitCell(const AstNode *pnode, AstNode *p)
{
	try {
		CAstSig temp(this); // temp is used to protect Sig
		int count = 0;
		// x={"bjk",noise(300), 4.5555}
		for (; p; count++, p = p->next)
			;
		p = pnode->child;
		Sig.Reset(1); // check.........8/19
		Sig.cell.reserve(count);
		for (; p; p = p->next)
		{
			CVar tsig = temp.Compute(p);
			Sig.appendcell(tsig);
		}
		if (pnode->str)
			SetVar(pnode->str, &Sig);
		return Sig;
	}
	catch (const CAstException &e) {
		throw CAstException(pnode, this, e.getErrMsg());
	}
}

CVar &CAstSig::Transpose(const AstNode *pnode, AstNode *p)
{
	Compute(p);
	Sig.transpose();
	return Sig;
}

CAstSig &CAstSig::Reset(const int fs, const char* path)
{
	map<string,UDF>::iterator it;
	for (it=pEnv->udf.begin(); it!=pEnv->udf.end(); it++)
		yydeleteAstNode(it->second.pAst, 0);
	Script = "";
	pEnv->udf.clear();
	Vars.clear();
	if (path)	pEnv->AuxPath=path; // I don't know if this might cause a trouble.
	if (fs > 1)
		pEnv->Fs = fs;
	return *this;
}

CAstSig &CAstSig::SetVar(const char *name, CVar *psig, CVar *pBase)
{// NULL pBase --> name will be the variable in the workspace.
 //non-NULL pBase --> pBase is a class variable. name will be a member variable under pBase.
	if (!pBase) // top scope
	{
		map<string, CVar>::iterator it = Vars.find(name);
		map<string, vector<CVar*>>::iterator jt = GOvars.find(name);
		if (psig->IsGO()) // name and psig should be fed to GOvars
		{
			//gca, gcf shouldn't be "push_backed" but instead replaced.
			if (!strcmp(name, "gca") || !strcmp(name, "gcf"))
				GOvars[name].clear();
			if (jt != GOvars.end())  GOvars.erase(jt);
			if (psig->GetFs() == 3)
			{
				if (psig->nSamples == 1)
				{
					psig = (CVar*)(INT_PTR)psig->value();
					GOvars[name].push_back(psig);
				}
				else
					for (unsigned int k = 0; k < psig->nSamples; k++)
						GOvars[name].push_back((CVar*)(INT_PTR)psig->buf[k]);
			}
			else
				GOvars[name].push_back(psig);
			if (it != Vars.end()) Vars.erase(it);
		}
		else // name and psig should be fed to Var
		{
			Vars[name] = *psig;
			if (jt != GOvars.end())  GOvars.erase(jt);
		}
	}
	else
	{
		if (psig->IsGO()) // name and psig should be fed to struts
		{
			pBase->struts[name].push_back(psig);
			auto it = pBase->strut.find(name);
			if (it != pBase->strut.end()) pBase->strut.clear();
		}
		else // name and psig should be fed to strut
		{
			pBase->strut[name] = *psig;
			auto jt = pBase->struts.find(name);
			if (jt != pBase->struts.end())  pBase->struts[name].clear();
		}
	}
	return *this;
}

CAstSigEnv &CAstSigEnv::SetPath(const char *path)
{
	string strPath = path;
	size_t len = strlen(path);
	if (len && path[len-1] != '\\')	strPath += '\\';
	AuxPath = strPath;
	return *this;
}

CAstSigEnv &CAstSigEnv::AddPath(const char *path)
{
	vector<string> paths;
	size_t nItems = str2vect(paths, AuxPath.c_str(), ";");
	string path0(path);
	size_t len0 = path0.length();
	if ((len0>0) && (path0[len0 - 1] != '\\'))
		path0 += '\\';
	for (size_t k = 0; k < nItems; k++)
	{
		if (paths[(int)k] == path0) return *this;
	}
	size_t len = AuxPath.length();
	if ( (len>0) && (AuxPath[len-1] != ';') )
		AuxPath += ';';
	AuxPath += path;
	return *this;
}

FILE *CAstSig::OpenFileInPath(string fname, string ext, string &fullfilename)
{ // in in out
	string pathscanned;
	char drive[64], dir[MAX_PATH], filename[MAX_PATH], extension[MAX_PATH];
	_splitpath(fname.c_str(), drive, dir, filename, extension);
	transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	char type[4];
	if (ext == "txt") strcpy(type, "rt");
	else						strcpy(type, "rb");
	size_t pdot = fname.rfind('.');
	if ((pdot == fname.npos || pdot < fname.length() - 4) && !extension[0])
		fname += "." + ext;

	if (drive[0] + dir[0] > 0)
	{ // if path is included in fname, open with fname
		fullfilename = fname;
		return fopen(fname.c_str(), type);
	}
	else {
		string path, ofname(fname);
//		if (ext == "aux") // AuxPath is for all, not just for aux files.
			path = pEnv->AuxPath;
		for (size_t ps=0; size_t p=path.find(';', ps); ps=p+1) {
			int pe = (p==string::npos) ? (int)path.length() - 1 : (int)p - 1;
			for (; ps < path.length() && isspace(path[ps]); ps++)
				;
			for (; pe >= 0 && isspace(path[pe]); pe--)
				;
			if ((int)ps <= pe)
			{
				pathscanned += path.substr(ps, pe - ps + 1);
				fname = path.substr(ps, pe - ps + 1) + ofname;
			}
			if (FILE *file = fopen(fname.c_str(), type))
			{
				fullfilename = fname;
				return file;
			}
			if (p == string::npos)
				break;
		}
		fullfilename = pathscanned;
		return NULL;
	}
}

string CAstSig::MakeFilename(string fname, const string ext)
{
	trim(fname,' ');
	size_t pdot = fname.rfind('.');
	if (pdot == fname.npos || pdot < fname.length()-4)
		fname += "." + ext;
	if (fname[0] != '\\' && fname[1] != ':')
		if (ext == "aux")
			return pEnv->AuxPath + fname;
	return fname;
}

bool CAstSig::IsStatement(const AstNode *pnode)
{
	if (!pnode) return false;
	switch (pnode->type) {
	case T_ID:
	case N_VECTOR:
	case N_MATRIX:
		//Add other types you find necessary
		return pnode->child!=NULL;
	default:
		return false;
	}
}

bool CAstSig::IsCondition(const AstNode *pnode)
{
	if (!pnode) return false;
	switch (pnode->type) {
	case '<':
	case '>':
	case T_LOGIC_EQ:
	case T_LOGIC_NE:
	case T_LOGIC_LE:
	case T_LOGIC_GE:
	case T_LOGIC_NOT:
	case T_LOGIC_AND:
	case T_LOGIC_OR:
		return true;
	default:
		return false;
	}
}

bool CAstSig::IsLooping(const AstNode *pnode)
{
	if (pnode->type == T_IF || pnode->type == T_FOR || pnode->type == T_WHILE) return true;
	return false;
}

bool CAstSig::IsCELL_STRUCT_pnode_TID_ARGS(const AstNode *pnode, const AstNode *p)
{
	if (p->type == N_CELL || p->type == N_STRUCT) return true;
	if (pnode->type == T_ID && p->type == N_ARGS) return true;
	return false;
}

bool CAstSig::IsTID(const AstNode *p)
{ return p->type == T_ID; }
bool CAstSig::IsBLOCK(const AstNode *p) 
{ return p->type == N_BLOCK; }
bool CAstSig::IsVECTOR(const AstNode *p) 
{ return p->type == N_VECTOR; }
bool CAstSig::IsSTRUCT(const AstNode *p)
{
	return p->type == N_STRUCT;
}

bool CAstSig::IsPortion(const AstNode *p) 
{ return (p->type == N_ARGS || p->type == N_TIME_EXTRACT); }

bool CAstSig::Var_never_updated(const AstNode *p)
{//For these node types, 100% chance that var was not changed---do not send WM__VAR_CHANGED
	if (p->type == T_NUMBER) return true;
	if (p->type == T_STRING) return true;
	return false;
}

string CAstSig::ComputeString(const AstNode *p)
{
	return (p->type==T_STRING) ? p->str : Compute(p).string();
}

void godeep(CVar *var, CVar *pref)
{
	if (pref->struts.find("children") == pref->struts.end()) return;
	for (vector<CVar *>::iterator it = pref->struts["children"].begin(); it != pref->struts["children"].end(); it++)
	{
		if ((*it)->value() == var->value())
			throw pref;
		for (vector<CVar *>::iterator kt = (*it)->struts["children"].begin(); kt != (*it)->struts["children"].end(); kt++)
			godeep(var, *kt);
	}
}

class ercarrier
{
	int layer;
	CVar *pp;
	ercarrier() { layer = 0;  pp = NULL; };
	virtual ~ercarrier() {};
};

void godeep2(CVar *var, CVar *pref)
{
	for (vector<CVar *>::iterator it = var->struts["children"].begin(); it != var->struts["children"].end(); it++)
	{
		if ((*it)->value() == pref->value())
			throw pref;
		for (vector<CVar *>::iterator kt = (*it)->struts["children"].begin(); kt != (*it)->struts["children"].end(); kt++)
			godeep2(*kt, pref);
	}
}

int CAstSig::updateGO(CVar &goref)
{
	CVar *pp;
	map<std::string, CVar>::iterator it = Vars.begin();
	
	try {
		for (it; it != Vars.end(); it++)
		{
			pp = &goref;
			if (it->second.IsGO())
			{
				if (it->second.value() == pp->value())
					continue; // throw pp;
				// if the variable is in the genealogy of goref, update the variable
				godeep(&it->second, pp);
				if (goref.struts.find("parent") == goref.struts.end()) 
					continue;
				pp = goref.struts["parent"].front();
				while (pp && !pp->IsEmpty())
				{
					if (it->second.value() == pp->value())
						throw pp;
					if (pp->struts.find("parent") == pp->struts.end())
						break;
					pp = pp->struts["parent"].front();
				}
				// if goref is in the genealogy of the variable, update the variable
				godeep2(&it->second, pp);
				if (it->second.struts.find("parent") == it->second.struts.end())
					continue;
				pp = it->second.struts["parent"].front();
				while (pp && !pp->IsEmpty())
				{
					if (goref.value() == pp->value())
						throw pp;
					if (pp->struts.find("parent") == pp->struts.end())
						break;
					pp = pp->struts["parent"].front();
				}
			}
		}
		return 0;
	}
	catch (CVar *pp)
	{
		Vars[it->first] = *pp;
		return 1;
	}
}

CAstSigEnv::CAstSigEnv(const int fs)
: Fs(fs), curLine(-1)
{
	shutdown = false;
	if (fs < 0)	throw "Internal error: Fs must be greater than 1.";
}


void clean_AstNode(AstNode *p)
{
	if (p)
	{
		if (p->str)		delete p->str;
		if (p->child) clean_AstNode(p->child);
		if (p->next) clean_AstNode(p->next);
		if (p->alt) clean_AstNode(p->alt);
//		if (p->tail) clean_AstNode(p->tail);
		p->str = NULL;
	}
}

CAstSigEnv::~CAstSigEnv()
{
	//To Do: udf nodes (T_FUNCTION's) should be cleaned only during the shutdown of application.

	//It's better to clean it here rather using a status variable (shutdown) rather than doing it in the middle (at the exist of CallUDF() or during handling of CException)
	//The way it works now, if there's no change in file content of a udf (whether that's local udf or not)
	//it re-uses udf nodes stored in pEnv->udf from the previous udf call
	//The nodes shouldn't be cleaned until the end of application.
	//10/14/2018

	if (shutdown)
		for (map<string, UDF>::iterator it = udf.begin(); it != udf.end(); it++)
		{
			clean_AstNode(it->second.pAst);
		}
}

int CAstSig::ClearVar(const char *var, CVar *psigBase)
{
	if (!psigBase)
	{
		if (var[0] == 0) // clear all
		{
			Vars.erase(Vars.begin(), Vars.end());
			GOvars.erase(GOvars.begin(), GOvars.end());
			return 1;
		}
		else
		{
			vector<string> vars;
			size_t res = str2vect(vars, var, " ");
			for (size_t k = 0; k < res; k++)
			{
				map<string, CVar>::iterator it1 = Vars.find(vars[(int)k]);
				if (it1 != Vars.end())
					Vars.erase(it1);
				else
				{
					map<string, vector<CVar *>>::iterator it2 = GOvars.find(vars[(int)k]);
					if (it2 != GOvars.end())
						GOvars.erase(it2);
				}
			}
			return 0;
		}
	}
	else
	{
		if (var[0] == 0) // clear all
		{
			psigBase->strut.erase(psigBase->strut.begin(), psigBase->strut.end());
			psigBase->struts.erase(psigBase->struts.begin(), psigBase->struts.end());
			return 1;
		}
		else
		{ // let's not think about the case of multiple member variables
			map<string, CVar>::iterator it = psigBase->strut.find(var);
			map<string, vector<CVar *>>::iterator jt = psigBase->struts.find(var);
			if (it!= psigBase->strut.end()) psigBase->strut.erase(it);
			if (jt != psigBase->struts.end()) psigBase->struts.erase(jt);
			return 0;
		}
	}
}

void CAstSig::EnumVar(vector<string> &var)
{
	var.clear();
	for (map<string, CVar>::iterator what=Vars.begin(); what!=Vars.end(); what++)
		var.push_back(what->first);
}

int CAstSig::checkNumArgs(const AstNode *pnode, const AstNode *p, string &FuncSigs, int *args)
{
	return checkNumArgs(pnode, p, FuncSigs, args[0], args[1]);
}
int CAstSig::checkNumArgs(const AstNode *pnode, const AstNode *p, string &FuncSigs, int minArgs, int maxArgs)
{
	ostringstream msg;
	if (minArgs>0 && !p) msg << "must have at least 1 argument.";
	else
	{
		int nArgs(0);
		if (!p || p->type == N_STRUCT) return nArgs;
		for (const AstNode *cp = p; cp; cp = cp->next)
			++nArgs;
		msg << "must have ";
		if (minArgs == 0 && maxArgs == 0 && nArgs > 0)
			msg << "0 argument.";
		else if (nArgs < minArgs && maxArgs == 0)
			msg << "at least " << minArgs << (minArgs > 1 ? " arguments." : " argument.");
		else if (nArgs < minArgs || maxArgs > 0 && nArgs > maxArgs) {
			msg << minArgs;
			for (int i = minArgs + 1; i < maxArgs; ++i)
				msg << ", " << i;
			if (minArgs != maxArgs)
				msg << " or " << maxArgs;
			msg << (maxArgs > 1 ? " arguments." : " argument.");
		}
		else
			return nArgs;
	}
	throw CAstException(pnode, this, FuncSigs, msg.str());
}

CVar *CAstSig::GetSig(const char *var)
{ // For GO, this returns front()
	map<string, CVar>::iterator what = Vars.find(var);
	if (what == Vars.end())
	{
		map<string, vector<CVar *>>::iterator kt = GOvars.find(var);
		if (kt == GOvars.end())
			return NULL;
		else
			return kt->second.front();
	}
	else
		return &(what->second);
}

UDF& UDF::operator=(const UDF& rhs)
{
	if (this != &rhs)
	{
		pAst = rhs.pAst;
		fullname = rhs.fullname;
		content = rhs.content;
		DebugBreaks = rhs.DebugBreaks;
		newrecruit = rhs.newrecruit;
	}
	return *this;
}


CAstSigEnv& CAstSigEnv::operator=(const CAstSigEnv& rhs)
{
	if (this != &rhs) 
	{
		Fs = rhs.Fs;
		AuxPath = rhs.AuxPath;
		udf = rhs.udf;
		shutdown = rhs.shutdown;
		inFunc = rhs.inFunc;
	}
	return *this;
}



void goaction::clear()
{
	frozen = NULL; psig = NULL;  type = NULL; 	RHS.Reset();
}