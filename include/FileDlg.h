#ifndef CFILEDLG
#define CFILEDLG

#include "windows.h"

class CFileDlg
{
public:
	OPENFILENAME ofn;
	void InitFileDlg (HWND hwnd, HINSTANCE hInst, const char *initDir);
	int FileOpenDlg (LPSTR pstrFileName, LPSTR pstrTitleName, LPSTR szFilter, LPSTR lpstrDefExt);
	int FileSaveDlg (LPSTR pstrFileName, LPSTR pstrTitleName, LPSTR szFilter, LPSTR lpstrDefExt);
	CFileDlg(void);
	char LastPath[MAX_PATH];
	char InitDir[MAX_PATH];
};

#endif //CFILEDLG