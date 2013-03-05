#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <Eigen/Dense>
#include "common.h"
#include <vector>

using namespace Eigen;

Vector3f convert_to_3d(Vector4f v);
Vector4f convert_to_4d(Vector3f v);

Matrix4f identity_matrix(); 
// TransformMatrix translate_matrix(float x, float y, float z) {

Matrix4f translation_matrix(float x, float y, float z);
Matrix4f scale_matrix(float x, float y, float z);

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

    Matrix4f inverse() {
        return matrix.inverse();
    };

    TransformMatrix translate_matrix(float x, float y, float z);
    
    void print() {
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
};

class Transformation
{
    public:
        std::vector<TransformMatrix> transformations;
        Matrix4f transform_ray_origin; 
        Matrix4f transform_ray_dir; 
        Matrix4f get_normal; // given transformed normal, get original normal 

        void initialize() {
            std::cout << "!!!!!!!!!! initializing transformation!!" << std::endl;
            transform_ray_origin_matrix();
            transform_ray_dir_matrix();
            get_normal_matrix();
        };

        void transform_ray_origin_matrix() {
            // returns inverse of all transformations in reverse order;
            transform_ray_origin = identity_matrix();
            for(std::vector<TransformMatrix>::reverse_iterator rit = transformations.rbegin();
                rit != transformations.rend(); ++rit) {
                TransformMatrix cur = *rit;
                transform_ray_origin = transform_ray_origin * cur.inverse();
            }
            std::cout << " transform ray origin matrix = " << std::endl;
            std::cout << transform_ray_origin << std::endl;
        };
        void transform_ray_dir_matrix() {
            // returns inverse of all transformations ( not translation) in reverse order;
            transform_ray_dir = identity_matrix();
            for(std::vector<TransformMatrix>::reverse_iterator rit = transformations.rbegin();
                rit != transformations.rend(); ++rit) {
                TransformMatrix cur = *rit;
                if (cur.type != 0) { // not a translation
                    transform_ray_dir = transform_ray_dir * cur.inverse();       
                }
            }
            std::cout << " transform_ray_dir matrix = " << std::endl;
            std::cout << transform_ray_dir << std::endl;
        };
        void get_normal_matrix() {
            // ??????
            get_normal = identity_matrix();

        }

        Transformation() {};
        
        void add_transformation(TransformMatrix m) {
            transformations.push_back(m);
        };

        void print() {
            std::cout << "print transformation: " << std::endl;
            for(std::vector<TransformMatrix>::iterator it = transformations.begin() ; it != transformations.end(); ++it) {
                (*it).print();
            }

        };

        // void rotation_matrix(float x, float y, float z, float theta);
        void transform_ray(Ray& ray) {
            // if M = T S then inverse ray transformation = S^ T ^
            // http://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html#SECTION00024100000000000000
            // apply scale & rotate inverses for distance
            // apply scale rotate translate inverses for position

            // apply inverse transformation M^-1 to matrix 
            // only do translations for now

            // std::cout << "before, ray pos = " << ray.pos << std::endl;
            
            Vector4f pos = convert_to_4d(ray.pos);
            Vector4f new_pos = transform_ray_origin*pos;
            ray.pos = convert_to_3d(new_pos);

            // std::cout << "after, ray pos = " << ray.pos << std::endl;

            std::cout << "before, ray dir = " << ray.dir << std::endl;

            Vector4f dir = convert_to_4d(ray.dir);
            Vector4f new_dir = transform_ray_dir*dir;
            ray.dir = convert_to_3d(new_dir);

            std::cout << "after, ray dir = " << ray.dir << std::endl;
            
        };


};



#endif
