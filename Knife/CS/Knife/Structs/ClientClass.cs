using System;
using System.Runtime.InteropServices;

namespace Knife.Structs {
    unsafe struct ClientClass {
        private fixed int _pad0[2];
        private IntPtr networkNamePointer;
        private RecvTable* recvTable;
        private ClientClass* next;
        private int classID;

        public string NetworkName => Marshal.PtrToStringAnsi(networkNamePointer);
        public int ID => classID;
        public RecvTable* ReceiveTable => recvTable;
        public ClientClass* Next => next;
    }
}