// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: wavplay
// Library to play audio signals 
// For Windows only
// 
// Version: 1.6
// Date: 7/6/2019
// 

#include "wavplay.h"
#define MAX_24BIT		(double)0x007fffff

#include "bjcommon_win.h" // for sendtoEventLogger

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
short * makemonobuffer(const CSignals &sig, int &length, int ch)
{	//For now this is only 16-bit playback (Sep 2008)
	//if there's a gap between current and next chain
	int gap = 0;
	int fs = sig.GetFs();
	short *buffer2Play;
	if (ch >0 && sig.tmark > 0)
	{ // this is done only for the first time makemonobuffer is run for a playback
		length = (int)(sig.tmark / 1000. * fs + .5);
		buffer2Play = new short[length];
		memset(buffer2Play, 0, length * sizeof(short));
	}
	else
	{
		if (sig.chain)
			gap = (int)(sig.chain->tmark / 1000. * fs + .5) - sig.nSamples;
		buffer2Play = new short[sig.nSamples + gap];
		_double_to_short(sig.buf, buffer2Play, sig.nSamples);
		memset(buffer2Play + sig.nSamples, 0, gap * sizeof(short));
		length = sig.nSamples + gap;
	}
	return buffer2Play;
}
short * makestereobuffer(CSignals &sig, int &length, CSignals &ghcopy)
{ // for 16-bit playback
	// sterero buffer is made for the duration where at least one channel is present, plus a zero buffer till the point where the signal is present again in either channel
	CSignals *p = (CSignals *)&sig;
	multimap<double, int> timepoints;
	const int fs = sig.GetFs();
	for (int k = 0; p && k < 2; k++, p=(CSignals*)p->next)
	{
		for (CTimeSeries *q = p; q; q = q->chain)
		{
			if (q->IsEmpty()) continue;
			timepoints.insert(pair<double, int>(q->tmark, 1));
			timepoints.insert(pair<double, int>(q->CSignal::endt(), -1));
			q->ghost = true;
		}
	}
	auto it = timepoints.begin();
	for (int sum = 0; it != timepoints.end(); it++)
	{
		sum += it->second;
		if (sum == 0)
		{
			auto jt = it;
			if (++jt == timepoints.end())
				break;
		}
	}
	const double lasttp = it->first;
	double lasttp_with_silence = lasttp;
	if (++it != timepoints.end())
		lasttp_with_silence = it->first;
	length = (int)(lasttp_with_silence / 1000. * fs + .5);
	int nChan = sig.next ? 2 : 1;
	short *buffer2Play;
	buffer2Play = new short[length*nChan];
	memset(buffer2Play, 0, sizeof(short)*length * nChan);
	p = (CSignals *)&sig;
	for (int ch = 0; p && ch < 2; ch++, p = (CSignals*)p->next)
	{
		for (CTimeSeries *q = p; q; q = q->chain)
		{
			if (q->tmark > lasttp) break;
			int offset = (int)(q->tmark / 1000. * fs + .5) * 2;
			for (unsigned int m = 0; m < q->nSamples; m++)
				buffer2Play[offset + m * nChan + ch] = (short)(_double_to_24bit(q->buf[m]) >> 8);
		}
	}
	ghcopy = sig;
	p = &sig;
	CTimeSeries *q, *q1=NULL, *q2=NULL;
	for (int k = 0; p && k < 2; k++, p = (CSignals*)p->next)
	{
		for (q=p; q; q = q->chain)
		{
			if (q->tmark > lasttp)
			{
				q->tmark -= lasttp_with_silence;
				CTimeSeries *tempNext = p->next;
				if (k == 0)
				{
					p->next = tempNext;
					q1 = q;
				}
				else
				{
					p->next = nullptr;
					q2 = q;
				}

				break;
			}
		}
	}
	if (q1) 
		ghcopy = *q1;
	if (q2)
		ghcopy.SetNextChan(q2);
	if (!q1 && !q2)
		ghcopy.Reset();
	return buffer2Play;
}
short * makebuffer(const CSignals &sig, int &nChan, int &length)
{	//For now this is only 16-bit playback (Sep 2008)
	short *Buffer2Play;
	CSignals *psig;
	if (sig.chain)
	{
		psig = new CSignals(sig);
		psig->MakeChainless();
	}
	else
		psig = (CSignals*)&sig;
	length = psig->nSamples;
	if (psig->next)
	{
		if (psig->next->chain)
			psig->next->MakeChainless();
		//if psig and psig->next have different length, match them
		if (psig->nSamples > psig->next->nSamples)
			psig->next->UpdateBuffer(psig->nSamples);
		else if (psig->nSamples < psig->next->nSamples)
			psig->UpdateBuffer(psig->next->nSamples);
		double *buf2 = psig->next->buf;
		Buffer2Play = new short[psig->nSamples * 2];
		for (unsigned int i = 0; i < psig->nSamples; ++i) 
		{
			Buffer2Play[i * 2] = (short)(_double_to_24bit(psig->buf[i]) >> 8);
			Buffer2Play[i * 2 + 1] = (short)(_double_to_24bit(buf2[i]) >> 8);
		}
		nChan = 2;
	}
	else
	{
		Buffer2Play = new short[psig->nSamples];
		_double_to_short(psig->buf, Buffer2Play, psig->nSamples);
		nChan = 1;
	}
	return Buffer2Play;
}
short * makebuffer(const ctimesig &sig, int &nChan, size_t &length)
{
	short *Buffer2Play;
	ctimesig *psig = (ctimesig *)&sig;
	length = 0;
	// assume that there's no null portion between blocks, so ignore tmarks for now.
	//for (ctimesig *p = psig;p;p=p->chain)
	//{
	//	length += p->pdata->size();
	//}
	length += sig.pdata->size();
	//if (sig.chain)
	//{
	//	psig = new CSignals(sig);
	//	psig->MakeChainless();
	//}
	//else
		psig = (ctimesig*)&sig;
//	if (sig.block.front().right && !sig.block.front().right->buf.empty())
	{
		//if (psig->next->chain)
		//	psig->next->MakeChainless();
		////if psig and psig->next have different length, match them
		//if (psig->nSamples > psig->next->nSamples)
		//	psig->next->UpdateBuffer(psig->nSamples);
		//else if (psig->nSamples < psig->next->nSamples)
		//	psig->UpdateBuffer(psig->next->nSamples);
		//double *buf2 = psig->next->buf;
		//Buffer2Play = new short[psig->nSamples * 2];
		//for (unsigned int i = 0; i < psig->nSamples; ++i)
		//{
		//	Buffer2Play[i * 2] = (short)(_double_to_24bit(psig->buf[i]) >> 8);
		//	Buffer2Play[i * 2 + 1] = (short)(_double_to_24bit(buf2[i]) >> 8);
		//}
		nChan = 2;
	}
//	else
	{
		Buffer2Play = new short[length];
		size_t cum = 0;
		ctimesig *p = psig;
//		for (ctimesig *p = psig; p; p = p->chain)
		{
			_double_to_short(&p->pdata->front(), Buffer2Play+cum, (int)p->pdata->size());
			cum += p->pdata->size();
		}
		nChan = 1;
	}
	return Buffer2Play;
}

