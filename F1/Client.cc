#include "stdafx.hh"

#include "F1_Cache.hh"

#include "CHack.hh"
#include "modules.hh"

#include <view_shared.h>

#include "CritHelper.hh"

#include "LagCompensation.hh"

#include "F1_Vprof.hh"

#include <tier0/memdbgon.h>

DEFINE_RECURSE_CALL_FUNCTION_1_ARG(_processCommandBeforePred, CUserCmd *);
DEFINE_RECURSE_CALL_FUNCTION_1_ARG(_processCommand, CUserCmd *);

bool wasSpeedCounterZero = false;

void CHack::CreateMoveCommon(CUserCmd *pCommand)
{
	{
		// clear the caches
		gCache.blow();

		// call this before ANY prediction takes place
		RecurseCall_processCommandBeforePred(pCommand, ACTIVE_HACKS);
	}

	bool lcEnabled = gHack.lagComp.Value();

	// by the time we get to this point there is no chance that this will be
	// null!
	// TODO: shouldn't we do this once and then never do it again !
	CBaseEntity *pBaseLocalEntity = GetLocalPlayer();

	// begin local client cmd prediction

	{
		F1_VPROF("CreateMoveCommon::PredictLocalPlayer");

		BYTE __movedata[512];

		CMoveData &moveData = *(CMoveData *)&__movedata;

		memset(&moveData, 0, sizeof(__movedata));

		// back up the globals
		float oldCurTime   = gInts->Globals->curtime;
		float oldFrameTime = gInts->Globals->frametime;
		int   oldTickcount = gInts->Globals->tickcount;

		// set up the globals
		gInts->Globals->tickcount = pBaseLocalEntity->GetTickBase();
		gInts->Globals->curtime   = pBaseLocalEntity->GetTickBase() * gInts->Globals->interval_per_tick;
		gInts->Globals->frametime = gInts->Globals->interval_per_tick;

		// set the current cmd
		pBaseLocalEntity->set<CUserCmd *>(0x107C, pCommand);

		//gInts->GameMovement->StartTrackPredictionErrors (pBaseLocalEntity);

		// do actual player cmd prediction
		gInts->Prediction->SetupMove(pBaseLocalEntity, pCommand, gInts->MoveHelper, &moveData);
		gInts->GameMovement->ProcessMovement(pBaseLocalEntity, &moveData);
		gInts->Prediction->FinishMove(pBaseLocalEntity, pCommand, &moveData);
		// gInts->Prediction->RunCommand( pLocal, pCommand, gInts->MoveHelper );

		//gInts->GameMovement->FinishTrackPredictionErrors (pBaseLocalEntity);

		// reset the current cmd
		pBaseLocalEntity->set<CUserCmd *>(0x107C, 0);

		// normalise tickbase

		//pBaseLocalEntity->GetTickBase() += 1;

		// cleanup from engine prediction
		// restore the globals
		gInts->Globals->curtime   = oldCurTime;
		gInts->Globals->frametime = oldFrameTime;
		gInts->Globals->tickcount = oldTickcount;
	}

	// end local entity prediction

	// run this after local entity prediction
	if (lcEnabled) {
		gInts->LagCompensation->StartLagCompensation(pBaseLocalEntity, pCommand);
	}

	// enumerate targets
	{
		TargetManagerBase::ThinkAll();
	}

	// call each hack
	{
		RecurseCall_processCommand(pCommand, ACTIVE_HACKS);
	}

	// run this before we clean up
	if (lcEnabled) {
		gInts->LagCompensation->FinishLagCompensation(pBaseLocalEntity);
	}

	// do this shit here
	if (gHack.fakeCrouch.Value() == 1) {
		// pCommand->viewangles.y -= 180;
		pCommand->viewangles.z = 90;
	} else if (gHack.fakeCrouch.Value() == 2) {
		// pCommand->viewangles.y += 180;
		pCommand->viewangles.z = 90;
	} else {
		pCommand->viewangles.z = 0;
	}
}

