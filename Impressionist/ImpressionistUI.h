//
// impressionistUI.h
//
// The header file for the UI part
//

#ifndef ImpressionistUI_h
#define ImpressionistUI_h

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_file_chooser.H>		// FLTK file chooser
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Multiline_Input.H>
#include "Impressionist.h"
#include "OriginalView.h"
#include "PaintView.h"

#include "ImpBrush.h"
#include "FilterBrush.h"

class ImpressionistUI {
public:
	ImpressionistUI();

	// The FLTK widgets
	Fl_Window*			m_mainWindow;
	Fl_Menu_Bar*		m_menubar;
								
	PaintView*			m_paintView;
	OriginalView*		m_origView;

	// for brush dialog
	Fl_Window*			m_brushDialog;
	Fl_Window*			m_paintlyDialog;
	Fl_Window*			m_colorSelectorDialog;
	Fl_Choice*			m_BrushTypeChoice;
	Fl_Choice*			m_StrokeDirectionChoice;

	Fl_Slider*			m_BrushSizeSlider;
	Fl_Slider*			m_LineWidthSlider;
	Fl_Slider*			m_AngleSlider;
	Fl_Slider*			m_AlphaSlider;
	Fl_Button*          m_ClearCanvasButton;

	Fl_Slider*			m_SpacingSlider;
	Fl_Button*			m_RandAttrButton;
	Fl_Button*			m_AutoDrawButton;

	Fl_Slider*			m_EdgeThresholdSlider;
	Fl_Button*			m_EdgeUpdateButton;
	Fl_Button*			m_AnotherGradientButton;
	Fl_Button*			m_EdgeClippingButton;
	Fl_Button*			m_FilterDialogButton;

	// for filter dialog
	Fl_Window*			m_FilterDialog;
	Fl_Choice*			m_FilterChoice;
	Fl_Slider*			m_FilterRadiusSlider;
	Fl_Slider*			m_FilterParamSlider;
	Fl_Multiline_Input* m_FilterKernelInput;
	Fl_Button*			m_FilterUpdateButton;
	Fl_Button*			m_FilterNormalizeKernelButton;

	// for background dialog
	Fl_Window*			m_backgroundDialog;
	Fl_Button*			m_BackgroundButton;
	Fl_Slider*			m_BackgroundAlphaSlider;

	// for paintly dialog
	Fl_Button*			m_pAutoDrawButton;
	Fl_Slider*			m_pAlphaSlider;
	Fl_Slider*			m_ThresholdSlider;
	Fl_Slider*			m_CurvatureFilterSlider;
	Fl_Slider*			m_BlurFactorSlider;
	Fl_Slider*			m_MinStrokeLengthSlider;
	Fl_Slider*			m_MaxStrokeLengthSlider;
	Fl_Slider*			m_GridSizeSlider;
	Fl_Slider*			m_PaintlyLevelSlider;
	Fl_Slider*			m_PaintlyR0Slider;
	Fl_Button*			m_PaintlyControlDirButton;
	Fl_Button*			m_PaintlyCheckColorButton;

	//for color selection
	Fl_Color_Chooser* m_colorSelector;

	// Member functions
	void				setDocument(ImpressionistDoc* doc);
	ImpressionistDoc*	getDocument();

	void				show();
	void				resize_windows(int w, int h);

	// Interface to get attribute

	int					getSize();
	void				setSize(int size);
	int					getLineWidth();
	void				setLineWidth(int size);
	int					getAngle();
	void				setAngle(int angle);
	double				getAlpha();
	void				setAlpha(double alpha);
	int					getSpacing();
	void				setSpacing(int);
	bool				getAttrRand();
	bool				getAnotherGradient();
	bool				getEdgeClipping();

	bool				getBackground();
	void				setBackground(bool);
	double				getBackgroundAlpha();
	void				setBackgroundAlpha(double);
	int					getEdgeThreshold();
	void				setEdgeThreshold(int);

	int					getThreshold();
	double				getCurvatureFilter();
	double				getBlurFactor();
	int					getMinStrokeLength();
	int					getMaxStrokeLength();
	double				getGridSize();
	int					getPaintlyLevel();
	int					getPaintlyR0();
	bool					getPaintlyControlDir();
	bool					getPaintlyCheckColor();

	int					getFilterRadius();
	double				getFilterSigma();
private:
	ImpressionistDoc*	m_pDoc;		// pointer to document to communicate with the document

	// All attributes here
	int		m_nSize;
	int     m_nLineWidth;
	int		m_nAngle;
	double  m_dAlpha;
	int		m_nSpacing;
	bool    m_bAttrRand;
	bool	m_bAnotherGradient;
	bool	m_bEdgeClipping;

