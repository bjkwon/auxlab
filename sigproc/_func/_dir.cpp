#include "sigproc.h"
#include <direct.h>

/* Use cases:
1)  dir
2)  dir("")
3)  dir("*.wav")
4)  dir("str")
5)  dir("str0\str")
6)  dir("drive:str")
   
Cases 1, 2, 3, 4, 5: Use path from past->pEnv->CAstSigEnv::AppPath.c_str() 
     cases 1, 2: dir [path]\*.*
	 cases 3, 4, 5: dir [path]\*.wav   dir [path]\str

Case 4, 5, 6: if str indicates a directory, dir should be applied to that directory,
           if str indicates not a directory, dir is only for that file

Test cases
dir
dir("")
dir("*.wav")
dir("str") when str doesn't exists
dir("str") when str exists as a directory
dir("str") when str exists as a file
dir("str\") when str exists as a directory
dir("str\") when str doesn't exist as a directory
dir("str\*.wav") str exists as a directory
dir("str\*.wav") str doesn't exists as a directory
dir("X:str") drive X doesn't exist
dir("C:str") drive C exists, str exists as a directory from pwd
dir("C:str") drive C exists, str exists as a file at pwd
dir("C:str") drive C exists, str doesn't exists as a directory or file from pwd

*/

static void update_dir_info2CVar(CVar& out, const WIN32_FIND_DATA& ls, const char *fname, const char* ext, const char* pathonly)
{
	CVar tp;
	tp.strut["name"] = string(fname);
	tp.strut["ext"] = string(ext);
	char fullname[256];
	if (pathonly[0])
		tp.strut["path"] = string(pathonly);
	else
	{
		char* pt = strstr(fullname, fname);
		if (pt) *pt = 0;
		if (fullname[0])
			tp.strut["path"] = string(fullname);
		else
			tp.strut["path"] = CAstSigEnv::AppPath;
	}
	tp.strut["bytes"] = CVar((double)(ls.nFileSizeHigh * ((uint64_t)MAXDWORD + 1) + ls.nFileSizeLow));
	FILETIME ft = ls.ftLastWriteTime;
	SYSTEMTIME lt;
	FileTimeToSystemTime(&ft, &lt);
	sprintf(fullname, "%02d/%02d/%4d, %02d:%02d:%02d", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
	tp.strut["date"] = string(fullname);
	CVar b(double(ls.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? 1. : 0.));
	tp.strut["isdir"] = b;
	out.appendcell(tp);
}

void _dir(CAstSig* past, const AstNode* pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	string arg0, arg;
	bool arg_ending_back_slash = false;
	if (p)
	{
		past->Sig.Reset(1);
		past->Compute(p);
		if (!past->Sig.IsString())
			throw CAstException(FUNC_SYNTAX, *past, p).proc("argument must be a string.");
		arg0 = past->Sig.string();
	}
	char dir_org[256] = { 0 };
	char dir_new[256] = { 0 };
	DWORD count = GetCurrentDirectory(256, dir_org);
	if (dir_org[strlen(dir_org) - 1] != '\\') strcat(dir_org, "\\");
	//First, is the argument a file or directory
	WIN32_FIND_DATA ls;
	arg = arg0;
	if (arg0.empty()) arg = ".";
	if (arg.back() == '\\')
	{
		arg_ending_back_slash = true;
		arg += "*.*";
	}
	if (arg == "..") arg += "\\*.*";
	HANDLE hFind = FindFirstFile(arg.c_str(), &ls);
	arg = arg0;
	if (hFind != INVALID_HANDLE_VALUE)
	{
		auto specified_arg_is_directory = ls.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		if (specified_arg_is_directory)
		{
			auto cx = chdir(arg.c_str());
			if (!cx) {
				char tbuf[256] = { 0 };
				count = GetCurrentDirectory(256, tbuf);
				strcpy(dir_new, tbuf);
				if (dir_new[strlen(dir_new) - 1] != '\\') strcat(dir_new, "\\");
				arg = "*.*";
			}
		}
	}
	else
	{
		// no file found. return NULL
		past->Sig.Reset(1);
		return;
	}
	hFind = FindFirstFile(arg.c_str(), &ls);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		char drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH], ext[MAX_PATH], pathonly[MAX_PATH] = {};
		past->Sig.Reset();
		do {
			auto specified_arg_is_directory = ls.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
			if (specified_arg_is_directory && !strcmp(ls.cFileName, ".")) continue;
			if (specified_arg_is_directory && !strcmp(ls.cFileName, "..")) continue;
			_splitpath(ls.cFileName, drive, dir, fname, ext);
			if (dir_new[0])
				update_dir_info2CVar(past->Sig, ls, fname, ext, dir_new);
			else
				update_dir_info2CVar(past->Sig, ls, fname, ext, dir_org);
		} while (FindNextFile(hFind, &ls));
	}
	auto cx = chdir(dir_org);
}
