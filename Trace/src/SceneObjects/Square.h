#ifndef __SQUARE_H__
#define __SQUARE_H__

#include "../scene/scene.h"

class Square
	: public MaterialSceneObject
{
public:
	Square( Scene *scene, Material *mat )
		: MaterialSceneObject( scene, mat )
	{
	}

	virtual bool intersectLocal( const ray& r, isect& i ) const;
	virtual bool hasBoundingBoxCapability() const { return true; }

    virtual BoundingBox ComputeLocalBoundingBox() const
    {
        BoundingBox localbounds;
        localbounds.min = vec3f(-0.5f, -0.5f, -RAY_EPSILON);
		localbounds.max = vec3f(0.5f, 0.5f, RAY_EPSILON);
        return localbounds;
    }

	virtual bool supports2DMap() const { return true; }
	virtual void isectTo2DMap(const isect&, const vec3f&, int density, int& x, int& y) const;
};

#endif // __SQUARE_H__
