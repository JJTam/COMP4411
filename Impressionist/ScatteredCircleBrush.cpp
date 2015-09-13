#include "impressionistDoc.h"
#include "impressionistUI.h"
#include "ScatteredCircleBrush.h"
#include <cmath>

extern float frand();
extern int irand(int);

ScatteredCircleBrush::ScatteredCircleBrush(ImpressionistDoc* pDoc, char* name) : ImpBrush(pDoc, name)
{

}

void ScatteredCircleBrush::BrushBegin(const Point source, const Point target)
{
	BrushMove(source, target);
}

void ScatteredCircleBrush::BrushMove(const Point source, const Point target)
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

	int loop_time = irand(2) + 2;
	for (int i = 0; i < loop_time; ++i)
	{
		glBegin(GL_POLYGON);
		int Xoffset = irand(this->radius) - this->radius/2;
		int Yoffset = irand(this->radius) - this->radius/2;
		SetColor(source.x + Xoffset,source.y + Yoffset);

		for (int i = 0; i < 360; ++i)
		{
			double theta = i * 3.14159 / 180;
			glVertex2d(target.x + Xoffset - radius * cos(theta), target.y + Yoffset - radius * sin(theta));
		}

		glEnd();
	}
}

void ScatteredCircleBrush::BrushEnd(const Point source, const Point target)
{

}