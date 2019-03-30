// AUXLAB 
//
// Copyright (c) 2009-2019 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: sigproc
// Signal Generation and Processing Library
// Platform-independent (hopefully) 
// 
// Version: 1.5
// Date: 3/30/2018
// 
#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "sigproc.h"

#define MAX_24BIT		(double)0x007fffff
#define iMAX_24BIT		0x007fffff
#define MAX_16BIT		(double)0x7fff
#define iMAX_16BIT		0x7fff

extern double playblock;

//from ellf.c
extern "C" int design_iir(double *num, double *den, int fs, int kind, int type, int n, double *freqs, double dbr /*rippledB*/, double dbd /*stopEdgeFreqORattenDB*/);
