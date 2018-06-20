#pragma once

#include "projectile.hh"

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

	void Init (void)
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

class WeaponHelper
{
	CBaseEntity *       owner;
	CBaseCombatWeapon * weapon;
	const WeaponData_t *data;
	float               speed;
	bool                isProjectile;

public:
	WeaponHelper (CBaseCombatWeapon *weapon)
	    : weapon (weapon), 
		owner (weapon ? weapon->GetOwner () : nullptr), 
		data (weapon ? weapon->GetWeaponData () : nullptr)
	{
		if (owner == nullptr || weapon == nullptr || data == nullptr) {
			Log::Console ("Weapon helper nullptrs!");
			return;
		}

		speed        = projectileHelper::GetProjectileSpeed (weapon);
		isProjectile = speed > 0.0f;

		//Log::Console ("speed %f %s", speed, isProjectile ? "true" : "false");
	}

	float AttackDamage ()
	{
		if (weapon == nullptr)
			return 0.0f;

		int   nBulletsPerShot = CEntTag (weapon).isMelee () ? 1 : static_cast<int> (weapon->GetAttributeFloat (static_cast<float> (data->m_nBulletsPerShot), "mult_bullets_per_shot"));
		float flDamage        = weapon->GetAttributeFloat (static_cast<float> (data->m_nDamage), "mult_dmg") * static_cast<float> (nBulletsPerShot);
		return flDamage;
	}

	float ProjectileSpeed ()
	{
		return speed;
	}

	bool IsProjectile ()
	{
		return isProjectile;
	}

	WeaponHelper &operator= (const WeaponHelper &other)
	{
		owner  = other.owner;
		weapon = other.weapon;
		data   = other.data;

		speed        = other.speed;
		isProjectile = other.isProjectile;

		return *this;
	}
};
