// The main ray tracer.

#include <Fl/fl_ask.h>

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "fileio/read.h"
#include "fileio/parse.h"
#include <random>
#include "ui/TraceUI.h"

bool distReflection;
bool distRefraction;
double distSize;
int distRays;
double distStep;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
vec3f RayTracer::trace( Scene *scene, double x, double y )
{
	int supersampling = m_pUI->m_nSupersampling;
	int jitter_grid_numbers = m_pUI->m_nJitter;

	if (supersampling != 0)
	{
		double lower_bound = 0.0;
		double upper_bound = 1.0*jitter_grid_numbers;
		std::uniform_real_distribution<double> rand(lower_bound, upper_bound);
		static std::default_random_engine re;
		vec3f sum;
		for (int i = 0; i < supersampling; ++i)
		{
			for (int j = 0; j < supersampling; ++j)
			{
				ray r(vec3f(0, 0, 0), vec3f(0, 0, 0));
				double offsetX = ((i + 0.5 + (rand(re) - upper_bound / 2)) / supersampling - 0.5) / buffer_width;
				double offsetY = ((j + 0.5 + (rand(re) - upper_bound / 2)) / supersampling - 0.5) / buffer_height;
				scene->getCamera()->rayThrough(x + offsetX, y + offsetY, r);
				sum += traceRay(scene, r, vec3f(1.0, 1.0, 1.0), 0).clamp();
			}
		}
		sum /= supersampling*supersampling;
		return sum;
	}
	else
	{
		ray r(vec3f(0, 0, 0), vec3f(0, 0, 0));
		scene->getCamera()->rayThrough(x, y, r);
		return traceRay(scene, r, vec3f(1.0, 1.0, 1.0), 0).clamp();
	}
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
vec3f RayTracer::traceRay(Scene *scene, const ray& r,
	const vec3f& thresh, int depth, bool isInSpace, double intensity)
{
	isect i;
	 
	if( scene->intersect( r, i ) ) {

		const Material& m = i.getMaterial();
		vec3f result = m.shade(scene, r, i);

		vec3f ones(1.0, 1.0, 1.0);
		vec3f ktInv = ones - m.kt;
		result = prod(ktInv, result);

		if (depth < m_pUI->getDepth() && (m_pUI->m_thresholdSlider->value() == 0 || intensity > m_pUI->m_thresholdSlider->value()))
		{
			vec3f nextRayPos = r.getPosition() + r.getDirection() * i.t;

			// if m is reflective
			if (m.kr[0] > 0 || m.kr[1] > 0 || m.kr[2] > 0)
			{
				
				vec3f reflectDir = (2 * ((-r.getDirection()) * i.N) * i.N - (-r.getDirection())).normalize();

				// distributed reflection
				if (!distReflection)
				{
					ray nextRay(nextRayPos, reflectDir);
					vec3f nextResult = traceRay(scene, nextRay, thresh, depth + 1, isInSpace,m.ks.length()*intensity);

					result += prod(m.ks, nextResult);
				}
				else
				{
					double theta1 = -distSize / 2;
					vec4f dirv4(reflectDir[0], reflectDir[1], reflectDir[2], 1);
					vec3f axis1(-reflectDir[0], 0, reflectDir[2]);
					vec3f axis2(0, 1, 0);
					vec3f ksScaled = m.ks / (distRays * distRays);
					for (int i1 = 0; i1 < distRays; ++i1)
					{
						mat4f rot1 = mat4f::rotate(axis1, theta1);
						double theta2 = -distSize / 2;
						for (int i2 = 0; i2 < distRays; ++i2)
						{
							mat4f rot2 = mat4f::rotate(axis2, theta2);
							vec4f newDirv4 = rot1 * (rot2 * dirv4);

							ray nextRay(nextRayPos, newDirv4);
							vec3f nextResult = traceRay(scene, nextRay, thresh, depth + 1, isInSpace, ksScaled.length()*intensity);

							result += prod(ksScaled, nextResult);

							theta2 += distStep;
						}
						theta1 += distStep;
					}
				}
			}

			// if m is transmissive
			if (m.kt[0] > 0 || m.kt[1] > 0 || m.kt[2] > 0)
			{
				if (i.getMaterial().index != 1.0)
				{
					double indexRatio = isInSpace ? 1.0 / i.getMaterial().index : i.getMaterial().index;
					double NI = -r.getDirection() * i.N;
					double cosThetaTsq = 1 - indexRatio * indexRatio * (1 - NI * NI);
					if (cosThetaTsq >= 0)
					{
						vec3f refractDir = (indexRatio * NI - sqrt(cosThetaTsq)) * i.N - indexRatio * -r.getDirection();
						refractDir = refractDir.normalize();

						if (!distRefraction)
						{
							ray nextRay(nextRayPos, refractDir);
							vec3f nextResult = traceRay(scene, nextRay, thresh, depth + 1, !isInSpace, m.kt.length()*intensity);
							result += prod(m.kt, nextResult);
						}
						else
						{
							double theta1 = -distSize / 2;
							vec4f dirv4(refractDir[0], refractDir[1], refractDir[2], 1);
							vec3f axis1(-refractDir[0], 0, refractDir[2]);
							vec3f axis2(0, 1, 0);
							vec3f ktScaled = m.kt / (distRays * distRays);
							for (int i1 = 0; i1 < distRays; ++i1)
							{
								mat4f rot1 = mat4f::rotate(axis1, theta1);
								double theta2 = -distSize / 2;
								for (int i2 = 0; i2 < distRays; ++i2)
								{
									mat4f rot2 = mat4f::rotate(axis2, theta2);
									vec4f newDirv4 = rot1 * (rot2 * dirv4);
									vec3f newDirv3(newDirv4[0], newDirv4[1], newDirv4[2]);
									if (newDirv4[3] != 0)
										newDirv3 = newDirv3 / newDirv4[3];
									ray nextRay(nextRayPos, newDirv3);
									vec3f nextResult = traceRay(scene, nextRay, thresh, depth + 1, !isInSpace, ktScaled.length()*intensity);

									result += prod(ktScaled, nextResult);

									theta2 += distStep;
								}
								theta1 += distStep;
							}
						}
					}
				}
				else
				{
					ray nextRay(r.getPosition() + r.getDirection() * i.t, r.getDirection());
					vec3f nextResult = traceRay(scene, nextRay, thresh, depth + 1, !isInSpace, m.kt.length()*intensity);
					result += prod(m.kt, nextResult);
				}
			}
		}

		return result;
	} else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.

		return vec3f( 0.0, 0.0, 0.0 );
	}
}

