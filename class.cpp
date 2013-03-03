// #include "util.h"
// #include <iostream>
// #include <cmath>
// #include "FreeImage.h"

#include "class.h"
#include <Eigen/Dense>
#include <Eigen/Geometry> 


#define PI 3.14159265

using namespace Eigen;

// B^2 - 4AC
float discriminant(float A, float B, float C) { 
    return pow(B, 2) - 4*A*C;
};

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
};

bool Sphere::intersect(Ray& ray, float* thit, Vector3f& intersect, Vector3f& normal){
    // A = dir * dir
    float A = ray.dir.dot(ray.dir);
    // B = 2 dir * (pos - center)
    float B = 2.0 * ray.dir.dot(ray.pos - center);
    // C =  (pos - center) * (pos - center) - r^2
    float C = (ray.pos - center).dot(ray.pos - center) - radius*radius;
    std::cout << " A = " << A << " B = " << B << " C = " << C << std::endl;
    if (discriminant(A, B, C) < 0) { // no intersection
        std::cout <<  " No intersection " << std::endl << std::endl;
        return false;
    }
    // discriminant >=0,  ray is tangent or intersects in 2 pts
    float t = solve_quadratic(A, B, C);
    // Ri = [xi, yi, zi] = [x0 + xd * ti ,  y0 + yd * ti,  z0 + zd * ti]
    intersect = ray.pos + ray.dir*t;
    //Unit N at surface SN = [(xi - xc)/Sr,   (yi - yc)/Sr,   (zi - zc)/Sr]
    normal = (intersect - center)/radius;
    std::cout << " intersect point = "<< intersect << " normal = "<< normal<< std::endl;
    return true;
}; 

bool Sampler::generateSample(Sample* sample) { 
    if (cur_w == width) {
        return false;
    }
    *sample = Sample(cur_w, cur_h);
    cur_h++;
    if (cur_h == height) {
        cur_w++;
        cur_h = 0;   
    }
    return true;
}

Film::Film(const char*  name, float w, float h) {
    filename = name;
    width = w;
    height = h;
    FreeImage_Initialise();
    bitmap = FreeImage_Allocate(width, height, 24); // BPP 24 what does this mean
    if (! bitmap ) 
        std::cout << "Error: bitmap not made" << std::endl;

}

void Film::commit(Sample& sample, Vector3f& color){
    image_color.rgbRed = color[0];
    image_color.rgbGreen = color[1];
    image_color.rgbBlue = color[2];
    FreeImage_SetPixelColor( bitmap , sample.x , sample.y, &image_color );
    // std::cout << "set pixel " << sample.x << " , " << sample.y << " to r=" << color.r << " g=" << color.g << " b=" << color.b << std::endl;

}

void Film::writeImage() {
    if (FreeImage_Save(FIF_PNG, bitmap ,filename, 0)){
        std::cout << " Film wrote image to " << filename << std::endl;
    }
    else {
        std::cout << " Error: Film image did not save"<< std::endl;
    }
    FreeImage_DeInitialise(); //Cleanup !
}

Camera::Camera(Vector3f lfrm, Vector3f lat, Vector3f u, float fov, float screen_width, float screen_height){
// http://courses.csusm.edu/cs697exz/camera.html    
    eye_pos = lfrm;
    lookat = lat;
    up = u;
    fov_y = fov * PI/180; // store as radians
    width = screen_width;
    height = screen_height;
 
    // W = eye - lookat
    W = (eye_pos - lookat);
    W.normalize();
    N = W * -1.0;
    U = up.cross(W);
    U.normalize();
    // V = W x U
    V = W.cross(U);
    V.normalize();

    std::cout << " Camera eye = " << eye_pos  << " V = "<< V << " U = " << U<< " N = " << N << std::endl;
    std::cout << " screen_width = " << screen_width << " screen_height = " << screen_height;
    std::cout << " image_height = " << image_height << " image_width = " << image_width;
    std::cout << " pixel_width = " << pixel_width << " pixel_height = " << pixel_height << std::endl;
}

