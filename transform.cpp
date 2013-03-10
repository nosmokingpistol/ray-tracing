#include "transform.h"
#include <cmath>

#define PI 3.14159265


Vector3f convert_to_3d(Vector4f v) { // converts vector by removing w
    return Vector3f(v(0), v(1), v(2));
}

Vector4f convert_to_4d(Vector3f v) {
   return Vector4f(v(0), v(1), v(2), 1);
}

Matrix4f identity_matrix() {
    Matrix4f identity = Matrix4f::Identity();
    return identity;
}

Matrix4f translation_matrix(float x, float y, float z) {
    Matrix4f transform;
    transform<< 1, 0, 0, x,
    0, 1, 0, y,
    0, 0, 1, z,
    0, 0, 0, 1; 
    return transform;
}

Matrix4f scale_matrix(float x, float y, float z) {
    Matrix4f scale;
    scale<< x, 0, 0, 0,
    0, y, 0, 0,
    0, 0, z, 0,
    0, 0, 0, 1; 
    return scale;
}

Matrix4f rotation_matrix(float x, float y, float z, float theta) {
    // R = u*u^t + cos(theta) (I - u*u^t) + sin(theta)S
    // s = [ 0 -Uz Uy]
    //     [Uz 0 -Ux ]
    //     [-Uy Ux 0 ]

    // u*u^t = [ UxUx UxUy UxUz]
    //         [ UyUx UyUy UyUz]
    //         [ UzUx UzUy UzUz]
    Matrix3f rotation;
    theta = theta*PI / 180.0;
    Vector3f u(x, y, z);
    u.normalize();
    
    Matrix<float, 3, 3> uut;
    uut << u(0)*u(0), u(0)*u(1), u(0)*u(2),
    u(1)*u(0), u(1)*u(1), u(1)*u(2),
    u(2)*u(0), u(2)*u(1), u(2)*u(2);
    // calculate cos(theta) (I - u*u^t)
    Matrix3f identity =  Matrix3f::Identity();
    identity = identity - uut;
    identity = identity * cos(theta);

    Matrix3f S;
    S << 0, -u(2), u(1),
    u(2), 0, u(0),
    -u(1), u(0), 0;
    rotation = uut + identity + S*sin(theta);

    // std::cout << " u = " << std::endl;
    // std::cout << u << std::endl;
    // std::cout << "u mult u transpose= " << std::endl;
    // std::cout << uut << std::endl; 
    // std::cout << "rotation= " << std::endl;
    // std::cout << rotation << std::endl;
    
    Matrix4f homogeneous_rot;
    homogeneous_rot << rotation(0,0), rotation(0,1), rotation(0,2), 0,
    rotation(1,0), rotation(1,1), rotation(1,2), 0,
    rotation(2,0), rotation(2,1), rotation(2,2), 0,
    0, 0, 0, 1;
    std::cout << "*************homogeneous_rot= " << std::endl;
    std::cout << homogeneous_rot << std::endl;
    return homogeneous_rot;
}

void TransformMatrix::print() {
    if (type == 0) {
        std::cout << " translation matrix = " << std::endl;
    }
    else if (type == 1) {
        std::cout << " scale matrix = " << std::endl;
    }
    else if (type == 2) {
        std::cout << " rotation matrix = " << std::endl;
    }
    std::cout << matrix << std::endl;
}

void Transformation::initialize() {
    get_intersection_matrix(); 
    transform_ray_origin_matrix();
    transform_ray_dir_matrix();
    get_normal_matrix();
}

void Transformation::transform_ray_origin_matrix() {
        // returns inverse of all transformations in reverse order;
    transform_ray_origin = get_intersection;
    transform_ray_origin = transform_ray_origin.inverse().eval();
    // transform_ray_origin = identity_matrix();
    // for(std::vector<TransformMatrix>::reverse_iterator rit = transformations.rbegin();
    //     rit != transformations.rend(); ++rit) {
    //     TransformMatrix cur = *rit;
    // transform_ray_origin = transform_ray_origin * cur.inverse();
    // }
// std::cout << " transform ray origin matrix = " << std::endl;
// std::cout << transform_ray_origin << std::endl;
}

void Transformation::transform_ray_dir_matrix() {
    // returns inverse of all transformations ( not translation) in reverse order;
    transform_ray_dir = identity_matrix();
    for(std::vector<TransformMatrix>::reverse_iterator rit = transformations.rbegin();
        rit != transformations.rend(); ++rit) {
        TransformMatrix cur = *rit;
        if (!cur.is_translation()) { // not a translation
            transform_ray_dir = transform_ray_dir * cur.inverse();       
        }
    }
    // std::cout << " transform_ray_dir matrix = " << std::endl;
    // std::cout << transform_ray_dir << std::endl;
}

void Transformation::get_normal_matrix() {
    get_normal = identity_matrix();
    for(std::vector<TransformMatrix>::iterator it = transformations.begin();
        it != transformations.end(); ++it) {
        TransformMatrix cur = *it;
        if (cur.is_rotation()) {
            get_normal = get_normal * cur.matrix;
        }
        else if (cur.is_scale()) {
            get_normal = get_normal * cur.inverse();
        }
    }
}

void Transformation::get_intersection_matrix() {
    get_intersection = identity_matrix();
    for(std::vector<TransformMatrix>::iterator it = transformations.begin();
        it != transformations.end(); ++it) {
        TransformMatrix cur = *it;
        get_intersection = get_intersection * cur.matrix;
    }   
}

void Transformation::print() {
    std::cout << "print transformation: " << std::endl;
    for(std::vector<TransformMatrix>::iterator it = transformations.begin() ; it != transformations.end(); ++it) {
        (*it).print();
    }
}

Ray Transformation::transform_ray(Ray ray) {
    // std::cout<< " transform ray!!!!!!!!!!!!!!!" << std::endl;
    // http://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html#SECTION00024100000000000000
    
    // std::cout << "before, ray pos = " << ray.pos << std::endl;
    Vector4f pos = convert_to_4d(ray.pos);
    Vector4f new_pos = transform_ray_origin*pos;
    ray.pos = convert_to_3d(new_pos);
    // std::cout << "after, ray pos = " << ray.pos << std::endl;

    // std::cout << "before, ray dir = " << ray.dir << std::endl;
    Vector4f dir = convert_to_4d(ray.dir);
    Vector4f new_dir = transform_ray_dir*dir;

    /// need to normalize dir???
    new_dir.normalize();
    
    ray.dir = convert_to_3d(new_dir);
    // std::cout << "after, ray dir = " << ray.dir << std::endl;
    return ray;
}
void Transformation::transform_intersection(Vector3f& intersect) {
    // std::cout << "transform_intersection , before =  " << std::endl << intersect << std::endl;
    Vector4f inter = convert_to_4d(intersect);
    Vector4f new_intersect = get_intersection*inter;
    intersect = convert_to_3d(new_intersect);
    // std::cout << "           , after =  " << std::endl << intersect << std::endl;
    return;
}

void Transformation::transform_normal(Vector3f& normal) {
    // std::cout << "transform_normal , before =  " << std::endl << normal << std::endl;

    Vector4f norm = convert_to_4d(normal);
    Vector4f new_normal = get_normal*norm;
    normal = convert_to_3d(new_normal);
    normal.normalize();
    // std::cout << "           , after =  " << std::endl << normal << std::endl;

    return;
}


