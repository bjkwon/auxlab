#include "sigproc.h"

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
	string arg;
	bool arg_ending_back_slash = false;
	if (p)
	{
		past->Sig.Reset(1);
		past->Compute(p);
		if (!past->Sig.IsString())
			throw CAstException(FUNC_SYNTAX, *past, p).proc("argument must be a string.");
		arg = past->Sig.string();
	}
	if (arg.empty()) arg += ".";
	bool dot_or_dotdot = false;
	if (arg == "." || "..") dot_or_dotdot = true;
	if (arg.back() == '\\')
	{
		arg_ending_back_slash = true;
		arg += ".";
	}
	if (arg == "..") arg += "\\.";
	WIN32_FIND_DATA ls;
	HANDLE hFind = FindFirstFile(arg.c_str(), &ls);
	char curDirPath[MAX_PATH]; // where the FindFirstFile operation is based

	char drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH], ext[MAX_PATH], pathonly[MAX_PATH] = {};
	_splitpath(arg.c_str(), drive, dir, fname, ext);
	// if dir begins with \ --> absolute path 
	//          if drive is empty, get current drive (from pEnv->CAstSigEnv::AppPath) and make curDirPath
	//          otherwise, make curDirPath from drive, dir
	// otherwise (relative path)
	//          if both drive and dir are empty, curDirPath is pEnv->CAstSigEnv::AppPath
	//          if only drive is empty, get current drive (from pEnv->CAstSigEnv::AppPath) followed by dir
	//          if only dir is empty, drive followed by current directory of the folder using GetFullPathName
	// In addition, if arg ends with \ and ls indicates a file, 
	// make that statement illegal (that's the behavior in the Windows cmd).
	// for example, dir ("name\") is called when name is a file, not a directory
	auto bb = ls.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	bool path3;
	if (hFind == INVALID_HANDLE_VALUE || (arg_ending_back_slash  && !(ls.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)))
	{
		// no file found. return NULL
		past->Sig.Reset(1);
		return;
	}	
	auto specified_arg_is_directory = ls.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && !dot_or_dotdot;
	if (dir[0] && dir[0] == '\\') // absolute path
	{
		if (!drive[0]) // drive empty
		{
			_splitpath(past->pEnv->CAstSigEnv::AppPath.c_str(), curDirPath, NULL, NULL, NULL);
			strcat(curDirPath, dir);
		}
		else
		{
			strcpy(curDirPath, drive);
			strcat(curDirPath, dir);
		}
	}
	else // relative path
	{
		if (!drive[0]) // drive empty
		{
			if (!dir[0]) // dir empty
			{
				strcpy(curDirPath, past->pEnv->CAstSigEnv::AppPath.c_str());
				if (curDirPath[strlen(curDirPath) - 1] != '\\') strcat(curDirPath, "\\");
			}
			else
			{
				char currentDir[MAX_PATH];
				_splitpath(past->pEnv->CAstSigEnv::AppPath.c_str(), curDirPath, currentDir, NULL, NULL);
				strcat(curDirPath, currentDir);
			}
		}
		else
		{
			path3 = true;
			if (!dir[0]) // dir empty
			{
				strcpy(curDirPath, drive);
				char** pt = { NULL };
				GetFullPathName(drive, sizeof(dir), dir, pt);
				strcpy(curDirPath, dir);
			}
		}
	}
	while (strcmp(ls.cFileName, ".") && ls.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{ // the first find is a directory, need to update curDirPath
		if (specified_arg_is_directory)
		{
			strcat(curDirPath, ls.cFileName);
			strcat(curDirPath, "\\");
		}
		arg += "\\*.*";
		hFind = FindFirstFile(arg.c_str(), &ls);
	}
	past->Sig.Reset();
	bool pathonly_corrected_dotdot = false;
	do {
		_splitpath(ls.cFileName, NULL, NULL, fname, ext);
		if (strcmp(ls.cFileName, ".") && strcmp(ls.cFileName, ".."))
		{
			if (!pathonly_corrected_dotdot && arg.size() > 1 && arg.substr(0, 2) == "..")
			{
				// if arg begins with dotdot .. , curDirPath should be go up by one
				if (curDirPath[strlen(curDirPath) - 1] == '\\') curDirPath[strlen(curDirPath) - 1] = 0;
				auto pt = strrchr(curDirPath, '\\');
				if (pt) *(pt+1) = 0;
				pathonly_corrected_dotdot = true;
			}
			update_dir_info2CVar(past->Sig, ls, fname, ext, curDirPath);
		}
	} while (FindNextFile(hFind, &ls));
}
