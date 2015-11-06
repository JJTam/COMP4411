#ifndef __CYLINDER_H__
#define __CYLINDER_H__

#include "../scene/scene.h"

class Cylinder
	: public MaterialSceneObject
{
public:
	Cylinder( Scene *scene, Material *mat , bool cap = true)
		: MaterialSceneObject( scene, mat ), capped( cap )
	{
	}

	virtual bool intersectLocal( const ray& r, isect& i ) const;
	virtual bool hasBoundingBoxCapability() const { return true; }

    virtual BoundingBox ComputeLocalBoundingBox() const
    {
        BoundingBox localbounds;
		localbounds.min = vec3f(-1.0f, -1.0f, 0.0f);
		localbounds.max = vec3f(1.0f, 1.0f, 1.0f);
        return localbounds;
    }

    bool intersectBody( const ray& r, isect& i ) const;
	bool intersectCaps( const ray& r, isect& i ) const;

	virtual bool supports2DMap() const { return true; }
	virtual void isectTo2DMap(const isect&, const vec3f&, int density, int& x, int& y) const;

protected:
	bool capped;
};

#endif // __CYLINDER_H__
