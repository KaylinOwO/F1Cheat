#include "stdafx.hh"

#include <WinSock2.h>

#include "F1_GameSystem.hh"

#include "../SDK/SDK.hh"
#include "shared/igamesystem.h"

#include "CHack.hh"

#include "Panels.hh"
#include "modules.hh"

//#include "../Knife/KnifeMain.hh"

#include "LagCompensation.hh"

#include <tier0/memdbgon.h>

DEFINE_RECURSE_CALL_FUNCTION_NO_ARGS(_perFrame);

class F1_StatManager : IGameEventListener2
{
public:
	bool inited = false;

	int killStat = 0;

	F1_StatManager()
	{
	}

	void Init()
	{
		if (!inited) {
			inited = true;
			gInts->EventManager->AddListener(this, "player_death", false);
		}
	}

	void Reset()
	{
		killStat = 0;
	}

	void FireGameEvent(IGameEvent *event)
	{
		int localUserID = gInts->Engine->GetPlayerInfo(gInts->Engine->GetLocalPlayerIndex()).userID;
		// Log::Console("%s Attacker: %i CustomKill: %i Me: %i Userid: %i Inflictor: %i Time: %f", event->GetName(), event->GetInt("attacker", 0),
		//			 event->GetInt("customkill", 0), me, event->GetInt("userid", 0), event->GetInt("inflictor_entindex", 0), gInts->Globals->curtime);
		if (localUserID == 0) // CBasePlayer::GetUserId() You can get this from the player_info_t struct as well.
			return;

		if (!strcmp(event->GetName(), "player_death")) {
			int attacker = event->GetInt("attacker", 0);
			int userId   = event->GetInt("userid", 0);

			if (attacker == localUserID) {
				if (attacker == userId)
					return;

				// track the kill
				killStat += 1;
			}
		}
	}

	int PostStats()
	{
		WSADATA     wsaData;
		SOCKET      Socket;
		SOCKADDR_IN SockAddr;
		std::string get_http;

		char buffer[10000];
		int  i = 0;

		Socket     = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		auto *host = gethostbyname("f1cheat.net");

		if (host == nullptr) {
			return 1;
		}

		SockAddr.sin_port        = htons(80);
		SockAddr.sin_family      = AF_INET;
		SockAddr.sin_addr.s_addr = *((unsigned long *)host->h_addr);

		if (connect(Socket, (SOCKADDR *)(&SockAddr), sizeof(SockAddr)) != 0) {
			// F1_FastFail<ERROR_WAIT_FOR_OPLOCK>(PF_FASTFAIL_AVAILABLE);
			return 1;
		}

		std::string requestString = std::string("apikey=none&userid=") + (discordUserId + 15) + "&num=" + std::to_string(killStat) + "&type=total";

		get_http += std::string("POST /api/v1/kills/ HTTP/1.1");
		get_http += "\r\nHost: f1cheat.net\r\n";
		get_http += "Content-Type: application/x-www-form-urlencoded\r\n";
		get_http += "Content-Length: " + std::to_string(requestString.length()) + "\r\n\r\n";
		get_http += requestString;
		// get_http += "\r\nConnection: close\r\n\r\n";
		send(Socket, get_http.c_str(), strlen(get_http.c_str()), 0);

		if (recv(Socket, buffer, sizeof(buffer), 0) == 0) {
			// F1_FastFail<FAIL_FAST_GENERATE_EXCEPTION_ADDRESS>(FAST_FAIL_DLOAD_PROTECTION_FAILURE);
			return 1;
		}

		closesocket(Socket);

		return 0;
	}
};

IGameSystem::~IGameSystem()
{
	// TODO: remove here
}

IGameSystemPerFrame::~IGameSystemPerFrame()
{
	// TODO: remove here
}

class F1_GameSystem : IGameSystemPerFrame
{

	F1_StatManager statManager;

public:
	// GameSystems are expected to implement these methods.
	virtual char const *Name()
	{
		return "F1_GameSystem";
	};

	// Init, shutdown
	// return true on success. false to abort DLL init!
	virtual bool Init()
	{
		return true;
	}
	virtual void PostInit()
	{
		Log::Console("Post Init");

		gInts->LagCompensation->Init();

		return;
	}
	virtual void Shutdown()
	{
		// shutdown c#
		//ShutdownKnife();
		return;
	}

	// Level init, shutdown
	virtual void LevelInitPreEntity()
	{

		Log::Console("Level Init Pre Entity");
		statManager.Init();

		// regenrate view matrix
		gDrawManager.Invalidate();

		gInts->Engine->ClientCmd_Unrestricted("f1_net_fix");

		return;
	}
	// entities are created / spawned / precached here
	virtual void LevelInitPostEntity()
	{
		Log::Console("Level Init Post Entity");
		gPlayerManager.registerAllPlayers();
		return;
	}

	virtual void LevelShutdownPreClearSteamAPIContext(){};
	virtual void LevelShutdownPreEntity()
	{
		Log::Console("Level Shutdown Pre Entity");
		gPlayerManager.deregisterAllPlayers();
		return;
	}
	// Entities are deleted / released here...
	virtual void LevelShutdownPostEntity()
	{
		Log::Console("Level Shutdown Post Entity");
		if (statManager.PostStats() != 0) {
			Log::Console("Failed to send stats - not resetting");
		} else {
			statManager.Reset();
		}

		// clear the history
		gInts->LagCompensation->Shutdown();
		return;
	}
	// end of level shutdown

	// Called during game save
	virtual void OnSave()
	{
		return;
	}

	// Called during game restore, after the local player has connected and entities have been fully restored
	virtual void OnRestore()
	{
		return;
	}

	// Called every frame. It's safe to remove an igamesystem from within this callback.
	virtual void SafeRemoveIfDesired()
	{
		return;
	}

	virtual bool IsPerFrame()
	{
		return true;
	}

	virtual void PreRender()
	{
		return;
	}

	virtual void Update(float frametime)
	{
		if (gHack.inited == true)
			RecurseCall_perFrame(ACTIVE_HACKS);
	}

	virtual void PostRender()
	{
		return;
	}

	F1_GameSystem()
	{
		// Call original Add() function
		// TODO: linxu
		DWORD AddFn = gSignatures.GetClientSignature("E8 ? ? ? ? 83 C4 04 8B 76 04 85 F6 75 D0") + 1;
		AddFn       = RESOLVE_CALLGATE(AddFn);

		// add ourselves to the non per system array
		((void(__cdecl *)(void *))AddFn)(this);

		//// becuase Add() does a dynamic cast to see if we are perframe and we dont inherit from it
		//// we need to add ourselves manually to the per frame list
		// auto addr = gSignatures.GetClientSignature("A1 ? ? ? ? 83 C4 0C 8D 04 B0 8B 75 08");
		// Log::Console("Addr is 0x%X", addr);
		// CUtlVector<void *> *s_GameSystemsPerFrame = **(CUtlVector<void *> ***)(AddFn + 0x92);
		// s_GameSystemsPerFrame->AddToTail(this);

		this->Init();
	}
};

F1_GameSystem gGameSystem;
