// 
// impressionistDoc.cpp
//
// It basically maintain the bitmap for answering the color query from the brush.
// It also acts as the bridge between brushes and UI (including views)
//

#include <FL/fl_ask.H>

#include "impressionistDoc.h"
#include "impressionistUI.h"

#include "ImpBrush.h"

// Include individual brush headers here.
#include "PointBrush.h"
#include "CircleBrush.h"
#include "LineBrush.h"
#include "ScatteredLineBrush.h"
#include "ScatteredPointBrush.h"
#include "ScatteredCircleBrush.h"
#include "CurvedBrush.h"
#include "FilterBrush.h"
#include "AlphaMappedBrush.h"

#include "ImageUtils.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>
#include <sstream>

#define DESTROY(p)	{  if ((p)!=NULL) {delete [] p; p=NULL; } }
#define ABS(x) (x >= 0 ? x : -x)
extern int irand(int);

ImpressionistDoc::ImpressionistDoc() 
{
	// Set NULL image name as init. 
	m_imageName[0]	='\0';	

	m_nDisplayMode = DOC_DISPLAY_ORIGINAL;

	m_nWidth		= -1;
	m_ucBitmap		= NULL;
	m_ucAnotherBitmap = NULL;
	m_ucPainting	= NULL;
	m_ucPreservedPainting = NULL;
	m_iGradient = NULL;
	m_iAnotherGradient = NULL;
	m_iGradientMagnitude = NULL;
	m_ucEdgeBitmap = NULL;
	m_ucBitmapBlurred = NULL;
	m_ucBitmapFiltered = NULL;
	m_ucAlphaBrush = NULL;
	m_nFilterType = FB_GAUSSIAN_FILTER;

	// create one instance of each brush
	ImpBrush::c_nBrushCount	= NUM_BRUSH_TYPE;
	ImpBrush::c_pBrushes	= new ImpBrush* [ImpBrush::c_nBrushCount];

	ImpBrush::c_pBrushes[BRUSH_POINTS]	= new PointBrush( this, "Points" );

	// Note: You should implement these 5 brushes.  They are set the same (PointBrush) for now
	ImpBrush::c_pBrushes[BRUSH_LINES]				
		= new LineBrush( this, "Lines" );
	ImpBrush::c_pBrushes[BRUSH_CIRCLES]				
		= new CircleBrush( this, "Circles" );
	ImpBrush::c_pBrushes[BRUSH_SCATTERED_POINTS]	
		= new ScatteredPointBrush(this, "Scattered Points");
	ImpBrush::c_pBrushes[BRUSH_SCATTERED_LINES]		
		= new ScatteredLineBrush( this, "Scattered Lines" );
	ImpBrush::c_pBrushes[BRUSH_SCATTERED_CIRCLES]	
		= new ScatteredCircleBrush(this, "Scattered Circles");
	ImpBrush::c_pBrushes[BRUSH_CURVED]
		= new CurvedBrush(this, "Curved Brush");
	ImpBrush::c_pBrushes[BRUSH_FILTERED]
		= new FilterBrush(this, "Filter Brush");
	ImpBrush::c_pBrushes[BRUSH_ALPHAMAPPED]
		= new AlphaMappedBrush(this, "Alpha Mapped Brush");
	// make one of the brushes current
	m_pCurrentBrush	= ImpBrush::c_pBrushes[0];
	m_nBrushDirection = SLIDER_AND_RIGHT_MOUSE;

	m_bHasPendingUndo = false;
	m_bHasPendingBgUpdate = false;
}


//---------------------------------------------------------
// Set the current UI 
//---------------------------------------------------------
void ImpressionistDoc::setUI(ImpressionistUI* ui) 
{
	m_pUI	= ui;
}

//---------------------------------------------------------
// Returns the active picture/painting name
//---------------------------------------------------------
char* ImpressionistDoc::getImageName() 
{
	return m_imageName;
}

