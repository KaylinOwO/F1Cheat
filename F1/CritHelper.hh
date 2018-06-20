#pragma once

#include "../SDK/IHack.hh"
#include "../SDK/SDK.hh"
#include "F1_ConVar.hh"

class CCritHelper : public IHack< CCritHelper >, IGameEventListener2
{
public:
    F1_ConVar< Switch > critSwitch{"Crit hack", "f1_crithack_switch", false};
    F1_BindableConVar critButton{"Crit button", "f1_crithack_critkey", false, &critSwitch};
    F1_ConVar< bool > fireIfCrit{"Fire if crit", "f1_crithack_fire_if_crit", false, &critSwitch};
    F1_ConVar< bool > onlyFireIfCrit{"Only fire if crit", "f1_crithack_only_fire", false, &critSwitch};

    void init();
    void processCommand( CUserCmd *pCommand );
    bool paint();

    void FireGameEvent( IGameEvent *event );
};

extern CCritHelper gCritHelper;
