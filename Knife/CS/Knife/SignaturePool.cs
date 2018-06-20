using System;
using System.Collections.Generic;
using SignatureSet = System.Collections.Generic.Dictionary<string, System.Tuple<string, int>>;

namespace Knife {
    // very crude signature searcher
    // WIP
    class SignaturePool {
        public SignaturePool() {
            Init();
        }

        private Dictionary<string, SignatureSet> SignatureMap { get; } = new Dictionary<string, SignatureSet> {
            //{ "engine.dll", new SignatureSet {
            //    {"AppSystem", Tuple.Create("A1 ? ? ? ? 8B 11 68 ? ? ? ? ", 1)},
            //    {"GlobalVar", Tuple.Create("A1 ? ? ? ? 8B 11 68 ? ? ? ? ", 8)}
            //} },
            //{ "client.dll", new SignatureSet {
            //    {"UpdateGlowEffectFn", Tuple.Create("55 8B EC 83 EC ? 56 8B F1 83 BE ? ? ? ? ? 74 ? 8B 06 FF 90", 0)},
            //    {"GetCrosshairScaleFn", Tuple.Create("55 8B EC 51 56 57 8B F9 E8 ? ? ? ? 8B F0 85 F6 0F 84 ? ? ? ? 8B 16 8B CE 8B 92 ? ? ? ? FF D2 84 C0 0F 84 ? ? ? ? 6A 01 6A 00 57", 0)}
            //} },
        };

        private Dictionary<string, Dictionary<string, IntPtr>> SignatureResult { get; } =
            new Dictionary<string, Dictionary<string, IntPtr>>();

        void Init() {
            SignatureMap.Each(modulePair => {
                var moduleName = modulePair.Key;
                var moduleSigDict = modulePair.Value;

                var module = Native.GetModuleBaseByName(moduleName);

                if (module == null) {
                    return;
                }

                if (!SignatureResult.ContainsKey(moduleName)) {
                    SignatureResult.Add(moduleName, new Dictionary<string, IntPtr>());
                }

                moduleSigDict.Each(sigPair => {
                    if (sigPair.Value == null) {
                        return;
                    }

                    SignatureResult[moduleName][sigPair.Key] = sigPair.Value.Item1.SignatureScan(module.Item1, module.Item2) + sigPair.Value.Item2;
                });
            });
        }

        public Dictionary<string, IntPtr> this[string key] => SignatureResult[key];
    }
}