//---------------------------------------------------------
// Called by the UI when the user changes the brush type.
// type: one of the defined brush types.
//---------------------------------------------------------
void ImpressionistDoc::setBrushType(int type)
{
	m_pCurrentBrush	= ImpBrush::c_pBrushes[type];
	
	// deactive all sliders
	// add new lines when new sliders are available
	m_pUI->m_BrushSizeSlider->deactivate();
	m_pUI->m_LineWidthSlider->deactivate();
	m_pUI->m_AngleSlider->deactivate();
	m_pUI->m_AlphaSlider->deactivate();

	// select used sliders
	// add other cases when new brushes are implemented
	switch (type)
	{
	case BRUSH_POINTS:
	case BRUSH_CIRCLES:
	case BRUSH_SCATTERED_POINTS:
	case BRUSH_SCATTERED_CIRCLES:
	case BRUSH_CURVED:
	case BRUSH_FILTERED:
		m_pUI->m_BrushSizeSlider->activate();
		m_pUI->m_AlphaSlider->activate();
		break;
	case BRUSH_LINES:
	case BRUSH_SCATTERED_LINES:
		m_pUI->m_BrushSizeSlider->activate();
		m_pUI->m_LineWidthSlider->activate();
		m_pUI->m_AngleSlider->activate();
		m_pUI->m_AlphaSlider->activate();
		break;
	case BRUSH_ALPHAMAPPED:
		m_pUI->m_AlphaSlider->activate();
		break;
	default:
		break;
	}
}

//---------------------------------------------------------
// Returns the size of the brush.
//---------------------------------------------------------
int ImpressionistDoc::getSize()
{
	return m_pUI->getSize();
}
int ImpressionistDoc::getLineWidth()
{
	return m_pUI->getLineWidth();
}
int ImpressionistDoc::getAngle()
{
	return m_pUI->getAngle();
}
double ImpressionistDoc::getAlpha()
{
	return m_pUI->getAlpha();
}
//---------------------------------------------------------
// Load the specified image
// This is called by the UI when the load image button is 
// pressed.
//---------------------------------------------------------
int ImpressionistDoc::loadImage(char *iname, bool isMural) 
{
	// try to open the image to read
	unsigned char*	data;
	int				width, 
					height;

	if ( (data=readBMP(iname, width, height))==NULL ) 
	{
		fl_alert("Can't load bitmap file");
		return 0;
	}

	// require the mural image to have the same dimension
	if (isMural)
	{
		if (width != m_nWidth || height != m_nHeight)
		{
			fl_alert("Mural image shall have the same dimension as the original image.");
			delete[] data;
			return 0;
		}
	}

	// reflect the fact of loading the new image
	m_nWidth		= width;
	m_nPaintWidth	= width;
	m_nHeight		= height;
	m_nPaintHeight	= height;

	// release old storage
	if (m_ucBitmap) delete [] m_ucBitmap;
	if (m_iGradient) delete[] m_iGradient;
	if (m_iGradientMagnitude) delete[] m_iGradientMagnitude;
	if (m_ucEdgeBitmap) delete[] m_ucEdgeBitmap;
	if (m_ucBitmapBlurred) delete[] m_ucBitmapBlurred;
	if (m_ucBitmapFiltered) delete[] m_ucBitmapFiltered;
	m_ucBitmapBlurred = NULL;

	// setup new original image
	m_ucBitmap		= data;
	m_ucBitmapFiltered = new unsigned char[width * height * 3];
	memcpy(m_ucBitmapFiltered, m_ucBitmap, width * height * 3);

	////// compute gradient

	// temp black and white image, intensity = 0.299R + 0.587G + 0.144B
	unsigned char* bw = ImageUtils::getSingleChannel(0.299, 0.587, 0.144, m_ucBitmap, width, height);
	//unsigned char* bwBlurred = ImageUtils::fastGaussianBlur(1, bw, width, height);

	
	static double kernel[25] = {
		0.000106788745393375, 0.002144909288579413, 0.005830467942838339, 0.002144909288579413, 0.000106788745393375,
		0.002144909288579413, 0.043081654712647834, 0.11710807914533564, 0.043081654712647834, 0.002144909288579413,
		0.005830467942838339, 0.11710807914533564, 0.3183327635065042, 0.11710807914533564, 0.005830467942838339,
		0.002144909288579413, 0.043081654712647834, 0.11710807914533564, 0.043081654712647834, 0.002144909288579413,
		0.000106788745393375, 0.002144909288579413, 0.005830467942838339, 0.002144909288579413, 0.000106788745393375
	};
	unsigned char* bwBlurred = ImageUtils::getFilteredImage(kernel, 5, 5, bw, width, height, 0, 0, 0, 0, 1, IMAGE_UTIL_WRAP_BOUNDARY);
	

	m_iGradient = ImageUtils::getGradientBySobel(bwBlurred, width, height);

	// compute gradient magnitude
	m_iGradientMagnitude = new int[width * height];
	memset(m_iGradientMagnitude, 0, width * height * sizeof(int));

	for (int i = 0; i < width * height; ++i)
	{
		int dx = m_iGradient[i * 2];
		int dy = m_iGradient[i * 2 + 1];
		m_iGradientMagnitude[i] = sqrt(dx * dx + dy * dy);
	}

	// release memory
	delete bw;
    delete bwBlurred;
    
	// update edge
	m_ucEdgeBitmap = new unsigned char[width * height * 3];
	updateEdge();

	// conditionally clear canvas and resize window
	if (!isMural)
	{
		// release memory
		if (m_ucPainting) delete[] m_ucPainting;
		if (m_ucPreservedPainting) delete[] m_ucPreservedPainting;
		m_lUndoList.clear();

		// allocate space for draw view
		m_ucPainting = new unsigned char[width*height * 4];
		memset(m_ucPainting, 0, width*height * 4);

		m_ucPreservedPainting = new unsigned char[width*height * 4];
		memset(m_ucPreservedPainting, 0, width*height * 4);

		// Resize the window
		m_pUI->m_mainWindow->resize(m_pUI->m_mainWindow->x(),
			m_pUI->m_mainWindow->y(),
			width * 2,
			height + 25);

		// On some platforms, the width and height both has 8 pixels missing for some reason
		int widthDelta = ABS(width * 2 - m_pUI->m_mainWindow->w());
		int heightDelta = ABS(height + 25 - m_pUI->m_mainWindow->h());
		if (widthDelta > 0 || heightDelta > 0)
		{
			m_pUI->m_mainWindow->resize(m_pUI->m_mainWindow->x(),
				m_pUI->m_mainWindow->y(),
				width * 2 + widthDelta,
				height + 25 + heightDelta);
		}

		// Resize windows
		m_pUI->m_origView->resizeWindow(width, height);
		m_pUI->m_paintView->resizeWindow(width, height);
	}

	m_pUI->m_origView->refresh();
	m_pUI->m_paintView->refresh();

	return 1;
}

