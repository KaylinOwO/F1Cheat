#include "stdafx.hh"

#include "Aimbot.hh"

#include "AimHelpers.hh"
#include "CHack.hh"
#include "F1_Cache.hh"
#include "F1_ConVar.hh"
#include "WeaponHelper.hh"
#include "projectile.hh"

#include <tier0/memdbgon.h>

Aimbot gAimbot;

bool Aimbot::is_player(const CBaseEntity *ent) const
{
    bool isPlayer = ent->GetClientClass()->classId == classId::CTFPlayer;

    if (isPlayer == false) {
        player_info_t info;
        return gInts->Engine->GetPlayerInfo(ent->GetIndex(), &info);
    }

    return true;
}

bool Aimbot::check_cond(const CBaseEntity *ent)
{
    int cond = ent->GetCond();

    // people we shouldnt hit
    // TODO: add more to this list
    // OR MAKE THAT COND WINDOW!
    if (cond & tf_cond::TFCond_Ubercharged || cond & tf_cond::TFCond_UberchargeFading || cond & tf_cond::TFCond_Bonked)
        return false;

    return true;
}

float Aimbot::get_rifle_damage()
{
    // this would get the attack damage of any weapon but its not important for any other weapon
    return local_weapon_helper.AttackDamage();
}

// TODO: global cache should be replaced by some kind of internal array
bool Aimbot::predict(const CBaseEntity *ent, CBaseCombatWeapon *localWeapon, Vector &localViewPos, Vector *correctionOut)
{
    if (local_weapon_helper.IsProjectile()) {
        F1_PredictionCache c = gCache.getPrediction(ent->GetIndex());

        if (c.isValid) {
            *correctionOut = c.prediction;
            return true;
        } else {
            // compute the prediction and store it for later so that we dont recalculate this alot
            Vector pred = projectileHelper::PredictCorrection(localWeapon, const_cast<CBaseEntity *>(ent), localViewPos, 1);

            if (pred != vec3_invalid) {
                *correctionOut = pred;
            } else {
                *correctionOut = vec3_origin; // (0, 0, 0)
                return false;
            }

            gCache.storePrediction(ent->GetIndex(), pred);
            return true;
        }
    }

    return false;
}

// TODO: move these over
ConVar f1_pedantic("f1_pedantic", "0", FCVAR_NONE, "Entities are only visible if the exact hitbox we are looking for is visible");
ConVar f1_projectileboundsmax("f1_projectileboundsmax", "1000", FCVAR_NONE, "The maximum distance that the end of the trace can be from the collideable to still be considered valid");

bool Aimbot::visible(const CBaseEntity *ent, Vector *vector, int hitboxToCheck)
{
    F1_VPROF_FUNCTION();

    //
    // this is called from threaded code so must contain all state to itself
    //

    trace_t     trace;
    Ray_t       ray;
    CBaseFilter filter;

    const CBaseEntity *pLocalEntity = GetLocalPlayer();
    Vector             localView    = pLocalEntity->GetViewPos();

    // TODO: this shouldnt get called when we are dead
    if (!pLocalEntity->IsAlive()) {
        Log::Console("visible shouldnt be called when we are dead!");
        return false;
    }

    filter.SetIgnoreEntity(pLocalEntity);

    // use the predicted origin the view offset is insignificant
    ray.Init(localView, *vector);

    // fire normally when:
    // 1. not using a projectile weapon
    // 2. target is not moving
    if (local_weapon_helper.IsProjectile() == false) {

        gInts->EngineTrace->TraceRay(ray, MASK_AIMBOT | CONTENTS_HITBOX, &filter, &trace);

        bool visible = false;

        if (trace.m_pEnt != nullptr && (CBaseEntity *)trace.m_pEnt != GetBaseEntity(0)) {
            if (f1_pedantic.GetBool()) {
                visible = ((CBaseEntity *)trace.m_pEnt == ent) && trace.hitbox == hitboxToCheck;
            } else {
                visible = ((CBaseEntity *)trace.m_pEnt == ent);
            }
        }

        return visible;
    } else {
        // if the targets position is different to the entities position, set
        // the entities position to the predicted position

        bool isvis = false;

        Vector origin = ent->GetAbsOrigin();
        Vector correction;

        if (predict((CBaseEntity *)ent, pLocalEntity->GetActiveWeapon(), localView, &correction) == false) {
            Log::Console("predict returned false");
        }

        Vector diff   = origin + correction;
        Vector newAim = *vector + correction;

        ray = Ray_t{};
        ray.Init(localView, newAim);

        // check if the player is visible using a traceray

        gInts->EngineTrace->TraceRay(ray, MASK_AIMBOT | CONTENTS_HITBOX, &filter, &trace);

        // check whether the end pos is within the bounds of the entity

        Vector collisionMin = ent->GetCollideableMins();
        Vector collisionMax = ent->GetCollideableMaxs();

        // transform the collideables by the delta change AND THE ORIGIN
        collisionMin += diff;
        collisionMax += diff;

        float dist = CalcSqrDistanceToAABB(collisionMin, collisionMax, trace.endpos);

        //Log::Console ("dist: %f", dist);

        // if we are within the bounds of the collideable then return true
        // TODO: account for splash damage
        if (dist < f1_projectileboundsmax.GetFloat()) {
            *vector = newAim;
            isvis   = true;
        }

        return isvis;
    }

    return false;
}

