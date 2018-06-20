#include "stdafx.hh"

#include "../SDK/CDrawManager.hh"
#include "../SDK/SDK.hh"
#include "CritHelper.hh"

#include <tier0/memdbgon.h>

struct WeaponData_t
{
	int   m_nDamage;
	int   m_nBulletsPerShot;
	float m_flRange;
	float m_flSpread;
	float m_flPunchAngle;
	float m_flTimeFireDelay;   // Time to delay between firing
	float m_flTimeIdle;        // Time to idle after firing
	float m_flTimeIdleEmpty;   // Time to idle after firing last bullet in clip
	float m_flTimeReloadStart; // Time to start into a reload (ie. shotgun)
	float m_flTimeReload;      // Time to reload
	bool  m_bDrawCrosshair;    // Should the weapon draw a crosshair
	int   m_iProjectile;       // The type of projectile this mode fires
	int   m_iAmmoPerShot;      // How much ammo each shot consumes
	float m_flProjectileSpeed; // Start speed for projectiles (nail, etc.); NOTE: union with something non-projectile
	float m_flSmackDelay;      // how long after swing should damage happen for melee weapons
	bool  m_bUseRapidFireCrits;

	void Init(void)
	{
		m_nDamage            = 0;
		m_nBulletsPerShot    = 0;
		m_flRange            = 0.0f;
		m_flSpread           = 0.0f;
		m_flPunchAngle       = 0.0f;
		m_flTimeFireDelay    = 0.0f;
		m_flTimeIdle         = 0.0f;
		m_flTimeIdleEmpty    = 0.0f;
		m_flTimeReloadStart  = 0.0f;
		m_flTimeReload       = 0.0f;
		m_iProjectile        = 0;
		m_iAmmoPerShot       = 0;
		m_flProjectileSpeed  = 0.0f;
		m_flSmackDelay       = 0.0f;
		m_bUseRapidFireCrits = false;
	};
};

// Reason for the critical hit result
enum iscrit_t
{
	// Server observed us firing too many crits
	ISCRIT_OBSERVED_CAP = -5,
	// Crit bucket is empty
	ISCRIT_BUCKET_EMPTY = -4,
	// CanFireCritShot() returns false
	ISCRIT_CANNOT_CRIT = -3,
	// Seed reused
	ISCRIT_REUSED_SEED = -2,
	// Just finished streaMing crits, wait 1 sec
	ISCRIT_STREAM_WAIT = -1,
	// Next  crit is deterMined by random chance
	ISCRIT_RANDOM = 0,
	// Currently critboosted
	ISCRIT_BOOSTED = 3,
	// Currently streaMing crits
	ISCRIT_STREAMinG = 4,
};

// Helper to calculate crits
class CTF2CritHelper
{
public:
	CTF2CritHelper();
	void init();
	void Setup(int seqnr);
	bool IsCrit(iscrit_t &crit);

	bool  CanFireCriticalShot();
	bool  CanFireRandomCriticalShot(float flChance);
	float AttackDamage();
	void  AddCritBucket(float flDamage);
	bool  BlowCritBucket(float flDamage);

	float GetCritBucket();

	friend class CCritHelper;

private:
	// State
	CBaseEntity *       pMe;
	CBaseCombatWeapon * pWeapon;
	const WeaponData_t *data;
	bool                bIsMelee;
	bool                bStreamCrits;

	float curtime;
	int   iRandomSeed;

	// Game keeps track of these stats
	struct stats_t
	{
		float flCritBucket; // A54
		int   iNumAttacks;  // A58
		int   iNumCrits;    // A5C
	} stats;

	// Memory layout for C_TFWeaponBase
	struct state_t
	{
		bool  bCurrentAttackIsCrit;
		bool  bLowered;                  // NETVAR 0xB40
		int   iAltFireHint;              // 0xB44
		int   iReloadStartClipAmount;    // 0xB48
		float flCritEndTime;             // 0xB4C
		float flLastCritCheckTime;       // NETVAR // 0xB50
		int   iLastCritCheckFrame;       // 0xB54
		int   iCurrentSeed;              // 0xB58
		float flLastCritCheckTimeNoPred; // 0xB5C
	};

	inline state_t &GetState()
	{
		NETVAR(n, bool, "DT_TFWeaponBase", "m_bLowered");
		return pWeapon->get<state_t &>(n.GetOffset() - 0x4);
	}

