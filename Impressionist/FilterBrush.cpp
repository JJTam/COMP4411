#include "FilterBrush.h"
#include "ImpressionistDoc.h"
#include "ImpressionistUI.h"

FilterBrush::FilterBrush(ImpressionistDoc* pDoc, char* name) :
ImpBrush(pDoc, name)
{
}

void FilterBrush::BrushBegin(const Point source, const Point target)
{
	BrushMove(source, target);
}

void FilterBrush::BrushMove(const Point source, const Point target)
{

}

void FilterBrush::BrushEnd(const Point source, const Point target)
{

}