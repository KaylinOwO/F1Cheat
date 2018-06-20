#pragma once

#include "../SDK/IHack.hh"
#include "../SDK/SDK.hh"
#include "../SDK/network.hh"
#include "F1_ConVar.hh"

// TODO
// try to make all hacks singletons of some form
class CAntiSmac : public IHack<CAntiSmac>
{

    std::vector<cvar_t> serverConvars;

    F1_ConVar<Switch> antiSmacSwitch = F1_ConVar<Switch> ("AntiSmac", "f1_antismace_switch", false);
    F1_ConVar<bool>   nameSpam       = F1_ConVar<bool> (" - Name spam", "f1_antismac_namespam", false, &antiSmacSwitch);
    F1_ConVar<bool>   debug          = F1_ConVar<bool> (" - Debug info", "f1_antismac_debug_info", false, &antiSmacSwitch);

public:
    CAntiSmac ()
    {
    }

    static CAntiSmac *getInst ()
    {
        extern CAntiSmac gAntiSmac;
        return &gAntiSmac;
    }

    CAntiSmac (const CAntiSmac &c) = delete;
    void operator= (CAntiSmac &o) = delete;

    const char *name () const;

    void init ();

    void processCommand (CUserCmd *pUserCmd);

    bool processGetCvarValue (SVC_GetCvarValue *msg);

    bool processSetConVar (NET_SetConVar *msg);

    bool processStringCmd (NET_StringCmd *msg);

    void menuUpdate (F1_IConVar **menuArray, int &currIndex);
};

extern CAntiSmac gAntiSmac;
