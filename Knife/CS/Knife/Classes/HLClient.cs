using System;
using System.Runtime.InteropServices;
using Knife.Hooking;
using Knife.Structs;

namespace Knife.Classes {
    unsafe class HLClient : VTableProxyBase {
        public HLClient(IntPtr self) : base(self) {
        }

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate ClientClass* GetAllClassesFn(IntPtr @this);
        public ClientClass *AllClasses => Self.GetVMTAndFnForThis<GetAllClassesFn>(8)(Self);
    }
}
