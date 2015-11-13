#include "modelerview.h"
#include "modelerapp.h"
#include "modelerdraw.h"
#include <FL/gl.h>
#include <cmath>
#include "kumaGlobals.h"
#include <vector>
#include "bitmap.h"
#include <FL/fl_ask.h>
#include "modelerui.h"
#include "kumaModel.h"
using namespace std;

extern void kumaInitControls(ModelerControl* controls);

#define KUMA_BODY_COLOR 1.0f, 0.945f, 0.9098f
#define KUMA_HAIR_COLOR 0.588f, 0.337f, 0.302f
#define KUMA_EYE_COLOR 0.588f, 0.337f, 0.302f
#define KUMA_MOUTH_COLOR 0.988f, 0.710f, 0.702f
#define KUMA_CLOTH_COLOR 1.0f, 1.0f, 1.0f
#define KUMA_CLOTH_PART2_COLOR 0.310f, 0.596f, 0.624f
#define KUMA_TIE_COLOR 1.0f, 0.01f, 0.01f

#define ANGLE2RAIDUS_FACTOR 3.141592654 / 180

// We need to make a creator function, mostly because of
// nasty API stuff that we'd rather stay away from.
ModelerView* createKumaModel(int x, int y, int w, int h, char *label)
{
	return new KumaModel(x, y, w, h, label);
}

void drawClothes(double clothBodyOffset, double clothThickness, double clothHeight, double clothPart2Height, double innerWidth, double innerHeight, double innerDepth)
{
	// left and back
	setDiffuseColor(KUMA_CLOTH_PART2_COLOR);
	glTranslated(-(clothBodyOffset + clothThickness), innerHeight, -(clothBodyOffset + clothThickness));
	drawBox(clothThickness, -clothPart2Height, innerDepth + clothThickness * 2 + clothBodyOffset * 2);
	drawBox(clothThickness * 2 + clothBodyOffset * 2 + innerWidth,
		-clothPart2Height, clothThickness);
	setDiffuseColor(KUMA_CLOTH_COLOR);
	glTranslated(0, -clothPart2Height, 0);
	drawBox(clothThickness, -clothHeight, innerDepth + clothThickness * 2 + clothBodyOffset * 2);
	drawBox(clothThickness * 2 + clothBodyOffset * 2 + innerWidth,
		-clothHeight, clothThickness);
	glTranslated(0, clothPart2Height, 0);

	// front and right
	setDiffuseColor(KUMA_CLOTH_PART2_COLOR);
	glTranslated(clothBodyOffset * 2 + clothThickness * 2 + innerWidth, 0, clothBodyOffset * 2 + clothThickness + innerDepth);
	drawBox(-(clothThickness * 2 + clothBodyOffset * 2 + innerWidth), -clothPart2Height, clothThickness);
	drawBox(-clothThickness, -clothPart2Height, -(innerDepth + clothThickness * 2 + clothBodyOffset));
	setDiffuseColor(KUMA_CLOTH_COLOR);
	glTranslated(0, -clothPart2Height, 0);
	drawBox(-(clothThickness * 2 + clothBodyOffset * 2 + innerWidth), -clothHeight, clothThickness);
	drawBox(-clothThickness, -clothHeight, -(innerDepth + clothThickness * 2 + clothBodyOffset));
}

