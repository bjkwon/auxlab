// AUXLAB 
//
// Copyright (c) 2009-2019 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.7
// Date: 5/24/2020
// 
#include <sstream>
#include <list>
#include <algorithm>
#include <exception>
#include <math.h>
#include <time.h>
#include <limits>
#include <assert.h>
#include "aux_classes.h"
#include "sigproc.h"
#include "bjcommon.h"

#include <algorithm> // for lowercase

#ifndef CISIGPROC
#include "psycon.tab.h"
#else
#include "cipsycon.tab.h"
#endif

#include "sigproc_internal.h"

//Application-wide global variables
vector<CAstSig*> xscope;
extern HWND hShowDlg;

#define PRINTLOG(FNAME,STR) \
{ FILE*__fp=fopen(FNAME,"at"); fprintf(__fp,STR);	fclose(__fp); }

/* An important change on 11/2/2019.

1) When defining a member variable of a class variable, the class variable must have been defined.
Otherwise, it will make an error. For example, previously
var.val=100
was acceptable without var having been defined. But that made the code excessively complicated when dot operators are nested.
Now, you must define var, even as a null variable, to define member variables. For example,
var=[]
var.val=100

2) The following (member) functions are added for the operation of class objects
.erase
	--- Removes the specified member variable. Returns the class.
		var.erase("val")
.head
	--- This is used to define a non-class object as a head object (non-class object)
		Works only with a class variable (this requirement can be a loose for now)

		audiodata=[]
		audiodata.cutoff = 1000
		audiodata.head(noise(1000).lpf(1000))

		*Alternatively, you may do the following
		audiodata = noise(1000).lpf(1000)
		audiodata.cutoff = 1000

		*But doing the following will clear any class property from the audiodata variable
		audiodata.cutoff = 1000
		audiodata = noise(1000).lpf(1000) // audiodata will no longer have the cutoff field
*/

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

void dummy_fun7(CAstSig *a, const char *b, const CVar &c)
{
}

map<string, vector<CVar *>> glovar_dummy; // need some kind of global definition
map<string, vector<CVar *>> CAstSigEnv::glovar = glovar_dummy;

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
		for (beginhere = xscope.front(); beginhere->son; )
			beginhere = beginhere->son;
	}
	for (CAstSig *pst = beginhere; xscope.size() > nLayersBase; )
	{
		xscope.pop_back();
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
	inTryCatch = src->inTryCatch;
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
		inTryCatch = src->inTryCatch;
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
		inTryCatch = src->inTryCatch;
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
	memset(callbackIdentifer, 0, LEN_CALLBACKIDENTIFIER);
	pLast = NULL;
	son = NULL;
	dad = NULL;
	lhs = NULL;

	FsFixed = false;
	pgo = NULL;
	Tick0 = 1;
	inTryCatch = 0;
}

CAstSig::~CAstSig()
{
	if (fAllocatedAst)
		yydeleteAstNode(pAst, 0);
}

unsigned long CAstSig::tic()
{
	return Tick0 = GetTickCount0();
}

