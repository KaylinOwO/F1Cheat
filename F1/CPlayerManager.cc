#include "stdafx.hh"

#include "CPlayerManager.hh"

#include <tier0/memdbgon.h>

CPlayerManager gPlayerManager;

CPlayerManager::CPlayerManager()
{
	playerModeSwitch.performLayoutFunc = [this]() { return this->PerformLayout(); };
}

CON_COMMAND(f1_playerlist_reset, "reset the playerlist")
{
	gPlayerManager.registerAllPlayers();
}

void CPlayerManager::init()
{
	gInts->EventManager->AddListener(this, "player_disconnect", false);
	gInts->EventManager->AddListener(this, "player_connect_client", true);
	gInts->EventManager->AddListener(this, "teamplay_round_start", false);
}

DWORD CPlayerManager::getColorForPlayer(int index)
{

	// this is SLOW! TODO: we need to bake this in somewhere
	auto info = gInts->Engine->GetPlayerInfo(index);

	if (playerArray[info.userID].isTarget == true) {
		return COLORCODE(170, 0, 255, 255);
	} else {
		auto *mode = playerArray[info.userID].mode;
		if (mode != nullptr) {
			switch (mode->Value()) {
			case playerMode::Normal:
				return 0xFFFFFFFF;
				break;
			case playerMode::Friend:
				return 0x00FF32FF;
				break;
			case playerMode::Rage:
				return 0xFF0000FF;
				break;
			}
		}
	}

	// return white by default (-1)
	return 0xFFFFFFFF;
}

CPlayerManager::playerMode CPlayerManager::getModeForPlayer(int index)
{
	F1_VPROF_FUNCTION();

	if (index > gInts->Engine->GetMaxClients())
		return playerMode::Normal;

	player_info_t info;
	if (gInts->Engine->GetPlayerInfo(index, &info)) {
		auto *mode = playerArray[info.userID].mode;
		if (mode != nullptr)
			return mode->Value();
		else
			return playerMode::Normal;
	}

	return playerMode::Normal;
}

CPlayerManager::playerState *CPlayerManager::getPlayer(int index)
{
	if (index > gInts->Engine->GetMaxClients())
		return nullptr;

	player_info_t info;
	if (gInts->Engine->GetPlayerInfo(index, &info)) {
		auto *state = &playerArray[info.userID];
		return state;
	}
	return nullptr;
}

bool CPlayerManager::paint()
{
	if (gInts->Engine->IsConnected()) {
		if (!isInGame) {
			registerAllPlayers();
			isInGame = true;
		}
	} else {
		isInGame = false;
	}

	return false;
}

void CPlayerManager::setTarget(int index)
{
	if (index != -1 && index > 1) {
		player_info_t info;

		if (gInts->Engine->GetPlayerInfo(index, &info)) {
			if (lastTargetIndex != -1) {
				playerArray[lastTargetIndex].isTarget = false;
			}

			playerArray[info.userID].isTarget = true;

			lastTargetIndex = info.userID;
		}
	} else {
		playerArray[lastTargetIndex].isTarget = false;
		lastTargetIndex                       = -1;
	}
}

std::vector<CPlayerManager::playerState *> CPlayerManager::getPlayersWithMode(playerMode mode)
{
	std::vector<playerState *> v;

	for (auto &p : playerArray) {
		auto *&modeCvar = p.second.mode;
		if (modeCvar != nullptr)
			if (modeCvar->Value() == mode)
				v.push_back(&p.second);
	}

	return v;
}

void CPlayerManager::FireGameEvent(IGameEvent *event)
{

	if (!strcmp(event->GetName(), "player_disconnect")) {
		deregisterPlayerEvent(event);
	} else if (!strcmp(event->GetName(), "player_connect_client")) {
		registerPlayerEvent(event);
	} else if (!strcmp(event->GetName(), "teamplay_round_start")) {
		Log::Console("Round Start - Reset Player list");
		registerAllPlayers();
	}
}

