using System;
using Knife.Classes;

namespace Knife.Hack.Hooking_Classes.PaintTraverse {
    class PlayerGlow : IPaintTraverse {
        public Func<bool> Condition => () => Ioc.Get<EngineClient>().InGame && Ioc.Get<EngineClient>().Connected && !Ioc.Get<EngineClient>().DrawingLoadingImage;
        public void OnMatSystem(uint vguiPanel, bool forceRepaint, bool allowForce) {
            foreach (var player in Ioc.Get<EntityList>().AllPlayers) {
                player.ShouldGlow = true;
                player.UpdateGlowEffect();
            }
        }
    }
}
