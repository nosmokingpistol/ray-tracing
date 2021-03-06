
#include "class.h"
#include "common.h"
#include "float.h"

#include <Eigen/Dense>
#include <Eigen/Geometry> 

#define PI 3.14159265


using namespace Eigen;

Vector3f g_ambience;

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

    scalarY = 2.0*tan(fov_y/2);
    fov_x = atan(tan(fov_y/2)*width/height)*2;
    scalarX = 2.0*tan(fov_x/2);

    Xinc = scalarX/width*U;
    Yinc = scalarY/height*V;

    // std::cout << " Camera eye = " << eye_pos  << " V = "<< V << " U = " << U<< " N = " << N << std::endl;
    // std::cout << " screen_width = " << screen_width << " screen_height = " << screen_height;
    // std::cout << " image_height = " << image_height << " image_width = " << image_width;
    // std::cout << " pixel_width = " << pixel_width << " pixel_height = " << pixel_height << std::endl;
}

void Camera::generateRay(Sample &sample, Ray* ray){
    // R(t) = E + t(P-E)
    ray->pos = eye_pos;
    // x = tan(fovx/2)((x - width/2 )) / width/2
    // float scalarY = 2.0*tan(fov_y/2);

    // Have to calculate fov_x;
    // float fov_x = atan(tan(fov_y/2)*width/height)*2;
    // float scalarX = 2.0*tan(fov_x/2);

    // Vector3f Xinc = scalarX/width*U;
    // Vector3f Yinc = scalarY/height*V;
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
    if (depth >= maxdepth) {
        return;
    }
    // std::cout << " depth = " << depth << std::endl;
    // for each object, determine closet object of intersection
    std::vector<Primitive*>::iterator itr;
    std::vector<Light*>::iterator l_itr;
    Vector3f normal;
    Vector3f intersect;
    float thit;
    Primitive closest_primitive; // only compute lighting for closest object
    Vector3f closest_normal;
    Vector3f closest_intersect;
    float closest_distance = FLT_MAX;

    Vector3f viewer_direction;
    viewer_direction = ray.dir * -1;
    viewer_direction.normalize();
    // loop through primitives and find closest intersection
    for(itr = primitives.begin(); itr != primitives.end(); ++itr) {
        Primitive& cur_prim = **itr;
        // cur_prim.transform.transform_ray(ray);
            // cur_prim.print();  
        // primitive's intersect will transform the ray to do calculation, and return transformed back intersect point and normal
        if (cur_prim.intersect(ray, &thit, intersect, normal)) {
            // update closest primitive
            float distance = manhattan_distance(eye_pos, intersect);
            if (distance < closest_distance) {
                closest_primitive = cur_prim;
                closest_distance = distance;
                closest_normal = Vector3f(normal(0), normal(1), normal(2));
                closest_intersect = intersect;
            }
        }
    }
    if (closest_distance != FLT_MAX) {
        // Compute ambient lighting
        Vector3f final_color = g_ambience;
        // add object's emission
        final_color = final_color + closest_primitive.emission;
	    for (l_itr = lights.begin(); l_itr != lights.end(); ++l_itr) {
            Light& cur_light = **l_itr;
            if (!cur_light.is_directional()) { // point lights cast shadows
        	    // Check for shadows.
                Vector3f shadow_vector = cur_light.coordinates-closest_intersect;
                shadow_vector.normalize();
        	Ray shadow_ray = Ray(closest_intersect, shadow_vector, 0.0f, FLT_MAX);
        	bool is_in_shadow = false;	    
        	    // If the shadow ray hits anything on the way back to the light, don't do any other light shading.
        	    for (itr = primitives.begin(); itr < primitives.end(); ++itr) {
                        Primitive& p = **itr;
        	        is_in_shadow = p.intersect(shadow_ray, &thit, intersect, normal);
        	    }
        	    // Otherwise, continue to add the diffuse and specular components.
        	    if (!is_in_shadow) {
                    final_color += cur_light.calc_diff(closest_primitive.diffuse, cur_light.intensities, closest_normal, closest_intersect);
                    final_color += cur_light.calc_spec(closest_primitive.specular, cur_light.intensities, closest_normal, viewer_direction, cur_light.l_vec, closest_primitive.shiny);
        	    }
            }
            else { // ignore shadow for directional lights
                final_color += cur_light.calc_diff(closest_primitive.diffuse, cur_light.intensities, closest_normal, closest_intersect);
                final_color += cur_light.calc_spec(closest_primitive.specular, cur_light.intensities, closest_normal, viewer_direction, cur_light.l_vec, closest_primitive.shiny);
            }
        }
        if ( depth == 0) { // not a reflective ray
            *color = final_color * 255;
        }
        else { // for reflective rays, modulate by specularity
            Vector3f scaled_color(255 * pow(closest_primitive.specular(0), depth) * final_color(0),
                            255 * pow(closest_primitive.specular(1), depth) * final_color(1),
                            255 * pow(closest_primitive.specular(2), depth) * final_color(2));
            *color = *color + scaled_color;
            // std::cout << " adding specular  component of " << std::endl << scaled_color << std::endl;

        }
        // std::cout << " color = " << std::endl << *color << std::endl;
         if ((*color)(0) >255) (*color)(0) = 255;
         if ((*color)(1) >255) (*color)(1) = 255;
         if ((*color)(2) >255) (*color)(2) = 255;
        // if hit object has specularity, do reflection
        if (closest_primitive.specular != Vector3f(0, 0, 0)) { // if specular, compute reflection
            Vector3f rd = ray.dir;
            rd.normalize();
            Vector3f reflection_dir = -rd + 2 * (rd.dot(normal)) * normal;
            reflection_dir.normalize();
            Ray reflection_ray = Ray(closest_intersect, reflection_dir, 0.0f, FLT_MAX);
            // std::cout << "  reflection_ray ray = " << std::endl; reflection_ray.print();
            trace(reflection_ray, depth + 1, color);
        }    
    }
    else { // no intersection, add global ambience
        // std::cout <<  " no intersection, depth = " << depth << std::endl;
         *color = *color + g_ambience;
    }          
};



