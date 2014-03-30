/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		Implementations of functions in raytracer.h,
		and the main function which specifies the
		scene to be rendered.

***********************************************************/


#include "raytracer.h"
#include "bmp_io.h"
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <sstream>

Raytracer::Raytracer() : _lightSource(NULL) {
	_root = new SceneDagNode();
}

Raytracer::~Raytracer() {
	delete _root;
}

SceneDagNode* Raytracer::addObject( SceneDagNode* parent,
		SceneObject* obj, Material* mat ) {
	SceneDagNode* node = new SceneDagNode( obj, mat );
	node->parent = parent;
	node->next = NULL;
	node->child = NULL;

	// Add the object to the parent's child list, this means
	// whatever transformation applied to the parent will also
	// be applied to the child.
	if (parent->child == NULL) {
		parent->child = node;
	}
	else {
		parent = parent->child;
		while (parent->next != NULL) {
			parent = parent->next;
		}
		parent->next = node;
	}

	return node;;
}

LightListNode* Raytracer::addLightSource( LightSource* light ) {
	LightListNode* tmp = _lightSource;
	_lightSource = new LightListNode( light, tmp );
	return _lightSource;
}

void Raytracer::rotate( SceneDagNode* node, char axis, double angle ) {
	Matrix4x4 rotation;
	double toRadian = 2*M_PI/360.0;
	int i;

	for (i = 0; i < 2; i++) {
		switch(axis) {
			case 'x':
				rotation[0][0] = 1;
				rotation[1][1] = cos(angle*toRadian);
				rotation[1][2] = -sin(angle*toRadian);
				rotation[2][1] = sin(angle*toRadian);
				rotation[2][2] = cos(angle*toRadian);
				rotation[3][3] = 1;
			break;
			case 'y':
				rotation[0][0] = cos(angle*toRadian);
				rotation[0][2] = sin(angle*toRadian);
				rotation[1][1] = 1;
				rotation[2][0] = -sin(angle*toRadian);
				rotation[2][2] = cos(angle*toRadian);
				rotation[3][3] = 1;
			break;
			case 'z':
				rotation[0][0] = cos(angle*toRadian);
				rotation[0][1] = -sin(angle*toRadian);
				rotation[1][0] = sin(angle*toRadian);
				rotation[1][1] = cos(angle*toRadian);
				rotation[2][2] = 1;
				rotation[3][3] = 1;
			break;
		}
		if (i == 0) {
		    node->trans = node->trans*rotation;
			angle = -angle;
		}
		else {
			node->invtrans = rotation*node->invtrans;
		}
	}
}

void Raytracer::translate( SceneDagNode* node, Vector3D trans ) {
	Matrix4x4 translation;

	translation[0][3] = trans[0];
	translation[1][3] = trans[1];
	translation[2][3] = trans[2];
	node->trans = node->trans*translation;
	translation[0][3] = -trans[0];
	translation[1][3] = -trans[1];
	translation[2][3] = -trans[2];
	node->invtrans = translation*node->invtrans;
}

void Raytracer::scale( SceneDagNode* node, Point3D origin, double factor[3] ) {
	Matrix4x4 scale;

	scale[0][0] = factor[0];
	scale[0][3] = origin[0] - factor[0] * origin[0];
	scale[1][1] = factor[1];
	scale[1][3] = origin[1] - factor[1] * origin[1];
	scale[2][2] = factor[2];
	scale[2][3] = origin[2] - factor[2] * origin[2];
	node->trans = node->trans*scale;
	scale[0][0] = 1/factor[0];
	scale[0][3] = origin[0] - 1/factor[0] * origin[0];
	scale[1][1] = 1/factor[1];
	scale[1][3] = origin[1] - 1/factor[1] * origin[1];
	scale[2][2] = 1/factor[2];
	scale[2][3] = origin[2] - 1/factor[2] * origin[2];
	node->invtrans = scale*node->invtrans;
}

