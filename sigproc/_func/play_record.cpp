#ifdef _WINDOWS
#ifdef NO_PLAYSND
#else
#include <time.h>
#include "sigproc.h"
#include "bjcommon.h"
#include "wavplay.h"
#include "..\psycon.tab.h"


extern HWND hShowDlg;
void _record(CAstSig *past, const AstNode *pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	int nArgs = 0, devID = 0, nChans = 1;
	if (pnode->type != N_STRUCT)
	{ // If not class calling
		if (pnode->alt && pnode->alt->type == N_ARGS)
			p = pnode->alt->child;
	}
	double block = CAstSig::record_block_ms;
	double duration = -1;
	AstNode *cbnode = nullptr;
	if (pnode->type==T_ID && pnode->alt->alt)
	{ // record(....).cbname
		cbnode = pnode->alt->alt;
//		yydeleteAstNode(pnode->alt->alt, 0);
		if (pnode->tail == pnode->alt->alt)
			(AstNode *)pnode->tail = nullptr;
		pnode->alt->alt = nullptr;
	}
	else if (pnode->type == N_STRUCT && pnode->alt)
	{ // v.record.cbname
		cbnode = pnode->alt;
		if (pnode->tail == pnode->alt)
			(AstNode *)pnode->tail = nullptr;
		(AstNode *)pnode->alt = nullptr; // check
	}
	for (const AstNode *cp = p; cp; cp = cp->next)
		++nArgs;
	switch (nArgs)
	{
	case 4:
		past->Compute(p->next->next->next);
		if (!past->Sig.IsScalar())
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("The fourth argument must be a constant representing the block size for the callback in milliseconds.");
		block = past->Sig.value();
	case 3:
		past->Compute(p->next->next);
		if (!past->Sig.IsScalar())
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("The third argument is either 1 (mono) or 2 (stereo) for recording.");
		nChans = (int)past->Sig.value();
		if (nChans != 1 && nChans != 2)
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("The third argument is either 1 (mono) or 2 (stereo) for recording.");
	case 2:
		past->Compute(p->next);
		if (!past->Sig.IsScalar())
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("The second argument must be a constant representing the duration to record, -1 means indefinite duration until stop is called.");
		duration = past->Sig.value();
	case 1:
		past->Compute(p);
		if (past->Sig.IsScalar())
		{
			devID = (int)past->Sig.value();
			past->Sig.Reset();
		}
		else if (past->Sig.IsStruct() && past->Sig.strut.find("type") != past->Sig.strut.end())
		{
			CVar *pobj = &past->Sig;
			string objname = "devID";
			if (pobj->strut.find(objname) == pobj->strut.end())
				pobj->strut[objname] = CVar((double)devID);
			else
				devID = (int)pobj->strut[objname].value();
			if (pobj->strut.find(objname = "dur") == pobj->strut.end())
				pobj->strut[objname] = CVar((double)duration);
			else
				duration = (int)pobj->strut[objname].value();
			if (pobj->strut.find(objname = "channels") == pobj->strut.end())
				pobj->strut[objname] = CVar(1.);
			else
				nChans = (int)pobj->strut[objname].value();
			if (pobj->strut.find(objname = "block") == pobj->strut.end())
				pobj->strut[objname] = CVar(block);
			else
				block = (int)pobj->strut[objname].value();
		}
		else
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("The first argument must be an audio_recorder object or a constant (integer) representing the device ID.");
		break;
	case 0:
		break;
	}
	srand((unsigned)time(0));
	CVar handle((double)rand());
	handle.strut["dev"] = CVar((double)devID);
	handle.strut["type"] = CVar(string("audio_record"));
	handle.strut["id"] = CVar(handle.value());
	handle.strut["callback"] = "";
	handle.strut["channels"] = CVar((double)nChans);
	handle.strut["durLeft"] = CVar(duration / 1000.);
	handle.strut["durRec"] = CVar(0.);
	handle.strut["block"] = CVar(block);
	handle.strut["active"] = CVar((double)(1 == 0));

	char errstr[256] = {};
	int newfs, recordID = (int)handle.value();
	if ((newfs = Capture(devID, WM__AUDIOEVENT2, past->pEnv->Fs, nChans, CAstSig::record_bytes, cbnode, duration, block, recordID, errstr)) < 0)
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc(errstr);
	handle.strut["active"] = CVar((double)(1 == 1));
	past->Sig.strut["h"] = handle;
	//output binding
	if (past->lhs)
	{
		if (past->lhs->type == N_VECTOR)
			past->outputbinding(past->lhs);
		else
			past->bind_psig(past->lhs, &past->Sig);
	}
	else
	{ // ans variable
		past->SetVar("ans", &past->Sig);
	}

	// for a statement, y=h.start, y is not from the RHS directly, but is updated laster after the callback
	// so we need to block the RHS from affecting the LHS.. Let's use -1 for suppress (to be used in CDeepProc::TID_tag in AstSig2.cpp)
	past->xtree->suppress = -1;
	if (newfs != past->pEnv->Fs)
	{
		past->pEnv->Fs = newfs;
		sformat(past->statusMsg, "(NOTE)Sample Rate of AUXLAB Environment is adjusted to %d Hz.", past->pEnv->Fs);
	}
	past->Sig.Reset(); // to shield the first LHS variable (callback output) from Sig // ??? 11/29/2019
}
void _play(CAstSig *past, const AstNode *pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	CVar sig = past->Sig;
	//sig must be either an audio signal or an audio handle.
	char errstr[256] = {};
	int nRepeats(1);
	double block = CAstSig::play_block_ms;
	if (sig.GetType() != CSIG_AUDIO)
	{
		if (!sig.IsScalar())
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("The base must be an audio signal or an audio handle (1)");
		if (sig.strut.find("type")== sig.strut.end())
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("The base must be an audio signal or an audio handle (2)");
		CVar type = sig.strut["type"];
		if (type.GetType()!=CSIG_STRING || type.string() != "audio_playback")
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("The base must be an audio signal or an audio handle (3)");
		if (!p)
			throw CAstException(FUNC_SYNTAX, *past, pnode).proc("Audio signal not given.");
		past->Compute(p);
		past->checkAudioSig(p, past->Sig);
		CVar audio = past->Sig;
		if (p->next)
		{
			past->Compute(p->next);
			if (!past->Sig.IsScalar())
				throw CAstException(FUNC_SYNTAX, *past, p).proc("Argument must be a scalar.");
			nRepeats = (int)past->Sig.value();
			if (nRepeats<1)
				throw CAstException(FUNC_SYNTAX, *past, p).proc("Repeat counter must be equal or greater than one.");
		}
		INT_PTR h = PlayCSignals((INT_PTR)sig.value(), audio, 0, WM__AUDIOEVENT1, &block, errstr, nRepeats);
		if (!h)
			past->Sig.SetValue(-1.);
		else
		{
			//Update the handle with the PlayArrayNext info
			//don't update sig, which is only a copy. Instead, go for Vars
			for (map<string, CVar>::iterator it = past->Vars.begin(); it != past->Vars.end(); it++)
			{
				if ((*it).second == sig.value())
				{
					(*it).second.strut["durLeft"].SetValue(sig.strut["durLeft"].value() + audio.alldur()*nRepeats / 1000);
					(*it).second.strut["durTotal"].SetValue(sig.strut["durTotal"].value()+ audio.alldur()*nRepeats / 1000);
					past->Sig = (*it).second;
				}
			}
		}
	}
	else
	{
		if (p)
		{
			past->Compute(p);
			if (!past->Sig.IsScalar())
				throw CAstException(FUNC_SYNTAX, *past, p).proc("Argument must be a scalar.");
			nRepeats = (int)past->Sig.value();
			if (nRepeats<1)
				throw CAstException(FUNC_SYNTAX, *past, p).proc("Repeat counter must be equal or greater than one.");
		}
		int devID = 0;
		INT_PTR h = PlayCSignals(sig, devID, WM__AUDIOEVENT1, GetHWND_WAVPLAY(), &block, errstr, nRepeats);
		if (!h)
		{ // PlayArray will return 0 if unsuccessful due to waveOutOpen failure. For other reasons.....
			past->Sig.strut.clear();
			//errstr should show the err msg. Use it if necessary 7/23/2018
		}
		else
		{
			double ad = sig.alldur();
			double addtime = ad * nRepeats / 1000.;
			AUD_PLAYBACK * p = (AUD_PLAYBACK*)h;
			p->sig.SetValue((double)(INT_PTR)h);
//			p->sig.strut["data"] = sig; // Let's not do this any more.. no strong need. 9/9/2019
			p->sig.strut.insert(pair<string, CVar>("type", string("audio_playback")));
			p->sig.strut.insert(pair<string, CVar>("devID", CVar((double)devID)));
			p->sig.strut.insert(pair<string, CVar>("durTotal", CVar(addtime)));
			if (p->sig.strut.find("durLeft") == p->sig.strut.end())
				p->sig.strut.insert(pair<string, CVar>("durLeft", CVar(addtime)));
			else
				*(p->sig.strut["durLeft"].buf) += addtime;
			p->sig.strut.insert(pair<string, CVar>("durPlayed", CVar(0.)));
			past->Sig = p->sig; //only to return to xcom
		}
	}
}

