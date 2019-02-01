// AUXLAB 
//
// Copyright (c) 2009-2018 Bomjun Kwon (bjkwon at gmail)
// Licensed under the Academic Free License version 3.0
//
// Project: auxlab
// Main Application. Based on Windows API  
// 
// 
// Version: 1.495
// Date: 12/13/2018
// 
#include "bjcommon.h"
#include "bjcommon_win.h"
#ifndef SIGPROC
#include "sigproc.h"
#endif

#include "xcom.h"

#define RETURN0IFINVALID(X) if ((X)<=0) {sprintf(errstr, "Error in ( "#X " )"); return 0;}
#define RETURN0IFINVALID0(X) if ((X)!=0) {sprintf(errstr, "Error in ( "#X " )"); return 0;}
#define RETURNWITHERRMSG(X) {sprintf(errstr, "Error in ( "#X " )"); return 0;}
#define RETURNWITHERRMSG2(X,Y) {sprintf(errstr, "Error in ( "#X #Y" )"); return 0;}
#define CATCHER(X) if ((X)<=0) {sprintf(errstr, "Error in ( "#X " )"); throw errstr;}

static char formattype[] = "A";

void nonnulintervals(CSignals *psig, string &out);

int xcom::write_axl_block(FILE *fp, string varname, CVar *pout, char *errstr, bool independent)
{try {
	char header[16], verstr[16] = {};
	int audiotype;
	if (independent)
	{
		sprintf(header, "AXL");
		CATCHER(fwrite(header, strlen(header), 1, fp))
		sprintf(verstr, "%s", AppVersion);
		verstr[7] = 0; //limit header to 7 chars
		CATCHER(fwrite(verstr, 7, 1, fp))
		CATCHER(fwrite((void*)&formattype, 1, 1, fp))
	}
	CATCHER(fwrite((void*)varname.c_str(), varname.size() + 1, 1, fp))
	char typePlus = (char)pout->GetTypePlus();
	CATCHER(fwrite((void*)&typePlus, 1, 1, fp))
	_int32 bufsize = (_int32)pout->bufBlockSize;
	CATCHER(fwrite((void*)&bufsize, sizeof(bufsize), 1, fp))
	switch (pout->GetType())
	{
	case CSIG_EMPTY:
		CATCHER(fwrite((void*)&pout->nSamples, sizeof(pout->nSamples), 1, fp))
		break;
	case CSIG_STRING:
		CATCHER(fwrite((void*)&pout->nSamples, sizeof(pout->nSamples), 1, fp))
		if (pout->nSamples > 0)	CATCHER(fwrite((void*)pout->strbuf, pout->nSamples, 1, fp))
			break;
	case CSIG_SCALAR:
//	case CSIG_COMPLEX:
	case CSIG_VECTOR:
	case CSIG_VECTOR - 1:
		CATCHER(fwrite((void*)&pout->nSamples, sizeof(pout->nSamples), 1, fp));
		CATCHER(fwrite((void*)pout->buf, pout->bufBlockSize, pout->nSamples, fp))
		break;
	case CSIG_AUDIO:
		audiotype = pout->IsStereo();//0 for mono, 1 for stereo...
		if (pout->IsLogical()) audiotype += 2; //2 for mono logical, 3 for stereo logical...
		CATCHER(fwrite((void*)&audiotype, sizeof(audiotype), 1, fp))
		pout->WriteAXL(fp);
		break;
/*	case CSIG_CELL:
		sz = (unsigned int)pout->cell.size();
		CATCHER(fwrite((void*)&sz, sizeof(sz), 1, fp));
		for (vector<CSignals>::iterator it=pout->cell.begin(); it!= pout->cell.end(); it++)
			write_axl_block(fp, ".", &*it, errstr, false);
		break;
	case CSIG_STRUCT:
		sz = (unsigned int)pout->strut.size();
		CATCHER(fwrite((void*)&sz, sizeof(sz), 1, fp));
		for (map<string, CSignals>::iterator it = pout->strut.begin(); it != pout->strut.end(); it++)
			write_axl_block(fp, it->first, &it->second, errstr, false);
		break;
*/	default:
		sprintf(errstr, "Invalid CSignals type.");
		throw errstr;
	}
	return 1;
	}
	catch (char *_errstr)
	{
		errstr = _errstr;
		return 0;
	}
}

