#include "kumaModel.h"
#include <FL/gl.h>
#include "modelerdraw.h"
#include "modelerapp.h"
#include "modelerui.h"
#include "kumaGlobals.h"
#include <fstream>
#include "camera.h"

#define KUMA_BODY_COLOR 1.0f, 0.945f, 0.9098f
#define KUMA_HAIR_COLOR 0.588f, 0.337f, 0.302f
#define KUMA_EYE_COLOR 0.588f, 0.337f, 0.302f
#define KUMA_MOUTH_COLOR 0.988f, 0.710f, 0.702f
#define KUMA_CLOTH_COLOR 1.0f, 1.0f, 1.0f
#define KUMA_CLOTH_PART2_COLOR 0.310f, 0.596f, 0.624f
#define KUMA_TIE_COLOR 1.0f, 0.01f, 0.01f
#define KUMA_SELECTION_AMBIENT 0.3f, 0.3f, 0.7f
#define RESET_AMBIENT setAmbientColor(0, 0, 0)

#define ANGLE2RAIDUS_FACTOR 3.141592654 / 180

const double ik_upperArmWidth = 0.22;
const double ik_upperArmHeight = 0.7;
const double ik_upperArmDepth = 0.22;
const double ik_upperArmBodyOffsetX = 0.03;
const double ik_upperArmBodyOffsetY = 0.40;

const double ik_lowerArmWidth = 0.2;
const double ik_lowerArmHeight = 0.7;
const double ik_lowerArmDepth = 0.2;

extern Mat4f getViewMat(Vec3f pos, Vec3f lookat, Vec3f up);
extern void solveIK(int maxIter, double step, Vec3f target, int numVals, double* vals, double* constrains, void transform(const double* in, Vec3f& out));
extern Vec3f calculateBSplineSurfacePoint(double u, double v, const vector<Vec3f>& ctrlpts);

vector<Vec3f> ctrlpts = { Vec3f(1.0, 1.0, 3.0), Vec3f(2.0, 1.0, 3.0), Vec3f(3.0, 1.0, 3.0), Vec3f(4.0, 1.0, 3.0),
Vec3f(1.0, 2.0, 3.0), Vec3f(2.0, 2.0, 5.0), Vec3f(3.0, 2.0, 5.0), Vec3f(4.0, 2.0, 3.0),
Vec3f(1.0, 3.0, 3.0), Vec3f(2.0, 3.0, 5.0), Vec3f(3.0, 3.0, 5.0), Vec3f(4.0, 3.0, 3.0),
Vec3f(1.0, 4.0, 3.0), Vec3f(2.0, 4.0, 3.0), Vec3f(3.0, 4.0, 3.0), Vec3f(4.0, 4.0, 3.0),
};

/* Variables for IK */
GLfloat currViewMat[16];
Mat4f currViewInv;
double leftArmConstrains[8] = { -180, 50, -90, 90, -180, 30, -120, 0 };
double rightArmConstrains[8] = { -180, 50, -90, 90, -30, 180, -120, 0 };

Mat4f getCurrentModelViewMatrix()
{
	GLfloat Mtmp[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, Mtmp);
	return Mat4f(Mtmp[0], Mtmp[4], Mtmp[8], Mtmp[12], Mtmp[1], Mtmp[5], Mtmp[9], Mtmp[13], Mtmp[2], Mtmp[6], Mtmp[10], Mtmp[14], Mtmp[3], Mtmp[7], Mtmp[11], Mtmp[15]);
}

void KumaModel::drawClothes(double clothHeight, double innerWidth, double innerHeight, double innerDepth, bool usePart2LargeHeight, bool useIndicatingColor, const float* indicatingColor)
{
	double part2Height = usePart2LargeHeight ? clothPart2Height : clothPart2Height2;
	if (useIndicatingColor)
		setAmbientColorv(indicatingColor);

	// left and back
	if (!useIndicatingColor)
		setDiffuseColor(KUMA_CLOTH_PART2_COLOR);
	glTranslated(-(clothBodyOffset + clothThickness), innerHeight, -(clothBodyOffset + clothThickness));
	drawBox(clothThickness, -part2Height, innerDepth + clothThickness * 2 + clothBodyOffset * 2);
	drawBox(clothThickness * 2 + clothBodyOffset * 2 + innerWidth,
		-part2Height, clothThickness);
	if (!useIndicatingColor)
		setDiffuseColor(KUMA_CLOTH_COLOR);
	glTranslated(0, -part2Height, 0);
	drawBox(clothThickness, -clothHeight, innerDepth + clothThickness * 2 + clothBodyOffset * 2);
	drawBox(clothThickness * 2 + clothBodyOffset * 2 + innerWidth,
		-clothHeight, clothThickness);
	glTranslated(0, part2Height, 0);

	// front and right
	if (!useIndicatingColor)
		setDiffuseColor(KUMA_CLOTH_PART2_COLOR);
	glTranslated(clothBodyOffset * 2 + clothThickness * 2 + innerWidth, 0, clothBodyOffset * 2 + clothThickness + innerDepth);
	drawBox(-(clothThickness * 2 + clothBodyOffset * 2 + innerWidth), -part2Height, clothThickness);
	drawBox(-clothThickness, -part2Height, -(innerDepth + clothThickness * 2 + clothBodyOffset));
	if (!useIndicatingColor)
		setDiffuseColor(KUMA_CLOTH_COLOR);
	glTranslated(0, -part2Height, 0);
	drawBox(-(clothThickness * 2 + clothBodyOffset * 2 + innerWidth), -clothHeight, clothThickness);
	drawBox(-clothThickness, -clothHeight, -(innerDepth + clothThickness * 2 + clothBodyOffset));
}