	bool	m_bBackground;
	double	m_dBackgroundAlpha;

	int		m_nEdgeThreshold;

	int		m_nThreshold;
	double	m_dCurvatureFilter;
	double  m_dBlurFactor;
	int		m_nMinStrokeLength;
	int		m_nMaxStrokeLength;
	double	m_dGridSize;

	int m_nPaintlyLevel;
	int m_nPaintlyR0;
	bool m_bPaintlyControlDir;
	bool m_bPaintlyCheckColor;

	int m_nFilterRadius;
	double m_dFilterSigma;

	// Static class members
	static Fl_Menu_Item		menuitems[];
	static Fl_Menu_Item		brushTypeMenu[NUM_BRUSH_TYPE+1];
	static Fl_Menu_Item		DirectionTypeMenu[NUM_DIRECTION_TYPE + 1];
	static Fl_Menu_Item		filterTypeMenu[NUM_FILTER_TYPE + 1];
	static ImpressionistUI*	whoami(Fl_Menu_* o);

	// All callbacks here.  Callbacks are declared 
	// static
	static void	cb_load_image(Fl_Menu_* o, void* v);
	static void	cb_load_another_image(Fl_Menu_* o, void* v);
	static void cb_load_edge_image(Fl_Menu_* o, void* v);
	static void	cb_save_image(Fl_Menu_* o, void* v);
	static void	cb_brushes(Fl_Menu_* o, void* v);
	static void cb_paintly(Fl_Menu_* o, void* v);
	static void cb_colorSelector(Fl_Menu_* o, void* v);
	static void	cb_clear_canvas(Fl_Menu_* o, void* v);
	static void	cb_exit(Fl_Menu_* o, void* v);
	static void	cb_about(Fl_Menu_* o, void* v);
	static void	cb_brushChoice(Fl_Widget* o, void* v);
	static void	cb_StrokeDirectionChoice(Fl_Widget* o, void* v);
	static void	cb_clear_canvas_button(Fl_Widget* o, void* v);
	static void	cb_sizeSlides(Fl_Widget* o, void* v);
	static void	cb_LineWidthSlides(Fl_Widget* o, void* v);
	static void	cb_AngleSlides(Fl_Widget* o, void* v);
	static void cb_AlphaSlides(Fl_Widget* o, void* v);
	static void cb_undo(Fl_Menu_* o, void* v);
	static void cb_FilterDialogButton(Fl_Widget* o, void* v);

	static void cb_SpacingSlides(Fl_Widget* o, void* v);
	static void cb_randattr_button(Fl_Widget* o, void* v);
	static void cb_autodraw_button(Fl_Widget* o, void* v);
	static void cb_anothergradient_button(Fl_Widget* o, void* v);
	static void cb_edgeclipping_button(Fl_Widget* o, void* v);

	static void cb_background(Fl_Menu_* o, void* v);
	static void cb_background_button(Fl_Widget* o, void* v);
	static void cb_backgroundAlphaSlides(Fl_Widget* o, void* v);

	static void cb_EdgeThresholdSlides(Fl_Widget* o, void* v);
	static void cb_EdgeUpdateButton(Fl_Widget* o, void* v);

	static void cb_ChooseDisplay(Fl_Menu_* o, void* v);

	static void cb_ThresholdSlides(Fl_Widget* o, void* v);
	static void cb_CurvatureFilterSlides(Fl_Widget* o, void* v);
	static void cb_BlurFactorSlides(Fl_Widget* o, void* v);
	static void cb_MinStrokeLengthSlides(Fl_Widget* o, void* v);
	static void cb_MaxStrokeLengthSlides(Fl_Widget* o, void* v);
	static void cb_GridSizeSlides(Fl_Widget* o, void* v);
	static void cb_paintlydraw_button(Fl_Widget* o, void* v);
	static void cb_PaintlyLevelSlides(Fl_Widget* o, void* v);
	static void cb_PaintlyR0Slides(Fl_Widget* o, void* v);
	static void cb_PaintlyControlDirButton(Fl_Widget* o, void* v);
	static void cb_PaintlyCheckColorButton(Fl_Widget* o, void* v);

	static void cb_FilterTypeChoose(Fl_Widget* o, void* v);
	static void cb_FilterRadiusSlides(Fl_Widget* o, void* v);
	static void cb_FilterSigmaSlides(Fl_Widget* o, void* v);
	static void cb_FilterUpdateButton(Fl_Widget* o, void* v);
	static void cb_FilterNormalizeKernelButton(Fl_Widget* o, void* v);
};

#endif
