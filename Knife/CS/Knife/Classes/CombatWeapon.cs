using System;
using System.Runtime.InteropServices;
using Knife.Enums.TF;
using Knife.Hack;

namespace Knife.Classes {
    unsafe class CombatWeapon : Entity {
        public CombatWeapon(IntPtr @this) : base(@this) {
        }

        public Weapon ItemDefinitionIndex =>
            *(Weapon*) (Self + Ioc.Get<NetVar>()["DT_BaseAttributableItem"]["m_AttributeManager"] +
                     Ioc.Get<NetVar>()["DT_AttributeContainer"]["m_Item"] +
                     Ioc.Get<NetVar>()["DT_ScriptCreatedItem"]["m_iItemDefinitionIndex"]);

        public bool ReadyToBackStab => *(byte*) (Self + Ioc.Get<NetVar>()["DT_TFWeaponKnife"]["m_bReadyToBackstab"]) > 0;

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate void GetCrosshairScaleFn(IntPtr @this, ref float scale);

        // TODO
        public bool AmbassadorReadyToCrit {
            get {
                if (ItemDefinitionIndex != Weapon.Ambassador) {
                    return false; // no, fuck off
                }
                var scale = 0.0f;
                Ioc.Get<MarshalCache>()
                    .GetDelForPtr<GetCrosshairScaleFn>(Ioc.Get<SignaturePool>()["client.dll"]["GetCrosshairScaleFn"])(Self, ref scale);
                return scale == 0.75f;
            }
        }

        public int OwnerHandle => *(int*) (Self + Ioc.Get<NetVar>()["DT_BaseCombatWeapon"]["m_hOwner"]);

        public CombatPlayer Owner =>
            new CombatPlayer(Ioc.Get<EntityList>().GetClientEntityFromHandle(OwnerHandle).Self);
    }
}
