#pragma once

#include <unordered_map>

#include "../SDK/baseHeaders.hh"

#ifdef F1_GCC

#include "mingw-threads/mingw.thread.hh"
#else
#include <thread>
#endif

#include "../SDK/backtrace_off.hh"

#include "CPlayerManager.hh"

extern void clamp_angle(QAngle &angle);

namespace __CTargetHelper {
inline QAngle CalcAngle(Vector PlayerPos, Vector EnemyPos)
{
    Vector d = PlayerPos - EnemyPos;

    QAngle ang;
    VectorAngles(d, ang);

    return ang;
}

inline float GetFov(QAngle &angle, Vector &src, Vector &dst)
{
    Vector aim, ang;
    QAngle aimAng = CalcAngle(src, dst);

    AngleVectors(angle, &aim);
    AngleVectors(aimAng, &ang);

    return RAD2DEG(acos(aim.Dot(ang) / (aim.LengthSqr())));
}

inline float GetYawDifference(QAngle &angle1, QAngle &angle2)
{
    QAngle clamped = angle1 - angle2;
    clamp_angle(clamped);

    return clamped.y;
}

inline float GetFovFromLocalPlayer(Vector dst)
{
    // CEntity<> local{ me };
    const CBaseEntity *local = GetLocalPlayer();

    Vector localPos = local->GetViewPos();

    // sin = opp / hyp

    Vector hyp = dst - localPos;

    //float fov = GetFov (local->GetAbsAngles (), dst, localPos);

    //return fov;

    // calculate the real fov
    // https://www.unknowncheats.me/forum/c-and-c/114805-real-aimbot-crosshair-distance.html

    QAngle aimAngles;
    VectorAngles(hyp, aimAngles);

    QAngle viewAngles;
    gInts->Engine->GetViewAngles(viewAngles);

    float fov = fabsf(GetYawDifference(aimAngles, viewAngles));

    float realFov = sin(DEG2RAD(fov)) * hyp.Length();

    return realFov;

    //return realFov;
}

inline float getDistanceToVector(Vector v)
{
    const CBaseEntity *pLocal = GetLocalPlayer();

    if (!pLocal)
        return 8192.0f;

    return pLocal->GetAbsOrigin().DistToSqr(v);
}
} // namespace __CTargetHelper

struct CTarget
{
    const CBaseEntity *ent;
    Vector             target;

    bool operator==(const CTarget other) const
    {
        return ent->GetIndex() == other.ent->GetIndex();
    }
};

// TODO: remove
enum class targetHelpers
{
    distance,
    fov,
};

// allows us to invoke think genericly
class TargetManagerBase
{
    CTarget targ;

    // linked list for target manager thinking
    TargetManagerBase *next = nullptr;

    static TargetManagerBase *head;

public:
    // this virtual call to think should be inlined by the compiler
    virtual void Think() = 0;

    static TargetManagerBase *Head()
    {
        return head;
    }

    TargetManagerBase *Next()
    {
        return next;
    }

    TargetManagerBase();
    ~TargetManagerBase();

    static void ThinkAll()
    {
        F1_VPROF("TargetManagerBase::ThinkAll");
        for (auto *manager = head; manager != nullptr; manager = manager->next) {
            manager->Think();
        }
    }

    CTarget get_best_target()
    {
        return targ;
    }

    virtual bool can_backtrack()
    {
        return false;
    }

    template <typename Derived>
    friend class TargetManager;
};

template <typename T>
class TargetManager : public TargetManagerBase
{

    CTarget bestRageTarg;
    CTarget bestTarg;

    friend class UtlSortLessHelper;

    class UtlSortLessHelper
    {
    public:
        bool Less(const CTarget &left, const CTarget &right, void *ctx)
        {
            // FIXME: this is all backwards wtf
            return ((TargetManager<T> *)ctx)->compare_target(left, right) == 0;
        }
    };

    CUtlVectorMT<CUtlSortVector<CTarget, UtlSortLessHelper>> normalTargetArray;
    CUtlVectorMT<CUtlSortVector<CTarget, UtlSortLessHelper>> rageTargetArray;

    // the thread pool for this target helper
    IThreadPool *threadPool;

private:
    T &Derived()
    {
        return static_cast<T &>(*this);
    }

public:
    TargetManager();
    ~TargetManager();

