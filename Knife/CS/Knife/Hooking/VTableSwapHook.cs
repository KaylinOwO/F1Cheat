using System;
using System.Runtime.InteropServices;
using Knife.Hack;
using Knife.Winapi;

namespace Knife.Hooking {
    // Credits for aganonki@UC
    unsafe class VTableSwapHook {

        public IntPtr BaseClass { get; }
        public IntPtr* OldVMT { get; }
        public IntPtr* VMT { get; }
        private uint Size { get; }

        // class { void ** *vtable; ... }
        // IntPtr == void *
        // class { IntPtr* *vtable; ... }
        public VTableSwapHook(IntPtr baseClass) {
            BaseClass = baseClass;
            VMT = *(IntPtr**)BaseClass;
            OldVMT = (IntPtr*)Marshal.AllocHGlobal((int)ByteSize);

            Kernel32.CopyMemory(
                src: (IntPtr)VMT,
                dest: (IntPtr)OldVMT,
                count: ByteSize
                );

            Size = 1000;
        }

        public IntPtr HookFunction(uint index, IntPtr fn) {
            Protection oldProtect;
            Kernel32.VirtualProtect(
                address: (IntPtr)VMT,
                size: Size,
                newProtect: Protection.ExecuteReadWrite,
                oldProtect: out oldProtect
                );
            VMT[index] = fn;
            Kernel32.VirtualProtect(
                address: (IntPtr)VMT,
                size: Size,
                newProtect: oldProtect,
                oldProtect: out oldProtect
                );
            return OldVMT[index];
        }

        public IntPtr HookFunction(uint index, Delegate fn)
            => //HookFunction(index, Marshal.GetFunctionPointerForDelegate(fn));
            HookFunction(index, Ioc.Get<MarshalCache>().GetPtrForDel(fn));


        public T GetOldVMT<T>(uint index) {
            return Ioc.Get<MarshalCache>().GetDelForPtr<T>(OldVMT[index]);
        }

        public void UnHookFunction(uint index) {
            Protection oldProtect;
            Kernel32.VirtualProtect(
                address: (IntPtr)VMT,
                size: Size,
                newProtect: Protection.ExecuteReadWrite,
                oldProtect: out oldProtect
                );
            VMT[index] = OldVMT[index];
            Kernel32.VirtualProtect(
                address: (IntPtr)VMT,
                size: Size,
                newProtect: oldProtect,
                oldProtect: out oldProtect
                );
        }

        public uint Members {
            get {
                var index = 0u;
                for (; VMT[index] != IntPtr.Zero; index++) ;
                return index;
            }
        }

        public uint ByteSize => Members * sizeof(uint);

        public void Dispose() {
            Protection oldProtect;
            Kernel32.VirtualProtect((IntPtr)VMT, Size, Protection.ExecuteReadWrite, out oldProtect);
            Kernel32.CopyMemory(
                dest: (IntPtr)OldVMT,
                src: (IntPtr)VMT,
                count: ByteSize
                ); // restore from old table
            Kernel32.VirtualProtect((IntPtr)VMT, Size, oldProtect, out oldProtect);
            Marshal.FreeHGlobal((IntPtr)OldVMT); // f**king typecast no f**king way
        }
    }
}