ConVar f1_multipoint_granularity("f1_multipoint_granularity", "5", 0, "sets the granularity for multipoint (5 will equal 10 steps accross) bigger numbers will be slower");

bool Aimbot::do_multipoint(const CBaseEntity *ent, float gran, Vector &min, Vector &centre, Vector &max, int hitboxToCheck, Vector *out)
{
    F1_VPROF_FUNCTION();

    // go from centre to centre min first
    for (float i = 0.0f; i <= 1.0f; i += gran) {
        Vector point = VectorLerp(centre, min, i);

        if (visible(ent, &point, hitboxToCheck)) {
            *out = point;
            return true;
        }
    }

    // now from centre to max
    // go from centre to centre min first
    for (float i = 0.0f; i <= 1.0f; i += gran) {
        Vector point = VectorLerp(centre, max, i);

        if (visible(ent, &point, hitboxToCheck)) {
            *out = point;
            return true;
        }
    }

    return false;
}

bool Aimbot::multipoint(const CBaseEntity *ent, Vector &centre, Vector &bbmin, Vector &bbmax, int hitboxToCheck, Vector *out)
{
    F1_VPROF_FUNCTION();

    // new multipoint begin
    float divisor = f1_multipoint_granularity.GetFloat();

    if (divisor == 0) {
        return false;
    }

    float granularity = 1.0f / divisor;

    //Log::Console ("%f", granularity);

    // this should make a plus shape along the hitbox

    Vector centreMinX = Vector(Lerp(0.5, bbmin.x, bbmax.x), bbmin.y, centre.z);
    Vector centreMaxX = Vector(Lerp(0.5, bbmin.x, bbmax.x), bbmax.y, centre.z);

    Vector centreMinY = Vector(bbmin.x, Lerp(0.5, bbmin.y, bbmax.y), centre.z);
    Vector centreMaxY = Vector(bbmax.x, Lerp(0.5, bbmin.y, bbmax.y), centre.z);

    if (do_multipoint(ent, granularity, centreMinX, centre, centreMaxX, hitboxToCheck, out) == true) {
        return true;
    } else if (do_multipoint(ent, granularity, centreMinY, centre, centreMaxY, hitboxToCheck, out) == true) {
        return true;
    }

    // TODO: check up + down if that is necessary

    return false;
}

