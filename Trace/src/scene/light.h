#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "scene.h"

class Light
	: public SceneElement
{
public:
	virtual vec3f shadowAttenuation(const vec3f& P) const = 0;
	virtual double distanceAttenuation( const vec3f& P ) const = 0;
	virtual vec3f getColor( const vec3f& P ) const = 0;
	virtual vec3f getDirection( const vec3f& P ) const = 0;
	virtual vec3f getAmbientColor(const vec3f& P) const = 0;


protected:
	Light( Scene *scene, const vec3f& col )
		: SceneElement( scene ), color( col ) {}

	vec3f 		color;
};

class DirectionalLight
	: public Light
{
public:
	DirectionalLight( Scene *scene, const vec3f& orien, const vec3f& color )
		: Light( scene, color ), orientation( orien ) {}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation( const vec3f& P ) const;
	virtual vec3f getColor( const vec3f& P ) const;
	virtual vec3f getDirection( const vec3f& P ) const;
	virtual vec3f getAmbientColor(const vec3f& P) const;


protected:
	vec3f 		orientation;
};

class PointLight
	: public Light
{
public:
	PointLight( Scene *scene, const vec3f& pos, const vec3f& color, double a,double b,double c)
		: Light(scene, color), position(pos), constant_attenuation_coeff(a), linear_attenuation_coeff(b), quadratic_attenuation_coeff(c){}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation( const vec3f& P ) const;
	virtual vec3f getColor( const vec3f& P ) const;
	virtual vec3f getDirection( const vec3f& P ) const;
	virtual vec3f getAmbientColor(const vec3f& P) const;

	double constant_attenuation_coeff;
	double linear_attenuation_coeff;
	double quadratic_attenuation_coeff;


protected:
	vec3f position;
};

class AmbientLight
	: public Light
{
public:
	AmbientLight(Scene *scene, const vec3f& color)
		: Light(scene, color) {}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation(const vec3f& P) const;
	virtual vec3f getDirection(const vec3f& P) const;
	virtual vec3f getColor(const vec3f& P) const;
	virtual vec3f getAmbientColor(const vec3f& P) const;


};

class SpotLight
	: public Light
{
public:
	SpotLight(Scene *scene, const vec3f& color, const vec3f& pos, const vec3f& dir, int coefficient, const vec3f& cone_dir,double con_angle)
		: Light(scene, color), pos(pos), dir(dir), coefficient(coefficient), cone_dir(cone_dir),con_angle(con_angle){}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation(const vec3f& P) const;
	virtual vec3f getDirection(const vec3f& P) const;
	virtual vec3f getColor(const vec3f& P) const;
	virtual vec3f getAmbientColor(const vec3f& P) const;


	vec3f pos;
	vec3f dir;
	int coefficient;
	vec3f cone_dir;
	double con_angle;
};

#endif // __LIGHT_H__
