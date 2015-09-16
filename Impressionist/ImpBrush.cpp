//
// ImpBrush.cpp
//
// The implementation of virtual brush. All the other brushes inherit from it.
//

#include "impressionistDoc.h"
#include "impressionistUI.h"
#include "ImpBrush.h"

// Static class member initializations
int			ImpBrush::c_nBrushCount	= 0;
ImpBrush**	ImpBrush::c_pBrushes	= NULL;

ImpBrush::ImpBrush(ImpressionistDoc*	pDoc, 
				   char*				name) :
					m_pDoc(pDoc), 
					m_pBrushName(name)
{
}

//---------------------------------------------------
// Return m_pDoc, which connects the UI and brushes
//---------------------------------------------------
ImpressionistDoc* ImpBrush::GetDocument(void)
{
	return m_pDoc;
}

//---------------------------------------------------
// Return the name of the current brush
//---------------------------------------------------
char* ImpBrush::BrushName(void)
{
	return m_pBrushName;
}

//----------------------------------------------------
// Set the color to paint with to the color at source,
// which is the coord at the original window to sample 
// the color from
//----------------------------------------------------
void ImpBrush::SetColor (const Point source)
{
	SetColor(source.x, source.y);
}
void ImpBrush::SetColor(int x, int y)
{
	ImpressionistDoc* pDoc = GetDocument();

	GLubyte color[4];

	memcpy(color, pDoc->GetOriginalPixel(x,y), 3);
	color[0] *= pDoc->m_pUI->m_colorSelector->r();
	color[1] *= pDoc->m_pUI->m_colorSelector->g();
	color[2] *= pDoc->m_pUI->m_colorSelector->b();
	color[3] = m_pDoc->getAlpha() * 255;

	glColor4ubv(color);
}