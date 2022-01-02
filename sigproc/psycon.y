/* AUXLAB
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully)
//
// Version: 1.7
// Date: 7/4/2020
*/

/* Psycon syntax parser */
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "psycon.yacc.h"
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
%token T_NEWLINE	"end of line"
%token T_IF		"if"
%token T_ELSE		"else"
%token T_ELSEIF		"elseif"
%token T_END		"end"
%token T_WHILE		"while"
%token T_FOR		"for"
%token T_BREAK		"break"
%token T_CONTINUE	"continue"
%token T_SWITCH		"switch"
%token T_CASE		"case"
%token T_OTHERWISE	"otherwise"
%token T_FUNCTION	"function"
%token T_STATIC		"static"
%token T_RETURN		"return"
%token T_SIGMA		"sigma"
%token T_TRY		"try"
%token T_CATCH		"catch"
%token T_OP_SHIFT	">>"
%token T_OP_CONCAT	"++"
%token T_LOGIC_EQ	"=="
%token T_LOGIC_NE	"!="
%token T_LOGIC_LE	"<="
%token T_LOGIC_GE	">="
%token T_LOGIC_AND	"&&"
%token T_LOGIC_OR	"||"
%token T_REPLICA	".."
%token T_MATRIXMULT	"**"

%token <dval> T_NUMBER "number"
%token <str> T_STRING "string"	T_ID "identifier"
%token T_ENDPOINT T_FULLRANGE

%type <pnode> block block_func line line_func stmt funcdef elseif_list condition conditional case_list id_list arg arg_list vector matrix range exp_range assign exp expcondition initcell compop assign2this tid csig varblock func_decl

%right '='
%right '\''
%left T_LOGIC_OR
%left T_LOGIC_AND
%left T_LOGIC_EQ T_LOGIC_NE
%left '<' '>' T_LOGIC_LE T_LOGIC_GE
%left ':' '~'
%left '-' '+' T_OP_CONCAT
%left T_OP_SHIFT "->" '%' '@'  '#'
%left '*' T_MATRIXMULT '/'
%right '^' /* exponentiation */
%left T_LOGIC_NOT T_POSITIVE T_NEGATIVE /* unary plus/minus */
%token T_TRANSPOSE
%token T_EOF	0	"end of text"

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
AstNode *makeFunctionCall(const char *name, AstNode *first, AstNode *second, YYLTYPE loc);
AstNode *makeBinaryOpNode(int op, AstNode *first, AstNode *second, YYLTYPE loc);
void print_token_value(FILE *file, int type, YYSTYPE value);
char *getT_ID_str(AstNode *p);
void handle_tilde(AstNode *proot, AstNode *pp, YYLTYPE loc);
%}

/* Note to myself 6/7/2018--------------
Q. What does this mean?
	$1->tail = $1->tail->next = $3;
A. Third arg, $3, becomes the node at $1->tail->next. Then, the tail is updated with it.
Q. What is tail?
A. When attaching an argument to the last position, sometimes it's cumbersome to find the last position, i.e., often we have to trace down the pointer all the way to the last.
   By saving it with the tail pointer, we can avoid tracing it down and just put the last argument right there. Once you do it, you just need to update it with a new last position.
End of Note to myself 6/7/2018--------------*/
/*
'(' exp '~' exp ')'
creates a conflict and is removed from the grammar rule.
Why? t1~t2 is reduced to arg_list
7/6/2020
*/
%% /* The grammar follows. */

input: /* empty */
	{ *pproot = NULL;}
	| block_func	/* can be NULL */
	{ *pproot = $1;}
;

block_func: line_func	/* block_func can be NULL */
	| block_func line_func
	{
		if ($2) {
			if ($1 == NULL)
				$$ = $2;
			else if ($1->type == N_BLOCK)
			{
				$1->tail->next = $2;
				$1->tail = $2;
			}
			else
			{ // a=1; b=2; ==> $1->type is '='. So first, a N_BLOCK tree should be made.
				$$ = newAstNode(N_BLOCK, @$);
				$$->next = $1;
				$1->next = $$->tail = $2;
			}
		} else
			$$ = $1;
	}
;

