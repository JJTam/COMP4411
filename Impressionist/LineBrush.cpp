//
// LineBrush.cpp
//
// The implementation of Line Brush. It is a kind of ImpBrush. All your brush implementations
// will look like the file with the different GL primitive calls.
//

#include "impressionistDoc.h"
#include "impressionistUI.h"
#include "LineBrush.h"
#include <cmath>

extern float frand();

LineBrush::LineBrush(ImpressionistDoc* pDoc, char* name) :
ImpBrush(pDoc, name)
{
}

void LineBrush::BrushBegin(const Point source, const Point target)
{
	ImpressionistDoc* pDoc = GetDocument();
	ImpressionistUI* dlg = pDoc->m_pUI;

	this->length = pDoc->getSize();
	this->angle = pDoc->getAngle();
	int length = pDoc->getLineWidth();
	glLineWidth(length);
	BrushMove(source, target);
}

void LineBrush::BrushMove(const Point source, const Point target)
{
	ImpressionistDoc* pDoc = GetDocument();
	ImpressionistUI* dlg = pDoc->m_pUI;

	if (pDoc == NULL) {
		printf("LineBrush::BrushMove  document is NULL\n");
		return;
	}

	glBegin(GL_LINES);
	SetColor(source);

	glVertex2f(target.x - this->length / 2 * cos(this->angle * 3.14159 / 180), target.y - this->length / 2 * sin(this->angle * 3.14159 / 180));
	glVertex2f(target.x + this->length / 2 * cos(this->angle * 3.14159 / 180), target.y + this->length / 2 * sin(this->angle * 3.14159 / 180));

	glEnd();
}

void LineBrush::BrushEnd(const Point source, const Point target)
{
	// do nothing so far
}