#include <windows.h>
#include <string>
#include "resource.h"

HINSTANCE hInst;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInst = hModule;
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

static int counter = 0;
//DO SOMETHING 9/18/2018
__declspec (dllexport) int ReadAUXP(int id, std::string &out)
{
	HRSRC res = FindResource(hInst, MAKEINTRESOURCE(id), RT_RCDATA);
	if (!res) return -1;
	HGLOBAL res_GL = LoadResource(hInst, res);
	if (!res_GL) return -2;
	LPVOID pblock = LockResource(res_GL);
	DWORD dw = SizeofResource(hInst, res);

	char *buffer = new char[dw + 1];
	memcpy(buffer, pblock, dw);
	buffer[dw] = 0;
	out = buffer;
	delete[] buffer;
	return (int)out.size();
}