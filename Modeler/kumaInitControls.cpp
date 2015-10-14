#include "modelerglobals.h"
#include "modelerapp.h"
#include "LSystem.h"
#include "animation.h"
#include <vector>

extern std::vector<AnimationDef*>* kumaAnimes;
extern std::vector<LSystem*>* kumaLSystems;

void kumaInitControls(ModelerControl* controls)
{
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
	controls[DRAW_TEXTURE] = ModelerControl("Draw Texture", 0, 1, 1, 0);
	controls[TEXTURE_SIZE] = ModelerControl("Texture Size", 1, 10, 1, 3);
	controls[TEXTURE_X] = ModelerControl("Texture X", -10, 10, 0.01f, 0);
	controls[TEXTURE_Z] = ModelerControl("Texture Z", -10, 10, 0.01f, -3.0);

	controls[ANIMATION_SELECTION] = ModelerControl("Animation Selection", 0, kumaAnimes->size(), 1, 0);
	controls[LSYSTEM_SELECTION] = ModelerControl("LSystem Selection", 0, kumaLSystems->size(), 1, 0);
	controls[LSYSTEM_ITER] = ModelerControl("LSystem Iterations", 0, 10, 1, 1);

	controls[IK_X] = ModelerControl("IK X", -3.0, 2.0, 0.1, -0.4);
	controls[IK_Y] = ModelerControl("IK Y", 0, 5.0, 0.1, 2.5);
	controls[IK_Z] = ModelerControl("IK Z", -2.0, 2.0, 0.1, 1.0);
	controls[IK_ON] = ModelerControl("IK ON", 0, 1, 1, 0);
	controls[IK_FAST] = ModelerControl("IK FAST", 0, 1, 1, 0);

	controls[DRAW_TORUS] = ModelerControl("Draw Torus", 0, 1, 1, 0);
	controls[DRAW_METABALLS] = ModelerControl("Metaballs", 0, 1, 1, 0);
	controls[METABALL_DEMOS] = ModelerControl("Metaball Demos", 0, 1, 1, 0);
	controls[METABALL_RADIUS] = ModelerControl("Metaball size", 0.6f, 1.0f, 0.01f, 1.0f);

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
}