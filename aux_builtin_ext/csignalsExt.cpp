// AUXLAB extention
//
// Copyright (c) 2009-2019 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: aux_builtin_ext
// opaque builtin functions
// Platform-independent (hopefully) 
// 
// Version: 0.9
// Date: 6/20/2019
// 

#include "samplerate.h"
#include "sigprocExt.h"
#include "bjcommon.h"
#include <vector>
#include <map>
#include <algorithm>
#include <deque>

#define PI 3.141592

static inline int maxcc(double *x1, int len1, double *x2, int len2, int prevmaxid)
{
	const int len = len1 + len2 - 1;
	double *buffer = new double[len];
	for (int k = 0; k < len; k++)
	{
		double tp = 0.;
		for (int q, p = 0; p <= k && p < len1; p++)
		{
			q = k - p;
			int p2 = len1 - p - 1;
			if (p2 < len1 && q < len2)
				tp += x1[p2] * x2[q];
		}
		buffer[k] = tp;
	}
	CSignal temp(buffer + len2, len - 2 * len2 + 1);
	delete[] buffer;
	CVar maxid(-1.); // need to initialize
	temp.parg = (void*)&maxid;
	temp._max();
	int newmaxid = (int)*maxid.buf;
	return (int)newmaxid;
}

CSignal& CSignalExt::pitchscale(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	CVar *pratio = (CVar *)parg;
	// If called with repeated tmarks, such as x#[0 .5 .5 1;][1 1 2 2]
	// adjust it to [0 .5 .5+margin 1;][1 1 2 2]
	// where margin is one synHop long
	//6/19/2019

	CTimeSeries *pLast = NULL;
	for (CTimeSeries *p = pratio; p; p = p->chain)
		if (!p->chain)
			pLast = p;

	for (CTimeSeries *p = pratio; p && p->chain; p = p->chain)
	{
		if (fabs(p->tmark - p->chain->tmark) < 10.)
		{
			p->chain->tmark = -1.;
			p = p->chain;
		}
	}
	CVar copy = *pratio;
	// Why timestretch-resample, when resample-timestretch would eliminate the need for the second timestretch?
	// I noticed that resample-timestretch creates more distortion (amplitude ripples across the blocks)
	// timestretch-resample reduces the ripple significantly. 
	// If the additional time taken for the second timestretch is the problem, go for resample-timestretch.
	// 5/27/2019
	timestretch(id0, len);
	vector<double> tmarks0;
	if (pratio->strut.find("tmarks") != pratio->strut.end())
		tmarks0 = pratio->strut["tmarks"].ToVector();
	else
	{
		for (CTimeSeries *p = &copy; p; p = p->chain)
			tmarks0.push_back(p->tmark);
	}
	pratio->strut.clear();
	resample(id0, len);
	//If ratio is a constant, at this point we have the correct nSamples.
	 //If not, at this point nSamples is close but different from our intended (original) nSamples.
	 //If ratio is linearly increasing from 0 to the end, we can still fix that. Just run timestretch one more time with the correct target nSamples
	if (pratio->GetType() == CSIG_TSERIES)
	{
		double num, den;
		CTimeSeries *p = pratio;
		for (auto it=tmarks0.begin(); it!=tmarks0.end()-1; it++, p = p->chain)
		{
			double newratio;
			den = p->chain->tmark - p->tmark;
			num = *(it+1) - *it;
			newratio = num / den;
			p->SetValue(newratio);
		}
		pratio->tmark = -1.;
		timestretch(id0, len);
		parg = (void*)pratio;
	}
	// If not, there's really no clear way to make the target nSamples a clean and elegant way, just take whatever you have here and move on. 5/11/2019
	return *this;
}

static inline double harmonicmean(double x1, double x2)
{
	return 2 * x1*x2 / (x1 + x2);
}

static inline double cal_ingrid(double prev, int id1, int hop, double ratio)
{
	double out = prev + hop/ratio;
	return out;
}

