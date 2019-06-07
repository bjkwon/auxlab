// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.495
// Date: 12/13/2018
// 
#pragma once

#include <math.h>
#include <map>
#include <string>
#include <time.h>
#include "aux_classes.h"
#include "sigproc.h"
using namespace std;

static map<string, string> builtin_fnsigs;
static map<string, bool> builtin_staticfn;
static map<string, int> builtin_fn_nArg1, builtin_fn_nArg2;



int OtherBuiltIn(const string &fname, CAstSig &ast, const AstNode *pnode, const AstNode *p, int nArgs, std::string &fnsigs);


complex<double> cmpexp(complex<double> x) { return exp(x); }
complex<double> cmpcos(complex<double> x) { return cos(x); }
complex<double> cmpcosh(complex<double> x) { return cosh(x); }
complex<double> cmplog(complex<double> x) { return log(x); }
complex<double> cmplog10(complex<double> x) { return log10(x); }
complex<double> cmpsin(complex<double> x) { return sin(x); }
complex<double> cmpsinh(complex<double> x) { return sinh(x); }
complex<double> cmptan(complex<double> x) { return tan(x); }
complex<double> cmptanh(complex<double> x) { return tanh(x); }

double cmpreal(complex<double> x) { return real(x); }
double cmpimag(complex<double> x) { return imag(x); }
double cmpabs(complex<double> x) { return abs(x); }
complex<double> cmpconj(complex<double> x) { return conj(x); }
complex<double> cmpsqrt(complex<double> x) { return sqrt(x); }
//double cmpnorm(complex<double> x) { return norm(x); }
double cmpangle(complex<double> x) { return arg(x); }

double aux_db(double x)
{
	return pow(10, x / 20);
}

double aux_sign(double x)
{
	return (x == 0.) ? 0. : ((x>0.) ? 1. : -1.);
}

double aux_round(double x)
{
	if (x >= 0)
		return (double)(int)(x + .5);
	else
		return -aux_round(-x);
}

double aux_fix(double x)
{
	return (double)(int)x;
}

double aux_pow(double base, double exponent)
{
	return pow(base, exponent);
}

complex<double> aux_cexp(complex<double> base, complex<double> exponent)
{
	return pow(base, exponent);
}

double aux_mod(double numer, double denom)
{
	return fmod(numer, denom);
}

double aux_passthru(double number)
{
	return number;
}

double aux_angle(double number)
{
	return number;
}

double aux_angle_4_real(double number)
{
	return number > 0 ? acos(1) : acos(-1);
}

int CAstSig::HandleMathFunc(bool compl, string &fname, double(**fn0)(double), double(**fn1)(double), double(**fn2)(double, double), double(**cfn0)(complex<double>), complex<double>(**cfn1)(complex<double>), complex<double>(**cfn2)(complex<double>, complex<double>) )
{
	if (fname == "abs") { if (compl) *cfn0 = cmpabs; 	else	*fn1 = fabs; }
	else if (fname == "conj") { if (compl) *cfn1 = cmpconj; 	else	*fn1 = fabs; }
	else if (fname == "real") {	if (compl) *cfn0 = cmpreal; 	else *fn1 = aux_passthru; }
	else if (fname == "imag") {	if (compl) *cfn0 = cmpimag; 	
	
			else {	
				Sig.SetReal(); 
				for (auto &val : Sig) val = 0.; 	
				*fn1 = aux_passthru;} }
	else if (fname == "angle") { if (compl) *cfn0 = cmpangle;	else *fn1 = aux_angle_4_real; }
	else if (fname == "sin")	if (compl)	*cfn1 = cmpsin; else *fn1 = sin;
	else if (fname == "cos")	if (compl)	*cfn1 = cmpcos; else *fn1 = cos;
	else if (fname == "tan")	if (compl)	*cfn1 = cmptan; else *fn1 = tan;
	else if (fname == "log")	return 1; 
	else if (fname == "log10")	return 1;
	else if (fname == "exp")	if (compl)	*cfn1 = cmpexp; else *fn1 = exp;
	else if (fname == "sqrt") return 1;
	else if (fname == "db")		*fn1 = aux_db;
	else if (fname == "sign")	*fn1 = aux_sign;
	else if (fname == "asin")	*fn1 = asin;
	else if (fname == "acos")	*fn1 = acos;
	else if (fname == "atan")	*fn1 = atan;
	else if (fname == "round")	*fn1 = aux_round;
	else if (fname == "fix")	*fn1 = aux_fix;
	else if (fname == "ceil")	*fn1 = ceil;
	else if (fname == "floor")	*fn1 = floor;
	else if (fname == "^" || fname == "pow") {
		*fn2 = aux_pow; return 2; // This should be treated with symmetry. do this later 2/22/2018
	}
	else if (fname == "mod") {
		*fn2 = aux_mod; return 2; // added 8/21/2018
	}

	if (*fn0 || *fn1 || *fn2 || *cfn0 || *cfn1 || *cfn2) // if fname is one of math functions above
		return 10;
	else
		return 0;
}