void KumaModel::drawModel(bool useIndicatingColor)
{
	auto pUI = ModelerApplication::Instance()->getPUI();

	// draw the floor
	setDiffuseColor(.8f, .8f, .8f);
	glPushMatrix();
	{
		glTranslated(-5, 0, -5);
		drawBox(10, 0.01f, 10);
	}
	glPopMatrix();

	if (pUI->m_pbtBSsurface->value() > 0)
	{
		if (pUI->m_pbtReadBSSfile->value() > 0)
		{
			pUI->m_pbtReadBSSfile->value(0);
			ifstream pointfile("points.txt");
			if (!pointfile.is_open())
			{
				cout << "FILE doesn't exist!" << endl;
			}
			vector<double> tmpvector;
			double tmp;
			for (int i = 0; i < 48; ++i)
			{
				pointfile >> tmp;
				if (pointfile.bad())
				{
					cout << "exception!" << endl;
					break;
				}
				tmpvector.push_back(tmp);
			}
			if (tmpvector.size() == 48)
			{
				cout << "using points from file" << endl;
				ctrlpts.clear();
				for (int i = 0; i < 16; ++i)
				{
					ctrlpts.push_back(Vec3f(tmpvector[3 * i], tmpvector[3 * i + 1], tmpvector[3 * i + 2]));
				}
			}
			pointfile.close();
		}
		setDiffuseColor(1.0f, 0.0f, 0.0f);
		int sampleRate = 20;
		for (int v = 0; v < sampleRate; ++v)
		{
			glBegin(GL_QUAD_STRIP);
			for (int u = 0; u <= sampleRate; ++u)
			{
				Vec3f tmp1 = calculateBSplineSurfacePoint((double)u / sampleRate, (double)v / sampleRate, ctrlpts);
				Vec3f tmp2 = calculateBSplineSurfacePoint((double)u / sampleRate, (double)(v + 1) / sampleRate, ctrlpts);
				if (u != sampleRate)
				{
					Vec3f tmp3 = calculateBSplineSurfacePoint((double)(u + 1) / sampleRate, (double)v / sampleRate, ctrlpts);
					Vec3f tmpN = (tmp2 - tmp1) ^ (tmp3 - tmp1);
					tmpN.normalize();
					glNormal3d(tmpN[0], tmpN[1], tmpN[2]);
				}
				else
				{
					Vec3f tmp3 = calculateBSplineSurfacePoint((double)(u - 1) / sampleRate, (double)v / sampleRate, ctrlpts);
					Vec3f tmpN = (tmp2 - tmp1) ^ (tmp1 - tmp3);
					tmpN.normalize();
					glNormal3d(tmpN[0], tmpN[1], tmpN[2]);
				}
				glVertex3f(tmp1[0], tmp1[1], tmp1[2]);
				glVertex3f(tmp2[0], tmp2[1], tmp2[2]);
			}
			glEnd();
		}
	}

	// setup vars
	currViewInv = getViewMat(m_camera->getPosition(), m_camera->getLookAt(), m_camera->getUpVector());
	currViewInv.getGLMatrix(currViewMat);
	currViewInv = currViewInv.inverse();

	// draw the model
	glPushMatrix();
	{
		glTranslated(VAL(XPOS), VAL(YPOS), VAL(ZPOS));
		drawTorso(false);
	}
	glPopMatrix();

	// draw IK targets
	if (pUI->m_pbtnDrawIKTarg->value() > 0)
	{
		glPushMatrix();
		{
			setDiffuseColor(1, 1, 1);
			glTranslated(VAL(LEFT_ARM_IK_X), VAL(LEFT_ARM_IK_Y), VAL(LEFT_ARM_IK_Z));
			glTranslated(-0.1, -0.1, -0.1);
			drawBox(0.2, 0.2, 0.2);
		}
		glPopMatrix();
		glPushMatrix();
		{
			setDiffuseColor(1, 1, 1);
			glTranslated(VAL(RIGHT_ARM_IK_X), VAL(RIGHT_ARM_IK_Y), VAL(RIGHT_ARM_IK_Z));
			glTranslated(-0.1, -0.1, -0.1);
			drawBox(0.2, 0.2, 0.2);
		}
		glPopMatrix();
	}
}

