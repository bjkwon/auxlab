#include "sigproc.h"
#include "psycon.tab.h"

void CNodeProbe::tree_NARGS(const AstNode* ptree, AstNode* ppar)
{
	if (psigBase->IsGO() && psigBase->GetFs() != 3)
	{
		if (pbase->Compute(ptree->child)->type() != 1)
			throw CAstException(USAGE, *pbase, ppar).proc("Invalid index of a graphic object arrary.");
	}
	else
	{
		if (psigBase->type() & TYPEBIT_CELL && ppar->type == T_ID)
			throw CAstException(USAGE, *pbase, ppar).proc("A cell array cannot be accessed with ( ).", ppar->str);
		if (psigBase->type() & TYPEBIT_AUDIO && ptree->child->next)
		{ // 2-D style notation for audio
			if (ptree->child->type == T_FULLRANGE)
				throw CAstException(USAGE, *pbase, ppar).proc("The first arg in () cannot be : for audio.", ppar->str);
			pbase->getchannel(psigBase, ptree, ppar); 
		}
		else
			ExtractByIndex(ppar, (AstNode*)ptree); //Sig updated. No change in psig
		//if child exists --> RHS --> Sig just computed is only used as replica. Otherwise, Sig will be ignored (overridden)
		if (ptree->child && root->child && pbase->searchtree(root->child, T_REPLICA))
			pbase->replica_prep(&pbase->Sig);
	}
	if (psigBase->IsGO())
		pbase->Sig = *(pbase->pgo = psigBase);
}

CVar* CNodeProbe::cell_indexing(CVar* pBase, AstNode* pn)
{
	size_t cellind = (size_t)(int)pbase->Compute(pn->alt->child)->value(); // check the validity of ind...probably it will be longer than this.
	if (pBase->type() & TYPEBIT_CELL)
	{
		if (cellind > pBase->cell.size())
			throw CAstException(RANGE, *pbase, pn->alt).proc("", pn->str, -1, (int)cellind);
		pbase->Sig = *(psigBase = &pBase->cell.at(cellind - 1));
		if (pn->child && root->child && pbase->searchtree(root->child, T_REPLICA))
			pbase->replica_prep(&pbase->Sig);
	}
	else
	{ // in this case x{2} means second chain
		if (cellind > pBase->CountChains())
			throw CAstException(RANGE, *pbase, pn->alt).proc("", pn->str, -1, (int)cellind);
		CTimeSeries* pout = pBase;
		for (size_t k = 0; k < cellind; k++, pout = pout->chain) {}
		psigBase = &(pbase->Sig = *pout);
	}
	return psigBase;
}

