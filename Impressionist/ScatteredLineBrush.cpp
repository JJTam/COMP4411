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

	this->length = pDoc->getSize();
	this->angle = pDoc->getAngle();
	int width = pDoc->getLineWidth();
	glLineWidth(width);

	glBegin(GL_LINES);
	int loop_time = irand(2) + 2;
	for (int i = 0; i < loop_time; ++i)
	{
		int Xoffset = irand(this->length) - this->length / 2;
		int Yoffset = irand(this->length) - this->length / 2;
		SetColor(source.x + Xoffset, source.y + Yoffset);
		if (!dlg->getEdgeClipping())
		{
			glVertex2f(target.x + Xoffset - this->length / 2 * cos(this->angle * 3.14159 / 180), target.y + Yoffset - this->length / 2 * sin(this->angle * 3.14159 / 180));
			glVertex2f(target.x + Xoffset + this->length / 2 * cos(this->angle * 3.14159 / 180), target.y + Yoffset + this->length / 2 * sin(this->angle * 3.14159 / 180));
		}
		else
		{
			bool reachLine = false;
			Point endPoint;
			//calculate first endpoint
			endPoint.x = target.x + Xoffset;
			endPoint.y = target.y + Yoffset;
			for (int i = 0; i <= length / 2; ++i)
			{
				endPoint.x = target.x + Xoffset - i * cos(this->angle * 3.14159 / 180);
				endPoint.y = target.y + Yoffset - i * sin(this->angle * 3.14159 / 180);
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
				if (reachLine || pDoc->m_ucEdgeBitmap[3 * pointIndex] != 0 || i == length / 2)
				{
					glVertex2f(endPoint.x, endPoint.y);
					break;
				}
			}
			//calculate second endpoint
			reachLine = false;
			endPoint.x = target.x + Xoffset;
			endPoint.y = target.y + Yoffset;
			for (int i = 0; i <= length / 2; ++i)
			{
				endPoint.x = target.x + Xoffset + i * cos(this->angle * 3.14159 / 180);
				endPoint.y = target.y + Yoffset + i * sin(this->angle * 3.14159 / 180);
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
				if (reachLine || pDoc->m_ucEdgeBitmap[3 * pointIndex] != 0 || i == length / 2)
				{
					glVertex2f(endPoint.x, endPoint.y);
					break;
				}
			}
		}
	}
	glEnd();
}

void ScatteredLineBrush::BrushEnd(const Point source, const Point target)
{
	// do nothing so far
}