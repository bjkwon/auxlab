// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: wavplay
// Library to play audio signals 
// For Windows only
// 
// Version: 1.7
// Date: 7/16/2019
// 

#include "wavplay.h"
#include <process.h>
#include <vector>
#include <algorithm>
#include <memory>
#include <thread>
#include <time.h>

#define MMERRTHROW(X,MM) rc=X; if (rc!=MMSYSERR_NOERROR) {char _estr_[256]; waveOutGetErrorText(rc, _estr_, 256); sprintf(errmsg, "Error on %s, code=%d\n%s", #MM, rc, _estr_); throw errmsg;}
#define MMERRRETURNFALSE(MM) pWP->cleanUp(); char errmsg[256], _estr_[256]; waveOutGetErrorText(rc, _estr_, 256); sprintf(errmsg, "Error in %s, code=%d\n%s", #MM, rc, _estr_); SendMessage(pWP->hWnd_calling, pWP->msgID,(WPARAM)errmsg,-1); return false;

using namespace std;

static vector<HWAVEIN> toClose;

//FYI: These two message are sent to different threads.
#define WM__RETURN		WM_APP+10
#define MM_DONE			WM_APP+11
#define MM_ERROR		WM_APP+12

#define DISPATCH_PLAYBACK_HANDLE			WM_APP+120
#define DISPATCH_PLAYBACK_HANDLE_STATUS		WM_APP+121
#define WM__STOP_RECORD						WM_APP+122

extern HWND hMainAppl;

#ifndef NO_PLAYSND

#define OK			0
#define ERR			-1

#define FINISHED_PLAYING_CLEANUP	11
#define PI 3.141592

static DWORD threadIDs[64]; // to track thread handle in the present thread (e.g., to track history)  
static int len_threadIDs(0); // to track thread handle in the present thread (e.g., to track history) 

#define WM__STOP			WM_APP+715

class CWaveRecord
{
// inBufferLen, totalSamples, recordedSamples all indicate
// the data count recorded, not byte count; counting both channels (i.e., for stereo, for a given duration intended,
// actual values will be doubled.
public:
	HWND			hWnd_calling;
	uintptr_t		hThread;
	DWORD			threadID;
	DWORD			callingThreadID;
	UINT			msgID;
	DWORD			totalSamples; // 0 for indefinite recording
	DWORD			recordedSamples; // 
	DWORD			nPlayedBlocks;
	char			*inBuffer[2];
	DWORD			inBufferLen; 
	UINT userDefinedMsgID;

	bool			blockMode;
	int				bitswidth;
	HWAVEIN			hwi;
	WAVEINCAPS		wic;
	WAVEOUTCAPS		woc;
	WAVEHDR			wh[2];
	WAVEFORMATEX	wfx;
	string			callbackname;
	short			devID;

	CWaveRecord();
	~CWaveRecord();
	int setPlayPoint(int id);
	int OnBlockDone(WAVEHDR* lpwh);
	int	cleanUp(int threadIDalreadycut = 0);
};

CWaveRecord::CWaveRecord()
	: hThread(NULL), bitswidth(1), nPlayedBlocks(0), recordedSamples(0)
{
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.cbSize = 0;
}

CWaveRecord::~CWaveRecord()
{
	// Do we need to free up any memory blocks that were somehow not taken care of?
	delete[] inBuffer[0];
	delete[] inBuffer[1];
}


int CWaveRecord::cleanUp(int IDcut)
{
	if (IDcut == 0)
		threadIDs[len_threadIDs-- - 1] = 0;
	return FINISHED_PLAYING_CLEANUP;
}

int CWaveRecord::setPlayPoint(int id)
{
	wh[id].dwFlags = 0;
	wh[id].dwBufferLength = wfx.wBitsPerSample / 8 * inBufferLen;
	// id is waveform header ID; either 0 or 1
	wh[id].lpData = inBuffer[id];
	return 0;
}

int CWaveRecord::OnBlockDone(WAVEHDR* lpwh)
{
	return 0;
}


