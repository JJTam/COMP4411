#ifndef LIQUIFYBRUSH_H
#define LIQUIFYBRUSH_H

#include "ImpBrush.h"

class LiquifyBrush : public ImpBrush
{
public:
	LiquifyBrush(ImpressionistDoc* pDoc = NULL, char* name = NULL);

	void BrushBegin(const Point source, const Point target);
	void BrushMove(const Point source, const Point target);
	void BrushEnd(const Point source, const Point target);
	char* BrushName(void);
};

#endif