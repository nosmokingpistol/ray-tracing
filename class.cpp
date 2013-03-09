
#include "class.h"
#include "common.h"

#include <Eigen/Dense>
#include <Eigen/Geometry> 

#define PI 3.14159265


using namespace Eigen;

Vector3f g_ambience, g_diffuse, g_specular;
float spec_power;

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
    float scalarY = 2.0*tan(fov_y/2);

    // Have to calculate fov_x;
    float fov_x = atan(tan(fov_y/2)*width/height)*2;
    float scalarX = 2.0*tan(fov_x/2);

    Vector3f Xinc = scalarX/width*U;
    Vector3f Yinc = scalarY/height*V;
    Vector3f y = (sample.y+0.5-(height/2))*Yinc;
    Vector3f x = (sample.x+0.5-(width/2))*Xinc;
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
    std::vector<Light*>::iterator l_itr;
    Vector3f normal;
    Vector3f intersect;
    float thit;
    for(itr = primitives.begin(); itr != primitives.end(); ++itr) {
            // std::cout<< " ****looping" << std::endl;
        Primitive& cur_prim = **itr;
        cur_prim.transform.transform_ray(ray);
            // cur_prim.print();  
        if (cur_prim.intersect(ray, &thit, intersect, normal)) {
	    Vector3f final_color;
	    for (l_itr = lights.begin(); l_itr != lights.end(); ++l_itr) {
	        Light& cur_light = **l_itr;
	        final_color += cur_light.calc_amb(::g_ambience, cur_light.intensities);

	        final_color += cur_light.calc_diff(g_diffuse, cur_light.intensities, normal);

	        /*final_color += cur_light.calc_spec(g_specular, cur_light.intensities, normal, cur_light.l_vec, spec_power);*/
	        
	    }
	    *color = final_color;
            /*std::cout << " setting color to red!!" << std::endl;
            *color = Vector3f(100, 0, 0);*/
                // std::cout << "intersection: t = " << thit << " intersect point = ";
                // intersect.print();
                // std::cout << "normal = ";
                // std::cout << std::endl;
        }
        else {// no primitive intersection
            *color = Vector3f(0, 0, 0);
                                      // cout << " BLACK!!" << endl;
        }
    }
}



void Scene::render() {
    Sampler sampler = Sampler(width, height);
    Sample sample;
    Film film = Film("test1.png", width, height);
    RGBQUAD image_color;
    Vector3f color;
    Ray ray;
    while (sampler.generateSample(&sample)) {
         // cout << "sample = " << sample.x << " " << sample.y << endl;
        camera.generateRay(sample, &ray);
        raytracer.trace(ray, 0, &color);
        film.commit(sample, color);
    }
    film.writeImage(); 
}

bool Sphere::intersect(Ray& ray, float* thit, Vector3f& intersect, Vector3f& normal){
    // A = dir * dir
    float A = ray.dir.dot(ray.dir);
    // B = 2 dir * (pos - center)
    float B = 2.0 * ray.dir.dot(ray.pos - center);
    // C =  (pos - center) * (pos - center) - r^2
    float C = (ray.pos - center).dot(ray.pos - center) - radius*radius;
    std::cout << " A = " << A << " B = " << B << " C = " << C << std::endl;
    if (discriminant(A, B, C) < 0) { // no intersection
        // std::cout <<  " No intersection " << std::endl << std::endl;
        return false;
    }
    // discriminant >=0,  ray is tangent or intersects in 2 pts
    float t = solve_quadratic(A, B, C);
    // Ri = [xi, yi, zi] = [x0 + xd * ti ,  y0 + yd * ti,  z0 + zd * ti]
    intersect = ray.pos + ray.dir*t;
    //Unit N at surface SN = [(xi - xc)/Sr,   (yi - yc)/Sr,   (zi - zc)/Sr]
    normal = (intersect - center)/radius;
    // std::cout << " intersect point = "<< intersect << " normal = "<< normal<< std::endl;
    return true;
}; 

// http://www.cs.washington.edu/education/courses/cse457/07sp/lectures/triangle_intersection.pdf

bool Triangle::intersect(Ray&ray, float* thit, Vector3f& intersect, Vector3f& normal) {
    // plane: (P0 + P1t ) * n = A*n    
    // t = (A*n - P0*N)/P1*N
    float t;
    float d = N.dot(A);
    if (N.dot(ray.dir) == 0) {
        return false;
    } else {
        t = (d - N.dot(ray.pos))/(N.dot(ray.dir));
    }
    // std::cout << "t calculated as: " << t << std::endl;
    intersect = ray.pos + (t*ray.dir);
    
    if (((B-A).cross(intersect-A)).dot(N) < 0
        || ((C-B).cross(intersect-B)).dot(N) < 0
        || ((A-C).cross(intersect-C)).dot(N) < 0) {
        // std::cout <<  " No intersection " << std::endl;
    return false;
} else {
    float alpha = ((C-B).cross(intersect-B)).dot(N)/((B-A).cross(C-A)).dot(N);
    float beta = ((A-C).cross(intersect-C)).dot(N)/((B-A).cross(C-A)).dot(N);
    float gamma = ((B-A).cross(intersect-A)).dot(N)/((B-A).cross(C-A)).dot(N);

    normal = N;
    intersect = ray.pos + (ray.dir * t);
    return true;
}
return false;
}

