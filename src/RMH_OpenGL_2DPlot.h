#pragma once

/*
 *  RMH_OpenGL_2DPlot.h
 *
 *  Author: Rune Mark Hansen
 *  Date: March 2023
 *
 */

// Included libraries
#include <windows.h>
#include <GL/GLU.h>
#include <GL/GL.h>
#include <iostream>
#include "RMH_Winforms_Library.h"
#include "RMH_MathConversions_Library.h"
#include "GlobalObjectsAndVariables.h"

// Associated namespaces
using namespace System;
using namespace System::Windows::Forms;
using namespace std;

// General 2D plot configuration macros
#define _2DPlotMaxNumberOfDataSets       10

// 2D plot X axis configuration macro
#define _2DPlotXAxesLength               1000

// 2D plot Y axis configuration macro
#define _2DPlotYAxesMaximumRangeResetValue   -10000.0
#define _2DPlotYAxesMinimumRangeResetValue    10000.0

// 2D plot padding configuration macros
#define _2DPlotTopPixelPadding           40
#define _2DPlotBottomPixelPadding        10
#define _2DPlotLeftPixelPadding          75
#define _2DPlotRightPixelPadding         30

// 2D plot additional offset position macros
#define _2DPlotTitleYOffsetValue         15
#define _2DPlotTitleXOffsetValue         30
#define _2DPlotXAxesLabelOffset          25
#define _2DPlotYAxesLabelOffset          60

// 2D plot line thickness configuration macros
#define _2DPlotTickLinePixelLength       5
#define _2DPlotAxesAndBoxLineWidth       1
#define _2DPlotTickLineWidth             1

// 2D plot data logging indicator configuration macros
#define _2DPlotIndicatorLabelYOffset     5

// 2D plot data set structure format
struct PlotDataSet {

	// 2D plot data set structure
	GLfloat PlotDataPoints[_2DPlotXAxesLength];
	GLfloat MaximumDataValue = _2DPlotYAxesMaximumRangeResetValue;
	GLfloat MinimumDataValue = _2DPlotYAxesMinimumRangeResetValue;
	unsigned int PlotLineDataIndexRenderOffset = _2DPlotXAxesLength;

};

// Global static class objects and variables
static bool EnabledPlotDataSets[_2DPlotMaxNumberOfDataSets];
static PlotDataSet PlotDataSets[_2DPlotMaxNumberOfDataSets];
static unsigned char PlotDataSetLineColorR[_2DPlotMaxNumberOfDataSets];
static unsigned char PlotDataSetLineColorG[_2DPlotMaxNumberOfDataSets];
static unsigned char PlotDataSetLineColorB[_2DPlotMaxNumberOfDataSets];
static GLfloat PlotDataSetLineWidth[_2DPlotMaxNumberOfDataSets];
static unsigned char PlotDataSetRenderingOrder[_2DPlotMaxNumberOfDataSets];
static unsigned char DataLoggingDisplayStringChar[] = {'D','a','t','a',' ','L','o','g','g','i','n','g',' ', '-',' ','D','u','r','a','t','i','o','n',':',' ','0','0','0',':','0','0',':','0','0',':','0','0','0'};

// OpenGL class definition
namespace OpenGL2DPlot {

	// ---------------------------------- Global Class Structure Objects --------------------------------- //

	// Format for axis-to-pixel coordinates
	struct AxesToPixelCoordFormat {

		// Structure variables
		GLfloat XAxesPixelResolution = 0.0;
		GLfloat YAxesPixelResolution = 0.0;
		GLfloat XPixelCoordinate = 0.0;
		GLfloat YPixelCoordinate = 0.0;

	};

	// ------------------------- Private Custom WinForms Transparent Panel Class ------------------------- //

	// Associated local class namespace object
	namespace NativeForm = System::Windows::Forms;

	// Transparent overlay panel class for the image rendering panel
	private ref class TextureOverlayPanel : System::Windows::Forms::Panel {

		// Local class objects
		protected: System::Drawing::Graphics^ graphics;

		protected: virtual property NativeForm::CreateParams^ CreateParams {

			// Override the configuration parameters of the panel
			NativeForm::CreateParams^ get() override {

				// Read the control parameters of the panel
				NativeForm::CreateParams^ PanalParams = __super::CreateParams;
				// The extended style of the panel must be transparent
				PanalParams->ExStyle |= WS_EX_TRANSPARENT;

				// Return the config parameters of the panel
				return PanalParams;

			}

		}

		public: TextureOverlayPanel() {

			// Transparent overlay panel class constructor

		}

		virtual void OnPaintBackground(PaintEventArgs^ e) override {

				// No background should be drawn

		}

		protected: virtual void OnPaint(PaintEventArgs^ e) override {

			// No additional graphics should be generated for the panel 

		}

	};

	// ----------------------------------------------------------------------------------------------------- //

