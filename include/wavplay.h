// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: wavplay
// Library to play audio signals 
// For Windows only
// 
// 
// Version: 1.495
// Date: 12/13/2018
// 
#pragma once

#ifndef WAVPLAY
#define WAVPLAY

#include <windows.h>
#include "sigproc.h"
#include "caux.h"

class AUD_PLAYBACK
{
public:
	INT_PTR pWavePlay;
	int DevID;
	int fs;
	INT_PTR blockDuration; // milliseconds
	INT_PTR totalDuration; // milliseconds
	INT_PTR remainingDuration; // milliseconds
	CVar sig;
	AUD_PLAYBACK() { };
	~AUD_PLAYBACK() {};
};

typedef struct
{
	short devID;
	short nChans;
	bool closing = 0;
	bool bufferID = 0;
	DWORD recordingThread;
	DWORD recordID;
	int fs;
	int len_buffer;
	double duration; // in duration to record, in milliseconds; -1 if indefinite
	char *buffer;
	char callbackfilename[256];
} callback_trasnfer_record;


#define WM__AUDIOEVENT1				WM_APP + WOM_OPEN
#define WM__AUDIOEVENT2				WM_APP + WOM_OPEN * 2

#define WM__RECORDING_ERR			WM_APP + 6971
#define WM__STOP_REQUEST			WM_APP + WOM_CLOSE
#define WM__RECORDING_THREADID		WM_APP + 0x7827



void TerminatePlay(int quick=1);
bool StopPlay(INT_PTR pWavePlay, bool quick);
bool PauseResumePlay(INT_PTR pWavePlay, bool fOnOff);
bool StopRecord(int recID, char *errstr);
bool PauseResumeRecord(int recID, bool fOnOff, char *errstr);
void SetHWND_WAVPLAY(HWND hAppl);
HWND GetHWND_WAVPLAY();
int WinMMGetVolume(INT_PTR pWavePlay, DWORD &vol, char *errstr);
int WinMMSetVolume(INT_PTR pWavePlay, DWORD vol, char *errstr);
int GetDevCaps(UINT_PTR id);

//In order to play in blocking mode, put PlayAsynch16(sig, devID, 0, NULL, 2, errstr);
//  Specify 2 for the nProgReport even though you are not utilizing any messaging back to hWnd..
//  This is just due to the way wavBuffer2snd is written in wavplay.cpp  Jan 19, 2013. BJ Kwon
INT_PTR PlayArray16(const ctimesig &sig, int DevID, UINT userDefinedMsgID, HWND hApplWnd, double *block_dur_ms, char *errstr, int loop);
INT_PTR PlayArray16(const CSignals &sig, int DevID, UINT userDefinedMsgID, HWND hApplWnd, double *block_dur_ms, char *errstr, int loop);
INT_PTR PlayArray16(const CSignals &sig, int DevID, UINT userDefinedMsgID, HWND hApplWnd, int nProgReport, char *errstr, int loop);
INT_PTR PlayArrayNext16(const CSignals &sig, INT_PTR pWP, int DevID, UINT userDefinedMsgID, double *block_dur_ms, char *errstr, int loop);
INT_PTR PlayArrayNext16(const CSignals &sig, INT_PTR pWP, int DevID, UINT userDefinedMsgID, int nProgReport, char *errstr, int loop);
INT_PTR PlayBufAsynch16(UINT DevID, short *dataBuffer, int length, int nChan, int fs, UINT userDefinedMsgID, HWND hApplWnd, int nProgReport, int loop, char* errstr);
INT_PTR QueuePlay(INT_PTR pWP, UINT DevID, SHORT *dataBuffer, int length, int nChan, UINT userDefinedMsgID, int nProgReport, char *errstr, int loop);
int Capture(int DevID, UINT userDefinedMsgID, HWND hApplWnd, int fs, short nChans, short bits, const char *callbackname, double duration, double block_dur_ms, int recordID, char *errmsg);

#endif