block:	line	/* complicated to prevent NULL (make empty block instead) or single statement block */
	{
		if ($1) // if cond1, x=1, end ==> x=1 comes here.
			$$ = $1;
		else
			$$ = newAstNode(N_BLOCK, @$);
	}
	| block line
	{
		if ($2) {
			if ($1->type == N_BLOCK) {
				if ($$->next) {
					$1->tail->next = $2;
					$1->tail = $2;
				} else {
					$$ = $2;
					free($1);
				}
			} else { //if the second argument doesn't have N_BLOCK, make one
				$$ = newAstNode(N_BLOCK, @$);
				$$->next = $1;
				$1->next = $$->tail = $2;
			}
		}
		else // only "block" is given
			$$ = $1;
	}
;

line:	T_NEWLINE
	{ $$ = NULL;}
	| error T_NEWLINE
	{
		$$ = NULL;
		yyerrok;
	}
	| stmt eol
	| stmt eol2
	{
		$$ = $1;
		$$->suppress=1;
	}
;

line_func: line
	| funcdef
;

eol: ',' | T_NEWLINE | T_EOF
;

eol2: ';'
;

func_end: /* empty */
    | T_EOF
;

func_decl: T_FUNCTION
	{
		$$ = newAstNode(T_FUNCTION, @$);
		$$->suppress = 2;
	}
	| T_STATIC T_FUNCTION
	{
		$$ = newAstNode(T_FUNCTION, @$);
		$$->suppress = 3;
	}
;

funcdef: func_decl T_ID block func_end
	{
		$$ = $1;
		$$->str = $2;
		$$->child = newAstNode(N_IDLIST, @$);
		$$->child->next = $3;
	}
	| func_decl varblock '=' T_ID block func_end
	{
		$$ = $1;
		$$->str = $4;
		$$->child = newAstNode(N_IDLIST, @$);
		$$->child->next = $5;
		if ($2->type!=N_VECTOR)
		{
			$$->alt = newAstNode(N_VECTOR, @2);
			AstNode *p = newAstNode(N_VECTOR, @2);
			p->alt = p->tail = $2;
			$$->alt->str = (char*)p;
		}
		else
		{
			$$->alt = $2;
		}
	}
	| func_decl T_ID '(' id_list ')' block func_end
	{
		$$ = $1;
		$$->str = $2;
		$$->child = $4;
		$4->next = $6;
	}
	| func_decl varblock '=' T_ID '(' id_list ')' block func_end
	{
		$$ = $1;
		$$->str = $4;
		$$->child = $6;
		$6->next = $8;
		if ($2->type!=N_VECTOR)
		{
			$$->alt = newAstNode(N_VECTOR, @2);
			AstNode *p = newAstNode(N_VECTOR, @2);
			p->alt = p->tail = $2;
			$$->alt->str = (char*)p;
		}
		else
		{
			$$->alt = $2;
		}
	}
;

case_list: /* empty */
	{ $$ = newAstNode(T_SWITCH, @$);}
	| T_NEWLINE
	{ $$ = newAstNode(T_SWITCH, @$);}
	| case_list T_CASE exp T_NEWLINE block
	{
		if ($1->alt)
			$1->tail->alt = $3;
		else
			$1->alt = $3;
		AstNode *p = $5;
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, @5);
			p->next = $5;
		}
		$1->tail = $3->next = p;
		$$ = $1;
	}
	| case_list T_CASE '{' arg_list '}' T_NEWLINE block
	{
		if ($1->alt)
			$1->tail->alt = $4;
		else
			$1->alt = $4;
		AstNode *p = $7;
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, @7);
			p->next = $7;
		}
		$1->tail = $4->next = p;
		$$ = $1;
	}
;

