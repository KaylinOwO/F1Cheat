#include "stdafx.hh"

#include "CMisc.hh"

#include "CHack.hh"

#include <tier0/memdbgon.h>

CMisc gMisc;

// TODO impl
static ConVar f1_change_name("f1_change_name", "0", FCVAR_NONE, "Sets your name to <name>\nUsage: f1_change_name <name>");

const char *CMisc::name() const
{
	return "MISC ACTIONS";
}

void CMisc::processCommandBeforePred(CUserCmd *pUserCmd)
{
	// CEntity<> local{me};
	auto *pLocalEntity = GetLocalPlayer();

	if (pLocalEntity == NULL)
		return;

	if (bunnyHop.Value() == true) {
		static bool firstjump = 0, fakejmp;

		if (pUserCmd->buttons & IN_JUMP)
			if (!firstjump)
				firstjump = fakejmp = 1;
			else if (!(pLocalEntity->GetFlags() & FL_ONGROUND))
				if (fakejmp)
					fakejmp = 0;
				else
					pUserCmd->buttons &= ~IN_JUMP;
			else
				fakejmp = 1;
		else
			firstjump = 0;
	}

	static ConVar *pNoPush = gInts->Cvar->FindVar("tf_avoidteammates_pushaway");

	if (pNoPush != NULL) {
		pNoPush->SetValue(!noPush.Value());
	}

	if (alwaysAttack2.Value()) {
		pUserCmd->buttons |= IN_ATTACK2;
	}

	/*
        if( actionSlotSpam.Value() )
        {
            // we only need to init this once as it will never change after that
            KeyValues *kv = new KeyValues( "use_action_slot_item_server", "use_action_slot_item_server", 0x1000 );

            CLC_CmdKeyValues kvPacket( kv );

            gInts->Engine->GetNetChannelInfo()->SendNetMsg( kvPacket );
        }


    if( airStuck.Value() )
    {
        if( gInts->Globals->tickcount % 60 == 0 )
        {
            NET_NOP packet;
            packet.SetReliable( false );

            gInts->Engine->GetNetChannelInfo()->SendNetMsg( packet, false, false );
            *gHack.bSendPacket = false;
        }
    }
*/
	return;
}

void CMisc::processEntity(CBaseEntity *pBaseEntity)
{

	if (pBaseEntity->GetClientClass()->classId != classId::CTFPlayer)
		return;

	if (removeDisguise.Value()) {
		int curCond = pBaseEntity->GetCond();

		if (curCond & tf_cond::TFCond_Disguised)
			pBaseEntity->SetCond(curCond & ~tf_cond::TFCond_Disguised);
	}

	return;
}

bool CMisc::paint()
{
	return false;
}
