// AUXLAB 
//
// Copyright (c) 2009-2019 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.5
// Date: 3/30/2019
// 
#include <sstream>
#include <list>
#include <algorithm>
#include <exception>
#include <math.h>
#include <time.h>
#include "sigproc.h"
#include "bjcommon.h"

#include <algorithm> // for lowercase

#ifndef CISIGPROC
#include "psycon.tab.h"
#else
#include "cipsycon.tab.h"
#endif

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
		throw ExceptionMsg(pAst, "toc called without tic");
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
		throw ExceptionMsg(pCalling, "Internal error! CheckPrepareCallUDF()");
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
			throw ExceptionMsg(pCalling, "Internal error! CheckPrepareCallUDF()", "supposed to be a local udf, but AstNode with that name not prepared");
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
		{
			ostringstream out("Internal error!UDF output should be alt and N_VECTOR");
			out << pOutParam->type;
			throw ExceptionMsg(pOutParam, out.str().c_str());
		}
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
			throw ExceptionMsg(u.pUDF, oss.str().c_str());
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
					Sig = son->Vars[arg]; //why is this needed? -- so update go Sig with non-go Sig 2/9/2019
				pgo = NULL;
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
		{//why is this necessary? 10/19/2018----yes this is...2/16/2019
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

bool CAstSig::checkcond(const AstNode *p)
{
	ConditionalOperation(p, p->child);
	if (!Sig.IsScalar())	throw ExceptionMsg(p, "--conditional op requires a scalar.");
	if (Sig.IsLogical()) 
		return Sig.logbuf[0];
	else				
		return Sig.value()!=0.;
}

void CAstSig::checkindexrange(const AstNode *pnode, CTimeSeries *inout, unsigned int id, string errstr)
{
	if (id>inout->nSamples) 
	{
		ostringstream out;
		out << errstr << " index " << id << " out of range.";
		throw ExceptionMsg(pnode, out.str().c_str());
	}
}

/* These two replace() member functions can be part of body in csignals.cpp for the point of logic,
(maybe even simpler if it stayed in csignals.cpp)
but moved here as a part of CAstSig because of exception handling (i.e., need pnode)
*/
CTimeSeries &CAstSig::replace(const AstNode *pnode, CTimeSeries *pobj, body &sec, body &index)
{
	//	this is to be used when items are replaced without changing the size.
	// except when sec is empty... in which case the "index"'ed items are deleted.
	// in that case index is assumed to be sorted ascending
	if (index.nSamples == 0) return *pobj;
	if (sec.bufBlockSize != pobj->bufBlockSize)
	{
		if (pobj->bufBlockSize == 1 && sec.bufBlockSize != 1) sec.MakeLogical();
		else if (pobj->bufBlockSize == 8 && sec.bufBlockSize == 1) sec.SetReal();
		else if (pobj->bufBlockSize == 8 && sec.bufBlockSize == 16) pobj->SetComplex();
		else if (pobj->bufBlockSize == 16 && sec.bufBlockSize == 1) sec.SetComplex();
		else if (pobj->bufBlockSize == 16 && sec.bufBlockSize == 8) sec.SetComplex();
	}
	for (unsigned int k = 0; k < index.nSamples; k++)
	{
		if (index.buf[k] < 1.)
			throw ExceptionMsg(pnode, "index must be greater than 0");
		unsigned int id = (int)index.buf[k];
		if (id - index.buf[k] > .25 || index.buf[k] - id > .25)
			throw ExceptionMsg(pnode, "index must be integer");
		if (id > pobj->nSamples)
			throw ExceptionMsg(pnode, "replace index exceeds the range.");
	}
	if (sec.nSamples == 0)
	{
		int trace = (int)(index.buf[0] - 1);
		for (unsigned int k = 0; k < index.nSamples; k++)
		{
			unsigned int diff = (unsigned int)(k < index.nSamples - 1 ? index.buf[k + 1] - index.buf[k] : pobj->nSamples + 1 - index.buf[k]);
			diff--;
			memcpy(pobj->logbuf + trace * pobj->bufBlockSize, pobj->logbuf + (int)index.buf[k] * pobj->bufBlockSize, diff*pobj->bufBlockSize);
			trace += diff;
		}
		pobj->nSamples -= index.nSamples;
	}
	else
		for (unsigned int k = 0; k < index.nSamples; k++)
		{
			if (index.buf[k] < 1.)
				throw ExceptionMsg(pnode, "index must be greater than 0");
			unsigned int id = (unsigned int)(index.buf[k]);
			if (id - index.buf[k] > .05 || index.buf[k] - id > .05)
				throw ExceptionMsg(pnode, "index must be integer");
			if (id > pobj->nSamples) 
				throw ExceptionMsg(pnode, "replace index exceeds the range.");
			id--; // zero-based
			if (sec.nSamples == 1) // items from id1 to id2 are to be replaced with sec.value()
				memcpy(pobj->logbuf + id * pobj->bufBlockSize, sec.logbuf, pobj->bufBlockSize);
			else
				memcpy(pobj->logbuf + id * pobj->bufBlockSize, sec.logbuf + k * pobj->bufBlockSize, pobj->bufBlockSize);
		}
	return *pobj;
}

CTimeSeries &CAstSig::replace(const AstNode *pnode, CTimeSeries *pobj, body &sec, int id1, int id2)
{ // this replaces the data body between id1 and id2 (including edges) with sec
	if (id1 < 0 || id2 < 0) throw ExceptionMsg(pnode, "replace index cannot be negative.");
	if (sec.bufBlockSize != pobj->bufBlockSize)
	{
		if (pobj->bufBlockSize == 1 && sec.bufBlockSize != 1) sec.MakeLogical();
		else if (pobj->bufBlockSize == 8 && sec.bufBlockSize == 1) sec.SetReal();
		else if (pobj->bufBlockSize == 8 && sec.bufBlockSize == 16) pobj->SetComplex();
		else if (pobj->bufBlockSize == 16 && sec.bufBlockSize == 1) sec.SetComplex();
		else if (pobj->bufBlockSize == 16 && sec.bufBlockSize == 8) sec.SetComplex();
	}
	//id1 and id2 are zero-based here.
	if (id1 > (int)pobj->nSamples - 1) throw ExceptionMsg(pnode, "replace index1 exceeds the range.");
	if (id2 > (int)pobj->nSamples - 1) throw ExceptionMsg(pnode, "replace index2 exceeds the range.");
	unsigned int secnsamples = sec.nSamples;
	bool ch = ((CSignal *)&sec)->bufBlockSize == 1;
	if (ch) secnsamples--;
	if (secnsamples == 1) // no change in length--items from id1 to id2 are to be replaced with sec.value()
	{
		if (ch)
			for (int k = id1; k <= id2; k++) pobj->strbuf[k] = sec.strbuf[0];
		else if (((CSignal *)&sec)->bufBlockSize == 16)
			for (int k = id1; k <= id2; k++) pobj->cbuf[k] = sec.cvalue();
		else
			for (int k = id1; k <= id2; k++) pobj->buf[k] = sec.value();
	}
	else
	{
		unsigned int nAdd = secnsamples;
		unsigned int nSubtr = id2 - id1 + 1;
		unsigned int newLen = pobj->nSamples + nAdd - nSubtr;
		unsigned int nToMove = pobj->nSamples - id2 - 1;
		if (nAdd > nSubtr) pobj->UpdateBuffer(newLen);
		bool *temp = new bool[nToMove*pobj->bufBlockSize];
		memcpy(temp, pobj->logbuf + (id2 + 1)*pobj->bufBlockSize, nToMove*pobj->bufBlockSize);
		memcpy(pobj->logbuf + id1 * pobj->bufBlockSize, sec.buf, secnsamples*pobj->bufBlockSize);
		memcpy(pobj->logbuf + (id1 + secnsamples)*pobj->bufBlockSize, temp, nToMove*pobj->bufBlockSize);
		delete[] temp;
		pobj->nSamples = newLen;
	}
	return *pobj;
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
			if (indsig.logbuf[k]) 
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
				if (sec.IsComplex())
				{
					inout->SetComplex();
					inout->cbuf[id - 1] = sec.cvalue();
				}
				else
					inout->buf[id - 1] = sec.value();
			}
		}
		else
		{
			unsigned int id = (unsigned int)round(indsig.value());
			checkindexrange(pnode, inout, id, "LHS");
			if (sec.IsComplex())
			{
				inout->SetComplex();
				inout->cbuf[id - 1] = sec.cvalue();
			}
			else
				replace(pnode, inout, sec, id - 1, id - 1);
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
						replace(pnode, inout, sec, indsig); // if sec is a scalar, just assignment call by index 
					else
					{
						checkindexrange(pnode, inout, id1, "LHS");
						checkindexrange(pnode, inout, id2, "LHS");
						if (sec.nSamples != id2 - id1 + 1)
						{
							if (!sec.IsScalar())
								throw ExceptionMsg(pnode, "to manipulate an audio signal by indices, LHS and RHS must be the same length or RHS is a scalar");
						}
						if (sec.IsComplex() && !inout->IsComplex()) inout->SetComplex();
						replace(pnode, inout, sec, id1 - 1, id2 - 1);
					}
				}
			}
			else if (!pnode->next && !pnode->str) // if s(conditional) is on the LHS, the RHS must be either a scalar, or the replica, i.e., s(conditional)
			{
				if (!tp && !indsig.IsLogical()) throw ExceptionMsg(pnode, "Internal logic error (insertreplace:0)--s(conditional?).");
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
				throw ExceptionMsg(pnode, "Internal logic error (insertreplace:1) --unexpected node type.");
		}
		else
		{
			// v(1:5) or v([contiguous]) = (any array) to replace
			// v(1:2:5) or v([non-contiguous]) = RHS; //LHS and RHS must match length.
			bool contig = isContiguous(indsig, id1, id2);
			if (!sec.IsEmpty() && !contig && sec.nSamples!=1 && sec.nSamples!=indsig.nSamples) throw ExceptionMsg(pnode, "the number of replaced items must be the same as that of replacing items.");
			if (contig)
				replace(pnode, inout, sec, id1-1, id2-1);
			else 
				replace(pnode, inout, sec, indsig);
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
		transform(Script.begin(), Script.end(), Script.begin(), ::tolower);
		pEnv->udf[Script].newrecruit = true;
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
	string namefrompnode = pnode4Func->str;
	transform(namefrompnode.begin(), namefrompnode.end(), namefrompnode.begin(), ::tolower);
	if (namefrompnode != udf_filename) // pnode4Func is NULL for local function call and it will crash.....8/1/
		throw ExceptionMsg(p, "inconsistent function name", string(string(string(udf_filename) + " vs ") + pnode4Func->str).c_str());

	pEnv->udf[udf_filename].pAst = pnode4Func;	
	pEnv->udf[udf_filename].fullname = fullfilename;
	pEnv->udf[udf_filename].content = filecontent.c_str();
	for (AstNode *pp = pnode4Func->next; pp; pp = pp->next)
	{// if one or more local functions exists
		UDF loc;
		loc.pAst = pp;
		namefrompnode = pp->str;
		transform(namefrompnode.begin(), namefrompnode.end(), namefrompnode.begin(), ::tolower);
		loc.fullname = fullfilename;
		loc.content = "see base function for content";
		pEnv->udf[udf_filename].local[namefrompnode] = loc;
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
		throw ExceptionMsg(pnode,"The last operand should be scalar or 2-element vector.");
	if (p->type == '@') 
	{ // trinary
		CSignals second = Compute(p->child->next);
		if (second.GetType()==CSIG_AUDIO)
		{	
			trinary = true;
			double srms = second.CSignal::RMS();
			if (srms == -std::numeric_limits<double>::infinity())
				throw ExceptionMsg(pnode,"x @ ref @ value ---- Invalid: RMS of ref is infinity.");
			if (isnan(srms))
				throw ExceptionMsg(pnode,"x @ ref @ value ---- Invalid: Left chan of ref is NULL.");
			refRMS.SetValue(srms);
			if (second.next && second.next->nSamples)
			{
				isig.SetValue(second.next->RMS());
				refRMS.SetNextChan(&isig);
			}
			Compute(p->child);	// Sig has the first operand
			if (!Sig.IsStereo() && second.IsStereo())
				throw ExceptionMsg(pnode,"x @ ref @ value ---- if x is mono, ref must be mono.");
		}
		else
			throw ExceptionMsg(pnode,"A @ B @ C ---- B must be an audio signal.");
	}
	else
	{ // binary
		refRMS.SetValue(-0.000262);	// the calculated rms value of a full scale sinusoid (necessary to avoid the clipping of rms adjusted full scale sinusoid)
		Compute(p); // Sig has the first operand
	}
	checkAudioSig(pnode,  Sig);
	if (!Sig.IsStereo() && !tsig.IsScalar()) throw ExceptionMsg(p->next, "Mono signal should be scaled with a scalar.");
	double mrms = Sig.CSignal::RMS();
	if (mrms == -std::numeric_limits<double>::infinity())
		throw ExceptionMsg(pnode,"Invalid: The signal is all-zero (-infinity RMS).");
	rms.SetValue(-mrms);
	if (Sig.IsStereo() && Sig.next->nSamples)
	{
		if (tsig.nSamples>2)
			throw ExceptionMsg(pnode,"For a stereo signal, the last operand should be a vector of two or less elements.");
		double msrms = Sig.next->CSignal::RMS();
		if (msrms == -std::numeric_limits<double>::infinity())
			throw ExceptionMsg(pnode,"Invalid: The signal (2nd Chan) is all-zero (-infinity RMS).");
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


CVar &CAstSig::TSeq(const AstNode *pnode, AstNode *p)
{ 
	//For now (6/12/2018) only [vector1][vector2] where two vectors have the same length.
	CVar tsig2, tsig = Compute(p);
	int type1 = tsig.GetType();
	if (type1 != CSIG_SCALAR && type1 != CSIG_VECTOR)
		strcpy(pnode->str, "TSEQ"),	throw ExceptionMsg(pnode,"Invalid tmark array");
	if (pnode->child->next) {
		//[tvalues][val] pnode->child->next is N_VECTOR for val
		//[tvalues][val;] pnode->child->next is N_MATRIX for val
		tsig2 = Compute(pnode->child->next);
		checkVector(pnode, tsig2);
		int type2 = tsig2.GetType();
		if (type2 != CSIG_SCALAR && type2 != CSIG_VECTOR)
			strcpy(pnode->str, "TSEQ"), throw ExceptionMsg(pnode,"Invalid t-sequence value array");
		if (tsig2.nGroups == 1)
		{
			//if 
			if (pnode->child->next->type == N_VECTOR)
			{
				if (tsig2.nSamples != tsig.nSamples)
					strcpy(pnode->str, "TSEQ"), throw ExceptionMsg(pnode,"Time point and value arrays must have the same length.");
			}
			else //pnode->child->next->type == N_MATRIX
			{
				//ok
			}
		}
		else
		{
			if (tsig2.nGroups != tsig.nSamples)
				strcpy(pnode->str, "TSEQ"), throw ExceptionMsg(pnode,"The number of time points and time sequences must have the same.");
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

CVar &CAstSig::pseudoVar(const AstNode *pnode, AstNode *p, CSignals *pout)
{
	int res;
	string dummy;
	if (!pout) pout = &Sig;
	if (pnode->type == N_HOOK)
		HandlePseudoVar(pnode);
	else
	{
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
				if (p->next->next)	throw ExceptionMsg(pnode,"Currently two channels or less for audio signals or t-series are allowed.");
				tsig.SetNextChan(&esig);
			}
			else if (audio > 0)
			{
				if (p->next->next)	throw ExceptionMsg(pnode,"Currently two channels or less for audio signals or t-series are allowed.");
				if (esig.GetType() != tsig.GetType())
					throw ExceptionMsg(pnode,"Signal type, Audio (or t-series), must be the same in both channels.");
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
			if (!esig.IsEmpty() && tsig.Len() != esig.Len()) throw ExceptionMsg(pnode,"All groups must have the same length for 2-D data. (All rows must be the same length in the matrix)");
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
	int audiofs = 0;
	for (p = p->next; p; p = p->next)
	{
		tsig = Compute(p);
		if (thisisGO && (tsig.GetType() != CSIG_HDLARRAY && !tsig.IsGO()))
			throw ExceptionMsg(pnode,"Graphic Handle and non-Graphic handle cannot be in an array");
		if ((!thisisGO && !beginswithempty) && (tsig.GetType() == CSIG_HDLARRAY || tsig.IsGO()))
			throw ExceptionMsg(pnode,"Graphic Handle and non-Graphic handle cannot be in an array");
		if (ngroups != tsig.nGroups)
			throw ExceptionMsg(pnode,"Every item in the brackets must have the same nGroups.");
		totalLen += Sig.Len();
		if (!compl) compl = tsig.IsComplex();
		thisisGO = tsig.GetType() == CSIG_HDLARRAY || tsig.IsGO();
		if (!audiofs && tsig.GetFs()>3) 
			audiofs = tsig.GetFs();
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
	if (audiofs) out.SetFs(audiofs);
	if (compl) out.SetComplex();
	int col; // column locator
	for (col = 0, p=pnode->child; p; p=p->next)
	{
		Compute(p);
		out.tmark = Sig.tmark; // is this right? 3/8/2019
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
		throw ExceptionMsg(pnode, "Time marker1 should be scalar");
	CSignals tsig2 = Compute(p->next);
	if (!tsig2.IsScalar())
		throw ExceptionMsg(pnode, "Time marker2 should be scalar");
	tsig1 += &tsig2;
	return Sig = tsig1;
}

#define MARKERCHARS "os.x+*d^v<>ph"

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
		{
			if (tsig.GetType() != CSIG_STRING || tsig.nSamples != 2)
				return false;
			char ch = (char)tsig.logbuf[0];
			return strchr(MARKERCHARS, ch) != NULL;
		}
		if (!strcmp(propname, "markersize") || !strcmp(propname, "width"))
			return (tsig.GetType() == CSIG_SCALAR);
		if (!strcmp(propname, "xdata") || !strcmp(propname, "ydata"))
			return (tsig.GetType() == CSIG_VECTOR || tsig.GetType() == CSIG_AUDIO);
		if (!strcmp(propname, "linestyle"))
			return true; // check at SetGOProperties() in graffy.cpp
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
		return res;
	}
	catch (const CAstException &e) {
		if (!e.out.empty())
		{
			for (size_t k=0; k<e.out.size(); k++)
				res.push_back(*e.out[(int)k]);
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

CVar *CAstSig::GetGlobalVariable(const AstNode *pnode, const char *varname, CVar *pvar)
{
	if (!varname)
		throw ExceptionMsg(pAst, "Internal error--varname not specified in GetGlobalVariable()");
	if (pvar)
	{
	}
	else
	{
		string dummy;
		if (pEnv->pseudo_vars.find(varname) != pEnv->pseudo_vars.end())
		{
			pEnv->inFunc[varname](this, pnode, NULL, dummy);
		}
		else 
		{
			map<string, vector<CVar*>>::iterator jt = pEnv->glovar.find(varname);
			if (jt == pEnv->glovar.end()) 
				return NULL;
			if ((*jt).second.front()->IsGO())
			{
				return GetGloGOVariable(varname, pvar);
			}
			else
			{
				Sig = *(*jt).second.front();
			}
		}
	}
	return &Sig;
}

CVar *CAstSig::GetGloGOVariable(const char *varname, CVar *pvar)
{ // To retrieve a GO variable. 
  // For a single element, returns its pointer
  // For a array GO, create a container showing the pointers of the elements and return its pointer
	try {
		CVar *pout(NULL);
		vector<CVar *> GOs;
		if (pvar)
			GOs = pvar->struts.at(varname); // invalid, this won't work. 1/30/2019
		else
			GOs = pEnv->glovar.at(varname);
		// If the retrieved GOs is a size of 1, return the front element pointer 
		// OK to return it even if the retrieved GOs is a GO container
		if (GOs.size() == 1)
			return GOs.front();
		//return the newly created container for multiple GOs
		return MakeGOContainer(GOs);
	}
	catch (out_of_range oor)
	{
		throw ExceptionMsg(pAst, "Internal error--GetGloGOVariable() should be called when global variable is sure to exist in pEnv->glovar");
	}
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
		throw ExceptionMsg(pAst, "Internal error--GetGOVariable() should be called when varname is sure to exist in GOvars");
	}
}

CVar *CAstSig::GetVariable(const char *varname, string &fullvarname, CVar *pvar)
{ //To retrive a variable from a workspace pvar is NULL (default)
  //To retrive a member variable, specify pvar as the base variable 
 // For multiple GO's, calls GetGOVariable()
	CVar *pout(NULL);
	if (!varname)
		throw ExceptionMsg(pAst, "Internal error--varname not specified in GetVariable()");
	if (pvar)
	{
		if (pvar->strut.find(varname) != pvar->strut.end())
			pout = &pvar->strut.at(varname);
		else if (pvar->struts.find(varname) == pvar->struts.end())
			return NULL;
		fullvarname += '.';
		fullvarname += varname;
	}
	else
	{
		if (Vars.find(varname) != Vars.end())
			pout = &Vars.at(varname);
		else if (GOvars.find(varname) == GOvars.end())
			return NULL;
		fullvarname = varname;
	}
	if (pout) return pout;
	return GetGOVariable(varname, pvar);
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
		throw ExceptionMsg(*pn, "Only a variable or member variable can be cleared.");
	if ((*pn)->str[0] == '#' || IsValidBuiltin((*pn)->str))
		throw ExceptionMsg(*pn, "Function cannot be cleared.");
	CVar *pres;
	string vam;
	if (!(pres = GetVariable((*pn)->str, vam, ndog.psigBase)))
		throw ExceptionMsg(*pn, "Variable not found.");
	AstNode * res = get_next_parsible_node(*pn);
	if (res && res->type == N_STRUCT && !strcmp(res->str, "clear"))
	{
		return NULL;
	}
	ndog.psigBase = pres;
	return res;
}

AstNode *CAstSig::read_node(CDeepProc &diggy, AstNode *pn,  AstNode *ppar)
{
	if (pn->type == T_OP_CONCAT || pn->type == '+' || pn->type == '-' || pn->type == T_TRANSPOSE || pn->type == T_MATRIXMULT
		|| pn->type == '*' || pn->type == '/' || pn->type == T_OP_SHIFT || pn->type == T_NEGATIVE || (pn==diggy.level.root && IsCondition(pn)))
	{ //No further actions
		return get_next_parsible_node(pn);
	}
	string emsg;
	AstNode *p = pn;
	int cellind, ind(0);
	CVar *pres;
	ostringstream out;
	AstNode *pUDF;
	if ((pUDF=ReadUDF(emsg, pn->str)))
	{
		if (pn->child)
			throw ExceptionMsg(pn, "Name conflict between the LHS variable and a user-defined function.");
//		ppar = get_parent_node(pAst, pn);
		if (ppar)
		{
			// See aux-indexing-parsing-plan.doc for more info. The parent node is lhs only if...
//			if ((higher->child == pn || higher->alt == pn) && (higher->type==T_ID || higher->type == N_CELL || higher->type == N_VECTOR))
//				lhs = higher;
			// Otherwise, the parent node is not necessarily the LHS. 10/14/2018
		}
		// if static function, diggy.level.psigBasemust be NULL
		if (pUDF->suppress==3 && diggy.level.psigBase)
			throw ExceptionMsg(pUDF, "Function declared as static cannot be called as a member function.");
		if (PrepareAndCallUDF(pn, diggy.level.psigBase)) // this probably won't return false
		{// if a function call follows N_ARGS, skip it for next_parsible_node
			diggy.level.psigBase = &Sig;
			if (pn->alt && pn->alt->type == N_ARGS)
				pn = pn->alt; // to skip N_ARGS
		}
	}
	else
	{
		if (!emsg.empty())	throw ExceptionMsg(pn, emsg.c_str());
		if (builtin_func_call(diggy, pn))
		{ // if a function call follows N_ARGS, skip it for next_parsible_node
			if (pn->alt)
			{
				if (pn->alt->type == N_ARGS || pn->alt->type == N_HOOK)
					pn = pn->alt; // to skip N_ARGS
			}
		}
		else if (pn->type == N_ARGS)
		{
			if (diggy.level.psigBase->IsGO() && diggy.level.psigBase->GetFs() != 3)
			{
				CVar tp = Compute(pn->child);
				if (tp.GetType()!=CSIG_SCALAR || tp.value()!=1.)
					throw ExceptionMsg(ppar, "Invalid index of a graphic object arrary.");
			}
			else
			{
				if (diggy.level.psigBase->GetType() == CSIG_CELL) throw ExceptionMsg(ppar, "A cell array cannot be accessed with ( ).");
				diggy.ExtractByIndex(ppar, pn); //Sig updated. No change in psig
			}
			if (diggy.level.psigBase->IsGO())
				pgo = diggy.level.psigBase;
		}
		else if (IsCondition(pn))
		{
			if (diggy.level.psigBase->GetType() == CSIG_CELL) throw ExceptionMsg(ppar, "A cell array cannot be accessed with ( ).");
			CVar isig, isig2;
			diggy.eval_indexing(pn, isig);
			Sig = diggy.extract(pn, isig);
			if (!pn->next) // 1D indexing, unGroup it
				Sig.nGroups = 1;
		}
		else if (pn->type == N_TIME_EXTRACT)
			diggy.TimeExtract(pn, pn->child);
		else if (pn->type == T_REPLICA || pn->type == T_ENDPOINT)
			Sig = *diggy.level.psigBase;
		else
		{
			if (pn->type == N_HOOK)
			{
				pres = GetGlobalVariable(pn, pn->str);
				if (!pres)
				{
					if (diggy.level.root->child && (!pn->alt || pn->alt->type == N_STRUCT)) 
						return NULL;
					throw ExceptionMsg(pn, "Gloval variable not available.");
				}
				diggy.level.psigBase = pres;
			}
			//Need to scan the whole statement whether this is a pgo statement requiring an update of GO
			else if (!(pres = GetVariable(pn->str, diggy.level.varname, diggy.level.psigBase)) )
			{
				if (diggy.level.root->child && (pn->type == N_STRUCT || pn->type == T_ID))
				{
					if (!pn->alt) return NULL; // if pn is the last node, no exception and continue to check RHS
					if (pn->alt->type == N_STRUCT) return NULL; // (something_not_existing).var = RHS
				}
				out << "Variable or function not available: " << pn->str;
				throw ExceptionMsg(pn, out.str().c_str());
			}
			if (pres->IsGO())
			{ // the variable pn->str is a GO
				Sig = *(diggy.level.psigBase = pgo = pres);
				if (pn->child) setgo.type = pn->str;
			}
			/* I had thought this would be necessary, but it works without this... what's going on? 11/6/2018*/
			// If pgo is not NULL but pres is not a GO, that means a struct member of a GO, such as fig.pos, then check RHS (child of root. if it is NULL, reset pgo, so that the non-GO result is displayed)
			else if (pgo && !diggy.level.root->child && !setgo.frozen)
				pgo = NULL;
			if (pn->alt)
			{
				p = pn->alt;
				if (p->type == N_CELL)
					//either x{2} or x{2}(3). x or x(2) doesn't come here.
					// I.E., p->child should be non-NULL
				{
					cellind = (int)Compute(p->child).value(); // check the validity of ind...probably it will be longer than this.
					out << "{" << cellind << "}";
					diggy.level.varname += out.str();
					Sig = *(diggy.level.psigBase = &pres->cell.at(cellind - 1)); // check the validity
				}
				else if (p->type == N_STRUCT)
				{
					if (!pres->cell.empty() || (!pres->IsStruct() && !pres->IsEmpty()))
					{
						if (p->str[0] != '#' && !IsValidBuiltin(p->str))
							if (!ReadUDF(emsg, p->str))
								if (!emsg.empty())
									throw ExceptionMsg(pn, emsg.c_str()); // check the message
								else
								{
									out << "Unknown variable, function, or keyword identifier : " << p->str;
									throw ExceptionMsg(pn, out.str().c_str());
								}
					}
					Sig = *(diggy.level.psigBase = pres);
				}
				else
					Sig = *(diggy.level.psigBase = pres);
			}
			else
				Sig = *(diggy.level.psigBase = pres);
		}
	}
	return get_next_parsible_node(pn);
}


AstNode *CAstSig::read_nodes(CDeepProc &diggy)
{
	AstNode *pn = diggy.level.root;
	AstNode *p, *pPrev=NULL;
	CVar pvar;
	while (pn)
	{
		p = read_node(diggy, pn, pPrev);
		if (!p) return pn;
		if (p->type == N_ARGS)
			pPrev = pn;
		else
			pPrev = NULL;
		pn = p;
	}
	return NULL; // shouldn't come thru here; only for the formality
}

CVar &CAstSig::TID(AstNode *pnode, AstNode *pRHS, CVar *psig)
{
	CDeepProc diggy(this, pnode, psig); // psig is NULL except for T_REPLICA
	if (pnode)
	{
		setgo.clear();
		char ebuf[256] = {};
		// a = sqrt(2) --> inside psigAtNode, without knowing whether this call is part of RHS handling or not, there's no way to know whether a string is something new to fill in later or just throw an exception.
		// by default the 3rd arg of psigAtNode is false, but if this call is made during RHS handling (where pRHS is NULL), it tells psigAtNode to try builtin_func_call first. 8/28/2018

		AstNode *pLast = read_nodes(diggy); // that's all about LHS.
		if (!diggy.level.psigBase)
		{
			lhs = pLast;
			diggy.level.side = 'L';
			Script = pnode->str;
			return define_new_variable(pnode, pRHS);
		}
		// At this point, Sig should be it
		// psig : the base content of Sig 
		// pLast: the node corresponding to psig
		if (!pRHS)
		{
			if (diggy.level.psigBase->IsGO()) return *pgo;
			else	return Sig;
		}
		setgo.frozen = true;
		diggy.level.side = 'R';
		lhs = pLast;
		CVar res = diggy.TID_RHS2LHS(pnode, pLast, pRHS, &Sig);
		if (setgo.type)
		{ // It works now but check this later. 2/5/2019
			if (res.IsGO())
				fpmsg.SetGoProperties(this, setgo.type, *diggy.level.psigBase);
			else
			{
				// For example, f.pos(2) = 200
				// we need to send the whole content of f.pos, not just f.pos(2), to SetGoProperties
				// 3/30/3029
				res = pgo->strut[setgo.type];
				fpmsg.SetGoProperties(this, setgo.type, res);
			}
		}
		Script = diggy.level.varname;
	}
	return Sig;
}

CVar &CAstSig::ConditionalOperation(const AstNode *pnode, AstNode *p)
{
//	why pgo = NULL; ? 
// pgo should be reset right after all Compute calls so upon exiting ConditionalOperation
// it shouldn't have any lingering pgo.
// pgo is supposed to be used only temporarily-- to relay go to the next step and it shouldn't linger too long.
// then at some point it may incorrectly try to process the GO when it is not about GO.
// 2/5/2019
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
		pgo = NULL;
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
		pgo = NULL;
		if (!Sig.IsLogical() || !rsig.IsLogical())
			throw ExceptionMsg(p, "Logical operation is only for logical arrays.");
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
		throw ExceptionMsg(pnode, "Script execution has been interrupted.");
	switch (pnode->type) {
	case T_ID:
		return TID((AstNode*)pnode, p);
	case N_HOOK:
		return TID((AstNode*)pnode, p);
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
			throw ExceptionMsg(p->next, "Second operand of '>>' must be a scalar.");
		Compute(p);
		checkAudioSig(pnode,  Sig);
		Sig >>= tsig.value();
		Sig.nGroups = tsig.nGroups;
		return TID((AstNode*)pnode, NULL, &Sig);
	case T_OP_CONCAT:
		Concatenate(pnode, p);
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
			pgo = NULL; // without this, go lingers on the next line 2/9/2019
			Sig.Reset(1); // without this, fs=3 lingers on the next line 2/9/2019
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
			throw ExceptionMsg(pnode, ("Calling sigma( )\n\nIn sigma expression:\n"+string(errmsg)).c_str());
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
			throw ExceptionMsg(pnode, "Internal error! CI Pulse without handler!");
		break;
	default:
	{
		ostringstream out;
		out << "Internal error! Unknown node type - " << pnode->type;
		throw ExceptionMsg(pnode, out.str().c_str());
	}
		
	}
} catch (const exception &e) {
	throw ExceptionMsg(pnode, (string("Internal error! ") + e.what()).c_str());
} 
return Sig;
}

CVar &CAstSig::Concatenate(const AstNode *pnode, AstNode *p)
{
	CVar tsig = Compute(p->next);
	if (pgo)
	{ //special treatment needed to multiple GO's
		vector<CVar*> tp;
		tp.push_back(pgo);
		Compute(p);
		if (Sig.IsEmpty()) return Sig = *pgo;
		//Now, Sig can be CSIG_HDLARRAY, then use it as is.
		if (!pgo)
			throw ExceptionMsg(p, "RHS is a graphic handle. LHS is not. Can't concatenate.");
		if (Sig.GetType() == CSIG_HDLARRAY)
		{
			Sig.UpdateBuffer(Sig.nSamples + 1);
			Sig.buf[Sig.nSamples - 1] = (double)(INT_PTR)tp.front();
		}
		return Sig;
	}
	Compute(p);
	if (pgo)
		throw ExceptionMsg(p, "LHS is a graphic handle. RHS is not. Can't concatenate.");
	if (Sig.nGroups > 1)
	if ((tsig.GetType() == CSIG_CELL) && (Sig.GetType() != CSIG_CELL))
		throw ExceptionMsg(p->next, "A cell variable cannot be appended to a non-cell variable.");
	if (Sig.GetType() == CSIG_CELL)
	{
		if (tsig.GetType() == CSIG_CELL)
		{
			for (size_t k = 0; k < tsig.cell.size(); k++)
				Sig.cell.push_back(tsig.cell[(int)k]);
		}
		else
			Sig.cell.push_back(tsig);
	}
	else
	{
		//Check rejection conditions
		if (tsig.nSamples * Sig.nSamples > 0) // if either is empty, no rejection
		{
			if (tsig.nGroups!=Sig.nGroups && tsig.Len() !=Sig.Len())
				throw ExceptionMsg(p->next, "To concatenate, the second operand must have the same number of elements or the same number of groups (i.e., rows) ");
		}
		//For matrix, Group-wise (i.e., row-wise) concatenation
		if (Sig.nGroups > 1 && Sig.Len()!=tsig.Len())
		{ //  append row-wise
			unsigned int len0 = Sig.Len();
			Sig.UpdateBuffer(Sig.nSamples + tsig.nSamples);
			unsigned int len1 = Sig.Len();
			unsigned int lent = tsig.Len();
			for (unsigned int k, kk = 0; kk < Sig.nGroups; kk++)
			{
				k = Sig.nGroups - kk - 1;
				memcpy(Sig.buf + len1 * k, Sig.buf + len0 * k, sizeof(double)*len0);
				memcpy(Sig.buf + len1 * k + len0, tsig.buf + lent * k, sizeof(double)*lent);
			}
		}
		else
		{ // Sig and tsig have both row and column counts, concatenation is done here... at the end of row--making more rows withthe same column counts (not row-wise concatenating)
			if (Sig.nGroups > 1) Sig.nGroups += tsig.nGroups;
			Sig += &tsig;
			Sig.MergeChains();
		}
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
		Sig.Reset(1);
		Sig.cell.reserve(count);
		for (; p; p = p->next)
			Sig.appendcell(temp.Compute(p));
		if (pnode->str)
			SetVar(pnode->str, &Sig);
		return Sig;
	}
	catch (const CAstException &e) {
		throw ExceptionMsg(pnode, e.getErrMsg().c_str());
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

CAstSig &CAstSig::SetGloVar(const char *name, CVar *psig, CVar *pBase)
{
	if (!pBase) // top scope
	{
		map<string, vector<CVar*>>::iterator jt = pEnv->glovar.find(name);
		if (jt != pEnv->glovar.end())  pEnv->glovar.erase(jt);
		if (psig->IsGO())
		{
			if (!strcmp(name, "gca") || !strcmp(name, "gcf"))
				pEnv->glovar[name].clear();
			if (psig->GetFs() == 3)
			{
				if (psig->nSamples == 1)
				{
					psig = (CVar*)(INT_PTR)psig->value();
					pEnv->glovar[name].push_back(psig);
				}
				else
					for (unsigned int k = 0; k < psig->nSamples; k++)
						pEnv->glovar[name].push_back((CVar*)(INT_PTR)psig->buf[k]);
			}
			else
				pEnv->glovar[name].push_back(psig);
		}
		else // name and psig should be fed to Var
		{
			CVar *pTemp = new CVar;
			*pTemp = *psig;
			pEnv->glovar[name].push_back(pTemp);
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
	throw ExceptionMsg(pnode, FuncSigs, msg.str());
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
		glovar = rhs.glovar;
	}
	return *this;
}



void goaction::clear()
{
	frozen = NULL; psig = NULL;  type = NULL; 	RHS.Reset();
}
