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
	auto color3 = centers[0].second;
	color[0] = get<0>(color3);
	color[1] = get<1>(color3);
	color[2] = get<2>(color3);
	glColor4ubv(color);
	
	if (centers.size() >= 3)
	{
		//calculate curves tangent
		double* tangentX = new double[centers.size()];
		double* tangentY = new double[centers.size()];
		tangentX[0] = centers[1].first.first - centers[0].first.first;
		tangentY[0] = centers[1].first.second - centers[0].first.second;
		tangentX[centers.size() - 1] = centers[centers.size() - 1].first.first - centers[centers.size() - 2].first.first;
		tangentY[centers.size() - 1] = centers[centers.size() - 1].first.second - centers[centers.size() - 2].first.second;
		for (int i = 1; i < centers.size() - 1; ++i)
		{
			tangentX[i] = centers[i].first.first - centers[i - 1].first.first;
			tangentY[i] = centers[i].first.second - centers[i - 1].first.second;
		}
		//calculate normal directions
		double* normalX = new double[centers.size()];
		double* normalY = new double[centers.size()];
		for (int i = 0; i < centers.size(); ++i)
		{
			double unit = sqrt(pow(tangentX[i], 2) + pow(tangentY[i], 2));
			normalX[i] = tangentY[i] / unit;
			normalY[i] = -tangentX[i] / unit;
		}
		//calculate boundary points and draw

		for (int i = 1; i < centers.size(); ++i)
		{
			glBegin(GL_POLYGON);
			glVertex2d(centers[i - 1].first.first + radius*normalX[i], centers[i - 1].first.second + radius*normalY[i]);
			glVertex2d(centers[i].first.first + radius*normalX[i], centers[i].first.second + radius*normalY[i]);
			glVertex2d(centers[i].first.first - radius*normalX[i], centers[i].first.second - radius*normalY[i]);
			glVertex2d(centers[i - 1].first.first - radius*normalX[i], centers[i - 1].first.second - radius*normalY[i]);
			glEnd();
		}

		delete tangentX;
		delete tangentY;
		delete normalX;
		delete normalY;
	}
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
