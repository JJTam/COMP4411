#include "impressionistDoc.h"
#include "impressionistUI.h"
#include "AlphaMappedBrush.h"

AlphaMappedBrush::AlphaMappedBrush(ImpressionistDoc* pDoc, char* name) :
ImpBrush(pDoc, name)
{
}

void AlphaMappedBrush::BrushBegin(const Point source, const Point target)
{
	BrushMove(source, target);
}

void AlphaMappedBrush::BrushMove(const Point source, const Point target)
{
	ImpressionistDoc* pDoc = GetDocument();
	ImpressionistUI* dlg = pDoc->m_pUI;

	if (pDoc == NULL) {
		printf("AlphaMappedBrush::BrushMove  document is NULL\n");
		return;
	}

	if (!pDoc->m_ucAlphaBrush) {
		return;
	}
	
	int alphaBrushWidth = pDoc->m_nAlphaBrushWidth;
	int alphaBrushHeight = pDoc->m_nAlphaBrushHeight;
	int xSizeHalf = alphaBrushWidth / 2;
	int ySizeHalf = alphaBrushHeight / 2;
	GLubyte color[4];
	memcpy(color, pDoc->GetFilteredPixel(source.x, source.y), 3);
	color[0] *= pDoc->m_pUI->m_colorSelector->r();
	color[1] *= pDoc->m_pUI->m_colorSelector->g();
	color[2] *= pDoc->m_pUI->m_colorSelector->b();

	glPointSize(1);
	glBegin(GL_POINTS);
	for (int dy = -ySizeHalf; dy <= ySizeHalf; ++dy)
	{
		int ty = target.y + dy;
		if (ty < 0 || ty > pDoc->m_nPaintHeight || dy + ySizeHalf >= alphaBrushHeight)
			continue;

		for (int dx = -xSizeHalf; dx <= xSizeHalf; ++dx)
		{
			int tx = target.x + dx;
			if (tx < 0 || tx > pDoc->m_nPaintHeight || dx + xSizeHalf >= alphaBrushWidth)
				continue;

			color[3] = pDoc->m_ucAlphaBrush[dx + xSizeHalf + (dy + ySizeHalf) * alphaBrushWidth] * pDoc->getAlpha();
			if (color[3] == 0)
				continue;

			glColor4ubv(color);
			glVertex2d(tx, ty);
		}
	}
	glEnd();
}

void AlphaMappedBrush::BrushEnd(const Point source, const Point target)
{
	// do nothing so far
}

