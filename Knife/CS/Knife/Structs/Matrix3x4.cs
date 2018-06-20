namespace Knife.Structs {
    unsafe struct Matrix3x4 {
        public fixed float m_matrixBase[3*4]; // it is not 4x4 in SourceEngine so no System.Numerics.Matrix4x4!!!!

        public float this[int i, int j] {
            get {
                fixed (Matrix3x4* @this = &this) {
                    return @this->m_matrixBase[i * 4 + j];
                }
            }
        }
    }
}
