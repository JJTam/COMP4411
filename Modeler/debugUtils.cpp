#include <FL/gl.h>
#include <gl/GLU.h>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

void displayGLModelviewMatrix();
void displayModelviewMatrix(float[16]);

void displayGLModelviewMatrix() {
	static GLfloat m[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, m);
	displayModelviewMatrix(m);
}

void displayModelviewMatrix(float MV[16]) {

	static const int Rx = 0;
	static const int Ry = 1;
	static const int Rz = 2;

	static const int Ux = 4;
	static const int Uy = 5;
	static const int Uz = 6;

	static const int Ax = 8;
	static const int Ay = 9;
	static const int Az = 10;

	static const int Tx = 12;
	static const int Ty = 13;
	static const int Tz = 14;

	int SPACING = 12;
	cout << left;
	cout << "\tMODELVIEW MATRIX\n";
	cout << "--------------------------------------------------" << endl;
	cout << setw(SPACING) << "R" << setw(SPACING) << "U" << setw(SPACING) << "A" << setw(SPACING) << "T" << endl;
	cout << "--------------------------------------------------" << endl;
	cout << setw(SPACING) << MV[Rx] << setw(SPACING) << MV[Ux] << setw(SPACING) << MV[Ax] << setw(SPACING) << MV[Tx] << endl;
	cout << setw(SPACING) << MV[Ry] << setw(SPACING) << MV[Uy] << setw(SPACING) << MV[Ay] << setw(SPACING) << MV[Ty] << endl;
	cout << setw(SPACING) << MV[Rz] << setw(SPACING) << MV[Uz] << setw(SPACING) << MV[Az] << setw(SPACING) << MV[Tz] << endl;
	cout << setw(SPACING) << MV[3] << setw(SPACING) << MV[7] << setw(SPACING) << MV[11] << setw(SPACING) << MV[15] << endl;
	cout << "--------------------------------------------------" << endl;
	cout << endl;
}