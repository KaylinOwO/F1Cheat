using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using Knife.Hooking;

namespace Knife.Classes {
    class EntityList : VTableProxyBase {
        public EntityList(IntPtr self) : base(self) {
        }
        // TODO: Should managed entity list be cached?
        // GC pressure is very high every time we create a new object

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate IntPtr GetClientEntityFn(IntPtr @this, int entnum);
        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate IntPtr GetClientEntityFromHandleFn(IntPtr @this, int hEnt);
        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate int GetHighestEntityIndexFn(IntPtr @this);

        public Entity GetClientEntity(int entnum) {
            var addr = Self.GetVMTAndFnForThis<GetClientEntityFn>(3)(Self, entnum);
            return addr != IntPtr.Zero ? new Entity(addr) : null;
        }

        public Entity GetClientEntityFromHandle(int hEnt) {
            var addr = Self.GetVMTAndFnForThis<GetClientEntityFromHandleFn>(4)(Self, hEnt);
            return addr != IntPtr.Zero ? new Entity(addr) : null;
        }

        public int HighestEntityIndex => Self.GetVMTAndFnForThis<GetHighestEntityIndexFn>(6)(Self);

        public Entity LocalEntity => GetClientEntity(Ioc.Get<EngineClient>().LocalPlayerEntityIndex);
        public CombatPlayer LocalCombatPlayer => new CombatPlayer(LocalEntity.Self);

        public IEnumerable<Entity> AllEntities {
            get {
                for (var i = 0; i < HighestEntityIndex; i++) {
                    var entity = GetClientEntity(i);
                    if (entity != null) {
                        yield return entity;
                    }
                }
            }
        }

        public IEnumerable<CombatPlayer> AllPlayers =>
            from clientEntity in AllEntities where clientEntity.IsPlayer select new CombatPlayer(clientEntity.Self);

        public IEnumerable<Entity> AllButPlayers =>
            from clientEntity in AllEntities where !clientEntity.IsPlayer select new Entity(clientEntity.Self);
    }
}
