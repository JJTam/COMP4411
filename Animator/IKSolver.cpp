#include <vector>
#include "mat.h"
#include "vec.h"

using namespace std;

bool stepperNext(int numVals, int* stepper)
{
	bool hasNext = false;
	for (int i = 0; i < numVals; ++i)
	{
		if (stepper[i] < 2)
		{
			stepper[i] += 1;
			hasNext = true;
			break;
		}
		else
		{
			stepper[i] = 0;
		}
	}

	return hasNext;
}

void solveIK(int maxIter, double step, Vec3f target, int numVals, double* vals, double* constrains, void transform(const double* in, Vec3f& out))
{
	int* stepper = new int[numVals];
	int* localsol = new int[numVals];
	double* globalsol = new double[numVals];
	memcpy(globalsol, vals, sizeof(double) * numVals);
	double* currvals = new double[numVals];
	double diff[3] = { -step, 0, step };
	double globalBest = 1e32;
	Vec3f outpos;

	for (int iteration = 0; iteration < maxIter; ++iteration)
	{
		// init stepper
		for (int i = 0; i < numVals; ++i)
			stepper[i] = 0;
		
		double localBest = globalBest;
		bool hasUpdate = false;
		do
		{
			bool constrainFail = false;
			for (int i = 0; i < numVals; ++i)
			{
				currvals[i] = globalsol[i] + diff[stepper[i]];
				if (currvals[i] < constrains[2 * i] || currvals[i] > constrains[2 * i + 1])
				{
					constrainFail = true;
					break;
				}
			}
			if (constrainFail)
				continue;

			transform(currvals, outpos);
			double currdiff = (target - outpos).length2();
			if (currdiff < localBest)
			{
				localBest = currdiff;
				memcpy(localsol, stepper, sizeof(int) * numVals);
				hasUpdate = true;
			}
		} while (stepperNext(numVals, stepper));

		if (hasUpdate)
		{
			globalBest = localBest;
			for (int i = 0; i < numVals; ++i)
				globalsol[i] = globalsol[i] + diff[localsol[i]];
		}
	}
	memcpy(vals, globalsol, sizeof(double) * numVals);

	delete[] stepper;
	delete[] localsol;
	delete[] globalsol;
	delete[] currvals;
}