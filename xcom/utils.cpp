#include "graffy.h" // this should come before the rest because of wxx820
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "showvar.h"
#include "histDlg.h"
#include "xcom.h"
#include "resource1.h"

extern xcom mainSpace;
extern vector<CWndDlg*> cellviewdlg;
extern CShowvarDlg mShowDlg;
extern CHistDlg mHistDlg;
extern unordered_map<string, CDebugDlg*> dbmap;
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

SIZE GetScreenSize()
{
	//auto nMonitors = GetSystemMetrics(SM_CMONITORS);
	//auto screenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
	//auto screenWidth2 = GetSystemMetrics(SM_CXMAXIMIZED);
	//auto screenWidth3 = GetSystemMetrics(SM_CXSCREEN);
	auto screenWidth4 = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	//auto screenHeight = GetSystemMetrics(SM_CYFULLSCREEN);
	//auto screenHeight2 = GetSystemMetrics(SM_CYMAXIMIZED);
	//auto screenHeight3 = GetSystemMetrics(SM_CYSCREEN);
	auto screenHeight4 = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	//HDC hdc = GetDC(GetConsoleWindow());
	//auto size_x = GetDeviceCaps(hdc, HORZSIZE);
	//auto size_y = GetDeviceCaps(hdc, VERTSIZE);
	return CSize(screenWidth4, screenHeight4);
}

static int writeINIs(const char *fname, char *estr, int fs)
{
	char errStr[256];
	if (!printfINI(errStr, fname, INI_HEAD_SRATE, "%d", fs)) { strcpy(estr, errStr); 	return 0; }
	if (!printfINI(errStr, fname, INI_HEAD_PLAYBLOCK, "%.1f", CAstSig::play_block_ms)) { strcpy(estr, errStr);	return 0; }
	if (!printfINI(errStr, fname, INI_HEAD_RECBLOCK, "%.1f", CAstSig::record_block_ms)) { strcpy(estr, errStr);	return 0; }
	if (!printfINI(errStr, fname, INI_HEAD_PLAYBYTES, "%d", CAstSig::play_bytes)) { strcpy(estr, errStr);	return 0; }
	if (!printfINI(errStr, fname, INI_HEAD_RECBYTES, "%d", CAstSig::record_bytes)) { strcpy(estr, errStr);	return 0; }
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

void closeXcom()
{
	// When the close button is pressed, i.e.,	CTRL_CLOSE_EVENT is posted, 
	// you have 5 seconds to finish all cleanup work
	// After that the Windows OS closes the console window and the application.
	// WinMain and the thread appear to be terminated when the application closes.
	// At that point, whatever was happening inside WinMain doesn't seem to matter.

	CAstSig *pcast = xscope.front();
	auto fs_session = pcast->GetFs();
	char estr[256], buffer[256];
	delete pcast->pEnv;

	CRect rt1, rt2, rt3;
	GetWindowRect(GetConsoleWindow(), &rt1);
	mShowDlg.GetWindowRect(rt2);
	mHistDlg.GetWindowRect(rt3);
	writeINIs(mainSpace.iniFile, estr, fs_session);
	int res = writeINI_pos(mainSpace.iniFile, estr, rt1, rt2, rt3);
	char dummy[256];
	string strRead;
	int fs;
	res = ReadINI(dummy, mainSpace.iniFile, INI_HEAD_SRATE, strRead);
	if (res > 0 && sscanf(strRead.c_str(), "%d", &fs) != EOF)
	{
		if (fs != fs_session)
			printfINI(dummy, mainSpace.iniFile, INI_HEAD_SRATE, "%d", fs_session);
	}
	string debugudfs("");
	for (unordered_map<string, CDebugDlg*>::iterator it = dbmap.begin(); it != dbmap.end(); it++)
	{
		debugudfs += it->second->fullUDFpath; debugudfs += "\r\n";
	}
	if (debugudfs.size() > 0)
	{
		if (!printfINI(estr, mainSpace.iniFile, "DEBUGGING UDFS", "%s", debugudfs.c_str())) strcpy(buffer, estr);
		unordered_map<string, CDebugDlg*>::iterator it = dbmap.begin();
		GetWindowRect(it->second->hParent->hDlg, &rt1);
		CString str;
		str.Format("%d %d %d %d", rt1.left, rt1.top, rt1.Width(), rt1.Height());
		if (!printfINI(estr, mainSpace.iniFile, "DEBUG VIEW POS", "%s", str.c_str())) { strcpy(buffer, estr);/*do something*/ }
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
	if (!printfINI(estr, mainSpace.iniFile, "DEBUG VIEW", "%d", debugview))
	{	//do something	
	}
	fclose(stdout);
	fclose(stdin);
}

BOOL CALLBACK AuxPathDlgProc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	string str;
	char pathPrev[4096], errstr[256];
	int res;
	switch (umsg)
	{
	case WM_INITDIALOG:
		str = xscope.front()->pEnv->path_delimited_semicolon();
		SetDlgItemText(hDlg, IDC_PATH, str.c_str());
		SetDlgItemText(hDlg, IDC_WD, xscope.front()->pEnv->AppPath.c_str());
		return 1;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			res = GetDlgItemText(hDlg, IDC_PATH, pathPrev, sizeof(pathPrev));
			str = xscope.front()->pEnv->path_delimited_semicolon();
			if (str != pathPrev)
			{
				vector<string> paths;
				str2vector(paths, pathPrev, ";\r\n \t");
				for (auto str : paths)
				{
					xscope.front()->pEnv->AddPath(str);
				}
				if (!printfINI(errstr, mainSpace.iniFile, "PATH", "%s", pathPrev)) {
					MessageBox(hDlg, "Path updated, but unable to update the ini file.", errstr, MB_OK);
					return 0;
				}
			}
			EndDialog(hDlg, 1);
			break;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		}
		return 1;
	}
	return 0;
}

string get_current_dir()
{
	size_t tbufferLen = MAX_PATH;
	char* tbuffer = new char[tbufferLen];
	tbuffer[0] = 0;
	DWORD count = GetCurrentDirectory(tbufferLen, tbuffer);
	while (count > tbufferLen)
	{
		tbufferLen *= 2;
		delete[] tbuffer;
		tbuffer = new char[tbufferLen];
		count = GetCurrentDirectory(tbufferLen, tbuffer);
	}
	string out = tbuffer;
	delete[] tbuffer;
	return out;
}