void KumaModel::drawTorso(bool useIndicatingColor)
{
	// torso
	RESET_AMBIENT;
	if (useIndicatingColor)
		setAmbientColorv(indicatingColors[KumaModelPart::TORSO]);
	else
		setDiffuseColor(KUMA_BODY_COLOR);
	glPushMatrix();
	{
		glTranslated(-torsoWidth / 2, lowerLegHeight + upperLegHeight + waistHeight + waistTorsoOffset + upperLegWaistOffset, -torsoDepth / 2);
		if (!useIndicatingColor && lastSelectedPart == KumaModelPart::TORSO)
		{
			setAmbientColor(KUMA_SELECTION_AMBIENT);
		}
		drawBox(torsoWidth, torsoHeight, torsoDepth);

		// torso clothes
		glPushMatrix();
		{
			drawClothes(torsoClothHeight, torsoWidth, torsoHeight, torsoDepth, true, useIndicatingColor, indicatingColors[KumaModelPart::TORSO]);
			glTranslated(0, clothPart2Height, 0);

			// tie
			if (!useIndicatingColor)
				setDiffuseColor(KUMA_TIE_COLOR);
			glTranslated(-(torsoWidth / 2 + clothBodyOffset + clothThickness), 0, 0);
			drawTriangle(0.15, 0, 0,
				-0.15, 0, 0,
				0, -0.85, 0.1);
		}
		glPopMatrix(); // torso clothes

		drawHead(useIndicatingColor);
		drawWaist(useIndicatingColor);
		drawLeftArm(useIndicatingColor);
		drawRightArm(useIndicatingColor);
	}
	glPopMatrix(); // torso
}

GLfloat leftArmPrev[16];
void leftArmIKfunc(const double* invars, Vec3f& out)
{
	glPushMatrix();
	{
		glLoadMatrixf(leftArmPrev);
		glRotated(invars[0], 1, 0, 0);
		glRotated(invars[1], 0, 0, 1);
		glRotated(invars[2], 0, 1, 0);
		glTranslated(-ik_upperArmWidth / 2, 0, -ik_upperArmDepth / 2);
		glTranslated((ik_upperArmWidth - ik_lowerArmWidth) / 2, -ik_upperArmHeight, (ik_upperArmDepth - ik_lowerArmDepth) / 2);
		glRotated(invars[3], 1, 0, 0);
		glTranslated(ik_lowerArmWidth / 2, -ik_lowerArmHeight, ik_lowerArmDepth / 2);
		Mat4f MM = getCurrentModelViewMatrix();
		Mat4f MV1M = currViewInv * MM;
		out[0] = MV1M[0][3] / MV1M[3][3];
		out[1] = MV1M[1][3] / MV1M[3][3];
		out[2] = MV1M[2][3] / MV1M[3][3];
	}
	glPopMatrix();
}

