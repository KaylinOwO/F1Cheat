#pragma once

#include "../SDK/SDK.hh"

#include "F1_VGUIMenu.hh"

#include "../SDK/IHack.hh"

#include "../SDK/F1_Gui.hh"

#ifdef __DEBUG
#include "CDumper.hh"
#endif

#define EXPOSE __declspec(dllexport)

// prepare to die
#define ACTIVE_HACKS \
	gAimbot, gAnnouncer, gAntiaim, *CAntiSmac::getInst(), gAutoAirblast, gBackstab, gEsp, gGlow, /* gNokick,*/ gPlayerManager, gPureBypass, gRadar, gMisc, gHack, gBackstab, /*gCritHelper, */ gKnifeInterface

class CHack : public IHack<CHack>
{
private:
	int createMoveEBP;

	// use of vector here allows for easy adding and removing of windows
	CUtlVector<F1_IComponent *> windowArray;

	// F1_Menu menu;

	F1_ConVar<Switch> hackSwitch{"Hack", "f1_hack_switch", false};

public:
	F1_ConVar<bool> fakeLag{" - Fake lag", "f1_hack_fakelag_enable", false, &hackSwitch};
	F1_ConVar<int>  fakeLagAmount{" - - Amount", "f1_hack_fakelag_amount", 15, 1, 100, 1, &hackSwitch};

private:
	F1_ConVar<int> fakeCrouch{" - Fake crouch", "f1_hack_fakecrouch", 0, 0, 2, 1, &hackSwitch};

public:
	F1_ConVar<int> tickCountConstant{" - Tick count", "f1_hack_tickcount", 0, -200000, 200000, 100, &hackSwitch};

private:
	F1_ConVar<int>    speedHackSpeed{" - Speedhack speed", "f1_hack_speedhack_speed", 7, 0, 100, 1, &hackSwitch};
	F1_BindableConVar speedHackKey{" - Speedhack key", "f1_hack_speedhack_enabled", false, &hackSwitch};
	F1_ConVar<bool>   speedHackForce{" - Force speedhack", "f1_hack_speedhack_force", false, &hackSwitch};

	F1_BindableConVar lagExploitKey{" - Lag exploit key", "f1_hack_lag_exploit", false, &hackSwitch};
	F1_ConVar<int>    lagExploitAmount{" - Lag exploit amount", "f1_hack_lag_exploit_amount", 20, 1, 100, 1, &hackSwitch};

	F1_ConVar<bool> antiaimThirdperson{" - Show AA in thirdperson", "f1_hack_aa_in_thirdperson", true, &hackSwitch};

public:
	F1_ConVar<bool> manualInterp{" - Enable reconstructed interp", "f1_hack_interp", true, &hackSwitch};
	F1_ConVar<bool> lagComp{" - Enable custom lag compensation", "f1_hack_lagcomp", true, &hackSwitch};
	F1_ConVar<bool> lagCompAnim{" - Mess with animations", "f1_hack_lagcomp_mess_anim", false, &hackSwitch};

	//F1_ConVar<bool> legacyMenu{" - Use legacy menu", "f1_hack_legacy_menu", false, &hackSwitch};

	F1_ConVar<bool> showLog{" - Show debug log", "f1_show_debug_log", true, &hackSwitch};
	F1_ConVar<int>  maxLogLines{" - Max log lines", "f1_max_debug_lines", 10, 3, 50, 1, &hackSwitch};

private:
	bool canIntro = false;

	srcFactory ClientFactory;
	srcFactory EngineFactory;
	srcFactory VGUIFactory;
	srcFactory VGUI2Factory;
	srcFactory MaterialSystemFactory;
	srcFactory PhysicsFactory;
	srcFactory CvarFactory;

	// CInterfaces interfaces;

	IMaterial *pFlatMaterial;
	IMaterial *pFlatHiddenMaterial;
	IMaterial *pTexturedMaterial;
	IMaterial *pTexturedHiddenMaterial;

	QAngle lastAngle;

public:
	HINSTANCE hinstanceBackup;

	bool *bSendPacket;
	bool  sendThisTick = true;

	float *CM_input_sample_time;

	CHack();
	~CHack();

public:
	// set up draw manager and netvars
	bool inited = false;

public:
	void intro();

	// setup the hack
	static void Init(HINSTANCE hInstance);

	static void __fastcall Hooked_OverrideView(PVOID ClientMode, int edx, CViewSetup *view);

	static bool __fastcall Hooked_CreateMove(PVOID ClientMode, int edx, float input_sample_frametime, CUserCmd *pCommand);

	static void __fastcall Hooked_CHLCreateMove(PVOID Client, int edx, int sequence_number, float input_sample_frametime, bool active);

	static void __fastcall Hooked_FrameStageNotify(PVOID Client, int edx, ClientFrameStage_t stage);

	static void __fastcall Hooked_PaintTraverse(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce);

	static int __fastcall Hooked_KeyEvent(PVOID CHLClient, int edx, int eventcode, ButtonCode_t keynum, const char *currentBinding);

	static CUserCmd *__fastcall Hooked_GetUserCmd(PVOID Input, int edx, int command_number);

	static bool __fastcall Hooked_IsPlayingDemo(PVOID EngineClient, int edx);

	static void __stdcall Hooked_DrawModelExecute(void *state, ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld);

	static LRESULT __stdcall Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void __forceinline CreateMoveCommon(CUserCmd *pCommand);

	static bool __fastcall Hooked_WriteUserCmdDeltaToBuffer(PVOID CHLClient, int edx, bf_write *buf, int from, int to, bool isNewCommand);

	static void __fastcall Hooked_RunCommand(PVOID Prediction, int edx, CBaseEntity *pBaseEntity, CUserCmd *pCommand, IMoveHelper *moveHelper);

	static bool __fastcall Hooked_OverrideConfig(void *MatSystem, int edx, MaterialSystem_Config_t *config, bool forceupdate);

	static void    EyeAnglesYawProxy(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void    EyeAnglesPitchProxy(const CRecvProxyData *pData, void *pStruct, void *pOut);
	static void    SimulationTimeProxy(const CRecvProxyData *pData, void *pStruct, void *pOut);
	RecvVarProxyFn OldSimTimeProxy;

	static void    CycleProxy(const CRecvProxyData *pData, void *pStruct, void *pOut);
	RecvVarProxyFn OldCycleProxy;

	// inline IHack **getHackArray() { return pHackArray; };

	void menuUpdate(F1_IConVar **menuArray, int &currIndex);
};

extern CHack gHack;
