#ifndef _KUMAMODEL_H
#define _KUMAMODEL_H

#include <FL/gl.h>
#include <map>
#include <string>
#include <list>
#include "modelerview.h"
#include "particleSystem.h"

enum class KumaModelPart
{
	TORSO, HEAD, WAIST,
	LEFT_ARM_UPPER, LEFT_ARM_LOWER, RIGHT_ARM_UPPER, RIGHT_ARM_LOWER,
	LEFT_LEG_UPPER, LEFT_LEG_LOWER, RIGHT_LEG_UPPER, RIGHT_LEG_LOWER,
	NONE
};

class KumaModel : public ModelerView
{
public:
	KumaModel(int x, int y, int w, int h, char *label);
	GLuint texName;
	unsigned char * image = NULL;
	virtual void draw();
	virtual int handle(int ev);
	void drawTexture(int drawWidth);
	void getWhichPart();

private:
	GLuint fbo;
	GLuint render_buf;
	GLuint depth_buf;
	GLubyte* hiddenBuffer;
	KumaModelPart lastSelectedPart;
	std::map<KumaModelPart, float*> indicatingColors;
	std::map<KumaModelPart, std::string> partNames;
	std::map<KumaModelPart, std::list<int>* > partControls;

	void updateParameters();

	// hierarchical drawing functions
	void drawModel(bool useIndicatingColor);
	void drawTorso(bool useIndicatingColor);
	void drawHead(bool useIndicatingColor);
	void drawFace(bool useIndicatingColor);
	void drawHair(bool useIndicatingColor);
	void drawWaist(bool useIndicatingColor);
	void drawLeftLeg(bool useIndicatingColor);
	void drawRightLeg(bool useIndicatingColor);
	void drawLeftArm(bool useIndicatingColor);
	void drawRightArm(bool useIndicatingColor);
	void drawClothes(double clothHeight, double innerWidth, double innerHeight, double innerDepth, bool usePart2LargeHeight, bool useIndicatingColor, const float* indicatingColor);

	ParticleSystem* particleSystem;

	// parameters that comes from user
	double headRotationX;
	double headRotationY;
	double headRotationZ;

	double waistRotationX;
	double waistRotationY;
	double waistRotationZ;

	double leftUpperLegRotationX;
	double leftUpperLegRotationY;
	double leftUpperLegRotationZ;
	double leftLowerLegRotationX;

	double rightUpperLegRotationX;
	double rightUpperLegRotationY;
	double rightUpperLegRotationZ;
	double rightLowerLegRotationX;

	double leftUpperArmRotationX;
	double leftUpperArmRotationY;
	double leftUpperArmRotationZ;
	double leftLowerArmRotationX;

	double rightUpperArmRotationX;
	double rightUpperArmRotationY;
	double rightUpperArmRotationZ;
	double rightLowerArmRotationX;

	// const parameters
	const double torsoWidth = 1.0;
	const double torsoHeight = 1.2;
	const double torsoDepth = 0.5;

	const double headWidth = 1.0;
	const double headHeight = 1.0;
	const double headDepth = 1.0;

	const double eyeOffsetX = 0.17;
	const double eyeOffsetY = 0.57;
	const double eyeWidth = 0.2;
	const double eyeHeight = 0.25;

	const double mouthWidth = 0.15;
	const double mouthHeight = 0.10;
	const double mouthOffsetY = 0.20;

	const double hairHeadOffset = 0.04;
	const double hairThickness = 0.1;
	const double sideHairDepth = 1.05;
	const double sideHairHeight = 1.2;
	const double backHairHeight = 1.8;
	const double frontHairHeight = 0.1;
	const double ahoHairScale = 0.7;

	const double upperArmWidth = 0.22;
	const double upperArmHeight = 0.7;
	const double upperArmDepth = 0.22;
	const double upperArmBodyOffsetX = 0.03;
	const double upperArmBodyOffsetY = 0.40;

	const double lowerArmWidth = 0.2;
	const double lowerArmHeight = 0.7;
	const double lowerArmDepth = 0.2;

	const double waistHeight = 0.2;
	const double waistTorsoOffset = 0.01;

	const double upperLegWidth = 0.4;
	const double upperLegHeight = 0.75;
	const double upperLegDepth = 0.4;
	const double upperLegOffsetX = 0.01;
	const double upperLegWaistOffset = 0.0;

	const double lowerLegWidth = 0.3;
	const double lowerLegHeight = 0.7;
	const double lowerLegDepth = 0.3;

	const double clothThickness = 0.02;
	const double clothBodyOffset = 0.03;
	const double clothTorsoHeightOffset = 0.05;
	const double clothPart2Height = 0.3;
	const double clothPart2Height2 = 0.05;
	const double torsoClothHeight = torsoHeight - clothPart2Height - 0.1;
	const double waistClothHeight = waistHeight - clothPart2Height2;
	const double upperArmClothHeight = upperArmHeight - clothPart2Height2 - 0.2;
	const double upperLegClothHeight = upperLegHeight - clothPart2Height2 - 0.2;
};

#endif // _KUMAMODEL_H