static inline double cal_harmonic_serise(int length, double r1, double r2)
{
	if (length == 1)
		return 1. / r1 + 1. / r2;
	double increment = (r2 - r1) / (length - 1);
	if (increment == 0.)
		return length / r1;
	double r = r1 - (r2 - r1) / (length - 1);
	double out =0;
	int k=0;
	for (; k < length; k++)
	{
		r += increment;
		out += 1. / r;
	}
	return out;
}

static inline vector<double> set_synHop_vector(int length, double r1, double r2)
{
	vector<double> out;
	if (length == 1)
	{
		out.push_back(1. / r1 + 1. / r2);
		return out;
	}
	double increment = (r2 - r1) / (length - 1);
	//if (increment == 0.)
	//{
	//	out.push_back(length / r1);
	//	return out;
	//}
	double r = r1 - (r2 - r1) / (length - 1);
	double last = 0;
	int k = 0;
	for (; k < length; k++)
	{
		r += increment;
		last += 1. / r;
		out.push_back(last);
	}
	return out;
}

static double adjust_hop(int length, double ratio, int hop, int &leftover)
{ // length: the length of the input array
  // ratio: ratio (such as 1.5 or 2.; for the case of dynamic ratios, put the harmonic mean
  // hop : [in] nominal hop (384 or 512)
// returns adjusted hop
	// new hop shall be (returned_hop)+1 for the first leftover, (returned_hop) for the rest (i.e., leftover+1 through L0
	int L0 = (int)(round((length * ratio)) / hop);
	int r0 = (int)(length * ratio) - L0 * hop;
	//now adjusting hop
	int a = r0 / L0;
	leftover = r0 - a * L0;
	hop += a;
	return (double)hop;
}

static inline int spreader(int nSamples, int nBlocks, int tol, double ratio1, double ratio2, int synHop, int *ingrid, int *outgrid)
{
	double hmean = harmonicmean(ratio1, ratio2);
	if (nBlocks <= 1)
	{
		ingrid[0] = nSamples + tol;
		outgrid[0] = (int)( (ingrid[0] - ingrid[-1]) * hmean);
		return 1;
	}
	int winLen = min((int)nSamples / 5, 512);
	int leftover=0;
	double hop, ratio, cum2 = 0., cum1 = (double)tol;
	double increment=0.;
	hop = nSamples / cal_harmonic_serise(nBlocks, ratio1, ratio2);
	if (ratio1 != ratio2)
	{
		adjust_hop(nSamples, hmean, synHop, leftover); // hop computed earlier taken; output from adjust_hop is ignored
		ratio = ratio1 - (ratio2 - ratio1) / (nBlocks - 1);
		increment = (ratio2 - ratio1) / (nBlocks - 1);
	}
	else
	{
		ratio = ratio1;
		increment = 0;
		hop = adjust_hop(nSamples, ratio, synHop, leftover);
	}
	if (nBlocks == 1)
		nBlocks++;
	for (int k = 0; k < nBlocks; k++)
	{
		cum2 += hop; // must be round-up, and keep nBlocks the same and deduct if remainder is negative
		ratio += increment;
		double in_diff = hop / ratio;
		cum1 += in_diff;
		if (leftover > 0 && k < leftover)
		{
			cum2++;
			if (ratio1 == ratio2) cum1 += 1 / hmean;
		}
		else if (leftover > 0 && k - leftover >= nBlocks)
		{
			cum2--;
			if (ratio1 == ratio2) cum1 -= 1 / hmean;
		}
		outgrid[k] = (int)cum2;
		ingrid[k] = (int)round(cum1);
	}
	return nBlocks;
}

static inline int set_time_grids(bool lengthadjust, int id1, int id2, double ratio1, double ratio2, int synHop, int *ingrid, int *outgrid, int outgridoffset)
{ // outgrid[0] is always 0
	if (lengthadjust) ratio2 = ratio1;
	double lastInGrid = (double)id1;
	int blocksizeIn = id2 - id1;
	int cumOutTP = 0;
	double harmean = harmonicmean(ratio1, ratio2);
	int nBlocks = (int)((id2-id1)*harmean/synHop); // this should not be round to make consistent with L0 in adjust_hop()
	double *_in = new double[nBlocks+50];
	double *_out = new double[nBlocks+50];
	_in[0] = (double)id1;
	_out[0] = 0.;
	nBlocks = spreader(id2-id1, nBlocks, id1, ratio1, ratio2, synHop,ingrid, outgrid);
	if (outgridoffset>0)
		for (int k = 0; k < nBlocks; k++)
			outgrid[k] += outgridoffset;
	delete[] _in;
	delete[] _out;
	return nBlocks;
}

