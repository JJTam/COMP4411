#ifndef _KUMAMODEL_H
#define _KUMAMODEL_H

#include <FL/gl.h>
#include "modelerview.h"

class KumaModel : public ModelerView
{
public:
	KumaModel(int x, int y, int w, int h, char *label)
		: ModelerView(x, y, w, h, label) { }
	GLuint texName;
	unsigned char * image = NULL;
	virtual void draw();
	void drawTexture();
};

#endif // _KUMAMODEL_H