bool Aimbot::find_hitbox(const CBaseEntity *ent, Vector *out)
{
    F1_VPROF_FUNCTION();

    hitboxes_t hitboxes;
    int        totalHitboxes = gCache.GetHitboxes(const_cast<CBaseEntity *>(ent), &hitboxes, MAXSTUDIOBONES, true);

    auto *localPlayer = GetLocalPlayer();
    auto *localWeapon = localPlayer->GetActiveWeapon();

    bool singleHitbox = false;

    // find the best hitbox for each class and weapon
    // by default, aim for torso
    auto bestHitbox = TFHitbox::spine_2;

    if (local_weapon_tag.isMelee() == false) {
        switch (local_class) {
        case tf_classes::TF2_Sniper: {
            if (local_weapon_tag.isSecondary()) {
                break; // smg wants to aim at the chest
            }

            if (localPlayer->IsZoomed() || local_weapon_helper.IsProjectile()) {
                // zoomed and huntsman want to aim at the head
                bestHitbox   = TFHitbox::head;
                singleHitbox = true;

                break;
            }

            if (localPlayer->IsZoomed() == false || ent->GetHealth() < get_rifle_damage())
                break; // if we are not zoomed then there is no reason to aim at the head
        }
        case tf_classes::TF2_Spy: {
            bestHitbox = TFHitbox::head;
            break;
        }
        case tf_classes::TF2_Soldier:
            // check whether we are using a projectile weapon
            if (local_weapon_helper.IsProjectile()) {
                bestHitbox = TFHitbox::origin;
                break;
            }
            break;
        default:
            break;
        }
    }

    // first check for single hitbox solutions
    int iBest = (int)bestHitbox;

    if (bestHitbox == TFHitbox::origin) {
        Vector origin = ent->GetAbsOrigin();
        if (visible(ent, &origin, iBest)) {
            *out = origin;
            return true;
        }
    } else if (visible(ent, &hitboxes.centre[iBest], iBest) == true) {
        *out = hitboxes.centre[iBest];
        return true;
    } else if (multipoint(ent, hitboxes.centre[iBest], hitboxes.bbmin[iBest], hitboxes.bbmax[iBest], iBest, out)) {
        return true;
    }

    // now check for other hitboxes and multipoints
    if (singleHitbox == false) {
        // iterate through all hitboxes
        for (int i = 0; i < totalHitboxes; i++) {
            // check if the hitbox is visible
            if (visible(ent, &hitboxes.centre[i], i)) {
                // if this hitbox is visible, use it
                *out = hitboxes.centre[i];
                return true;
            } else {
                if (local_weapon_helper.IsProjectile() == false) {
                    //multipoint (ent, hitboxes.centre[i], hitboxes.bbmin[i], hitboxes.bbmax[i], iBest, out);
                }
            }
        }
    }

    return false;
}

void Aimbot::processCommand(CUserCmd *user_cmd)
{
    F1_VPROF_FUNCTION();

    if (enabled.Value() == false)
        return;

    if (has_valid_target == false) {
        // we are not valid for doing anything this tick
        return;
    }

    QAngle new_angles;

    auto local_entity = GetLocalPlayer();
    auto local_weapon = local_entity->GetActiveWeapon();

    // handle melee weapons independently of weapons
    // we only attack with melee weapons when we are close enough
    // TODO: aim with melee weapons when charging / other situations
    if (local_weapon_tag.isMelee() && local_weapon->GetClientClass()->classId != classId::CTFKnife) {

        // test for trace
        // TODO: we need to do this after aiming / override aiming or something
        trace_t t;
        if (CTFBaseWeaponMelee::FromBaseEntity(local_weapon)->DoSwingTrace(t)) {
            if (t.m_pEnt) {
                CBaseEntity *hit = (CBaseEntity *)t.m_pEnt;
                if ((hit->GetTeam() != local_entity->GetTeam() && is_player(hit) == true)) {
                    user_cmd->buttons |= IN_ATTACK;
                }
            }
        }

        return;
    }

    CTarget target = get_best_target();

    const CBaseEntity *target_entity = target.ent;
    if (target_entity == nullptr) return;

    bool can_attack = false;

    if ((user_cmd->buttons & IN_ATTACK) == false) {
        // we are not IN_ATTACK

        // dont aim if aimkey not pressed
        if (use_aim_key.Value() == true && aim_key.Value() == false) return;

        if (local_class == tf_classes::TF2_Sniper) {

            auto is_zoomed = local_entity->IsZoomed();
            if (is_zoomed == false && zoom_only.Value() && local_weapon_tag.isPrimary()) return;

            if (is_zoomed) {
                // if we cant get them with the no charge
                if (target_entity->GetHealth() > get_rifle_damage()) {
                    float zoom_time = local_entity->GetZoomTime();

                    if (zoom_time < 0.3f) return;
                }
            }
        } else if (local_class == tf_classes::TF2_Spy) {
            auto local_weapon_id = local_entity->GetActiveWeapon()->GetClientClass()->classId;

            if (local_weapon_id == classId::CTFRevolver) {
                CTFBaseWeaponGun *tf_weapon = CTFBaseWeaponGun::FromBaseEntity(local_weapon);

                if (tf_weapon->GetWeaponID() == WPN_Ambassador && tf_weapon->WeaponGetSpread() != 0.0f) return;
            } else if (local_weapon_id == classId::CTFWeaponSapper) {
                // TODO: add aim at buildings
                return;
            }
        }

        can_attack = auto_shoot.Value();
    } else {
        // player is IN_ATTACK

        if (use_aim_key.Value()) {
            if (aim_key.lastKeyCode != ButtonCode_t::MOUSE_LEFT && aim_key.Value() == false) {
                // remove the in_attack bit from the buttons aswell
                user_cmd->buttons &= ~IN_ATTACK;
                return;
            }
        }

        // player wants an attack
        // so give it to them
        can_attack = true;
    }

    if (bulletTime(local_entity) == false) can_attack = false;

    if (can_attack == false) return;

    if (can_attack == true) {
        Vector target_vector = target.target - local_entity->GetViewPos();
        VectorAngles(target_vector, new_angles);

        new_angles -= local_entity->GetPunchAngles();

        // clampangles also normalizes them for us
        clamp_angle(new_angles);

        // fixes movement and sets angles
        silent_movement_fix(Vector{user_cmd->forwardmove, user_cmd->sidemove, 0}, new_angles);

        if (use_silent.Value() != true) {
            gInts->Engine->SetViewAngles(new_angles);
        }

        user_cmd->buttons |= IN_ATTACK; // attack when autoshoot enabled
        // always send shoot packets
        gHack.sendThisTick = true;

        *gHack.CM_input_sample_time = 0.0f;
    }

    return;
}

