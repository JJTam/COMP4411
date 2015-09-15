#include "impressionistDoc.h"
#include "impressionistUI.h"
#include "CircleBrush.h"
#include <cmath>

extern float frand();

CircleBrush::CircleBrush(ImpressionistDoc* pDoc, char* name) : ImpBrush(pDoc, name)
{

}

void CircleBrush::BrushBegin(const Point source, const Point target)
{
	BrushMove(source, target);
}

void CircleBrush::BrushMove(const Point source, const Point target)
{
	ImpressionistDoc* pDoc = GetDocument();
	ImpressionistUI* dlg = pDoc->m_pUI;

	if (pDoc == NULL)
	{
		printf("CircleBrush::BrushMove document is NULL\n");
		return;
	}

	int size = pDoc->getSize();
	this->radius = (double)size;

	glBegin(GL_POLYGON);
	SetColor(source);
	
	for (int i = 0; i < 36; ++i)
	{
		double theta = i * 10 * 3.14159 / 180;
		glVertex2d(target.x - radius * cos(theta), target.y - radius * sin(theta));
	}

	glEnd();
}

void CircleBrush::BrushEnd(const Point source, const Point target)
{

}
