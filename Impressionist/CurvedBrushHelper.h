#ifndef _CURVEDBRUSHHELPER_H
#define _CURVEDBRUSHHELPER_H

#include <vector>

class CurvedBrushHelper
{
public:
	static std::vector<std::pair<std::pair<int, int>, std::tuple<unsigned char, unsigned char, unsigned char> > >
		getCurvedBrushPoints(const unsigned char* imageSource, const int* sourceGradient, const unsigned char* canvas, int width, int height, int x, int y, int radius, int minStrokeLength, int maxStrokeLength, double curvatureFilter);
};

#endif // _CURVEDBRUSHHELPER_H