int ImpressionistDoc::loadAnotherImage(char *iname)
{
	// try to open the image to read
	unsigned char*	data;
	int				width, height;

	if ((data = readBMP(iname, width, height)) == NULL)
	{
		fl_alert("Can't load bitmap file");
		return 0;
	}

	if (m_nWidth != width || m_nHeight != height)
	{
		fl_alert("The size must be same!");
		delete[] data;
		return 0;
	}

	// release old storage
	if (m_ucAnotherBitmap) delete[] m_ucAnotherBitmap;
	if (m_iAnotherGradient) delete[] m_iAnotherGradient;

	m_ucAnotherBitmap = data;

	// compute gradient
	unsigned char* bw = ImageUtils::getSingleChannel(0.299, 0.587, 0.144, m_ucAnotherBitmap, width, height);
	unsigned char* bwBlurred = ImageUtils::fastGaussianBlur(1, bw, width, height);

	m_iAnotherGradient = ImageUtils::getGradientBySobel(bw, width, height);

	// release memory
	delete bw;
	delete bwBlurred;

	return 1;
}

int ImpressionistDoc::loadEdgeImage(char *iname)
{
	// try to open the image to read
	unsigned char*	data;
	int				width, height;

	if ((data = readBMP(iname, width, height)) == NULL)
	{
		fl_alert("Can't load bitmap file");
		return 0;
	}

	if (m_nWidth != width || m_nHeight != height)
	{
		fl_alert("The size must be same!");
		delete[] data;
		return 0;
	}

	// release old storage
	if (m_ucEdgeBitmap) delete[] m_ucEdgeBitmap;

	m_ucEdgeBitmap = data;

	return 1;
}
//----------------------------------------------------------------
// Save the specified image
// This is called by the UI when the save image menu button is 
// pressed.
//----------------------------------------------------------------
int ImpressionistDoc::saveImage(char *iname, int type) 
{

	// writeBMP(iname, m_nPaintWidth, m_nPaintHeight, m_ucPainting);
	
	int sourceDim = 4;
	unsigned char* source = m_ucPreservedPainting;

	switch (type)
	{
	case DOC_SAVE_DRAWING:
		break;
	case DOC_SAVE_DRAWING_WITH_BG:
		source = m_ucPainting;
		break;
	case DOC_SAVE_EDGE:
		source = m_ucEdgeBitmap;
		sourceDim = 3;
		break;
	case DOC_SAVE_FILTERED:
		source = m_ucBitmapFiltered;
		sourceDim = 3;
		break;
	default:
		break;
	}

	// copy data
	if (sourceDim != 3)
	{
		unsigned char* rgb = new unsigned char[m_nWidth*m_nHeight * 3];
		for (int i = 0; i < m_nWidth*m_nHeight; ++i)
		{
			rgb[i * 3] = source[i * sourceDim];
			rgb[i * 3 + 1] = source[i * sourceDim + 1];
			rgb[i * 3 + 2] = source[i * sourceDim + 2];
		}
		writeBMP(iname, m_nWidth, m_nHeight, rgb);
		delete[] rgb;
	}
	else
	{
		writeBMP(iname, m_nWidth, m_nHeight, source);
	}

	return 1;
}

