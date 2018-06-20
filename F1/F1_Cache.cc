#include "stdafx.hh"

#include "F1_Cache.hh"

#include "SetupBones.hh"

#include "LagCompensation.hh"

#include <tier0/memdbgon.h>

F1_GlobalCache gCache;

F1_GlobalCache::F1_GlobalCache()
{
}

void F1_GlobalCache::blow()
{
	for (auto &c : predCache) {
		c.isValid = false;
	}
}

#include "CHack.hh"

ConVar f1_setupusetfsetupbones("f1_setupusetfsetupbones", "0", FCVAR_NONE, "Use the tf2 setupbones instead of mine");
ConVar f1_setupusesimtimediff("f1_setupusesimtimediff", "0", FCVAR_NONE, "Use the sim time diff in SetupBones");

// this really needs to be changed to return a bool and fill out a Vector * instead
Vector F1_GlobalCache::getHitboxPosition(CBaseEntity *pBaseEntity, TFHitbox hb, bool recalc)
{

	Vector centre;
	Vector vecOrigin;

	auto model = pBaseEntity->GetModel();

	if (model == nullptr)
		goto exit;

	auto pStudioHdr = gInts->ModelInfo->GetStudioModel(model);

	if (!pStudioHdr)
		goto exit;

	// request for an invalid hitbox so die!
	if ((int)hb > GetHitboxset(pStudioHdr)->numhitboxes)
		return vec3_invalid;

	mstudiobbox_t *box = nullptr;

	box = GetHitbox((int)hb, pStudioHdr);

	if (box == nullptr)
		return vec3_invalid;

	// remove conds that break hitbox alignment
	bool wasDisguised = false;
	if (pBaseEntity->GetClass() == tf_classes::TF2_Spy) {
		int &cond = pBaseEntity->GetCond();
		if (cond & tf_cond::TFCond_Disguised) {
			// TODO: you need to call UpdateModel() here to cause this to change effect
			// calls SetModel(modelname)
			cond &= ~tf_cond::TFCond_Disguised;
			wasDisguised = true;
		}
	}

	float lerp       = GetInterpolationAmount();
	float actualDiff = lerp - TICKS_TO_TIME(TIME_TO_TICKS(lerp));
	float simDiff    = pBaseEntity->GetSimulationTime() - gInts->Globals->curtime;

	// force copy instead of ref
	float m_flCycle = float(pBaseEntity->GetCycle());

	if (f1_setupusesimtimediff.GetBool()) {
		pBaseEntity->SetCycle(fmod(10.0f + m_flCycle + pBaseEntity->GetPlaybackRate() * simDiff, 1.0f));
	}

	{
		matrix3x4_t BoneToWorld[MAXSTUDIOBONES];

		// Always run custom setupbones
		if (f1_setupusetfsetupbones.GetBool()) {
			bool bSetup = pBaseEntity->SetupBones(BoneToWorld, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, 0.0f);
		} else {

			CSetupBonesEntity setupBonesEnt(pBaseEntity);
			if (setupBonesEnt.SetupBones(BoneToWorld, MAXSTUDIOBONES, 0x100) == false) {
				// this failed so we will fail with it
				goto exit;
			}

			if (box->bone > setupBonesEnt.numBonesSetup)
				goto exit;
		}

		// this happens sometime apparently
		if (box->bone < 0)
			goto exit;

		{
			QAngle angRotation;
			MatrixAngles(BoneToWorld[box->bone], angRotation, vecOrigin);

			matrix3x4_t fRotateMatrix;
			AngleMatrix(angRotation, fRotateMatrix);

			Vector MinRotated, MaxRotated;
			VectorRotate(box->bbmin, fRotateMatrix, MinRotated);
			VectorRotate(box->bbmax, fRotateMatrix, MaxRotated);

			centre = VectorLerp(MinRotated, MaxRotated, 0.5f);
		}
	}

exit: // gotos are bad but this helps us do what we want without being too bad

	if (wasDisguised) {
		pBaseEntity->SetCond(pBaseEntity->GetCond() | tf_cond::TFCond_Disguised);
		wasDisguised = false;
	}

	if (f1_setupusesimtimediff.GetBool()) {
		pBaseEntity->GetCycle() = m_flCycle;
	}

	return centre + vecOrigin;
}

