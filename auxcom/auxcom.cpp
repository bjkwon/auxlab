// auxcom.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "graffy.h" // this should come before the rest because of wxx820
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
double CAstSig::play_block_ms = 300;
double CAstSig::record_block_ms = 300;
short CAstSig::play_bytes = 2;
short CAstSig::record_bytes = 2;

// These are not used, but need to be declared
#include <condition_variable>
HANDLE hEventRecordingReady; 
condition_variable cv_closeFig; 
HWND hwnd_AudioCapture = NULL;
mutex mtx4PlotDlg;


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
	CAstSigEnv::AppPath = mainSpace.AppPath;

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

	char modname[256];
	try {
		if (argc > 1)
		{
			string emsg;
			strcpy_s(modname, sizeof(modname), argv[1]);
			for (int k = 2; k < argc; k++)
			{
				if (k==2) strcat_s(modname, sizeof(modname), "(");
				strcat_s(modname, sizeof(modname), "\"");
				strcat_s(modname, sizeof(modname), argv[k]);
				strcat_s(modname, sizeof(modname), "\"");
				if (k==argc-1) strcat_s(modname, sizeof(modname), ")");
				else strcat_s(modname, sizeof(modname), ",");
			}
			pglobalEnv->InitBuiltInFunctions();
			CAstSig cast(pglobalEnv);
			cast.u.application = "auxcom";
			xscope.push_back(&cast);
			initGraffy(&cast);
			char* str_autocorrect = (char*)calloc(strlen(modname) * 2, 1);

			if (!(cast.xtree = cast.parse_aux(modname, emsg, str_autocorrect)))
			{
				throw emsg.c_str();
			}
			cast.statusMsg.clear();
			cast.Compute();
			delete[] str_autocorrect;
			res = (int)cast.Sig.buf[0];
		}
	}
	catch (const char *error)
	{
		if (strncmp(error, "Invalid", strlen("Invalid")))
			cout << "ERROR: " << error << endl;
		else
			cout << error << endl;
	}
	delete pglobalEnv;
	return res;
}
