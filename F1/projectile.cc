#include "stdafx.hh"

#include "projectile.hh"

#include <tier0/memdbgon.h>

// Rocket launchers:
//        break;
//    Direct hit:
//        Speed = 1980.0f;
//    LibertyLauncher:
//        Speed = 1540.0f;
//    Grenade launchers:
//        Speed = 1215.0f;
//        Gravity = 200.0f;
//        LochNLoad:
//        Speed = 1510.0f;
//        Gravity = 250.0f;
//    Flaregun, Detonator, ScorchShot:
//        Speed = 2000.0f;
//        Gravity = 250.0f;
//    WPN_ManMelter:
//        Speed = 3000.0f;
//        Gravity = 325.0f;
//    SyringeGun, Blutsauger, Overdose:
//        Speed = 1000.f;
//        Gravity = 240;

float projectileHelper::GetProjectileSpeed(CBaseCombatWeapon *wep)
{

    if (wep == nullptr)
        return 0.0f;

    // return wep->GetProjectileSpeed();

    classId id = wep->GetClientClass()->classId;

    float BeginCharge;
    float Charge;

    // return flSpeed;
    switch (id) {

    case classId::CTFRocketLauncher_DirectHit: {
        return 1980.0f;
    }

    case classId::CTFRocketLauncher:
    case classId::CTFRocketLauncher_Mortar:
        return 1100.0f;

    case classId::CTFFlareGun:
        // this could also be 1500
        return 2000.0f;

    case classId::CTFBat_Wood:
        return 1940.0f;

    case classId::CTFSyringeGun:
        return 1000.0f;

    case classId::CTFGrenadeLauncher:
    case classId::CTFCompoundBow: {
        return wep->GetProjectileSpeed();

        BeginCharge = wep->GetChargeTime();

        Charge = BeginCharge == 0.0f ? 0.0f : gInts->Globals->curtime - BeginCharge;

        if (Charge > 1.0f)
            Charge = 1.0f;

        return (800 * Charge + 1800);
    }

    default:
        return 0.0f;
    }

    return 0.0f;
}

float projectileHelper::GetProjectileGravity(CBaseCombatWeapon *wep)
{

    if (wep == nullptr)
        return 0.0f;

    classId id = wep->GetClientClass()->classId;

    static ConVar *sv_gravity = gInts->Cvar->FindVar("sv_gravity");

    switch (id) {
    case classId::CTFCompoundBow: {
        return wep->GetProjectileGravity() * sv_gravity->GetFloat();
        float BeginCharge;
        float Charge;

        BeginCharge = wep->GetChargeTime();

        Charge = BeginCharge == 0.0f ? 0.0f : gInts->Globals->curtime - BeginCharge;

        if (Charge > 1.0f)
            Charge = 1.0f;

        Log::Console("grav for huntsman is %f", ((1.3 - Charge) / 3) * 1000);

        return (((1.3 - Charge) / 3) * 1000);
        break;
    }
    case classId::CTFFlareGun:
        return 0.3 * sv_gravity->GetFloat();

    case classId::CTFGrenadeLauncher:
        return 0.5 * sv_gravity->GetFloat(); // TODO CORRECT THIS

    case classId::CTFSyringeGun:
        return 0.3f * sv_gravity->GetFloat();

    default:
        return 0.0f;
    }
}

void projectileHelper::PhysicsClipVelocity(const Vector &in, const Vector &normal, Vector &out, float overbounce)
{
    float backoff = /*DotProduct(in, normal)*/ in.Dot(normal) * overbounce;

    for (int i = 0; i < 3; ++i) {
        float change = normal[i] * backoff;
        out[i]       = in[i] - change;

        if (out[i] > -0.1f && out[i] < 0.1f)
            out[i] = 0.0f;
    }

    float adjust = /*DotProduct( out, normal )*/ out.Dot(normal);

    if (adjust < 0.0f)
        out -= (normal * adjust);
}

bool projectileHelper::PhysicsApplyFriction(const Vector &in, Vector &out, float flSurfaceFriction, float flTickRate)
{
    static ConVar *sv_friction  = gInts->Cvar->FindVar("sv_friction");
    static ConVar *sv_stopspeed = gInts->Cvar->FindVar("sv_stopspeed");

    float speed = in.Length() / flTickRate;

    if (speed < 0.1f)
        return false;

    float drop = 0.0f;

    if (flSurfaceFriction != -1.0f) {
        float friction = sv_friction->GetFloat() * flSurfaceFriction;
        float control  = (speed < sv_stopspeed->GetFloat()) ? sv_stopspeed->GetFloat() : speed;
        drop += control * friction * flTickRate;
    }

    float newspeed = speed - drop;

    if (newspeed < 0.0f)
        newspeed = 0.0f;

    if (newspeed != speed) {
        newspeed /= speed;
        out = in * newspeed; // VectorScale(in, newspeed, out);
    }

    out -= in * (1.0f - newspeed);
    out *= flTickRate;
    return true;
}

