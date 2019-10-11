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
#define SET_RECORDINGSETTINGS 1100

extern CAudcapStatus mCaptureStatus;
extern mutex mtx;
mutex mtx4PlotDlg;
HWND hwnd_AudioCapture = NULL;
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
	cumAverageblockTime = 0.;
}

CAudcapStatus::~CAudcapStatus(void)
{
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam); // from graffy.cpp

BOOL CAudcapStatus::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	CWndDlg::OnInitDialog(hwndFocus, lParam);
	GRAFWNDDLGCHILDSTRUCT in;
	in.hIcon = NULL;
	in.hWndAppl = hDlg;
	in.threadCaller = GetCurrentThreadId();
	GetClientRect(hDlg, in.rt);
	in.rt.bottom -= 50;
	cfig = (CFigure *)OpenChildGraffy(in);
	hPlot = cfig->m_dlg->hDlg;
	CPosition cp(.1, .1, .88, .78);
	ax = (CAxes *)AddAxes(cfig, cp);
	ax->color = RGB(130, 130, 140);
	OnSize(0, 0, 0);
	::SetWindowText(GetDlgItem(IDC_EDIT1), "see\n");
//	::SetWindowText(GetDlgItem(IDC_EDIT1),"abcdefghijklmnopqrstuvxwyz01234567890first lineabcdefghijklmnopqrstuvxwyz01234567890first lineabcdefghijklmnopqrstuvxwyz01234567890first line\n");
	EditPrintf(GetDlgItem(IDC_EDIT1), "abcdefghijklmnopqrstuvxwyz01234567890first lineabcdefghijklmnopqrstuvxwyz01234567890first lineabcdefghijklmnopqrstuvxwyzfirst line\n");
	//EditPrintf(GetDlgItem(IDC_EDIT1), "you will see from this line\n");
	//EditPrintf(GetDlgItem(IDC_EDIT1), "trecherous rain snow bjkwon\n");
	return 0;
}

void CAudcapStatus::OnSize(UINT state, int cx, int cy)
{
	CRect rtDlg, rt;
	GetClientRect(hDlg, &rtDlg);
	rt.top = 270;
	rt.left = 0;
	rt.bottom = rtDlg.Height();
	rt.right = rt.left + rtDlg.Width();
	int res = MoveDlgItem(hDlg, IDC_EDIT1, rt, 1);
	if (cx*cy == 0)
	{
		cx = rtDlg.Width();
		cy = rtDlg.Height();
	}
	::SendMessage(hPlot, WM_SIZE, WS_CHILD, MAKELONG(cx,cy*2/3));
}

void CAudcapStatus::OnClose()
{
	ShowWindow(SW_HIDE);
}

void CAudcapStatus::OnDestroy()
{
	DestroyWindow();
}

// SET_RECORDINGSETTINGS: HIWORD(wParam): block duration (ms), lParam: first callback processing time (ms)
// UPDATE_CONTENT: LOWORD(lParam): block processing time, HIWORD(lParam): callback processing time