// TODO: move into player manager?
void CHack::EyeAnglesPitchProxy(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	CBaseEntity *pBaseEntity = (CBaseEntity *)pStruct;
	if (pBaseEntity->GetIndex() == gInts->Engine->GetLocalPlayerIndex()) {
		*(float *)pOut = pData->m_Value.m_Float;
		return;
	}

	float fl = pData->m_Value.m_Float;

	// apply corrections according to the user
	auto *playerState = gPlayerManager.getPlayer(pBaseEntity->GetIndex());

	if (playerState != nullptr && playerState->isValid) {
		if (playerState->angles != nullptr) {
			if (playerState->angles->Value() == CPlayerManager::anglesMode::Fake) {
				if (fabs(fl) == 90.0f)
					fl = -fl + 1;
			} else if (playerState->angles->Value() == CPlayerManager::anglesMode::Manual) {
				if (playerState->manualPitchCorrection != nullptr) {
					if (playerState->addOrForce != nullptr) {
						if (playerState->addOrForce->Value() == false) {
							fl += playerState->manualPitchCorrection->Value();
						} else {
							fl = playerState->manualPitchCorrection->Value();
						}
					}
				}
			}
		}
	}

	*(float *)pOut = fl;
}

void CHack::EyeAnglesYawProxy(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	CBaseEntity *pBaseEntity = (CBaseEntity *)pStruct;

	float fl = pData->m_Value.m_Float;

	// apply corrections according to the user
	auto *playerState = gPlayerManager.getPlayer(pBaseEntity->GetIndex());

	if (playerState != nullptr && playerState->isValid) {
		if (playerState->angles != nullptr) {
			if (playerState->angles->Value() == CPlayerManager::anglesMode::Manual) {
				if (playerState->manualYawCorrection != nullptr) {
					if (playerState->addOrForce != nullptr) {
						if (playerState->addOrForce->Value() == false) {
							fl += playerState->manualYawCorrection->Value();
						} else {
							fl = playerState->manualYawCorrection->Value();
						}
					}
				}
			}
		}
	}

	*(float *)pOut = fl;
}

void CHack::SimulationTimeProxy(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	CBaseEntity *pBaseEntity = (CBaseEntity *)pStruct;

	// call the original proxy (does delta conversion so on...)
	gHack.OldSimTimeProxy(pData, pStruct, pOut);
}
void CHack::CycleProxy(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	CBaseEntity *pBaseEntity = (CBaseEntity *)pStruct;

	// call the original proxy (does delta conversion so on...)
	//gHack.OldCycleProxy (pData, pStruct, pOut);

	//Log::Console ("PROXY: m_flCycle: %f", pData->m_Value.m_Float);
	pBaseEntity->SetCycle(pData->m_Value.m_Float);
}
//============================================================================================
bool __fastcall CHack::Hooked_CreateMove(PVOID ClientMode, int edx, float input_sample_frametime, CUserCmd *pCommand)
{
#if 0

	__try {

		auto &hook    = VMTManager::GetHook(ClientMode);
		bool  bReturn = hook.GetMethod<bool(__thiscall *)(PVOID, float, CUserCmd *)>(Offsets::createMoveOffset)(ClientMode, input_sample_frametime, pCommand);

		gHack.CreateMoveCommon(pCommand);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Log::Console("Exception in `Hooked_CreateMove()`");
	}

	return false;

#else

	gHack.CM_input_sample_time = &input_sample_frametime;

	auto &hook    = VMTManager::GetHook(ClientMode);
	bool  bReturn = hook.GetMethod<bool(__thiscall *)(PVOID, float, CUserCmd *)>(
        Offsets::createMoveOffset)(ClientMode, input_sample_frametime, pCommand);

	return bReturn;
#endif
}

ConVar f1_dontforceattackpacket("f1_dontforceattackpacket", "0", FCVAR_NONE, "Dont force the sending of a command with IN_ATTACK");

