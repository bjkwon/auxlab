#pragma once
#ifndef SIGPROCINTERNAL
#define SIGPROCINTERNAL
#endif

enum AUX_PARSING_ERR
{
	UNKNOWN_NAME,
	UNKNOWN_GLOBAL_VARNAME,
	INDEX_OOR,
	CELL_INDEX_OOR,
	TREATING_CELL_WITH_PARENTH,
	RESERVED_AS_BUILT_IN,
};

class CAstExceptionInvalidUsage : public CAstException
{
public:
	CAstExceptionInvalidUsage(const CAstSig & scope, const AstNode *p0, const char * _basemsg, const char * tidname="", string extra = "")
	{
		pCtx = &scope;
		pnode = p0;
		msgonly = basemsg = _basemsg;
		msgonly += string(" ") + (tidstr = tidname);
		if (!extra.empty())
			msgonly += string("\n") + extra;
		arrayindex = cellindex = -1;
		if (msgonly.find("[GOTO_BASE]")==string::npos)		msgonly.insert(0, "[GOTO_BASE]");
		addLineCol();
		outstr = msgonly + sourceloc;
		if (scope.inTryCatch)
			findTryLine(scope);
	};
	
	virtual ~CAstExceptionInvalidUsage() 
	{
		clean();
	};
}; 

class CAstInvalidFuncSyntax : public CAstException
{
public:
	CAstInvalidFuncSyntax(const CAstSig & scope, const AstNode *p0, string fnsig, const char * _basemsg)
	{
		pCtx = &scope;
		pnode = p0;
		msgonly = basemsg = _basemsg;
		if (!fnsig.empty())
			msgonly += string("\n  Usage: ") + fnsig;
		arrayindex = cellindex = -1;
		if (msgonly.find("[GOTO_BASE]") == string::npos)		msgonly.insert(0, "[GOTO_BASE]");
		addLineCol();
		outstr = msgonly + sourceloc;
		if (scope.inTryCatch)
			findTryLine(scope);
	};

	virtual ~CAstInvalidFuncSyntax()
	{
		clean();
	};
};

class CAstExceptionRange : public CAstException
{
public:
	CAstExceptionRange(const CAstSig & scope, const AstNode *p0, const char * _basemsg, const char * varname, int indexSpecified, int indexSpecifiedCell=-1)
	{
		pCtx = &scope;
		pnode = p0;
		msgonly = basemsg = _basemsg;
		if (strlen(varname)>0)
			msgonly += (tidstr = varname);
		arrayindex = indexSpecified;
		cellindex = indexSpecifiedCell;
		if (msgonly.find("[GOTO_BASE]") == string::npos)		msgonly.insert(0, "[GOTO_BASE]");
		ostringstream oss;
		if (cellindex>=0) // if cell index is invalid, it doesn't check indexSpecified
			oss << msgonly << " cell index " << cellindex << " out of range.";
		else
			oss << msgonly << " index " << indexSpecified << " out of range.";
		msgonly = oss.str().c_str();
		addLineCol();
		outstr = msgonly + sourceloc;
		if (scope.inTryCatch)
			findTryLine(scope);
	};
	virtual ~CAstExceptionRange()
	{
		clean();
	};
};

class CAstExceptionInvalidArg : public CAstException
{
public:
	CAstExceptionInvalidArg(const CAstSig & scope, const AstNode *p0, const char * _basemsg, const char * funcname, int argIndex)
	{
		basemsg = _basemsg;
		ostringstream oss;
		oss << "" << funcname << " : " << "argument " << argIndex << ' ' << _basemsg;
		pCtx = &scope;
		pnode = p0;
		msgonly = oss.str().c_str();
		arrayindex = argIndex;
		cellindex = -1;
		if (msgonly.find("[GOTO_BASE]") == string::npos)		msgonly.insert(0, "[GOTO_BASE]");
		addLineCol();
		outstr = msgonly + sourceloc;
		if (scope.inTryCatch)
			findTryLine(scope);
	};
	virtual ~CAstExceptionInvalidArg()
	{
		clean();
	};
};

class CAstExceptionInternal : public CAstException
{
public:
	CAstExceptionInternal(const CAstSig & scope, const AstNode *p0, const char * _basemsg, int type=-1)
	{
		pCtx = &scope;
		pnode = p0;
		msgonly = basemsg = _basemsg;
		char buf[256];
		sprintf(buf, " node type=%d", type);
		msgonly += buf;
		arrayindex = cellindex = -1;
		if (msgonly.find("[GOTO_BASE]") == string::npos)		msgonly.insert(0, "[GOTO_BASE]");
		addLineCol();
		outstr = msgonly + sourceloc;
		outstr += str1;
		if (scope.inTryCatch)
			findTryLine(scope);
	};
	virtual ~CAstExceptionInternal()
	{
		clean();
	};
};

class CAstExceptionSyntax : public CAstException
{
public:
	CAstExceptionSyntax(const char * basemsg);
	virtual ~CAstExceptionSyntax() {};
};
