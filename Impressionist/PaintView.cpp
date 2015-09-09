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
#include <cmath>

#ifndef WIN32
#define min(a, b)	( ( (a)<(b) ) ? (a) : (b) )
#define max(a, b)	( ( (a)>(b) ) ? (a) : (b) )
#endif

static int		eventToDo;
static int		isAnEvent=0;
static Point    coord;
static Point    rightClickBegin;
static Point    rightClickEnd;
static Point    prevPoint;
static int      prevEvent;
static bool		isAuto = false;

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

	bool isDealingPending = m_pDoc->m_bHasPendingAutoFlush || m_pDoc->m_bHasPendingBgUpdate || m_pDoc->m_bHasPendingUndo;

	bool shallRestoreContent = ((m_pDoc->m_ucPainting && !isAnEvent) || (isAnEvent && isPointerOutOfRange)) && !isDealingPending;

	bool shallDrawContent = (m_pDoc->m_ucPainting && isAnEvent && !isPointerOutOfRange) || isDealingPending;

	bool shallRestoreDrawing = shallDrawContent;

	bool shallPushUndo = !isAuto
		&& (eventToDo == PV_LEFT_MOUSE_DOWN || (eventToDo == PV_LEFT_MOUSE_DRAG && prevEvent == PV_LEFT_MOUSE_UP))
		&& !isDealingPending;

	bool shallUpdatePointerDir = eventToDo == PV_LEFT_MOUSE_DOWN || eventToDo == PV_LEFT_MOUSE_DRAG &&
		(((m_pDoc->m_pCurrentBrush) == ImpBrush::c_pBrushes[BRUSH_LINES] || (m_pDoc->m_pCurrentBrush) == ImpBrush::c_pBrushes[BRUSH_SCATTERED_LINES])
		&& m_pDoc->m_nBrushDirection == BRUSH_DIRECTION);

	bool shallBrush = !isDealingPending;

	bool shallUpdatePreservedDrawing = (shallBrush && (eventToDo == PV_LEFT_MOUSE_DOWN || eventToDo == PV_LEFT_MOUSE_DRAG || eventToDo == PV_LEFT_MOUSE_UP)) || m_pDoc->m_bHasPendingAutoFlush;

	bool shallUpdateContentView = !isAuto;

	bool shallFlush = !isAuto;

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
			if (prevPoint.x != target.x || prevPoint.y != target.y)
			{
				int newAngle = (int)(atan((double)((target.y - prevPoint.y)) / (target.x - prevPoint.x)) / 3.14159 * 180);
				while (newAngle < 0)
					newAngle += 180;

				m_pDoc->m_pUI->setAngle(newAngle);
			}
			prevPoint.x = target.x;
			prevPoint.y = target.y;
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
				int bgAlpha = 255 * (1 - m_pDoc->m_pUI->getBackgroundAlpha());
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

	if (shallFlush)
	{
		glFlush();
	}

	#ifndef MESA
	glDrawBuffer(GL_BACK);
	#endif // !MESA

	// Reset flags
	isAuto = false;
	isAnEvent = 0;
	prevEvent = eventToDo;
	m_pDoc->m_bHasPendingUndo = false;
	m_pDoc->m_bHasPendingBgUpdate = false;
	m_pDoc->m_bHasPendingAutoFlush = false;
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
int PaintView::SimulateMouse(int x, int y, int click_type, bool setAuto)
{
	isAuto = setAuto;
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
