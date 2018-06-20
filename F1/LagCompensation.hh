#pragma once

#include "../SDK/SDK.hh"

#define MAX_PLAYERS (33)

#define LC_NONE 0
#define LC_ALIVE (1 << 0)

#define LC_ORIGIN_CHANGED (1 << 8)
#define LC_ANGLES_CHANGED (1 << 9)
#define LC_SIZE_CHANGED (1 << 10)
#define LC_ANIMATION_CHANGED (1 << 11)

// these are hardcoded for tf2
#define MAX_LAYER_RECORDS (15)

#define MAX_POSE_PARAMETERS (24)

struct LayerRecord
{
	int   m_sequence;
	float m_cycle;
	float m_weight;
	int   m_order;

	float m_flLayerAnimtime;
	float m_flLayerFadeOuttime;

	float m_flBlendIn;
	float m_flBlendOut;

	bool m_bClientBlend;

	float m_flPlaybackRate;

	LayerRecord ()
	{
		m_sequence = 0;
		m_cycle    = 0;
		m_weight   = 0;
		m_order    = 0;
	}

	LayerRecord (const LayerRecord &src)
	{
		m_sequence = src.m_sequence;
		m_cycle    = src.m_cycle;
		m_weight   = src.m_weight;
		m_order    = src.m_order;

		m_flLayerAnimtime    = src.m_flLayerAnimtime;
		m_flLayerFadeOuttime = src.m_flLayerFadeOuttime;

		m_flBlendIn  = src.m_flBlendIn;
		m_flBlendOut = src.m_flBlendOut;

		m_bClientBlend = src.m_bClientBlend;

		m_flPlaybackRate = src.m_flPlaybackRate;
	}
};

struct LagRecord
{
public:
	LagRecord ()
	{
		m_fFlags = 0;
		m_vecOrigin.Init ();
		m_vecAngles.Init ();
		m_vecRealAngles.Init ();
		m_vecMinsPreScaled.Init ();
		m_vecMaxsPreScaled.Init ();
		m_flSimulationTime = -1;
		m_flChokedTime     = 0;
		m_nChokedTicks     = 0;
		m_masterSequence   = 0;
		m_masterCycle      = 0;
	}

	LagRecord (const LagRecord &src)
	{
		m_fFlags           = src.m_fFlags;
		m_vecOrigin        = src.m_vecOrigin;
		m_vecAngles        = src.m_vecAngles;
		m_vecRealAngles    = src.m_vecRealAngles;
		m_vecMinsPreScaled = src.m_vecMinsPreScaled;
		m_vecMaxsPreScaled = src.m_vecMaxsPreScaled;
		m_flSimulationTime = src.m_flSimulationTime;
		m_flChokedTime     = src.m_flChokedTime;
		m_nChokedTicks     = src.m_nChokedTicks;
		for (int layerIndex = 0; layerIndex < MAX_LAYER_RECORDS; ++layerIndex) {
			m_layerRecords[layerIndex] = src.m_layerRecords[layerIndex];
		}
		m_masterSequence = src.m_masterSequence;
		m_masterCycle    = src.m_masterCycle;
		memcpy (m_poseParameters, src.m_poseParameters, sizeof (float) * MAX_POSE_PARAMETERS);
	}

	// Did player die this frame
	int m_fFlags;

	// Player position, orientation and bbox
	Vector m_vecOrigin;
	QAngle m_vecAngles;
	QAngle m_vecRealAngles;
	Vector m_vecMinsPreScaled;
	Vector m_vecMaxsPreScaled;

	float m_flSimulationTime;

	float m_flChokedTime;
	int   m_nChokedTicks;

	int buttons;

	// Player animation details, so we can get the legs in the right spot.
	LayerRecord m_layerRecords[MAX_LAYER_RECORDS];
	int         m_masterSequence;
	float       m_masterCycle;
	float       m_poseParameters[MAX_POSE_PARAMETERS];
};

float GetClientInterpAmount ();
int   GetClientInterpTicks ();

// varients with +1
int   GetInterpolationTicks ();
float GetInterpolationAmount ();

abstract_class ILagCompensationManager
{
public:
	// Called during player movement to set up/restore after lag compensation
	virtual void StartLagCompensation (CBaseEntity * player, CUserCmd * cmd) = 0;
	virtual void FinishLagCompensation (CBaseEntity * player)                = 0;
	virtual bool UpdatePlayer (CBaseEntity * player)                         = 0;
	virtual bool IsCurrentlyDoingLagCompensation () const                    = 0;

	// All of these function are only valid during lag compensation
	virtual void       HandleNewCommand (CBaseEntity * player, CUserCmd * pCommand) = 0;
	virtual LagRecord *GetLastRecord (int index)                                    = 0;
	virtual float      GetPlayerTime (CBaseEntity * player)                         = 0;

	virtual void Init ()                       = 0;
	virtual void Shutdown ()                   = 0;
	virtual void FrameUpdatePostEntityThink () = 0;
	virtual void FrameUpdateRenderStart ()     = 0;
};
