#include "light.h"
#include "common.h"

#include <Eigen/Dense>
#include <Eigen/Geometry>

using namespace Eigen;

Vector3f Light::calc_amb(Vector3f ambience, Vector3f intens)
{
    Vector3f amb_values = ambience.cwiseProduct(intens);
    return amb_values;
};


Vector3f Directional_Light::calc_diff(Vector3f diffuse, Vector3f intens, Vector3f normal)
{
    l_vec = coordinates;
    Vector3f diff_values = diffuse.cwiseProduct(intens)*(std::max(0.0f, normal.dot(coordinates)));
    return diff_values;
};

Vector3f Point_Light::calc_diff (Vector3f diffuse, Vector3f intens, Vector3f normal)
{
    Vector3f l_vector = coordinates-normal;
    l_vec = l_vector;
    Vector3f diff_values = diffuse.cwiseProduct(intens)*(std::max(0.0f, normal.dot(l_vector)));
    return diff_values;
};

Vector3f Light::calc_spec (Vector3f specular, Vector3f intens, Vector3f normal, Vector3f l_vector, float specular_power)
{
    Vector3f reflection = (2.0*normal.dot(l_vector)*normal)-l_vector;
    Vector3f z;
    z << 0, 0, 0;
    z.normalize();

    Vector3f spec_values = specular.cwiseProduct(intensities)*pow(std::max(0.0f, reflection.dot(z)), specular_power);
    return spec_values;
};
