#pragma once

#include "../SDK/IHack.hh"
#include "F1_ConVar.hh"

class CESP : public IHack<CESP>
{
	enum class BoxType
	{
		None,
		Flat,
		Corner,
		Third,
	};

	F1_ConVar<Switch>        espSwitch = F1_ConVar<Switch> ("ESP", "f1_esp_switch", false);
	F1_ConVar<Enum<BoxType>> renderBox =
	    F1_ConVar<Enum<BoxType>> ("Box",
	                              "f1_esp_boxtype",
	                              {BoxType::None,
	                               {
	                                   {BoxType::None, "None"},
	                                   {BoxType::Flat, "2D"},
	                                   {BoxType::Corner, "2D corners"},
	                                   {BoxType::Third, "3D"},
	                               }},
	                              BoxType::None,
	                              BoxType::Third,
	                              &espSwitch);
	F1_ConVar<bool> renderBones     = F1_ConVar<bool> (" - Bones", "f1_esp_bones", false, &espSwitch);
	F1_ConVar<bool> renderGUID      = F1_ConVar<bool> (" - GUID", "f1_esp_guid", false, &espSwitch);
	F1_ConVar<bool> renderName      = F1_ConVar<bool> (" - Name", "f1_esp_name", false, &espSwitch);
	F1_ConVar<bool> renderHealth    = F1_ConVar<bool> (" - Health", "f1_esp_health", false, &espSwitch);
	F1_ConVar<bool> renderBar       = F1_ConVar<bool> (" - Health Bar", "f1_esp_healthbar", false, &espSwitch);
	F1_ConVar<bool> renderClass     = F1_ConVar<bool> (" - Class", "f1_esp_class", false, &espSwitch);
	F1_ConVar<bool> renderIndex     = F1_ConVar<bool> (" - Index", "f1_esp_index", false, &espSwitch);
	F1_ConVar<bool> renderObjectID  = F1_ConVar<bool> (" - Object ESP", "f1_esp_object_esp", false, &espSwitch);
	F1_ConVar<bool> renderViewESP   = F1_ConVar<bool> (" - View", "f1_esp_view", false, &espSwitch);
	F1_ConVar<int>  viewESPLength   = F1_ConVar<int> (" - View Length", "f1_esp_view_length", 500, 100, 2000, 100, &espSwitch);
	F1_ConVar<bool> hitboxes        = F1_ConVar<bool> (" - Show hitboxes", "f1_esp_show_hitboxes", false, &espSwitch);
	F1_ConVar<bool> renderLocalTeam = F1_ConVar<bool> (" - Render local team", "f1_esp_local_team", false, &espSwitch);
	F1_ConVar<bool> showDebugInfo   = F1_ConVar<bool> (" - Show debug info", "f1_esp_debug_info", false, &espSwitch);
	F1_ConVar<bool> renderLocalESP  = F1_ConVar<bool> (" - Render local player", "f1_esp_local_player", false, &espSwitch);
	F1_ConVar<bool> renderFov       = F1_ConVar<bool> (" - Render fov from crosshair", "f1_esp_fov", false, &espSwitch);

	// creds to Roskonix for these nice esp boxes
	void DynamicBox (CBaseEntity *pBaseEntity, DWORD dwColor, bool object = false);

	void FrameHitbox (CBaseEntity *pBaseEntity, int iHitbox);
	void DrawHitboxes (CBaseEntity *pBaseEntity);

	void drawBoneEsp (CBaseEntity *pBaseEntity, DWORD color);

	void renderHealthBar (int x, int y, int h, int health, int maxHealth);

public:
	CESP ()
	{
	}

	const char *name () const;

	void processEntity (CBaseEntity *pBaseEntity);

	bool paint ();
};

extern CESP gEsp;
