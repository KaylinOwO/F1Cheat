#include "stdafx.hh"

#include "../SDK/CDrawManager.hh"
#include "../SDK/SDK.hh"
#include "Panels.hh"

#include "CHack.hh"
#include "modules.hh"

#include <tier0/memdbgon.h>

unsigned int vguiMatSystemTopPanel = 0;
unsigned int vguiFocusOverlayPanel = 0;
unsigned int vguiScopePanel        = 0;

CON_COMMAND(f1_build_date, "Gets the date of this build")
{
	Log::Console("Build: " __DATE__ " " __TIME__);
}

DEFINE_RECURSE_CALL_FUNCTION_NO_ARGS(_paint);
DEFINE_RECURSE_CALL_FUNCTION_1_ARG(_processEntity, CBaseEntity *);
DEFINE_RECURSE_CALL_FUNCTION_NO_ARGS(_everyFrame);

#if 1
char discordUserId[] = "DISCORDUSERID: 0000000000000000";
#else
char discordUserId[] = "DISCORDUSERID: 000000000000000000";
#endif

bool ranMatSystemPanel = false;

//===================================================================================
void __fastcall CHack::Hooked_PaintTraverse(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{

	gInts->Surface->SetCursorAlwaysVisible(CUtil::cursorVisible);

	if (vguiMatSystemTopPanel == 0) // check to see if we found the panel
	{
		const char *szName = gInts->Panels->GetName(vguiPanel);
		if (strcmp(szName, "MatSystemTopPanel") == 0) {
			vguiMatSystemTopPanel = vguiPanel;
			while (gHack.canIntro == false) {
				// do nothing
				Sleep(100);
			}
			gHack.intro();
		}
	}

	if (vguiFocusOverlayPanel == 0) // check to see if we found the panel
	{
		const char *szName = gInts->Panels->GetName(vguiPanel);
		if (strcmp(szName, "FocusOverlayPanel")) {
			vguiFocusOverlayPanel = vguiPanel;
			vguiFocusOverlayPanel = gInts->EngineVGUI->GetPanel(VGuiPanel_t::PANEL_ROOT);
		}
	}

	if (vguiScopePanel == 0) {
		const char *szName = gInts->Panels->GetName(vguiPanel);

		if (!strcmp("HudScope", szName)) {
			vguiScopePanel = vguiPanel;
		}
	}

	if (vguiScopePanel == 0 || (vguiScopePanel != vguiPanel || !gMisc.noZoom.Value())) {
		// Get a pointer to the instance of your VMTManager with the
		// function GetHook.
		VMTManager &hook = VMTManager::GetHook(pPanels);
		hook.GetMethod<void(__thiscall *)(PVOID, unsigned int, bool, bool)>(Offsets::paintTraverseOffset)(pPanels, vguiPanel, forceRepaint, allowForce);
	}

	if (vguiPanel == vguiMatSystemTopPanel) {
		ranMatSystemPanel = true;
		if (gHack.inited) {
			RecurseCall_everyFrame(ACTIVE_HACKS);
		}
	}

	if (vguiFocusOverlayPanel == vguiPanel && ranMatSystemPanel) // If we're on MatSystemTopPanel, call our drawing code.
	{
		ranMatSystemPanel = false;

		gDrawManager.DrawString(gDrawManager.HudFont, 0, 0, COLOR_TEAMONE, "F1Cheat.net " __DATE__ " %s", discordUserId); // Remove this if you want.

		if (GetLocalPlayer() == nullptr)
			return;

		// gInts->DebugOverlay->ClearAllOverlays();

		// TODO: Convar for checking to clean screenshots
		if (gInts->Engine->IsDrawingLoadingImage() || !gInts->Engine->IsInGame() || !gInts->Engine->IsConnected() || gInts->Engine->Con_IsVisible() || ((GetAsyncKeyState(VK_F12) || gInts->Engine->IsTakingScreenshot()))) {
			return; // We don't want to draw at the menu.
		}

		{
			F1_VPROF("RecurseCall_paint");
			RecurseCall_paint(ACTIVE_HACKS);
		}

		{
			F1_VPROF("RecurseCall_processEntity");

			int localPlayerIndex = gInts->Engine->GetLocalPlayerIndex();

			// this can probably be improved
			// index 1 is not always local player, so we have to check this
			for (int j = 1; j < gInts->EntList->GetHighestEntityIndex(); ++j) {
				auto *pBaseEntity = gInts->EntList->GetClientEntity(j);

				// do not call for null entitys (theres no point)
				if (pBaseEntity != NULL && pBaseEntity->GetIndex() != localPlayerIndex) {
					RecurseCall_processEntity(pBaseEntity, ACTIVE_HACKS);
				}
			}
		}
		{
			F1_VPROF("Log::RenderLog");
			Log::RenderLog();
		}

		for (auto &window : gHack.windowArray) {

			// TODO maybe switch the order of these?

			// render each window
			window->render();

			// think each window
			window->think();
		}
	}
}