	// ConVars
	static const float tf_weapon_criticals_bucket_cap;
	static const float tf_weapon_criticals_bucket_bottom;
	static const bool  tf_weapon_criticals_nopred;
};

const float CTF2CritHelper::tf_weapon_criticals_bucket_cap    = 1000.0f;
const float CTF2CritHelper::tf_weapon_criticals_bucket_bottom = -250.0f;
const bool  CTF2CritHelper::tf_weapon_criticals_nopred        = true;

CTF2CritHelper::CTF2CritHelper()
{
}
void CTF2CritHelper::init()
{
	pMe          = (GetLocalPlayer());
	pWeapon      = (pMe->GetActiveWeapon());
	data         = (pWeapon->GetWeaponData());
	bIsMelee     = (CEntTag(pWeapon).isMelee());
	bStreamCrits = (data->m_bUseRapidFireCrits);
	curtime      = (pMe->GetTickBase() * gInts->Globals->interval_per_tick);
	// replace with sig
	// addtocritbucket: 55 8B EC A1 ? ? ? ? F3 0F 10 81 ? ? ? ? F3 0F 10 48
	// static DWORD dwCritBucketOffset = gSignatures.GetClientSignature("F3 0F 10 81 ? ? ? ? F3 0F 10 48 ? 0F 2F C8") + 4;
	// stats.flCritBucket = (float *)(pWeapon + 0xA54);
}
void CTF2CritHelper::Setup(int seqnr)
{
	// Copy the current stats
	stats = pWeapon->get<stats_t>(0xA54);
	// Calculate the random seed
	int seedmod = (pWeapon->GetIndex() << 8) | pMe->GetIndex();
	if (bIsMelee)
		seedmod <<= 8;
	iRandomSeed = MD5_PseudoRandom(seqnr) ^ seedmod;
}
bool CTF2CritHelper::IsCrit(iscrit_t &crit)
{

	// maybe use a function hook here?
	// this is on the vtable for C_TFWeaponBase or C_TFWeaponBaseMelee

	// pl = pWeapon->GetOwner() and dynamic_cast to C_TFPlayer

	// pl->IsPlayer() (why?!)

	// pWeapon->CanFireCriticalShot()
	if (!CanFireCriticalShot()) {
		crit = ISCRIT_CANNOT_CRIT;
		return false;
	}

	if (pMe->IsCritBoosted()) {
		crit = ISCRIT_BOOSTED;
		return true;
	}

	float flCritMult = pMe->GetCritMult();

	state_t &state = GetState();

	if (bIsMelee) {
		flCritMult = pWeapon->GetAttributeFloat(flCritMult * 0.15f, "mult_crit_chance");

		if (iRandomSeed == state.iCurrentSeed) {
			// Dunno predict what will happen...
			crit = ISCRIT_REUSED_SEED;
			return false;
		}
		RandomSeed(iRandomSeed);

		// Game does some logic with pMe->m_iNextMeleeCrit (NETVAR)
		// int iNextMeleeCrit = pMe->iNextMeleeCrit();
		// state.bCurrentAttackIsCrit = iNextMeleeCrit!=0;
		// if ( iNextMeleeCrit==2 )
		//{
		//    return true;
		//}

		float flDamage = AttackDamage();
		AddCritBucket(flDamage);

		stats.iNumAttacks++;

		bool result = static_cast<float>(RandomInt(0, 9999)) <= (flCritMult * 10000.0f);

		if (result && !BlowCritBucket(flDamage)) {
			crit = ISCRIT_BUCKET_EMPTY;
			return false;
		} else {
			crit = ISCRIT_RANDOM;
			return result;
		}
	} else {
		// Still streaMing crits
		if (bStreamCrits && state.flCritEndTime > curtime) {
			crit = ISCRIT_STREAMinG;
			return true;
		}

		float flDamage = AttackDamage();
		AddCritBucket(flDamage);

		//[ebp+var_1] = 0
		bool result = false;

		//[pWeapon+0xAAB] = 1

		if (bStreamCrits) {
			if (tf_weapon_criticals_nopred) {
				if (state.flLastCritCheckTimeNoPred + 1.0f > curtime) {
					crit = ISCRIT_STREAM_WAIT;
					return false;
				}
				// state.flLastCritCheckTimeNoPred = curtime;
			} else {
				if (state.flLastCritCheckTime + 1.0f > curtime) {
					crit = ISCRIT_STREAM_WAIT;
					return false;
				}
				// if ( state.flLastCritCheckTime!=curtime )
				//{
				//    state.flLastCritCheckTime = curtime;
				//}
			}

			// Modify flCritMult
			flCritMult = 1.0f / (2.0f / (flCritMult * 0.02f) - 2.0f);
			flCritMult = pWeapon->GetAttributeFloat(flCritMult, "mult_crit_chance");

			if (iRandomSeed == state.iCurrentSeed) {
				// Dunno predict what will happen...
				crit = ISCRIT_REUSED_SEED;
				return false;
			}
			RandomSeed(iRandomSeed);

			result = static_cast<float>(RandomInt(0, 9999)) <= (flCritMult * 10000.0f);
		} else {
			// Modify flCritMult
			flCritMult = pWeapon->GetAttributeFloat(flCritMult * 0.02f, "mult_crit_chance");

			if (iRandomSeed == state.iCurrentSeed) {
				// Dunno predict what will happen...
				crit = ISCRIT_REUSED_SEED;
				return false;
			}
			RandomSeed(iRandomSeed);

			result = static_cast<float>(RandomInt(0, 9999)) < (flCritMult * 10000.0f);
		}

		//if (gInts->Prediction->()) {
		//	stats.iNumAttacks++;
		//}

		crit = ISCRIT_RANDOM;

		if (result) {
			if (CanFireRandomCriticalShot(flCritMult)) {
				// Adjust for stream crits
				if (bStreamCrits) {
					flDamage = 2.0f / data->m_flTimeFireDelay * flDamage;

					if ((flDamage * 3.0f) > tf_weapon_criticals_bucket_cap) {
						flDamage = tf_weapon_criticals_bucket_cap / 3.0f;
					}
				}

				result = BlowCritBucket(flDamage);

				if (!result)
					crit = ISCRIT_BUCKET_EMPTY;

				// if ( result )
				//{
				//    state.flCritEndTime = curtime + 2.0f;
				//}
			} else {
				crit   = ISCRIT_OBSERVED_CAP;
				result = false;
			}
		}

		return result;
	}
}
bool CTF2CritHelper::CanFireCriticalShot()
{
	typedef bool(__thiscall * OriginalFn)(CBaseCombatWeapon *, int);
	return getvfunc<OriginalFn>(pWeapon, 417)(pWeapon, 0);
}
bool CTF2CritHelper::CanFireRandomCriticalShot(float flChance)
{
	float flObservedCritCap = flChance + 0.1f;
	return pWeapon->GetObservedCritChance() <= flObservedCritCap;
}
float CTF2CritHelper::AttackDamage()
{
	int   nBulletsPerShot = bIsMelee ? 1 : static_cast<int>(pWeapon->GetAttributeFloat(static_cast<float>(data->m_nBulletsPerShot), "mult_bullets_per_shot"));
	float flDamage        = pWeapon->GetAttributeFloat(static_cast<float>(data->m_nDamage), "mult_dmg") * static_cast<float>(nBulletsPerShot);
	return flDamage;
}
void CTF2CritHelper::AddCritBucket(float flDamage)
{
	if (stats.flCritBucket < tf_weapon_criticals_bucket_cap) {
		stats.flCritBucket = Min(stats.flCritBucket + flDamage, tf_weapon_criticals_bucket_cap);
	}
}
bool CTF2CritHelper::BlowCritBucket(float flDamage)
{
	stats.iNumCrits++;

	// Calculate the cost of firing a critical hit
	float flCostMult;
	if (bIsMelee) {
		flCostMult = 0.5f;
	} else {
		float flCritRatio = static_cast<float>(stats.iNumCrits) / static_cast<float>(stats.iNumAttacks);
		flCostMult        = (flCritRatio - 0.1f) / 0.9f;
		flCostMult        = Max(flCostMult, 0.0f);
		flCostMult        = Min(flCostMult, 1.0f);
		flCostMult        = flCostMult * 2.0f + 1.0f;
	}

	// Cost of firing a critical hit
	float flCritCost = flDamage * 3.0f * flCostMult;

	// If not enough damage stored, deny crit
	if (flCritCost > stats.flCritBucket) {
		return false;
	}

	// RemoveFromCritBucket()
	stats.flCritBucket = Max(stats.flCritBucket - flCritCost, tf_weapon_criticals_bucket_bottom);

	return true;
}

