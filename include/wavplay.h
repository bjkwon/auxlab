#pragma once

#ifndef WAVPLAY
#define WAVPLAY

#include <windows.h>
//#include <stdio.h>
#include "sigproc.h"


class AUD_PLAYBACK
{
public:
	INT_PTR pWavePlay;
	int DevID;
	int fs;
	INT_PTR blockDuration; // milliseconds
	INT_PTR totalDuration; // milliseconds
	INT_PTR remainingDuration; // milliseconds
	CVar *pvar=NULL;
	AUD_PLAYBACK() {};
	~AUD_PLAYBACK() { if (pvar) delete pvar; };
} ;

void TerminatePlay(int quick=1);
bool StopPlay(INT_PTR pWavePlay, bool quick);
bool PauseResumePlay(INT_PTR pWavePlay, bool fOnOff);
void SetHWND_WAVPLAY(HWND hAppl);
HWND GetHWND_WAVPLAY();
int WinMMGetVolume(INT_PTR pWavePlay, DWORD &vol, char *errstr);
int WinMMSetVolume(INT_PTR pWavePlay, DWORD vol, char *errstr);
int GetDevCaps(UINT_PTR id);

INT_PTR PlayBufAsynch16(UINT DevID, short *dataBuffer, int length, int nChan, int fs, UINT userDefinedMsgID, HWND hApplWnd, int nProgReport, int loop, char* errstr);
INT_PTR QueuePlay(INT_PTR pWP, UINT DevID, SHORT *dataBuffer, int length, int nChan, UINT userDefinedMsgID, int nProgReport, char *errstr, int loop);

#endif