void KumaModel::drawLeftArm(bool useIndicatingColor)
{
	// left arm
	RESET_AMBIENT;
	if (useIndicatingColor)
		setAmbientColorv(indicatingColors[KumaModelPart::LEFT_ARM_UPPER]);
	else
		setDiffuseColor(KUMA_BODY_COLOR);
	if (!useIndicatingColor && lastSelectedPart == KumaModelPart::LEFT_ARM_UPPER)
	{
		setAmbientColor(KUMA_SELECTION_AMBIENT);
	}
	glPushMatrix();
	{
		glTranslated(-(upperArmWidth + upperArmBodyOffsetX), (torsoHeight - upperArmBodyOffsetY), (torsoDepth - upperArmDepth) / 2);
		glTranslated(upperArmWidth / 2, 0, upperArmDepth / 2);
		glGetFloatv(GL_MODELVIEW_MATRIX, leftArmPrev);

		if (ModelerApplication::Instance()->getPUI()->m_pbtnEnableIK->value() > 0)
		{
			Vec3f targ(VAL(LEFT_ARM_IK_X), VAL(LEFT_ARM_IK_Y), VAL(LEFT_ARM_IK_Z));
			double vars[4] = { leftUpperArmRotationX, leftUpperArmRotationZ, leftUpperArmRotationY, leftLowerArmRotationX };
			solveIK(100, 0.57, targ, 4, vars, leftArmConstrains, leftArmIKfunc);
			leftUpperArmRotationX = vars[0];
			leftUpperArmRotationY = vars[1];
			leftUpperArmRotationZ = vars[2];
			leftLowerArmRotationX = vars[3];
		}
		glRotated(leftUpperArmRotationX, 1, 0, 0);
		glRotated(leftUpperArmRotationZ, 0, 0, 1);
		glRotated(leftUpperArmRotationY, 0, 1, 0);
		glTranslated(-upperArmWidth / 2, 0, -upperArmDepth / 2);
		drawBox(upperArmWidth, -upperArmHeight, upperArmDepth);

		// clothes (upper arm)
		glPushMatrix();
		{
			// add a cover
			if (!useIndicatingColor)
				setDiffuseColor(KUMA_CLOTH_COLOR);
			glTranslated(-(clothBodyOffset + clothThickness), clothBodyOffset + clothThickness, -(clothBodyOffset + clothThickness));
			drawBox(clothBodyOffset * 2 + clothThickness * 2 + upperArmWidth, -clothThickness, clothBodyOffset * 2 + clothThickness * 2 + upperArmDepth);
			glTranslated(clothBodyOffset + clothThickness, -(clothBodyOffset + clothThickness), clothBodyOffset + clothThickness);

			glRotated(180, 0, 0, 1);
			glTranslated(-upperArmWidth, upperArmClothHeight, 0);
			drawClothes(upperArmClothHeight, upperArmWidth, 0, upperArmDepth, false, useIndicatingColor, indicatingColors[KumaModelPart::LEFT_ARM_UPPER]);
		}
		glPopMatrix();

		// lower arm
		RESET_AMBIENT;
		if (useIndicatingColor)
			setAmbientColorv(indicatingColors[KumaModelPart::LEFT_ARM_LOWER]);
		else
			setDiffuseColor(KUMA_BODY_COLOR);
		if (!useIndicatingColor && lastSelectedPart == KumaModelPart::LEFT_ARM_LOWER)
		{
			setAmbientColor(0.3, 0.3, 0.3);
		}
		glPushMatrix();
		{
			glTranslated((upperArmWidth - lowerArmWidth) / 2, -upperArmHeight, (upperArmDepth - lowerArmDepth) / 2);
			glRotated(leftLowerArmRotationX, 1, 0, 0);
			drawBox(lowerArmWidth, -lowerArmHeight, lowerArmDepth);
			glTranslated(lowerArmWidth / 2, -lowerArmHeight, lowerArmDepth / 2);

			if (hasMouseDelta && lastSelectedPart == KumaModelPart::LEFT_ARM_LOWER)
			{
				Vec3f& targ = ikTarget[KumaModelPart::LEFT_ARM_LOWER];

				Mat4f MV1M = currViewInv * getCurrentModelViewMatrix();
				targ[0] = MV1M[0][3] / MV1M[3][3];
				targ[1] = MV1M[1][3] / MV1M[3][3];
				targ[2] = MV1M[2][3] / MV1M[3][3];
				
				targ[0] += lastMouseDeltaInWorld[0];
				targ[1] -= lastMouseDeltaInWorld[1];
				targ[2] += lastMouseDeltaInWorld[2];

				double vars[4] = { leftUpperArmRotationX, leftUpperArmRotationZ, leftUpperArmRotationY, leftLowerArmRotationX };
				
				solveIK(100, 0.02, targ, 4, vars, leftArmConstrains, leftArmIKfunc);

				SETVAL(LEFT_UPPER_ARM_ROTATION_X, vars[0]);
				SETVAL(LEFT_UPPER_ARM_ROTATION_Y, vars[1]);
				SETVAL(LEFT_UPPER_ARM_ROTATION_Z, vars[2]);
				SETVAL(LEFT_LOWER_ARM_ROTATION_X, vars[3]);
				hasMouseDelta = false;
			}
		}
		glPopMatrix();
	}
	glPopMatrix(); // left arm
}

GLfloat rightArmPrev[16];
void rightArmIKfunc(const double* invars, Vec3f& out)
{
	glPushMatrix();
	{
		glLoadMatrixf(rightArmPrev);
		glRotated(invars[0], 1, 0, 0);
		glRotated(invars[1], 0, 0, 1);
		glRotated(invars[2], 0, 1, 0);
		glTranslated(-ik_upperArmWidth / 2, 0, -ik_upperArmDepth / 2);
		glTranslated((ik_upperArmWidth - ik_lowerArmWidth) / 2, -ik_upperArmHeight, (ik_upperArmDepth - ik_lowerArmDepth) / 2);
		glRotated(invars[3], 1, 0, 0);
		glTranslated(ik_lowerArmWidth / 2, -ik_lowerArmHeight, ik_lowerArmDepth / 2);
		Mat4f MM = getCurrentModelViewMatrix();
		Mat4f MV1M = currViewInv * MM;
		out[0] = MV1M[0][3] / MV1M[3][3];
		out[1] = MV1M[1][3] / MV1M[3][3];
		out[2] = MV1M[2][3] / MV1M[3][3];
	}
	glPopMatrix();
}