Matrix4x4 Raytracer::initInvViewMatrix( Point3D eye, Vector3D view,
		Vector3D up ) {
	Matrix4x4 mat;
	Vector3D w;
	view.normalize();
	up = up - up.dot(view)*view;
	up.normalize();
	w = view.cross(up);

	mat[0][0] = w[0];
	mat[1][0] = w[1];
	mat[2][0] = w[2];
	mat[0][1] = up[0];
	mat[1][1] = up[1];
	mat[2][1] = up[2];
	mat[0][2] = -view[0];
	mat[1][2] = -view[1];
	mat[2][2] = -view[2];
	mat[0][3] = eye[0];
	mat[1][3] = eye[1];
	mat[2][3] = eye[2];

	return mat;
}

void Raytracer::traverseScene( SceneDagNode* node, Ray3D& ray ) {
	SceneDagNode *childPtr;

	// Applies transformation of the current node to the global
	// transformation matrices.
	_modelToWorld = _modelToWorld*node->trans;
	_worldToModel = node->invtrans*_worldToModel;
	if (node->obj) {
		// Perform intersection.
		if (node->obj->intersect(ray, _worldToModel, _modelToWorld)) {
			ray.intersection.mat = node->mat;
		}
	}
	// Traverse the children.
	childPtr = node->child;
	while (childPtr != NULL) {
		traverseScene(childPtr, ray);
		childPtr = childPtr->next;
	}

	// Removes transformation of the current node from the global
	// transformation matrices.
	_worldToModel = node->trans*_worldToModel;
	_modelToWorld = _modelToWorld*node->invtrans;
}

void Raytracer::computeShading( Ray3D& ray ) {
	LightListNode* curLight = _lightSource;
	for (;;) {
		if (curLight == NULL) break;
		// Each lightSource provides its own shading function.


		/** Start basic shadow implementation **/
		// Implement shadows here if needed.
		Ray3D shadowRay;

		shadowRay.dir = (curLight->light->get_position() - ray.intersection.point);
		shadowRay.dir.normalize();

		shadowRay.origin = ray.intersection.point;
		shadowRay.origin = shadowRay.origin + 0.01*shadowRay.dir;

		traverseScene(_root, shadowRay);
		if (!shadowRay.intersection.none && SHADOWS_ON) {
			ray.inShadow = true;
		}

		/** End basic shadow implementation **/

		curLight->light->shade(ray);
		curLight = curLight->next;
		ray.inShadow = false;

	}
}

void Raytracer::initPixelBuffer() {
	int numbytes = _scrWidth * _scrHeight * sizeof(unsigned char);
	_rbuffer = new unsigned char[numbytes];
	_gbuffer = new unsigned char[numbytes];
	_bbuffer = new unsigned char[numbytes];
	for (int i = 0; i < _scrHeight; i++) {
		for (int j = 0; j < _scrWidth; j++) {
			_rbuffer[i*_scrWidth+j] = 0;
			_gbuffer[i*_scrWidth+j] = 0;
			_bbuffer[i*_scrWidth+j] = 0;
		}
	}
}

void Raytracer::flushPixelBuffer( char *file_name ) {
	bmp_write( file_name, _scrWidth, _scrHeight, _rbuffer, _gbuffer, _bbuffer );
	delete _rbuffer;
	delete _gbuffer;
	delete _bbuffer;
}

