//
// CurvedBrush.h
//
// The header file for Point Brush. 
//

#ifndef CURVEDBRUSH_H
#define CURVEDBRUSH_H

#include "ImpBrush.h"

class CurvedBrush : public ImpBrush
{
public:
	CurvedBrush(ImpressionistDoc* pDoc = NULL, char* name = NULL);

	void BrushBegin(const Point source, const Point target);
	void BrushMove(const Point source, const Point target);
	void BrushEnd(const Point source, const Point target);
	char* BrushName(void);

	double radius;
	int minStrokeLength;
	int maxStrokeLength;
	double curvatureFilter;
	unsigned char* blurredSource = NULL;
};

#endif