void KumaModel::drawRightArm(bool useIndicatingColor)
{
	// right arm
	RESET_AMBIENT;
	if (useIndicatingColor)
		setAmbientColorv(indicatingColors[KumaModelPart::RIGHT_ARM_UPPER]);
	else
		setDiffuseColor(KUMA_BODY_COLOR);
	if (!useIndicatingColor && lastSelectedPart == KumaModelPart::RIGHT_ARM_UPPER)
	{
		setAmbientColor(KUMA_SELECTION_AMBIENT);
	}
	glPushMatrix();
	{
		glTranslated((torsoWidth + upperArmBodyOffsetX), (torsoHeight - upperArmBodyOffsetY), (torsoDepth - upperArmDepth) / 2);
		glTranslated(upperArmWidth / 2, 0, upperArmDepth / 2);
		glGetFloatv(GL_MODELVIEW_MATRIX, rightArmPrev);

		if (ModelerApplication::Instance()->getPUI()->m_pbtnEnableIK->value() > 0)
		{
			Vec3f targ(VAL(RIGHT_ARM_IK_X), VAL(RIGHT_ARM_IK_Y), VAL(RIGHT_ARM_IK_Z));
			double vars[4] = { rightUpperArmRotationX, rightUpperArmRotationZ, rightUpperArmRotationY, rightLowerArmRotationX };
			solveIK(100, 0.57, targ, 4, vars, rightArmConstrains, rightArmIKfunc);
			rightUpperArmRotationX = vars[0];
			rightUpperArmRotationZ = vars[1];
			rightUpperArmRotationY = vars[2];
			rightLowerArmRotationX = vars[3];
		}
		glRotated(rightUpperArmRotationX, 1, 0, 0);
		glRotated(rightUpperArmRotationZ, 0, 0, 1);
		glRotated(rightUpperArmRotationY, 0, 1, 0);
		glTranslated(-upperArmWidth / 2, 0, -upperArmDepth / 2);
		drawBox(upperArmWidth, -upperArmHeight, upperArmDepth);

		// clothes (upper arm)
		glPushMatrix();
		{
			// add a cover
			if (!useIndicatingColor)
				setDiffuseColor(KUMA_CLOTH_COLOR);
			glTranslated(-(clothBodyOffset + clothThickness), clothBodyOffset + clothThickness, -(clothBodyOffset + clothThickness));
			drawBox(clothBodyOffset * 2 + clothThickness * 2 + upperArmWidth, -clothThickness, clothBodyOffset * 2 + clothThickness * 2 + upperArmDepth);
			glTranslated(clothBodyOffset + clothThickness, -(clothBodyOffset + clothThickness), clothBodyOffset + clothThickness);

			glRotated(180, 0, 0, 1);
			glTranslated(-upperArmWidth, upperArmClothHeight, 0);
			drawClothes(upperArmClothHeight, upperArmWidth, 0, upperArmDepth, false, useIndicatingColor, indicatingColors[KumaModelPart::RIGHT_ARM_UPPER]);
		}
		glPopMatrix();

		// lower arm
		RESET_AMBIENT;
		if (useIndicatingColor)
			setAmbientColorv(indicatingColors[KumaModelPart::RIGHT_ARM_LOWER]);
		else
			setDiffuseColor(KUMA_BODY_COLOR);
		if (!useIndicatingColor && lastSelectedPart == KumaModelPart::RIGHT_ARM_LOWER)
		{
			setAmbientColor(KUMA_SELECTION_AMBIENT);
		}
		glPushMatrix();
		{
			glTranslated((upperArmWidth - lowerArmWidth) / 2, -upperArmHeight, (upperArmDepth - lowerArmDepth) / 2);
			glRotated(rightLowerArmRotationX, 1, 0, 0);
			drawBox(lowerArmWidth, -lowerArmHeight, lowerArmDepth);
			glTranslated(lowerArmWidth / 2, -lowerArmHeight, lowerArmDepth / 2);

			if (hasMouseDelta && lastSelectedPart == KumaModelPart::RIGHT_ARM_LOWER)
			{
				Vec3f& targ = ikTarget[KumaModelPart::RIGHT_ARM_LOWER];

				Mat4f MV1M = currViewInv * getCurrentModelViewMatrix();
				targ[0] = MV1M[0][3] / MV1M[3][3];
				targ[1] = MV1M[1][3] / MV1M[3][3];
				targ[2] = MV1M[2][3] / MV1M[3][3];

				targ[0] += lastMouseDeltaInWorld[0];
				targ[1] -= lastMouseDeltaInWorld[1];
				targ[2] += lastMouseDeltaInWorld[2];

				double vars[4] = { rightUpperArmRotationX, rightUpperArmRotationZ, rightUpperArmRotationY, rightLowerArmRotationX };

				solveIK(100, 0.02, targ, 4, vars, rightArmConstrains, rightArmIKfunc);

				SETVAL(RIGHT_UPPER_ARM_ROTATION_X, vars[0]);
				SETVAL(RIGHT_UPPER_ARM_ROTATION_Y, vars[1]);
				SETVAL(RIGHT_UPPER_ARM_ROTATION_Z, vars[2]);
				SETVAL(RIGHT_LOWER_ARM_ROTATION_X, vars[3]);
				hasMouseDelta = false;
			}
		}
		glPopMatrix();
	}
	glPopMatrix();
}