stmt: expcondition
	| assign
	| initcell
	| T_IF conditional block elseif_list T_END
	{ // This works, too, for "if cond, act; end" without else, because elseif_list can be empty
		$$ = newAstNode(T_IF, @$);
		AstNode *p = $3;
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, @3);
			p->next = $3;
		}
		$$->child = $2;
		$2->next = p;
		AstNode *pElse = $4;
		if (pElse->type!=N_BLOCK)
		{
			pElse = newAstNode(N_BLOCK, @4);
			pElse->next = $4;
		}
		$$->alt = pElse;
		if ($4->child==NULL && $4->next==NULL) // When elseif_list is empty, T_IF is made, but no child and next
		{
			yydeleteAstNode($$->alt, 1);
			$$->alt=NULL;
		}
		$$->line = @$.first_line;
		$$->col = @$.first_column;
	}
	| T_SWITCH exp case_list T_END
	{ // case is cascaded through alt
		$$ = $3;
		$$->alt = $3->alt;
		$$->child = $2;
		$$->line = @$.first_line;
		$$->col = @$.first_column;
	}
	| T_SWITCH exp case_list T_OTHERWISE block T_END
	{
		$$ = $3;
		$$->alt = $3->alt;
		$$->child = $2;
		AstNode *p = newAstNode(T_OTHERWISE, @5);
		p->next = $5;
		$$->tail = $3->tail->alt = p;
		$$->line = @$.first_line;
		$$->col = @$.first_column;
	}
	| T_TRY block T_CATCH T_ID block T_END
	{
		$$ = newAstNode(T_TRY, @$);
		$$->child = $2;
		$$->alt = newAstNode(T_CATCH, @4);
		$$->alt->child = newAstNode(T_ID, @4);
		$$->alt->child->str = $4;
		$$->alt->next = $5;
	}
	| T_WHILE conditional block T_END
	{
		$$ = newAstNode(T_WHILE, @$);
		$$->child = $2;
		AstNode *p = $3;
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, @3);
			p->next = $3;
		}
		$$->alt = p;
	}
	| T_FOR T_ID '=' exp_range block T_END
	{
		$$ = newAstNode(T_FOR, @$);
		$$->child = newAstNode(T_ID, @2);
		$$->child->str = $2;
		$$->child->child = $4;
		AstNode *p = $5;
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, @5);
			p->next = $5;
		}
		$$->alt = p;
	}
	| T_FOR T_ID '=' exp_range ',' block T_END
	{
		$$ = newAstNode(T_FOR, @$);
		$$->child = newAstNode(T_ID, @2);
		$$->child->str = $2;
		$$->child->child = $4;
		AstNode *p = $6;
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, @6);
			p->next = $6;
		}
		$$->alt = p;
	}
	| T_RETURN
	{
		$$ = newAstNode(T_RETURN, @$);
	}
	| T_BREAK
	{ $$ = newAstNode(T_BREAK, @$);}
	| T_CONTINUE
	{ $$ = newAstNode(T_CONTINUE, @$);}
;


conditional: condition	| condition eol	| exp	| exp eol
;

elseif_list: /*empty*/
	{
		$$ = newAstNode(T_IF, @$);
	}
	| T_ELSEIF conditional block elseif_list
	{
		$$ = newAstNode(T_IF, @$);
		$$->child = $2;
		AstNode *p = $3;
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, @3);
			p->next = $3;
		}
		$2->next = p;
		$$->alt = $4;
	}
	| elseif_list T_ELSE block
	{
		AstNode *p = $3;
		if (p->type!=N_BLOCK)
		{
			p = newAstNode(N_BLOCK, @3);
			p->next = $3;
		}
		if ($1->child==NULL) // if there's no elseif; i.e., elseif_list is empty
		{
			yydeleteAstNode($1, 1);
			$$ = p;
		}
		else
		{
			$$ = $1;
			$1->alt = p;
		}
	}
;

expcondition: csig
;

csig: exp_range
;

initcell: '{' arg_list '}'
	{
		$$ = $2;
		$$->type = N_INITCELL;
		$$->line = @$.first_line;
		$$->col = @$.first_column;
	}
;

