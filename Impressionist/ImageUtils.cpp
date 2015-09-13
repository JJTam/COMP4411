#include "ImageUtils.h"
#include <cstring>

unsigned char * ImageUtils::getSingleChannel(double rFactor, double gFactor, double bFactor, 
	const unsigned char * image, int width, int height)
{
	unsigned char* result = new unsigned char[width * height];
	for (int i = 0; i < width * height; ++i)
	{
		double val = rFactor * image[i * 3]
			+ gFactor * image[i * 3 + 1]
			+ bFactor * image[i * 3 + 2];

		result[i] = (val > 255.0) ? 255 : (unsigned char)val;
	}

	return result;
}

int* ImageUtils::getGradientBySobel(const unsigned char* image, int width, int height)
{
	int* result = new int[width * height * 2];
	memset(result, 0, width * height * 2 * sizeof(int));

// short-cut to get index of (x, y)
#define IXY(x, y) (x + (y) * width)
	for (int y = 1; y < height - 1; ++y)
		for (int x = 1; x < width - 1; ++x)
		{
			int idx = 2 * (x + y * width);
			int dx = 0
				+ (int)(image[IXY(x + 1, y - 1)])
				+ (int)(image[IXY(x + 1, y)]) * 2
				+ (int)(image[IXY(x + 1, y + 1)])
				- (int)(image[IXY(x - 1, y - 1)])
				- (int)(image[IXY(x - 1, y)]) * 2
				- (int)(image[IXY(x - 1, y + 1)]);

			int dy = 0
				+ (int)(image[IXY(x - 1, y - 1)])
				+ (int)(image[IXY(x, y - 1)]) * 2
				+ (int)(image[IXY(x + 1, y - 1)])
				- (int)(image[IXY(x - 1, y + 1)])
				- (int)(image[IXY(x, y + 1)]) * 2
				- (int)(image[IXY(x + 1, y + 1)]);

			result[idx] = dx;
			result[idx + 1] = dy;
		}
#undef IXY

	return result;
}
