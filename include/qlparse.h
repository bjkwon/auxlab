#pragma once
#include <vector>
#include <string>

__declspec (dllexport) std::vector<std::string> qlparse(const char* in, std::string &errmsg);

