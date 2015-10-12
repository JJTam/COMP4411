#include "kumaModel.h"
#include <FL/gl.h>
#include "modelerapp.h"
#include "modelerglobals.h"
#include "modelerui.h"

void KumaModel::drawTexture()
{
	auto pui = ModelerApplication::Instance()->GetPUI();
	static int width = 0;
	static int height = 0;

	if (image == NULL || pui->hasNewTexture)
	{
		pui->hasNewTexture = false;

		// try to open the image to read
		unsigned char*	imagedata;

		if (pui->textureFileName == nullptr ||
			(imagedata = readBMP(pui->textureFileName, width, height)) == NULL)
		{
			fl_alert("Can't load bitmap file");
			SETVAL(DRAW_TEXTURE, 0);
			return;
		}

		image = imagedata;
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glShadeModel(GL_FLAT);
		glEnable(GL_DEPTH_TEST);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(1, &texName);
		glBindTexture(GL_TEXTURE_2D, texName);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	}

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (width > 0 && height > 0)
	{
		double hwfactor = height / (double)width;
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glBindTexture(GL_TEXTURE_2D, texName);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(-2.0, 0.0, 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3f(-2.0, 2.0 * hwfactor, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3f(0.0, 2.0 * hwfactor, 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
		glEnd();
		glFlush();
		glDisable(GL_TEXTURE_2D);
	}
}