//----------------------------------------------------------------
// Clear the drawing canvas
// This is called by the UI when the clear canvas menu item is 
// chosen
//-----------------------------------------------------------------
int ImpressionistDoc::clearCanvas() 
{
	// Release old storage
	if (m_ucPreservedPainting)
	{
		//delete[] m_ucPreservedPainting;
		//m_ucPreservedPainting = new unsigned char[m_nWidth*m_nHeight * 4];

		// push it to undo
		unsigned char* t = new unsigned char[m_nWidth * m_nHeight * 4];
		memcpy(t, m_ucPreservedPainting, m_nWidth * m_nHeight * 4);
		m_lUndoList.push_back(t);

		memset(m_ucPreservedPainting, 0, m_nWidth*m_nHeight * 4);
	}

	if ( m_ucPainting ) 
	{
		//delete [] m_ucPainting;
		//m_ucPainting = new unsigned char[m_nWidth*m_nHeight * 4];
		memset(m_ucPainting, 0, m_nWidth*m_nHeight * 4);

		// refresh paint view as well	
		// mark bg update
		m_bHasPendingBgUpdate = true;
		m_pUI->m_paintView->refresh();
	}
	
	return 0;
}

int ImpressionistDoc::autoDraw()
{
	m_pUI->m_paintView->SimulateMouse(0, 0, PV_NORMAL_AUTO);
	return 0;
}
int ImpressionistDoc::paintlyDraw()
{
	clearCanvas();
	m_pUI->m_paintView->flush();

	m_bIsPaintlyBegin = true;
	for (int l = 0; l < m_pUI->getPaintlyLevel(); ++l)
	{
		int currentR = m_pUI->getPaintlyR0() - l;
		if (currentR < 0)
		{
			break;
		}
		else
		{
			m_nPaintlySize = 1 << currentR;
			m_pUI->m_paintView->SimulateMouse(0, 0, PV_PAINTLY_AUTO);
		}
		m_bIsPaintlyBegin = false;
	}
	
	return 0;
}
//------------------------------------------------------------------
// Get the color of the pixel in the original image at coord x and y
//------------------------------------------------------------------
GLubyte* ImpressionistDoc::GetOriginalPixel( int x, int y )
{
	if ( x < 0 ) 
		x = 0;
	else if ( x >= m_nWidth ) 
		x = m_nWidth-1;

	if ( y < 0 ) 
		y = 0;
	else if ( y >= m_nHeight ) 
		y = m_nHeight-1;

	return (GLubyte*)(m_ucBitmap + 3 * (y*m_nWidth + x));
}

GLubyte* ImpressionistDoc::GetFilteredPixel(int x, int y)
{
	if (x < 0)
		x = 0;
	else if (x >= m_nWidth)
		x = m_nWidth - 1;

	if (y < 0)
		y = 0;
	else if (y >= m_nHeight)
		y = m_nHeight - 1;

	return (GLubyte*)(m_ucBitmapFiltered + 3 * (y*m_nWidth + x));
}

//----------------------------------------------------------------
// Get the color of the pixel in the original image at point p
//----------------------------------------------------------------
GLubyte* ImpressionistDoc::GetOriginalPixel( const Point p )
{
	return GetOriginalPixel( p.x, p.y );
}

