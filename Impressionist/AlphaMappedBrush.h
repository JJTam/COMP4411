#ifndef _ALPHAMAPPEDBRUSH_H
#define _ALPHAMAPPEDBRUSH_H

#include "ImpBrush.h"

class AlphaMappedBrush : public ImpBrush
{
public:
	AlphaMappedBrush(ImpressionistDoc* pDoc = NULL, char* name = NULL);

	void BrushBegin(const Point source, const Point target);
	void BrushMove(const Point source, const Point target);
	void BrushEnd(const Point source, const Point target);
	char* BrushName(void);
};

#endif // _ALPHAMAPPEDBRUSH_H