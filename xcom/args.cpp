// AUXLAB
//
// Copyright (c) 2009-2020 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: auxlab
// Main Application. Based on Windows API
//
//
// Version: 1.709
// Date: 7/7/2021
//
#include "graffy.h" // this should come before the rest because of wxx820
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <vector>
#include "sigproc.h"
#include "audstr.h"
#include "resource1.h"
#include "showvar.h"
#include "histDlg.h"
#include "consts.h"
#include "xcom.h"
#include "echo.h"
#include "utils.h"
#include "wavplay.h"
#include "qlparse.h"

#include <condition_variable>
#include <mutex>
#include <thread>

/* 
* -m    module
* -s    settings
* --debug
* 
*/

int xcom::args(const vector<string>& str)
{
	return 1;
}