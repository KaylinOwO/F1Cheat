using System;
using System.Runtime.InteropServices;

namespace Knife.Structs {
    unsafe struct RecvProp {
        IntPtr variableNamePointer;
        fixed byte _pad0[12 + 1 + 20];
        RecvTable* dataTable;
        int offset;
        fixed int _pad1[3];

        public string VariableName => Marshal.PtrToStringAnsi(variableNamePointer);
        public int Offset => offset;
        public RecvTable* ChildDataTable => dataTable;
    }
}