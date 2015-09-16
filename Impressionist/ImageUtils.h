/*
ImageUtils.h

Header file of class ImageUtils, which contains some helper functions.
*/
#ifndef _IMAGE_UTILS
#define _IMAGE_UTILS

enum ImageUitlsBoundaryMode
{
	IMAGE_UTIL_EXTEND_BOUNDARY,
	IMAGE_UTIL_WRAP_BOUNDARY,
	IMAGE_UTIL_IGNORE_BOUNDARY
};

class ImageUtils
{
public:
	static unsigned char* getSingleChannel(double rFactor, double gFactor, double bFactor,
		const unsigned char * image, int width, int height);
	static unsigned char* fastGaussianBlur(double sigma, const unsigned char* image, int width, int height);
	static int* getGradientBySobel(const unsigned char* image, int width, int height);

	// Get an array of dimension (xEnd-xBegin)*(yEnd-yBegin)*channels that contains the image
	// filtered by the input filter channel-wise.
	// If xEnd <= xBegin or yEnd <= yBegin they will be set to width-1 and height-1 respectively
	// Returns NULL if input is invalid or error occurs.
	static unsigned char* getFilteredImage(const double* kernel, unsigned int kernelWidth, unsigned int kernelHeight,
		const unsigned char* image, unsigned int width, unsigned int height,
		unsigned int xBegin, unsigned int xEnd, unsigned int yBegin, unsigned int yEnd,
		unsigned int channels, int boundaryMode = IMAGE_UTIL_EXTEND_BOUNDARY);
};

#endif // _IMAGE_UTILS