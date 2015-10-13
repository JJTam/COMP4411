#include "modelerglobals.h"
#include "modelerapp.h"
#include <cmath>
#include <cstdio>
#include <FL/gl.h>
#include "modelerdraw.h"
#include "mat.h"

using namespace std;
#define ANGLE2RAIDUS_FACTOR (3.141592654 / 180)

extern void displayGLModelviewMatrix();

void kumaIK() {

	double targetX = -((VAL(UPPER_ARM_WIDTH) + VAL(UPPER_ARM_BODY_OFFSET_X)) + VAL(TORSO_WIDTH) / 2 + VAL(IK_X));
	double targetY = (VAL(LOWER_LEG_HEIGHT) + VAL(UPPER_LEG_HEIGHT) + VAL(WAIST_HEIGHT) + VAL(WAIST_TORSO_OFFSET) + VAL(UPPER_LEG_OFFSET_WAIST)) + VAL(TORSO_HEIGHT) - VAL(UPPER_ARM_BODY_OFFSET_Y) - VAL(IK_Y);
	double targetZ = -VAL(IK_Z);

	double arX = VAL(LEFT_UPPER_ARM_ROTATION_X) * ANGLE2RAIDUS_FACTOR;
	double arY = VAL(LEFT_UPPER_ARM_ROTATION_Y) * ANGLE2RAIDUS_FACTOR;
	double arZ = VAL(LEFT_UPPER_ARM_ROTATION_Z) * ANGLE2RAIDUS_FACTOR;
	double brX = VAL(LEFT_LOWER_ARM_ROTATION_X) * ANGLE2RAIDUS_FACTOR;

	double lA = VAL(UPPER_ARM_HEIGHT);
	double lB = VAL(LOWER_ARM_HEIGHT);

	Mat4f m1(1, 0, 0, 0, 
		0, cos(arX), -sin(arX), 0, 
		0, sin(arX), cos(arX), 0, 
		0, 0, 0, 1); 
	Mat4f m2(cos(arZ), -sin(arZ), 0, 0, 
		sin(arZ), cos(arZ), 0, 0, 
		0, 0, 1, 0, 
		0, 0, 0, 1);
	Mat4f m3(cos(arY), 0, sin(arY), 0, 
		0, 1, 0, 0, 
		-sin(arY), 0, cos(arY), 0, 
		0, 0, 0, 1);
	Mat4f m4(1, 0, 0, 0, 
		0, 1, 0, lA, 
		0, 0, 1, 0, 
		0, 0, 0, 1);
	Mat4f m5(1, 0, 0, 0, 
		0, cos(brX), -sin(brX), 0, 
		0, sin(brX), cos(brX), 0, 
		0, 0, 0, 1);
	Mat4f m6(1, 0, 0, 0, 
		0, 1, 0, lB, 
		0, 0, 1, 0, 
		0, 0, 0, 1);


	Mat4f m7 = m1 * (m2 * (m3 * (m4 * (m5 * m6))));
	double Bx = m7[0][3];
	double By = m7[1][3];
	double Bz = m7[2][3];

	
	double dist = (Bx - targetX) * (Bx - targetX) + (By - targetY) * (By - targetY) + (Bz - targetZ) * (Bz - targetZ);
	double minDist = dist;
	
	double bestRes[4];
	bestRes[0] = arX;
	bestRes[1] = arY;
	bestRes[2] = arZ;
	bestRes[3] = brX;
	double localBest[4];
	localBest[0] = arX;
	localBest[1] = arY;
	localBest[2] = arZ;
	localBest[3] = brX;
	int localSigns[4];
	localSigns[0] = localSigns[1] = localSigns[2] = localSigns[3] = 0;

	double step = 0.01;

	double tempDist = dist;

#define UPDATE_MATRIX m3[0][0] = cos(arY);m3[0][2] = sin(arY);m3[2][0] = -sin(arY);m3[2][2] = cos(arY);m1[1][1] = cos(arX);m1[1][2] = -sin(arX);m1[2][1] = sin(arX);m1[2][2] = cos(arX);m2[0][0] = cos(arZ);m2[0][1] = -sin(arZ);m2[1][0] = sin(arZ);m2[1][1] = cos(arZ);m5[1][1] = cos(brX);m5[1][2] = -sin(brX);m5[2][1] = sin(brX);m5[2][2] = cos(brX);
#define UPDATE_M7_AND_DIST m7 = m1 * (m2 * (m3 * (m4 * (m5 * m6)))); Bx = m7[0][3]; By = m7[1][3]; Bz = m7[2][3]; dist = (Bx - targetX) * (Bx - targetX) + (By - targetY) * (By - targetY) + (Bz - targetZ) * (Bz - targetZ);
#define UPDATE_LOCAL UPDATE_M7_AND_DIST; if (dist < localMin) { hasUpdate = true;localMin = dist; localBest[0] = arX; localBest[1] = arY; localBest[2] = arZ; localBest[3] = brX; }
#define RUN_UPDATES if (brX < 0 && arY < 1.5708 && arY > -1.5708 && arZ < 0 && arZ > -3.14159) { UPDATE_MATRIX; UPDATE_LOCAL; }

	for (int i = 0; i < 400; ++i)
	{
		double localMin = minDist;
		bool hasUpdate = false;

		if (VAL(IK_FAST) > 0)
		{
			localSigns[0] = localSigns[1] = localSigns[2] = localSigns[3] = 0;

			brX = bestRes[3] + step;
			if (brX < 0 && brX > -3.1415926)
			{
				m5[1][1] = cos(brX); m5[1][2] = -sin(brX); m5[2][1] = sin(brX); m5[2][2] = cos(brX);
				UPDATE_M7_AND_DIST;
				if (dist < localMin)
				{
					localSigns[3] = 1;
				}
			}
			if (localSigns[3] == 0)
			{
				brX = bestRes[3] - step;
				if (brX < 0 && brX > -3.1415926)
				{
					m5[1][1] = cos(brX); m5[1][2] = -sin(brX); m5[2][1] = sin(brX); m5[2][2] = cos(brX);
					UPDATE_M7_AND_DIST;
					if (dist < localMin)
					{
						localSigns[3] = -1;
					}
				}
			}

			arX = bestRes[0] + step;
			if (arX < 3.1415926 && arX > -3.1415926)
			{
				m1[1][1] = cos(arX); m1[1][2] = -sin(arX); m1[2][1] = sin(arX); m1[2][2] = cos(arX);
				UPDATE_M7_AND_DIST;
				if (dist < localMin)
				{
					localSigns[0] = 1;
				}
			}
			if (localSigns[0] == 0)
			{
				arX = bestRes[0] - step;
				if (arX < 3.1415926 && arX > -3.1415926)
				{
					m1[1][1] = cos(arX); m1[1][2] = -sin(arX); m1[2][1] = sin(arX); m1[2][2] = cos(arX);
					UPDATE_M7_AND_DIST;
					if (dist < localMin)
					{
						localSigns[0] = -1;
					}
				}
			}

			arZ = bestRes[2] + step;
			if (arZ < 0 && arZ > -3.14159)
			{
				m2[0][0] = cos(arZ); m2[0][1] = -sin(arZ); m2[1][0] = sin(arZ); m2[1][1] = cos(arZ);
				UPDATE_M7_AND_DIST;
				if (dist < localMin)
				{
					localSigns[2] = 1;
				}
			}
			if (localSigns[2] == 0)
			{
				arZ = bestRes[2] - step;
				if (arZ < 0 && arZ > -3.14159)
				{
					m2[0][0] = cos(arZ); m2[0][1] = -sin(arZ); m2[1][0] = sin(arZ); m2[1][1] = cos(arZ);
					UPDATE_M7_AND_DIST;
					if (dist < localMin)
					{
						localSigns[2] = -1;
					}
				}
			}

			arY = bestRes[1] + step;
			if (arY < 1.5708 && arY > -1.5708)
			{
				m3[0][0] = cos(arY); m3[0][2] = sin(arY); m3[2][0] = -sin(arY); m3[2][2] = cos(arY);
				UPDATE_M7_AND_DIST;
				if (dist < localMin)
				{
					localSigns[1] = 1;
				}
			}
			if (localSigns[1] == 0)
			{
				arY = bestRes[1] - step;
				if (arY < 1.5708 && arY > -1.5708)
				{
					m3[0][0] = cos(arY); m3[0][2] = sin(arY); m3[2][0] = -sin(arY); m3[2][2] = cos(arY);
					UPDATE_M7_AND_DIST;
					if (dist < localMin)
					{
						localSigns[1] = -1;
					}
				}
			}

			if (localSigns[0] != 0 || localSigns[1] != 0 || localSigns[2] != 0 || localSigns[3] != 0)
			{
				localBest[0] += localSigns[0] * step;
				localBest[1] += localSigns[1] * step;
				localBest[2] += localSigns[2] * step;
				localBest[3] += localSigns[3] * step;
				arX = localBest[0];
				arY = localBest[1];
				arZ = localBest[2];
				brX = localBest[3];
				UPDATE_MATRIX;
				UPDATE_M7_AND_DIST;
				localMin = dist;
				hasUpdate = true;
			}
		}
		else
		{
			for (int dArY = -1; dArY <= 1; ++dArY)
			{
				arY = bestRes[1] + dArY * step;
				if (arY > 1.5708 || arY < -1.5708)
					continue;
				m3[0][0] = cos(arY); m3[0][2] = sin(arY); m3[2][0] = -sin(arY); m3[2][2] = cos(arY);
				for (int dArZ = -1; dArZ <= 1; ++dArZ)
				{
					arZ = bestRes[2] + dArZ * step;
					if (arZ > 0 || arZ < -3.14159)
						continue;
					m2[0][0] = cos(arZ); m2[0][1] = -sin(arZ); m2[1][0] = sin(arZ); m2[1][1] = cos(arZ);
					for (int dArX = -1; dArX <= 1; ++dArX)
					{
						arX = bestRes[0] + dArX * step;
						m1[1][1] = cos(arX); m1[1][2] = -sin(arX); m1[2][1] = sin(arX); m1[2][2] = cos(arX);
						for (int dBrX = -1; dBrX <= 1; ++dBrX)
						{
							brX = bestRes[3] + dBrX * step;
							if (brX > 0 || brX < -3.1415926)
								continue;
							m5[1][1] = cos(brX); m5[1][2] = -sin(brX); m5[2][1] = sin(brX); m5[2][2] = cos(brX);

							UPDATE_LOCAL;
						}
					}
				}
			}
		}

		if (hasUpdate)
		{
			minDist = localMin;

			// an extra test to avoid Y-dir shaking
			arY = bestRes[1];
			UPDATE_MATRIX;
			m7 = m1 * (m2 * (m3 * (m4 * (m5 * m6)))); Bx = m7[0][3]; By = m7[1][3]; Bz = m7[2][3]; dist = (Bx - targetX) * (Bx - targetX) + (By - targetY) * (By - targetY) + (Bz - targetZ) * (Bz - targetZ);
			if (dist - localMin < 0.001)
			{
				localMin = dist;
				minDist = dist;
				localBest[1] = arY;
			}

			bestRes[0] = localBest[0];
			bestRes[1] = localBest[1];
			bestRes[2] = localBest[2];
			bestRes[3] = localBest[3];
		}
		if (!hasUpdate || dist < 0.005)
			break;
	}
	SETVAL(LEFT_UPPER_ARM_ROTATION_X, bestRes[0] / ANGLE2RAIDUS_FACTOR);
	SETVAL(LEFT_UPPER_ARM_ROTATION_Y, bestRes[1] / ANGLE2RAIDUS_FACTOR);
	SETVAL(LEFT_UPPER_ARM_ROTATION_Z, bestRes[2] / ANGLE2RAIDUS_FACTOR);
	SETVAL(LEFT_LOWER_ARM_ROTATION_X, bestRes[3] / ANGLE2RAIDUS_FACTOR);
	//printf("%.2f, %.2f, %.2f\n", Bx, By, Bz);
	//printf("%.2f, %.2f, %.2f\n", targetX, targetY, targetZ);
}