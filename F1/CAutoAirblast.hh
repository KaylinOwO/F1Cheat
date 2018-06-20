#pragma once

#include "../SDK/IHack.hh"
#include "F1_ConVar.hh"

#include "AimHelpers.hh"
#include "TargetManager.hh"

class CAutoAirblast : public IHack<CAutoAirblast>, public TargetManager<CAutoAirblast>
{
    F1_ConVar<Switch> autoAirblastSwitch = F1_ConVar<Switch>("Auto Airblast", "f1_autoairblast_switch", false);
    F1_ConVar<bool>   enabled            = F1_ConVar<bool>(" - Enabled", "f1_autoairblast_enable", false, &autoAirblastSwitch);
    F1_ConVar<bool>   aimMode            = F1_ConVar<bool>(" - Aim mode", "f1_autoairblast_aim_mode_enable", false, &autoAirblastSwitch);
    F1_ConVar<float>  maxFOV             = F1_ConVar<float>(" - Max FOV", "f1_autoairblast_max_fov", 360, 360, 360, 1, &autoAirblastSwitch);

public:
    CAutoAirblast();

    void processCommand(CUserCmd *pUserCmd);

    bool is_valid_target(const CBaseEntity *ent);
    bool is_visible_target(const CBaseEntity *ent, Vector &t);
    bool compare_target(const CTarget &bestTarget, const CTarget &newTarget);

    bool is_only_players();

    bool can_backtrack();

private:
    // int findBestTarget();
};

extern CAutoAirblast gAutoAirblast;
