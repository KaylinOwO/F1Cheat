using System;
using Knife.Enums.TF;
using Knife.Hack;

namespace Knife.Classes {
    unsafe class TFGrenadePipebombProjectile : Entity {
        public TFGrenadePipebombProjectile(IntPtr @this) : base(@this) {
        }

        public bool SurfaceTouched => *(byte*)(Self + Ioc.Get<NetVar>()["DT_TFProjectile_Pipebomb"]["m_bTouched"]) > 0;
        public TFSticky StickyType => *(TFSticky*) (Self + Ioc.Get<NetVar>()["DT_TFProjectile_Pipebomb"]["m_iType"]);
        public bool IsSticky => StickyType == TFSticky.PipeBomb || IsScottishResistanceSticky;
        public bool IsScottishResistanceSticky => *(byte*)(Self + Ioc.Get<NetVar>()["DT_TFProjectile_Pipebomb"]["m_bDefensiveBomb"]) > 0;
        public int LauncherHandle => *(int*)(Self + Ioc.Get<NetVar>()["DT_TFProjectile_Pipebomb"]["m_hLauncher"]);
        public CombatWeapon Launcher =>
            new CombatWeapon(Ioc.Get<EntityList>().GetClientEntityFromHandle(LauncherHandle).Self);
    }
}