condition: '(' condition ')'
	{
		$$ = $2;
		$$->line = @$.first_line;
		$$->col = @$.first_column;
	}
	| exp '<' exp
	{ $$ = makeBinaryOpNode('<', $1, $3, @$);}
	| exp '>' exp
	{ $$ = makeBinaryOpNode('>', $1, $3, @$);}
	| exp T_LOGIC_EQ exp
	{ $$ = makeBinaryOpNode(T_LOGIC_EQ, $1, $3, @$);}
	| exp T_LOGIC_NE exp
	{ $$ = makeBinaryOpNode(T_LOGIC_NE, $1, $3, @$);}
	| exp T_LOGIC_GE exp
	{ $$ = makeBinaryOpNode(T_LOGIC_GE, $1, $3, @$);}
	| exp T_LOGIC_LE exp
	{ $$ = makeBinaryOpNode(T_LOGIC_LE, $1, $3, @$);}
	| '!' expcondition %prec T_LOGIC_NOT
	{
		$$ = newAstNode(T_LOGIC_NOT, @$);
		$$->child = $2;
	}
	| expcondition T_LOGIC_AND expcondition
	{ $$ = makeBinaryOpNode(T_LOGIC_AND, $1, $3, @$);}
	| expcondition T_LOGIC_OR expcondition
	{ $$ = makeBinaryOpNode(T_LOGIC_OR, $1, $3, @$);}
;

id_list: /* empty */
	{
		$$ = newAstNode(N_IDLIST, @$);
	}
	| T_ID
	{
		$$ = newAstNode(N_IDLIST, @$);
		$$->child = $$->tail = newAstNode(T_ID, @$);
		$$->tail->str = $1;
	}
	| id_list ',' T_ID
	{
		$1->tail = $1->tail->next = newAstNode(T_ID, @3);
		$$ = $1;
		$$->tail->str = $3;
	}
;

arg: ':'
	{	$$ = newAstNode(T_FULLRANGE, @$); }
	| exp_range
	| initcell
;

arg_list: arg
	{
		$$ = newAstNode(N_ARGS, @$);
		$$->tail = $$->child = $1;
	}
	| arg_list ',' arg
	{
		$$ = $1;
		if ($$->tail)
			$$->tail = $$->tail->next = $3;
		else
			$$->tail = $$->next = $3;
	}
;

matrix: /* empty */
	{
	// N_MATRIX consists of "outer" N_MATRIX--alt for dot notation
	// and "inner" N_VECTOR--alt for all successive items thru next
	// the str field of the outer N_MATRIX node is cast to the inner N_VECTOR.
	// this "fake" str pointer is freed during normal clean-up
	// 11/4/2019
		$$ = newAstNode(N_MATRIX, @$);
		AstNode * p = newAstNode(N_VECTOR, @$);
		$$->str = (char*)p;
	}
	| vector
	{
		$$ = newAstNode(N_MATRIX, @$);
		AstNode * p = newAstNode(N_VECTOR, @$);
		p->alt = p->tail = $1;
		$$->str = (char*)p;
	}
	| matrix ';' vector
	{
		$$ = $1;
		AstNode * p = (AstNode *)$1->str;
		p->tail = p->tail->next = (AstNode *)$3;
	}
	| matrix ';'
;

vector: exp_range
	{
	// N_VECTOR consists of "outer" N_VECTOR--alt for dot notation
	// and "inner" N_VECTOR--alt for all successive items thru next
	// Because N_VECTOR doesn't use str, the inner N_VECTOR is created there and cast for further uses.
	// this "fake" str pointer is freed during normal clean-up
	// 11/4/2019
		$$ = newAstNode(N_VECTOR, @$);
		AstNode * p = newAstNode(N_VECTOR, @$);
		p->alt = p->tail = $1;
		$$->str = (char*)p;
	}
	| vector exp_range
	{
		AstNode * p = (AstNode *)$1->str;
		p->tail = p->tail->next = $2;
		$$ = $1;
	}
	| vector ',' exp_range
	{
		AstNode * p = (AstNode *)$1->str;
		p->tail = p->tail->next = $3;
		$$ = $1;
	}
;

range: exp ':' exp
	{
		$$ = makeFunctionCall(":", $1, $3, @$);
	}
	| exp ':' exp ':' exp
	{
		$$ = makeFunctionCall(":", $1, $5, @$);
		$5->next = $3;
	}
;

exp_range: exp	| range 	| condition
;

