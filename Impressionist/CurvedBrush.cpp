//
// CurvedBrush.cpp
//
// The implementation of Point Brush. It is a kind of ImpBrush. All your brush implementations
// will look like the file with the different GL primitive calls.
//

#include "impressionistDoc.h"
#include "impressionistUI.h"
#include "curvedbrush.h"
#include "CurvedBrushHelper.h"
#include <iostream>
#include <vector>
#include <tuple>

using namespace std;

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

	int width = pDoc->m_nWidth;
	int height = pDoc->m_nHeight;

	radius = pDoc->getSize();
	minStrokeLength = pDoc->m_pUI->getMinStrokeLength();
	maxStrokeLength = pDoc->m_pUI->getMaxStrokeLength();
	curvatureFilter = pDoc->m_pUI->getCurvatureFilter();

	unsigned char* imageSource = (pDoc->m_ucBitmapBlurred == NULL) ? pDoc->m_ucBitmap : pDoc->m_ucBitmapBlurred;

	vector<pair< pair<int, int>, tuple<unsigned char, unsigned char, unsigned char> > > centers = CurvedBrushHelper::getCurvedBrushPoints(imageSource, pDoc->m_iGradient, pDoc->m_ucPreservedPainting, width, height, source.x, source.y, radius, minStrokeLength, maxStrokeLength, curvatureFilter);

	GLubyte color[4];
	color[3] = pDoc->getAlpha() * 255;
	for (auto c : centers)
	{
		auto point = c.first;
		auto color3 = c.second;
		color[0] = get<0>(color3);
		color[1] = get<1>(color3);
		color[2] = get<2>(color3);
		glColor4ubv(color);

		glBegin(GL_POLYGON);
		for (int i = 0; i < 36; ++i)
		{
			double theta = i * 10 * 3.14159 / 180;
			glVertex2d(point.first - radius * cos(theta), point.second - radius * sin(theta));
		}
		glEnd();
	}
}

void CurvedBrush::BrushEnd(const Point source, const Point target)
{
	// do nothing so far
}
