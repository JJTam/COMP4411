#include "impressionistDoc.h"
#include "impressionistUI.h"
#include "liquifybrush.h"
#include <iostream>
extern float frand();

LiquifyBrush::LiquifyBrush(ImpressionistDoc* pDoc, char* name) :
ImpBrush(pDoc, name)
{
}

void LiquifyBrush::BrushBegin(const Point source, const Point target)
{
	BrushMove(source, target);
}

void LiquifyBrush::BrushMove(const Point source, const Point target)
{
	ImpressionistDoc* pDoc = GetDocument();
	ImpressionistUI* dlg = pDoc->m_pUI;

	if (pDoc == NULL) {
		printf("PointBrush::BrushMove  document is NULL\n");
		return;
	}

	int size = pDoc->getSize();
	int c = 6;

	int height = pDoc->m_nHeight;
	int width = pDoc->m_nWidth;
	unsigned char* originPainting = pDoc->m_ucPreservedPainting;

	GLubyte color[4];
	glPointSize(1);
	glBegin(GL_POINTS);
	for (int y = - size; y <= + size; ++y)
	{
		for (int x = - size; x <= + size; ++x)
		{
			// Check if the pixel inside circle
			if (x*x + y*y <= size*size)
			{

				// Get pixel array position
				if (target.x + x < 0 || target.x + x >= width || target.y + y < 0 || target.y + y >= height)
				{
					continue;
				}

				// Transform the pixel Cartesian coordinates (x, y) to polar coordinates (r, alpha)
				double r = sqrt(x*x + y*y);
				double alpha = atan2(y, x);
				double degrees = (alpha*180.0) / 3.14159;

				double interpolationFactor = r / size;
				r = interpolationFactor * r + (1.0 - interpolationFactor) * c * sqrt(r);
				// Transform back from polar coordinates to Cartesian 
				alpha = (degrees * 3.14159) / 180.0;
				int newY = r * sin(alpha);
				int newX = r * cos(alpha);

				// Get the new pixel location
				if (target.x + newX < 0 || target.x + newX >= width || target.y + newY < 0 || target.y + newY >= height)
				{
					continue;
				}
				int sourcePosition = (newY + target.y) * width + newX + target.x;
				sourcePosition *= 4;
				
				memcpy(color, originPainting + sourcePosition, 4);
				glColor4ubv(color);
				glVertex2d(target.x + x, target.y + y);
			}
		}
	}
	glEnd();
}

void LiquifyBrush::BrushEnd(const Point source, const Point target)
{
	// do nothing so far
}

