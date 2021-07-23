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
	null = -1,
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

enum EXCEPTIONTYPE
{
	USAGE = 1,
	FUNC_SYNTAX,
	RANGE,
	ARGS,
	INTERNAL,
	ERR_POST_FNC,
	UNDEFINED_TID,
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
#include <algorithm>

#ifndef PSYCONYACC
#include "psycon.yacc.h"
#endif

using namespace std;

class CAstSig;
class CNodeProbe;

#include "aux_classes.h"

class UDF
{
public:
	AstNode *uxtree; // the syntax tree for the content of the UDF; begining with T_FUNCTION
	string fullname;
	string content;
	vector<int> DebugBreaks;
	map<string, UDF> local;
	bool newrecruit;
	UDF& operator=(const UDF& rhs);
	UDF() { uxtree = NULL; newrecruit = false;	};
	multimap<CVar, AstNode *> switch_case; // why multimap instead of map? Because the key for case may appear in multiple switch blocks
	vector<int> switch_case_undefined;
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
	void(*func)(CAstSig *, const AstNode *);
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
	vector<string> AuxPath;
	bool shutdown;
	void InitBuiltInFunctions();
	void InitErrorCodes();
	string path_delimited_semicolon();
	vector<string> InitBuiltInFunctionsExt(const vector<string>& externalModules);
	map<string, Cfunction> builtin;