int F1_GlobalCache::GetHitboxes(CBaseEntity *ent, hitboxes_t *out, int totalOut, bool createPose)
{
	auto model = ent->GetModel();

	if (model == nullptr)
		return false;

	auto pStudioHdr = gInts->ModelInfo->GetStudioModel(model);

	if (pStudioHdr == nullptr)
		return false;

	// TODO: perform model changing actions here

	int  restore = 0;
	int &cond    = ent->GetCond();

	if (cond & tf_cond::TFCond_Disguised) {
		// TODO: you need to call UpdateModel() here to cause this to change effect
		// calls SetModel(modelname)
		cond &= ~tf_cond::TFCond_Disguised;
		restore |= tf_cond::TFCond_Disguised;
	}

	int totalHitboxes = GetHitboxes(ent, pStudioHdr, out, totalOut);

	// TODO: perform model changing actions revert here

	if (restore != 0) {
		cond |= restore; // re-set those cond bits
	}

	return totalHitboxes;
}

int F1_GlobalCache::GetHitboxes(CBaseEntity *ent, studiohdr_t *hdr, hitboxes_t *out, int totalOut, bool createPose)
{
	matrix3x4_t boneToWorld[MAXSTUDIOBONES];

	// Get the bone matrix
	if (f1_setupusetfsetupbones.GetBool()) {
		if (ent->SetupBones(boneToWorld, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, gInts->Globals->curtime) == false) {
			return 0;
		}
	} else {

		CSetupBonesEntity setupBonesEnt(ent);
		setupBonesEnt.shouldCreatePose = createPose;
		if (setupBonesEnt.SetupBones(boneToWorld, MAXSTUDIOBONES, 0x100) == false) {
			// this failed so we will fail with it
			return 0;
		}
	}

	Vector centre;
	Vector origin;

	int numHitboxes = GetHitboxset(hdr)->numhitboxes;

	// warn if there is not enough space for all the hitboxes
	if (totalOut < numHitboxes) {
		Log::Console("F1_GlobalCache::GetHitboxes Not enough output space for all hitboxes (%d out vs %d hitboxes)", totalOut, numHitboxes);
	}

	// used incase any of the boxes are null we can still pack them in properly
	int outDiff = 0;

	// never iterate invalid hitboxes
	int total = Min(totalOut, numHitboxes);

	for (int i = 0; i < total; i++) {
		// TODO: this is a sanity check and can probably be removed
		mstudiobbox_t *box = nullptr;

		box = GetHitbox(i, hdr);

		if (box == nullptr) {
			outDiff -= 1;
			Log::Console("F1_GlobalCache::GetHitboxes box == nullptr!");
			continue;
		}

		QAngle rotation;
		MatrixAngles(boneToWorld[box->bone], rotation, origin);

		matrix3x4_t rotateMatrix;
		AngleMatrix(rotation, rotateMatrix);

		Vector minRotated, maxRotated;
		VectorRotate(box->bbmin, rotateMatrix, minRotated);
		VectorRotate(box->bbmax, rotateMatrix, maxRotated);

		centre = VectorLerp(minRotated, maxRotated, 0.5f);

		// account for difference to allow proper packing
		int index = i + outDiff;

		out->centre[index] = (origin + centre);
		out->bbmin[index]  = origin + minRotated;
		out->bbmax[index]  = origin + maxRotated;
	}

	return total;
}

F1_PredictionCache &F1_GlobalCache::getPrediction(int index)
{
	return predCache[index];
}

void F1_GlobalCache::storePrediction(int index, Vector pred)
{
	F1_PredictionCache t;
	t.isValid        = true;
	t.prediction     = pred;
	predCache[index] = t;
}

// DEFINED HERE FOR UTIL.CPP
void GetBoneTransform(int index, int iBone, matrix3x4_t &pBoneToWorld)
{
	matrix3x4_t       BoneToWorld[MAXSTUDIOBONES];
	CBaseEntity *     pBaseEntity = GetBaseEntity(index);
	CSetupBonesEntity setupBonesEnt(pBaseEntity);

	float lerp       = GetInterpolationAmount();
	float actualDiff = lerp - TICKS_TO_TIME(TIME_TO_TICKS(lerp));
	float simDiff    = pBaseEntity->GetSimulationTime() - gInts->Globals->curtime;

	// force copy instead of ref
	float m_flCycle = float(pBaseEntity->GetCycle());

	if (f1_setupusesimtimediff.GetBool()) {
		pBaseEntity->GetCycle() = fmod(10.0f + m_flCycle + pBaseEntity->GetPlaybackRate() * simDiff, 1.0f);
	}

	if (setupBonesEnt.SetupBones(BoneToWorld, MAXSTUDIOBONES, 0x100) == false) { /*do nothing thanks*/
	}

	if (f1_setupusesimtimediff.GetBool()) {
		pBaseEntity->GetCycle() = m_flCycle;
	}

	pBoneToWorld = BoneToWorld[iBone];
}

Vector F1_GetBoneTransform(CBaseEntity *ent, int iBone)
{
	return gCache.getHitboxPosition(ent, (TFHitbox)iBone, true);
}
