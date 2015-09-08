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

#define DESTROY(p)	{  if ((p)!=NULL) {delete [] p; p=NULL; } }
#define ABS(x) (x >= 0 ? x : -x)
extern int irand(int);
ImpressionistDoc::ImpressionistDoc() 
{
	// Set NULL image name as init. 
	m_imageName[0]	='\0';	

	m_nWidth		= -1;
	m_ucBitmap		= NULL;
	m_ucPainting	= NULL;
	m_ucPreservedPainting = NULL;

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
	//printf("Width = %d, window->w() = %d, Height+25 = %d, window->h() = %d\n", width, m_pUI->m_mainWindow->w(), height+25, m_pUI->m_mainWindow->h());
	
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
	//printf("Width = %d, window->w() = %d, Height+25 = %d, window->h() = %d\n", width, m_pUI->m_mainWindow->w(), height + 25, m_pUI->m_mainWindow->h());

	// display it on origView
	m_pUI->m_origView->resizeWindow(width, height);	
	m_pUI->m_origView->refresh();

	// refresh paint view as well
	m_pUI->m_paintView->resizeWindow(width, height);	
	m_pUI->m_paintView->refresh();


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

	int Spacing = m_pUI->getSpacing();
	bool AttrRand = m_pUI->getAttrRand();
	
	int oSize = m_pUI->getSize();
	int oLineWidth = m_pUI->getLineWidth();
	int oAngle = m_pUI->getAngle();

	int x_counts = m_nWidth / Spacing;
	int y_counts = m_nHeight / Spacing;

	std::vector<int> temp;
	for (int i = 0; i < x_counts*y_counts; ++i)
	{
		temp.push_back(i);
	}
	
	// change this to brush type later
	std::random_shuffle(temp.begin(), temp.end());
	
	for (int i = 0; i < x_counts*y_counts; ++i)
	{
		int x = temp[i] % x_counts * Spacing + Spacing / 2;
		int y = temp[i] / x_counts * Spacing + Spacing / 2;
		if (AttrRand)
		{
			m_pUI->setSize(oSize + irand(10) - 5);
			m_pUI->setLineWidth(oLineWidth + irand(10) - 5);
			m_pUI->setAngle(oAngle + irand(10) - 5);
		}
		m_pUI->m_paintView->SimulateMouse(x, y, 1, false);
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