void __fastcall CHack::Hooked_CHLCreateMove(PVOID CHLClient, int edx, int sequence_number, float input_sample_time, bool active)
{
	// sequence_number = 188;

	F1_VPROF_FUNCTION();

	//SetUnhandledExceptionFilter (&UnhandledSehExceptionHandler);

	CUserCmd *pCommand = gInts->Input->GetUserCmd(sequence_number);

	gHack.sendThisTick = true;

	static int   iSpeedCounter = 0; // Setup a global counter.
	static float step          = 0;
	// If I'm pressing MOUSE4 and the counter was not 0.
	if (iSpeedCounter > 0 &&
	    (gHack.speedHackKey.Value() || gHack.speedHackForce.Value())) {
		wasSpeedCounterZero = false;
		iSpeedCounter--;
		pCommand->tick_count--; // Normalize tick_count.
#ifdef F1_GCC
		// TODO: this is probably wrong make sure its not !
		__asm__ __volatile__(
		    "pushl     %%eax \n"
		    "movl      %%ss:(%%ebp), %%eax \n"
		    "movl      (%%eax), %%eax \n"
		    "lea       4(%%eax), %%eax \n"
		    "subb      $5, (%%eax) \n"
		    "popl      %%eax \n"
		    :
		    :
		    :);
#else
		__asm {
			push eax; // Preserve EAX to the stack.
			mov eax, dword ptr ss : [ebp]; // Move EBP in to EAX.
			mov eax, [eax]; // Derefrence the base pointer.
			lea eax, [eax + 0x4]; // Load the return address in to EAX.
#ifdef __clang__
			subl[eax], 0x5; // Make it return to -5 where it would normally.
#else
			sub[eax], 0x5;
#endif
			pop eax; // Restore EAX
		}
#endif
	}

// grab ebp
#ifdef F1_GCC

	// TODO: register keyword is gone in c++17
	// so we will need another method of doing this
	register int cmEBP asm("ebp");

#else

	int       cmEBP = 0;
	__asm mov cmEBP, ebp;

#endif

	gHack.createMoveEBP = cmEBP;

	auto *bSendPacket = (bool *)(*(char **)cmEBP - 0x1);

	// THIS HAS BEEN MOVED HERE FROM AFTERWARDS TO ALLOW HACKS TO SEE WHETHER A
	// PACKET SHOULD BE SENT run fakelag here
	if (gHack.fakeLag.Value() == true &&
	    (!(pCommand->buttons & IN_ATTACK) && !f1_dontforceattackpacket.GetBool())) {
		static int currLagIndex = 0;

		int maxLagIndex = gHack.fakeLagAmount.Value();

		if (currLagIndex < maxLagIndex) {
			gHack.sendThisTick = false;
			*bSendPacket       = false;
			currLagIndex++;
		} else {
			gHack.sendThisTick = true;
			*bSendPacket       = true;
			currLagIndex       = 0;
		}
	} else {
		//*bSendPacket = true;
	}

	CBaseEntity *pLocalEntity = GetLocalPlayer();
	if (pLocalEntity == nullptr)
		return;

	// force zoomtime refresh
	pLocalEntity->GetZoomTime();

	if (pLocalEntity->GetObserverMode() == 0) {
		if (pLocalEntity->IsAlive() == false) {
			Log::Console("Forcing observer mode");
			pLocalEntity->GetObserverMode() = OBS_MODE_DEATHCAM;
		}
	} else {
		if (pLocalEntity->IsAlive() == true) {
			Log::Console("Forcing observer mode");
			pLocalEntity->GetObserverMode() = OBS_MODE_NONE;
		}
	}

	VMTManager &hook = VMTManager::GetHook(CHLClient);
	hook.GetMethod<void(__thiscall *)(PVOID, int, float, bool)>(Offsets::createMoveOffset)(CHLClient, sequence_number, input_sample_time, active);

	if (!pCommand)
		return;

#ifdef _MSC_VER
// blow the animating bone cache first
// TODO: we dont need to do this when we force reconstructed setupbones
// static long *g_iModelBoneCounter = *(long **)(gSignatures.GetClientSignature ("A1 ? ? ? ? D9 45 0C") + 1);
// (*g_iModelBoneCounter)++;
#endif

	// call the common code
	// TODO: should we only be calling this once per real tick?
	// if(wasSpeedCounterZero)
	gHack.CreateMoveCommon(pCommand);

	gHack.lastAngle = pCommand->viewangles;

	if (gHack.lagExploitKey.Value()) {
		//*bSendPacket = ( sequence_number % 10 ) == 0;

		int mul = gHack.lagExploitAmount.Value();

		// use multiples of 150 because thats the size of the command-buffer
		pCommand->command_number += mul * 90;
		pCommand->hasbeenpredicted = true;
		gInts->ClientState->lastoutgoingcommand += mul * 90;
		gInts->Engine->GetNetChannelInfo()->m_nOutSequenceNr += mul * 90;

		// always send skipped ticks
		gHack.sendThisTick = true;
	}

	if (!gHack.sendThisTick) {
		*bSendPacket = false;
		// pCommand->tick_count--;
	} else {
		*bSendPacket = true;
	}

	// process which keys are down and which are up
	F1_BindableConVar::KeyInputThread();

	// resign the cmd
	// TODO: we no longer need to do this
	//CVerifiedUserCmd *pSafeCommand = (CVerifiedUserCmd *)(*(DWORD *)(gInts->Input + 0x100) + (sizeof(CVerifiedUserCmd) * (sequence_number % 90)));
	//pSafeCommand->m_cmd            = *pCommand;
	//pSafeCommand->m_crc            = GetChecksumForCmd(pSafeCommand->m_cmd);
}
//============================================================================================
float OldYawDeltas[33];
float OldLowerBodyYaws[33];
void __fastcall CHack::Hooked_FrameStageNotify(PVOID Client, int edx, ClientFrameStage_t stage)
{
	VMTManager &hook = VMTManager::GetHook(Client);

	auto *pLocalEntity = GetLocalPlayer();

	bool execOriginal = true;

	// FRAME_NET_UPDATE_POSTDATAUPDATE_START
	if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START) {
	} else if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_END) {
		// backtrack all entities here
		gInts->LagCompensation->FrameUpdatePostEntityThink();
	} else if (stage == ClientFrameStage_t::FRAME_RENDER_START) {
		if (pLocalEntity != nullptr) {

			gInts->LagCompensation->FrameUpdateRenderStart();

			QAngle punchAngles             = pLocalEntity->GetPunchAngles();
			pLocalEntity->GetPunchAngles() = {0, 0, 0};

			if (gHack.antiaimThirdperson.Value() && gInts->Input->CAM_IsThirdPerson()) {
				static Netvar<QAngle> v_ang(0x4, "DT_BasePlayer", "pl", "deadflag");
				v_ang.SetValue(pLocalEntity, gHack.lastAngle);
			}

			hook.GetMethod<void(__thiscall *)(PVOID, ClientFrameStage_t)>(
			    Offsets::frameStageNotify)(Client, stage); // Call the original.

			execOriginal = false;

			pLocalEntity->GetPunchAngles() = punchAngles;
		}
	}

	if (execOriginal)
		hook.GetMethod<void(__thiscall *)(PVOID, ClientFrameStage_t)>(
		    Offsets::frameStageNotify)(Client, stage); // Call the original.
}
//============================================================================================

