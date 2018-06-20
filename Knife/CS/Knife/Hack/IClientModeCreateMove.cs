using System;
using Knife.Structs;

namespace Knife.Hack
{
    unsafe interface IClientModeCreateMove {
        Func<bool> Condition { get; }
        void OnCreateMove(float flInputSampleTime, UserCmd *cmd, ref bool outBool);
    }
}
