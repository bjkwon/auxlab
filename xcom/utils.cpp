#include "graffy.h" // this should come before the rest because of wxx820
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "showvar.h"
#include "histDlg.h"
#include "xcom.h"

extern vector<CWndDlg*> cellviewdlg;
extern char iniFile[256];
extern CShowvarDlg mShowDlg;
extern CHistDlg mHistDlg;
extern unordered_map<string, CDebugDlg*> dbmap;
extern xcom mainSpace;
extern uintptr_t hShowvarThread;
extern uintptr_t hHistoryThread;
extern uintptr_t hDebugThread2;

CWndDlg * Find_cellviewdlg(const char *name)
{ // Currently this finds only CVectorsheetDlg... Expand to search other CShowDlg
	char buf[256];
	if (!name) return NULL;
	vector<CWndDlg*>::iterator it;
	for (it = cellviewdlg.begin(); it != cellviewdlg.end(); it++)
	{
		GetWindowText((*it)->hDlg, buf, sizeof(buf));
		if (!strcmp(buf, name))
			return *it;
	}
	return NULL;
}

static int writeINIs(const char *fname, char *estr, int fs, const char *path)
{
	char errStr[256];
	if (!printfINI(errStr, fname, INI_HEAD_SRATE, "%d", fs)) { strcpy(estr, errStr); 	return 0; }
	if (!printfINI(errStr, fname, INI_HEAD_PLAYBLOCK, "%.1f", CAstSig::play_block_ms)) { strcpy(estr, errStr);	return 0; }
	if (!printfINI(errStr, fname, INI_HEAD_RECBLOCK, "%.1f", CAstSig::record_block_ms)) { strcpy(estr, errStr);	return 0; }
	if (!printfINI(errStr, fname, INI_HEAD_PLAYBYTES, "%d", CAstSig::play_bytes)) { strcpy(estr, errStr);	return 0; }
	if (!printfINI(errStr, fname, INI_HEAD_RECBYTES, "%d", CAstSig::record_bytes)) { strcpy(estr, errStr);	return 0; }
	if (!printfINI(errStr, fname, "PATH", "%s", path)) { strcpy(estr, errStr); return 0; }
	return 1;
}

static int writeINI_pos(const char *fname, char *estr, CRect rtMain, CRect rtShowDlg, CRect rtHistDlg)
{
	char errStr[256];
	CString str;
	str.Format("%d %d %d %d", rtMain.left, rtMain.top, rtMain.Width(), rtMain.Height());
	if (!printfINI(errStr, fname, "WINDOW POS", "%s", str.c_str())) { strcpy(estr, errStr); return 0; }
	str.Format("%d %d %d %d", rtShowDlg.left, rtShowDlg.top, rtShowDlg.Width(), rtShowDlg.Height());
	if (!printfINI(errStr, fname, "VAR VIEW POS", "%s", str.c_str())) { strcpy(estr, errStr); return 0; }
	str.Format("%d %d %d %d", rtHistDlg.left, rtHistDlg.top, rtHistDlg.Width(), rtHistDlg.Height());
	if (!printfINI(errStr, fname, "HIST POS", "%s", str.c_str())) { strcpy(estr, errStr); return 0; }
	return 1;
}

void closeXcom(const char *EnvAppPath, int fs, const char *AppPath)
{
	// When the close button is pressed, i.e.,	CTRL_CLOSE_EVENT is posted, 
	// you have 5 seconds to finish all cleanup work
	// After that the Windows OS closes the console window and the application.
	// WinMain and the thread appear to be terminated when the application closes.
	// At that point, whatever was happening inside WinMain doesn't seem to matter.

	CAstSig *pcast = xscope.front();
	char estr[256], buffer[256];
	const char* pt = strstr(EnvAppPath, AppPath);
	string pathnotapppath;
	size_t loc;
	if (pt != NULL)
	{
		pathnotapppath.append(EnvAppPath, EnvAppPath - pt);
		string  str(pt + strlen(AppPath));
		loc = str.find_first_not_of(';');
		pathnotapppath.append(pt + strlen(AppPath) + loc);
	}
	else
		pathnotapppath.append(EnvAppPath);
	int res = writeINIs(iniFile, estr, fs, pathnotapppath.c_str());
	delete pcast->pEnv;

	CRect rt1, rt2, rt3;
	GetWindowRect(GetConsoleWindow(), &rt1);
	mShowDlg.GetWindowRect(rt2);
	mHistDlg.GetWindowRect(rt3);
	res = writeINI_pos(iniFile, estr, rt1, rt2, rt3);

	string debugudfs("");
	for (unordered_map<string, CDebugDlg*>::iterator it = dbmap.begin(); it != dbmap.end(); it++)
	{
		debugudfs += it->second->fullUDFpath; debugudfs += "\r\n";
	}
	if (debugudfs.size() > 0)
	{
		if (!printfINI(estr, iniFile, "DEBUGGING UDFS", "%s", debugudfs.c_str())) strcpy(buffer, estr);
		unordered_map<string, CDebugDlg*>::iterator it = dbmap.begin();
		GetWindowRect(it->second->hParent->hDlg, &rt1);
		CString str;
		str.Format("%d %d %d %d", rt1.left, rt1.top, rt1.Width(), rt1.Height());
		if (!printfINI(estr, iniFile, "DEBUG VIEW POS", "%s", str.c_str())) { strcpy(buffer, estr);/*do something*/ }
	}
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	vector<string> in;
	sprintf(buffer, "//\t[%02d/%02d/%4d, %02d:%02d:%02d] AUXLAB closes------", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
	in.push_back(buffer);
	mainSpace.LogHistory(in);
	if (hShowvarThread != NULL) PostThreadMessage(GetWindowThreadProcessId(mShowDlg.hDlg, NULL), WM__ENDTHREAD, 0, 0);
	if (hHistoryThread != NULL) PostThreadMessage(GetWindowThreadProcessId(mHistDlg.hDlg, NULL), WM__ENDTHREAD, 0, 0);
	int debugview(0);
	if (hDebugThread2)
		debugview = 1;
	else
	{
		unordered_map<string, CDebugDlg*>::iterator it = dbmap.begin();
		if (it != dbmap.end())
			PostThreadMessage(GetWindowThreadProcessId((it->second)->hDlg, NULL), WM__ENDTHREAD, 0, 0);
	}
	if (!printfINI(estr, iniFile, "DEBUG VIEW", "%d", debugview))
	{	//do something	
	}
	// Investigate--when there's an error during a udf and exit the application the delete *it causes a crash. 8/15/2020
//	for (vector<CAstSig*>::iterator it = xscope.begin()+1; it != xscope.end(); it++)
//		delete *it;
	fclose(stdout);
	fclose(stdin);
}

