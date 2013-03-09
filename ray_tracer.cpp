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
  vector<Transformation> transform_stack;
  ifstream inpfile(file.c_str());
  if(!inpfile.is_open()) {
    cout << "Unable to open file" << endl;
  	} 
  else {
    string line;
    //MatrixStack mst;

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
        // maxdepth: atoi(splitline[1].c_str())
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
		Transformation identity = Transformation(); // empty transform
		transform_stack.push_back(identity);       
        // push identity transform to stack?
        //   cout<< "pushing identity matrix" << endl << identity << endl;
      }

      //sphere x y z radius
      //  Deï¬nes a sphere with a given position and radius.
      else if(!splitline[0].compare("sphere")) {
      	cout << "adding sphere" << endl;
      	Sphere *sphere = new Sphere(atof(splitline[1].c_str()),
			        atof(splitline[2].c_str()),
			        atof(splitline[3].c_str()),
					atof(splitline[4].c_str()));
      	sphere->set_transform(transform_stack.back());
		raytracer.add_primitive(sphere);
		cout << "finished adding sphere" << endl << endl << endl;

        // Create new sphere:
        //   Store 4 numbers
        //   Store current property values
        //   Store current top of matrix stack
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
      	// Transformation cur = Transformation(transform_stack);
        tri->set_transform(transform_stack.back());
        raytracer.add_primitive(tri);
        // Create new triangle:
        //   Store pointer to array of vertices
        //   Store 3 integers to index into array
        //   Store current property values
        //   Store current top of matrix stack
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
      	cout<< endl << endl << " added translation, now transformation = " << endl;
      	transform_stack.back().print();
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
      	cout<< endl << endl << " added rotation, now transformation = " << endl;
      	transform_stack.back().print();
        // Update top of matrix stack
      }
      //scale x y z
      //  Scale by the corresponding amount in each axis (a non-uniform scaling).
      else if(!splitline[0].compare("scale")) {
      	Matrix4f scale = scale_matrix(
        	atof(splitline[1].c_str()),
        	atof(splitline[2].c_str()),
        	atof(splitline[3].c_str()));
      	    transform_stack.back().add_transformation(TransformMatrix(scale, 1));
      	    cout<< endl << endl << " added scale, now transformation = " << endl;
      		transform_stack.back().print();
      }
      //pushTransform
      //  Push the current modeling transform on the stack as in OpenGL. 
      //  You might want to do pushTransform immediately after setting 
      //   the camera to preserve the â€œidentityâ€ transformation.
      else if(!splitline[0].compare("pushTransform")) {
      	transform_stack.push_back(Transformation());
        //mst.push();
      }
      //popTransform
      //  Pop the current transform from the stack as in OpenGL. 
      //  The sequence of popTransform and pushTransform can be used if 
      //  desired before every primitive to reset the transformation 
      //  (assuming the initial camera transformation is on the stack as 
      //  discussed above).
      else if(!splitline[0].compare("popTransform")) {
      	transform_stack.pop_back();
        //mst.pop();
      }

      //directional x y z r g b
      //  The direction to the light source, and the color, as in OpenGL.
      else if(!splitline[0].compare("directional")) {
        float x_directional = atof(splitline[1].c_str());
        float y_directional = atof(splitline[2].c_str());
        float z_directional = atof(splitline[3].c_str());
        float r_directional = atof(splitline[4].c_str());
        float g_directional = atof(splitline[5].c_str());
        float b_directional = atof(splitline[6].c_str());
        // add light to scene...
	Directional_Light *d = new Directional_Light(x_directional, y_directional, z_directional, r_directional, g_directional, b_directional);
	raytracer.add_light(d);
      }
      //point x y z r g b
      //  The location of a point source and the color, as in OpenGL.
      else if(!splitline[0].compare("point")) {
        float x_point = atof(splitline[1].c_str());
        float y_point = atof(splitline[2].c_str());
        float z_point = atof(splitline[3].c_str());
        float r_point = atof(splitline[4].c_str());
        float g_point = atof(splitline[5].c_str());
        float b_point = atof(splitline[6].c_str());
        // add light to scene...
	Point_Light *p = new Point_Light(x_point, y_point, z_point, r_point, g_point, b_point);
	raytracer.add_light(p);
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
      }

      //diï¬€use r g b
      //  speciï¬es the diï¬€use color of the surface.
      else if(!splitline[0].compare("diffuse")) {
        float r_diff = atof(splitline[1].c_str());
        float g_diff = atof(splitline[2].c_str());
        float b_diff = atof(splitline[3].c_str());
        // Update current properties
	g_diffuse << r_diff, g_diff, b_diff;
      }
      //specular r g b 
      //  speciï¬es the specular color of the surface.
      else if(!splitline[0].compare("specular")) {
        // r: atof(splitline[1].c_str())
        // g: atof(splitline[2].c_str())
        // b: atof(splitline[3].c_str())
        // Update current properties
      }
      //shininess s
      //  speciï¬es the shininess of the surface.
      else if(!splitline[0].compare("shininess")) {
        // shininess: atof(splitline[1].c_str())
        // Update current properties
      }
      //emission r g b
      //  gives the emissive color of the surface.
      else if(!splitline[0].compare("emission")) {
        // r: atof(splitline[1].c_str())
        // g: atof(splitline[2].c_str())
        // b: atof(splitline[3].c_str())
        // Update current properties
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
