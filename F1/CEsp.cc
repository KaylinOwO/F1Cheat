#include "stdafx.hh"

#include "CEsp.hh"

#include "../SDK/CDrawManager.hh"

#include "CPlayerManager.hh"
#include "TargetManager.hh"

#include "LagCompensation.hh"

#include "F1_Cache.hh"

#include <tier0/memdbgon.h>

CESP gEsp;

void CESP::renderHealthBar(int x, int y, int h, int health, int maxHealth)
{
	if (renderBar.Value()) {
		int healthBar      = (float)h / (float)maxHealth * (float)health;
		int healthBarDelta = h - healthBar;

		gDrawManager.OutlineRect(x - 5, y - 1, 5, h + 2, COLOR_BLACK);
		gDrawManager.DrawRect(x - 4, y + healthBarDelta, 3, healthBar, redGreenGradiant(health, maxHealth));
	}
}

void CESP::processEntity(CBaseEntity *pBaseEntity)
{
	F1_VPROF_FUNCTION();

	// no dormants
	if (pBaseEntity->IsDormant())
		return;

	Vector vecWorld, vecScreen;
	Vector min, max, origin, localOrigin;

	pBaseEntity->GetRenderBounds(min, max);

	origin = pBaseEntity->GetAbsOrigin();

	pBaseEntity->GetWorldSpaceCenter(vecWorld);

	if (!gDrawManager.WorldToScreen(vecWorld, vecScreen))
		return;

	DWORD team = pBaseEntity->GetTeam();
	if (renderLocalTeam.Value() != true) {
		// dont render if they are on our team
		if (team == GetLocalPlayer()->GetTeam())
			return;
	}

	DWORD teamColor = COLORCODE(255, 255, 255, 255);

	DWORD playerColor = gPlayerManager.getColorForPlayer(pBaseEntity->GetIndex());
	if ((playerColor != 0xFFFFFFFF))
		teamColor = playerColor;
	else
		teamColor = gDrawManager.dwGetTeamColor(team);

	float distance = (localOrigin - origin).Length();

	// Draw on the player.

	classId id = pBaseEntity->GetClientClass()->classId;

	if (id == classId::CTFPlayer) {

		// no deads
		if (pBaseEntity->IsAlive() == false)
			return;

		player_info_t info;
		if (!gInts->Engine->GetPlayerInfo(pBaseEntity->GetIndex(), &info))
			return;

		if (hitboxes.Value() == true) {
			//for (int i = 0; i < 17; i++)
			//	FrameHitbox (pBaseEntity, i);
			DrawHitboxes(pBaseEntity);
		}

		if (renderBox.Value() != BoxType::None) {
			DynamicBox(pBaseEntity, teamColor);
		}

		if (renderGUID.Value() == true) {
			gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, COLOR_OBJ, XorString("%s"), info.guid);
			vecScreen.y += gDrawManager.GetESPHeight();
		}

		if (renderName.Value() == true) {
			// use widestring for name
			gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, L"%S", info.name);
			vecScreen.y += gDrawManager.GetESPHeight();
		}

		if (renderHealth.Value() == true) {
			gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, redGreenGradiant(pBaseEntity->GetHealth(), getMaxHealth(pBaseEntity->GetClass())), XorString("%i"), pBaseEntity->GetHealth() /*gInts->GameResource->getHealth(index)*/); // Draw on the player.
			vecScreen.y += gDrawManager.GetESPHeight();
		}

		if (renderClass.Value() == true) {
			gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor,
			                        "%s", GetTFClassName(pBaseEntity->GetClass())); // Draw on the player.
			vecScreen.y += gDrawManager.GetESPHeight();
		}

		if (renderIndex.Value() == true) {
			gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, XorString("%i"), pBaseEntity->GetIndex()); // Draw on the player.
			vecScreen.y += gDrawManager.GetESPHeight();
		}

		if (renderFov.Value() == true) {
			gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor,
			                        "%f", __CTargetHelper::GetFovFromLocalPlayer(vecWorld));
			vecScreen.y += gDrawManager.GetESPHeight();
		}

		if (renderViewESP.Value() == true) {
			// QAngle angles = pBaseEntity->GetAbsAngles();
			QAngle eyeangles = pBaseEntity->GetEyeAngles();
			Vector eyeForward;
			AngleVectors(eyeangles, &eyeForward);

			Vector eyepos = pBaseEntity->GetViewPos();
			eyeForward    = eyeForward * viewESPLength.Value() + eyepos;

			Vector screenForward, screenEyepos;
			if (gDrawManager.WorldToScreen(eyepos, screenEyepos) && gDrawManager.WorldToScreen(eyeForward, screenForward)) {
				gDrawManager.drawLine(screenForward.x, screenForward.y, screenEyepos.x, screenEyepos.y, teamColor);
			}
		}

		if (showDebugInfo.Value()) {
			gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "seq: %d", pBaseEntity->GetSequence());
			vecScreen.y += gDrawManager.GetESPHeight();
			gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "cyc: %f", pBaseEntity->GetCycle());
			vecScreen.y += gDrawManager.GetESPHeight();
			gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "pbr: %f", pBaseEntity->GetPlaybackRate());
			vecScreen.y += gDrawManager.GetESPHeight();
			gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "num: %d", pBaseEntity->GetNumAnimOverlays());
			vecScreen.y += gDrawManager.GetESPHeight();
			gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "rat: %f", pBaseEntity->GetAnimTime());
			vecScreen.y += gDrawManager.GetESPHeight();
			gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "oat: %f", pBaseEntity->GetOldAnimTime());
			vecScreen.y += gDrawManager.GetESPHeight();
			gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "rst: %f", pBaseEntity->GetSimulationTime());
			vecScreen.y += gDrawManager.GetESPHeight();
			gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "ost: %f", pBaseEntity->GetOldSimulationTime());
			vecScreen.y += gDrawManager.GetESPHeight();
		}

		// TODO: put the if(boneesp) here rather than in the function itself
		drawBoneEsp(pBaseEntity, teamColor);
	} else {
		if (renderObjectID.Value()) {
			switch (id) {
			case classId::CObjectDispenser: {
				CObjectDispenser *pDispenser = reinterpret_cast<CObjectDispenser *>(pBaseEntity);

				if (!pDispenser->GetLevel())
					return;

				if (pDispenser->IsSapped()) {
					gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "**SAPPED**");
					vecScreen.y += gDrawManager.GetESPHeight();
				}

				gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "Dispenser: %i", pDispenser->GetLevel());
				vecScreen.y += gDrawManager.GetESPHeight();

				// If it was building
				if (pDispenser->IsBuilding()) {
					gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "Building: %0.f", pDispenser->GetPercentageConstructed() * 100);
					vecScreen.y += gDrawManager.GetESPHeight();
				} else {
					// Normal state then
					gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "%i hp", pDispenser->GetHealth());
					vecScreen.y += gDrawManager.GetESPHeight();

					if (pDispenser->GetLevel() < 3) {
						gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "Upgraded: %i", pDispenser->GetLevel());
						vecScreen.y += gDrawManager.GetESPHeight();
					}

					gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "%i Metal", pDispenser->GetMetalReserve());

					vecScreen.y += gDrawManager.GetESPHeight();
				}

				if (renderBox.Value() != BoxType::None) {
					DynamicBox(pBaseEntity, teamColor, true);
				}

				break;
			}
			case classId::CObjectSapper: {
				break;
			}
			case classId::CObjectSentrygun: {
				CObjectSentryGun *pSentryGun = reinterpret_cast<CObjectSentryGun *>(pBaseEntity);

				if (!pSentryGun->GetLevel())
					return;

				if (pSentryGun->IsSapped()) {
					gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor,
					                        "**SAPPED**");
					vecScreen.y += gDrawManager.GetESPHeight();
				}

				gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor,
				                        "Sentry: %i", pSentryGun->GetLevel());
				vecScreen.y += gDrawManager.GetESPHeight();

				// If it is a building
				if (pSentryGun->IsBuilding()) {

					gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "Building: %0.f", pSentryGun->GetPercentageConstructed() * 100);
					vecScreen.y += gDrawManager.GetESPHeight();
				} else {
					// Normal state then
					gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "%i hp", pSentryGun->GetHealth());
					vecScreen.y += gDrawManager.GetESPHeight();

					if (pSentryGun->GetLevel() < 3) {
						gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "Upgraded: %i", pSentryGun->GetUpgradedMetal() / 2); // (Metal / 200) * 100 == Metal / 2
						vecScreen.y += gDrawManager.GetESPHeight();
					}

					if (pSentryGun->IsControlled())
						gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "%s", "Controlling");
					else
						gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "%s", pSentryGun->GetStateString());

					vecScreen.y += gDrawManager.GetESPHeight();

					gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "%i Ammo", pSentryGun->GetAmmo());
					vecScreen.y += gDrawManager.GetESPHeight();

					if (pSentryGun->GetLevel() == 3) {
						gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "%i Rockets", pSentryGun->GetRocket());
						vecScreen.y += gDrawManager.GetESPHeight();
					}
				}
				if (renderBox.Value() != BoxType::None) {
					DynamicBox(pBaseEntity, teamColor, true);
				}
				break;
			}
			case classId::CObjectTeleporter:
				CObjectTeleporter *pTeleporter;
				pTeleporter = reinterpret_cast<CObjectTeleporter *>(pBaseEntity);

				if (pTeleporter == NULL)
					return;

				if (!pTeleporter->GetLevel())
					return;

				if (pTeleporter->IsSapped()) {
					gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "**SAPPED**");
					vecScreen.y += gDrawManager.GetESPHeight();
				}

				gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "Teleporter: %i", pTeleporter->GetLevel());
				vecScreen.y += gDrawManager.GetESPHeight();

				// If it was building
				if (pTeleporter->IsBuilding()) {
					gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "Building: %0.f", pTeleporter->GetPercentageConstructed() * 100);
					vecScreen.y += gDrawManager.GetESPHeight();
				} else {
					// Normal state then
					gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "%i", pTeleporter->GetHealth());
					vecScreen.y += gDrawManager.GetESPHeight();

					if (pTeleporter->GetLevel() < 3) {
						gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "Upgraded: %i", pTeleporter->GetUpgradedMetal() / 2);
						vecScreen.y += gDrawManager.GetESPHeight();
					}

					gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y, teamColor, "%s", pTeleporter->GetStateString());

					vecScreen.y += gDrawManager.GetESPHeight();
				}
				if (renderBox.Value() != BoxType::None) {
					DynamicBox(pBaseEntity, teamColor, true);
				}
				break;
			default:
				break;
			}
		}

		if (showDebugInfo.Value()) {
			const char *name = gInts->ModelInfo->GetModelName(pBaseEntity->GetModel());
			if (name != nullptr && name[0] != '?') {
				gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y += gDrawManager.GetESPHeight(), COLOR_OBJ, "modelName: %s", name);
				gDrawManager.DrawString(gDrawManager.EspFont, vecScreen.x, vecScreen.y += gDrawManager.GetESPHeight(), COLOR_OBJ, "modelindex: %d", pBaseEntity->GetModelIndex());
			}
		}
	}
	return;
}

