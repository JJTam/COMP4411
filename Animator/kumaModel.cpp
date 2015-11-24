#define GLEW_STATIC
#include <GL/glew.h>
#include <FL/gl.h>
#include "modelerview.h"
#include "modelerapp.h"
#include "modelerdraw.h"
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
	// MAGIC NUMBERS! DO NOT MODIFY!!
	indicatingColors[KumaModelPart::NONE] = new float[] { 0.0f, 0, 0 };
	indicatingColors[KumaModelPart::TORSO] = new float[] { 0.12f, 0, 0 };
	indicatingColors[KumaModelPart::HEAD] = new float[] { 0.24f, 0, 0 };
	indicatingColors[KumaModelPart::LEFT_ARM_UPPER] = new float[] { 0.36f, 0, 0 };
	indicatingColors[KumaModelPart::LEFT_ARM_LOWER] = new float[] { 0.48f, 0, 0 };
	indicatingColors[KumaModelPart::RIGHT_ARM_UPPER] = new float[] { 0.58f, 0, 0 };
	indicatingColors[KumaModelPart::RIGHT_ARM_LOWER] = new float[] { 0.70f, 0, 0 };
	indicatingColors[KumaModelPart::LEFT_LEG_UPPER] = new float[] { 0.84f, 0.12f, 0 };
	indicatingColors[KumaModelPart::LEFT_LEG_LOWER] = new float[] { 1.0f, 0.24f, 0 };
	indicatingColors[KumaModelPart::RIGHT_LEG_UPPER] = new float[] { 1.0f, 0.36f, 0 };
	indicatingColors[KumaModelPart::RIGHT_LEG_LOWER] = new float[] { 1.0f, 0.48f, 0 };

	partNames[KumaModelPart::NONE] = "Air";
	partNames[KumaModelPart::TORSO] = "Torso";
	partNames[KumaModelPart::HEAD] = "Head";
	partNames[KumaModelPart::LEFT_ARM_UPPER] = "Left upper arm";
	partNames[KumaModelPart::LEFT_ARM_LOWER] = "Left lower arm";
	partNames[KumaModelPart::RIGHT_ARM_UPPER] = "Right upper arm";
	partNames[KumaModelPart::RIGHT_ARM_LOWER] = "Right lower arm";
	partNames[KumaModelPart::LEFT_LEG_UPPER] = "Left upper leg";
	partNames[KumaModelPart::LEFT_LEG_LOWER] = "Left lower leg";
	partNames[KumaModelPart::RIGHT_LEG_UPPER] = "Right upper leg";
	partNames[KumaModelPart::RIGHT_LEG_LOWER] = "Right lower leg";
	hiddenBuffer = nullptr;
}

int KumaModel::handle(int ev)
{
	unsigned eventCoordX = Fl::event_x();
	unsigned eventCoordY = Fl::event_y();
	unsigned eventButton = Fl::event_button();
	unsigned eventState = Fl::event_state();

	switch (ev)
	{
		case FL_PUSH:
			if (eventButton == FL_LEFT_MOUSE && hiddenBuffer != nullptr)
			{
				int offset = (eventCoordX + (h() - eventCoordY) * w()) * 3;
				double val = hiddenBuffer[offset] / 255.0;
				int refIndex = 0;
				if (val > 0.9)
				{
					val = hiddenBuffer[offset + 1] / 255.0;
					refIndex = 1;
				}

				double mindiff = 100;
				KumaModelPart part = KumaModelPart::NONE;
				for (auto pair : indicatingColors)
				{
					double diff = val - pair.second[refIndex];
					if (diff > 0 && diff < mindiff)
					{
						mindiff = diff;
						part = pair.first;
					}
				}

				// printf("val = %.2f, ref = %d, mindiff = %.2f, you clicked on %s.\n", val, refIndex, mindiff, partNames[part].c_str());
				printf("You clicked on %s.\n", partNames[part].c_str());
			}
			break;
		default:
			break;
	}

	return ModelerView::handle(ev);
}

// Override draw() to draw out Kuma
void KumaModel::draw()
{
	updateParameters();

	static bool glewInitialized = false;
	static bool glewInitializationFailed = false;
	static int prevW = -1;
	static int prevH = -1;

	static GLfloat lightPosition0[] = { 4, 2, -4, 0 };
	static GLfloat lightDiffuse0[] = { 1, 1, 1, 1 };
	static GLfloat lightPosition1[] = { -2, 1, 5, 0 };
	static GLfloat lightDiffuse1[] = { 1, 1, 1, 1 };
	static GLfloat lightZeros[] = { 0, 0, 0, 0 };
	static GLfloat lightAmbient[] = { 0.9, 0.9, 0.9, 0.9 };
	ModelerView::draw();

	int drawWidth = w();
	int drawHeight = h();

	if (!glewInitialized && !glewInitializationFailed)
	{
		GLenum err = glewInit();
		if (err != GLEW_OK)
		{
			glewInitializationFailed = true;
			printf("glewInit() failed!\n");
			printf("Error: %s\n", glewGetErrorString(err));
		}
		else
		{
			glewInitialized = true;
		}
	}

	if (glewInitialized)
	{
		if (prevW != drawWidth || prevH != drawHeight)
		{
			if (hiddenBuffer != nullptr)
				delete[] hiddenBuffer;
			hiddenBuffer = new GLubyte[drawWidth * drawHeight * 3];
		}

		if (prevW > 0)
		{
			glDeleteRenderbuffers(1, &render_buf);
			glDeleteRenderbuffers(1, &depth_buf);
			glDeleteFramebuffers(1, &fbo);
		}

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenRenderbuffers(1, &render_buf);
		glBindRenderbuffer(GL_RENDERBUFFER, render_buf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, drawWidth, drawHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, render_buf);

		glGenRenderbuffers(1, &depth_buf);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_buf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, drawWidth, drawHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buf);
		prevW = drawWidth;
		prevH = drawHeight;
	}

	/* PHRASE 1: Render to off-screen buffer */
	
	if (glewInitialized)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Warning: DRAW_FRAMEBUFFER is not complete!\n");
		}

		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeros);
		glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
		setDiffuseColor(0, 0, 0);
		glDisable(GL_LIGHT1);

		glPushMatrix();
		{
			glTranslated(VAL(XPOS), VAL(YPOS), VAL(ZPOS));

			drawTorso(true);
		}
		glPopMatrix();

		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(0, 0, drawWidth, drawHeight, GL_RGB, GL_UNSIGNED_BYTE, hiddenBuffer);
	}
	
	/* PHRASE 2: Render to screen */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// setup lights for phrase 2
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightZeros);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse0);
	glLightfv(GL_LIGHT1, GL_AMBIENT, lightZeros);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse1);
	setAmbientColor(0, 0, 0);

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
