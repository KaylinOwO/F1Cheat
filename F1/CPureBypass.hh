#pragma once

#include "../SDK/IHack.hh"

#include "../SDK/SDK.hh"

class CPureBypass : public IHack<CPureBypass>
{
	DWORD *dwPureLoc = NULL;

public:
	const char *name () const;

	void init ();

	void everyFrame (/*CUserCmd **/);
};

extern CPureBypass gPureBypass;
