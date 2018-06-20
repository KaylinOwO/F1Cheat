using System;

namespace Knife.Hack.Hooking_Classes.PaintTraverse {
    class PaintTraverseTest : IPaintTraverse {
        public Func<bool> Condition => () => true;
        public void OnMatSystem(uint vguiPanel, bool forceRepaint, bool allowForce) {
            Console.WriteLine("hello");
        }
    }
}
