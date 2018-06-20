using System;
using System.Runtime.InteropServices;
using Knife.Hooking;
using Knife.Structs;

namespace Knife.Classes {
    unsafe class Networkable : VTableProxyBase {
        public Networkable(IntPtr @this) : base(@this) {
        }

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate ClientClass *GetClientClassFn(IntPtr @this);
        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate int GetIndexFn(IntPtr @this);

        public ClientClass* ClientClass => Self.GetVMTAndFnForThis<GetClientClassFn>(2)(Self);
        public int Index => Self.GetVMTAndFnForThis<GetIndexFn>(9)(Self);
    }
}
