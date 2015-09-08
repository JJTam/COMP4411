// 
// impressionistDoc.h
//
// header file for Doc 
//

#ifndef ImpressionistDoc_h
#define ImpressionistDoc_h

#include "impressionist.h"
#include "bitmap.h"
#include <list>

class ImpressionistUI;

class ImpressionistDoc 
{
public:
	ImpressionistDoc();

	void	setUI(ImpressionistUI* ui);		// Assign the UI to use

	int		loadImage(char *iname);			// called by the UI to load image
	int		saveImage(char *iname);			// called by the UI to save image


	int     clearCanvas();                  // called by the UI to clear the drawing canvas
	void	setBrushType(int type);			// called by the UI to set the brushType
	int		getSize();						// get the UI size
	int		getLineWidth();
	int		getAngle();
	double  getAlpha();
	void	setSize(int size);				// set the UI size
	void	setLineWidth(int size);
	void	setAngle(int angle);
	char*	getImageName();					// get the current image name
	
	int		autoDraw();

	void undo();
	void pushToUndo();

// Attributes
public:
	// Dimensions of original window.
	int				m_nWidth, 
					m_nHeight;
	// Dimensions of the paint window.
	int				m_nPaintWidth, 
					m_nPaintHeight;	
	// Bitmaps for original image and painting.
	unsigned char*	m_ucBitmap;
	unsigned char*	m_ucPainting;
	unsigned char*  m_ucPreservedPainting;

	// Undo
	std::list<unsigned char*> m_lUndoList;
	bool m_bHasPendingUndo;

	// The current active brush.
	ImpBrush*			m_pCurrentBrush;

	// current brush direction
	int m_nBrushDirection;

	// Size of the brush.
	int m_nSize;							
	int m_nLineWidth;
	int m_nAngle;
	ImpressionistUI*	m_pUI;

// Operations
public:
	// Get the color of the original picture at the specified coord
	GLubyte* GetOriginalPixel( int x, int y );   
	// Get the color of the original picture at the specified point	
	GLubyte* GetOriginalPixel( const Point p );  


private:
	char			m_imageName[256];
};

extern void MessageBox(char *message);

#endif
