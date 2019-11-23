// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.503
// Date: 6/4/2019
// 
#pragma once

#ifndef SIGPROC
#define SIGPROC

#ifdef DYNAMIC_LINK
#define CVAR_EXP __declspec (dllexport)
#else
#define CVAR_EXP
#endif //DYNAMIC_LINK

#define PI	3.141592
#define LOG10DIV20 0.1151292546497
//#define round(x)	((int)((x)+((x)<0?-.5:.5)))

#define CSIG_EMPTY		0
#define CSIG_STRING		1
#define CSIG_NULL		2
#define CSIG_SCALAR		4
#define CSIG_CELL		8
#define CSIG_STRUCT		10
#define CSIG_HDLARRAY	12
#define CSIG_HANDLE		15
#define CSIG_VECTOR		17
#define CSIG_MATRIX		19
#define CSIG_AUDIO		65
#define CSIG_TSERIES	81 // Think about this... improve it... 5/24/2018

// Used for communication bet sigproc and xcom (i.e., AstSig.cpp and showvar.cpp)
#define WM__DEBUG		WM_APP+4000 
#define ID_DEFAULT	99
#define ID_DEBUG_STEP 108
#define ID_DEBUG_STEPIN 109
#define ID_DEBUG_CONTINUE 110
#define ID_DEBUG_EXIT 111
#define ID_DEBUG 112
#define LEN_CALLBACKIDENTIFIER 300

enum DEBUG_STATUS
{
    null=-1,
    entering,
    progress,
	stepping,
	stepping_in,
	continuing,
    exiting,
    cleanup,
	aborting,
	purgatory,
	refresh,
	typed_line,
};

//End of Used for communication bet sigproc and xcom (i.e., AstSig.cpp and showvar.cpp)

#ifdef _WINDOWS
#include <windows.h>
#endif // _WINDOWS

#if defined(_WIN64) 
typedef __int64 INT_PTR;
#else 
typedef int INT_PTR;
#endif // _WIN64

#include <string>
#include <iostream>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <complex>

#ifndef PSYCONYACC
#include "psycon.yacc.h"
#endif

using namespace std;

class CAstSig;
class CNodeProbe;

#include "aux_classes.h"

class CAstException {
public:
	const AstNode *pnode;
	CAstSig *pCtx; // pointer to the context, AKA AstSig, that threw the exception
	string str1, str2, outstr;
	CAstException(const AstNode *p, CAstSig *pAst, const string s1);
	CAstException(const AstNode *p, CAstSig *pAst, const string s1, const string s2);
	CAstException(CAstSig *pContext, const string s1, const string s2);
	CAstException(const AstNode *p0, CAstSig *past, const char* msg);
	string getErrMsg() const {return outstr;};
private:
	void makeOutStr();
};

class UDF
{
public:
	AstNode *pAst;
	string fullname;
	string content;
	vector<int> DebugBreaks;
	map<string, UDF> local;
	bool newrecruit;
	UDF& operator=(const UDF& rhs);
	UDF() {	pAst = NULL; newrecruit = false;	};
	virtual ~UDF() {};
};

class Cfunction
{
public:
	Cfunction() { alwaysstatic = opaque = false; };
	virtual ~Cfunction() {};
	string funcsignature;
	bool alwaysstatic; // true means the function cannot be called as a member function
	bool opaque;
	int narg1, narg2;
	void(*func)(CAstSig *, const AstNode *, const AstNode *, string &);
};
class CAstSigEnv
{
public:
	static string AppPath;
	static map<string, vector<CVar *>> glovar;
	static map<string, Cfunction> pseudo_vars;
	map<string, UDF> udf;
	int Fs;
	int curLine; // used for control F10
	string AuxPath;
	bool shutdown;
	void InitBuiltInFunctions(HWND h);
	void InitBuiltInFunctionsExt(const char *dllname);
	map<string, Cfunction> builtin;

	CAstSigEnv(const int fs = 1);
	virtual ~CAstSigEnv();
	CAstSigEnv& operator=(const CAstSigEnv& rhs);
	CAstSigEnv &SetPath(const char *path);
	CAstSigEnv &AddPath(const char *path);
};

class CDebugStatus
{
public:
	DEBUG_STATUS status;
	bool GUI_running;
	bool inPurgatory;
	bool local;
	CDebugStatus() { status = null;  GUI_running = inPurgatory = local = false; };
	virtual ~CDebugStatus() {};
};

class goaction {
public:
	CVar *psig;
	const char *type;
	CVar RHS;
	bool frozen;
	void clear();
	goaction() { frozen = NULL;  psig = NULL;  type = NULL; };
	virtual ~goaction() {};
} ;

