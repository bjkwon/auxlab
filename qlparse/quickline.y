/* AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.497
// Date: 12/27/2018
*/ 

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lqsyn.yacc.h"
#define strdup _strdup
/*#define DEBUG*/

char *qErrorMsg = NULL;
int qqlex (void);
void qqerror (AstNode **pproot, char **errmsg, char const *s);
%}

/* Bison declarations. */
%locations
%token-table
%error-verbose
%debug

%union {
	double dval;
	char *str;
	AstNode *pnode;
}
%token T_UNKNOWN
%token <str> T_STRING "string" T_ID "identifier" 

%type <pnode> block_string one_word

%parse-param {AstNode **pproot}
%parse-param {char **errmsg}

%initial-action
{
  if (qErrorMsg) {
	free(qErrorMsg);
	qErrorMsg = NULL;
  }
  *errmsg = NULL;
};

%destructor
{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName($$));
#endif
  qqdeleteAstNode($$, 0);
} <pnode>
%destructor
{
#ifdef DEBUG
    printf("discarding string \"%s\"\n", $$);
#endif
  free($$);
} <str>

%{
AstNode *neuAstNode(int type, YYLTYPE loc);
%}

%% /* The grammar follows. */
input: /* empty */
	{ *pproot = NULL;}
	| block_string	/* can be NULL */
	{ *pproot = $1;}
;

block_string: one_word	/*  block_string can be NULL */
	{ $$ = $1; }
	|  block_string  one_word
	{ 
		if ($$->tail)
			$$->tail->next = $2;
		else
			$$->next = $2;
		$$->tail = $2;
	}
;
one_word: T_STRING
	{
		$$ = neuAstNode(T_STRING, @$);
		$$->str = malloc(strlen($1)+1);
		strcpy($$->str, $1);
	}
	| T_ID
	{
		$$ = neuAstNode(T_ID, @$);
		$$->str = malloc(strlen($1)+1);
		strcpy($$->str, $1);
	}
;

%%

/* Called by qqparse on error. */
void qqerror (AstNode **pproot, char **errmsg, char const *s)
{
  static size_t errmsg_len = 0;
#define ERRMSG_MAX 999
  char msgbuf[ERRMSG_MAX], *p;
  size_t msglen;

  sprintf_s(msgbuf, ERRMSG_MAX, "Line %d, Col %d: %s.\n", qqlloc.first_line, qqlloc.first_column, s + (strncmp(s, "syntax error, ", 14) ? 0 : 14));
  if ((p=strstr(msgbuf, "$undefined"))) {
	sprintf_s(p, 10, "'%c'(%d)", qqchar, qqchar);
    strcpy(p+strlen(p), p+10);
  }
  if ((p=strstr(msgbuf, "end of text or ")))
    strcpy(p, p+15);
  if ((p=strstr(msgbuf, " or ','")))
    strcpy(p, p+7);
  msglen = strlen(msgbuf);
  if (qErrorMsg == NULL)
    errmsg_len = 0;
  qErrorMsg = (char *)realloc(qErrorMsg, errmsg_len+msglen+1);
  strcpy_s(qErrorMsg+errmsg_len, msglen+1, msgbuf);
  errmsg_len += msglen;
  *errmsg = qErrorMsg;
}


AstNode *neuAstNode(int type, YYLTYPE loc)
{
#ifdef DEBUG
    static int cnt=0;
#endif
  AstNode *nn;

  nn = (AstNode *)malloc(sizeof(AstNode));
  if (nn==NULL)
    exit(2);
  memset(nn, 0, sizeof(AstNode));
  nn->type = type;
#ifdef DEBUG
    printf("created node %d: %s\n", ++cnt, getAstNodeName(nn));
#endif
  nn->line = loc.first_line;
  nn->col = loc.first_column;
  return nn;
}


int qqdeleteAstNode(AstNode *p, int fSkipNext)
{
#ifdef DEBUG
    static int cnt=0;
#endif
  AstNode *tmp, *next;
  
  if (!p)
	return 0;
#ifdef DEBUG
    printf("deleting node %d: %s\n", ++cnt, getAstNodeName(p));
#endif
  if (p->str)
    free(p->str);
  if (!fSkipNext && p->next) {
	for (tmp=p->next; tmp; tmp=next) {
      next = tmp->next;
      qqdeleteAstNode(tmp, 1);
    }
  }
  free(p);
  return 0;
}

int qqPrintf(const char *msg, AstNode *p)
{
	if (p)
		printf("[%16s]token type: %d, %s, \n", msg, p->type, p->str);
	return 1;
}