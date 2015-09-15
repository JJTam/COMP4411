//
// paintview.cpp
//
// The code maintaining the painting view of the input images
//

#include "impressionist.h"
#include "impressionistDoc.h"
#include "impressionistUI.h"
#include "paintview.h"
#include "ImpBrush.h"
#include "ImageUtils.h"
#include "CurvedBrush.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

#ifndef WIN32
#define min(a, b)	( ( (a)<(b) ) ? (a) : (b) )
#define max(a, b)	( ( (a)>(b) ) ? (a) : (b) )
#endif

extern int irand(int);

static int		eventToDo = PV_LEFT_MOUSE_UP;
static int		isAnEvent=0;
static Point    coord;
static Point    rightClickBegin;
static Point    rightClickEnd;
static Point    prevPoint;
static int      prevEvent;

PaintView::PaintView(int			x, 
					 int			y, 
					 int			w, 
					 int			h, 
					 const char*	l)
						: Fl_Gl_Window(x,y,w,h,l)
{
	m_nWindowWidth	= w;
	m_nWindowHeight	= h;
	this->mode(FL_ALPHA);
}


int getAngle(int dx, int dy)
{
	int angle = (dy == 0) ? 90 : (int)(atan((double)dx / dy) / M_PI * 180);
	while (angle < 0)
		angle += 180;
	return angle;
}

void doAuto(ImpressionistDoc* pDoc, int width, int height, int startRow, int windowHeight, bool shallUpdatePointerDir)
{
	ImpressionistUI* pUI = pDoc->m_pUI;
	ImpBrush* pBrush = pDoc->m_pCurrentBrush;

	int Spacing = pUI->getSpacing();
	bool AttrRand = pUI->getAttrRand();

	int oSize = pUI->getSize();
	int oLineWidth = pUI->getLineWidth();
	int oAngle = pUI->getAngle();

	int x_counts = width / Spacing;
	int y_counts = height / Spacing;
	int total_points = x_counts * y_counts;

	std::vector<int> pointIndexes;
	for (int i = 0; i < total_points; ++i)
	{
		pointIndexes.push_back(i);
	}

	std::random_shuffle(pointIndexes.begin(), pointIndexes.end());

	Point coord;
	Point source;
	Point target;
	Point prevPoint;
	int currentAngle = oAngle;

	prevPoint.x = pointIndexes[0] % x_counts * Spacing + Spacing / 2;
	prevPoint.y = pointIndexes[0] / x_counts * Spacing + Spacing / 2;
	for (int i = 0; i < total_points; ++i)
	{
		coord.x = pointIndexes[i] % x_counts * Spacing + Spacing / 2;
		coord.y = pointIndexes[i] / x_counts * Spacing + Spacing / 2;
		source.x = coord.x;
		source.y = startRow + height - coord.y;
		target.x = coord.x;
		target.y = windowHeight - coord.y;

		if (shallUpdatePointerDir)
		{
			int dx = 0;
			int dy = 0;

			if (pDoc->m_nBrushDirection == BRUSH_DIRECTION)
			{
				dx = target.x - prevPoint.x;
				dy = target.y - prevPoint.y;
				prevPoint.x = target.x;
				prevPoint.y = target.y;
			}
			else
			{
				if (pDoc->m_pUI->getAnotherGradient() && pDoc->m_ucAnotherBitmap)
				{

					int idx = 2 * (target.x + pDoc->m_nPaintWidth * target.y);
					dx = pDoc->m_iAnotherGradient[idx];
					dy = pDoc->m_iAnotherGradient[idx + 1];
				}
				else
				{
					int idx = 2 * (target.x + pDoc->m_nPaintWidth * target.y);
					dx = pDoc->m_iGradient[idx];
					dy = pDoc->m_iGradient[idx + 1];
				}
			}

			if (dx != 0 || dy != 0)
			{
				currentAngle = getAngle(dx, dy);
				pDoc->m_pUI->setAngle(currentAngle);
			}
		}
		
		if (AttrRand)
		{
			pUI->setSize(oSize + irand(10) - 5);
			pUI->setLineWidth(oLineWidth + irand(10) - 5);
			pUI->setAngle(currentAngle + irand(10) - 5);
		}
		pBrush->BrushBegin(source, target);
		pBrush->BrushEnd(source, target);
	}

	pUI->setSize(oSize);
	pUI->setLineWidth(oLineWidth);
	pUI->setAngle(oAngle);
}

