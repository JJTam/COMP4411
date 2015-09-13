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
#include <vector>
#include <algorithm>
#include <cmath>

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
int ImpressionistDoc::loadImage(char *iname) 
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

	// reflect the fact of loading the new image
	m_nWidth		= width;
	m_nPaintWidth	= width;
	m_nHeight		= height;
	m_nPaintHeight	= height;

	// release old storage
	if ( m_ucBitmap ) delete [] m_ucBitmap;
	if ( m_ucPainting ) delete [] m_ucPainting;
	if (m_ucPreservedPainting) delete[] m_ucPreservedPainting;
	if (m_iGradient) delete[] m_iGradient;
	if (m_iGradientMagnitude) delete[] m_iGradientMagnitude;
	if (m_ucEdgeBitmap) delete[] m_ucEdgeBitmap;

	m_ucBitmap		= data;

	// allocate space for draw view
	m_ucPainting = new unsigned char[width*height * 4];
	memset(m_ucPainting, 0, width*height * 4);

	m_ucPreservedPainting = new unsigned char[width*height * 4];
	memset(m_ucPreservedPainting, 0, width*height * 4);

	m_pUI->m_mainWindow->resize(m_pUI->m_mainWindow->x(), 
								m_pUI->m_mainWindow->y(), 
								width*2, 
								height+25);

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

	// display it on origView
	m_pUI->m_origView->resizeWindow(width, height);	
	m_pUI->m_origView->refresh();

	// refresh paint view as well
	m_pUI->m_paintView->resizeWindow(width, height);	
	m_pUI->m_paintView->refresh();

	////// compute gradient

	m_iGradient = new int[width * height * 2];
	memset(m_iGradient, 0, width * height * 2 * sizeof(int));
	m_iGradientMagnitude = new int[width * height];
	memset(m_iGradientMagnitude, 0, width * height * sizeof(int));

	// short-cut to get index of (x, y)
#define IXY(x, y) (x + (y) * width)
	// temp black and white image, intensity = 0.299R + 0.587G + 0.144B
	unsigned char* bw = new unsigned char[width * height];
	for (int i = 0; i < width*height; ++i)
	{
		double val = 0.299 * m_ucBitmap[i * 3]
		             + 0.587 * m_ucBitmap[i * 3 + 1]
		             + 0.144 * m_ucBitmap[i * 3 + 2];
		
		// bw[i] = (val > 255.0) ? (unsigned char)val : 255; // What I wrote first. DO NOT code when you feel tired.
		bw[i] = (val > 255.0) ? 255 : (unsigned char)val;
	}

	for (int y = 1; y < height - 1; ++y)
		for (int x = 1; x < width - 1; ++x)
		{
			int idx = 2 * (x + y * width);
			int dx = 0
				+ (int)(bw[IXY(x + 1, y - 1)])
				+ (int)(bw[IXY(x + 1, y)]) * 2
				+ (int)(bw[IXY(x + 1, y + 1)])
				- (int)(bw[IXY(x - 1, y - 1)])
				- (int)(bw[IXY(x - 1, y)]) * 2
				- (int)(bw[IXY(x - 1, y + 1)]);

			int dy = 0
				+ (int)(bw[IXY(x - 1, y - 1)])
				+ (int)(bw[IXY(x, y - 1)]) * 2
				+ (int)(bw[IXY(x + 1, y - 1)])
				- (int)(bw[IXY(x - 1, y + 1)])
				- (int)(bw[IXY(x, y + 1)]) * 2
				- (int)(bw[IXY(x + 1, y + 1)]);

			m_iGradient[idx] = dx;
			m_iGradient[idx + 1] = dy;
			m_iGradientMagnitude[x + y * width] = sqrt(dx * dx + dy * dy);
		}

	// release memory
	delete bw;

	// update edge
	m_ucEdgeBitmap = new unsigned char[width * height * 3];
	updateEdge();

#undef IXY
	return 1;
}