static inline void stretch(bool nostretch, unsigned int nSamples, double *pout, double *overlapWind, const CSignal &input2,
	int winLen, int synHop, size_t blockBegin, size_t blockEnd, int *ingr, int *outgr,
	int &targetSize, int &nextOutIndex, int &del, size_t gridsize)
{
	winLen = 2 * (outgr[blockBegin+1] - outgr[blockBegin]);
	double *wind = new double[winLen];
	for (int k = 0; k < winLen; k++)
		wind[k] = .5 * (1 - cos(2.0*PI*k / (winLen - 1.0))); //hanning

// timestretch_log.py #0
	const int winLenHalf = (int)(winLen / 2. + .5);
	int tolerance = ingr[0];
	int lastInPoint = ingr[blockEnd] + synHop;
// timestretch_log.py #1
	int xid0, yid0;
// timestretch_log.py #2
	int nOverlap2 = 0;
	nextOutIndex = 0;
	int len1 = winLen + 2 * tolerance;
	int maxid = -1;
	if (nostretch)
	{
		int _synHop = outgr[1] - outgr[0];
		int winLenHalf = winLen / 2;
		for (size_t m = blockBegin; m < blockEnd-1; m++)
		{
			xid0 = ingr[m] + del;
			yid0 = outgr[m];
			int xid, yid, k = 0;
			for (; k < winLen; k++)
			{
				xid = xid0 + k;
				yid = yid0 + k;
				pout[yid] += input2.buf[xid] * wind[(k + winLenHalf) % winLen];
				overlapWind[yid] += wind[ (k + winLenHalf) % winLen];
				pout[yid] += input2.buf[xid] * wind[k % winLen];
				overlapWind[yid] += wind[k % winLen];
			}
			nextOutIndex = yid0 + k;
			if (m == blockEnd - 2)
			{
				for (; k < winLen + winLenHalf; k++)
				{
					xid = xid0 + k;
					yid = yid0 + k;
					pout[yid] += input2.buf[xid] * wind[(k + winLenHalf) % winLen];
					overlapWind[yid] += wind[(k + winLenHalf) % winLen];
				}
				int _synHop = outgr[m + 1] - outgr[m];
				// This is crosscorrelation between the next input block including tolerance regions before & after
				// and "natural progression of the last copied input segment (from Jonathan Driedger)"
				int corrIDX1 = ingr[m + 1] - tolerance;
				int corrIDX2 = ingr[m] + _synHop + del;
				maxid = maxcc(&input2.buf[corrIDX1], len1, &input2.buf[corrIDX2], winLen, maxid);
				del = tolerance - maxid + 1;
			}
		}
	}
	else
	{
		for (size_t m = blockBegin; ; m++)
		{
			xid0 = ingr[m] + del;
			yid0 = outgr[m];
			int xid, yid, k = 0;
			for (; k < winLen; k++)
			{
				xid = xid0 + k;
				yid = yid0 + k;
				pout[yid] += input2.buf[xid] * wind[k];
				overlapWind[yid] += wind[k];
				if (blockEnd == gridsize && xid0 + k == lastInPoint - 1)
				{
					nOverlap2++;
					break;
				}
			}
			nextOutIndex = yid0 + k;
			//		if (m >= blockEnd-1) break;
			if (m == blockEnd)
				break;
			if (1) //(m < gridsize - 1)
			{
				int _synHop = outgr[m + 1] - outgr[m];
				// This is crosscorrelation between the next input block including tolerance regions before & after
				// and "natural progression of the last copied input segment (from Jonathan Driedger)"
				int corrIDX1 = ingr[m + 1] - tolerance;
				int corrIDX2 = ingr[m] + _synHop + del;
// timestretch_log.py #3
				maxid = maxcc(&input2.buf[corrIDX1], len1, &input2.buf[corrIDX2], winLen, maxid);
// timestretch_log.py #4
				del = tolerance - maxid + 1;
				if (m == gridsize - 1)
				{
					if (del > 0 || outgr[m + 1] > nextOutIndex)
						break;
				}
// timestretch_log.py #5
			}
// timestretch_log.py #6
		}
	}
// timestretch_log.py #7
	targetSize = lastInPoint + outgr[blockEnd] - ingr[blockEnd] - synHop - outgr[blockBegin];
	if (blockEnd== gridsize && nextOutIndex < targetSize + outgr[blockBegin])
		nextOutIndex = targetSize + outgr[blockBegin] - 1;
	delete[] wind;
}

