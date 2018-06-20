using System;

namespace Knife.Classes {
    abstract class VTableProxyBase {
        public IntPtr Self { get; set; }

        protected VTableProxyBase(IntPtr @this) {
            Self = @this;
        }
    }
}
