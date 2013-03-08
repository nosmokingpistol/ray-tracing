#ifndef COMMON_H
#define COMMON_H

#include <Eigen/Dense>
#include <iostream>
using namespace Eigen;

float discriminant(float A, float B, float C);
float solve_quadratic(float A, float B, float C);
float manhattan_distance(Vector3f A, Vector3f B);

class Ray
{
    public:
        Vector3f pos;
        Vector3f dir;
        float t_min, t_max;
        Ray() {};
        Ray(Vector3f p, Vector3f d, float min, float max):
            pos(p), dir(d), t_min(min), t_max(max) {};
        void print() {
            std::cout << " ray(t) = "<< pos <<"+ t*" << dir << std::endl;
        };
};

#endif 