CSignalExt& CSignalExt::operator=(CSignal& rhs)
{
	fs = rhs.fs;
	tmark = rhs.tmark;
	pf_basic2 = rhs.pf_basic2;
	body::operator=(rhs);
	return *this;
}

CSignal& CSignalExt::timestretch(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	CVar *pratio = (CVar *)parg;
	std::map<std::string, CVar> opt;
//	int winLen = (int)(692.93 + fs / 34100.*256.); // window size. 1024 for fs=48000, 618 for fs=10000
	int winLen = min((int)len/5, 512);
	if (!pratio->strut.empty())
	{
		auto finder = pratio->strut.find("windowsize");
		if (finder != pratio->strut.end())
			winLen = (int)(*finder).second.value();
		if (winLen < 50 || winLen>4096 * 2)
		{
			std::string errout;
			sformat(errout, "windowsize must be >= 50 or <= 8192");
			pratio->SetString(errout.c_str());
			return *this;
		}
		opt = pratio->strut;
		pratio->strut.clear();
	}
	//pratio is either a constant or time sequence of scalars (not relative time)
	int synHop = winLen / 2;
//	int tolerance = 1024;
	int tolerance = synHop;
	//pratio must be either real constant or T_SEQ then value at each time point is the ratio for that segment
	map<int, double> anchor;
	vector<int> vanchor;
	bool lengthadjust = false;
	bool returnTmarks = false;
	//Prepare--if time grid in tmarks for the task are too narrow, give them a rasonable margin.
	if (pratio->GetType() == CSIG_TSERIES)
	{
		//temporary hack---used only in pitchscale()
		//treating as if CSIG_TSERIES with constant ratio in each interval
		if (pratio->tmark < 0)
			lengthadjust = true, pratio->tmark = 0.;

		for (CTimeSeries *p = pratio; p; p = p->chain)
		{
			if (p->tmark < 0) // from pitchscale
			{
				vanchor.push_back(vanchor.back() + synHop);
				returnTmarks = true;
			}
			else
				vanchor.push_back((int)ceil(p->tmark * fs / 1000) + tolerance);
			anchor[vanchor.back()] = p->value();
		}
	}
	else
	{
		vanchor.push_back(tolerance);
		anchor[vanchor.back()] = pratio->value();
		vanchor.push_back(nSamples + tolerance);
		anchor[vanchor.back()] = pratio->value();
	}
	int outputLength;
	CTimeSeries *p;
	double cumTpointsY = 0.;
	unsigned int nTSLayers = 1;
	if (pratio->GetType() != CSIG_TSERIES)
	{
		outputLength = (int)ceil(pratio->value()*nSamples);
		cumTpointsY = (double)outputLength;
	}
	else
	{
		nTSLayers = pratio->CountChains();
		p = pratio;
		for (unsigned int k = 0; k < nTSLayers - 1; k++, p = p->chain)
			cumTpointsY += harmonicmean(p->chain->value(), p->value()) * (p->chain->tmark - p->tmark) * fs / 1000;
		outputLength = (int)ceil(cumTpointsY);
	}
	int nBlocks = (int)ceil(cumTpointsY / synHop);
	int *ingrid = new int[nTSLayers*(nBlocks + 50)]; // give some margin for CSIG_TSERIES pratio
	int *outgrid = new int[nTSLayers*(nBlocks + 50)];
	ingrid[0] = tolerance;
	outgrid[0] = 0;
	vector<int> newtpoints; // new sample indices corresponding to the input indices for the ratio; begins with the second index (i.e., for a constant ratio, only the last index is shown)
	vector<size_t> chainIDX(1,0);
	if (pratio->GetType() != CSIG_TSERIES)
	{
		nBlocks = set_time_grids(lengthadjust, tolerance, nSamples + tolerance, pratio->value(), pratio->value(), synHop, ingrid+1, outgrid+1, 0);
		chainIDX.push_back(nBlocks);
	}
	else
	{
		int nBlocksCum = 0;
		int last_outgridID = 0;
		for (auto it = vanchor.begin(); it != vanchor.end()-1; it++)
		{
			nBlocks = set_time_grids(lengthadjust, *it, *(it+1), anchor[*it], anchor[*(it + 1)], synHop,
				ingrid+ last_outgridID +1, outgrid+ last_outgridID +1, outgrid[last_outgridID]);
			last_outgridID += nBlocks;
			chainIDX.push_back(last_outgridID);
		}
	}
	CSignal out;
	out.pf_basic2 = pf_basic2;
	out.tmark = tmark;
	out.UpdateBuffer(outgrid[chainIDX.back()]+ synHop + 3 * winLen);
	int filledID = 0;
	int cumProcessed = 0, del = 0;
	const int nOutReserve = outgrid[chainIDX.back() - 1] + 2*winLen;
	double *pout = new double[nOutReserve];
	double *overlapWind = new double[nOutReserve];
	memset(pout, 0, sizeof(double) * nOutReserve);
	memset(overlapWind, 0, sizeof(double) * nOutReserve);
	CSignal input(fs, synHop + tolerance); // making the input with zero padding1
	input += this; // input copied to the new buffer 
	CSignal secondzeropadds(fs, 2 * tolerance);
	input += &secondzeropadds; // zero padding2
	int targetSize=0, lastOutIndex=0;
	p = pratio;
	CTimeSeries *pchain = pratio->chain;
	for (auto it = chainIDX.begin()+1; it != chainIDX.end(); it++)
	{
		int target;
		bool nostretch = p->value() == 1. && pchain->value() == 1;
		stretch(nostretch, ingrid[*it] - ingrid[0], pout, overlapWind,
			input, winLen, synHop, *(it - 1), *it, ingrid, outgrid,
			target, lastOutIndex, del, chainIDX.back());
		targetSize += target;
		if (pchain) pchain->tmark = targetSize * 1000. / fs;
		// remove zeropading at the beginning and ending. Begin at winLenHalf and take outputLength elements
		// memcpy is done to make the target size from the end of the actual end; i.e., lastOutIndex
		cumProcessed += ingrid[*it] - ingrid[*(it - 1)];
		filledID += target;
		p = pchain;
		if (pchain) {
			pchain = pchain->chain;
		}
	}
	for (int p = 0; p < targetSize; p++)
	{
		if (overlapWind[p] > .001)
			pout[p] /= overlapWind[p];
	}
	memcpy(out.buf, pout + lastOutIndex - targetSize + 1, sizeof(double)*targetSize);

// timestretch_log.py #8
	out.SetFs(fs);
	out.nSamples = filledID;
	*this = out;
	delete[] ingrid;
	delete[] outgrid;
	delete[] overlapWind;
	delete[] pout;
	if (returnTmarks)
	{
		CVar tmarks(1);
		tmarks.UpdateBuffer((unsigned int)vanchor.size());
		int jj = 0;
		for (auto it = vanchor.begin(); it != vanchor.end(); it++)
		{
			tmarks.buf[jj++] = (*it - vanchor.front())*1000. / fs;
		}
		pratio->strut["tmarks"] = tmarks;
	}
	return *this;
}

