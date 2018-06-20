using System;

namespace Knife.Hack {
    interface IPaintTraverse {
        Func<bool> Condition { get; }
        void OnMatSystem(uint vguiPanel, bool forceRepaint, bool allowForce);
    }
}
