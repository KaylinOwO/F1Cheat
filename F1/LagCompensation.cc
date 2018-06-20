#include "stdafx.hh"

#include "../SDK/Trace.hh"
#include "../SDK/baseHeaders.hh"

#include "../SDK/BaseAnimatingLayer.hh"

#include "CHack.hh"
#include "CPlayerManager.hh"
#include "LagCompensation.hh"

#include <tier0/memdbgon.h>

// TODO: move or remove
class CInterpolationContext
{
public:
    CInterpolationContext()
    {
        if (s_bAllowExtrapolation == nullptr || s_flLastTimeStamp == nullptr || s_pHead == nullptr) {
            s_flLastTimeStamp     = *(float **)(gSignatures.GetClientSignature("F3 0F 11 05 ? ? ? ? A3 ? ? ? ? 74 0C") + 4);
            s_bAllowExtrapolation = *(bool **)(gSignatures.GetClientSignature("A2 ? ? ? ? 8B 45 D0") + 1);
            s_pHead               = *(CInterpolationContext **)(gSignatures.GetClientSignature("A1 ? ? ? ? 89 45 D0 8D 45 D0") + 1);
        }

        m_bOldAllowExtrapolation = *s_bAllowExtrapolation;
        m_flOldLastTimeStamp     = *s_flLastTimeStamp;

        // By default, disable extrapolation unless they call EnableExtrapolation.
        s_bAllowExtrapolation = false;

        // this is the context stack
        m_pNext = s_pHead;
        s_pHead = this;
    }

    ~CInterpolationContext()
    {
        // restore values from prev stack element
        *s_bAllowExtrapolation = m_bOldAllowExtrapolation;
        *s_flLastTimeStamp     = m_flOldLastTimeStamp;

        s_pHead = m_pNext;
    }

    static void EnableExtrapolation(bool state)
    {
        *s_bAllowExtrapolation = state;
    }

    static bool IsThereAContext()
    {
        return s_pHead != NULL;
    }

    static bool IsExtrapolationAllowed()
    {
        return s_bAllowExtrapolation;
    }

    static void SetLastTimeStamp(float timestamp)
    {
        *s_flLastTimeStamp = timestamp;
    }

    static float GetLastTimeStamp()
    {
        return *s_flLastTimeStamp;
    }

    CInterpolationContext *m_pNext;
    bool                   m_bOldAllowExtrapolation;
    float                  m_flOldLastTimeStamp;

    static CInterpolationContext *s_pHead;
    static bool *                 s_bAllowExtrapolation;
    static float *                s_flLastTimeStamp;
};

CInterpolationContext *CInterpolationContext::s_pHead;
bool *                 CInterpolationContext::s_bAllowExtrapolation;
float *                CInterpolationContext::s_flLastTimeStamp;

// TODO: FindVar can return nullptr which is a big problem!

float GetUpdateRate()
{
    static ConVar *cl_updaterate = gInts->Cvar->FindVar("cl_updaterate");

    static ConVar *sv_minupdaterate = gInts->Cvar->FindVar("sv_minupdaterate");
    static ConVar *sv_maxupdaterate = gInts->Cvar->FindVar("sv_maxupdaterate");

    return Clamp(cl_updaterate->GetFloat(), sv_minupdaterate->GetFloat(), sv_maxupdaterate->GetFloat());
}

float GetInterpRatio()
{
    static ConVar *cl_interpolate = gInts->Cvar->FindVar("cl_interpolate");

    if (cl_interpolate->GetBool() == false)
        return 0.0f;

    static ConVar *cl_interp_ratio = gInts->Cvar->FindVar("cl_interp_ratio");

    static ConVar *sv_client_min_interp_ratio = gInts->Cvar->FindVar("sv_client_min_interp_ratio");
    static ConVar *sv_client_max_interp_ratio = gInts->Cvar->FindVar("sv_client_max_interp_ratio");

    return Clamp(cl_interp_ratio->GetFloat(), sv_client_min_interp_ratio->GetFloat(), sv_client_max_interp_ratio->GetFloat());
}

// TODO: what about if cl_interpolate is 0??
float GetClientInterpAmount()
{
    static ConVar *cl_interp = gInts->Cvar->FindVar("cl_interp");

    float updateRate = GetUpdateRate();

    return Max(cl_interp->GetFloat(), GetInterpRatio() / updateRate);
}

int GetClientInterpTicks()
{
    return TIME_TO_TICKS(GetClientInterpAmount());
}

int GetInterpolationTicks()
{
    return GetClientInterpTicks() + 1;
}

float GetInterpolationAmount()
{
    return GetClientInterpAmount() + TICK_INTERVAL;
}

class CUserCmd;

#define MAX_PLAYERS (33)

#define LC_NONE 0
#define LC_ALIVE (1 << 0)

