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
#include "mat.h"
#include "camera.h"

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
	indicatingColors[KumaModelPart::WAIST] = new float[] {1.0f, 0.58f, 0};

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
	partNames[KumaModelPart::WAIST] = "Waist";

	partControls[KumaModelPart::NONE] = new list<int>{ 0 };
	partControls[KumaModelPart::TORSO] = new list<int>{ XPOS, YPOS, ZPOS };
	partControls[KumaModelPart::HEAD] = new list<int>{ HEAD_ROTATION_X, HEAD_ROTATION_Y, HEAD_ROTATION_Z };
	partControls[KumaModelPart::LEFT_ARM_UPPER] = new list<int>{ LEFT_UPPER_ARM_ROTATION_X, LEFT_UPPER_ARM_ROTATION_Y, LEFT_UPPER_ARM_ROTATION_Z };
	partControls[KumaModelPart::LEFT_ARM_LOWER] = new list<int>{ LEFT_LOWER_ARM_ROTATION_X };
	partControls[KumaModelPart::RIGHT_ARM_UPPER] = new list<int>{ RIGHT_UPPER_ARM_ROTATION_X, RIGHT_UPPER_ARM_ROTATION_Y, RIGHT_UPPER_ARM_ROTATION_Z };
	partControls[KumaModelPart::RIGHT_ARM_LOWER] = new list<int>{ RIGHT_LOWER_ARM_ROTATION_X };
	partControls[KumaModelPart::LEFT_LEG_UPPER] = new list<int>{ LEFT_UPPER_LEG_ROTATION_X, LEFT_UPPER_LEG_ROTATION_Y, LEFT_UPPER_LEG_ROTATION_Z };
	partControls[KumaModelPart::LEFT_LEG_LOWER] = new list<int>{ LEFT_LOWER_LEG_ROTATION_X };
	partControls[KumaModelPart::RIGHT_LEG_UPPER] = new list<int>{ RIGHT_UPPER_LEG_ROTATION_X, RIGHT_UPPER_LEG_ROTATION_Y, RIGHT_UPPER_LEG_ROTATION_Z };
	partControls[KumaModelPart::RIGHT_LEG_LOWER] = new list<int>{ RIGHT_LOWER_LEG_ROTATION_X };
	partControls[KumaModelPart::WAIST] = new list<int>{ WAIST_ROTATION_X, WAIST_ROTATION_Y, WAIST_ROTATION_Z };

	hiddenBuffer = nullptr;
	projBitmap = nullptr;
	useProjTexture = true;
	lastSelectedPart = KumaModelPart::NONE;
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
				// determine which part of the model is clicked

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

				// activate the coresponding curves
				if (part != KumaModelPart::NONE)
				{
					auto pUI = ModelerApplication::getPUI();
					pUI->m_pbrsBrowser->deselect();
					for (int ctrl : (*partControls[part]))
					{
						pUI->m_pbrsBrowser->select(ctrl + 1);
					}
					pUI->m_pbrsBrowser->do_callback();
				}

				lastSelectedPart = part;

				// printf("val = %.2f, ref = %d, mindiff = %.2f, you clicked on %s.\n", val, refIndex, mindiff, partNames[part].c_str());
				// printf("You clicked on %s.\n", partNames[part].c_str());
			}
			break;
		default:
			break;
	}

	return ModelerView::handle(ev);
}

void KumaModel::drawScene(bool useIndicatingColor)
{
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

	static GLfloat textureXform[16];

	if (useProjTexture && projBitmap == nullptr)
	{
		projBitmap = readBMP("projected_texture.bmp", projBitmapWidth, projBitmapHeight);
		if (projBitmap == nullptr)
		{
			printf("Failed loading projected texture bitmap.\n");
			useProjTexture = false;
		}
		else
		{
			static GLfloat borderColor[4] = { 1.0, 1.0, 1.0, 1.0 };
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, projBitmapWidth, projBitmapHeight, GL_RGB, GL_UNSIGNED_BYTE, projBitmap);
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		}
	}

	if (useProjTexture && projBitmap != nullptr)
	{
		auto pUI = ModelerApplication::getPUI();
		Vec3f projPos(m_camera->getPosition() / 2);
		Vec3f projAt(m_camera->getLookAt() / 2);
		Vec3f projUp(m_camera->getUpVector());
		projUp.normalize();

		GLfloat M_t[16];
		Vec3f F(projAt - projPos); F.normalize();
		Vec3f normalS = F^projUp; normalS.normalize();
		Vec3f u = normalS^F; u.normalize();
		Mat4f M(normalS[0], normalS[1], normalS[2], 0,
			u[0], u[1], u[2], 0,
			-F[0], -F[1], -F[2], 0,
			0, 0, 0, 1);
		glGetFloatv(GL_MODELVIEW, M_t);

		M = Mat4f::createScale(0.5, 0.5, 0.5) * M;
		M = Mat4f::createTranslation(0.5, 0.5, 0.5) * M;
		M = M.transpose();
		M.getGLMatrix(M_t);

		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glRotated(M_PI, 0, 1, 0);
		glTranslatef(projAt[0] , -projAt[1] , projAt[2]);
		
		glMatrixMode(GL_MODELVIEW);

		glTexGenfv(GL_S, GL_EYE_PLANE, M_t);
		glTexGenfv(GL_T, GL_EYE_PLANE, M_t + 4);
		glTexGenfv(GL_R, GL_EYE_PLANE, M_t + 8);
		glTexGenfv(GL_Q, GL_EYE_PLANE, M_t + 12);

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glEnable(GL_TEXTURE_GEN_Q);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
		drawScene(false);

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		glDisable(GL_TEXTURE_GEN_Q);

		glMatrixMode(GL_TEXTURE);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}
	else
	{
		drawScene(false);
	}
	endDraw();
}
