#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <Eigen/Dense>
#include "common.h"
#include <vector>

using namespace Eigen;

Vector3f convert_to_3d(Vector4f v);
Vector4f convert_to_4d(Vector3f v);

Matrix4f identity_matrix(); 
Matrix4f translation_matrix(float x, float y, float z);
Matrix4f scale_matrix(float x, float y, float z);
Matrix4f rotation_matrix(float x, float y, float z, float theta);

class TransformMatrix 
{
    public:
    Matrix4f matrix;
    int type; // 0 = translation, 1 = scale, 2 = rotation
    TransformMatrix() {};
    TransformMatrix(Matrix4f m, int transform_type) {
        matrix = m;
        type = transform_type;
    };

    Matrix4f inverse() { return matrix.inverse();};
    bool is_translation() { return (type == 0);};
    bool is_scale() { return (type == 1);};
    bool is_rotation() { return (type == 2);};
    void print();

};

class Transformation
{
    public:
        std::vector<TransformMatrix> transformations;
        Matrix4f transform_ray_origin; 
        Matrix4f transform_ray_dir; 
        Matrix4f get_intersection; // given transformed normal, get original normal 
        Matrix4f get_normal; // given transformed normal, get original normal 

        Transformation() {};
        void initialize(); 
        void transform_ray_origin_matrix();
        void transform_ray_dir_matrix();
        void get_normal_matrix();
        void get_intersection_matrix();
        void add_transformation(TransformMatrix m) {transformations.push_back(m);};
        void transform_ray(Ray& ray);
        void transform_intersection(Vector3f& intersect);
        void transform_normal(Vector3f& normal);
        void print();

};

#endif
