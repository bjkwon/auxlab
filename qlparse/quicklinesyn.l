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

/* scanner for Line string quotation syntax parser */
%{
#include <stdlib.h>
#include <string.h>
#include "quickline.yacc.h"
#include "quickline.tab.h"
#define strdup _strdup
#define LEX_STRING_MAX_INITIAL 1024

int LexStringqqLen = 0, LexStringqqMax = 0;
char *LexStringqq = NULL;
int appendLexStringqq(void);
%}
%pointer
%option noyywrap
%option stack
%x STR
ID	[[:alnum:]\\_:~`'!@#$%^&()-=+|\[\]\{\},'<>]{0,512}
%%
%{
	qqlloc.first_line = qqlloc.last_line;
	qqlloc.first_column = qqlloc.last_column;
%}
[[:blank:]]+ {	/* eat up whitespace */
	qqlloc.last_column += qqleng;
	}
\"	{	/* start of a string */
	qqlloc.last_column += qqleng;
	LexStringqqLen = 0;
	LexStringqq[0] = '\0';
	yy_push_state(STR);
	}
<STR>[^"\n]*	{
	qqlloc.last_column += qqleng;
	if (appendLexStringqq())
		return -1;
	}
<STR>\n	{
	++qqlloc.last_line;
	qqlloc.last_column = 1;
	if (appendLexStringqq())
		return -1;
	}
<STR>\"\"	{
	qqlloc.last_column += qqleng;
	qqleng = 1;
	if (appendLexStringqq())
		return -1;
	}
<STR>\"	{	/* end of a string */
	qqlloc.last_column += qqleng;
	yy_pop_state();
	qqlval.str = strdup(LexStringqq);
	return T_STRING;
	}
{ID}	{
	qqlloc.last_column += qqleng;
	qqlval.str = strdup(qqtext);
	return T_ID;
	}
.	{
	qqlloc.last_column += qqleng;
	return (unsigned char)qqtext[0];
	/* printf( "Unrecognized character: %s\n", qqtext ); */
	}
%%
int appendLexStringqq(void)
{
	if (LexStringqqMax < LexStringqqLen + qqleng + 1) {
		LexStringqqMax *= 2;
		LexStringqq = (char *)realloc(LexStringqq, LexStringqqMax);
		if (LexStringqq == NULL)
			return 2;
	}
	strncpy_s(LexStringqq+LexStringqqLen, LexStringqqMax-LexStringqqLen, qqtext, qqleng);
	LexStringqqLen += qqleng;
	LexStringqq[LexStringqqLen] = '\0';
	return 0;
}

int qqsetNewStringToScan(const char *source)
{
	if (LexStringqqMax != LEX_STRING_MAX_INITIAL) {
		/* (re)initialize LexStringqq and LexStringqqMax - can be shrinking for memory efficiency */
		LexStringqqMax = LEX_STRING_MAX_INITIAL;
		LexStringqq = (char *)realloc(LexStringqq, LexStringqqMax);
		if (LexStringqq == NULL)
			return 2;
	}
	qqrestart(NULL);
	qq_delete_buffer(YY_CURRENT_BUFFER);
	qq_scan_string(source);
	BEGIN(INITIAL);
	qqlloc.last_line = qqlloc.first_line = 1;
	qqlloc.last_column = qqlloc.first_column = 1;
	return 0;
}

int qqsetNewFileToScan(FILE *source)
{
	if (LexStringqqMax != LEX_STRING_MAX_INITIAL) {
		/* (re)initialize LexStringqq and LexStringqqMax - can be shrinking for memory efficiency */
		LexStringqqMax = LEX_STRING_MAX_INITIAL;
		LexStringqq = (char *)realloc(LexStringqq, LexStringqqMax);
		if (LexStringqq == NULL)
			return 2;
	}
	qq_delete_buffer(YY_CURRENT_BUFFER);
	qqrestart(source);
	BEGIN(INITIAL);
	qqlloc.last_line = qqlloc.first_line = 1;
	qqlloc.last_column = qqlloc.first_column = 1;
	return 0;
}