CSignal& CSignalExt::resample(unsigned int id0, unsigned int len)
{
	//This doesn't mean real "resampling" because this does not change fs.
	//pratio < 1 means generate more samples (interpolation)-->longer duration and lower pitch
	//pratio > 1 means downsample--> shorter duration and higher pitch
	//On return, pratio is set with the actual ratio (incoming size / result size) for each chain
	if (len == 0) len = nSamples;
	CSignals *pratio = (CSignals *)parg;
	char errstr[256] = {};
	SRC_DATA conv;
	float *data_in = new float[nSamples];
	int lastSize = 1, lastPt = 0;
	float *data_out = new float[lastSize];
	int errcode, cum = 0;
	SRC_STATE* handle = src_new(SRC_SINC_MEDIUM_QUALITY, 1, &errcode);
	for (unsigned int k = 0; k < nSamples; k++) data_in[k] = (float)buf[k];
	conv.data_in = data_in;
	if (pratio->GetType() != CSIG_TSERIES)
	{
		conv.src_ratio = 1. / pratio->value();
		conv.input_frames = nSamples;
		conv.output_frames = (long)(nSamples * conv.src_ratio + .5);
		conv.data_out = data_out = new float[conv.output_frames];
		conv.end_of_input = 1;
		errcode = src_process(handle, &conv);
		if (errcode)
		{
			pratio->SetString(src_strerror(errcode));
			delete[] data_in;	delete[] data_out;
			return *this;
		}
		UpdateBuffer(conv.output_frames);
		long k;
		for (k = 0; k < conv.output_frames_gen; k++)
			buf[k] = conv.data_out[k];
		for (k = conv.output_frames_gen; k < conv.output_frames; k++)
			buf[k] = 0;
	}
	else
	{
		int blockCount = 0;
		//libsamplerate takes float buffers, CSignals takes double; so separate input, output buffers should be used.
		//data_in is just the float version of buf, the input
		//data_out is the float buffer used in each of the src_process call--smaller buffer is used than data_in
		//after src_process, the data is transferred to outbuffer (double)--just accumulated.
		//after all the blocks, outbuffer is memcpy'ed to buf for the output.
		//3/20/2019
		vector<double> outbuffer;
		//inspect pratio to estimate the output length
		int icum = 0, cumID = 0;
		for (CTimeSeries *p = pratio; p && p->chain; p = p->chain)
			icum += (int)((p->chain->tmark - p->tmark) * fs / 1000 * p->value());
		outbuffer.reserve(icum);
		int lastSize = 1, lastPt = 0;
		data_out = new float[lastSize];
		long inputSamplesLeft = (long)nSamples;
		int orgSampleCounts = 0;
		//assume that pratio time sequence is well prepared--
		deque<double> store;
		for (CTimeSeries *p = pratio; p && p->chain; p = p->chain)
		{
			conv.end_of_input = 0;
			unsigned int i1, i2;
			double ratio_mean;
			int inBuffersize, outBuffersize;
			if (p->value() == p->chain->value())
				src_set_ratio(handle, conv.src_ratio = ratio_mean = 1. / p->value());
			else
			{
				src_set_ratio(handle, 1. / p->value());
				conv.src_ratio = 1. / p->chain->value();
				ratio_mean = (2 * 1. / p->value()*1. / p->chain->value() / (1. / p->value() + 1. / p->chain->value())); // harmonic mean
			}
			//current p covers from p->tmark to p->chain->tmark
			if (!p->chain->chain)
				conv.input_frames = inputSamplesLeft;
			else
			{
				//current p covers from p->tmark to p->chain->tmark
				i1 = (int)(p->tmark * fs / 1000);
				i2 = (int)(p->chain->tmark * fs / 1000);
				conv.input_frames = i2 - i1;
			}
			conv.output_frames = (long)(conv.input_frames * ratio_mean + .5); // when the begining and ending ratio is different, use the harmonic mean for the estimate.
			if (conv.output_frames > lastSize)
			{
				delete[] data_out;
				data_out = new float[lastSize = conv.output_frames + 20000];//reserve the buffer size big enough to avoid memory crash, but find out a better than this.... 3/20/2019
			}
			conv.data_out = data_out;
			errcode = src_process(handle, &conv);
			inBuffersize = conv.input_frames_used;
			if (errcode)
			{
				std::string errout;
				sformat(errout, "Error in block %d--%s", blockCount++, src_strerror(errcode));
				pratio->SetString(errout.c_str());
				delete[] data_in;	delete[] data_out;
				return *this;
			}
			outBuffersize = conv.output_frames_gen;
			for (int k = 0; k < conv.output_frames_gen; k++)
				outbuffer.push_back(data_out[k]);
			lastPt += conv.input_frames_used;
			if (p->chain->chain)
			{
				conv.data_in = &data_in[lastPt];
				inputSamplesLeft -= conv.input_frames_used;
			}
			while (conv.input_frames)
			{
				conv.src_ratio = 1. / p->chain->value();
				conv.data_in = &data_in[lastPt];
				conv.input_frames -= conv.input_frames_used;
				conv.end_of_input = conv.input_frames == 0 ? 1 : 0;
				errcode = src_process(handle, &conv);
				inBuffersize += conv.input_frames_used;
				outBuffersize += conv.output_frames_gen;
				for (int k = 0; k < conv.output_frames_gen; k++)
					outbuffer.push_back(data_out[k]);
				lastPt += conv.input_frames_used;
			}
			src_reset(handle);
			store.push_back(1000. / fs * outBuffersize);
		}
		double cum = 0;
		for (CTimeSeries *p = pratio; p && p->chain; p = p->chain)
		{
			cum += store.front();
			p->chain->tmark = cum;
			store.pop_front();
		}
		UpdateBuffer((unsigned int)outbuffer.size());
		memcpy(buf, &outbuffer[0], sizeof(double)*outbuffer.size());
	}
	src_delete(handle);
	delete[] data_in;
	delete[] data_out;
	return *this;
}

