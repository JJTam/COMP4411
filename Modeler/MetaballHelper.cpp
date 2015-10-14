#include "METABALL.h"
#include "CUBE_GRID.h"
#include "VECTOR3D.h"
#include "kumaModel.h"
#include <FL/gl.h>
#include <vector>

using namespace std;

void drawMetaball(int numMetaballs,const vector< vector<float> >& balls)
{
	const int minGridSize = 10;
	int gridSize = 30;
	float threshold = 1.0f;
	CUBE_GRID cubeGrid;

	METABALL* metaballs = new METABALL[numMetaballs];
	for (int i = 0; i<numMetaballs; i++)
		metaballs[i].Init(VECTOR3D(balls[i][0], balls[i][1], balls[i][2]), balls[i][3] * balls[i][3]);

	//init marching cube
	cubeGrid.CreateMemory();
	cubeGrid.Init(gridSize);

	for (int i = 0; i<cubeGrid.numVertices; i++)
	{
		cubeGrid.vertices[i].value = 0.0f;
		cubeGrid.vertices[i].normal.LoadZero();
	}
	VECTOR3D ballToPoint;
	float squaredRadius;
	VECTOR3D ballPosition;
	float normalScale;
	for (int i = 0; i<numMetaballs; i++)
	{
		squaredRadius = metaballs[i].squaredRadius;
		ballPosition = metaballs[i].position;

		//VC++6 standard does not inline functions
		//by inlining these maually, in this performance-critical area,
		//almost a 100% increase in speed is found
		for (int j = 0; j<cubeGrid.numVertices; j++)
		{
			//ballToPoint=cubeGrid.vertices[j].position-ballPosition;
			ballToPoint.x = cubeGrid.vertices[j].position.x - ballPosition.x;
			ballToPoint.y = cubeGrid.vertices[j].position.y - ballPosition.y;
			ballToPoint.z = cubeGrid.vertices[j].position.z - ballPosition.z;

			//get squared distance from ball to point
			//float squaredDistance=ballToPoint.GetSquaredLength();
			float squaredDistance = ballToPoint.x*ballToPoint.x +
				ballToPoint.y*ballToPoint.y +
				ballToPoint.z*ballToPoint.z;
			if (squaredDistance == 0.0f)
				squaredDistance = 0.0001f;

			//value = r^2/d^2
			cubeGrid.vertices[j].value += squaredRadius / squaredDistance;

			//normal = (r^2 * v)/d^4
			normalScale = squaredRadius / (squaredDistance*squaredDistance);
			//cubeGrid.vertices[j].normal+=ballToPoint*normalScale;
			cubeGrid.vertices[j].normal.x += ballToPoint.x*normalScale;
			cubeGrid.vertices[j].normal.y += ballToPoint.y*normalScale;
			cubeGrid.vertices[j].normal.z += ballToPoint.z*normalScale;
		}
	}
	//hints
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	//need to normalize!
	glEnable(GL_NORMALIZE);
	glEnable(GL_CULL_FACE);
	cubeGrid.DrawSurface(threshold);
	glDisable(GL_CULL_FACE);
	cubeGrid.FreeMemory();
	delete[] metaballs;
}