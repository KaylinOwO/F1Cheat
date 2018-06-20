using System;
using System.Numerics;
using System.Runtime.InteropServices;
using Knife.Hooking;
using Knife.Structs;

namespace Knife.Classes {
    unsafe class Renderable : VTableProxyBase {
        public Renderable(IntPtr @this) : base(@this) {
        }

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate Vector3* GetRenderOriginFn(IntPtr @this);

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate Vector3* GetRenderAnglesFn(IntPtr @this);

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate IntPtr GetModelFn(IntPtr @this);

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        [return: MarshalAs(UnmanagedType.U1)]
        delegate bool SetupBoneFn(IntPtr @this, out Matrix3x4 bone, int maxBones, int boneMask, float currentTime);

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate void GetRenderBoundsFn(IntPtr @this, ref Vector3 mins, ref Vector3 maxs);

        public Vector3 RenderOrigin => *Self.GetVMTAndFnForThis<GetRenderOriginFn>(1)(Self);
        public Vector3 RenderAngles => *Self.GetVMTAndFnForThis<GetRenderAnglesFn>(2)(Self);

        public Model Model => new Model(Self.GetVMTAndFnForThis<GetModelFn>(9)(Self));

        // TODO
        public Matrix3x4 SetupBone(int maxBones, int boneMask, float currentTime) {
            Matrix3x4 mat;
            return Self.GetVMTAndFnForThis<SetupBoneFn>(16)(Self, out mat, maxBones, boneMask, currentTime) ? mat : default(Matrix3x4);
        }

        public Tuple<Vector3, Vector3> RenderBounds {
            get {
                var mins = new Vector3();
                var maxs = new Vector3();
                Self.GetVMTAndFnForThis<GetRenderBoundsFn>(20)(Self, ref mins, ref maxs);
                return Tuple.Create(mins, maxs);
            }
        }
    }
}
