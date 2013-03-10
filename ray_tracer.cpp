#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <vector>
#include "FreeImage.h"
#include "class.h"

#define WIDTH 800
#define HEIGHT 600
#define BPP 24 // Since we ' re o u t p u tti n g t h r e e 8 b i t RGB v a l u e s

using namespace std ;

void Scene::loadScene(string file) {

  //store variables and set stuff at the end
  std::string fname = "output.bmp";
  vector<Vector3f> vertices;
  vector<Vector3f> emission_stack;
  vector<Vector3f> specular_stack;
  vector<Vector3f> diffuse_stack;
  vector<float> shiny_stack;


  vector<Transformation> transform_stack;
  ifstream inpfile(file.c_str());
  if(!inpfile.is_open()) {
    cout << "Unable to open file" << endl;
  	} 
  else {
    string line;

    while(inpfile.good()) {
      vector<string> splitline;
      string buf;

      getline(inpfile,line);
      stringstream ss(line);

      while (ss >> buf) {
        splitline.push_back(buf);
      }
      //Ignore blank lines
      if(splitline.size() == 0) {
        continue;
      }

      //Ignore comments
      if(splitline[0][0] == '#') {
        continue;
      }

      //Valid commands:
      //size width height
      //  must be first command of file, controls image size
      else if(!splitline[0].compare("size")) {
        width = atoi(splitline[1].c_str());
        height = atoi(splitline[2].c_str());
      }
      //maxdepth depth
      //  max # of bounces for ray (default 5)
      else if(!splitline[0].compare("maxdepth")) {
       raytracer.set_max_depth(atoi(splitline[1].c_str()));
      }
      //output filename
      //  output file to write image to 
      else if(!splitline[0].compare("output")) {
        fname = splitline[1];
      }

      //camera lookfromx lookfromy lookfromz lookatx lookaty lookatz upx upy upz fov
      //  speciï¬es the camera in the standard way, as in homework 2.
      else if(!splitline[0].compare("camera")) {
        // lookfrom:
        Vector3f lookfrom(atof(splitline[1].c_str()),
	           			atof(splitline[2].c_str()),
	           			atof(splitline[3].c_str()));
        // lookat:
        Vector3f lookat(atof(splitline[4].c_str()),
				       atof(splitline[5].c_str()),
				       atof(splitline[6].c_str()));
        // up:
        Vector3f up(atof(splitline[7].c_str()),
           atof(splitline[8].c_str()),
           atof(splitline[9].c_str()));
        float fov = atof(splitline[10].c_str());
        camera = Camera(lookfrom, lookat, up, fov, width, height);
        raytracer.eye_pos = lookfrom;
    		Transformation identity = Transformation(); // empty transform
    		transform_stack.push_back(identity);       

        emission_stack.push_back(Vector3f(0, 0, 0)); // default 0 emission
        specular_stack.push_back(Vector3f(0, 0, 0)); // default 0 specular
        diffuse_stack.push_back(Vector3f(0, 0, 0)); // default 0 emission
        shiny_stack.push_back(0.0); // default 0 shininess;
        g_ambience = Vector3f(0,0,0);
      }

      //sphere x y z radius
      //  Deï¬nes a sphere with a given position and radius.
      else if(!splitline[0].compare("sphere")) {
      	Sphere *sphere = new Sphere(atof(splitline[1].c_str()),
			        atof(splitline[2].c_str()),
			        atof(splitline[3].c_str()),
				      atof(splitline[4].c_str()));
      	sphere->set_transform(transform_stack.back());
        
        sphere->set_emission(emission_stack.back());
        sphere->set_specular(specular_stack.back());
        sphere->set_diffuse(diffuse_stack.back());
        sphere->set_shiny(shiny_stack.back());
    		raytracer.add_primitive(sphere);


        // cout<< " sphere emission = " <<endl << emission_stack.back() << endl;
        // cout<< " sphere specular = " <<endl << specular_stack.back() << endl;
        // cout<< " sphere diffuse = " <<endl << diffuse_stack.back() << endl;
        // cout<< " sphere shiny = " <<endl << shiny_stack.back() << endl;

      }
      //maxverts number
      //  Deï¬nes a maximum number of vertices for later triangle speciï¬cations. 
      //  It must be set before vertices are deï¬ned.
      else if(!splitline[0].compare("maxverts")) {
        // Care if you want
        // Here, either declare array size
        // Or you can just use a STL vector, in which case you can ignore this
      }
      //maxvertnorms number
      //  Deï¬nes a maximum number of vertices with normals for later speciï¬cations.
      //  It must be set before vertices with normals are deï¬ned.
      else if(!splitline[0].compare("maxvertnorms")) {
        // Care if you want
      }
      //vertex x y z
      //  Deï¬nes a vertex at the given location.
      //  The vertex is put into a pile, starting to be numbered at 0.
      else if(!splitline[0].compare("vertex")) {
      	Vector3f vertex(atof(splitline[1].c_str()),
        	atof(splitline[2].c_str()),
        	atof(splitline[3].c_str()));
      		vertices.push_back(vertex);
        // Create a new vertex with these 3 values, store in some array
      }
      //vertexnormal x y z nx ny nz
      //  Similar to the above, but deï¬ne a surface normal with each vertex.
      //  The vertex and vertexnormal set of vertices are completely independent
      //  (as are maxverts and maxvertnorms).
      else if(!splitline[0].compare("vertexnormal")) {
        // x: atof(splitline[1].c_str()),
        // y: atof(splitline[2].c_str()),
        // z: atof(splitline[3].c_str()));
        // nx: atof(splitline[4].c_str()),
        // ny: atof(splitline[5].c_str()),
        // nz: atof(splitline[6].c_str()));
        // Create a new vertex+normal with these 6 values, store in some array
      }
      //tri v1 v2 v3
      //  Create a triangle out of the vertices involved (which have previously been speciï¬ed with
      //  the vertex command). The vertices are assumed to be speciï¬ed in counter-clockwise order. Your code
      //  should internally compute a face normal for this triangle.
      else if(!splitline[0].compare("tri")) {
      	Vector3f A = vertices[atof(splitline[1].c_str())];
        Vector3f B = vertices[atof(splitline[2].c_str())];
        Vector3f C = vertices[atof(splitline[3].c_str())];
        Triangle *tri = new Triangle(A, B, C);
        tri->set_transform(transform_stack.back());
        tri->set_emission(emission_stack.back());
        tri->set_specular(specular_stack.back());
        tri->set_diffuse(diffuse_stack.back());
        tri->set_shiny(shiny_stack.back());
        raytracer.add_primitive(tri);

        // cout<< " triangle emission = " <<endl << emission_stack.back() << endl;
        // cout<< " triangle specular = " <<endl << specular_stack.back() << endl;
        // cout<< " triangle diffuse = " <<endl << diffuse_stack.back() << endl;
        // cout<< " triangle shiny = " <<endl << shiny_stack.back() << endl;

      }
      //trinormal v1 v2 v3
      //  Same as above but for vertices speciï¬ed with normals.
      //  In this case, each vertex has an associated normal, 
      //  and when doing shading, you should interpolate the normals 
      //  for intermediate points on the triangle.
      else if(!splitline[0].compare("trinormal")) {
        // v1: atof(splitline[1].c_str())
        // v2: atof(splitline[2].c_str())
        // v3: atof(splitline[3].c_str())
        // Create new triangle:
        //   Store pointer to array of vertices (Different array than above)
        //   Store 3 integers to index into array
        //   Store current property values
        //   Store current top of matrix stack
      }

      //translate x y z
      //  A translation 3-vector
      else if(!splitline[0].compare("translate")) {
      	Matrix4f translation = translation_matrix(
        	atof(splitline[1].c_str()),
        	atof(splitline[2].c_str()),
        	atof(splitline[3].c_str()));
      	transform_stack.back().add_transformation(TransformMatrix(translation, 0));
      	// cout<< endl << endl << " added translation, now transformation = " << endl;
      }
      //rotate x y z angle
      //  Rotate by angle (in degrees) about the given axis as in OpenGL.
      else if(!splitline[0].compare("rotate")) {
      	Matrix4f rotation = rotation_matrix(
        	atof(splitline[1].c_str()),
        	atof(splitline[2].c_str()),
        	atof(splitline[3].c_str()),
        	atof(splitline[4].c_str()));
      	transform_stack.back().add_transformation(TransformMatrix(rotation, 2));
      	// cout<< endl << endl << " added rotation, now transformation = " << endl;
      }
      //scale x y z
      //  Scale by the corresponding amount in each axis (a non-uniform scaling).
      else if(!splitline[0].compare("scale")) {
      	Matrix4f scale = scale_matrix(
        	atof(splitline[1].c_str()),
        	atof(splitline[2].c_str()),
        	atof(splitline[3].c_str()));
      	transform_stack.back().add_transformation(TransformMatrix(scale, 1));
      	 // cout<< endl << endl << " added scale, now transformation = " << endl;
      }
      //pushTransform
      //  Push the current modeling transform on the stack as in OpenGL. 
      //  You might want to do pushTransform immediately after setting 
      //   the camera to preserve the â€œidentityâ€ transformation.
      else if(!splitline[0].compare("pushTransform")) {
      	transform_stack.push_back(Transformation());
      }
      //popTransform
      //  Pop the current transform from the stack as in OpenGL. 
      //  The sequence of popTransform and pushTransform can be used if 
      //  desired before every primitive to reset the transformation 
      //  (assuming the initial camera transformation is on the stack as 
      //  discussed above).
      else if(!splitline[0].compare("popTransform")) {
      	transform_stack.pop_back();
      }
      //directional x y z r g b
      //  The direction to the light source, and the color, as in OpenGL.
      else if(!splitline[0].compare("directional")) {
          Light *light = new Directional_Light(
            atof(splitline[1].c_str()),
            atof(splitline[2].c_str()),
            atof(splitline[3].c_str()),
            atof(splitline[4].c_str()),
            atof(splitline[5].c_str()),
            atof(splitline[6].c_str()));
          raytracer.add_light(light);
      }
      //point x y z r g b
      //  The location of a point source and the color, as in OpenGL.
      else if(!splitline[0].compare("point")) {
          Light *light = new Point_Light(
            atof(splitline[1].c_str()),
            atof(splitline[2].c_str()),
            atof(splitline[3].c_str()),
            atof(splitline[4].c_str()),
            atof(splitline[5].c_str()),
            atof(splitline[6].c_str()));
          raytracer.add_light(light);
      }
      //attenuation const linear quadratic
      //  Sets the constant, linear and quadratic attenuations 
      //  (default 1,0,0) as in OpenGL.
      else if(!splitline[0].compare("attenuation")) {
        // const: atof(splitline[1].c_str())
        // linear: atof(splitline[2].c_str())
        // quadratic: atof(splitline[3].c_str())
      }
      //ambient r g b
      //  The global ambient color to be added for each object 
      //  (default is .2,.2,.2)
      else if(!splitline[0].compare("ambient")) {
        float r_amb = atof(splitline[1].c_str());
        float g_amb = atof(splitline[2].c_str());
        float b_amb = atof(splitline[3].c_str());
	      g_ambience << r_amb, g_amb, b_amb;
        cout << " adding global ambience = " << endl << g_ambience << endl;
      }

      //diï¬€use r g b
      //  speciï¬es the diï¬€use color of the surface.
      else if(!splitline[0].compare("diffuse")) {
        Vector3f diffuse = Vector3f(atof(splitline[1].c_str()),
                                    atof(splitline[2].c_str()),
                                    atof(splitline[3].c_str()));
        diffuse_stack.push_back(diffuse);
      }
      //specular r g b 
      //  speciï¬es the specular color of the surface.
      else if(!splitline[0].compare("specular")) {
        Vector3f specular = Vector3f(atof(splitline[1].c_str()),
                                    atof(splitline[2].c_str()),
                                    atof(splitline[3].c_str()));
        specular_stack.push_back(specular);
      }
      //shininess s
      //  speciï¬es the shininess of the surface.
      else if(!splitline[0].compare("shininess")) {
        shiny_stack.push_back(atof(splitline[1].c_str()));
      }
      //emission r g b
      //  gives the emissive color of the surface.
      else if(!splitline[0].compare("emission")) {
        Vector3f emission(atof(splitline[1].c_str()),
           atof(splitline[2].c_str()),
           atof(splitline[3].c_str()));
        emission_stack.push_back(emission);
      } else {
        cerr << "Unknown command: " << splitline[0] << endl;
      }
    }

    inpfile.close();
  }

}


int main ( int argc , char * argv [] ) {
	Scene scene = Scene();
	cout << " file to load = " << argv[1] << endl;
	scene.loadScene(argv[1]);
	scene.render();

}