void KumaModel::drawHead(bool useIndicatingColor)
{
	// head
	RESET_AMBIENT;
	if (useIndicatingColor)
		setAmbientColorv(indicatingColors[KumaModelPart::HEAD]);
	else
		setDiffuseColor(KUMA_BODY_COLOR);
	if (!useIndicatingColor && lastSelectedPart == KumaModelPart::HEAD)
	{
		setAmbientColor(KUMA_SELECTION_AMBIENT);
	}
	glPushMatrix();
	{
		glTranslated((torsoWidth - headWidth) / 2, torsoHeight, -(headDepth - torsoDepth) / 2.0);
		glTranslated(headWidth / 2, 0, headDepth / 2);
		glRotated(headRotationX, 1, 0, 0);
		glRotated(headRotationZ, 0, 0, 1);
		glRotated(headRotationY, 0, 1, 0);
		glTranslated(-headWidth / 2, 0, -headDepth / 2);
		drawBox(headWidth, headHeight, headDepth);

		drawFace(useIndicatingColor);
		drawHair(useIndicatingColor);
	}
	glPopMatrix(); // head
}

void KumaModel::drawFace(bool useIndicatingColor)
{
	if (useIndicatingColor)
		setAmbientColorv(indicatingColors[KumaModelPart::HEAD]);

	bool useTexture = false; // handled later...
	if (useTexture)
	{
		glPushMatrix();
		{
			glTranslated(0, 0, headDepth + 0.01);
			drawTexture(headWidth);
		}
		glPopMatrix();
	}
	else
	{
		// eyes
		if (!useIndicatingColor)
			setDiffuseColor(KUMA_EYE_COLOR);
		glPushMatrix();
		{
			glTranslated(eyeOffsetX, eyeOffsetY, headDepth - 0.01);
			drawBox(eyeWidth, eyeHeight, 0.02);

			glTranslated(headWidth - 2 * eyeOffsetX - eyeWidth, 0, 0);
			drawBox(eyeWidth, eyeHeight, 0.02);
		}
		glPopMatrix();

		// mouth
		if (!useIndicatingColor)
			setDiffuseColor(KUMA_MOUTH_COLOR);
		glPushMatrix();
		{
			glTranslated((headWidth - mouthWidth) / 2, mouthOffsetY, headDepth - 0.01);
			drawBox(mouthWidth, mouthHeight, 0.02);

			// If particle system exists, draw it
			if (!useIndicatingColor && this->particleSystem != NULL && t > 0) {
				// this should be done inside particle system
				setDiffuseColor(0.3, 0.3, 1.0);
				this->particleSystem->drawParticles(t);
			}
		}
		glPopMatrix();
	}
}

void KumaModel::drawHair(bool useIndicatingColor)
{
	// hair
	if (useIndicatingColor)
		setAmbientColorv(indicatingColors[KumaModelPart::HEAD]);
	else
		setDiffuseColor(KUMA_HAIR_COLOR);
	glPushMatrix();
	{
		glTranslated(-(hairHeadOffset + hairThickness), headHeight + hairHeadOffset, -(hairHeadOffset + hairThickness));
		// left
		drawBox(hairThickness, -sideHairHeight, sideHairDepth + hairThickness + hairHeadOffset);
		// top
		drawBox(hairThickness * 2 + hairHeadOffset * 2 + headWidth,
			hairThickness, sideHairDepth + hairThickness + hairHeadOffset);
		// back
		drawBox(hairThickness * 2 + hairHeadOffset * 2 + headWidth,
			-backHairHeight, hairThickness + hairHeadOffset);
		// right
		glTranslated(hairThickness + hairHeadOffset * 2 + headWidth, 0, 0);
		drawBox(hairThickness, -sideHairHeight, sideHairDepth + hairThickness + hairHeadOffset);
		// front
		glTranslated(hairThickness, 0, sideHairDepth + hairHeadOffset);
		drawBox(-(hairThickness * 2 + hairHeadOffset * 2 + headWidth), -frontHairHeight, hairThickness);
	}
	glPopMatrix();

	// Aho hair
	glPushMatrix();
	{
		glTranslated(headWidth / 2, headHeight + hairHeadOffset + hairThickness, headDepth / 2);
		glScaled(ahoHairScale, ahoHairScale, ahoHairScale);

		// l1
		drawTriangle(0, 0, 0,
			0.2, 0.2, -0.1,
			0.15, 0.3, -0.2);
		// l2
		drawTriangle(0.2, 0.2, -0.1,
			-0.225, 0.7, 0.15,
			-0.15, 0.56, 0.275);
		drawTriangle(0.2, 0.2, -0.1,
			0.15, 0.3, -0.2,
			-0.225, 0.7, 0.15);
		// l3
		drawTriangle(-0.225, 0.7, 0.15,
			0.8, 1.2, -0.7,
			0.7, 1.38, -0.85);
		drawTriangle(-0.225, 0.7, 0.15,
			-0.15, 0.56, 0.275,
			0.8, 1.2, -0.7);
		// l4
		drawTriangle(0.8, 1.2, -0.7,
			0.7, 1.38, -0.85,
			-1.5, 1.1, 0.3);
	}
	glPopMatrix();
}

