using System.Runtime.InteropServices;

namespace Knife.Winapi {
    [StructLayout(LayoutKind.Explicit)]
    struct ImageNtHeaders32 {
        [FieldOffset(24)]
        public ImageOptionalHeader32 OptionalHeader;
    }
}