void CAudcapStatus::OnCommand(int idc, HWND hwndCtl, UINT event)
{
	string addp, str;
	vector<HANDLE> figs;
	char buffer[256];
	int id;
	WORD index, cbtime;
	CRect rt;
	CTimeSeries pp, pp2;
	static CSize sz;
	int rightLimit;
	switch (idc)
	{
	case SET_RECORDINGSETTINGS:
		deleteObj(lineBlocktime);
		deleteObj(lineSymAvgb);
		deleteObj(lineSymAvga);
		deleteObj(lineCallbacktime);
		deleteObj(txtId);
		deleteObj(txtAvg);
		deleteObj(txtTitle);
		lineBlocktime = nullptr;
		lineSymAvga = nullptr;
		lineSymAvgb = nullptr;
		lineCallbacktime = nullptr;
		txtId = nullptr;
		txtAvg = nullptr;
		blocktime = event;
		cbtime = (WORD)(LONG_PTR)hwndCtl;
		sprintf(buffer, "Block size %d ms\nInitial callback processing time %d ms.\nSince the initial callback...\n", blocktime, cbtime);
		SetDlgItemText(IDC_EDIT1, buffer);
		memset(blockproctimeHistory, 0, sizeof(blockproctimeHistory));
		memset(cbproctimeHistory, 0, sizeof(cbproctimeHistory));
		ax->setRange('y', 0, blocktime*1.5);
		ax->ylimfixed = true;
		ax->color = RGB(130, 130, 140);
		ax->ytick.tics1 = ax->gengrids('y');
		sprintf(buffer, "Capture Block Size %d ms; Current Block", blocktime);
		txtTitle = (CText *)AddText(cfig, buffer, .09, .9, .1, .25);
		::InvalidateRect(hPlot, NULL, 1);
		GetTextExtentPoint32(GetDC(hDlg), "888", 3, &sz); // get the screen size for a wide 4 digit number
		break;
	case UPDATE_CONTENT:
		index = event;
		blocktime = HIWORD(hwndCtl);
		cumAverageblockTime = ((index - 1) * cumAverageblockTime + blocktime) / index;
		cbtime = LOWORD(hwndCtl);
		id = index - (index-1) / 50 * 50;
		blockproctimeHistory[id - 1] = blocktime;
		blockproctimeMA[id - 1] = cumAverageblockTime;
		pp = CSignal(blockproctimeHistory, id);
		pp2 = CSignal(cumAverageblockTime);
		{
			double xref1 = -2., xref2 = -1.;
			unique_lock<mutex> locker(mtx4PlotDlg);
			//while this part is progressing, don't start OnBegin()
			RECT cumAvgRt = {};
			if (index > 1)
			{
				cumAvgRt = lineSymAvgb->rti;
				cumAvgRt.left = lineSymAvga->rti.left;
				rightLimit = lineBlocktime->rtf.right;
			}
			RECT txtAvgRt = cumAvgRt;
			deleteObj(lineBlocktime);
			deleteObj(lineSymAvga);
			deleteObj(lineSymAvgb);
			deleteObj(lineCallbacktime);
			deleteObj(txtId);
			deleteObj(txtAvg);
			lineBlocktime = ax->plot(NULL, &pp, RGB(0, 155, 102), 'o');
			lineSymAvga = ax->plot(&xref1, &pp2, RGB(240, 55, 0), '>');
			lineSymAvgb = ax->plot(&xref2, &pp2, RGB(240, 55, 0), '<');
			lineBlocktime->filled = true;
			lineSymAvga->filled = true;
			lineSymAvgb->filled = true;
			lineSymAvga->markersize = 5;
			lineSymAvgb->markersize = 5;
			sprintf(buffer, "%d", index);
			txtId = (CText *)AddText(cfig, buffer, 0, 0, 0, 0);
			txtId->posmode = 1;
			txtId->textRect = CRect(CPoint(txtTitle->textRect.right, txtTitle->textRect.top), sz);
			sprintf(buffer, "avg %.1f", cumAverageblockTime);
			if (index > 1)
			{
				txtAvg = (CText *)AddText(cfig, buffer, 0, 0, 0, 0);
				txtAvg->posmode = 1;
				txtAvg->ChangeFont("", 10);
				txtAvgRt.left -= sz.cx;
				txtAvgRt.right += sz.cx/2;
				txtAvgRt.top += 10;
				txtAvgRt.bottom += 10 + sz.cy;
				txtAvg->textRect = txtAvgRt;
			}
			if (id > 1)
			{
				cbproctimeHistory[id - 2] = cbtime;
				pp2 = CSignal(cbproctimeHistory, id - 1);
				lineCallbacktime = ax->plot(NULL, &pp2, RGB(200, 102, 0), 'd');
				lineCallbacktime->filled = true;
			}
			else if (index > 1 && id == 1)
			{ //the beginning of new cycle of plotting except for the very first cycle
				cbproctimeHistory[49] = cbtime;
				pp2 = CSignal(cbproctimeHistory, 50);
				lineCallbacktime = ax->plot(NULL, &pp2, RGB(200, 102, 0), 'd');
				lineCallbacktime->filled = true;
				int px = ax->double2pixel(50, 'x');
				int py = ax->double2pixel(cbtime, 'y');
				RECT rt2 = { px - 4, py - 4, px + 4, py + 4 };
				::InvalidateRect(hPlot, &rt2, 1);
			}
			ax->setRange('x', -3, 51);
			rt = ax->rct;
			if (id == 50)
				ax->color = RGB(130, 130, 140);
			else
			{
				ax->color = RGB(240, 240, 255);
				if (index == 1)
				{
					//set the repaint area for x-axis--there should be a better way that I have prepared... 8/31/2019
					RECT rt2(ax->rct);
					rt2.top = rt2.bottom;
					rt2.bottom = rt2.top + 50;
					rt2.left -= 50;
					rt2.right += 50;
					ax->xtick.tics1.push_back(0.);
					ax->xtick.tics1.push_back(10.);
					ax->xtick.tics1.push_back(20.);
					ax->xtick.tics1.push_back(30.);
					ax->xtick.tics1.push_back(40.);
					ax->xtick.tics1.push_back(50.);
					::InvalidateRect(hPlot, &rt2, 1);
				}
				if (id == 1)
				{
					rt.left = rt.top = 0;
					int px = ax->double2pixel(id, 'x');
					RECT rtc;
					GetClientRect(hDlg, &rtc);
					rt.bottom = rtc.bottom;
					rt.right = px + 4;
				}
				else
				{
					rt.left += (LONG)(round((double)rt.Width()*id / 54)) - 50;
					rt.right = rightLimit+10;
				}
			}
			::InvalidateRect(hPlot, rt, 1);
			::InvalidateRect(hPlot, &cumAvgRt, 1);
			::InvalidateRect(hPlot, txtId->textRect, 1);
			::InvalidateRect(hPlot, txtAvg->textRect, 1);
			//======================================= end of mtx4PlotDlg scope
		}
		if (index == 1)
			sprintf(buffer, "Elapsed time for block 1: %d ms.\n", blocktime);
		else
			sprintf(buffer, "%d: block time %d, cum avg %.1f\n", index, blocktime, cumAverageblockTime);
		EditPrintf(GetDlgItem(IDC_EDIT1), buffer);

		break;
	case IDC_BUTTON222:
		EditPrintf(GetDlgItem(IDC_EDIT1), "more more more coming\n");
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
		int blockDuration = (int)(1000. * pmsg->cbp.len_buffer / pmsg->cbp.fs);
		SendMessage(mCaptureStatus.hDlg, WM_COMMAND, MAKELONG(SET_RECORDINGSETTINGS, blockDuration), (LPARAM)pmsg->elapsed);
	}

	while (1)
	{
		unique_lock<mutex> lk(mtx);
		cv.wait(lk);
		lk.unlock();
		audiocapture_status_carry p = *msgq.front();
		if (p.ind==1)
			sprintf(buffer, "Elapsed time for block 1: %d ms.\n", p.elapsed);
		else
		{
			int blockDuration = (int)(1000. * p.cbp.len_buffer / p.cbp.fs);
			sprintf(buffer, "(%d ms for the last block callback) %d ms elapsed for block %d", p.lastCallbackTimeTaken, p.elapsed, p.ind);
		}
		SendMessage(mCaptureStatus.hDlg, WM_COMMAND, MAKELONG(UPDATE_CONTENT, p.ind), MAKELONG(p.lastCallbackTimeTaken, p.elapsed));
		msgq.pop();
	}
}