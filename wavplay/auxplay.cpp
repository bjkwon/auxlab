// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: wavplay
// Library to play audio signals 
// For Windows only
// 
// Version: 1.503
// Date: 6/6/2019
// 

#include "wavplay.h"
#define MAX_24BIT		(double)0x007fffff

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
	for (int i = 0; i < len; i++)
		pshort[i] = (short)(_double_to_24bit(dint[i]) >> 8);
}
short * makebuffer(CSignals sig, int &nChan)
{	//For now this is only 16-bit playback (Sep 2008)
	short *Buffer2Play;
	sig.MakeChainless();
	if (sig.next)
	{
		double *buf2 = sig.next->buf;
		Buffer2Play = new short[sig.nSamples * 2];
		for (unsigned int i = 0; i < sig.nSamples; ++i) 
		{
			Buffer2Play[i * 2] = (short)(_double_to_24bit(sig.buf[i]) >> 8);
			Buffer2Play[i * 2 + 1] = (short)(_double_to_24bit(buf2[i]) >> 8);
		}
		nChan = 2;
	}
	else
	{
		Buffer2Play = new short[sig.nSamples];
		_double_to_short(sig.buf, Buffer2Play, sig.nSamples);
		nChan = 1;
	}
	return Buffer2Play;
}

INT_PTR PlayArray16(const CSignals &sig, int DevID, UINT userDefinedMsgID, HWND hApplWnd, double *block_dur_ms, char *errstr, int loop)
{// returns a negative number if error occurrs
 // This play the sound by specified block duration, generating event notification in every block
 // block_dur_ms is adjusted by the quantization of fs. Therefore, user should check if it has been adjusted during this call.
 // But block_dur_ms is not adjusted by the playbuffer situation that happens inside PlayBufAsynch16 (wavplay.cpp).
	int nSamples4Block = (int)(*block_dur_ms / (1000. / (double)sig.GetFs()) + .5);
	*block_dur_ms = (double)nSamples4Block *1000. / (double)sig.GetFs();
	double _nBlocks = (double)sig.nSamples / nSamples4Block;
	int nBlocks = max(2, (int)ceil(_nBlocks));
	return PlayArray16(sig, DevID, userDefinedMsgID, hApplWnd, nBlocks, errstr, loop);
}
INT_PTR PlayArray16(const CSignals &sig, int DevID, UINT userDefinedMsgID, HWND hApplWnd, int nProgReport, char *errstr, int loop)
{// Re-do error treatment 6/1/2016 bjk
	errstr[0] = 0;
	int nChan, ecode(MMSYSERR_NOERROR);
	short *Buffer2Play = makebuffer(sig, nChan);
	return (INT_PTR)PlayBufAsynch16(DevID, Buffer2Play, sig.nSamples, nChan, sig.GetFs(), userDefinedMsgID, hApplWnd, nProgReport, loop, errstr);
}

INT_PTR PlayArrayNext16(const CSignals &sig, INT_PTR pWP, int DevID, UINT userDefinedMsgID, int nProgReport, char *errstr, int loop)
{
	errstr[0] = 0;
	int nChan, ecode(MMSYSERR_NOERROR);
	short *Buffer2Play = makebuffer(sig, nChan);
	return QueuePlay(pWP, DevID, Buffer2Play, sig.nSamples, nChan, userDefinedMsgID, nProgReport, errstr, loop);
}

INT_PTR PlayArrayNext16(const CSignals &sig, INT_PTR pWP, int DevID, UINT userDefinedMsgID, double *block_dur_ms, char *errstr, int loop)
{// returns a negative number if error occurrs
 // This play the sound by specified block duration, generating event notification in every block
 // block_dur_ms is adjusted by the quantization of fs. Therefore, user should check if it has beend adjusted during this call.
	int nSamples4Block = (int)(*block_dur_ms / (1000. / (double)sig.GetFs()) + .5);
	*block_dur_ms = (double)nSamples4Block *1000. / (double)sig.GetFs();
	double _nBlocks = (double)sig.nSamples / nSamples4Block;
	int nBlocks = (int)_nBlocks;
	if (_nBlocks - (double)nBlocks > 0.1) nBlocks++;
	return PlayArrayNext16(sig, pWP, DevID, userDefinedMsgID, nBlocks, errstr, loop);
}

