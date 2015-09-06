//
// LineBrush.cpp
//
// The implementation of Scattered Line Brush. It is a kind of ImpBrush. All your brush implementations
// will look like the file with the different GL primitive calls.
//

#include "impressionistDoc.h"
#include "impressionistUI.h"
#include "ScatteredLineBrush.h"
#include <cmath>

extern float frand();
extern int irand(int max);
ScatteredLineBrush::ScatteredLineBrush(ImpressionistDoc* pDoc, char* name) :
ImpBrush(pDoc, name)
{
}

void ScatteredLineBrush::BrushBegin(const Point source, const Point target)
{
	ImpressionistDoc* pDoc = GetDocument();
	ImpressionistUI* dlg = pDoc->m_pUI;

	this->length = pDoc->getSize();
	this->angle = pDoc->getAngle();
	int length = pDoc->getLineWidth();
	glLineWidth(length);
	BrushMove(source, target);
}

void ScatteredLineBrush::BrushMove(const Point source, const Point target)
{
	ImpressionistDoc* pDoc = GetDocument();
	ImpressionistUI* dlg = pDoc->m_pUI;

	if (pDoc == NULL) {
		printf("LineBrush::BrushMove  document is NULL\n");
		return;
	}

	glBegin(GL_LINES);
	int loop_time = irand(2) + 2;
	for (int i = 0; i < loop_time; ++i)
	{
		int Xoffset = irand(10) - 5;
		int Yoffset = irand(10) - 5;
		SetColor(source.x + Xoffset, source.y + Yoffset);
		glVertex2f(target.x + Xoffset - this->length / 2 * cos(this->angle * 3.14159 / 180), target.y + Yoffset - this->length / 2 * sin(this->angle * 3.14159 / 180));
		glVertex2f(target.x + Xoffset + this->length / 2 * cos(this->angle * 3.14159 / 180), target.y + Yoffset + this->length / 2 * sin(this->angle * 3.14159 / 180));
	}
	glEnd();
}

void ScatteredLineBrush::BrushEnd(const Point source, const Point target)
{
	// do nothing so far
}