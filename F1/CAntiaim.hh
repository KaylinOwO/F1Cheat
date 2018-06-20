#pragma once

#include "../SDK/IHack.hh"
#include "F1_ConVar.hh"

class CAntiaim : public IHack<CAntiaim>
{
    F1_ConVar<Switch> AASwitch{"AntiAim", "f1_antiaim_switch", false};

    F1_ConVar<bool> enabled{" - Enabled", "f1_antiaim_enabled", false, &AASwitch};
    F1_ConVar<bool> fakeUp{" - Fake up", "f1_antiaim_fakeup", false, &AASwitch};
    F1_ConVar<bool> fakeDown{" - Fake down", "f1_antiaim_fakedown", false, &AASwitch};
    F1_ConVar<bool> faceBackwards{" - Face backwards", "f1_antiaim_facebackwards", false, &AASwitch};
    F1_ConVar<bool> spinBot{" - Spinbot", "f1_antiaim_spinbot_enable", false, &AASwitch};
    F1_ConVar<int>  spinSpeed{" - Spin speed", "f1_antiaim_spinbot_speed", 60, 1, 1000, 5, &AASwitch};
    F1_ConVar<int>  staticYaw{" - Static yaw", "f1_antiaim_static_yaw", 0, 0, 360, 0, &AASwitch};
    F1_ConVar<bool> jitter{" - Jitter", "f1_antiaim_jitter", false, &AASwitch};
    F1_ConVar<bool> lisp{" - Lisp", "f1_antiaim_lisp", false, &AASwitch};

    F1_ConVar<int>  pitch{" - Pitch", "f1_antiaim_pitch", 0, 0, 0, 1, &AASwitch};
    F1_ConVar<bool> switchPitch{" - Switch pitch", "f1_antiaim_pitch_switch", false, &AASwitch};
    F1_ConVar<bool> fakepitch{" - Fake pitch", "f1_antiaim_pitch_fake", false, &AASwitch};
    F1_ConVar<int>  yaw{" - Yaw", "f1_antiaim_yaw", 0, 0, 0, 1, &AASwitch};
    F1_ConVar<bool> switchYaw{" - Switch yaw", "f1_antiaim_yaw_switch", false, &AASwitch};
    F1_ConVar<bool> fakeyaw{" - Fake yaw", "f1_antiaim_yaw_fake", false, &AASwitch};

public:
    CAntiaim()
    {
    }

    // Inherited via IHack
    const char *name() const;
    void        processCommand(CUserCmd *pUserCmd);
    void        menuUpdate(F1_IConVar **menuArray, int &currIndex);
};

extern CAntiaim gAntiaim;
