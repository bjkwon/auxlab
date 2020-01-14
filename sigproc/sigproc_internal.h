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

class CAstExceptionUnknownTID : public CAstException
{
public:
	CAstExceptionUnknownTID(const CAstSig & scope, const char * _basemsg, const char * tidname, const char *extra = "")
	{
		pTarget = nullptr;
		pCtx = &scope;
		pnode = scope.pAst;
		str1 = basemsg = _basemsg;
		str1 += (tidstr = tidname);
		arrayindex = cellindex = -1;
		str1.insert(0, "[GOTO_BASE]");
		outstr += str1;
		makeOutStr();
		if (scope.inTryCatch)
		{
			//go upstream from pLast until T_TRY is found
			//then update pLast with the catch node
			//begin with scope.pAst which is pnode
			for (const AstNode* p = pnode; p; p = p->next)
			{
				if (p->type == T_TRY) pTarget = p->alt;
				if (p == scope.pLast)
				{
					break;
				}
			}
		}
	};
	virtual ~CAstExceptionUnknownTID() {};
}; 

class CAstExceptionRange : public CAstException
{
public:
	CAstExceptionRange(const char * basemsg, const char * varname, int range);
	virtual ~CAstExceptionRange() {};
};

class CAstExceptionArg : public CAstException
{
public:
	CAstExceptionArg(const char * basemsg, int order);
	virtual ~CAstExceptionArg() {};
};

class CAstExceptionInternal : public CAstException
{
public:
	CAstExceptionInternal(const char * basemsg, int type);
	virtual ~CAstExceptionInternal() {};
};

class CAstExceptionSyntax : public CAstException
{
public:
	CAstExceptionSyntax(const char * basemsg);
	virtual ~CAstExceptionSyntax() {};
};
