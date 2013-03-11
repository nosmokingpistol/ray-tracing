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


Vector3f Directional_Light::calc_diff(Vector3f diffuse, Vector3f intens, Vector3f normal, Vector3f intersection)
{
    l_vec = coordinates;
    Vector3f diff_values = diffuse.cwiseProduct(intens)*(std::max(0.0f, normal.dot(coordinates)));
    // std::cout << "****directional light diffuse = " << std::endl << diff_values << std::endl;
    return diff_values;
};

Vector3f Point_Light::calc_diff (Vector3f diffuse, Vector3f intens, Vector3f normal, Vector3f intersection)
{
    Vector3f l_vector = coordinates-intersection;
    l_vector.normalize();
    l_vec = l_vector; // save l for reflection
    Vector3f diff_values = diffuse.cwiseProduct(intens)*(std::max(0.0f, normal.dot(l_vector)));
    // std::cout << " ****point light diffuse = " << std::endl << diff_values << std::endl;

    return diff_values;
};

Vector3f Light::calc_spec (Vector3f specular, Vector3f intens, Vector3f normal, Vector3f viewer_direction, Vector3f l_vector, float specular_power)
{
    //normal.normalize();
    // Calculate reflection vector, 2(n*l)n - l
    Vector3f reflection = (2.0*l_vector.dot(normal)*normal) - l_vector;
    reflection.normalize();
    // std::cout << " specular reflection = " << std::endl << reflection << std::endl;
    float r_v = reflection.dot(viewer_direction);
    if (r_v  < 0) {
        return Vector3f(0, 0, 0);
    }
    Vector3f spec_values = specular.cwiseProduct(intens)*pow(r_v, specular_power);
    // std::cout << " ****point light specular = " << std::endl << spec_values << std::endl;
    return spec_values;
};

void Light::print() {
    std::cout << " light coordinates = " << std::endl << coordinates << std::endl;
    std::cout << " light intensities = " << std::endl << intensities << std::endl;

}