void CPlayerManager::registerPlayerUserId(int uid, const char *name)
{
	char newName[35] = " - ";

	strcat_s(newName, name);

	char f1_internal_name_mode[35]   = "f1_playerlist_player_";
	char f1_internal_name_angles[35] = "f1_playerlist_player_";
	char f1_internal_name_pitch[35]  = "f1_playerlist_player_";
	char f1_internal_name_yaw[35]    = "f1_playerlist_player_";
	char f1_internal_name_force[35]  = "f1_playerlist_player_";

	char playerid[35];

	itoa(uid, playerid, 10);

	strcat_s(f1_internal_name_mode, playerid);
	strcat_s(f1_internal_name_mode, "_mode");

	strcat_s(f1_internal_name_angles, playerid);
	strcat_s(f1_internal_name_angles, "_angles");

	strcat_s(f1_internal_name_pitch, playerid);
	strcat_s(f1_internal_name_pitch, "_pitch");

	strcat_s(f1_internal_name_yaw, playerid);
	strcat_s(f1_internal_name_yaw, "_yaw");

	strcat_s(f1_internal_name_force, playerid);
	strcat_s(f1_internal_name_force, "_force");

	auto &p = playerArray[uid];

	if (p.mode == nullptr)
		p.mode = new F1_ConVar<Enum<playerMode>>(newName, f1_internal_name_mode, playerStateEnum, playerMode::Normal, playerMode::Rage, &playerModeSwitch);

	if (p.angles == nullptr)
		p.angles = new F1_ConVar<Enum<anglesMode>>(newName, f1_internal_name_angles, playerAnglesEnum, anglesMode::Real, anglesMode::Manual, &playerModeSwitch);

	if (p.manualPitchCorrection == nullptr)
		p.manualPitchCorrection = new F1_ConVar<float>(newName, f1_internal_name_pitch, 0, 0, 0, 0, &playerModeSwitch);

	if (p.manualYawCorrection == nullptr)
		p.manualYawCorrection = new F1_ConVar<float>(newName, f1_internal_name_yaw, 0, 0, 0, 0, &playerModeSwitch);

	if (p.addOrForce == nullptr)
		p.addOrForce = new F1_ConVar<bool>(newName, f1_internal_name_force, false, &playerModeSwitch);

	p.uid = uid;
	p.pid = gInts->Engine->GetPlayerForUserID(uid);

	p.isValid = true;
}

void CPlayerManager::registerPlayerEvent(IGameEvent *event)
{
	Log::Console("Player Register");

	auto name = event->GetString("name", "<null>");
	auto uid  = event->GetInt("userid");

	registerPlayerUserId(uid, name);
}

void CPlayerManager::registerPlayerEntity(CBaseEntity *entity)
{
	Log::Console("Player Register");

	player_info_t playerInfo = gInts->Engine->GetPlayerInfo(entity->GetIndex());

	registerPlayerUserId(playerInfo.userID, playerInfo.name);
}

void CPlayerManager::deregisterPlayerEvent(IGameEvent *event, int uid)
{
	Log::Console("Player Deregister");

	uid = uid ? uid : event->GetInt("userid");

	auto &p = playerArray[uid];

	// playerArray[ uid ].mode;
	p.isValid = false;
	p.uid     = -1;
	p.pid     = -1;

	// cleanup convar
	delete p.mode;
	p.mode = nullptr;

	delete p.angles;
	p.angles = nullptr;

	delete p.manualPitchCorrection;
	p.manualPitchCorrection = nullptr;

	delete p.manualYawCorrection;
	p.manualYawCorrection = nullptr;

	delete p.addOrForce;
	p.addOrForce = nullptr;
}

void CPlayerManager::deregisterAllPlayers()
{
	playerArray.clear();
}

void CPlayerManager::registerAllPlayers()
{
	// deregisterAllPlayers();
	int localPlayerIndex = gInts->Engine->GetLocalPlayerIndex();
	for (int i = 1; i < gInts->Globals->maxclients; i++) {
		auto *player = GetBaseEntity(i);

		if (player == NULL)
			continue;
		if (player->GetIndex() == localPlayerIndex)
			continue;
		if (player->GetClientClass()->classId == classId::CTFPlayer) {
			registerPlayerEntity(player);
		}
	}
}

void CPlayerManager::deregisterIfGone(int index)
{
	player_info_t playerInfo;

	if (!gInts->Engine->GetPlayerInfo(index, &playerInfo)) {
		deregisterPlayerEvent(nullptr, playerInfo.userID);
	}
}