Colour Raytracer::shadeRay( Ray3D& ray, int depth ) {
	Colour col(0.0, 0.0, 0.0);
	traverseScene(_root, ray);

	// Don't bother shading if the ray didn't hit
	// anything.
	if (!ray.intersection.none) {
		computeShading(ray);
		col = ray.col;
	}

	int maxDepth = 2;

	// You'll want to call shadeRay recursively (with a different ray,
	// of course) here to implement reflection/refraction effects.

	/* Reflection Code starts here */

	// SceneDagNode *nextNode = node->next;
	if (!ray.intersection.none && depth < maxDepth) {

		Ray3D reflectionRay;


		Vector3D rayNormal = ray.intersection.normal;
		rayNormal.normalize();

		Vector3D rayDir = ray.dir;
		rayDir.normalize();

		reflectionRay.dir =
			(rayDir - (2*(rayDir.dot(rayNormal))*rayNormal));
		reflectionRay.dir.normalize();

		reflectionRay.origin = ray.intersection.point;
		reflectionRay.origin = reflectionRay.origin + 0.01*reflectionRay.dir;

		if ((GLOSSY_REFLECTION_ON || REFLECTION_ON) && ray.intersection.mat->damp_factor > 0) {
			if (!GLOSSY_REFLECTION_ON) {
				col = col + ray.intersection.mat->damp_factor*shadeRay(reflectionRay, depth+1);
				col.clamp();
			} else {

				double numSamples = 4;
				for (int x = 0; x < numSamples; x++) {

						double x_diff = ((((double)rand() / (double)RAND_MAX)*3) - 1) * 0.005;
						double y_diff = ((((double)rand() / (double)RAND_MAX)*3) - 1) * 0.005;
						double z_diff = ((((double)rand() / (double)RAND_MAX)*3) - 1) * 0.005;


						Point3D new_origin = reflectionRay.origin;
						new_origin[0] += x_diff;
						new_origin[1] += y_diff;
						new_origin[2] += z_diff;
						Ray3D newRay(new_origin + 0.01*reflectionRay.dir, reflectionRay.dir);
						col = col + ray.intersection.mat->damp_factor*shadeRay(newRay, depth+1);
				}

				col = (1.0/(numSamples))*col;
				col.clamp();

			}
		}

		/* Refraction Code starts here */

		Ray3D refractionRay;

		double c1 = 1; //c1 is for air
		double c2 = ray.intersection.mat->ref_index;
		double c1byc2 = c1/(c2*1.0);

		if (depth % 2 == 1) {
			// then we're going from meterial to air
			c1byc2 = 1/c1byc2;
		}

		double costheta2 = -rayNormal.dot(rayDir);
		double sintheta2 = sqrt(1 - (costheta2*costheta2));
		double sintheta1 = (c1byc2) * sintheta2;
		double costheta1 = sqrt(1 - (sintheta1*sintheta1));

		double cosT2 = 1 - c1byc2*c1byc2*sintheta2*sintheta2;

		bool tir = c1byc2 == sintheta1;


		if (REFRACTION_ON) {
			Vector3D refrDir = -c1byc2*rayDir + (c1byc2*costheta2 - costheta1)*rayNormal;
			refractionRay.dir = refrDir;
			refractionRay.dir.normalize();
			refractionRay.origin = ray.intersection.point + 0.01*refractionRay.dir;
			col = col + shadeRay(refractionRay, depth + 1);
			col.clamp();
		}

		/* Refraction Code ends here */
	}

	/* Reflection Code ends here */
	col.clamp();
	return col;
}

void Raytracer::render( int width, int height, Point3D eye, Vector3D view,
		Vector3D up, double fov, char* fileName ) {
	Matrix4x4 viewToWorld;
	_scrWidth = width;
	_scrHeight = height;
	double factor = (double(height)/2)/tan(fov*M_PI/360.0);

	initPixelBuffer();
	viewToWorld = initInvViewMatrix(eye, view, up);

	double numSamples = ANTIALIAS_ON? 4 : 1;

	double focal_length =  10;

	// Construct a ray for each pixel.
	for (int i = 0; i < _scrHeight; i++) {
		for (int j = 0; j < _scrWidth; j++) {

			Colour col(0,0,0);

			for (double x = 0; x < numSamples; ++x) {
				for (double y = 0; y < numSamples; ++y) {
					double distx = (x + 1.0*rand()/RAND_MAX)/numSamples;
					double disty = (y + 1.0*rand()/RAND_MAX)/numSamples;

					// Sets up ray origin and direction in view space,
					// image plane is at z = -1.
					Point3D origin(0, 0, 0);
					Ray3D ray;
					Point3D imagePlane;

					if (ANTIALIAS_ON) {
						imagePlane[0] = (-double(width)/2 + distx + j)/factor;
						imagePlane[1] = (-double(height)/2 + disty + i)/factor;
						imagePlane[2] = -1;
					} else {
						imagePlane[0] = (-double(width)/2 + 0.5 + j)/factor;
						imagePlane[1] = (-double(height)/2 + 0.5 + i)/factor;
						imagePlane[2] = -1;
					}

					ray.origin = viewToWorld * origin;
					ray.dir = viewToWorld*imagePlane - eye;


					if(DOF_ON) {
						Point3D focal_point = ray.origin + (focal_length * ray.dir);
						ray.origin = ray.origin + Vector3D(((((double)rand() / (double)RAND_MAX)*3) - 1)*0.05,
															((((double)rand() / (double)RAND_MAX)*3) - 1)*0.05,
															((((double)rand() / (double)RAND_MAX)*3) - 1)*0.05);
						ray.dir = focal_point - ray.origin;


					}

					// ray.dir.normalize();

					col = col + shadeRay(ray, 0);
				}

			}

			col = (1/(numSamples*numSamples))*col;
			col.clamp();


			_rbuffer[i*width+j] = int(col[0]*255);
			_gbuffer[i*width+j] = int(col[1]*255);
			_bbuffer[i*width+j] = int(col[2]*255);
		}
	}

	flushPixelBuffer(fileName);
}

