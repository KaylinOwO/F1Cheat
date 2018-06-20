using System;
using Knife.Hack;

namespace Knife.Hooking {
    static class VTable {
        public static unsafe IntPtr GetVMTForThis(this IntPtr @this, uint offset = 0) {
            return *(IntPtr*) (@this.ToInt64() + offset); // vtable = *(base + 0) or *(base + offset)
        }

        public static unsafe IntPtr GetVFuncForVtable(this IntPtr vtable, uint index) {
            return ((IntPtr*)vtable)[index];
        }

        public static T GetVFuncFnForVtable<T>(this IntPtr vtable, uint index) {
            return Ioc.Get<MarshalCache>().GetDelForPtr<T>(vtable.GetVFuncForVtable(index));
        }

        public static IntPtr GetVMTAndFnForThis(this IntPtr @this, uint index, uint offset = 0) {
            return @this.GetVMTForThis().GetVFuncForVtable(index);
        }

        public static T GetVMTAndFnForThis<T>(this IntPtr @this, uint index, uint offset = 0) {
            return @this.GetVMTForThis().GetVFuncFnForVtable<T>(index);
        }

    }
}