#define LC_ORIGIN_CHANGED (1 << 8)
#define LC_ANGLES_CHANGED (1 << 9)
#define LC_SIZE_CHANGED (1 << 10)
#define LC_ANIMATION_CHANGED (1 << 11)

#define LAG_COMPENSATION_EPS_SQR (0.1f * 0.1f)
// Allow 4 units of error ( about 1 / 8 bbox width )
#define LAG_COMPENSATION_ERROR_EPS_SQR (4.0f * 4.0f)

static ConVar f1_showlagcompensation("f1_bt_showlagcompensation", "0", FCVAR_NONE, "Show lag compensation");
static ConVar f1_debuglagcompensation("f1_bt_debuglagcompensation", "0", FCVAR_NONE, "Show debug info for lag compensation");
static ConVar f1_dontshootifnotattack("f1_bt_dontshootifnotattack", "0", FCVAR_NONE, "Allow backtracker to block attacks if they are not on a tick where the target attacks");
static ConVar f1_useplusone("f1_bt_useplusone", "0", FCVAR_NONE, "Use +1");
static ConVar f1_interp("f1_bt_interp", "1", FCVAR_NONE, "use simtime rather than tickcount in the calculation");
static ConVar f1_alwayslagcompensate("f1_bt_alwayslagcompensate", "1", FCVAR_NONE, "always backtrack entities even on local servers");

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

// TODO: shouldnt we move these out of here?
void UTIL_TraceEntity(CBaseEntity *pEntity, const Vector &vecAbsStart, const Vector &vecAbsEnd, unsigned int mask, const CBaseEntity *pIgnore, int nCollisionGroup, trace_t *ptr)
{
    CBaseFilter traceFilter;
    traceFilter.SetIgnoreEntity((CBaseEntity *)pIgnore);

    gInts->EngineTrace->SweepCollideable(pEntity, vecAbsStart, vecAbsEnd, vec3_angle, mask, &traceFilter, ptr);
}

void UTIL_TraceEntity(CBaseEntity *pEntity, const Vector &vecAbsStart, const Vector &vecAbsEnd, unsigned int mask, trace_t *ptr)
{
    CBaseFilter traceFilter;
    traceFilter.SetIgnoreEntity(pEntity);

    gInts->EngineTrace->SweepCollideable(pEntity, vecAbsStart, vecAbsEnd, vec3_angle, mask, &traceFilter, ptr);
}

float       g_flFractionScale = 0.95;
static void RestorePlayerTo(CBaseEntity *pPlayer, const Vector &vWantedPos)
{
    // Try to move to the wanted position from our current position.
    trace_t tr;
    // VPROF_BUDGET( "RestorePlayerTo", "CLagCompensationManager" );
    UTIL_TraceEntity(pPlayer, vWantedPos, vWantedPos, MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_PLAYER_MOVEMENT, &tr);
    if (tr.startsolid || tr.allsolid) {

        UTIL_TraceEntity(pPlayer, pPlayer->GetRenderOrigin(), vWantedPos, MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_PLAYER_MOVEMENT, &tr);
        if (tr.startsolid || tr.allsolid) {
            // In this case, the guy got stuck back wherever we lag compensated
            // him to. Nasty.
        } else {
            // We can get to a valid place, but not all the way back to where we
            // were.
            Vector vPos;
            VectorLerp(pPlayer->GetRenderOrigin(), vWantedPos, tr.fraction * g_flFractionScale, vPos);
            pPlayer->GetRenderOrigin() = vPos;
            pPlayer->SetAbsOrigin(vPos);
        }
    } else {
        // Cool, the player can go back to whence he came.
        pPlayer->GetRenderOrigin() = tr.endpos;
        pPlayer->SetAbsOrigin(tr.endpos);
    }
}

static ConVar f1_extrapolate("f1_extrapolate", "1", FCVAR_NONE, "enable extrapolation in interpolation context");
static ConVar f1_interpolate("f1_interpolate", "0", FCVAR_NONE, "enable interpolation in interpolation context\n\t0 = no interpolation, 1 = interpolation (with correction), 2 = interpolation (no correction)");

class CLagCompensationManager : public ILagCompensationManager
{
    static bool __stdcall Hooked_IsPaused()
    {
        // the only time this gets called when we want to mess with stuff is when
        // there is a interpolation context

        static bool *s_bInterpolate = *(bool **)(gSignatures.GetClientSignature("0F 95 05 ? ? ? ? 8B 01") + 3);

        static void *s_pHead = *(void **)(gSignatures.GetClientSignature("A1 ? ? ? ? 89 45 D0 8D 45 D0") + 1);

        CInterpolationContext *pHead = (CInterpolationContext *)s_pHead;

        if (s_pHead != nullptr) {
            *s_bInterpolate = f1_interpolate.GetBool();

            CLagCompensationManager *LagCompensationManager = (CLagCompensationManager *)gInts->LagCompensation;

            return !f1_extrapolate.GetBool(); // return true to disable the extrapolation
        }

        return gHookManager.getMethod<bool(__thiscall *)()>(gInts->Engine, 84)();
    }

public:
    CLagCompensationManager(char const *name)
        : m_flTeleportDistanceSqr(64 * 64)
    {
        m_isCurrentlyDoingCompensation = false;
    }

