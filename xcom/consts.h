// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: auxlab
// Main Application. Based on Windows API  
// 
// 
// Version: 1.495
// Date: 12/13/2018
// 
#define HISTORY_FILENAME	"_history"

#define LOGHISTORY(MSG) { FILE* __fp_ = fopen(mHistDlg.logfilename,"at"); if (__fp_) { fprintf(__fp_,"%s\n", (MSG)); fclose(__fp_); } else { char __temp[256]; sprintf(__temp, "NOT FOUND: %s", mHistDlg.logfilename); ::MessageBox(mHistDlg.hDlg, __temp, "LOGHISTORY", 0); } }

#define PRINTF_WIN(MSG) WriteFile(hStdout, (MSG), strlen((MSG)), &dw, NULL);


#define DISPLAYLIMIT 10
#define TEXTLINEDISPLAYLIMIT 30

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HMODULE_THIS  ((HINSTANCE)&__ImageBase)

#define WM__ENDTHREAD	WM_APP+342

