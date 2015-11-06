#include <cmath>

#include "Sphere.h"

#ifndef M_PI
#define M_PI 3.14159265358979
#endif

bool Sphere::intersectLocal( const ray& r, isect& i ) const
{
	vec3f v = -r.getPosition();
	double b = v.dot(r.getDirection());
	double discriminant = b*b - v.dot(v) + 1;

	if( discriminant < 0.0 ) {
		return false;
	}

	discriminant = sqrt( discriminant );
	double t2 = b + discriminant;

	if( t2 <= RAY_EPSILON ) {
		return false;
	}

	i.obj = this;

	double t1 = b - discriminant;

	if( t1 > RAY_EPSILON ) {
		i.t = t1;
		i.N = r.at( t1 ).normalize();
		if (i.N * r.getDirection() > 0)
			i.N = -i.N;
	} else {
		i.t = t2;
		i.N = r.at( t2 ).normalize();
		if (i.N * r.getDirection() > 0)
			i.N = -i.N;
	}

	return true;
}

void Sphere::isectTo2DMap(const isect& i, const vec3f& pos, int density, int& x, int& y) const
{
	vec3f posLocal = transform->globalToLocalCoords(pos);

	double u = acos(posLocal[1] / posLocal.length());

	double v;

	if (abs(posLocal[0]) > 1e-8)
	{
		v = atan(posLocal[2] / posLocal[0]);
	}
	else
	{
		v = M_PI / 2;
	}

	x = (2 - u / M_PI) * density;

	if (v < 0)
		y = v / (2 * M_PI) * density + density / 2;
	else
		y = v / (2 * M_PI) * density;

	if (x < 0) x = 0;
	if (y < 0) y = 0;
}