void Aimbot::processCommandBeforePred(CUserCmd *command)
{
    // setup some stuff that should only be read
    auto local_player  = GetLocalPlayer();
    auto active_weapon = local_player->GetActiveWeapon();

    if (active_weapon == nullptr) {
        has_valid_target = false;
        return;
    }

    if (enabled.Value()) {
        local_class      = local_player->GetClass();
        local_weapon_tag = CEntTag(active_weapon);

        this->local_weapon_helper = WeaponHelper(active_weapon);

        has_valid_target = true;
    } else {
        has_valid_target = false;
        return;
    }
}

bool Aimbot::is_valid_target(const CBaseEntity *ent)
{
    if (has_valid_target == false) {
        // we dont want to do any of this when we are not valid
        // returning false here prevents all the stages of the targetmanager hierarchy
        return false;
    }

    if (ent->GetIndex() == gInts->Engine->GetLocalPlayerIndex())
        return false;

    if (ent->IsAlive() == false)
        return false;

    if (ent->GetTeam() == GetLocalPlayer()->GetTeam())
        return false;

    // TODO: deal with objects when that happens
    if (is_player(ent) == false)
        return false;

    if (check_cond(ent) == false)
        return false;

    return true;
}

inline bool Aimbot::is_visible_target(const CBaseEntity *entity, Vector &hit)
{
    bool success = find_hitbox(entity, &hit);

    if (success == true) {

        // TODO: distance based target system takes into account fov limit
        if (target_system.Value() == targetHelpers::fov) {
            if (__CTargetHelper::GetFovFromLocalPlayer(hit) > fov_limit.Value()) {
                return false;
            }
        }

        return true;
    }

    // not visible
    return false;
}

inline bool Aimbot::compare_target(const CTarget &best_target, const CTarget &new_target)
{
    // return true to show that the new target is better than the old target
    if (target_system.Value() == targetHelpers::fov) {
        if (__CTargetHelper::GetFovFromLocalPlayer(best_target.target) > __CTargetHelper::GetFovFromLocalPlayer(new_target.target)) {
            return true;
        }
    } else if (target_system.Value() == targetHelpers::distance) {
        if (__CTargetHelper::getDistanceToVector(best_target.target) > __CTargetHelper::getDistanceToVector(new_target.target)) {
            return true;
        }
    }

    return false;
}
