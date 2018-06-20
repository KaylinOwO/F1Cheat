using System.Runtime.InteropServices;

namespace Knife.Winapi {
    [StructLayout(LayoutKind.Explicit)]
    struct ImageOptionalHeader32 {
        [FieldOffset(4)]
        public uint SizeOfCode;

        [FieldOffset(20)]
        public uint BaseOfCode;
    }
}
