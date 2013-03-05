#include <cmath>
#include "common.h"

// B^2 - 4AC
float discriminant(float A, float B, float C) { 
    return pow(B, 2) - 4*A*C;
}

// returns a float >= 0, assumes discriminant >= 0
//(-B +-(B^2 -4AC)) ^ .5   / 2a
float solve_quadratic(float A, float B, float C) {
    float dis = discriminant(A, B, C); // B^2 - 4AC
    // the smaller positive root is the closest intersection point
    // compute t0 and if it is positive, then we are done, else compute t1
    float t0 = (-B - sqrt(dis)) / (2*A); 
    if (t0 >=0) return t0;
    float t1 = (-B + sqrt(dis)) / (2*A); 
    return t1;
}

