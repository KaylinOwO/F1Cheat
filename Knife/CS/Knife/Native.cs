using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Runtime.InteropServices;
using Knife.Hack;
using Knife.Winapi;

namespace Knife {
    // Put all the unsafe content e.g. pointers and handles or native content e.g api call here
    static unsafe class Native {

        private static readonly char[] g_AcceptableMask = { 'x', '?' };

        private static bool MatchPattern(byte *ptr, IReadOnlyList<byte> pattern, IReadOnlyList<char> mask) {
            try {
                int j;
                for (j = 0; j < mask.Count; j++) {
                    if (mask[j] == 'x' && ptr[j] != pattern[j]) {
                        return false;
                    }
                }
                return j == pattern.Count;
            }
            catch {
                return false;
            }
        }

        public static IntPtr SignatureScan(this IntPtr address, byte[] pattern, char[] mask, uint len, int offset = 0) {
            if (mask.Except(g_AcceptableMask).Any()) {
                return IntPtr.Zero;
            }

            if (pattern.Length != mask.Length || pattern.Length > len) {
                return IntPtr.Zero;
            }

            for (var i = 0; i < len; i++) {
                if (MatchPattern((byte*) (address + i).ToPointer(), pattern, mask)) {
                    return address + i + offset;
                }
            }
            return IntPtr.Zero;
        }

        public static IntPtr SignatureScan(this IntPtr address, string pattern, string mask, uint len, int offset = 0) {
            return SignatureScan(address, pattern.ToRawByteArray(), mask.ToCharArray(), len, offset);
        }

        public static IntPtr SignatureScan(this IntPtr address, string signature, uint len, int offset = 0) {
            var tup = signature.CompileSmartSignature();

            return SignatureScan(address, tup.Item1, tup.Item2, len, offset);
        }

        public static IntPtr SignatureScan(this string signature, IntPtr address, uint len, int offset = 0) {
            var tup = signature.CompileSmartSignature();

            return SignatureScan(address, tup.Item1, tup.Item2, len, offset);
        }

        public static Tuple<byte[], char[]> CompileSmartSignature(this string sig) {
            var pattern = new List<byte>();
            var mask = new List<char>();

            new List<string>(sig.Split(new[] { ' ', '\r', '\n', '\t' }, StringSplitOptions.RemoveEmptyEntries)).ForEach(str => {
                if (str.Length == 1 && str[0] == '?') {
                    pattern.Add(0x00);
                    mask.Add('?');
                    return;
                }

                if (str.Length % 2 != 0) {
                    throw new FormatException("Signature size is odd sized");
                }

                for (var i = 0; i < str.Length; i += 2) {
                    var hex = $"{str[i]}{str[i + 1]}";

                    if (hex == "??") {
                        pattern.Add(0x00);
                        mask.Add('?');
                    }
                    else {
                        byte result;
                        if (!byte.TryParse(hex, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out result)) {
                            continue;
                        }
                        pattern.Add(result);
                        mask.Add('x');
                    }
                }
            });

            return Tuple.Create(pattern.ToArray(), mask.ToArray());
        }

        public static IntPtr GetModuleHandleSafe(string moduleName) {
            var ptr = IntPtr.Zero;

            while (ptr == IntPtr.Zero) {
                ptr = Kernel32.GetModuleHandle(moduleName);
            }

            return ptr;
        }

        public static Tuple<IntPtr, uint> GetModuleBaseByName(string name) {
            var @base = GetModuleHandleSafe(name);
            if (@base == IntPtr.Zero) {
                return null;
            }

            var dosHeader = (ImageDosHeader*)@base;
            var ntHeader = (ImageNtHeaders32*)(@base + dosHeader->e_lfanew);

            return Tuple.Create(
                (IntPtr)(@base.ToInt32() + ntHeader->OptionalHeader.BaseOfCode),
                (uint)(@base.ToInt32() + ntHeader->OptionalHeader.SizeOfCode)
            );
        }

        public static IntPtr MakeNativeVTableClassByDelegates(this Delegate[] vtableField) {
            var classBase = (IntPtr*) Marshal.AllocHGlobal(IntPtr.Size * (vtableField.Length + 1));

            classBase[0] = IntPtr.Add((IntPtr)classBase, IntPtr.Size); // ptr to vtable
            for (var i = 1; i <= vtableField.Length; i++) {
                classBase[i] = Ioc.Get<MarshalCache>().GetPtrForDel(vtableField[i]);
            }

            return (IntPtr) classBase;
        }
    }
}
