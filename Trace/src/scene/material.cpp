#include "ray.h"
#include "material.h"
#include "light.h"

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
vec3f Material::shade( Scene *scene, const ray& r, const isect& i ) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
    // You will need to call both distanceAttenuation() and shadowAttenuation()
    // somewhere in your code in order to compute shadows and light falloff.
	
	vec3f zero;
	// sum of ambient
	vec3f sum1;
	for (auto j = scene->beginLights(); j != scene->endLights(); ++j)
	{
		sum1 += prod(ka, (*j)->getAmbientColor(zero));
	}

	// sum of lights
	vec3f sum2;
	for (auto j = scene->beginLights(); j != scene->endLights(); ++j)
	{
		auto currlight = *j;
		double NL = (i.N * currlight->getDirection(r.getPosition() + i.t*r.getDirection()));
		vec3f Rm = (2 * NL * i.N - currlight->getDirection(r.getPosition() + i.t*r.getDirection())).normalize();
		double VR = Rm * -r.getDirection();
		if (NL < 0)NL = 0;
		if (VR < 0)VR = 0;
		VR = pow(VR, shininess * 128);

		//a = constant_attenuation_coeff(from the.ray file)
		//b = linear_attenuation_coeff(from the.ray file)
		//c = quadratic_attenuation_coeff(from the.ray file)
		double a = 0.25;
		double b = 0.01;
		double c = 0.01;
		
		sum2 += prod(currlight->getColor(zero), NL*kd + VR * ks * currlight->distanceAttenuation(r.getPosition() + i.t*r.getDirection()));
	}
	return ke+sum1+sum2;
}
