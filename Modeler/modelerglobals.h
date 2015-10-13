#ifndef _MODELER_GLOBALS_H
#define _MODELER_GLOBALS_H

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502
#endif

// This is a list of the controls for the SampleModel
// We'll use these constants to access the values 
// of the controls from the user interface.
enum SampleModelControls
{ 
	// WARNING: DO NOT TOUCH THIS REGION, OR YOU WILL BREAK THE ANIMATIONS!
	// FIXED REGION BEGINS [--
	XPOS = 0, YPOS, ZPOS, 
	
	DRAW_LIGHT,
	LIGHT0_X, LIGHT0_Y, LIGHT0_Z, LIGHT0_DIFFUSE,
	LIGHT1_X, LIGHT1_Y, LIGHT1_Z, LIGHT1_DIFFUSE,

	HEAD_ROTATION_X, HEAD_ROTATION_Y, HEAD_ROTATION_Z,

	LEFT_UPPER_ARM_ROTATION_X, LEFT_UPPER_ARM_ROTATION_Y, LEFT_UPPER_ARM_ROTATION_Z,
	LEFT_LOWER_ARM_ROTATION_X,
	RIGHT_UPPER_ARM_ROTATION_X, RIGHT_UPPER_ARM_ROTATION_Y, RIGHT_UPPER_ARM_ROTATION_Z,
	RIGHT_LOWER_ARM_ROTATION_X,

	LEFT_UPPER_LEG_ROTATION_X, LEFT_UPPER_LEG_ROTATION_Y, LEFT_UPPER_LEG_ROTATION_Z,
	LEFT_LOWER_LEG_ROTATION_X,
	RIGHT_UPPER_LEG_ROTATION_X, RIGHT_UPPER_LEG_ROTATION_Y, RIGHT_UPPER_LEG_ROTATION_Z,
	RIGHT_LOWER_LEG_ROTATION_X,

	WAIST_ROTATION_X, WAIST_ROTATION_Y, WAIST_ROTATION_Z,
	// --] FIXED REGION ENDS
	// MAKE ANY CHANGES BELOW

	DRAW_LEVEL, DRAW_CLOTHES,
	DRAW_TEXTURE, TEXTURE_SIZE, TEXTURE_X, TEXTURE_Z,
	ANIMATION_SELECTION, LSYSTEM_SELECTION, LSYSTEM_ITER,

	IK_X, IK_Y, IK_Z, IK_ON,

	DRAW_TORUS, DRAW_METABALLS, METABALL_RADIUS,

	TORSO_WIDTH, TORSO_HEIGHT, TORSO_DEPTH,
	HEAD_WIDTH, HEAD_HEIGHT, HEAD_DEPTH,
	EYE_OFFSET_X, EYE_OFFSET_Y, EYE_WIDTH, EYE_HEIGHT,
	MOUTH_WIDTH, MOUTH_HEIGHT, MOUTH_OFFSET_Y,
	HAIR_HEAD_OFFSET, HAIR_THICKNESS, SIDEHAIR_DEPTH, SIDEHAIR_HEIGHT, BACKHAIR_HEIGHT, FRONTHAIR_HEIGHT, AHO_HAIR_SIZE,
	UPPER_ARM_WIDTH, UPPER_ARM_HEIGHT, UPPER_ARM_DEPTH, UPPER_ARM_BODY_OFFSET_X, UPPER_ARM_BODY_OFFSET_Y,
	LOWER_ARM_WIDTH, LOWER_ARM_HEIGHT, LOWER_ARM_DEPTH,
	WAIST_HEIGHT, WAIST_TORSO_OFFSET,
	UPPER_LEG_WIDTH, UPPER_LEG_HEIGHT, UPPER_LEG_DEPTH, UPPER_LEG_OFFSET_X, UPPER_LEG_OFFSET_WAIST,
	LOWER_LEG_WIDTH, LOWER_LEG_HEIGHT, LOWER_LEG_DEPTH,
	NUMCONTROLS
};

// Colors
#define COLOR_RED		1.0f, 0.0f, 0.0f
#define COLOR_GREEN		0.0f, 1.0f, 0.0f
#define COLOR_BLUE		0.0f, 0.0f, 1.0f

// We'll be getting the instance of the application a lot; 
// might as well have it as a macro.
#define VAL(x) (ModelerApplication::Instance()->GetControlValue(x))
#define SETVAL(x, v) (ModelerApplication::Instance()->SetControlValue(x, v))
#endif