/* Draws the original scene.. Egg on plane */
void drawOriginalScene(int width, int height) {

	Raytracer raytracer;

	// Camera parameters.
	Point3D eye(0, 0, 1);
	Vector3D view(0, 0, -1);
	Vector3D up(0, 1, 0);
	double fov = 60;

	// Defines a material for shading.
	Material gold( Colour(0.3, 0.3, 0.3), Colour(0.75164, 0.60648, 0.22648),
			Colour(0.628281, 0.555802, 0.366065),
			51.2, 1.0, 0);

	Material jade( Colour(0, 0, 0), Colour(0.54, 0.89, 0.63),
			Colour(0.316228, 0.316228, 0.316228),
			12.8, 0.0, 0 );


	// Defines a point light source.
	raytracer.addLightSource( new PointLight(Point3D(0, 0, 5),
				Colour(0.9, 0.9, 0.9) ) );

	// Add a unit square into the scene with material mat.
	SceneDagNode* sphere = raytracer.addObject( new UnitSphere(), &gold );
	SceneDagNode* plane = raytracer.addObject( new UnitSquare(), &jade );

	// Apply some transformations to the unit square.
	double factor1[3] = { 1.0, 2.0, 1.0 };
	double factor2[3] = { 6.0, 6.0, 6.0 };

	raytracer.translate(sphere, Vector3D(0, 0, -5));
	raytracer.rotate(sphere, 'x', -45);
	raytracer.rotate(sphere, 'z', 45);
	raytracer.scale(sphere, Point3D(0, 0, 0), factor1);

	raytracer.translate(plane, Vector3D(0, 0, -7));
	raytracer.rotate(plane, 'z', 40);
	raytracer.scale(plane, Point3D(0, 0, 0), factor2);

	// Render the scene, feel free to make the image smaller for
	// testing purposes.
	raytracer.render(width, height, eye, view, up, fov, "view1.bmp");

	// Render it from a different point of view.
	Point3D eye2(4, 2, 1);
	Vector3D view2(-4, -2, -6);
	raytracer.render(width, height, eye2, view2, up, fov, "view2.bmp");
}

