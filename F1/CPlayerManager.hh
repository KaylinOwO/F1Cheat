#pragma once

#include "../SDK/CDrawManager.hh"
#include "../SDK/IHack.hh"
#include "F1_ConVar.hh"

#include <mutex>

class CPlayerManager : public IHack<CPlayerManager>, public IGameEventListener2
{

    F1_ConVar<Switch> playerModeSwitch = F1_ConVar<Switch> ("Player List", "f1_playerlist_switch", false);

    friend class PlayerManagerListPanel;

    class vgui::ListPanel *listPanel            = nullptr;
    bool                   performlayoutoneTime = true;

public:
    enum class playerMode
    {
        Normal,
        Friend,
        Rage,
    };

    enum class anglesMode
    {
        Real,
        Fake,
        Manual,
    };

    struct playerState
    {
        // playerMode mode;
        F1_ConVar<Enum<playerMode>> *mode;
        F1_ConVar<Enum<anglesMode>> *angles;
        F1_ConVar<float> *           manualYawCorrection;
        F1_ConVar<float> *           manualPitchCorrection;
        F1_ConVar<bool> *            addOrForce;

        bool isTarget = false;
        bool isValid  = false;
        int  uid      = -1;
        int  pid      = -1;

        ~playerState ()
        {
            delete mode;
            mode = nullptr;
            delete angles;
            angles = nullptr;
            delete manualYawCorrection;
            manualYawCorrection = nullptr;
            delete manualPitchCorrection;
            manualPitchCorrection = nullptr;
        }
    };

    CPlayerManager ();

    void init ();

    // returns -1 or 0xFFFFFFFF on no custom color (use team color)
    DWORD getColorForPlayer (int index);

    playerMode getModeForPlayer (int index);

    playerState *getPlayer (int index);

    void menuUpdate (F1_IConVar **menuArray, int &currIndex);

    bool paint ();

    void setTarget (int index);

    std::vector<playerState *> getPlayersWithMode (playerMode mode);

    void FireGameEvent (IGameEvent *event) override;

    void registerPlayerUserId (int userID, const char *name);
    void registerPlayerEvent (IGameEvent *event);
    void registerPlayerEntity (CBaseEntity *entity);
    void deregisterPlayerEvent (IGameEvent *event, int uid = 0);
    void deregisterAllPlayers ();
    void registerAllPlayers ();
    void deregisterIfGone (int index);

private:
    void PerformLayout ();

    std::unordered_map<int, playerState> playerArray;

    int lastTargetIndex = -1;

    Enum<playerMode> playerStateEnum = {playerMode::Normal,
                                        {
                                            {playerMode::Normal, "Normal"},
                                            {playerMode::Friend, "Friend"},
                                            {playerMode::Rage, "Rage"},
                                        }};

    Enum<anglesMode> playerAnglesEnum = {anglesMode::Real,
                                         {
                                             {anglesMode::Real, "Real"},
                                             {anglesMode::Fake, "Fake"},
                                             {anglesMode::Manual, "Manual"},
                                         }};

    bool isInGame = false;
};

extern CPlayerManager gPlayerManager;
