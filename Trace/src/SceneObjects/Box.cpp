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
	// X,Y,Z planes
	for (int plane = 0; plane < 3; ++plane)
	{
		if (dir[plane] == 0)
		{
			if (origin[plane]<-0.5 || origin[plane]>0.5)
			{
				return false;
			}
		}
		else
		{
			T1 = (-0.5 - origin[plane]) / dir[plane];
			T2 = (0.5 - origin[plane]) / dir[plane];
			vec3f tmpN(0, 0, 0);
			tmpN[plane] = -1;
			if (T1 > T2)
			{
				swap<double>(T1, T2);
				tmpN[plane] = 1;
			}
			if (T1 > Tnear)
			{
				Tnear = T1;
				i.N = tmpN;
			}
			if (T2 < Tfar)Tfar = T2;
			if (Tnear > Tfar || Tfar <= error)return false;
		}
	}

	i.t = Tnear;
	i.obj = this;
	const Material& tmp = getMaterial();
	Material *m = new Material();
	*m += tmp;
	i.setMaterial(m);
	return true;
}
