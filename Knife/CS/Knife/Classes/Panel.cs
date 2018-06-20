using System;
using System.Runtime.InteropServices;
using Knife.Hooking;

namespace Knife.Classes {
    class Panel : VTableProxyBase {
        public Panel(IntPtr self) : base(self) {
        }

        [UnmanagedFunctionPointer(CallingConvention.ThisCall)]
        delegate IntPtr GetNameFn(IntPtr @this, uint vguiPanel);
        public string GetName(uint vguiPanel) => Marshal.PtrToStringAnsi(Self.GetVMTAndFnForThis<GetNameFn>(36)(Self, vguiPanel));
    }
}
