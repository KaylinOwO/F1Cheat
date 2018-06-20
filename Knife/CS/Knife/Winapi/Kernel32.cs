using System;
using System.Runtime.InteropServices;

namespace Knife.Winapi {
    
    class Kernel32 {
        private const string K32 = "kernel32.dll";

        [DllImport(K32, SetLastError = true)]
        public static extern void CloseHandle(
            IntPtr handle);

        [DllImport(K32, SetLastError = true)]
        public static extern bool AllocConsole();

        [DllImport(K32, SetLastError = true)]
        public static extern bool FreeConsole();

        [DllImport(K32, SetLastError = true)]
        public static extern IntPtr LoadLibrary(
            string fileName);

        [DllImport(K32, SetLastError = true)]
        public static extern bool FreeLibrary(
            IntPtr module);

        [DllImport(K32, SetLastError = true)]
        public static extern IntPtr GetProcAddress(
            IntPtr module, string procName);

        [DllImport(K32, SetLastError = true)]
        public static extern int VirtualQuery(
            IntPtr adress,
            out QueryInformation processQuery, int length);

        [DllImport(K32, SetLastError = true)]
        public static extern IntPtr GetModuleHandle(
            string moduleName);

        [DllImport(K32, SetLastError = true)]
        public static extern bool VirtualProtect(
            IntPtr address, uint size,
            Protection newProtect, out Protection oldProtect);

        [DllImport(K32, SetLastError = true)]
        public static extern void CopyMemory(
            IntPtr dest, 
            IntPtr src, uint count);
    }
}
