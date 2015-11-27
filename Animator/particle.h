#ifndef _PARTICLE_H_
#define _PARTICLE_H_

#include "vec.h"
#include "FL/gl.h"

enum class ParticleType
{
	BOX,
	BALL
};

class Particle
{
public:
	Particle(ParticleType type, double mass, int life, const Vec3f& sizes, const Vec3f& position, const Vec3f& velocity, const Vec3f& acceleration, GLfloat* initMat);
	Particle::Particle(const Particle& other);
	~Particle();

	ParticleType type;
	int life;
	Vec3f sizes;
	Vec3f velocity;
	Vec3f acceleration;
	Vec3f position;
	GLfloat* initMat;
	double mass;
};

#endif // _PARTICLE_H_