RayTracer::RayTracer()
{
	buffer = NULL;
	buffer_width = buffer_height = 256;
	scene = NULL;
	all = NULL;

	m_bSceneLoaded = false;
}


RayTracer::~RayTracer()
{
	delete [] buffer;
	delete scene;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return scene ? scene->getCamera()->getAspectRatio() : 1;
}

bool RayTracer::sceneLoaded()
{
	return m_bSceneLoaded;
}

bool RayTracer::loadScene( char* fn )
{
	try
	{
		scene = readScene( fn );
	}
	catch( ParseError pe )
	{
		fl_alert( "ParseError: %s\n", pe );
		return false;
	}

	if( !scene )
		return false;
	
	buffer_width = 256;
	buffer_height = (int)(buffer_width / scene->getCamera()->getAspectRatio() + 0.5);

	bufferSize = buffer_width * buffer_height * 3;
	buffer = new unsigned char[ bufferSize ];
	
	// separate objects into bounded and unbounded
	scene->initScene();
	
	// Add any specialized scene loading code here
	
	m_bSceneLoaded = true;

	return true;
}

void RayTracer::traceSetup( int w, int h )
{
	if( buffer_width != w || buffer_height != h )
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete [] buffer;
		buffer = new unsigned char[ bufferSize ];
	}
	memset( buffer, 0, w*h*3 );
}

void RayTracer::traceLines( int start, int stop )
{
	vec3f col;
	if( !scene )
		return;

	if( stop > buffer_height )
		stop = buffer_height;

	for( int j = start; j < stop; ++j )
		for( int i = 0; i < buffer_width; ++i )
			tracePixel(i,j);
}

