using System;
using System.Runtime.InteropServices;

namespace Knife.Winapi {
    [StructLayout(LayoutKind.Sequential)]
    struct QueryInformation {
        public IntPtr BaseAdress; // void*
        public IntPtr AllocationBase; // void*
        public int AllocationProtect; // DWORD
        public uint RegionSize; // size_t
        public uint State; // DWORD
        public uint Protect; // DWORD
        public uint Type; // DWORD
    }
}