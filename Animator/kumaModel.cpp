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

	ikTarget[KumaModelPart::LEFT_ARM_LOWER] = Vec3f(0, 0, 0);
	ikTarget[KumaModelPart::RIGHT_ARM_LOWER] = Vec3f(0, 0, 0);
	
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

				GLfloat Mtmp[16];
				glGetFloatv(GL_MODELVIEW_MATRIX, Mtmp);
				Mat4f MM(Mtmp[0], Mtmp[4], Mtmp[8], Mtmp[12], Mtmp[1], Mtmp[5], Mtmp[9], Mtmp[13], Mtmp[2], Mtmp[6], Mtmp[10], Mtmp[14], Mtmp[3], Mtmp[7], Mtmp[11], Mtmp[15]);
				MM = MM.inverse();

				lastMouseDeltaInWorld[0] = lastMouseDelta[0];
				lastMouseDeltaInWorld[1] = lastMouseDelta[1];
				lastMouseDeltaInWorld[2] = lastMouseDeltaInWorld[3] = 0;
				lastMouseDeltaInWorld = MM * lastMouseDeltaInWorld;
				lastMouseDeltaInWorld = lastMouseDeltaInWorld * abs(MM[3][3]) * 0.05f;

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

Mat4f fromGLMat(GLfloat* glmat)
{
	return Mat4f(glmat[0], glmat[4], glmat[8], glmat[12], glmat[1], glmat[5], glmat[9], glmat[13], glmat[2], glmat[6], glmat[10], glmat[14], glmat[3], glmat[7], glmat[11], glmat[15]);
}

bool createShadowFBO(GLuint& fboID, GLuint& depthTextureID, int width, int height)
{
	glGenTextures(1, &depthTextureID);
	glBindTexture(GL_TEXTURE_2D, depthTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffersEXT(1, &fboID);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboID);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depthTextureID, 0);

	GLenum fbostatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	bool ok = true;
	if (fbostatus != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		glDeleteTextures(1, &depthTextureID);
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	return ok;
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
			printf("Shader loading failed!\n");
		}
		shaderLoaded[shaderSelection] = true;

		if (!shaderFailed[shaderSelection])
		{
			glUseProgram(shaderPrograms[shaderSelection]);

			static GLuint shadowTextureID;
			static GLuint shadowFboID;
			static bool shadowSuccess = false;

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
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, projBitmapWidth, projBitmapHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, projBitmap);
				}
			}

			if (shaderSelection == KUMA_PHONG_PROJECTIVE_SHADER && !projBitmapFailed)
			{
				glUseProgramObjectARB(0);
				GLfloat M_temp[16];

				// projector parameters
				auto pUI = ModelerApplication::Instance()->getPUI();
				Vec3f projPos(-pUI->m_projTextPosX->value(), -pUI->m_projTextPosY->value(), -pUI->m_projTextPosZ->value());
				Vec3f projAt(-pUI->m_projTextAtX->value(), -pUI->m_projTextAtY->value(), -pUI->m_projTextAtZ->value());
				Vec3f projUp(0, 1, 0);

				// projector view matrix
				Mat4f MProjView = getViewMat(projPos, projAt, projUp);

				// projector proj mat
				glPushMatrix();
				{
					glLoadIdentity();
					gluPerspective(15.0f, 1.0f, 1.0f, 100.0f);
					glGetFloatv(GL_MODELVIEW_MATRIX, M_temp);
				}
				glPopMatrix();
				Mat4f MProj = fromGLMat(M_temp);

				// projection bias mat
				Mat4f MSB = Mat4f::createTranslation(0.5, 0.5, 0.5) * Mat4f::createScale(0.5, 0.5, 0.5);

				// projection view and MVP mat
				GLfloat M_projView[16];
				MProjView.getGLMatrix(M_projView);
				GLfloat M_projMVP[16];
				(MSB * MProj * MProjView).getGLMatrix(M_projMVP);

				// compute camera view inverse
				GLfloat M_viewinv[16];
				Mat4f MView = getViewMat(m_camera->getPosition(), m_camera->getLookAt(), m_camera->getUpVector());
				MView = MView.inverse();
				MView.getGLMatrix(M_viewinv);

				// render from the view of projector
				shadowSuccess = createShadowFBO(shadowFboID, shadowTextureID, drawWidth, drawHeight);
				if (shadowSuccess)
				{
					glMatrixMode(GL_PROJECTION);
					glPushMatrix();
					glMatrixMode(GL_MODELVIEW);
					glPushMatrix();
					{
						glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, shadowFboID);
						glMatrixMode(GL_PROJECTION);
						glLoadIdentity();
						gluPerspective(15.0f, 1.0f, 1.0f, 100.0f);

						glMatrixMode(GL_MODELVIEW);
						glLoadIdentity();
						glMultMatrixf(M_projView);
						glDepthMask(GL_TRUE);
						glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
						glCullFace(GL_FRONT);

						if (ModelerApplication::getPUI()->m_pbtnTeapot->value() > 0)
							drawTeapot();
						drawModel(false);

						glMatrixMode(GL_MODELVIEW);
						glCullFace(GL_BACK);
						glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
						glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
						
					}
					glPopMatrix();
					glMatrixMode(GL_PROJECTION);
					glPopMatrix();
					glMatrixMode(GL_MODELVIEW);

					glEnable(GL_TEXTURE_2D);
					glActiveTextureARB(GL_TEXTURE7);
					glBindTexture(GL_TEXTURE_2D, shadowTextureID);
					glActiveTextureARB(GL_TEXTURE0);
				}
				else
				{
					printf("Failed generating shadow texture.\n");
				}

				glUseProgramObjectARB(shaderPrograms[shaderSelection]);
				glUniformMatrix4fvARB(glGetUniformLocationARB(shaderPrograms[shaderSelection], "projMatrix"), 1, GL_FALSE, M_projMVP);
				glUniformMatrix4fvARB(glGetUniformLocationARB(shaderPrograms[shaderSelection], "viewInv"), 1, GL_FALSE, M_viewinv);
				glUniform1iARB(glGetUniformLocationARB(shaderPrograms[shaderSelection], "textureSampler"), 0);
				glUniform1iARB(glGetUniformLocationARB(shaderPrograms[shaderSelection], "shadowSampler"), 7);
				glUniform1iARB(glGetUniformLocationARB(shaderPrograms[shaderSelection], "useShadow"), shadowSuccess ? 1 : 0);
				glUniform4fARB(glGetUniformLocationARB(shaderPrograms[shaderSelection], "projPos"), projPos[0], projPos[1], projPos[2], 1);
			}

			if (ModelerApplication::getPUI()->m_pbtnTeapot->value() > 0)
				drawTeapot();
			drawModel(false);

			if (shadowSuccess)
			{
				glDeleteFramebuffers(1, &shadowFboID);
				glDeleteTextures(1, &shadowTextureID);
				shadowSuccess = false;
			}

			glDisable(GL_TEXTURE_2D);
			glUseProgramObjectARB(0);
		}
	}
	else
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (ModelerApplication::getPUI()->m_pbtnTeapot->value() > 0)
			drawTeapot();
		drawModel(false);
	}

	endDraw();
}