	CAstSigEnv(const int fs = 1);
	virtual ~CAstSigEnv();
	CAstSigEnv& operator=(const CAstSigEnv& rhs);
	void AddPath(string path);
	AstNode* checkout_udf(const string& udf_filename, const string& filecontent);
	AstNode* checkin_udf(const string& udf_filename, const string& fullpath, const string& filecontent, string& emsg);
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
	AstNode *t_func; // T_FUNCTION node
	AstNode *t_func_base; // T_FUNCTION node for base udf
	vector<string> argout; // formal output argument list; to be filled out in PrepareAndCallUDF()
	int nextBreakPoint;
	int currentLine;
	int nargin, nargout;
	map<string, CVar*> static_vars;
	CDebugStatus debug;
	map<HWND, RECT> rt2validate;
	string application;
	bool repaint;
	CUDF() {	nextBreakPoint = currentLine = -1; pLastRead = NULL; repaint = false;	};
	virtual ~CUDF() {};
	AstNode *pLastRead; //used for isthisUDFscope only, to mark the last pnode processed in 
};

class CAstException;

class CAstSig
{
	friend class CNodeProbe;
public:
#ifndef GRAFFY
	static double play_block_ms;
	static double record_block_ms;
	static short play_bytes; // waveform format 1, 2 or 3 bytes for 8, 16 or 24 bits
	static short record_bytes; // waveform format 1, 2 or 3 bytes for 8, 16 or 24 bits
	static bool IsStatement(const AstNode *p);
	static bool IsConditional(const AstNode *p);
	static bool IsLooping(const AstNode *p);
	static bool IsTID(const AstNode *p);
	static bool IsBLOCK(const AstNode *p);
	static bool IsVECTOR(const AstNode *p);
	static bool IsSTRUCT(const AstNode *p);
	static bool IsCELL_STRUCT_pnode_TID_ARGS(const AstNode *pnode, const AstNode *p);
	static bool Var_never_updated(const AstNode *p);
	static AstNode *goto_line(const AstNode *pnode, int line);
	static CVar *HandleSig(CVar *ptarget, CVar *pGraffyobj);
	static const AstNode* findParentNode(const AstNode* p, const AstNode* pME, bool altonly = false);
	static const AstNode* findDadNode(const AstNode *p, const AstNode *pME);
	static char *showGraffyHandle(char *out, CVar *pvar);
#endif //GRAFFY
	int level;
	vector<int> baselevel;
	map<string, CVar> Vars;
	map<string, vector<CVar *>> GOvars;
	AstNode *xtree; // syntax tree; where an aux-statement is parsed and broken down to a tree of nodes containing imparsible units
	CVar Sig;
	CVar *pgo; // pointer, not a copy, of the last computed object; used for graffy functions
	string Script;
	CAstSigEnv *pEnv;
	CUDF u;
	CVar *GetGOVariable(const char *varname, CVar *pvar = NULL);
	CVar *MakeGOContainer(vector<CVar *> GOs);
	CVar *MakeGOContainer(vector<INT_PTR> GOs);
	vector<unique_ptr<CVar*>> Sigs; // used to store results of Compute for additional outputs.
	AstNode *lhs;
	unique_ptr<CAstSig> son;
	CAstSig *dad;
	const AstNode *pLast;
	string statusMsg; // to display any message during processing inside of AstSig.cpp in the application
	unsigned long Tick0, Tick1;
	bool fBreak, fAllocatedAst;
	bool FsFixed;
	bool wait4cv;
	CFuncPointers fpmsg; // function pointer "messenger"
	void init();
	static const int DefaultFs = 22050;
	int inTryCatch;
	CVar replica;
	double endpoint;
	bool fExit, fContinue;
	char callbackIdentifer[LEN_CALLBACKIDENTIFIER];
	goaction setgo;

private:
	int HandleMathFunc(bool , string& fname, double(**)(double), double(**)(double), double(**)(double, double), double(**)(complex<double>), complex<double>(**)(complex<double>), complex<double>(**)(complex<double>, complex<double>));
	void HandleAuxFunctions(const AstNode *pnode, AstNode *pRoot = NULL);
	bool HandlePseudoVar(const AstNode *pnode);
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
	CVar * getchannel(CVar *pin, const AstNode *pnode, const AstNode* ppar);
	void interweave_indices(CVar &isig, CVar &isig2, unsigned int len);
	void index_array_satisfying_condition(CVar &isig);
	void replica_prep(CVar *psig);
	void outputbinding(const AstNode *pCalling, size_t nArgout);
	CVar *eval_RHS(AstNode *pnode);
	inline void throw_LHS_lvalue(const AstNode *pn, bool udf);
	CVar * NodeVector(const AstNode *pnode);
	CVar * NodeVectorGO(CVar *psig, const AstNode *pnode);
	CVar * NodeMatrix(const AstNode *pnode);
	CVar * Dot(AstNode *p);
	void switch_case_handler(const AstNode *pnode);
	size_t udfcompx(AstNode* pfirst);

public:
	string ExcecuteCallback(const AstNode *pCalling, vector<unique_ptr<CVar*>> &inVars, vector<unique_ptr<CVar*>> &outVars, bool defaultcallback);
	string adjustfs(int newfs);
	CVar * ConditionalOperation(const AstNode *pnode, AstNode *p);
	void ClearVar(AstNode *pnode, CVar *psig);
	int updateGO(CVar &ref);
	CVar * pseudoVar(const AstNode *pnode, AstNode *p, CSignals *pout = NULL);
	CVar * TSeq(const AstNode *pnode, AstNode *p);
	void astsig_init(void(*fp1)(CAstSig *, DEBUG_STATUS, int), void(*fp2)(CAstSig *, const AstNode *), bool(*fp3)(const char *), void(*fp4)(CAstSig *), void(*fp5)(int), void(*fp6a)(const char*), void(*fp7)(CAstSig *, const char *, const CVar &), void(*fp8)(CAstSig *));
	bool IsThisBreakpoint(const AstNode *pnode);
	void checkAudioSig(const AstNode *pnode, const CVar &checkthis, string addmsg = "");
	void checkTSeq(const AstNode *pnode, const CVar &checkthis, string addmsg="");
	void checkComplex (const AstNode *pnode, CVar &checkthis);
	void checkVector(const AstNode *pnode, const CVar &checkthis, string addmsg = "");
	void checkSignal(const AstNode *pnode, const CVar &checkthis, string addmsg="");
	void checkScalar(const AstNode *pnode, const CVar &checkthis, string addmsg = "");
	void checkString(const AstNode *pnode, const CVar &checkthis, string addmsg="");
	void blockCell(const AstNode *pnode, const CVar &checkthis, string addmsg = "");
	bool blockCell_allowGO(const AstNode *pnode, const CVar &checkthis, string addmsg = "");
	void blockEmpty(const AstNode *pnode, const CVar &checkthis, string addmsg = "");
	void blockScalar(const AstNode *pnode, const CVar &checkthis, string addmsg = "");
	void blockTemporal(const AstNode *pnode, const CVar &checkthis, string addmsg = "");
	void blockString(const AstNode* pnode, const CVar& checkthis, string addmsg = "");
	void blockLogical(const AstNode *pnode, const CVar &checkthis, string addmsg = "");
	void blockComplex(const AstNode *pnode, const CVar &checkthis, string addmsg = "");
	const AstNode *getparentnode(const AstNode *pnode, const AstNode *p);
	bool need2repaintnow(const AstNode *pnode, AstNode *p = NULL, bool udfdone = true);
	bool GOpresent(const AstNode *pnode, AstNode *p = NULL);
	bool isthisUDFscope(const AstNode *pnode, AstNode *p = NULL);
	bool PrepareAndCallUDF(const AstNode *pnode, CVar *pBase, CVar *pStaticVars=NULL);
	size_t CallUDF(const AstNode *pnode4UDFcalled, CVar *pBase);
	void outputbinding(const AstNode *plhs);
	void bind_psig(AstNode *pn, CVar *tsig);
	vector<string> erase_GO(const char* varname);
	vector<string> erase_GO(CVar* obj);
	vector<CVar*> get_GO_children(const vector<CVar*>& obj);
	CVar* Try_here(const AstNode* pnode, AstNode* p);

#ifdef _WINDOWS
	string LoadPrivateUDF(HMODULE h, int id, string &emsg);
#endif
	CAstException ExceptionMsg(const AstNode *pnode, const string s1, const string s2);
	CAstException ExceptionMsg(const AstNode *pnode, const char *msg);