int __fastcall CHack::Hooked_KeyEvent(PVOID CHLClient, int edx, int eventcode, ButtonCode_t keynum, const char *currentBinding)
{
	int ret = 0;
	_INSTALL_SEH_TRANSLATOR();

	VMTManager &hook =
	    VMTManager::GetHook(CHLClient); // Get a pointer to the instance
	// of your VMTManager with the
	// function GetHook.
	ret = hook.GetMethod<int(__thiscall *)(PVOID, int, int, const char *)>(
	    Offsets::keyEvent)(CHLClient, eventcode, static_cast<int>(keynum), currentBinding); // Call the original.

	if (eventcode == 1) {

		// this isnt called when we are in vgui mode !
		// the setValue(false) is basically useless but oh well
		if (keynum == ButtonCode_t::KEY_INSERT) {
			if (f1_show_menu_panel.GetBool() == false) {
				mypanel->Activate();
				f1_show_menu_panel.SetValue(true);
			} else {
				f1_show_menu_panel.SetValue(false);
			}
		}
	}

	return ret;
}

// no checks here ;)
CUserCmd *__fastcall CHack::Hooked_GetUserCmd(PVOID pInput, int edx, int sequence_number)
{
	// TODO: DONT HARDCODE
	// TODO: linxu
	return &(*(CUserCmd **)((DWORD)gInts->Input + 0xFC))[sequence_number % 90];
}

bool __fastcall CHack::Hooked_IsPlayingDemo(PVOID EngineClient, int edx)
{
	CBaseEntity *pBaseEntity = GetLocalPlayer();

	if (pBaseEntity != nullptr) {
		if (pBaseEntity->IsAlive() /*&& gInts->Engine->IsConnected()*/) {
			return true;
		}
	}

	return gHookManager.getMethod<bool(__thiscall *)(PVOID)>(EngineClient,
	                                                         Offsets::isPlayingDemo)(EngineClient);
}