void doPaintlyAuto(ImpressionistDoc* pDoc,const int width,const int height)
{
	static double kernel[25];
	static double e = 2.718281828;
	static double prevSigma = -1;

	ImpressionistUI* pUI = pDoc->m_pUI;
	CurvedBrush* pBrush = (CurvedBrush*)ImpBrush::c_pBrushes[BRUSH_CURVED];
	
	// Apply Gaussian blur
	if (prevSigma != pUI->getBlurFactor() || pDoc->m_ucBitmapBlurred == NULL)
	{
		prevSigma = pUI->getBlurFactor();
		if (pDoc->m_ucBitmapBlurred != NULL)
		{
			delete[] pDoc->m_ucBitmapBlurred;
		}

		if (prevSigma == 0.0)
		{
			pDoc->m_ucBitmapBlurred = new unsigned char[pDoc->m_nWidth * pDoc->m_nHeight * 3];
			memcpy(pDoc->m_ucBitmapBlurred, pDoc->m_ucBitmap, pDoc->m_nWidth * pDoc->m_nHeight * 3);
		}
		else
		{
			for (int y = 0; y < 5; ++y)
				for (int x = 0; x < 5; ++x)
				{
					int dx = x - 2;
					int dy = y - 2;
					double t = pow(e, -((dx * dx + dy * dy) / (2 * prevSigma)));
					kernel[x + y * 5] = t / (sqrt(2 * M_PI * prevSigma));
				}
			pDoc->m_ucBitmapBlurred = ImageUtils::getFilteredImage(kernel, 5, 5, pDoc->m_ucBitmap, pDoc->m_nWidth, pDoc->m_nHeight, 0, 0, 0, 0, 3, IMAGE_UTIL_WRAP_BOUNDARY);
			if (pDoc->getDisplayMode() == DOC_DISPLAY_BLURRED)
			{
				pUI->m_origView->refresh();
			}
		}
	}
	
	// Create a pointwise difference image
	double* differenceMap = new double[width*height];
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			GLubyte originColor[3];
			GLubyte canvasColor[3];
			memcpy(originColor, (void*)(pDoc->m_ucBitmapBlurred + 3 * (y * width + x)), 3);
			memcpy(canvasColor, (GLubyte*)(pDoc->m_ucPainting + 4 * (y*pDoc->m_nWidth + x)), 3);

			double currentDiff = sqrt(pow(originColor[0] - canvasColor[0], 2) + pow(originColor[1] - canvasColor[1], 2) + pow(originColor[2] - canvasColor[2], 2));
			differenceMap[x + y*width] = currentDiff;
		}

	}
	std::vector<int> pointIndexes;

	// Get parameters
	int threshold = pDoc->m_pUI->getThreshold();
	double gridSize = pDoc->m_pUI->getGridSize();
	int grid = gridSize * pUI->getSize();

	for (int x = 0 + grid/2; x < width - grid/2; x += grid)
	{
		for (int y = 0 + grid/2; y < height - grid/2; y += grid)
		{
			// Sum the error near (x,y)
			double areaError = 0;
			for (int i = x - grid / 2; i < x + grid / 2 + 1; i++)
			{
				for (int j = y - grid / 2; j < y + grid / 2 + 1; j++)
				{
					areaError += differenceMap[i + j*width];
				}
			}
			areaError /= pow(2 * (grid / 2) + 1, 2);

			if (areaError > threshold)
			{
				// Find the largest error point

				int maxX = x - grid / 2;
				int maxY = y - grid / 2;
				double maxError = differenceMap[maxX + maxY*width];

				for (int i = x - grid / 2; i < x + grid / 2 + 1; i++)
				{
					for (int j = y - grid / 2; j < y + grid / 2 + 1; j++)
					{
						if (maxError < differenceMap[i + j*width]);
						{
							maxX = i;
							maxY = j;
							maxError = differenceMap[i + j*width];
						}
					}
				}
				pointIndexes.push_back(maxX + maxY*width);
			}
		}
	}
	delete differenceMap;

	// DO the painting
	std::random_shuffle(pointIndexes.begin(), pointIndexes.end());
	for (int i = 0; i < pointIndexes.size(); ++i)
	{
		Point currentPoint;
		currentPoint.x = pointIndexes[i] % width;
		currentPoint.y = pointIndexes[i] / width;
		pBrush->BrushBegin(currentPoint, currentPoint);
		pBrush->BrushEnd(currentPoint, currentPoint);

	}
}
void PaintView::draw()
{
	bool shallDrawBackground = m_pDoc->m_pUI->getBackground();
	
	#ifndef MESA
	glDrawBuffer(GL_FRONT_AND_BACK);
	#endif // !MESA

	if(!valid())
	{
		glClearColor(0.7f, 0.7f, 0.7f, 0);
		// We're only using 2-D, so turn off depth 
		glDisable(GL_DEPTH_TEST);
		ortho();
		glClear(GL_COLOR_BUFFER_BIT);
	}

	// Local parameters
	Point scrollpos;// = GetScrollPosition();
	scrollpos.x = 0;
	scrollpos.y	= 0;

	m_nWindowWidth	= w();
	m_nWindowHeight	= h();

	int drawWidth, drawHeight;
	drawWidth = min( m_nWindowWidth, m_pDoc->m_nPaintWidth );
	drawHeight = min( m_nWindowHeight, m_pDoc->m_nPaintHeight );
	
	int startrow = m_pDoc->m_nPaintHeight - (scrollpos.y + drawHeight);
	if ( startrow < 0 ) startrow = 0;

	// Setup class members
	m_pPaintBitstart = m_pDoc->m_ucPainting + 
		4 * ((m_pDoc->m_nPaintWidth * startrow) + scrollpos.x);
	m_pPreservedPaintBitstart = m_pDoc->m_ucPreservedPainting + 
		4 * ((m_pDoc->m_nPaintWidth * startrow) + scrollpos.x);

	m_nDrawWidth   	= drawWidth;
	m_nDrawHeight	= drawHeight;
	m_nStartRow		= startrow;
	m_nEndRow		= startrow + drawHeight;
	m_nStartCol		= scrollpos.x;
	m_nEndCol		= m_nStartCol + drawWidth;
	
	// Deciding what to do
	bool isPointerOutOfRange = coord.x > drawWidth || coord.y > drawHeight;

	bool isDealingPending = m_pDoc->m_bHasPendingBgUpdate || m_pDoc->m_bHasPendingUndo;

	bool shallRestoreContent = ((m_pDoc->m_ucPainting && !isAnEvent) || (isAnEvent && isPointerOutOfRange)) && !isDealingPending;

	bool shallDrawContent = (m_pDoc->m_ucPainting && isAnEvent && !isPointerOutOfRange) || isDealingPending;

	bool shallRestoreDrawing = shallDrawContent;

	bool shallPushUndo = (eventToDo == PV_LEFT_MOUSE_DOWN || (eventToDo == PV_LEFT_MOUSE_DRAG && prevEvent == PV_LEFT_MOUSE_UP) || eventToDo == PV_NORMAL_AUTO || eventToDo == PV_PAINTLY_AUTO) && !isDealingPending;

	bool isUsingPointerDir = (m_pDoc->m_pCurrentBrush == ImpBrush::c_pBrushes[BRUSH_LINES] || m_pDoc->m_pCurrentBrush == ImpBrush::c_pBrushes[BRUSH_SCATTERED_LINES])
		&& (m_pDoc->m_nBrushDirection == BRUSH_DIRECTION || m_pDoc->m_nBrushDirection == GRADIENT);

	bool shallUpdatePointerDir = (eventToDo == PV_LEFT_MOUSE_DOWN || eventToDo == PV_LEFT_MOUSE_DRAG) && isUsingPointerDir;

	bool shallBrush = !isDealingPending;

	bool shallUpdatePreservedDrawing = (shallBrush && (eventToDo == PV_LEFT_MOUSE_DOWN || eventToDo == PV_LEFT_MOUSE_DRAG || eventToDo == PV_LEFT_MOUSE_UP || eventToDo == PV_NORMAL_AUTO || eventToDo == PV_PAINTLY_AUTO));

	bool shallUpdateContentView = true;

	if (shallRestoreContent) 
	{
		RestoreContent();
	}

	if (shallDrawContent)
	{
		// Local parameters
		Point source(coord.x + m_nStartCol, m_nEndRow - coord.y);
		Point target(coord.x, m_nWindowHeight - coord.y);

		// Do things
		if (shallRestoreDrawing)
		{
			glDisable(GL_BLEND);
			glClearColor(0.7f, 0.7f, 0.7f, 0);
			glClear(GL_COLOR_BUFFER_BIT);

			glRasterPos2i(0, m_nWindowHeight - drawHeight);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, m_pDoc->m_nWidth);
			glDrawPixels(drawWidth, drawHeight, GL_RGBA, GL_UNSIGNED_BYTE, m_pPreservedPaintBitstart);
		}
		
		if (shallPushUndo)
		{
			m_pDoc->pushToUndo();
			m_pPreservedPaintBitstart = m_pDoc->m_ucPreservedPainting +
				4 * ((m_pDoc->m_nPaintWidth * startrow) + scrollpos.x);
		}

		if (shallUpdatePointerDir)
		{
			int dx = 0;
			int dy = 0;

			if (m_pDoc->m_nBrushDirection == BRUSH_DIRECTION)
			{
				dx = target.x - prevPoint.x;
				dy = target.y - prevPoint.y;
				prevPoint.x = target.x;
				prevPoint.y = target.y;
			}
			else
			{
				if (m_pDoc->m_pUI->getAnotherGradient() && m_pDoc->m_ucAnotherBitmap)
				{

					int idx = 2 * (target.x + m_pDoc->m_nPaintWidth * target.y);
					dx = m_pDoc->m_iAnotherGradient[idx];
					dy = m_pDoc->m_iAnotherGradient[idx + 1];
				}
				else
				{
					int idx = 2 * (target.x + m_pDoc->m_nPaintWidth * target.y);
					dx = m_pDoc->m_iGradient[idx];
					dy = m_pDoc->m_iGradient[idx + 1];
				}
			}

			if (dx != 0 || dy != 0)
				m_pDoc->m_pUI->setAngle(getAngle(dx, dy));
		}

		if (shallBrush)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			switch (eventToDo) 
			{
			case PV_LEFT_MOUSE_DOWN:
				m_pDoc->m_pCurrentBrush->BrushBegin( source, target );
				break;
			case PV_LEFT_MOUSE_DRAG:
				m_pDoc->m_pCurrentBrush->BrushMove( source, target );
				break;
			case PV_LEFT_MOUSE_UP:
				m_pDoc->m_pCurrentBrush->BrushEnd( source, target );
				break;
			case PV_NORMAL_AUTO:
				doAuto(m_pDoc, drawWidth, drawHeight, startrow, m_nWindowHeight, isUsingPointerDir);
				break;
			case PV_PAINTLY_AUTO:
				doPaintlyAuto(m_pDoc, drawWidth, drawHeight);
				break;
			case PV_RIGHT_MOUSE_DOWN:
				rightClickBegin.x = target.x;
				rightClickBegin.y = target.y;
				break;
			case PV_RIGHT_MOUSE_DRAG:
				if (m_pDoc->m_nBrushDirection == SLIDER_AND_RIGHT_MOUSE)
				{
					glBegin(GL_LINES);
					glColor3ub(255, 0, 0);
					glVertex2d(rightClickBegin.x, rightClickBegin.y);
					glVertex2d(target.x, target.y);
					glEnd();
				}
				break;
			case PV_RIGHT_MOUSE_UP:
				rightClickEnd.x = target.x;
				rightClickEnd.y = target.y;

				if (rightClickBegin.x == rightClickEnd.x &&
					rightClickBegin.y == rightClickEnd.y)
				{
					// allow user to toggle background by right-clicking
					shallDrawBackground = !shallDrawBackground;
					m_pDoc->m_pUI->setBackground(shallDrawBackground);
				}
				else
				{
					if (m_pDoc->m_nBrushDirection == SLIDER_AND_RIGHT_MOUSE)
					{
						// update angle
						int newAngle = (int)(atan((double)((rightClickEnd.y - rightClickBegin.y)) / (rightClickEnd.x - rightClickBegin.x)) / 3.14159 * 180);
						while (newAngle < 0)
							newAngle += 180;

						m_pDoc->m_pUI->setAngle(newAngle);
					}
				}
				break;

			default:
				printf("Unknown event!!\n");		
				break;
			}
		}

		if (shallUpdatePreservedDrawing)
		{
			glReadPixels(0,
						m_nWindowHeight - m_nDrawHeight,
						m_nDrawWidth,
						m_nDrawHeight,
						GL_RGBA,
						GL_UNSIGNED_BYTE,
						m_pPreservedPaintBitstart);
		}

		if (shallUpdateContentView)
		{
			glReadPixels(0,
				m_nWindowHeight - m_nDrawHeight,
				m_nDrawWidth,
				m_nDrawHeight,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				m_pPaintBitstart);

			GLubyte* bits = (GLubyte*)m_pPaintBitstart;
			GLubyte* pbits = (GLubyte*)m_pPreservedPaintBitstart;

			glRasterPos2i(0, m_nWindowHeight - drawHeight);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, m_pDoc->m_nWidth);
		
			if (shallDrawBackground)
			{
				int bgAlpha = (int)(255 * (1 - m_pDoc->m_pUI->getBackgroundAlpha()));
				for (int i = 0; i < drawWidth * drawHeight; ++i)
				{
					if (bits[i * 4 + 3] == 0)
						bits[i * 4 + 3] = bgAlpha;
				}
			
				glRasterPos2i(0, m_nWindowHeight - drawHeight);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glPixelStorei(GL_UNPACK_ROW_LENGTH, m_pDoc->m_nWidth);
			
				glDrawPixels(drawWidth, drawHeight, GL_RGB, GL_UNSIGNED_BYTE, m_pDoc->m_ucBitmap);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDrawPixels(drawWidth, drawHeight, GL_RGBA, GL_UNSIGNED_BYTE, m_pPaintBitstart);
			}
			else
			{
				glDisable(GL_BLEND);
				glDrawPixels(drawWidth, drawHeight, GL_RGBA, GL_UNSIGNED_BYTE, m_pPaintBitstart);
			}

			SaveCurrentContent();
		}
	}

	glFlush();

	#ifndef MESA
	glDrawBuffer(GL_BACK);
	#endif // !MESA

	// Reset flags
	isAnEvent = 0;
	prevEvent = eventToDo;
	m_pDoc->m_bHasPendingUndo = false;
	m_pDoc->m_bHasPendingBgUpdate = false;

	// Edge clipping
	if (eventToDo == PV_NORMAL_AUTO || eventToDo == PV_PAINTLY_AUTO)
	{
		this->refresh();
	}
}


