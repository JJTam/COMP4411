#include "kumaModel.h"
#include <FL/gl.h>
#include "modelerdraw.h"
#include "modelerapp.h"
#include "kumaGlobals.h"

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
		if (!useIndicatingColor && lastSelectedPart == KumaModelPart::LEFT_ARM_LOWER)
		{
			setAmbientColor(0.3, 0.3, 0.3);
		}
		glPushMatrix();
		{
			glTranslated((upperArmWidth - lowerArmWidth) / 2, -upperArmHeight, (upperArmDepth - lowerArmDepth) / 2);
			glRotated(leftLowerArmRotationX, 1, 0, 0);
			drawBox(lowerArmWidth, -lowerArmHeight, lowerArmDepth);
		}
		glPopMatrix();
	}
	glPopMatrix(); // left arm
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
		if (!useIndicatingColor && lastSelectedPart == KumaModelPart::RIGHT_ARM_LOWER)
		{
			setAmbientColor(KUMA_SELECTION_AMBIENT);
		}
		glPushMatrix();
		{
			glTranslated((upperArmWidth - lowerArmWidth) / 2, -upperArmHeight, (upperArmDepth - lowerArmDepth) / 2);
			glRotated(rightLowerArmRotationX, 1, 0, 0);
			drawBox(lowerArmWidth, -lowerArmHeight, lowerArmDepth);
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
			if (!useIndicatingColor && this->particleSystem != NULL) {
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
		setAmbientColorv(indicatingColors[KumaModelPart::NONE]);
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
			drawClothes(waistClothHeight, torsoWidth, waistHeight, torsoDepth, false, useIndicatingColor, indicatingColors[KumaModelPart::NONE]);
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