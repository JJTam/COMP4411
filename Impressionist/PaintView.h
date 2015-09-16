//
// paintview.h
//
// The header file for painting view of the input images
//

#ifndef PAINTVIEW_H
#define PAINTVIEW_H

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>

#define PV_LEFT_MOUSE_DOWN		1
#define PV_LEFT_MOUSE_DRAG		2
#define PV_LEFT_MOUSE_UP		3
#define PV_RIGHT_MOUSE_DOWN    4
#define PV_RIGHT_MOUSE_DRAG    5
#define PV_RIGHT_MOUSE_UP		6
#define PV_NORMAL_AUTO	7
#define	PV_PAINTLY_AUTO 8

class ImpressionistDoc;

class PaintView : public Fl_Gl_Window
{
public:
	PaintView(int x, int y, int w, int h, const char* l);
	void draw();
	int handle(int event);

	int SimulateMouse(int x, int y, int click_type);

	void refresh();
	
	void resizeWindow(int width, int height);

	void SaveCurrentContent();

	void RestoreContent();

	ImpressionistDoc *m_pDoc;

private:
	GLvoid* m_pPaintBitstart;
	GLvoid* m_pPreservedPaintBitstart;
	int		m_nDrawWidth,
			m_nDrawHeight,
			m_nStartRow, 
			m_nEndRow,
			m_nStartCol, 
			m_nEndCol,
			m_nWindowWidth, 
			m_nWindowHeight;
};

#endif