bool CESP::paint()
{
	if (renderLocalESP.Value() == true) {
		this->processEntity(GetLocalPlayer());
	}
	return false;
}

void CESP::DrawHitboxes(CBaseEntity *pBaseEntity)
{
	hitboxes_t hitboxes;
	int        total = gCache.GetHitboxes(pBaseEntity, &hitboxes);

	for (int i = 0; i < total; i++) {
		Vector &vMin = hitboxes.bbmin[i];
		Vector &vMax = hitboxes.bbmax[i];

		Vector pointList[] = {
		    Vector(vMin.x, vMin.y, vMin.z), Vector(vMin.x, vMax.y, vMin.z),
		    Vector(vMax.x, vMax.y, vMin.z), Vector(vMax.x, vMin.y, vMin.z),
		    Vector(vMax.x, vMax.y, vMax.z), Vector(vMin.x, vMax.y, vMax.z),
		    Vector(vMin.x, vMin.y, vMax.z), Vector(vMax.x, vMin.y, vMax.z)};

		gDrawManager.DrawBox(pointList, COLOR_OBJ);
	}
}

void CESP::DynamicBox(CBaseEntity *pBaseEntity, DWORD dwColor, bool object)
{
	const matrix3x4 &trans = pBaseEntity->GetRgflCoordinateFrame();

	Vector min = pBaseEntity->GetCollideableMins();
	Vector max = pBaseEntity->GetCollideableMaxs();

	Vector pointList[] = {
	    Vector(min.x, min.y, min.z), Vector(min.x, max.y, min.z),
	    Vector(max.x, max.y, min.z), Vector(max.x, min.y, min.z),
	    Vector(max.x, max.y, max.z), Vector(min.x, max.y, max.z),
	    Vector(min.x, min.y, max.z), Vector(max.x, min.y, max.z)};

	Vector transformed[8];

	for (int i = 0; i < 8; i++)
		VectorTransform(pointList[i], trans, transformed[i]);

	Vector flb, brt, blb, frt, frb, brb, blt, flt;

	if (!gDrawManager.WorldToScreen(transformed[3], flb) || !gDrawManager.WorldToScreen(transformed[0], blb) || !gDrawManager.WorldToScreen(transformed[2], frb) || !gDrawManager.WorldToScreen(transformed[6], blt) || !gDrawManager.WorldToScreen(transformed[5], brt) || !gDrawManager.WorldToScreen(transformed[4], frt) || !gDrawManager.WorldToScreen(transformed[1], brb) || !gDrawManager.WorldToScreen(transformed[7], flt))
		return;

	Vector arr[] = {flb, brt, blb, frt, frb, brb, blt, flt};

	float left   = flb.x;
	float top    = flb.y;
	float right  = flb.x;
	float bottom = flb.y;

	for (int i = 0; i < 8; i++) {
		if (left > arr[i].x) {
			left = arr[i].x;
		}
		if (top < arr[i].y) {
			top = arr[i].y;
		}
		if (right < arr[i].x) {
			right = arr[i].x;
		}
		if (bottom > arr[i].y) {
			bottom = arr[i].y;
		}
	}

	float x = left;
	float y = bottom;
	float w = right - left;
	float h = top - bottom;

	if (renderBox.Value() == BoxType::Flat) {

		gDrawManager.OutlineRect(x - 1, y - 1, w + 2, h + 1, COLOR_BLACK);
		gDrawManager.OutlineRect(x, y, w, h - 1, dwColor);
	} else if (renderBox.Value() == BoxType::Corner) {
		gDrawManager.DrawCornerBox(x - 1, y - 1, w + 2, h + 1, 3, 5, COLOR_BLACK);
		gDrawManager.DrawCornerBox(x, y, w, h - 1, 3, 5, dwColor);
	} else {
		gDrawManager.DrawBox(transformed, dwColor);
	}

	if (object == false) {
		int health = pBaseEntity->GetHealth();

		tf_classes Class     = pBaseEntity->GetClass();
		int        maxHealth = getMaxHealth(Class);

		renderHealthBar(x, y, h, health, maxHealth);
	}
}

