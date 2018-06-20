#include "stdafx.hh"

#include "Announcer.hh"

#include <tier0/memdbgon.h>

Announcer gAnnouncer;

void Announcer::init()
{
    // Log::Console("adding listeners");
    gInts->EventManager->AddListener(this, "player_death", false);
    gInts->EventManager->AddListener(this, "player_spawn", false);

    return;
}

void Announcer::FireGameEvent(IGameEvent *event)
{
    int local_user_id = gInts->Engine->GetPlayerInfo(gInts->Engine->GetLocalPlayerIndex()).userID;

    if (local_user_id == 0) // CBasePlayer::GetUserId() You can get this from the player_info_t struct as well.
        return;

    if (strcmp(event->GetName(), "player_death") == 0) {
        int attacker = event->GetInt("attacker", 0);
        int user_id  = event->GetInt("userid", 0);
        if (attacker == local_user_id) {
            if (attacker == user_id) return;

            kill_counter++;
            streak_counter++;

            if ((gInts->Globals->curtime - last_kill_time) < streak_timeout) {
                // This is done to prevent getting spammed with killstreak sounds when killing like 4 people at once with a crit sticky, so it will play the
                // most recent sound when Think is called.
                switch (streak_counter) {
                case 2:
                    strcpy_s(current_sound, "UT2k4/Double_Kill.wav");
                    break;
                case 3:
                    strcpy_s(current_sound, "Quake3/quake_tripplekill.wav");
                    break;
                case 4:
                    strcpy_s(current_sound, "UT99/multikill.wav");
                    break;
                case 5:
                    strcpy_s(current_sound, "UT2k4/MegaKill.wav");
                    break;
                case 6:
                    strcpy_s(current_sound, "UT99/ultrakill.wav");
                    break;
                case 7:
                    strcpy_s(current_sound, "UT99/monsterkill.wav");
                    break;
                case 8:
                    strcpy_s(current_sound, "UT2k4/LudicrousKill.wav");
                    break;
                default: // 9 or more kills in a row, play 'Holy Shit!' every time.
                    strcpy_s(current_sound, "UT2k4/HolyShit.wav");
                }
            } else {
                // This scenario is when you kill someone outside the killStreakTimeout variable.
                streak_counter = 1;
            }

            switch (static_cast<tf_customkills>(event->GetInt("customkill", 0))) {
            case tf_customkills::HEADSHOT:
            case tf_customkills::HEADSHOT_DECAPITATION:
            case tf_customkills::PENETRATE_HEADSHOT:
                play_sound("UT99/headshot.wav");
                break;
            }

            switch (kill_counter) {
#ifdef USE_TF2_KILLSTREAKS
            case 5:
                play_sound("UT99/killingspree.wav");
                break;
            case 10:
                play_sound("UT99/unstoppable.wav");
                break;
            case 15:
                play_sound("UT99/rampage.wav");
                break;
            default: // If the player gets a 20 or more killstreak, just play Godlike every 5 kills.
                if (killCounter % 5 == 0)
                    play_sound("UT99/godlike.wav");
#else
            case 5:
                play_sound("UT99/killingspree.wav");
                break;
            case 10:
                play_sound("UT99/rampage.wav");
                break;
            case 15:
                play_sound("UT2k4/Dominating.wav");
                break;
            case 20:
                play_sound("UT99/unstoppable.wav");
                break;
            default: // If the player gets a 25 or more killstreak, just play Godlike every 5 kills.
                if (kill_counter % 5 == 0)
                    play_sound("UT99/godlike.wav");
#endif
            }
            last_kill_time = gInts->Globals->curtime;
        }
        return;
    }

    if (strcmp(event->GetName(), "player_spawn") == 0) // This will also get called when the player changes class.
    {
        if (event->GetInt("userid", 0) == local_user_id) {
            kill_counter   = 0;
            last_kill_time = 0.0f;
        }
        return;
    }
}

void Announcer::processCommand(CUserCmd *)
{
    if (current_sound != nullptr) {
        if (strcmp(current_sound, "") != 0) // Check if the killstreak has something queued.
        {
            play_sound(current_sound);
            strcpy_s(current_sound, ""); // Once done, zero out the current_sound member.
        }
    }
    return;
}

void Announcer::play_sound(const char *soundName)
{
    // other method
    // auto filt = CSingleUserRecipientFilter{gInts->EntList->GetClientEntity(me)};
    // gInts->SoundEngine->EmitSound(filt, -1, 0, soundName, 100, 100);

    // works fine when running sv_pure 0 or -1
    // meaning that it works fine with the sv_pure bypass
    gInts->Surface->PlaySound(soundName);
}