void projectileHelper::DrawDebugArrow(const Vector &vecFrom, const Vector &vecTo, const DWORD color)
{
    QAngle angRotation;
    VectorAngles(vecTo - vecFrom, angRotation);
    Vector vecForward, vecRight, vecUp;
    AngleVectors(angRotation, &vecForward, &vecRight, &vecUp);
    gInts->DebugOverlay->AddLineOverlay(vecFrom, vecTo, RED(color), GREEN(color), BLUE(color), true, 0.0f);
    gInts->DebugOverlay->AddLineOverlay(vecFrom, vecFrom - vecRight * 10.0f, RED(color), GREEN(color), BLUE(color), true, 0.0f);
}

float projectileHelper::GetEntityGravity(CBaseEntity *ent)
{
    moveTypes type = ent->GetMoveType();

    if (type == moveTypes::noclip || type == moveTypes::step || type == moveTypes::fly)
        return 0.0f;

    return 1.0f;
}

unsigned projectileHelper::PhysicsSolidMaskForEntity(CBaseEntity *ent)
{
    typedef unsigned int(__thiscall * OriginalFn)(PVOID);

    return getvfunc<OriginalFn>(ent, 128)(ent);
}

ConVar f1_projectile_showdebugarrows("f1_projectile_showdebugarrows", "0", FCVAR_NONE, "Show the ncc debug arrows on predicted players");

Vector projectileHelper::PredictCorrection(CBaseCombatWeapon *weapon, CBaseEntity *target, Vector from, int qual)
{
    float speed = GetProjectileSpeed(weapon);
    if (speed <= 0.0f)
        return from;

    static ConVar *sv_gravity = gInts->Cvar->FindVar("sv_gravity");

    float interpolationLag = 0.0f;

    bool         targetOnGround = target->GetFlags() & FL_ONGROUND;
    unsigned int physicsMask    = PhysicsSolidMaskForEntity(target);

    Vector projGravity  = {0, 0, sv_gravity->GetFloat() * GetProjectileGravity(weapon) * TICK_INTERVAL * TICK_INTERVAL};
    Vector worldGravity = {0, 0, sv_gravity->GetFloat() * GetEntityGravity(target) * TICK_INTERVAL * TICK_INTERVAL};

    Vector targetVelocity      = EstimateAbsVelocity(target);
    Vector targetStartPosition = target->GetAbsOrigin();

    Vector targetCurrentVelocity = targetVelocity;
    Vector targetCurrentPosition = targetStartPosition;

    Vector collideableMin = target->GetCollideableMins();
    Vector collideableMax = target->GetCollideableMaxs();

    trace_t trace;
    Ray_t   ray;

    CBaseFilter filter;
    filter.SetIgnoreSelf(target);

    float arrivalTime = ((from - targetCurrentPosition).Length() / speed) + TICK_INTERVAL;

    for (float travelTime = 0.0f; travelTime < arrivalTime; travelTime += TICK_INTERVAL) {
        ray.Init(targetCurrentPosition, targetCurrentPosition + targetCurrentVelocity, collideableMin, collideableMax);
        gInts->EngineTrace->TraceRay(ray, physicsMask, &filter, &trace);

        if (trace.startsolid) // we started in a wall!?
            break;

        if (trace.fraction != 1.0f) // we didnt get all the way there
            PhysicsClipVelocity(targetCurrentPosition, trace.plane.normal, targetCurrentVelocity, 1.0f);

        targetCurrentPosition = trace.endpos;

        ray.Init(targetCurrentPosition, targetCurrentPosition + worldGravity, collideableMin, collideableMax);
        gInts->EngineTrace->TraceRay(ray, physicsMask, &filter, &trace);

        if (trace.fraction == 1.0f) {
            targetOnGround = false;
            targetCurrentVelocity += worldGravity;
        } else if (targetOnGround == false) {
            float surfaceFriction = 1.0f;
            gInts->PhysicsSurfaceProps->GetPhysicsProperties(trace.surface.surfaceProps, nullptr, nullptr, &surfaceFriction, nullptr);

            if (PhysicsApplyFriction(targetCurrentVelocity, targetCurrentVelocity, surfaceFriction, TICK_INTERVAL) == false)
                break;
        }

        if (targetCurrentVelocity.Length() > speed)
            break; // target faster than projectile

        arrivalTime = ((from - targetCurrentPosition).Length() / speed) + TICK_INTERVAL;
    }

    return targetCurrentPosition - targetStartPosition;
}

float GetClientDistanceToGround(CBaseEntity *target)
{

    if (target->IsTouchingGround())
        return 0.0f;

    Vector fOrigin, fGround;
    fOrigin = target->GetAbsOrigin();

    fOrigin[2] += 10.0;

    trace_t       tr;
    Ray_t         ray;
    CCustomFilter filter([&](IHandleEntity *h, int mask) -> bool {
        if (!h)
            return false;
        if (auto *ent = GetBaseEntity(h->GetRefEHandle())) {
            if (ent->GetClientClass()->classId == classId::CWorld)
                return true;
        }

        return false;
    });

    gInts->EngineTrace->TraceRay(ray, projectileHelper::PhysicsSolidMaskForEntity(target), &filter, &tr);

    if (tr.m_pEnt != nullptr) {
        fGround = tr.endpos;
        fOrigin[2] -= 10.0;
        return (fOrigin - fGround).Length();
    }
    return 0.0;
}

