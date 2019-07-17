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
#define WM__QUICK_STOP						WM_APP+122

extern HWND hMainAppl;

#ifndef NO_PLAYSND

#define OK			0
#define ERR			-1

#define FINISHED_PLAYING_CLEANUP	11
#define PI 3.141592
double _24bit_to_double(int x); // from csignals.cpp
int _double_to_24bit(double x); // from csignals.cpp

static DWORD threadIDs[64]; // to track thread handle in the present thread (e.g., to track history)  
static int len_threadIDs(0); // to track thread handle in the present thread (e.g., to track history) 

#define WM__STOP			WM_APP+715




typedef struct
{
	char *inBuffer[2];
	int		length;
	int		nChan;
	int		nProgReport;
	int		DevID;
	int		playcount;
} NP;

class CWaveRecord
{
public:
	HWND			hWnd_calling;
	uintptr_t		hThread;
	DWORD			threadID;
	DWORD			callingThreadID;
	UINT			msgID;
	DWORD			totalSamples;
	DWORD			lastPt;
	DWORD			nTotalBlocks;
	DWORD			nPlayedBlocks;
	char *inBuffer[2];
	DWORD			inBufferLen; // Data count being played, not byte count. For stereo, as twice long as mono.

	bool			blockMode;
	int				playcount; // 1 for regular, > 1 for repeated (looping) play. Don't allow 0.
	int				fading;
	int				nFadingBlocks; // The number of blocks for fading.
	vector<NP>		nextPlay;
	HWAVEIN			hwi;
	WAVEINCAPS		wic;
	HWAVEOUT		hwo;
	WAVEOUTCAPS		woc;
	WAVEHDR			wh[2];
	WAVEFORMATEX	wfx;
	AUD_PLAYBACK hPlayStruct;

	CWaveRecord();
	~CWaveRecord();
	int setPlayPoint(int id);
	int OnBlockDone(WAVEHDR* lpwh);
	int preparenextchunk(char *errstr);
	int	cleanUp(int threadIDalreadycut = 0);
private:
	bool passing;
};

static vector<CWaveRecord*> pWlist;

#define FILEOUT {HANDLE hf; DWORD dw; hf = CreateFile("wavex.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);\
	SetFilePointer(hf, NULL,NULL, FILE_END); WriteFile(hf, fout, strlen(fout), &dw, NULL); CloseHandle(hf);}

CWaveRecord::CWaveRecord()
	: hThread(NULL), playcount(1), fading(0), lastPt(0), nPlayedBlocks(0), passing(true)
{
	threadID = GetCurrentThreadId();
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.cbSize = 0;
}

CWaveRecord::~CWaveRecord()
{
	// Do we need to free up any memory blocks that were somehow not taken care of?
}


int CWaveRecord::cleanUp(int IDcut)
{
	PostThreadMessage(threadID, WM__QUICK_STOP, 0, 0);
	if (IDcut == 0)
		threadIDs[len_threadIDs-- - 1] = 0;
	return FINISHED_PLAYING_CLEANUP;
}

int CWaveRecord::setPlayPoint(int id)
{
	wh[id].dwFlags = 0;
	wh[id].dwBufferLength = inBufferLen / wfx.wBitsPerSample * 8;
	// id is waveform header ID; either 0 or 1
	wh[id].lpData = inBuffer[id];
	return 0;
}

int CWaveRecord::OnBlockDone(WAVEHDR* lpwh)
{
	nPlayedBlocks++;
	MMRESULT	rc;
	char errmsg[256];
	double lastPlayedDuration = (double)inBufferLen / hPlayStruct.fs / wfx.nChannels;
	//Assume that hPlayStruct.sig.strut["durLeft"] is CSIG_SCALAR
	double *dbuf = hPlayStruct.sig.strut["durLeft"].buf;
	//Directly update the content of the dbuffer
	*dbuf -= lastPlayedDuration;
	dbuf = hPlayStruct.sig.strut["durPlayed"].buf;
	*dbuf += lastPlayedDuration;

	setPlayPoint((lpwh == wh) ? 0 : 1);
	MMERRTHROW(waveOutPrepareHeader(hwo, lpwh, sizeof(WAVEHDR)), "waveOutPrepareHeader_WOM_DONE")
	MMERRTHROW(waveOutWrite(hwo, lpwh, sizeof(WAVEHDR)), "waveOutWrite_WOM_DONE")
	return 0;
}

