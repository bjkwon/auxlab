// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: wavplay
// Library to play audio signals 
// For Windows only
// 
// Version: 1.499
// Date: 12/23/2018
// 

#include "wavplay.h"
#include <process.h>
#include <vector>
#include <time.h>

#define MMERRTHROW(X,MM) rc=X; if (rc!=MMSYSERR_NOERROR) {char _estr_[256]; waveOutGetErrorText(rc, _estr_, 256); sprintf(errmsg, "Error on %s, code=%d\n%s", #MM, rc, _estr_); throw errmsg;}
#define MMERRRETURNFALSE(MM) pWP->cleanUp(); char errmsg[256], _estr_[256]; waveOutGetErrorText(rc, _estr_, 256); sprintf(errmsg, "Error in %s, code=%d\n%s", #MM, rc, _estr_); SendMessage(pWP->hWnd_calling, pWP->msgID,(WPARAM)errmsg,-1); return false;

using namespace std;

//FYI: These two message are sent to different threads.
#define WM__RETURN		WM_APP+10
#define MM_DONE			WM_APP+11
#define MM_ERROR		WM_APP+12

#define DISPATCH_PLAYBACK_HANDLE			WM_APP+120
#define DISPATCH_PLAYBACK_HANDLE_STATUS		WM_APP+121
#define WM__QUICK_STOP						WM_APP+122

HWND hMainAppl(NULL);

// Temporary
void GetLocalTimeStr(char *buff)
{
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	sprintf(buff, "[%02d:%02d:%02d:%02d]", lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
}
// End of Temporary

void SetHWND_WAVPLAY(HWND h)
{
	hMainAppl = h;
}

HWND GetHWND_WAVPLAY()
{
	return hMainAppl;
}

#ifndef NO_PLAYSND

#define OK			0
#define ERR			-1

#define FINISHED_PLAYING_CLEANUP	11
#define PI 3.141592
double _24bit_to_double(int x); // from csignals.cpp
int _double_to_24bit(double x); // from csignals.cpp

DWORD threadIDs[64]; // to track thread handle in the present thread (e.g., to track history)  
int len_threadIDs(0); // to track thread handle in the present thread (e.g., to track history) 

vector<HWAVEOUT> toClose;

#define WM__STOP			WM_APP+715
#define WM__FADEOUT			WM_APP+717

#define LOG(X) fprintf(fp,(X));

typedef struct
{
	VOID*	playBuffer;
	int		length;
	int		nChan;
	int		nProgReport;
	int		DevID;
	int		playcount;
} NP;

class CWavePlay
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
	DWORD			playBufferLen; // Data count being played, not byte count. For stereo, as twice long as mono.

	bool			blockMode;
	int				playcount; // 1 for regular, > 1 for repeated (looping) play. Don't allow 0.
	int				fading;
	int				nFadeOutSamples;
	int				nFadingBlocks; // The number of blocks for fading.
	VOID*			playBuffer;
	void*			doomedPt;
	vector<NP>		nextPlay;
	HWAVEOUT		hwo;
	WAVEOUTCAPS		woc;
	WAVEHDR			wh[2];
	WAVEFORMATEX	wfx;
	vector<short*>	buffer2Clean;
	double *fadeoutEnv;
	AUD_PLAYBACK hPlayStruct;

	CWavePlay();
	~CWavePlay();
	int setPlayPoint(int id);
	void *FadeOut(DWORD offset);
	int OnBlockDone(WAVEHDR* lpwh);
	int preparenextchunk(char *errstr);
	int	cleanUp(int threadIDalreadycut = 0);
private:
	bool passing;
};

vector<CWavePlay*> pWlist;

#define FILEOUT {HANDLE hf; DWORD dw; hf = CreateFile("wavex.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);\
	SetFilePointer(hf, NULL,NULL, FILE_END); WriteFile(hf, fout, strlen(fout), &dw, NULL); CloseHandle(hf);}

CWavePlay::CWavePlay()
	: playBuffer(NULL), hThread(NULL), playcount(1), fading(0), lastPt(0), nPlayedBlocks(0), doomedPt(NULL), fadeoutEnv(NULL), passing(true)
{
	threadID = GetCurrentThreadId();
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.wBitsPerSample = 16;
	wfx.cbSize = 16;
}