void drawNewScene(int width, int height) {
	Raytracer raytracer;

	// Camera parameters.
	Point3D eye(0, 0, 1);
	Vector3D view(0, 0, -1);
	Vector3D up(0, 1, 0);
	double fov = 60;

	//208, 32, 144
	// Defines a material for shading.
	Material gold( Colour(0.3, 0.3, 0.3), Colour(0.75164, 0.60648, 0.22648),
			Colour(0.628281, 0.555802, 0.366065),
			51.2, 0.0, 0 );
	Material jade( Colour(0, 0, 0), Colour(0.54, 0.89, 0.63),
			Colour(0.316228, 0.316228, 0.316228),
			12.8, 0, 0 );
	Material pink( Colour(0.815, 0.125, 0.56), Colour(0.6, 0.5, 0.5),
			Colour(0.6, 0.6, 0.6), 30, 0.0, 0.0);
	Material blue( Colour(0.27, 0.5, 0.7), Colour(0.6, 0.5, 0.5),
			Colour(0.316228, 0.316228, 0.316228), 12, 0.0, 0);
	Material silver( Colour(0.19225,0.19225,0.19225), Colour(0.50754, 0.50754, 0.50754),
					Colour(0.508273, 0.508273, 0.508273), 51.2, 1.0, 0);
	Material chrome( Colour(.25,0.25,0.25), Colour(0.4,0.4,0.4),
					 Colour(0.774597,0.774597,0.774597), 76.8, 0.0, 0);
	Material brass ( Colour(0.329412,0.223529,0.027451), Colour(0.780392,0.568627,0.113725),
					 Colour(0.992157, 0.941176, 0.807843), 27.89743616, 0.0, 0);
	// Material white( Colour(0.8,0.7,0.7), Colour(0.5,0.5,0.4), Colour(0.7,0.7,0.04), 10, 0);
	Material white( Colour(0,0,0), Colour(0.55,0.55,0.55), Colour(0.7,0.7,0.7), 32, 0.0, 0);
	Material black( Colour(0, 0, 0), Colour(0, 0, 0),
			Colour(0, 0, 0),
			51.2, 0.75, 0);

	// Defines a point light source.
	raytracer.addLightSource( new PointLight(Point3D(1,1,3),
				Colour(0.9, 0.9, 0.9) ) );

	// Add a unit square into the scene with material mat.
	SceneDagNode* sphere1 = raytracer.addObject( new UnitSphere(), &brass );
	SceneDagNode* sphere2 = raytracer.addObject( new UnitSphere(), &pink );
	// SceneDagNode* planeL = raytracer.addObject( new UnitSquare(), &jade );
	// SceneDagNode* planeR = raytracer.addObject( new UnitSquare(), &jade );
	SceneDagNode* planeT = raytracer.addObject( new UnitSquare(), &blue);
	// SceneDagNode* planeB = raytracer.addObject( new UnitSquare(), &white );


	// Apply some transformations to the unit square.
	double factor1[3] = { 1, 1, 1 };
	double factor2[3] = { 100.0, 100.0, 100.0 };
	double factor3[3] = { 0.15, 0.15, 0.15 };

	raytracer.translate(sphere1, Vector3D(0.5, 0.5, -8));


	raytracer.translate(sphere2, Vector3D(-1, -0.5, -4));

	// raytracer.translate(planeL, Vector3D(-3, 0, -6));
	// raytracer.rotate(planeL, 'y', 90);
	// raytracer.scale(planeL, Point3D(0, 0, 0), factor2);

	// raytracer.translate(planeR, Vector3D(3, 0, -6));
	// raytracer.rotate(planeR, 'y', -90);
	// raytracer.scale(planeR, Point3D(0, 0, 0), factor2);

	raytracer.translate(planeT, Vector3D(0, -3, -50));
	raytracer.rotate(planeT, 'x', 90);
	raytracer.scale(planeT, Point3D(0, 0, 0), factor2);

	// raytracer.translate(planeB, Vector3D(0, 0, -9));
	// raytracer.scale(planeB, Point3D(0, 0, 0), factor2);


	// Render the scene, feel free to make the image smaller for
	// testing purposes.
	raytracer.render(width, height, eye, view, up, fov, "view3.bmp");

	// // Render it from a different point of view.
	// Point3D eye2(4, 2, 1);
	// Vector3D view2(-4, -2, -6);
	// raytracer.render(width, height, eye2, view2, up, fov, "view3.bmp");
}

