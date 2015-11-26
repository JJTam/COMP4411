#include "vec.h"
#include "mat.h"
#include <vector>
using namespace std;
Vec3f calculateBSplineSurfacePoint(double u, double v, const vector<Vec3f>& ctrlpts)
{
	Vec4f U(u*u*u, u*u, u, 1);
	Vec4f V(v*v*v, v*v, v, 1);
	Mat4f M(-1, 3, -3, 1,
		3, -6, 3, 0,
		-3, 0, 3, 0,
		1, 4, 1, 0);
	Mat4f Gx(ctrlpts[0][0], ctrlpts[1][0], ctrlpts[2][0], ctrlpts[3][0],
		ctrlpts[4][0], ctrlpts[5][0], ctrlpts[6][0], ctrlpts[7][0],
		ctrlpts[8][0], ctrlpts[9][0], ctrlpts[10][0], ctrlpts[11][0],
		ctrlpts[12][0], ctrlpts[13][0], ctrlpts[14][0], ctrlpts[15][0]);
	Mat4f Gy(ctrlpts[0][1], ctrlpts[1][1], ctrlpts[2][1], ctrlpts[3][1],
		ctrlpts[4][1], ctrlpts[5][1], ctrlpts[6][1], ctrlpts[7][1],
		ctrlpts[8][1], ctrlpts[9][1], ctrlpts[10][1], ctrlpts[11][1],
		ctrlpts[12][1], ctrlpts[13][1], ctrlpts[14][1], ctrlpts[15][1]);
	Mat4f Gz(ctrlpts[0][2], ctrlpts[1][2], ctrlpts[2][2], ctrlpts[3][2],
		ctrlpts[4][2], ctrlpts[5][2], ctrlpts[6][2], ctrlpts[7][2],
		ctrlpts[8][2], ctrlpts[9][2], ctrlpts[10][2], ctrlpts[11][2],
		ctrlpts[12][2], ctrlpts[13][2], ctrlpts[14][2], ctrlpts[15][2]);

	Vec3f result;
	result[0] = U * (M * Gx * M.transpose() * V)/36;
	result[1] = U * (M * Gy * M.transpose() * V)/36;
	result[2] = U * (M * Gz * M.transpose() * V)/36;

	return result;
}