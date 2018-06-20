#pragma once
#include <unordered_map>

#include "../SDK/IHack.hh"
#include "../SDK/SDK.hh"

#include "F1_ConVar.hh"

class CGlow : public IHack<CGlow>
{

	friend class CEntGlowEffect;

	F1_ConVar<Switch> glowSwitch      = F1_ConVar<Switch>("Glow", "f1_glow_switch", false);
	F1_ConVar<bool>   enabled         = F1_ConVar<bool>(" - Enabled", "f1_glow_enable", false, &glowSwitch);
	F1_ConVar<bool>   enemyOnly       = F1_ConVar<bool>(" - Enemy only", "f1_glow_enemy_only", false, &glowSwitch);
	F1_ConVar<bool>   useCustomColors = F1_ConVar<bool>(" - Use Custom Colors", "f1_glow_use_custom_colors", false, &glowSwitch);

	// F1_ConVar<int> blurAmount = F1_ConVar<int>(" - Glow blur amount", "f1_glow_blur_amount", 5, 0, 10, 1, &glowSwitch);

	F1_ConVar<int> reda = F1_ConVar<int>(" - Red Team Alpha", "f1_glow_red_a", 255, 0, 255, 1, &glowSwitch);
	F1_ConVar<int> redr = F1_ConVar<int>(" - Red Team Red", "f1_glow_red_r", 200, 0, 255, 1, &glowSwitch);
	F1_ConVar<int> redg = F1_ConVar<int>(" - Red Team Green", "f1_glow_red_g", 50, 0, 255, 1, &glowSwitch);
	F1_ConVar<int> redb = F1_ConVar<int>(" - Red Team Blue", "f1_glow_red_b", 50, 0, 255, 1, &glowSwitch);

	F1_ConVar<int> bluea = F1_ConVar<int>(" - Blue Team Alpha", "f1_glow_blue_a", 255, 0, 255, 1, &glowSwitch);
	F1_ConVar<int> bluer = F1_ConVar<int>(" - Blue Team Red", "f1_glow_blue_r", 80, 0, 255, 1, &glowSwitch);
	F1_ConVar<int> blueg = F1_ConVar<int>(" - Blue Team Green", "f1_glow_blue_g", 100, 0, 255, 1, &glowSwitch);
	F1_ConVar<int> blueb = F1_ConVar<int>(" - Blue Team Blue", "f1_glow_blue_b", 200, 0, 255, 1, &glowSwitch);

public:
	CGlow()
	{
	}

	const char *name() const;
	void        init();
	bool        paint();
	void        processEntity(CBaseEntity *pBaseEntity);
	void        menuUpdate(F1_IConVar **menuArray, int &currIndex);
};

extern CGlow gGlow;