class CFuncPointers
{
public:
	void(*UpdateDebuggerGUI)(CAstSig *debugAstSig, DEBUG_STATUS debug_status, int line); // debug_appl_manager in xcom
	void(*HoldAtBreakPoint)(CAstSig *pastsig, const AstNode *pnode);
	bool(*IsCurrentUDFOnDebuggerDeck)(const char*);
	void(*ShowVariables)(CAstSig *pcast);
	void(*Back2BaseScope)(int);
	void(*ValidateFig)(const char*);
	void(*SetGoProperties)(CAstSig *past, const char *type, const CVar & RHS);
	void(*RepaintGO)(CAstSig *pctx);
	CFuncPointers();
	virtual ~CFuncPointers() {};
	CFuncPointers& operator=(const CFuncPointers& rhs);
};

class CUDF
{
public:
	string title;
	string base;
	AstNode *pUDF; // T_FUNCTION node
	AstNode *pUDF_base; // T_FUNCTION node for base udf
	vector<string> argout; // formal output argument list; to be filled out in PrepareAndCallUDF()
	int nextBreakPoint;
	int currentLine;
	int nargin, nargout;
	map<string, CVar*> static_vars;
	CDebugStatus debug;
	map<HWND, RECT> rt2validate;
	const char* application;
	bool repaint;
	CUDF() {	application = nullptr;  nextBreakPoint = currentLine = -1;		CallbackCIPulse = NULL; CallbackHook = NULL; pLastRead = NULL; repaint = false;	};
	virtual ~CUDF() {};
	void(*CallbackCIPulse)(const AstNode *, CAstSig *);
	int(*CallbackHook)(CAstSig *past, const AstNode *pnode, const AstNode *p);
	AstNode *pLastRead; //used for isthisUDFscope only, to mark the last pnode processed in 
};

class CAstSig
{
	friend class CNodeProbe;
public:
#ifndef GRAFFY
	static void cleanup_nodes(CAstSig *beginhere = NULL);
	static double play_block_ms;
	static double record_block_ms;
	static short play_bytes; // waveform format 1, 2 or 3 bytes for 8, 16 or 24 bits
	static short record_bytes; // waveform format 1, 2 or 3 bytes for 8, 16 or 24 bits
	static bool IsStatement(const AstNode *p);
	static bool IsCondition(const AstNode *p);
	static bool IsLooping(const AstNode *p);
	static bool IsTID(const AstNode *p);
	static bool IsBLOCK(const AstNode *p);
	static bool IsVECTOR(const AstNode *p);
	static bool IsSTRUCT(const AstNode *p);
	static bool IsCELL_STRUCT_pnode_TID_ARGS(const AstNode *pnode, const AstNode *p);
	static bool Var_never_updated(const AstNode *p);
	static AstNode *goto_line(const AstNode *pnode, int line);
	static CVar *HandleSig(CVar *ptarget, CVar *pGraffyobj);
	static AstNode *findParentNode(AstNode *p, AstNode *pME, bool altonly = false);
	static char *showGraffyHandle(char *out, CVar *pvar);
#endif //GRAFFY
	map<string, CVar> Vars;
	map<string, vector<CVar *>> GOvars;
	AstNode *pAst;
	CVar Sig;
	CVar *pgo;
	string Script;
	CAstSigEnv *pEnv;
	CUDF u;
	CVar *GetGOVariable(const char *varname, CVar *pvar = NULL);
	CVar *GetGloGOVariable(const char *varname, CVar *pvar = NULL);
	CVar *MakeGOContainer(vector<CVar *> GOs);
	CVar *MakeGOContainer(vector<INT_PTR> GOs);
	vector<CSignals> outarg;
	AstNode *lhs;
	CAstSig *son;
	CAstSig *dad;
	const AstNode *pLast;
	string statusMsg; // to display any message during processing inside of AstSig.cpp in the application
	unsigned long Tick0, Tick1;
	bool fBreak, fAllocatedAst;
	bool FsFixed;
	CFuncPointers fpmsg; // function pointer "messenger"
	void init();
	static const int DefaultFs = 22050;

