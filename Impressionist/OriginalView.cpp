//
// originalview.cpp
//
// The code maintaining the original view of the input images
//

#include "impressionist.h"
#include "impressionistDoc.h"
#include "originalview.h"

#ifndef WIN32
#define min(a, b)	( ( (a)<(b) ) ? (a) : (b) )
#endif

OriginalView::OriginalView(int			x, 
						   int			y, 
						   int			w, 
						   int			h, 
						   const char*	l)
							: Fl_Gl_Window(x,y,w,h,l)
{
	m_nWindowWidth	= w;
	m_nWindowHeight	= h;
}

GLubyte preserve[300];

void OriginalView::draw()
{
	if(!valid())
	{
		glClearColor(0.7f, 0.7f, 0.7f, 1.0);

		// We're only using 2-D, so turn off depth 
		glDisable( GL_DEPTH_TEST );

		// Tell openGL to read from the front buffer when capturing
		// out paint strokes 
		glReadBuffer( GL_FRONT );
		ortho();

	}

	glClear( GL_COLOR_BUFFER_BIT );

	if ( m_pDoc->m_ucBitmap ) 
	{
		// note that both OpenGL pixel storage and the Windows BMP format
		// store pixels left-to-right, BOTTOM-to-TOP!!  thus all the fiddling
		// around with startrow.

		m_nWindowWidth=w();
		m_nWindowHeight=h();

		int drawWidth, drawHeight;
		GLvoid* bitstart;

		// we are not using a scrollable window, so ignore it
		Point scrollpos;	// = GetScrollPosition();
		scrollpos.x=scrollpos.y=0;

		drawWidth	= min( m_nWindowWidth, m_pDoc->m_nWidth );
		drawHeight	= min( m_nWindowHeight, m_pDoc->m_nHeight );
		
		int	startrow	= m_pDoc->m_nHeight - (scrollpos.y + drawHeight);
		if ( startrow < 0 ) 
			startrow = 0;


		bitstart = m_pDoc->m_ucBitmap + 3 * ((m_pDoc->m_nWidth * startrow) + scrollpos.x);

		GLubyte* bits = (GLubyte*)bitstart;

		int imageWidth = m_pDoc->m_nWidth;
		int mouse_indicator_draw_y = drawHeight - mouse_indicator_y;
		int mouse_indicator_center = (mouse_indicator_draw_y * drawWidth + mouse_indicator_x) * 3;
		if (mouse_indicator_center > 0 && mouse_indicator_center < drawHeight * drawWidth * 3)
		{
			GLubyte R = bits[mouse_indicator_center];
			GLubyte G = bits[mouse_indicator_center + 1];
			GLubyte B = bits[mouse_indicator_center + 2];
			// draw the mouse indicator
			for (int i = 0; i < 10; ++i)
			{
				int x = mouse_indicator_x - 5 + i;
				if (x < 0 || x >= imageWidth)
					continue;

				for (int j = 0; j < 10; ++j)
				{
					int y = mouse_indicator_draw_y - 5 + j;
					if (y < 0 || y >= drawHeight)
						continue;

					preserve[3 * (j * 10 + i)] = bits[3 * (y * imageWidth + x)];
					preserve[3 * (j * 10 + i) + 1] = bits[3 * (y * imageWidth + x) + 1];
					preserve[3 * (j * 10 + i) + 2] = bits[3 * (y * imageWidth + x) + 2];

					if (i == 0 || i == 9 || j == 0 || j == 9)
					{
						bits[3 * (y * imageWidth + x)] = 255;
						bits[3 * (y * imageWidth + x) + 1] = 0;
						bits[3 * (y * imageWidth + x) + 2] = 0;
					}
					else
					{
					
						bits[3 * (y * imageWidth + x)] = R;
						bits[3 * (y * imageWidth + x) + 1] = G;
						bits[3 * (y * imageWidth + x) + 2] = B;
					}
				}
			}
		}


		// just copy image to GLwindow conceptually
		glRasterPos2i( 0, m_nWindowHeight - drawHeight );
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glPixelStorei( GL_UNPACK_ROW_LENGTH, m_pDoc->m_nWidth );
		glDrawBuffer( GL_BACK );
		glDrawPixels( drawWidth, drawHeight, GL_RGB, GL_UNSIGNED_BYTE, bitstart );

		if (mouse_indicator_center > 0 && mouse_indicator_center < drawHeight * drawWidth * 3)
		{
			for (int i = 0; i < 10; ++i)
			{
				int x = mouse_indicator_x - 5 + i;
				if (x < 0 || x >= imageWidth)
					continue;

				for (int j = 0; j < 10; ++j)
				{
					int y = mouse_indicator_draw_y - 5 + j;
					if (y < 0 || y >= drawHeight)
						continue;

					bits[3 * (y * imageWidth + x)] = preserve[3 * (j * 10 + i)];
					bits[3 * (y * imageWidth + x) + 1] = preserve[3 * (j * 10 + i) + 1];
					bits[3 * (y * imageWidth + x) + 2] = preserve[3 * (j * 10 + i) + 2];
				}
			}
		}
	}

	glFlush();
}

void OriginalView::refresh()
{
	redraw();
}

void OriginalView::resizeWindow(int	width, 
								int	height)
{
	resize(x(), y(), width, height);
}

void OriginalView::move_mouse_ident_box(int x, int y)
{
	mouse_indicator_x = x;
	mouse_indicator_y = y;
	redraw();
}