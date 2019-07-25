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

using namespace std;

extern DWORD threadID;

char waveErrMsg[256]; // to be used to send message to the main app (OnSoundEvent)

#define MMERRTHROW(X,MM) {rc=X; \
if (rc!=MMSYSERR_NOERROR) { sprintf(waveErrMsg, "Error in %s\n", #MM); \
char _estr_[256]; waveOutGetErrorText(rc, _estr_, 256); strcat(waveErrMsg, _estr_); \
PostThreadMessage(threadID, pWP->msgID, (WPARAM)waveErrMsg, -1); return ;}}

#define MMERRRETURNFALSE(MM) pWP->cleanUp(); char errmsg[256],  sprintf(errmsg, "Error in %s, code=%d\n%s", #MM, rc, _estr_); SendMessage(pWP->hWnd_calling, pWP->msgID,(WPARAM)errmsg,-1); return false;


static vector<HWAVEIN> toClose;

//FYI: These two message are sent to different threads.
#define WM__RETURN		WM_APP+10
#define MM_DONE			WM_APP+11
#define MM_ERROR		WM_APP+12

#define DISPATCH_RECORD_INITIATION			WM_APP+120
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

typedef struct {
	short devID;
	short bytes;
	int fs;
	int nChans;
	HWND hWnd_calling;
	DWORD msgID;
	DWORD callingThreadID;
	double duration;
	double block_dur_ms;
	string callback;
} record_param;

typedef struct
{
	short devID;
	bool closing = 0;
	DWORD recordingThread;
	int fs;
	int len_buffer;
	char *buffer;
	char callbackfilename[256];
} record_identifier;

void ThreadCapture(const record_param &p)
{
	MSG        msg;
	bool ch(false);
	bool hist(false);

//	unique_ptr<CWaveRecord> pWP = make_unique<CWaveRecord>();
	CWaveRecord * pWP = new CWaveRecord;
	pWP->threadID = GetCurrentThreadId();
	pWP->wfx.wBitsPerSample = 8 * p.bytes;
	pWP->totalSamples = lrint(p.duration / 1000.*p.fs) * p.nChans;
	pWP->inBufferLen = lrint(p.block_dur_ms / 1000.*p.fs) * p.nChans;
	pWP->inBuffer[0] = new char[pWP->inBufferLen * p.bytes];
	pWP->inBuffer[1] = new char[pWP->inBufferLen * p.bytes];
	pWP->hWnd_calling = p.hWnd_calling;
	pWP->msgID = p.msgID;
	pWP->callingThreadID = p.callingThreadID;
	pWP->wfx.nChannels = p.nChans;
	pWP->wfx.nSamplesPerSec = p.fs;
	pWP->wfx.nBlockAlign = p.bytes * p.nChans;
	pWP->wfx.nAvgBytesPerSec = pWP->wfx.nSamplesPerSec * pWP->wfx.nBlockAlign;
	pWP->bitswidth = p.bytes; // LOOK HERE----------------
	pWP->devID = p.devID;
	pWP->callbackname = p.callback;


	MMRESULT	rc = waveInOpen(&pWP->hwi, 0, &pWP->wfx, (DWORD_PTR)pWP->threadID, (DWORD_PTR)0, CALLBACK_THREAD);
	if (rc != MMSYSERR_NOERROR) {
		PostThreadMessage(pWP->callingThreadID, MM_ERROR, (WPARAM)rc, (LPARAM)"waveInOpen"); return;
	}
	PostThreadMessage(pWP->callingThreadID, DISPATCH_RECORD_INITIATION, (WPARAM)0, (LPARAM)"");

	// Insert LOGGING4
	WAVEHDR *pwh;
	record_identifier send2OnSoundEven;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM__STOP_RECORD) break;
		switch (msg.message)
		{
		case WIM_OPEN:
			strcpy(send2OnSoundEven.callbackfilename, pWP->callbackname.c_str());
			send2OnSoundEven.devID = pWP->devID;
			send2OnSoundEven.fs = pWP->wfx.nSamplesPerSec;
			send2OnSoundEven.len_buffer = pWP->inBufferLen;
			send2OnSoundEven.recordingThread = pWP->threadID;
			SendMessage(pWP->hWnd_calling, pWP->msgID, (WPARAM)&send2OnSoundEven, WIM_OPEN); // send the opening status to the main application 
				// if its multi-channel(i.e., stereo), inBufferLen must be multiple of nChan, otherwise left-right channels might be swapped around in the middle
			pWP->wh[0].dwLoops = pWP->wh[1].dwLoops = 0;
			// block 0
			pWP->wh[0].dwFlags = 0;
			pWP->setPlayPoint(0);
			MMERRTHROW(waveInPrepareHeader(pWP->hwi, &pWP->wh[0], sizeof(WAVEHDR)), "waveInPrepareHeader")
			MMERRTHROW(waveInAddBuffer(pWP->hwi, &pWP->wh[0], sizeof(WAVEHDR)), "waveInAddBuffer")
			// block 1
			pWP->wh[1].dwFlags = 0;
			pWP->setPlayPoint(1);
			MMERRTHROW(waveInPrepareHeader(pWP->hwi, &pWP->wh[1], sizeof(WAVEHDR)), "waveInPrepareHeader")
			MMERRTHROW(waveInAddBuffer(pWP->hwi, &pWP->wh[1], sizeof(WAVEHDR)), "waveInAddBuffer")
			MMERRTHROW(waveInStart(pWP->hwi), "waveOutRestart")
			break;
		case WIM_DATA:
			pWP->nPlayedBlocks++;
			pwh = (WAVEHDR *)msg.lParam;
			send2OnSoundEven.buffer = (char*)pwh->lpData;
			if (!send2OnSoundEven.closing)
				SendMessage(pWP->hWnd_calling, pWP->msgID, (WPARAM)&send2OnSoundEven, WIM_DATA);
			pWP->recordedSamples += pwh->dwBufferLength;
			if (pWP->recordedSamples < pWP->totalSamples)
			{
				pwh->dwFlags = 0;
				pwh->dwBytesRecorded = 0;
				MMERRTHROW(waveInPrepareHeader(pWP->hwi, pwh, sizeof(WAVEHDR)), "waveInPrepareHeader")
				MMERRTHROW(waveInAddBuffer(pWP->hwi, pwh, sizeof(WAVEHDR)), "waveInAddBuffer")
			}
			else
			{
				MMERRTHROW(waveInStop(pWP->hwi), "waveInStop")
				MMERRTHROW(waveInReset(pWP->hwi), "waveInReset")
				for (int k = 0; k < 2; k++)
					rc=waveInUnprepareHeader(pWP->hwi, &pWP->wh[k], sizeof(WAVEHDR));
				MMERRTHROW(waveInClose(pWP->hwi), "waveInReset")
				SendMessage(pWP->hWnd_calling, pWP->msgID, (WPARAM)0, WIM_CLOSE); // send the closing status to the main application 
				return;
			}
			break;
		} 
	}
	waveInStop(pWP->hwi);
	waveInReset(pWP->hwi);
	for (int k=0; k<2; k++)
		waveInUnprepareHeader(pWP->hwi, &pWP->wh[k], sizeof(WAVEHDR));
	waveInClose(pWP->hwi);
	SendMessage(pWP->hWnd_calling, pWP->msgID, (WPARAM)0, WIM_CLOSE); // send the closing status to the main application 
}