int CWaveRecord::preparenextchunk(char *errstr)
{
	MMRESULT	rc=0;
	char errmsg[256];
	int nSamplesInBlock; // this is per each channel
	NP thisnp = nextPlay.back();
	inBuffer[0] = thisnp.inBuffer[0];
	inBuffer[1] = thisnp.inBuffer[1];
	if (thisnp.nChan != wfx.nChannels)
	{
		wfx.nChannels = thisnp.nChan;
		wfx.nBlockAlign = 2 * wfx.nChannels;
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
		//waveOutClose is not necessary before calling waveOutOpen; 
		// a call to waveOutOpen without calling waveOutClose simply updates the content of &hwo and it is harmless. 2/24/2019
		MMERRTHROW(waveOutOpen(&hwo, thisnp.DevID, &wfx, (DWORD_PTR)threadID, (DWORD_PTR)545, CALLBACK_THREAD), "waveOutOpen_playnext")
	}
	totalSamples = thisnp.length * wfx.nChannels;
	playcount = thisnp.playcount;
	nTotalBlocks = max(thisnp.nProgReport, 1);
	nSamplesInBlock = thisnp.length / nTotalBlocks;
	inBufferLen = nSamplesInBlock * wfx.nChannels;

	return rc;
}


typedef struct {
	int DevID;
	char *dataBuffer[2];
	int length;
	int nChan;
	int fs;
	UINT userDefinedMsgID;
	HWND hApplWnd;
	int nProgReport;
	int playcount;
	DWORD callingthreadID;
} PARAM4sndplay;

static vector<PARAM4sndplay> pbparam;

