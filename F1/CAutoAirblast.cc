#include "stdafx.hh"

#include "CAutoAirblast.hh"

#include "LagCompensation.hh"

#include <tier0/memdbgon.h>

CAutoAirblast gAutoAirblast;

CAutoAirblast::CAutoAirblast()
{
}

void CAutoAirblast::processCommand(CUserCmd *pUserCmd)
{
    if (enabled.Value() == false)
        return;

    auto *localPlayer = GetLocalPlayer();

    auto *activeWeapon = localPlayer->GetActiveWeapon();

    if (activeWeapon && activeWeapon->GetClientClass()->classId == classId::CTFFlameThrower) {
        CTarget closeIndex = get_best_target();

        if (closeIndex.ent == nullptr)
            return;

        if (aimMode.Value() == true) {
            // pUserCmd->viewangles.z += 23421341234151324123543514534534534590.0f;
            aimer.aim(pUserCmd, closeIndex.target, true);
        }

        //Log::Console ("Target!");

        pUserCmd->buttons |= IN_ATTACK2;
    }
    return;
}

bool CAutoAirblast::is_valid_target(const CBaseEntity *pBaseEntity)
{

    // CBaseEntity *pBaseEntity = GetBaseEntity(index);

    // if the projectile is from our own team we dont want or need to reflect it
    if (pBaseEntity->GetTeam() == GetLocalPlayer()->GetTeam()) {
        return false;
    }

    auto entTag = CEntTag(const_cast<CBaseEntity *>(pBaseEntity));

    if (entTag.isProjectile()) {
        //Log::Console ("Is projectile");
        return true;
    }

    return false;
}

bool CAutoAirblast::is_visible_target(const CBaseEntity *pBaseEntity, Vector &t)
{
    // CBaseEntity *pBaseEntity = GetBaseEntity(index);

    Vector vel = EstimateAbsVelocity(const_cast<CBaseEntity *>(pBaseEntity));

    if (vel == Vector(0, 0, 0)) {
        return false;
    }

    Vector origin;
    pBaseEntity->GetWorldSpaceCenter(origin);

    Vector eyePos = GetLocalPlayer()->GetViewPos();

    // float latency = gInts->Engine->GetNetChannelInfo()->GetLatency( FLOW_INCOMING ) + gInts->Engine->GetNetChannelInfo()->GetLatency( FLOW_OUTGOING );

    // Vector target = origin + ( vel * latency );
    t = origin + vel * TICK_INTERVAL;

    float length = (eyePos - t).Length();

    //gInts->DebugOverlay->AddEntityTextOverlay (pBaseEntity->GetIndex (), 0, 0.0f, 255, 255, 255, 255, "valid target");
    //gInts->DebugOverlay->AddEntityTextOverlay (pBaseEntity->GetIndex (), 1, 0.0f, 255, 255, 255, 255, "distance %.2f", length);

    if (length <= (185.0f)) {
        //Log::Console ("Target at %f dist %f %f %f", length, eyePos.x, eyePos.y, eyePos.z);
        if (__CTargetHelper::GetFovFromLocalPlayer(t) > maxFOV.Value()) {
            return false;
        }
        return true;
    }
    return false;
}

bool CAutoAirblast::compare_target(const CTarget &bestTarget, const CTarget &newTarget)
{
    // doesnt really matter as once one is within range, we will airblast
    if (__CTargetHelper::getDistanceToVector(bestTarget.target) < __CTargetHelper::getDistanceToVector(newTarget.target)) {
        return false;
    }

    return true;
}

bool CAutoAirblast::is_only_players()
{
    return false;
}

bool CAutoAirblast::can_backtrack()
{
    return false;
}