    void Init() override
    {
        // look for "C_BaseEntity::InterpolateServerEntities"
        gHookManager.hookMethod(gInts->Engine, 84, &Hooked_IsPaused);
    }

    void Shutdown() override
    {
        ClearHistory();
    }

    void LevelShutdownPostEntity()
    {
        ClearHistory();
    }

    virtual void FrameUpdatePostEntityThink() override;
    virtual void FrameUpdateRenderStart() override;

    // Called during player movement to set up/restore after lag compensation
    void StartLagCompensation(CBaseEntity *player, CUserCmd *cmd) override;
    void FinishLagCompensation(CBaseEntity *player) override;

    bool UpdatePlayer(CBaseEntity *player) override;

    bool IsCurrentlyDoingLagCompensation() const override
    {
        return m_isCurrentlyDoingCompensation;
    }

    float GetIncomingLatency() const
    {
        return gInts->Engine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING);
    }

    float GetOutgoingLatency() const
    {
        return gInts->Engine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
    }

    LagRecord *GetLastRecord(int index) override
    {
        auto attackRecord = m_PlayerAttackRecord[index - 1];
        if (attackRecord != nullptr) {
            return attackRecord;
        }

        auto thisTrack = &m_PlayerTrack[index - 1];
        if (thisTrack->Count() == 0)
            return nullptr;
        return &thisTrack->Element(thisTrack->Head());
    }

    LagRecord *&GetLastAttackRecord(int index)
    {
        return m_PlayerAttackRecord[index - 1];
    }

    void HandleNewCommand(CBaseEntity *player, CUserCmd *pCommand) override;

    float GetPlayerTime(CBaseEntity *player)
    {
        // int pl_index = player->GetIndex() - 1;

        LagRecord *pRecord = GetLastRecord(player->GetIndex());

        if (pRecord != nullptr) {
            return pRecord->m_flSimulationTime;
        }

        return gInts->Globals->curtime;
    }

private:
    void BacktrackPlayer(CBaseEntity *player);

    void ClearHistory()
    {
        for (int i = 0; i < MAX_PLAYERS; i++)
            m_PlayerTrack[i].Purge();
    }

    // keep a list of lag records for each player
    CUtlFixedLinkedList<LagRecord> m_PlayerTrack[MAX_PLAYERS];
    LagRecord *                    m_PlayerAttackRecord[MAX_PLAYERS];

    // Scratchpad for determining what needs to be restored
    CBitVec<MAX_PLAYERS> m_RestorePlayer;
    bool                 m_bNeedToRestore;
    LagRecord            m_RestoreData[MAX_PLAYERS]; // player data before we moved him back
    LagRecord            m_ChangeData[MAX_PLAYERS];  // player data where we moved him back

    // TODO: calculate this in the place where we actually use it !
    float        m_flTeleportDistanceSqr;
    CBaseEntity *m_pCurrentPlayer; // The player we are doing lag compensation for
    CUserCmd *   m_pCurrentCommand;
    bool         m_isCurrentlyDoingCompensation; // Sentinel to prevent calling
                                                 // StartLagCompensation a second time
                                                 // before a Finish.
};

static CLagCompensationManager g_LagCompensationManager("F1_Backtrack");
ILagCompensationManager *      CInterfaces::LagCompensation = &g_LagCompensationManager;

