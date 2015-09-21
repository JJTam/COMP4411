#include "FilterBrush.h"
#include "ImpressionistDoc.h"
#include "ImpressionistUI.h"

FilterBrush::FilterBrush(ImpressionistDoc* pDoc, char* name) :
ImpBrush(pDoc, name)
{
}

void FilterBrush::BrushBegin(const Point source, const Point target)
{
	BrushMove(source, target);
}

void FilterBrush::BrushMove(const Point source, const Point target)
{
	ImpressionistDoc* pDoc = GetDocument();
	ImpressionistUI* dlg = pDoc->m_pUI;

	if (pDoc == NULL) {
		printf("PointBrush::BrushMove  document is NULL\n");
		return;
	}

	int sizeHalf = pDoc->getSize() / 2;
	GLubyte color[4];
	color[3] = pDoc->getAlpha() * 255;

	glPointSize(1);
	glBegin(GL_POINTS);
	for (int dy = -sizeHalf; dy <= sizeHalf; ++dy)
	{
		int ty = target.y + dy;
		int sy = source.y + dy;
		if (ty < 0 || ty > pDoc->m_nPaintHeight)
			continue;

		for (int dx = -sizeHalf; dx <= sizeHalf; ++dx)
		{
			int tx = target.x + dx;
			int sx = source.x + dx;
			if (tx < 0 || tx > pDoc->m_nPaintWidth)
				continue;

			memcpy(color, pDoc->GetFilteredPixel(sx, sy), 3);
			color[0] *= pDoc->m_pUI->m_colorSelector->r();
			color[1] *= pDoc->m_pUI->m_colorSelector->g();
			color[2] *= pDoc->m_pUI->m_colorSelector->b();
			glColor4ubv(color);

			glVertex2d(tx, ty);
		}
	}
	glEnd();
}

void FilterBrush::BrushEnd(const Point source, const Point target)
{

}