float CTF2CritHelper::GetCritBucket()
{
	return stats.flCritBucket;
}

CTF2CritHelper gTFCritHelper;

void CCritHelper::init()
{
	gInts->EventManager->AddListener(this, "player_hurt", false);
}

iscrit_t lastCrit = ISCRIT_CANNOT_CRIT;

int latestCmd = 0;

void CCritHelper::processCommand(CUserCmd *pCommand)
{
	CBaseEntity *pActiveWeapon = GetLocalPlayer()->GetActiveWeapon();
	if (pActiveWeapon != nullptr) {
		gTFCritHelper.init();

		if (critButton.Value() && pCommand->buttons & IN_ATTACK) {
			// backup the important stuff
			CTF2CritHelper::state_t state;
			CTF2CritHelper::stats_t stats;

			memcpy(&state, &gTFCritHelper.GetState(), sizeof(CTF2CritHelper::state_t));
			memcpy(&state, &gTFCritHelper.stats, sizeof(CTF2CritHelper::stats_t));

			// REGENERATE
			if (latestCmd)
				for (int i = 0; i < 200; i++) {
					int newcmd = pCommand->command_number + i;
					gTFCritHelper.Setup(newcmd);

					iscrit_t &iscrit = lastCrit;

					if (gTFCritHelper.IsCrit(iscrit)) {
						// we found a critable cmdn

						// assign our command number to this cmdn
						pCommand->command_number = newcmd;

						if (fireIfCrit.Value())
							pCommand->buttons |= IN_ATTACK;

						break;
					} else if (this->onlyFireIfCrit.Value()) {
						pCommand->buttons &= IN_ATTACK;
					}
				}

			// restore the state
			memcpy(&gTFCritHelper.GetState(), &state, sizeof(CTF2CritHelper::state_t));
			memcpy(&gTFCritHelper.stats, &stats, sizeof(CTF2CritHelper::stats_t));
		}
	}
}

