// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: qlparse
// Library to parse a directory string
// 
// 
// Version: 1.495
// Date: 12/13/2018
// 
#pragma once
#include <vector>
#include <string>

__declspec (dllexport) std::vector<std::string> qlparse(const char* in, std::string &errmsg);

