#define GLEW_STATIC

#include <GL/glew.h>
#include <FL/gl.h>
#include <FL/glut.h>

#include <cmath>
#include <vector>

#include "modelerview.h"
#include "modelerapp.h"
#include "modelerdraw.h"
#include "modelerui.h"
#include "kumaGlobals.h"
#include "kumaModel.h"
#include "bitmap.h"
#include "shaderHelper.h"
#include "mat.h"
#include "camera.h"

using namespace std;

extern void kumaInitControls(ModelerControl* controls);

enum KUMA_BUILTIN_SHADERS
{
	KUMA_PHONG_SHADER = 0,
	KUMA_CEL_SHADER,
	KUMA_PHONG_PROJECTIVE_SHADER,
	KUMA_BUILTIN_SHADER_NUM
};

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
	indicatingColors[KumaModelPart::WAIST] = new float[] {1.0f, 0.62f, 0};

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
	projBitmapFailed = false;
	hasMouseDelta = false;
	lastSelectedPart = KumaModelPart::NONE;
}

int KumaModel::handle(int ev)
{
	static Vec3f prevMousePos(0, 0, 0);
	unsigned eventCoordX = Fl::event_x();
	unsigned eventCoordY = Fl::event_y();
	unsigned eventButton = Fl::event_button();
	unsigned eventState = Fl::event_state();
	KumaModelPart part = KumaModelPart::NONE;
	auto pUI = ModelerApplication::getPUI();

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
					pUI->m_pbrsBrowser->deselect();
					for (int ctrl : (*partControls[part]))
					{
						pUI->m_pbrsBrowser->select(ctrl + 1);
					}
					pUI->m_pbrsBrowser->do_callback();

					prevMousePos[0] = eventCoordX;
					prevMousePos[1] = eventCoordY;
				}

				lastSelectedPart = part;
			}
			break;
		case FL_DRAG:
			if (lastSelectedPart != KumaModelPart::NONE && eventButton == FL_LEFT_MOUSE)
			{
				lastMouseDelta = Vec3f(eventCoordX, eventCoordY, 0.0f) - prevMousePos;
				prevMousePos[0] = eventCoordX;
				prevMousePos[1] = eventCoordY;

				hasMouseDelta = true;
			}
			break;
		default:
			break;
	}

	if (part != KumaModelPart::NONE)
	{
		return 1;
	}
	else
	{
		return ModelerView::handle(ev);
	}
}

void drawTeapot()
{
	glPushMatrix();
	{
		setSpecularColor(1, 1, 1);
		setDiffuseColor(0.8, 0.4, 0.4);
		setShininess(80);
		glTranslated(-2, 1, -2);
		glRotated(50, 0, -1, 0);
		glutSolidTeapot(1);
		setSpecularColor(0, 0, 0);
	}
	glPopMatrix();
}

void printMat(const GLfloat * m)
{
	for (int x = 0; x < 4; ++x)
	{
		for (int y = 0; y < 4; ++y)
		{
			printf("%.2f  ", m[4 * y + x]);
		}
		printf("\n");
	}
	printf("[");
	for (int x = 0; x < 4; ++x)
	{
		
		for (int y = 0; y < 4; ++y)
		{
			printf("%.2f", m[4 * y + x]);
			if (y != 3)printf(",");
		}
		if (x != 3)printf(";");
	}
	printf("]");
	printf("\n\n");
}