unsigned int WINAPI ThreadCapture(PVOID p)
{
	MSG        msg;
	char errmsg[256];
	bool ch(false);
	bool hist(false);
	MMRESULT	rc;
	vector<CWaveRecord*>::iterator it;
	PARAM4sndplay param = pbparam.back();
	pbparam.pop_back();

	for (auto it=toClose.begin(); it!=toClose.end(); it++)
		waveInClose(*it);
	toClose.clear();

	CWaveRecord *pWP = new CWaveRecord;
	pWP->blockMode = (param.userDefinedMsgID == 0);	// userDefinedMsgID == 0 : blocking (synchronous)
	pWP->inBuffer[0] = param.dataBuffer[0];
	pWP->inBuffer[1] = param.dataBuffer[1];
	pWP->hWnd_calling = param.hApplWnd;
	pWP->msgID = param.userDefinedMsgID;
	pWP->callingThreadID = param.callingthreadID;
	pWP->wfx.nChannels = param.nChan;
	pWP->wfx.nSamplesPerSec = param.fs;
	pWP->wfx.nBlockAlign = 2 * param.nChan;
	pWP->wfx.wBitsPerSample = param.playcount;
	pWP->wfx.nAvgBytesPerSec = pWP->wfx.nSamplesPerSec * pWP->wfx.nBlockAlign;
	pWP->playcount = 0;
	try
	{
		rc = waveInOpen(&pWP->hwi, param.DevID, &pWP->wfx, (DWORD_PTR)pWP->threadID, (DWORD_PTR)0, CALLBACK_THREAD);
		if (rc != MMSYSERR_NOERROR)
		{
			PostThreadMessage(pWP->callingThreadID, MM_ERROR, (WPARAM)rc, (LPARAM)"");
			threadIDs[len_threadIDs-- - 1] = 0;
			delete pWP;
			return 0;
		}
		pWlist.push_back(pWP);

		pWP->totalSamples = 0 ;
		pWP->nTotalBlocks = 0;
		// Initiate playing after double-buffering, any leftover is processed in hMmBase


		pWP->hPlayStruct.pWavePlay = (INT_PTR)pWP;
		pWP->hPlayStruct.fs = param.fs;
		pWP->hPlayStruct.DevID = param.DevID;
		pWP->hPlayStruct.blockDuration = (INT_PTR)((double)param.length / param.fs * 1000.);
		pWP->hPlayStruct.totalDuration = pWP->hPlayStruct.blockDuration * param.playcount;
		double playedPortionsBlock = 0.;
		int originalLoopCounts = param.playcount;
		pWP->hPlayStruct.remainingDuration = pWP->hPlayStruct.totalDuration;
		PostThreadMessage(pWP->callingThreadID, DISPATCH_PLAYBACK_HANDLE, (WPARAM)&pWP->hPlayStruct, (LPARAM)"");

		// Insert LOGGING4
		bool already_double_buffered = false;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (msg.message == WM__QUICK_STOP) break;
			switch (msg.message)
			{
				//WM_APP+WOM_OPEN sent to showvarDlg to notify the beginning and ending of playback
			case WIM_OPEN:
				if (!already_double_buffered)
				{
					SendMessage(pWP->hWnd_calling, pWP->msgID, 0, WIM_OPEN); // send the opening status to the main application 
						// if its multi-channel(i.e., stereo), inBufferLen must be multiple of nChan, otherwise left-right channels might be swapped around in the middle
					pWP->inBufferLen = param.length;
					pWP->nFadingBlocks = 0;
					pWP->wh[0].dwLoops = pWP->wh[1].dwLoops = 0;
					// block 0
					pWP->wh[0].dwFlags = 0;
					pWP->setPlayPoint(0);
					MMERRTHROW(waveInPrepareHeader(pWP->hwi, &pWP->wh[0], sizeof(WAVEHDR)), "waveInPrepareHeader")
					// waveOutPause must come before the first waveOutWrite call; otherwise, occassionally a tiny "blip" in the beginning occurs regardless of the play buffer size. 7/23/2018
					MMERRTHROW(waveInAddBuffer(pWP->hwi, &pWP->wh[0], sizeof(WAVEHDR)), "waveInAddBuffer")

	//				MMERRTHROW(waveInPause(pWP->hwi), "waveInPause")
	//				MMERRTHROW(waveOutWrite(pWP->hwi, &pWP->wh[0], sizeof(WAVEHDR)), "waveOutWrite_0")
					// block 1
					pWP->wh[1].dwFlags = 0;
					pWP->setPlayPoint(1);
					MMERRTHROW(waveInPrepareHeader(pWP->hwi, &pWP->wh[1], sizeof(WAVEHDR)), "waveInPrepareHeader")
					MMERRTHROW(waveInAddBuffer(pWP->hwi, &pWP->wh[1], sizeof(WAVEHDR)), "waveInAddBuffer")
					MMERRTHROW(waveInStart(pWP->hwi), "waveOutRestart")
					already_double_buffered = true;
				}
				break;
			case WOM_CLOSE: //This is no longer processed because waveOutClose is not called while this message playcount is running (i.e., it is called either before or after the message playcount)
				break;
			case WIM_DATA:
				pWP->nPlayedBlocks++;
				SendMessage(pWP->hWnd_calling, pWP->msgID, pWP->nPlayedBlocks, msg.lParam);
				break;
			case WOM_DONE:
				playedPortionsBlock = (double)pWP->nPlayedBlocks / pWP->nTotalBlocks;
				pWP->hPlayStruct.remainingDuration = (INT_PTR)(pWP->hPlayStruct.blockDuration * (pWP->playcount - playedPortionsBlock));
				pWP->OnBlockDone((WAVEHDR *)msg.lParam); // Here, the status (block done playing) is sent to the main application 
				break;
			} 
		}
		pWP->hPlayStruct.sig.strut["durLeft"].SetValue(0.);
		pWP->hPlayStruct.sig.strut["durPlayed"].SetValue(pWP->hPlayStruct.sig.strut["durTotal"].value());
		//It exits the message playcount when cleanUp is called the second time around (the WOM_DONE message for the block 1 is posted)
		rc = waveInUnprepareHeader(pWP->hwi, &pWP->wh[0], sizeof(WAVEHDR));
		rc = waveInUnprepareHeader(pWP->hwi, &pWP->wh[1], sizeof(WAVEHDR));
		toClose.push_back(pWP->hwi);
		if (pWP->blockMode)	PostThreadMessage(pWP->callingThreadID, WM__RETURN, OK, 0);
		if (pWP->blockMode)
			PostThreadMessage(pWP->callingThreadID, MM_DONE, 0, 0); // This seems redundant with if (blockMode)	PostThreadMessage (callingThreadID, WM__RETURN, OK, 0); above...
															   //Previously I thought that it was OK to post MM_DONE again just in case and wouldn't do any harm. 
															   //But, in fact, if MM_DONE is posted after the thread message handler is no longer available (because the call PlayArray() already returned), 
															   //it lurks around and when the message handler is available again for a subsequent PlayArray() call, it picks up right away...
															   //causing a blocking PlayArray() call to return right away... 11/9/2017 bjk
		SendMessage(pWP->hWnd_calling, pWP->msgID, (WPARAM)&pWP->hPlayStruct.sig, WOM_CLOSE); // send the closing status to the main application 
		it = find(pWlist.begin(), pWlist.end(), pWP);
		if (it != pWlist.end())
			pWlist.erase(it);
		delete pWP;
		return (unsigned int)msg.wParam;
	}
	catch (const char * emsg)
	{ // emsg shows where the error occured and the error code converted into a text string, separated by a tab.4
		SendMessage(pWP->hWnd_calling, pWP->msgID, (WPARAM)emsg, -1);
		it = find(pWlist.begin(), pWlist.end(), pWP);
		if (it != pWlist.end())
			pWlist.erase(it);
		threadIDs[len_threadIDs-- - 1] = 0;
		delete pWP;
		return 0;
	}
}


