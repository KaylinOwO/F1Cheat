using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using Knife.Enums;
using Knife.Enums.TF;
using Knife.Hack;

namespace Knife.Classes {
    unsafe class CombatPlayer : Entity {
        public CombatPlayer(IntPtr @this) : base(@this) {
        }

        public int Health => *(int*) (Self + Ioc.Get<NetVar>()["DT_BasePlayer"]["m_iHealth"]);

        public TFClass Class =>
            *(TFClass*)(Self + Ioc.Get<NetVar>()["DT_TFPlayer"]["m_PlayerClass"] + Ioc.Get<NetVar>()["DT_TFPlayerClassShared"]["m_iClass"]);

        public LifeState LifeState => *(LifeState*) (Self + Ioc.Get<NetVar>()["DT_BasePlayer"]["m_lifeState"]);
        public IntPtr Shared => Self + Ioc.Get<NetVar>()["DT_BasePlayer"]["m_Shared"];
        public PlayerCondition* PlayerCondition
            => (PlayerCondition*) (Self + Ioc.Get<NetVar>()["DT_TFPlayer"]["m_Shared"] + Ioc.Get<NetVar>()["DT_TFPlayerShared"]["m_nPlayerCond"]);
        public PlayerExtraCondition* PlayerExtraCondition
            => (PlayerExtraCondition*)(Self + Ioc.Get<NetVar>()["DT_BasePlayer"]["m_Shared"] + Ioc.Get<NetVar>()["DT_TFPlayerShared"]["m_nPlayerCondEx"]);
        public int ActiveWeaponHandle => *(int*)(Self + Ioc.Get<NetVar>()["DT_BaseCombatCharacter"]["m_hActiveWeapon"]);

        public CombatWeapon ActiveWeapon
            => new CombatWeapon(Ioc.Get<EntityList>().GetClientEntityFromHandle(ActiveWeaponHandle).Self);

        public uint TickBase => *(uint*) (Self + Ioc.Get<NetVar>()["DT_LocalPlayerExclusive"]["m_nTickBase"]);

        public Vector3 ViewOffset
            => *(Vector3*) (Self + Ioc.Get<NetVar>()["DT_LocalPlayerExclusive"]["m_vecViewOffset[0]"]);

        public Vector3 EyePosition
            => AbstractOrigin + ViewOffset;

        public List<TFGrenadePipebombProjectile> StickyBombs => new List<TFGrenadePipebombProjectile>().With(_ => {
            if (Class != TFClass.DemoMan) {
                return;
            }

            _.AddRange(Ioc.Get<EntityList>().AllButPlayers
                .Where(entity => (*entity.Networkable.ClientClass).NetworkName == "CTFGrenadePipebombProjectile")
                .Select(entity => new TFGrenadePipebombProjectile(entity.Self))
                .Where(grenade => grenade.IsSticky && grenade.Launcher.Owner.Self == Self));

        });
    }
}