bool CLagCompensationManager::UpdatePlayer(CBaseEntity *player)
{
    F1_VPROF_FUNCTION();

    if (player == nullptr || player->GetIndex() == gInts->Engine->GetLocalPlayerIndex())
        return false;

    if (player->GetClientClass()->classId != classId::CTFPlayer)
        return false;

    int pl_index = player->GetIndex() - 1;

    static ConVar *sv_maxunlag = gInts->Cvar->FindVar("sv_maxunlag");
    int            flDeadtime  = gInts->Globals->curtime - sv_maxunlag->GetFloat();

    CUtlFixedLinkedList<LagRecord> *track = &m_PlayerTrack[pl_index];

    if (!player) {
        if (track->Count() > 0) {
            track->RemoveAll();
        }

        return false;
    }

    LagRecord *&AttackRecord = GetLastAttackRecord(pl_index + 1);

    // remove tail records that are too old
    int tailIndex = track->Tail();
    while (track->IsValidIndex(tailIndex)) {
        LagRecord &tail = track->Element(tailIndex);

        // if tail is within limits, stop
        if (tail.m_flSimulationTime >= flDeadtime)
            break;

        // remove the attack record if it is now dead
        if (&tail == AttackRecord)
            AttackRecord = nullptr;

        // remove tail, get new tail
        track->Remove(tailIndex);
        tailIndex = track->Tail();
    }

    // check if head has a simulation time that is the same or newer
    if (track->Count() > 0) {
        LagRecord &head = track->Element(track->Head());

        // check if player changed simulation time since last time updated
        // TODO do not hardcode
        // TODO: is this right?
        if (head.m_flSimulationTime >= player->GetSimulationTime()) {
            // Log::Console ("Dont add new entry for same or older time");
            return false; // don't add new entry for same or older time
        }
    }

    // add new record to player track
    LagRecord &record = track->Element(track->AddToHead());

    record.m_fFlags = 0;
    if (player->IsAlive()) {
        record.m_fFlags |= LC_ALIVE;
    }

    record.m_flSimulationTime = player->GetSimulationTime();

    float flSimulationTime = player->GetSimulationTime();
    float flSimDiff        = gInts->Globals->curtime - flSimulationTime;
    float latency          = GetIncomingLatency();

    record.m_flChokedTime = Max(0.0f, flSimDiff - latency);
    record.m_nChokedTicks = TIME_TO_TICKS(record.m_flChokedTime);

    // dont use the absorigin here as it is interpolated
    record.m_vecOrigin        = player->GetRenderOrigin();
    record.m_vecMinsPreScaled = player->GetCollideableMinsPrescaled();
    record.m_vecMaxsPreScaled = player->GetCollideableMaxsPrescaled();

    record.m_vecRealAngles = player->GetEyeAngles();
    record.m_vecAngles     = record.m_vecRealAngles;

    const CStudioHdr *hdr = player->GetStudioHdr();

    record.m_vecAngles = record.m_vecRealAngles;

    int layerCount = player->GetNumAnimOverlays();
    // int layerCount = 15;
    for (int layerIndex = 0; layerIndex < layerCount; ++layerIndex) {
        C_AnimationLayer *currentLayer = player->GetAnimOverlay(layerIndex);
        if (currentLayer) {
            record.m_layerRecords[layerIndex].m_cycle    = currentLayer->m_flCycle;
            record.m_layerRecords[layerIndex].m_order    = currentLayer->m_nOrder;
            record.m_layerRecords[layerIndex].m_sequence = currentLayer->m_nSequence;
            record.m_layerRecords[layerIndex].m_weight   = currentLayer->m_flWeight;
        }
    }
    record.m_masterSequence = player->GetSequence();
    record.m_masterCycle    = player->GetCycle();

    //gInts->DebugOverlay->AddEntityTextOverlay (pl_index + 1, 0, 0.0f, 255, 0, 0, 255, "%f", record.m_masterCycle);
    //gInts->DebugOverlay->AddEntityTextOverlay (pl_index + 1, 1, 0.0f, 0, 255, 0, 255, "%f", player->GetCycle ());

    // backup the hdr parameters for this player
    // TODO: do we need to do this
    if (hdr != nullptr) {
        auto *parameters = ((DT_BaseAnimating *)player)->flPoseParameter();

        for (int paramIndex = 0; paramIndex < (*(studiohdr_t **)hdr)->numlocalposeparameters; paramIndex++) {
            record.m_poseParameters[paramIndex] = parameters[paramIndex];
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Called once per frame after all entities have had a chance to think
//-----------------------------------------------------------------------------
void CLagCompensationManager::FrameUpdatePostEntityThink()
{
    static ConVar *sv_unlag = gInts->Cvar->FindVar("sv_unlag");

    if (((gInts->Globals->maxclients <= 1) || !sv_unlag->GetBool()) && !f1_alwayslagcompensate.GetBool()) {
        ClearHistory();
        return;
    }

    // TODO: move this where it is actually used instead of just setting it here
    static ConVar *sv_lagcompensation_teleport_dist = gInts->Cvar->FindVar("sv_lagcompensation_teleport_dist");
    float          flLagcompensation_teleport_dist  = sv_lagcompensation_teleport_dist->GetFloat();

    m_flTeleportDistanceSqr = flLagcompensation_teleport_dist * flLagcompensation_teleport_dist;

    int maxclients = gInts->Globals->maxclients;

    // Iterate all active players
    for (int i = 1; i <= maxclients; i++) {
        CBaseEntity *pPlayer = gInts->EntList->GetClientEntity(i);

        UpdatePlayer(pPlayer);
    }

    // Clear the current player.
    m_pCurrentPlayer = NULL;
}

void CLagCompensationManager::FrameUpdateRenderStart()
{
    // restore s_bInterpolate before interpolateServerEntities is called again
}

bool WantsLagCompensationOnEntity(CBaseEntity *local, CBaseEntity *player, const CUserCmd *pCmd)
{
    // Team members shouldn't be adjusted unless friendly fire is on.
    if (/*!friendlyfire.GetInt() && */ player->GetTeam() == local->GetTeam())
        return false;

    // If this entity hasn't been transmitted to us and acked, then don't bother
    // lag compensating it.
    if (player->IsDormant())
        return false;

    return true;
}

// Called during player movement to set up/restore after lag compensation
void CLagCompensationManager::StartLagCompensation(CBaseEntity *player, CUserCmd *cmd)
{
    F1_VPROF_FUNCTION();

    static ConVar *sv_maxunlag = gInts->Cvar->FindVar("sv_maxunlag");
    Assert(!m_isCurrentlyDoingCompensation);

    // DONT LAG COMP AGAIN THIS FRAME IF THERES ALREADY ONE IN PROGRESS
    // IF YOU'RE HITTING THIS THEN IT MEANS THERES A CODE BUG
    if (m_pCurrentPlayer) {
        Log::Console("Trying to start a new lag compensation session while one is already active!");
        // Assert( m_pCurrentPlayer == NULL );
        // Warning( "Trying to start a new lag compensation session while one is
        // already active!\n" );
        return;
    }

    // Assume no players need to be restored
    m_RestorePlayer.ClearAll();
    m_bNeedToRestore = false;

    m_pCurrentPlayer  = player;
    m_pCurrentCommand = cmd;

    memset(m_RestoreData, 0, sizeof(m_RestoreData));
    memset(m_ChangeData, 0, sizeof(m_ChangeData));

    m_isCurrentlyDoingCompensation = true;

    // Iterate all active players
    // const CBitVec<MAX_EDICTS> *pEntityTransmitBits =
    // engine->GetEntityTransmitBitsForClient(player->entindex() - 1);
    // BeginExecuteParallel();
    for (int i = 1; i <= gInts->Engine->GetMaxClients(); i++) {
        CBaseEntity *playerToBacktrack = GetBaseEntity(i);

        if (!playerToBacktrack) {
            continue;
        }

        // Don't lag compensate yourself you loser...
        if (player == playerToBacktrack) {
            continue;
        }

        // Custom checks for if things should lag compensate (based on things
        // like what team the player is on).
        if (!WantsLagCompensationOnEntity(player, playerToBacktrack, cmd))
            continue;

        BacktrackPlayer(playerToBacktrack);
    }
    // EndExecuteParallel();
}

void CLagCompensationManager::HandleNewCommand(CBaseEntity *player, CUserCmd *pCommand)
{
    int pl_index = player->GetIndex() - 1;

    LagRecord *lastRecord = GetLastRecord(pl_index + 1);

    if (lastRecord != nullptr) {
        int buttons         = pCommand->buttons;
        lastRecord->buttons = buttons;

        if (buttons & IN_ATTACK) {
            // allow quicker access to this record so that we dont have to
            // iterate through whole history to find this
            m_PlayerAttackRecord[pl_index] = lastRecord;
        }
    }
}

void CLagCompensationManager::BacktrackPlayer(CBaseEntity *player)
{
    F1_VPROF_FUNCTION();

    int pl_index = player->GetIndex() - 1;

    // get track history of this player
    CUtlFixedLinkedList<LagRecord> *track = &m_PlayerTrack[pl_index];

    // check if we have at leat one entry
    if (track->Count() <= 0)
        return;

    LagRecord *prevRecord = nullptr;
    LagRecord *record     = nullptr;

    Vector prevOrg = player->GetRenderOrigin();

    // Always use the most recent update UNLESS we have a record where the target is IN_ATTACK

    int curr = track->Head();
    if (m_PlayerAttackRecord[pl_index] == nullptr) {
        record = &track->Element(curr);
    } else {
        record = m_PlayerAttackRecord[pl_index];
    }

    // do the checks that we would do otherwise however
    Vector delta = record->m_vecOrigin - prevOrg;
    if (delta.Length2DSqr() > m_flTeleportDistanceSqr) {
        // Log::Console( "Lost track, too much difference" );
        // lost track, too much difference
        return;
    }

    if (!(record->m_fFlags & LC_ALIVE)) {
        // player must be alive, lost track
        return;
    }

    if (record == nullptr)
        return;

    // there is now no reason to interpolate records - so all of that cruft can be removed

    Vector org;
    Vector minsPreScaled;
    Vector maxsPreScaled;
    QAngle ang;

    {
        // we found the exact record or no other record to interpolate with
        // just copy these values since they are the best we have
        org           = record->m_vecOrigin;
        ang           = record->m_vecAngles;
        minsPreScaled = record->m_vecMinsPreScaled;
        maxsPreScaled = record->m_vecMaxsPreScaled;
    }

    static ConVar *sv_unlag_fixstuck = gInts->Cvar->FindVar("sv_unlag_fixstuck");
    // See if this is still a valid position for us to teleport to
    if (sv_unlag_fixstuck->GetBool()) {
        // Try to move to the wanted position from our current position.
        trace_t tr;
        UTIL_TraceEntity(player, org, org, MASK_PLAYERSOLID, &tr);
        if (tr.startsolid || tr.allsolid) {
            // if (sv_unlag_debug->GetBool())
            Log::Console("WARNING: BackupPlayer trying to back player into a bad position - client %s\n",
                         gInts->Engine->GetPlayerInfo(player->GetIndex()).name);

            CBaseEntity *pHitPlayer = (CBaseEntity *)tr.m_pEnt;

            // don't lag compensate the current player
            if (pHitPlayer && (pHitPlayer != m_pCurrentPlayer)) {
                // If we haven't backtracked this player, do it now
                // this deliberately ignores WantsLagCompensationOnEntity.
                if (!m_RestorePlayer.Get(pHitPlayer->GetIndex() - 1)) {
                    // prevent recursion - save a copy of m_RestorePlayer,
                    // pretend that this player is off-limits
                    int pl_index = player->GetIndex() - 1;

                    // Temp turn this flag on
                    m_RestorePlayer.Set(pl_index);

                    BacktrackPlayer(pHitPlayer);

                    // Remove the temp flag
                    m_RestorePlayer.Clear(pl_index);
                }
            }

            // now trace us back as far as we can go
            UTIL_TraceEntity(player, player->GetRenderOrigin(), org, MASK_PLAYERSOLID, &tr);

            if (tr.startsolid || tr.allsolid) {
                // Our starting position is bogus

                // if (sv_unlag_debug.GetBool())
                Log::Console("Backtrack failed completely, bad starting position");
            } else {
                // We can get to a valid place, but not all the way to the
                // target
                Vector vPos;
                VectorLerp(player->GetRenderOrigin(), org, tr.fraction * g_flFractionScale, vPos);

                // This is as close as we're going to get
                org = vPos;

                // if (sv_unlag_debug.GetBool())
                Log::Console("Backtrack got most of the way");
            }
        }
    }

    // See if this represents a change for the player
    int        flags   = 0;
    LagRecord *restore = &m_RestoreData[pl_index];
    LagRecord *change  = &m_ChangeData[pl_index];

    QAngle angdiff = player->GetAbsAngles() - ang;
    Vector orgdiff = player->GetRenderOrigin() - org;

    // Always remember the pristine simulation time in case we need to restore
    // it.
    restore->m_flSimulationTime = player->GetSimulationTime();

    // shouldnt we just do this all the time?
    if (angdiff.LengthSqr() > LAG_COMPENSATION_EPS_SQR) {
        flags |= LC_ANGLES_CHANGED;
        restore->m_vecAngles = player->GetAbsAngles();
        player->SetAbsAngles(ang);
        // player->SetAbsAngles(ang);
        change->m_vecAngles = ang;
    }

    // Use absolute equality here
    {
        Vector prescaledMin = player->GetCollideableMinsPrescaled();
        Vector prescaledMax = player->GetCollideableMaxsPrescaled();
        if (minsPreScaled != prescaledMin || maxsPreScaled != prescaledMax) {
            flags |= LC_SIZE_CHANGED;

            restore->m_vecMinsPreScaled = prescaledMin;
            restore->m_vecMaxsPreScaled = prescaledMax;

            player->SetSize(minsPreScaled, maxsPreScaled);
            // Log::Console("Tell F1ssi0N To Set size: Backtrack player!");

            change->m_vecMinsPreScaled = minsPreScaled;
            change->m_vecMaxsPreScaled = maxsPreScaled;
        }
    }

    // Note, do origin at end since it causes a relink into the k/d tree
    if (orgdiff.LengthSqr() > LAG_COMPENSATION_EPS_SQR) {
        flags |= LC_ORIGIN_CHANGED;
        restore->m_vecOrigin      = player->GetRenderOrigin();
        player->GetRenderOrigin() = org;
        player->SetAbsOrigin(org);
        change->m_vecOrigin = org;
    }

    auto *animating = (DT_BaseAnimating *)player;

    // Sorry for the loss of the optimization for the case of people
    // standing still, but you breathe even on the server.
    // This is quicker than actually comparing all bazillion floats.

    flags |= LC_ANIMATION_CHANGED;

    restore->m_masterSequence = player->GetSequence();
    restore->m_masterCycle    = player->GetCycle();

    // actually set the backed up cycle value
    // cycle is ALWAYS 0
    player->GetSequence() = record->m_masterSequence;
    player->SetCycle(record->m_masterCycle);

    ////////////////////////
    // Now do all the layers
    int layerCount = player->GetNumAnimOverlays();
    // int layerCount = 15;
    for (int layerIndex = 0; layerIndex < layerCount; ++layerIndex) {
        C_AnimationLayer *currentLayer = player->GetAnimOverlay(layerIndex);
        if (currentLayer) {
            restore->m_layerRecords[layerIndex].m_cycle    = currentLayer->m_flCycle;
            restore->m_layerRecords[layerIndex].m_order    = currentLayer->m_nOrder;
            restore->m_layerRecords[layerIndex].m_sequence = currentLayer->m_nSequence;
            restore->m_layerRecords[layerIndex].m_weight   = currentLayer->m_flWeight;

            currentLayer->m_flCycle   = record->m_layerRecords[layerIndex].m_cycle;
            currentLayer->m_nOrder    = record->m_layerRecords[layerIndex].m_order;
            currentLayer->m_nSequence = record->m_layerRecords[layerIndex].m_sequence;
            currentLayer->m_flWeight  = record->m_layerRecords[layerIndex].m_weight;
        }
    }

    const CStudioHdr *hdr = player->GetStudioHdr();

    if (hdr != nullptr) {
        auto *poseParameters = animating->flPoseParameter();

        for (int paramIndex = 0; paramIndex < (*(studiohdr_t **)hdr)->numlocalposeparameters; paramIndex++) {
            float poseParameter        = record->m_poseParameters[paramIndex];
            poseParameters[paramIndex] = poseParameter;
        }
    }

    player->UpdateAnimation();

    // if (sv_lagflushbonecache.GetBool())
    //	player->InvalidateBoneCache();

    // char text[256]; Q_snprintf(text, sizeof(text), "time %.2f",
    // flTargetTime); player->DrawServerHitboxes(10);
    // gInts->DebugOverlay->AddTextOverlay(org, 0, 10.0f, text);

    if (flags) {
        m_RestorePlayer.Set(pl_index); // remember that we changed this player
        m_bNeedToRestore  = true;      // we changed at least one player
        restore->m_fFlags = flags;     // we need to restore these flags
        change->m_fFlags  = flags;     // we have changed these flags
    }

    if (f1_debuglagcompensation.GetBool()) {
        gInts->DebugOverlay->AddEntityTextOverlay(player->GetIndex(), 0, 0.0f, 255, 255, 255, 255, " choke %d", record->m_nChokedTicks);
        gInts->DebugOverlay->AddEntityTextOverlay(player->GetIndex(), 1, 0.0f, 255, 255, 255, 255, " choke %f", record->m_flChokedTime);
        gInts->DebugOverlay->AddEntityTextOverlay(player->GetIndex(), 2, 0.0f, 255, 255, 255, 255, "   sim %f", record->m_flSimulationTime);
        gInts->DebugOverlay->AddEntityTextOverlay(player->GetIndex(), 3, 0.0f, 255, 255, 255, 255, "attack %d", record->buttons & IN_ATTACK);
    }
}

#include "../SDK/Util.hh"
#include "TargetManager.hh"

ConVar f1_bt_tick_count_add("f1_bt_tick_count_add", "0", 0, "");

void CLagCompensationManager::FinishLagCompensation(CBaseEntity *player)
{
    F1_VPROF_FUNCTION();
    // TODO: do we really need to restore entity positions?

    m_pCurrentPlayer = NULL;

    if (f1_interpolate.GetInt() == 0 || f1_interpolate.GetInt() == 1) {

        const CBaseEntity *target = nullptr;

        for (TargetManagerBase *manager = TargetManagerBase::Head(); manager != nullptr; manager = manager->Next()) {
            if (manager->can_backtrack()) {
                target = manager->get_best_target().ent;
                break;
            }
        }
        float chokedTime = 0.0f;

        if (target != nullptr) {
            LagRecord *lastRecord = GetLastRecord(target->GetIndex());

            if (lastRecord == nullptr) {
                Log::Console("No record for target entity - tell f1ssi0n");
                return;
            }

            int tick_count = f1_bt_tick_count_add.GetInt();
            // use sim time as the tickbase
            if (f1_interpolate.GetInt() == 1) {
                tick_count += m_pCurrentCommand->tick_count;
            } else if (f1_interp.GetBool() == true) {
                tick_count += TIME_TO_TICKS(lastRecord->m_flSimulationTime);
            } else {
                tick_count += m_pCurrentCommand->tick_count;
            }

            float correct = 0.0f;

            correct += GetIncomingLatency();

            int lerpTicks;

            if (f1_interpolate.GetInt() == 1) {
                lerpTicks = 0;
            } else if (f1_useplusone.GetBool()) {
                lerpTicks = TIME_TO_TICKS(GetInterpolationAmount());
            } else {
                lerpTicks = TIME_TO_TICKS(GetClientInterpAmount());
            }

            correct += TICKS_TO_TIME(lerpTicks);

            // the correction is always positive - check the SSDK
            static ConVar *sv_maxunlag = gInts->Cvar->FindVar("sv_maxunlag");
            correct                    = Clamp(correct, 0.0f, sv_maxunlag->GetFloat());

            int targetTick = tick_count + lerpTicks;

            float deltaTime = correct - TICKS_TO_TIME(tick_count - targetTick);

            if (fabs(deltaTime) > 0.2f) {
                Log::Console("Deltatime to big (%.2f)", deltaTime);
                targetTick = tick_count + TIME_TO_TICKS(correct);
            }

            int delta = targetTick - m_pCurrentCommand->tick_count;

            if (f1_debuglagcompensation.GetBool())
                Log::Console("cur %d %f new %d %f del %d %f dt %f",
                             m_pCurrentCommand->tick_count,
                             TICKS_TO_TIME(m_pCurrentCommand->tick_count),
                             targetTick, TICKS_TO_TIME(targetTick),
                             delta,
                             TICKS_TO_TIME(delta), deltaTime);

            m_pCurrentCommand->tick_count = targetTick;

            if (f1_dontshootifnotattack.GetBool() == true) {
                if (lastRecord != nullptr) {
                    if ((lastRecord->buttons & IN_ATTACK) == false)
                        m_pCurrentCommand->buttons &= ~IN_ATTACK;
                }
            }

            if (f1_showlagcompensation.GetBool() && m_pCurrentCommand->buttons & IN_ATTACK) {
                if (!bulletTime(GetLocalPlayer()))
                    DrawClientHitboxes(target, 4, true);
            }
        }
    } else {
        if (f1_useplusone.GetBool()) {
            m_pCurrentCommand->tick_count += 1;
        }
    }

    if (!m_bNeedToRestore) {
        m_isCurrentlyDoingCompensation = false;
        return; // no player was changed at all
    }

    // restore all player positions now that we have finished
    for (int i = 1; i <= gInts->Engine->GetMaxClients(); i++) {
        int pl_index = i - 1;

        if (!m_RestorePlayer.Get(pl_index)) {
            continue;
        }

        CBaseEntity *player = GetBaseEntity(i);
        if (player == nullptr) {
            continue;
        }

        LagRecord *restore = &m_RestoreData[pl_index];
        LagRecord *change  = &m_ChangeData[pl_index];

        bool restoreSimulationTime = false;

        if (restore->m_fFlags & LC_SIZE_CHANGED) {
            restoreSimulationTime = true;

            if (player->GetCollideableMinsPrescaled() == change->m_vecMinsPreScaled && player->GetCollideableMaxsPrescaled() == change->m_vecMaxsPreScaled) {
                player->SetSize(restore->m_vecMinsPreScaled, restore->m_vecMaxsPreScaled);
            }
        }

        if (restore->m_fFlags & LC_ANGLES_CHANGED) {
            restoreSimulationTime = true;

            {
                player->SetAbsAngles(restore->m_vecRealAngles);
                player->SetEyeAngles(restore->m_vecRealAngles);
            }
        }

        if (restore->m_fFlags & LC_ORIGIN_CHANGED) {
            restoreSimulationTime = true;

            Vector delta = player->GetRenderOrigin() - change->m_vecOrigin;

            // If it moved really far, just leave the player in the new spot!!!
            if (delta.Length2DSqr() < m_flTeleportDistanceSqr) {
                // TODO: undo
                RestorePlayerTo(player, restore->m_vecOrigin + delta);
            }
        }

        player->SetSequence(restore->m_masterSequence);
        player->SetCycle(restore->m_masterCycle);

        restoreSimulationTime = true;

        int layerCount = player->GetNumAnimOverlays();
        for (int layerIndex = 0; layerIndex < layerCount; ++layerIndex) {
            C_AnimationLayer *currentLayer = player->GetAnimOverlay(layerIndex);
            if (currentLayer) {
                currentLayer->m_flCycle   = restore->m_layerRecords[layerIndex].m_cycle;
                currentLayer->m_nOrder    = restore->m_layerRecords[layerIndex].m_order;
                currentLayer->m_nSequence = restore->m_layerRecords[layerIndex].m_sequence;
                currentLayer->m_flWeight  = restore->m_layerRecords[layerIndex].m_weight;
            }
        }

        if (restoreSimulationTime) {
            player->SetSimulationTime(restore->m_flSimulationTime);
        }
    }

    m_isCurrentlyDoingCompensation = false;

    m_pCurrentCommand = NULL;
}
