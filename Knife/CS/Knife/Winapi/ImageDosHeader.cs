using System.Runtime.InteropServices;

namespace Knife.Winapi {
    [StructLayout(LayoutKind.Explicit)]
    struct ImageDosHeader {
        [FieldOffset(60)]
        public int e_lfanew;
    }
}