void _stop(CAstSig *past, const AstNode *pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	char errstr[256];
	CVar sig = past->Sig;
	if (!sig.IsScalar())
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("Argument must be a scalar.");
	string fname = past->xtree->str;
	if (!p)
		fname = past->xtree->alt->str;
	if (sig.strut["type"].string() == "audio_playback" && (fname == "qstop" || fname == "stop"))
	{
		if (!StopPlay((INT_PTR)sig.value(), fname == "qstop"))
		{
			past->Sig.strut.clear();
			past->Sig.SetValue(-1.);
		}
	}
	else if (sig.strut["type"].string() == "audio_record" && fname == "stop")
	{
		StopRecord((int)sig.value(), errstr);
	}
	else
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("stop() applies only to audio_playback or audio_record.");
}

void _pause_resume(CAstSig *past, const AstNode *pnode)
{
	const AstNode* p = get_first_arg(pnode, (*(past->pEnv->builtin.find(pnode->str))).second.alwaysstatic);
	CVar sig = past->Sig;
	if (!sig.IsScalar())
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("Argument must be a scalar.");
	string fname = past->xtree->str;
	if (!p)
		fname = past->xtree->alt->str;
	if (sig.strut["type"].string() == "audio_playback")
	{
		if (!PauseResumePlay((INT_PTR)sig.value(), fname == "resume"))
		{
			past->Sig.strut.clear();
			past->Sig.SetValue(-1.);
		}
	}
	else if (sig.strut["type"].string() == "audio_record")
	{

	}
	else
		throw CAstException(FUNC_SYNTAX, *past, pnode).proc("pause() or resume() applies only to audio_playback or audio_record.");
}

#endif // NO_PLAYSND


#include "audstr.h"


#endif //_WINDOWS this
