#include "stdafx.hh"

#include "../SDK/CDrawManager.hh"
#include "CHack.hh"
#include "LagCompensation.hh"
#include "Panels.hh"

#include "F1_Glow.hh"

#include <tier0/icommandline.h>

#include <crtdbg.h>

//#include "../Knife/KnifeMain.hh"
#include "modules.hh"

#include <tier0/memdbgon.h>

// TODO: should we define this in the sdk?
CScreenSize gScreenSize;

// this can be easily fixed by making gInts a pointer
static CInterfaces interfaces;
CInterfaces *      gInts = &interfaces;

DEFINE_RECURSE_CALL_FUNCTION_NO_ARGS(init);

static int f1_cvar_list_autocomplete(char const *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{

    const auto findLastSpace = [](const char *string) {
        for (int i = strlen(string); i != 0; i--)
            if (isspace(string[i]))
                return &(string[i + 1]);
        return (const char *)0;
    };

    const auto findLastSpaceIndex = [](const char *string) {
        for (int i = strlen(string); i != 0; i--)
            if (isspace(string[i]))
                return i;
        return 0;
    };

    const char *arg = findLastSpace(partial);

    int count = 0;
    for (auto p = F1_ConVarAccessor::staticConVarTail; p != nullptr; p = p->next) {
        if (count == 64)
            break;

        if (strlen(arg) == 0 || strstr(p->InternalName(), arg) == 0)
            continue;

        char command[64];

        V_strncpy((char *)command, partial, findLastSpaceIndex(partial) + 2);

        V_strcat(command, p->InternalName(), COMMAND_COMPLETION_ITEM_LENGTH);

        V_strcpy_safe(commands[count], command);
        count += 1;
    }
    return count;
}

CON_COMMAND_F_COMPLETION(f1_cvar_get_value, "Get the value of an internal var", 0, f1_cvar_list_autocomplete)
{
    if (args.ArgC() < 2) {
        gInts->Cvar->ConsoleColorPrintf({255, 0, 0, 255}, "f1_cvar_get_value takes exactly 1 argument\n");
    } else {
        for (auto p = F1_ConVarAccessor::staticConVarTail; p != nullptr; p = p->next) {
            if (strcmp(p->InternalName(), args.Arg(1)) == 0) {
                gInts->Cvar->ConsolePrintf("%s\n", p->print());
            }
        }
    }
}

CON_COMMAND_F_COMPLETION(f1_cvar_set_value, "Set the value of an internal name", 0, f1_cvar_list_autocomplete)
{
    if (args.ArgC() < 3) {
        gInts->Cvar->ConsoleColorPrintf({255, 0, 0, 255}, "f1_cvar_set_value takes exactly 2 arguments\n");
    } else {
        for (auto p = F1_ConVarAccessor::staticConVarTail; p != nullptr; p = p->next) {
            if (strcmp(p->InternalName(), args.Arg(1)) == 0) {
                p->SetFromString(args.Arg(2));
                gInts->Cvar->ConsolePrintf("%s\n", p->print());
            }
        }
    }
}

CON_COMMAND(f1_cvar_get_config, "Dumps current config to console buffer")
{
    for (auto p = F1_ConVarAccessor::staticConVarTail; p != nullptr; p = p->next) {
        gInts->Cvar->ConsolePrintf("f1_cvar_set_value %s %s\n", p->InternalName(), p->print());
    }
}

CON_COMMAND(f1_cvar_save_config, "Save config to file")
{
    if (args.ArgC() < 2) {
        gInts->Cvar->ConsoleColorPrintf({255, 0, 0, 255},
                                        "Usage: f1_cvar_save_config <cfg name>\n"
                                        "To load a config, execute "
                                        "like a normal script file\n");
        return;
    }

    std::fstream f;

    f.open(std::string("tf//cfg//") + args.Arg(1) + ".cfg", std::ios::trunc | std::ios::in | std::ios::out);

    if (f.is_open() && f.good()) {
        for (auto p = F1_ConVarAccessor::staticConVarTail; p != nullptr; p = p->next) {
            f << "f1_cvar_set_value " << p->InternalName() << " " << p->print() << "\r\n";
        }

        f.close();
    } else {
        gInts->Cvar->ConsoleColorPrintf({255, 0, 0, 255}, "Unable to open or write to file\n");
    }
}

CON_COMMAND(f1_cvar_get_list, "Get a list of the internal names, real names and values")
{

    gInts->Cvar->ConsolePrintf("name | internalName | value\n");

    for (auto p = F1_ConVarAccessor::staticConVarTail; p != nullptr; p = p->next) {
        gInts->Cvar->ConsolePrintf("%s | %s | %s\n", p->name(), p->InternalName(), p->print());
    }
}

CON_COMMAND_F_COMPLETION(f1_cvar_get_valid_values, "Get a list of the valid values for an enum cvar", 0, f1_cvar_list_autocomplete)
{
    if (args.ArgC() < 2) {
        gInts->Cvar->ConsoleColorPrintf(
            {255, 0, 0, 255},
            "f1_cvar_get_valid_values takes exactly 1 argument\n");
    } else {
        for (auto p = F1_ConVarAccessor::staticConVarTail; p != nullptr; p = p->next) {
            if (strcmp(args.Arg(1), p->InternalName()) == 0) {
                if (auto *pEnum = dynamic_cast<F1_ConVar<IEnum> *>(p)) {
                    // this is a horrible but necessary conversion
                    for (auto &t : ((F1_ConVar<Enum<int>> *)pEnum)->getEnum().translations)
                        gInts->Cvar->ConsolePrintf("v: %d | t: %s\n", t.v, t.t);
                } else {
                    gInts->Cvar->ConsoleColorPrintf({255, 0, 0, 255}, "%s is not an enum cvar!", args.Arg(1));
                }
            }
        }
    }
}

CON_COMMAND(f1_net_fix, "get optimum net settings")
{
    // always force cl_interp_ratio to 2 (works every time /shrug)
    if (gHack.manualInterp.Value())
        gInts->Engine->ExecuteClientCmd("cl_interpolate 1;cl_interp 0.015;cl_interp_ratio 1;cl_updaterate 66;cl_cmdrate 66");
}

CON_COMMAND(f1_esp_reload, "fix visuals")
{
    gInts->Engine->ExecuteClientCmd("hud_reloadscheme");
}

CON_COMMAND(f1_reset_panel, "fix panel problems")
{
    vguiFocusOverlayPanel = 0;
}

void ChatSpam()
{
}

CON_COMMAND(f1_chatspam, "send a chatspam message")
{
    ThreadExecute(&ChatSpam);
}

CON_COMMAND(f1_knife_load, "Load Knife")
{
#ifdef KNIFE
    if (SetupKnife() == -1) {
        gInts->Cvar->ConsoleColorPrintf({255, 0, 0, 255}, "Error loading knife\n");
    }
#endif
}

CON_COMMAND(f1_knife_unload, "Unload Knife")
{
#ifdef KNIFE
    ShutdownKnife();
    gInts->Cvar->ConsoleColorPrintf({0, 255, 0, 255}, "Knife shutdown successfully\n");
#endif
}

CON_COMMAND(f1_knife_alloc, "TEMPORARY - allocate a function")
{
}

FILE *f1_vprof_file;

void __cdecl f1_dump_report(const char *fmt, ...)
{
    if (f1_vprof_file != nullptr) {
        va_list vlist;
        va_start(vlist, fmt);
        {
            vfprintf(f1_vprof_file, fmt, vlist);
        }
        va_end(vlist);
    } else {
        // this shouldnt happen
    }
}

CON_COMMAND(f1_vprof_report, "Generate a vprof report")
{
    const char *homeDir = getenv("USERPROFILE");

    SYSTEMTIME sysTime;
    GetSystemTime(&sysTime);

    char time[10];
    char date[10];

    GetDateFormat(LOCALE_USER_DEFAULT, 0, &sysTime, "yyyyMMdd", date, 10);
    GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &sysTime, "hhmmss", time, 10);

    auto vprof_file = (std::string(homeDir) + "\\F12017\\VPROF\\" + date + "_" + time + ".txt");

    f1_vprof_file = fopen(vprof_file.c_str(), "w");

    if (f1_vprof_file == nullptr) {
        Msg("\nUnable to open vprof file\n");
        return;
    }

    g_VProfCurrentProfile.SetOutputStream(&f1_dump_report);

    g_VProfCurrentProfile.OutputReport(VPRT_FULL);

    g_VProfCurrentProfile.SetOutputStream(nullptr);

    fclose(f1_vprof_file);

    Msg("Report generated to %s", vprof_file.c_str());
}

