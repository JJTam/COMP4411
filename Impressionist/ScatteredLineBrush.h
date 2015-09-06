//
//LineBrush.h
//
// The header file for Scattered Line Brush. 
//

#ifndef ScatteredLINEBRUSH_H
#define ScatteredLINEBRUSH_H

#include "ImpBrush.h"

class ScatteredLineBrush : public ImpBrush
{
public:
	ScatteredLineBrush(ImpressionistDoc* pDoc = NULL, char* name = NULL);

	void BrushBegin(const Point source, const Point target);
	void BrushMove(const Point source, const Point target);
	void BrushEnd(const Point source, const Point target);
	char* BrushName();

private:
	int length;
	int angle;
};

#endif