WPARAM init_capture_thread(UINT DevID, UINT userDefinedMsgID, HWND hApplWnd, int fs, int nChan, int bits, char *buffer1, char *buffer2, int bufferlen, double block_dur_ms, char *errstr)
{
	PARAM4sndplay param;
	param.dataBuffer[0] = buffer1;
	param.dataBuffer[1] = buffer2;
	param.DevID = DevID;
	param.fs = fs;
	param.hApplWnd = hApplWnd;
	param.length = 0;
	param.nChan = nChan;
	param.nProgReport = 0;
	param.userDefinedMsgID = userDefinedMsgID;
	param.callingthreadID = GetCurrentThreadId();
	param.playcount = bits; // LOOK HERE----------------
	param.length = bufferlen;
	pbparam.push_back(param);

	static NP thisnp;

	//	if (mt==NULL)  mt = CreateMutex(0,0,0);
	threadIDs[len_threadIDs++] = GetThreadId((HANDLE)_beginthreadex(NULL, 0, ThreadCapture, NULL, NULL, 0));

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		switch (msg.message)
		{
		case MM_ERROR:
			//error message in msg.lParam??  7/8/2018
			waveOutGetErrorText((MMRESULT)msg.wParam, errstr, 256);
			return 0;
		case DISPATCH_PLAYBACK_HANDLE:
			return msg.wParam; // audio playback handle is returned. 7/7/2018
		}
	}
	// Does the while playcount above ever exit? 3/18/2017 bjkwon
	return 0;
}

INT_PTR Capture(int DevID, UINT userDefinedMsgID, HWND hApplWnd, int fs, int nChans, int bits, double *block_dur_ms, char *errmsg)
{
	WAVEINCAPS cap;
	MMRESULT	rc = 0;
	MMERRTHROW(waveInGetDevCaps(DevID, &cap, sizeof(WAVEINCAPS)), "waveInGetDevCaps")
		if (bits != 8 && bits != 16) return 0; // error
	int _bits = bits;
	_bits /= 8;
	_bits *= 2;
	if (nChans > 2) return 0; // error
	DWORD format;
	if (nChans > 2) return 0; // error
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
	format *= _bits;
	if (!(format & cap.dwFormats))
	{
		strcpy(errmsg, "Invalid wave format");
		return 0;
	}
	char *inBuffer[2];
	int bufferlen = 4096;
	inBuffer[0] = new char[bufferlen];
	inBuffer[1] = new char[bufferlen];

	return (INT_PTR)init_capture_thread(DevID, userDefinedMsgID, hApplWnd, fs, nChans, bits, inBuffer[0], inBuffer[1], bufferlen, *block_dur_ms, errmsg);
}

#endif