CWavePlay::~CWavePlay()
{
	if (fadeoutEnv)	
		delete[] fadeoutEnv;
	fadeoutEnv = NULL;
	// Do we need to free up any memory blocks that were somehow not taken care of?
	// Probably not, because buffer2Clean takes care of cleaning well. 
	// But think about a possibility where somehow cleanUp() is not called properly. 7/11/2016 bjk
}


int CWavePlay::cleanUp(int IDcut)
{
	PostThreadMessage(threadID, WM__QUICK_STOP, 0, 0);
	if (IDcut == 0)
		threadIDs[len_threadIDs-- - 1] = 0;
	return FINISHED_PLAYING_CLEANUP;
}

void *CWavePlay::FadeOut(DWORD offset)
{
	int remaining(0);
	double val;
	short *sbuf = (short*)playBuffer;
	if (offset == totalSamples) {
		offset = 0;
	}
	DWORD nn;
	if (wfx.wBitsPerSample == 16)
	{
		DWORD k(0), kr(offset);
		for (; k < (DWORD)nFadeOutSamples && kr<totalSamples; k++, kr++)
		{
			val = _24bit_to_double(sbuf[kr]);
			val *= fadeoutEnv[k];
			sbuf[kr] = _double_to_24bit(val);
		}
		if (kr == totalSamples)
		{ 
			// If nextPlay is not empty, reset k at 0 and that buffer block should be modified. Do it in the future 2/20/2019
			for (kr = 0; k < (DWORD)nFadeOutSamples; kr++, k++)
			{
				val = _24bit_to_double(sbuf[kr]);
				val *= fadeoutEnv[k];
				sbuf[kr] = _double_to_24bit(val);
			}
			//Need to zero-padd until the end of current playbuffer
			//Current playbuffer ends  
			nn = (DWORD)ceil((double)nFadeOutSamples / playBufferLen);
			memset(sbuf + kr, 0, sizeof(short)*(nn*playBufferLen - kr));
		}
		else
		{
			// nFadingBlocks was overestimated.
			// If the call is made during early looping, nFadingBlocks should be reduced by one
			// Otherwise, OnBlockDone leads to another setPlayPoint, after fading out is over... so it's like faded out then BLIPPED!
			//2/20/2019
			nFadingBlocks--;
			nn = (DWORD)ceil((double)(offset+nFadeOutSamples) / playBufferLen);
			memset(sbuf + kr, 0, sizeof(short)*(nn*playBufferLen - kr));
		}
	}
	return (short*)playBuffer + offset;
}

int CWavePlay::setPlayPoint(int id)
{
	// id is waveform header ID; either 0 or 1
	if (wfx.wBitsPerSample == 8)
		wh[id].lpData = (char*)playBuffer + lastPt;
	else if (wfx.wBitsPerSample == 16)
		wh[id].lpData = (char*)((short*)playBuffer + lastPt);
	wh[id].dwFlags = 0;
	wh[id].dwBufferLength = playBufferLen * wfx.wBitsPerSample / 8;
	lastPt += playBufferLen;

	return 0;
}

