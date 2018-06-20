#pragma once

#include "../SDK/IHack.hh"
#include "F1_ConVar.hh"

class CBaseCombatWeapon;

class CBackstab : public IHack< CBackstab >
{
    F1_ConVar< Switch > autoBackstabSwitch = F1_ConVar< Switch >( "Auto backstab", "f1_autobackstab_switch", false );
    F1_ConVar< bool > enabled = F1_ConVar< bool >( " - Enabled", "f1_autobackstab_enable", false, &autoBackstabSwitch );

public:
    CBackstab()
    {
    }

    const char *name() const;
    void processCommandBeforePred( CUserCmd *pUserCmd );

    void menuUpdate( F1_IConVar **menuArray, int &currIndex );

private:
    // internal
    // replaces netvar
    bool canBackstab( CBaseCombatWeapon *pBaseCombatWeapon, CBaseEntity *pBaseEntity );

    bool isBehind( CBaseEntity *pBaseEntity, CBaseEntity *pLocalEntity );

    bool engineCanBackstab( CBaseCombatWeapon *weapon, CBaseEntity *target );
};

extern CBackstab gBackstab;
