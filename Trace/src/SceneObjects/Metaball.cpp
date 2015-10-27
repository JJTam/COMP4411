#include <cmath>
#include <assert.h>
#include <float.h>
#include <utility>
#include "Metaball.h"
#include <iostream>
using namespace std;


bool intersectCircle(const ray& r0, isect& i, const vec3f& center)
{
	ray r(r0.getPosition() - center, r0.getDirection());

	vec3f v = -r.getPosition();
	double b = v.dot(r.getDirection());
	double discriminant = b*b - v.dot(v) + 1;

	if (discriminant < 0.0) {
		return false;
	}

	discriminant = sqrt(discriminant);
	double t2 = b + discriminant;

	if (t2 <= RAY_EPSILON) {
		return false;
	}



	double t1 = b - discriminant;

	if (t1 > RAY_EPSILON) {
		i.t = t1;
		i.N = r.at(t1).normalize();
		if (i.N * r.getDirection() > 0)
			i.N = -i.N;
	}
	else {
		i.t = t2;
		i.N = r.at(t2).normalize();
		if (i.N * r.getDirection() > 0)
			i.N = -i.N;
	}

	return true;
}

double calvalue(const vec3f& point, const vec3f& ball1pos, const vec3f& ball2pos)
{
	vec3f balltopoint;
	double value = 0;
	double Distance;
	//ball1
	balltopoint = point - ball1pos;
	Distance = balltopoint.length();
	if (0 < Distance&&Distance < 1)
	{
		value += 1 - Distance * Distance * Distance * (Distance * (Distance * 6 - 15) + 10);
	}

	//ball2
	balltopoint = point - ball2pos;
	Distance = balltopoint.length();
	if (0 < Distance&&Distance < 1)
	{
		value += 1 - Distance * Distance * Distance * (Distance * (Distance * 6 - 15) + 10);
	}

	return value;
}

bool Metaball::intersectLocal(const ray& r, isect& i) const
{
	isect tmp1;
	isect tmp2;
	if (intersectCircle(r, tmp1, ball1pos) == false && intersectCircle(r, tmp2, ball2pos) == false) return false;
	for (double t = 5; t < 10.0; t += 0.02)
	{
		vec3f point = r.getPosition() + t * r.getDirection();
		double value = calvalue(point, ball1pos, ball2pos);

		if (value > threshold)
		{
			vec3f normal;
			normal += 2 * (point - ball1pos) / ((point - ball1pos).length()*(point - ball1pos).length());
			normal += 2 * (point - ball2pos) / ((point - ball2pos).length()*(point - ball2pos).length());
			normal = normal.normalize();
			i.t = t;
			i.N = normal;
			i.obj = this;
			return true;
		}
	}
	return false;
}