compop: "+="
	{
		$$ = newAstNode('+', @$);
	}
	| "-="
	{ 		$$ = newAstNode('-', @$);	}
	| "*="
	{ 		$$ = newAstNode('*', @$);	}
	| "/="
	{ 		$$ = newAstNode('/', @$);	}
	| "@="
	{ 		$$ = newAstNode('@', @$);	}
	| "@@="
	{
		$$ = newAstNode('@', @$);
		$$->child = newAstNode('@', @$);
	}
	| ">>="
	{ 		$$ = newAstNode(T_OP_SHIFT, @$);	}
	| "%="
	{ 		$$ = newAstNode('%', @$);	}
	| "->="
	{
		$$ = newAstNode(T_ID, @$);
		$$->str = (char*)calloc(16, 1);
		strcpy($$->str, "movespec");
		$$->tail = $$->alt = newAstNode(N_ARGS, @$);
	}
	| "~="
	{
		$$ = newAstNode(T_ID, @$);
		$$->str = (char*)calloc(16, 1);
		strcpy($$->str, "respeed");
		$$->tail = $$->alt = newAstNode(N_ARGS, @$);
	}
	| "<>="
	{
		$$ = newAstNode(T_ID, @$);
		$$->str = (char*)calloc(16, 1);
		strcpy($$->str, "timestretch");
		$$->tail = $$->alt = newAstNode(N_ARGS, @$);
	}
	| "#="
	{
		$$ = newAstNode(T_ID, @$);
		$$->str = (char*)calloc(16, 1);
		strcpy($$->str, "pitchscale");
		$$->tail = $$->alt = newAstNode(N_ARGS, @$);
	}
;

assign2this: '=' exp_range
	{
		$$ = $2;
	}
	| "++=" condition
	{
		$$ = newAstNode(T_OP_CONCAT, @$);
		AstNode *p = $$;
		if (p->alt)
			p = p->alt;
		p->child = 	newAstNode(T_REPLICA, @2);
		p->tail = p->child->next = $2;
	}
	| "++=" exp
	{
		$$ = newAstNode(T_OP_CONCAT, @$);
		AstNode *p = $$;
		if (p->alt)
			p = p->alt;
		p->child = 	newAstNode(T_REPLICA, @2);
		p->tail = p->child->next = $2;
	}
	| compop exp_range
	{
		$$ = $1;
		if ($$->child) // compop should be "@@=" and $$->child->type should be '@'  (64)
		{
			$$->child->child = newAstNode(T_REPLICA, @$);
			$$->child->tail = $$->child->child->next = newAstNode(T_REPLICA, @$);
			$$->tail = $$->child->next = $2;
		}
		else
		{
			AstNode *p = $$;
			if (p->alt)
				p = p->alt;
			p->child = 	newAstNode(T_REPLICA, @2);
			p->tail = p->child->next = $2;
		}
	}
;

varblock:	 T_ID
	{
		$$ = newAstNode(T_ID, @$);
		$$->str = $1;
	}
	|	tid '.' T_ID
	{
		$$ = $1;
		AstNode *p = newAstNode(N_STRUCT, @$);
		p->str = $3;
		if ($$->type==N_CELL)
		{
			$$->alt->alt = p; //always initiating
			$$->tail = p; // so that next concatenation can point to p, even thought p is "hidden" underneath $$->alt
		}
		if ($$->tail)
		{
			$$->tail = $$->tail->alt = p;
		}
		else
		{
			$$->tail = $$->alt = p;
		}
	}
	| varblock '{' exp '}'
	{
		$$ = $1;
		$$->tail = $$->alt->alt = newAstNode(N_CELL, @$);
		$$->tail->child = $3;
	}
	| '[' vector ']'
	{//tid-vector --> what's this comment? 12/30/2020
		$$ = $2;
	}
	| '$' varblock
	{
		$$ = newAstNode(N_HOOK, @$);
		$$->str = (char*)calloc(1, strlen($2->str)+1);
		strcpy($$->str, $2->str);
		$$->alt = $2->alt;
	}
;