void Scene::render() {
    Sampler sampler = Sampler(width, height);
    Sample sample;
    Film film = Film(fname, width, height);
    RGBQUAD image_color;
    Vector3f color;
    Ray ray;
    while (sampler.generateSample(&sample)) {
         // cout << "sample = " << sample.x << " " << sample.y << endl;
        camera.generateRay(sample, &ray);
        color = Vector3f(0, 0, 0); // initialize color to black before recursive ray tracing
        raytracer.trace(ray, 0, &color);
        film.commit(sample, color);
    }
    film.writeImage(); 
}

bool Sphere::intersect(Ray& ray, float* thit, Vector3f& intersect, Vector3f& normal){
    // do transformation
    Ray transformed_ray = transform.transform_ray(ray);

    // std::cout << " ray = " << std::endl;
    // ray.print();
    // std::cout <<" transformed_ray = " << std::endl;
    // transformed_ray.print();

    // A = dir * dir
    float A = transformed_ray.dir.dot(transformed_ray.dir);
    // B = 2 dir * (pos - center)
    float B = 2.0 * transformed_ray.dir.dot(transformed_ray.pos - center);
    // C =  (pos - center) * (pos - center) - r^2
    float C = (transformed_ray.pos - center).dot(transformed_ray.pos - center) - radius*radius;
    // std::cout << " A = " << A << " B = " << B << " C = " << C << std::endl;
    if (discriminant(A, B, C) < 0) { // no intersection
        return false;
    }
    // discriminant >=0,  ray is tangent or intersects in 2 pts
    float t = solve_quadratic(A, B, C);
    // Ri = [xi, yi, zi] = [x0 + xd * ti ,  y0 + yd * ti,  z0 + zd * ti]

    intersect = transformed_ray.pos + transformed_ray.dir*t;
    //Unit N at surface SN = [(xi - xc)/Sr,   (yi - yc)/Sr,   (zi - zc)/Sr]
    normal = (intersect - center)/radius;
    normal.normalize();
    // std::cout << " intersect point = "<< intersect << " normal = "<< normal<< std::endl;
    //transform back to world coordinates
    transform.transform_intersection(intersect);

    // intersect = ray.pos + ray.dir*t;
    transform.transform_normal(normal);



    return true;
}; 

// http://www.cs.washington.edu/education/courses/cse457/07sp/lectures/triangle_intersection.pdf

bool Triangle::intersect(Ray&ray, float* thit, Vector3f& intersect, Vector3f& normal) {
     // do transformation
    Ray transformed_ray = transform.transform_ray(ray);
    // std::cout << " ray = " << std::endl;
    // ray.print();
    // std::cout <<" transformed_ray = " << std::endl;
    // transformed_ray.print();

    // plane: (P0 + P1t ) * n = A*n    
    // t = (A*n - P0*N)/P1*N
    float t;
    float d = N.dot(A);
    if (N.dot(transformed_ray.dir) == 0) {
        return false;
    } else {
        t = (d - N.dot(transformed_ray.pos))/(N.dot(transformed_ray.dir));
    }
    // std::cout << "t calculated as: " << t << std::endl;
    intersect = transformed_ray.pos + (t*transformed_ray.dir);
    
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

        //transform back to world coordinates
        transform.transform_intersection(intersect);
        transform.transform_normal(normal);

        return true;
    }
return false;
}

