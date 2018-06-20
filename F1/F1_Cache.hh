#pragma once

#include <array>
#include <unordered_map>
#include <vector>

#include "../SDK/SDK.hh"

#include "../SDK/Util.hh"

#include "TargetManager.hh"

struct F1_BaseCache
{
	bool isValid = false;
};
struct F1_HitboxCache : public F1_BaseCache
{
	Vector position;
};
struct F1_BonesCache : public F1_BaseCache
{
	matrix3x4 BoneToWorld[128];
};
struct F1_PredictionCache : public F1_BaseCache
{
	Vector prediction;
};

template <>
struct std::hash<TFHitbox>
{
	size_t operator() (const TFHitbox &c) const
	{
		// we dont need to hash this, each index is unique anyway
		return (size_t)c;
	}
};

struct hitboxes_t
{
	Vector centre[MAXSTUDIOBONES];

	// these are the actual points in space
	Vector bbmin[MAXSTUDIOBONES];
	Vector bbmax[MAXSTUDIOBONES];
};

// TODO: this is no longer a cache as it doesnt cache anything.
// the name of this class should be changed and it should probably just be turned into
// a namespace becuase it doesnt require any state
// only becoming a collection of helper functions
// ------------
// that being said it does store the predictions of projectile prediction
// although that could probably just be stored in aimbot instead of over here
// ------------
// it is more likely that hitbox operations should just be moved over into a different class
class F1_GlobalCache
{
	std::array<F1_PredictionCache, 33> predCache;

public:
	F1_GlobalCache ();

	void blow ();

	Vector getHitboxPosition (CBaseEntity *ent, TFHitbox hb, bool recalc = false);

	// returns the total number of hitboxes or 0 on failure
	int GetHitboxes (CBaseEntity *ent, hitboxes_t *out, int totalOut = MAXSTUDIOBONES, bool createPose = false);
	int GetHitboxes (CBaseEntity *ent, studiohdr_t *hdr, hitboxes_t *out, int totalOut, bool createPose = false);

	F1_PredictionCache &getPrediction (int index);
	void                storePrediction (int index, Vector pred);
};

extern F1_GlobalCache gCache;
