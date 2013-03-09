#ifndef LIGHT_H
#define LIGHT_H

#include <Eigen/Dense>
#include <vector>
#include "common.h"

class Light
{
    public:
    Vector3f coordinates, intensities, l_vec;

    Vector3f calc_amb (Vector3f ambience, Vector3f intensities);
    virtual Vector3f calc_diff (Vector3f diffuse, Vector3f intensities, Vector3f normal){};
    Vector3f calc_spec (Vector3f specular, Vector3f intensities, Vector3f normal, Vector3f l_vector, float specular_power);
    void print();
};

class Directional_Light : public Light
{
    public:
    Directional_Light (float x, float y, float z, float r, float g, float b) {
        coordinates << x, y, z;
        intensities << r, g, b;
    }

    virtual Vector3f calc_diff (Vector3f diffuse, Vector3f intensities, Vector3f normal);
};

class Point_Light : public Light
{
    public:
    Point_Light (float x, float y, float z, float r, float g, float b) {
        coordinates << x, y, z;
        intensities << r, g, b;
    }
    virtual Vector3f calc_diff (Vector3f diffuse, Vector3f intensities, Vector3f normal);
};
#endif
