#include "ImageUtils.h"
#include <cstring>
#include <cmath>

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
			dy = -dy;

			result[idx] = dx;
			result[idx + 1] = dy;
		}
#undef IXY

	return result;
}

double* ImageUtils::getGaussianKernel(double sigma, unsigned int radius)
{
	static double e = 2.718281828;
	int size = radius * 2 + 1;
	double* kernel = new double[size * size];
	for (int y = 0; y < size; ++y)
		for (int x = 0; x < size; ++x)
		{
			int dx = x - radius;
			int dy = y - radius;
			double t = pow(e, -((dx * dx + dy * dy) / (2 * sigma)));
			kernel[x + y * size] = t / (sqrt(2 * 3.14159265 * sigma));
		}
	return kernel;
}

/* Fast gaussian algorithm found here:
   http://blog.ivank.net/fastest-gaussian-blur.html
*/
int* getGaussianBoxes(double sigma, int n)
{
	double idealWidth = sqrtf(12 * sigma * sigma / (float)n + 1);
	int widthLower = floor(idealWidth);
	if (widthLower % 2 == 0)
		--widthLower;
	int widthUpper = widthLower + 2;

	int m = round((12 * sigma*sigma - n*widthLower*widthLower - 4 * n*widthLower - 3 * n) / (-4 * widthLower - 4));

	int* result = new int[n];
	for (int i = 0; i < n; ++i)
	{
		result[i] = i < m ? widthLower : widthUpper;
	}
	return result;
}

void boxBlurH(unsigned char* image, unsigned char* target, int width, int height, int radius)
{
	double iarr = 1 / (double)(2 * radius + 1);
	for (int i = 0; i < height; ++i)
	{
		int ti = i * width;
		int li = ti;
		int ri = ti + radius;
		int fv = image[ti];
		int lv = image[ti + width - 1];
		int val = (radius + 1) * fv;

		for (int j = 0; j < radius; ++j)
			val += image[ti + j];
		for (int j = 0; j <= radius; ++j)
		{
			val += image[ri] - fv;
			target[ti] = round(val * iarr);
			++ri;
			++ti;
		}
		for (int j = radius + 1; j < width - radius; ++j)
		{
			val += image[ri] - image[li];
			target[ti] = round(val * iarr);
			++li;
			++ri;
			++ti;
		}
		for (int j = width - radius; j < width; ++j)
		{
			val += lv - image[li];
			target[ti] = round(val * iarr);
			++li;
			++ti;
		}
	}
}

void boxBlurT(unsigned char* image, unsigned char* target, int width, int height, int radius)
{
	double iarr = 1 / (double)(2 * radius + 1);
	for (int i = 0; i < width; ++i)
	{
		int ti = i;
		int li = ti;
		int ri = ti + radius * width;
		int fv = image[ti];
		int lv = image[ti + width * (height - 1)];
		int val = (radius + 1) * fv;

		for (int j = 0; j < radius; ++j)
			val += image[ti + j * width];
		for (int j = 0; j <= radius; ++j)
		{
			val += image[ri] - fv;
			target[ti] = round(val * iarr);
			ri += width;
			ti += width;
		}
		for (int j = radius + 1; j < height - radius; ++j)
		{
			val += image[ri] - image[li];
			target[ti] = round(val * iarr);
			ri += width;
			li += width;
			ti += width;
		}
		for (int j = height - radius; j < height; ++j)
		{
			val += lv - image[li];
			target[ti] = round(val * iarr);
			li += width;
			ti += width;
		}
	}
}

void boxBlur(unsigned char* image, unsigned char* target, int width, int height, int radius)
{
	memcpy(target, image, width * height);
	boxBlurH(target, image, width, height, radius);
	boxBlurT(image, target, width, height, radius);
}

unsigned char* ImageUtils::fastGaussianBlur(double sigma, const unsigned char* image, int width, int height)
{
	unsigned char* image2 = new unsigned char[width * height];
	unsigned char* result = new unsigned char[width * height];
	memcpy(image2, image, width * height);
	memcpy(result, image, width * height);

	int* boxes = getGaussianBoxes(sigma, 3);
	boxBlur(image2, result, width, height, (boxes[0] - 1) / 2);
	boxBlur(result, image2, width, height, (boxes[1] - 1) / 2);
	boxBlur(image2, result, width, height, (boxes[2] - 1) / 2);

	delete boxes;
	delete image2;
	return result;
}

#define BOUND_TO(x, l, u) (x < l ? l : (x > u ? u : x))

