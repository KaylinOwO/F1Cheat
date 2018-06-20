#include "stdafx.hh"

#include "CGlow.hh"

#include "../SDK/CDrawManager.hh"
#include "Color.h"

#include <tier0/memdbgon.h>

CGlow gGlow;

// team 3: 0.490000 0.660000 0.770000 blue
// team 2: 0.740000 0.230000 0.230000 red

void CGlow::init()
{
	return;
}

bool CGlow::paint()
{
	// return true;
	return false;
}

#include "F1_Glow.hh"

void CGlow::processEntity(CBaseEntity *pBaseEntity)
{

	// no dormants or deads - included check for if glow is disabled here since we have to tell the engine to stop the glow rather than just ending the loop
	if (pBaseEntity->IsDormant() || !enabled.Value() || (enemyOnly.Value() && (GetLocalPlayer()->GetTeam() == pBaseEntity->GetTeam()) || pBaseEntity->IsAlive() == false)) {

		// notify effect
		F1_GetGlowEffect()->DeregisterEnt(pBaseEntity);
		return;
	}

	classId id     = pBaseEntity->GetClientClass()->classId;
	int     mi     = pBaseEntity->GetModelIndex();
	auto    entTag = CEntTag{pBaseEntity};

	if (id == classId::CTFPlayer) {
		F1_GetGlowEffect()->RegisterEnt(pBaseEntity);

		if (useCustomColors.Value() == false) {
			DWORD healthColor = redGreenGradiant(pBaseEntity->GetHealth(), getMaxHealth(pBaseEntity->GetClass()));
			F1_GetGlowEffect()->SetEntColor(CHandle<CBaseEntity>(pBaseEntity), Color(RED(healthColor), GREEN(healthColor), BLUE(healthColor), 255));
		} else {
			auto team = pBaseEntity->GetTeam();
			if (team == 2) // red
			{
				F1_GetGlowEffect()->SetEntColor(CHandle<CBaseEntity>(pBaseEntity), Color(redr.Value(), redg.Value(), redb.Value(), reda.Value()));
			} else if (team == 3) // blue
			{
				F1_GetGlowEffect()->SetEntColor(CHandle<CBaseEntity>(pBaseEntity), Color(bluer.Value(), blueg.Value(), blueb.Value(), bluea.Value()));
			}
		}

		F1_GetGlowEffect()->SetEntGlowScale(EHANDLE(pBaseEntity), 1.0f);

		// pBaseCharacter->UpdateGlowEffect();
	} else if (entTag.isBuilding() || mi == 914) {
		F1_GetGlowEffect()->RegisterEnt(pBaseEntity);
		auto team = pBaseEntity->GetTeam();
		if (team == 2) // red
		{
			F1_GetGlowEffect()->SetEntColor(CHandle<CBaseEntity>(pBaseEntity), Color(redr.Value(), redg.Value(), redb.Value(), reda.Value()));
		} else if (team == 3) // blue
		{
			F1_GetGlowEffect()->SetEntColor(CHandle<CBaseEntity>(pBaseEntity), Color(bluer.Value(), blueg.Value(), blueb.Value(), bluea.Value()));
		}

		F1_GetGlowEffect()->SetEntGlowScale(EHANDLE(pBaseEntity), 1.0f);
	}

	// these need to be gotten dynamically

	/*ammo small 925
	ammo med   875
	ammo large 880
	medkit sm  574
	medkit med 624
	medkit lar 888
	cap point  914
	*/

	/*

	else if (mi == 925 || mi == 880 || mi == 779 || mi == 780) // ammo
	{
		F1_GetGlowEffect ()->RegisterEnt (pBaseEntity);
		F1_GetGlowEffect ()->SetEntColor (EHANDLE (pBaseEntity), Color (200, 200, 200, 255));
		F1_GetGlowEffect ()->SetEntGlowScale (EHANDLE (pBaseEntity), 1.0f);
	} else if (mi == 574 || mi == 624 || mi == 888 || mi == 537 || mi == 890 || mi == 886) // health
	{
		F1_GetGlowEffect ()->RegisterEnt (pBaseEntity);
		F1_GetGlowEffect ()->SetEntColor (EHANDLE (pBaseEntity), Color (0, 255, 0, 255));
		F1_GetGlowEffect ()->SetEntGlowScale (EHANDLE (pBaseEntity), 1.0f);
	*/
	if (id == classId::CTFDroppedWeapon) {
		F1_GetGlowEffect()->RegisterEnt(pBaseEntity);
		F1_GetGlowEffect()->SetEntColor(EHANDLE(pBaseEntity), Color(255, 255, 255, 255));
		F1_GetGlowEffect()->SetEntGlowScale(EHANDLE(pBaseEntity), 1.0f);
	} else if (entTag.isWeap()) {
		if (auto owner = pBaseEntity->GetOwner()) {
			if (owner->IsDormant() == false && owner->GetIndex() != gInts->Engine->GetLocalPlayerIndex()) {
				F1_GetGlowEffect()->RegisterEnt(pBaseEntity);
				F1_GetGlowEffect()->SetEntColor(EHANDLE(pBaseEntity), Color(255, 255, 255, 255));
				F1_GetGlowEffect()->SetEntGlowScale(EHANDLE(pBaseEntity), 1.0f);
			}
		}
	}

	return;
}