PVOID tempCM;
PVOID clientModeThread()
{
    // Log::Error("Clientmode thread");
    // the new not_null means that we need a temp here to help us
    do {
        DWORD dwClientModeAddress = gSignatures.GetClientSignature(XorString("8B 0D ? ? ? ? 8B 02 D9 05"));

        tempCM = **reinterpret_cast<void ***>(dwClientModeAddress + 0x2);

        Sleep(10);
    } while (tempCM == nullptr);

    return tempCM;
}

CHack::CHack()
{
    // init code now done in Init()
}

CHack::~CHack()
{
}

// TODO: init needs to become a bootstrapper that loads the essentials to get us to the main thread
// from there we need to perform the rest of the initialisation process

void CHack::Init(HINSTANCE hInstance)
{
    gHack.ClientFactory         = srcFactory(GetProcAddress(gSignatures.GetModuleHandleSafe("client.dll"), "CreateInterface"));
    gHack.EngineFactory         = srcFactory(GetProcAddress(gSignatures.GetModuleHandleSafe("engine.dll"), "CreateInterface"));
    gHack.VGUIFactory           = srcFactory(GetProcAddress(gSignatures.GetModuleHandleSafe("vguimatsurface.dll"), "CreateInterface"));
    gHack.VGUI2Factory          = srcFactory(GetProcAddress(gSignatures.GetModuleHandleSafe("vgui2.dll"), "CreateInterface"));
    gHack.MaterialSystemFactory = srcFactory(GetProcAddress(gSignatures.GetModuleHandleSafe("MaterialSystem.dll"), "CreateInterface"));
    gHack.PhysicsFactory        = srcFactory(GetProcAddress(gSignatures.GetModuleHandleSafe("vphysics.dll"), "CreateInterface"));
    gHack.CvarFactory           = srcFactory(GetProcAddress(gSignatures.GetModuleHandleSafe("vstdlib.dll"), "CreateInterface"));

    // do our init code

    // == CLIENT ==

    // we have to create a CHLClient hook here as we need to get the createmove
    // function for cinput

    interfaces.Client = gHack.ClientFactory.get<CHLClient *>("VClient017");
    gHookManager.findOrCreateHook(interfaces.Client);

    interfaces.EntList      = gHack.ClientFactory.get<CEntList *>("VClientEntityList003");
    interfaces.Prediction   = gHack.ClientFactory.get<CPrediction *>("VClientPrediction001");
    interfaces.GameMovement = gHack.ClientFactory.get<IGameMovement *>("GameMovement001");
    interfaces.ClientMode   = static_cast<ClientModeShared *>(clientModeThread());

    // == ENGINE ==

    interfaces.Engine       = gHack.EngineFactory.get<EngineClient *>("VEngineClient013");
    interfaces.ModelInfo    = gHack.EngineFactory.get<CModelInfo *>("VModelInfoClient006");
    interfaces.ModelRender  = gHack.EngineFactory.get<CModelRender *>("VEngineModel016");
    interfaces.RenderView   = gHack.EngineFactory.get<CRenderView *>("VEngineRenderView014");
    interfaces.EngineTrace  = gHack.EngineFactory.get<CEngineTrace *>("EngineTraceClient003");
    interfaces.RandomStream = gHack.EngineFactory.get<CUniformRandomStream *>("VEngineRandom001");
    interfaces.EventManager = gHack.EngineFactory.get<IGameEventManager2 *>("GAMEEVENTSMANAGER002");
    interfaces.DebugOverlay = gHack.EngineFactory.get<IVDebugOverlay *>("VDebugOverlay003");
    interfaces.SoundEngine  = gHack.EngineFactory.get<IEngineSound *>("IEngineSoundClient003");
    interfaces.Partition    = gHack.EngineFactory.get<ISpatialPartition *>("SpatialPartition001");

#ifdef _MSC_VER
    interfaces.DemoPlayer  = *reinterpret_cast<CDemoPlayer **>(gSignatures.GetEngineSignature("8B 0D ? ? ? ? 50 FF 56 14") + 0x2);
    interfaces.ClientState = *reinterpret_cast<CClientState **>(gSignatures.GetEngineSignature("B9 ? ? ? ? E8 ? ? ? ? 83 F8 FF 5E") + 0x1);
    interfaces.Input       = **reinterpret_cast<CInput ***>((*(DWORD **)gInts->Client)[15] + 0x2);
#else
    interfaces.DemoPlayer  = *reinterpret_cast<CDemoPlayer **>(gSignatures.GetEngineSignature("A1 ? ? ? ? 8B 10 0F 84 D5 00 00 00 89 45 08 8B 42 44") + 0x1);
    interfaces.ClientState = *reinterpret_cast<CClientState **>(gSignatures.GetEngineSignature("C7 04 24 ? ? ? ? E8 ? ? ? ? 83 F8 FF 0F 95 C0") + 0x3);
    interfaces.Input       = nullptr; // TODO: find me, should be similar to the windows one but we can never be sure!
#endif

    interfaces.Surface = gHack.VGUIFactory.get<CSurface *>("VGUI_Surface030");
    interfaces.Panels  = gHack.VGUI2Factory.get<CPanel *>("VGUI_Panel009");
    interfaces.Cvar    = gHack.CvarFactory.get<ICvar *>("VEngineCvar004");

#ifdef _MSC_VER
    interfaces.Globals = *reinterpret_cast<CGlobals **>(gSignatures.GetEngineSignature("A1 ? ? ? ? 8B 11 68") + 8);
#else
    interfaces.Globals     = *reinterpret_cast<CGlobals **>(gSignatures.GetEngineSignature("C7 44 24 0C ? ? ? ? 89 54 24 08 89 54 24 04 89 04 24 FF 11") + 4);
#endif

    // == VPHYSICS ==
    interfaces.PhysicsSurfaceProps = gHack.PhysicsFactory.get<IPhysicsSurfaceProps *>("VPhysicsSurfaceProps001");

    // == MATERIALSYSTEM ==
    interfaces.MatSystem = gHack.MaterialSystemFactory.get<CMaterialSystem *>("VMaterialSystem081");

    interfaces.EngineVGUI = gHack.EngineFactory.get<IEngineVGui *>("VEngineVGui001");

    // TODO: linxu
    interfaces.ScreenSpaceEffectManager = **reinterpret_cast<IScreenSpaceEffectManager ***>(gSignatures.GetClientSignature("8B 0D ? ? ? ? FF 75 10 FF 75 0C 8B 01 FF 75 08 FF 50 28") + 2);

    gInts->thisDll = hInstance;

#ifdef _MSC_VER
    DWORD dwAppSystem = gSignatures.GetEngineSignature("A1 ? ? ? ? 8B 11 68");
    XASSERT(dwAppSystem);
    DWORD dwAppSystemAddress = **reinterpret_cast<PDWORD *>((dwAppSystem) + 1);
    XASSERT(dwAppSystemAddress);
#else
    DWORD dwAppSystem =
        gSignatures.GetEngineSignature("C7 44 24 0C ? ? ? ? 89 54 24 08 89 54 24 04 89 04 24 FF 11") - 6;
    XASSERT(dwAppSystem);
    DWORD dwAppSystemAddress = **reinterpret_cast<DWORD **>((dwAppSystem) + 2);
    XASSERT(dwAppSystemAddress);
#endif

    CreateInterfaceFn AppSysFactory = reinterpret_cast<CreateInterfaceFn>(dwAppSystemAddress);

    ConnectTier1Libraries(&AppSysFactory, 1);
    ConnectTier2Libraries(&AppSysFactory, 1);
    ConnectTier3Libraries(&AppSysFactory, 1);
    gInts->Cvar = g_pCVar;

    // register all convars
    ConVar_Register(0);

    MathLib_Init();

    // hook panels
    // this will bootstrap our code into the main thread
    gHookManager.hookMethod(gInts->Panels, Offsets::paintTraverseOffset, &CHack::Hooked_PaintTraverse);

    // TODO: Fixme
    gHack.windowArray.AddToTail(&gRadar);

    // Log::Error("canIntro=true");

    // Finally call our intro code
    gHack.canIntro = true;

    return;
}