int PaintView::handle(int event)
{
	switch(event)
	{
	case FL_ENTER:
	    redraw();
		break;
	case FL_PUSH:
		coord.x = Fl::event_x();
		coord.y = Fl::event_y();
		if (Fl::event_button()>1)
			eventToDo=PV_RIGHT_MOUSE_DOWN;
		else
			eventToDo=PV_LEFT_MOUSE_DOWN;
		isAnEvent=1;
		redraw();
		break;
	case FL_DRAG:
		coord.x = Fl::event_x();
		coord.y = Fl::event_y();
		if (Fl::event_button()>1)
			eventToDo=PV_RIGHT_MOUSE_DRAG;
		else
			eventToDo=PV_LEFT_MOUSE_DRAG;
		isAnEvent=1;
		redraw();
		break;
	case FL_RELEASE:
		coord.x = Fl::event_x();
		coord.y = Fl::event_y();
		if (Fl::event_button()>1)
			eventToDo=PV_RIGHT_MOUSE_UP;
		else
			eventToDo=PV_LEFT_MOUSE_UP;
		isAnEvent=1;
		redraw();
		break;
	case FL_MOVE:
		coord.x = Fl::event_x();
		coord.y = Fl::event_y();
		// dirty trick to solve "drag-without-down" problem
		if (prevEvent != PV_RIGHT_MOUSE_DRAG && prevEvent != PV_LEFT_MOUSE_DRAG)
		{
			rightClickBegin.x = coord.x;
			rightClickBegin.y = m_nWindowHeight - coord.y;
			prevPoint.x = coord.x;
			prevPoint.y = m_nWindowHeight - coord.y;
		}
		break;
	default:
		return 0;
		break;

	}

	// move the mouse indicator in OriginalView
	this->m_pDoc->m_pUI->m_origView->move_mouse_ident_box(coord.x, coord.y);

	return 1;
}

