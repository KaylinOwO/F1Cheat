#include "stdafx.hh"

#include "CBackstab.hh"

#include "../SDK/Trace.hh"

#include <tier0/memdbgon.h>

CBackstab gBackstab;

const char *CBackstab::name() const
{
	return "AUTO-BACKSTAB";
}

void CBackstab::processCommandBeforePred(CUserCmd *pUserCmd)
{
	// if the hack is not enabled
	if (enabled.Value() == false)
		return;

	CBaseEntity *pLocalEntity = GetLocalPlayer();

	if (pLocalEntity == nullptr)
		return;

	CBaseCombatWeapon *pBaseCombatWeapon = (CBaseCombatWeapon *)pLocalEntity->GetActiveWeapon();

	if (pBaseCombatWeapon == nullptr) // is not null
		return;

	if (pBaseCombatWeapon->GetClientClass()->classId == classId::CTFKnife) {
		if (canBackstab(pBaseCombatWeapon, pLocalEntity) != true)
			return;

		pUserCmd->buttons |= IN_ATTACK;
	} else {
		return;
	}

	return;
}

// backstab helper
bool CBackstab::canBackstab(CBaseCombatWeapon *pBaseCombatWeapon, CBaseEntity *pBaseEntity)
{
	trace_t trace;

	CTFBaseWeaponMelee *pBaseMeleeWeapon = CTFBaseWeaponMelee::FromBaseEntity(pBaseCombatWeapon);

	bool istrace = pBaseMeleeWeapon->DoSwingTrace(trace);

	CBaseEntity *traceent = (CBaseEntity *)trace.m_pEnt;

	if (istrace != true)
		return false;

	if (traceent == nullptr)
		return false;

	if (traceent->IsAlive() == false)
		return false;

	classId cid = traceent->GetClientClass()->classId;

	if (cid != classId::CTFPlayer)
		return false;

	int otherTeam = traceent->GetTeam(); // so we dont have to get the netvar every time

	if (otherTeam == pBaseEntity->GetTeam() || (otherTeam < 2 || otherTeam > 3)) // check team is not our team or invalid team
		return false;

	if (isBehind(traceent, pBaseEntity)) {
		// Log::Console("Can Backstab!");
		return true;
	}

	return false;
}

bool CBackstab::isBehind(CBaseEntity *pBaseEntity, CBaseEntity *pLocalEntity)
{
	if (pBaseEntity == nullptr)
		return false;

	if (pLocalEntity == nullptr)
		return false;

	// Get the forward view vector of the target, ignore Z
	Vector vecVictimForward;
	AngleVectors(pBaseEntity->GetPrevLocalAngles(), &vecVictimForward);
	vecVictimForward.z = 0.0f;
	vecVictimForward.NormalizeInPlace();

	// Get a vector from my origin to my targets origin
	Vector vecToTarget;
	Vector localWorldSpace;
	pLocalEntity->GetWorldSpaceCenter(localWorldSpace);
	Vector otherWorldSpace;
	pBaseEntity->GetWorldSpaceCenter(otherWorldSpace);
	vecToTarget   = otherWorldSpace - localWorldSpace;
	vecToTarget.z = 0.0f;
	vecToTarget.NormalizeInPlace();

	// Get a forward vector of the attacker.
	Vector vecOwnerForward;
	AngleVectors(pLocalEntity->GetPrevLocalAngles(), &vecOwnerForward);
	vecOwnerForward.z = 0.0f;
	vecOwnerForward.NormalizeInPlace();

	float flDotOwner  = vecOwnerForward.Dot(vecToTarget);
	float flDotVictim = vecVictimForward.Dot(vecToTarget);

	// Make sure they're actually facing the target.
	// This needs to be done because lag compensation can place target slightly behind the attacker.
	if (flDotOwner > 0.5)
		return (flDotVictim > -0.1);

	return false;
}
