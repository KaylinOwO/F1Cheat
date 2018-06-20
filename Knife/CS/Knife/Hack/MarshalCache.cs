using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Knife.Hack {
    // 
    class MarshalCache {
        private readonly IDictionary<IntPtr, object> m_ptrDelMap = new ConcurrentDictionary<IntPtr, object>();
        private readonly IDictionary<Delegate, IntPtr> m_delPtrMap = new ConcurrentDictionary<Delegate, IntPtr>();

        public T GetDelForPtr<T>(IntPtr ptr) {
            if (!m_ptrDelMap.ContainsKey(ptr)) {
                m_ptrDelMap[ptr] = Marshal.GetDelegateForFunctionPointer<T>(ptr);
            }

            return (T)m_ptrDelMap[ptr];
        }

        public IntPtr GetPtrForDel(Delegate del) {
            if (!m_delPtrMap.ContainsKey(del)) {
                m_delPtrMap[del] = Marshal.GetFunctionPointerForDelegate(del);
            }

            return m_delPtrMap[del];
        }
    }
}