void Camera::generateRay(Sample &sample, Ray* ray){
    // R(t) = E + t(P-E)
    ray->pos = eye_pos;
    // x = tan(fovx/2)((x - width/2 )) / width/2
    Vector3f x = U*(sample.x-(width/2.0));
    Vector3f y = V*((height/2.0)-sample.y);
    Vector3f pixel = x + y;    

    ray->dir = (pixel + N);
    ray->dir.normalize();
   
    // cout << " x = "; x.print (); cout << " y = "; y.print(); cout << endl;
    // std::cout << "ray for sample " << sample.x << ", " << sample.y << " has pos ";
    // ray->pos.print();
    // std::cout << " has dir ";
    // ray_direction.print();
    // std::cout << std::endl;
}

void RayTracer::trace(Ray& ray, int depth, Vector3f* color) {
        // for each object, determine closet object of intersection
        std::vector<Primitive*>::iterator itr;
        Vector3f normal;
        Vector3f intersect;
        float thit;
        for(itr = primitives.begin(); itr != primitives.end(); ++itr) {
            // std::cout<< " ****looping" << std::endl;
            Primitive& cur_prim = **itr;
            // cur_prim.print();  
            if (cur_prim.intersect(ray, &thit, intersect, normal)) {
                std::cout << " setting color to red!!" << std::endl;
                *color = Vector3f(100, 0, 0);
                // std::cout << "intersection: t = " << thit << " intersect point = ";
                // intersect.print();
                // std::cout << "normal = ";
                // std::cout << std::endl;
            }
            else {// no intersection
                *color = Vector3f(0, 0, 0);
                                      // cout << " BLACK!!" << endl;
                                  }

        }
    }

bool Triangle::intersect(Ray&ray, float* thit, Vector3f& intersect, Vector3f& normal) {
    // plane: (P0 + P1t ) * n = A*n    
    // t = (A*n - P0*N)/P1*N
    // t = (Vector(A,P)*N )/(ray.dir*N);
    float t = (intersect - A).dot(N) / ray.dir.dot(N);
    // float t = dot_product(Vector(A,intersect), N)/dot_product(ray.dir, N);
    
    // Check if is inside triangle
    // Use www .cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld018.htm
    // Vector v_1_ab = Vector(ray.pos,A);
    // Vector v_2_ab = Vector(ray.pos,B);
    // Vector N_ab = normalize(vector_cross_product(v_2_ab, v_1_ab));
    Vector3f v_1_ab = A - ray.pos;
    Vector3f v_2_ab = B - ray.pos;
    Vector3f N_ab = v_2_ab.cross(v_1_ab);
    N_ab.normalize();

    // Vector v_1_bc = Vector(ray.pos,B);
    // Vector v_2_bc = Vector(ray.pos,C);
    // Vector N_bc = normalize(vector_cross_product(v_2_bc, v_1_bc));
    Vector3f v_1_bc = B - ray.pos;
    Vector3f v_2_bc = C - ray.pos;
    Vector3f N_bc = v_2_bc.cross(v_1_bc);
    N_bc.normalize();
    
    // Vector v_1_ca = Vector(ray.pos,C);
    // Vector v_2_ca = Vector(ray.pos,A);
    // Vector N_ca = normalize(vector_cross_product(v_2_ca, v_1_ca));
    Vector3f v_1_ca = C - ray.pos;
    Vector3f v_2_ca = A - ray.pos;
    Vector3f N_ca = v_2_ca.cross(v_1_ca);
    N_ca.normalize();
    
    // if ((dot_product(intersect, N_ab) + dot_product(Vector(ray.pos, intersect), N_ab))<0
    //     && (dot_product(intersect, N_bc) + dot_product(Vector(ray.pos, intersect), N_bc))<0
    //     && (dot_product(intersect, N_ca) + dot_product(Vector(ray.pos, intersect), N_ca))<0){
    //     cout <<  " No intersection ";
    //     return false;
    // } else {
    //     Point i = point_vector_add(ray.pos, vector_scalar_multiply(ray.dir, t));
    //     normal = N;
    //     intersect = i;
    //     return true;
    // }

    if ((intersect.dot(N_ab) + (intersect - ray.pos).dot(N_ab) < 0)
        && (intersect.dot(N_bc) + (intersect - ray.pos).dot(N_bc) < 0)
        && (intersect.dot(N_ca) + (intersect - ray.pos).dot(N_ca) < 0)) {
        std::cout <<  " No intersection ";
        return false;
    }
    else {
        normal = N;
        intersect = ray.pos + (ray.dir * t);
        return true;
    }

}
