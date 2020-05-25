// AUXLAB 
//
// Copyright (c) 2009-2020 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: wavplay
// Library to play audio signals 
// For Windows only
// 
// Version: 1.7
// Date: 5/24/2020

#include "wavplay.h"
#define MAX_24BIT		(double)0x007fffff

#include "bjcommon_win.h" // for sendtoEventLogger

WPARAM wavBuffer2snd(UINT DevID, SHORT *dataBuffer, int length, int nChan, int fs, UINT userDefinedMsgID, HWND hApplWnd, int nProgReport, int playcount, char *errstr);

double _24bit_to_double(int x)
{ // converts a short variable into double in a scale of 16388608.
	return ((double)x + .5) / MAX_24BIT;
}
void _short_to_24bit(short* pshort, int* pint, int len)
{ // Assumes that memory blocks have been prepared.
	for (int i = 0; i < len; i++) pint[i] = pshort[i] << 8;
}

void _24bit_to_short(int* pint, short* pshort, int len)
{ // Assumes that memory blocks have been prepared.
	for (int i = 0; i < len; i++) pshort[i] = (short)(pint[i] >> 8);
}
int _double_to_24bit(double x)
{
	// This maps a double variable raning -1 to 1, to a short variable ranging -16388608 to 16388607.
	return (int)(max(min(x, 1), -1)*MAX_24BIT - .5);
}
void _double_to_short(double* dint, short* pshort, int len)
{
	short ss;
	for (int i = 0; i < len; i++)
	{
		ss = (short)(_double_to_24bit(dint[i]) >> 8);
		pshort[i] = (short)(_double_to_24bit(dint[i]) >> 8);
	}
}

INT_PTR _continuePlay(INT_PTR pWP, const CSignals &sig, int DevID, UINT userDefinedMsgID, 
	int length, int nSamples4Block, double blockDurMs, char *errstr, int loop)
{
	CSignals nextblock, nextblock2;
	nextblock <= sig;
	int nChan = sig.next == NULL ? 1 : 2;
	INT_PTR res;
	do
	{
		double tp1, tp2;
		int length = nextblock.getBufferLength(tp1, tp2, blockDurMs);
		short * buffer2Play = new short[length*nChan]; // buffer2Play is cleaned inside QueuePlay
 		nextblock.makebuffer<short>(buffer2Play, length, tp1, tp2, nextblock2);
		double _nBlocks = (double)length / nSamples4Block;
		int nBlocks = (int)_nBlocks;
		if (_nBlocks - (double)nBlocks > 0.1) nBlocks++;
		res = QueuePlay(pWP, DevID, buffer2Play, length, nChan, userDefinedMsgID, nBlocks, errstr, loop);
		nextblock = nextblock2;
	} while (!nextblock2.IsEmpty());
	return res;
}

int preparebuffer(const CSignals &sig, double &block_dur_ms, short * buffer, double &nBlocks, int &nSamples4Block, int length, double tp1, double tp2, CSignals &nextblock)
{
	nSamples4Block = (int)round(block_dur_ms / 1000. * (double)sig.GetFs());
	block_dur_ms = (double)nSamples4Block *1000. / (double)sig.GetFs();
	CSignals temp;
	temp <= sig;
	if (!temp.makebuffer<short>(buffer, length, tp1, tp2, nextblock)) // sig is empty
		return 0;
	nBlocks = round((double)length / nSamples4Block);
	return length;
}

INT_PTR PlayCSignals(const CSignals &sig, int DevID, UINT userDefinedMsgID, HWND hApplWnd, double *block_dur_ms, char *errstr, int loop)
{
	errstr[0] = 0;
	int ecode = MMSYSERR_NOERROR;
	int nSamples4Block;
	double nBlocks, blockDurMs = *block_dur_ms;
	int nChan = sig.next == NULL ? 1 : 2;
	CSignals nextblock;
	double tp1, tp2;
	//if there's a null portion in the beginning greater than blockDur, treat it separately
	// in that case both tp1 and tp2 shall be zero.
	//include_null true means treat null portion as a separate, blank audio block
	int length = sig.getBufferLength(tp1, tp2, *block_dur_ms);
	bool include_null = tp2 - tp1 >= *block_dur_ms;
	short * buffer2Play = new short[length * nChan]; // buffer2Play is cleaned inside wavBuffer2snd
	length = preparebuffer(sig, blockDurMs, buffer2Play, nBlocks, nSamples4Block, length, tp1, tp2, nextblock);
	if (!buffer2Play) return 0;
	*block_dur_ms = blockDurMs;
	// Do I need to worry about userDefinedMsgID being zero?--probably not. 9/27/2019
	INT_PTR hAudPlay = (INT_PTR)wavBuffer2snd(DevID, buffer2Play, length, nChan, sig.GetFs(),
		userDefinedMsgID, hApplWnd, (int)nBlocks, loop, errstr);
	if (nextblock.IsEmpty())
		return hAudPlay;
	else
		return _continuePlay(hAudPlay, nextblock, DevID, userDefinedMsgID, length, nSamples4Block, blockDurMs, errstr, loop);
}
INT_PTR PlayCSignals(INT_PTR pWP, const CSignals &sig, int DevID, UINT userDefinedMsgID,  double *block_dur_ms, char *errstr, int loop)
{ // to be called by the application for "cue-playing;" i.e., playing sig when existing, if any, play even is done
	errstr[0] = 0;
	int ecode = MMSYSERR_NOERROR;
	int nSamples4Block;
	double nBlocks, blockDurMs = *block_dur_ms;
	int nChan = sig.next == NULL ? 1 : 2;
	CSignals nextblock;
	double tp1, tp2;
	int length = sig.getBufferLength(tp1, tp2, *block_dur_ms);
	bool include_null = tp2 - tp1 >= *block_dur_ms;
	short * buffer2Play = new short[length*nChan]; // buffer2Play is cleaned inside QueuePlay
	length = preparebuffer(sig, blockDurMs, buffer2Play, nBlocks, nSamples4Block, length, tp1, tp2, nextblock);
	if (!buffer2Play) return 0;
	*block_dur_ms = blockDurMs;
	INT_PTR hAudPlay = QueuePlay(pWP, DevID, buffer2Play, length, nChan, userDefinedMsgID,
		(int)nBlocks, errstr, loop);
	if (nextblock.IsEmpty())
		return hAudPlay;
	else
		return _continuePlay(hAudPlay, nextblock, DevID, userDefinedMsgID, length, nSamples4Block, blockDurMs, errstr, loop);
}