void KumaModel::drawWaist(bool useIndicatingColor)
{
	// waist
	RESET_AMBIENT;
	if (useIndicatingColor)
		setAmbientColorv(indicatingColors[KumaModelPart::WAIST]);
	else
		setDiffuseColor(KUMA_BODY_COLOR);
	if (!useIndicatingColor && lastSelectedPart == KumaModelPart::WAIST)
	{
		setAmbientColor(KUMA_SELECTION_AMBIENT);
	}
	glPushMatrix();
	{
		glTranslated(0, -(waistHeight + waistTorsoOffset), 0);
		glTranslated(torsoWidth / 2, waistHeight, torsoDepth / 2);
		glRotated(waistRotationX, 1, 0, 0);
		glRotated(waistRotationZ, 0, 0, 1);
		glRotated(waistRotationY, 0, 1, 0);
		glTranslated(-torsoWidth / 2, -waistHeight, -torsoDepth / 2);
		drawBox(torsoWidth, waistHeight, torsoDepth);

		// clothes (waist)
		glPushMatrix();
		{
			drawClothes(waistClothHeight, torsoWidth, waistHeight, torsoDepth, false, useIndicatingColor, indicatingColors[KumaModelPart::WAIST]);
		}
		glPopMatrix();

		drawLeftLeg(useIndicatingColor);
		drawRightLeg(useIndicatingColor);
	}
	glPopMatrix(); // waist
}

void KumaModel::drawLeftLeg(bool useIndicatingColor)
{
	// left upper leg
	RESET_AMBIENT;
	if (useIndicatingColor)
		setAmbientColorv(indicatingColors[KumaModelPart::LEFT_LEG_UPPER]);
	else
		setDiffuseColor(KUMA_BODY_COLOR);
	if (!useIndicatingColor && lastSelectedPart == KumaModelPart::LEFT_LEG_UPPER)
	{
		setAmbientColor(KUMA_SELECTION_AMBIENT);
	}
	glPushMatrix();
	{
		glTranslated(upperLegOffsetX, -upperLegWaistOffset, (torsoDepth - upperLegDepth) / 2);
		glTranslated(upperLegWidth / 2, 0, upperLegDepth / 2);
		glRotated(leftUpperLegRotationX, 1, 0, 0);
		glRotated(leftUpperLegRotationZ, 0, 0, 1);
		glRotated(leftUpperLegRotationY, 0, 1, 0);
		glTranslated(-upperLegWidth / 2, 0, -upperLegDepth / 2);
		drawBox(upperLegWidth, -upperLegHeight, upperLegDepth);

		// clothes (upper leg)
		glPushMatrix();
		{
			glRotated(180, 0, 0, 1);
			glTranslated(-upperLegWidth, upperLegClothHeight, 0);
			drawClothes(upperLegClothHeight, upperLegWidth, 0, upperLegDepth, false, useIndicatingColor, indicatingColors[KumaModelPart::LEFT_LEG_UPPER]);
		}
		glPopMatrix();

		// left lower leg
		RESET_AMBIENT;
		if (useIndicatingColor)
			setAmbientColorv(indicatingColors[KumaModelPart::LEFT_LEG_LOWER]);
		else
			setDiffuseColor(KUMA_BODY_COLOR);
		if (!useIndicatingColor && lastSelectedPart == KumaModelPart::LEFT_LEG_LOWER)
		{
			setAmbientColor(KUMA_SELECTION_AMBIENT);
		}
		glPushMatrix();
		{
			glTranslated((upperLegWidth - lowerLegWidth) / 2, -upperLegHeight, (upperLegDepth - lowerLegDepth) / 2);
			glTranslated(0, 0, lowerLegDepth);
			glRotated(leftLowerLegRotationX, 1, 0, 0);
			glTranslated(0, 0, -lowerLegDepth);
			drawBox(lowerLegWidth, -lowerLegHeight, lowerLegDepth);
		}
		glPopMatrix();

	}
	glPopMatrix(); // left upper leg
}