Mat4f getViewMat(Vec3f pos, Vec3f lookat, Vec3f up)
{
	Vec3f F(lookat - pos); F.normalize();
	Vec3f normalS = F^up; normalS.normalize();
	Vec3f u = normalS^F; u.normalize();
	Mat4f M(normalS[0], normalS[1], normalS[2], 0,
		u[0], u[1], u[2], 0,
		-F[0], -F[1], -F[2], 0,
		0, 0, 0, 1);
	return M * Mat4f::createTranslation(-pos[0], -pos[1], -pos[2]);
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
	static GLfloat lightAmbient[] = { 0.9, 0.9, 0.9, 1 };
	static GLfloat lightSpecular[] = { 0.1, 0.1, 0.1, 1 };
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
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, lightZeros);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
	setAmbientColor(0, 0, 0);

	static bool shaderStaticInitialized = false;
	static bool shaderLoaded[KUMA_BUILTIN_SHADER_NUM];
	static bool shaderFailed[KUMA_BUILTIN_SHADER_NUM];
	static const char* shaderVertFilenames[KUMA_BUILTIN_SHADER_NUM];
	static const char* shaderFragFilenames[KUMA_BUILTIN_SHADER_NUM];
	static GLhandleARB shaderPrograms[KUMA_BUILTIN_SHADER_NUM];

	if (!shaderStaticInitialized)
	{
		for (int i = 0; i < KUMA_BUILTIN_SHADER_NUM; ++i)
		{
			shaderLoaded[i] = false;
			shaderFailed[i] = false;
		}
		shaderVertFilenames[KUMA_PHONG_SHADER] = "phongshader.vert";
		shaderVertFilenames[KUMA_CEL_SHADER] = "celshader.vert";
		shaderVertFilenames[KUMA_PHONG_PROJECTIVE_SHADER] = "projtext.vert";
		shaderFragFilenames[KUMA_PHONG_SHADER] = "phongshader.frag";
		shaderFragFilenames[KUMA_CEL_SHADER] = "celshader.frag";
		shaderFragFilenames[KUMA_PHONG_PROJECTIVE_SHADER] = "projtext.frag";

		shaderStaticInitialized = true;
	}

	int shaderSelection = ModelerApplication::getPUI()->m_pchoShading->value();
	--shaderSelection; // 0 is default, so minus one

	if (shaderSelection >= 0)
	{
		if (!shaderLoaded[shaderSelection] && !shaderFailed[shaderSelection] &&
			!createProgramWithTwoShaders(shaderVertFilenames[shaderSelection], shaderFragFilenames[shaderSelection], shaderPrograms[shaderSelection]))
		{
			shaderFailed[shaderSelection] = true;
		}
		shaderLoaded[shaderSelection] = true;

		if (!shaderFailed[shaderSelection])
		{
			glUseProgram(shaderPrograms[shaderSelection]);

			if (!projBitmapFailed && shaderSelection == KUMA_PHONG_PROJECTIVE_SHADER && projBitmap == nullptr)
			{
				projBitmap = readBMP("projected_texture.bmp", projBitmapWidth, projBitmapHeight);
				if (projBitmap == NULL)
				{
					printf("Failed loading projected texture bitmap.\n");
					projBitmapFailed = true;
				}
				else
				{
					static GLfloat borderColor[4] = { 1.0, 1.0, 1.0, 1.0 };
					glActiveTexture(GL_TEXTURE0);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
					// gluBuild2DMipmaps(GL_TEXTURE_2D, 3, projBitmapWidth, projBitmapHeight, GL_RGB, GL_UNSIGNED_BYTE, projBitmap);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, projBitmapWidth, projBitmapHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, projBitmap);
				}
			}

			if (shaderSelection == KUMA_PHONG_PROJECTIVE_SHADER && !projBitmapFailed)
			{
				auto pUI = ModelerApplication::Instance()->getPUI();
				Vec3f projPos(-pUI->m_projTextPosX->value(), -pUI->m_projTextPosY->value(), -pUI->m_projTextPosZ->value());
				Vec3f projAt(-pUI->m_projTextAtX->value(), -pUI->m_projTextAtY->value(), -pUI->m_projTextAtZ->value());
				Vec3f projUp(0, 1, 0);
				projUp.normalize();
				GLfloat M_t[16];
				// proj view mat
				Mat4f M = getViewMat(projPos, projAt, projUp);
				M.getGLMatrix(M_t);
				Mat4f MProjView(M_t[0], M_t[4], M_t[8], M_t[12], M_t[1], M_t[5], M_t[9], M_t[13], M_t[2], M_t[6], M_t[10], M_t[14], M_t[3], M_t[7], M_t[11], M_t[15]);
				// proj proj mat
				glPushMatrix();
				{
					glLoadIdentity();
					gluPerspective(15.0f, 1.0f, 1.0f, 100.0f);
					glGetFloatv(GL_MODELVIEW_MATRIX, M_t);
				}
				glPopMatrix();
				Mat4f MProj(M_t[0], M_t[4], M_t[8], M_t[12], M_t[1], M_t[5], M_t[9], M_t[13], M_t[2], M_t[6], M_t[10], M_t[14], M_t[3], M_t[7], M_t[11], M_t[15]);
				// proj bias mat
				Mat4f MSB = Mat4f::createTranslation(0.5, 0.5, 0.5) * Mat4f::createScale(0.5, 0.5, 0.5);

				(MSB * MProj * MProjView).getGLMatrix(M_t);

				// compute view inverse
				GLfloat M_viewinv[16];
				Mat4f MView = getViewMat(m_camera->getPosition(), m_camera->getLookAt(), m_camera->getUpVector());
				MView = MView.inverse();
				MView.getGLMatrix(M_viewinv);

				glEnable(GL_TEXTURE_2D);

				glUniformMatrix4fvARB(glGetUniformLocationARB(shaderPrograms[shaderSelection], "projMatrix"), 1, GL_FALSE, M_t);
				glUniformMatrix4fvARB(glGetUniformLocationARB(shaderPrograms[shaderSelection], "viewInv"), 1, GL_FALSE, M_viewinv);
				glUniform1iARB(glGetUniformLocationARB(shaderPrograms[shaderSelection], "textureSampler"), 0);
				glUniform4fARB(glGetUniformLocationARB(shaderPrograms[shaderSelection], "projPos"), projPos[0], projPos[1], projPos[2], 1);
			}

			if (ModelerApplication::getPUI()->m_pbtnTeapot->value() > 0)
				drawTeapot();
			drawModel(false);

			glDisable(GL_TEXTURE_2D);

			glUseProgram(0);
		}
	}
	else
	{
		if (ModelerApplication::getPUI()->m_pbtnTeapot->value() > 0)
			drawTeapot();
		drawModel(false);
	}

	endDraw();
}
