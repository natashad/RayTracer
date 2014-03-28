/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		implements light_source.h

***********************************************************/

#include <cmath>
#include <ctgmath>
#include "light_source.h"

void PointLight::shade( Ray3D& ray ) {
	// TODO: implement this function to fill in values for ray.col
	// using phong shading.  Make sure your vectors are normalized, and
	// clamp colour values to 1.0.
	//
	// It is assumed at this point that the intersection information in ray
	// is available.  So be sure that traverseScene() is called on the ray
	// before this function.

	Colour KA = (*ray.intersection.mat).ambient;
	Colour KD = (*ray.intersection.mat).diffuse;
	Colour KS = (*ray.intersection.mat).specular;


	double alpha = (*ray.intersection.mat).specular_exp;

	Colour IA = _col_ambient;
	Colour ID = _col_diffuse;
	Colour IS = _col_specular;


	// std::cout << ray.intersection.untransformedPoint

	double x = 0;
	double y = 0;
	x = ray.intersection.untransformedPoint[0]+1;
	y = ray.intersection.untransformedPoint[1]+1;

	x*=1000;
	y*=1000;

	// std::cout << x << "," << y << std::endl;
	if ((int)x/20 % 2 == (int)y/20 % 2) {
		KA = (*ray.intersection.mat).ambient2;
		KD = (*ray.intersection.mat).diffuse2;
		KS = (*ray.intersection.mat).specular2;
	}


	if (ray.inShadow) {
		Colour shade = KA*IA;
		shade.clamp();
		ray.col = shade;
		return;
	}

	Vector3D N = (ray.intersection.normal);
	N.normalize();

	Vector3D L = (_pos - ray.intersection.point);
	L.normalize();

	Vector3D V = (-ray.dir);
	V.normalize();

	Vector3D R = ((2 * (L.dot(N)) * N) - L);
	R.normalize();

	double max1 = fmax(0.0, N.dot(L));

	double vr_alpha = pow(V.dot(R), alpha);
	double max2 = fmax(0.0, vr_alpha);

	Colour shade = KA * IA + KD * (max1 * ID) + KS * (max2 * IS);



	shade.clamp();
	ray.col = shade;

}

