using System;
using System.Linq;
using Knife.Classes;
using Knife.Enums.TF;

namespace Knife.Hack.Hooking_Classes.PaintTraverse {
    unsafe class SpyUndisguise : IPaintTraverse {
        public Func<bool> Condition => () => Ioc.Get<EngineClient>().InGame && Ioc.Get<EngineClient>().Connected && !Ioc.Get<EngineClient>().DrawingLoadingImage;
        public void OnMatSystem(uint vguiPanel, bool forceRepaint, bool allowForce) {
            foreach (var player in Ioc.Get<EntityList>().AllPlayers.Where(_ => _.Class == TFClass.Spy)) {
                if ((*player.PlayerCondition & PlayerCondition.Disguising) == PlayerCondition.Disguising) {
                    *player.PlayerCondition &= ~PlayerCondition.Disguising;
                }

                if ((*player.PlayerCondition & PlayerCondition.Disguised) == PlayerCondition.Disguised) {
                    *player.PlayerCondition &= ~PlayerCondition.Disguised;
                }

                if ((*player.PlayerCondition & PlayerCondition.Cloaked) == PlayerCondition.Cloaked) {
                    *player.PlayerCondition &= ~PlayerCondition.Cloaked;
                }
            }
        }
    }
}
