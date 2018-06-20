using System;

namespace Knife.Enums {
    [Flags]
    enum PlayerControls {
        InAttack = 1 << 0,
        InJump = 1 << 1,
        InDuck = 1 << 2,
        InForward = 1 << 3,
        InBack = 1 << 4,
        InUse = 1 << 5,
        InCancel = 1 << 6,
        InLeft = 1 << 7,
        InRight = 1 << 8,
        InMoveLeft = 1 << 9,
        InMoveRight = 1 << 10,
        InAttack2 = 1 << 11,
        InRun = 1 << 12,
        InReload = 1 << 13,
        InAlt1 = 1 << 14,
        InAlt2 = 1 << 15,
        InScore = 1 << 16,   // Used by client.dll for when scoreboard is held down
        InSpeed = 1 << 17,   // Player is holding the speed key
        InWalk = 1 << 18,    // Player holding walk key
        InZoom = 1 << 19,    // Zoom key for HUD zoom
        InWeapon1 = 1 << 20, // weapon defines these bits
        InWeapon2 = 1 << 21, // weapon defines these bits
        InBullrush = 1 << 22,
    };
}