tid: varblock
	|  T_ID '(' arg_list ')'
	{
		$$ = newAstNode(T_ID, @$);
		$$->str = $1;
		handle_tilde($$, $3, @3);
	}
	| T_ID '{' exp '}'
	{
		$$ = newAstNode(T_ID, @$);
		$$->str = $1;
		$$->tail = $$->alt = newAstNode(N_CELL, @$);
		$$->alt->child = $3;
	}
	| T_ID '{' exp '}' '(' arg_list ')'
	{
		$$ = newAstNode(T_ID, @$);
		$$->str = $1;
		$$->alt = newAstNode(N_CELL, @$);
		$$->alt->child = $3;
		handle_tilde($$->alt, $6, @6);
		$$->tail = $$->alt->alt; // we need this; or tail is broken and can't put '.' tid at the end
	}
	| varblock '(' arg_list ')'
	{
		$$ = $1;
		handle_tilde($$, $3, @3);
	}
	| varblock '(' ')'
	{
		if ($$->alt != NULL  && $$->alt->type==N_STRUCT)
		{ // dot notation with a blank parentheses, e.g., a.sqrt() or (1:2:5).sqrt()
			$$ = $1;
		}
		else // no longer used.,,.. absorbed by tid:  T_ID
		{ // udf_func()
			$$ = newAstNode(N_CALL, @$);
			$$->str = getT_ID_str($1);
		}
	}
	| T_REPLICA
	{
		$$ = newAstNode(T_REPLICA, @$);
	}
	|  T_REPLICA '(' arg_list ')'
	{
		$$ = newAstNode(T_REPLICA, @$);
		handle_tilde($$, $3, @3);
	}
	|  T_REPLICA '{' exp '}'
	{
		$$ = newAstNode(T_REPLICA, @$);
		$$->tail = $$->alt = newAstNode(N_CELL, @$);
		$$->alt->child = $3;
	}
	| T_REPLICA '{' exp '}' '(' arg_list ')'
	{
		$$ = newAstNode(T_REPLICA, @$);
		$$->alt = newAstNode(N_CELL, @$);
		$$->alt->child = $3;
		handle_tilde($$->alt, $6, @6);
		$$->tail = $$->alt->alt; // we need this; or tail is broken and can't put '.' tid at the end
	}
	| tid '\''
	{
		$$ = newAstNode(T_TRANSPOSE, @$);
 		$$->child = $1;
	}
	| '[' vector ']' '[' ']'
	{
		$$ = newAstNode(N_TSEQ, @$);
		$$->child = $2;
	}
	| '[' vector ']' '[' vector ']'
	{
		$$ = newAstNode(N_TSEQ, @$);
		$$->child = $2;
		$$->child->next = $5;
	}
	| '[' vector ']' '[' matrix ']'
	{
		$$ = newAstNode(N_TSEQ, @$);
		$$->child = $2;
		$$->child->next = $5;
	}
	| '[' matrix ']' '[' vector ']'
	{
		$$ = newAstNode(N_TSEQ, @$);
		$$->str = (char*)malloc(8);
		strcpy($$->str, "R");
		$$->child = $2;
		$$->child->next = $5;
	}
	| '[' matrix ']' '[' matrix ']'
	{
		$$ = newAstNode(N_TSEQ, @$);
		$$->str = (char*)malloc(8);
		strcpy($$->str, "R");
		$$->child = $2;
		$$->child->next = $5;
	}
	| '[' matrix ']'
	{
		$$ = $2;
	}
	| '(' exp_range ')'
	{
		$$ = $2;
		$$->line = @$.first_line;
		$$->col = @$.first_column;
	}
;

/* Here, both rules of tid and varblock should be included... tid assign2this alone is not enough 8/18/2018*/
assign: tid assign2this
	{
		$$ = $1;
		$$->child = $2;
	}
	| varblock assign2this
	{
		$$ = $1;
		$$->child = $2;
	}
	| varblock '=' assign
	{ //c=a(2)=44
		if (!$1->child)
			$1->child = $3;
		else
			for (AstNode *p = $1->child; p; p=p->child)
			{
				if (!p->child)
				{
					p->child = $3;
					break;
				}
			}
		$$ = $1;
	}
	| tid '=' assign
	{ //a(2)=d=11
		if (!$1->child)
			$1->child = $3;
		else
			for (AstNode *p = $1->child; p; p=p->child)
			{
				if (!p->child)
				{
					p->child = $3;
					break;
				}
			}
		$$ = $1;
	}
	| varblock '=' initcell
	{ // x={"bjk",noise(300), 4.5555}
		$$->str = getT_ID_str($1);
		$$->child = $3;
		$$->line = @$.first_line;
		$$->col = @$.first_column;
	}
