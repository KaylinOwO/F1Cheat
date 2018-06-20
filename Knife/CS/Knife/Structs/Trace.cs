using System;
using System.Numerics;
using System.Runtime.InteropServices;
using Knife.Classes;


namespace Knife.Structs {
    unsafe struct Trace {
        Vector3 startPos;
        Vector3 endPos;
        public struct Plane {
            Vector3 normal;
            float dist;
            byte type;
            byte signbits;
            fixed byte pad[2];

            public Vector3 Normal => normal;
            public float Disttance => dist;
            public byte Type => type;
            public byte SignBits => signbits;
        }
        Plane plane;
        float fraction;
        int contents;
        ushort dispFlags;
        [MarshalAs(UnmanagedType.U1)]
        bool allSolid;
        [MarshalAs(UnmanagedType.U1)]
        bool startSolid;
        float fractionLeftSolid;
        public struct Surface {
            IntPtr namePtr;
            short surfaceProps;
            ushort flags;

            public string Name => Marshal.PtrToStringAnsi(namePtr);
            public short SurfaceProps => surfaceProps;
            public ushort Flags => flags;
        }
        Surface surface;
        int hitGroup;
        short physicsBone;
        IntPtr m_pEnt;
        int hitBox;

        public Vector3 StartingPosition => startPos;
        public Vector3 EndingPosition => endPos;
        public Plane ImpactPlane => plane;
        public float Fraction => fraction;
        public int Contnets => contents;
        public ushort DisplacementFlags => dispFlags;
        public bool AllSolid => allSolid;
        public bool StartSolid => startSolid;
        public float FractionLeftSolid => fractionLeftSolid;
        public Surface HitSurface => surface;
        public int HitGroup => hitGroup;
        public short PhysicsBone => physicsBone;
        public Entity HitEntity => new Entity(m_pEnt);
        public int HitBox => hitBox;
    }
}
