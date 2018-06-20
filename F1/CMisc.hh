#pragma once

#include "../SDK/IHack.hh"
#include "F1_ConVar.hh"

class CMisc : public IHack< CMisc >
{
    // bool bunnyhop;
    // bool tauntsilde;

    // var bunny_bool{ "Bunny hop", &bunnyhop };
    // var taunt_bool{ "Taunt slide", &tauntsilde };

    // VAR( bool, autoTaunt, "Auto taunt" );
    // VAR( int, fovChanger, "Fov", 10, 179, 110, 1 );

    F1_ConVar< Switch > miscSwitch = F1_ConVar< Switch >( "Misc", "f1_misc_switch", false );
    F1_ConVar< bool > bunnyHop = F1_ConVar< bool >( " - Bunnyhop", "f1_misc_bunnyhop", false, &miscSwitch );
    F1_ConVar< bool > tauntSlide = F1_ConVar< bool >( " - Taunt slide", "f1_misc_taunt_slide", false, &miscSwitch );
    F1_ConVar< bool > removeDisguise = F1_ConVar< bool >( " - Remove disguise", "f1_misc_remove_disguise", false, &miscSwitch );
    F1_ConVar< bool > alwaysAttack2 = F1_ConVar< bool >( " - Always attack2", "f1_misc_always_attack2", false, &miscSwitch );
    F1_ConVar< bool > noPush = F1_ConVar< bool >( " - No push", "f1_misc_no_push", false, &miscSwitch );

    // F1_ConVar< bool > actionSlotSpam = F1_ConVar< bool >( " - Noisemaker spam", "f1_misc_action_slot_spam", false, &miscSwitch );

    // F1_BindableConVar airStuck = F1_BindableConVar( " - Airstuck", "f1_misc_airstuck", false, &miscSwitch );

public:
    F1_ConVar< int > fovChanger = F1_ConVar< int >( " - Fov changer", "f1_misc_fov", 110, 10, 179, 1, &miscSwitch );
    F1_ConVar< bool > fovChangeWhenZoomed = F1_ConVar< bool >( " - Change fov when zoomed", "f1_misc_change_zoomed_fov", false, &miscSwitch );
    F1_ConVar< bool > noZoom = F1_ConVar< bool >( " - No Zoom", "f1_misc_no_zoom", false, &miscSwitch );

    CMisc()
    {
    }

    const char *name() const;
    void processCommandBeforePred( CUserCmd *pUserCmd );

    void processEntity( CBaseEntity *pBaseEntity );

    bool paint();
};

extern CMisc gMisc;