unsigned long CAstSig::toc(const AstNode *p)
{
	if (Tick0==1)
		throw CAstExceptionInvalidUsage(*this, p, "toc called without tic");
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
void * GetHWND_WAVPLAY()
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


CVar * CAstSig::Eval(AstNode *pnode)
{
	try {
		if (!pnode)
			return &Sig;
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

void CAstSig::astsig_init(void(*fp1)(CAstSig *, DEBUG_STATUS, int), void(*fp2)(CAstSig *, const AstNode *), bool(*fp3)(const char *), void(*fp4)(CAstSig *), void(*fp5)(int), void(*fp6a)(const char*), void(*fp7)(CAstSig *, const char *, const CVar &), void(*fp8)(CAstSig *))
{
	fpmsg.UpdateDebuggerGUI = fp1;
	fpmsg.HoldAtBreakPoint = fp2;
	fpmsg.IsCurrentUDFOnDebuggerDeck = fp3;
	fpmsg.ShowVariables = fp4;
	fpmsg.Back2BaseScope = fp5;
	fpmsg.ValidateFig = fp6a;
	fpmsg.SetGoProperties = fp7;
	fpmsg.RepaintGO = fp8;
}

CFuncPointers::CFuncPointers()
{
	UpdateDebuggerGUI = dummy_fun1;
	HoldAtBreakPoint = dummy_fun2;
	IsCurrentUDFOnDebuggerDeck = dummy_fun3;
	ShowVariables = dummy_fun4;
	Back2BaseScope = dummy_fun5;
	ValidateFig = dummy_fun6a;
	SetGoProperties = dummy_fun7;
	RepaintGO = dummy_fun4;
}

CFuncPointers& CFuncPointers::operator=(const CFuncPointers& rhs)
{
	UpdateDebuggerGUI = rhs.UpdateDebuggerGUI;
	HoldAtBreakPoint = rhs.HoldAtBreakPoint;
	IsCurrentUDFOnDebuggerDeck = rhs.IsCurrentUDFOnDebuggerDeck;
	ShowVariables = rhs.ShowVariables;
	Back2BaseScope = rhs.Back2BaseScope;
	SetGoProperties = rhs.SetGoProperties;
	RepaintGO = rhs.RepaintGO;
	return *this;
}

string CAstSig::ExcecuteCallback(const AstNode *pCalling, vector<unique_ptr<CVar*>> &inVars, vector<unique_ptr<CVar*>> &outVars, bool customcallback)
{
	u.currentLine = pCalling->line; // ? 10/18/2018
	AstNode *p;
	CAstSigEnv tempEnv(*pEnv);
	//input parameter binding
	// required nArgin
	size_t nargin_expected = 0;
	for (AstNode *p = (AstNode *)pCalling->child; p; p = p->next, nargin_expected++)
	{
		if (p->type == N_IDLIST)
			p = p->child;
	}
	if (inVars.size() > nargin_expected)
		throw CAstExceptionInvalidUsage(*this, pCalling, " too many input args.");
	if (inVars.size() < nargin_expected)
		throw CAstExceptionInvalidUsage(*this, pCalling, " insufficient input args.");
	son = new CAstSig(&tempEnv);
	son->u = u;
	son->u.title = pCalling->str;
	son->u.debug.status = null;
	auto itInVar = inVars.begin();
	for (AstNode *p = (AstNode *)pCalling->child; p; p = p->next)
	{
		if (p->type == N_IDLIST)
			p = p->child;
		son->SetVar(p->str, **itInVar);
		itInVar++;
	}
	if (pCalling->alt->type == N_VECTOR)
	{
		p = ((AstNode *)pCalling->alt->str)->alt;
		auto itOutVar = outVars.begin();
		for (; p; p = p->next, itOutVar++)
			son->SetVar(p->str, **itOutVar);
	}
	son->lhs = lhs;
	son->dad = this; // necessary when debugging exists with stepping (F10), the stepping can continue in tbe calling scope without breakpoints. --=>check 7/25
	son->fpmsg = fpmsg;
	auto itUDF = pEnv->udf.find(pCalling->str);
	son->u.pUDF = (*itUDF).second.pAst;
	son->u.pUDF_base = son->u.pUDF;
	son->u.base = son->u.pUDF->str;
	son->pAst = son->u.pUDF_base->child->next;
	son->u.argout.clear(); //output argument string list
	AstNode *pOutParam = son->u.pUDF->alt;
	if (pOutParam) {
		if (pOutParam->type == N_VECTOR)
		{
			pOutParam = ((AstNode *)pOutParam->str)->alt;
			for (AstNode *pf = pOutParam; pf; pf = pf->next)
				son->u.argout.push_back(pf->str);
		}
		else
			throw CAstExceptionInternal(*this, pCalling, "[INTERNAL] UDF output should be alt and N_VECTOR", pOutParam->type);
	}
	if (lhs)
	{
		ostringstream oss;
		p = lhs->type == N_VECTOR ? ((AstNode *)lhs->str)->alt : lhs;
		for (son->u.nargout = 0; p && p->line == lhs->line; p = p->next, son->u.nargout++) {}
		if (son->u.nargout > (int)son->u.argout.size()+1) {
			oss << "Usage:\n";
			if (pOutParam->type == N_VECTOR)
				oss << "[" << pOutParam->str;
			oss << ", handle";
			for (p = pOutParam->next; p; p = p->next)
				oss << ", " << p->str;
			oss << "] = " << pCalling->str << "(...)";
			throw CAstExceptionInvalidUsage(*this, pCalling, "More output arguments specified than in the function definition", pCalling->str, oss.str().c_str());
		}
	}
	son->u.nargout = (int)son->u.argout.size();

	//If the line invoking the udf res = var.udf(arg1, arg2...), binding of the first arg, var, is done separately via pBase. The rest, arg1, arg2, ..., are done below with pf->next
	//if this is for udf object function call, put that psigBase for pf->str and the rest from pa
	//No need for input param binding/
	if (u.debug.status == stepping_in) son->u.debug.status = stepping;
	xscope.push_back(son);
	if (customcallback)
		sprintf(callbackIdentifer, "%s::%u", pCalling->str, son->Tick1);
	string out = callbackIdentifer;
	size_t nArgout = son->CallUDF(pCalling, NULL);
	// output parameter binding (internal)
	p = ((AstNode*)pCalling->alt->str)->alt;
	auto itpvar = outVars.begin();
	for (auto it : son->u.argout)
	{
		if (son->Vars.find(it) != son->Vars.end())
		{
			CVar *newtemp = new CVar;
			*newtemp = son->Vars[it];
			**itpvar = newtemp;
		}
		else
		{
			if (son->GOvars.find(it) == son->GOvars.end())
				continue;
			else
			{
				if (son->GOvars[it].size() > 1) // need to make an CSIG_HDLARRAY object
					**itpvar = MakeGOContainer(son->GOvars[it]);
				else
					**itpvar = son->GOvars[it].front();
			}
		}
		p = p->next;
		itpvar++;
		if (itpvar == outVars.end()) break;
	}
	// output parameter binding
	const char *varstr = lhs ? (lhs->type==N_VECTOR ? ((AstNode*)lhs->str)->alt->str : lhs->str) : "ans";
	if (lhs)
	{
		if (lhs->type == T_ID)
			varstr = lhs->str;
		else // should be lhs->type == N_VECTOR
			varstr = ((AstNode *)lhs->str)->alt->str;
	}
	if (customcallback)
	{
		for (auto sonvarname : son->u.argout)
		{
			CVar *psigfromson;
			if (son->GOvars.find(sonvarname) == son->GOvars.end())
				psigfromson = &son->Vars[sonvarname];
			else
				psigfromson = son->GOvars[sonvarname].front();
			SetVar(sonvarname.c_str(), psigfromson, &Vars.find(varstr)->second);
		}
	}
	else  // for default callback, instead of SetVar, directly apply CSignals::operator= (to keep strut etc)
		(*Vars.find(varstr)).second.CSignals::operator=(son->Vars[son->u.argout.front()]);
	double nargin = son->Vars["nargin"].value();
	double nargout = son->Vars["nargout"].value();
	if (lhs && lhs->type == N_VECTOR && ((AstNode *)lhs->str)->alt->next)
	{
		// In [x, h, a, b] = record(...)
		// h was SetVar'ed in _record() in AuxFunc.cpp, as a special case; don't mess with it and just skip it
		p = ((AstNode *)lhs->str)->alt->next->next; // that's why there's another next
		auto itvarname = son->u.argout.begin()+1;
		for (; p; p = p->next, itvarname++)
		{
			SetVar(p->str, &son->Vars[*itvarname]);
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
	u.pLastRead = NULL;
	if (pgo) pgo->functionEvalRes = true;
	Sig.functionEvalRes = true;
	//	if (need2repaintnow(pCalling)) // or maybe pBase??
	xscope.pop_back(); // move here????? to make purgatory work...
	return out;
}

void CAstSig::outputbinding(const AstNode *plhs)
{
	assert(plhs->type == N_VECTOR);
	if (Sigs.empty())
	{
		AstNode *p = ((AstNode *)plhs->str)->alt;
		if (p->next)
			throw CAstExceptionInvalidUsage(*this, p, "Too many output arguments.");
		bind_psig(p, &Sig);
	}
	else
	{
		vector<unique_ptr<CVar*>>::iterator it = Sigs.begin();
		for (AstNode *p = ((AstNode *)plhs->str)->alt; p; p = p->next)
		{
			bind_psig(p, *it->release());
			it++;
			if (it==Sigs.end() && p->next)
				throw CAstExceptionInvalidUsage(*this, p, "Too many output arguments.");
		}
	}
}

void CAstSig::outputbinding(const AstNode *pnode, size_t nArgout)
{
	auto count = 0;
	assert(lhs);
	// lhs must be T_ID, N_VECTOR, or N_ARGS
	AstNode *pp = lhs;
	if (lhs->type == N_VECTOR) 
		pp = ((AstNode *)lhs->str)->alt;
	for (auto varname : son->u.argout)
	{
		if (son->Vars.find(varname) != son->Vars.end())
		{
			bind_psig(pp, &son->Vars[varname]);
			if (count++ == 0)
			{
				Sig = son->Vars[varname]; //why is this needed? -- so update go Sig with non-go Sig 2/9/2019
				pgo = NULL;
			}
		}
		else if (son->GOvars.find(varname) != son->GOvars.end())
		{
			if (son->GOvars[varname].size() > 1) // need to make an CSIG_HDLARRAY object
				pgo = MakeGOContainer(son->GOvars[varname]);
			else
				pgo = son->GOvars[varname].front();
			if (count++ == 0)
				Sig = *pgo; //Ghost output to console
			SetVar(pp->str, pgo);
		}
		if (--nArgout == 0) break;
		pp = pp->next;
	}
	lhs = nullptr;
}
bool CAstSig::PrepareAndCallUDF(const AstNode *pCalling, CVar *pBase, CVar *pStaticVars)
{
	if (!pCalling->str)
		throw CAstExceptionInternal(*this, pCalling, "[INTERNAL] p->str null pointer in PrepareAndCallUDF(p,...)");
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
	if (GOvars.find("?foc") != GOvars.end()) son->GOvars["?foc"] = GOvars["?foc"];
	if (GOvars.find("gcf") != GOvars.end())	son->GOvars["gcf"] = GOvars["gcf"];
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
			throw CAstExceptionInternal(*this, pCalling, "[INTERNAL] PrepareCallUDF():supposed to be a local udf, but AstNode with that name not prepared");
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
		{
			if (pOutParam->str)
				pOutParam = ((AstNode *)pOutParam->str)->alt;
			else
				pOutParam = pOutParam->alt;
			for (AstNode *pf = pOutParam; pf; pf = pf->next)
				son->u.argout.push_back(pf->str);
		}
		else
			throw CAstExceptionInternal(*this, pCalling, "[INTERNAL] PrepareCallUDF():UDF output should be alt and N_VECTOR", pOutParam->type);
	}
	AstNode *pf, *pa;	 // formal & actual parameter; pa is for this, pf is for son
	//Checking requested output arguments vs formal output arguments
	if (lhs)
	{
		ostringstream oss;
		AstNode *p = lhs->type == N_VECTOR ? ((AstNode *)lhs->str)->alt : lhs;
		if (lhs->type == N_VECTOR) for (son->u.nargout = 0; p && p->line == lhs->line; p = p->next, son->u.nargout++) {}
		if (son->u.nargout > (int)son->u.argout.size()) {
			oss << "More output arguments than in the definition (" << son->u.argout.size() << ").";
			throw CAstExceptionInvalidUsage(*this, pCalling, oss.str().c_str(), son->u.pUDF->str);
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
	xscope.push_back(son);
	//son->SetVar("_________",pStaticVars); // how can I add static variables here???
	size_t nArgout = son->CallUDF(pCalling, pBase);
	// output parameter binding
	vector<CVar *> holder;
	size_t cnt = 0;
	// output argument transfer from son to this
	double nargin = son->Vars["nargin"].value();
	double nargout = son->Vars["nargout"].value();
	//Undefined output variables are defined as empty
	for (auto v : son->u.argout)
		if (son->Vars.find(v) == son->Vars.end() && (son->GOvars.find(v) == son->GOvars.end() || son->GOvars[v].front()->IsEmpty()))
			son->SetVar(v.c_str(), new CVar);
	//lhs is either NULL (if not specified), T_ID or N_VECTOR
	if (lhs)
	{
		outputbinding(pCalling, nArgout);
	}
	else // no output parameter specified. --> first formal output arg goes to ans
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
	if ((son->u.debug.status == stepping || son->u.debug.status == continuing) && u.debug.status == null)
	{ // no b.p set in the main udf, but in these conditions, as the local udf is finishing, the stepping should continue in the main udf, or debug.status should be set progress, so that the debugger would be properly exiting as it finishes up in CallUDF()
		u.debug.GUI_running = true;
		if (son->u.debug.status == stepping) // b.p. set in a local udf
			u.debug.status = stepping; 
		else // b.p. set in other udf 
			u.debug.status = progress;
	}
	if (son->GOvars.find("?foc") != son->GOvars.end()) GOvars["?foc"] = son->GOvars["?foc"];
	if (son->GOvars.find("gcf") != son->GOvars.end()) GOvars["gcf"] = son->GOvars["gcf"];
	delete son;
	son = NULL;
	u.pLastRead = NULL;
	if (pgo) pgo->functionEvalRes = true;
	Sig.functionEvalRes = true;
//	if (need2repaintnow(pCalling)) // or maybe pBase??
	xscope.pop_back(); // move here????? to make purgatory work...
	return true;
}
//FYI--hangs at hold_at_break_point and ...
//FYI--Go to HoldAtBreakPoint in xcom if you want to see how things are handled during debugging when the execution hangs 

size_t CAstSig::CallUDF(const AstNode *pnode4UDFcalled, CVar *pBase)
{	
	try {

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
		/* When this is called by auxcon, the memory for xscope is different from when called by xcom, so as a temporary measure,
		fpmsg.ShowVariables now includes xscope.push_back(son) and a duplicate xscope is managed on the xcom side.
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

		p = pFirst;
		while (p)
		{
			pLast = p;
			// T_IF, T_WHILE, T_FOR are checked here to break right at the beginning of the loop
			if (p->type == T_ID || p->type == T_FOR || p->type == T_IF || p->type == T_WHILE || p->type == N_IDLIST || p->type == N_VECTOR)
				hold_at_break_point(p);
			Compute(p);
			//		pgo = NULL; // without this, go lingers on the next line
			//		Sig.Reset(1); // without this, fs=3 lingers on the next line
			if (fExit) break;
			p = p->next;
		}
		if (!strcmp(u.application, "auxcon") && Script == "auxconscript01927362()")
			fpmsg.Back2BaseScope(0);
		if (u.debug.status != null)
		{
			//		currentLine = -1; // to be used in CDebugDlg::ProcessCustomDraw() (re-drawing with default background color)... not necessary for u.debug.status==stepping (because currentLine has been updated stepping) but won't hurt
			if (u.debug.GUI_running == true)
			{
				// send to purgatory and standby for another debugging key action, if dad is the base scope
				if (dad == xscope.front() && u.debug.status == stepping)
				{
					fpmsg.UpdateDebuggerGUI(this, purgatory, -1);
					fpmsg.HoldAtBreakPoint(this, pLast);
				}
				u.currentLine = -1;
				u.debug.inPurgatory = false; // necessary to reset the color of debug window listview.
				//when exiting from a inside udf (whether local or not) to a calling udf with F10 or F11, the calling udf now should have stepping.
				fpmsg.UpdateDebuggerGUI(this, exiting, -1);
			}
			if (xscope.size() > 2) // pvevast hasn't popped yet... This means son is secondary udf (either a local udf or other udf called by the primary udf)
			{//why is this necessary? 10/19/2018----yes this is...2/16/2019
				if (u.debug.status == stepping && fpmsg.IsCurrentUDFOnDebuggerDeck && !fpmsg.IsCurrentUDFOnDebuggerDeck(Script.c_str()))
					fpmsg.UpdateDebuggerGUI(this, entering, -1);
			}
		}
	}
	catch (const CAstException &e) {
		// Fix
		// try catch seems to work for usual cases
		// but if eval() leads to exception thrown, line col info are messed up.
		// 3/5/2020 
		if (inTryCatch)
		{ // Make a new struct variable from child of e.pTarget (which is T_CATCH)
			CVar temp;
			const char *name = pAst->alt->child->str;
			SetVar(name, &temp); // new variable; OK this way, temp is deep-copied
			string emsg = e.outstr;
			size_t id = emsg.find("[GOTO_BASE]");
			if (id != string::npos) emsg = emsg.substr(id + string("[GOTO_BASE]").size());
			CVar msg(emsg);
			SetVar("full", &msg, &Vars[name]);
			msg = e.basemsg;
			SetVar("base", &msg, &Vars[name]);
			emsg = e.msgonly;
			if (id != string::npos) emsg = emsg.substr(id + string("[GOTO_BASE]").size());
			msg = emsg;
			SetVar("body", &msg, &Vars[name]);
			msg = e.sourceloc;
			SetVar("source", &msg, &Vars[name]);
			msg.SetValue(e.line);
			SetVar("errline", &msg, &Vars[name]);
			msg.SetValue(e.col);
			SetVar("errcol", &msg, &Vars[name]);
			Compute(pAst->alt->next);
		}
		else
			throw e;
	}
	return u.nargout;
}

bool CAstSig::GOpresent(const AstNode *pnode, AstNode *p)
{ // returns true if the statement 
	string fname = pnode->str;
	if (fname == "figure" || fname == "plot" || fname == "line" || fname == "axes" || fname == "text" || fname == "replicate")
		return true;
	else
		return false;
}

bool CAstSig::need2repaintnow(const AstNode *pnode, AstNode *p)
{
	// if pnode is on the tree, i.e., part of the udf, no need to repaint, unless this is inside of debugger.
	if (!u.repaint) return false;
	bool partofudf = isthisUDFscope(pnode);
	if (partofudf)
	{
		//if ( /* inside the debugger*/ )
		//	return true;
		//else
			return false;
	}
	// if pnode is not on the tree, that means this is an independently standing statement, then repaint
	return true;
}

bool CAstSig::isthisUDFscope(const AstNode *pnode, AstNode *p)
{
	//Is this is a call made insde of a UDF?
	//If so, pnode should appear in next of chlld

	//This depends on the node structure made in psycon.y; 
	//If you change it, this should be adjusted as well.
	//look for T_IF, T_FOR and T_WHILE
	if (!u.pUDF) return false;
	if (!p)
	{
		if (!u.pLastRead)		p = u.pUDF->child;
		else p = u.pLastRead;
	}
	while (p)
	{
		if (p == pnode) { 
			u.pLastRead = p;  return true; }
		if (p->type == T_IF)
		{
			// p->child->next must be N_BLOCK
			// p->alt must be N_BLOCK if there's else
			if (isthisUDFscope(pnode, p->child->next->next))
			{
				if (p->next) u.pLastRead = p->next;
				return true;
			}
			if (p->alt)
			{
				if (!p->alt->next) return false; // alt->next being NULL means elseif (something) but no "else" before end
				if (isthisUDFscope(pnode, p->alt->next)) return true;
			}
			// In this code, if a, elseif b, elseif c, else d, end ==> c and d are not processed.
			// Get the bug!! 4/6/2019
		}
		if (pnode->line == p->line)
		{
			if (searchtree(pnode, p)) return true; // u.pLastRead is updated inside searchtree
		}
		p = p->next;
	}
	return false;

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

AstNode *CAstSig::searchtree(const AstNode *pTarget, AstNode *pStart)
{ // search pTarget in all lower nodes (child, alt, next) in pStart
	// return NULL if not found
	if (pStart->line != pTarget->line) return NULL;
	AstNode *p = pStart;
	while (p)
	{
		if (p == pTarget)  return u.pLastRead = p;
		if (p->alt) if (searchtree(pTarget, p->alt)) { u.pLastRead = pStart;  return p->alt;	}
		if (p->child) if (searchtree(pTarget, p->child)) { u.pLastRead = pStart;  return p->child; }
		if (p->next) if (searchtree(pTarget, p->next)) { u.pLastRead = pStart;  return p->next; }
		return NULL;
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
	if (!Sig.IsScalar())	throw CAstExceptionInvalidUsage(*this, p, "Logical operation applied to a non-scalar.", p->str);
	if (Sig.IsLogical()) 
		return Sig.logbuf[0];
	else				
		return Sig.value()!=0.;
}

void CAstSig::checkindexrange(const AstNode *pnode, CTimeSeries *inout, unsigned int id, string errstr)
{
	if (id>inout->nSamples) 
		throw CAstExceptionRange(*this, pnode, errstr.c_str(), "", id);
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

#ifdef _WINDOWS
string CAstSig::LoadPrivateUDF(HMODULE h, int id, string &emsg)
{
	PF pt = (PF)GetProcAddress(h, (LPCSTR)MAKELONG(1, 0)); // ReadAUXP
	string read;
	size_t res = pt(id, read); 
	CAstSig aux(pEnv);
	AstNode *tempAst = aux.SetNewScript(emsg, read.c_str());
	if (tempAst) 
	{
		char *newname = (char*)malloc(strlen(aux.pAst->str) + 1);
		newname[0] = '?';
		strcpy(newname + 1, aux.pAst->str);
		free(aux.pAst->str);
		aux.pAst->str = newname;
		size_t wh1, wh2;
		string fname, out;
		wh1 = read.find('=');
		wh2 = read.find('(');
		out = fname = read.substr(wh1 + 1, wh2 - wh1 - 1);
		trim(fname, ' ');
		fname.insert(0, "private://?");
		RegisterUDF(aux.pAst, fname.c_str(), read);
		aux.pAst = NULL;// To keep aux.pAst from deallocating when this function returns. This will be cleaned during the app shutdown. 9/19/2018
		return out;
	}
	return "";
}
#endif // _WINDOWS

multimap<CVar, AstNode *> CAstSig::register_switch_cvars(const AstNode *pnode, vector<int> &undefined)
{
	multimap<CVar, AstNode *> out;
	// This evaluates the udf and computes all case keys for switch..case... loops.
	AstNode *p, *_pnode = (AstNode *)pnode;
	for (; _pnode; _pnode = _pnode->next)
	{
		if (_pnode->type == T_SWITCH)
		{
			for (p = _pnode->alt; p && p->next; p = p->next->alt)
			{
				if (p->type == N_ARGS)
				{
					for (AstNode *pa = p->child; pa; pa = pa->next)
					{
						const CVar tsig2 = Compute(pa);
						out.emplace(tsig2, p->next);
					}
				}
				else if (p->type == T_OTHERWISE)
					break;
				else
				{
					try {
						CVar tsig2 = Compute(p);
						out.emplace(tsig2, p->next);
					}
					catch (const CAstException &e)
					{
						string emsg = e.outstr;
						string estr = e.tidstr;
						int line = e.line;
						line++; line--;
						undefined.push_back(e.line);
					}
				}
			}
		}
	}
	return  out;
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
	{ // Read aux private functions`
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
			auto fd = pEnv->udf.find(udf_filename);
			pEnv->udf.erase(fd);
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
	{
		auto it = pEnv->udf.find(p->str);
		if (it!= pEnv->udf.end())	pEnv->udf.erase(pEnv->udf.find(p->str));
		string emsg = string(string(udf_filename) + " vs ") + pnode4Func->str;
		throw CAstExceptionInvalidUsage(*this, p, "inconsistent function name", emsg.c_str());
	}
	vector<int> undefined;
	auto vv = register_switch_cvars(pnode4Func->child->next->next, undefined);

	pEnv->udf[udf_filename].pAst = pnode4Func;	
	pEnv->udf[udf_filename].fullname = fullfilename;
	pEnv->udf[udf_filename].content = filecontent.c_str();
	pEnv->udf[udf_filename].switch_case = vv;
	pEnv->udf[udf_filename].switch_case_undefined = undefined;
	
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

CVar * CAstSig::SetLevel(const AstNode *pnode, AstNode *p)
{
	CVar sigRMS, refRMS, dB = Compute(p->next);
	// if tsig is scalar -- apply it across the board of Sig
	// if tsig is two-element vector -- if Sig is stereo, apply each; if not, take only the first vector and case 1
	// if tsig is stereo-scalar, apply the scalar to each L and R of Sig. If Sig is mono, ignore tsig.next
	// if tsig is tseq, it must have the same chain and next structure (exception otherwise)
	if (p->type == '@')
	{// trinary
		if (dB.type() & TYPEBIT_TSEQ)
			throw CAstExceptionInvalidUsage(*this, pnode, "sig @ ref @ level ---- level cannot be time sequence.");
		refRMS <= Compute(p->child->next);
		if (!refRMS.IsAudio())
			throw CAstExceptionInvalidUsage(*this, pnode, "sig @ ref @ level ---- ref must be an audio signal.");
		refRMS.RMS(); // this should be called here, once another Compute is called refRMS.buf won't be valid
		Compute(p->child);
		sigRMS <= Sig;
		sigRMS.RMS();
		sigRMS -= refRMS;
	}
	else
	{
		Sig = Compute(p);
		sigRMS <= Sig;
		if (dB.type() == TYPEBIT_TSEQ + 1) // scalar time sequence
		{// for tseq leveling, % operator is used.
			for (CTimeSeries *p = &dB; p; p = p->chain)
				p->buf[0] = pow(10, p->buf[0] / 20.);
			for (CTimeSeries *p = dB.next; p; p = p->chain)
				p->buf[0] = pow(10, p->buf[0] / 20.);
			Sig % dB;
			return &Sig;
		}
		// sig @ level
		// if sig is chained, level is applied across all chains
		// i.e., currently there's no simple way to specify chain-specific levels 6/17/2020

		// A known hole in the logic here---if dB is stereo but first channel is scalar
		// and next is chained, or vice versa, this will not work
		// Currently it's difficult to define dB that way (maybe possible, but I can't think about an easy way)
		// A new, simpler and intuitive way to define T_SEQ should be in place
		// before fixing this hole.  10/7/2019

		//Being fixed.... but need further checking
		//6/17/2020
		if (dB.chain || (dB.next && dB.next->chain))
			sigRMS = sigRMS.runFct2getvals(&CSignal::RMS);
		else
			sigRMS.RMS(); // this should be called before another Compute is called (then, refRMS.buf won't be valid)
	}
	//Reject dB if empty, if string, or if bool
	if (dB.IsEmpty() || dB.IsString() || dB.IsBool())
		throw CAstExceptionInvalidUsage(*this, pnode, "Target_RMS_dB after @ must be a real value.");
	if (dB.nSamples > 1 && !dB.next)
		dB.SetNextChan(new CSignals(dB.buf[1]));
	sigRMS = dB - sigRMS;
	Sig | sigRMS;
	return &Sig;
}

void CAstSig::prepare_endpoint(const AstNode *p, CVar *pvar)
{  // p is the node the indexing starts (e.g., child of N_ARGS... wait is it also child of conditional p?
	if (p->next) // first index in 2D 
		endpoint = (double)pvar->nGroups;
	else
		endpoint = (double)pvar->nSamples;
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
	CTimeSeries *p = &isig;
	CVar out;
	CTimeSeries *p_out = &out;
	while (p)
	{
		int count = 0;
		for (unsigned int k = 0; k < p->nSamples; k++)
			if (p->logbuf[k]) count++;
		CSignal part;
		part.UpdateBuffer(count);
		part.nGroups = p->nGroups;
		part.tmark = p->tmark;
		count = 0;
		for (unsigned int k = 0; k < p->nSamples; k++)
			if (p->logbuf[k]) part.buf[count++] = k + 1;
		*p_out = part;
		p = p->chain;
		if (p)
		{
			p_out->chain = new CTimeSeries;
			p_out = p_out->chain;
		}
	}
	isig = out;
}


CVar * CAstSig::TSeq(const AstNode *pnode, AstNode *p)
{ 
	//For now (6/12/2018) only [vector1][vector2] where two vectors have the same length.
	CVar tsig2, tsig = Compute(p);
	int type1 = tsig.GetType();
	if (type1 != CSIG_SCALAR && type1 != CSIG_VECTOR)
		strcpy(pnode->str, "TSEQ"),	throw CAstExceptionInvalidUsage(*this, pnode,"Invalid tmark array");
	if (pnode->child->next) {
		//[tvalues][val] pnode->child->next is N_VECTOR for val
		//[tvalues][val;] pnode->child->next is N_MATRIX for val
		tsig2 = Compute(pnode->child->next);
		checkVector(pnode, tsig2);
		int type2 = tsig2.GetType();
		if (type2 != CSIG_SCALAR && type2 != CSIG_VECTOR)
			strcpy(pnode->str, "TSEQ"), throw CAstExceptionInvalidUsage(*this, pnode,"Invalid t-sequence value array");
		if (tsig2.nGroups == 1)
		{
			//if 
			if (pnode->child->next->type == N_VECTOR)
			{
				if (tsig2.nSamples != tsig.nSamples)
					strcpy(pnode->str, "TSEQ"), throw CAstExceptionInvalidUsage(*this, pnode,"Time point and value arrays must have the same length.");
			}
			else //pnode->child->next->type == N_MATRIX
			{
				//ok
			}
		}
		else
		{
			if (tsig2.nGroups != tsig.nSamples)
				strcpy(pnode->str, "TSEQ"), throw CAstExceptionInvalidUsage(*this, pnode,"The number of time points and time sequences must have the same.");
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
	Sig.setsnap();
	return &Sig;
}

string CAstSig::adjustfs(int newfs)
{
	string out;
	if (pEnv->Fs == newfs) return out;
	for (auto &it : Vars)
	{
		CSignals ratio(1);
		if (it.second.GetType() == CSIG_AUDIO)
		{
			CSignals gcopy, level1, level2;
			gcopy <= it.second;
			level1 = gcopy.RMS();
			ratio.SetValue( (double) pEnv->Fs / newfs);
			it.second.runFct2modify(&CSignal::resample, &ratio);
			if (ratio.IsString()) // this means there was an error during resample
			{
				sformat(out, "Error while resampling the variable %s\n[from libsamplerate]%s", it.first.c_str(), ratio.string().c_str());
				return out;
			}
			it.second.SetFs(newfs);
			gcopy <= it.second;
			level2 = gcopy.RMS();
			it.second *= CSignals(level2.value() / level1.value());
		}
	}
	pEnv->Fs = newfs; //Sample rate adjusted
	return out;
}

CVar * CAstSig::pseudoVar(const AstNode *pnode, AstNode *p, CSignals *pout)
{
//	int res;
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
			// _reserve_sel is not used any more
//			res = _reserve_sel(this, p, pout);
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
	return &Sig; // nominal return value 
}

CVar * CAstSig::NodeMatrix(const AstNode *pnode)
{ //[x1; x2]  if for a stereo audio signal, both x1 and x2 must be audio
	// if none of these elements are audio, it can have multiple rows [x1; x2; x3; .... xn]. But these elements must be the same length.
	AstNode *p = ((AstNode *)pnode->str)->alt;
	CVar esig, tsig = Compute(p);
	if (!p) return &(Sig = tsig);
	blockCell(pnode,  tsig);
	int audio(0);
	if (tsig.GetType() == CSIG_AUDIO ) audio = 1;
	else if (tsig.GetType() == CSIG_TSERIES) audio = 10;
	else if (tsig.GetType() == CSIG_TSERIES) audio = 10;
	else if (tsig.GetType() == CSIG_STRING || tsig.GetType() == CSIG_SCALAR || tsig.GetType() == CSIG_VECTOR) audio = -1;
	if (audio == 0 && !p->next)
		return &Sig.Reset();
	CVar *psig = &tsig;
	unsigned int k(1);
	if (audio >= 0)
	{
		if (!p->next) // must be audio > 0
		{
			CSignals *nullnext = new CSignals(tsig.GetFs());
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
				if (p->next->next)	throw CAstExceptionInvalidUsage(*this, pnode, "Currently two channels or less for audio signals or t-series are allowed.");
				tsig.SetNextChan(&esig);
			}
			else if (audio > 0)
			{
				if (p->next->next)	throw CAstExceptionInvalidUsage(*this, pnode, "Currently two channels or less for audio signals or t-series are allowed.");
				if (esig.GetType() != tsig.GetType())
					throw CAstExceptionInvalidUsage(*this, pnode, "Signal type, Audio (or t-series), must be the same in both channels.");
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
			if (!esig.IsEmpty() && tsig.Len() != esig.Len()) throw CAstExceptionInvalidUsage(*this, pnode,"All groups must have the same length for 2-D data. (All rows must be the same length in the matrix)");
			tsig += &esig;
			tsig.nGroups += esig.nGroups;
		}
	}
	return &(Sig = tsig);
}

CVar * CAstSig::NodeVector(const AstNode *pn)
{
	unsigned int len;
	vector<double> databuf;
	vector<complex<double>> cdatabuf;
	//First it checks whether every item has the same nGroups
	//Also checks if it's complex
	AstNode *p = pn->str ? (AstNode *)pn->str : (AstNode *)pn;
	//for vector, p is not NULL; p->alt should be non-null (check)
	CVar tsig = Compute(p->alt);
	if (tsig.IsAudio()) return &(Sig = tsig);
	bool thisisGO = false;
	bool beginswithempty = tsig.IsEmpty();
	thisisGO = tsig.GetType() == CSIG_HDLARRAY || tsig.IsGO();
	unsigned int ngroups = tsig.nGroups; // final number of rows 
	unsigned int totalLen = tsig.Len(); // final number of cols
	bool compl=tsig.IsComplex();
	int audiofs = 0;
	for (p = p->alt->next; p; p = p->next)
	{
		tsig = Compute(p);
		if (thisisGO && (tsig.GetType() != CSIG_HDLARRAY && !tsig.IsGO()))
			throw CAstExceptionInvalidUsage(*this, pn,"Graphic Handle and non-Graphic handle cannot be in an array");
		if ((!thisisGO && !beginswithempty) && (tsig.GetType() == CSIG_HDLARRAY || tsig.IsGO()))
			throw CAstExceptionInvalidUsage(*this, pn,"Graphic Handle and non-Graphic handle cannot be in an array");
		if (ngroups != tsig.nGroups)
			throw CAstExceptionInvalidUsage(*this, pn,"Every item in the brackets must have the same nGroups.");
		totalLen += Sig.Len();
		if (!compl) compl = tsig.IsComplex();
		thisisGO = tsig.GetType() == CSIG_HDLARRAY || tsig.IsGO();
		if (!audiofs && tsig.GetType() == CSIG_AUDIO)
			audiofs = tsig.GetFs();
	}
	CVar out;
	if (thisisGO)
	{
		out.SetFs(3);
		for (p = pn->child; p; p = p->next)
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
		return &(Sig = out);
	}
	out.UpdateBuffer(ngroups*totalLen);
	out.nGroups = ngroups;
	if (audiofs) out.SetFs(audiofs);
	if (compl) out.SetComplex();
	int col; // column locator
	p = pn->str ? (AstNode *)pn->str : (AstNode *)pn;
	for (col = 0, p = p->alt; p; p=p->next)
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
	return &(Sig = out);
}

vector<double> CAstSig::gettimepoints(const AstNode *pnode, AstNode *p)
{ // assume: pnode type is N_TIME_EXTRACT
	vector<double> out;
	CVar tsig1 = Compute(p);
	if (!tsig1.IsScalar())
		throw CAstExceptionInvalidUsage(*this, pnode, "Time marker1 should be scalar");
	CVar tsig2 = Compute(p->next);
	if (!tsig2.IsScalar())
		throw CAstExceptionInvalidUsage(*this, pnode, "Time marker2 should be scalar");
	out.push_back(tsig1.value());
	out.push_back(tsig2.value());
	return out;
}

#define MARKERCHARS "os.x+*d^v<>ph"

bool CAstSig::isThisAllowedPropGO(CVar *psig, const char *propname, const CVar &tsig)
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
			return ((tsig.GetType() == CSIG_VECTOR || tsig.GetType() == CSIG_AUDIO) && tsig.nSamples == 2);
		if (!strcmp(propname, "tick"))
			return (tsig.GetType() == CSIG_VECTOR || tsig.GetType() == CSIG_EMPTY);
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
		if (!strcmp(propname, "fontname") || !strcmp(propname, "string"))
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

vector<CVar *> CAstSig::Compute(void)
{ 
	// There are many reasons to use this function as a Gateway function in the application, avoiding calling Compute(pAst) directly.
	// Call Compute(pAst) only if you know exactly what's going on. 11/8/2017 bjk
	vector<CVar*> res;
	Sig.cell.clear();
	Sig.strut.clear();
	Sig.struts.clear();
	Sig.outarg.clear();
	Sig.outarg2.clear();
	Sig.SetNextChan(NULL);
	Sig.functionEvalRes = false;
	inTryCatch = 0;
//	pgo = NULL;
	lhs = NULL;
	try {
		if (!pAst) {
			res.push_back(&Sig);
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
				lhs = NULL; // to clear lhs from the last statement in the block 7/23/2019
//				pgo = nullptr; // 10/11/2019
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
		char errmsg[2048];
		strncpy(errmsg, e.getErrMsg().c_str(), sizeof(errmsg) / sizeof(*errmsg));
		errmsg[sizeof(errmsg) / sizeof(*errmsg) - 1] = '\0';
		throw errmsg;
	}
}

CAstSig &CAstSig::SetGloVar(const char *name, CVar *psig, CVar *pBase)
{
	if (!pBase) // top scope
	{
		map<string, vector<CVar*>>::iterator jt = CAstSigEnv::glovar.find(name);
		if (jt != CAstSigEnv::glovar.end())  CAstSigEnv::glovar.erase(jt);
		if (psig->IsGO())
		{
			if (!strcmp(name, "gca") || !strcmp(name, "gcf"))
				CAstSigEnv::glovar[name].clear();
			if (psig->GetFs() == 3)
			{
				if (psig->nSamples == 1)
				{
					psig = (CVar*)(INT_PTR)psig->value();
					CAstSigEnv::glovar[name].push_back(psig);
				}
				else
					for (unsigned int k = 0; k < psig->nSamples; k++)
						CAstSigEnv::glovar[name].push_back((CVar*)(INT_PTR)psig->buf[k]);
			}
			else
				CAstSigEnv::glovar[name].push_back(psig);
		}
		else // name and psig should be fed to Var
		{
			CVar *pTemp = new CVar;
			*pTemp = *psig;
			CAstSigEnv::glovar[name].push_back(pTemp);
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
{ // tidy up 8/15/2019
	if (!varname)
		throw CAstExceptionInvalidUsage(*this, pAst, "[INTERNAL] GetGlobalVariable(): NULL varname");
	if (pvar)
	{
	}
	else
	{
		string dummy;
		auto it = pEnv->pseudo_vars.find(varname);
		if ( it != pEnv->pseudo_vars.end())
		{
			(*it).second.func(this, pnode, NULL, dummy);
		}
		else 
		{
			map<string, vector<CVar*>>::iterator jt = CAstSigEnv::glovar.find(varname);
			if (jt == CAstSigEnv::glovar.end())
				return NULL;
			if ((*jt).second.front()->IsGO())
			{
				return NULL; // just for now 8/16/2019
			}
			else
			{
				Sig = *(*jt).second.front();
			}
		}
	}
	return &Sig;
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
		throw CAstExceptionInvalidUsage(*this, pAst, "[INTERNAL] GetGOVariable() should be called when varname is sure to exist in GOvars");
	}
}

CVar *CAstSig::GetVariable(const char *varname, CVar *pvar)
{ //To retrive a variable from a workspace pvar is NULL (default)
  //To retrive a member variable, specify pvar as the base variable 
 // For multiple GO's, calls GetGOVariable()
	string fullvarname = "";
	CVar *pout(NULL);
	if (!varname)
		throw CAstExceptionInvalidUsage(*this, pAst, "[INTERNAL] GetVariable(): NULL varname");
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
	CNodeProbe prober(this, pnode, psig);
	AstNode *pn = prober.root = pnode;
	while (pn)
	{
		AstNode *p = NULL;
		if (pn->type != T_ID && pn->type != N_STRUCT)
			throw CAstExceptionInvalidUsage(*this, pn, "Only a variable or member variable can be cleared."); // is it possible? 1/16/2020
		if (pn->str[0] == '#' || IsValidBuiltin(pn->str))
			throw CAstExceptionInvalidUsage(*this, pn, "Function cannot be cleared.");
		CVar *pres;
		if (!(pres = GetVariable(pn->str, prober.psigBase)))
			throw CAstExceptionInvalidUsage(*this, pn, "Variable not found.", pn->str);
		AstNode * res = get_next_parsible_node(pn);
		if (res && res->type == N_STRUCT && !strcmp(res->str, "clear"))
		{
			ClearVar(pn->str, prober.psigBase);
			return;
		}
		prober.psigBase = pres;
		pn = res;
	}
}

CVar *CAstSig::eval_RHS(AstNode *pnode)
{
	CVar *out = nullptr;
	AstNode *p = pnode->child;
	//while (p)
	for (AstNode *p = pnode; ; p=p->child)
	{
		if (!p->child)
		{
			out = Compute(p);
			break;
		}
	}
	return out;
}

inline void CAstSig::throw_LHS_lvalue(const AstNode *pn, bool udf)
{
	ostringstream out;
	out << "LHS must be an l-value. "; 
	if (udf)
	{
		out << "Name conflict between the LHS variable " << endl;
		out << "\"" << pn->str << "\" and a user-defined function" << endl;
		out << pEnv->udf[pn->str].fullname;
	}
	else if (pn->type==N_MATRIX)
	{
		out << " (cannot have a matrix on the LHS). ";
	}
	else if (pn->type == N_VECTOR)
	{
		out << " (LHS can be a vector [...] only if RHS is a function call.) ";
	}
	else
	{
		out << pn->str << " is a built-in function.";
	}
	throw CAstExceptionInvalidUsage(*this, pn, out.str().c_str());
}

CVar * CAstSig::getchannel(CVar *pin, const AstNode *pnode)
{
	if (pnode->child->next->type != T_FULLRANGE)
		throw CAstExceptionInvalidUsage(*this, pnode, "2-D style extraction of audio signal not implemented yet.");
	CVar tsig = Compute(pnode->child);
	checkScalar(pnode, tsig, "first arg should be either 1 or 2.");
	if (tsig.value() == 2.)
	{
		if (!pin->next)
			Sig.Reset(1);
		else
			Sig = *pin->next;
	}
	else if (tsig.value() == 1.)
	{
		delete Sig.next;
		Sig.next = NULL;
	}
	else
		throw CAstExceptionInvalidUsage(*this, pnode, "x(1,:) for left, x(2,:) for right");
	return &Sig;
}

AstNode *CAstSig::read_node(CNodeProbe &np, AstNode *pn, AstNode *ppar, bool &RHSpresent)
{
	if (pn->type == T_OP_CONCAT || pn->type == '+' || pn->type == '-' || pn->type == T_TRANSPOSE || pn->type == T_MATRIXMULT
		|| pn->type == '*' || pn->type == '/' || pn->type == T_OP_SHIFT || pn->type == T_NEGATIVE || (pn==np.root && IsCondition(pn)))
	{ //No further actions
		return get_next_parsible_node(pn);
	}
	string emsg;
	AstNode *p = pn;
	int ind(0);
	CVar *pres;
	ostringstream out;
	AstNode *pUDF;
	if (pn->type == T_ID || pn->type == N_STRUCT)
	{
		if (pn->str[0] == '?' && this==xscope.front())
			throw CAstExceptionInvalidUsage(*this, pn, "The caracter '?' cannot be used as a function or variable name in the base workspace.", pn->str);
	}
	if ((pUDF=ReadUDF(emsg, pn->str)))
	{
		if (pn->child || RHSpresent)	throw_LHS_lvalue(pn, true);
		// if static function, np.psigBase must be NULL
		if (pUDF->suppress==3 && np.psigBase)
			throw CAstExceptionInvalidUsage(*this, pUDF, "Function declared as static cannot be called as a member function.", pn->str);
		if (PrepareAndCallUDF(pn, np.psigBase)) // this probably won't return false
		{// if a function call follows N_ARGS, skip it for next_parsible_node
			np.psigBase = &Sig;
			if (pn->alt && pn->alt->type == N_ARGS)
				pn = pn->alt; // to skip N_ARGS
		}
	}
	else
	{
		if (!emsg.empty())	throw CAstExceptionInvalidUsage(*this, pn, emsg.c_str());
		if (builtin_func_call(np, pn))
		{
			if (pn->child || RHSpresent)	throw_LHS_lvalue(pn, false);
			np.psigBase = &Sig;
			// if a function call follows N_ARGS, skip it for next_parsible_node
			if (pn->alt)
			{
				if (pn->alt->type == N_ARGS || pn->alt->type == N_HOOK)
					pn = pn->alt; // to skip N_ARGS
			}
		}
		else if (pn->type == N_ARGS)
		{
			if (np.psigBase->IsGO() && np.psigBase->GetFs() != 3)
			{
				if (Compute(pn->child)->type()!= 1)
					throw CAstExceptionInvalidUsage(*this, ppar, "Invalid index of a graphic object arrary.");
			}
			else
			{
				if (np.psigBase->type() & TYPEBIT_CELL)
					throw CAstExceptionInvalidUsage(*this, ppar, "A cell array cannot be accessed with ( ).", ppar->str);
				if (np.psigBase->type() & TYPEBIT_AUDIO)
				{
					if (pn->child->next) // 2-D style notation 
					{
						if (pn->child->type == T_FULLRANGE)
							throw CAstExceptionInvalidUsage(*this, ppar, "The first arg in () cannot be : for audio.", ppar->str);
						np.psigBase = getchannel(np.psigBase, p); // special case; x(1,:) or x(2,:)
					}
				}
				else
					np.ExtractByIndex(ppar, pn); //Sig updated. No change in psig
				//if child exists --> RHS --> Sig just computed is only used as replica. Otherwise, Sig will be ignored (overridden)
				if (pn->child && np.root->child && searchtree(np.root->child, T_REPLICA))
					replica_prep(&Sig);
			}
			if (np.psigBase->IsGO())
				pgo = np.psigBase;
		}
		else if (pn->type == N_TIME_EXTRACT)
			np.TimeExtract(pn, pn->child);
		else if (pn->type == T_REPLICA || pn->type == T_ENDPOINT)
			Sig = *np.psigBase;
		else
		{
			if (pn->type == N_HOOK)
			{
				pres = GetGlobalVariable(pn, pn->str);
				if (!pres)
				{
					if (np.root->child && (!pn->alt || pn->alt->type == N_STRUCT)) 
						return NULL;
					throw CAstExceptionInvalidUsage(*this, pn, "Gloval variable not available.", pn->str);
				}
				np.psigBase = pres;
			}
			else
			{
				// If RHS exists (i.e., child is non-null), no need to get the LHS variable completely,
				// i.e., a.prop_layer_1.prop_layer_2 = (something else)
				// you don't need to getvariable at the level of prop_layer_2
				// (you still need to go down to the level prop_layer_1, though)
				//when pn is conditional, there's no RHS, i.e., pn->child doesn't mean RHS; just skip
				if (!IsCondition(pn))
					RHSpresent |= pn->child != nullptr;
				// With RHS, if pn->alt is null, no need to GetVariable; just return (no update of Sig or pgo)
				if (RHSpresent && !searchtree(np.root->child, T_REPLICA))
				{
					if (pn->type == N_VECTOR && pn->alt) throw_LHS_lvalue(pn, false);
					if (pn->alt == nullptr) return nullptr;
				}
				if (IsCondition(pn))
				{
					if (np.psigBase->GetType() == CSIG_CELL)
						throw CAstExceptionInvalidUsage(*this, ppar, "A cell array cannot be accessed with ( ).", ppar->str);
					CVar isig, isig2;
					np.eval_indexing(pn, isig);
					pres = np.extract(pn, isig);
					if (!pn->next) // 1D indexing, unGroup it
						Sig.nGroups = 1;
				}
				else
				{
					//Need to scan the whole statement whether this is a pgo statement requiring an update of GO
					if (!np.varname.empty()) np.varname += '.';
					np.varname += pn->str;
					if (!(pres = GetVariable(pn->str, np.psigBase)))
					{
						string varname;
						if (pn->type == N_STRUCT) varname = '.';
						varname += pn->str;
						throw CAstExceptionInvalidUsage(*this, pn, "Variable or function not available: ", varname.c_str());
					}
					if (pres->IsGO())
//					{ // the variable pn->str is a GO
						Sig = *(np.psigBase = pgo = pres);
					if (pgo && RHSpresent && pn->alt)
					{
						if (pn->alt->type == N_STRUCT) setgo.type = pn->alt->str;
						else if (pn->alt->type == N_ARGS) setgo.type = pn->str;
					}
				}
				/* I had thought this would be necessary, but it works without this... what's going on? 11/6/2018*/
				// If pgo is not NULL but pres is not a GO, that means a struct member of a GO, such as fig.pos, then check RHS (child of root. if it is NULL, reset pgo, so that the non-GO result is displayed)
//				else if (pgo && !np.root->child && !setgo.frozen)
//					pgo = NULL;
				if (searchtree(np.root->child, T_REPLICA))
				{
					replica_prep(pres);
					// Updating replica with pres, the variable reading at current node, is necessary whenever replica is present
					// But, if current node is final and type is one of these, don't update np.psigBase
					if (!pn->alt && (pn->type == N_STRUCT || pn->type == N_ARGS || pn->type == N_TIME_EXTRACT))
						return get_next_parsible_node(pn);
				}
				if (IsCondition(pn)) return get_next_parsible_node(pn);

				if (pn->alt && pn->alt->type == N_CELL)
				//either cellvar{2} or cellvar{2}(3). cellvar or cellvar(2) doesn't come here.
				// i.e., pn->alt->child should be non-NULL
				{
					size_t cellind = (size_t)(int)Compute(pn->alt->child)->value(); // check the validity of ind...probably it will be longer than this.
					if (pres->type() & TYPEBIT_CELL)
					{
						if (cellind > pres->cell.size())
							throw CAstExceptionRange(*this, pn->alt, "", pn->str, -1, (int)cellind);
						Sig = *(np.psigBase = &pres->cell.at(cellind - 1));
					}
					else
					{ // in this case x{2} means second chain
						if (cellind > pres->CountChains())
							throw CAstExceptionRange(*this, pn->alt, "", pn->str, -1, (int)cellind);
						CTimeSeries *pout = pres;
						for (size_t k = 0; k < cellind; k++, pout = pout->chain) {}
						Sig = *pout;
						np.psigBase = &Sig;
					}
				}
				else
				{
					p = pn->alt;
					if (p && p->type == N_STRUCT && !RHSpresent)
					{
						if (!pres->cell.empty() || (!pres->IsStruct() && !pres->IsEmpty()))
						{
							if (p->str[0] != '#' && !IsValidBuiltin(p->str))
								if (!ReadUDF(emsg, p->str))
									if (!emsg.empty())
										throw CAstExceptionInvalidUsage(*this, pn, emsg.c_str()); // check the message
									else
									{
										out << "Unknown variable, function, or keyword identifier : " << p->str;
										if (pn->str) out << " for " << pn->str;
										throw CAstExceptionInvalidUsage(*this, pn, out.str().c_str());
									}
						}
					}
					Sig = *(np.psigBase = pres);
				}
			}
		}
	}
	return get_next_parsible_node(pn);
}


AstNode *CAstSig::read_nodes(CNodeProbe &np, bool bRHS)
{
	AstNode *pn = np.root;
	AstNode *p, *pPrev=NULL;
	CVar pvar;
	bool RHSpresent = bRHS;
	while (pn)
	{
		// Sig gets the info on the last node after this call.
		// when np.root->child is not NULL, 
		// if pn->alt is terminal (not null), it doesn't have to go thru getvariable.
		p = read_node(np, pn, pPrev, RHSpresent);
		if (!p) return pn;
		if (p->type == N_ARGS)
			pPrev = pn;
		else
			pPrev = NULL;
		pn = p;
	}
	return NULL; // shouldn't come thru here; only for the formality
}

/* CAstSig::bind_psig and CNodeProbe::TID_RHS2LHS perform a similar task.
The difference:
	bind_psig binds *psig (already computed) to an existing variable (no new creation of a variable)
	TID_RHS2LHS computes RHS and binds it to a variable, existing or new.
	11/6/2019
*/

void CAstSig::bind_psig(AstNode *pn, CVar *psig)
{ // update psig with newsig
	// if pn is T_ID without alt-->SetVar 
	// if pn is N_AGRS or N_STRUCT --> update psig with newsig according to the indices or dot 
	assert(pn->type==T_ID);
	if (!pn->alt)
	{
		SetVar(pn->str, psig);
	}
	else 
	{
		if (pn->alt->type == N_ARGS)
		{
			CNodeProbe ndprob(this, pn, NULL);
			// ndprob.psigBase should be prepared to do indexing
			AstNode *lhs_now = read_nodes(ndprob, true); 
			ndprob.TID_indexing(pn->alt, NULL, psig);
		}
		else if (pn->alt->type == N_TIME_EXTRACT)
		{
		}
		else
		{
			assert(pn->alt->type == N_STRUCT);
			CVar *pbasesig = GetVariable(pn->str);
			SetVar(pn->alt->str, psig, pbasesig);
		}
	}

}

CVar * CAstSig::TID(AstNode *pnode, AstNode *pRHS, CVar *psig)
{
	//CVar *tmp;
	//if (tmp = dfast(pnode, 0)) return tmp;

	CNodeProbe np(this, pnode, psig); // psig is NULL except for T_REPLICA
	if (pnode)
	{
		setgo.clear();
		char ebuf[256] = {};
		// a = sqrt(2) --> inside psigAtNode, without knowing whether this call is part of RHS handling or not, there's no way to know whether a string is something new to fill in later or just throw an exception.
		// by default the 3rd arg of psigAtNode is false, but if this call is made during RHS handling (where pRHS is NULL), it tells psigAtNode to try builtin_func_call first. 8/28/2018

		AstNode *pLast = read_nodes(np); // that's all about LHS.
		// var = (any expression): pLast is T_ID and no alt, child represents (any expression)
		// var(id) = (any expression): pLast is N_ARGS
		// var.prop = (any expression): pLast is N_STRUCT
		/* when var is not available, i.e., np.psigBase is NULL,
			var = (any expression) : from RHS to LHS
			var(id) = (any expression) : error
			var.prop = (any expression) : var is generated as a class variable with prop
			(expression) : pnode and pLast are the same
		*/
		/* if pnode is a terminal T_ID (i.e., no alt), np.psigBase is NULL; pLast is the same as pnode,
		   whether the variable exists or not, and the RHS will be computed and bound to the LHS.
		   np.psigBase points to the variable of the base object (if pnode->alt is N_STRUCT)
		   or the variable itself (if pnode->alt is N_ARGS), in which case pLast points the appropriate
		   lower node (the one before the terminal node), so that the last binding is done appropriately,
		   example1) var.prop1.prop2 = RHS
		   pLast is the node corresponding to var.prop1;  np.psigBase points to the object var.prop1.
		   example2) var.prop1.prop2(id) = RHS
		   pLast is the node corresponding to var.prop1.prop2;  np.psigBase points to the object var.prop1.prop2
		*/
		AstNode *lhsCopy = nullptr;
		if (pRHS)
			lhsCopy = lhs = pLast;
		else
		{
			if (np.psigBase && np.psigBase->IsGO())
				return np.psigBase;
			else	return &Sig;
		}
		CVar *pres;
		if (!np.psigBase)
		{
			Script = pnode->str;
			Script += np.varname;
		}
		pres = np.TID_RHS2LHS(pnode, pLast, pRHS, np.psigBase);
		lhs = lhsCopy;
		if (np.psigBase)
			Script = np.varname;
		// At this point, Sig should be it
		// psig : the base content of Sig 
		// pLast: the node corresponding to psig
		setgo.frozen = true;
		if (setgo.type)
		{ // It works now but check this later. 2/5/2019
			if (pres->GetFs() == 3)
				throw CAstExceptionInternal(*this, pnode, "In this version, don't adjust properties of multiple GOs on LHS.");
			if (pres->IsGO())
				fpmsg.SetGoProperties(this, setgo.type, *np.psigBase);
			else
			{
				// For example, f.pos(2) = 200
				// we need to send the whole content of f.pos, not just f.pos(2), to SetGoProperties
				// 3/30/2019
				if (pnode->tail->type==N_ARGS)
					pres = &pgo->strut[setgo.type];
				if (GOpresent(pnode)) u.repaint = true;
				fpmsg.SetGoProperties(this, setgo.type, *pres);
			}
		}
	}
	return &Sig;
}

CVar * CAstSig::Dot(AstNode *p)
{ // apply dot operator to Sig that was computed from the previous node
	// At this point, p is alt from a previous node and should not have child (i.e., RHS);
	// therefore, lhs should not be updated here.
	CNodeProbe np(this, p, &Sig);
	read_nodes(np);
	if (np.psigBase->IsGO()) return pgo;
	else	return &Sig;
}


CVar * CAstSig::ConditionalOperation(const AstNode *pnode, AstNode *p)
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
		if (Sig.IsString())
			Sig.SetValue((double)((CSignal)Sig == (CSignal)rsig));
		else
			Sig.LogOp(rsig, pnode->type);
		break;
	case T_LOGIC_NOT:
		Sig = Compute(p);
		blockCell(pnode, Sig);
		Sig.MakeLogical();
		Sig.LogOp(rsig, pnode->type); // rsig is a dummy for func signature.
		break;
	case T_LOGIC_AND:
		rsig = ConditionalOperation(p, p->child);
		if (rsig.nSamples==1 && !rsig.logbuf[0])
		{
			Sig.bufBlockSize = 1;
			Sig.logbuf[0] = false;
			break;
		}
		ConditionalOperation(p->next, p->next->child);
		Sig.LogOp(rsig, pnode->type);
		break;
	case T_LOGIC_OR:
		rsig = ConditionalOperation(p, p->child);
		if (rsig.nSamples == 1 && rsig.logbuf[0])
		{
			Sig.bufBlockSize = 1;
			Sig.logbuf[0] = true;
			break;
		}
		ConditionalOperation(p->next, p->next->child);
		Sig.LogOp(rsig, pnode->type);
		break;
	case T_NUMBER:
		Compute(pnode);
		Sig.MakeLogical();
		if (Sig.value() == 0.)		Sig.logbuf[0] = false;
		else Sig.logbuf[0] = true;
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
/* Dot operation (either member variable or member function) is defined on "alt"
examples each type with non-NULL alt
T_ID	x.val
N_HOOK	?
N_TSEQ	?
N_IDLIST	?
T_NUMBER	(5).sqrt
T_STRING	("bjkwon").length
N_MATRIX	[4 2 9; 8 7 3].max
N_VECTOR	[4 2 9].sqrt
*/
CVar * CAstSig::Compute(const AstNode *pnode)
{
	CVar tsig, isig, lsig, rsig;
	bool trinary(false);
	if (!pnode) 
	{	Sig.Reset(1); return &Sig; }
	AstNode *p = pnode->child;
	if (GfInterrupted)
		throw CAstExceptionInvalidUsage(*this, pnode, "Script execution has been interrupted.");
	switch (pnode->type) {
	case T_ID:
		return TID((AstNode*)pnode, p);
	case T_TRY:
		inTryCatch++; //
		Compute(p);
		break;
	case T_CATCH:
		inTryCatch--; //???? Maybe no longer needed?? 3/7/2020
		// p is T_ID for ME (exception message caught)
		// continue here............1/12/2020
		Compute(pnode->next);
		break;
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
		return &Sig;
	case T_NUMBER:
		Sig.Reset();
		Sig.SetValue(pnode->dval);
		return TID(pnode->alt, p, &Sig);
	case T_STRING:
		Sig.Reset();
		Sig.SetString(pnode->str);
		return TID(pnode->alt, p, &Sig);
	case N_MATRIX:
		if (p) throw_LHS_lvalue(pnode, false);
		NodeMatrix(pnode);
		return Dot(pnode->alt);
	case N_VECTOR:
		//Non-null p is allowed only if RHS is function (built-in or udf). Otherwise, throw.
		if (p)
		{ // if p (RHS exists), evaluate RHS first; then go to LHS (different from usual ways)
			string emsg;
			string funcname;
			if (p->type == T_ID)
				funcname = p->str;
			if (p->alt && p->alt->type == N_STRUCT)
				funcname = p->alt->str;
			if (!ReadUDF(emsg, funcname.c_str()) && !IsValidBuiltin(funcname) && emsg.empty())
				throw_LHS_lvalue(pnode, false);
			// Now, evaluate RHS 
			// why not TID(((AstNode*)pnode->str), p), which might be more convenient? (that's the "inner" N_VECTOR node)
			// Because then there's no way to catch [out1 out2].sqrt = func
			return TID((AstNode*)pnode, p);
		}
		else
		{
			NodeVector(pnode);
			return Dot(pnode->alt);
		}
		break;
	case T_REPLICA:
		return TID((AstNode*)pnode, NULL, &replica); //Make sure replica has been prepared prior to this
	case T_ENDPOINT:
		tsig.SetValue(endpoint);
		return TID((AstNode*)pnode, NULL, &tsig); //Make sure endpoint has been prepared prior to this
	case '+':
	case '-':
		if (pnode->type=='+')	tsig = Compute(p->next);
		else					tsig = -*Compute(p->next);
		blockCell(pnode, Sig);
		blockString(pnode, Sig);
		Compute(p);
		blockCell(pnode, Sig);
		blockString(pnode, Sig);
		blockEmpty(pnode, Sig);
		Sig += tsig;
		return TID((AstNode*)pnode->alt, NULL, &Sig);
	case '*':
	case '/':
	case T_MATRIXMULT: // "**"
		tsig = Compute(p);
		blockCell(pnode,  Sig);
		blockString(pnode,  Sig);
		if (pnode->type=='*' || pnode->type == '/')	Compute(p->next);
		else
		{
			checkVector(pnode, tsig);
			Compute(p->next);
			checkVector(pnode, Sig);
			Sig = (CSignals)tsig.matrixmult(&Sig);
			return TID((AstNode*)pnode, NULL, &Sig);
		}
		blockString(pnode,  Sig);
		blockEmpty(pnode, Sig);
		// reciprocal should be after blocking string (or it would corrupt the heap) 6/3/2020
		if (pnode->type == '/') Sig.reciprocal();
		Sig *= tsig;
		return TID((AstNode*)pnode->alt, NULL, &Sig);
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
		return TID((AstNode*)pnode->alt, NULL, &Sig);
		break;
	case T_NEGATIVE:
		-*Compute(p);
		blockString(pnode,  Sig);
		break;
	case T_OP_SHIFT:
		tsig = Compute(p->next);
		blockCell(pnode,  Sig);
		if (!tsig.IsScalar())
			throw CAstExceptionInvalidArg(*this, p->next, "must be a scalar.", ">>", 2);
		Compute(p);
		checkAudioSig(pnode,  Sig);
		Sig >>= tsig.value();
		Sig.nGroups = tsig.nGroups;
		return TID((AstNode*)pnode, NULL, &Sig);
		break;
	case T_OP_CONCAT:
		Concatenate(pnode, p);
		return TID((AstNode*)pnode->alt, NULL, &Sig);
		break;
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
//			pgo = NULL; // without this, go lingers on the next line 2/9/2019
			Sig.Reset(1); // without this, fs=3 lingers on the next line 2/9/2019
		}
		break;
	case T_IF:
		if (!p) break;
		pLast = p;
		hold_at_break_point(p);
		if (checkcond(p))
			Compute(p->next);
		else if (pnode->alt) 
			Compute(pnode->alt);
		break;
	case T_SWITCH:
	{
		switch_case_handler(pnode);
		tsig = Compute(p);
		auto fd = find(pEnv->udf[u.base].switch_case_undefined.begin(), pEnv->udf[u.base].switch_case_undefined.end(), p->line);
		if (fd != pEnv->udf[u.base].switch_case_undefined.end())
		{ // update switch_case
			CVar tsig2 = Compute(p->alt);
			pEnv->udf[u.base].switch_case.emplace(tsig2, p->next);
		}
		if (pEnv->udf[u.base].switch_case.find(tsig) == pEnv->udf[u.base].switch_case.end())
		{
			Compute(pnode->tail->next);
		}
		else
		{
			auto ret = pEnv->udf[u.base].switch_case.equal_range(tsig);
			auto it = ret.first;
			for (; it != ret.second; it++)
			{
				int endline = (*it).second->line + 1;
				if (pnode->next)
					endline = pnode->next->line;
				if ((*it).second->line > pnode->line && (*it).second->line < endline)
				{
					Compute((*it).second);
					break;
				}
			}
			if (it == ret.second)
				Compute(pnode->tail->next);
		}
	}
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
			//	assuming that (pnode->alt->type == N_BLOCK) 
			// Now, not going through N_BLOCK 1/4/2020
			// 1) When running in a debugger, it must go through N_BLOCK 
			// 2) check if looping through pa->next is bullet-proof 
			for (AstNode *pa = pnode->alt->next; pa; pa = pa->next)
			{
				pLast = pa;
				hold_at_break_point(pa);
				Compute(pa);
			}
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
		} catch (const char *errmsg) { // do it again 1/16/2020
			throw CAstExceptionInvalidUsage(*this, pnode, ("Calling sigma( )\n\nIn sigma expression:\n"+string(errmsg)).c_str());
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
			throw CAstExceptionInternal(*this, pnode, "[INTERNAL] CI Pulse without handler!");
		break;
	default:
		throw CAstExceptionInternal(*this, pnode, "[INTERNAL] Unknown node type", pnode->type);
	}
	return &Sig;
}

void CAstSig::switch_case_handler(const AstNode *pnode)
{
	// if switch is first encountered, take care of switch_case_undefined
	multimap<CVar, AstNode *> more;
	vector<int> less = pEnv->udf[u.base].switch_case_undefined;
	AstNode *p = pnode->alt;
	for (; p && p->next && !less.empty(); p = p->next->alt)
	{
		if (p->line == less.front())
		{
			pLast = p; // to show the error line correctly inside the switch block
			more.emplace(Compute(p), p->next);
			auto fd = find(less.begin(), less.end(), p->line);
			less.erase(fd);
		}
	}
	pLast = pnode;
	//check if there's a duplicate
	//first is there a duplicate within more?
	for (auto mi = more.begin(); mi != more.end(); mi++)
	{
		auto va = (*mi).first;
		for (auto mj = next(mi, 1); mj != more.end(); mj++)
		{
			bool b1 = (*mi).first < (*mj).first;
			bool b2 = (*mj).first < (*mi).first;
			if ( !b1 && !b2 )
			{
				string extra;
				sformat(extra, "expressions on line %d and line %d", (*mi).second->line - 1, (*mj).second->line - 1);
				throw CAstExceptionInvalidUsage(*this, pnode, "case duplates detected in a switch block--", "", extra);
			}
		}
	}
	// check more against pEnv->udf[u.base].switch_case
	for (auto ck = more.begin(); ck != more.end(); ck++)
	{
		auto fd = pEnv->udf[u.base].switch_case.find((*ck).first);
		if (fd != pEnv->udf[u.base].switch_case.end())
		{ // duplicate found. Is it in the same switch range?
			int a = (*ck).second->line;
			int endline = (*ck).second->line + 1;
			if (pnode->next)
				endline = pnode->next->line;
			if ((*ck).second->line > pnode->line && (*ck).second->line < endline)
			{
				string extra;
				sformat(extra, "expressions on line %d and line %d", (*ck).second->line - 1, (*fd).second->line - 1);
				throw CAstExceptionInvalidUsage(*this, pnode, "case duplates detected in a switch block--", "", extra);
			}
		}
	}
	for (auto ck = more.begin(); ck != more.end(); ck++)
		pEnv->udf[u.base].switch_case.emplace((*ck).first, (*ck).second);
	pEnv->udf[u.base].switch_case_undefined.clear();
}

void CAstSig::Concatenate(const AstNode *pnode, AstNode *p)
{
	ostringstream oss;
	CVar tsig = Compute(p->next);
	if (pgo)
	{ //special treatment needed to multiple GO's
		vector<CVar*> tp;
		tp.push_back(pgo);
		Compute(p);
		if (Sig.IsEmpty())
		{
			Sig = *pgo;
			return;
		}
		//Now, Sig can be CSIG_HDLARRAY, then use it as is.
		if (!pgo)
			throw CAstExceptionInvalidUsage(*this, p, "RHS is a graphic handle. LHS is not. Can't concatenate.");
		if (Sig.GetType() == CSIG_HDLARRAY)
		{
			Sig.UpdateBuffer(Sig.nSamples + 1);
			Sig.buf[Sig.nSamples - 1] = (double)(INT_PTR)tp.front();
		}
		return;
	}
	Compute(p);
	uint16_t a = tsig.type();
	uint16_t b = Sig.type();
	if (a & TYPEBIT_CELL)
	{
		if (b & TYPEBIT_CELL)
		{
			for (size_t k = 0; k < tsig.cell.size(); k++)
				Sig.cell.push_back(tsig.cell[(int)k]);
		}
		else
			Sig.cell.push_back(tsig);
	}
	else
	{
		if (b > 0 && a >> 2 != b >> 2)
			throw CAstExceptionInvalidUsage(*this, p, "Different object type between LHS and RHS. Can't concatenate.");
		//Check rejection conditions
		if (tsig.nSamples * Sig.nSamples > 0) // if either is empty, no rejection
		{
			if (tsig.nGroups != Sig.nGroups && tsig.Len() != Sig.Len())
				throw CAstExceptionInvalidUsage(*this, p->next, "To concatenate, the second operand must have the same number of elements or the same number of groups (i.e., rows) ");
		}
		//For matrix, Group-wise (i.e., row-wise) concatenation
		if (Sig.nGroups > 1 && Sig.Len() != tsig.Len())
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

CVar * CAstSig::InitCell(const AstNode *pnode, AstNode *p)
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
			Sig.appendcell(*temp.Compute(p));
		if (pnode->str)
			SetVar(pnode->str, &Sig);
		return &Sig;
	}
	catch (const CAstException &e) {
		throw CAstExceptionInvalidUsage(*this, pnode, e.getErrMsg().c_str());
	}
}

void CAstSig::Transpose(const AstNode *pnode, AstNode *p)
{
	Compute(p);
	Sig.transpose();
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

CAstSig * CAstSig::SetVarwithIndex(const CSignal& indices, CVar *psig, CVar *pBase)
{ // to modify the content of an existing variable defined with pBase
   // indices.nSamples must be = psig->nSamples
	if (indices.nSamples != psig->nSamples)
		return nullptr;
	//No changes of nSamples of existing variable, pBase
	for (unsigned int k = 0; k < indices.nSamples; k++)
	{
		assert(indices.buf[k] >= 0.);
		assert((unsigned int)indices.buf[k] < pBase->nSamples);
		pBase->buf[(int)indices.buf[k]] = psig->buf[k];
	}
	return this;
}

CAstSig &CAstSig::SetVar(const char *name, CVar *psig, CVar *pBase)
// To do--chanage CVar *psig to const CVar &tsig and make sure the second arg is the const, target signal to use
//to do so, improve the case of psig->GetFs() == 3, so that psig is not changed, let's think about it how.
//11/6/2019
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

#if !defined(MAX_PATH)
#define MAX_PATH          260
#endif

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

bool CAstSig::Var_never_updated(const AstNode *p)
{//For these node types, 100% chance that var was not changed---do not send WM__VAR_CHANGED
	if (p->type == T_NUMBER) return true;
	if (p->type == T_STRING) return true;
	return false;
}

string CAstSig::ComputeString(const AstNode *p)
{
	return (p->type==T_STRING) ? p->str : Compute(p)->string();
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

vector<string> CAstSig::ClearVar(const char *var, CVar *psigBase)
{
	vector<string> out;
	vector<string> vars;
	str2vect(vars, var, " ");
	if (!psigBase)
	{
		if (var[0] == 0) // clear all
		{
			for (auto it = Vars.begin(); it != Vars.end(); it++)
				out.push_back((*it).first);
			Vars.erase(Vars.begin(), Vars.end());
			for (auto it = GOvars.begin(); it != GOvars.end(); it++)
				out.push_back((*it).first);
			GOvars.erase(GOvars.begin(), GOvars.end());
		}
		else
		{
			for (auto v : vars)
			{
				auto it = Vars.find(v);
				if (it != Vars.end())
				{
					out.push_back((*it).first);
					Vars.erase(it);
				}
				else
				{
					auto it = GOvars.find(v);
					if (it != GOvars.end()) 
					{
						out.push_back((*it).first);
						GOvars.erase(it);
					}
				}
			}
		}
	}
	else
	{
		if (var[0] == 0) // clear all
		{
			for (auto it = psigBase->strut.begin(); it != psigBase->strut.end(); it++)
				out.push_back((*it).first);
			psigBase->strut.erase(psigBase->strut.begin(), psigBase->strut.end());
			for (auto it = psigBase->struts.begin(); it != psigBase->struts.end(); it++)
				out.push_back((*it).first);
			psigBase->struts.erase(psigBase->struts.begin(), psigBase->struts.end());
		}
		else
		{
			for (auto v : vars)
			{
				auto it = psigBase->strut.find(v);
				if (it != psigBase->strut.end())
				{
					out.push_back((*it).first);
					psigBase->strut.erase(it);
				}
				else
				{
					auto it = psigBase->struts.find(v);
					if (it != psigBase->struts.end())
					{
						out.push_back((*it).first);
						psigBase->struts.erase(it);
					}
				}
			}
		}
	}
	return out;
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
	throw CAstInvalidFuncSyntax(*this, pnode, FuncSigs, msg.str().c_str());
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
		glovar = rhs.glovar;
	}
	return *this;
}



void goaction::clear()
{
	frozen = NULL; psig = NULL;  type = NULL; 	RHS.Reset();
}
