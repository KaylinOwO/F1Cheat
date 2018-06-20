#pragma once

#include "../SDK/IHack.hh"
#include "F1_ConVar.hh"

class CKnifeInterface : public IHack<CKnifeInterface>
{
public:
	void *initFnPtrs[255];
	void  init ()
	{
		for (auto i = 0; initFnPtrs[i] != nullptr; i++) {
			((void(__cdecl *) (void)) (initFnPtrs[i])) ();
		}
		return;
	}

	void *paintFnPtrs[255];
	bool  paint ()
	{
		for (auto i = 0; paintFnPtrs[i] != nullptr; i++) {
			((void(__cdecl *) (void)) (initFnPtrs[i])) ();
		}
		return false;
	}

	void *processCommandBeforePredFnPtrs[255];
	void  processCommandBeforePred (CUserCmd *pCommand)
	{
		for (auto i = 0; processCommandBeforePredFnPtrs[i] != nullptr; i++) {
			((void(__cdecl *) (CUserCmd *)) (initFnPtrs[i])) (pCommand);
		}
		return;
	}

	void *processCommandFnPtrs[255];
	void  processCommand (CUserCmd *pUserCmd)
	{
		for (auto i = 0; processCommandFnPtrs[i] != nullptr; i++) {
			((void(__cdecl *) (CUserCmd *)) (processCommandFnPtrs[i])) (pUserCmd);
		}
		return;
	}

	void *keyEventFnPtrs[255];
	bool  keyEvent (ButtonCode_t keynum)
	{
		bool retval = true;
		for (auto i = 0; keyEventFnPtrs[i] != nullptr; i++) {
			retval = ((bool(__cdecl *) (ButtonCode_t)) (keyEventFnPtrs[i])) (keynum) ? retval : false;
		}
		return retval;
	}

	void *processEntityFnPtrs[255];
	void  processEntity (CBaseEntity *pBaseEntity)
	{
		for (auto i = 0; processEntityFnPtrs[i] != nullptr; i++) {
			((void(__cdecl *) (CBaseEntity *)) (processEntityFnPtrs[i])) (pBaseEntity);
		}
		return;
	}

	CKnifeInterface ()
	{
		memset (initFnPtrs, 0, sizeof (initFnPtrs));
		memset (paintFnPtrs, 0, sizeof (paintFnPtrs));
		memset (processCommandBeforePredFnPtrs, 0, sizeof (processCommandBeforePredFnPtrs));
		memset (processCommandFnPtrs, 0, sizeof (processCommandFnPtrs));
		memset (keyEventFnPtrs, 0, sizeof (keyEventFnPtrs));
		memset (processEntityFnPtrs, 0, sizeof (processEntityFnPtrs));
	}
};

extern CKnifeInterface gKnifeInterface;