int ImpressionistDoc::loadAnotherImage(char *iname)
{
	// try to open the image to read
	unsigned char*	data;
	int				width,
		height;

	if ((data = readBMP(iname, width, height)) == NULL)
	{
		fl_alert("Can't load bitmap file");
		return 0;
	}
	if (!m_ucBitmap)
	{
		fl_alert("Must load original image first!");
		return 0;
	}
	if (m_nWidth != width || m_nHeight != height)
	{
		fl_alert("The size must be same!");
		return 0;
	}


	// release old storage
	if (m_ucAnotherBitmap) delete[] m_ucAnotherBitmap;
	if (m_iAnotherGradient) delete[] m_iAnotherGradient;

	m_ucAnotherBitmap = data;

	////// compute gradient

	m_iAnotherGradient = new int[width * height * 2];
	memset(m_iAnotherGradient, 0, width * height * 2 * sizeof(int));


	// short-cut to get index of (x, y)
#define IXY(x, y) (x + (y) * width)
	// temp black and white image, intensity = 0.299R + 0.587G + 0.144B
	unsigned char* bw = new unsigned char[width * height];
	for (int i = 0; i < width*height; ++i)
	{
		double val = 0.299 * m_ucAnotherBitmap[i * 3]
			+ 0.587 * m_ucAnotherBitmap[i * 3 + 1]
			+ 0.144 * m_ucAnotherBitmap[i * 3 + 2];

		// bw[i] = (val > 255.0) ? (unsigned char)val : 255; // What I wrote first. DO NOT code when you feel tired.
		bw[i] = (val > 255.0) ? 255 : (unsigned char)val;
	}

	for (int y = 1; y < height - 1; ++y)
		for (int x = 1; x < width - 1; ++x)
		{
			int idx = 2 * (x + y * width);
			int dx = 0
				+ (int)(bw[IXY(x + 1, y - 1)])
				+ (int)(bw[IXY(x + 1, y)]) * 2
				+ (int)(bw[IXY(x + 1, y + 1)])
				- (int)(bw[IXY(x - 1, y - 1)])
				- (int)(bw[IXY(x - 1, y)]) * 2
				- (int)(bw[IXY(x - 1, y + 1)]);

			int dy = 0
				+ (int)(bw[IXY(x - 1, y - 1)])
				+ (int)(bw[IXY(x, y - 1)]) * 2
				+ (int)(bw[IXY(x + 1, y - 1)])
				- (int)(bw[IXY(x - 1, y + 1)])
				- (int)(bw[IXY(x, y + 1)]) * 2
				- (int)(bw[IXY(x + 1, y + 1)]);

			m_iAnotherGradient[idx] = dx;
			m_iAnotherGradient[idx + 1] = dy;
		}

	// release memory
	delete bw;


#undef IXY
	return 1;
}
//----------------------------------------------------------------
// Save the specified image
// This is called by the UI when the save image menu button is 
// pressed.
//----------------------------------------------------------------
int ImpressionistDoc::saveImage(char *iname) 
{

	// writeBMP(iname, m_nPaintWidth, m_nPaintHeight, m_ucPainting);
	unsigned char* rgb = new unsigned char[m_nPaintWidth*m_nPaintHeight * 3];
	for (int i = 0; i < m_nPaintWidth*m_nPaintHeight; ++i)
	{
		rgb[i * 3] = m_ucPainting[i * 4];
		rgb[i * 3 + 1] = m_ucPainting[i * 4 + 1];
		rgb[i * 3 + 2] = m_ucPainting[i * 4 + 2];
	}
	writeBMP(iname, m_nPaintWidth, m_nPaintHeight, rgb);
	delete[] rgb;
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
		delete[] m_ucPreservedPainting;
		m_ucPreservedPainting = new unsigned char[m_nPaintWidth*m_nPaintHeight * 4];
		memset(m_ucPreservedPainting, 0, m_nPaintWidth*m_nPaintHeight * 4);
	}

	if ( m_ucPainting ) 
	{
		delete [] m_ucPainting;

		// allocate space for draw view
		m_ucPainting	= new unsigned char [m_nPaintWidth*m_nPaintHeight*4];
		memset(m_ucPainting, 0, m_nPaintWidth*m_nPaintHeight*4);

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
		delete m_ucPreservedPainting;
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
	case DOC_DISPLAY_EDGE:
	case DOC_DISPLAY_ORIGINAL:
		m_nDisplayMode = mode;
		break;
	default:
		break;
	}
	m_pUI->m_origView->refresh();
}