	public ref class RMHOpenGL2DPlot : public System::Windows::Forms::NativeWindow {

	private:

		// Private global class objects and variables
		private: HDC m_hDC;
		private: HGLRC m_hglrc;
		private: GLuint BaseFont;
		private: GLint iPixelFormat;
		private: unsigned int OpenGLWindowWidth;
		private: unsigned int OpenGLWindowHeight;
		private: bool OverlayPanelIsClick = false;
		private: GLdouble CurrentTexturePanelWidth;
		private: GLdouble CurrentTexturePanelHeight;
		private: GLfloat TextureToPanelWidthOffset = 0.0;
		private: GLfloat TextureToPanelHeightOffset = 0.0;
		private: CreateParams^ ControlParams = gcnew CreateParams;
		private: TextureOverlayPanel^ OverlayPanel = gcnew TextureOverlayPanel();

		// Associated 2D plot variables and objects
		private: GLfloat XAxesLineX0 = 0.0;
		private: GLfloat XAxesLineY0 = 0.0;
		private: GLfloat XAxesLineX1 = 0.0;
		private: GLfloat XAxesLineY1 = 0.0;
		private: GLfloat YAxesLineX0 = 0.0;
		private: GLfloat YAxesLineY0 = 0.0;
		private: GLfloat YAxesLineX1 = 0.0;
		private: GLfloat YAxesLineY1 = 0.0;
		private: GLfloat XAxesLinePixelLength = 0.0;
		private: GLfloat YAxesLinePixelLength = 0.0;
		private: GLfloat XAxesTickSpacing = 0.0;
		private: GLfloat YAxesTickSpacing = 0.0;
		private: GLdouble XDataMaxMinSpacing = 0.0;
		private: GLdouble YDataMaxMinSpacing = 0.0;
		private: GLdouble XDataLabelValue = 0.0;
		private: GLdouble YDataLabelValue = 0.0;
		private: GLfloat PlotYAxesMaximumRangeValue = _2DPlotYAxesMaximumRangeResetValue;
		private: GLfloat PlotYAxesMinimumRangeValue = _2DPlotYAxesMinimumRangeResetValue;
		private: GLfloat PlotXAxesMaximumRangeValue = 0.0;
		private: GLfloat PlotXAxesMinimumRangeValue = 0.0;
		private: bool Show2DPlotBoxFlag = true;
		private: bool Show2DPlotGridFlag = true;
		private: bool DataLoggingFLag = false;
		private: unsigned char XAxesNumberOfTicks = 25;
		private: unsigned char YAxesNumberOfTicks = 10;
		private: unsigned short RenderingLoopIterationLength = 0;
		private: unsigned long LoggingTimerLabelMilliSecValue = 0;
		private: GLfloat MousePointerXPosition = 0;
		private: GLfloat MousePointerYPosition = 0;
		private: bool EnablePlotMouseCursorDataFlag = false;
		private: GLfloat MouseCursorDataLabelXOffset = 12.0;
		private: GLfloat MouseCursorDataLabelYOffset = -10.0;

		// 2D plot color configuration variables 
		private: unsigned char PlotAxesColorR = 100;
		private: unsigned char PlotAxesColorG = 100;
		private: unsigned char PlotAxesColorB = 100;
		private: unsigned char PlotBoxColorR = 100;
		private: unsigned char PlotBoxColorG = 100;
		private: unsigned char PlotBoxColorB = 100;
		private: unsigned char PlotGridLinesColorR = 60;
		private: unsigned char PlotGridLinesColorG = 60;
		private: unsigned char PlotGridLinesColorB = 60;
		private: unsigned char PlotTickLinesColorR = 100;
		private: unsigned char PlotTickLinesColorG = 100;
		private: unsigned char PlotTickLinesColorB = 100;
		private: unsigned int FontRebuildAttempts = 0;   // Number of times the text font was rebuilt at draw time
		private: unsigned int DiagFrameCount = 0;        // Number of rendered frames (diagnostic log)
		private: unsigned char PlotTitleColorR = 255;
		private: unsigned char PlotTitleColorG = 255;
		private: unsigned char PlotTitleColorB = 255;
		private: unsigned char MouseCursorDataLabelColorR = 255;
		private: unsigned char MouseCursorDataLabelColorG = 255;
		private: unsigned char MouseCursorDataLabelColorB = 255;

	public:

		// ------------------------- 2D Plot Constructor Routines -------------------------- //

		RMHOpenGL2DPlot(System::Windows::Forms::Panel^ TexturePanel, unsigned char WidthScaleFactor, unsigned char HeightScaleFactor) {

			// Set the position of the control class
			ControlParams->X = 0;
			ControlParams->Y = 0;
			ControlParams->Width = (GLdouble)TexturePanel->Width * WidthScaleFactor;
			ControlParams->Height = (GLdouble)TexturePanel->Height * HeightScaleFactor;

			// Read the pixel width and height of the OpenGL window
			OpenGLWindowWidth = ControlParams->Width;
			OpenGLWindowHeight = ControlParams->Height;

			// Configure the texture parent handler
			ControlParams->Parent = TexturePanel->Handle;
			// Create a "child" of the selected "parent" and make it OpenGL compliant
			ControlParams->Style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

			// Generate the texture window handle
			this->CreateHandle(ControlParams);

			// Pointer to the texture handle
			m_hDC = GetDC((HWND)this->Handle.ToPointer());

			// Is this handle active
			if (m_hDC) {

				// Configure the texture pixel format
				RMH_OpenGL_SetTexturePixelFormat(m_hDC);
				// Configure the size of the texture
				RMH_OpenGL_ResizeOpenGLWinformsScene(ControlParams->Width, ControlParams->Height);
				// Initialize OpenGL
				RMH_OpenGL_Init();
				RMH_Diag_Init();

			}

			// Add an overlaid transparent panel to the texture panel
			RMH_OpenGL_AddOverlayPanelToMainTexturePanel(TexturePanel);

			// Reset the data set enable array of the 2D plot
			RMH_OpenGL_ResetPlotDataSetEnableArray();
			// Reset the line thickness array of the 2D plot 
			RMH_OpenGL_ResetPlotLineWidthArray();

			// Set the default line colors of the data sets
			RMH_OpenGL_SetDataSetLineColor(0, 255, 0, 0);
			RMH_OpenGL_SetDataSetLineColor(1, 0, 0, 255);
			RMH_OpenGL_SetDataSetLineColor(2, 50, 205, 50);
			RMH_OpenGL_SetDataSetLineColor(3, 255, 255, 0);
			RMH_OpenGL_SetDataSetLineColor(4, 255, 128, 0);
			RMH_OpenGL_SetDataSetLineColor(5, 255, 0, 255);
			RMH_OpenGL_SetDataSetLineColor(6, 0, 255, 255);
			RMH_OpenGL_SetDataSetLineColor(7, 255, 255, 255);
			RMH_OpenGL_SetDataSetLineColor(8, 128, 128, 128);
			RMH_OpenGL_SetDataSetLineColor(9, 128, 255, 128);

		}

		// ----------- Texture Overlaid Transparent Panel Setup Routine ------------ //

		private: GLvoid RMH_OpenGL_AddOverlayPanelToMainTexturePanel(System::Windows::Forms::Panel^ TexturePanel) {

			// This routine adds an overlaid transparent panel to the texture panel
			// This overlaid panel is used to manipulate objects on the image texture

			// The overlaid panel must not have any margin or padding
			OverlayPanel->Margin = System::Windows::Forms::Padding(0, 0, 0, 0);
			OverlayPanel->Padding = System::Windows::Forms::Padding(0, 0, 0, 0);

			// The overlaid transparent panel must fill the whole texture panel
			OverlayPanel->Dock = System::Windows::Forms::DockStyle::Fill;

			// Add the overlaid transparent panel as a "child" of the texture panel
			TexturePanel->Controls->Add(OverlayPanel);

			// Make sure a right-click on the overlaid transparent panel opens the context menu of the texture panel
			OverlayPanel->ContextMenuStrip = TexturePanel->ContextMenuStrip;

			// Enable mouse handler events for the transparent panel
			OverlayPanel->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &RMHOpenGL2DPlot::TexturePanel_MouseDown);
			OverlayPanel->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &RMHOpenGL2DPlot::TexturePanel_MouseUp);
			OverlayPanel->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &RMHOpenGL2DPlot::TexturePanel_MouseMove);
			OverlayPanel->MouseWheel += gcnew System::Windows::Forms::MouseEventHandler(this, &RMHOpenGL2DPlot::TexturePanel_MouseWheel);

		}

		// ----------------- Texture Initialization And Handling Routines ----------------- //

		// ---------------- Diagnostic log of the 2D plot (written to IRCAM_plot_diagnostic.txt next to the program) ---------------- //

		private: GLvoid RMH_Diag_Write(System::String^ Text) {

			// Appends a line to the diagnostic log file
			try {
				System::String^ LogPath = System::IO::Path::Combine(System::Windows::Forms::Application::StartupPath, "IRCAM_plot_diagnostic.txt");
				System::IO::File::AppendAllText(LogPath, Text + System::Environment::NewLine);
			}
			catch (System::Exception^) {}

		}

		private: System::String^ RMH_Diag_YesNo(bool Value) {

			if (Value == true) { return gcnew System::String("yes"); }
			return gcnew System::String("NO");

		}

		private: System::String^ RMH_Diag_GLString(GLenum Name) {

			const GLubyte* GLText = glGetString(Name);
			if (GLText == NULL) { return gcnew System::String("NULL (no current render context)"); }
			return gcnew System::String((const char*)GLText);

		}

		private: GLvoid RMH_Diag_Init() {

			// Logs the graphics driver and pixel format information once, when the plot is created
			System::Text::StringBuilder^ Line = gcnew System::Text::StringBuilder();

			RMH_Diag_Write(gcnew System::String("==== 2D plot diagnostic ===="));
			RMH_Diag_Write(System::DateTime::Now.ToString());
			RMH_Diag_Write(gcnew System::String("GL_VERSION:  ") + RMH_Diag_GLString(GL_VERSION));
			RMH_Diag_Write(gcnew System::String("GL_VENDOR:   ") + RMH_Diag_GLString(GL_VENDOR));
			RMH_Diag_Write(gcnew System::String("GL_RENDERER: ") + RMH_Diag_GLString(GL_RENDERER));

			PIXELFORMATDESCRIPTOR DiagPfd;
			int DiagFormat = GetPixelFormat(m_hDC);
			DescribePixelFormat(m_hDC, DiagFormat, sizeof(PIXELFORMATDESCRIPTOR), &DiagPfd);

			Line->Append("Pixel format ");
			Line->Append(System::Convert::ToString(DiagFormat));
			Line->Append(": flags=0x");
			Line->Append(System::Convert::ToString((int)DiagPfd.dwFlags, 16));
			Line->Append(" color=");
			Line->Append(System::Convert::ToString((int)DiagPfd.cColorBits));
			Line->Append(" alpha=");
			Line->Append(System::Convert::ToString((int)DiagPfd.cAlphaBits));
			Line->Append(" depth=");
			Line->Append(System::Convert::ToString((int)DiagPfd.cDepthBits));
			Line->Append(" Microsoft software format=");
			Line->Append(RMH_Diag_YesNo((DiagPfd.dwFlags & PFD_GENERIC_FORMAT) != 0 && (DiagPfd.dwFlags & PFD_GENERIC_ACCELERATED) == 0));
			RMH_Diag_Write(Line->ToString());

			Line->Length = 0;
			Line->Append("Render context is current=");
			Line->Append(RMH_Diag_YesNo(wglGetCurrentContext() == m_hglrc));
			Line->Append(" font list exists=");
			Line->Append(RMH_Diag_YesNo(glIsList(BaseFont) == GL_TRUE));
			Line->Append(" glGetError=");
			Line->Append(System::Convert::ToString((int)glGetError()));
			RMH_Diag_Write(Line->ToString());

		}

		private: GLvoid RMH_Diag_Frame() {

			// Logs the plot state for a few of the first rendered frames
			DiagFrameCount++;
			if (DiagFrameCount != 2 && DiagFrameCount != 60 && DiagFrameCount != 300) { return; }

			GLint DiagViewport[4] = { 0, 0, 0, 0 };
			glGetIntegerv(GL_VIEWPORT, DiagViewport);
			GLfloat DiagClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			glGetFloatv(GL_COLOR_CLEAR_VALUE, DiagClearColor);
			GLubyte DiagPixel[4] = { 0, 0, 0, 0 };
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glReadBuffer(GL_BACK);
			glReadPixels(10, 10, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, DiagPixel);

			System::Text::StringBuilder^ Line = gcnew System::Text::StringBuilder();

			Line->Append("-- frame ");
			Line->Append(System::Convert::ToString((int)DiagFrameCount));
			Line->Append(": viewport=");
			Line->Append(System::Convert::ToString(DiagViewport[0]));
			Line->Append(",");
			Line->Append(System::Convert::ToString(DiagViewport[1]));
			Line->Append(",");
			Line->Append(System::Convert::ToString(DiagViewport[2]));
			Line->Append(",");
			Line->Append(System::Convert::ToString(DiagViewport[3]));
			Line->Append(" panel=");
			Line->Append(System::Convert::ToString((double)CurrentTexturePanelWidth));
			Line->Append(" x ");
			Line->Append(System::Convert::ToString((double)CurrentTexturePanelHeight));
			RMH_Diag_Write(Line->ToString());

			Line->Length = 0;
			Line->Append("   clear color=");
			Line->Append(System::Convert::ToString((double)DiagClearColor[0]));
			Line->Append(",");
			Line->Append(System::Convert::ToString((double)DiagClearColor[1]));
			Line->Append(",");
			Line->Append(System::Convert::ToString((double)DiagClearColor[2]));
			Line->Append("  back buffer pixel(10,10)=");
			Line->Append(System::Convert::ToString((int)DiagPixel[0]));
			Line->Append(",");
			Line->Append(System::Convert::ToString((int)DiagPixel[1]));
			Line->Append(",");
			Line->Append(System::Convert::ToString((int)DiagPixel[2]));
			Line->Append("  glGetError=");
			Line->Append(System::Convert::ToString((int)glGetError()));
			Line->Append("  font list exists=");
			Line->Append(RMH_Diag_YesNo(glIsList(BaseFont) == GL_TRUE));
			Line->Append("  font rebuilds=");
			Line->Append(System::Convert::ToString((int)FontRebuildAttempts));
			RMH_Diag_Write(Line->ToString());

			Line->Length = 0;
			Line->Append("   Y axis range=");
			Line->Append(System::Convert::ToString((double)PlotYAxesMinimumRangeValue));
			Line->Append(" to ");
			Line->Append(System::Convert::ToString((double)PlotYAxesMaximumRangeValue));
			RMH_Diag_Write(Line->ToString());

			// Log the newest values of each enabled data set
			for (unsigned int ds = 0; ds < _2DPlotMaxNumberOfDataSets; ds++) {

				if (EnabledPlotDataSets[ds] == true) {

					unsigned int BadValues = 0;
					GLfloat MinValue = 1.0e30f, MaxValue = -1.0e30f;
					for (unsigned int k = _2DPlotXAxesLength - 100; k < _2DPlotXAxesLength; k++) {
						GLfloat V = PlotDataSets[ds].PlotDataPoints[k];
						if (!(V > -1.0e30f && V < 1.0e30f)) { BadValues++; }
						else { if (V < MinValue) { MinValue = V; } if (V > MaxValue) { MaxValue = V; } }
					}

					Line->Length = 0;
					Line->Append("   data set ");
					Line->Append(System::Convert::ToString((int)ds));
					Line->Append(": render offset=");
					Line->Append(System::Convert::ToString((int)PlotDataSets[ds].PlotLineDataIndexRenderOffset));
					Line->Append("  last 100 values min=");
					Line->Append(System::Convert::ToString((double)MinValue));
					Line->Append(" max=");
					Line->Append(System::Convert::ToString((double)MaxValue));
					Line->Append("  invalid=");
					Line->Append(System::Convert::ToString((int)BadValues));
					RMH_Diag_Write(Line->ToString());

					Line->Length = 0;
					Line->Append("      newest 12 values: ");
					for (unsigned int k = _2DPlotXAxesLength - 12; k < _2DPlotXAxesLength; k++) {
						Line->Append(System::Convert::ToString((double)PlotDataSets[ds].PlotDataPoints[k]));
						Line->Append(" ");
					}
					RMH_Diag_Write(Line->ToString());

				}

			}

		}

		private: GLvoid RMH_OpenGL_MakeRenderContextCurrent() {

			// This routine makes the associated render context the current render context

			// Make the associated render context the current render context
			wglMakeCurrent(m_hDC, m_hglrc);

		}

		private: GLvoid RMH_OpenGL_MakeRenderContextNULL() {

			// This routine resets the associated render context

			// Reset the render context
			wglMakeCurrent(NULL, NULL);

		}

		private: GLvoid RMH_OpenGL_UpdateTextureFieldOfView(unsigned int TextureWidth, unsigned int TextureHeight) {

			// This routine sets the viewing angle (field of view) of the texture for display in the control handler component

			// Local variables
			GLdouble PlaneXLook = 0.0;
			GLdouble PlaneYLook = 0.0;
			GLdouble PlaneFieldOfView = 60.0;
			GLdouble HalfFieldOfView = 0.5235986291;   // 3.141592654 * PlaneFieldOfView * (1 / 360); 
			GLdouble TanHalfFieldOfView = 1.732051393; // tanf(HalfFieldOfView);
			GLdouble PlaneDistance = 0.0;
			GLdouble PlaneAspectRatio = 0.0;

			// Read the height and width of the data frame
			PlaneXLook = (GLdouble)TextureWidth * 0.5;
			PlaneYLook = (GLdouble)TextureHeight * 0.5;

			// Calculate the texture aspect ratio
			PlaneAspectRatio = ((GLdouble)TextureWidth / (GLdouble)TextureHeight);

			// Calculate the distance between the frame data plane and the texture plane
			PlaneDistance = (GLdouble)TextureHeight * TanHalfFieldOfView; 

			// Update the viewing angle (field of view) of the texture
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(PlaneFieldOfView, PlaneAspectRatio, 0.01f, 2000.0);
			gluLookAt(PlaneXLook, PlaneYLook, PlaneDistance * 0.5, PlaneXLook, PlaneYLook, 0, 0, 1, 0);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

		}

		// ------------------ Label Rendering And Handling Routines ------------------- //

		private: GLvoid RMH_OpenGL_glPrint(const char* CharArray) {

			// This routine renders a set of characters on an OpenGL texture 

			// Add the font list properties
			glPushAttrib(GL_LIST_BIT);
			// Use the FONT base list
			glListBase(BaseFont - 32);
			// Execute and render the characters on the texture
			glCallLists(strlen(CharArray), GL_UNSIGNED_BYTE, CharArray);
			// Restore the list properties
			glPopAttrib();

		}

		private: GLvoid RMH_OpenGL_RenderStringOnTexture(GLfloat StringX, GLfloat StringY, std::string DisplayString, GLubyte ColorR, GLubyte ColorG, GLubyte ColorB) {

			// This routine renders a given string on an OpenGL texture

			// Configure the text color
			glColor3ub(ColorR, ColorG, ColorB);
			// Set the position of the text on the texture
			glRasterPos2f(StringX, StringY);

			// Render the given string on the texture
			RMH_OpenGL_glPrint(DisplayString.c_str());

		}

		private: GLvoid RMH_OpenGL_glPrintUnsigned(unsigned char* CharArray, unsigned int CharArrayLength) {

			// This routine renders a set of characters on an OpenGL texture 

			// Add the font list properties
			glPushAttrib(GL_LIST_BIT);
			// Use the FONT base list
			glListBase(BaseFont - 32);
			// Execute and render the characters on the texture
			glCallLists(CharArrayLength, GL_UNSIGNED_BYTE, CharArray);
			// Restore the list properties
			glPopAttrib();

		}

		private: GLvoid RMH_OpenGL_RenderStringOnTextureChar(GLfloat StringX, GLfloat StringY, unsigned char *DisplayString, unsigned int CharArrayLength, GLubyte ColorR, GLubyte ColorG, GLubyte ColorB) {

			// This routine renders a given string on an OpenGL texture

			// Configure the text color
			glColor3ub(ColorR, ColorG, ColorB);
			// Set the position of the text on the texture
			glRasterPos2f(StringX, StringY);

			// Render the given string on the texture
			RMH_OpenGL_glPrintUnsigned(DisplayString, CharArrayLength);

		}

		// ---------- 2D Plot Coordinate And Data Formatting/Handling Routines ---------- //

		private: AxesToPixelCoordFormat RMH_OpenGL_2DPLotConvertXYAxesCoordinatesToPixelCoordinates(GLfloat XCoordinate, GLfloat YCoordinate, GLfloat XAxesMaxRange, GLfloat XAxesMinRange, GLfloat YAxesMaxRange, GLfloat YAxesMinRange) {

			// This routine converts a given set of X/Y axis coordinates to actual pixel coordinates

			// Local variables
			AxesToPixelCoordFormat XYPixelCoords;
			
			// Calculate the pixel resolution of the X axis
			XYPixelCoords.XAxesPixelResolution = (XAxesMaxRange - XAxesMinRange) / XAxesLinePixelLength;
			// Calculate the actual X pixel coordinate value
			XYPixelCoords.XPixelCoordinate = (XCoordinate - XAxesMinRange) / XYPixelCoords.XAxesPixelResolution;

			// Calculate the pixel resolution of the X axis
			XYPixelCoords.YAxesPixelResolution = (YAxesMaxRange - YAxesMinRange) / YAxesLinePixelLength;
			// Calculate the actual X pixel coordinate value
			XYPixelCoords.YPixelCoordinate = (YAxesMaxRange - YCoordinate) / XYPixelCoords.YAxesPixelResolution;

			// Compensate for the padding of the 3D plot
			XYPixelCoords.XPixelCoordinate = (XYPixelCoords.XPixelCoordinate + _2DPlotLeftPixelPadding);
			XYPixelCoords.YPixelCoordinate = (XYPixelCoords.YPixelCoordinate + _2DPlotTopPixelPadding);

			// Return the pixel coordinates
			return XYPixelCoords;

		}

		private: GLvoid RMH_OpenGL_ResetPlotDataSetEnableArray() {

			// This routine resets the data set enable array of the 2D plot 

			// Loop up to and including the maximum number of 2D plot data sets
			for (unsigned int i = 0; i < _2DPlotMaxNumberOfDataSets; i++) {

				// Reset the data set enable array of the 2D plot
				EnabledPlotDataSets[i] = false;

			}

		}

		private: GLvoid RMH_OpenGL_ResetPlotLineWidthArray() {

			// This routine resets the line thickness array of the 2D plot 

			// Loop up to and including the maximum number of 2D plot data sets
			for (unsigned int i = 0; i < _2DPlotMaxNumberOfDataSets; i++) {

				// Reset the data set line thickness array of the 2D plot
				PlotDataSetLineWidth[i] = 1.0;

			}

		}

		// ----------------- 2D Plot Texture Rendering And Plot Routines ------------------ //

		private: GLvoid RMH_OpenGL_ClearTextureBuffer() {

			// This routine clears the associated texture buffers

			// Clear the texture color and bit buffers
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		}

		private: GLvoid RMH_OpenGL_RenderLine(GLfloat LineX0, GLfloat LineY0, GLfloat LineX1, GLfloat LineY1, GLfloat LineWidth, GLfloat ColorR, GLfloat ColorG, GLfloat ColorB) {

			// This routine renders a simple line with the given input parameters

			// Enable OpenGL 1D texture
			glEnable(GL_TEXTURE_1D);

			// Set the color of the arrow
			glColor3ub(ColorR, ColorG, ColorB);
			// Set the line thickness of the arrow
			glLineWidth(LineWidth);

			// Render lines on the texture
			glBegin(GL_LINES);

			// Render the primary line of the arrow
			glVertex2f(LineX0, LineY0);
			glVertex2f(LineX1, LineY1);

			// End of configuration
			glEnd();
			// Disable the 1D texture
			glDisable(GL_TEXTURE_1D);

		}

		private: GLvoid RMH_OpenGL_Render2DPlotBaseStructure(unsigned int PlotPanelWidth, unsigned int PlotPanelHeight, unsigned int NmbOfXAxesTicks, unsigned int NmbOfYAxesTicks, GLdouble YAxesMaxRange, GLdouble YAxesMinRange, bool Show2DPlorBox, bool Show2DPlotGrid, std::string PlotTitleString, System::String^ TempUnitString) {

			// This routine renders the X and Y axis lines, ticks, grid lines, tick labels and axis descriptions of the 2D plot

			// Set the coordinates of the X/Y axes of the 2D plot 
			XAxesLineX0 = _2DPlotLeftPixelPadding;
			XAxesLineY0 = PlotPanelHeight - _2DPlotBottomPixelPadding;
			XAxesLineX1 = PlotPanelWidth - _2DPlotRightPixelPadding;
			XAxesLineY1 = PlotPanelHeight - _2DPlotBottomPixelPadding;
			YAxesLineX0 = _2DPlotLeftPixelPadding;
			YAxesLineY0 = _2DPlotTopPixelPadding;
			YAxesLineX1 = _2DPlotLeftPixelPadding;
			YAxesLineY1 = PlotPanelHeight - _2DPlotBottomPixelPadding;

			// Set the Y data start label values to the range minimum
			YDataLabelValue = YAxesMaxRange;

			// Calculate the pixel length of the X and Y axes of the 2D plot
			XAxesLinePixelLength = XAxesLineX1 - XAxesLineX0;
			YAxesLinePixelLength = YAxesLineY1 - YAxesLineY0;

			// Calculate the distances between each tick line on the X and Y axes
			XAxesTickSpacing = XAxesLinePixelLength / (GLfloat)NmbOfXAxesTicks;
			YAxesTickSpacing = YAxesLinePixelLength / (GLfloat)NmbOfYAxesTicks;

			// Calculate the Y axis data distribution between each Y tick 
			YDataMaxMinSpacing = (YAxesMaxRange - YAxesMinRange) / (GLdouble)NmbOfYAxesTicks;

			// Render 2D plot title
			RMH_OpenGL_RenderStringOnTexture((XAxesLinePixelLength / 2.0) - _2DPlotTitleXOffsetValue, _2DPlotTopPixelPadding - _2DPlotTitleYOffsetValue, PlotTitleString, PlotTitleColorR, PlotTitleColorG, PlotTitleColorB);

			// Render the 2D plot X-axis line
			RMH_OpenGL_RenderLine(XAxesLineX0, XAxesLineY0, XAxesLineX1, XAxesLineY1, _2DPlotAxesAndBoxLineWidth, PlotAxesColorR, PlotAxesColorG, PlotAxesColorB);
			// Render the 2D plot Y-axis line
			RMH_OpenGL_RenderLine(YAxesLineX0, YAxesLineY0, YAxesLineX1, YAxesLineY1, _2DPlotAxesAndBoxLineWidth, PlotAxesColorR, PlotAxesColorG, PlotAxesColorB);

			// Should the box of the 2D plot be shown
			if (Show2DPlorBox == true) {

				// Render the surrounding box of the 2D plot
				RMH_OpenGL_RenderLine(XAxesLineX1, XAxesLineY0, XAxesLineX1, _2DPlotTopPixelPadding, _2DPlotAxesAndBoxLineWidth, PlotBoxColorR, PlotBoxColorG, PlotBoxColorB);
				RMH_OpenGL_RenderLine(_2DPlotLeftPixelPadding, _2DPlotTopPixelPadding, XAxesLineX1, _2DPlotTopPixelPadding, _2DPlotAxesAndBoxLineWidth, PlotBoxColorR, PlotBoxColorG, PlotBoxColorB);

			}

			// Check which axis is to have the most ticks rendered
			if (NmbOfXAxesTicks > NmbOfYAxesTicks) {

				// Set the rendering iteration length to the maximum number of X ticks
				RenderingLoopIterationLength = NmbOfXAxesTicks;

			}
			else {

				// Set the rendering iteration length to the maximum number of Y ticks
				RenderingLoopIterationLength = NmbOfYAxesTicks;

			}

			// ----------------------------------- Render 2D Plot Tick Labels ------------------------------------ //

			// Loop up to and including the maximum number of ticks for a given axis
			for (unsigned int i = 0; i < RenderingLoopIterationLength + 1; i++) {

				// Render up to and including the number of Y tick lines
				if (i <= NmbOfYAxesTicks) {

					// Render the X-axis labels
					RMH_OpenGL_RenderStringOnTexture(YAxesLineX0 - _2DPlotYAxesLabelOffset, YAxesLineY0, RMH_Conversion_SystemStringToStdString(YDataLabelValue.ToString("F2") + " " + TempUnitString), PlotTitleColorR, PlotTitleColorG, PlotTitleColorB);

					// Render each X-axis tick line with the calculated distance
					YAxesLineY0 = YAxesLineY0 + YAxesTickSpacing;

					// Format the Y label tick data from the data spacing difference
					YDataLabelValue = YDataLabelValue - YDataMaxMinSpacing;

				}

			}

			// Reset the coordinates of the X/Y axes of the 2D plot 
			XAxesLineX0 = _2DPlotLeftPixelPadding;
			XAxesLineY0 = PlotPanelHeight - _2DPlotBottomPixelPadding;
			XAxesLineX1 = PlotPanelWidth - _2DPlotRightPixelPadding;
			XAxesLineY1 = PlotPanelHeight - _2DPlotBottomPixelPadding;
			YAxesLineX0 = _2DPlotLeftPixelPadding;
			YAxesLineY0 = _2DPlotTopPixelPadding;
			YAxesLineX1 = _2DPlotLeftPixelPadding;
			YAxesLineY1 = PlotPanelHeight - _2DPlotBottomPixelPadding;

			// ------------------------------------- Render 2D Plot Structure ------------------------------------- //

			// Enable OpenGL 1D texture
			glEnable(GL_TEXTURE_1D);

			// Set the thickness of the tick lines
			glLineWidth(_2DPlotTickLineWidth);

			// Render lines on the texture
			glBegin(GL_LINES);

			// Loop up to and including the maximum number of ticks for a given axis
			for (unsigned int i = 0; i < RenderingLoopIterationLength + 1; i++) {

				// Render up to and including the number of X tick lines
				if (i <= NmbOfXAxesTicks) {

					// Should the grid of the 2D plot be shown
					if (Show2DPlotGrid == true && i > 0 && i < NmbOfXAxesTicks) {

						// Set the color of the grid line
						glColor3ub(PlotGridLinesColorR, PlotGridLinesColorG, PlotGridLinesColorB);

						// Render the tick lines of the Y axis - with grid length
						glVertex2f(XAxesLineX0, _2DPlotTopPixelPadding);
						glVertex2f(XAxesLineX0, XAxesLineY0 + _2DPlotTickLinePixelLength);

					}

					// Set the color of the tick line
					glColor3ub(PlotTickLinesColorR, PlotTickLinesColorG, PlotTickLinesColorB);

					// Render the tick lines of the Y axis - without grid length
					glVertex2f(XAxesLineX0, XAxesLineY0);
					glVertex2f(XAxesLineX0, XAxesLineY0 + _2DPlotTickLinePixelLength);

					// Render each Y-axis tick line with the calculated distance
					XAxesLineX0 = XAxesLineX0 + XAxesTickSpacing;

				}

				// Render up to and including the number of Y tick lines
				if (i <= NmbOfYAxesTicks) {

					// Should the grid of the 2D plot be shown
					if (Show2DPlotGrid == true && i > 0 && i < NmbOfYAxesTicks) {

						// Set the color of the grid line
						glColor3ub(PlotGridLinesColorR, PlotGridLinesColorG, PlotGridLinesColorB);

						// Render the tick lines of the X axis
						glVertex2f(XAxesLineX1, YAxesLineY0);
						glVertex2f(YAxesLineX0 - _2DPlotTickLinePixelLength, YAxesLineY0);

					}

					// Set the color of the tick line
					glColor3ub(PlotTickLinesColorR, PlotTickLinesColorG, PlotTickLinesColorB);

					// Render the tick lines of the X axis
					glVertex2f(YAxesLineX0, YAxesLineY0);
					glVertex2f(YAxesLineX0 - _2DPlotTickLinePixelLength, YAxesLineY0);

					// Render each X-axis tick line with the calculated distance
					YAxesLineY0 = YAxesLineY0 + YAxesTickSpacing;

				}

			}

			// End of configuration
			glEnd();
			// Disable the 1D texture
			glDisable(GL_TEXTURE_1D);

			// --------------------------------------------------------------------------------------------------- //

		}

		private: GLvoid RMH_OpenGL_PlotDataSet(unsigned char DataSetIndex, GLfloat XAxesMaxRange, GLfloat XAxesMinRange, GLfloat YAxesMaxRange, GLfloat YAxesMinRange, GLfloat LineWidth, GLubyte LineColorR, GLubyte LineColorG, GLubyte LineColorB) {

			// This routine plots a series of data sets on the 2D plot

			// Read the temporary array data and sort the kernel array
			GLfloat FirstDataValue = 0.0;
			GLfloat SecondDataValue = 0.0;
			AxesToPixelCoordFormat FirstDataPointCoords;
			AxesToPixelCoordFormat SecondDataPointCoords;

			// Reset the Y-axis max/min range variables of the 2D plot
			PlotDataSets[DataSetIndex].MaximumDataValue = _2DPlotYAxesMaximumRangeResetValue;
			PlotDataSets[DataSetIndex].MinimumDataValue = _2DPlotYAxesMinimumRangeResetValue;

			// Loop up to and including the length of the data array
			for (unsigned int i = PlotDataSets[DataSetIndex].PlotLineDataIndexRenderOffset; i < XAxesMaxRange - 1; i++) {

				// Read the first and second input data values
				FirstDataValue = PlotDataSets[DataSetIndex].PlotDataPoints[i];
				SecondDataValue = PlotDataSets[DataSetIndex].PlotDataPoints[i + 1];

				// Update the maximum and minimum Y range variables of the 2D plot from the max/min values of the data
				if (FirstDataValue > PlotDataSets[DataSetIndex].MaximumDataValue) { PlotDataSets[DataSetIndex].MaximumDataValue = FirstDataValue; }
				if (FirstDataValue < PlotDataSets[DataSetIndex].MinimumDataValue) { PlotDataSets[DataSetIndex].MinimumDataValue = FirstDataValue; }

				// Convert the plot point data coordinates to pixel point coordinates
				FirstDataPointCoords = RMH_OpenGL_2DPLotConvertXYAxesCoordinatesToPixelCoordinates(i, FirstDataValue, XAxesMaxRange, XAxesMinRange, YAxesMaxRange, YAxesMinRange);
				SecondDataPointCoords = RMH_OpenGL_2DPLotConvertXYAxesCoordinatesToPixelCoordinates(i + 1, SecondDataValue, XAxesMaxRange, XAxesMinRange, YAxesMaxRange, YAxesMinRange);

				// Plot the data line with the selected thickness and color
				RMH_OpenGL_RenderLine(FirstDataPointCoords.XPixelCoordinate, FirstDataPointCoords.YPixelCoordinate, SecondDataPointCoords.XPixelCoordinate, SecondDataPointCoords.YPixelCoordinate, LineWidth, LineColorR, LineColorG, LineColorB);

			}

			// Check whether the value of the line rendering index offset is 0
			if (PlotDataSets[DataSetIndex].PlotLineDataIndexRenderOffset != 0) {

				// Increment the line rendering index offset
				PlotDataSets[DataSetIndex].PlotLineDataIndexRenderOffset = PlotDataSets[DataSetIndex].PlotLineDataIndexRenderOffset - 1;

			}

		}

		private: GLvoid RMH_OpenGL_PlotDataSets() {

			// This routine plots the enabled plot data sets

			// Read the temporary array data and sort the kernel array
			unsigned int RenderingOrderIndex = 0;

			// Loop up to and including the maximum number of 2D plot data sets
			for (unsigned int i = 0; i < _2DPlotMaxNumberOfDataSets; i++) {

				// Is the data set enabled and to be plotted
				if (EnabledPlotDataSets[i] == true) {

					// Plot the data set with the selected line thickness and line color
					RMH_OpenGL_PlotDataSet(i, _2DPlotXAxesLength, 0, PlotYAxesMaximumRangeValue, PlotYAxesMinimumRangeValue, PlotDataSetLineWidth[i], PlotDataSetLineColorR[i], PlotDataSetLineColorG[i], PlotDataSetLineColorB[i]);

					// Read the data set rendering order of the 2D plot
					PlotDataSetRenderingOrder[RenderingOrderIndex] = i;

					// Increment the rendering order index
					RenderingOrderIndex = RenderingOrderIndex + 1;

				}

			}

		}

		private: GLvoid RMH_OpenGL_RenderDataLoggingIndicatorLabelWithTimer(GLfloat LabelX, GLfloat LabelY, unsigned long MilliSecondsValue, bool LoggingDataFlag) {

			// This routine renders a data logging indicator

			// Read the temporary array data and sort the kernel array
			unsigned int HoursValue = 0;
			unsigned int MinutesValue = 0;
			unsigned int SecondsValue = 0;
			unsigned int MilliSecValue = 0;

			// Check whether data logging is active
			if (LoggingDataFlag == true) {

				// Convert milliseconds to hours
				HoursValue = ((MilliSecondsValue / 3600000) % 720);
				// Convert milliseconds to minutes
				MinutesValue = ((MilliSecondsValue / 60000) % 60);
				// Convert milliseconds to seconds
				SecondsValue = ((MilliSecondsValue / 1000) % 60);
				// Convert milliseconds to milliseconds
				MilliSecValue = MilliSecondsValue % 1000;

				// Convert hours, minutes, seconds and milliseconds to unsigned char
				DataLoggingDisplayStringChar[25] = (HoursValue / 100) % 10 + 48;
				DataLoggingDisplayStringChar[26] = (HoursValue / 10) % 10 + 48;
				DataLoggingDisplayStringChar[27] = HoursValue % 10 + 48;
				DataLoggingDisplayStringChar[29] = (MinutesValue / 10) % 10 + 48;
				DataLoggingDisplayStringChar[30] = MinutesValue % 10 + 48;
				DataLoggingDisplayStringChar[32] = (SecondsValue / 10) % 10 + 48;
				DataLoggingDisplayStringChar[33] = SecondsValue % 10 + 48;
				DataLoggingDisplayStringChar[35] = (MilliSecValue / 100) % 10 + 48;
				DataLoggingDisplayStringChar[36] = (MilliSecValue / 10) % 10 + 48;
				DataLoggingDisplayStringChar[37] = MilliSecValue % 10 + 48;

				// Render the active data logging label
				RMH_OpenGL_RenderStringOnTextureChar(LabelX, LabelY, DataLoggingDisplayStringChar, 38, 50, 205, 50);

			}
			else {

				// Render the inactive data logging label
				RMH_OpenGL_RenderStringOnTextureChar(LabelX, LabelY, DataLoggingDisplayStringChar, 38, 60, 60, 60);

			}

		}

		// ----------------- Combined 2D Plot Graphics Rendering Routine ----------------- //

		public: GLvoid RMH_OpenGL_Enable2DPlotBox(bool BoxLinesEnableFlag) {

			// This routine enables or disables the box lines of the 2D plot

			// Update the box lines flag of the 2D plot
			Show2DPlotBoxFlag = BoxLinesEnableFlag;

		}

		public: GLvoid RMH_OpenGL_Enable2DPlotGridLines(bool GridLinesEnableFlag) {

			// This routine enables or disables the grid lines of the 2D plot

			// Update the grid lines flag of the 2D plot
			Show2DPlotGridFlag = GridLinesEnableFlag;

		}

		public: GLvoid RMH_OpenGL_Set2DPlotNumberOfXTicks(System::Object^ sender) {

			// This routine sets the number of X ticks of the 2D plot

			// Cast the sender object as a WinForms ToolStrip object
			System::Windows::Forms::ToolStripMenuItem^ NumberOfXticks = (System::Windows::Forms::ToolStripMenuItem^)sender;

			// Read the sub context menu identification tag
			unsigned char NewNumberOfXticks = Convert::ToInt16(NumberOfXticks->Tag);

			// Update the number of X ticks
			XAxesNumberOfTicks = NewNumberOfXticks;

		}

		public: GLvoid RMH_OpenGL_Set2DPlotNumberOfYTicks(System::Object^ sender) {

			// This routine sets the number of Y ticks of the 2D plot

			// Cast the sender object as a WinForms ToolStrip object
			System::Windows::Forms::ToolStripMenuItem^ NumberOfYticks = (System::Windows::Forms::ToolStripMenuItem^)sender;

			// Read the sub context menu identification tag
			unsigned char NewNumberOfYticks = Convert::ToInt16(NumberOfYticks->Tag);

			// Update the number of Y ticks
			YAxesNumberOfTicks = NewNumberOfYticks;

		}

		public: GLvoid RMH_OpenGL_SetDataSetLineWidth(unsigned char DataSetIndex, GLfloat DataSetLineWidth) {

			// This routine sets the line thickness of the selected data set when plotting

			// Check whether the input index value is within the maximum number of plot data sets
			if (DataSetIndex < 0 || DataSetIndex > _2DPlotMaxNumberOfDataSets) {}
			else {

				// Set the line thickness for the data set
				PlotDataSetLineWidth[DataSetIndex] = DataSetLineWidth;

			}

		}

		public: GLvoid RMH_OpenGL_SetDataSetLineColor(unsigned char DataSetIndex, GLubyte LineColorR, GLubyte LineColorG, GLubyte LineColorB) {

			// This routine sets the line thickness of the selected data set when plotting

			// Check whether the input index value is within the maximum number of plot data sets
			if (DataSetIndex < 0 || DataSetIndex > _2DPlotMaxNumberOfDataSets) {}
			else {

				// Set the line color for the data set
				PlotDataSetLineColorR[DataSetIndex] = LineColorR;
				PlotDataSetLineColorG[DataSetIndex] = LineColorG;
				PlotDataSetLineColorB[DataSetIndex] = LineColorB;

			}

		}

		public: GLvoid RMH_OpenGL_AddDataPointToPlotDataSet(GLfloat PointData, unsigned char DataSetIndex) {

			// This routine adds a measurement to the selected plot data set by shifting it into the last position of the buffer array
	
			// Is the selected data set enabled for plotting
			if (EnabledPlotDataSets[DataSetIndex] == true) {

				// Check whether the input index value is within the maximum number of plot data sets
				if (DataSetIndex < 0 || DataSetIndex > _2DPlotMaxNumberOfDataSets) {}
				else {

					// Shift all data of the buffer array one position to the left
					for (unsigned int i = 0; i < _2DPlotXAxesLength - 1; i++) {

						// Shift the index once to the left
						PlotDataSets[DataSetIndex].PlotDataPoints[i] = PlotDataSets[DataSetIndex].PlotDataPoints[i + 1];

					}

					// Add the newest data to the last index position of the buffer array
					PlotDataSets[DataSetIndex].PlotDataPoints[_2DPlotXAxesLength - 1] = PointData;

				}

			}

		}

		public: GLvoid RMH_OpenGL_EnablePlotOfDataSetx(unsigned char DataSetIndex, bool EnablePlotFlag) {

			// This routine enables plotting of a selected data set index

			// Check whether the input index value is within the maximum number of plot data sets
			if (DataSetIndex < 0 || DataSetIndex > _2DPlotMaxNumberOfDataSets) {}
			else {

				// Enable the data set index
				EnabledPlotDataSets[DataSetIndex] = EnablePlotFlag;

			}

		}

		public: GLvoid RMH_OpenGL_ReadDataSetsMaxMinDataRangeValues() {

			// This routine reads the maximum and minimum values of all active data sets and sets the Y-axis range variables of the plot
			// Can be called in a separate process...

			// Reset the Y-axis range variables of the 2D plot to start values
			PlotYAxesMaximumRangeValue = _2DPlotYAxesMaximumRangeResetValue;
			PlotYAxesMinimumRangeValue = _2DPlotYAxesMinimumRangeResetValue;

			// Loop up to and including the maximum number of 2D plot data sets
			for (unsigned char i = 0; i < _2DPlotMaxNumberOfDataSets; i++) {

				// Is the data set enabled and to be plotted
				if (EnabledPlotDataSets[i] == true) {

					// Read the maximum data value of the data set, if it is higher than the current one read
					if (PlotDataSets[i].MaximumDataValue > PlotYAxesMaximumRangeValue) {

						// Set the maximum Y-axis range value of the plot 
						PlotYAxesMaximumRangeValue = PlotDataSets[i].MaximumDataValue;

					}

					// Read the minimum data value of the data set, if it is lower than the current one read
					if (PlotDataSets[i].MinimumDataValue < PlotYAxesMinimumRangeValue) {

						// Set the minimum Y-axis range value of the plot 
						PlotYAxesMinimumRangeValue = PlotDataSets[i].MinimumDataValue;

					}

				}

			}

		}

		public: GLvoid RMH_OpenGL_ResetDataSetLineDataIndexRenderOffset(unsigned char DataSetIndex) {

			// This routine resets the line data rendering index offset value of the selected data set

			// Check whether the input index value is within the maximum number of plot data sets
			if (DataSetIndex < 0 || DataSetIndex > _2DPlotMaxNumberOfDataSets) {}
			else {

				// Enable the data set index
				PlotDataSets[DataSetIndex].PlotLineDataIndexRenderOffset = _2DPlotXAxesLength;

			}

		}

		public: GLvoid RMH_OpenGL_Clear2DPlot() {

			// This routine resets the data of the 2D plot and clears the 2D plot

			// Reset the Y-axis range variables of the 2D plot to start values
			PlotYAxesMaximumRangeValue = _2DPlotYAxesMaximumRangeResetValue;
			PlotYAxesMinimumRangeValue = _2DPlotYAxesMinimumRangeResetValue;

			// Loop up to and including the maximum number of 2D plot data sets
			for (unsigned char i = 0; i < _2DPlotMaxNumberOfDataSets; i++) {

				// Reset the max/min data values of the data set
				PlotDataSets[i].MaximumDataValue = PlotYAxesMaximumRangeValue;
				PlotDataSets[i].MinimumDataValue = PlotYAxesMinimumRangeValue;

				// Reset the data set line data rendering index offset value
				RMH_OpenGL_ResetDataSetLineDataIndexRenderOffset(i);

			}

		}

		public: GLvoid RMH_OpenGL_Get2DPlotDataSetRenderingOrder(unsigned char *DataSetRenderingOrder) {

			// This routine returns the rendering order index array of the 2D plot as a pointer

			// Point to the rendering order index array
			DataSetRenderingOrder = &PlotDataSetRenderingOrder[0];

		}

		public: GLboolean RMH_OpenGL_ReadPlotDataSetEnabledState(unsigned char DataSetIndex) {

			// This routine reads whether the selected plot data set is enabled or disabled

			// Read the temporary array data and sort the kernel array
			bool DataSetEnableFlag = false;

			// Check whether the input index value is within the maximum number of plot data sets
			if (DataSetIndex < 0 || DataSetIndex > _2DPlotMaxNumberOfDataSets) {}
			else {

				// Read the plot data set enable flag
				DataSetEnableFlag = EnabledPlotDataSets[DataSetIndex];

			}

			// Return the plot data set enable flag
			return DataSetEnableFlag;

		}

		public: GLvoid RMH_OpenGL_SetDataLoggingLabelStateAndTimer(bool DataLoggingActiveFlag, unsigned long MilliSecondsValue) {

			// This routine updates the state of the data logging label of the 2D plot
			// Whether active data logging is enabled or disabled

			// Update the data logging flag
			DataLoggingFLag = DataLoggingActiveFlag;

			// Update the private class logging timer variables
			LoggingTimerLabelMilliSecValue = MilliSecondsValue;

		}
		
		public: GLvoid  RMH_2DPlot_EnablePlotMouseCursorPointData(System::Object^ sender) {

			// This routine enables or disables the mouse cursor plot point data

			// Cast the sender object as a WinForms ToolStrip object
			System::Windows::Forms::ToolStripMenuItem^ EnableDisableTagValue = (System::Windows::Forms::ToolStripMenuItem^)sender;
			// Read the sub context menu identification tag
			unsigned int MouseCursorDataFlag = Convert::ToInt32(EnableDisableTagValue->Tag);

			// Enable or disable the rendering of the mouse cursor plot point data
			EnablePlotMouseCursorDataFlag = (bool)MouseCursorDataFlag;

		}

		private: GLvoid RMH_2DPlot_RenderPlotMouseCursorPointData(unsigned int PlotPanelWidth, unsigned int PlotPanelHeight, System::String^ TempUnitString) {

			// This routine renders the mouse cursor plot point data as a label 
			
			// Local variables
			System::String^ LabelString;
			GLfloat PlotAreaPixelHeight = 0.0;	
			GLfloat MouseCursorYTemperature = 0.0;
			GLfloat MouseCursorPlotYCoordinate = 0.0;
			GLfloat PlotYAxesPixelTemperatureStep = 0.0;
			
			// Should the mouse cursor plot point data be rendered
			if (EnablePlotMouseCursorDataFlag == true) {

				// Check whether the mouse cursor is within the limits of the X axis
				if (MousePointerXPosition > _2DPlotLeftPixelPadding && MousePointerXPosition < PlotPanelWidth - _2DPlotRightPixelPadding) {
					
					// Check whether the mouse cursor is within the limits of the Y axis
					if (PlotPanelHeight - MousePointerYPosition >= _2DPlotBottomPixelPadding && PlotPanelHeight - MousePointerYPosition <= PlotPanelHeight - _2DPlotTopPixelPadding) {

						// Calculate the actual pixel height of the 2D plot
						PlotAreaPixelHeight = (PlotPanelHeight - _2DPlotBottomPixelPadding) - _2DPlotTopPixelPadding;
						// Calculate the pixel temperature step resolution value of the Y axis of the 2D plot
						PlotYAxesPixelTemperatureStep = (PlotYAxesMaximumRangeValue - PlotYAxesMinimumRangeValue) / PlotAreaPixelHeight;
						// Calculate the actual 2D plot Y-axis pixel coordinate of the mouse cursor
						MouseCursorPlotYCoordinate = (PlotPanelHeight - MousePointerYPosition) - _2DPlotBottomPixelPadding;
						// Calculate the temperature value of the current plot Y position of the mouse cursor 
						MouseCursorYTemperature = (MouseCursorPlotYCoordinate * PlotYAxesPixelTemperatureStep) + PlotYAxesMinimumRangeValue;

						// Convert the calculated temperature value to a string
						LabelString = "Temperature: " + MouseCursorYTemperature.ToString("F2") + " " + TempUnitString;
					
					}
					else {

						// Update the rendered string (outside the plot area)
						LabelString = "Temperature: N/A " + TempUnitString;

					}

				}
				else {

					// Update the rendered string (outside the plot area)
					LabelString = "Temperature: N/A " + TempUnitString;

				}

				// Render the mouse cursor plot point data label
				RMH_OpenGL_RenderStringOnTexture(MousePointerXPosition + MouseCursorDataLabelXOffset, MousePointerYPosition + MouseCursorDataLabelYOffset, RMH_Conversion_SystemStringToStdString(LabelString), MouseCursorDataLabelColorR, MouseCursorDataLabelColorG, MouseCursorDataLabelColorB);

			}

		}

		public: GLvoid RMH_OpenGL_Render2DPlot(unsigned int PlotPanelWidth, unsigned int PlotPanelHeight, System::String^ TempUnitString) {

			// This routine renders a 2-dimensional X/Y plot from the given input data

			// Read the current pixel height and width of the texture panel
			CurrentTexturePanelWidth = (GLfloat)PlotPanelWidth;
			CurrentTexturePanelHeight = (GLfloat)PlotPanelHeight;

			// Calculate the pixel offset between the texture area and the associated GUI panel
			TextureToPanelWidthOffset = OpenGLWindowWidth - CurrentTexturePanelWidth;
			TextureToPanelHeightOffset = OpenGLWindowHeight - CurrentTexturePanelHeight;

			// Make the associated render context the current render context
			RMH_OpenGL_MakeRenderContextCurrent();

			// Make sure the background color and the text font are set up in the render context that is used for drawing
			glClearColor(0.13725f, 0.13725f, 0.13725f, 1.0f);
			if (glIsList(BaseFont) == GL_FALSE && FontRebuildAttempts < 3) {
				FontRebuildAttempts++;
				RMH_Diag_Write(gcnew System::String("Font list missing at draw time - rebuilding the font, attempt ") + System::Convert::ToString((int)FontRebuildAttempts));
				RMH_OpenGL_BuildFont();
			}

			// Clear the texture color and bit buffers
			RMH_OpenGL_ClearTextureBuffer();
			// Update the texture field of view
			RMH_OpenGL_UpdateTextureFieldOfView(CurrentTexturePanelWidth, CurrentTexturePanelHeight);

			// Update the texture viewport to the center of the surface plot
			glViewport(0, TextureToPanelHeightOffset, CurrentTexturePanelWidth, CurrentTexturePanelHeight);

			// Rotate the texture to match the correct image orientation
			glTranslatef(0.0f, (GLfloat)CurrentTexturePanelHeight, 0.0f);
			glRotatef(180.0f, 1.0f, 0.0f, 0.0f);

			// ------------------------------------ Render 2D Plot ------------------------------------ //

			// Render the X and Y axis lines, ticks, grid lines, tick labels and axis descriptions of the 2D plot
			RMH_OpenGL_Render2DPlotBaseStructure(PlotPanelWidth, PlotPanelHeight, XAxesNumberOfTicks, YAxesNumberOfTicks, PlotYAxesMaximumRangeValue, PlotYAxesMinimumRangeValue, Show2DPlotBoxFlag, Show2DPlotGridFlag, "2D Temperature Measurements Plot", TempUnitString);

			// Plot the enabled data sets
			RMH_OpenGL_PlotDataSets();

			// Render the data logging indicator label
			RMH_OpenGL_RenderDataLoggingIndicatorLabelWithTimer(_2DPlotLeftPixelPadding, (_2DPlotTopPixelPadding / 2.0) + _2DPlotIndicatorLabelYOffset, LoggingTimerLabelMilliSecValue, DataLoggingFLag);

			// Render the mouse cursor plot point data as a label
			RMH_2DPlot_RenderPlotMouseCursorPointData(PlotPanelWidth, PlotPanelHeight, TempUnitString);

			// ---------------------------------------------------------------------------------------- //

			// Mark the end of an OpenGL rendering sequence
			RMH_OpenGL_RenderingFinishedMark();

		}

		// ------------ Texture Panel Interaction Cursor Event Callback Routines ----------- //

		private: GLvoid TexturePanel_MouseDown(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {

			// Update the overlay panel click flag
			OverlayPanelIsClick = true;

			// Check whether the left mouse button has been pressed
			if (e->Button == System::Windows::Forms::MouseButtons::Left) {

				// Toggle the rendering of the mouse cursor plot point data
				EnablePlotMouseCursorDataFlag = EnablePlotMouseCursorDataFlag ^ 1;

			}

		}

		private: GLvoid TexturePanel_MouseUp(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {

			// Update the overlay panel click flag
			OverlayPanelIsClick = false;

		}

		private: GLvoid TexturePanel_MouseMove(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {

			// Read the mouse cursor position
			MousePointerXPosition = e->X;
			MousePointerYPosition = e->Y;

			// If the panel has not yet been clicked
			if (OverlayPanelIsClick == false) {

				// Do not continue
				return;

			}

		}

		private: GLvoid TexturePanel_MouseWheel(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {



		}

		// -------------------- OpenGL Rendering Endpoint Routines -------------------- //

		private: GLvoid RMH_OpenGL_SwapOpenGLBuffers(GLvoid) {

			// This routine swaps the front/back buffers

			// Swap the buffers
			RMH_Diag_Frame();
			SwapBuffers(m_hDC);

		}

		private: GLvoid RMH_OpenGL_RenderingFinishedMark(GLvoid) {

			// This routine marks the end of an OpenGL rendering sequence
			// and must always be called last, when all object renderings have been executed

			// Swap the texture buffers
			RMH_OpenGL_SwapOpenGLBuffers();

		}

		// --------------------------------------------------------------------------------- //

		private:

		// ------------- Additional OpenGL Handling And Setup Routines ------------- //

		~RMHOpenGL2DPlot(GLvoid) {

			// Delete the OpenGL context
			DeleteOpenGL();

			// Destroy the OpenGL handler object
			this->DestroyHandle();

			// Garbage Collect managed data
			System::GC::Collect();

		}

		private: GLvoid DeleteOpenGL(GLvoid) {

			// This routine deletes the whole OpenGL context

			// Read the temporary array data and sort the kernel array
			HGLRC hglrc;
			HDC  hdc;

			// Read the thread context
			hglrc = wglGetCurrentContext();
			// Read the associated device context 
			hdc = wglGetCurrentDC();
			// Make the render context the current context
			wglMakeCurrent(NULL, NULL);
			// Release the device context
			ReleaseDC(NULL, hdc);
			// Delete the render context
			wglDeleteContext(hglrc);

			// Reset the context variables
			m_hglrc = nullptr;
			m_hDC = nullptr;

		}

		private: bool RMH_OpenGL_SetTexturePixelFormat(HDC hdc) {

			// This routine configures the pixel format of the texture

			// The format tells Windows how the texture data should be interpreted
			PIXELFORMATDESCRIPTOR pfd = {

				sizeof(PIXELFORMATDESCRIPTOR),				// Size of this pixel format descriptor
				1,											// Version number of the format 
				PFD_DRAW_TO_WINDOW |						// The format must support Windows
				PFD_SUPPORT_OPENGL |						// The format must support OpenGL
				PFD_DOUBLEBUFFER,							// The format must support "double buffering"
				PFD_TYPE_RGBA,								// Request an RGBA format
				16,										    // Select the "color depth" (16-bit)
				0, 0, 0, 0, 0, 0,							// Color bits are to be ignored
				0,											// No "alpha buffer"
				0,											// Shift bit is to be ignored
				0,											// No "accumulation buffer"
				0, 0, 0, 0,									// Accumulator bits are to be ignored
				16,											// 16-bit Z-buffer (buffer depth)  
				0,											// No "stencil buffer"
				0,											// No "auxiliary buffer"
				PFD_MAIN_PLANE,								// Set as the main "drawing" layer
				0,											// Reserved
				0, 0, 0										// Layer "masks" are to be ignored

			};

			// Select the pixel format for the control handler
			if ((iPixelFormat = ChoosePixelFormat(hdc, &pfd)) == 0) {
				// Write error message - if an error is registered
				MessageBox::Show("ChoosePixelFormat Failed");
				// Return error
				return false;
			}

			// Set the pixel format for the control handler 
			if (SetPixelFormat(hdc, iPixelFormat, &pfd) == FALSE) {
				// Write error message - if an error is registered
				MessageBox::Show("SetPixelFormat Failed");
				// Return error
				return false;
			}

			if ((m_hglrc = wglCreateContext(hdc)) == NULL) {
				// Write error message - if an error is registered
				MessageBox::Show("wglCreateContext Failed");
				// Return error
				return false;
			}

			if ((wglMakeCurrent(hdc, m_hglrc)) == NULL) {
				// Write error message - if an error is registered
				MessageBox::Show("wglMakeCurrent Failed");
				// Return error
				return false;
			}

			// Return status OK
			return true;
		}

		private: GLvoid RMH_OpenGL_ResizeOpenGLWinformsScene(unsigned int TotalTextureWidth, unsigned int TotalTextureHeight) {

			// Format the size and initialize the OpenGL window in WinForms

			// Prevent division by '0'
			if (TotalTextureHeight == 0) {
				// The pixel height is always at least '1'
				TotalTextureHeight = 1;
			}

			// Reset the current "viewport"
			glViewport(0, 0, TotalTextureWidth, TotalTextureHeight);
			// Select the projection matrix
			glMatrixMode(GL_PROJECTION);
			// Reset the projection matrix
			glLoadIdentity();
			// Calculate the aspect ratio of the window
			gluPerspective(60.0f, (GLfloat)TotalTextureWidth / (GLfloat)TotalTextureHeight, 0.01f, 10000.0f);
			// Select the "model view" matrix
			glMatrixMode(GL_MODELVIEW);
			// Reset the "model view" matrix
			glLoadIdentity();

		}

		private: GLvoid RMH_OpenGL_BuildFont(GLvoid) {

			// This routine generates the FONT for display in the OpenGL texture

			// Read the temporary array data and sort the kernel array
			HFONT TextureFont;

			// Update the font list
			BaseFont = glGenLists(96);

			// Generate the structured font object
			TextureFont = CreateFont(
				-12,                            // nHeight
				0,								// nWidth
				0,								// nEscapement
				0,				     			// nOrientation
				FW_BOLD,						// nWeight
				FALSE,							// bItalic
				FALSE,							// bUnderline
				FALSE,							// cStrikeOut
				ANSI_CHARSET,					// nCharSet
				OUT_TT_PRECIS,					// nOutPrecision
				CLIP_DEFAULT_PRECIS,			// nClipPrecision
				ANTIALIASED_QUALITY,			// nQuality
				FF_ROMAN | DEFAULT_PITCH,		// nPitchAndFamily
				L"Arial");				        // lpszFacename

			// Set the FONT to the OpenGL object structure
			SelectObject(m_hDC, TextureFont);
			// Generate the bitmap display FONT list
			wglUseFontBitmaps(m_hDC, 32, 96, BaseFont);

		}

		private: bool RMH_OpenGL_Init(GLvoid) {

			// This routine initializes OpenGL in WinForms C++/CLR

			// Enable "flat shader" mode
			glShadeModel(GL_SMOOTH);
			// Default background color
			glClearColor(0.13725f, 0.13725f, 0.13725f, 1.0f);
			// Set up the "depth buffer"
			glClearDepth(1.0f);
			// Disable OpenGL "depth testing"
			glDisable(GL_DEPTH_TEST);
			// For perspective - use "very nice" calculations
			glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

			// Generate the FONT object
			RMH_OpenGL_BuildFont();

			// Return the "OpenGL setup" finished flag
			return true;

		}

		// --------------------------------------------------------------------------------- //

	};

	// ------------------------------------------------------------------------------------- //

}