	CVar replica;
	double endpoint;
	bool fExit, fContinue;
	char callbackIdentifer[LEN_CALLBACKIDENTIFIER];
	goaction setgo;

private:
	void HandleAuxFunctions(const AstNode *pnode, AstNode *pRoot = NULL);
	bool HandlePseudoVar(const AstNode *pnode);
	int HandleMathFunc(bool compl, string &fname, double(**)(double), double(**)(double), double(**)(double, double), double(**)(complex<double>), complex<double>(**)(complex<double>), complex<double>(**)(complex<double>, complex<double>));
	bool IsValidBuiltin(string funcname);
	void checkindexrange(const AstNode *pnode, CTimeSeries *inout, unsigned int id, string errstr);
	bool isContiguous(body &id, unsigned int &begin,unsigned int &end);
	AstNode *searchtree(const AstNode *pTarget, AstNode *pStart);
	AstNode *searchtree(AstNode *pp, int type);
	bool checkcond(const AstNode *p);
	void hold_at_break_point(const AstNode *pnode);
	void prepare_endpoint(const AstNode *p, CVar *pvar);
	bool builtin_func_call(CNodeProbe &diggy, AstNode *p);
	void Concatenate(const AstNode *pnode, AstNode *p);
	AstNode *read_node(CNodeProbe &diggy, AstNode *pn, AstNode *pPrev, bool &RHSpresent);
	AstNode *read_nodes(CNodeProbe &diggy, bool bRHS = false);
	void interweave_indices(CVar &isig, CVar &isig2, unsigned int len);
	void index_array_satisfying_condition(CVar &isig);
	void replica_prep(CVar *psig);
	void outputbinding(const AstNode *pCalling, size_t nArgout);
	CVar *eval_RHS(AstNode *pnode);
	inline void throw_LHS_lvalue(const AstNode *pn, bool udf);
	CVar * NodeVector(const AstNode *pnode);
	CVar * NodeMatrix(const AstNode *pnode);
	CVar * Dot(AstNode *p);
	void bind_psig(AstNode *pn, CVar *tsig);

public:
	string ExcecuteCallback(const AstNode *pCalling, CVar *pStaticVars, vector<CVar *> &pOutVars);
	string adjustfs(int newfs);
	CVar * ConditionalOperation(const AstNode *pnode, AstNode *p);
	void ClearVar(AstNode *pnode, CVar *psig);
	int updateGO(CVar &ref);
	CVar * pseudoVar(const AstNode *pnode, AstNode *p, CSignals *pout = NULL);
	CVar * TSeq(const AstNode *pnode, AstNode *p);
	bool isThisAllowedPropGO(CVar *psig, const char *type, const CVar &tsig);
	void astsig_init(void(*fp1)(CAstSig *, DEBUG_STATUS, int), void(*fp2)(CAstSig *, const AstNode *), bool(*fp3)(const char *), void(*fp4)(CAstSig *), void(*fp5)(int), void(*fp6a)(const char*), void(*fp7)(CAstSig *, const char *, const CVar &), void(*fp8)(CAstSig *));
	bool IsThisBreakpoint(const AstNode *pnode);
	void checkAudioSig(const AstNode *pnode, CVar &checkthis, string addmsg = "");
	void checkTSeq(const AstNode *pnode, CVar &checkthis, string addmsg="");
	void checkComplex (const AstNode *pnode, CVar &checkthis);
	void checkVector(const AstNode *pnode, CVar &checkthis, string addmsg = "");
	void checkSignal(const AstNode *pnode, CVar &checkthis, string addmsg="");
	void checkScalar(const AstNode *pnode, CVar &checkthis, string addmsg = "");
	void checkString(const AstNode *pnode, CVar &checkthis, string addmsg="");
	void blockCell(const AstNode *pnode, CVar &checkthis);
	void blockScalar(const AstNode *pnode, CVar &checkthis);
	void blockString(const AstNode *pnode, CVar &checkthis);
	void blockComplex(const AstNode *pnode, CVar &checkthis);
	const AstNode *getparentnode(const AstNode *pnode, const AstNode *p);
	bool need2repaintnow(const AstNode *pnode, AstNode *p = NULL);
	bool GOpresent(const AstNode *pnode, AstNode *p = NULL);
	bool isthisUDFscope(const AstNode *pnode, AstNode *p=NULL);
	bool PrepareAndCallUDF(const AstNode *pnode, CVar *pBase, CVar *pStaticVars=NULL);
	size_t CallUDF(const AstNode *pnode4UDFcalled, CVar *pBase);
#ifdef _WINDOWS
	string LoadPrivateUDF(HMODULE h, int id, string &emsg);
#endif
	CAstException ExceptionMsg(const AstNode *pnode, const string s1, const string s2);
	CAstException ExceptionMsg(const AstNode *pnode, const char *msg);

