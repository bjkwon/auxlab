// AUXLAB 
//
// Copyright (c) 2009-2019 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: AUXLib
// dynamic library for sigproc
// 
// Version: 0.1
// Date: 8/29/2021

#ifdef LINK_AUXLIB
#define AUXLIB_EXP __declspec (dllimport)
#else 
#define AUXLIB_EXP __declspec  (dllexport)
#endif


AUXLIB_EXP int AUXNew(int sample_rate, const char *auxpath);
AUXLIB_EXP void AUXDelete(int hAUX);
AUXLIB_EXP int AUXDef(int hAUX, const char* strIn);
AUXLIB_EXP int AUXEval(int hAUX, const char *strIn, double **buffer, int *length);
AUXLIB_EXP int AUXPlay(int hAUX, int DevID);
AUXLIB_EXP int AUXWavwrite(int hAUX, const char *filename);
AUXLIB_EXP const char *AUXGetErrMsg(void);
AUXLIB_EXP int AUXGetInfo(int hAUX, const char *name, void *output);
