#include "particle.h"
#include "FL/gl.h"
Particle::Particle(ParticleType t, double m, int l, const Vec3f& s, const Vec3f& p, const Vec3f& v, const Vec3f& a, GLfloat* initM)
{
	this->type = t;
	this->mass = m;
	this->life = l;
	this->sizes = s;
	this->position = p;
	this->velocity = v;
	this->acceleration = a;
	this->initMat = initM;
}

Particle::Particle(const Particle& other) : Particle(other.type, other.mass, other.life, other.sizes, other.position, other.velocity, other.acceleration, nullptr)
{
	this->initMat = new GLfloat[16];
	for (int i = 0; i < 16; ++i)
		this->initMat[i] = other.initMat[i];
}

Particle::~Particle()
{
	delete[] initMat;
}