#include "../SDK/CDumper.hh"

// set up draw manager and netvars
void CHack::intro()
{
    gNetvars.init();
    gDrawManager.Initialize(); // Initalize the drawing class.

    // intro printing stuff to console

    Color c(0, 0, 255, 255);
    gInts->Cvar->ConsoleColorPrintf(
        {255, 0, 0, 255},
        "\n \n \n"
        "      .:::::::::::. \n"
        "    .yMMMMMMMMMMd/` \n"
        "  .yMMMMMMMMMMm/`   \n"
        "  dMMMMh-             https://f1ssi0n.com\n"
        "  `..+NMNNNNNNNNNy-   F1Cheat loaded scucessfully.\n"
        "   :dMMMMMMMMMNh-   \n"
        "  yMMMMmo++++/-     \n"
        "  dMMMm/            \n"
        "  dMmo`             \n"
        "  /o`               \n\n\n");

    CSteamID localID = gInts->SteamContext.SteamUser()->GetSteamID();

    gInts->Cvar->ConsoleColorPrintf({255, 255, 255, 255}, "You are user: %lld\n", localID.ConvertToUint64());
    gInts->Cvar->ConsoleColorPrintf({255, 255, 255, 255}, "     discord: %s\n", discordUserId + 15);

    RecurseCallinit(ACTIVE_HACKS);

    killCvars(NULL);

    // gInts->Engine->ExecuteClientCmd("cl_interpolate 0");
    gInts->Engine->ExecuteClientCmd("-voicerecord");
    gInts->Engine->ExecuteClientCmd("voice_forcemicrecord 0");
    gInts->Engine->ExecuteClientCmd("voice_loopback 1");

    // ConVar_Register(0);

    TrackSteamID(localID.ConvertToUint64());

    CDumper dumper;
    dumper.SaveDump();

    mypanel->Create(g_pVGuiSurface->GetEmbeddedPanel());

    gHack.lagExploitKey.updateKey("SHIFT", F1_BindableConVar::mode::holddown);

    pFlatMaterial           = gInts->MatSystem->CreateMaterial("custom_flat_material", true, false, false);
    pFlatHiddenMaterial     = gInts->MatSystem->CreateMaterial("custom_flat_hidden_material", true, true, false);
    pTexturedMaterial       = gInts->MatSystem->CreateMaterial("custom_textured_material", false, false, false);
    pTexturedHiddenMaterial = gInts->MatSystem->CreateMaterial("custom_textured_hidden_material", false, true, false);

    gInts->ScreenSpaceEffectManager->InitScreenSpaceEffects();
    gInts->ScreenSpaceEffectManager->EnableScreenSpaceEffect(F1_GetGlowEffect());

    gInts->LagCompensation->Init();

    // finally hook the rest of the functions here
    gHookManager.hookMethod(gInts->Client, Offsets::createMoveOffset, &CHack::Hooked_CHLCreateMove);

    // client
    gHookManager.hookMethod(gInts->Client, Offsets::keyEvent, &CHack::Hooked_KeyEvent);
    gHookManager.hookMethod(gInts->Client, Offsets::frameStageNotify, &CHack::Hooked_FrameStageNotify);
    gHookManager.hookMethod(gInts->Client, Offsets::writeUserCmdToBufferOffset, &CHack::Hooked_WriteUserCmdDeltaToBuffer);

    // clientmode
    gHookManager.hookMethod(gInts->ClientMode, Offsets::createMoveOffset, &CHack::Hooked_CreateMove);
    gHookManager.hookMethod(gInts->ClientMode, Offsets::overrideView, &CHack::Hooked_OverrideView);

    // input
    gHookManager.hookMethod(gInts->Input, Offsets::getUserCmdOffset, &CHack::Hooked_GetUserCmd);

    gHookManager.hookMethod(gInts->Prediction, Offsets::runCommandOffset, &CHack::Hooked_RunCommand);

    gHookManager.hookMethod(gInts->MatSystem, Offsets::overrideConfig, &CHack::Hooked_OverrideConfig);

    Log::Msg(XorString("Injection Successful"));
    inited = true;
}

