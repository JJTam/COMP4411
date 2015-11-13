#pragma warning(disable : 4786)

#include "particleSystem.h"
#include "particle.h"
#include "modelerdraw.h"
#include <FL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <random>
#include <chrono>

using namespace std;

/***************
 * Constructors
 ***************/

ParticleSystem::ParticleSystem(int genSpeed, int life, const Vec3f& size, const Vec3f& color, ParticleType type, int fps)
{
	simulate = false;

	particleLife = life;
	particleGenerationSpeed = genSpeed;
	particleColor = color;
	particleType = type;
	particleSize = size;

	if (fps <= 0)
		fps = 30;
	bake_fps = fps;
	spf = 1.0 / bake_fps;

	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	rnd_generator = new default_random_engine(seed);
}


/*************
 * Destructor
 *************/

ParticleSystem::~ParticleSystem() 
{
	clearBaked();
}


/******************
 * Simulation fxns
 ******************/

int ParticleSystem::bakeTimeToIndex(float t)
{
	return (t - bake_start_time) / spf;
}

/** Start the simulation */
void ParticleSystem::startSimulation(float t)
{
    
	bake_start_time = t;

	// These values are used by the UI ...
	// -ve bake_end_time indicates that simulation
	// is still progressing, and allows the
	// indicator window above the time slider
	// to correctly show the "baked" region
	// in grey.
	bake_end_time = -1;
	simulate = true;
	dirty = true;

}

/** Stop the simulation */
void ParticleSystem::stopSimulation(float t)
{
    
	bake_end_time = t;

	// These values are used by the UI
	simulate = false;
	dirty = true;

}

/** Reset the simulation */
void ParticleSystem::resetSimulation(float t)
{
    
	bake_start_time = bake_end_time = 0;

	// These values are used by the UI
	simulate = false;
	dirty = true;

}

/** Compute forces and update particles **/
void ParticleSystem::computeForcesAndUpdateParticles(int idx)
{
	// right now, gravity and air resistance only
	static float arfact = 0.1;

	list<Particle>* lst = bakedParticles[idx];
	for (auto iter = lst->begin(); iter != lst->end(); ++iter)
	{
		auto& p = *iter;
		Vec3f force(0, -9.8 * p.mass, 0);
		force -= prod(p.velocity, p.velocity) * arfact;

		p.acceleration = force / p.mass;
	}
}

/** Render particles */
void ParticleSystem::drawParticles(float t)
{
	if (t < bake_start_time || (bake_end_time >= bake_start_time && t > bake_end_time))
	{
		return;
	}

	bakeParticles(t);
	const list<Particle>* lst = bakedParticles[bakeTimeToIndex(t)];

	// based on the implementation of bakeParticles, this should never happen
	if (lst == nullptr)
	{
		printf("Warning: drawParticles get null list after calling bakeParticles! called t = %.2f, idx = %d\n", t, bakeTimeToIndex(t));
		return;
	}
	
	glPushMatrix();
	{
		for (auto iter = lst->cbegin(); iter != lst->cend(); ++iter)
		{
			glPushMatrix();
			{
				glTranslatef(iter->position[0], iter->position[1], iter->position[2]);
				glScalef(iter->sizes[0], iter->sizes[1], iter->sizes[2]);
				switch (iter->type)
				{
				case ParticleType::BOX:
					drawBox(1, 1, 1);
					break;
				case ParticleType::BALL:
					drawSphere(0.5);
				default:
					break;
				}
			}
			glPopMatrix();
		}
	}
	glPopMatrix();
}

/** Adds the current configuration of particles to
  * your data structure for storing baked particles **/
void ParticleSystem::bakeParticles(float t) 
{
	// printf("Baking time %.2f, start time %.2f, index %d.\n", t, bake_start_time, bakeTimeToIndex(t));
	int index = bakeTimeToIndex(t);
	int size = bakedParticles.size();
	
	// check if the time is already baked
	if (index < size && bakedParticles[index] != nullptr)
		return;

	// push the frames into buffer, and find the last available frame
	// push nullptr to handle frame skipping
	while (size++ <= index)
		bakedParticles.push_back(nullptr);
	int lastBaked = index;
	while (--lastBaked >= 0 && bakedParticles[lastBaked] == nullptr);


	if (lastBaked < 0)
	{
		list<Particle>* zeroLst = new list<Particle>;

		// construct the first frame
		for (int i = 0; i < particleGenerationSpeed; ++i)
		{
			zeroLst->push_back(generateNewParticle());
		}
		bakedParticles[0] = zeroLst;

		lastBaked = 0;
		if (index == 0)
			return;
	}

	// now construct a new list of particles based on the last frame
	const list<Particle>* lastLst = bakedParticles[lastBaked];
	list<Particle>* currLst = new list<Particle>;
	int frameDiff = index - lastBaked;
	float timeDiff = frameDiff * spf;
	computeForcesAndUpdateParticles(lastBaked);
	for (auto iter = lastLst->cbegin(); iter != lastLst->cend(); ++iter)
	{
		const Particle& p = *iter;
		if (p.life < 0)
			continue;

		Particle p2(p);
		p2.life -= frameDiff;
		
		p2.position += timeDiff * p2.velocity / 2;
		p2.velocity += timeDiff * p2.acceleration;
		p2.position += timeDiff * p2.velocity / 2;

		currLst->push_back(p2);
	}
	// generate new particles
	for (int i = 0; i < particleGenerationSpeed * frameDiff; ++i)
	{
		currLst->push_back(generateNewParticle());
	}
	bakedParticles[index] = currLst;
	// printf("Computed index %d, ref last index %d\n", index, lastBaked);
}

/** Clears out your data structure of baked particles */
void ParticleSystem::clearBaked()
{
	for (auto iter = bakedParticles.begin(); iter != bakedParticles.end(); ++iter)
	{
		if (*iter)
			delete *iter;
	}

	bakedParticles.clear();
}


Particle ParticleSystem::generateNewParticle()
{
	ParticleType type = ParticleType::BOX;
	Vec3f size(particleSize[0], particleSize[1], particleSize[2]);
	Vec3f initPos(0, 0, 0);
	uniform_real_distribution<double> dist(-0.5, 0.5);
	Vec3f initSpeed(0 + dist(*rnd_generator), 0.5 + dist(*rnd_generator), 3 + dist(*rnd_generator));
	Vec3f initAcc(0, 0, 0);
	return Particle(type, 1, particleLife, size, initPos, initSpeed, initAcc);
}