bool CCritHelper::paint()
{
	int y = 300;

	//gDrawManager.DrawString("hud", 0, y += gDrawManager.GetHudHeight(), COLOR_LINE, "Bucket: %f", gTFCritHelper.GetCritBucket());

	//switch (lastCrit) {
	//case ISCRIT_OBSERVED_CAP:
	//	gDrawManager.DrawString("hud", 0, y += gDrawManager.GetHudHeight(), COLOR_LINE, "last: cap");
	//	break;
	//case ISCRIT_BUCKET_EMPTY:
	//	gDrawManager.DrawString("hud", 0, y += gDrawManager.GetHudHeight(), COLOR_LINE, "last: empty");
	//	break;
	//case ISCRIT_CANNOT_CRIT:
	//	gDrawManager.DrawString("hud", 0, y += gDrawManager.GetHudHeight(), COLOR_LINE, "last: cannot");
	//	break;
	//case ISCRIT_REUSED_SEED:
	//	gDrawManager.DrawString("hud", 0, y += gDrawManager.GetHudHeight(), COLOR_LINE, "last: reused");
	//	break;
	//case ISCRIT_STREAM_WAIT:
	//	gDrawManager.DrawString("hud", 0, y += gDrawManager.GetHudHeight(), COLOR_LINE, "last: wait");
	//	break;
	//case ISCRIT_RANDOM:
	//	gDrawManager.DrawString("hud", 0, y += gDrawManager.GetHudHeight(), COLOR_LINE, "last: random");
	//	break;
	//case ISCRIT_BOOSTED:
	//	gDrawManager.DrawString("hud", 0, y += gDrawManager.GetHudHeight(), COLOR_LINE, "last: boosted");
	//	break;
	//case ISCRIT_STREAMinG:
	//	gDrawManager.DrawString("hud", 0, y += gDrawManager.GetHudHeight(), COLOR_LINE, "last: streaMing");
	//	break;
	//default:
	//	break;
	//}

	return false;
}

void CCritHelper::FireGameEvent(IGameEvent *event)
{
	// if (!strcmp(event->GetName(), "player_hurt"))
	//{
	//  int localUserID = gInts->Engine->GetPlayerInfo(me).userID;
	//  if (event->GetInt("attacker") == localUserID)
	//  {
	//      float damage = event->GetFloat("damageamount", 0);
	//      gTFCritHelper.AddCritBucket(damage);
	//  }
	//}
}

CCritHelper gCritHelper;
