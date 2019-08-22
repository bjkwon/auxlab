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
#include <queue>
#include "audio_capture_status.h"
#include "resource1.h"
#include "xcom.h"

#include <mutex>
#include <thread>

#define UPDATE_CONTENT 1000

extern CAudcapStatus mCaptureStatus;
extern mutex mtx;
extern queue<audiocapture_status_carry *> msgq;
extern condition_variable cv;

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
	char buffer[256];
	switch(idc)
	{
	case UPDATE_CONTENT:
		sprintf(buffer, "%s\n", (char*)hwndCtl);
		EditPrintf(GetDlgItem(IDC_EDIT1), buffer);
		break;
	case IDCANCEL:
		OnClose();
		break;
	}
}

void AudioCaptureStatus(unique_ptr<audiocapture_status_carry> pmsg)
{
	char buffer[256];
	if (pmsg->ind == 0)
	{
		if (!mCaptureStatus.hDlg)
			CreateDialog(pmsg->hInst, "AUDIOCAPTURE", pmsg->hParent, (DLGPROC)audiocaptuestatusProc);
		//else
		//	mCaptureStatus.ShowWindow(SW_SHOW);
		int blockDuration = (int)(1000. * pmsg->cbp.len_buffer / pmsg->cbp.fs);
		sprintf(buffer, "Block size %d ms\r\nInitial callback processing time %d ms.\r\nSince the initial callback...", blockDuration, pmsg->elapsed);
		SetDlgItemText(mCaptureStatus.hDlg, IDC_EDIT1, "");
		SendMessage(mCaptureStatus.hDlg, WM_COMMAND, UPDATE_CONTENT, (WPARAM)buffer);
	}

	while (1)
	{
		unique_lock<mutex> lk(mtx);
		cv.wait(lk);
		lk.unlock();
		audiocapture_status_carry p = *msgq.front();
		if (p.ind==1)
		{
			sprintf(buffer, "Elapsed time for block 1: %d ms.\n", pmsg->lastCallbackTimeTaken);
			SendMessage(mCaptureStatus.hDlg, WM_COMMAND, MAKELONG(UPDATE_CONTENT, pmsg->ind), (WPARAM)buffer);
		}
		else
		{
			int blockDuration = (int)(1000. * p.cbp.len_buffer / p.cbp.fs);
			sprintf(buffer, "(%d ms for the last block callback) %d ms elapsed for block %d", p.lastCallbackTimeTaken, p.elapsed, p.ind);
			SendMessage(mCaptureStatus.hDlg, WM_COMMAND, MAKELONG(UPDATE_CONTENT, p.ind), (WPARAM)buffer);
		}
		msgq.pop();
	}
}