Vector projectileHelper::PredictCorrection2(CBaseCombatWeapon *weapon, CBaseEntity *target, Vector vecFrom, int passes)
{
    Vector out;
    Vector playerLoc = vecFrom;

    Vector targetLoc = target->GetAbsOrigin();
    Vector targetVel = EstimateAbsVelocity(target);

    // get current weapon's projectile velocity
    float projectileVel = GetProjectileSpeed(weapon);

    // get gravity
    static ConVar *sv_gravity = gInts->Cvar->FindVar("sv_gravity");
    float          g          = -sv_gravity->GetFloat() * GetEntityGravity(target);

    float distance, time, gfactor;

    float distanceToGround = GetClientDistanceToGround(target);
    float minDistance      = 20.0;

    // calculate the ideal location using n passes
    for (int i = 0; i < passes; i++) {

        // calculate travel time by projectile from player to target
        if (i == 0)
            distance = (targetLoc - playerLoc).Length();
        else
            distance = (playerLoc - out).Length();
        if (projectileVel != 0.0)
            time = distance / projectileVel;
        else
            time = 0.0; // hitscan or melee weapon, no travel time

        // compensate for weapon warmup and latency on final pass
        // if(i == passes - 1)
        //{
        //	time += GetWarmup(player);
        //	time += GetClientAvgLatency(player, NetFlow_Both) / 2;
        //}

        // extrapolate along the trajectory (time) seconds ahead
        out[0] = targetLoc[0] + targetVel[0] * time;
        out[1] = targetLoc[1] + targetVel[1] * time;

        // determine if gravity should be accounted for or not
        gfactor = 0.5 * g * time * time;
        if (distanceToGround < minDistance)
            gfactor = 0.0;

        out[2] = targetLoc[2] + targetVel[2] * time + gfactor;
    }

    // raise target to chest level, most tf2 class models are 100 HU tall
    // out[2] += 50;
    return out;
}

void projectileHelper::PredictPath(CBaseEntity *target)
{
    gInts->DebugOverlay->ClearAllOverlays();
    if (target == nullptr)
        return;

    static ConVar *sv_gravity = gInts->Cvar->FindVar("sv_gravity");

    Vector vecVelocity = EstimateAbsVelocity(target);

    bool         bIsOnGround     = target->GetFlags() & FL_ONGROUND;
    unsigned int mask            = PhysicsSolidMaskForEntity(target);
    Vector       vecWorldGravity = Vector(0, 0, -sv_gravity->GetFloat() * GetEntityGravity(target) * gInts->Globals->interval_per_tick * gInts->Globals->interval_per_tick);
    Vector       vecStepPos      = target->GetAbsOrigin();

    // auto pCollideable = target->GetCollideable();

    // if(pCollideable == nullptr)
    //	return;

    // Vector vecMins = target->GetCollideable()->OBBMins();
    // Vector vecMaxs = target->GetCollideable()->OBBMaxs();

    // we can get the collideable through the netvars
    Vector vecMins = target->GetCollideableMins();
    Vector vecMaxs = target->GetCollideableMaxs();

    vecVelocity *= gInts->Globals->interval_per_tick;

    trace_t     tr;
    Ray_t       ray;
    CBaseFilter filter;

    filter.SetIgnoreSelf(target);

    for (int i = 0; i < 100; ++i) {
        ray.Init(vecStepPos, vecStepPos + vecVelocity, vecMins, vecMaxs);
        gInts->EngineTrace->TraceRay(ray, mask, &filter, &tr);

        if (tr.startsolid)
            break;

        if (tr.fraction != 1.0f)
            PhysicsClipVelocity(vecVelocity, tr.plane.normal, vecVelocity, 1.0f);

        DrawDebugArrow(vecStepPos, tr.endpos, redGreenGradiant(i, 100));
        vecStepPos = tr.endpos;

        ray.Init(vecStepPos, vecStepPos + vecWorldGravity, vecMins, vecMaxs);
        gInts->EngineTrace->TraceRay(ray, mask, &filter, &tr);

        if (tr.fraction == 1.0f) {
            bIsOnGround = false;
            vecVelocity += vecWorldGravity;
        } else if (!bIsOnGround) {
            float surfaceFriction = 1.0f;
            gInts->PhysicsSurfaceProps->GetPhysicsProperties(tr.surface.surfaceProps, NULL, NULL, &surfaceFriction, NULL);

            if (!PhysicsApplyFriction(vecVelocity, vecVelocity, surfaceFriction, gInts->Globals->interval_per_tick))
                break;
        }
    }
}
