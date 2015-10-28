#include <cmath>
#include <assert.h>
#include <float.h>
#include <utility>
#include "Metaball.h"
#include <iostream>
using namespace std;


bool intersectCircle(const ray& r0, double& tnear, double& tfar, const vec3f& center)
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
		tnear = t1;
		tfar = t2;
	}
	else {
		tnear = 0;
		tfar = t2;
	}

	return true;
}

// function for calculate potential
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
	bool inside = false;
	if (calvalue(r.getPosition(), ball1pos, ball2pos)>threshold)inside = true;

	//determine possible intersect range
	double t11=0, t12=0, t21=0, t22=0;
	double tmin, tmax;
	bool i1, i2;
	i1 = intersectCircle(r, t11, t12, ball1pos);
	i2 = intersectCircle(r, t21, t22, ball2pos);
	if (!i1 && !i2) return false;
	else if (!i1 && i2)
	{
		tmin = t21;
		tmax = t22;
	}
	else if (i1 && !i2)
	{
		tmin = t11;
		tmax = t12;
	}
	else
	{
		tmin = min(t11, t21);
		tmax = max(t12, t22);
	}

	for (double t = tmin; t < tmax; t += 0.001)
	{
		vec3f point = r.getPosition() + t * r.getDirection();
		double value = calvalue(point, ball1pos, ball2pos);

		if ((!inside && value > threshold) || (inside && value < threshold))
		{
			// prevent fake intersect
			if (inside && t < 0.01)return false;

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