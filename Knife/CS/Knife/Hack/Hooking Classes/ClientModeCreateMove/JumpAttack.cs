using System;
using Knife.Enums;
using Knife.Structs;

namespace Knife.Hack.Hooking_Classes.ClientModeCreateMove {
    class JumpAttack : IClientModeCreateMove {
        public Func<bool> Condition => () => true;
        public unsafe void OnCreateMove(float flInputSampleTime, UserCmd* cmd, ref bool outBool) {
            if ((cmd->Buttons & PlayerControls.InJump) == PlayerControls.InJump) {
                cmd->Buttons |= PlayerControls.InAttack; //Set the IN_ATTACK flag.
            }
        }
    }
}