#include <vgui_controls/ListPanel.h>

using namespace vgui;

class PlayerManagerListPanel : public ListPanel
{
	DECLARE_CLASS_SIMPLE(PlayerManagerListPanel, ListPanel);

	Menu *pModeMenu;
	Menu *pAngleMenu;
	Menu *pPitchMenu;
	Menu *pYawMenu;
	Menu *pForceMenu;

public:
	PlayerManagerListPanel(Panel *parent, const char *name)
	    : BaseClass(parent, name)
	{
		ConstructCascade();
	}

	void ConstructCascade()
	{
		AddActionSignalTarget(this);
		pModeMenu = new Menu(this, "ModeMenu");
		for (auto &translation : gPlayerManager.playerStateEnum.translations) {
			auto *kv = new KeyValues("updateValue", "Mode", (int)translation.v);
			pModeMenu->AddMenuItem(translation.t, translation.t, "updateValue", this, kv);
			kv->deleteThis();
		}
		pAngleMenu = new Menu(this, "AngleMenu");
		for (auto &translation : gPlayerManager.playerAnglesEnum.translations) {
			auto *kv = new KeyValues("updateValue", "Angles", (int)translation.v);
			pAngleMenu->AddMenuItem(translation.t, translation.t, "updateValue", this, kv);
			kv->deleteThis();
		}
		pYawMenu = new Menu(this, "YawMenu");
		for (int i = -180; i <= 180; i += 10) {
			auto kv = new KeyValues("updateValue", "Yaw", i);
			pYawMenu->AddMenuItem(std::to_string(i).c_str(), std::to_string(i).c_str(), "updateValue", this, kv);
			kv->deleteThis();
		}
		pPitchMenu = new Menu(this, "PitchMenu");
		for (int i = -180; i <= 180; i += 10) {
			auto kv = new KeyValues("updateValue", "Pitch", i);
			pPitchMenu->AddMenuItem(std::to_string(i).c_str(), std::to_string(i).c_str(), "updateValue", this, kv);
			kv->deleteThis();
		}
		pForceMenu = new Menu(this, "ForceMenu");
		auto kv    = new KeyValues("updateValue", "AddForce", 0);
		pForceMenu->AddMenuItem("Force", "Force", "updateValue", this, kv);
		kv->deleteThis();
		kv = new KeyValues("updateValue", "AddForce", 1);
		pForceMenu->AddMenuItem("Add", "Add", "updateValue", this, kv);
		kv->deleteThis();
	}

	MESSAGE_FUNC_INT(OnOpenContextMenu, "OpenContextMenu", itemID)
	{
		if (hMenu.Get() != nullptr) {
			delete hMenu.Get();
		}

		hMenu = new Menu(this, "contextMenu");

		ConstructCascade();

		hMenu->AddCascadingMenuItem("Target mode", this, pModeMenu);
		hMenu->AddCascadingMenuItem("Angle mode", this, pAngleMenu);
		hMenu->AddCascadingMenuItem("Yaw delta", this, pYawMenu);
		hMenu->AddCascadingMenuItem("Pitch delta", this, pPitchMenu);
		hMenu->AddCascadingMenuItem("Add or force", this, pForceMenu);

		int x, y;
		gInts->Surface->SurfaceGetCursorPos(x, y);

		hMenu->SetPos(x, y);
		hMenu->SetVisible(true);
	}