    CAstSig(const CAstSig &org);
	CAstSig(const CAstSig *src);
	CAstSig(const char *str, const CAstSig *src);
	CAstSig(AstNode *pNode, const CAstSig *src);
	CAstSig(CAstSigEnv *env);
	CAstSig(const char *str, CAstSigEnv *env);
	CAstSig(AstNode *pNode, CAstSigEnv *env);
	~CAstSig();

	AstNode *SetNewScript(string &emsg, const char *str, const char *premsg = NULL);
	AstNode *SetNewScriptFromFile(string &emsg, const char *full_filename, const char *str, string &filecontent);
	vector<CVar*> Compute(void);
	CVar * Compute(const AstNode *pnode);
	CVar * InitCell(const AstNode *pnode, AstNode *p);
	CVar * SetLevel(const AstNode *pnode, AstNode *p);
	void define_new_variable(const AstNode *pnode, AstNode *pRHS, const char *fullvarname);
	CVar *GetGlobalVariable(const AstNode *pnode, const char *varname, CVar *pvar = NULL);
	CVar *GetVariable(const char *varname, CVar *pvar = NULL);
	CVar * TID(AstNode *pnode, AstNode *p, CVar *psig=NULL);
	CVar * Eval(AstNode *pnode);
	void Transpose(const AstNode *pnode, AstNode *p);
	CAstSig &Reset(const int fs = 0, const char* path=NULL);
	CAstSig &SetVar(const char *name, CVar *psig, CVar *pBase = NULL);
	CAstSig * SetVarwithIndex(const CSignal& indices, CVar *psig, CVar *pBase);
	CAstSig &SetGloVar(const char *name, CVar *psig, CVar *pBase = NULL);
	const char *GetPath() {return pEnv->AuxPath.c_str();}
	int GetFs(void) {return pEnv->Fs;}
	string ComputeString(const AstNode *p) ;
	string GetScript() {return Script;}
	void set_interrupt(bool ch);
	bool isInterrupted(void);
	vector<double> gettimepoints(const AstNode *pnode, AstNode *p);
	unsigned long tic();
	unsigned long toc();

	int isthislocaludf(void);

	int ClearVar(const char *var, CVar *psigBase=NULL);
	void EnumVar(vector<string> &var);
	CVar *GetSig(const char *var);

	string MakeFilename(string fname, const string ext);
	FILE *OpenFileInPath(string fname, string ext, string &fullfilename);
	AstNode *ReadUDF(string &emsg, const char *udf_filename, const char *internaltransport=NULL);
	AstNode *RegisterUDF(const AstNode *pnode, const char *udf_filename, string &filecontent);
	int checkNumArgs(const AstNode *pnode, const AstNode *p, std::string &FuncSigs, int minArgs, int maxArgs);
	int checkNumArgs(const AstNode *pnode, const AstNode *p, std::string &FuncSigs, int *args);
};

class CNodeProbe
{
	friend class CAstSig;
public:
	CNodeProbe(CAstSig *past, AstNode *pnode, CVar *psig);
	virtual ~CNodeProbe() {};
	CAstSig *pbase;
	AstNode *root;
	CVar *psigBase;
	string varname; // tracks the "full" name of variable including the index, the dot or { }, etc.
	char status[8]; // limit to 7 characters

	CVar * TID_condition(const AstNode *pnode, AstNode *pLHS, AstNode *pRHS, CVar *psig);
	CVar * TID_RHS2LHS(const AstNode *pnode, AstNode *p, AstNode *pRHS, CVar *psig);
	CVar &ExtractByIndex(const AstNode *pnode, AstNode *p);
	CVar &eval_indexing(const AstNode *pInd, CVar &indSig);
	CVar * TID_indexing(AstNode *p, AstNode *pRHS, CVar *psig);
	CVar * TID_tag(const AstNode *pnode, AstNode *p, AstNode *pRHS, CVar *psig);
	CVar * extract(const AstNode *pnode, CTimeSeries &isig);
	CVar * TID_time_extract(const AstNode *pnode, AstNode *p, AstNode *pRHS);
	CVar * TimeExtract(const AstNode *pnode, AstNode *p);
	void insertreplace(const AstNode *pnode, CVar &sec, CVar &indsig);
	CTimeSeries &replace(const AstNode *pnode, CTimeSeries *pobj, body &sec, int id1, int id2);
	CTimeSeries &replace(const AstNode *pnode, CTimeSeries *pobj, body &sec, body &index);
	CAstException ExceptionMsg(const AstNode *pnode, const string s1, const string s2);
	CAstException ExceptionMsg(const AstNode *pnode, const char *msg);
};



#endif // SIGPRO;C