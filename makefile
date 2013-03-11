raytracer: class.cpp ray_tracer.cpp common.cpp transform.cpp light.cpp
	g++ -I./ -L./ -lfreeimage class.cpp ray_tracer.cpp common.cpp transform.cpp light.cpp