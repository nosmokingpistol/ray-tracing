#include "transform.h"

// Vector3f convert_to_3d(Vector4f v) { // converts vector by removing w
//     return Vector3f(v(0), v(1), v(2));
// };
// Vector4f convert_to_4d(Vector3f v) {
//      return Vector4f(v(0), v(1), v(2), 1);
// };


Matrix4f identity_matrix() {
    Matrix4f identity = Matrix4f::Identity();
    return identity;
};

Matrix4f translation_matrix(float x, float y, float z) {
    Matrix4f transform;
    transform<< 1, 0, 0, x,
            0, 1, 0, y,
            0, 0, 1, z,
            0, 0, 0, 1; 
    return transform;
};

Matrix4f scale_matrix(float x, float y, float z) {
    Matrix4f scale;
    scale<< x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1; 
    return scale;
};

