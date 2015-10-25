#include <cmath>
#include <assert.h>
#include <float.h>
#include <utility>
#include "Box.h"
#include <iostream>
using namespace std;

bool Box::intersectLocal( const ray& r, isect& i ) const
{
	// YOUR CODE HERE:
    // Add box intersection code here.
	// it currently ignores all boxes and just returns false.
	vec3f origin = r.getPosition();
	vec3f dir = r.getDirection();
	double Tnear = -DBL_MAX;
	double Tfar = DBL_MAX;
	double T1, T2;
	double error = 1.0e-10;
	// X plane
	if (dir[0] == 0)
	{
		if (origin[0]<-0.5 || origin[0]>0.5)
		{
			return false;
		}
	}
	else
	{
		T1 = (-0.5 - origin[0]) / dir[0];
		T2 = (0.5 - origin[0]) / dir[0];
		vec3f tmpN(-1, 0, 0);
		if (T1 > T2)
		{
			swap<double>(T1, T2);
			tmpN[0] = 1;
		}
		if (T1 > Tnear)
		{
			Tnear = T1;
			i.N = tmpN;
		}
		if (T2 < Tfar)Tfar = T2;
		if (Tnear > Tfar || Tfar <= error)return false;
	}
	// Y plane
	if (dir[1] == 0)
	{
		if (origin[1]<-0.5 || origin[1]>0.5)
		{
			return false;
		}
	}
	else
	{
		T1 = (-0.5 - origin[1]) / dir[1];
		T2 = (0.5 - origin[1]) / dir[1];
		vec3f tmpN(0, -1, 0);
		if (T1 > T2)
		{
			swap<double>(T1, T2);
			tmpN[1] = 1;
		}
		if (T1 > Tnear)		
		{
			Tnear = T1;
			i.N = tmpN;
		}
		if (T2 < Tfar)Tfar = T2;
		if (Tnear > Tfar || Tfar <= error)return false;
	}
	// Z plane
	if (dir[2] == 0)
	{
		if (origin[2]<-0.5 || origin[2]>0.5)
		{
			return false;
		}
	}
	else
	{
		T1 = (-0.5 - origin[2]) / dir[2];
		T2 = (0.5 - origin[2]) / dir[2];
		vec3f tmpN(0, 0, -1);
		if (T1 > T2)
		{
			swap<double>(T1, T2);
			tmpN[2] = 1;
		}
		if (T1 > Tnear)		
		{
			Tnear = T1;
			i.N = tmpN;
		}
		if (T2 < Tfar)Tfar = T2;
		if (Tnear > Tfar || Tfar <= error)return false;
	}
	i.t = Tnear;
	i.obj = this;
	const Material& tmp = getMaterial();
	Material *m = new Material();
	*m += tmp;
	i.setMaterial(m);
	return true;
}
