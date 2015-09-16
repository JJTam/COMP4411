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

GLubyte preserve[363];

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
		//printf("Window W/H is %d, %d\n", m_nWindowWidth, m_nWindowHeight);
		int drawWidth, drawHeight;
		GLvoid* bitstart;

		// we are not using a scrollable window, so ignore it
		Point scrollpos;	// = GetScrollPosition();
		scrollpos.x=scrollpos.y=0;

		drawWidth	= min( m_nWindowWidth, m_pDoc->m_nWidth );
		drawHeight	= min( m_nWindowHeight, m_pDoc->m_nHeight );
		//printf("Draw height of OriginalView is %d\n", drawHeight);
		int	startrow	= m_pDoc->m_nHeight - (scrollpos.y + drawHeight);
		if ( startrow < 0 ) 
			startrow = 0;

		int bitstartOffset = 3 * ((m_pDoc->m_nWidth * startrow) + scrollpos.x);
		switch (m_pDoc->getDisplayMode())
		{
		case DOC_DISPLAY_ORIGINAL:
			bitstart = m_pDoc->m_ucBitmap + bitstartOffset;
			break;
		case DOC_DISPLAY_EDGE:
			bitstart = m_pDoc->m_ucEdgeBitmap + bitstartOffset;
			break;
		case DOC_DISPLAY_ANOTHER:
			bitstart = m_pDoc->m_ucAnotherBitmap + bitstartOffset;
			break;
		case DOC_DISPLAY_BLURRED:
			bitstart = m_pDoc->m_ucBitmapBlurred + bitstartOffset;
			break;
		default:
			bitstart = m_pDoc->m_ucBitmap + bitstartOffset;
			break;
		}

		GLubyte* bits = (GLubyte*)bitstart;

		int imageWidth = m_pDoc->m_nWidth;
		int imageHeight = m_pDoc->m_nHeight;
		int mouse_indicator_draw_y = drawHeight - mouse_indicator_y;
		//printf("mouse_indicator_draw_y = %d - %d = %d\n", mouse_indicator_draw_y, drawHeight, mouse_indicator_y);
		int mouse_indicator_center = (mouse_indicator_draw_y * drawWidth + mouse_indicator_x) * 3;
		if (mouse_indicator_center > 0 && mouse_indicator_center < drawHeight * drawWidth * 3)
		{
			//printf("GetOriginalPixel(%d, %d)\n", mouse_indicator_x, mouse_indicator_draw_y);
			GLubyte* centerByte = m_pDoc->GetOriginalPixel(mouse_indicator_x, mouse_indicator_draw_y + startrow);
			GLubyte R = centerByte[0];
			GLubyte G = centerByte[1];
			GLubyte B = centerByte[2];
			// draw the mouse indicator
			for (int i = 0; i < 11; ++i)
			{
				int x = mouse_indicator_x - 5 + i;
				if (x < 0 || x >= imageWidth)
					continue;

				for (int j = 0; j < 11; ++j)
				{
					int y = mouse_indicator_draw_y - 5 + j;
					if (y < 0 || y >= drawHeight)
						continue;

					preserve[3 * (j * 11 + i)] = bits[3 * (y * imageWidth + x)];
					preserve[3 * (j * 11 + i) + 1] = bits[3 * (y * imageWidth + x) + 1];
					preserve[3 * (j * 11 + i) + 2] = bits[3 * (y * imageWidth + x) + 2];

					if (i == 0 || i == 10 || j == 0 || j == 10)
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
			for (int i = 0; i < 11; ++i)
			{
				int x = mouse_indicator_x - 5 + i;
				if (x < 0 || x >= imageWidth)
					continue;

				for (int j = 0; j < 11; ++j)
				{
					int y = mouse_indicator_draw_y - 5 + j;
					if (y < 0 || y >= drawHeight)
						continue;

					bits[3 * (y * imageWidth + x)] = preserve[3 * (j * 11 + i)];
					bits[3 * (y * imageWidth + x) + 1] = preserve[3 * (j * 11 + i) + 1];
					bits[3 * (y * imageWidth + x) + 2] = preserve[3 * (j * 11 + i) + 2];
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