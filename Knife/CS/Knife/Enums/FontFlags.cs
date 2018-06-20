using System;

namespace Knife.Enums {
    [Flags]
    enum FontFlags {
        None,
        Italic = 0x001,
        Underline = 0x002, // not 'Under Line'
        StrikeOut = 0x004,
        Symbol = 0x008,
        AntiAlias = 0x010,
        GaussianBlur = 0x020,
        Rotary = 0x040,
        DropShadow = 0x080,
        Additive = 0x100,
        Outline = 0x200,
        Custom = 0x400,        // custom generated font - never fall back to asian compatibility mode
        Bitmap = 0x800,        // compiled bitmap font - no fallbacks
    };
}
