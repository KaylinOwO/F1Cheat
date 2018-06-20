using System;
using System.Runtime.InteropServices;

namespace Knife.Structs {
    unsafe struct RecvTable {
        RecvProp* properties;
        int propsCount;
        void* unused;
        IntPtr tableNamePointer;
        fixed byte _pad0[2];

        public RecvProp* Properties => properties;
        public int PropertiesCount => propsCount;
        public string TableName => Marshal.PtrToStringAnsi(tableNamePointer);
    }
}