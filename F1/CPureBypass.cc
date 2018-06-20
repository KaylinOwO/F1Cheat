#include "stdafx.hh"

#include "CPureBypass.hh"

#include <tier0/memdbgon.h>

CPureBypass gPureBypass;

// just return true in order to bypass pure
bool __stdcall Hooked_IsPlayingBack()
{
	// get our return address
	//DWORD retaddr = (DWORD)_ReturnAddress();
	//static DWORD ConsistencyCheckReturn = gSignatures.GetClientSignature( "84 C0 0F 85 ? ? ? ? 83 BB" );
	//if( retaddr == ConsistencyCheckReturn )
	//    Log::Console( "Called from consistency check!" );
	return true;
}

const char *CPureBypass::name() const
{
	return "PURE-BYPASS";
}

void CPureBypass::init()
{
	// this is m_pServerStartupTable
	// it is only used otherwise for "QueryPort" which i dont think matters at all
	dwPureLoc = *reinterpret_cast<DWORD **>(gSignatures.GetEngineSignature("A1 ? ? ? ? 56 33 F6 85 C0") + 0x1);
	XASSERT(dwPureLoc);
	//gHookManager.hookMethod(gInts->DemoPlayer, gOffsets.isPlayingBack, &Hooked_IsPlayingBack);

	return;
}

void CPureBypass::everyFrame(/*CUserCmd **/)
{
	// Works as expected.
	if (*dwPureLoc) {
		//Log::Console ("USING FORCE !");
		*dwPureLoc = NULL;
	}

	// gInts->DemoPlayer->m_bInterpolateView = true;

	return;
}
