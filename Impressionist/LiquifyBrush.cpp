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
	unsigned char* originPainting = new unsigned char[height * width * 4];
	memcpy(originPainting, pDoc->m_ucPainting, height * width * 4);

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
				int destPosition = (y + target.y)*width + x + target.x;
				destPosition *= 4;

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
				pDoc->m_ucPainting[destPosition + 0] = originPainting[sourcePosition + 0];
				pDoc->m_ucPainting[destPosition + 1] = originPainting[sourcePosition + 1];
				pDoc->m_ucPainting[destPosition + 2] = originPainting[sourcePosition + 2];
				pDoc->m_ucPainting[destPosition + 3] = originPainting[sourcePosition + 3];
			}
		}
	}

	glDisable(GL_BLEND);
	glClearColor(0.7f, 0.7f, 0.7f, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	int drawHeight = min(pDoc->m_pUI->m_paintView->h(), pDoc->m_nPaintHeight);
	int drawWidth = min(pDoc->m_pUI->m_paintView->w(), pDoc->m_nPaintWidth);
	int startrow = pDoc->m_nPaintHeight - drawHeight;
	if (startrow < 0) startrow = 0;
	int paintBitOffset = 4 * (pDoc->m_nPaintWidth * startrow);
	GLvoid* m_pPreservedPaintBitstart = pDoc->m_ucPainting + paintBitOffset;

	glRasterPos2i(0, pDoc->m_pUI->m_paintView->h() - drawHeight);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, pDoc->m_nWidth);
	glDrawPixels(drawWidth, drawHeight, GL_RGBA, GL_UNSIGNED_BYTE, m_pPreservedPaintBitstart);
}

void LiquifyBrush::BrushEnd(const Point source, const Point target)
{
	// do nothing so far
}

