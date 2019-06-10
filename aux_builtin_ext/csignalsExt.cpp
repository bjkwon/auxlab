#include "samplerate.h"
#include "sigprocExt.h"
#include "bjcommon.h"
#include <vector>
#include <map>
#include <algorithm>

//Moved from sigproc::csignals.cpp
//The uggliest code (because of timestretch and pitchscale functions)!!
//This code should avoid having complex signal processing routines.
//timestretch and pitchscale functions turned out to be way more complidated than expected.
//In the future revision, these complex signal processing part should be separated.
//Also, the quality of this version is not as good as earlier one (not git'ed)--producing somewhat "warbling" sound
//This is due to the synHop not being constant and adjusted as it goes to precisely honor the input time grid.
//In the future revision, I will try to make hop constant as much as possible between grids and adjusted toward the grids.
// 5/28/2019 BJ Kwon

#define PI 3.141592

static inline int maxcc(double *x1, int len1, double *x2, int len2)
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
	CVar maxid(-1.); // need to initialize
	temp.parg = (void*)&maxid;
	temp._max();
	delete[] buffer;
	return (int)((CVar*)temp.parg)->value();
}

CSignal& CSignalExt::pitchscale(unsigned int id0, unsigned int len)
{
	if (len == 0) len = nSamples;
	CVar *pratio = (CVar *)parg;
	vector<unsigned int> sectionlength;
	unsigned int lastLength = 0;
	for (CTimeSeries *p = pratio; p && p->chain; p = p->chain)
	{
		sectionlength.push_back((unsigned int)(p->chain->tmark / 1000.*p->fs)- lastLength);
		lastLength = sectionlength.back();
	}
	CVar copy = *pratio;
	// Why timestretch-resample, when resample-timestretch would eliminate the need for the second timestretch?
	// I noticed that resample-timestretch creates more distortion (amplitude ripples across the blocks)
	// timestretch-resample reduces the ripple significantly. 
	// If the additional time taken for the second timestretch is the problem, go for resample-timestretch.
	// 5/27/2019
	timestretch(id0, len);
	resample(id0, len);
	 //If ratio is a constant, at this point we have the correct nSamples.
	 //If not, at this point nSamples is close but different from our intended (original) nSamples.
	 //If ratio is linearly increasing from 0 to the end, we can still fix that. Just run timestretch one more time with the correct target nSamples
	CTimeSeries *p = pratio;
	if (sectionlength.size()==2)
		for (auto it = sectionlength.begin(); it!= sectionlength.end(); it++)
		{
			if (p->value() != p->chain->value()) // sliding ratio
			{
				double newratio = (double)(*it) / ((unsigned int)(p->chain->tmark / 1000.*p->fs) - (unsigned int)(p->tmark / 1000.*p->fs));
				p->SetValue(newratio);
				p->chain->SetValue(newratio);
				((CVar*)parg)->strut = copy.strut;
				timestretch(id0, len);
			}
			p = p->chain;
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

static inline int spreader(int nSamples, int nBlocks, int tol, double ratio1, double ratio2, int synHop, int *ingrid, int *outgrid, int remainder)
{
	double lastOutPoint = outgrid[nBlocks-1]+remainder;
	double hop = harmonicmean(ratio1, ratio2);
	if (nBlocks > 2)
	{
//		double remainder = outgrid[nBlocks] - outgrid[nBlocks - 1];
		if (synHop == (int)remainder)
		{
			for (int k = 0; k < nBlocks; k++)
				outgrid[k] = (int)remainder * (k + 1);
			double ratio = ratio1 - (ratio2 - ratio1) / (nBlocks - 2);
			double increment = (ratio2 - ratio1) / (nBlocks - 2);
			int cum = 19;
			for (int k = 0; k < nBlocks; k++)
			{
				ratio += increment;
				ingrid[k] = (int)(cum + synHop / ratio);
				cum = ingrid[k];
			}
		}
		else
		{
			int leftover = remainder - remainder / nBlocks * nBlocks;
			hop = nSamples / cal_harmonic_serise(nBlocks, ratio1, ratio2);
			int cum1 = 0;
			double cum2 = (double)tol;
			double ratio = ratio1 - (ratio2 - ratio1) / (nBlocks - 2);
			double increment = (ratio2 - ratio1) / (nBlocks - 2);
			for (int k = 0; k < nBlocks; k++)
			{
				cum1 += (int)round(hop); // must be round-up, and keep nBlocks the same and deduct if remainder is negative
				ratio += increment;
				double add2cum2 = hop / ratio;
				cum2 += add2cum2;
				if (leftover > 0)
				{
					if (k < leftover)
						cum1++; 
				}
				else if (leftover < 0)
				{
					if (k - leftover >= nBlocks) 
						cum1--; 
				}
				outgrid[k] = cum1;
				ingrid[k] = (int)round(cum2);
			}
		}
	}
	else 
	{ // check this part.... 5/28/2019
		double delta_ingrid = ingrid[nBlocks] - ingrid[0];
		nBlocks++;
		hop = delta_ingrid / cal_harmonic_serise(nBlocks - 1, ratio1, ratio2);

		for (int k = 1; k < nBlocks; k++)
			outgrid[k] = (int)(hop * k);
		double ratio = ratio1 - (ratio2 - ratio1) / (nBlocks - 2);
		double increment = (ratio2 - ratio1) / (nBlocks - 2);
		for (int k = 1; k < nBlocks; k++)
		{
			ratio += increment;
			ingrid[k] = ingrid[k - 1] + (int)(hop / ratio);
		}
	}
	return nBlocks;
}

static inline int get_nLength(int id1, int id2, double ratio1, double ratio2, int synHop, int *ingrid, int *outgrid, int outgridoffset)
{ // outgrid[0] is always 0
	int k = 1;
	double lastInGrid = (double)id1;
	int blocksizeIn = id2 - id1;
	int cumOutTP = 0;
	double harmean = harmonicmean(ratio1, ratio2);
	int nBlocks = (int)round((id2-id1)*harmean/synHop);
	double remainder;
	remainder = (id2 - id1) - (int)(synHop * nBlocks / harmean);
	double *_in = new double[nBlocks+50];
	double *_out = new double[nBlocks+50];
	_in[0] = (double)id1;
	_out[0] = 0.;
	bool loop = true;
//	double ratio;
	if (remainder >= synHop / 2.)
		remainder = synHop - remainder;
	// End of Think about this
	if (nBlocks > 1)
	{
		nBlocks = spreader(id2-id1, nBlocks, id1, ratio1, ratio2, synHop,ingrid, outgrid, (int)remainder);
//		for (int k = 0; k < nBlocks - 1; k++)
//		{
//			ingrid[k] = (int)(_in[k + 1] + .5);
//			outgrid[k] = outgridoffset + (int)(_out[k + 1] + .5);
//		}
	}
	else // nBlocks==1
	{
		ingrid[0] = (int)(_in[1] + .5);
		outgrid[0] = outgridoffset + (int)(_out[1] + .5);
	}
	delete[] _in;
	delete[] _out;
	return nBlocks;
}

static inline void stretch(double *pout, double *overlapWind, unsigned int nSamples, double *buf, const CSignal &input2, 
	int synHop, size_t blockBegin, size_t blockEnd, int *ingr, int *outgr, double *wind,
	int winLen, int &targetSize, int &lastOutIndex, int &del, size_t gridsize)
{
// timestretch_log.py #0
	const int winLenHalf = (int)(winLen / 2. + .5);
	int tolerance = ingr[0];
	int lastInPoint = nSamples + winLenHalf + tolerance;
// timestretch_log.py #1
	int xid0;
// timestretch_log.py #2
	int nOverlap2 = 0;
	lastOutIndex = 0;
	for (size_t m = blockBegin; m < blockEnd; m++)
	{
		xid0 = ingr[m] + del;
		int yid0 = outgr[m];
		int k = 0;
		for (; k < winLen; k++)
		{
			int xid = xid0 + k;
			int yid = (int)yid0 + k;
			pout[yid] += input2.buf[xid] * wind[k];
			overlapWind[yid] += wind[k];
			if (xid0 + k == lastInPoint - 1)
			{
				nOverlap2++;
				break;
			}
		}
		lastOutIndex = max(lastOutIndex, (int)yid0 + k);
		if (m < gridsize - 1)
		{
			double ratio0 = (double)(outgr[m + 1] - outgr[m]) / (ingr[m + 1] - ingr[m]);
			double div = 10 + (ratio0 - 1) * 10;
			int _synHop = synHop;
			double tol = (double)_synHop / div;
			tolerance = (int)(tol + .5);
			// This is crosscorrelation between the next input block including tolerance regions before & after
			// and "natural progression of the last copied input segment (from Jonathan Driedger)"
			int corrIDX1 = ingr[m + 1] - tolerance;
			int corrIDX2 = ingr[m] + _synHop + del;
			int len1 = winLen + 2 * tolerance;
// timestretch_log.py #3
			int maxid = maxcc(&input2.buf[corrIDX1], len1, &input2.buf[corrIDX2], winLen);
// timestretch_log.py #4
			del = tolerance - maxid + 1;
// timestretch_log.py #5
		}
// timestretch_log.py #6
	}
// timestretch_log.py #7
	int lastOutPoint = lastInPoint + outgr[blockEnd] - ingr[blockEnd]  - del; // This is the target
	targetSize = lastOutPoint - winLenHalf + del - outgr[blockBegin];
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
	int winLen = (int)(692.93 + fs / 34100.*256.); // window size. 1024 for fs=48000, 618 for fs=10000
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
	int tolerance = synHop / 20;
	//pratio must be either real constant or T_SEQ then value at each time point is the ratio for that segment
	map<int, double> anchor;
	vector<int> vanchor;
	for (CTimeSeries *p = pratio; p; p = p->chain)
	{
		vanchor.push_back((int)ceil(p->tmark * fs / 1000) + tolerance);
		anchor[vanchor.back()] = p->value();
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
		nBlocks = get_nLength(tolerance, nSamples + tolerance, pratio->value(), pratio->value(), synHop, ingrid+1, outgrid+1, 0);
		chainIDX.push_back(nBlocks);
	}
	else
	{
		int nBlocksCum = 0;
		int last_outgridID = 0;
		for (auto it = vanchor.begin(); it != vanchor.end()-1; it++)
		{
			nBlocks = get_nLength(*it, *(it+1), anchor[*it], anchor[*(it + 1)], synHop, 
				ingrid+ last_outgridID +1, outgrid+ last_outgridID +1, outgrid[last_outgridID]);
			last_outgridID += (nBlocks-1);
			chainIDX.push_back(last_outgridID);
		}
	}
	double *wind = new double[winLen];
	for (int k = 0; k < winLen; k++)
		wind[id0 + k] = .5 * (1 - cos(2.0*PI*k / (winLen - 1.0))); //hanning
	CSignal out;
	out.pf_basic2 = pf_basic2;
	out.tmark = tmark;
	int ddd = outgrid[chainIDX.back()];
	ddd += synHop + 3 * tolerance;
	out.UpdateBuffer(outgrid[chainIDX.back()]+ synHop + 3 * winLen);
	int filledID = 0;
	int cumProcessed = 0, del = 0;
	const int nOutReserve = outgrid[chainIDX.back() - 1] + winLen;
	double *pout = new double[nOutReserve];
	double *overlapWind = new double[nOutReserve];
	memset(pout, 0, sizeof(double) * nOutReserve);
	memset(overlapWind, 0, sizeof(double) * nOutReserve);
	int additionals = synHop + 2 * tolerance + (int)ceil((double)(ingrid[1] - tolerance) / synHop)*winLen;
	CSignal temp(fs, nSamples + additionals);
	memcpy(temp.buf + synHop + tolerance, buf, sizeof(double)*nSamples);
	int targetSize=0, lastOutIndex=0;
	CTimeSeries *pchain = pratio->chain;
	for (auto it = chainIDX.begin()+1; it != chainIDX.end(); it++)
	{
		int target;
		stretch(pout, overlapWind, ingrid[*it]-ingrid[0], 
			buf, temp, synHop, *(it - 1), *it, ingrid, outgrid,
			wind, winLen, target, lastOutIndex, del, chainIDX.back());
		targetSize += target;
		if (pchain) pchain->tmark = targetSize * 1000. / fs;
		// remove zeropading at the beginning and ending. Begin at winLenHalf and take outputLength elements
		// memcpy is done to make the target size from the end of the actual end; i.e., lastOutIndex
		cumProcessed += ingrid[*it] - ingrid[*(it - 1)];
		filledID += target;
		if (pchain) pchain = pchain->chain;
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
	delete[] wind;
	delete[] ingrid;
	delete[] outgrid;
	delete[] overlapWind;
	delete[] pout;
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
	float *data_out, *data_in = new float[nSamples];
	int errcode;
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
		int cum = 0, cumID = 0;
		for (CTimeSeries *p = pratio; p && p->chain; p = p->chain)
			cum += (int)((p->chain->tmark - p->tmark) * fs / 1000 * p->value());
		outbuffer.reserve(cum);
		int lastSize = 1, lastPt = 0;
		data_out = new float[lastSize];
		long inputSamplesLeft = (long)nSamples;
		int orgSampleCounts = 0;
		//assume that pratio time sequence is well prepared--
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
			p->chain->tmark = p->tmark + 1000. / fs * outBuffersize;
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
			(p->*(this->pf_basic2))(0, p->nSamples);
		}
	}
	return *this;
}

CSignalsExt& CSignalsExt::basic(CSignal& (CSignalExt::*pf_exe2)(unsigned int, unsigned int), void *popt)
{
	CTimeSeriesExt::basic(pf_basic2, popt);
	if (next != NULL)
	{
		next->pf_basic2 = pf_basic2;
		next->basic(pf_basic2, popt);
	}
	parg = nullptr;
	return *this;
}

CVar &CSignalsExt::make_CVar()
{
	if (nSamples>0)
		ref.body::operator=(*this);
	ref.body::operator=(*this);
	ref.CSignal::operator=(*this);
	ref.chain = (CTimeSeries*)chain;
	ref.next = (CTimeSeries*)next;
	return ref;
}