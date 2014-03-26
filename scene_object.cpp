/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		implements scene_object.h

***********************************************************/

#include <cmath>
#include <iostream>
#include "scene_object.h"

bool UnitSquare::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld ) {
	// TODO: implement intersection code for UnitSquare, which is
	// defined on the xy-plane, with vertices (0.5, 0.5, 0),
	// (-0.5, 0.5, 0), (-0.5, -0.5, 0), (0.5, -0.5, 0), and normal
	// (0, 0, 1).
	//
	// Your goal here is to fill ray.intersection with correct values
	// should an intersection occur.  This includes intersection.point,
	// intersection.normal, intersection.none, intersection.t_value.
	//
	// HINT: Remember to first transform the ray into object space
	// to simplify the intersection test.
	Point3D origin = worldToModel * ray.origin;
	Vector3D dir = worldToModel * ray.dir;
	// dir.normalize();

	double t = -origin[2]/dir[2];

	if (t <= 0) {
		return false;
	}

	double x_check = origin[0] + (t * dir[0]);
	double y_check = origin[1] + (t * dir[1]);
	Point3D intersectionPoint(x_check, y_check, 0);

	if ((x_check <= 0.5 && x_check >= -0.5) &&
		(y_check <= 0.5 && y_check >= -0.5)) {

		// An intersection has occurred.
		if (ray.intersection.none ||
			t < ray.intersection.t_value) {
			// We need to update the intersection.

			Vector3D intersectionNormal = transNorm(
											worldToModel,
											Vector3D(0,0,1));

			ray.intersection.t_value = t;
			ray.intersection.point = modelToWorld * intersectionPoint;
			intersectionNormal.normalize();
			ray.intersection.normal = intersectionNormal;
			ray.intersection.none = false;

			return true;

		}
	}

	return false;
}

bool UnitSphere::intersect( Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld ) {
	// TODO: implement intersection code for UnitSphere, which is centred
	// on the origin.
	//
	// Your goal here is to fill ray.intersection with correct values
	// should an intersection occur.  This includes intersection.point,
	// intersection.normal, intersection.none, intersection.t_value.
	//
	// HINT: Remember to first transform the ray into object space
	// to simplify the intersection test.

	Point3D origin = worldToModel * ray.origin;
	Vector3D dir = worldToModel * ray.dir;
	// dir.normalize();

	Point3D center(0,0,0);
	double radius = 1;

	double a = dir.dot(dir);
	double b = (origin - center).dot(dir);
	double c = (origin - center).dot(origin - center) - radius*radius;

	double discriminant = (b*b) - (a*c);

	if (discriminant < 0) {
		return false;
	}

	double t0 = -b/a - sqrt(discriminant)/a;
	double t1 = -b/a + sqrt(discriminant)/a;
	double t = t0;

	if (t0 < 0 && t1 < 0) {
		return false;
	}

	if (t0 > 0 && t1 < 0) {
		t = t0;
	}

	if (t0 > t1 && t1 > 0) {
		t = t1;
	}


	// double t = t0;

	// if (t0 > t1) {
	// 	//ensure that t0 is smaller.
	// 	double temp = t0;
	// 	t0 = t1;
	// 	t1 = temp;
	// }

	// // t0 < t1

	// if (t1 < 0) {
	// 	return false;
	// }

	// if (t0 < 0) {
	// 	t = t1;
	// }

	if (ray.intersection.none ||
		t < ray.intersection.t_value) {
		//update the intersection.

		double x = origin[0] + t*dir[0];
		double y = origin[1] + t*dir[1];
		double z = origin[2] + t*dir[2];

		Point3D intersectionPoint(x,y,z);

		Vector3D intersectionNormal = transNorm(
											worldToModel,
											Vector3D(2*x, 2*y, 2*z));
		// intersectionNormal.normalize();

		ray.intersection.t_value = t;
		ray.intersection.point = modelToWorld * intersectionPoint;
		ray.intersection.normal = intersectionNormal;
		ray.intersection.none = false;

		return true;
	}

	return false;
}

