//
// CurvedBrush.cpp
//
// The implementation of Point Brush. It is a kind of ImpBrush. All your brush implementations
// will look like the file with the different GL primitive calls.
//

#include "impressionistDoc.h"
#include "impressionistUI.h"
#include "curvedbrush.h"
#include <iostream>
extern float frand();

CurvedBrush::CurvedBrush(ImpressionistDoc* pDoc, char* name) :
ImpBrush(pDoc, name)
{
}

void CurvedBrush::BrushBegin(const Point source, const Point target)
{
	BrushMove(source, target);
}

void CurvedBrush::BrushMove(const Point source, const Point target)
{
	ImpressionistDoc* pDoc = GetDocument();
	ImpressionistUI* dlg = pDoc->m_pUI;

	if (pDoc == NULL) {
		printf("PointBrush::BrushMove  document is NULL\n");
		return;
	}

	radius = pDoc->getSize();
	minStrokeLength=4;
	maxStrokeLength=16;
	curvatureFilter=0.9;
	glBegin(GL_POLYGON);
	SetColor(source);

	for (int i = 0; i < 360; ++i)
	{
		double theta = i * 3.14159 / 180;
		glVertex2d(target.x - radius * cos(theta), target.y - radius * sin(theta));
	}
	glEnd();
	int x = source.x;
	int y = source.y;
	double dx = 0, dy = 0, lastDx = 0, lastDy = 0;

	for (int i = 0; i < maxStrokeLength; ++i)
	{
		if (i > minStrokeLength)
		{
			GLubyte originColor[3];
			GLubyte brushColor[3];
			GLubyte canvasColor[3];

			memcpy(originColor, pDoc->GetOriginalPixel(x, y), 3);
			memcpy(brushColor, pDoc->GetOriginalPixel(source.x, source.y), 3);
			memcpy(canvasColor, (GLubyte*)(pDoc->m_ucPainting + 4 * (y*pDoc->m_nWidth + x)), 3);

			int drawDiff = sqrt(pow(originColor[0] - brushColor[0], 2) + pow(originColor[1] - brushColor[1], 2) + pow(originColor[2] - brushColor[2], 2));
			int currentDiff = sqrt(pow(originColor[0] - canvasColor[0], 2) + pow(originColor[1] - canvasColor[1], 2) + pow(originColor[2] - canvasColor[2], 2));
			if (currentDiff < drawDiff)
			{
				break;
			}
		}

		// Detect vanishing gradient
		if (pDoc->m_iGradientMagnitude[y*pDoc->m_nWidth + x] == 0)
		{
			break;
		}

		// Get unit vector of gradient
		double gx = pDoc->m_iGradient[2 * (y*pDoc->m_nWidth + x)];
		double gy = pDoc->m_iGradient[2 * (y*pDoc->m_nWidth + x) + 1];
		// Compute normalize direction
		double dx = -gy / sqrt(gx * gx + gy * gy);
		double dy = gx / sqrt(gx * gx + gy * gy);

		// If necessary, reverse direction
		if (lastDx * dx + lastDy * dy < 0)
		{
			dx = -dx;
			dy = -dy;
		}

		// Filter the stroke direction
		dx = curvatureFilter * dx + (1 - curvatureFilter) * lastDx;
		dy = curvatureFilter * dy + (1 - curvatureFilter) * lastDy;
		gx = dx / sqrt(dx*dx + dy*dy);
		gy = dy / sqrt(dx*dx + dy*dy);
		dx = gx;
		dy = gy;
		x = x + radius * dx;
		y = y + radius * dy;
		if (x < 0)
			x = 0;
		else if (x >= pDoc->m_nWidth) 
			x = (pDoc->m_nWidth - 1);
		if (y < 0)
			y = 0;
		else if (y >= pDoc->m_nHeight)
			y = (pDoc->m_nHeight - 1);
		lastDx = dx;
		lastDy = dy;
		glBegin(GL_POLYGON);
		SetColor(source);
		for (int i = 0; i < 360; ++i)
		{
			double theta = i * 3.14159 / 180;
			glVertex2d(x - radius * cos(theta), y - radius * sin(theta));
		}
		glEnd();
	}
}

void CurvedBrush::BrushEnd(const Point source, const Point target)
{
	// do nothing so far
}