	void OnCommand(const char *pCommand) override
	{
		if (strcmp(pCommand, "updateValue") == 0) {
			if (auto menu = hMenu.Get()) {
				auto *menukv   = menu->GetItemUserData(menu->GetCurrentlyHighlightedItem());
				auto  newMode  = menukv->GetInt("Mode", -1);
				auto  newAngle = menukv->GetInt("Angles", -1);
				auto  newYaw   = menukv->GetInt("Yaw", -1);
				auto  newPitch = menukv->GetInt("Pitch", -1);
				auto  newForce = menukv->GetInt("AddForce", -1);

				auto *rowkv = GetItem(GetSelectedItem(0));

				if (rowkv != nullptr) {
					auto uid = rowkv->GetInt("UserID", -1);

					if (uid != -1) {
						auto &player = gPlayerManager.playerArray[uid];
						if (player.isValid) {
							if (newMode != -1 && player.mode != nullptr) {
								player.mode->set(newMode);
								rowkv->SetString("Mode", player.mode->print());
							}

							if (newAngle != -1 && player.angles != nullptr) {
								player.angles->set(newAngle);
								rowkv->SetString("Angles", player.angles->print());
							}

							if (newYaw != -1 && player.manualYawCorrection != nullptr) {
								player.manualYawCorrection->set(newYaw);
								rowkv->SetString("Yaw", player.manualYawCorrection->print());
							}

							if (newPitch != -1 && player.manualPitchCorrection != nullptr) {
								player.manualPitchCorrection->set(newPitch);
								rowkv->SetString("Pitch", player.manualPitchCorrection->print());
							}

							if (newForce != -1 && player.addOrForce != nullptr) {
								player.addOrForce->SetValue(newForce);
								rowkv->SetString("Force", player.addOrForce->print());
							}

							ApplyItemChanges(GetSelectedItem(0));
						}
					}
				}
			}
		}

		BaseClass::OnCommand(pCommand);
	}

	DHANDLE<Menu> hMenu;
};

void CPlayerManager::PerformLayout()
{

	auto switchPanel = playerModeSwitch.getPanel();
	int  panelw, panelh;
	switchPanel->GetSize(panelw, panelh);
	int cury = 4;

	if (performlayoutoneTime) {
		performlayoutoneTime = false;

		if (listPanel != nullptr) {
			delete listPanel;
			listPanel = nullptr;
		}

		listPanel = new PlayerManagerListPanel(switchPanel, "list");
		listPanel->AddColumnHeader(0, "UserID", "UserID", 10, ListPanel::COLUMN_RESIZEWITHWINDOW);
		listPanel->AddColumnHeader(1, "Name", "Name", 200, ListPanel::COLUMN_RESIZEWITHWINDOW);
		listPanel->AddColumnHeader(2, "Mode", "Mode", 100, ListPanel::COLUMN_RESIZEWITHWINDOW);
		listPanel->AddColumnHeader(3, "Angles", "Angles", 100, ListPanel::COLUMN_RESIZEWITHWINDOW);
		listPanel->AddColumnHeader(4, "Yaw", "Yaw", 100, ListPanel::COLUMN_RESIZEWITHWINDOW);
		listPanel->AddColumnHeader(5, "Pitch", "Pitch", 100, ListPanel::COLUMN_RESIZEWITHWINDOW);
		listPanel->AddColumnHeader(6, "Force", "Force", 100, ListPanel::COLUMN_RESIZEWITHWINDOW);
	}

	listPanel->SetSize(panelw - 8, panelh - 24);
	listPanel->SetPos(4, 4);

	for (auto &p : playerArray) {
		if (p.second.mode != nullptr && p.second.angles != nullptr && p.second.isValid) {

			if (listPanel->GetItem(p.second.angles->InternalName()) != -1)
				continue; // already in list

			auto *kv = new KeyValues(p.second.angles->InternalName());

			kv->SetInt("UserID", p.second.uid);
			kv->SetString("Name", p.second.mode->name());
			kv->SetString("Mode", p.second.mode->print());
			kv->SetString("Angles", p.second.angles->print());
			kv->SetString("Yaw", p.second.manualYawCorrection->print());
			kv->SetString("Pitch", p.second.manualPitchCorrection->print());
			kv->SetString("Force", p.second.addOrForce->print());

			listPanel->AddItem(kv, 0, false, true);

			// int w = 0, h = 0;

			// p.second.mode->getPanel()->GetSize(w, h);

			// p.second.mode->getPanel()->SetParent(switchPanel);
			// p.second.mode->getPanel()->SetPos(4, cury += 4);
			// p.second.mode->getPanel()->SetSize(panelw, h);
			// cury += h;

			// p.second.angles->getPanel()->GetSize(w, h);

			// p.second.angles->getPanel()->SetParent(switchPanel);
			// p.second.angles->getPanel()->SetPos(4, cury += 4);
			// p.second.angles->getPanel()->SetSize(panelw, h);
			// cury += h;
			kv->deleteThis();
		}
	}
}
