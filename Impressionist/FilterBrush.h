#ifndef _FILTERBRUSH_H
#define _FILTERBRUSH_H

#include "ImpBrush.h"

enum FilterTypes {
	FB_GAUSSIAN_FILTER = 0,
	FB_MEAN_FILTER,
	FB_MEDIAN_FILTER,
	FB_CUSTOM_FILTER,
	NUM_FILTER_TYPE // Make sure this stays at the end!
};

class FilterBrush : public ImpBrush
{
public:
	FilterBrush(ImpressionistDoc* pDoc = NULL, char* name = NULL);

	void BrushBegin(const Point source, const Point target);
	void BrushMove(const Point source, const Point target);
	void BrushEnd(const Point source, const Point target);
	char* BrushName(void);
};

#endif // _FILTERBRUSH_H