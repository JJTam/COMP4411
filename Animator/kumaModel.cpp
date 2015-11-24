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

KumaModel::KumaModel(int x, int y, int w, int h, char *label)
	: ModelerView(x, y, w, h, label)
{
	this->particleSystem = ModelerApplication::Instance()->GetParticleSystem();

	// setup indicating colors
	indicatingColors[KumaModelPart::NONE] = new float[] { 0.0f, 0, 0 };
	indicatingColors[KumaModelPart::TORSO] = new float[] { 0.1f, 0, 0 };
	indicatingColors[KumaModelPart::HEAD] = new float[] { 0.2f, 0, 0 };
	indicatingColors[KumaModelPart::LEFT_ARM_UPPER] = new float[] { 0.3f, 0, 0 };
	indicatingColors[KumaModelPart::LEFT_ARM_LOWER] = new float[] { 0.4f, 0, 0 };
	indicatingColors[KumaModelPart::RIGHT_ARM_UPPER] = new float[] { 0.5f, 0, 0 };
	indicatingColors[KumaModelPart::RIGHT_ARM_LOWER] = new float[] { 0.6f, 0, 0 };
	indicatingColors[KumaModelPart::LEFT_LEG_UPPER] = new float[] { 0.7f, 0, 0 };
	indicatingColors[KumaModelPart::LEFT_LEG_LOWER] = new float[] { 0.8f, 0, 0 };
	indicatingColors[KumaModelPart::RIGHT_LEG_UPPER] = new float[] { 0.9f, 0, 0 };
	indicatingColors[KumaModelPart::RIGHT_LEG_LOWER] = new float[] { 1.0f, 0, 0 };

	// TODO: prepare hidden buffer
	
}

// Override draw() to draw out Kuma
void KumaModel::draw()
{
	static GLfloat lightPosition0[] = { 4, 2, -4, 0 };
	static GLfloat lightDiffuse0[] = { 1, 1, 1, 1 };
	static GLfloat lightPosition1[] = { -2, 1, 5, 0 };
	static GLfloat lightDiffuse1[] = { 1, 1, 1, 1 };
	static GLfloat lightZeros[] = { 0,0,0,0 };
	ModelerView::draw();

	/* PHRASE 1: Render to hidden buffer */
	// TODO: render to buffer instead of screen
	/*
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeros);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightDiffuse0);
	setDiffuseColor(0, 0, 0);
	glDisable(GL_LIGHT1);
	
	glPushMatrix();
	{
		glTranslated(VAL(XPOS), VAL(YPOS), VAL(ZPOS));

		drawTorso(true);
	}
	glPopMatrix();
	*/

	/* PHRASE 2: Render to screen */
	// setup lights for phrase 2
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightZeros);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse0);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse1);
	
	// draw the floor
	setDiffuseColor(.8f, .8f, .8f);
	glPushMatrix();
	{
		glTranslated(-5, 0, -5);
		drawBox(10, 0.01f, 10);
	}
	glPopMatrix();

	// draw the model
	glPushMatrix();
	{
		glTranslated(VAL(XPOS), VAL(YPOS), VAL(ZPOS));

		drawTorso(false);
	}
	glPopMatrix();

	endDraw();
}