CSignalExt::~CSignalExt()
{
	if (buf) {
		delete[] buf;
		buf = NULL;
	}
}

CTimeSeriesExt& CTimeSeriesExt::basic(CSignal& (CSignalExt::*pf_exe2)(unsigned int, unsigned int), void *popt)
{
	parg = popt;
	if (GetType() == CSIG_TSERIES)
	{
		//out.Reset(1); // 1 means new fs
		//CSignal tp = TSeries2CSignal();
		//out.SetValue((tp.*(this->pf_basic2))(0, 0));
	}
	else
	{
		unsigned int len = Len();
		for (unsigned int k = 0; k < nGroups; k++)
			(this->*(this->pf_exe2))(k*len, len);
		for (CTimeSeriesExt *p = chain; p; p = p->chain)
		{
			p->parg = popt;
			(p->*(this->pf_exe2))(0, p->nSamples);
		}
	}
	return *this;
}

CSignalsExt& CSignalsExt::basic(CSignal& (CSignalExt::*pf_exe2)(unsigned int, unsigned int), void *popt)
{
	CTimeSeriesExt::basic(pf_exe2, popt);
	if (next != NULL)
	{
		next->pf_exe2 = pf_exe2;
		next->basic(pf_exe2, popt);
	}
	parg = nullptr;
	return *this;
}

