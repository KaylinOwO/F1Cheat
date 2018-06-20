using System;
using System.Drawing;
using System.Numerics;
using System.Runtime.InteropServices;
using Knife.Hooking;

namespace Knife.Classes {
    class EngineClient : VTableProxyBase {
        public EngineClient(IntPtr self) : base(self) {
        }

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate int GetLocalPlayerFn(IntPtr @this);

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate void GetScreenSizeFn(IntPtr @this, ref int width, ref int height);

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate void GetViewAngleFn(IntPtr @this, ref Vector3 va);

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate void SetViewAngleFn(IntPtr @this, ref Vector3 va);

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate int GetMaxClientFn(IntPtr @this);

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        [return: MarshalAs(UnmanagedType.I1)]
        delegate bool IsInGameFn(IntPtr @this);

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        [return: MarshalAs(UnmanagedType.I1)]
        delegate bool IsConnectedFn(IntPtr @this);

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        [return: MarshalAs(UnmanagedType.I1)]
        delegate bool IsDrawingLoadingImageFn(IntPtr @this);

        public Size ScreenSize => new Size().With(_ => {
            int width = 0, height = 0;
            Self.GetVMTAndFnForThis<GetScreenSizeFn>(5)(Self, ref width, ref height);
            _.Width = width;
            _.Height = height;
        });

        public int LocalPlayerEntityIndex => Self.GetVMTAndFnForThis<GetLocalPlayerFn>(12)(Self);

        public Vector3 ViewAngles {
            get {
                return new Vector3().With(_ => Self.GetVMTAndFnForThis<GetViewAngleFn>(19)(Self, ref _));
            }
            set {
                Self.GetVMTAndFnForThis<SetViewAngleFn>(20)(Self, ref value);
            }
        }

        public int MaxClients => Self.GetVMTAndFnForThis<GetMaxClientFn>(21)(Self);
        public bool InGame => Self.GetVMTAndFnForThis<IsInGameFn>(26)(Self);
        public bool Connected => Self.GetVMTAndFnForThis<IsConnectedFn>(27)(Self);
        public bool DrawingLoadingImage => Self.GetVMTAndFnForThis<IsDrawingLoadingImageFn>(28)(Self);

    }
}