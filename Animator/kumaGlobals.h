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
	XPOS = 0, YPOS, ZPOS,

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

	NUMCONTROLS
};

// We'll be getting the instance of the application a lot; 
// might as well have it as a macro.
#define VAL(x) (ModelerApplication::Instance()->GetControlValue(x))
#define SETVAL(x, v) (ModelerApplication::Instance()->SetControlValue(x, v))
#endif