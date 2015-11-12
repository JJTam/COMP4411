#ifndef INCLUDED_BEZIER_CURVE_EVALUATOR_H
#define INCLUDED_BEZIER_CURVE_EVALUATOR_H

#pragma warning(disable : 4786)  

#include "CurveEvaluator.h"

//using namespace std;

class BezierCurveEvaluator : public CurveEvaluator
{
public:
	void evaluateCurve(const std::vector<Point>& ptvCtrlPts,
		std::vector<Point>& ptvEvaluatedCurvePts,
		const float& fAniLength,
		const bool& bWrap) const;
	Point calculateBezier(float t, const Point& p1, const Point& p2, const Point& p3, const Point& p4) const;
	int adaptiveBezier(std::vector<Point>& ptvEvaluatedCurvePts, const Point& p1, const Point& p2, const Point& p3, const Point& p4, double flatness)const;
};

#endif