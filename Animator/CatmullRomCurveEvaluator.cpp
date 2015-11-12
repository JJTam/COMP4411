#include "CatmullRomCurveEvaluator.h"
#include <assert.h>
#include "vec.h"
#include "mat.h"

Point calculateCatmullRom(double t, const Point& p1, const Point& p2, const Point& p3, const Point& p4)
{
	double tension = 0.5;
	Point result;
	Vec4f T(t*t*t, t*t, t, 1);
	Mat4f M(2, -2, 1, 1,
		-3, 3, -2, -1,
		0, 0, 1, 0,
		1, 0, 0, 0);
	Vec4f Gx(p2.x, p3.x, tension*(p3.x - p1.x), tension*(p4.x - p2.x));
	Vec4f Gy(p2.y, p3.y, tension*(p3.y - p1.y), tension*(p4.y - p2.y));

	result.x = (T*M)*Gx;
	result.y = (T*M)*Gy;
	return result;
}


void CatmullRomCurveEvaluator::evaluateCurve(const std::vector<Point>& ptvCtrlPts,
	std::vector<Point>& ptvEvaluatedCurvePts,
	const float& fAniLength,
	const bool& bWrap) const
{
	int sampleRate = 120;
	ptvEvaluatedCurvePts.clear();

	double lastX = -1;
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
				tmp = calculateCatmullRom((float)j / sampleRate, mp0, mp1, ptvCtrlPts[0], ptvCtrlPts[1]);
				if (tmp.x >= 0 && tmp.x >= lastX && tmp.x < ptvCtrlPts[0].x)
				{
					lastX = tmp.x;
					ptvEvaluatedCurvePts.push_back(tmp);
				}
			}
			for (int j = 0; j <= sampleRate; ++j)
			{
				tmp = calculateCatmullRom((float)j / sampleRate, mp1, ptvCtrlPts[0], ptvCtrlPts[1], p0);
				if (tmp.x >= 0 && tmp.x >= lastX && tmp.x <= fAniLength && tmp.x < ptvCtrlPts[1].x)
				{
					lastX = tmp.x;
					ptvEvaluatedCurvePts.push_back(tmp);
				}
			}
			for (int j = 0; j <= sampleRate; ++j)
			{
				Point tmp;
				tmp = calculateCatmullRom((float)j / sampleRate, ptvCtrlPts[0], ptvCtrlPts[1], p0, p1);
				if (tmp.x >= lastX && tmp.x <= fAniLength)
				{
					lastX = tmp.x;
					ptvEvaluatedCurvePts.push_back(tmp);
				}
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
				tmp = calculateCatmullRom((float)j / sampleRate, mp0, mp1, ptvCtrlPts[0], ptvCtrlPts[1]);
				if (tmp.x >= 0 && tmp.x >= lastX && tmp.x < ptvCtrlPts[0].x)
				{
					lastX = tmp.x;
					ptvEvaluatedCurvePts.push_back(tmp);
				}
			}
			for (int j = 0; j <= sampleRate; ++j)
			{
				tmp = calculateCatmullRom((float)j / sampleRate, mp1, ptvCtrlPts[0], ptvCtrlPts[1], ptvCtrlPts[2]);
				if (tmp.x >= 0 && tmp.x >= lastX && tmp.x < ptvCtrlPts[1].x)
				{
					lastX = tmp.x;
					ptvEvaluatedCurvePts.push_back(tmp);
				}
			}
			for (int j = 0; j <= sampleRate; ++j)
			{
				tmp = calculateCatmullRom((float)j / sampleRate, ptvCtrlPts[ptvCtrlPts.size() - 3], ptvCtrlPts[ptvCtrlPts.size() - 2], ptvCtrlPts[ptvCtrlPts.size() - 1], p0);
				if (tmp.x >= lastX && tmp.x <= fAniLength && tmp.x < ptvCtrlPts[ptvCtrlPts.size() - 1].x)
				{
					lastX = tmp.x;
					ptvEvaluatedCurvePts.push_back(tmp);
				}
			}
			for (int j = 0; j <= sampleRate; ++j)
			{
				Point tmp;
				tmp = calculateCatmullRom((float)j / sampleRate, ptvCtrlPts[ptvCtrlPts.size() - 2], ptvCtrlPts[ptvCtrlPts.size() - 1], p0, p1);
				if (tmp.x >= lastX && tmp.x <= fAniLength)
				{
					lastX = tmp.x;
					ptvEvaluatedCurvePts.push_back(tmp);
				}
			}
		}
		ctrlPts.insert(ctrlPts.end(), ptvCtrlPts.begin(), ptvCtrlPts.end());
	}
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

		ctrlPts.push_back(ptvCtrlPts[0]);
		ctrlPts.insert(ctrlPts.end(), ptvCtrlPts.begin(), ptvCtrlPts.end());
		ctrlPts.push_back(ptvCtrlPts[ptvCtrlPts.size() - 1]);

	}

	lastX = 0;
	if (ctrlPts.size() >= 4)
	{
		for (int i = 0; i < ctrlPts.size() - 3; ++i)
		{
			for (int j = 0; j < sampleRate; ++j)
			{
				Point tmp = calculateCatmullRom((float)j / sampleRate, ctrlPts[i], ctrlPts[i + 1], ctrlPts[i + 2], ctrlPts[i + 3]);
				if (tmp.x > lastX && tmp.x < ctrlPts[i + 3].x)
				{
					ptvEvaluatedCurvePts.push_back(tmp);
					lastX = tmp.x;
				}
			}
		}
	}

}