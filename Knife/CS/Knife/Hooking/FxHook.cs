using System;
using System.Runtime.InteropServices;
using Knife.Hack;
using Knife.Winapi;

namespace Knife.Hooking {
    class FxHook : IDisposable {
        const int nBytes = 5;

        public IntPtr HookAddress { get; }
        public IntPtr TrampolineAddress { get; }

        private readonly Protection old;
        private readonly byte[] orig = new byte[5];
        private readonly byte[] jmp = new byte[5];

        public FxHook(IntPtr src, IntPtr destination) {
            HookAddress = src;
            TrampolineAddress = destination;

            Kernel32.VirtualProtect(HookAddress, nBytes, Protection.ExecuteReadWrite, out old);
            Marshal.Copy(
                source: src, 
                destination: orig,
                startIndex: 0, 
                length: nBytes
            ); // copy original 5 bytes

            jmp[0] = 0xE9; // JMP 0xddccbbaa
            Array.Copy(
                sourceArray: BitConverter.GetBytes(TrampolineAddress.ToInt32() - src.ToInt32() - nBytes), // offset to new address
                sourceIndex: 0, 
                destinationArray: jmp,
                destinationIndex: 1, // ignore the first entry --- JMP
                length: nBytes - 1
            );
        }
        //public FxHook(IntPtr src, Delegate dest) : this(src, Marshal.GetFunctionPointerForDelegate(dest)) { }
        public FxHook(IntPtr src, Delegate dest) : this(src, Ioc.Get<MarshalCache>().GetPtrForDel(dest)) { }

        public void Install() => Marshal.Copy(
            source: jmp,
            destination: HookAddress,
            startIndex: 0,
            length: nBytes
        ); // copy managed trampoline into addr

        public void Uninstall() => Marshal.Copy(
            source: orig,
            destination: HookAddress,
            startIndex: 0,
            length: nBytes
        ); // copy original orig into addr

        public T GetHookAddressToDeleagte<T>() {
            return Ioc.Get<MarshalCache>().GetDelForPtr<T>(HookAddress);
            //return Marshal.GetDelegateForFunctionPointer<T>(HookAddress);
        }

        public void Dispose() {
            Uninstall();
            Protection x;
            Kernel32.VirtualProtect(
                address: HookAddress, 
                size: nBytes, 
                newProtect: old, 
                oldProtect: out x
            );
        }

    }

}
