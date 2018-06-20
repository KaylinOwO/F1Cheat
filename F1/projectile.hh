#pragma once

#include "../SDK/SDK.hh"

#include "../SDK/Trace.hh"
#include "../SDK/Util.hh"

namespace projectileHelper {
float GetProjectileSpeed (CBaseCombatWeapon *wep);

float GetProjectileGravity (CBaseCombatWeapon *wep);

void PhysicsClipVelocity (const Vector &in, const Vector &normal, Vector &out, float overbounce);

bool PhysicsApplyFriction (const Vector &in, Vector &out, float flSurfaceFriction, float flTickRate);

void DrawDebugArrow (const Vector &vecFrom, const Vector &vecTo, const DWORD color);

float GetEntityGravity (CBaseEntity *ent);

unsigned int PhysicsSolidMaskForEntity (CBaseEntity *ent);

Vector PredictCorrection (CBaseCombatWeapon *weapon, CBaseEntity *target, Vector vecFrom, int qual);
Vector PredictCorrection2 (CBaseCombatWeapon *weapon, CBaseEntity *target, Vector vecFrom, int qual);

void PredictPath (CBaseEntity *target);
} // namespace projectileHelper
