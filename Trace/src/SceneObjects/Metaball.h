#ifndef __METABALL_H__
#define __METABALL_H__

#include "../scene/scene.h"

class Metaball
	: public MaterialSceneObject
{
public:
	Metaball(Scene *scene, Material *mat, const vec3f& pos1, const vec3f& pos2,  double tt)
		: MaterialSceneObject(scene, mat), ball1pos(pos1), ball2pos(pos2), threshold(tt)
	{
	}

	virtual bool intersectLocal(const ray& r, isect& i) const;
	virtual bool hasBoundingBoxCapability() const { return true; }
	virtual BoundingBox ComputeLocalBoundingBox()
	{
		BoundingBox localbounds;
		localbounds.max = vec3f(0.5, 0.5, 0.5);
		localbounds.min = vec3f(-0.5, -0.5, -0.5);
		return localbounds;
	}
	vec3f ball1pos;
	vec3f ball2pos;
	double threshold;
};

#endif // __METABALL_H__