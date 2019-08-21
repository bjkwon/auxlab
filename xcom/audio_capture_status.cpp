// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: auxlab
// Main Application. Based on Windows API  
// 
// 
// Version: 1.62
// Date: 8/20/2019


#include "graffy.h" // this should come before the rest because of wxx820
#include <process.h>
#include "audio_capture_status.h"
#include "resource1.h"
#include "xcom.h"

#include <thread>

extern CAudcapStatus mCaptureStatus;

int MoveDlgItem(HWND hDlg, int id, CRect rt, BOOL repaint);

BOOL CALLBACK audiocaptuestatusProc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	switch (umsg)
	{
	chHANDLE_DLGMSG (hDlg, WM_INITDIALOG, mCaptureStatus.OnInitDialog);
	chHANDLE_DLGMSG (hDlg, WM_SIZE, mCaptureStatus.OnSize);
	chHANDLE_DLGMSG (hDlg, WM_CLOSE, mCaptureStatus.OnClose);
	chHANDLE_DLGMSG (hDlg, WM_COMMAND, mCaptureStatus.OnCommand);
	default:
		return FALSE;
	}
	return TRUE;
}

CAudcapStatus::CAudcapStatus()
{
}

CAudcapStatus::~CAudcapStatus(void)
{
}


BOOL CAudcapStatus::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	CWndDlg::OnInitDialog(hwndFocus, lParam);
	OnSize(0, 0, 0);
	return TRUE;
}

void CAudcapStatus::OnSize(UINT state, int cx, int cy)
{
	POINT titlebarDim;
	CRect rtDlg, rt;
	GetClientRect(hDlg, &rtDlg);
	rt.top = 0;
	rt.left = 0;
	rt.bottom = rtDlg.Height();
	rt.right = rt.left + rtDlg.Width();
	int res = MoveDlgItem(hDlg, IDC_EDIT1, rt, 1);
}

void CAudcapStatus::OnClose()
{
	ShowWindow(SW_HIDE);
}

void CAudcapStatus::OnDestroy()
{
	DestroyWindow();
}


void CAudcapStatus::OnCommand(int idc, HWND hwndCtl, UINT event)
{
	string addp, str;
	vector<HANDLE> figs;
	static char fullfname[256], fname[256];
	static int jj = 0;
	char buffer[256];
	int res;
	switch(idc)
	{
	case 9983:
		sprintf(buffer, "%d: %s\n", jj++, (char*)hwndCtl);
		EditPrintf(GetDlgItem(IDC_EDIT1), buffer);
//		res = SetDlgItemText(IDC_EDIT1, buffer);
		break;
	case IDCANCEL:
		OnClose();
		break;
	}
}

void AudioCaptureStatus(unique_ptr<audiocapture_status_carry> pmsg)
{
	if (mCaptureStatus.hDlg)
		mCaptureStatus.ShowWindow(SW_SHOW);
	else
		CreateDialog(pmsg->hInst, "AUDIOCAPTURE", pmsg->hParent, (DLGPROC)audiocaptuestatusProc);

	int blockDuration = (int)(1000. *pmsg->cbp.len_buffer / pmsg->cbp.fs);
	char buffer[256];
	sprintf(buffer, "Underflow detected--Processing %d ms block took %d\n", blockDuration, pmsg->elapsed);
	SendMessage(mCaptureStatus.hDlg, WM_COMMAND, 9983, (WPARAM)buffer);
}