vec3f RayTracer::trace(Scene* scene, double x, double y, int depth)
{
	double x1 = (x - 0.5) / buffer_width;
	double x2 = (x - 0.5 + 1.0 / pow(2, depth)) / buffer_width;
	double y1 = (y - 0.5) / buffer_height;
	double y2 = (y - 0.5 + 1.0 / pow(2, depth)) / buffer_height;

	ray r1(vec3f(0, 0, 0), vec3f(0, 0, 0));
	scene->getCamera()->rayThrough(x1, y1 ,r1);
	vec3f tmp1 = traceRay(scene, r1, vec3f(1.0, 1.0, 1.0), 0).clamp();
	ray r2(vec3f(0, 0, 0), vec3f(0, 0, 0));
	scene->getCamera()->rayThrough(x2, y1, r2);
	vec3f tmp2 = traceRay(scene, r2, vec3f(1.0, 1.0, 1.0), 0).clamp();
	ray r3(vec3f(0, 0, 0), vec3f(0, 0, 0));
	scene->getCamera()->rayThrough(x1, y2, r3);
	vec3f tmp3 = traceRay(scene, r3, vec3f(1.0, 1.0, 1.0), 0).clamp();
	ray r4(vec3f(0, 0, 0), vec3f(0, 0, 0));
	scene->getCamera()->rayThrough(x2, y2, r4);
	vec3f tmp4 = traceRay(scene, r4, vec3f(1.0, 1.0, 1.0), 0).clamp();

	vec3f avg = (tmp1 + tmp2 + tmp3 + tmp4) / 4;
	if (depth < m_pUI->m_nAdaptiveDepth)
	{
		if ((tmp1 - avg).length() > 1.0e-4 || (tmp2 - avg).length() > 1.0e-4 || (tmp3 - avg).length() > 1.0e-4 || (tmp4 - avg).length() > 1.0e-4)
		{
			vec3f sum = trace(scene, x, y, depth + 1) + trace(scene, x + 1 / pow(2, depth + 1), y, depth + 1) +
				trace(scene, x, y + 1 / pow(2, depth + 1), depth + 1) + trace(scene, x + 1 / pow(2, depth + 1), y + 1 / pow(2, depth + 1), depth + 1);
			return sum / 4;
		}
	}
	return avg;
}

void RayTracer::tracePixel( int i, int j )
{
	vec3f col;


	if( !scene )
		return;

	distReflection = m_pUI->m_distReflSlider->value() > 0;
	distRefraction = m_pUI->m_distRefrSlider->value() > 0;
	distSize = m_pUI->m_distSizeSlider->value();
	distRays = m_pUI->m_distRaysSlider->value();
	distStep = distSize / (distRays - 1);

	if (m_pUI->m_nAdaptiveDepth>0)
	{
		if (i == 0 && j == 0)
		{
			calculateAll();
		}
		//caculate avg;
		vec3f avg = all[i + j*(buffer_width + 1)] + all[i + 1 + j*(buffer_width + 1)] + all[i + (j + 1)*(buffer_width + 1)] + all[i + 1 + (j + 1)*(buffer_width + 1)];
		avg = avg / 4;
		if ((avg - all[i + j*(buffer_width + 1)]).length() > 1.0e-4 || (avg - all[i + 1 + j*(buffer_width + 1)]).length() > 1.0e-4
			|| (avg - all[i + (j + 1)*(buffer_width + 1)]).length() > 1.0e-4 || (avg - all[(i + 1) + (j + 1)*(buffer_width + 1)]).length() > 1.0e-4)
		{
			col = trace(scene, i, j, 1) + trace(scene, i+0.5, j, 1) + trace(scene, i, j+0.5, 1) + trace(scene, i+0.5, j+0.5, 1);
			col = col / 4;
		}
		else col = avg;

	}
	else
	{
		double x = double(i) / double(buffer_width);
		double y = double(j) / double(buffer_height);

		col = trace(scene, x, y);
	}
	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}

void RayTracer::calculateAll()
{
	if (all != NULL)
	{
		delete[] all;
	}

	all = new vec3f[(buffer_height+1)*(buffer_width+1)];
	for (int y = 0; y < buffer_height+1; ++y)
	{
		for (int x = 0; x < buffer_width+1; ++x)
		{
			ray r(vec3f(0, 0, 0), vec3f(0, 0, 0));
			scene->getCamera()->rayThrough((x-0.5)/buffer_width, (y-0.5)/buffer_height, r);
			all[x + y*(buffer_width+1)] = traceRay(scene, r, vec3f(1.0, 1.0, 1.0), 0).clamp();
		}
	}

}