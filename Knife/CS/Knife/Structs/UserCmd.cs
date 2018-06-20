using System;
using System.Numerics;
using System.Runtime.InteropServices;
using Knife.Enums;

namespace Knife.Structs
{
	[StructLayout(LayoutKind.Sequential, Pack = 1)]
    struct UserCmd {
        private IntPtr Destructor { get; set; }
        public int CommandNumber { get; set; }
        public int TickCount { get; set; }
        public Vector3 ViewAngles { get; set; }
        public float ForwardMove { get; set; }
        public float SideMove { get; set; }
        public float UpMove { get; set; }
        public PlayerControls Buttons { get; set; }
        public byte Impulse { get; set; }
        public int WeaponSelect { get; set; }
        public int WeaponSubtype { get; set; }
        public int RandomSeed { get; set; }
        public short MouseXDerivative { get; set; }
        public short MouseYDerivative { get; set; }
        public byte HasBeenPredicted { get; set; }

        public override string ToString() => $"CommandNumber: {CommandNumber};\n" +
                                             $"TickCount: {TickCount};\n" +
                                             $"ViewAngles: \"{ViewAngles}\";\n" +
                                             $"ForwardMove: {ForwardMove};\n" +
                                             $"SideMove: {SideMove};\n" +
                                             $"UpMove: {CommandNumber};\n" +
                                             $"Buttons: {Buttons};\n" +
                                             $"Impulse: {Impulse};\n" +
                                             $"WeaponSelect: {ViewAngles};\n" +
                                             $"WeaponSubtype: {ForwardMove};\n" +
                                             $"RandomSeed: {RandomSeed};\n" +
                                             $"MouseXDerivative: {MouseXDerivative};\n" +
                                             $"MouseYDerivative: {MouseYDerivative};\n" +
                                             $"HasBeenPredicted: {HasBeenPredicted};\n";
        
    }
}