INT_PTR Capture(int DevID, UINT userDefinedMsgID, HWND hApplWnd, int fs, short nChans, short bytes, const char *callbackname, double duration, double *block_dur_ms, char *errmsg)
{
	//This returns error (-1) immediately if waveInGetDevCaps or waveInOpen fails, or any input parameter is improper
	//Any unsuccessful results from waveInXXXX calls are to be handled during event notification
	try {
		char estr[256];
		WAVEINCAPS cap;
		MMRESULT	rc = 0;
		rc = waveInGetDevCaps(DevID, &cap, sizeof(WAVEINCAPS));
		if (rc != MMSYSERR_NOERROR)
		{
			strcpy(errmsg, "Error in waveInGetDevCaps\n");
			waveInGetErrorText(rc, estr, 256);
			strcat(errmsg, estr);
			throw errmsg;
		}

		if (bytes < 1 || bytes > 3)
			throw "Waveform format bytes must be 1 2 or 3.";
		if (nChans > 2)
			throw "Number of channels must be 1 or 2.";
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
		default: // not necessary because fs was locked into the grid above.
			throw "Invalid sampling rate";
		}
		format *= nChans;
		format *= bytes;
		if (!(format & cap.dwFormats))
			throw "Invalid WAVE format";

		record_param carrier;

		carrier.callingThreadID = GetCurrentThreadId();
		carrier.bytes = bytes;
		carrier.fs = fs;
		carrier.nChans = nChans;
		carrier.duration = duration;
		carrier.block_dur_ms = *block_dur_ms;
		carrier.hWnd_calling = hApplWnd;
		carrier.msgID = userDefinedMsgID;
		carrier.callback = callbackname;

		thread recordingThread(ThreadCapture, carrier);
//		recordingThread.join();

		MSG msg;
		FILE*fp;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			switch (msg.message)
			{
			case MM_ERROR:
				sprintf(errmsg, "Error in %s\n", (char*)msg.lParam);
				waveInGetErrorText(rc, estr, 256);
				strcat(errmsg, estr);
				throw errmsg;
			case DISPATCH_RECORD_INITIATION:
				fp = fopen("log.txt", "at");
				fprintf(fp, "DISPATCH_RECORD_INITIATION\n");
				fclose(fp);
				recordingThread.detach();
				return 1; //success
			}
		}
	}
	catch (const char *emsg)
	{
		return -1;
	}
	return 0; // unlikely to come here
}

#endif