;

exp: initcell
	| tid
	| T_NUMBER
	{
		$$ = newAstNode(T_NUMBER, @$);
		$$->dval = $1;
	}
	| T_STRING
	{
		$$ = newAstNode(T_STRING, @$);
		$$->str = $1;
	}
	| T_ENDPOINT
	{
		$$ = newAstNode(T_ENDPOINT, @$);
	}
	| '-' exp %prec T_NEGATIVE
	{
		$$ = newAstNode(T_NEGATIVE, @$);
		$$->child = $2;
	}
	| '+' exp %prec T_POSITIVE
	{
		$$ = $2;
		$$->line = @$.first_line;
		$$->col = @$.first_column;
	}
	| T_SIGMA '(' tid '=' exp_range ',' exp ')'
	{
		$$ = newAstNode(T_SIGMA, @$);
		$$->child = newAstNode(T_ID, @3);
		$$->child->str = getT_ID_str($3);
		$$->child->child = $5;
		$$->child->next = $7;
	}
	| exp '+' exp
	{ $$ = makeBinaryOpNode('+', $1, $3, @$);}
	| exp '-' exp
	{ $$ = makeBinaryOpNode('-', $1, $3, @$);}
	| exp '*' exp
	{ $$ = makeBinaryOpNode('*', $1, $3, @$);}
	| exp '/' exp
	{ $$ = makeBinaryOpNode('/', $1, $3, @$);}
	| exp T_MATRIXMULT exp
	{ $$ = makeBinaryOpNode(T_MATRIXMULT, $1, $3, @$);}
	| exp '^' exp
	{ $$ = makeFunctionCall("^", $1, $3, @$);}
	| exp '%' exp
	{ $$ = makeFunctionCall("mod", $1, $3, @$);}
	| exp '~' exp
	{ $$ = makeFunctionCall("respeed", $1, $3, @$);}
	| exp '#' exp
	{ $$ = makeFunctionCall("pitchscale", $1, $3, @$);}
	| exp "<>" exp
	{ $$ = makeFunctionCall("timestretch", $1, $3, @$);}
	| exp "->" exp
	{ $$ = makeFunctionCall("movespec", $1, $3, @$);}
	| exp '@' exp
	{ $$ = makeBinaryOpNode('@', $1, $3, @$);}
	| exp T_OP_SHIFT exp
	{ $$ = makeBinaryOpNode(T_OP_SHIFT, $1, $3, @$);}
	| exp T_OP_CONCAT exp
	{ $$ = makeBinaryOpNode(T_OP_CONCAT, $1, $3, @$);}
;


//tseq:
// vector '[' ']'
//	{
//		$$ = newAstNode(N_TSEQ, @$);
//		$$->child = $1;
//	}
//	|
// vector matrix
//	{
//		$$ = newAstNode(N_TSEQ, @$);
//		$$->child = $1;
//		$$->child->next = $2;
//	}

%%

