#include "modelerglobals.h"
#include "modelerapp.h"
#include <cmath>
#include <cstdio>

using namespace std;
#define ANGLE2RAIDUS_FACTOR 3.141592654 / 180

void kumaIK() {
	double targetX = VAL(IK_X);
	double targetY = VAL(IK_Y);
	double targetZ = VAL(IK_Z);

	double arX = VAL(LEFT_UPPER_ARM_ROTATION_X) * ANGLE2RAIDUS_FACTOR;
	double arY = VAL(LEFT_UPPER_ARM_ROTATION_Y) * ANGLE2RAIDUS_FACTOR;
	double arZ = VAL(LEFT_UPPER_ARM_ROTATION_Z) * ANGLE2RAIDUS_FACTOR;
	double brX = VAL(LEFT_LOWER_ARM_ROTATION_X) * ANGLE2RAIDUS_FACTOR;

	double lA = VAL(UPPER_ARM_HEIGHT);
	double Ax = lA * cos(arZ) * cos(arX);
	double Ay = lA * sin(arZ);
	double Az = lA * cos(arZ) * sin(arX);

	// printf("%.2f, %.2f, %.2f\n", Ax, Ay, Az);
}