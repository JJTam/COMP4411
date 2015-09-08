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

#define LEFT_MOUSE_DOWN		1
#define LEFT_MOUSE_DRAG		2
#define LEFT_MOUSE_UP		3
#define RIGHT_MOUSE_DOWN	4
#define RIGHT_MOUSE_DRAG	5
#define RIGHT_MOUSE_UP		6


#ifndef WIN32
#define min(a, b)	( ( (a)<(b) ) ? (a) : (b) )
#define max(a, b)	( ( (a)>(b) ) ? (a) : (b) )
#endif

static int		eventToDo;
static int		isAnEvent=0;
static Point	coord;
static Point    rightClickBegin;
static Point    rightClickEnd;
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


void PaintView::draw()
{
	// TODO: TEMPORARY. Its value shall come from a user setting from the UI.
	static bool shallDrawBackground = false;

	#ifndef MESA
	// To avoid flicker on some machines.
	glDrawBuffer(GL_FRONT_AND_BACK);
	#endif // !MESA

	if(!valid())
	{

		glClearColor(0.7f, 0.7f, 0.7f, 0);

		// We're only using 2-D, so turn off depth 
		glDisable( GL_DEPTH_TEST );

		ortho();

		glClear( GL_COLOR_BUFFER_BIT );
	}

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

	m_pPaintBitstart = m_pDoc->m_ucPainting + 
		4 * ((m_pDoc->m_nPaintWidth * startrow) + scrollpos.x);
	m_pPreservedPaintBitstart = m_pDoc->m_ucPreservedPainting + 
		4 * ((m_pDoc->m_nPaintWidth * startrow) + scrollpos.x);

	m_nDrawWidth	= drawWidth;
	m_nDrawHeight	= drawHeight;

	m_nStartRow		= startrow;
	m_nEndRow		= startrow + drawHeight;
	m_nStartCol		= scrollpos.x;
	m_nEndCol		= m_nStartCol + drawWidth;

	if ( (m_pDoc->m_ucPainting && !isAnEvent) && (!m_pDoc->m_bHasPendingUndo)) 
	{
		RestoreContent();

	}

	if ( (m_pDoc->m_ucPainting && isAnEvent) || (m_pDoc->m_bHasPendingUndo)) 
	{
		bool updatePreservedDrawing = false;
		
		if (coord.x > drawWidth || coord.y > drawHeight)
		{
			RestoreContent();
			glFlush();
#ifndef MESA
			// To avoid flicker on some machines.
			glDrawBuffer(GL_BACK);
#endif // !MESA
			return;
		}

		// Clear it after processing.
		isAnEvent	= 0;	

		// Restore the drawing for... drawing
		glDisable(GL_BLEND);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glRasterPos2i(0, m_nWindowHeight - drawHeight);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, m_pDoc->m_nWidth);
		glDrawPixels(drawWidth, drawHeight, GL_RGBA, GL_UNSIGNED_BYTE, m_pPreservedPaintBitstart);

		Point source( coord.x + m_nStartCol, m_nEndRow - coord.y );
		Point target( coord.x, m_nWindowHeight - coord.y );
		
		if (eventToDo == LEFT_MOUSE_DOWN || (eventToDo == LEFT_MOUSE_DRAG && prevEvent == LEFT_MOUSE_UP) && (!m_pDoc->m_bHasPendingUndo))
		{
			//printf("Pushing to undo list... ");
			m_pDoc->pushToUndo();
			//printf("%d\n", m_pDoc->m_lUndoList.size());
			m_pPreservedPaintBitstart = m_pDoc->m_ucPreservedPainting +
				4 * ((m_pDoc->m_nPaintWidth * startrow) + scrollpos.x);
		}
		
		// This is the event handler
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//printf("Calling source is (%d, %d)\n", source.x, source.y);

		if (!m_pDoc->m_bHasPendingUndo)
		switch (eventToDo) 
		{
		case LEFT_MOUSE_DOWN:
			m_pDoc->m_pCurrentBrush->BrushBegin( source, target );
			updatePreservedDrawing = true;
			break;
		case LEFT_MOUSE_DRAG:
			m_pDoc->m_pCurrentBrush->BrushMove( source, target );
			updatePreservedDrawing = true;
			break;
		case LEFT_MOUSE_UP:
			m_pDoc->m_pCurrentBrush->BrushEnd( source, target );
			updatePreservedDrawing = true;
			//SaveCurrentContent();
			//RestoreContent();
			break;
		case RIGHT_MOUSE_DOWN:
			rightClickBegin.x = target.x;
			rightClickBegin.y = target.y;
			//printf("[1]Setting begin to (%d, %d)\n", target.x, target.y);
			break;
		case RIGHT_MOUSE_DRAG:
			glBegin(GL_LINES);
				glColor3ub(255, 0, 0);
				glVertex2d(rightClickBegin.x, rightClickBegin.y);
				glVertex2d(target.x, target.y);
			glEnd();
			break;
		case RIGHT_MOUSE_UP:
			rightClickEnd.x = target.x;
			rightClickEnd.y = target.y;

			if (rightClickBegin.x == rightClickEnd.x &&
				rightClickBegin.y == rightClickEnd.y)
			{
				// TODO: TEMPORARY. Its value shall come from a user setting from the UI.
				shallDrawBackground = !shallDrawBackground;
			}
			else
			{
				// update angle
				int newAngle = (int)(atan((double)((rightClickEnd.y - rightClickBegin.y)) / (rightClickEnd.x - rightClickBegin.x)) / 3.14159 * 180);
				while (newAngle < 0)
					newAngle += 360;

				m_pDoc->m_pUI->m_AngleSlider->value(newAngle);
				m_pDoc->m_pUI->setAngle(newAngle);
			}
			break;

		default:
			printf("Unknown event!!\n");		
			break;
		}

		prevEvent = eventToDo;

		// Preserve the drawing
		if (updatePreservedDrawing)
		{
			glReadPixels(0,
						m_nWindowHeight - m_nDrawHeight,
						m_nDrawWidth,
						m_nDrawHeight,
						GL_RGBA,
						GL_UNSIGNED_BYTE,
						m_pPreservedPaintBitstart);
		}

		m_pDoc->m_bHasPendingUndo = false;

		glReadPixels(0,
			m_nWindowHeight - m_nDrawHeight,
			m_nDrawWidth,
			m_nDrawHeight,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			m_pPaintBitstart);

		GLubyte* bits = (GLubyte*)m_pPaintBitstart;
		GLubyte* pbits = (GLubyte*)m_pPreservedPaintBitstart;

		// setup for visual drawing
		glRasterPos2i(0, m_nWindowHeight - drawHeight);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, m_pDoc->m_nWidth);
		
		if (shallDrawBackground)
		{
			for (int i = 0; i < drawWidth * drawHeight; ++i)
			{
				if (bits[i * 4 + 3] == 0)
					// TODO: TEMPORARY. The alpha value (180) shall come from a user setting from the UI.
					bits[i * 4 + 3] = 180;
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

	glFlush();

	#ifndef MESA
	// To avoid flicker on some machines.
	glDrawBuffer(GL_BACK);
	#endif // !MESA
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
			eventToDo=RIGHT_MOUSE_DOWN;
		else
			eventToDo=LEFT_MOUSE_DOWN;
		isAnEvent=1;
		redraw();
		break;
	case FL_DRAG:
		coord.x = Fl::event_x();
		coord.y = Fl::event_y();
		if (Fl::event_button()>1)
			eventToDo=RIGHT_MOUSE_DRAG;
		else
			eventToDo=LEFT_MOUSE_DRAG;
		isAnEvent=1;
		redraw();
		break;
	case FL_RELEASE:
		coord.x = Fl::event_x();
		coord.y = Fl::event_y();
		if (Fl::event_button()>1)
			eventToDo=RIGHT_MOUSE_UP;
		else
			eventToDo=LEFT_MOUSE_UP;
		isAnEvent=1;
		redraw();
		break;
	case FL_MOVE:
		coord.x = Fl::event_x();
		coord.y = Fl::event_y();
		// dirty trick to solve "drag-without-down" problem
		if (prevEvent != RIGHT_MOUSE_DRAG)
		{
			rightClickBegin.x = coord.x;
			rightClickBegin.y = m_nWindowHeight - coord.y;
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