/* Called by yyparse on error. */
void yyerror (AstNode **pproot, char **errmsg, char const *s)
{
  static size_t errmsg_len = 0;
#define ERRMSG_MAX 999
  char msgbuf[ERRMSG_MAX], *p;
  size_t msglen;

  sprintf(msgbuf, "Invalid syntax: Line %d, Col %d: %s.\n", yylloc.first_line, yylloc.first_column, s + (strncmp(s, "syntax error, ", 14) ? 0 : 14));
  if ((p=strstr(msgbuf, "$undefined"))) {
	sprintf(p, "'%c'(%d)", yychar, yychar);
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
  strcpy(ErrorMsg+errmsg_len, msgbuf);
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
	if (type == T_ID)
		fprintf (file, "%s", value.str);
	else if (type == T_NUMBER)
		fprintf (file, "%f", value.dval);
}


char *getAstNodeName(AstNode *p)
{
#define N_NAME_MAX 99
  static char buf[N_NAME_MAX];

  if (!p)
	return NULL;
  switch (p->type) {
  case '=':
    sprintf(buf, "[%s=]", p->str);
    break;
  case T_ID:
    sprintf(buf, "[%s]", p->str);
    break;
  case T_STRING:
    sprintf(buf, "\"%s\"", p->str);
    break;
  case N_CALL:
    sprintf(buf, "%s()", p->str);
    break;
  case N_CELL:
    sprintf(buf, "%s()", p->str);
    break;
  case T_NUMBER:
    sprintf(buf, "%.1f", p->dval);
    break;
  case N_BLOCK:
    sprintf(buf, "BLOCK");
    break;
  case N_ARGS:
    sprintf(buf, "ARGS");
    break;
  case N_MATRIX:
    sprintf(buf, "MATRIX");
    break;
  case N_VECTOR:
    sprintf(buf, "VECTOR");
    break;
  case N_IDLIST:
    sprintf(buf, "ID_LIST");
    break;
  case N_TIME_EXTRACT:
    sprintf(buf, "TIME_EXTRACT");
    break;
  case N_CELLASSIGN:
    sprintf(buf, "INITCELL");
    break;
  case N_IXASSIGN:
    sprintf(buf, "ASSIGN1");
    break;
  default:
    if (YYTRANSLATE(p->type) == 2)
      sprintf(buf, "[%d]", p->type);
    else
      sprintf(buf, "%s", yytname[YYTRANSLATE(p->type)]);
  }
  return buf;
}

/* As of 4/17/2018
In makeFunctionCall and makeBinaryOpNode,
node->tail is removed, because it caused conflict with tail made in
tid: tid '.' T_ID or tid: tid '(' arg_list ')'
or possibly other things.
The only downside from this change is, during debugging, the last argument is not seen at the top node where several nodes are cascaded: e.g., a+b+c
*/

AstNode *makeFunctionCall(const char *name, AstNode *first, AstNode *second, YYLTYPE loc)
{
	AstNode *node;

	node = newAstNode(T_ID, loc);
	node->str = (char*)calloc(1, strlen(name)+1);
	strcpy(node->str, name);
	node->tail = node->alt = newAstNode(N_ARGS, loc);
	node->alt->child = first;
	first->next = second;
	return node;
}

AstNode *makeBinaryOpNode(int op, AstNode *first, AstNode *second, YYLTYPE loc)
{
	AstNode *node;

	node = newAstNode(op, loc);
	node->child = first;
	first->next = second;
	return node;
}

AstNode *newAstNode(int type, YYLTYPE loc)
{
#ifdef DEBUG
    static int cnt=0;
#endif
  AstNode *node;

  node = (AstNode *)malloc(sizeof(AstNode));
  if (node==NULL)
    exit(2);
  memset(node, 0, sizeof(AstNode));
  node->type = type;
#ifdef DEBUG
    printf("created node %d: %s\n", ++cnt, getAstNodeName(node));
#endif
  node->line = loc.first_line;
  node->col = loc.first_column;
  return node;
}

char *getT_ID_str(AstNode *p)
{
	if (p->type==T_ID)
		return p->str;
	printf("Must be T_ID\n");
	return NULL;
}

void handle_tilde(AstNode *proot, AstNode *pp, YYLTYPE loc)
{
	AstNode *p = pp->child;
	if (p->type==T_ID && !strcmp(p->str,"respeed"))
	{ // x{2}(t1~t2) checks here because t1~t2 can be arg_list through
        AstNode *q = newAstNode(N_TIME_EXTRACT, loc);
		q->child = p->alt->child;
		q->child->next = p->alt->child->next;
        if (proot->tail)
            proot->tail = proot->tail->alt = q;
		else
			proot->tail = proot->alt = q;
		p->alt->child = NULL;
		yydeleteAstNode(p, 1);
	}
	else
	{
	    if (proot->tail)
			proot->tail = proot->tail->alt = pp;
		else
			proot->tail = proot->alt = pp;
	}
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
  if (p->child)
    yydeleteAstNode(p->child, 0);
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