CON_COMMAND(f1_load_custom_item_schema, "load a custom item schema from the F12017 directory")
{
#ifdef _MSC_VER
    typedef bool(__thiscall * BInitTextBufferFn)(PVOID, CUtlBuffer &, int);
    static volatile DWORD BInitTextBuffer =
        gSignatures.GetClientSignature("E8 ? ? ? ? 83 BD ? ? ? ? ? 8A D8") + 1;
    XASSERT(BInitTextBuffer);

    BInitTextBuffer = ((*(PDWORD)(BInitTextBuffer)) + BInitTextBuffer + 4);

    XASSERT(BInitTextBuffer);

    typedef PVOID(__thiscall * GetItemSchemaFn)(void);
    static volatile DWORD getItemSchemaFunc =
        gSignatures.GetClientSignature("A1 ? ? ? ? 85 C0 75 41");
    XASSERT(getItemSchemaFunc);

    GetItemSchemaFn GetItemSchema = (GetItemSchemaFn)(getItemSchemaFunc);

    char *buffer = new char[4000000];

    char *      homeDir  = getenv("HOMEPATH");
    std::string filePath = std::string(homeDir) + "\\F12017\\custom_schema.txt";

    FILE *items_game = fopen(filePath.c_str(), "r");

    if (items_game) {
        size_t newLen = fread(buffer, sizeof(char), 4000000, items_game);

        CUtlBuffer buf(buffer, newLen, 9);

        if (ferror(items_game) != 0) {
            fputs("Error reading file", stderr);
        } else {
            buffer[newLen++] = '\0'; // Just to be safe.
        }
        bool ret = ((BInitTextBufferFn)BInitTextBuffer)(GetItemSchema(), buf, 0);

        Log::Error("Returned %s", ret ? "true" : "false");
    } else {
        Log::Error("Error loading %s", filePath.c_str());
    }

    delete[] buffer;

#endif
}

#include "F1_ex.hh"

CON_COMMAND(f1_ex, "f1script executor")
{
    if (args.ArgC() < 2 || args.ArgC() > 2) {
        gInts->Cvar->ConsoleColorPrintf({255, 0, 0, 255},
                                        "ex takes exactly 1 argument\n");
    }

    // const char *a = args.Arg(1);

    // F1_ParseEx(a);
}

// TODO: make me a panel!
void Log::RenderLog()
{
    Log::MaxQueueItems = gHack.maxLogLines.Value();

    if (gHack.showLog.Value() == false)
        return;

    int y = 100;
    // draw a log to the side
    for (auto i = Log::LogQueue.First(); Log::LogQueue.IsValid(i); i = Log::LogQueue.Next(i)) {
        gDrawManager.DrawString(gDrawManager.HudFont, 0, y += gDrawManager.GetHudHeight(), COLOR_OBJ, "%s", Log::LogQueue.Element(i));
    }
}
