#include "C2interpolatingCurveEvaluator.h"
#include "vec.h"
#include "mat.h"
#include "modelerapp.h"
#include "modelerui.h"

vector<Point> CalculateDerivative(const vector<Point>& ctrlpts)
{
	vector<float> a(ctrlpts.size(), 0);
	vector<Point> b(ctrlpts.size(), Point(0, 0));
	vector<Point> result(ctrlpts.size(), Point(0, 0));
	int i = 0;

	a[i] = 1 / 2;
	for (i = 1; i < ctrlpts.size() - 1; ++i)
	{
		a[i] = 1 / (4 - a[i - 1]);
	}
	a[i] = 1 / (2 - a[i - 1]);

	i = 0;
	b[i] = Point(3 * a[i] * (ctrlpts[i + 1].x - ctrlpts[i].x), 3 * a[i] * (ctrlpts[i + 1].y - ctrlpts[i].y));
	for (i = 1; i < ctrlpts.size() - 1; ++i)
	{
		b[i] = Point(a[i] * (3 * (ctrlpts[i + 1].x - ctrlpts[i - 1].x) - b[i - 1].x), a[i] * (3 * (ctrlpts[i + 1].y - ctrlpts[i - 1].y) - b[i - 1].y));
	}
	b[i] = Point(a[i] * (3 * (ctrlpts[i].x - ctrlpts[i - 1].x) - b[i - 1].x), a[i] * (3 * (ctrlpts[i].y - ctrlpts[i - 1].y) - b[i - 1].y));

	i = ctrlpts.size() - 1;
	result[i] = b[i];
	for (i = i - 1; i != 0; --i)
	{
		result[i] = Point(b[i].x - a[i] * result[i + 1].x, b[i].y - a[i] * result[i + 1].y);
	}
	return result;
}

Point calculateHermite(double t, const Point& p1, const Point& p2, const Point& p3, const Point& p4)
{
	Point result;
	Vec4f T(t*t*t, t*t, t, 1);
	Mat4f M(2, -2, 1, 1,
		-3, 3, -2, -1,
		0, 0, 1, 0,
		1, 0, 0, 0);
	Vec4f Gx(p1.x, p2.x, p3.x, p4.x);
	Vec4f Gy(p1.y, p2.y, p3.y, p4.y);

	result.x = (T*M)*Gx;
	result.y = (T*M)*Gy;
	return result;
}
void C2interpolatingCurveEvaluator::evaluateCurve(const std::vector<Point>& ptvCtrlPts,
	std::vector<Point>& ptvEvaluatedCurvePts,
	const float& fAniLength,
	const bool& bWrap) const
{
	int sampleRate = 120;
	ptvEvaluatedCurvePts.clear();


	vector<Point> D;
	vector<Point> ctrlPts = ptvCtrlPts;

	if (bWrap)
	{
		ctrlPts.clear();
		ctrlPts.push_back(Point(ptvCtrlPts[ptvCtrlPts.size() - 1].x - fAniLength, ptvCtrlPts[ptvCtrlPts.size() - 1].y));
		ctrlPts.insert(ctrlPts.end(), ptvCtrlPts.begin(), ptvCtrlPts.end());
		ctrlPts.push_back(Point(ptvCtrlPts[0].x + fAniLength, ptvCtrlPts[0].y));
		D = CalculateDerivative(ctrlPts);
		for (int i = 0; i < ctrlPts.size() - 1; ++i)
		{
			for (int j = 0; j <= sampleRate; ++j)
			{
				Point tmp = calculateHermite((float)j / sampleRate, ctrlPts[i], ctrlPts[i + 1], D[i], D[i + 1]);
				if (tmp.x <= ctrlPts[i + 1].x && tmp.x >= ctrlPts[i].x) ptvEvaluatedCurvePts.push_back(tmp);
			}
		}
	}
	else
	{
		D = CalculateDerivative(ctrlPts);
		if (ModelerApplication::getPUI()->m_pbtAdaptive->value() != 0)
		{
			D[0] = Point(ModelerApplication::getPUI()->m_pC2X1ValueInput->value(), ModelerApplication::getPUI()->m_pC2Y1ValueInput->value());
			D[D.size() - 1] = Point(ModelerApplication::getPUI()->m_pC2X2ValueInput->value(), ModelerApplication::getPUI()->m_pC2Y2ValueInput->value());
		}
		ptvEvaluatedCurvePts.push_back(Point(0, ptvCtrlPts[0].y));
		ptvEvaluatedCurvePts.push_back(Point(fAniLength, ptvCtrlPts[ptvCtrlPts.size() - 1].y));
		for (int i = 0; i < ctrlPts.size() - 1; ++i)
		{
			for (int j = 0; j <= sampleRate; ++j)
			{
				Point tmp = calculateHermite((float)j / sampleRate, ctrlPts[i], ctrlPts[i + 1], D[i], D[i + 1]);
				if (tmp.x <= ctrlPts[i + 1].x&& tmp.x >= ctrlPts[i].x) ptvEvaluatedCurvePts.push_back(tmp);
			}
		}
	}
}