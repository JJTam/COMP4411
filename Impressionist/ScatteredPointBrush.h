//
//ScatteredPointBrush.h
//
// The header file for ScatteredPoint Brush. 
//

#ifndef ScatteredPOINTBRUSH_H
#define ScatteredPOINTBRUSH_H

#include "ImpBrush.h"

class ScatteredPointBrush : public ImpBrush
{
public:
	ScatteredPointBrush(ImpressionistDoc* pDoc = NULL, char* name = NULL);

	void BrushBegin(const Point source, const Point target);
	void BrushMove(const Point source, const Point target);
	void BrushEnd(const Point source, const Point target);
	char* BrushName(void);
};

#endif