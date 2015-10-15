#include "modelerview.h"
#include "modelerapp.h"
#include "modelerdraw.h"
#include <FL/gl.h>
#include <cmath>
#include "modelerglobals.h"
#include "animation.h"
#include <vector>
#include "LSystem.h"
#include "kumaLSystemOP.h"
#include "bitmap.h"
#include <FL/fl_ask.h>
#include "modelerui.h"
#include "kumaModel.h"
using namespace std;

extern vector<AnimationDef*>* kumaAnimes;
extern void kumaAnimationsSetup();
extern vector<LSystem*>* kumaLSystems;
extern void kumaLSystemSetup();
extern void kumaIK();
extern void kumaInitControls(ModelerControl* controls);
extern void drawMetaball(int numMetaballs, const vector< vector<float> >& balls);
extern void drawTorus(double posX, double posY, double posZ, double innerR, double outerR, int numc, int numt);
extern void drawDiamond();

// functions that are in helpers
extern void kumaSetupLights();
extern void kumaDrawLSystems();
extern void kumaHandleAnime();

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

	// Animation support
	kumaHandleAnime();

	// Change the lights
	kumaSetupLights();

	// draw the floor
	setAmbientColor(.1f, .1f, .1f);
	setDiffuseColor(1.0, 1.0, 1.0);
	glPushMatrix();
	{
		glTranslated(-5, 0, -5);
		drawBox(10, 0.01f, 10);
	}
	glPopMatrix();

	// draw the LSystem
	kumaDrawLSystems();

	// draw the texture
	/*
	if (VAL(DRAW_TEXTURE) > 0) {
		glPushMatrix();
		{
			double textureScale = VAL(TEXTURE_SIZE);
			double textureX = VAL(TEXTURE_X);
			double textureZ = VAL(TEXTURE_Z);
			glTranslated(textureX, 0, textureZ);
			glScaled(textureScale, textureScale, textureScale);
			drawTexture();
		}
		glPopMatrix();
	}
	*/
	// metaballs
	static vector< vector<float> > ball1 = { { -6.2f, 6.1f, -3.0f, 3.0f }, { 0.0f, 3.5f, -4.0f, 3.6f }, { 0.0f, 7.0f, -2.0f, 2.1f} };
	setDiffuseColor(KUMA_CLOTH_PART2_COLOR);
	if (VAL(DRAW_METABALLS) > 0 && VAL(METABALL_DEMOS) > 0) {
		glPushMatrix();
		{
			float sizeFactor = (float)VAL(METABALL_RADIUS);
			ball1[0][3] = 3.0f * sizeFactor;
			ball1[1][3] = 3.6f * sizeFactor;
			ball1[2][3] = 2.1f * sizeFactor;
			glTranslated(-2, 1, -2);
			glScaled(0.4, 0.4, 0.4);
			drawMetaball(ball1.size(), ball1);
		}
		glPopMatrix();
	}

	// IK
	if (VAL(IK_ON) > 0) {
		// draw IK box
		glPushMatrix();
		{
			setDiffuseColor(1.0, 1.0, 1.0);
			glTranslated(VAL(XPOS), VAL(YPOS), VAL(ZPOS));
			glTranslated(VAL(IK_X) - 0.1, VAL(IK_Y) - 0.1, VAL(IK_Z) - 0.1);
			drawBox(0.2, 0.2, 0.2);
		}
		glPopMatrix();
		// run IK
		kumaIK();
	}

	if (VAL(DRAW_TRIANGLES) > 0)
	{
		
		glPushMatrix();
		{
			glScaled(0.5, 0.5, 0.5);
			glTranslated(3.0, 2.0, 1.5);
			glRotated(-20, 1, 0, 1);
			glPushMatrix();
			{
				setDiffuseColor(0.4, 0.4, 1.0);
				glScaled(0.1, 0.1, 0.1);
				drawDiamond();
			}
			glPopMatrix();
			setDiffuseColor(1.0, 1.0, 1.0);
			drawTorus(0, -1, 0, 0.1, 1, 20, 20);
		}
		glPopMatrix();
	}

	// draw the model
	setAmbientColor(.1f, .1f, .1f);
	setDiffuseColor(KUMA_BODY_COLOR);
	glPushMatrix();
	{
		glTranslated(VAL(XPOS), VAL(YPOS), VAL(ZPOS));

		double detailLevel = VAL(DRAW_LEVEL);
		bool shallDrawClothes = VAL(DRAW_CLOTHES) > 0;

		// parameters
		double torsoWidth = VAL(TORSO_WIDTH);
		double torsoHeight = VAL(TORSO_HEIGHT);
		double torsoDepth = VAL(TORSO_DEPTH);

		double headWidth = VAL(HEAD_WIDTH);
		double headHeight = VAL(HEAD_HEIGHT);
		double headDepth = VAL(HEAD_DEPTH);

		double eyeOffsetX = VAL(EYE_OFFSET_X);
		double eyeOffsetY = VAL(EYE_OFFSET_Y);
		double eyeWidth = VAL(EYE_WIDTH);
		double eyeHeight = VAL(EYE_HEIGHT);

		double mouthWidth = VAL(MOUTH_WIDTH);
		double mouthHeight = VAL(MOUTH_HEIGHT);
		double mouthOffsetY = VAL(MOUTH_OFFSET_Y);

		double hairHeadOffset = VAL(HAIR_HEAD_OFFSET);
		double hairThickness = VAL(HAIR_THICKNESS);
		double sideHairDepth = VAL(SIDEHAIR_DEPTH);
		double sideHairHeight = VAL(SIDEHAIR_HEIGHT);
		double backHairHeight = VAL(BACKHAIR_HEIGHT);
		double frontHairHeight = VAL(FRONTHAIR_HEIGHT);
		double ahoHairScale = VAL(AHO_HAIR_SIZE);

		double upperArmWidth = VAL(UPPER_ARM_WIDTH);
		double upperArmHeight = VAL(UPPER_ARM_HEIGHT);
		double upperArmDepth = VAL(UPPER_ARM_DEPTH);
		double upperArmBodyOffsetX = VAL(UPPER_ARM_BODY_OFFSET_X);
		double upperArmBodyOffsetY = VAL(UPPER_ARM_BODY_OFFSET_Y);

		double lowerArmWidth = VAL(LOWER_ARM_WIDTH);
		double lowerArmHeight = VAL(LOWER_ARM_HEIGHT);
		double lowerArmDepth = VAL(LOWER_ARM_DEPTH);

		double waistHeight = VAL(WAIST_HEIGHT);
		double waistTorsoOffset = VAL(WAIST_TORSO_OFFSET);

		double upperLegWidth = VAL(UPPER_LEG_WIDTH);
		double upperLegHeight = VAL(UPPER_LEG_HEIGHT);
		double upperLegDepth = VAL(UPPER_LEG_DEPTH);
		double upperLegOffsetX = VAL(UPPER_LEG_OFFSET_X);
		double upperLegWaistOffset = VAL(UPPER_LEG_OFFSET_WAIST);

		double lowerLegWidth = VAL(LOWER_LEG_WIDTH);
		double lowerLegHeight = VAL(LOWER_LEG_HEIGHT);
		double lowerLegDepth = VAL(LOWER_LEG_DEPTH);

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
				if (VAL(DRAW_METABALLS) > 0)
				{
					glPushMatrix();
					{
						glTranslated(headWidth / 2, 0.50, headDepth / 2);
						glScaled(0.1, 0.1, 0.1);
						drawMetaball(headBalls.size(), headBalls);
					}
					glPopMatrix();
				}
				else
				{
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

						if (VAL(DRAW_TORUS) > 0)
						{
							setDiffuseColor(1.0f, 1.0f, 0.5f);
							glPushMatrix();
							{
								glRotated(90, 1, 0, 0);
								drawTorus(0.0f, 0.0f, -0.5f, 0.15f, 0.8f, 20, 20);
							}
							glPopMatrix();
						}
					}
					glPopMatrix();
				}
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
}

int main()
{
	// Setup the animations
	kumaAnimationsSetup();
	kumaLSystemSetup();

	// Initialize the controls
	// Constructor is ModelerControl(name, minimumvalue, maximumvalue, 
	// stepsize, defaultvalue)
	ModelerControl controls[NUMCONTROLS];
	kumaInitControls(controls);
	
	ModelerApplication::Instance()->Init(&createKumaModel, controls, NUMCONTROLS);
	return ModelerApplication::Instance()->Run();
}