void ThreadCapture(unique_ptr<CWaveRecord> p)
{
	MSG        msg;
	char errmsg[256];
	bool ch(false);
	bool hist(false);
	MMRESULT	rc;
	p->threadID = GetCurrentThreadId();
	try
	{
		rc = waveInOpen(&p->hwi, p->devID, &p->wfx, (DWORD_PTR)p->threadID, (DWORD_PTR)0, CALLBACK_THREAD);
		if (rc != MMSYSERR_NOERROR)
		{
			PostThreadMessage(p->callingThreadID, MM_ERROR, (WPARAM)rc, (LPARAM)"");
			threadIDs[len_threadIDs-- - 1] = 0;
			return;
		}

		struct
		{
			short devID;
			bool closing = 0;
			DWORD recordingThread;
			int fs;
			int len_buffer;
			char *buffer;
			char callbackfilename[256];
		} record_identifier;

		PostThreadMessage(p->callingThreadID, DISPATCH_PLAYBACK_HANDLE, (WPARAM)0, (LPARAM)"");

		// Insert LOGGING4
		bool already_double_buffered = false;
		WAVEHDR *pwh;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (msg.message == WM__STOP_RECORD) break;
			switch (msg.message)
			{
			case WIM_OPEN:
				if (!already_double_buffered)
				{
					strcpy(record_identifier.callbackfilename, p->callbackname.c_str());
					record_identifier.devID = p->devID;
					record_identifier.fs = p->wfx.nSamplesPerSec;
					record_identifier.len_buffer = p->inBufferLen;
					record_identifier.recordingThread = p->threadID;
					SendMessage(p->hWnd_calling, p->msgID, (WPARAM)&record_identifier, WIM_OPEN); // send the opening status to the main application 
						// if its multi-channel(i.e., stereo), inBufferLen must be multiple of nChan, otherwise left-right channels might be swapped around in the middle
					p->wh[0].dwLoops = p->wh[1].dwLoops = 0;
					// block 0
					p->wh[0].dwFlags = 0;
					p->setPlayPoint(0);
					MMERRTHROW(waveInPrepareHeader(p->hwi, &p->wh[0], sizeof(WAVEHDR)), "waveInPrepareHeader")
					MMERRTHROW(waveInAddBuffer(p->hwi, &p->wh[0], sizeof(WAVEHDR)), "waveInAddBuffer")
					// block 1
					p->wh[1].dwFlags = 0;
					p->setPlayPoint(1);
					MMERRTHROW(waveInPrepareHeader(p->hwi, &p->wh[1], sizeof(WAVEHDR)), "waveInPrepareHeader")
					MMERRTHROW(waveInAddBuffer(p->hwi, &p->wh[1], sizeof(WAVEHDR)), "waveInAddBuffer")
					MMERRTHROW(waveInStart(p->hwi), "waveOutRestart")
					already_double_buffered = true;
				}
				break;
			case WIM_DATA:
				p->nPlayedBlocks++;
				pwh = (WAVEHDR *)msg.lParam;
				record_identifier.buffer = (char*)pwh->lpData;
				if (!record_identifier.closing)
					SendMessage(p->hWnd_calling, p->msgID, (WPARAM)&record_identifier, WIM_DATA);
				p->recordedSamples += pwh->dwBufferLength;
				if (p->recordedSamples < p->totalSamples)
				{
					pwh->dwFlags = 0;
					pwh->dwBytesRecorded = 0;
					MMERRTHROW(waveInPrepareHeader(p->hwi, pwh, sizeof(WAVEHDR)), "waveInPrepareHeader")
					MMERRTHROW(waveInAddBuffer(p->hwi, pwh, sizeof(WAVEHDR)), "waveInAddBuffer")
				}
				else
				{
					MMERRTHROW(waveInStop(p->hwi), "waveInStop")
					MMERRTHROW(waveInReset(p->hwi), "waveInReset")
					for (int k = 0; k < 2; k++)
						rc=waveInUnprepareHeader(p->hwi, &p->wh[k], sizeof(WAVEHDR));
					MMERRTHROW(waveInClose(p->hwi), "waveInReset")
					SendMessage(p->hWnd_calling, p->msgID, (WPARAM)0, WIM_CLOSE); // send the closing status to the main application 
					return;
				}
				break;
			} 
		}
		waveInStop(p->hwi);
		waveInReset(p->hwi);
		for (int k=0; k<2;k++)
			waveInUnprepareHeader(p->hwi, &p->wh[k], sizeof(WAVEHDR));
		waveInClose(p->hwi);
		SendMessage(p->hWnd_calling, p->msgID, (WPARAM)0, WIM_CLOSE); // send the closing status to the main application 
		return ;
	}
	catch (const char * emsg)
	{ // emsg shows where the error occured and the error code converted into a text string, separated by a tab.4
		SendMessage(p->hWnd_calling, p->msgID, (WPARAM)emsg, -1);
		threadIDs[len_threadIDs-- - 1] = 0;
		return ;
	}
}