int CWavePlay::OnBlockDone(WAVEHDR* lpwh)
{
	nPlayedBlocks++;
	MMRESULT	rc;
	char errmsg[256], errstr[256];
	unsigned int remainingSamples;
	double lastPlayedDuration = (double)playBufferLen / hPlayStruct.fs / wfx.nChannels;
	//Assume that hPlayStruct.sig.strut["durLeft"] is CSIG_SCALAR
	double *dbuf = hPlayStruct.sig.strut["durLeft"].buf;
	//Directly update the content of the dbuffer
	*dbuf -= lastPlayedDuration;
	dbuf = hPlayStruct.sig.strut["durPlayed"].buf;
	*dbuf += lastPlayedDuration;

	if (!blockMode)
		SendMessage(hWnd_calling, msgID, (WPARAM)&hPlayStruct.sig, nPlayedBlocks);
	remainingSamples = totalSamples - lastPt;
	if (remainingSamples < 2) remainingSamples = 0;
	if (remainingSamples >= playBufferLen && remainingSamples < 2 * playBufferLen)
		playBufferLen = remainingSamples;
	if (doomedPt)
	{
		if (lpwh->lpData == doomedPt) 	fading = 1;
		if (fading)
		{
			if (nFadingBlocks > 1)
				nFadingBlocks--;
			else
			{
				playcount = 0;
				// waveOutReset is necessary to stop further posting WOM_DONE (i.e., without this call, it may post even after "this" pointer is cleared. 7/23/2018 
				MMERRTHROW(waveOutReset(hwo), "waveOutReset_WOM_DONE")
				return cleanUp();
			}
		}
	}
	if (remainingSamples < playBufferLen)
	{
		if (playcount == 1)
		{
			if (nextPlay.empty())
			{
				if (passing)
				{
					passing = false;
					return 0;
				}
				else
				{
					nPlayedBlocks = lastPt = 0;
					return cleanUp();
				}
			}
			else
			{
				if (passing)
				{
					passing = false;
					preparenextchunk(errstr); // initiate nextplay 
				}
				//if remainingSamples is not zero, it should pass through and call waveOutPrepareHeader one more time
				//next time around, remainingSamples should be zero and setPlayPoint is called with the new header
				if (remainingSamples == 0) //the last OnDone
				{
					nPlayedBlocks = 0;
					lastPt = 0; // Resetting for setPlayPoint
					passing = true;
				}
			}
		}
		else
		{
			playcount--;
			nPlayedBlocks = lastPt = 0;
			passing = true;
			playBufferLen = totalSamples / nTotalBlocks * wfx.nChannels;
		}
	}
	setPlayPoint((lpwh == wh) ? 0 : 1);
	MMERRTHROW(waveOutPrepareHeader(hwo, lpwh, sizeof(WAVEHDR)), "waveOutPrepareHeader_WOM_DONE")
	MMERRTHROW(waveOutWrite(hwo, lpwh, sizeof(WAVEHDR)), "waveOutWrite_WOM_DONE")
	return 0;
}

int CWavePlay::preparenextchunk(char *errstr)
{
	MMRESULT	rc=0;
	char errmsg[256];
	int nSamplesInBlock; // this is per each channel
	NP thisnp = nextPlay.back();
	playBuffer = thisnp.playBuffer;
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
	playBufferLen = nSamplesInBlock * wfx.nChannels;

	nextPlay.pop_back();
	buffer2Clean.push_back((short*)playBuffer);
	return rc;
}


