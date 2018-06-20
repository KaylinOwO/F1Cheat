#pragma once

#include "../SDK/IHack.hh"
#include "../SDK/SDK.hh"
#include "TargetManager.hh"

#include "WeaponHelper.hh"

class Aimbot : public IHack<Aimbot>, public TargetManager<Aimbot>
{
	CEntTag      local_weapon_tag;
	tf_classes   local_class;
	WeaponHelper local_weapon_helper;

	bool has_valid_target;

	F1_ConVar<Switch>              aimbot_switch{"Aimbot", "f1_aimbot_switch", false};
	F1_ConVar<bool>                enabled{" - Enabled", "f1_aimbot_enabled", false, &aimbot_switch};
	F1_ConVar<bool>                zoom_only{" - Zoom only", "f1_aimbot_zoom_only", false, &aimbot_switch};
	F1_ConVar<bool>                use_aim_key{" - Use aimkey", "f1_aimbot_use_aimkey", false, &aimbot_switch};
	F1_BindableConVar              aim_key{" - Aim key", "f1_aimbot_aimkey", false, &aimbot_switch};
	F1_ConVar<bool>                use_silent{" - Silent", "f1_aimbot_silent", true, &aimbot_switch};
	F1_ConVar<bool>                auto_shoot{" - Autoshoot", "f1_aimbot_autoshoot", true, &aimbot_switch};
	F1_ConVar<Enum<targetHelpers>> target_system{
	    " - Target system",
	    "f1_aimbot_target_system",
	    {targetHelpers::distance,
	     {{targetHelpers::distance, "Distance"}, {targetHelpers::fov, "FOV"}}},
	    targetHelpers::distance,
	    targetHelpers::fov,
	    &aimbot_switch};
	F1_ConVar<int> fov_limit{" - Fov limit", "f1_aimbot_fov_limit", 50, 1, 360, 1, &aimbot_switch};

	bool  is_player(const CBaseEntity *ent) const;
	bool  check_cond(const CBaseEntity *ent);
	float get_rifle_damage();

	bool predict(const CBaseEntity *ent, CBaseCombatWeapon *localWeapon, Vector &localViewPos, Vector *correctionOut);

	// is this vector visible to the player
	bool visible(const CBaseEntity *ent, Vector *vector, int hitboxToCheck);

	bool do_multipoint(const CBaseEntity *ent, float gran, Vector &min, Vector &centre, Vector &max, int hitboxToCheck, Vector *out);
	bool multipoint(const CBaseEntity *ent, Vector &centre, Vector &bbmin, Vector &bbmax, int hitboxToCheck, Vector *out);

	// Get the best hitbox for this player
	bool find_hitbox(const CBaseEntity *ent, Vector *out);

public:
	// IHack functions
	void processCommand(CUserCmd *pUserCmd);
	void processCommandBeforePred(CUserCmd *command);

	// TargetManager functions
public:
	bool is_valid_target(const CBaseEntity *pBaseEntity);

	inline bool is_visible_target(const CBaseEntity *pBaseEntity, Vector &hit);
	inline bool compare_target(const CTarget &bestTarget, const CTarget &newTarget);
	inline bool is_only_players()
	{
		// TODO: change when we add object aimbot
		return true;
	}
	inline bool can_backtrack()
	{
		return enabled.Value();
	}

public:
	Aimbot()
	    : local_weapon_helper(nullptr)
	{
	}
};

extern Aimbot gAimbot;