int xcom::read_axl_block(FILE *fp, string &varname, CVar *pout, char *errstr, bool independent)
{
	unsigned char type;
	char formattype[8], verstr[16], header[8], readbuf[256], _varname[256]; // let's limit variable name to 255 characters.
	int nChannels, wherenow(0), nElem(0), count(0);
	size_t res;
	_int32 bufsize;
	unsigned int sz;
	CVar tp;
	try {
		if (independent)
		{
			CATCHER(res = fread((void*)&header, 1, 3, fp))
			header[res] = 0;
			if (strncmp(header, "AXL", 3) != 0) CATCHER("AXL header not found");
			CATCHER(res = fread((void*)&verstr, 1, 7, fp))
			verstr[res] = 0;
			CATCHER(res = fread((void*)&formattype, 1, 1, fp))
			formattype[res] = 0;
		}
		wherenow = ftell(fp);
		CATCHER((res = fread((void*)&readbuf, 1, sizeof(readbuf), fp)));
		strncpy(_varname, readbuf, res); 		_varname[255] = 0;
		if (strlen(_varname) >= sizeof(readbuf) - 1) { sprintf(errstr, "varname longer than specified."); throw errstr; }
		varname = _varname;
		res = fseek(fp, wherenow + (int)varname.size() + 1, SEEK_SET);
		CATCHER(fread((void*)&type, 1, 1, fp));
		CATCHER(fread((void*)&bufsize, 4, 1, fp));
//		wherenow = ftell(fp);
//		res = fseek(fp, wherenow + (int)varname.size() + 1, SEEK_SET);
		CATCHER(res = fread((void*)&sz, sizeof(sz), 1, fp))
		switch (type)
		{
		case CSIG_EMPTY:
		case CSIG_STRING:
			tp.Reset(2);
			tp.UpdateBuffer((int)sz);
			res = fread((void*)tp.buf, sz, 1, fp);
			break;
		case CSIG_SCALAR:
			if (bufsize == 16)
				tp.bufBlockSize = 16;
			tp.UpdateBuffer(1);
			CATCHER(res = fread(tp.buf, tp.bufBlockSize, sz, fp))
			break;
		case CSIG_VECTOR:
		case CSIG_VECTOR - 1:
			tp.Reset(1);
			if (type / 2 * 2 == type) //if odd, logical
				tp.MakeLogical();
			if (bufsize == 16)
				tp.bufBlockSize = 16;
			tp.UpdateBuffer((int)sz);
			CATCHER(res = fread(tp.logbuf, tp.bufBlockSize, sz, fp))
			break;
		case CSIG_CELL: // for a cell variable, sz means the element counts
		case CSIG_STRUCT:
			nElem = (int)sz;
			break;
		case CSIG_AUDIO:
		case CSIG_AUDIO - 1:
			nChannels = (int)sz;
			tp.ReadAXL(fp, nChannels>1, errstr);
			if (nChannels == 1 || nChannels == 3) // stereo
			{
				CSignals sec;
				if (sec.ReadAXL(fp, nChannels>1, errstr)) tp.SetNextChan(&sec);
			}
			break;
		default:
			sprintf(errstr, "Invalid CSignals type.");
			throw errstr;
		}
		if (type != CSIG_CELL && type != CSIG_STRUCT)
			*pout = tp;
		else
		{
			CVar *prop = new CVar;
			string propname;
			for (int k = 0; k < nElem; k++)
			{
				read_axl_block(fp, propname, prop, errstr, false); // so header can be skipped
				if (propname == ".")
					pout->cell.push_back(*prop);
				else
					pout->strut[propname] = *prop;
			}
		}
		return 1;
	}
	catch (char *_errstr)
	{
		errstr = _errstr;
		return 0;
	}
}

int xcom::load_axl(FILE *fp, char *errstr)
{
	CAstSig *pabteg = CAstSig::vecast.back();
	string varname;
	CVar in;
	fseek(fp, 0, SEEK_END);
	int wherenow(0), filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	while (wherenow < filesize)
	{
		if (!read_axl_block(fp, varname, &in, errstr)) return 0;
		pabteg->SetVar(varname.c_str(), &in);
		wherenow = ftell(fp);
	}
	return 1;
}