unsigned char* ImageUtils::getFilteredImage(
	const double* kernel, unsigned int kernelWidth, unsigned int kernelHeight,
	const unsigned char* image, unsigned int width, unsigned int height,
	unsigned int xBegin, unsigned int xEnd, unsigned int yBegin, unsigned int yEnd,
	unsigned int channels, int boundaryMode)
{
	if (kernelWidth == 0 ||
		kernelHeight == 0 ||
		width == 0 ||
		height == 0 ||
		xBegin >= width || 
		yBegin >= height || 
		xEnd >= width || 
		yEnd >= height || 
		channels == 0 ||
		channels > 4 ||
		kernelWidth % 2 != 1 || 
		kernelHeight % 2 != 1)
	{
		return NULL;
	}

	if (xEnd <= xBegin)
		xEnd = width - 1;
	if (yEnd <= yBegin)
		yEnd = height - 1;

	unsigned int resultSize = (xEnd - xBegin + 1) * (yEnd - yBegin + 1) * channels;
	unsigned char* result = new unsigned char[resultSize];
	memset(result, 0, resultSize);

	double kernelWeightSum = 0;
	for (unsigned int i = 0; i < kernelWidth * kernelHeight; ++i)
	{
		kernelWeightSum += kernel[i];
	}

	int dxMax = (kernelWidth - 1) / 2;
	int dyMax = (kernelHeight - 1) / 2;

	double sum[4];
	double currSum;
	unsigned int resultIdx = 0;
	for (int y = yBegin; y <= yEnd; ++y)
		for (int x = xBegin; x <= xEnd; ++x)
		{
			// zero the sum
			for (int channel = 0; channel < channels; ++channel)
				sum[channel] = 0;
			// compute the sum
			bool innerRunning = true;
			unsigned int kernelIdx = 0;
			for (int fy = y - dyMax; innerRunning && fy <= y + dyMax; ++fy)
				for (int fx = x - dxMax; fx <= x + dxMax; ++fx)
				{
					int cx = fx;
					int cy = fy;
					if (cx < 0 || cx >= width || cy < 0 || cy >= height)
					{
						switch (boundaryMode)
						{
						case IMAGE_UTIL_EXTEND_BOUNDARY:
							cx = BOUND_TO(cx, 0, width - 1);
							cy = BOUND_TO(cy, 0, height - 1);
							break;
						case IMAGE_UTIL_WRAP_BOUNDARY:
							cx = cx < 0 ? -cx : (cx >= width ? 2 * width - cx - 1: cx);
							cy = cy < 0 ? -cy : (cy >= height ? 2 * height - cy - 1 : cy);
							break;
						case IMAGE_UTIL_IGNORE_BOUNDARY:
							innerRunning = false;
							break;
						default:
							cx = BOUND_TO(cx, 0, width - 1);
							cy = BOUND_TO(cy, 0, height - 1);
							break;
						}
					}

					for (int channel = 0; channel < channels; ++channel)
						sum[channel] += kernel[kernelIdx] * image[(cx + cy * width) * channels + channel];
					++kernelIdx;
				}

			// normalize the result
			for (int channel = 0; channel < channels; ++channel)
			{
				currSum = sum[channel] / kernelWeightSum;
				result[resultIdx + channel] = (unsigned char)BOUND_TO(currSum, 0, 255);
			}
			resultIdx += channels;
		}

	return result;
}

unsigned char* ImageUtils::getFilteredImageCB(
	unsigned char(*kernel)(unsigned char*, int, int, int, int), unsigned int kernelWidth, unsigned int kernelHeight,
	const unsigned char* image, unsigned int width, unsigned int height,
	unsigned int xBegin, unsigned int xEnd, unsigned int yBegin, unsigned int yEnd,
	unsigned int channels, int boundaryMode)
{
	if (kernelWidth == 0 ||
		kernelHeight == 0 ||
		width == 0 ||
		height == 0 ||
		xBegin >= width ||
		yBegin >= height ||
		xEnd >= width ||
		yEnd >= height ||
		channels == 0 ||
		channels > 4 ||
		kernelWidth % 2 != 1 ||
		kernelHeight % 2 != 1)
	{
		return NULL;
	}

	if (xEnd <= xBegin)
		xEnd = width - 1;
	if (yEnd <= yBegin)
		yEnd = height - 1;

	unsigned int resultSize = (xEnd - xBegin + 1) * (yEnd - yBegin + 1) * channels;
	unsigned char* result = new unsigned char[resultSize];
	memset(result, 0, resultSize);

	int dxMax = (kernelWidth - 1) / 2;
	int dyMax = (kernelHeight - 1) / 2;

	unsigned char* kernelParam = new unsigned char[kernelWidth * kernelHeight];
	unsigned int resultIdx = 0;
	for (int y = yBegin; y <= yEnd; ++y)
		for (int x = xBegin; x <= xEnd; ++x)
		{
			for (int channel = 0; channel < channels; ++channel)
			{
				memset(kernelParam, 0, kernelWidth * kernelHeight);
				int kernelX = 0;
				int kernelY = 0;
				// prepare the kernel parameters
				bool innerRunning = true;
				for (int fy = y - dyMax; innerRunning && fy <= y + dyMax; ++fy, ++kernelY)
				{
					kernelX = 0;
					for (int fx = x - dxMax; fx <= x + dxMax; ++fx, ++kernelX)
					{
						int cx = fx;
						int cy = fy;
						if (cx < 0 || cx >= width || cy < 0 || cy >= height)
						{
							switch (boundaryMode)
							{
							case IMAGE_UTIL_EXTEND_BOUNDARY:
								cx = BOUND_TO(cx, 0, width - 1);
								cy = BOUND_TO(cy, 0, height - 1);
								break;
							case IMAGE_UTIL_WRAP_BOUNDARY:
								cx = cx < 0 ? -cx : (cx >= width ? 2 * width - cx - 1 : cx);
								cy = cy < 0 ? -cy : (cy >= height ? 2 * height - cy - 1 : cy);
								break;
							case IMAGE_UTIL_IGNORE_BOUNDARY:
								innerRunning = false;
								break;
							default:
								cx = BOUND_TO(cx, 0, width - 1);
								cy = BOUND_TO(cy, 0, height - 1);
								break;
							}
						}
						kernelParam[kernelY * kernelHeight + kernelX] = image[(cx + cy * width) * channels + channel];
					}
				}
				result[resultIdx + channel] = kernel(kernelParam, kernelWidth, kernelHeight, x, y);
			}
			resultIdx += channels;
		}

	return result;
}