INT_PTR PlayArray16(const ctimesig &sig, int DevID, UINT userDefinedMsgID, HWND hApplWnd, double *block_dur_ms, char *errstr, int loop)
{
	int fs = sig.fs;
	
	int nSamples4Block = (int)(*block_dur_ms / (1000. / fs) + .5);
	*block_dur_ms = nSamples4Block * 1000. / fs;
	double _nBlocks = (double)sig.pdata->size()/ nSamples4Block;
	int nBlocks = max(2, (int)ceil(_nBlocks));
//	return PlayArray16(sig, DevID, userDefinedMsgID, hApplWnd, nBlocks, errstr, loop);
	errstr[0] = 0;
	int nChan, ecode(MMSYSERR_NOERROR);
	size_t length;
	short *Buffer2Play = makebuffer(sig, nChan, length);
	return (INT_PTR)PlayBufAsynch16(DevID, Buffer2Play, (int)length, nChan, sig.fs, 
		userDefinedMsgID, hApplWnd, nBlocks, loop, errstr);
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
	int length;
	short *Buffer2Play = makebuffer(sig, nChan, length);
	return (INT_PTR)PlayBufAsynch16(DevID, Buffer2Play, length, nChan, sig.GetFs(), 
		userDefinedMsgID, hApplWnd, nProgReport, loop, errstr);
}
INT_PTR PlayCSignals(const CSignals &sig, int DevID, UINT userDefinedMsgID, HWND hApplWnd, double *block_dur_ms, char *errstr, int loop)
{
	int nSamples4Block = (int)(*block_dur_ms / (1000. / (double)sig.GetFs()) + .5);
	*block_dur_ms = (double)nSamples4Block *1000. / (double)sig.GetFs();
	int nChan = 1, ecode(MMSYSERR_NOERROR);

	INT_PTR res;
	if (0) //(!sig.next)
	{ // mono
		if (sig.IsEmpty()) 
			return 0;
		int length;
		CTimeSeries *p = (CTimeSeries *)&sig;
		short *Buffer2Play = makemonobuffer(sig, length, 1);
		double _nBlocks = (double)length / nSamples4Block;
		int nBlocks = max(2, (int)ceil(_nBlocks));
		res = (INT_PTR)PlayBufAsynch16(DevID, Buffer2Play, length, nChan, sig.GetFs(),
			userDefinedMsgID, hApplWnd, nBlocks, loop, errstr);
		if (sig.tmark > 0)
		{ // if there was a null portion prior to actual signal, Buffer2Play made above is a zero memory block
		  // here's makemonobuffer is called again with ghost copy of sig with tmark of zero.
			CSignals ghcopy(sig.GetFs());
			*(bool*)&sig.ghost = true;
			ghcopy = sig;
			//time-shift ghcopy by sig.tmark (so that it starts at 0)
			double shift = ghcopy.tmark;
			for (CTimeSeries *tp=&ghcopy; tp; tp=tp->chain)
				tp->tmark -= shift; 
			short *Buffer2Play2 = makemonobuffer(ghcopy, length, 0);
			_nBlocks = (double)length / nSamples4Block;
			nBlocks = max(2, (int)ceil(_nBlocks));
			res = (INT_PTR)QueuePlay(res, DevID, Buffer2Play2, length, nChan, userDefinedMsgID, nBlocks, errstr, loop);
		}
		p = p->chain;
		for (; p; p = p->chain)
		{
			short *Buffer2PlayMore = makemonobuffer(*p, length, 0);
			_nBlocks = (double)length / nSamples4Block;
			nBlocks = (int)_nBlocks;
			if (_nBlocks - (double)nBlocks > 0.1) nBlocks++;
			res = QueuePlay(res, DevID, Buffer2PlayMore, length, nChan, userDefinedMsgID, nBlocks, errstr, loop);
		}
	}
	else // stereo
	{
		nChan = sig.next ? 2 : 1;
		int length;
		CSignals nextblock;
		short *Buffer2Play = makestereobuffer((CSignals&)sig, length, nextblock);
		if (!Buffer2Play) // sig is empty
			return 0;
		double _nBlocks = (double)length / nSamples4Block;
		int nBlocks = (int)_nBlocks;
		if (_nBlocks - (double)nBlocks > 0.1) nBlocks++;
		res = (INT_PTR)PlayBufAsynch16(DevID, Buffer2Play, length , nChan, sig.GetFs(),
			userDefinedMsgID, hApplWnd, nBlocks, loop, errstr);
		while (!nextblock.IsEmpty())
		{
			CSignals nextblock2(nextblock);
			short *Buffer2PlayMore = makestereobuffer(nextblock2, length, nextblock);
			_nBlocks = (double)length / nSamples4Block;
			nBlocks = (int)_nBlocks;
			if (_nBlocks - (double)nBlocks > 0.1) nBlocks++;
			res = QueuePlay(res, DevID, Buffer2PlayMore, length, nChan, userDefinedMsgID, nBlocks, errstr, loop);
		}
	}
	return res;
}

INT_PTR PlayArrayNext16(const CSignals &sig, INT_PTR pWP, int DevID, UINT userDefinedMsgID, int nProgReport, char *errstr, int loop)
{
	errstr[0] = 0;
	int length;
	int nChan, ecode(MMSYSERR_NOERROR);
	short *Buffer2Play = makebuffer(sig, nChan, length);
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