void ImpressionistDoc::undo()
{
	if (m_lUndoList.size() >= 1)
	{
		//printf("Poping from undo list...\n");
		delete[] m_ucPreservedPainting;
		m_ucPreservedPainting = m_lUndoList.back();
		m_lUndoList.pop_back();
		m_bHasPendingUndo = true;

		m_pUI->m_paintView->redraw();
	}
}

void ImpressionistDoc::pushToUndo()
{
	m_lUndoList.push_back(m_ucPreservedPainting);

	unsigned char* t = new unsigned char[m_nWidth * m_nHeight * 4];
	memcpy(t, m_ucPreservedPainting, m_nWidth * m_nHeight * 4);
	m_ucPreservedPainting = t;
}

void ImpressionistDoc::updateBg()
{
	if (m_pUI->m_paintView)
	{
		m_bHasPendingBgUpdate = true;
		m_pUI->m_paintView->redraw();
	}
}

void ImpressionistDoc::updateEdge()
{
	if (m_ucEdgeBitmap)
	{
		int t = m_pUI->getEdgeThreshold();

		memset(m_ucEdgeBitmap, 0, m_nWidth * m_nHeight * 3);
		for (int i = 0; i < m_nWidth * m_nHeight; ++i)
		{
			if (m_iGradientMagnitude[i] >= t)
			{
				m_ucEdgeBitmap[3 * i] = 255;
				m_ucEdgeBitmap[3 * i + 1] = 255;
				m_ucEdgeBitmap[3 * i + 2] = 255;
			}
		}

		if (m_nDisplayMode == DOC_DISPLAY_EDGE)
			m_pUI->m_origView->refresh();
	}
}

int ImpressionistDoc::getDisplayMode()
{
	return m_nDisplayMode;
}

void ImpressionistDoc::setDisplayMode(int mode)
{
	if (mode == m_nDisplayMode)
		return;
	switch (mode)
	{
	case DOC_DISPLAY_ANOTHER:
		if (!m_ucAnotherBitmap)
		{
			fl_alert("Please load another bitmap first.");
			mode = m_nDisplayMode;
		}
		break;
	case DOC_DISPLAY_BLURRED:
		if (!m_ucBitmapBlurred)
		{
			fl_alert("The bitmap has not been blurred yet.");
			mode = m_nDisplayMode;
		}
		break;
	case DOC_DISPLAY_EDGE:
	case DOC_DISPLAY_ORIGINAL:
	case DOC_DISPLAY_FILTERED:
		break;
	default:
		mode = DOC_DISPLAY_ORIGINAL;
		break;
	}

	m_nDisplayMode = mode;
	m_pUI->m_origView->refresh();
}

void ImpressionistDoc::setFilterType(int type)
{
	switch (type)
	{
	case FB_GAUSSIAN_FILTER:
	case FB_MEAN_FILTER:
	case FB_MEDIAN_FILTER:
	case FB_CUSTOM_FILTER:
		break;
	default:
		type = FB_GAUSSIAN_FILTER;
		break;
	}
	m_nFilterType = type;
}

bool parseKernel(ImpressionistDoc* pDoc, double*& kernel, int& width, int& height)
{
	using namespace std;
	string s = pDoc->m_pUI->m_FilterKernelInput->value();
	stringstream ss(s);
	ss >> width; ss >> height;
	if (!ss.good() || width <= 0 || height <= 0 || width % 2 == 0 || height % 2 == 0)
	{
		return false;
	}

	double* result = new double[width * height];
	for (int i = 0; i < width * height; ++i)
	{
		ss >> result[i];
		if (!ss.good() && !(i == width * height - 1 && ss.eof()))
		{
			delete[] result;
			return false;
		}
	}

	kernel = result;
	return true;
}

