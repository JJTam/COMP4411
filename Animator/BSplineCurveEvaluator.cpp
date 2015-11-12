#include "BSplineCurveEvaluator.h"
#include <assert.h>
#include "vec.h"
#include "mat.h"

Point calculateBSpline(float t, const Point& p1, const Point& p2, const Point& p3, const Point& p4)
{
	Point result;
	Vec4f T(t*t*t, t*t, t, 1);
	Mat4f M(-1, 3, -3, 1,
		3, -6, 3, 0,
		-3, 0, 3, 0,
		1, 4, 1, 0);
	Vec4f Gx(p1.x, p2.x, p3.x, p4.x);
	Vec4f Gy(p1.y, p2.y, p3.y, p4.y);

	result.x = (T*M)*Gx / 6;
	result.y = (T*M)*Gy / 6;
	return result;
}


void BSplineCurveEvaluator::evaluateCurve(const std::vector<Point>& ptvCtrlPts,
	std::vector<Point>& ptvEvaluatedCurvePts,
	const float& fAniLength,
	const bool& bWrap) const
{
	int sampleRate = 120;
	ptvEvaluatedCurvePts.clear();
	vector<Point> ctrlPts;

	if (bWrap)
	{
		if (ptvCtrlPts.size() == 2)
		{
			Point tmp;
			Point mp0(ptvCtrlPts[0].x - fAniLength, ptvCtrlPts[0].y);
			Point mp1(ptvCtrlPts[1].x - fAniLength, ptvCtrlPts[1].y);
			Point p0(ptvCtrlPts[0].x + fAniLength, ptvCtrlPts[0].y);
			Point p1(ptvCtrlPts[1].x + fAniLength, ptvCtrlPts[1].y);
			for (int j = 0; j <= sampleRate; ++j)
			{
				tmp = calculateBSpline((float)j / sampleRate, mp0, mp1, ptvCtrlPts[0], ptvCtrlPts[1]);
				if (tmp.x >= 0)ptvEvaluatedCurvePts.push_back(tmp);
			}
			for (int j = 0; j <= sampleRate; ++j)
			{
				tmp = calculateBSpline((float)j / sampleRate, mp1, ptvCtrlPts[0], ptvCtrlPts[1], p0);
				if (tmp.x >= 0 && tmp.x <= fAniLength)ptvEvaluatedCurvePts.push_back(tmp);
			}
			for (int j = 0; j <= sampleRate; ++j)
			{
				Point tmp;
				tmp = calculateBSpline((float)j / sampleRate, ptvCtrlPts[0], ptvCtrlPts[1], p0, p1);
				if (tmp.x <= fAniLength)ptvEvaluatedCurvePts.push_back(tmp);
			}
		}
		else
		{
			Point tmp;
			Point mp0(ptvCtrlPts[ptvCtrlPts.size() - 2].x - fAniLength, ptvCtrlPts[ptvCtrlPts.size() - 2].y);
			Point mp1(ptvCtrlPts[ptvCtrlPts.size() - 1].x - fAniLength, ptvCtrlPts[ptvCtrlPts.size() - 1].y);
			Point p0(ptvCtrlPts[0].x + fAniLength, ptvCtrlPts[0].y);
			Point p1(ptvCtrlPts[1].x + fAniLength, ptvCtrlPts[1].y);
			for (int j = 0; j <= sampleRate; ++j)
			{
				tmp = calculateBSpline((float)j / sampleRate, mp0, mp1, ptvCtrlPts[0], ptvCtrlPts[1]);
				if (tmp.x >= 0)ptvEvaluatedCurvePts.push_back(tmp);
			}
			for (int j = 0; j <= sampleRate; ++j)
			{
				tmp = calculateBSpline((float)j / sampleRate, mp1, ptvCtrlPts[0], ptvCtrlPts[1], ptvCtrlPts[2]);
				if (tmp.x >= 0 && tmp.x <= fAniLength)ptvEvaluatedCurvePts.push_back(tmp);
				tmp = calculateBSpline((float)j / sampleRate, ptvCtrlPts[ptvCtrlPts.size() - 3], ptvCtrlPts[ptvCtrlPts.size() - 2], ptvCtrlPts[ptvCtrlPts.size() - 1], p0);
				if (tmp.x >= 0 && tmp.x <= fAniLength)ptvEvaluatedCurvePts.push_back(tmp);
			}
			for (int j = 0; j <= sampleRate; ++j)
			{
				Point tmp;
				tmp = calculateBSpline((float)j / sampleRate, ptvCtrlPts[ptvCtrlPts.size() - 2], ptvCtrlPts[ptvCtrlPts.size() - 1], p0, p1);
				if (tmp.x <= fAniLength)ptvEvaluatedCurvePts.push_back(tmp);
			}
		}
		ctrlPts.insert(ctrlPts.end(), ptvCtrlPts.begin(), ptvCtrlPts.end());
		/*
		float h = ptvCtrlPts[0].y + ptvCtrlPts[ptvCtrlPts.size() - 1].y;
		h /= 2.0;
		ctrlPts.push_back(Point(0, h));
		ctrlPts.push_back(Point(0, h));
		ctrlPts.push_back(Point(0, h));
		ctrlPts.insert(ctrlPts.end(), ptvCtrlPts.begin(), ptvCtrlPts.end());
		ctrlPts.push_back(Point(fAniLength, h));
		ctrlPts.push_back(Point(fAniLength, h));
		ctrlPts.push_back(Point(fAniLength, h));
		*/
	}
	else
	{
		ptvEvaluatedCurvePts.push_back(Point(0, ptvCtrlPts[0].y));
		ptvEvaluatedCurvePts.push_back(Point(fAniLength, ptvCtrlPts[ptvCtrlPts.size() - 1].y));
		ctrlPts.push_back(ptvCtrlPts[0]);
		ctrlPts.push_back(ptvCtrlPts[0]);
		ctrlPts.insert(ctrlPts.end(), ptvCtrlPts.begin(), ptvCtrlPts.end());
		ctrlPts.push_back(ptvCtrlPts[ptvCtrlPts.size() - 1]);
		ctrlPts.push_back(ptvCtrlPts[ptvCtrlPts.size() - 1]);
	}


	if (ctrlPts.size() >= 4)
	{
		for (int i = 0; i < ctrlPts.size() - 3; ++i)
		{
			for (int j = 0; j <= sampleRate; ++j)
			{
				ptvEvaluatedCurvePts.push_back(calculateBSpline((float)j / sampleRate, ctrlPts[i], ctrlPts[i + 1], ctrlPts[i + 2], ctrlPts[i + 3]));
			}
		}
	}

}