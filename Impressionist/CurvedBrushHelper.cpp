#include "CurvedBrushHelper.h"
#include "ImageUtils.h"
#include <cmath>
#include <vector>
#include <tuple>

using namespace std;

vector<pair<pair<int, int>, tuple<unsigned char, unsigned char, unsigned char> > > CurvedBrushHelper::getCurvedBrushPoints(const unsigned char* imageSource, const int* sourceGradient, const unsigned char* canvas, int width, int height, int x, int y, int radius, int minStrokeLength, int maxStrokeLength, double curvatureFilter)
{
	vector<pair< pair<int, int>, tuple<unsigned char, unsigned char, unsigned char> > > result;

	unsigned char color[3];
	memcpy(color, (void*)(imageSource + (y * width + x) * 3), 3);
	result.push_back(make_pair(make_pair(x, y), make_tuple(color[0], color[1], color[2])));

	int brushColor[3];
	brushColor[0] = color[0];
	brushColor[1] = color[1];
	brushColor[2] = color[2];

	double dx = 0, dy = 0, lastDx = 0, lastDy = 0;

	for (int i = 1; i <= maxStrokeLength; ++i)
	{
		if (i > minStrokeLength)
		{
			int originColor[3];
			int canvasColor[3];
			for (int i = 0; i < 3; ++i)
			{
				originColor[i] = imageSource[3 * (y * width + x) + i];
				canvasColor[i] = canvas[4 * (y*width + x) + i];
			}

			double drawDiff = sqrt(pow(originColor[0] - brushColor[0], 2) + pow(originColor[1] - brushColor[1], 2) + pow(originColor[2] - brushColor[2], 2));
			double currentDiff = sqrt(pow(originColor[0] - canvasColor[0], 2) + pow(originColor[1] - canvasColor[1], 2) + pow(originColor[2] - canvasColor[2], 2));
			if (currentDiff < drawDiff)
			{
				break;
			}
		}

		// Get vector of gradient
		double gx = sourceGradient[2 * (y*width + x)];
		double gy = sourceGradient[2 * (y*width + x) + 1];
		if (gx == 0 && gy == 0)
		{
			break;
		}

		// Compute normalize direction
		dx = -gy / sqrt(gx * gx + gy * gy);
		dy = gx / sqrt(gx * gx + gy * gy);

		// If necessary, reverse direction
		if (lastDx * dx + lastDy * dy < 0)
		{
			dx = -dx;
			dy = -dy;
		}

		// Filter the stroke direction
		dx = curvatureFilter * dx + (1 - curvatureFilter) * lastDx;
		dy = curvatureFilter * dy + (1 - curvatureFilter) * lastDy;
		gx = dx / sqrt(dx*dx + dy*dy);
		gy = dy / sqrt(dx*dx + dy*dy);
		dx = gx;
		dy = gy;
		x = x + radius * dx;
		y = y + radius * dy;
		if (x < 0)
			x = 0;
		else if (x >= width)
			x = (width - 1);
		if (y < 0)
			y = 0;
		else if (y >= height)
			y = (height - 1);
		lastDx = dx;
		lastDy = dy;

		result.push_back(make_pair(make_pair(x, y), make_tuple(color[0], color[1], color[2])));
	}

	return result;
}