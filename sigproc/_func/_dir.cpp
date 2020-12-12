#include "sigproc.h"

void _dir(CAstSig* past, const AstNode* pnode, const AstNode* p, string& fnsigs)
{
	past->Sig.Reset(1);
	past->Compute(p);
	if (!past->Sig.IsString())
		throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "argument must be a string.");
	string arg = past->Sig.string();
	char drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH], ext[MAX_PATH], pathonly[MAX_PATH] = {};
	_splitpath(arg.c_str(), drive, dir, fname, ext);
	sprintf(pathonly, "%s%s", drive, dir);
	if (strlen(fname) == 0 && strlen(ext) == 0)
		arg += "\\*.*";
#ifdef _WINDOWS
	WIN32_FIND_DATA ls;
	HANDLE hFind = FindFirstFile(arg.c_str(), &ls);
	if (hFind == INVALID_HANDLE_VALUE)
		//	if ((hFile = _findfirst(, &c_file)) == -1L)
		throw CAstException(FUNC_SYNTAX, *past, p).proc(fnsigs, "No files in the specified directory");
	else
	{
		past->Sig.Reset();
		do {
			if (!pathonly[0])
				_splitpath(ls.cFileName, drive, dir, fname, ext);
			else
				_splitpath(ls.cFileName, NULL, NULL, fname, ext);
			char fullname[256];
			CVar tp;
			tp.strut["name"] = string(fname);
			tp.strut["ext"] = string(ext);
			if (fname[0] != '.' || fname[1] != '\0')
			{
				if (pathonly[0])
					tp.strut["path"] = string(pathonly);
				else
				{
					char* pt = strstr(fullname, fname);
					if (pt) *pt = 0;
					tp.strut["path"] = string(fullname);
				}
				tp.strut["bytes"] = CVar((double)(ls.nFileSizeHigh * ((uint64_t)MAXDWORD + 1) + ls.nFileSizeLow));
				FILETIME ft = ls.ftLastWriteTime;
				SYSTEMTIME lt;
				FileTimeToSystemTime(&ft, &lt);
				sprintf(fullname, "%02d/%02d/%4d, %02d:%02d:%02d", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
				tp.strut["date"] = string(fullname);
				CVar b(double(ls.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
				tp.strut["isdir"] = b;
				past->Sig.appendcell(tp);
			}
		} while (FindNextFile(hFind, &ls));
	}
#elif
#endif
}