void __stdcall CHack::Hooked_DrawModelExecute(void *state, ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld)
{

	auto &hook = VMTManager::GetHook(gInts->ModelRender);

	if (pInfo.pModel) {
		std::string pszModelName =
		    gInts->ModelInfo->GetModelName((model_t *)pInfo.pModel);

		if (pszModelName.find("arms") != std::string::npos) {
			// Log::Console("mat name: %s", pszModelName.c_str());

			IMaterial *Hands = gInts->MatSystem->FindMaterial(pszModelName.c_str(), TEXTURE_GROUP_MODEL);
			Hands->SetMaterialVarFlag(MaterialVarFlags_t::MATERIAL_VAR_NO_DRAW, true);
			gInts->ModelRender->ForcedMaterialOverride(Hands, OverrideType_t::OVERRIDE_NORMAL);
		}

		if (pszModelName.find("models/player") != std::string::npos) {
			auto *pBaseEntity = GetBaseEntity(pInfo.entity_index);

			if (pBaseEntity == nullptr)
				goto exit;

			if (pBaseEntity->IsDormant())
				goto exit;

			if (pBaseEntity->IsAlive() == false)
				goto exit;

			// Color render_color_visible = pBaseEntity->GetTeam() ==
			// GetBaseEntity(me)->GetTeam() ? Color(84, 167, 255) : Color(200,
			// 60, 60);

			float teamColor[]        = {0.4, 0.6, 1.0};
			float teamHiddenColor[]  = {1.0, 0.4, 0.0};
			float enemyColor[]       = {0.8, 0.2, 0.2};
			float enemyHiddenColor[] = {0.4, 0.7, 1.0};

			pBaseEntity->GetModelScale() = 2.0f;

			float originalColorMod[4];
			gInts->RenderView->GetColorModulation(originalColorMod);

			gInts->RenderView->SetColorModulation(
			    pBaseEntity->GetTeam() == GetLocalPlayer()->GetTeam() ? teamHiddenColor : enemyHiddenColor);
			gInts->ModelRender->ForcedMaterialOverride(gHack.pFlatHiddenMaterial, OverrideType_t::OVERRIDE_NORMAL);
			gHack.pFlatHiddenMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
			hook.GetMethod<void(__thiscall *)(PVOID, void *, ModelRenderInfo_t &, matrix3x4 *)>(
			    Offsets::drawModelExecute)(gInts->ModelRender, state, pInfo, pCustomBoneToWorld);

			gInts->RenderView->SetColorModulation(
			    pBaseEntity->GetTeam() == GetLocalPlayer()->GetTeam() ? teamColor : enemyColor);
			gInts->ModelRender->ForcedMaterialOverride(gHack.pFlatMaterial, OverrideType_t::OVERRIDE_NORMAL);
			gHack.pFlatMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
			hook.GetMethod<void(__thiscall *)(PVOID, void *, ModelRenderInfo_t &, matrix3x4 *)>(
			    Offsets::drawModelExecute)(gInts->ModelRender, state, pInfo, pCustomBoneToWorld);

			// gInts->RenderView->SetColorModulation(pBaseEntity->GetTeam() ==
			// GetBaseEntity(me)->GetTeam() ? teamColor : enemyColor);
			// gInts->ModelRender->ForcedMaterialOverride(gHack.pFlatMaterial,
			// OverrideType_t::OVERRIDE_NORMAL);

			// hook.GetMethod<void(__thiscall *)(PVOID, void *,
			// ModelRenderInfo_t &, matrix3x4
			// *)>(Offsets::drawModelExecute)(gInts->ModelRender, state, pInfo,
			// pCustomBoneToWorld);

			// gInts->ModelRender->ForcedMaterialOverride(NULL,
			// OverrideType_t::OVERRIDE_NORMAL);

			// gHack.pFlatMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ,
			// true);
		}
	}

exit:
	// call the original

	gInts->ModelRender->ForcedMaterialOverride(NULL, OverrideType_t::OVERRIDE_NORMAL);
	hook.GetMethod<void(__thiscall *)(PVOID, void *, ModelRenderInfo_t &, matrix3x4 *)>(
	    Offsets::drawModelExecute)(gInts->ModelRender, state, pInfo, pCustomBoneToWorld);
}

