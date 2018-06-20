using System;
using Knife.Classes;
using Knife.Enums.TF;
using Knife.Structs;
using System.Numerics;

namespace Knife.Hack.Hooking_Classes.ClientModeCreateMove {
    unsafe class SniperTriggerbot : IClientModeCreateMove {
        public Func<bool> Condition => () => Ioc.Get<EntityList>().LocalCombatPlayer.Class == TFClass.Sniper;
        public void OnCreateMove(float flInputSampleTime, UserCmd *cmd, ref bool outBool) {
			// Get local viewangles
			// angle to vector
			// traceray
			// if hit then shoot
        }
    }
}
