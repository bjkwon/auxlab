// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: auxlab
// Main Application. Based on Windows API  
// 
// 
// Version: 1.495
// Date: 12/13/2018
// 

#ifndef XCOM
#define XCOM

#define MAIN_PROMPT "AUX> "
#define DEBUG_PROMPT "k> "

enum DEBUG_KEY
{
	non_debug=0,
    debug_F5,
    debug_F10,
    debug_F11,
    debug_Shift_F5,
    debug_Ctrl_F10,
};

class cfigdlg
{
public:
	cfigdlg() { scope = "base workspace"; };
	cfigdlg(const char *scopename = NULL) { if (!scopename || !strlen(scopename)) scope = "base workspace"; else scope = scopename; };
	virtual ~cfigdlg() {};
	DWORD threadID;
	HWND hDlg;
	string var;
	string scope;
	CAstSig *pcast;
};

class xcom
{
public:
	vector<string> history;
	vector<string> LoadedModule;
	size_t nHistFromFile;
	size_t comid; // command ID
	char AppPath[256];
	char AppVersion[16];
	INPUT_RECORD debug_command_frame[6];
	string comPrompt;
	bool need2validate;

	xcom();
	virtual ~xcom();
	void console();
	void checkdebugkey(INPUT_RECORD *in, int len);
	DEBUG_KEY getinput(char* readbuffer);
	size_t ReadHist();
	void ShowWS_CommandPrompt(CAstSig *pcast, bool success=true);
	bool IsNowDebugging(CAstSig *pcast);
	int computeandshow(const char *input, CAstSig *pTemp=NULL);
	int cleanup_debug();
	void echo(const char *var, CVar *pvar, int offset = 1, const char *postscript = "");
	void echo(CAstSig *pctx, const AstNode *pnode, CVar *pvar=NULL);
	void setfs(CAstSig *past, int newfs);
	int ClearVar(const char *var);
	int load_axl(FILE *fp, char *errstr);

	size_t ctrlshiftright(const char *buf, DWORD offset);
	size_t ctrlshiftleft(const char *buf, DWORD offset);
	int hook(CAstSig *ast, string HookName, const char* args);
	int SAVE_axl(CAstSig *ast, const char* filename, vector<string> varlist, char *errstr);
	void LogHistory(vector<string> input);

	static ostringstream outstream_tmarks(CTimeSeries *psig, bool unit);
	static ostringstream outstream_tseq(CTimeSeries *psig, bool unit);
	static ostringstream outstream_vector(CSignal*pvar, unsigned int id0, int offset);
	static ostringstream outstream_value(double val, int offset);


private:
	int read_axl_block(FILE *fp, string &varname, CVar *pout, char *errstr, bool independent = true);
	int write_axl_block(FILE *fp, string varname, CVar *pout, char *errstr, bool independent = true);
};


extern HWND hLog;
#define WM__LOG	WM_APP+0x2020
#define WM__RCI	WM_APP+0x2022


#define PRINTLOG(FNAME,STR) \
{ FILE*__fp=fopen(FNAME,"at"); fprintf(__fp,STR);	fclose(__fp); }

#else if //if XCOM was already defined, skip


#endif