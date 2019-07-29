/* AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.61
// Date: 7/27/2019
*/ 

/* scanner for Psycon syntax parser */
%{
#include <stdlib.h>
#include <string.h>
#include "psycon.yacc.h"
#include "psycon.tab.h"
#define strdup _strdup
#define LEX_STRING_MAX_INITIAL 1024

int LexStringLen = 0, LexStringMax = 0;
char *LexString = NULL;
int appendLexString(void);
%}
%pointer
%option noyywrap
%option stack
%x STR
%s BRACKET
%s PARENTH
%s BRACE
%s TSEQ
ID	[[:alpha:]_?][[:alnum:]_]{0,512}
NUM	[[:digit:]]+"."?[[:digit:]]*([eE][+-]?[[:digit:]]+)?|"."[[:digit:]]+
%%
%{
	yylloc.first_line = yylloc.last_line;
	yylloc.first_column = yylloc.last_column;
%}
{NUM}	{
	yylloc.last_column += yyleng;
	yylval.dval = strtod(yytext, NULL);
	return T_NUMBER;
	}
\[ {		/* opening bracket */
	yylloc.last_column += yyleng;
	yy_push_state(BRACKET);
	return (unsigned char)yytext[0];
	}
<BRACKET>\r?\n {
	++yylloc.last_line;
	yylloc.last_column = 1;
	}
<BRACKET>[[:blank:]]+[+-]{NUM} {
	yylloc.last_column += yyleng;
	yylval.dval = strtod(yytext, NULL);
	return T_NUMBER;
	}
<BRACKET>\r?\n[+-]{NUM} {
	++yylloc.last_line;
	yylloc.last_column = 1;
	yylval.dval = strtod(yytext, NULL);
	return T_NUMBER;
	}

<BRACKET>\] {	/* closing bracket */
	yylloc.last_column += yyleng;
	yy_pop_state();
	return (unsigned char)yytext[0];
	}
\(	{	/* opening parenthesis */
	yylloc.last_column += yyleng;
	yy_push_state(PARENTH);
	return (unsigned char)yytext[0];
	}
<PARENTH>end {
	yylloc.last_column += yyleng;
	return T_ENDPOINT;
	}
<PARENTH>\) {	/* closing parenthesis */
	yylloc.last_column += yyleng;
	yy_pop_state();
	return (unsigned char)yytext[0];
	}
\{	{	/* opening brace */
	yylloc.last_column += yyleng;
	yy_push_state(BRACE);
	return (unsigned char)yytext[0];
	}
<BRACE>\} {	/* closing brace */
	yylloc.last_column += yyleng;
	yy_pop_state();
	return (unsigned char)yytext[0];
	}
\<	{	/* opening tseq */
	yylloc.last_column += yyleng;
	yy_push_state(TSEQ);
	return (unsigned char)yytext[0];
	}
<TSEQ>\> {	/* closing tseq */
	yylloc.last_column += yyleng;
	yy_pop_state();
	return (unsigned char)yytext[0];
	}
if|else|elseif|end|while|for|break|continue|switch|case|otherwise|function|static|return|sigma|"=="|"!="|"<="|">="|"<>"|"->"|">>"|"&&"|"**"|"||"|"++"|"+="|"-="|"*="|"/="|"^="|"->="|"<>="|"#="|"~="|"@="|"@@="|"++="|".."|"->"|->=|">>="|"%=" {
	yylloc.last_column += yyleng;
	return getTokenID(yytext);
	}
{ID}	{
	yylloc.last_column += yyleng;
	yylval.str = strdup(yytext);
	return T_ID;
	}
"//"[^\n]* {	/* eat up a comment line */
	yylloc.last_column += yyleng;
	}
[[:blank:]]+ {	/* eat up whitespace */
	yylloc.last_column += yyleng;
	}
\r?\n	{
	++yylloc.last_line;
	yylloc.last_column = 1;
	return T_NEWLINE;
	}
\"	{	/* start of a string */
	yylloc.last_column += yyleng;
	LexStringLen = 0;
	LexString[0] = '\0';
	yy_push_state(STR);
	}
<STR>[^"\n]*	{
	yylloc.last_column += yyleng;
	if (appendLexString())
		return -1;
	}
<STR>\n	{
	++yylloc.last_line;
	yylloc.last_column = 1;
	if (appendLexString())
		return -1;
	}
<STR>\"\"	{
	yylloc.last_column += yyleng;
	yyleng = 1;
	if (appendLexString())
		return -1;
	}
<STR>\"	{	/* end of a string */
	yylloc.last_column += yyleng;
	yy_pop_state();
	yylval.str = strdup(LexString);
	return T_STRING;
	}
.	{
	yylloc.last_column += yyleng;
	return (unsigned char)yytext[0];
	/* printf( "Unrecognized character: %s\n", yytext ); */
	}
%%
int appendLexString(void)
{
	if (LexStringMax < LexStringLen + yyleng + 1) {
		LexStringMax *= 2;
		LexString = (char *)realloc(LexString, LexStringMax);
		if (LexString == NULL)
			return 2;
	}
	strncpy_s(LexString+LexStringLen, LexStringMax-LexStringLen, yytext, yyleng);
	LexStringLen += yyleng;
	LexString[LexStringLen] = '\0';
	return 0;
}

int yysetNewStringToScan(const char *source)
{
	if (LexStringMax != LEX_STRING_MAX_INITIAL) {
		/* (re)initialize LexString and LexStringMax - can be shrinking for memory efficiency */
		LexStringMax = LEX_STRING_MAX_INITIAL;
		LexString = (char *)realloc(LexString, LexStringMax);
		if (LexString == NULL)
			return 2;
	}
	yyrestart(NULL);
	yy_delete_buffer(YY_CURRENT_BUFFER);
	yy_scan_string(source);
	BEGIN(INITIAL);
	yylloc.last_line = yylloc.first_line = 1;
	yylloc.last_column = yylloc.first_column = 1;
	return 0;
}

int yysetNewFileToScan(FILE *source)
{
	if (LexStringMax != LEX_STRING_MAX_INITIAL) {
		/* (re)initialize LexString and LexStringMax - can be shrinking for memory efficiency */
		LexStringMax = LEX_STRING_MAX_INITIAL;
		LexString = (char *)realloc(LexString, LexStringMax);
		if (LexString == NULL)
			return 2;
	}
	yy_delete_buffer(YY_CURRENT_BUFFER);
	yyrestart(source);
	BEGIN(INITIAL);
	yylloc.last_line = yylloc.first_line = 1;
	yylloc.last_column = yylloc.first_column = 1;
	return 0;
}