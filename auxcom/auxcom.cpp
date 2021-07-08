// auxcom.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <vector>
#include "sigproc.h"
#include "..\xcom\xcom.h"

#include "bjcommon_win.h"

#define INI_SRATE "SAMPLE RATE"


/* auxcom launches the AUXLAB enviromnent with the default parameters in auxlab.ini
*/
xcom mainSpace;

int main(int argc, char **argv)
{
	char estr[256], fname[256], fullmoduleName[256], module[256] = {};
	char drive[256], dir[256], ext[256];
	string strRead;
	int val, fs;
	if (argc>1)
		strcpy_s(module, argv[1]);

	HMODULE h = GetModuleHandle(NULL);
	GetModuleFileName(h, fullmoduleName, MAX_PATH);
	_splitpath(fullmoduleName, drive, dir, fname, ext);
	sprintf_s(mainSpace.AppPath, "%s%s", drive, dir);
	sprintf_s(mainSpace.iniFile, "%s%s.ini", mainSpace.AppPath, fname);

	int res = ReadINI(estr, fname, INI_HEAD_SRATE, strRead);
	if (res > 0 && sscanf(strRead.c_str(), "%d", &val) != EOF && val > 500)
		fs = val;
	else
		fs = CAstSig::DefaultFs;
	CAstSigEnv* pglobalEnv = new CAstSigEnv(fs);
	if (ReadINI(estr, mainSpace.iniFile, "PATH", strRead) > 0)
		pglobalEnv->SetPath(strRead.c_str());
	else
		cout << "PATH information not available in " << mainSpace.iniFile << endl;






}