void CESP::drawBoneEsp(CBaseEntity *pBaseEntity, DWORD color)
{
	if (renderBones.Value()) {

		static const TFHitbox leftArm[]  = {TFHitbox::hand_L, TFHitbox::lowerArm_L, TFHitbox::upperArm_L, TFHitbox::spine_2};
		static const TFHitbox rightArm[] = {TFHitbox::hand_R, TFHitbox::lowerArm_R, TFHitbox::upperArm_R, TFHitbox::spine_2};
		static const TFHitbox head[]     = {TFHitbox::head, TFHitbox::spine_2, TFHitbox::pelvis};
		static const TFHitbox leftLeg[]  = {TFHitbox::foot_L, TFHitbox::knee_L, TFHitbox::pelvis};
		static const TFHitbox rightLeg[] = {TFHitbox::foot_R, TFHitbox::knee_R, TFHitbox::pelvis};

		hitboxes_t hitboxes;
		int        totalBones = gCache.GetHitboxes(pBaseEntity, &hitboxes);

		auto drawBoneChain = [](const TFHitbox *bones, int count, hitboxes_t *hitboxes) {
			Vector startPos, endPos;
			Vector startWorld, endWorld;

			endPos = hitboxes->centre[(int)bones[0]];

			for (int i = 1; i < count; i++) {

				startPos = endPos;

				TFHitbox h = bones[i];
				endPos     = hitboxes->centre[(int)h];

				if (!gDrawManager.WorldToScreen(startPos, startWorld) || !gDrawManager.WorldToScreen(endPos, endWorld))
					continue;

				gDrawManager.drawLine(startWorld.x, startWorld.y, endWorld.x, endWorld.y, COLORCODE(255, 255, 255, 255));
			}
		};

		drawBoneChain(leftArm, 4, &hitboxes);
		drawBoneChain(rightArm, 4, &hitboxes);
		drawBoneChain(head, 3, &hitboxes);
		drawBoneChain(leftLeg, 3, &hitboxes);
		drawBoneChain(rightLeg, 3, &hitboxes);
	}
}