CVar &CSignalsExt::make_CVar()
{
	ref.body::operator=(*this);
	ref.CSignal::operator=(*this);
	CTimeSeries *pchain = (CTimeSeries*)chain;
	while (pchain)
	{
		ref.AddChain(*pchain);
		pchain = pchain->chain;
	}
	CTimeSeries *pnext = (CTimeSeries*)next;
	if (next)
	{
		pchain = (CTimeSeries*)next->chain;
		while (pchain)
		{
			pnext->AddChain(*pchain);
			pchain = pchain->chain;
		}
		pnext->chain = NULL;
		ref.SetNextChan(pnext);
	}
	return ref;
}
//
//CTimeSeries& CTimeSeriesExt::make_CTimeSeries(const CTimeSeriesExt& rhs)
//{
//	CSignalExt::operator=(rhs);
//	CTimeSeries *p = (CTimeSeries*)rhs.chain;
//	while (p)
//	{
//		ref.AddChain(*p);
//		p = p->chain;
//	}
//	return *output;
//}

CSignal& CSignalExt::operator=(const CSignalExt& rhs)
{
	if (this != &rhs)
	{
		fs = rhs.fs;
		tmark = rhs.tmark;
		UpdateBuffer(rhs.nSamples);
		nGroups = rhs.nGroups;
		bufBlockSize = rhs.bufBlockSize;
		memcpy(logbuf, rhs.logbuf, rhs.nSamples *bufBlockSize);
	}
	return *this;
}
