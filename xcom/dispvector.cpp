// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: auxlab
// Main Application. Based on Windows API  
// 
// 
// Version: 1.499
// Date: 3/11/2019
// 
#include "wxx_wincore.h" // Win32++ 8.2. This must be placed prior to <windows.h> to avoid including winsock.h

#include <process.h>
#include "showvar.h"
#include "resource1.h"
#include "bjcommon.h"
#include "bjcommon_win.h"
#include "audstr.h"
#include "xcom.h"

extern CWndDlg wnd;
extern CShowvarDlg mShowDlg;
extern char udfpath[4096];
extern vector<CWndDlg*> cellviewdlg;
extern vector<UINT> exc;

map<string, CRect> dlgpos;
ostringstream outstream_complex(complex<double> cval); 

BOOL CALLBACK showvarDlgProc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam);
int MoveDlgItem(HWND hDlg, int id, CRect rt, BOOL repaint);

BOOL CALLBACK vectorsheetDlg (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	// This calls original OnInitDialog(), setting up hinst, AppPath, etc.. Derived OnInitDialog is to be called separately after CreateWindow
//	if (umsg == WM_INITDIALOG) return wnd.OnInitDialog((HWND)(wParam), lParam);

	CVectorsheetDlg *cvDlg(NULL);
	for (vector<CWndDlg*>::iterator it = cellviewdlg.begin(); it != cellviewdlg.end(); it++)
		if (!(*it)->hDlg || (*it)->hDlg == hDlg)
		{
			cvDlg = (CVectorsheetDlg *)*it; break;
		}
	//spyWM(hDlg, umsg, wParam, lParam, "c:\\temp\\rec", exc, "[vectorsheetDlg]");
	//FILE *fp = fopen("c:\\temp\\rec", "at");
	//fprintf(fp, "cvDlg=%x\n", cvDlg);
	//fclose(fp);
	if (cvDlg!=NULL)
	{
		switch (umsg)
		{
		chHANDLE_DLGMSG(hDlg, WM_INITDIALOG, cvDlg->OnInitDialog);
		chHANDLE_DLGMSG (hDlg, WM_MOVE, cvDlg->OnMove);
		chHANDLE_DLGMSG (hDlg, WM_SIZE, cvDlg->OnSize);
		chHANDLE_DLGMSG(hDlg, WM_DESTROY, cvDlg->OnDestroy);
		chHANDLE_DLGMSG (hDlg, WM_CLOSE, cvDlg->OnClose);
		chHANDLE_DLGMSG (hDlg, WM_SHOWWINDOW, cvDlg->OnShowWindow);
		chHANDLE_DLGMSG (hDlg, WM_COMMAND, cvDlg->OnCommand);
		case WM_NOTIFY:
			cvDlg->OnNotify(hDlg, (int)(wParam), lParam);
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	else
		return FALSE; // this is in fact impossible case, but just keep it....
}

CVectorsheetDlg::CVectorsheetDlg(CWndDlg* hPar)
	:id1(0), jd1(0), nRows(20), nCols(10)
{
	dwUser = 8383;
	hParent = hPar;
	cellviewdlg.push_back(this);
	sigvector = NULL;
	psig = NULL;
	clean_psig = false;
	label[0] = 0;
	memset(&LvItem, 0, sizeof(LvItem));
	memset(&LvCol, 0, sizeof(LvCol));
	LvItem.mask = LVIF_TEXT;   // Text Style
	LvItem.cchTextMax = 256; // Max size of text
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
}

CVectorsheetDlg::~CVectorsheetDlg(void)
{
	if (clean_psig)
		delete psig;
	for (vector<CWndDlg*>::iterator it = cellviewdlg.begin(); it != cellviewdlg.end(); it++)
	{
		if (this == *it)
		{
			cellviewdlg.erase(it);
			break;
		}
	}
}

BOOL CVectorsheetDlg::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	CWndDlg::OnInitDialog(hwndFocus, lParam);
//	SetWindowText((char*)lParam);
	hList1 = GetDlgItem(IDC_LISTVECTOR);
	LRESULT res;
	HWND h = GetDlgItem(IDC_LISTVECTOR);
	res = ::SendMessage(hList1, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
//	res = SendDlgItemMessage(IDC_LISTVECTOR, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

	CRect rt0, rt1, rt2;
	GetDlgItemRect(IDC_EDIT_ID1, rt1);
	head_height = rt1.bottom;
	WORD sss1 = HIWORD(ListView_ApproximateViewRect(hList1, -1, -1, 1));
	WORD sss2 = HIWORD(ListView_ApproximateViewRect(hList1, -1, -1, 2));
	slope = sss2 - sss1;
	offset = sss1 - slope;
	return TRUE;
}

void CVectorsheetDlg::Init2(const char *windowtitle)
{
	bool old = width.size() > 0;
	SendDlgItemMessage(IDC_LISTVECTOR, LVM_DELETEALLITEMS, 0, 0);
	while (SendDlgItemMessage(IDC_LISTVECTOR, LVM_DELETECOLUMN, 0, 0))
		;
	SetWindowText(windowtitle);
	CRect rt, rPar;
	GetWindowRect(&rt);
	hParent->GetWindowRect(&rPar);
	CRect rt0;
	if (old)
	{
		rt0 = rt;
		width.clear();
	}
	else
	{
		//rt0 is the initial rect for the dlgbox set relative to the showvar dlg.
		rt0.SetRect(rPar.right + 10, (rPar.top * 2 + rPar.bottom) / 3, rPar.right + 10 + 145, (rPar.top * 2 + rPar.bottom) / 3 + (rt.bottom - rt.top));
		for (map<string, CRect>::reverse_iterator it = dlgpos.rbegin(); it != dlgpos.rend(); it++)
			if (rt0.left == it->second.left && rt0.top == it->second.top) { rt0.OffsetRect(20, 32); it = dlgpos.rbegin(); }
	}

	// based on psig, create columns and set header text
	HDC	hdc = GetDC(hList1);
	CSize sz;
	char buf[256];
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	if (psig->GetType() == CSIG_TSERIES)
		strcpy(buf, "time");
	else if (psig->nGroups > 1)
		strcpy(buf, "mtx");
	else
		strcpy(buf, "idx");
	LvCol.pszText = buf;
	GetTextExtentPoint32(hdc, buf, (int)strlen(buf), &sz);
	LvCol.cx = sz.cx + 6;
	width.push_back(LvCol.cx);
	SendDlgItemMessage(IDC_LISTVECTOR, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol);
	ReleaseDC(NULL, hdc);

	nCols = min(nCols, psig->Len());
	//Column titles are set in FillUp. Here, columns are inserted with blank and width is set at 20.
	int newWidthrequired(width.front());
	LvCol.pszText = "";
	for (unsigned int k = 0; k < nCols; k++)
	{
		newWidthrequired += 20;
		width.push_back(20);
		SendDlgItemMessage(IDC_LISTVECTOR, LVM_INSERTCOLUMN, k+1, (LPARAM)&LvCol);
	}
	rt0.right = rt0.left + newWidthrequired;
	MoveWindow(&rt0, TRUE);
}

void CVectorsheetDlg::OnSize(UINT state, int cx, int cy)
{
	RECT rt;
	GetWindowRect(&rt);
	char buf[256];
	GetWindowText(buf, sizeof(buf));
	dlgpos[buf] = rt;
	int count = ListView_GetItemCount(hList1);
	DWORD reg1 = ListView_ApproximateViewRect(hList1, -1, -1, count);
	WORD wid = LOWORD(reg1);
	WORD hi = HIWORD(reg1);

	WORD sss = HIWORD(ListView_ApproximateViewRect(hList1, -1, -1, 1));
	WORD sss2 = HIWORD(ListView_ApproximateViewRect(hList1, -1, -1, 2));
	WORD colwidth1 = LOWORD(ListView_ApproximateViewRect(hList1, -1, -1, 1));
	WORD colwidth2 = LOWORD(ListView_ApproximateViewRect(hList1, -1, -1, -1));

	// if cx increased enough to accommodate another column, call FillUp
	int totalwidth (0);
	for (auto& n : width)	totalwidth += n;
	if (cx - totalwidth > width.back() && nCols < psig->Len())
	{
		nCols++;
		SendDlgItemMessage(IDC_LISTVECTOR, LVM_INSERTCOLUMN, nCols, (LPARAM)&LvCol);
		width.push_back(0);
		FillUp();
	}
	else if (totalwidth - cx > width.back())
	{
		SendDlgItemMessage(IDC_LISTVECTOR, LVM_DELETECOLUMN, nCols, (LPARAM)&LvCol);
		nCols--;
		width.pop_back();
		FillUp();
	}
	// The minimum height to display all items
	int minHeight = slope * nRows + offset + slope ; // add when all can be displayed
	//if cy exceeds this, add item
	unsigned int _nRows = (cy - offset) / slope;
	unsigned int sig_nRow = psig->GetType() == CSIG_TSERIES ? psig->CountChains() : psig->nGroups;
	if (_nRows > nRows && nRows < sig_nRow)
	{
		nRows = _nRows;
		FillUp();
	}
	else if (_nRows <= nRows - 1)
	{
		nRows = _nRows + 1;
		FillUp();
	}
}
void CVectorsheetDlg::ResetDraw(unsigned int p, unsigned int q)
{
	SendDlgItemMessage(IDC_LISTVECTOR, LVM_DELETEALLITEMS, 0, 0);
	while (SendDlgItemMessage(IDC_LISTVECTOR, LVM_DELETECOLUMN, 0, 0)) {};
	
//	nRows = min(20, p);
//	nCols = min(10, q);
//	id1 = jd1 = 0;
	width.clear();

	//Same as the later part of Init2()
	HDC	hdc = GetDC(hList1);
	CSize sz;
	char buf[256];
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	if (psig->GetType() == CSIG_TSERIES)
		strcpy(buf, "time(ms)");
	else if (psig->nGroups > 1)
		strcpy(buf, "mtx");
	else
		strcpy(buf, "idx");
	LvCol.pszText = buf;
	GetTextExtentPoint32(hdc, buf, (int)strlen(buf), &sz);
	LvCol.cx = sz.cx + 6;
	width.push_back(LvCol.cx);
	SendDlgItemMessage(IDC_LISTVECTOR, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol);
	ReleaseDC(NULL, hdc);

//	nCols = min(nCols, psig->Len());
	//Column titles are set in FillUp. Here, columns are inserted with blank and width is set at 20.
	int newWidthrequired(width.front());
	LvCol.pszText = "";
	for (unsigned int k = 0; k < nCols; k++)
	{
		newWidthrequired += 20;
		width.push_back(20);
		SendDlgItemMessage(IDC_LISTVECTOR, LVM_INSERTCOLUMN, k + 1, (LPARAM)&LvCol);
	}
	ReleaseDC(NULL, hdc);
}


void CVectorsheetDlg::FillUp(vector<CVar *> *_sigvector)
{
	SendDlgItemMessage(IDC_LISTVECTOR, LVM_DELETEALLITEMS, 0, 0);
	while (SendDlgItemMessage(IDC_LISTVECTOR, LVM_DELETECOLUMN, 0, 0)) {};
	SendDlgItemMessage(IDC_LISTVECTOR, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, 0);	
	int k(0), width[] = { 40, 120, };
	char buf[256];
	LvCol.cx = width[0];	LvCol.pszText = "index";
	SendDlgItemMessage(IDC_LISTVECTOR, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol);
	LvCol.cx = width[1];	LvCol.pszText = "value";
	SendDlgItemMessage(IDC_LISTVECTOR, LVM_INSERTCOLUMN, 1, (LPARAM)&LvCol);
	for (vector<CVar*>::iterator it = _sigvector->begin(); it != _sigvector->end(); it++, k++)
	{
		LvItem.iItem = k; 
		LvItem.iSubItem = 0;
		LvItem.pszText = itoa(k + 1, buf, 10);
		SendDlgItemMessage(IDC_LISTVECTOR, LVM_INSERTITEM, 0, (LPARAM)&LvItem);
		LvItem.iSubItem = 1;
		if ((*it)->IsGO())
		{
			if ((*it)->bufBlockSize == 1) // figure window with showvar-Enter
				sprintf(buf, "%s", (*it)->string().c_str());
			else
				sprintf(buf, "%14.0f", (*it)->value());
			LvCol.pszText = buf;
			SendDlgItemMessage(IDC_LISTVECTOR, LVM_SETITEM, 0, (LPARAM)&LvItem);
		}
		else // cannot happen
			MessageBox("Internal Error 47463");
	}
	int res = (int)SendDlgItemMessage(IDC_LISTVECTOR, LVM_APPROXIMATEVIEWRECT, -1, MAKELONG(-1, -1));
	WORD height = HIWORD(res);
	CRect rtDlg, rtList;
	GetWindowRect(rtDlg);
	int totalwidth(0);
	for (auto& n : width)	totalwidth += n;
	rtDlg.right = rtDlg.left + totalwidth + 20;
	rtDlg.bottom = rtDlg.top + height + 35;
	MoveWindow(rtDlg);
}

void CVectorsheetDlg::AppendRow(char *title, int rowID)
{
	CSize sz;
	LvItem.iItem = LvItem.iSubItem = 0;
	LvItem.pszText = title;
	SendDlgItemMessage(IDC_LISTVECTOR, LVM_INSERTITEM, 0, (LPARAM)&LvItem);
}

int CVectorsheetDlg::SetCell(unsigned int row, unsigned int col, char *buf, bool large)
{
	CSize sz;
	LvItem.iItem = row;
	LvItem.iSubItem = col;
	LvCol.pszText = buf;
	SendDlgItemMessage(IDC_LISTVECTOR, LVM_SETITEM, 0, (LPARAM)&LvItem);
//	if (large)	strcat(buf, "_");
//	GetTextExtentPoint32(hdc, buf, (int)strlen(buf), &sz);
//	int offset = large ? 7 : 3;
//	width[col] = max(sz.cx + offset, width[col]);
	return width[col];
}

void CVectorsheetDlg::SetColumnTitles(vector<size_t> &nChars, HDC hdc, unsigned int m1, unsigned m2, char *title)
{
	char buf[64];
	LVCOLUMN colprop;
	colprop.mask = LVCF_TEXT;
	colprop.pszText = buf;
	colprop.cchTextMax = 64;
	for (unsigned int q = m1; q <= m2; q++)
	{
		if (title)
			strcpy(colprop.pszText, title);
		else
		{
//			nDigits = width[q - m1 + 1] / unit;
//			sprintf(prec, "%%%dd", nDigits);
//			sprintf(buf, prec, q + 1);
			sprintf(buf, "%d", q + 1);
		}
		ListView_SetColumn(hList1, q - m1 + 1, &colprop);
		nChars[q - m1+1] = max(nChars[q - m1+1], strlen(buf));
	}
}

void CVectorsheetDlg::FillUp()
{ // width is updated in SetCell.
	char buf[256] = {};
	SendDlgItemMessage(IDC_LISTVECTOR, LVM_DELETEALLITEMS, 0, 0);
	//	while (SendDlgItemMessage(IDC_LISTVECTOR,LVM_DELETECOLUMN,0,0)) {};

	bool tseq(psig->GetType() == CSIG_TSERIES);
	bool mtx(psig->nGroups > 1);
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

	HDC	hdc = GetDC(hList1);
	CSize sz;
	int res;
	vector<size_t> nCharRequired;
	unsigned int len;
	if (tseq)
	{
		unsigned int maxElementcount, nChains;
		size_t maxstrlen(0);
		nRows = min(nRows, nChains = psig->CountChains(&maxElementcount));
		nCols = min(nCols, maxElementcount);
		id2 = min(id1 + nCols - 1, maxElementcount - 1);
		id1 = id2 - nCols + 1;
		jd2 = min(jd1 + nRows - 1, nChains - 1);
		jd1 = jd2 - nRows + 1;
		for (unsigned int k = 0; k <= jd2 - jd1; k++)
		{
			CSignal *p = psig->ChainOrd(jd2-k);
			sprintf(buf, "%.1f", p->tmark);
			if (p->GetFs() > 0) strcat(buf, "ms");
			maxstrlen = max(maxstrlen, strlen(buf));
			AppendRow(buf, 0);
		}
		bool aud(psig->GetType() == CSIG_AUDIO);
		bool logical = false, stringdata = false;
		bool compl = psig->body::IsComplex();
		if (psig->bufBlockSize == 1)
		{
			if (psig->GetFs() == 2) stringdata = true;
			else logical = true;
		}
		nCharRequired.resize(id2 - id1 + 2);
		nCharRequired.front() = maxstrlen;
		for (unsigned int k = 0; k <= jd2 - jd1; k++)
		{
			CSignal *p = psig->ChainOrd(k + jd1);
			LvItem.pszText = buf;
			for (unsigned int q = id1; q <= id2; q++)
			{
				if (aud && !psig->body::IsComplex())
					sprintf(buf, xcom::outstream_value(p->buf[q], 0).str().c_str());
				else if (aud && psig->body::IsComplex())
					sprintf(buf, outstream_complex(p->cbuf[q]).str().c_str());
				else if (logical)
					sprintf(buf, "%d", p->logbuf[q]);
				else if (stringdata)
					sprintf(buf, "%c(%u)", p->logbuf[q], (unsigned char)psig->logbuf[q]);
				else if (compl)
					sprintf(buf, outstream_complex(p->cbuf[q]).str().c_str());
				else
					sprintf(buf, xcom::outstream_value(p->buf[q], 0).str().c_str());
				SetCell(k, q - id1 + 1, buf, !aud);
				nCharRequired[q - id1 + 1] = max(nCharRequired[q - id1 + 1], strlen(buf));
			}
		}
	}
	else
	{
		nCols = min(nCols, psig->Len());
		nRows = min(nRows, psig->nGroups);
		//id1, id2: column begining, ending
		id2 = min(id1 + nCols - 1, psig->Len() - 1);
		id1 = id2 - nCols + 1;
		//jd1, jd2: row begining, ending
		jd2 = min(jd1 + nRows - 1, psig->nGroups - 1);
		jd1 = jd2 - nRows + 1;
		//k is for row, q is for col
		len = psig->Len();
		for (unsigned int k = 0; k <= jd2 - jd1; k++)
		{
			if (psig->GetType() == CSIG_AUDIO)
			{
				double tp = 1000. * ((jd2 - k) * len + id1) / psig->GetFs() + psig->tmark;
				sprintf(buf, "%.1fms", tp);
			}
			else if (mtx) LvItem.pszText = itoa(jd2 - k + 1, buf, 10);
			AppendRow(buf, 0);
		}
		bool aud(psig->GetType() == CSIG_AUDIO);
		bool logical = false, stringdata = false;
		bool compl = psig->body::IsComplex();
		if (psig->bufBlockSize == 1)
		{
			if (psig->GetFs() == 2) stringdata = true;
			else logical = true;
		}
		nCharRequired.resize(id2 - id1 + 2);
		nCharRequired.front() = 3; //"idx" or "mtx"
		for (unsigned int k = 0; k <= jd2 - jd1; k++)
		{
			LvItem.pszText = buf;
			for (unsigned int q = id1; q <= id2; q++)
			{
				if (aud && !psig->body::IsComplex())
					sprintf(buf, xcom::outstream_value(psig->buf[(k + jd1) * len + q], 0).str().c_str());
				else if (aud && psig->body::IsComplex())
					sprintf(buf, outstream_complex(psig->cbuf[(k + jd1) * len + q]).str().c_str());
				else if (logical)
					sprintf(buf, "%d", psig->logbuf[(k + jd1) * len + q]);
				else if (stringdata)
					sprintf(buf, "%c(%u)", psig->logbuf[(k + jd1) * len + q], (unsigned char)psig->logbuf[(k + jd1) * len + q]);
				else if (compl)
					sprintf(buf, outstream_complex(psig->cbuf[(k + jd1) * len + q]).str().c_str());
				else
					sprintf(buf, "%g", psig->buf[(k + jd1) * len + q]);
				SetCell(k, q - id1 + 1, buf, !aud);
				nCharRequired[q - id1+1] = max(nCharRequired[q - id1+1], strlen(buf));
			}
		}
	}
	//At this point nCharRequired should be ready
	SetColumnWidths(hdc, nCharRequired);
	SetColumnTitles(nCharRequired, hdc, id1, id2);
	int totalwidth(0);
	for (auto& n : width)	totalwidth += n;
	int p(0);
	for (vector<int>::iterator it = width.begin(); it != width.end(); it++, p++)
		ListView_SetColumnWidth(hList1, p, *it);
	if (totalwidth < 120)
	{ // 120 is the minimum width of a window. If the current width is below that, adjust them evenly
		int diff = 120 - totalwidth;
		if (psig->Len() == 1)
		{
			width[0] += diff / 3;
			width[1] = (120 - width[0]);
		}
		else
		{
			int sum(0);
			vector<int>::iterator it = width.begin();
			for (; it != width.end(); it++)
			{
				*it += diff / (int)width.size();
				sum += *it;
			}
			width.back() += (120 - sum);
		}
		p = 0;
		for (vector<int>::iterator it = width.begin(); it != width.end(); it++, p++)
			ListView_SetColumnWidth(hList1, p, *it);
		totalwidth = 120;
	}
	res = (int)SendDlgItemMessage(IDC_LISTVECTOR, LVM_APPROXIMATEVIEWRECT, -1, MAKELONG(-1, -1));
	WORD height = HIWORD(res);
	CRect rtDlg, rtList;
	GetWindowRect(rtDlg);
	rtDlg.right = rtDlg.left + totalwidth + 20;
	rtDlg.bottom = rtDlg.top + height + 35;
	MoveWindow(rtDlg);
	int listHeight = rtDlg.Height() - head_height;
	rtList.top = head_height;
	rtList.bottom = rtList.top + listHeight;
	rtList.left = 0;
	rtList.right = rtList.left + totalwidth + 5;
	MoveDlgItem(hDlg, IDC_LISTVECTOR, rtList, 1);
	ReleaseDC(NULL, hdc);
}

vector<int> CVectorsheetDlg::SetColumnWidths(HDC hdc, vector<size_t> &nChars)
{
	CSize sz;
	GetTextExtentPoint32(hdc, "A", 1, &sz);
	int k = 0;
	for (vector<size_t>::iterator it = nChars.begin(); it != nChars.end(); it++)
		width[k++] = (int) (sz.cx * (*it+2)); // add 2 (blanks in the left and right) to the required number of chars
	return width;
}

void CVectorsheetDlg::OnShowWindow(BOOL fShow, UINT status)
{
}

void CVectorsheetDlg::OnMove(int cx, int cy)
{
	char buf[64];
	GetWindowText(buf, sizeof(buf));
	RECT rt;
	GetWindowRect(&rt);
	dlgpos[buf] = rt;
}


void CVectorsheetDlg::OnClose()
{
	DestroyWindow(); //This will post WM_DESTROY through the message loop
}

void CVectorsheetDlg::OnDestroy()
{
	delete this;
}

void CVectorsheetDlg::OnCommand(int idc, HWND hwndCtl, UINT event)
{
	string addp;
	switch(idc)
	{
	case IDCANCEL: 
		OnClose();
		break;
	}
}

void CVectorsheetDlg::OnNotify(HWND hwnd, int idcc, LPARAM lParam)
{
	int k, res(0);
	static char buf[256];
	char buffer[256];
	LPNMHDR pnm = (LPNMHDR)lParam;
	LPNMLISTVIEW pview = (LPNMLISTVIEW)lParam;
	LPNMLVKEYDOWN lvnkeydown;
	NMLVDISPINFO *pdi;
	UINT code=pnm->code;
	LPNMITEMACTIVATE lpnmitem;
	HWND hEdit;
	static int clickedRow;
	switch(code)
	{
	case NM_CLICK:
		lpnmitem = (LPNMITEMACTIVATE) lParam;
		clickedRow = lpnmitem->iItem;
		break;
	case NM_DBLCLK:
		lpnmitem = (LPNMITEMACTIVATE) lParam;
		clickedRow = lpnmitem->iItem;
		if ((clickedRow = lpnmitem->iItem)==-1) break;
		ListView_GetItemText(lpnmitem->hdr.hwndFrom, ListView_GetSelectionMark(lpnmitem->hdr.hwndFrom), 0, buffer, 256);
		if (sigvector)
		{
			CShowvarDlg *cvdlg = new CShowvarDlg(hParent, NULL);
			CVar *tp = sigvector->at(clickedRow);
			cvdlg->pVars = &tp->strut;
			cvdlg->pGOvars = &tp->struts;
			cvdlg->hDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SHOWVAR), mShowDlg.hDlg, (DLGPROC)showvarDlgProc);
			cvdlg->Sig = *sigvector->at(clickedRow); // This is necessary so that when a cell element is clicked recursively, only the proper sig within the cell is retrived.
			if (cvdlg->hDlg != NULL)
			{
				sprintf(buf, "%s(%s)", name.c_str(), buffer);
				cvdlg->InitVarShow(CSIG_STRUCT, buf);
				cvdlg->ShowWindow(SW_SHOW);
			}
			cvdlg->Fillup(NULL, &cvdlg->Sig);
		}
		break;
	case LVN_BEGINLABELEDIT:
		lpnmitem = (LPNMITEMACTIVATE)lParam;
		clickedRow = lpnmitem->iItem;
		hEdit = ListView_GetEditControl(hList1);
		::GetWindowText(hEdit, label, sizeof(label));
		break;
	case LVN_ENDLABELEDIT:
		pdi = (NMLVDISPINFO*)lParam;
		hEdit = ListView_GetEditControl(hList1);
		if ((k = (int)::SendMessage(hList1, LVM_GETNEXTITEM, -1, LVNI_FOCUSED)) != -1)
		{
			LvItem.iSubItem = 0;
			if (pdi->item.pszText)
			{
				::GetWindowText(hEdit, buffer, sizeof(buffer));
				//Not needed but don't delete it. This is another way to change the column title. 7/28/2018
//				LvItem.pszText = buffer;
//				::SendMessage(hList1, LVM_SETITEMTEXT, (WPARAM)k, (LPARAM)&LvItem);
				if (psig->GetType() == CSIG_AUDIO)
				{
					string strIn(buffer);
					trim(strIn,' ');
					size_t pt = strIn.find("ms");
					if (pt != string::npos)
					{
						buffer[pt] = 0;
						double dk;
						if (sscanf(buffer, "%lf", &dk) != EOF)
						{
							if (CTimeSeries * thisone = psig->AtTimePoint(dk))
							{
								psig = thisone;
								id1 = (int)((dk-thisone->tmark) * thisone->GetFs() / 1000. + .5);
								FillUp();
							}
							break;
						}
					}
				}
				//update the viewing range
				int p, q;
				if (psig->nGroups > 1)
				{
					// For a matrix, the string must have a comma, because the format must be either 
					// a) two numbers delimited by a comma (e.g., 3,4), 
					// b) number followed by a comma (e.g., 3,) 
					// or c) a comma followed by a number (e.g., ,3)
					string strIn(buffer);
					size_t pt = strIn.find(',');
					if (pt != string::npos)
					{
						buffer[pt] = 0;
						char buffer2[256];
						strcpy(buffer2, strIn.substr(pt + 1).c_str());
						int res1 = sscanf(buffer, "%d", &p);
						int res2 = sscanf(buffer2, "%d", &q);
						if (res1 == EOF && res2 == EOF) break;
						if ( res1 != EOF && res2 != EOF)
						{ // case a
							id1 = q-1;
							jd1 = p- pdi->item.iItem-1;
						}
						else if (res1 != EOF)
						{ // case b
							jd1 = p- pdi->item.iItem-1;
						}
						else if (res2 != EOF)
						{ // case c
							id1 = q - 1;
						}
					}
				}
				else
				{// For a non-matrix, the string must be a plain number.
					if (sscanf(buffer, "%d", &p) == EOF) break;
					id1 = p - 1;
				}
				FillUp();
			}
			else
			{
				LvItem.pszText = label;
				::SendMessage(hList1, LVM_SETITEMTEXT, (WPARAM)k, (LPARAM)&LvItem);
			}
		}
		break;
	case LVN_KEYDOWN:
		lvnkeydown = (LPNMLVKEYDOWN)lParam;
		clickedRow = ListView_GetSelectionMark(lvnkeydown->hdr.hwndFrom);
		switch (lvnkeydown->wVKey)
		{
		case VK_RIGHT:
			if (id2 < psig->Len()-1)
			{
				id1 += nCols / 2;
				FillUp();
				//ListView_SetItemState(hList1, -1, LVIS_FOCUSED | LVIS_SELECTED, 0);
			}
			break;
		case VK_LEFT:
			if (id1)
			{
				if (id1 < nCols / 2) id1 = 0;
				else  id1 -= nCols / 2;
				FillUp();
			}
			break;
		case VK_DOWN:
			if (clickedRow < (int)nRows-1)
				break;
		case VK_NEXT:
			jd1 += nRows / 2;
			FillUp();
			break;
		case VK_UP:
			if (clickedRow > 0)
				break;
		case VK_PRIOR:
			if (jd1 < nRows / 2) jd1 = 0;
			else  jd1 -= nRows / 2;
			FillUp();
			break;
		}
		break;

	case NM_CHAR:
		break;
	case NM_CUSTOMDRAW:
        ::SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG)ProcessCustomDraw(pnm));
        return;
	default:
		break;
	}
}

LRESULT CVectorsheetDlg::ProcessCustomDraw(NMHDR *lParam)
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;
	switch (lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;
	case CDDS_ITEMPREPAINT:
		return CDRF_NOTIFYSUBITEMDRAW;
	case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		if (lplvcd->iSubItem == 0)
		{
			lplvcd->clrText = RGB(255, 255, 0);
			lplvcd->clrTextBk = RGB(0, 0, 0);
		}
		else
		{
			lplvcd->clrText = RGB(20, 26, 158);
			lplvcd->clrTextBk = RGB(200, 200, 10);
		}
		return CDRF_NEWFONT;
	}
	return CDRF_DODEFAULT;
}