//MouseSimulator for auto draw
int PaintView::SimulateMouse(int x, int y, int click_type)
{
	coord.x = x;
	coord.y = y;
	eventToDo = click_type;
	isAnEvent = 1;
	this->flush();
	return 1;
}

void PaintView::refresh()
{
	redraw();
}

void PaintView::resizeWindow(int width, int height)
{
	resize(x(), y(), width, height);
}

void PaintView::SaveCurrentContent()
{
	// Tell openGL to read from the front buffer when capturing
	// out paint strokes
	glReadBuffer(GL_FRONT);

	glPixelStorei( GL_PACK_ALIGNMENT, 1 );
	glPixelStorei( GL_PACK_ROW_LENGTH, m_pDoc->m_nPaintWidth );
	
	glReadPixels( 0, 
				  m_nWindowHeight - m_nDrawHeight, 
				  m_nDrawWidth, 
				  m_nDrawHeight, 
				  GL_RGBA, 
				  GL_UNSIGNED_BYTE, 
				  m_pPaintBitstart );

	for (int i = 0; i < m_nDrawHeight * m_nDrawWidth; ++i)
	{
		((GLubyte*)m_pPaintBitstart)[i * 4 + 3] = 255;
	}
}


void PaintView::RestoreContent()
{
	glDrawBuffer(GL_BACK);

	glClear( GL_COLOR_BUFFER_BIT );

	glRasterPos2i( 0, m_nWindowHeight - m_nDrawHeight );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glPixelStorei( GL_UNPACK_ROW_LENGTH, m_pDoc->m_nPaintWidth );
	glDrawPixels( m_nDrawWidth, 
				  m_nDrawHeight, 
				  GL_RGBA, 
				  GL_UNSIGNED_BYTE, 
				  m_pPaintBitstart);

//	glDrawBuffer(GL_FRONT);
}
