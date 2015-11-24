#include "SubdivisionCurveEvaluator.h"
#include "modelerapp.h"
#include "modelerui.h"
#include "Utils.h"
void SubdivisionCurveEvaluator::evaluateCurve(const std::vector<Point>& ptvCtrlPts,
	std::vector<Point>& ptvEvaluatedCurvePts,
	const float& fAniLength,
	const bool& bWrap) const
{
	vector<double> averageMask;
	string amstr(ModelerApplication::getPUI()->averageMask->value());
	auto amstr_s = Utils::split(amstr, ' ');
	for (auto& s : amstr_s)
	{
		try
		{
			averageMask.push_back(stod(s));
		}
		catch (exception& e)
		{

		}
	}

	int depth = ModelerApplication::getPUI()->m_pDepthSlider->value();

	vector<Point> ctrlpts = ptvCtrlPts;
	vector<Point> PtsAfterSplitting;
	for (int n = 0; n < depth; ++n)
	{
		PtsAfterSplitting.clear();
		// splitting part
		for (int i = ctrlpts.size() - averageMask.size() ; i < ctrlpts.size(); ++i)
		{
			PtsAfterSplitting.push_back(Point(ctrlpts[i].x - fAniLength, ctrlpts[i].y));
			// Add midpoint
			if (i == ctrlpts.size() - 1)PtsAfterSplitting.push_back(Point((ctrlpts[i].x - fAniLength + ctrlpts[0].x) / 2, (ctrlpts[i].y + ctrlpts[0].y) / 2));
			else PtsAfterSplitting.push_back(Point((ctrlpts[i].x - fAniLength + ctrlpts[i + 1].x - fAniLength) / 2, (ctrlpts[i].y + ctrlpts[i + 1].y) / 2));
		}

		for (int i = 0; i < ctrlpts.size()-1; ++i)
		{
			PtsAfterSplitting.push_back(ctrlpts[i]);
			PtsAfterSplitting.push_back(Point((ctrlpts[i].x + ctrlpts[i + 1].x) / 2, (ctrlpts[i].y + ctrlpts[i + 1].y) / 2));
		}
		PtsAfterSplitting.push_back(ctrlpts[ctrlpts.size() - 1]);

		for (int i = 0; i < averageMask.size(); ++i)
		{
			//Add midpoint
			if (i == 0)PtsAfterSplitting.push_back(Point((ctrlpts[i].x + fAniLength + ctrlpts[ctrlpts.size() - 1].x) / 2, (ctrlpts[i].y + ctrlpts[ctrlpts.size() - 1].y) / 2));
			else PtsAfterSplitting.push_back(Point((ctrlpts[i].x + fAniLength + ctrlpts[i - 1].x + fAniLength) / 2, (ctrlpts[i].y + ctrlpts[i - 1].y) / 2));
			PtsAfterSplitting.push_back(Point(ctrlpts[i].x + fAniLength, ctrlpts[i].y));
		}

		//average part
		ctrlpts.clear();
		for (int i = 0; i < PtsAfterSplitting.size() - averageMask.size() + 1; ++i)
		{
			Point tmp;
			for (int j = 0; j < averageMask.size(); ++j)
			{
				tmp.x += PtsAfterSplitting[i + j].x*averageMask[j];
				tmp.y += PtsAfterSplitting[i + j].y*averageMask[j];
			}
			if(tmp.x>=0 && tmp.x <=fAniLength)ctrlpts.push_back(tmp);
		}
	}
	ptvEvaluatedCurvePts = ctrlpts;
	ptvEvaluatedCurvePts.push_back(Point(0, ctrlpts[0].y));
	ptvEvaluatedCurvePts.push_back(Point(fAniLength, ctrlpts[ctrlpts.size()-1].y));
}