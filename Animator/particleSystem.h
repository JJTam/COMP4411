/***********************
 * ParticleSystem class
 ***********************/

/**
 * The particle system class simply "manages" a collection of particles.
 * Its primary responsibility is to run the simulation, evolving particles
 * over time according to the applied forces using Euler's method.
 * This header file contains the functions that you are required to implement.
 * (i.e. the rest of the code relies on this interface)
 * In addition, there are a few suggested state variables included.
 * You should add to this class (and probably create new classes to model
 * particles and forces) to build your system.
 */

#ifndef __PARTICLE_SYSTEM_H__
#define __PARTICLE_SYSTEM_H__

#include "vec.h"
#include "particle.h"
#include <vector>
#include <list>
#include <random>

class ParticleSystem {

public:

	/** Constructor **/
	ParticleSystem(int genSpeed, int life, const Vec3f& size, const Vec3f& color, ParticleType type, int fps = 30);

	/** Destructor **/
	virtual ~ParticleSystem();

	/** Simulation fxns **/
	// This fxn should render all particles in the system,
	// at current time t.
	virtual void drawParticles(float t);

	// This fxn should save the configuration of all particles
	// at current time t.
	virtual void bakeParticles(float t);

	// This function should reset the system to its initial state.
	// When you need to reset your simulation, PLEASE USE THIS FXN.
	// It sets some state variables that the UI requires to properly
	// update the display.  Ditto for the following two functions.
	virtual void resetSimulation(float t);

	// This function should start the simulation
	virtual void startSimulation(float t);

	// This function should stop the simulation
	virtual void stopSimulation(float t);

	// This function should clear out your data structure
	// of baked particles (without leaking memory).
	virtual void clearBaked();	

	// These accessor fxns are implemented for you
	float getBakeStartTime() { return bake_start_time; }
	float getBakeEndTime() { return bake_end_time; }
	int getBakeFps() { return bake_fps; }
	bool isSimulate() { return simulate; }
	bool isDirty() { return dirty; }
	void setDirty(bool d) { dirty = d; }
	int getParticleLife() { return particleLife; }
	void setParticleLife(int l) { particleLife = l; }
	int getParticleGenerationSpeed() { return particleGenerationSpeed; }
	void setParticleGenerationSpeed(int s) { particleGenerationSpeed = s; }
	Vec3f getColor() { return particleColor; }
	void setColor(const Vec3f& c) { particleColor = c; }
	ParticleType getParticleType() { return particleType; }
	void setParticleType(ParticleType t) { particleType = t; }
	int getBakeFPS() { return bake_fps; }

protected:

	/** Some baking-related state **/
	int bake_fps;
	float spf;
	float bake_start_time;
	float bake_end_time;
	// store the baked particles
	std::vector< std::list<Particle>* > bakedParticles;

	/** General state variables **/
	bool simulate;						// flag for simulation mode
	bool dirty;							// flag for updating ui (don't worry about this)

	// the number of frames that a particle would survive
	int particleLife;
	// the number of new particles at each frame
	int particleGenerationSpeed;
	// size scale of particles
	Vec3f particleSize;
	// color of particles
	Vec3f particleColor;
	// type of particles
	ParticleType particleType;

	std::default_random_engine* rnd_generator;

	int bakeTimeToIndex(float t);
	virtual Particle generateNewParticle();
	// This function should compute forces acting on all particles
	// and update their state (pos and vel) appropriately.
	virtual void computeForcesAndUpdateParticles(int idx);
};


#endif	// __PARTICLE_SYSTEM_H__