    // entity checks to see if this target is valid
    // this shouldnt do visible checks as this does fast discard
    bool is_valid_target(const CBaseEntity *entity)
    {
        return Derived().is_valid_target(entity);
    }

    // entity checks to see if a target is visible
    // if it is then targetLocation should be the location of the target where
    // it is visible
    // fov checks should occur here
    bool is_visible_target(const CBaseEntity *entity, Vector &targetLocation)
    {
        return Derived().is_visible_target(entity, targetLocation);
    }

    // compares two targets too each other
    // return true to show that the new target is the best target
    // return false to show that the old target is the best target
    bool compare_target(const CTarget &bestTarget, const CTarget &newTarget)
    {
        return Derived().compare_target(bestTarget, newTarget);
    }

    // evalulates targets based on system used, rage targets, friends
    // call once per tick
    void Think() override;

    void ThinkInner(const CBaseEntity *pBaseEntity, int i);

    // should we only look for players
    bool is_only_players()
    {
        return Derived().is_only_players();
    }
};

template <typename T>
inline TargetManager<T>::TargetManager()
    : TargetManagerBase()
{
    threadPool = CreateThreadPool();

    normalTargetArray.SetLessContext(this);
    rageTargetArray.SetLessContext(this);
}

template <typename T>
TargetManager<T>::~TargetManager()
{
    // cleanup
    DestroyThreadPool(threadPool);
}

extern ConVar f1_dumpalltargetmanagerexceptions;

// TODO: this is probably a massive race condition i dont know where to begin
template <typename T>
void TargetManager<T>::ThinkInner(const CBaseEntity *pBaseEntity, int i)
{
    Vector t;

    bool isNormalLocked = false;
    bool isRageLocked   = false;

    __try {
        // do we care about whether a target is closer after prediction?

        if (this->is_visible_target(pBaseEntity, t)) {
            auto mode = gPlayerManager.getModeForPlayer(i);
            if (mode == CPlayerManager::playerMode::Normal) {
                normalTargetArray.Lock();
                {
                    isNormalLocked = true;
                    normalTargetArray.Insert(CTarget{pBaseEntity, t});
                }
                normalTargetArray.Unlock();
            } else if (mode == CPlayerManager::playerMode::Rage) {
                rageTargetArray.Lock();
                {
                    isRageLocked = true;
                    rageTargetArray.Insert(CTarget{pBaseEntity, t});
                }
                rageTargetArray.Unlock();
            }

            return;
        }
    } __except (f1_dumpalltargetmanagerexceptions.GetBool() ? UnhandledSehExceptionFilter(GetExceptionInformation(), true, "TM") : EXCEPTION_EXECUTE_HANDLER) {
        // this is an easy fix for a much larger problem

        Log::Console("Target manager exception");

        if (isNormalLocked)
            normalTargetArray.Unlock();
        if (isRageLocked)
            rageTargetArray.Unlock();
    }
}

// evalulates targets based on system used, rage targets, friends
// call once per tick

template <typename T>
void TargetManager<T>::Think()
{
    int maxEntity = Derived().is_only_players() ? gInts->Engine->GetMaxClients() : gInts->EntList->GetHighestEntityIndex();

    // clean array
    rageTargetArray.RemoveAll();
    normalTargetArray.RemoveAll();

    // parallel process targets
    {
        CJobSet jobSet;
        for (int i = 1; i <= maxEntity; i++) {
            const CBaseEntity *pBaseEntity = GetBaseEntity(i);

            if (pBaseEntity == nullptr) {
                continue;
            }

            if (pBaseEntity->IsDormant()) {
                continue;
            }

            if (!is_valid_target(pBaseEntity)) {
                continue;
            }

            ExecuteParallel(this, &TargetManager::ThinkInner, pBaseEntity, i);
        }

        jobSet.WaitForFinish(threadPool);
    }

    if (rageTargetArray.Size() > 0) {
        targ = rageTargetArray.Head();
    } else if (normalTargetArray.Size() > 0) {
        targ = normalTargetArray.Head();
    } else {
        targ.ent = nullptr;
    }

    // multiple target managers you can technically have multiple targets
    // TODO:
    // gPlayerManager.setTarget( targ.ent );
}