    CAstSig(const CAstSig &org);
	CAstSig(const CAstSig *src);
	CAstSig(const char *str, const CAstSig *src);
	CAstSig(CAstSigEnv *env);
	~CAstSig();

	AstNode * parse_aux(const char* str, string& emsg, char* str_autocorrect = NULL);
	vector<CVar*> Compute(void);
	CVar * Compute(const AstNode *pnode);
	CVar * InitCell(const AstNode *pnode, AstNode *p);
	CVar * SetLevel(const AstNode *pnode, AstNode *p);
	void define_new_variable(const AstNode *pnode, AstNode *pRHS, const char *fullvarname);
	CVar * GetGlobalVariable(const AstNode *pnode, const char *varname, CVar *pvar = NULL);
	CVar * GetVariable(const char *varname, CVar *pvar = NULL);
	CVar * TID(AstNode *pnode, AstNode *p, CVar *psig=NULL);
	void Transpose(const AstNode *pnode, AstNode *p);
	CAstSig & Reset(const int fs = 0);// , const char* path = NULL);
	CAstSig & SetVar(const char *name, CVar *psig, CVar *pBase = NULL);
	CAstSig * SetVarwithIndex(const CSignal& indices, CVar *psig, CVar *pBase);
	CAstSig & SetGloVar(const char *name, CVar *psig, CVar *pBase = NULL);
	int GetFs(void) {return pEnv->Fs;}
	string ComputeString(const AstNode *p) ;
	string GetScript() {return Script;}
	void set_interrupt(bool ch);
	bool isInterrupted(void);
	vector<double> gettimepoints(const AstNode *pnode, AstNode *p);
	unsigned long tic();
	unsigned long toc(const AstNode *p);

	int isthislocaludf(void);

