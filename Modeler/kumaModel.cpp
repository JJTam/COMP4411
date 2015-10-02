#include "modelerview.h"
#include "modelerapp.h"
#include "modelerdraw.h"
#include <FL/gl.h>

#include "modelerglobals.h"

#define KUMA_BODY_COLOR 1.0f, 0.945f, 0.9098f
#define KUMA_HAIR_COLOR 0.588f, 0.337f, 0.302f
#define KUMA_EYE_COLOR 0.588f, 0.337f, 0.302f
#define KUMA_MOUTH_COLOR 0.988f, 0.710f, 0.702f
#define KUMA_CLOTH_COLOR 1.0f, 1.0f, 1.0f
#define KUMA_CLOTH_PART2_COLOR 0.310f, 0.596f, 0.624f
#define KUMA_TIE_COLOR 1.0f, 0.0f, 0.0f

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

		// parameters
		double torsoWidth = 1.0;
		double torsoHeight = 1.20;
		double torsoDepth = 0.5;

		double headWidth = 1.0;
		double headHeight = 1.0;
		double headDepth = 1.0;

		double eyeOffsetX = 0.167;
		double eyeOffsetY = 0.5714;
		double eyeWidth = 0.20;
		double eyeHeight = 0.25;

		double mouthWidth = 0.15;
		double mouthHeight = 0.10;
		double mouthOffsetY = 0.20;

		double hairHeadOffset = 0.04;
		double hairThickness = 0.02;
		double sideHairDepth = 1.05;
		double sideHairHeight = 1.2;
		double backHairHeight = 1.8;
		double frontHairHeight = 0.1;
		double ahoHairScale = VAL(AHO_HAIR_SIZE);

		double upperArmWidth = 0.22;
		double upperArmHeight = 0.7;
		double upperArmDepth = 0.22;
		double upperArmBodyOffsetX = 0.03;
		double upperArmBodyOffsetY = 0.40;

		double lowerArmWidth = 0.20;
		double lowerArmHeight = 0.7;
		double lowerArmDepth = 0.20;

		double waistHeight = 0.20;
		double waistTorsoOffset = 0.02;

		double upperLegWidth = 0.40;
		double upperLegHeight = 0.60;
		double upperLegDepth = 0.40;
		double upperLegOffsetX = 0.05;
		double upperLegWaistOffset = 0.0;

		double lowerLegWidth = 0.30;
		double lowerLegHeight = 0.60;
		double lowerLegDepth = 0.30;

		double footWidth = 0.30;
		double footHeight = 0.10;
		double footDepth = 0.50;

		double clothThickness = 0.02;
		double clothBodyOffset = 0.03;
		double clothTorsoHeightOffset = 0.05;
		double clothPart2Height = 0.3;
		double clothPart2Height2 = 0.05;
		double torsoClothHeight = torsoHeight - clothPart2Height - 0.1;
		double waistClothHeight = waistHeight - clothPart2Height2;
		double upperArmClothHeight = upperArmHeight - clothPart2Height2 - 0.2;
		double upperLegClothHeight = upperLegHeight - clothPart2Height2 - 0.2;

		double leftUpperLegRotation = 10;
		double leftLowerLegRotation = -4;
		double rightUpperLegRotation = -10;
		double rightLowerLegRotation = 0;

		double leftUpperArmRotationX = -40;
		double leftUpperArmRotationY = 0;
		double leftUpperArmRotationZ = 0;
		double leftLowerArmRotationX = -20;

		double rightUpperArmRotationX = 30;
		double rightUpperArmRotationY = 0;
		double rightUpperArmRotationZ = 0;
		double rightLowerArmRotationX = 0;

		// torso
		glPushMatrix();
		{
			glTranslated(-torsoDepth / 2, footHeight + lowerLegHeight + upperLegHeight + waistHeight + waistTorsoOffset + upperLegWaistOffset, -0.5 / 2);
			drawBox(torsoWidth, torsoHeight, torsoDepth);

			// head
			glPushMatrix();
			{
				glTranslated(0, torsoHeight, -(headDepth - torsoDepth) / 2.0);
				drawBox(headWidth, headHeight, headDepth);

				// eyes
				setDiffuseColor(KUMA_EYE_COLOR);
				glPushMatrix();
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
				{
					glTranslated((headWidth - mouthWidth) / 2, mouthOffsetY, 0.99);
					drawBox(mouthWidth, mouthHeight, 0.02);
				}
				glPopMatrix();

				// hair
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
			glPopMatrix(); // head


			// clothes
			glPushMatrix();
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
			{
				glTranslated(0, -(waistHeight + waistTorsoOffset), 0);
				setDiffuseColor(KUMA_BODY_COLOR);
				drawBox(torsoWidth, waistHeight, torsoDepth);

				// left upper leg
				glPushMatrix();
				{
					glTranslated(upperLegOffsetX, -upperLegWaistOffset, (torsoDepth - upperLegDepth) / 2);
					glRotated(leftUpperLegRotation, 1, 0, 0);
					drawBox(upperLegWidth, -upperLegHeight, upperLegDepth);

					// left lower leg
					glPushMatrix();
					{
						glTranslated((upperLegWidth - lowerLegWidth) / 2, -upperLegHeight, (upperLegDepth - lowerLegDepth) / 2);
						glRotated(leftLowerLegRotation, 1, 0, 0);
						drawBox(lowerLegWidth, -lowerLegHeight, lowerLegDepth);

						// left foot
						glPushMatrix();
						{

						}
						glPopMatrix();
					}
					glPopMatrix();

					// clothes (upper leg)
					glPushMatrix();
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
				{
					glTranslated(torsoWidth - upperLegWidth - upperLegOffsetX, -upperLegWaistOffset, (torsoDepth - upperLegDepth) / 2);
					glRotated(rightUpperLegRotation, 1, 0, 0);
					setDiffuseColor(KUMA_BODY_COLOR);
					drawBox(upperLegWidth, -upperLegHeight, upperLegDepth);

					// right lower leg
					glPushMatrix();
					{
						glTranslated((upperLegWidth - lowerLegWidth) / 2, -upperLegHeight, (upperLegDepth - lowerLegDepth) / 2);
						glRotated(rightLowerLegRotation, 1, 0, 0);
						drawBox(lowerLegWidth, -lowerLegHeight, lowerLegDepth);

						// right foot
						glPushMatrix();
						{

						}
						glPopMatrix();
					}
					glPopMatrix();

					// clothes (upper leg)
					glPushMatrix();
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
				{
					drawClothes(clothBodyOffset, clothThickness, waistClothHeight, clothPart2Height2, torsoWidth, waistHeight, torsoDepth);
				}
				glPopMatrix();
			}
			glPopMatrix(); // waist

			// left arm
			glPushMatrix();
			{
				glTranslated(-(upperArmWidth + upperArmBodyOffsetX), (torsoHeight - upperArmBodyOffsetY), (torsoDepth - upperArmWidth) / 2);
				glRotated(leftUpperArmRotationX, 1, 0, 0);
				glRotated(leftUpperArmRotationY, 0, 1, 0);
				glRotated(leftUpperArmRotationZ, 0, 0, 1);

				setDiffuseColor(KUMA_BODY_COLOR);
				drawBox(upperArmWidth, -upperArmHeight, upperArmDepth);

				// lower arm
				glPushMatrix();
				{
					glTranslated((upperArmWidth - lowerArmWidth) / 2, -upperArmHeight, (upperArmDepth - lowerArmDepth) / 2);
					glRotated(leftLowerArmRotationX, 1, 0, 0);
					drawBox(lowerArmWidth, -lowerArmHeight, lowerArmDepth);
				}
				glPopMatrix();

				// clothes (upper arm)
				glPushMatrix();
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
			{
				glTranslated((torsoWidth + upperArmBodyOffsetX), (torsoHeight - upperArmBodyOffsetY), (torsoDepth - upperArmWidth) / 2);
				glRotated(rightUpperArmRotationX, 1, 0, 0);
				glRotated(rightUpperArmRotationY, 0, 1, 0);
				glRotated(rightUpperArmRotationZ, 0, 0, 1);

				setDiffuseColor(KUMA_BODY_COLOR);
				drawBox(upperArmWidth, -upperArmHeight, upperArmDepth);

				// lower arm
				glPushMatrix();
				{
					glTranslated((upperArmWidth - lowerArmWidth) / 2, -upperArmHeight, (upperArmDepth - lowerArmDepth) / 2);
					glRotated(rightLowerArmRotationX, 1, 0, 0);
					drawBox(lowerArmWidth, -lowerArmHeight, lowerArmDepth);
				}
				glPopMatrix();

				// clothes (upper arm)
				glPushMatrix();
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
	controls[AHO_HAIR_SIZE] = ModelerControl("Aho Hair Size", 0.1, 1.5, 0.1f, 0.7);
	ModelerApplication::Instance()->Init(&createKumaModel, controls, NUMCONTROLS);
	return ModelerApplication::Instance()->Run();
}
