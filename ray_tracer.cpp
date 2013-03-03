#include <iostream>
#include <stdlib.h>
#include "FreeImage.h"
#include "class.h"
#define WIDTH 800
#define HEIGHT 600
#define BPP 24 // Since we ' re o u t p u tti n g t h r e e 8 b i t RGB v a l u e s

using namespace std ;




int main ( int argc , char * argv [] ) {
	float width = 20;
	float height = 10;
	Sampler sampler = Sampler(width, height);
	Sample sample;
	Film film = Film("bsod.png", width, height);


	Vector3f lfrm(0,0,0);
	Vector3f lat(0, 0, -1);
	Vector3f u(0, 1, 0);
	float fov = 90;

	Camera camera = Camera(lfrm, lat, u, fov, width, height);

	Sphere*  sphere = new Sphere(1, 0, -2, 1.0);
	RayTracer raytracer = RayTracer();
	raytracer.add_primitive(sphere);


	RGBQUAD image_color;
	Vector3f color;
	Ray ray;
    while (sampler.generateSample(&sample)) {
            camera.generateRay(sample, &ray);
            raytracer.trace(ray, 0, &color);
    		cout << "sample = " << sample.x << " " << sample.y << endl;
            film.commit(sample, color);
	}
	film.writeImage(); 

	return 0;
}