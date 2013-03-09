#ifndef CLASS_H
#define CLASS_H

#include <iostream>
#include <vector>
#include <Eigen/Dense>

#include "common.h"
#include "transform.h"
#include "light.h"
#include "FreeImage.h"

using namespace Eigen;

extern Vector3f g_ambience;

class Primitive // this is a base class, necessary?
{
    public:
        Transformation transform;
        Vector3f emission;
        Vector3f specular;
        Vector3f diffuse;
        float shiny;
        // Test if ray intersects with the shape or not (in object space), if so,
        // return intersection point and normal
        virtual bool intersect(Ray& ray, float* thit, Vector3f& intersect, Vector3f& normal){};
        virtual void print() {};

        void set_transform(Transformation t) {
            transform = Transformation(t);
            transform.initialize();
            // t.print();
        };
        void set_emission(Vector3f e) {emission = e;};
        void set_specular(Vector3f s) {specular = s;};
        void set_diffuse(Vector3f d) {diffuse = d;};
        void set_shiny(float s) {shiny = s;};

        // Triangle and Sphere are probably best implemented here
        // The intersection with the ray at t outside the range [t_min, t_max]
        // should return false.
};

class Sphere: public Primitive
{
    public:
        Vector3f center;
        float radius;
        Sphere(float x, float y, float z, float r) {
            center << x, y, z;
            radius = r;
        };
        virtual bool intersect(Ray& ray, float* thit, Vector3f& intersect, Vector3f& normal);

        virtual void print() {
            std::cout<< "Sphere center = " << std::endl << center << " radius = " << std::endl << radius << std::endl;
        };
};

class Triangle : public Primitive
{
    public:
        Vector3f A, B, C;
        Vector3f N;
        Triangle(Vector3f a, Vector3f b, Vector3f c) {
            A = a;
            B = b;
            C = c;
            N = (B-A).cross((C-A));
            N.normalize();
        };
        virtual bool intersect(Ray& ray, float* thit, Vector3f& intersect, Vector3f& normal);             

};

class Sample
{
    public:
	   float x, y; 
        Sample(float x_pos, float y_pos): x(x_pos), y(y_pos) {}; // store screen coordinate
        Sample(){};
};

class Sampler
{
    public:
		// generate samples in order of 
		// for 0..width
			// for 0..height
		float cur_w; // current sample's width
		float cur_h; // current sample's height 
	    float width, height; // dimensions of image
        Sampler() {};
        Sampler(float w, float h): width(w), height(h), cur_w(0), cur_h(0) {};
         //It will generate (x,y) of a screen sample and return true.
        //Next time it gets called, it will generate another sample for the next pixel.
        //It will return false when all the samples from all the pixels are generated. 
        //(In our case, we generate 1 sample per pixel, at the pixel sample. Later on, if we want to do multi-sample per pixel, we need to modify this class.

        bool generateSample(Sample* sample);
};

class Camera
{
    public:
        Vector3f eye_pos; //lookfrom is the location of the eye (where your rays originate from)
        Vector3f lookat; // lookat is the location in space that you are looking at
        Vector3f up;// up is the up vector
        float fov_y; // fov angle specifies the vertical field of view
        float height; 
        float width; 

        Vector3f N; // viewing direction

        Vector3f W;
        Vector3f U; // corresponds to "X" axis
        Vector3f V; // up vector


        float scalarY, fov_x, scalarX;
        Vector3f Xinc, Yinc;


        Camera() {};
        Camera(Vector3f lookfrom, Vector3f lat, Vector3f u, float fov, float screen_width, float screen_height);
        // Camera(Point eye, Point UL, Point UR, Point LR, Point LL, float screen_width, float screen_height);
        void generateRay(Sample& sample, Ray* ray);
    //    Create a ray starting from the camera that passes through the
    //    corresponding pixel (sample.x, sample.y) on the image plane.
};

class Film // integrate with freeimage
{
    public:
        float width, height;
        const char* filename;
        FIBITMAP* bitmap; // = FreeImage_Allocate(WIDTH, HEIGHT, BPP);
        RGBQUAD image_color;
        Film() {};
        Film(const char*  filename, float width, float height);
        // Will write the color to (sample.x, sample.y) on the image
        void commit(Sample& sample, Vector3f& color);
        // Output image to a file
        void writeImage();
    //    Can be implemented just by a 2D array of Color (Later on, we can
    //    implement more complicated things such as multi-sample per pixel, or
    //    post processing, eg. tone mapping in this class)
    
};

class RayTracer
{
    public:
        std::vector<Primitive*> primitives;
	    std::vector<Light*> lights;
        Vector3f eye_pos;

        int depth_threshold;
        RayTracer() {}; // initialize vector
        void add_primitive(Primitive *prim) {
            primitives.push_back(prim);
            // std::cout<< "added primitve" << std::endl;
            // prim->print();
        }
        void add_light(Light *light) {
            lights.push_back(light);
            // std::cout<< "added light" << std::endl;
            // light->print();
        }
        void trace(Ray& ray, int depth, Vector3f* color);
};



class Scene
{
    public:
        float width, height;
        float maxdepth;
        Camera camera;
        RayTracer raytracer;
        Scene() {
            raytracer = RayTracer();
        };
        void loadScene(std::string file);
        void render();
};

#endif

