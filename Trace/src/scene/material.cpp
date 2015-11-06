#include "ray.h"
#include "material.h"
#include "light.h"

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
vec3f Material::shade( Scene *scene, const ray& r, const isect& i ) const
{
	vec3f zero;
	// sum of ambient
	vec3f sum1;
	for (auto j = scene->beginLights(); j != scene->endLights(); ++j)
	{
		sum1 += prod(ka, (*j)->getAmbientColor(zero));
	}

	// sum of lights
	vec3f sum2;
	vec3f isectpos = r.getPosition() + i.t*r.getDirection();
	for (auto j = scene->beginLights(); j != scene->endLights(); ++j)
	{
		auto currlight = *j;
		if (currlight->getDirection(isectpos).iszero())
			continue;
		double NL = (i.N * currlight->getDirection(isectpos));
		vec3f Rm = (2 * NL * i.N - currlight->getDirection(isectpos)).normalize();
		double VR = Rm * -r.getDirection();
		if (NL < 0)NL = 0;
		if (VR < 0)VR = 0;
		VR = pow(VR, shininess * 128);

		vec3f shadowA = currlight->shadowAttenuation(isectpos);
		shadowA[0] = ((int)(shadowA[0] * 255)) / 40 * 40 / 255.0;
		shadowA[1] = ((int)(shadowA[0] * 255)) / 40 * 40 / 255.0;
		shadowA[2] = ((int)(shadowA[0] * 255)) / 40 * 40 / 255.0;
		double distA = currlight->distanceAttenuation(isectpos);
		distA = ((int)(distA * 255)) / 40 * 40 / 255.0;
		sum2 += prod(shadowA, prod(currlight->getColor(isectpos), (NL*kd + VR * ks) * distA));
	}

	if (i.obj->supports2DMap() && textureBitmap != NULL && textureBitmapWidth > 0 && textureBitmapHeight > 0)
	{
		int x = 0;
		int y = 0;
		i.obj->isectTo2DMap(i, isectpos, textureDensity, x, y);
		x %= textureBitmapWidth;
		y %= textureBitmapHeight;
		vec3f tKe(textureBitmap[3 * (y * textureBitmapWidth + x)] / 255.0,
			textureBitmap[3 * (y * textureBitmapWidth + x) + 1] / 255.0,
			textureBitmap[3 * (y * textureBitmapWidth + x) + 2] / 255.0);
		return tKe + sum1 + sum2;
	}
	else
	{
		return ke + sum1 + sum2;
	}
}
