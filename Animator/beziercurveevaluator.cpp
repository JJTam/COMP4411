#include "BezierCurveEvaluator.h"
#include <assert.h>
#include "vec.h"
#include "mat.h"


Point calculateBezier(float t, const Point& p1, const Point& p2, const Point& p3, const Point& p4)
{
	Point result;
	Vec4f T(t*t*t,t*t,t,1);
	Mat4f M(-1, 3, -3, 1,
		3, -6, 3, 0,
		-3, 3, 0, 0,
		1, 0, 0, 0);
	Vec4f Gx (p1.x, p2.x, p3.x, p4.x);
	Vec4f Gy (p1.y, p2.y, p3.y, p4.y);

	result.x = (T*M)*Gx;
	result.y = (T*M)*Gy;
	return result;
}


void BezierCurveEvaluator::evaluateCurve(const std::vector<Point>& ptvCtrlPts,
	std::vector<Point>& ptvEvaluatedCurvePts,
	const float& fAniLength,
	const bool& bWrap) const
{
	int sampleRate = 30;
	ptvEvaluatedCurvePts.clear();

	int CtrlPtGroupCount = (ptvCtrlPts.size()-1) / 3;
	int RemainPtCount = ptvCtrlPts.size() - CtrlPtGroupCount * 3 - 1;

	if (bWrap)
	{
		// when #remain_pts==2,they can form a bezier curve with the first ctrl pt
		if (RemainPtCount == 2)
		{
			cout << "!" << endl;
			Point p0(ptvCtrlPts[0].x + fAniLength, ptvCtrlPts[0].y);
			int pos = ptvCtrlPts.size() - RemainPtCount;
			for (int j = 0; j <= sampleRate; ++j)
			{
				Point tmp = calculateBezier((float)j / sampleRate, ptvCtrlPts[pos - 1], ptvCtrlPts[pos], ptvCtrlPts[pos + 1], p0);
				if (tmp.x >= fAniLength)ptvEvaluatedCurvePts.push_back(Point(tmp.x - fAniLength,tmp.y));
				else ptvEvaluatedCurvePts.push_back(tmp);
			}

		}
		// else do linearly interpolate
		else
		{
			float y1;
			if ((ptvCtrlPts[0].x + fAniLength) - ptvCtrlPts[ptvCtrlPts.size() - 1].x > 0.0f) 
			{
				y1 = (ptvCtrlPts[0].y * (fAniLength - ptvCtrlPts[ptvCtrlPts.size() - 1].x) +
					ptvCtrlPts[ptvCtrlPts.size() - 1].y * ptvCtrlPts[0].x) /
					(ptvCtrlPts[0].x + fAniLength - ptvCtrlPts[ptvCtrlPts.size() - 1].x);
			}
			else
				y1 = ptvCtrlPts[0].y;
			ptvEvaluatedCurvePts.push_back(Point(0, y1));
			ptvEvaluatedCurvePts.push_back(Point(fAniLength, y1));
		}
	}
	else
	{
		//handle the line segment before first and after last ctrl pt
		ptvEvaluatedCurvePts.push_back(Point(0, ptvCtrlPts[0].y));
		ptvEvaluatedCurvePts.push_back(Point(fAniLength, ptvCtrlPts[ptvCtrlPts.size() - 1].y));
	}

	for (int i = 0; i < CtrlPtGroupCount; ++i)
	{
		for (int j = 0; j <= sampleRate; ++j)
		{
			Point tmp = calculateBezier((float)j / sampleRate, ptvCtrlPts[3 * i], ptvCtrlPts[3 * i + 1], ptvCtrlPts[3 * i + 2], ptvCtrlPts[3 * i + 3]);
			if (tmp.x <= fAniLength)ptvEvaluatedCurvePts.push_back(tmp);
		}
	}
	//handle pts which can't form a groud of ctrl pts
	if (RemainPtCount != 2 || !bWrap)
	{
		for (int i = ptvCtrlPts.size() - RemainPtCount; i < ptvCtrlPts.size(); ++i)
		{
			ptvEvaluatedCurvePts.push_back(ptvCtrlPts[i]);
		}
	}

}