INT_PTR Capture(int DevID, UINT userDefinedMsgID, HWND hApplWnd, int fs, short nChans, short bytes, const char *callbackname, double duration, double *block_dur_ms, char *errmsg)
{
	WAVEINCAPS cap;
	MMRESULT	rc = 0;
	MMERRTHROW(waveInGetDevCaps(DevID, &cap, sizeof(WAVEINCAPS)), "waveInGetDevCaps")
		if (bytes < 1 || bytes > 3) return 0; // error
	if (nChans > 2) return 0; // error
	int listedfs[] = { 11025, 22050, 44100, 48000, 96000 };
	vector<int> v(listedfs, listedfs + 5);
	vector<int>::iterator where = lower_bound(v.begin(), v.end(), fs);
	if (where == v.end()) where--;
	fs = *where;
	DWORD format;
	switch (fs)
	{
	case 11025:
		format = 0x00000001;
		break;
	case 22050:
		format = 0x00000010;
		break;
	case 44100:
		format = 0x00000100;
		break;
	case 48000:
		format = 0x00001000;
		break;
	case 96000:
		format = 0x00010000;
		break;
	default:
		strcpy(errmsg, "Invalid sampling rate");
		return 0;
	}
	format *= nChans;
	format *= bytes;
	if (!(format & cap.dwFormats))
	{
		strcpy(errmsg, "Invalid wave format");
		return 0;
	}

	unique_ptr<CWaveRecord> pWP(new CWaveRecord);
	pWP->wfx.wBitsPerSample = bytes;
	pWP->totalSamples = lrint(duration / 1000.*fs) * nChans;
	pWP->inBufferLen = lrint(*block_dur_ms / 1000.*fs) * nChans;
	pWP->inBuffer[0] = new char[pWP->inBufferLen * bytes];
	pWP->inBuffer[1] = new char[pWP->inBufferLen * bytes];
	pWP->hWnd_calling = hApplWnd;
	pWP->msgID = userDefinedMsgID;
	pWP->callingThreadID = GetCurrentThreadId();
	pWP->wfx.nChannels = nChans;
	pWP->wfx.nSamplesPerSec = fs;
	pWP->wfx.nBlockAlign = bytes * nChans;
	pWP->wfx.nAvgBytesPerSec = pWP->wfx.nSamplesPerSec * pWP->wfx.nBlockAlign;
	pWP->bitswidth = bytes; // LOOK HERE----------------
	pWP->devID = DevID;
	pWP->callbackname = callbackname;

	thread recordingThread(ThreadCapture, move(pWP));
	recordingThread.join();

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		switch (msg.message)
		{
		case MM_ERROR:
			//error message in msg.lParam??  7/8/2018
			waveOutGetErrorText((MMRESULT)msg.wParam, errmsg, 256);
			return 0;
		case DISPATCH_PLAYBACK_HANDLE:
			return msg.wParam; // audio playback handle is returned. 7/7/2018
		}
	}
	return 0;
}

#endif