typedef struct {
	int DevID;
	SHORT *dataBuffer;
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

unsigned int WINAPI Thread4MM(PVOID p)
{
	MSG        msg;
	char errmsg[256];
	bool ch(false);
	bool hist(false);
	MMRESULT	rc;
	vector<CWavePlay*>::iterator it;
	PARAM4sndplay param = pbparam.back();
	pbparam.pop_back();

	for (vector<HWAVEOUT>::iterator it=toClose.begin(); it!=toClose.end(); it++)
		waveOutClose(*it);
	toClose.clear();

	CWavePlay *pWP = new CWavePlay;
	pWP->blockMode = (param.userDefinedMsgID == 0);	// userDefinedMsgID == 0 : blocking (synchronous)
	pWP->playBuffer = param.dataBuffer;
	pWP->hWnd_calling = param.hApplWnd;
	pWP->msgID = param.userDefinedMsgID;
	pWP->callingThreadID = param.callingthreadID;
	pWP->wfx.nChannels = param.nChan;
	pWP->wfx.nSamplesPerSec = param.fs;
	pWP->wfx.nBlockAlign = 2 * param.nChan;
	pWP->wfx.nAvgBytesPerSec = pWP->wfx.nSamplesPerSec * pWP->wfx.nBlockAlign;
	pWP->playcount = param.playcount;

	try
	{
		rc = waveOutOpen(&pWP->hwo, param.DevID, &pWP->wfx, (DWORD_PTR)pWP->threadID, (DWORD_PTR)0, CALLBACK_THREAD);
		if (rc != MMSYSERR_NOERROR)
		{
			PostThreadMessage(pWP->callingThreadID, MM_ERROR, (WPARAM)rc, (LPARAM)"");
			threadIDs[len_threadIDs-- - 1] = 0;
			delete pWP;
			return 0;
		}
		pWlist.push_back(pWP);

		pWP->totalSamples = param.nChan *param.length ;
		pWP->nTotalBlocks = param.nProgReport;
		// Initiate playing after double-buffering, any leftover is processed in hMmBase

		//Fade-Out duration = 350 ms for now (12/8/2017). if fs is 10000Hz, this is 3500
		pWP->nFadeOutSamples = (int)(.35 * pWP->wfx.nSamplesPerSec);
		pWP->fadeoutEnv = new double[pWP->nFadeOutSamples];
		for (int k = 1; k <= pWP->nFadeOutSamples; k++)
			pWP->fadeoutEnv[k - 1] = (exp((double)(pWP->nFadeOutSamples - k) / pWP->nFadeOutSamples) - 1) / (exp(1.) - 1);

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
			case WOM_OPEN:
				if (!already_double_buffered)
				{
					SendMessage(pWP->hWnd_calling, pWP->msgID, 0, WOM_OPEN); // send the opening status to the main application 
					// if its multi-channel(i.e., stereo), playBufferLen must be multiple of nChan, otherwise left-right channels might be swapped around in the middle
					pWP->playBufferLen = pWP->wfx.nChannels * param.length / pWP->nTotalBlocks;
					pWP->nFadingBlocks = (int)ceil(((double)pWP->nFadeOutSamples / pWP->playBufferLen));
					pWP->wh[0].dwLoops = pWP->wh[1].dwLoops = 0;
					// block 0
					pWP->setPlayPoint(0);
					pWP->wh[0].dwFlags |= WHDR_BEGINLOOP;
					MMERRTHROW(waveOutPrepareHeader(pWP->hwo, &pWP->wh[0], sizeof(WAVEHDR)), "waveOutPrepareHeader_0")
					// waveOutPause must come before the first waveOutWrite call; otherwise, occassionally a tiny "blip" in the beginning occurs regardless of the play buffer size. 7/23/2018
					MMERRTHROW(waveOutPause(pWP->hwo), "waveOutPause")
					MMERRTHROW(waveOutWrite(pWP->hwo, &pWP->wh[0], sizeof(WAVEHDR)), "waveOutWrite_0")
					// block 1
					pWP->setPlayPoint(1);
					pWP->wh[1].dwFlags |= WHDR_ENDLOOP;
					MMERRTHROW(waveOutPrepareHeader(pWP->hwo, &pWP->wh[1], sizeof(WAVEHDR)), "waveOutPrepareHeader_1")
					MMERRTHROW(waveOutWrite(pWP->hwo, &pWP->wh[1], sizeof(WAVEHDR)), "waveOutWrite_1")
					MMERRTHROW(waveOutRestart(pWP->hwo), "waveOutRestart")
					// Insert LOGGING5
					// IMPORTANT--DO NOT ASSUME THAT pWP->playBuffer == param->dataBuffer in a multithread environment.
					pWP->buffer2Clean.push_back((SHORT*)pWP->playBuffer);
					//	pWP->buffer2Clean.push_back(param->dataBuffer); // this will lead to an incorrect pointer and proper buffer is not stored and something else might be stored twice, which will crash the application. 7/11/2016 bjk
					already_double_buffered = true;
				}
				break;
			case WOM_CLOSE: //This is no longer processed because waveOutClose is not called while this message playcount is running (i.e., it is called either before or after the message playcount)
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
		rc = waveOutUnprepareHeader(pWP->hwo, &pWP->wh[0], sizeof(WAVEHDR));
		rc = waveOutUnprepareHeader(pWP->hwo, &pWP->wh[1], sizeof(WAVEHDR));
		toClose.push_back(pWP->hwo);
		if (pWP->blockMode)	PostThreadMessage(pWP->callingThreadID, WM__RETURN, OK, 0);
		for (size_t k = pWP->buffer2Clean.size(); k > 0; k--)
		{
			if (pWP->buffer2Clean.back() == pWP->buffer2Clean[k - 1])
				pWP->buffer2Clean.pop_back();
		}
		for (size_t k = 0; k < pWP->buffer2Clean.size(); k++)
			delete[] pWP->buffer2Clean[k];
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

WPARAM wavBuffer2snd(UINT DevID, SHORT *dataBuffer, int length, int nChan, int fs, UINT userDefinedMsgID, HWND hApplWnd, int nProgReport, int playcount, char *errstr)
// userDefinedMsgID == 0 : blocking (synchronous)
{
	PARAM4sndplay param;
	param.dataBuffer = dataBuffer;
	param.DevID = DevID;
	param.fs = fs;
	param.hApplWnd = hApplWnd;
	param.length = length;
	param.nChan = nChan;
	param.nProgReport = max(nProgReport,1);
	param.userDefinedMsgID = userDefinedMsgID;
	param.callingthreadID = GetCurrentThreadId();
	param.playcount = playcount;
	pbparam.push_back(param);

	static NP thisnp;

	//	if (mt==NULL)  mt = CreateMutex(0,0,0);
	threadIDs[len_threadIDs++] = GetThreadId((HANDLE)_beginthreadex(NULL, 0, Thread4MM, NULL, NULL, 0));

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

//DO NOT CALL PlayBufAsynch16 DIRECTLY FROM OUTSIDE.... IT WILL CRASH AS IT TRIES TO FREE dataBuffer
// Instead, call the member function PlayArray to play something.

INT_PTR PlayBufAsynch16(UINT DevID, short *dataBuffer, int length, int nChan, int fs, UINT userDefinedMsgID, HWND hApplWnd, int nProgReport, int playcount, char* errstr)
// playcount play when nProgReport == -1
{
	if (nProgReport == -1 && userDefinedMsgID == 0)
		userDefinedMsgID = WM_USER + 293;	// default message for event notification
	return wavBuffer2snd(DevID, dataBuffer, length, nChan, fs, userDefinedMsgID, hApplWnd, nProgReport, playcount, errstr);
}

bool FindCWavePlay(INT_PTR pHandle)
{
	AUD_PLAYBACK *paud = (AUD_PLAYBACK*)pHandle;
	for (vector<CWavePlay*>::iterator it = pWlist.begin(); it != pWlist.end(); it++)
		if (paud->pWavePlay == (INT_PTR)*it)
			return true;
	return false;
}

INT_PTR QueuePlay(INT_PTR pHandle, UINT DevID, SHORT *dataBuffer, int length, int nChan, UINT userDefinedMsgID, int nProgReport, char *errstr, int playcount)
{
	if (!FindCWavePlay(pHandle)) return NULL;
	CWavePlay *pWP = (CWavePlay *)((AUD_PLAYBACK *)pHandle)->pWavePlay;
	DWORD id = threadIDs[len_threadIDs - 1];
	if (id == 0) 	return NULL;
	static NP thisnp;
	thisnp.nChan = nChan;
	thisnp.length = length;
	thisnp.nProgReport = nProgReport;
	thisnp.playBuffer = dataBuffer;
	thisnp.DevID = DevID;
	thisnp.playcount = playcount;

	if (!pWP) // in AUXLAB, pWP is never null. If the application needs to call pWP with null, check this line again (at least length should be checked again) 2/24/2019
		return (INT_PTR)wavBuffer2snd(DevID, dataBuffer, length, nChan, pWP->wfx.nSamplesPerSec, userDefinedMsgID, pWP->hWnd_calling, nProgReport, playcount, errstr);
	else
	{
		pWP->nextPlay.push_back(thisnp);
		//Assume that pWP->hPlayStruct.sig.strut["durLeft"] is CSIG_SCALAR
		double *buf = pWP->hPlayStruct.sig.strut["durLeft"].buf;
		//Directly update the content of the buffer
		*buf += (double)length / pWP->wfx.nSamplesPerSec * playcount;
	}
	return (INT_PTR)pHandle;
}

void TerminatePlay(int quick) 
{ // still used in auxcondlg.cpp but soon to be gone 8/16/2018
	if (len_threadIDs == 0 || threadIDs[len_threadIDs - 1] == 0) return;
	if (quick)
		PostThreadMessage(threadIDs[len_threadIDs - 1], WM__STOP, 0, 1); //
	else
		PostThreadMessage(threadIDs[len_threadIDs - 1], WM__FADEOUT, 0, 1); //
	threadIDs[len_threadIDs-- - 1] = 0;
}

int WinMMGetVolume(INT_PTR pHandle, DWORD &vol, char *errstr)
{
	CWavePlay *pWP = (CWavePlay *)((AUD_PLAYBACK *)pHandle)->pWavePlay;
	MMRESULT res = waveOutGetVolume(pWP->hwo, &vol);
	if (res != MMSYSERR_NOERROR) return 0;
	else
	{
		sprintf(errstr, "code=%d", res);
		return -1;
	}
}

int WinMMSetVolume(INT_PTR pHandle, DWORD vol, char *errstr)
{
	CWavePlay *pWP = (CWavePlay *)((AUD_PLAYBACK *)pHandle)->pWavePlay;
	MMRESULT res = waveOutSetVolume(pWP->hwo, vol);
	if (res != MMSYSERR_NOERROR) return 0;
	else
	{
		sprintf(errstr, "code=%d", res);
		return -1;
	}
}

bool _qstopplay(CWavePlay *pWP)
{
	pWP->cleanUp();
	MMRESULT	rc= waveOutReset(pWP->hwo);
	if (rc == MMSYSERR_NOERROR) return true;
	MMERRRETURNFALSE("waveOutRestart or waveOutPause")
}

bool StopPlay(INT_PTR pHandle, bool quick)
{
	if (!pHandle) // stop all audio playing events
	{
		bool res(true);
		for (vector<CWavePlay*>::iterator it = pWlist.begin(); it != pWlist.end(); it++)
			res &= _qstopplay(*it);
		return res;
	}
	if (!FindCWavePlay(pHandle)) return false;
	CWavePlay *pWP = (CWavePlay *)((AUD_PLAYBACK *)pHandle)->pWavePlay;
	if (quick)
	{
		_qstopplay(pWP);
	}
	else
	{
		// Upon the user request for stop, FadeOut modifies the buffer and "activates" doomedPt
		// so that from the subsequent processing of OnBlockDone, it starts its own countdown to stop.
		// til it reaches the full fade out interval, it continues the operation of waveOutWrite--OnBlockDone.
		//Fade-out duration = lesser of 200ms and what's left in the buffer --from previous comment, needs to check 7/16/2018
		pWP->doomedPt = pWP->FadeOut(pWP->lastPt);
	}
	return true;
}

bool PauseResumePlay(INT_PTR pHandle, bool fOnOff)
{
	if (pHandle == NULL) return false;
	if (!FindCWavePlay(pHandle)) return false;
	CWavePlay *pWP = (CWavePlay *)((AUD_PLAYBACK *)pHandle)->pWavePlay;
	int rc;
	if (fOnOff)
		rc = waveOutRestart(pWP->hwo);
	else
		rc = waveOutPause(pWP->hwo);
	if (rc == MMSYSERR_NOERROR) return true;
	MMERRRETURNFALSE("waveOutRestart or waveOutPause")
}

int GetDevCaps(UINT_PTR id)
{
	UINT res = waveOutGetNumDevs();
	WAVEOUTCAPS woc;
	MMRESULT rc = waveOutGetDevCaps(id, &woc, sizeof(woc));
	bool a1 = woc.dwSupport & WAVECAPS_PITCH;
	bool a2 = woc.dwSupport &WAVECAPS_PLAYBACKRATE;
	bool a3 = woc.dwSupport &WAVECAPS_VOLUME;
	bool a4 = woc.dwSupport &WAVECAPS_LRVOLUME;
	bool a5 = woc.dwSupport &WAVECAPS_SYNC;
	bool a6 = woc.dwSupport &WAVECAPS_SAMPLEACCURATE;

	if (!rc)
		return 1;
	else
		return 0;
}

#endif // NO_PLAYSND