void drawBasicScene(int width, int height) {
	Raytracer raytracer;

	// Camera parameters.
	Point3D eye(0, 0, 1);
	Vector3D view(0, 0, -1);
	Vector3D up(0, 1, 0);
	double fov = 60;

	Material black( Colour(0, 0, 0), Colour(0, 0, 0),
			Colour(0.2, 0.2, 0.2),
			12, 0.5, 1.5);
	Material jade( Colour(0, 0, 0), Colour(0.54, 0.89, 0.63),
			Colour(0.316228, 0.316228, 0.316228),
			12.8, 0, 0);
	Material checkerBoard( Colour(0, 0, 0), Colour(0,0,0),
			Colour(0,0,0),
			12.8, 0, 0,
			Colour(0, 0, 0), Colour(0.5, 0, 1.0),
			Colour(0.7, 0.6, 0.6),
			32, 0, 0  );
	Material silver( Colour(0.19225,0.19225,0.19225), Colour(0.50754, 0.50754, 0.50754),
					Colour(0.508273, 0.508273, 0.508273), 51.2, 1.0, 1.5);

	double factor2[3] = { 6.0, 6.0, 6.0 };
	double factor3[3] = { 0.15, 0.15, 0.15 };

	// Defines a point light source.
	raytracer.addLightSource( new PointLight(Point3D(1, 3, 3),
				Colour(0.9, 0.9, 0.9) ) );

	// Add a unit square into the scene with material mat.
	SceneDagNode* sphere1 = raytracer.addObject( new UnitSphere(), &black );
	SceneDagNode* planeT = raytracer.addObject( new UnitSquare(), &jade );

	raytracer.rotate(planeT, 'x', -80);
	raytracer.translate(planeT, Vector3D(0, 0, 0));
	raytracer.scale(planeT, Point3D(0, 0, 0), factor2);

	raytracer.translate(sphere1, Vector3D(0, 0.0, 0.6));
	raytracer.scale(sphere1, Point3D(0, 0, 0), factor3);

	// Point3D eye2(4, 2, 1);
	// Vector3D view2(-4, -2, -6);
	// raytracer.render(width, height, eye2, view2, up, fov, "reflection2.bmp");
	raytracer.render(width, height, eye, view, up, fov, "reflections.bmp");

}

void drawCylinderScene(int width, int height) {
	Raytracer raytracer;

	// Camera parameters.
	Point3D eye(0, 0, 1);
	Vector3D view(0, 0, -1);
	Vector3D up(0, 1, 0);
	double fov = 60;

	Material black( Colour(0, 0, 0), Colour(0, 0, 0),
			Colour(0, 0, 0),
			12, 0.8, 1.5);
	Material jade( Colour(0, 0, 0), Colour(0.54, 0.89, 0.63),
			Colour(0.316228, 0.316228, 0.316228),
			12.8, 0, 0);
	Material checkerBoard( Colour(0, 0, 0), Colour(0,0,0),
			Colour(0,0,0),
			12.8, 0, 0,
			Colour(0, 0, 0), Colour(0.5, 0, 1.0),
			Colour(0.7, 0.6, 0.6),
			32, 0, 0  );


	Material silver( Colour(0.19225,0.19225,0.19225), Colour(0.50754, 0.50754, 0.50754),
					Colour(0.508273, 0.508273, 0.508273), 51.2, 0.8, 1.5);

	double factor2[3] = { 6.0, 6.0, 6.0 };
	double factor3[3] = { 0.5, 1.0, 0.5};

	// Defines a point light source.
	raytracer.addLightSource( new PointLight(Point3D(1, 3, 3),
				Colour(0.9, 0.9, 0.9) ) );

	// Add a unit square into the scene with material mat.
	SceneDagNode* cylinder = raytracer.addObject( new UnitCylinder(), &jade);
	SceneDagNode* planeT = raytracer.addObject( new UnitSquare(), &silver );

	raytracer.rotate(planeT, 'x', -80);
	raytracer.translate(planeT, Vector3D(0, 0, -0.8));
	raytracer.scale(planeT, Point3D(0, 0, 0), factor2);

	raytracer.translate(cylinder, Vector3D(0, 0, -2.5));
	raytracer.rotate(cylinder, 'y', 45);
	raytracer.rotate(cylinder, 'x', 90);
	raytracer.scale(cylinder, Point3D(0, 0.3, 0), factor3);

	raytracer.render(width, height, eye, view, up, fov, "cylinder.bmp");

}


int main(int argc, char* argv[])
{
	// Build your scene and setup your camera here, by calling
	// functions from Raytracer.  The code here sets up an example
	// scene and renders it from two different view points, DO NOT
	// change this if you're just implementing part one of the
	// assignment.
	// int width = 160;
	// int height = 120;

	int width = 320;
	int height = 240;

	// int width = 640;
	// int height = 480;

	// int width = 1920;
	// int height = 1080;

	if (argc == 3) {
		width = atoi(argv[1]);
		height = atoi(argv[2]);
	}

	drawOriginalScene(width, height);
	// drawNewScene(width, height);
	// drawBasicScene(width, height);
	drawCylinderScene(width, height);


	return 0;
}

