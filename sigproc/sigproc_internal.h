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
	CAstExceptionInvalidUsage(const CAstSig & scope, const AstNode *p0, const char * _basemsg, const char * tidname="", const char *extra = "")
	{
		pCtx = &scope;
		pnode = p0;
		str1 = basemsg = _basemsg;
		str1 += string(" ") + (tidstr = tidname);
		if (strlen(extra)>0)
			str1 += string(" ") + extra;
		arrayindex = cellindex = -1;
		str1.insert(0, "[GOTO_BASE]");
		outstr += str1;
		addLineCol();
		if (scope.inTryCatch)
			findTryLine(scope);
	};
	
	virtual ~CAstExceptionInvalidUsage() 
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
		str1 = basemsg = _basemsg;
		if (strlen(varname)>0)
			str1 += (tidstr = varname);
		arrayindex = indexSpecified;
		cellindex = indexSpecifiedCell;
		str1.insert(0, "[GOTO_BASE]");
		ostringstream oss;
		if (cellindex>=0) // if cell index is invalid, it doesn't check indexSpecified
			oss << str1 << " cell index " << cellindex << " out of range.";
		else
			oss << str1 << " index " << indexSpecified << " out of range.";
		outstr += oss.str().c_str();
		addLineCol();
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
		ostringstream oss;
		oss << "" << funcname << " : " << "argument " << argIndex << ' ' << _basemsg;
		pCtx = &scope;
		pnode = p0;
		str1 = oss.str().c_str();
		arrayindex = argIndex;
		cellindex = -1;
		str1.insert(0, "[GOTO_BASE]");
		outstr += str1;
		addLineCol();
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
		str1 = basemsg = _basemsg;
		char buf[256];
		sprintf(buf, " node type=%d", type);
		str1 += buf;
		arrayindex = cellindex = -1;
		str1.insert(0, "[GOTO_BASE]");
		outstr += str1;
		addLineCol();
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
