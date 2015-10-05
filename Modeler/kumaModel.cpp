#include "modelerview.h"
#include "modelerapp.h"
#include "modelerdraw.h"
#include <FL/gl.h>
#include <cmath>
#include "modelerglobals.h"

#define KUMA_BODY_COLOR 1.0f, 0.945f, 0.9098f
#define KUMA_HAIR_COLOR 0.588f, 0.337f, 0.302f
#define KUMA_EYE_COLOR 0.588f, 0.337f, 0.302f
#define KUMA_MOUTH_COLOR 0.988f, 0.710f, 0.702f
#define KUMA_CLOTH_COLOR 1.0f, 1.0f, 1.0f
#define KUMA_CLOTH_PART2_COLOR 0.310f, 0.596f, 0.624f
#define KUMA_TIE_COLOR 1.0f, 0.01f, 0.01f

#define ANGLE2RAIDUS_FACTOR 3.141592654 / 180

// Inherit off of ModelerView
class KumaModel : public ModelerView
{
public:
	KumaModel(int x, int y, int w, int h, char *label)
		: ModelerView(x, y, w, h, label) { }

	virtual void draw();
};

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

	// change the light
	static GLfloat light0pos[4];
	static GLfloat light0diff[4];
	static GLfloat light1pos[4];
	static GLfloat light1diff[4];
	light0pos[0] = VAL(LIGHT0_X); light0pos[1] = VAL(LIGHT0_Y); light0pos[2] = VAL(LIGHT0_Z); light0pos[3] = 0;
	light1pos[0] = VAL(LIGHT1_X); light1pos[1] = VAL(LIGHT1_Y); light1pos[2] = VAL(LIGHT1_Z); light1pos[3] = 0;
	light0diff[0] = light0diff[1] = light0diff[2] = light0diff[3] = VAL(LIGHT0_DIFFUSE);
	light1diff[0] = light1diff[1] = light1diff[2] = light1diff[3] = VAL(LIGHT1_DIFFUSE);
	glLightfv(GL_LIGHT0, GL_POSITION, light0pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0diff);
	glLightfv(GL_LIGHT1, GL_POSITION, light1pos);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1diff);

	// draw the light sources
	if (VAL(DRAW_LIGHT) > 0)
	{
		setDiffuseColor(1.0, 1.0, 1.0);
		glPushMatrix();
		{
			glTranslated(VAL(LIGHT0_X) - 0.1, VAL(LIGHT0_Y) - 0.1, VAL(LIGHT0_Z) - 0.1);
			drawBox(0.2, 0.2, 0.2);
		}
		glPopMatrix();
		glPushMatrix();
		{
			glTranslated(VAL(LIGHT1_X) - 0.1, VAL(LIGHT1_Y) - 0.1, VAL(LIGHT1_Z) - 0.1);
			drawBox(0.2, 0.2, 0.2);
		}
		glPopMatrix();
	}

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
			glTranslated(-torsoDepth / 2, lowerLegHeight + upperLegHeight + waistHeight + waistTorsoOffset + upperLegWaistOffset, -0.5 / 2);
			drawBox(torsoWidth, torsoHeight, torsoDepth);

			// head
			glPushMatrix();
			if (detailLevel > 1)
			{
				glTranslated(0, torsoHeight, -(headDepth - torsoDepth) / 2.0);
				glTranslated(headWidth / 2, 0, headDepth / 2);
				glRotated(headRotationY, 0, 1, 0);
				glRotated(headRotationX, cos(headRotationY * ANGLE2RAIDUS_FACTOR), 0, sin(headRotationY * ANGLE2RAIDUS_FACTOR));
				glRotated(headRotationZ, -sin(headRotationY * ANGLE2RAIDUS_FACTOR), sin(headRotationX * ANGLE2RAIDUS_FACTOR), cos(headRotationY * ANGLE2RAIDUS_FACTOR));
				glTranslated(-headWidth / 2, 0, -headDepth / 2);
				drawBox(headWidth, headHeight, headDepth);

				// eyes
				setDiffuseColor(KUMA_EYE_COLOR);
				glPushMatrix();
				if (detailLevel > 2)
				{
					glTranslated(eyeOffsetX, eyeOffsetY, 0.99);
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
					glTranslated((headWidth - mouthWidth) / 2, mouthOffsetY, 0.99);
					drawBox(mouthWidth, mouthHeight, 0.02);
				}
				glPopMatrix();

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
				glRotated(waistRotationY, 0, 1, 0);
				glRotated(waistRotationX, cos(waistRotationY * ANGLE2RAIDUS_FACTOR), 0, sin(waistRotationY * ANGLE2RAIDUS_FACTOR));
				glRotated(waistRotationZ, -sin(waistRotationY * ANGLE2RAIDUS_FACTOR), sin(waistRotationX * ANGLE2RAIDUS_FACTOR), cos(waistRotationY * ANGLE2RAIDUS_FACTOR));
				glTranslated(-torsoWidth / 2, -waistHeight, -torsoDepth / 2);

				setDiffuseColor(KUMA_BODY_COLOR);
				drawBox(torsoWidth, waistHeight, torsoDepth);

				// left upper leg
				glPushMatrix();
				if (detailLevel > 3)
				{
					glTranslated(upperLegOffsetX, -upperLegWaistOffset, (torsoDepth - upperLegDepth) / 2);
					glTranslated(upperLegWidth / 2, 0, upperLegDepth / 2);
					glRotated(leftUpperLegRotationY, 0, 1, 0);
					glRotated(leftUpperLegRotationX, cos(leftUpperLegRotationY * ANGLE2RAIDUS_FACTOR), 0, sin(leftUpperLegRotationY * ANGLE2RAIDUS_FACTOR));
					glRotated(leftUpperLegRotationZ, -sin(leftUpperLegRotationY * ANGLE2RAIDUS_FACTOR), sin(leftUpperLegRotationX * ANGLE2RAIDUS_FACTOR), cos(leftUpperLegRotationY * ANGLE2RAIDUS_FACTOR));
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
					glRotated(rightUpperLegRotationY, 0, 1, 0);
					glRotated(rightUpperLegRotationX, cos(rightUpperLegRotationY * ANGLE2RAIDUS_FACTOR), 0, sin(rightUpperLegRotationY * ANGLE2RAIDUS_FACTOR));
					glRotated(rightUpperLegRotationZ, -sin(rightUpperLegRotationY * ANGLE2RAIDUS_FACTOR), sin(rightUpperLegRotationX * ANGLE2RAIDUS_FACTOR), cos(rightUpperLegRotationY * ANGLE2RAIDUS_FACTOR));
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
				glRotated(leftUpperArmRotationY, 0, 1, 0);
				glRotated(leftUpperArmRotationX, cos(leftUpperArmRotationY * ANGLE2RAIDUS_FACTOR), 0, sin(leftUpperArmRotationY * ANGLE2RAIDUS_FACTOR));
				glRotated(leftUpperArmRotationZ, -sin(leftUpperArmRotationY * ANGLE2RAIDUS_FACTOR), sin(leftUpperArmRotationX * ANGLE2RAIDUS_FACTOR), cos(leftUpperArmRotationY * ANGLE2RAIDUS_FACTOR));
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
				glRotated(rightUpperArmRotationY, 0, 1, 0);
				glRotated(rightUpperArmRotationX, cos(rightUpperArmRotationY * ANGLE2RAIDUS_FACTOR), 0, sin(rightUpperArmRotationY * ANGLE2RAIDUS_FACTOR));
				glRotated(rightUpperArmRotationZ, -sin(rightUpperArmRotationY * ANGLE2RAIDUS_FACTOR), sin(rightUpperArmRotationX * ANGLE2RAIDUS_FACTOR), cos(rightUpperArmRotationY * ANGLE2RAIDUS_FACTOR));
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
	// Initialize the controls
	// Constructor is ModelerControl(name, minimumvalue, maximumvalue, 
	// stepsize, defaultvalue)
	ModelerControl controls[NUMCONTROLS];
	controls[XPOS] = ModelerControl("X Position", -5, 5, 0.1f, 0);
	controls[YPOS] = ModelerControl("Y Position", 0, 5, 0.1f, 0);
	controls[ZPOS] = ModelerControl("Z Position", -5, 5, 0.1f, 0);

	controls[DRAW_LIGHT] = ModelerControl("Draw lights", 0, 1, 1, 0);

	controls[LIGHT0_X] = ModelerControl("Light 0 X", -10, 10, 0.1f, 4);
	controls[LIGHT0_Y] = ModelerControl("Light 0 Y", -10, 10, 0.1f, 2);
	controls[LIGHT0_Z] = ModelerControl("Light 0 Z", -10, 10, 0.1f, -4);
	controls[LIGHT0_DIFFUSE] = ModelerControl("Light 0 Diffuse", 0, 5, 0.01f, 1);

	controls[LIGHT1_X] = ModelerControl("Light 1 X", -10, 10, 0.1f, -2);
	controls[LIGHT1_Y] = ModelerControl("Light 1 Y", -10, 10, 0.1f, 1);
	controls[LIGHT1_Z] = ModelerControl("Light 1 Z", -10, 10, 0.1f, 5);
	controls[LIGHT1_DIFFUSE] = ModelerControl("Light 1 Diffuse", 0, 5, 0.01f, 1);

	controls[HEAD_ROTATION_X] = ModelerControl("Head rotation X", -30, 30, 1, 0);
	controls[HEAD_ROTATION_Y] = ModelerControl("Head rotation Y", -60, 60, 1, 0);
	controls[HEAD_ROTATION_Z] = ModelerControl("Head rotation Z", -30, 30, 1, 0);

	controls[LEFT_UPPER_ARM_ROTATION_X] = ModelerControl("Right arm rotation X", -180, 50, 1, 0);
	controls[LEFT_UPPER_ARM_ROTATION_Y] = ModelerControl("Right arm rotation Y", -90, 90, 1, 0);
	controls[LEFT_UPPER_ARM_ROTATION_Z] = ModelerControl("Right arm rotation Z", -180, 30, 1, 0);
	controls[LEFT_LOWER_ARM_ROTATION_X] = ModelerControl("Right lower arm rotation X", -180, 0, 1, 0);
	controls[RIGHT_UPPER_ARM_ROTATION_X] = ModelerControl("Left arm rotation X", -180, 50, 1, 0);
	controls[RIGHT_UPPER_ARM_ROTATION_Y] = ModelerControl("Left arm rotation Y", -90, 90, 1, 0);
	controls[RIGHT_UPPER_ARM_ROTATION_Z] = ModelerControl("Left arm rotation Z", -30, 180, 1, 0);
	controls[RIGHT_LOWER_ARM_ROTATION_X] = ModelerControl("Left lower arm rotation X", -180, 0, 1, 0);

	controls[LEFT_UPPER_LEG_ROTATION_X] = ModelerControl("Right leg rotation X", -120, 50, 1, 0);
	controls[LEFT_UPPER_LEG_ROTATION_Y] = ModelerControl("Right leg rotation Y", -90, 90, 1, 0);
	controls[LEFT_UPPER_LEG_ROTATION_Z] = ModelerControl("Right leg rotation Z", -180, 180, 1, 0);
	controls[LEFT_LOWER_LEG_ROTATION_X] = ModelerControl("Right lower leg rotation X", 0, 120, 1, 0);
	controls[RIGHT_UPPER_LEG_ROTATION_X] = ModelerControl("Left leg rotation X", -120, 50, 1, 0);
	controls[RIGHT_UPPER_LEG_ROTATION_Y] = ModelerControl("Left leg rotation Y", -90, 90, 1, 0);
	controls[RIGHT_UPPER_LEG_ROTATION_Z] = ModelerControl("Left leg rotation Z", -180, 180, 1, 0);
	controls[RIGHT_LOWER_LEG_ROTATION_X] = ModelerControl("Left lower leg rotation X", 0, 120, 1, 0);

	controls[WAIST_ROTATION_X] = ModelerControl("Waist rotation X", -90, 90, 1, 0);
	controls[WAIST_ROTATION_Y] = ModelerControl("Waist rotation Y", -90, 90, 1, 0);
	controls[WAIST_ROTATION_Z] = ModelerControl("Waist rotation Z", -30, 30, 1, 0);

	controls[DRAW_LEVEL] = ModelerControl("Level of detail", 0, 5, 1, 5);
	controls[DRAW_CLOTHES] = ModelerControl("Draw clothes", 0, 1, 1, 1);

	controls[TORSO_WIDTH] = ModelerControl("Torso width", 0.0, 2.0, 0.01f, 1.0);
	controls[TORSO_HEIGHT] = ModelerControl("Torso height", 0.0, 2.0, 0.01f, 1.2);
	controls[TORSO_DEPTH] = ModelerControl("Torso depth", 0.0, 2.0, 0.01f, 0.5);
	controls[HEAD_WIDTH] = ModelerControl("Head width", 0.0, 2.0, 0.01f, 1.0);
	controls[HEAD_HEIGHT] = ModelerControl("Head height", 0.0, 2.0, 0.01f, 1.0);
	controls[HEAD_DEPTH] = ModelerControl("Head depth", 0.0, 2.0, 0.01f, 1.0);
	controls[EYE_OFFSET_X] = ModelerControl("Eye offset X", 0.0, 2.0, 0.01f, 0.17);
	controls[EYE_OFFSET_Y] = ModelerControl("Eye offset Y", 0.0, 2.0, 0.01f, 0.57);
	controls[EYE_WIDTH] = ModelerControl("Eye width", 0.0, 2.0, 0.01f, 0.20);
	controls[EYE_HEIGHT] = ModelerControl("Eye height", 0.0, 2.0, 0.01f, 0.25);
	controls[MOUTH_WIDTH] = ModelerControl("Mouth width", 0.0, 2.0, 0.01f, 0.15);
	controls[MOUTH_HEIGHT] = ModelerControl("Mouth height", 0.0, 2.0, 0.01f, 0.10);
	controls[MOUTH_OFFSET_Y] = ModelerControl("Mouth offset Y", 0.0, 2.0, 0.01f, 0.20);
	controls[HAIR_HEAD_OFFSET] = ModelerControl("Hair head offset", 0.0, 0.1, 0.01f, 0.04);
	controls[HAIR_THICKNESS] = ModelerControl("Hair thickness", 0.0, 0.5, 0.01f, 0.1);
	controls[SIDEHAIR_DEPTH] = ModelerControl("Sidehair depth", 0.0, 2.0, 0.01f, 1.05);
	controls[SIDEHAIR_HEIGHT] = ModelerControl("Sidehair height", 0.0, 3.0, 0.01f, 1.2);
	controls[BACKHAIR_HEIGHT] = ModelerControl("Backhair height", 0.0, 2.0, 0.01f, 1.8);
	controls[FRONTHAIR_HEIGHT] = ModelerControl("Fronthair height", 0.0, 2.0, 0.01f, 0.1);
	controls[AHO_HAIR_SIZE] = ModelerControl("Aho hair size", 0.1, 2.0, 0.1f, 0.7);
	controls[UPPER_ARM_WIDTH] = ModelerControl("Upper arm width", 0.0, 2.0, 0.01f, 0.22);
	controls[UPPER_ARM_HEIGHT] = ModelerControl("Upper arm height", 0.0, 2.0, 0.01f, 0.7);
	controls[UPPER_ARM_DEPTH] = ModelerControl("Upper arm depth", 0.0, 2.0, 0.01f, 0.22);
	controls[UPPER_ARM_BODY_OFFSET_X] = ModelerControl("Upper arm body offset X", 0.0, 2.0, 0.01f, 0.03);
	controls[UPPER_ARM_BODY_OFFSET_Y] = ModelerControl("Upper arm body offset Y", 0.0, 2.0, 0.01f, 0.40);
	controls[LOWER_ARM_WIDTH] = ModelerControl("Lower arm width", 0.0, 2.0, 0.01f, 0.2);
	controls[LOWER_ARM_HEIGHT] = ModelerControl("Lower arm height", 0.0, 2.0, 0.01f, 0.7);
	controls[LOWER_ARM_DEPTH] = ModelerControl("Lower arm depth", 0.0, 2.0, 0.01f, 0.2);
	controls[WAIST_HEIGHT] = ModelerControl("Waist height", 0.0, 2.0, 0.01f, 0.2);
	controls[WAIST_TORSO_OFFSET] = ModelerControl("Waist torso offset", 0.0, 2.0, 0.01f, 0.02);
	controls[UPPER_LEG_WIDTH] = ModelerControl("Upper leg width", 0.0, 2.0, 0.01f, 0.4);
	controls[UPPER_LEG_HEIGHT] = ModelerControl("Upper leg height", 0.0, 2.0, 0.01f, 0.75);
	controls[UPPER_LEG_DEPTH] = ModelerControl("Upper leg depth", 0.0, 2.0, 0.01f, 0.4);
	controls[UPPER_LEG_OFFSET_X] = ModelerControl("Upper leg offset X", 0.0, 2.0, 0.01f, 0.05);
	controls[UPPER_LEG_OFFSET_WAIST] = ModelerControl("Upper leg offset waist", 0.0, 2.0, 0.01f, 0.0);
	controls[LOWER_LEG_WIDTH] = ModelerControl("Lower leg width", 0.0, 2.0, 0.01f, 0.3);
	controls[LOWER_LEG_HEIGHT] = ModelerControl("Lower leg height", 0.0, 2.0, 0.01f, 0.7);
	controls[LOWER_LEG_DEPTH] = ModelerControl("Lower leg depth", 0.0, 2.0, 0.01f, 0.3);

	ModelerApplication::Instance()->Init(&createKumaModel, controls, NUMCONTROLS);
	return ModelerApplication::Instance()->Run();
}