	vector<string> ClearVar(const char *var, CVar *psigBase = NULL);
	void EnumVar(vector<string> &var);
	CVar *GetSig(const char *var);

	string makefullfile(const string &fname, char *extension = NULL);
	FILE * fopen_from_path(string fname, string ext, string &fullfilename);
	AstNode * ReadUDF(string &emsg, const char *udf_filename);
	multimap<CVar, AstNode *> register_switch_cvars(const AstNode *pnode, vector<int> &undefined);
	AstNode * RegisterUDF(const AstNode *pnode, const char *udf_filename, const string &filecontent);
	int checkNumArgs(const AstNode *pnode, const AstNode *p, std::string &FuncSigs, int minArgs, int maxArgs);
	int checkNumArgs(const AstNode *pnode, const AstNode *p, std::string &FuncSigs, int *args);
};

class CAstException
{
public:
	CAstException(EXCEPTIONTYPE extp, const CAstSig &base, const AstNode *pnode);
	CAstException &proc(const char * _basemsg, const char * tidname = "", string extra = ""); //FUNC_SYNTAX; INTERNAL; invalid usage
	CAstException& proc(const string &tidstr, const string &errmsg); //Error post func
	CAstException &proc(const char * _basemsg, const char * varname, int indexSpecified, int indexSpecifiedCell); //range
	CAstException &proc(const char * _basemsg, const char * funcname, int argIndex); // arg
	CAstException() {
		pTarget = nullptr;	pnode = nullptr;
	};
	~CAstException() {};

//private:
	EXCEPTIONTYPE type;
	const AstNode *pnode;
	const CAstSig *pCtx; // pointer to the context, AKA AstSig, that threw the exception
	int line, col;
	string str1, str2;
	void findTryLine(const CAstSig & scope);
	string getErrMsg() const { return outstr; };
	string basemsg, tidstr;
	string msgonly; // including basemsg, tidstr, and extra
	string sourceloc; // source location; where the error occurred (line, col and file)
	string outstr; // msgonly \n sourceloc
	int arrayindex, cellindex;
	const AstNode *pTarget;
protected:
	void addLineCol();
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
	double *lhsref; // actual double buffer location that the lhs is supposed to modify
	bool lhsref_single; // true if lhs indicates a scalar--this guarantees a valid lhsref.
	string varname; // tracks the "full" name of variable including the index, the dot or { }, etc.
	char status[8]; // limit to 7 characters

	CVar * TID_condition(const AstNode *pnode, AstNode *pLHS, AstNode *pRHS);
	CVar * TID_RHS2LHS(const AstNode *pnode, AstNode *p, AstNode *pRHS);
	CVar &ExtractByIndex(const AstNode *pnode, AstNode *p);
	CVar &eval_indexing(const AstNode *pInd, CVar &indSig);
	CVar * TID_indexing(const AstNode* pnode, AstNode *p, AstNode *pRHS);
	CVar * TID_assign(const AstNode *pnode, AstNode *p, AstNode *pRHS);
	CVar * extract(const AstNode *pnode, CTimeSeries &isig);
	CVar * TID_time_extract(const AstNode *pnode, AstNode *p, AstNode *pRHS);
	CVar * TimeExtract(const AstNode *pnode, AstNode *p);
	void insertreplace(const AstNode *pnode, CVar &sec, CVar &indsig);
	CTimeSeries &replace(const AstNode *pnode, CTimeSeries *pobj, CSignal &sec, int id1, int id2);
	CTimeSeries &replace(const AstNode *pnode, CTimeSeries *pobj, body &sec, body &index);
	//CAstException ExceptionMsg(const AstNode *pnode, const string s1, const string s2);
	//CAstException ExceptionMsg(const AstNode *pnode, const char *msg);
	void tree_NARGS(const AstNode* pnode, AstNode* ppar);
	CVar* cell_indexing(CVar* pBase, AstNode* pn);
};

const AstNode* get_first_arg(const AstNode* pnode, bool staticfunc = false);


#endif // SIGPRO;C