// Override draw() to draw out Kuma
void KumaModel::draw()
{
	// This call takes care of a lot of the nasty projection 
	// matrix stuff.  Unless you want to fudge directly with the 
	// projection matrix, don't bother with this ...
	ModelerView::draw();

	// draw the floor
	setAmbientColor(.1f, .1f, .1f);
	setDiffuseColor(1.0, 1.0, 1.0);
	glPushMatrix();
	{
		glTranslated(-5, 0, -5);
		drawBox(10, 0.01f, 10);
	}
	glPopMatrix();

	// draw the model
	setAmbientColor(.1f, .1f, .1f);
	setDiffuseColor(KUMA_BODY_COLOR);
	glPushMatrix();
	{
		glTranslated(VAL(XPOS), VAL(YPOS), VAL(ZPOS));

		int detailLevel = 10;
		bool shallDrawClothes = true;

		// parameters
		double torsoWidth = 1.0;
		double torsoHeight = 1.2;
		double torsoDepth = 0.5;

		double headWidth = 1.0;
		double headHeight = 1.0;
		double headDepth = 1.0;

		double eyeOffsetX = 0.17;
		double eyeOffsetY = 0.57;
		double eyeWidth = 0.2;
		double eyeHeight = 0.25;

		double mouthWidth = 0.15;
		double mouthHeight = 0.10;
		double mouthOffsetY = 0.20;

		double hairHeadOffset = 0.04;
		double hairThickness = 0.1;
		double sideHairDepth = 1.05;
		double sideHairHeight = 1.2;
		double backHairHeight = 1.8;
		double frontHairHeight = 0.1;
		double ahoHairScale = 0.7;

		double upperArmWidth = 0.22;
		double upperArmHeight = 0.7;
		double upperArmDepth = 0.22;
		double upperArmBodyOffsetX = 0.03;
		double upperArmBodyOffsetY = 0.40;

		double lowerArmWidth = 0.2;
		double lowerArmHeight = 0.7;
		double lowerArmDepth = 0.2;

		double waistHeight = 0.2;
		double waistTorsoOffset = 0.01;

		double upperLegWidth = 0.4;
		double upperLegHeight = 0.75;
		double upperLegDepth = 0.4;
		double upperLegOffsetX = 0.01;
		double upperLegWaistOffset = 0.0;

		double lowerLegWidth = 0.3;
		double lowerLegHeight = 0.7;
		double lowerLegDepth = 0.3;

		double clothThickness = 0.02;
		double clothBodyOffset = 0.03;
		double clothTorsoHeightOffset = 0.05;
		double clothPart2Height = 0.3;
		double clothPart2Height2 = 0.05;
		double torsoClothHeight = torsoHeight - clothPart2Height - 0.1;
		double waistClothHeight = waistHeight - clothPart2Height2;
		double upperArmClothHeight = upperArmHeight - clothPart2Height2 - 0.2;
		double upperLegClothHeight = upperLegHeight - clothPart2Height2 - 0.2;

		double headRotationX = VAL(HEAD_ROTATION_X);
		double headRotationY = VAL(HEAD_ROTATION_Y);
		double headRotationZ = VAL(HEAD_ROTATION_Z);

		double waistRotationX = VAL(WAIST_ROTATION_X);
		double waistRotationY = VAL(WAIST_ROTATION_Y);
		double waistRotationZ = VAL(WAIST_ROTATION_Z);

		double leftUpperLegRotationX = VAL(LEFT_UPPER_LEG_ROTATION_X);
		double leftUpperLegRotationY = VAL(LEFT_UPPER_LEG_ROTATION_Y);
		double leftUpperLegRotationZ = VAL(LEFT_UPPER_LEG_ROTATION_Z);
		double leftLowerLegRotationX = VAL(LEFT_LOWER_LEG_ROTATION_X);

		double rightUpperLegRotationX = VAL(RIGHT_UPPER_LEG_ROTATION_X);
		double rightUpperLegRotationY = VAL(RIGHT_UPPER_LEG_ROTATION_Y);
		double rightUpperLegRotationZ = VAL(RIGHT_UPPER_LEG_ROTATION_Z);
		double rightLowerLegRotationX = VAL(RIGHT_LOWER_LEG_ROTATION_X);

		double leftUpperArmRotationX = VAL(LEFT_UPPER_ARM_ROTATION_X);
		double leftUpperArmRotationY = VAL(LEFT_UPPER_ARM_ROTATION_Y);
		double leftUpperArmRotationZ = VAL(LEFT_UPPER_ARM_ROTATION_Z);
		double leftLowerArmRotationX = VAL(LEFT_LOWER_ARM_ROTATION_X);

		double rightUpperArmRotationX = VAL(RIGHT_UPPER_ARM_ROTATION_X);
		double rightUpperArmRotationY = VAL(RIGHT_UPPER_ARM_ROTATION_Y);
		double rightUpperArmRotationZ = VAL(RIGHT_UPPER_ARM_ROTATION_Z);
		double rightLowerArmRotationX = VAL(RIGHT_LOWER_ARM_ROTATION_X);

		// torso
		glPushMatrix();
		if (detailLevel > 0)
		{
			glTranslated(-torsoWidth / 2, lowerLegHeight + upperLegHeight + waistHeight + waistTorsoOffset + upperLegWaistOffset, -torsoDepth / 2);
			drawBox(torsoWidth, torsoHeight, torsoDepth);

			// head
			glPushMatrix();
			if (detailLevel > 1)
			{
				glTranslated((torsoWidth - headWidth) / 2, torsoHeight, -(headDepth - torsoDepth) / 2.0);
				glTranslated(headWidth / 2, 0, headDepth / 2);
				glRotated(headRotationX, 1, 0, 0);
				glRotated(headRotationZ, 0, 0, 1);
				glRotated(headRotationY, 0, 1, 0);
				glTranslated(-headWidth / 2, 0, -headDepth / 2);

				static vector< vector<float> > headBalls = { { 0.0f, 0.0f, 0.0f, 5.0f }, { -6.0f, 6.7f, 0.0f, 2.0f }, { 6.0f, 6.7f, 0.0f, 2.0f } };
					drawBox(headWidth, headHeight, headDepth);

					if (VAL(DRAW_TEXTURE) > 0)
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
						setDiffuseColor(KUMA_EYE_COLOR);
						glPushMatrix();
						if (detailLevel > 2)
						{
							glTranslated(eyeOffsetX, eyeOffsetY, headDepth - 0.01);
							drawBox(eyeWidth, eyeHeight, 0.02);

							glTranslated(headWidth - 2 * eyeOffsetX - eyeWidth, 0, 0);
							drawBox(eyeWidth, eyeHeight, 0.02);
						}
						glPopMatrix();

						// mouth
						setDiffuseColor(KUMA_MOUTH_COLOR);
						glPushMatrix();
						if (detailLevel > 2)
						{
							glTranslated((headWidth - mouthWidth) / 2, mouthOffsetY, headDepth - 0.01);
							drawBox(mouthWidth, mouthHeight, 0.02);

							// If particle system exists, draw it
							ParticleSystem *ps = ModelerApplication::Instance()->GetParticleSystem();
							if (ps != NULL) {
								// this should be done inside particle system
								setDiffuseColor(0.3, 0.3, 1.0);
								ps->drawParticles(t);
							}

						}
						glPopMatrix();
					}

					// hair
					setDiffuseColor(KUMA_HAIR_COLOR);
					glPushMatrix();
					if (detailLevel > 2)
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
					if (detailLevel > 2)
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
			glPopMatrix(); // head

			// clothes
			glPushMatrix();
			if (shallDrawClothes)
			{
				drawClothes(clothBodyOffset, clothThickness, torsoClothHeight, clothPart2Height, torsoWidth, torsoHeight, torsoDepth);
				glTranslated(0, clothPart2Height, 0);

				// tie
				setDiffuseColor(KUMA_TIE_COLOR);
				glTranslated(-(torsoWidth / 2 + clothBodyOffset + clothThickness), 0, 0);
				drawTriangle(0.15, 0, 0,
					-0.15, 0, 0,
					0, -0.85, 0.1);
			}
			glPopMatrix(); // clothes

			// waist
			glPushMatrix();
			if (detailLevel > 2)
			{
				glTranslated(0, -(waistHeight + waistTorsoOffset), 0);
				glTranslated(torsoWidth / 2, waistHeight, torsoDepth / 2);
				glRotated(waistRotationX, 1, 0, 0);
				glRotated(waistRotationZ, 0, 0, 1);
				glRotated(waistRotationY, 0, 1, 0);
				glTranslated(-torsoWidth / 2, -waistHeight, -torsoDepth / 2);
				setDiffuseColor(KUMA_BODY_COLOR);
				drawBox(torsoWidth, waistHeight, torsoDepth);

				// left upper leg
				glPushMatrix();
				if (detailLevel > 3)
				{
					glTranslated(upperLegOffsetX, -upperLegWaistOffset, (torsoDepth - upperLegDepth) / 2);
					glTranslated(upperLegWidth / 2, 0, upperLegDepth / 2);
					glRotated(leftUpperLegRotationX, 1, 0, 0);
					glRotated(leftUpperLegRotationZ, 0, 0, 1);
					glRotated(leftUpperLegRotationY, 0, 1, 0);
					glTranslated(-upperLegWidth / 2, 0, -upperLegDepth / 2);

					drawBox(upperLegWidth, -upperLegHeight, upperLegDepth);

					// left lower leg
					glPushMatrix();
					if (detailLevel > 4)
					{
						glTranslated((upperLegWidth - lowerLegWidth) / 2, -upperLegHeight, (upperLegDepth - lowerLegDepth) / 2);
						glTranslated(0, 0, lowerLegDepth);
						glRotated(leftLowerLegRotationX, 1, 0, 0);
						glTranslated(0, 0, -lowerLegDepth);
						drawBox(lowerLegWidth, -lowerLegHeight, lowerLegDepth);
					}
					glPopMatrix();

					// clothes (upper leg)
					glPushMatrix();
					if (shallDrawClothes)
					{
						glRotated(180, 0, 0, 1);
						glTranslated(-upperLegWidth, upperLegClothHeight, 0);
						drawClothes(clothBodyOffset, clothThickness, upperLegClothHeight, clothPart2Height2, upperLegWidth, 0, upperLegDepth);
					}
					glPopMatrix();
				}
				glPopMatrix(); // left upper leg

				// right upper leg
				glPushMatrix();
				if (detailLevel > 3)
				{
					glTranslated(torsoWidth - upperLegWidth - upperLegOffsetX, -upperLegWaistOffset, (torsoDepth - upperLegDepth) / 2);
					glTranslated(upperLegWidth / 2, 0, upperLegDepth / 2);
					glRotated(rightUpperLegRotationX, 1, 0, 0);
					glRotated(rightUpperLegRotationZ, 0, 0, 1);
					glRotated(rightUpperLegRotationY, 0, 1, 0);
					glTranslated(-upperLegWidth / 2, 0, -upperLegDepth / 2);

					setDiffuseColor(KUMA_BODY_COLOR);
					drawBox(upperLegWidth, -upperLegHeight, upperLegDepth);

					// right lower leg
					glPushMatrix();
					if (detailLevel > 4)
					{
						glTranslated((upperLegWidth - lowerLegWidth) / 2, -upperLegHeight, (upperLegDepth - lowerLegDepth) / 2);
						glTranslated(0, 0, lowerLegDepth);
						glRotated(rightLowerLegRotationX, 1, 0, 0);
						glTranslated(0, 0, -lowerLegDepth);
						drawBox(lowerLegWidth, -lowerLegHeight, lowerLegDepth);
					}
					glPopMatrix();

					// clothes (upper leg)
					glPushMatrix();
					if (shallDrawClothes)
					{
						glRotated(180, 0, 0, 1);
						glTranslated(-upperLegWidth, upperLegClothHeight, 0);
						drawClothes(clothBodyOffset, clothThickness, upperLegClothHeight, clothPart2Height2, upperLegWidth, 0, upperLegDepth);
					}
					glPopMatrix();
				}
				glPopMatrix(); // right upper leg

				// clothes (waist)
				glPushMatrix();
				if (shallDrawClothes)
				{
					drawClothes(clothBodyOffset, clothThickness, waistClothHeight, clothPart2Height2, torsoWidth, waistHeight, torsoDepth);
				}
				glPopMatrix();
			}
			glPopMatrix(); // waist

			// left arm
			glPushMatrix();
			if (detailLevel > 2)
			{
				glTranslated(-(upperArmWidth + upperArmBodyOffsetX), (torsoHeight - upperArmBodyOffsetY), (torsoDepth - upperArmDepth) / 2);
				glTranslated(upperArmWidth / 2, 0, upperArmDepth / 2);
				glRotated(leftUpperArmRotationX, 1, 0, 0);
				glRotated(leftUpperArmRotationZ, 0, 0, 1);
				glRotated(leftUpperArmRotationY, 0, 1, 0);
				glTranslated(-upperArmWidth / 2, 0, -upperArmDepth / 2);

				setDiffuseColor(KUMA_BODY_COLOR);
				drawBox(upperArmWidth, -upperArmHeight, upperArmDepth);

				// lower arm
				glPushMatrix();
				if (detailLevel > 3)
				{
					glTranslated((upperArmWidth - lowerArmWidth) / 2, -upperArmHeight, (upperArmDepth - lowerArmDepth) / 2);
					glRotated(leftLowerArmRotationX, 1, 0, 0);
					drawBox(lowerArmWidth, -lowerArmHeight, lowerArmDepth);
				}
				glPopMatrix();
				 
				// clothes (upper arm)
				glPushMatrix();
				if (shallDrawClothes)
				{
					// add a cover
					setDiffuseColor(KUMA_CLOTH_COLOR);
					glTranslated(-(clothBodyOffset + clothThickness), clothBodyOffset + clothThickness, -(clothBodyOffset + clothThickness));
					drawBox(clothBodyOffset * 2 + clothThickness * 2 + upperArmWidth, -clothThickness, clothBodyOffset * 2 + clothThickness * 2 + upperArmDepth);
					glTranslated(clothBodyOffset + clothThickness, -(clothBodyOffset + clothThickness), clothBodyOffset + clothThickness);

					glRotated(180, 0, 0, 1);
					glTranslated(-upperArmWidth, upperArmClothHeight, 0);
					drawClothes(clothBodyOffset, clothThickness, upperArmClothHeight, clothPart2Height2, upperArmWidth, 0, upperArmDepth);
				}
				glPopMatrix();
			}
			glPopMatrix(); // left arm

			// right arm
			glPushMatrix();
			if (detailLevel > 2)
			{
				glTranslated((torsoWidth + upperArmBodyOffsetX), (torsoHeight - upperArmBodyOffsetY), (torsoDepth - upperArmDepth) / 2);
				glTranslated(upperArmWidth / 2, 0, upperArmDepth / 2);
				glRotated(rightUpperArmRotationX, 1, 0, 0);
				glRotated(rightUpperArmRotationZ, 0, 0, 1);
				glRotated(rightUpperArmRotationY, 0, 1, 0);
				glTranslated(-upperArmWidth / 2, 0, -upperArmDepth / 2);

				setDiffuseColor(KUMA_BODY_COLOR);
				drawBox(upperArmWidth, -upperArmHeight, upperArmDepth);

				// lower arm
				glPushMatrix();
				if (detailLevel > 3)
				{
					glTranslated((upperArmWidth - lowerArmWidth) / 2, -upperArmHeight, (upperArmDepth - lowerArmDepth) / 2);
					glRotated(rightLowerArmRotationX, 1, 0, 0);
					drawBox(lowerArmWidth, -lowerArmHeight, lowerArmDepth);
				}
				glPopMatrix();

				// clothes (upper arm)
				glPushMatrix();
				if (shallDrawClothes)
				{
					// add a cover
					setDiffuseColor(KUMA_CLOTH_COLOR);
					glTranslated(-(clothBodyOffset + clothThickness), clothBodyOffset + clothThickness, -(clothBodyOffset + clothThickness));
					drawBox(clothBodyOffset * 2 + clothThickness * 2 + upperArmWidth, -clothThickness, clothBodyOffset * 2 + clothThickness * 2 + upperArmDepth);
					glTranslated(clothBodyOffset + clothThickness, -(clothBodyOffset + clothThickness), clothBodyOffset + clothThickness);

					glRotated(180, 0, 0, 1);
					glTranslated(-upperArmWidth, upperArmClothHeight, 0);
					drawClothes(clothBodyOffset, clothThickness, upperArmClothHeight, clothPart2Height2, upperArmWidth, 0, upperArmDepth);
				}
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix(); // torso
	}
	glPopMatrix();

	endDraw();
}

int main()
{
	// Initialize the controls
	// Constructor is ModelerControl(name, minimumvalue, maximumvalue, 
	// stepsize, defaultvalue)
	ModelerControl controls[NUMCONTROLS];
	kumaInitControls(controls);
	
	Vec3f pcolor(0.2, 0.2, 1.0);
	Vec3f psize(0.05, 0.05, 0.05);
	ParticleSystem *ps = new ParticleSystem(5, 20, psize, pcolor, ParticleType::BALL);
	ModelerApplication::Instance()->SetParticleSystem(ps);

	ModelerApplication::Instance()->Init(&createKumaModel, controls, NUMCONTROLS);
	return ModelerApplication::Instance()->Run();
}
