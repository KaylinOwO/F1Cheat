#pragma once

#include "../SDK/baseHeaders.hh"

#include "../SDK/IHack.hh"

class Announcer : public IHack<Announcer>, public IGameEventListener2
{
public:
    Announcer()
    {
        streak_timeout = 4.0f;
        memset(current_sound, 0, sizeof(current_sound));
    }

    // IGameEventListener2 inherits
    void FireGameEvent(IGameEvent *pEvent) override;

    // IHack inherits
    void init();
    void processCommand(CUserCmd *pUserCmd);

private:
    void play_sound(const char *soundFile);
    char current_sound[50];

protected:
    float last_kill_time;
    int   kill_counter;
    int   streak_counter;
    float streak_timeout;
};

extern Announcer gAnnouncer;
