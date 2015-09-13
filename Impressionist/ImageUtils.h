/*
ImageUtils.h

Header file of class ImageUtils, which contains some helper functions.
*/
#ifndef _IMAGE_UTILS
#define _IMAGE_UTILS

class ImageUtils
{
public:
	static unsigned char* getSingleChannel(double rFactor, double gFactor, double bFactor,
		const unsigned char * image, int width, int height);
	static unsigned char* getGaussianBlur(int size, const unsigned char* image, int width, int height);
	static int* getGradientBySobel(const unsigned char* image, int width, int height);

private:
	static unsigned char* getFilteredImage(const int* filter, int filterWidth, int filterHeight,
		const unsigned char* image, int width, int height, bool ignoreOutOfRange);
};

#endif // _IMAGE_UTILS