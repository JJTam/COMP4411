#include "BezierCurveEvaluator.h"
#include <assert.h>
#include "vec.h"
#include "mat.h"
#include "modelerapp.h"
#include "modelerui.h"


Point midpoint(const Point& p1, const Point& p2)
{
	return(Point(0.5*p1.x + 0.5*p2.x, 0.5*p1.y + 0.5*p2.y));
}
Point BezierCurveEvaluator::calculateBezier(float t, const Point& p1, const Point& p2, const Point& p3, const Point& p4) const
{
	Point result;
	Vec4f T(t*t*t, t*t, t, 1);
	Mat4f M(-1, 3, -3, 1,
		3, -6, 3, 0,
		-3, 3, 0, 0,
		1, 0, 0, 0);
	Vec4f Gx(p1.x, p2.x, p3.x, p4.x);
	Vec4f Gy(p1.y, p2.y, p3.y, p4.y);

	result.x = (T*M)*Gx;
	result.y = (T*M)*Gy;
	return result;
}

int BezierCurveEvaluator::adaptiveBezier(vector<Point>& ptvEvaluatedCurvePts, const Point& p1, const Point& p2, const Point& p3, const Point& p4, double flatness)const
{
	double currFlat = (p1.distance(p2) + p2.distance(p3) + p3.distance(p4)) / p1.distance(p4);
	if (currFlat - 1 < flatness)
	{
		ptvEvaluatedCurvePts.push_back(p1);
		ptvEvaluatedCurvePts.push_back(p4);
		return 1;
	}
	else
	{
		Point L1 = p1;
		Point L2 = midpoint(p1, p2);
		Point H = midpoint(p2, p3);
		Point R3 = midpoint(p3, p4);
		Point R4 = p4;
		Point L3 = midpoint(L2, H);
		Point R2 = midpoint(R3, H);
		Point L4 = midpoint(L3, R2);
		Point R1 = L4;
		return adaptiveBezier(ptvEvaluatedCurvePts, L1, L2, L3, L4, flatness) + adaptiveBezier(ptvEvaluatedCurvePts, R1, R2, R3, R4, flatness);
	}

}

void BezierCurveEvaluator::evaluateCurve(const std::vector<Point>& ptvCtrlPts,
	std::vector<Point>& ptvEvaluatedCurvePts,
	const float& fAniLength,
	const bool& bWrap) const
{
	double flatness = ModelerApplication::getPUI()->m_pflatnessSlider->value();
	bool adaptive = ModelerApplication::getPUI()->m_pbtAdaptive->value();

	int count = 0;
	int sampleRate = 60;
	ptvEvaluatedCurvePts.clear();

	int CtrlPtGroupCount = (ptvCtrlPts.size() - 1) / 3;
	int RemainPtCount = ptvCtrlPts.size() - CtrlPtGroupCount * 3;


	// adaptive case
	if (adaptive)
	{
		if (bWrap)
		{
			// when #remain_pts==2,they can form a bezier curve with the first ctrl pt
			if (RemainPtCount == 3)
			{

				Point p0(ptvCtrlPts[0].x + fAniLength, ptvCtrlPts[0].y);
				int pos = ptvCtrlPts.size() - RemainPtCount;
				count += adaptiveBezier(ptvEvaluatedCurvePts, ptvCtrlPts[pos], ptvCtrlPts[pos + 1], ptvCtrlPts[pos + 2], p0, flatness);
				//since it may not be continous and lack point on Xmin and Xmax
				bool first = true;
				int currSize = ptvEvaluatedCurvePts.size();
				for (int i = 0; i < currSize; ++i)
				{
					if (ptvEvaluatedCurvePts[i].x > fAniLength)
					{
						if(!first)ptvEvaluatedCurvePts[i].x -= fAniLength;
						else
						{
							first = false;
							//ensure there is a point on Xmin
							ptvEvaluatedCurvePts.push_back(Point(ptvEvaluatedCurvePts[i - 1].x - fAniLength, ptvEvaluatedCurvePts[i - 1].y));
							// do not modify current point ensures that there is a poiont on Xmax
							ptvEvaluatedCurvePts.push_back(Point(ptvEvaluatedCurvePts[i].x - fAniLength, ptvEvaluatedCurvePts[i].y));
						}
					}
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
			count += adaptiveBezier(ptvEvaluatedCurvePts, ptvCtrlPts[3 * i], ptvCtrlPts[3 * i + 1], ptvCtrlPts[3 * i + 2], ptvCtrlPts[3 * i + 3], flatness);
		}
		//handle pts which can't form a groud of ctrl pts
		if (RemainPtCount != 3 || !bWrap)
		{
			for (int i = ptvCtrlPts.size() - RemainPtCount; i < ptvCtrlPts.size(); ++i)
			{
				ptvEvaluatedCurvePts.push_back(ptvCtrlPts[i]);
			}
		}
		cout << "Number of lines: " << count << endl;
		return;
	}

	// Normal case
	if (bWrap)
	{
		// when #remain_pts==3,they can form a bezier curve with the first ctrl pt
		if (RemainPtCount == 3)
		{
			Point p0(ptvCtrlPts[0].x + fAniLength, ptvCtrlPts[0].y);
			int pos = ptvCtrlPts.size() - RemainPtCount;
			for (int j = 0; j <= sampleRate; ++j)
			{
				Point tmp = calculateBezier((float)j / sampleRate, ptvCtrlPts[pos], ptvCtrlPts[pos + 1], ptvCtrlPts[pos + 2], p0);
				if (tmp.x >= fAniLength)ptvEvaluatedCurvePts.push_back(Point(tmp.x - fAniLength, tmp.y));
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
	if (RemainPtCount != 3 || !bWrap)
	{
		for (int i = ptvCtrlPts.size() - RemainPtCount; i < ptvCtrlPts.size(); ++i)
		{
			ptvEvaluatedCurvePts.push_back(ptvCtrlPts[i]);
		}
	}

}