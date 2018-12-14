/* Psycon syntax parser */
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lqsyn.yacc.h"
#define strdup _strdup
#define YYPRINT(file, type, value) print_token_value (file, type, value)
/*#define DEBUG*/

char *ErrorMsg = NULL;
int yylex (void);
void yyerror (AstNode **pproot, char **errmsg, char const *s);
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
  if (ErrorMsg) {
	free(ErrorMsg);
	ErrorMsg = NULL;
  }
  *errmsg = NULL;
};

%destructor
{
#ifdef DEBUG
    printf("discarding node %s\n", getAstNodeName($$));
#endif
  yydeleteAstNode($$, 0);
} <pnode>
%destructor
{
#ifdef DEBUG
    printf("discarding string \"%s\"\n", $$);
#endif
  free($$);
} <str>

%{
AstNode *newAstNode(int type, YYLTYPE loc);
void print_token_value(FILE *file, int type, YYSTYPE value);
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
		$$ = newAstNode(T_STRING, @$);
		$$->str = malloc(strlen($1)+1);
		strcpy($$->str, $1);
	}
	| T_ID
	{
		$$ = newAstNode(T_ID, @$);
		$$->str = malloc(strlen($1)+1);
		strcpy($$->str, $1);
	}
;

%%

/* Called by yyparse on error. */
void yyerror (AstNode **pproot, char **errmsg, char const *s)
{
  static size_t errmsg_len = 0;
#define ERRMSG_MAX 999
  char msgbuf[ERRMSG_MAX], *p;
  size_t msglen;

  sprintf_s(msgbuf, ERRMSG_MAX, "Line %d, Col %d: %s.\n", yylloc.first_line, yylloc.first_column, s + (strncmp(s, "syntax error, ", 14) ? 0 : 14));
  if ((p=strstr(msgbuf, "$undefined"))) {
	sprintf_s(p, 10, "'%c'(%d)", yychar, yychar);
    strcpy(p+strlen(p), p+10);
  }
  if ((p=strstr(msgbuf, "end of text or ")))
    strcpy(p, p+15);
  if ((p=strstr(msgbuf, " or ','")))
    strcpy(p, p+7);
  msglen = strlen(msgbuf);
  if (ErrorMsg == NULL)
    errmsg_len = 0;
  ErrorMsg = (char *)realloc(ErrorMsg, errmsg_len+msglen+1);
  strcpy_s(ErrorMsg+errmsg_len, msglen+1, msgbuf);
  errmsg_len += msglen;
  *errmsg = ErrorMsg;
}


int getTokenID(const char *str)
{
	size_t len, i;
	len = strlen(str);
	for (i = 0; i < YYNTOKENS; i++) {
		if (yytname[i] != 0
			&& yytname[i][0] == '"'
			&& !strncmp (yytname[i] + 1, str, len)
			&& yytname[i][len + 1] == '"'
			&& yytname[i][len + 2] == 0)
				break;
	}
	if (i < YYNTOKENS)
		return yytoknum[i];
	else
		return T_UNKNOWN;
}

void print_token_value(FILE *file, int type, YYSTYPE value)
{
	fprintf (file, "%s", value.str);
}

AstNode *newAstNode(int type, YYLTYPE loc)
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


int yydeleteAstNode(AstNode *p, int fSkipNext)
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
      yydeleteAstNode(tmp, 1);
    }
  }
  free(p);
  return 0;
}

int yyPrintf(const char *msg, AstNode *p)
{
	if (p)
		printf("[%16s]token type: %d, %s, \n", msg, p->type, p->str);
	return 1;
}