void ImpressionistDoc::updateFiltered()
{
	unsigned char* preservedBitmap = m_ucBitmapFiltered;
	unsigned char* filterTarget = m_ucBitmap;
	if (m_pUI->getFilterOnCurrent())
	{
		filterTarget = m_ucBitmapFiltered;
	}

	double* kernel = NULL;
	int kernelSize = m_pUI->getFilterRadius() * 2 + 1;
	// for custom kernel
	int cKW = 0;
	int cKH = 0;
	switch (m_nFilterType)
	{
	case FB_GAUSSIAN_FILTER:
		kernel = ImageUtils::getGaussianKernel(m_pUI->getFilterSigma(), m_pUI->getFilterRadius());
		m_ucBitmapFiltered = ImageUtils::getFilteredImage(kernel, kernelSize, kernelSize, filterTarget, m_nWidth, m_nHeight, 0, 0, 0, 0, 3, IMAGE_UTIL_WRAP_BOUNDARY);
		break;
	case FB_MEAN_FILTER:
		kernel = new double[kernelSize * kernelSize];
		for (int i = 0; i < kernelSize * kernelSize; ++i)
			kernel[i] = 1;
		m_ucBitmapFiltered = ImageUtils::getFilteredImage(kernel, kernelSize, kernelSize, filterTarget, m_nWidth, m_nHeight, 0, 0, 0, 0, 3, IMAGE_UTIL_WRAP_BOUNDARY);
		break;
	case FB_MEDIAN_FILTER:
		if (kernelSize >= 1)
			m_ucBitmapFiltered = ImageUtils::getFilteredImageCB([](unsigned char* p, int kW, int kH, int, int)->unsigned char {
			
			qsort(p, kW * kH, sizeof(unsigned char), [](const void* a, const void* b)->int { return *(unsigned char*)a - *(unsigned char*)b; });
			return p[kW * kH / 2];

		}
		, kernelSize, kernelSize, filterTarget, m_nWidth, m_nHeight, 0, 0, 0, 0, 3, IMAGE_UTIL_WRAP_BOUNDARY);
		break;
	case FB_CUSTOM_FILTER:
		if (parseKernel(this, kernel, cKW, cKH))
		{
			m_ucBitmapFiltered = ImageUtils::getFilteredImage(kernel, cKW, cKH, filterTarget, m_nWidth, m_nHeight, 0, 0, 0, 0, 3, IMAGE_UTIL_WRAP_BOUNDARY);
		}
		else
		{
			fl_alert("Error parsing your kernel.");
		}
		break;
	default:
		break;
	}

	if (kernel)
		delete[] kernel;

	if (m_ucBitmapFiltered == NULL)
	{
		m_ucBitmapFiltered = preservedBitmap;
		fl_alert("Some error occurred while processing.");
	}
	else
	{
		if (m_ucBitmapFiltered != preservedBitmap)
		{
			delete[] preservedBitmap;
		}

		m_nDisplayMode = DOC_DISPLAY_FILTERED;
		m_pUI->m_origView->refresh();
	}
}

void ImpressionistDoc::normalizeKernel()
{
	double* kernel = NULL;
	int kw = 0;
	int kh = 0;
	double ksum = 0;
	if (parseKernel(this, kernel, kw, kh))
	{
		using namespace std;
		stringstream ss;
		ss << kw << ' ' << kh << endl;
		for (int i = 0; i < kh * kw; ++i)
			ksum += kernel[i];

		if (ksum == 0.0)
		{
			fl_alert("Sum of your kernel is zero. Stopping normalization.");
		}
		else
		{
			for (int i = 0; i < kh; ++i)
				for (int j = 0; j < kw; ++j)
				{
					ss << kernel[i * kw + j] / ksum;
					if (j == kw - 1)
						ss << endl;
					else
						ss << ' ';
				}
			string s = ss.str();
			m_pUI->m_FilterKernelInput->value(s.c_str());
		}

		delete[] kernel;
	}
	else
	{
		fl_alert("Error parsing your kernel.");
	}
}

int ImpressionistDoc::loadAlphaBrush(char* iname)
{
	// try to open the image to read
	unsigned char*	data;
	int				width, height;

	if ((data = readBMP(iname, width, height)) == NULL)
	{
		fl_alert("Can't load bitmap file");
		return 0;
	}

	if (m_ucAlphaBrush)
		delete[] m_ucAlphaBrush;
	m_ucAlphaBrush = new unsigned char[width * height];
	m_nAlphaBrushHeight = height;
	m_nAlphaBrushWidth = width;

	for (int i = 0; i < width * height; ++i)
	{
		m_ucAlphaBrush[i] = ((int)data[i * 3] + (int)data[i * 3 + 1] + (int)data[i * 3 + 2]) / 3;
	}

	return 1;
}