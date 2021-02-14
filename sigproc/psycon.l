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
char *str_mirror;
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
^[ \t;,]*   ; /*To ignore a line of empty content in a udf*/
{NUM}	{
	yylloc.last_column += yyleng;
	yylval.dval = strtod(yytext, NULL);
    if (str_mirror) strcat(str_mirror, yytext);
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
\(	{	/* opening parenthesis 8 */
	yylloc.last_column += yyleng;
	yy_push_state(PARENTH);
    if (str_mirror) strcat(str_mirror, "(");
	return (unsigned char)yytext[0];
	}
<PARENTH,BRACKET,BRACE>end {
	yylloc.last_column += yyleng;
	return T_ENDPOINT;
	}
<PARENTH>\b\f\v { /* Force to close parenthesis*/
	yylloc.last_column += yyleng;
	yy_pop_state();
    if (str_mirror) strcat(str_mirror, ")");
	return (unsigned char)')';
	}
<PARENTH>\) {	/* closing parenthesis 11 */
	yylloc.last_column += yyleng;
	yy_pop_state();
    if (str_mirror) strcat(str_mirror, ")");
	return (unsigned char)yytext[0];
	}
\{	{	/* opening brace */
	yylloc.last_column += yyleng;
	yy_push_state(BRACE);
	return (unsigned char)yytext[0];
	}
<BRACE>\b\f\v { /* Force to close brace*/
	yylloc.last_column += yyleng;
	yy_pop_state();
	return (unsigned char)'}';
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
<TSEQ>\> {	/* closing tseq 16 */
	yylloc.last_column += yyleng;
	yy_pop_state();
	return (unsigned char)yytext[0];
	}
if|else|elseif|end|while|for|break|continue|switch|case|otherwise|function|static|return|sigma|try|catch|"=="|"!="|"<="|">="|"<>"|"->"|">>"|"&&"|"**"|"||"|"++"|"+="|"-="|"*="|"/="|"^="|"->="|"<>="|"#="|"~="|"@="|"@@="|"++="|".."|"->"|->=|">>="|"%=" {
	yylloc.last_column += yyleng;
    if (str_mirror) strcat(str_mirror, yytext);
	return getTokenID(yytext);
	}
{ID}	{
	yylloc.last_column += yyleng;
	yylval.str = strdup(yytext);
    if (str_mirror) strcat(str_mirror, yytext);
	return T_ID;
	}
"//"[^\r\n]* {	/* eat up a comment line 19 */
	yylloc.last_column += yyleng;
	}
[[:blank:]]+ {	/* eat up whitespace 20 */
	yylloc.last_column += yyleng;
	}
\r?\n	{
	++yylloc.last_line;
	yylloc.last_column = 1;
	return T_NEWLINE;
	}
\"	{	/* start of a string 22 */
	yylloc.last_column += yyleng;
	LexStringLen = 0;
	LexString[0] = '\0';
	yy_push_state(STR);
    if (str_mirror) strcat(str_mirror, "\"");
	}
<STR>[^"\b\f\v]*	{
	yylloc.last_column += yyleng;
    if (str_mirror) strcat(str_mirror, yytext);
	if (appendLexString())
		return -1;
	}
<STR>\b\f\v	{ /* force ending of a string 24 */
	yylloc.last_column += yyleng;
    if (str_mirror) strcat(str_mirror, "\"");
    if (yy_top_state()==PARENTH)
        unput(')');
	yy_pop_state();
	yylval.str = strdup(LexString);
	return T_STRING;
	}
<STR>\"\"	{
	yylloc.last_column += yyleng;
	yyleng = 1;
    if (str_mirror) strcat(str_mirror, "\"");
	if (appendLexString())
		return -1;
	}
<STR>\"	{	/* end of a string 26 */
	yylloc.last_column += yyleng;
	yy_pop_state();
	yylval.str = strdup(LexString);
    if (str_mirror) strcat(str_mirror, "\"");
	return T_STRING;
	}
\b\f\v ; /* Now all command line input has this string at the end. Without this rule, syntax error will occur with a correct statement.*/
.	{
	yylloc.last_column += yyleng;
    if (str_mirror) strcat(str_mirror, yytext);
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

int yysetNewStringToScan(const char *source, char *mirror)
{
	if (LexStringMax != LEX_STRING_MAX_INITIAL) {
		/* (re)initialize LexString and LexStringMax - can be shrinking for memory efficiency */
		LexStringMax = LEX_STRING_MAX_INITIAL;
		LexString = (char *)realloc(LexString, LexStringMax);
		if (LexString == NULL)
			return 2;
	}
    str_mirror = mirror;
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