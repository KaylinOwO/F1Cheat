#include "stdafx.hh"

#include "AimHelpers.hh"
#include "CAntiaim.hh"

#include "CHack.hh"

#include <tier0/memdbgon.h>

CAntiaim gAntiaim;

bool switchPitch = false;
bool switchYaw   = false;

void CAntiaim::processCommand(CUserCmd *pUserCmd)
{
    if (!enabled.Value())
        return;
    // if ( pUserCmd->buttons & IN_ATTACK )
    //  return true;

    // pUserCmd->viewangles.z -= 180;
    // newAngles.y -= 180;
    // pUserCmd->forwardmove = -pUserCmd->forwardmove;

    // if(!(pUserCmd->buttons & IN_ATTACK))
    // pUserCmd->sidemove = -pUserCmd->sidemove;
    // pUserCmd->forwardmove = -pUserCmd->forwardmove;

    if ((pUserCmd->buttons & IN_ATTACK) == false) {

        QAngle newAngles = pUserCmd->viewangles;

        if (fakeUp.Value() == true) {
            newAngles.x = -271;
            // invert movement for movement fix while using this angle
            // pUserCmd->forwardmove = -pUserCmd->forwardmove;
            // pUserCmd->sidemove = -pUserCmd->sidemove;
            // return true;
        }

        if (fakeDown.Value() == true) {
            newAngles.x = 271;
            // return true;
        }

        if (faceBackwards.Value() == true) {
            newAngles.y -= 180;
            // pUserCmd->forwardmove = -pUserCmd->forwardmove;
            // pUserCmd->sidemove = -pUserCmd->sidemove;
            // return true;
        }

        if (spinBot.Value() == true) {
            // fix movement
            // pUserCmd->forwardmove = -pUserCmd->forwardmove;
            // pUserCmd->sidemove = -pUserCmd->sidemove;

            int          random    = 160 + rand() % spinSpeed.Value();
            static float current_y = newAngles.y;
            current_y += random;
            newAngles.y = current_y;
            if (current_y >= 360) {
                current_y = fmod(current_y, 360.0f);
            }
            // return true;
        }

        if (staticYaw.Value() != 0) {
            newAngles.y += staticYaw.Value();
        }

        if (jitter.Value() == true) {
            // fix movement
            // pUserCmd->forwardmove = -pUserCmd->forwardmove;
            // pUserCmd->sidemove = -pUserCmd->sidemove;

            // BIG JITTER
            newAngles.x += rand();
            newAngles.y += rand();
        }

        int newPitch = pitch.Value();

        if (fakepitch.Value() && newPitch != 0) {
            // coordinate with fakelag to send the fake value on the correct tick
            if (gHack.fakeLag.Value()) {
                if (*gHack.bSendPacket == false) {
                    // use the fake packet
                    newAngles.x = pitch.Value();
                } else {
                    // real value here
                }
            } else {
                // do this ourselves
                if (::switchPitch == true) {
                    // dont send real angle
                    *gHack.bSendPacket = false;
                    ::switchPitch      = false;
                    if (switchPitch.Value())
                        newAngles.x = pitch.Value();
                } else {
                    // bsendpacket = true
                    // send fake angle
                    newAngles.x   = pitch.Value();
                    ::switchPitch = true;
                }
            }
        } else {
            if (::switchPitch == false) {
                // if we are using fake angles just use the default angle
                newAngles.x = pitch.Value();

                if (switchPitch.Value())
                    ::switchPitch = true;
            } else if (::switchPitch == true) {
                newAngles.x = pitch.Value();
                //*gHack.bSendPacket = false;
                ::switchPitch = false;
            }
        }

        int newYaw = yaw.Value();

        if (fakeyaw.Value() && newYaw != 0) {
            // coordinate with fakelag to send the fake value on the correct tick
            if (gHack.fakeLag.Value()) {
                if (*gHack.bSendPacket == false) {
                    // use the fake packet
                    newAngles.y += yaw.Value();
                } else {
                    // real value here
                }
            } else {
                // do this ourselves
                if (::switchYaw == true) {
                    // dont send real angle
                    if (fakeyaw.Value())
                        *gHack.bSendPacket = false;
                    ::switchYaw = false;
                    if (switchYaw.Value())
                        newAngles.y += yaw.Value();
                } else {
                    // bsendpacket = true
                    // send fake angle
                    newAngles.y -= yaw.Value();
                    ::switchYaw = true;
                }
            }
        } else {
            if (::switchYaw == false) {
                // if we are using fake angles just use the default angle
                newAngles.y += yaw.Value();
                if (switchYaw.Value())
                    ::switchYaw = true;
            } else if (::switchYaw == true) {
                newAngles.y -= yaw.Value();
                //*gHack.bSendPacket = false;
                ::switchYaw = false;
            }
        }

        silent_movement_fix(pUserCmd, newAngles);

        pUserCmd->viewangles = newAngles;
    }

    // gInts->Engine->SetViewAngles(pUserCmd->viewangles);

    return;
}

void CAntiaim::menuUpdate(F1_IConVar **menuArray, int &currIndex)
{
    menuArray[currIndex++] = &AASwitch;

    menuArray[currIndex++] = &enabled;
    menuArray[currIndex++] = &fakeUp;
    menuArray[currIndex++] = &fakeDown;
    menuArray[currIndex++] = &faceBackwards;
    menuArray[currIndex++] = &spinBot;
    menuArray[currIndex++] = &spinSpeed;
    menuArray[currIndex++] = &jitter;
    menuArray[currIndex++] = &lisp;
    menuArray[currIndex++] = &staticYaw;
}
