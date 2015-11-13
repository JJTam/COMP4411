#include "particle.h"

Particle::Particle(ParticleType t, double m, int l, const Vec3f& s, const Vec3f& p, const Vec3f& v, const Vec3f& a)
{
	this->type = t;
	this->mass = m;
	this->life = l;
	this->sizes = s;
	this->position = p;
	this->velocity = v;
	this->acceleration = a;
}