void KumaModel::drawRightLeg(bool useIndicatingColor)
{
	// right upper leg
	RESET_AMBIENT;
	if (useIndicatingColor)
		setAmbientColorv(indicatingColors[KumaModelPart::RIGHT_LEG_UPPER]);
	else
		setDiffuseColor(KUMA_BODY_COLOR);
	if (!useIndicatingColor && lastSelectedPart == KumaModelPart::RIGHT_LEG_UPPER)
	{
		setAmbientColor(KUMA_SELECTION_AMBIENT);
	}
	glPushMatrix();
	{
		glTranslated(torsoWidth - upperLegWidth - upperLegOffsetX, -upperLegWaistOffset, (torsoDepth - upperLegDepth) / 2);
		glTranslated(upperLegWidth / 2, 0, upperLegDepth / 2);
		glRotated(rightUpperLegRotationX, 1, 0, 0);
		glRotated(rightUpperLegRotationZ, 0, 0, 1);
		glRotated(rightUpperLegRotationY, 0, 1, 0);
		glTranslated(-upperLegWidth / 2, 0, -upperLegDepth / 2);
		drawBox(upperLegWidth, -upperLegHeight, upperLegDepth);

		// clothes (upper leg)
		glPushMatrix();
		{
			glRotated(180, 0, 0, 1);
			glTranslated(-upperLegWidth, upperLegClothHeight, 0);
			drawClothes(upperLegClothHeight, upperLegWidth, 0, upperLegDepth, false, useIndicatingColor, indicatingColors[KumaModelPart::RIGHT_LEG_UPPER]);
		}
		glPopMatrix();

		// right lower leg
		RESET_AMBIENT;
		if (useIndicatingColor)
			setAmbientColorv(indicatingColors[KumaModelPart::RIGHT_LEG_LOWER]);
		else
			setDiffuseColor(KUMA_BODY_COLOR);
		if (!useIndicatingColor && lastSelectedPart == KumaModelPart::RIGHT_LEG_LOWER)
		{
			setAmbientColor(KUMA_SELECTION_AMBIENT);
		}
		glPushMatrix();
		{
			glTranslated((upperLegWidth - lowerLegWidth) / 2, -upperLegHeight, (upperLegDepth - lowerLegDepth) / 2);
			glTranslated(0, 0, lowerLegDepth);
			glRotated(rightLowerLegRotationX, 1, 0, 0);
			glTranslated(0, 0, -lowerLegDepth);
			drawBox(lowerLegWidth, -lowerLegHeight, lowerLegDepth);
		}
		glPopMatrix();
	}
	glPopMatrix(); // right upper leg
}

void KumaModel::updateParameters()
{
	headRotationX = VAL(HEAD_ROTATION_X);
	headRotationY = VAL(HEAD_ROTATION_Y);
	headRotationZ = VAL(HEAD_ROTATION_Z);

	waistRotationX = VAL(WAIST_ROTATION_X);
	waistRotationY = VAL(WAIST_ROTATION_Y);
	waistRotationZ = VAL(WAIST_ROTATION_Z);

	if (ModelerApplication::Instance()->getPUI()->m_pbtnEnableIK->value() < 1)
	{
		leftUpperLegRotationX = VAL(LEFT_UPPER_LEG_ROTATION_X);
		leftUpperLegRotationY = VAL(LEFT_UPPER_LEG_ROTATION_Y);
		leftUpperLegRotationZ = VAL(LEFT_UPPER_LEG_ROTATION_Z);
		leftLowerLegRotationX = VAL(LEFT_LOWER_LEG_ROTATION_X);

		rightUpperLegRotationX = VAL(RIGHT_UPPER_LEG_ROTATION_X);
		rightUpperLegRotationY = VAL(RIGHT_UPPER_LEG_ROTATION_Y);
		rightUpperLegRotationZ = VAL(RIGHT_UPPER_LEG_ROTATION_Z);
		rightLowerLegRotationX = VAL(RIGHT_LOWER_LEG_ROTATION_X);

		leftUpperArmRotationX = VAL(LEFT_UPPER_ARM_ROTATION_X);
		leftUpperArmRotationY = VAL(LEFT_UPPER_ARM_ROTATION_Y);
		leftUpperArmRotationZ = VAL(LEFT_UPPER_ARM_ROTATION_Z);
		leftLowerArmRotationX = VAL(LEFT_LOWER_ARM_ROTATION_X);

		rightUpperArmRotationX = VAL(RIGHT_UPPER_ARM_ROTATION_X);
		rightUpperArmRotationY = VAL(RIGHT_UPPER_ARM_ROTATION_Y);
		rightUpperArmRotationZ = VAL(RIGHT_UPPER_ARM_ROTATION_Z);
		rightLowerArmRotationX = VAL(RIGHT_LOWER_ARM_ROTATION_X);
	}
}