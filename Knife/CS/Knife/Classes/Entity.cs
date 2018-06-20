using System;
using System.Numerics;
using System.Runtime.InteropServices;
using Knife.Enums.TF;
using Knife.Hack;

namespace Knife.Classes {
    unsafe class Entity : VTableProxyBase {
        public Entity(IntPtr @this) : base(@this) {
        }

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate void UpdateGlowEffectFn(IntPtr @this);

        public Renderable Renderable => new Renderable(Self + 0x4);
        public Networkable Networkable => new Networkable(Self + 0x8);

        public bool IsPlayer => (*Networkable.ClientClass).NetworkName == "CTFPlayer";

        public int Index {
            get {
                for (var i = 0; i < 2048; i++) {
                    if (Ioc.Get<EntityList>().GetClientEntity(i).Self == Self) {
                        return i;
                    }
                }
                return -1;
            }
        }

        public bool ShouldGlow {
            get {
                return *(byte*)(Self + Ioc.Get<NetVar>()["DT_TFPlayer"]["m_bGlowEnabled"]) > 0;
            }
            set {
                *(byte*) (Self + Ioc.Get<NetVar>()["DT_TFPlayer"]["m_bGlowEnabled"]) = value ? (byte) 1 : (byte) 0;
            }
        }

        public void UpdateGlowEffect() => 
            Ioc.Get<MarshalCache>().GetDelForPtr<UpdateGlowEffectFn>(Ioc.Get<SignaturePool>()["client.dll"]["UpdateGlowEffectFn"])(Self);

        public TFTeam Team => *(TFTeam*) (Self + Ioc.Get<NetVar>()["DT_BaseEntity"]["m_iTeamNum"]);
        public bool Dormant => *(byte*)(Self + 0x00E9) > 0;

        public Vector3 AbstractOrigin {
            get { return *(Vector3*) (Self + Ioc.Get<NetVar>()["DT_BaseEntity"]["m_vecOrigin"]); }
            set { throw new NotImplementedException(); }
        }

        public Vector3 AbstractAngles {
            get { return *(Vector3*)(Self + 0x298); } // not in netvar
            set { throw new NotImplementedException(); }
        }

        public Vector3 WorldSpaceCenter => AbstractOrigin.With(_ => {
            var bounds = Renderable.RenderBounds;
            _.Z += (bounds.Item1.Z + bounds.Item2.Z)/2;
        });

    }
}