// TODO: use iinputsystem rather than this shit !
// TODO: linxu
LRESULT __stdcall CHack::Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	mouseButton mb = mouseButton::NONE;

	switch (uMsg) {
	case WM_LBUTTONDOWN:
		mb = mouseButton::LDOWN;
		break;
	case WM_LBUTTONUP:
		mb = mouseButton::LUP;
		break;
	case WM_RBUTTONDOWN:
		mb = mouseButton::RDOWN;
		break;
	case WM_RBUTTONUP:
		mb = mouseButton::RUP;
		break;
	case WM_MOUSEWHEEL:
		if ((int)wParam < 0)
			mb = mouseButton::SCROLLDOWN;
		else
			mb = mouseButton::SCROLLUP;
		break;
	default:
		break;
	}

	if (mb != mouseButton::NONE) {

		int x, y;
		gInts->Surface->SurfaceGetCursorPos(x, y);
		// handle input
		// TODO: bench this should it be parallel?
		for (auto &window : gHack.windowArray)
			window->handleMouseInput(x, y, mb);
	}

	return CallWindowProc(gInts->oldWindowProc, hWnd, uMsg, wParam, lParam);
}

void __fastcall CHack::Hooked_OverrideView(PVOID ClientMode, int edx, CViewSetup *view)
{
	if (gMisc.fovChangeWhenZoomed.Value() == true) {
		view->fov = gMisc.fovChanger.Value();
	} else {
		CBaseEntity *pLocal = GetLocalPlayer();

		if (pLocal) {
			if (pLocal->IsZoomed() != true) {
				view->fov = gMisc.fovChanger.Value();
			}
		}
	}

	// TODO: do we need to do this
	memcpy(gDrawManager.GetLastSetup(), view, sizeof(CViewSetup));

	auto &hook = VMTManager::GetHook(ClientMode);
	return hook.GetMethod<void(__thiscall *)(PVOID, CViewSetup *)>(
	    Offsets::overrideView)(ClientMode, view);
}

bool __fastcall CHack::Hooked_WriteUserCmdDeltaToBuffer(PVOID CHLClient, int edx, bf_write *buf, int from, int to, bool isNewCommand)
{
	using WriteUserCmdFn = void(__cdecl *)(bf_write *, CUserCmd *, CUserCmd *);

	static WriteUserCmdFn writeUserCmd = (WriteUserCmdFn)gSignatures.GetClientSignature(
	    "55 8B EC 83 EC 08 53 8B 5D 0C 56 8B 75 10");

	CUserCmd nullcmd, *pFrom, *pTo;

	if (from == -1) {
		pFrom = &nullcmd;
	} else {
		pFrom = gInts->Input->GetUserCmd(from);

		if (!pFrom)
			pFrom = &nullcmd;
	}

	pTo = gInts->Input->GetUserCmd(to);

	if (!pTo)
		pTo = &nullcmd;

	writeUserCmd(buf, pTo, pFrom);

	if (*(bool *)((DWORD)buf + 0x10)) //IsOverflowed
		return false;

	return true;

	return true;
}

void CHack::Hooked_RunCommand(PVOID Prediction, int edx, CBaseEntity *pBaseEntity, CUserCmd *pCommand, IMoveHelper *moveHelper)
{
	if (moveHelper != nullptr)
		gInts->MoveHelper = moveHelper;

	if (pBaseEntity != nullptr) {
		if (pBaseEntity->GetIndex() == gInts->Engine->GetLocalPlayerIndex()) {
			// roll back the ticks please
			if (gHack.lagExploitKey.Value()) {
				// pCommand->command_number -= gHack.lagExploitAmount.Value()
				// * 90;

				// TODO: we should be marking the packets as already predicted instead of doing this shit

				// just return no need to predict this
				return;
			}
		} else {
			gInts->LagCompensation->HandleNewCommand(pBaseEntity, pCommand);
		}
	}

	return gHookManager.getMethod<void(__thiscall *)(PVOID, CBaseEntity *, CUserCmd *, IMoveHelper *)>(Prediction, Offsets::runCommandOffset)(Prediction, pBaseEntity, pCommand, moveHelper);
}

#include "materialsystem\materialsystem_config.h"

bool __fastcall CHack::Hooked_OverrideConfig(void *MatSystem, int edx, MaterialSystem_Config_t *config, bool forceupdate)
{
	//config->bShowLowResImage = true;
	//config->bDrawFlat = true;

	return gHookManager.getMethod<bool(__thiscall *)(void *, MaterialSystem_Config_t *, bool)>(MatSystem, Offsets::overrideConfig)(MatSystem, config, forceupdate);
}
