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

#include <windows.h>
#include <string>
#include <map>
#include "sigproc.h"
#include "sigprocExt.h"
#include "resource.h"

HINSTANCE hInst;
vector<CAstSig*> xcomvecast;
vector<CAstSig*> CAstSig::vecast = xcomvecast;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInst = hModule;
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

void _time_freq_manipulate_ext(CAstSig *past, const AstNode *pnode, const AstNode *p, string &fnsigs)
{
	//check qualifers
	past->checkAudioSig(pnode, past->Sig);
	CVar param, paramopt;
	string fname;
	try {
		CAstSig tp(past);
		param = tp.Compute(p);
		if (p->next)
		{
			paramopt = tp.Compute(p->next);
			if (paramopt.strut.empty())
				throw past->ExceptionMsg(p, fnsigs, "Third parameter, if used, must be a struct variable.");
		}
		int type = param.GetType();
		if (type != CSIG_TSERIES && type != CSIG_SCALAR)
			throw past->ExceptionMsg(p, fnsigs, "parameter must be either a scalar or a time sequence.");
		if (param.GetType() == CSIG_TSERIES)
		{
			double audioDur = past->Sig.dur();
			if (param.GetFs() == 0) // relative
				for (CTimeSeries *p = &param; p; p = p->chain)
				{
					p->tmark *= audioDur;
					p->SetFs(past->Sig.GetFs());
				}
			//If the first tmark is not 0, make one with 0 tmark and bring the value at zero
			if (param.tmark != 0.)
			{
				CTimeSeries newParam(past->Sig.GetFs());
				newParam.tmark = 0.;
				newParam.SetValue(param.value());
				newParam.chain = new CTimeSeries;
				*newParam.chain = param; // this way the copied version goes to chain
				param = newParam;
			}
			//If the last tmark is not the end of the signal, make one with 0 tmark and bring the value at zero
			CTimeSeries *pLast=NULL;
			for (CTimeSeries *p = &param; p; p = p->chain)
				if (!p->chain)
					pLast = p;
			if (pLast && fabs(pLast->tmark - past->Sig.dur()) > 10.) // give a 10 milliseconds margin to the end edge time point.
			{
				CTimeSeries newParam(past->Sig.GetFs());
				newParam.tmark = past->Sig.dur();
				newParam.SetValue(pLast->value());
				pLast->chain = new CTimeSeries;
				*pLast->chain = newParam; // this way the copied version goes to chain
			}
			//IF tsequence goes beyond the audio duration, cut it out.
			for (CTimeSeries *p = &param; p; p = p->chain)
				if (p->chain && p->chain->tmark > audioDur)
				{
					delete p->chain;
					p->chain = NULL;
				}
			//if same tmark repeats, with different values, adjust them with a rasonable margin.
			//for (CTimeSeries *p = &param; p && p->chain; p = p->chain)
			//{
			//	double margin = 25.;
			//	if (p->tmark == p->chain->tmark)
			//	{
			//		p->tmark -= margin;
			//		p->chain->tmark += margin;
			//	}
			//}
		}
		fname = pnode->str;
		CSignalsExt out(past->Sig);
		if (fname == "timestretch") out.pf_exe2 = &CSignalExt::timestretch;
		else if (fname == "pitchscale") out.pf_exe2 = &CSignalExt::pitchscale;
		else if (fname == "respeed") out.pf_exe2 = &CSignal::resample;
		for (auto it = paramopt.strut.begin(); it != paramopt.strut.end(); it++)
			param.strut[(*it).first] = (*it).second;
		out.basic(out.pf_exe2, &param);
		past->Sig = out.make_CVar();
//		past->Sig.basic(past->Sig.pf_basic2, &param);
		if (param.IsString())
			throw past->ExceptionMsg(pnode, ("Error in respeed:" + param.string()).c_str());
	}
	catch (const CAstException &e) { throw past->ExceptionMsg(pnode, fnsigs, e.getErrMsg()); }
	if (fname == "timestretch" || fname == "respeed")
	{ // Take care of overlapping chains after processing
		past->Sig.MergeChains();
	}
}

__declspec (dllexport) map<string, Cfunction>  Init()
{
	map<string, Cfunction> out;
	string name;
	Cfunction ft;

	ft.narg1 = 2;	ft.narg2 = 2;
	name = "movespec";
	ft.funcsignature = "(audio_signal, frequency_to_shift)";
	ft.func = &_time_freq_manipulate_ext;
	out[name] = ft;

	ft.narg1 = 2;	ft.narg2 = 3;
	name = "timestretch";
	ft.funcsignature = "(array, ratio [, optional])";
	ft.func = &_time_freq_manipulate_ext;
	out[name] = ft;

	name = "pitchscale";
	ft.funcsignature = "(array, ratio [, optional])";
	ft.func = &_time_freq_manipulate_ext;
	out[name] = ft;

	name = "respeed";
	ft.funcsignature = "(audio_signal, playback_rate_change_ratio [, optional])";
	ft.func = &_time_freq_manipulate_ext;
	out[name] = ft;
	
	return out;
}