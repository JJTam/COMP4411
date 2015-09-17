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
#include <iostream>

extern float frand();

LineBrush::LineBrush(ImpressionistDoc* pDoc, char* name) :
ImpBrush(pDoc, name)
{
}

void LineBrush::BrushBegin(const Point source, const Point target)
{
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

	this->angle = pDoc->getAngle();
	this->length = pDoc->getSize();
	int width = pDoc->getLineWidth();
	glLineWidth(width);

	glBegin(GL_LINES);
	SetColor(source);

	if (!dlg->getEdgeClipping() || !pDoc->m_ucEdgeBitmap)
	{
		glVertex2f(target.x - this->length / 2 * cos(this->angle * 3.14159 / 180), target.y - this->length / 2 * sin(this->angle * 3.14159 / 180));
		glVertex2f(target.x + this->length / 2 * cos(this->angle * 3.14159 / 180), target.y + this->length / 2 * sin(this->angle * 3.14159 / 180));
	}
	else
	{
		bool sourceIsEdge = pDoc->m_ucEdgeBitmap[target.y * pDoc->m_nWidth + target.x] != 0;
		bool reachLine = false;
		Point endPoint;
		//calculate first endpoint
		endPoint.x = target.x;
		endPoint.y = target.y;
		for (int i = 0; i <= length / 2; ++i)
		{
			endPoint.x = target.x - i * cos(this->angle * 3.14159 / 180);
			endPoint.y = target.y - i * sin(this->angle * 3.14159 / 180);
			if (endPoint.x < 0)
			{
				endPoint.x = 0;
				reachLine = true;
			}
			if (endPoint.x > pDoc->m_nWidth)
			{
				endPoint.x = pDoc->m_nWidth;
				reachLine = true;
			}
			if (endPoint.y < 0)
			{
				endPoint.y = 0;
				reachLine = true;
			}
			if (endPoint.y > pDoc->m_nHeight)
			{
				endPoint.y = pDoc->m_nHeight;
				reachLine = true;
			}

			int pointIndex = endPoint.x + endPoint.y * pDoc->m_nWidth;
			bool pointIsEdge = pDoc->m_ucEdgeBitmap[3 * pointIndex] != 0;
			if (reachLine || (pointIsEdge != sourceIsEdge) || i == length / 2)
			{
				glVertex2d(endPoint.x, endPoint.y);
				break;
			}
		}
		//calculate second endpoint
		reachLine = false;
		endPoint.x = target.x;
		endPoint.y = target.y;
		for (int i = 0; i <= length / 2; ++i)
		{
			endPoint.x = target.x + i * cos(this->angle * 3.14159 / 180);
			endPoint.y = target.y + i * sin(this->angle * 3.14159 / 180);
			if (endPoint.x < 0)
			{
				endPoint.x = 0;
				reachLine = true;
			}
			if (endPoint.x > pDoc->m_nWidth)
			{
				endPoint.x = pDoc->m_nWidth;
				reachLine = true;
			}
			if (endPoint.y < 0)
			{
				endPoint.y = 0;
				reachLine = true;
			}
			if (endPoint.y > pDoc->m_nHeight)
			{
				endPoint.y = pDoc->m_nHeight;
				reachLine = true;
			}

			int pointIndex = endPoint.x + endPoint.y * pDoc->m_nWidth;
			bool pointIsEdge = pDoc->m_ucEdgeBitmap[3 * pointIndex] != 0;
			if (reachLine || (pointIsEdge != sourceIsEdge) || i == length / 2)
			{
				glVertex2d(endPoint.x, endPoint.y);
				break;
			}
		}
	}
	glEnd();
}

void LineBrush::BrushEnd(const Point source, const Point target)
{
	// do nothing so far
}