namespace Knife.Structs {
    struct GlobalVarsBase {
        public float Realtime { get; set; }
        public int FrameCount { get; set; }
        public float AbsoluteFrameTime { get; set; }
        public float CurrentTime { get; set; }
        public float FrameTime { get; set; }
        public int MaxClients { get; set; }
        public int TickCount { get; set; }
        public float IntervalPerTick { get; set; }
        public float InterpolationAmount { get; set; }
        public int SimTicksThisFrame { get; set; }
        public int NetworkProtocol { get; set; }
    };
}
