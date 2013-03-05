#include "FreeImage.h"
#include <iostream>
#include <cmath>
#include <vector>

#include <Eigen/Dense>
using namespace Eigen;

float discriminant(float A, float B, float C);
float solve_quadratic(float A, float B, float C);

Vector3f convert_to_3d(Vector4f v);
Vector4f convert_to_4d(Vector3f v);

Matrix4f identity_matrix(); 

Matrix4f translation_matrix(float x, float y, float z);

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
            // std::cout << "!!!!!!!!!! initializing transformation!!" << std::endl;
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
            // std::cout << " transform ray origin matrix = " << std::endl;
            // std::cout << transform_ray_origin << std::endl;
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
            // std::cout << " transform_ray_dir matrix = " << std::endl;
            // std::cout << transform_ray_dir << std::endl;
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

            // std::cout << "before, ray dir = " << ray.dir << std::endl;

            Vector4f dir = convert_to_4d(ray.dir);
            Vector4f new_dir = transform_ray_dir*dir;
            ray.dir = convert_to_3d(new_dir);

            // std::cout << "after, ray dir = " << ray.dir << std::endl;
            
        };


};

class Primitive // this is a base class, necessary?
{
    public:
        Transformation transform;
	// Test if ray intersects with the shape or not (in object space), if so,
	// return intersection point and normal
    virtual bool intersect(Ray& ray, float* thit, Vector3f& intersect, Vector3f& normal){};
    // virtual bool intersect(Ray& ray){}; 

    virtual void print() {};

    void set_transform(Transformation t) {
        transform = Transformation(t);
        transform.initialize();
        // t.print();
    };
  
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
            std::cout<< "Sphere center = " << center << " radius = " << radius << std::endl;
        };
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
        float image_height;
        float image_width;
                
        Vector3f N; // viewing direction

        Vector3f W;
        Vector3f U; // corresponds to "X" axis
        Vector3f V; // up vector
        float pixel_width; // direction increment vectors for pixels not in center of screen
        float pixel_height;

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
        int depth_threshold;
        RayTracer() {}; // initialize vector
        void add_primitive(Primitive *prim) {
            primitives.push_back(prim);
            std::cout<< "added primitve" << std::endl;
            prim->print();
        }
        void trace(Ray& ray, int depth, Vector3f* color);
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
 

