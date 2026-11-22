#pragma once

/*
 *  RMH_OpenGL_Winforms.h
 *
 *  Author: Rune Mark Hansen
 *  Date: January 2023
 *
 */

// Included libraries
#include "RMH_MathConversions_Library.h"
#include <windows.h>
#include <glew.h>
#include <glfw3.h>
#include <GL/GLU.h>
#include <GL/GL.h>
#include <iostream>
#include <math.h>
#include "RMH_Winforms_Library.h"

// Rendered object Z-order setting macros
#define _ZOrden_CrosshairsInFront	      0
#define _ZOrden_RectanglesInFront	      1
#define _ZOrden_LinesInFront			  2

// Global fixed configuration macros
#define _MaxNumberOfMovableRectangles     12   // 10x ROI + 1x Palette ROI + 1x Zoom ROI
#define _NumberOfNonMainLiveViewROIs      2    // 1x Palette ROI + 1x Zoom ROI
#define _MaxNumberOfMovableCrosshairs     10 
#define _MaxNumberOfMovableLines          5 
#define _MovableLinesMaxPixelLength       680 

// Static global class variables
static GLdouble RectX0[_MaxNumberOfMovableRectangles + 1];
static GLdouble RectY0[_MaxNumberOfMovableRectangles + 1];
static GLdouble RectWidth[_MaxNumberOfMovableRectangles + 1];
static GLdouble RectHeight[_MaxNumberOfMovableRectangles + 1];
static unsigned short RectOrderIndex[_MaxNumberOfMovableRectangles + 1];
static bool RectFixedAspectRatioFlags[_MaxNumberOfMovableRectangles + 1];
static GLdouble CrosshairX0[_MaxNumberOfMovableCrosshairs + 1];
static GLdouble CrosshairY0[_MaxNumberOfMovableCrosshairs + 1];
static unsigned short CrosshairOrderIndex[_MaxNumberOfMovableCrosshairs + 1];
static GLdouble MovableLineX0[_MaxNumberOfMovableLines + 1];
static GLdouble MovableLineY0[_MaxNumberOfMovableLines + 1];
static GLdouble MovableLineX1[_MaxNumberOfMovableLines + 1];
static GLdouble MovableLineY1[_MaxNumberOfMovableLines + 1];
static unsigned short MovableLineOrderIndex[_MaxNumberOfMovableLines + 1];

// Associated namespaces
using namespace System;
using namespace System::Windows::Forms;
using namespace std;

// Class position-adjustable rectangle enum
enum RectangelSizableSides {

	TopLine,
	LeftLine,
	RightLine,
	BottomLine,
	None

};

// Class position-adjustable crosshair enum
enum CrosshairSizableSides {

	Crosshair,
	Default

};

// Class position-adjustable line enum
enum LineMovableSides {

	LeftSide,
	Middle,
	RightSide,
	Outside

};

// OpenGL class definition
namespace OpenGLWinForms {

	// ---------------------------------- Global Class Structure Objects --------------------------------- //

	// Rectangle position data class structure
	class RectangelPosition {
	public:

		// Rectangle position parameters
		GLdouble RectangleX0Pos = 0.0;
		GLdouble RectangleY0Pos = 0.0;
		GLdouble RectangleWidth = 0.0;
		GLdouble RectangleHeight = 0.0;

	};

	// Crosshair with label position data class structure
	class CrosshairWLabelPosition {
	public:

		// Crosshair position parameters
		GLfloat CrosshairX0Pos = 0.0;
		GLfloat CrosshairY0Pos = 0.0;

	};

	// Mouse cursor position data class structure
	class MouseCursorPosition {
	public:

		// Mouse cursor position parameters
		GLdouble CursorXPos = 0.0;
		GLdouble CursorYPos = 0.0;

	};

	// Line specification data class structure
	class LineSpecsPosition {
	public:

		// Line specification parameters
		unsigned short LineType = 0;
		GLfloat LineSlope = 0.0;
		unsigned short LinePixelLength = 0;
		GLfloat LineXOffset = 0.0;
		GLfloat LineYOffset = 0.0;
		GLfloat LineX0Pos = 0.0;
		GLfloat LineY0Pos = 0.0;
		GLfloat LineX1Pos = 0.0;
		GLfloat LineY1Pos = 0.0;
		GLfloat LineXCordinates[_MovableLinesMaxPixelLength];
		GLfloat LineYCordinates[_MovableLinesMaxPixelLength];

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

	// --------------------------------------- Main OpenGL Class --------------------------------------- //
	
	public ref class RMHOpenGLWF : public System::Windows::Forms::NativeWindow {

	private:

		// OpenGL & texture rendering variables
		private: HDC m_hDC;
		private: HGLRC m_hglrc;
		private: GLuint BaseFont;
		private: unsigned int TextureWidth;
		private: unsigned int TextureHeight;
		private: GLint iPixelFormat;
		private: GLuint* ImageTecture;
		private: GLdouble LiveViewPosX0 = 0.0;
		private: GLdouble LiveViewPosY0 = 0.0;
		private: GLdouble LiveViewPosX1 = 0.0;
		private: GLdouble LiveViewPosY1 = 0.0;
		private: GLdouble InitialTextureWidth;
		private: GLdouble InitialTextureHeight;
		private: GLdouble TextureResScaleFactor;
		private: GLdouble AspectRatioWidthOffSet;
		private: GLdouble AspectRatioHeightOffSet;
		private: GLdouble CurrentTexturePanelWidth;
		private: GLdouble CurrentTexturePanelHeight;
		private: GLdouble TotalTextureScalableWidth;
		private: GLdouble TotalTextureScalableHeight;
		private: GLdouble CurrentPanelWidthFixedAspect;
		private: GLdouble CurrentPanelHeightFixedAspect;
		private: CreateParams^ ControlParams = gcnew CreateParams;
		private: GLdouble ImageDataPixelWidth;
		private: GLdouble ImageDataPixelHeight;
		private: GLdouble ImageDataPixelAspectRatio;
		private: GLdouble ImageDataPixelAspectRatioReciprok;
		private: TextureOverlayPanel^ OverlayPanel = gcnew TextureOverlayPanel();
		private: bool OverlayPanelIsClick = false;
		private: bool LocalAspectRatioFlag = false;
		private: bool ClassEnableInterpolationFlag = false;
		private: unsigned short TextureRotation = 0;
		private: unsigned short MovableObjectsRenderZOrder = _ZOrden_RectanglesInFront;
		private: GLdouble LiveViewRotationDegrees = 0.0;
		private: bool LiveViewMouseScrollWheelRotationEnablFlag = false;
		private: bool LiveViewRotationChangedFlag = false;
		private: GLfloat UltraResolutionFrameOffsetValue = 0.5;
		private: unsigned int UltraResolutionTextureScaleFactor = 2;
			   
		// Mouse cursor tracking variables
		private: bool CursorTrackingEnableFlag = false;
		private: GLdouble CursorTrackTextureXPos = 0.0f;
		private: GLdouble CursorTrackTextureYPos = 0.0f;
		private: GLdouble CursorTrackXPos = 0.0f;
		private: GLdouble CursorTrackYPos = 0.0f;
		private: GLfloat MouseLabelQuadrant1LabelXOffset = -17.0;
		private: GLfloat MouseLabelQuadrant1LabelYOffset = 3.0;
		private: GLfloat MouseLabelQuadrant4LabelXOffset = -17.0;
		private: GLfloat MouseLabelQuadrant4LabelYOffset = -2.0;

		// Position-adjustable rectangle variables
		private: RectangelSizableSides SelectedRectSide = RectangelSizableSides::None;
		private: GLdouble ClickRectXPositionOffset;
		private: GLdouble ClickRectYPositionOffset;
		private: GLdouble ClickRectXPosition;
		private: GLdouble ClickRectYPosition;
		private: GLdouble ClickRectWidthPosition;
		private: GLdouble ClickRectHeightPosition;
		private: unsigned short SelectedRectTagIndex = 0;
		private: unsigned short NmbOfActiveRects = 0;
		private: unsigned short OldNmbOfActiveRects = 0;
		private: unsigned int RenderedRectanglesCounter = 0;
		private: bool RectangleMoveFlag = false;

		// Position-adjustable crosshair with label variables
		private: CrosshairSizableSides SelectedCrosshairSide = CrosshairSizableSides::Default;
		private: GLdouble ClickCrosshairXPositionOffset;
		private: GLdouble ClickCrosshairYPositionOffset;
		private: unsigned int RenderedCrosshairCounter = 0;
		private: unsigned short SelectedCrosshairTagIndex = 0;
		private: unsigned short NmbOfActiveCrosshairs = 0;
		private: unsigned short OldNmbOfActiveCrosshairs = 0;
		private: bool CrosshairMoveFlag = false;

		// Position-adjustable line variables
		private: LineMovableSides SelectedLineSide = LineMovableSides::Outside;
		private: GLdouble ClickLineX0PositionOffset;
		private: GLdouble ClickLineY0PositionOffset;
		private: GLdouble ClickLineX1PositionOffset;
		private: GLdouble ClickLineY1PositionOffset;
		private: unsigned int RenderedLineCounter = 0;
		private: unsigned short SelectedLineTagIndex = 0;
		private: unsigned short NmbOfActiveLines = 0;
		private: unsigned short OldNmbOfActiveLines = 0;
		private: bool LineMoveFlag = false;
	
		// Default position-adjustable rectangle configuration parameters
		public: GLdouble MinimumRectHeight = 20.0;
		public: GLdouble MinimumRectWidth = 20.0;
		private: GLdouble InitialRectHeight = 0.0;
		private: GLdouble InitialRectWidth = 48.0;
		public: GLdouble DefaultRectLineWidth = 1.0;
		private: GLdouble CursorChangeOffset = 2.0;
		private: GLdouble RectTextureBorderPadding = 5.0;
		private: GLdouble ROIIdentifierLabelXPixelOffset = 1.0;
		private: GLdouble ROIIdentifierLabelYPixelOffset = 2.0;
		private: GLdouble ROIRectangleLiveViewBorderPixelPadding = 2.0;

		// Default position-adjustable crosshair configuration parameters
		private: GLfloat CrosshairSize = 2.0;
		private: GLfloat CrosshairInsideAreaPadding = 5.0;
		private: unsigned short CrosshairLineWidth = 2;
		private: GLfloat MovableLineQuadrant1LabelXOffset = -10.0;
		private: GLfloat MovableLineQuadrant1LabelYOffset = 3.0;
		private: GLfloat MovableLineQuadrant2LabelXOffset = 2.0;
		private: GLfloat MovableLineQuadrant2LabelYOffset = 3.0;
		private: GLfloat MovableLineQuadrant3LabelXOffset = 2.0;
		private: GLfloat MovableLineQuadrant3LabelYOffset = -2.0;
		private: GLfloat MovableLineQuadrant4LabelXOffset = -10.0;
		private: GLfloat MovableLineQuadrant4LabelYOffset = -2.0;

		// Default crosshair with center label configuration parameters
		private: GLfloat ChrosshairWithCenterLabelXOffset = -4.0;
		private: GLfloat ChrosshairWithCenterLabelYOffset = 2.8;

		// Default position-adjustable line configuration parameters
		private: unsigned char MovableLineCursorOffset = 4;
		private: unsigned short MovableLineCurnerToMiddleOffset = 4;
		private: unsigned short MovableLineMinimumLength = 10;
		private: unsigned short LineTextureBorderPadding = 3;

		// Text label and label background configuration parameters
		private: bool EnableLabelBackgroundFlag = true;
		private: GLfloat LabelBackgroundXOffset = 0.2;
		private: GLfloat LabelBackgroundYOffset = 1.1;
		private: GLfloat LabelBackgroundWidth = 8.5;
		private: GLfloat LabelBackgroundHeight = 1.5;
		private: GLfloat CenterLabelBackgroundXOffset = 0.3;
		private: GLfloat CenterLabelBackgroundYOffset = 1.1;
		private: GLfloat CenterLabelBackgroundWidth = 9.8;
		private: GLfloat CenterLabelBackgroundHeight = 1.5;
		private: GLfloat MouseLabelBackgroundXOffset = 0.3;
		private: GLfloat MouseLabelBackgroundYOffset = 1.1;
		private: GLfloat MouseLabelBackgroundWidth = 16.2;
		private: GLfloat MouseLabelBackgroundHeight = 1.5;
		private: GLubyte LabelBackgroundColorR = 35;
		private: GLubyte LabelBackgroundColorG = 35;
		private: GLubyte LabelBackgroundColorB = 35;
		private: GLubyte CommonLabelColorR = 255;
		private: GLubyte CommonLabelColorG = 255;
		private: GLubyte CommonLabelColorB = 255;
		private: GLubyte CommonLabelBackgroundAlpha = 180;

	public:

		// ------------------------------- OpenGL Texture Object Constructor Routines ------------------------------- //

		RMHOpenGLWF(System::Windows::Forms::Panel^ TexturePanel, unsigned char ResolutionScaleFactor) {

			// This routine sets up an OpenGL-supported graphics area for rendering
			// A WinForms panel is given as the physical texture area.

			// Set the initial texture parameters
			InitialTextureWidth = (GLdouble)TexturePanel->Width;
			InitialTextureHeight = (GLdouble)TexturePanel->Height;
			TextureResScaleFactor = (GLdouble)ResolutionScaleFactor;

			// Calculate the total scalable texture height and width
			TotalTextureScalableWidth = 1.0 / (InitialTextureWidth * TextureResScaleFactor);
			TotalTextureScalableHeight = 1.0 / (InitialTextureHeight * TextureResScaleFactor);

			// Set the position of the form
			ControlParams->X = 0;
			ControlParams->Y = 0;
			ControlParams->Width = InitialTextureWidth * TextureResScaleFactor;
			ControlParams->Height = InitialTextureHeight * TextureResScaleFactor;

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
				// Initialize OpenGL for WinForms C++
				RMH_OpenGL_Init();

			}

			// Add an overlaid transparent panel to the texture panel
			RMH_OpenGL_AddOverlayPanelToMainTexturePanel(TexturePanel);

		}

		// ---------------- Position Access For Saved Sessions (Session Menu) ---------------- //

		// Reads the position of a position-adjustable object into "Values" (4 elements). ObjectType 0 = temperature measurement crosshair (X, Y), 1 = temperature line
		// (X0, Y0, X1, Y1), 2 = ROI rectangle (X, Y, width, height). "Tag" is the order priority that the object is rendered with (the same number that is given to
		// RMH_OpenGL_RenderMovableCrossHairWithLabel / RMH_OpenGL_RenderMovableLine / RMH_OpenGL_RenderMovableRectangle). Returns false for a wrong type or tag.
		bool RMH_OpenGL_GetMovableObjectPosition(int ObjectType, int Tag, cli::array<double>^ Values) {

			if (Values == nullptr || Values->Length < 4) { return false; }

			if (ObjectType == 0 && Tag >= 1 && Tag <= _MaxNumberOfMovableCrosshairs) {
				Values[0] = CrosshairX0[Tag]; Values[1] = CrosshairY0[Tag]; Values[2] = 0.0; Values[3] = 0.0;
				return true;
			}
			if (ObjectType == 1 && Tag >= 1 && Tag <= _MaxNumberOfMovableLines) {
				Values[0] = MovableLineX0[Tag]; Values[1] = MovableLineY0[Tag]; Values[2] = MovableLineX1[Tag]; Values[3] = MovableLineY1[Tag];
				return true;
			}
			if (ObjectType == 2 && Tag >= 1 && Tag <= _MaxNumberOfMovableRectangles) {
				Values[0] = RectX0[Tag]; Values[1] = RectY0[Tag]; Values[2] = RectWidth[Tag]; Values[3] = RectHeight[Tag];
				return true;
			}

			return false;

		}

		// Sets the position of a position-adjustable object (see RMH_OpenGL_GetMovableObjectPosition). The values are kept inside the image (a session file can
		// come from a camera with another image size). Returns false for a wrong type or tag, or for values that are not numbers.
		bool RMH_OpenGL_SetMovableObjectPosition(int ObjectType, int Tag, cli::array<double>^ Values) {

			if (Values == nullptr || Values->Length < 4) { return false; }

			for (int i = 0; i < 4; i++) { if (System::Double::IsNaN(Values[i]) || System::Double::IsInfinity(Values[i])) { return false; } }

			double MaxX = (ImageDataPixelWidth > 2.0) ? (double)ImageDataPixelWidth : 1.0e9;
			double MaxY = (ImageDataPixelHeight > 2.0) ? (double)ImageDataPixelHeight : 1.0e9;

			if (ObjectType == 0 && Tag >= 1 && Tag <= _MaxNumberOfMovableCrosshairs) {
				CrosshairX0[Tag] = System::Math::Min(System::Math::Max(Values[0], 1.0), MaxX);
				CrosshairY0[Tag] = System::Math::Min(System::Math::Max(Values[1], 1.0), MaxY);
				return true;
			}
			if (ObjectType == 1 && Tag >= 1 && Tag <= _MaxNumberOfMovableLines) {
				MovableLineX0[Tag] = System::Math::Min(System::Math::Max(Values[0], 1.0), MaxX);
				MovableLineY0[Tag] = System::Math::Min(System::Math::Max(Values[1], 1.0), MaxY);
				MovableLineX1[Tag] = System::Math::Min(System::Math::Max(Values[2], 1.0), MaxX);
				MovableLineY1[Tag] = System::Math::Min(System::Math::Max(Values[3], 1.0), MaxY);
				return true;
			}
			if (ObjectType == 2 && Tag >= 1 && Tag <= _MaxNumberOfMovableRectangles) {
				double NewWidth = System::Math::Min(System::Math::Max(Values[2], 2.0), MaxX - 1.0);
				double NewHeight = System::Math::Min(System::Math::Max(Values[3], 2.0), MaxY - 1.0);
				RectX0[Tag] = System::Math::Min(System::Math::Max(Values[0], 0.0), MaxX - NewWidth);
				RectY0[Tag] = System::Math::Min(System::Math::Max(Values[1], 0.0), MaxY - NewHeight);
				RectWidth[Tag] = NewWidth;
				RectHeight[Tag] = NewHeight;
				return true;
			}

			return false;

		}

		GLvoid RMH_OpenGL_InitArray(GLdouble *Array, unsigned short ArraySize, GLdouble Value) {

			// This routine writes a given value "value" to all positions of a given array

			// Loop through the whole array
			for (unsigned short i = 0; i < ArraySize; i++) {

				// Write the value to the array index positions
				*(Array + i) = Value;

			}

		}

		GLvoid RMH_OpenGL_InitArray(bool *Array, unsigned short ArraySize, bool Value) {

			// This routine writes a given value "value" to all positions of a given array

			// Loop through the whole array
			for (unsigned short i = 0; i < ArraySize; i++) {

				// Write the value to the array index positions
				*(Array + i) = Value;

			}

		}

		GLvoid RMH_OpenGL_InitArray(unsigned short *Array, unsigned short ArraySize, unsigned short Value) {

			// This routine writes a given value "value" to all positions of a given array

			// Loop through the whole array
			for (unsigned short i = 0; i < ArraySize; i++) {

				// Write the value to the array index positions
				*(Array + i) = Value;

			}

		}

		GLvoid RMH_OpenGL_InitializeGlobalVariabelsAndArrays(unsigned int FrameWidth, unsigned int FrameHeight) {

			// This routine initializes various class arrays and variables with start values

			// Local variables
			GLdouble CenterTextureWidth = (GLdouble)FrameWidth * 0.5;
			GLdouble CenterTextureHeight = (GLdouble)FrameHeight * 0.5;

			// Calculate the default start pixel height of the rectangles
			InitialRectHeight = InitialRectWidth * (1.0 / ((GLdouble)FrameWidth / (GLdouble)FrameHeight));

			// Initialize global arrays with start values - position-adjustable rectangles
			RMH_OpenGL_InitArray(&RectX0[0], _MaxNumberOfMovableRectangles + 1, 10);
			RMH_OpenGL_InitArray(&RectY0[0], _MaxNumberOfMovableRectangles + 1, 10);
			RMH_OpenGL_InitArray(&RectWidth[0], _MaxNumberOfMovableRectangles + 1, InitialRectWidth);
			RMH_OpenGL_InitArray(&RectHeight[0], _MaxNumberOfMovableRectangles + 1, InitialRectHeight);
			RMH_OpenGL_InitArray(&RectOrderIndex[0], _MaxNumberOfMovableRectangles + 1, 0);
			RMH_OpenGL_InitArray(&RectFixedAspectRatioFlags[0], _MaxNumberOfMovableRectangles + 1, false);

			// Initialize global arrays with start values - position-adjustable crosshair with label
			RMH_OpenGL_InitArray(&CrosshairX0[0], _MaxNumberOfMovableCrosshairs + 1, CenterTextureWidth);
			RMH_OpenGL_InitArray(&CrosshairY0[0], _MaxNumberOfMovableCrosshairs + 1, CenterTextureHeight);
			RMH_OpenGL_InitArray(&CrosshairOrderIndex[0], _MaxNumberOfMovableCrosshairs + 1, 0);

			// Initialize global arrays with start values - position-adjustable line
			RMH_OpenGL_InitArray(&MovableLineX0[0], _MaxNumberOfMovableLines + 1, CenterTextureWidth - 50);
			RMH_OpenGL_InitArray(&MovableLineY0[0], _MaxNumberOfMovableLines + 1, CenterTextureHeight);
			RMH_OpenGL_InitArray(&MovableLineX1[0], _MaxNumberOfMovableLines + 1, CenterTextureWidth + 50);
			RMH_OpenGL_InitArray(&MovableLineY1[0], _MaxNumberOfMovableLines + 1, CenterTextureHeight);
			RMH_OpenGL_InitArray(&MovableLineOrderIndex[0], _MaxNumberOfMovableLines + 1, 0);

		}

		// ----------------------- Texture Overlaid Transparent Panel Setup Routine ------------------------ //

		GLvoid RMH_OpenGL_AddOverlayPanelToMainTexturePanel(System::Windows::Forms::Panel^ TexturePanel) {

			// This routine adds an overlaid transparent panel to the texture panel
			// This overlaid panel is used to manipulate objects on the image texture

			// The overlaid panel must not have any margin or padding
			OverlayPanel->Margin = System::Windows::Forms::Padding(0, 0, 0, 0);
			OverlayPanel->Padding = System::Windows::Forms::Padding(0, 0, 0, 0);

			// The overlaid transparent panel must fill the whole texture panel
			OverlayPanel->Dock = System::Windows::Forms::DockStyle::Fill;

			// Add the overlaid transparent panel as a "child" of the texture panel
			TexturePanel->Controls->Add(OverlayPanel);

			// Enable mouse handler events for the transparent panel
			OverlayPanel->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &RMHOpenGLWF::TexturePanel_MouseDown);
			OverlayPanel->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &RMHOpenGLWF::TexturePanel_MouseUp);
			OverlayPanel->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &RMHOpenGLWF::TexturePanel_MouseMove);
			OverlayPanel->MouseWheel += gcnew System::Windows::Forms::MouseEventHandler(this, &RMHOpenGLWF::TexturePanel_MouseWheel);

		}

		// ---------------------------- Texture Panel To Texture Conversion Routines ----------------------------- //

		GLdouble RMH_OpenGL_TranslateOverlayPanelMouseXPosToTextureXPos(System::Windows::Forms::MouseEventArgs^ OverlayPanelMouseEvent) {

			// This routine translates the overlaid panel mouse positions to the actual texture panel mouse positions

			// Local variables
			GLdouble MouseTextureXPos = 0.0;
			GLdouble PanelsWidthDifference = 0.0;

			// Calculate the pixel difference between the overlaid panel and the texture panel 
			PanelsWidthDifference = CurrentTexturePanelWidth - OverlayPanel->Width;

			// Check the selected aspect ratio setting
			if (LocalAspectRatioFlag == true) {

				// If horizontal aspect ratio compensation is needed
				if (LiveViewPosX0 <= 0.0) {

					// Convert the overlaid panel mouse position to the actual texture panel mouse position
					MouseTextureXPos = (OverlayPanelMouseEvent->X + PanelsWidthDifference) * (ImageDataPixelWidth / CurrentTexturePanelWidth);
					
				}
				else {

					// Convert the overlaid panel mouse position to the actual texture panel mouse position - fixed aspect ratio mode
					MouseTextureXPos = ImageDataPixelWidth * ((GLdouble)OverlayPanelMouseEvent->X + PanelsWidthDifference - 0.5f * CurrentTexturePanelWidth + 0.5f * CurrentPanelWidthFixedAspect) / CurrentPanelWidthFixedAspect;
					
				}

			}
			else {

				// Convert the overlaid panel mouse position to the actual texture panel mouse position
				MouseTextureXPos = (OverlayPanelMouseEvent->X + PanelsWidthDifference) * (ImageDataPixelWidth / CurrentTexturePanelWidth);

			}

			// Handling at the minimum texture mouse position 
			if (MouseTextureXPos <= 0) {
				// Set the mouse position to the minimum value
				MouseTextureXPos = 0;
			}

			// Handling at the maximum texture mouse position 
			if (MouseTextureXPos >= ImageDataPixelWidth) {
				// Set the mouse position to the maximum value
				MouseTextureXPos = ImageDataPixelWidth;
			}
			
			// Round the mouse position up to the nearest integer
			MouseTextureXPos = RMH_Math_Round(MouseTextureXPos);

			// Return the current texture panel mouse position
			return MouseTextureXPos;

		}

		GLdouble RMH_OpenGL_TranslateOverlayPanelMouseYPosToTextureYPos(System::Windows::Forms::MouseEventArgs^ OverlayPanelMouseEvent) {

			// This routine translates the overlaid panel mouse positions to the actual texture panel mouse positions

			// Local variables
			GLdouble MouseTextureYPos = 0.0;
			GLdouble PanelsHeightDifference = 0.0;

			// Calculate the pixel difference between the overlaid panel and the texture panel 
			PanelsHeightDifference = CurrentTexturePanelHeight - OverlayPanel->Height;

			// Check the selected aspect ratio setting
			if (LocalAspectRatioFlag == true) {

				// If horizontal aspect ratio compensation is needed
				if (LiveViewPosX0 <= 0.0) {

					// Convert the overlaid panel mouse position to the actual texture panel mouse position - fixed aspect ratio mode
					MouseTextureYPos = ImageDataPixelHeight * ((GLdouble)OverlayPanelMouseEvent->Y + PanelsHeightDifference - 0.5f * CurrentTexturePanelHeight + 0.5f * CurrentPanelHeightFixedAspect) / CurrentPanelHeightFixedAspect;

				}
				else {

					// Convert the overlaid panel mouse position to the actual texture panel mouse position
					MouseTextureYPos = (OverlayPanelMouseEvent->Y + PanelsHeightDifference) * (ImageDataPixelHeight / CurrentTexturePanelHeight);

				}

			}
			else {

				// Convert the overlaid panel mouse position to the actual texture panel mouse position
				MouseTextureYPos = (OverlayPanelMouseEvent->Y + PanelsHeightDifference) * (ImageDataPixelHeight / CurrentTexturePanelHeight);

			}

			// Handling at the minimum texture mouse position 
			if (MouseTextureYPos <= 0) {
				// Set the mouse position to the minimum value
				MouseTextureYPos = 0;
			}

			// Handling at the maximum texture mouse position 
			if (MouseTextureYPos >= ImageDataPixelHeight) {
				// Set the mouse position to the maximum value
				MouseTextureYPos = ImageDataPixelHeight;
			}

			// Round the mouse position up to the nearest integer
			MouseTextureYPos = RMH_Math_Round(MouseTextureYPos);

			// Return the current texture panel mouse position
			return MouseTextureYPos;

		}

		// -------------------------- Image Texture Generation And Rendering Routines -------------------------- //

		GLvoid RMH_OpenGL_MakeRenderContextCurrent() {

			// This routine makes the associated render context the current render context

			// Make the associated render context the current render context
			wglMakeCurrent(m_hDC, m_hglrc);

		}

		GLvoid RMH_OpenGL_MakeRenderContextNULL() {

			// This routine resets the associated render context

			// Reset the render context
			wglMakeCurrent(NULL, NULL);

		}

		GLvoid RMH_OpenGL_EnableTextureLinearInterpolation(bool EnableInterpolationFlag) {

			// This routine enables or disables linear texture interpolation

			// Update the global class flag
			ClassEnableInterpolationFlag = EnableInterpolationFlag;

			// Make the associated render context the current render context
			RMH_OpenGL_MakeRenderContextCurrent();

			// Enable OpenGL 2D texture
			glEnable(GL_TEXTURE_2D);

			// Bind the texture as a 2D texture
			glBindTexture(GL_TEXTURE_2D, ImageTecture[0]);

			// Should linear interpolation be enabled
			if (EnableInterpolationFlag == true) {

				// Configure the texture parameters
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

			}
			else {

				// Configure the texture parameters
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

			}

			// Disable the texture
			glDisable(GL_TEXTURE_2D);

		}

		GLvoid RMH_OpenGL_SetGraphicsElementsPixelParameters(unsigned int FrameWidth, unsigned int FrameHeight) {

			// This routine sets the pixel size of the graphical elements and objects of the class

			// Reference resolution the values were dimensioned for ->
			GLfloat ReferenceFrameWidth = 384.0;
			GLfloat ReferenceFrameHeight = 288.0;
			GLfloat RefFrameWidthToCurrentRatio = (FrameWidth / ReferenceFrameWidth);
			GLfloat RefFrameHeightToCurrentRatio = (FrameHeight / ReferenceFrameHeight);

			// Reset and update the default position-adjustable rectangle configuration parameters
			MinimumRectHeight = 20.0 * RefFrameHeightToCurrentRatio;
			MinimumRectWidth = 20.0 * RefFrameWidthToCurrentRatio;
			InitialRectWidth = 48.0 * ((RefFrameWidthToCurrentRatio + RefFrameHeightToCurrentRatio) * 0.5);
			DefaultRectLineWidth = 1.0 * RefFrameWidthToCurrentRatio;
			CursorChangeOffset = 2.0 * ((RefFrameWidthToCurrentRatio + RefFrameHeightToCurrentRatio) * 0.5);
			RectTextureBorderPadding = 5.0 * ((RefFrameWidthToCurrentRatio + RefFrameHeightToCurrentRatio) * 0.5);
			ROIIdentifierLabelXPixelOffset = 1.0 * RefFrameWidthToCurrentRatio;
			ROIIdentifierLabelYPixelOffset = 2.0 * RefFrameHeightToCurrentRatio;

			// Reset and update the default position-adjustable crosshair configuration parameters
			CrosshairSize = 2.0 * ((RefFrameWidthToCurrentRatio + RefFrameHeightToCurrentRatio) * 0.5); // Crosshair size
			CrosshairLineWidth = 2.0; // Line thickness - 2 for all resolutions
			MovableLineQuadrant1LabelXOffset = -10.0 * RefFrameWidthToCurrentRatio;
			MovableLineQuadrant1LabelYOffset = 3.0 * RefFrameHeightToCurrentRatio;
			MovableLineQuadrant2LabelXOffset = 2.0 * RefFrameWidthToCurrentRatio;
			MovableLineQuadrant2LabelYOffset = 3.0 * RefFrameHeightToCurrentRatio;
			MovableLineQuadrant3LabelXOffset = 2.0 * RefFrameWidthToCurrentRatio;
			MovableLineQuadrant3LabelYOffset = -2.0 * RefFrameHeightToCurrentRatio;
			MovableLineQuadrant4LabelXOffset = -10.0 * RefFrameWidthToCurrentRatio;
			MovableLineQuadrant4LabelYOffset = -2.0 * RefFrameHeightToCurrentRatio;

			// Reset and update the default crosshair with center label configuration parameters
			ChrosshairWithCenterLabelXOffset = -4.0 * RefFrameWidthToCurrentRatio;
			ChrosshairWithCenterLabelYOffset = 2.8 * RefFrameHeightToCurrentRatio;

			// Reset and update the default position-adjustable line configuration parameters
			MovableLineCursorOffset = 4.0 * ((RefFrameWidthToCurrentRatio + RefFrameHeightToCurrentRatio) * 0.5);
			MovableLineCurnerToMiddleOffset = 4.0 * ((RefFrameWidthToCurrentRatio + RefFrameHeightToCurrentRatio) * 0.5);
			MovableLineMinimumLength = 10.0 * ((RefFrameWidthToCurrentRatio + RefFrameHeightToCurrentRatio) * 0.5);
			LineTextureBorderPadding = 3.0 * ((RefFrameWidthToCurrentRatio + RefFrameHeightToCurrentRatio) * 0.5);

			// Reset and update the default text label background configuration parameters
			LabelBackgroundXOffset = 0.2 * RefFrameWidthToCurrentRatio;
			LabelBackgroundYOffset = 1.1 * RefFrameHeightToCurrentRatio;
			LabelBackgroundWidth = 8.5 * RefFrameWidthToCurrentRatio;
			LabelBackgroundHeight = 1.5 * RefFrameHeightToCurrentRatio;

			// Reset and update the default center text label background configuration parameters
			CenterLabelBackgroundXOffset = 0.3 * RefFrameWidthToCurrentRatio;
			CenterLabelBackgroundYOffset = 1.1 * RefFrameHeightToCurrentRatio;
			CenterLabelBackgroundWidth = 9.8 * RefFrameWidthToCurrentRatio;
			CenterLabelBackgroundHeight = 1.5 * RefFrameHeightToCurrentRatio;

			// Reset and update the default mouse text label background configuration parameters
			MouseLabelBackgroundXOffset = 0.3 * RefFrameWidthToCurrentRatio;
			MouseLabelBackgroundYOffset = 1.1 * RefFrameHeightToCurrentRatio;
			MouseLabelBackgroundWidth = 16.2 * RefFrameWidthToCurrentRatio;
			MouseLabelBackgroundHeight = 1.5 * RefFrameHeightToCurrentRatio;
			MouseLabelQuadrant1LabelXOffset = -17.0 * RefFrameWidthToCurrentRatio;
			MouseLabelQuadrant1LabelYOffset = 3.0 * RefFrameHeightToCurrentRatio;
			MouseLabelQuadrant4LabelXOffset = -17.0 * RefFrameWidthToCurrentRatio;
			MouseLabelQuadrant4LabelYOffset = -2.0 * RefFrameHeightToCurrentRatio;

		}

		GLvoid RMH_OpenGL_InitImageTexture(unsigned int FrameWidth, unsigned int FrameHeight) {

			// This routine is used to set up an OpenGL texture for graphics rendering

			// Set the texture parameters - width and height must be a multiple of 2
			TextureWidth = FrameWidth;
			TextureHeight = FrameHeight;
			ImageTecture = new GLuint[1];

			// Make the associated render context the current render context
			RMH_OpenGL_MakeRenderContextCurrent();

			// Set the pixel size of the graphical elements and objects of the class
			RMH_OpenGL_SetGraphicsElementsPixelParameters(FrameWidth, FrameHeight);

			// Initialize class arrays and variables with start values
			RMH_OpenGL_InitializeGlobalVariabelsAndArrays(FrameWidth, FrameHeight);

			// Build the texture that renders the image data
			glGenTextures(1, ImageTecture);

			// Enable OpenGL 2D texture
			glEnable(GL_TEXTURE_2D);

			// Enable texture blending
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Bind the texture as a 2D texture
			glBindTexture(GL_TEXTURE_2D, ImageTecture[0]);

			// Configure the texture parameters (FrameWidth * UltraResolutionTextureScaleFactor, FrameHeight * UltraResolutionTextureScaleFactor - ultra resolution)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, FrameWidth * UltraResolutionTextureScaleFactor, FrameHeight * UltraResolutionTextureScaleFactor, 0, GL_RGB, GL_UNSIGNED_SHORT, NULL);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

			// Is linear interpolation enabled
			if (ClassEnableInterpolationFlag == true) {

				// Set the interpolation method - linear
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			}
			else {

				// Set the interpolation method - nearest
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			}
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

			// Disable the texture
			glDisable(GL_TEXTURE_2D);

		}

		GLvoid RMH_OpenGL_WriteImageDataToTexture(unsigned short* FrameData, unsigned int FrameWidth, unsigned int FrameHeight) {

			// This routine writes image data to the generated texture
			
			// Update the texture data with the image data
			glBindTexture(GL_TEXTURE_2D, ImageTecture[0]);
			
			// Upload the image data to the texture
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FrameWidth, FrameHeight, GL_RGB, GL_UNSIGNED_SHORT, FrameData);

		}

		GLvoid RMH_OpenGL_UpdateTextureFieldOfView(unsigned int FrameWidth, unsigned int FrameHeight) {

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
			gluPerspective(PlaneFieldOfView, PlaneAspectRatio, 0.1, 500.0);
			gluLookAt(PlaneXLook, PlaneYLook, PlaneDistance * 0.5, PlaneXLook, PlaneYLook, 0, 0, 1, 0);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

		}
		
		// ------------------------------ Label Rendering And Handling Routines ------------------------------- //

		GLvoid RMH_OpenGL_glPrint(const char* CharArray) {

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

		GLvoid RMH_OpenGL_RenderStringOnTexture(GLfloat StringX, GLfloat StringY, std::string DisplayString, GLubyte ColorR, GLubyte ColorG, GLubyte ColorB) {

			// This routine renders a given string on an OpenGL texture

			// Configure the text color
			glColor3ub(ColorR, ColorG, ColorB);

			// Set the position of the text on the texture
			glRasterPos2f(StringX, StringY);

			// Render the given string on the texture
			RMH_OpenGL_glPrint(DisplayString.c_str());

		}

		// ------------------------------ Line Rendering And Handling Routines ------------------------------- //

		GLvoid RMH_OpenGL_RenderLine(GLfloat LineX0, GLfloat LineY0, GLfloat LineX1, GLfloat LineY1, GLfloat LineWidth, GLubyte ColorR, GLubyte ColorG, GLubyte ColorB) {

			// This routine renders a line on the texture with the given input coordinates

			// Check the selected aspect ratio setting
			if (LocalAspectRatioFlag == true) {

				// If horizontal aspect ratio compensation is needed
				if (LiveViewPosX0 <= 0.0) {

					// Calculate the X0/X1 coordinates of the line when scaling the texture window - auto aspect ratio mode
					LineX0 = ((GLfloat)LineX0 * CurrentTexturePanelWidth * TotalTextureScalableWidth);
					LineX1 = ((GLfloat)LineX1 * CurrentTexturePanelWidth * TotalTextureScalableWidth);

					// Calculate the X0/X1 coordinates of the line when scaling the texture window - fixed aspect ratio mode
					LineY0 = ((GLfloat)LineY0 * CurrentPanelHeightFixedAspect * TotalTextureScalableHeight);
					LineY1 = ((GLfloat)LineY1 * CurrentPanelHeightFixedAspect * TotalTextureScalableHeight);
					LineY0 = LineY0 + AspectRatioHeightOffSet;
					LineY1 = LineY1 + AspectRatioHeightOffSet;

				}
				else {

					// Calculate the Y0/Y1 coordinates of the line when scaling the texture window
					LineY0 = ((GLfloat)LineY0 * CurrentTexturePanelHeight * TotalTextureScalableHeight);
					LineY1 = ((GLfloat)LineY1 * CurrentTexturePanelHeight * TotalTextureScalableHeight);

					// Calculate the X0/X1 coordinates of the line when scaling the texture window - fixed aspect ratio mode
					LineX0 = ((GLfloat)LineX0 * CurrentPanelWidthFixedAspect * TotalTextureScalableWidth);
					LineX1 = ((GLfloat)LineX1 * CurrentPanelWidthFixedAspect * TotalTextureScalableWidth);
					LineX0 = LineX0 + AspectRatioWidthOffSet;
					LineX1 = LineX1 + AspectRatioWidthOffSet;

				}

			}
			else {

				// Calculate the Y0/Y1 coordinates of the line when scaling the texture window
				LineY0 = ((GLfloat)LineY0 * CurrentTexturePanelHeight * TotalTextureScalableHeight);
				LineY1 = ((GLfloat)LineY1 * CurrentTexturePanelHeight * TotalTextureScalableHeight);

				// Calculate the X0/X1 coordinates of the line when scaling the texture window - auto aspect ratio mode
				LineX0 = ((GLfloat)LineX0 * CurrentTexturePanelWidth * TotalTextureScalableWidth);
				LineX1 = ((GLfloat)LineX1 * CurrentTexturePanelWidth * TotalTextureScalableWidth);

			}

			// Enable OpenGL 1D texture
			glEnable(GL_TEXTURE_1D);

			// Set the color of the line
			glColor3ub(ColorR, ColorG, ColorB);

			// Set the thickness of the line
			glLineWidth(LineWidth);

			// Render the line on the texture
			glBegin(GL_LINES);

			// Render the line with the given coordinates
			glVertex2f(LineX0, LineY0);
			glVertex2f(LineX1, LineY1);

			// End of configuration
			glEnd();
			// Disable the 1D texture
			glDisable(GL_TEXTURE_1D);

			// Render the line identification label
			RMH_OpenGL_RenderStringOnTexture((LineX0 + LineX1) * 0.5, (LineY0 + LineY1) * 0.5, "L" + RMH_Conversion_IntToStdString(RenderedLineCounter + 1), 255 - ColorR, 255 - ColorG, 255 - ColorB);

		}

		// -------------------- Position-Adjustable Line Rendering And Handling Routines --------------------- //

		LineSpecsPosition RMH_OpenGL_ReadLineType(GLfloat LineX0, GLfloat LineY0, GLfloat LineX1, GLfloat LineY1) {

			// This routine checks whether a line, with given coordinates, is a type 1 line

			// Read the temporary array data and sort the kernel array
			LineSpecsPosition LineParameters;

			// Reset the pixel length parameter of the line
			LineParameters.LinePixelLength = 0;
			// Reset the type parameter of the line
			LineParameters.LineType = 0;
			// Reset the slope parameter of the line
			LineParameters.LineSlope = 0;

			// ------------------------------ Line Type 1 ------------------------------ //

			// Check the line type
			if ((LineX0 < LineX1) && (LineY0 < LineY1)) {

				// Update the type parameter of the line
				LineParameters.LineType = 1;

				// Check which coordinate set has the greatest length
				if ((LineX1 - LineX0) >= (LineY1 - LineY0)) {

					// Calculate the line pixel length
					LineParameters.LinePixelLength = LineX1 - LineX0;

					// Calculate the slope of the line
					LineParameters.LineSlope = (LineY1 - LineY0) / (LineX1 - LineX0);

					// Set the X/Y offsets of the line
					LineParameters.LineXOffset = 1;
					LineParameters.LineYOffset = LineParameters.LineSlope;

				}
				else {

					// Calculate the line pixel length
					LineParameters.LinePixelLength = LineY1 - LineY0;

					// Calculate the slope of the line
					LineParameters.LineSlope = (LineX0 - LineX1) / (LineY1 - LineY0);

					// Set the X/Y offsets of the line
					LineParameters.LineXOffset = -LineParameters.LineSlope;
					LineParameters.LineYOffset = 1;

				}
				
			}

			// ------------------------------ Line Type 2 ------------------------------ //

			// Check the line type
			if ((LineX0 > LineX1) && (LineY0 < LineY1)) {

				// Update the type parameter of the line
				LineParameters.LineType = 2;

				// Check which coordinate set has the greatest length
				if ((LineX0 - LineX1) >= (LineY1 - LineY0)) {

					// Calculate the line pixel length
					LineParameters.LinePixelLength = LineX0 - LineX1;

					// Calculate the slope of the line
					LineParameters.LineSlope = (LineY1 - LineY0) / (LineX0 - LineX1);

					// Set the X/Y offsets of the line
					LineParameters.LineXOffset = -1;
					LineParameters.LineYOffset = LineParameters.LineSlope;

				}
				else {

					// Calculate the line pixel length
					LineParameters.LinePixelLength = LineY1 - LineY0;

					// Calculate the slope of the line
					LineParameters.LineSlope = (LineX1 - LineX0) / (LineY1 - LineY0);

					// Set the X/Y offsets of the line
					LineParameters.LineXOffset = LineParameters.LineSlope;
					LineParameters.LineYOffset = 1;

				}

			}

			// ------------------------------ Line Type 3 ------------------------------ //

			// Check the line type
			if ((LineX0 > LineX1) && (LineY0 > LineY1)) {

				// Update the type parameter of the line
				LineParameters.LineType = 3;

				// Check which coordinate set has the greatest length
				if ((LineX0 - LineX1) >= (LineY0 - LineY1)) {

					// Calculate the line pixel length
					LineParameters.LinePixelLength = LineX0 - LineX1;

					// Calculate the slope of the line
					LineParameters.LineSlope = (LineY0 - LineY1) / (LineX0 - LineX1);

					// Set the X/Y offsets of the line
					LineParameters.LineXOffset = -1;
					LineParameters.LineYOffset = -LineParameters.LineSlope;

				}
				else {

					// Calculate the line pixel length
					LineParameters.LinePixelLength = LineY0 - LineY1;

					// Calculate the slope of the line
					LineParameters.LineSlope = (LineX1 - LineX0) / (LineY0 - LineY1);

					// Set the X/Y offsets of the line
					LineParameters.LineXOffset = LineParameters.LineSlope;
					LineParameters.LineYOffset = -1;

				}

			}

			// ------------------------------ Line Type 4 ------------------------------ //

			// Check the line type
			if ((LineX0 < LineX1) && (LineY0 > LineY1)) {

				// Update the type parameter of the line
				LineParameters.LineType = 4;

				// Check which coordinate set has the greatest length
				if ((LineX1 - LineX0) >= (LineY0 - LineY1)) {

					// Calculate the line pixel length
					LineParameters.LinePixelLength = LineX1 - LineX0;

					// Calculate the slope of the line
					LineParameters.LineSlope = (LineY0 - LineY1) / (LineX1 - LineX0);

					// Set the X/Y offsets of the line
					LineParameters.LineXOffset = 1;
					LineParameters.LineYOffset = -LineParameters.LineSlope;

				}
				else {

					// Calculate the line pixel length
					LineParameters.LinePixelLength = LineY0 - LineY1;

					// Calculate the slope of the line
					LineParameters.LineSlope = (LineX0 - LineX1) / (LineY0 - LineY1);

					// Set the X/Y offsets of the line
					LineParameters.LineXOffset = -LineParameters.LineSlope;
					LineParameters.LineYOffset = -1;

				}

			}

			// ------------------------------ Line Type 5 ------------------------------ //

			// Check the line type
			if ((LineY0 == LineY1) && (LineX0 < LineX1)) {

				// Update the type parameter of the line
				LineParameters.LineType = 5;

				// Calculate the line pixel length
				LineParameters.LinePixelLength = LineX1 - LineX0;

				// Calculate the slope of the line
				LineParameters.LineSlope = 1;

				// Set the X/Y offsets of the line
				LineParameters.LineXOffset = 1;
				LineParameters.LineYOffset = 0;

			}

			// ------------------------------ Line Type 6 ------------------------------ //

			// Check the line type
			if ((LineY0 == LineY1) && (LineX0 > LineX1)) {

				// Update the type parameter of the line
				LineParameters.LineType = 6;

				// Calculate the line pixel length
				LineParameters.LinePixelLength = LineX0 - LineX1;

				// Calculate the slope of the line
				LineParameters.LineSlope = 1;

				// Set the X/Y offsets of the line
				LineParameters.LineXOffset = -1;
				LineParameters.LineYOffset = 0;

			}

			// ------------------------------ Line Type 7 ------------------------------ //

			// Check the line type
			if ((LineX0 == LineX1) && (LineY0 < LineY1)) {

				// Update the type parameter of the line
				LineParameters.LineType = 7;

				// Calculate the line pixel length
				LineParameters.LinePixelLength = LineY1 - LineY0;

				// Calculate the slope of the line
				LineParameters.LineSlope = 1;

				// Set the X/Y offsets of the line
				LineParameters.LineXOffset = 0;
				LineParameters.LineYOffset = 1;

			}

			// ------------------------------ Line Type 8 ------------------------------ //

			// Check the line type
			if ((LineX0 == LineX1) && (LineY0 > LineY1)) {

				// Update the type parameter of the line
				LineParameters.LineType = 8;

				// Calculate the line pixel length
				LineParameters.LinePixelLength = LineY0 - LineY1;

				// Calculate the slope of the line
				LineParameters.LineSlope = 1;

				// Set the X/Y offsets of the line
				LineParameters.LineXOffset = 0;
				LineParameters.LineYOffset = -1;

			}

			// -------------------------------------------------------------------------- //

			// Return the line parameters
			return LineParameters;

		}
		
		LineSpecsPosition RMH_OpenGL_ReadLinePixelCoordinates(GLfloat LineX0, GLfloat LineY0, GLfloat LineX1, GLfloat LineY1) {

			// This routine calculates and stores which pixels the line touches 

			// Read the temporary array data and sort the kernel array
			LineSpecsPosition LineParameters;

			// Read the parameters of the line - type, slope and X/Y offset
			LineParameters = RMH_OpenGL_ReadLineType(LineX0, LineY0, LineX1, LineY1);

			// Read the position coordinates of the line
			LineParameters.LineX0Pos = LineX0;
			LineParameters.LineY0Pos = LineY0;
			LineParameters.LineX1Pos = LineX1;
			LineParameters.LineY1Pos = LineY1;

			// Loop up to and including the calculated line pixel length
			for (unsigned int i = 0; i < LineParameters.LinePixelLength; i++) {

				// Read and store the pixels of the line in the class array
				LineParameters.LineXCordinates[i] = RMH_Math_Round(LineX0);
				LineParameters.LineYCordinates[i] = RMH_Math_Round(LineY0);
				
				// Increment the X0/Y0 coordinates of the line by the X/Y slope of the line
				LineX0 = LineX0 + LineParameters.LineXOffset;
				LineY0 = LineY0 + LineParameters.LineYOffset;

			}

			// Return the pixel length of the line
			return LineParameters;

		}

		bool RMH_OpenGL_IsCursorInsideLine(GLdouble MouseXPosition, GLdouble MouseYPosition, GLfloat LineX0, GLfloat LineY0, GLfloat LineX1, GLfloat LineY1) {

			// This routine checks whether the mouse cursor is within the crosshair area

			// Read the temporary array data and sort the kernel array
			LineSpecsPosition LineParameters;
			bool IsInsideStatus = false;

			// Read which pixels the line touches
			LineParameters = RMH_OpenGL_ReadLinePixelCoordinates(LineX0, LineY0, LineX1, LineY1);

			// Loop up to and including the calculated line pixel length
			for (unsigned int i = 0; i < LineParameters.LinePixelLength; i++) {

				// Check whether the mouse cursor is inside the area of the line. For the X position 
				if (MouseXPosition >= LineParameters.LineXCordinates[i] - MovableLineCursorOffset &&
					MouseXPosition <= LineParameters.LineXCordinates[i] + MovableLineCursorOffset) {

					// Check whether the mouse cursor is inside the area of the line. For the Y position
					if (MouseYPosition >= LineParameters.LineYCordinates[i] - MovableLineCursorOffset &&
						MouseYPosition <= LineParameters.LineYCordinates[i] + MovableLineCursorOffset) {

						//cout << "Inside" << endl;

						// Update the cursor position status
						IsInsideStatus = true;

						// break the for loop
						break;

					}

				}

			}

			// Return the cursor position status
			return IsInsideStatus;

		}

		LineMovableSides RMH_OpenGL_GetSellectedLineMovableSide(GLdouble MouseXPosition, GLdouble MouseYPosition, unsigned short LineTag) {

			// This routine reads and returns which side of the line the mouse is positioned at.

			// Read the temporary array data and sort the kernel array
			LineSpecsPosition LineParameters;
			LineMovableSides ReturnedSide = LineMovableSides::Outside;

			// Read which pixels the line touches
			LineParameters = RMH_OpenGL_ReadLinePixelCoordinates(
				MovableLineX0[LineTag], MovableLineY0[LineTag], 
				MovableLineX1[LineTag], MovableLineY1[LineTag]);

			// Loop up to and including the calculated line pixel length
			for (unsigned int i = 0; i < LineParameters.LinePixelLength; i++) {

				// Check whether the mouse cursor is inside the area of the line. For the X position 
				if (MouseXPosition >= LineParameters.LineXCordinates[i] - MovableLineCursorOffset &&
					MouseXPosition <= LineParameters.LineXCordinates[i] + MovableLineCursorOffset) {

					// Check whether the mouse cursor is inside the area of the line. For the Y position
					if (MouseYPosition >= LineParameters.LineYCordinates[i] - MovableLineCursorOffset &&
						MouseYPosition <= LineParameters.LineYCordinates[i] + MovableLineCursorOffset) {

						// Update the returned selected line enum value
						ReturnedSide = LineMovableSides::Middle;

						// break the for loop
						break;

					}

				}

			}

			// Check whether the mouse cursor is inside the left area of the line - the X0 position
			if (MouseXPosition >= MovableLineX0[LineTag] - (GLfloat)MovableLineCursorOffset &&
				MouseXPosition <= MovableLineX0[LineTag] + (GLfloat)MovableLineCursorOffset) {

				// Check whether the mouse cursor is inside the left area of the line - the X0 position
				if (MouseYPosition >= MovableLineY0[LineTag] - (GLfloat)MovableLineCursorOffset &&
					MouseYPosition <= MovableLineY0[LineTag] + (GLfloat)MovableLineCursorOffset) {

					// Update the returned selected line enum value
					ReturnedSide = LineMovableSides::LeftSide;

				}

			}

			// Check whether the mouse cursor is inside the right area of the line - the X1 position
			if (MouseXPosition >= MovableLineX1[LineTag] - (GLfloat)MovableLineCursorOffset &&
				MouseXPosition <= MovableLineX1[LineTag] + (GLfloat)MovableLineCursorOffset) {

				// Check whether the mouse cursor is inside the right area of the line - the X1 position
				if (MouseYPosition >= MovableLineY1[LineTag] - (GLfloat)MovableLineCursorOffset &&
					MouseYPosition <= MovableLineY1[LineTag] + (GLfloat)MovableLineCursorOffset) {

					// Update the returned selected line enum value
					ReturnedSide = LineMovableSides::RightSide;

				}

			}

			// Return the line enum value
			return ReturnedSide;

		}

		System::Windows::Forms::Cursor^ RMH_OpenGL_GetLineCursor(LineMovableSides LineSide) {

			// This routine returns the relevant associated cursor for a given line position 

			// Selection of the line side
			switch (LineSide) {

				// Return the relevant cursor
				case LineMovableSides::LeftSide: return Cursors::SizeAll;
				case LineMovableSides::Middle: return Cursors::SizeAll;
				case LineMovableSides::RightSide: return Cursors::SizeAll;
				default: return Cursors::Default;

			}

		}

		GLvoid RMH_OpenGL_ChangeLineCursor(GLdouble MouseXPosition, GLdouble MouseYPosition, unsigned short LineTag) {

			// This routine updates the cursor of the overlaid panel, depending on which line side the mouse touches

			// Update the cursor of the overlaid panel
			OverlayPanel->Cursor = RMH_OpenGL_GetLineCursor(RMH_OpenGL_GetSellectedLineMovableSide(MouseXPosition, MouseYPosition, LineTag));

		}

		GLvoid RMH_OpenGL_HandleLineMouseDownEvents(GLdouble MouseXPosition, GLdouble MouseYPosition) {

			// This routine handles events and states when a position-adjustable line is clicked

			// Reset the selected crosshair index value
			SelectedLineTagIndex = 0;

			// If a rendered rectangle or crosshair is not selected
			if (RectangleMoveFlag == false && CrosshairMoveFlag == false) {

				// Loop through all active lines 
				for (unsigned short i = 0; i < NmbOfActiveLines; i++) {

					// Check whether the mouse cursor is inside the area of the active line
					// The highest-priority line is always selected first in the layering 
					if (RMH_OpenGL_IsCursorInsideLine(MouseXPosition, MouseYPosition,
						MovableLineX0[MovableLineOrderIndex[i]], MovableLineY0[MovableLineOrderIndex[i]], 
						MovableLineX1[MovableLineOrderIndex[i]], MovableLineY1[MovableLineOrderIndex[i]])) {

						// Update the move flag of the line
						LineMoveFlag = true;

						// Store the tag index of the selected line
						SelectedLineTagIndex = MovableLineOrderIndex[i];

						// Break the for loop
						break;

					}

				}

			}

			// Update the "old state" number of rendered lines variable
			OldNmbOfActiveLines = NmbOfActiveLines;

			// Reset the selected line side enum
			SelectedLineSide = LineMovableSides::Outside;
			// Read which side of the line has been selected
			SelectedLineSide = RMH_OpenGL_GetSellectedLineMovableSide(MouseXPosition, MouseYPosition, SelectedLineTagIndex);

			// Read the current rectangle coordinates/positions on a new click
			ClickLineX0PositionOffset = MouseXPosition - MovableLineX0[SelectedLineTagIndex];
			ClickLineY0PositionOffset = MouseYPosition - MovableLineY0[SelectedLineTagIndex];
			ClickLineX1PositionOffset = MouseXPosition - MovableLineX1[SelectedLineTagIndex];
			ClickLineY1PositionOffset = MouseYPosition - MovableLineY1[SelectedLineTagIndex];

		}

		GLvoid RMH_OpenGL_HandleLineMouseMoveEvents(GLdouble MouseXPosition, GLdouble MouseYPosition) {

			// This routine handles events and states when a clicked position-adjustable line is to move

			// Read the temporary array data and sort the kernel array
			GLdouble LineXPosition = 0;
			GLdouble LineYPosition = 0;

			// Check the number of active lines and the selected index
			if (NmbOfActiveLines > 0 && SelectedLineTagIndex != 0) {

				// Update the cursor type of the overlaid panel
				RMH_OpenGL_ChangeLineCursor(MouseXPosition, MouseYPosition, SelectedLineTagIndex);

				// If the panel has not yet been clicked
				if (OverlayPanelIsClick == false) {

					// Do not continue
					return;

				}

				// Which line side position should be updated
				switch (SelectedLineSide) {

					// Left line corner
					case LineMovableSides::LeftSide:

						// Read the new position of the left corner of the line
						LineXPosition = MouseXPosition - ClickLineX0PositionOffset;
						LineYPosition = MouseYPosition - ClickLineX0PositionOffset;

						if (LineXPosition >= MovableLineX1[SelectedLineTagIndex] - MovableLineMinimumLength &&
							LineXPosition <= MovableLineX1[SelectedLineTagIndex] + MovableLineMinimumLength) {

							if (LineYPosition >= MovableLineY1[SelectedLineTagIndex] - MovableLineMinimumLength &&
								LineYPosition <= MovableLineY1[SelectedLineTagIndex] + MovableLineMinimumLength) {

								// Break case (do not update the position)
								break;

							}

						}

						// Update the position of the left corner of the line
						MovableLineX0[SelectedLineTagIndex] = LineXPosition;
						MovableLineY0[SelectedLineTagIndex] = LineYPosition;

					break;

					// Middle of the line
					case LineMovableSides::Middle:

						// Should the whole line move
						if (LineMoveFlag == true) {

							// Update the position of the left and right corner of the line
							MovableLineX0[SelectedLineTagIndex] = MouseXPosition - ClickLineX0PositionOffset;
							MovableLineY0[SelectedLineTagIndex] = MouseYPosition - ClickLineY0PositionOffset;
							MovableLineX1[SelectedLineTagIndex] = MouseXPosition - ClickLineX1PositionOffset;
							MovableLineY1[SelectedLineTagIndex] = MouseYPosition - ClickLineY1PositionOffset;

						}

					break;

					// Right line corner
					case LineMovableSides::RightSide:

						// Read the new position of the right corner of the line
						LineXPosition = MouseXPosition - ClickLineX1PositionOffset;
						LineYPosition = MouseYPosition - ClickLineX1PositionOffset;

						if (LineXPosition >= MovableLineX0[SelectedLineTagIndex] - MovableLineMinimumLength &&
							LineXPosition <= MovableLineX0[SelectedLineTagIndex] + MovableLineMinimumLength) {

							if (LineYPosition >= MovableLineY0[SelectedLineTagIndex] - MovableLineMinimumLength &&
								LineYPosition <= MovableLineY0[SelectedLineTagIndex] + MovableLineMinimumLength) {

								// Break case (do not update the position)
								break;

							}

						}

						// Update the position of the right corner of the line
						MovableLineX1[SelectedLineTagIndex] = LineXPosition;
						MovableLineY1[SelectedLineTagIndex] = LineYPosition;

					break;

				}

				// Limit the position of the line to the texture area
				if (MovableLineX0[SelectedLineTagIndex] <= LineTextureBorderPadding) MovableLineX0[SelectedLineTagIndex] = LineTextureBorderPadding;
				if (MovableLineX0[SelectedLineTagIndex] > ImageDataPixelWidth - LineTextureBorderPadding) MovableLineX0[SelectedLineTagIndex] = (ImageDataPixelWidth) - LineTextureBorderPadding;
				if (MovableLineY0[SelectedLineTagIndex] <= LineTextureBorderPadding) MovableLineY0[SelectedLineTagIndex] = LineTextureBorderPadding;
				if (MovableLineY0[SelectedLineTagIndex] > ImageDataPixelHeight - LineTextureBorderPadding) MovableLineY0[SelectedLineTagIndex] = (ImageDataPixelHeight) - LineTextureBorderPadding;
				if (MovableLineX1[SelectedLineTagIndex] <= LineTextureBorderPadding) MovableLineX1[SelectedLineTagIndex] = LineTextureBorderPadding;
				if (MovableLineX1[SelectedLineTagIndex] > ImageDataPixelWidth - LineTextureBorderPadding) MovableLineX1[SelectedLineTagIndex] = (ImageDataPixelWidth) - LineTextureBorderPadding;
				if (MovableLineY1[SelectedLineTagIndex] <= LineTextureBorderPadding) MovableLineY1[SelectedLineTagIndex] = LineTextureBorderPadding;
				if (MovableLineY1[SelectedLineTagIndex] > ImageDataPixelHeight - LineTextureBorderPadding) MovableLineY1[SelectedLineTagIndex] = (ImageDataPixelHeight) - LineTextureBorderPadding;

			}

		}

		LineSpecsPosition RMH_OpenGL_RenderMovableLine(unsigned short OrderPriority, GLfloat LineWidth, GLubyte SelectedColorR, GLubyte SelectedColorG, GLubyte SelectedColorB, GLubyte PassiveColorR, GLubyte PassiveColorG, GLubyte PassiveColorB) {

			// This routine renders a position-adjustable line

			// Read the temporary array data and sort the kernel array
			LineSpecsPosition RenderedLinePosition;

			// Check for the maximum tag order value
			if (OrderPriority < 1 || OrderPriority > _MaxNumberOfMovableCrosshairs) {

				// Write status message in the terminal
				cout << "Order Must Be Higher Then 0 And Less That Max" << endl;

			}
			else {

				// Read the position parameters of the line 
				RenderedLinePosition = RMH_OpenGL_ReadLinePixelCoordinates(
					MovableLineX0[OrderPriority], MovableLineY0[OrderPriority], 
					MovableLineX1[OrderPriority], MovableLineY1[OrderPriority]);
				
				// Store the rendering order of the line
				MovableLineOrderIndex[RenderedLineCounter] = OrderPriority;

				// Is this rendered line the last selected line
				if (OrderPriority == SelectedLineTagIndex) {

					// Render the line on the texture
					RMH_OpenGL_RenderLine(
						RenderedLinePosition.LineX0Pos,
						RenderedLinePosition.LineY0Pos,
						RenderedLinePosition.LineX1Pos,
						RenderedLinePosition.LineY1Pos, LineWidth,
						SelectedColorR, SelectedColorG, SelectedColorB);
				}
				else {

					// Render the line on the texture
					RMH_OpenGL_RenderLine(
						RenderedLinePosition.LineX0Pos,
						RenderedLinePosition.LineY0Pos,
						RenderedLinePosition.LineX1Pos,
						RenderedLinePosition.LineY1Pos, LineWidth,
						PassiveColorR, PassiveColorG, PassiveColorB);

				}
				
				// Increment the line rendering counter variable
				RenderedLineCounter = RenderedLineCounter + 1;

			}

			// Return the position of the rectangle
			return RenderedLinePosition;

		}

		// ---------------------------- Crosshair Rendering And Handling Routines ----------------------------- //

		GLvoid RMH_OpenGL_EnableLabelBackground(bool EnableFlag) {

			// This routine enables the rendering of a background rectangle for all rendered text labels

			// Update the label background enable flag
			EnableLabelBackgroundFlag = EnableFlag;

		}

		GLvoid RMH_OpenGL_ChangeRenderedLabelsColor(GLubyte LabelColorR, GLubyte LabelColorG, GLubyte LabelColorB) {

			// This routine updates the color of all rendered text labels
		
			// Update the color of all rendered text labels
			CommonLabelColorR = LabelColorR;
			CommonLabelColorG = LabelColorG;
			CommonLabelColorB = LabelColorB;

		}

		GLvoid RMH_OpenGL_ChangeLabelBackgroundColor(GLubyte BackgroundColorR, GLubyte BackgroundColorG, GLubyte BackgroundColorB) {

			// This routine updates the color of the label background

			// Set the color of the label background
			LabelBackgroundColorR = BackgroundColorR;
			LabelBackgroundColorG = BackgroundColorG;
			LabelBackgroundColorB = BackgroundColorB;

		}

		GLvoid RMH_OpenGL_RenderCrossHairWithLabel(GLfloat X, GLfloat Y, bool EnableLabel, System::String^ LabelString, GLubyte CrosshairColorR, GLubyte CrosshairColorG, GLubyte CrosshairColorB) {

			// This routine renders a crosshair on the texture, with or without an associated label

			// Read the temporary array data and sort the kernel array
			GLfloat QuadrantXOffset = 0.0;
			GLfloat QuadrantYOffset = 0.0;

			// Should a label be added to the crosshair
			if (EnableLabel == true) {

				// Check whether the position is in quadrant 1
				if (X >= ((GLfloat)ImageDataPixelWidth * 0.5) && Y <= ((GLfloat)ImageDataPixelHeight * 0.5)) {

					// Check the live view rotation setting
					if (LiveViewRotationDegrees == 0) {

						// Update the quadrant offset values - 0 degrees rotation
						QuadrantXOffset = MovableLineQuadrant1LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant1LabelYOffset;

					}
					if (LiveViewRotationDegrees == 90) {

						// Update the quadrant offset values - 90 degrees rotation
						QuadrantXOffset = MovableLineQuadrant2LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant2LabelYOffset;

					}
					if (LiveViewRotationDegrees == 180) {

						// Update the quadrant offset values - 180 degrees rotation
						QuadrantXOffset = MovableLineQuadrant3LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant3LabelYOffset;

					}
					if (LiveViewRotationDegrees == 270) {

						// Update the quadrant offset values - 270 degrees rotation
						QuadrantXOffset = MovableLineQuadrant4LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant4LabelYOffset;

					}

				}

				// Check whether the position is in quadrant 2
				if (X <= ((GLfloat)ImageDataPixelWidth * 0.5) && Y <= ((GLfloat)ImageDataPixelHeight * 0.5)) {
	
					// Check the live view rotation setting
					if (LiveViewRotationDegrees == 0) {

						// Update the quadrant offset values - 0 degrees rotation
						QuadrantXOffset = MovableLineQuadrant2LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant2LabelYOffset;

					}
					if (LiveViewRotationDegrees == 90) {

						// Update the quadrant offset values - 90 degrees rotation
						QuadrantXOffset = MovableLineQuadrant3LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant3LabelYOffset;

					}
					if (LiveViewRotationDegrees == 180) {

						// Update the quadrant offset values - 180 degrees rotation
						QuadrantXOffset = MovableLineQuadrant4LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant4LabelYOffset;

					}
					if (LiveViewRotationDegrees == 270) {

						// Update the quadrant offset values - 270 degrees rotation
						QuadrantXOffset = MovableLineQuadrant1LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant1LabelYOffset;

					}

				}

				// Check whether the position is in quadrant 3
				if (X <= ((GLfloat)ImageDataPixelWidth * 0.5) && Y >= ((GLfloat)ImageDataPixelHeight * 0.5)) {

					// Check the live view rotation setting
					if (LiveViewRotationDegrees == 0) {

						// Update the quadrant offset values - 0 degrees rotation
						QuadrantXOffset = MovableLineQuadrant3LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant3LabelYOffset;

					}
					if (LiveViewRotationDegrees == 90) {

						// Update the quadrant offset values - 90 degrees rotation
						QuadrantXOffset = MovableLineQuadrant4LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant4LabelYOffset;

					}
					if (LiveViewRotationDegrees == 180) {

						// Update the quadrant offset values - 180 degrees rotation
						QuadrantXOffset = MovableLineQuadrant1LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant1LabelYOffset;

					}
					if (LiveViewRotationDegrees == 270) {

						// Update the quadrant offset values - 270 degrees rotation
						QuadrantXOffset = MovableLineQuadrant2LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant2LabelYOffset;

					}

				}

				// Check whether the position is in quadrant 4
				if (X >= ((GLfloat)ImageDataPixelWidth * 0.5) && Y >= ((GLfloat)ImageDataPixelHeight * 0.5)) {
	
					// Check the live view rotation setting
					if (LiveViewRotationDegrees == 0) {

						// Update the quadrant offset values - 0 degrees rotation
						QuadrantXOffset = MovableLineQuadrant4LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant4LabelYOffset;

					}
					if (LiveViewRotationDegrees == 90) {

						// Update the quadrant offset values - 90 degrees rotation
						QuadrantXOffset = MovableLineQuadrant1LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant1LabelYOffset;

					}
					if (LiveViewRotationDegrees == 180) {

						// Update the quadrant offset values - 180 degrees rotation
						QuadrantXOffset = MovableLineQuadrant2LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant2LabelYOffset;

					}
					if (LiveViewRotationDegrees == 270) {

						// Update the quadrant offset values - 270 degrees rotation
						QuadrantXOffset = MovableLineQuadrant3LabelXOffset;
						QuadrantYOffset = MovableLineQuadrant3LabelYOffset;

					}

				}

			}

			// Check the selected aspect ratio setting
			if (LocalAspectRatioFlag == true) {

				// Check the live view rotation setting
				if (LiveViewRotationDegrees == 0) {

					// If horizontal aspect ratio compensation is needed
					if (LiveViewPosX0 <= 0.0) {

						// Calculate the crosshair Y coordinate when scaling the texture window - fixed aspect ratio mode
						Y = Y * CurrentPanelHeightFixedAspect * TotalTextureScalableHeight;
						Y = Y + AspectRatioHeightOffSet;

						// Calculate the crosshair X coordinate when scaling the texture window
						X = X * CurrentTexturePanelWidth * TotalTextureScalableWidth;

					}
					else {

						// Calculate the crosshair Y coordinate when scaling the texture window
						Y = Y * CurrentTexturePanelHeight * TotalTextureScalableHeight;

						// Calculate the crosshair X coordinate when scaling the texture window - fixed aspect ratio mode
						X = X * CurrentPanelWidthFixedAspect * TotalTextureScalableWidth;
						X = X + AspectRatioWidthOffSet;

					}

				}
				if (LiveViewRotationDegrees == 90) {

					// Scale the X/Y coordinates
					X = X / ImageDataPixelAspectRatio;
					Y = Y * ImageDataPixelAspectRatio;

					// If horizontal aspect ratio compensation is needed
					if (LiveViewPosX0 <= 0.0) {

						// Calculate the crosshair Y coordinate when scaling the texture window - fixed aspect ratio mode
						X = X * CurrentPanelHeightFixedAspect * TotalTextureScalableHeight;
						
						// Calculate the crosshair X coordinate when scaling the texture window
						Y = Y * CurrentTexturePanelWidth * TotalTextureScalableWidth;

					}
					else {

						// Calculate the crosshair Y coordinate when scaling the texture window
						X = X * CurrentTexturePanelHeight * TotalTextureScalableHeight;

						// Calculate the crosshair X coordinate when scaling the texture window - fixed aspect ratio mode
						Y = Y * CurrentPanelWidthFixedAspect * TotalTextureScalableWidth;
						Y = Y + AspectRatioWidthOffSet;

					}

					// Adjust the X coordinate
					X = LiveViewPosY1 - X;

				}
				if (LiveViewRotationDegrees == 180) {

					// Scale the X/Y coordinates
					Y = ImageDataPixelHeight - Y;
					X = ImageDataPixelWidth - X;

					// If horizontal aspect ratio compensation is needed
					if (LiveViewPosX0 <= 0.0) {

						// Calculate the crosshair Y coordinate when scaling the texture window - fixed aspect ratio mode
						Y = Y * CurrentPanelHeightFixedAspect * TotalTextureScalableHeight;
						Y = Y + AspectRatioHeightOffSet;

						// Calculate the crosshair X coordinate when scaling the texture window
						X = X * CurrentTexturePanelWidth * TotalTextureScalableWidth;

					}
					else {

						// Calculate the crosshair Y coordinate when scaling the texture window
						Y = Y * CurrentTexturePanelHeight * TotalTextureScalableHeight;

						// Calculate the crosshair X coordinate when scaling the texture window - fixed aspect ratio mode
						X = X * CurrentPanelWidthFixedAspect * TotalTextureScalableWidth;
						X = X + AspectRatioWidthOffSet;

					}

				}
				if (LiveViewRotationDegrees == 270) {

					// Scale the X/Y coordinates
					X = (ImageDataPixelWidth - X) / ImageDataPixelAspectRatio;
					Y = (ImageDataPixelHeight - Y) * ImageDataPixelAspectRatio;

					// If horizontal aspect ratio compensation is needed
					if (LiveViewPosX0 <= 0.0) {

						// Calculate the crosshair Y coordinate when scaling the texture window - fixed aspect ratio mode
						X = X * CurrentPanelHeightFixedAspect * TotalTextureScalableHeight;

						// Calculate the crosshair X coordinate when scaling the texture window
						Y = Y * CurrentTexturePanelWidth * TotalTextureScalableWidth;

					}
					else {

						// Calculate the crosshair Y coordinate when scaling the texture window
						X = X * CurrentTexturePanelHeight * TotalTextureScalableHeight;

						// Calculate the crosshair X coordinate when scaling the texture window - fixed aspect ratio mode
						Y = Y * CurrentPanelWidthFixedAspect * TotalTextureScalableWidth;
						Y = Y + AspectRatioWidthOffSet;

					}

					// Adjust the X coordinate
					X = LiveViewPosY1 - X;

				}

			}
			else {

				// Check the live view rotation setting
				if (LiveViewRotationDegrees == 0) {

					// Calculate the crosshair Y coordinate when scaling the texture window
					Y = Y * CurrentTexturePanelHeight * TotalTextureScalableHeight;

					// Calculate the crosshair X coordinate when scaling the texture window - auto aspect ratio mode
					X = X * CurrentTexturePanelWidth * TotalTextureScalableWidth;

				}
				if (LiveViewRotationDegrees == 90) {

					// Scale the X/Y coordinates
					X = X / ImageDataPixelAspectRatio;
					Y = Y * ImageDataPixelAspectRatio;

					// Calculate the crosshair Y coordinate when scaling the texture window
					Y = Y * CurrentTexturePanelWidth * TotalTextureScalableWidth;

					// Calculate the crosshair X coordinate when scaling the texture window - auto aspect ratio mode
					X = X * CurrentTexturePanelHeight * TotalTextureScalableHeight;

					// Adjust the X coordinate
					X = LiveViewPosY1 - X;

				}
				if (LiveViewRotationDegrees == 180) {

					// Scale the X/Y coordinates
					Y = ImageDataPixelHeight - Y;
					X = ImageDataPixelWidth - X;

					// Calculate the crosshair Y coordinate when scaling the texture window
					Y = Y * CurrentTexturePanelHeight * TotalTextureScalableHeight;

					// Calculate the crosshair X coordinate when scaling the texture window - auto aspect ratio mode
					X = X * CurrentTexturePanelWidth * TotalTextureScalableWidth;

				}
				if (LiveViewRotationDegrees == 270) {

					// Scale the X/Y coordinates
					X = (ImageDataPixelWidth - X) / ImageDataPixelAspectRatio;
					Y = (ImageDataPixelHeight - Y) * ImageDataPixelAspectRatio;

					// Calculate the crosshair Y coordinate when scaling the texture window
					Y = Y * CurrentTexturePanelWidth * TotalTextureScalableWidth;

					// Calculate the crosshair X coordinate when scaling the texture window - auto aspect ratio mode
					X = X * CurrentTexturePanelHeight * TotalTextureScalableHeight;

					// Adjust the X coordinate
					X = LiveViewPosY1 - X;

				}

			}

			// Should a background be rendered for the label
			if (EnableLabelBackgroundFlag == true) {

				// Enable OpenGL 1D texture
				glEnable(GL_TEXTURE_1D);
				glEnable(GL_BLEND);

				// Set the color of the label background
				glColor4ub(LabelBackgroundColorR, LabelBackgroundColorG, LabelBackgroundColorB, CommonLabelBackgroundAlpha);

				// Render the rectangle on the texture
				glBegin(GL_QUADS);

				// Check the live view rotation setting
				if (LiveViewRotationDegrees == 0) {

					// Render the positions of the label rectangle
					glVertex2f((X + QuadrantXOffset) - LabelBackgroundXOffset, (Y + QuadrantYOffset) - LabelBackgroundYOffset);
					glVertex2f((X + QuadrantXOffset) - LabelBackgroundXOffset + LabelBackgroundWidth, (Y + QuadrantYOffset) - LabelBackgroundYOffset);
					glVertex2f((X + QuadrantXOffset) - LabelBackgroundXOffset + LabelBackgroundWidth, (Y + QuadrantYOffset) - LabelBackgroundYOffset + LabelBackgroundHeight);
					glVertex2f((X + QuadrantXOffset) - LabelBackgroundXOffset, (Y + QuadrantYOffset) - LabelBackgroundYOffset + LabelBackgroundHeight);

				}
				if (LiveViewRotationDegrees == 90) {

					// Render the positions of the label rectangle
					glVertex2f((Y + QuadrantXOffset) - LabelBackgroundXOffset, (X + QuadrantYOffset) - LabelBackgroundYOffset);
					glVertex2f((Y + QuadrantXOffset) - LabelBackgroundXOffset + LabelBackgroundWidth, (X + QuadrantYOffset) - LabelBackgroundYOffset);
					glVertex2f((Y + QuadrantXOffset) - LabelBackgroundXOffset + LabelBackgroundWidth, (X + QuadrantYOffset) - LabelBackgroundYOffset + LabelBackgroundHeight);
					glVertex2f((Y + QuadrantXOffset) - LabelBackgroundXOffset, (X + QuadrantYOffset) - LabelBackgroundYOffset + LabelBackgroundHeight);

				}
				if (LiveViewRotationDegrees == 180) {

					// Render the positions of the label rectangle
					glVertex2f((X + QuadrantXOffset) - LabelBackgroundXOffset, (Y + QuadrantYOffset) - LabelBackgroundYOffset);
					glVertex2f((X + QuadrantXOffset) - LabelBackgroundXOffset + LabelBackgroundWidth, (Y + QuadrantYOffset) - LabelBackgroundYOffset);
					glVertex2f((X + QuadrantXOffset) - LabelBackgroundXOffset + LabelBackgroundWidth, (Y + QuadrantYOffset) - LabelBackgroundYOffset + LabelBackgroundHeight);
					glVertex2f((X + QuadrantXOffset) - LabelBackgroundXOffset, (Y + QuadrantYOffset) - LabelBackgroundYOffset + LabelBackgroundHeight);

				}
				if (LiveViewRotationDegrees == 270) {

					// Render the positions of the label rectangle
					glVertex2f((Y + QuadrantXOffset) - LabelBackgroundXOffset, (X + QuadrantYOffset) - LabelBackgroundYOffset);
					glVertex2f((Y + QuadrantXOffset) - LabelBackgroundXOffset + LabelBackgroundWidth, (X + QuadrantYOffset) - LabelBackgroundYOffset);
					glVertex2f((Y + QuadrantXOffset) - LabelBackgroundXOffset + LabelBackgroundWidth, (X + QuadrantYOffset) - LabelBackgroundYOffset + LabelBackgroundHeight);
					glVertex2f((Y + QuadrantXOffset) - LabelBackgroundXOffset, (X + QuadrantYOffset) - LabelBackgroundYOffset + LabelBackgroundHeight);

				}

				// End of configuration
				glEnd();
				// Disable the 1D texture
				glDisable(GL_TEXTURE_1D);
				glDisable(GL_BLEND);

			}

			// Should a label be added to the crosshair
			if (EnableLabel == true) {

				// Check the live view rotation setting
				if (LiveViewRotationDegrees == 0) {

					// Add the label to the crosshair
					RMH_OpenGL_RenderStringOnTexture(X + QuadrantXOffset, Y + QuadrantYOffset, RMH_Conversion_SystemStringToStdString(LabelString), CommonLabelColorR, CommonLabelColorG, CommonLabelColorB);

				}
				if (LiveViewRotationDegrees == 90) {

					// Add the label to the crosshair
					RMH_OpenGL_RenderStringOnTexture(Y + QuadrantXOffset, X + QuadrantYOffset, RMH_Conversion_SystemStringToStdString(LabelString), CommonLabelColorR, CommonLabelColorG, CommonLabelColorB);

				}
				if (LiveViewRotationDegrees == 180) {

					// Add the label to the crosshair
					RMH_OpenGL_RenderStringOnTexture(X + QuadrantXOffset, Y + QuadrantYOffset, RMH_Conversion_SystemStringToStdString(LabelString), CommonLabelColorR, CommonLabelColorG, CommonLabelColorB);

				}
				if (LiveViewRotationDegrees == 270) {

					// Add the label to the crosshair
					RMH_OpenGL_RenderStringOnTexture(Y + QuadrantXOffset, X + QuadrantYOffset, RMH_Conversion_SystemStringToStdString(LabelString), CommonLabelColorR, CommonLabelColorG, CommonLabelColorB);

				}
			}

			// Enable OpenGL 1D texture
			glEnable(GL_TEXTURE_1D);

			// Set the crosshair color
			glColor3ub(CrosshairColorR, CrosshairColorG, CrosshairColorB);

			// Set the crosshair line thickness
			glLineWidth(CrosshairLineWidth);

			// Render the line on the texture
			glBegin(GL_LINES);

			// Check the live view rotation setting
			if (LiveViewRotationDegrees == 0) {

				// Vertical line
				glVertex2f(X, Y - (CrosshairSize * 0.5));
				glVertex2f(X, Y + (CrosshairSize * 0.5));

				// Horizontal line
				glVertex2f(X - (CrosshairSize * 0.5), Y);
				glVertex2f(X + (CrosshairSize * 0.5), Y);

			}
			if (LiveViewRotationDegrees == 90) {

				// Vertical line
				glVertex2f(Y - (CrosshairSize * 0.5), X);
				glVertex2f(Y + (CrosshairSize * 0.5), X);

				// Horizontal line
				glVertex2f(Y, X - (CrosshairSize * 0.5));
				glVertex2f(Y, X + (CrosshairSize * 0.5));

			}
			if (LiveViewRotationDegrees == 180) {

				// Vertical line
				glVertex2f(X, Y - (CrosshairSize * 0.5));
				glVertex2f(X, Y + (CrosshairSize * 0.5));

				// Horizontal line
				glVertex2f(X - (CrosshairSize * 0.5), Y);
				glVertex2f(X + (CrosshairSize * 0.5), Y);

			}
			if (LiveViewRotationDegrees == 270) {

				// Vertical line
				glVertex2f(Y - (CrosshairSize * 0.5), X);
				glVertex2f(Y + (CrosshairSize * 0.5), X);

				// Horizontal line
				glVertex2f(Y, X - (CrosshairSize * 0.5));
				glVertex2f(Y, X + (CrosshairSize * 0.5));

			}

			// End of configuration
			glEnd();
			// Disable the 1D texture
			glDisable(GL_TEXTURE_1D);

		}

		GLvoid RMH_OpenGL_RenderCrossHairCenterLabel(GLfloat X, GLfloat Y, System::String^ LabelString, GLubyte CrosshairColorR, GLubyte CrosshairColorG, GLubyte CrosshairColorB) {

			// This routine renders a crosshair on the texture, where the associated label is centered at the bottom

			// Calculate the crosshair Y coordinate when scaling the texture window
			Y = ((GLfloat)Y * CurrentTexturePanelHeight * TotalTextureScalableHeight);

			// Check the selected aspect ratio setting
			if (LocalAspectRatioFlag == true) {

				// Calculate the crosshair X coordinate when scaling the texture window - fixed aspect ratio mode
				X = ((GLfloat)X * CurrentPanelWidthFixedAspect * TotalTextureScalableWidth);
				X = X + AspectRatioWidthOffSet;
			}
			else {

				// Calculate the crosshair X coordinate when scaling the texture window - auto aspect ratio mode
				X = ((GLfloat)X * CurrentTexturePanelWidth * TotalTextureScalableWidth);

			}

			// Should a background be rendered for the label
			if (EnableLabelBackgroundFlag == true) {

				// Enable OpenGL 1D texture
				glEnable(GL_TEXTURE_1D);
				glEnable(GL_BLEND);

				// Set the color of the label background
				glColor4ub(LabelBackgroundColorR, LabelBackgroundColorG, LabelBackgroundColorB, CommonLabelBackgroundAlpha);

				// Render the rectangle on the texture
				glBegin(GL_QUADS);

				// Render the positions of the label rectangle
				glVertex2f((X + ChrosshairWithCenterLabelXOffset) - CenterLabelBackgroundXOffset, (Y + ChrosshairWithCenterLabelYOffset) - CenterLabelBackgroundYOffset);
				glVertex2f((X + ChrosshairWithCenterLabelXOffset) - CenterLabelBackgroundXOffset + CenterLabelBackgroundWidth, (Y + ChrosshairWithCenterLabelYOffset) - CenterLabelBackgroundYOffset);
				glVertex2f((X + ChrosshairWithCenterLabelXOffset) - CenterLabelBackgroundXOffset + CenterLabelBackgroundWidth, (Y + ChrosshairWithCenterLabelYOffset) - CenterLabelBackgroundYOffset + CenterLabelBackgroundHeight);
				glVertex2f((X + ChrosshairWithCenterLabelXOffset) - CenterLabelBackgroundXOffset, (Y + ChrosshairWithCenterLabelYOffset) - CenterLabelBackgroundYOffset + CenterLabelBackgroundHeight);

				// End of configuration
				glEnd();
				// Disable the 1D texture
				glDisable(GL_TEXTURE_1D);
				glDisable(GL_BLEND);

			}

			// Add the label to the crosshair
			RMH_OpenGL_RenderStringOnTexture(X + ChrosshairWithCenterLabelXOffset, Y + ChrosshairWithCenterLabelYOffset, RMH_Conversion_SystemStringToStdString(LabelString), CommonLabelColorR, CommonLabelColorG, CommonLabelColorB);

			// Enable OpenGL 1D texture
			glEnable(GL_TEXTURE_1D);

			// Set the crosshair color
			glColor3ub(CrosshairColorR, CrosshairColorG, CrosshairColorB);

			// Set the crosshair line thickness
			glLineWidth(CrosshairLineWidth);

			// Render the line on the texture
			glBegin(GL_LINES);

			// Vertical line
			glVertex2f(X, Y - (CrosshairSize * 0.5));
			glVertex2f(X, Y + (CrosshairSize * 0.5));

			// Horizontal line
			glVertex2f(X - (CrosshairSize * 0.5), Y);
			glVertex2f(X + (CrosshairSize * 0.5), Y);

			// End of configuration
			glEnd();
			// Disable the 1D texture
			glDisable(GL_TEXTURE_1D);

		}

		// ---------------------- Position-Adjustable Crosshair And Label Handling Routines ---------------------- //

		bool RMH_OpenGL_IsCursorInsideCrosshair(GLdouble MouseXPosition, GLdouble MouseYPosition, GLdouble CrossHairX0, GLdouble CrossHairY0) {

			// This routine checks whether the mouse cursor is within the crosshair area
			
			// Read the temporary array data and sort the kernel array
			bool IsInsideStatus = false;

			// Check whether the mouse cursor is inside the crosshair area - the X position
			if (MouseXPosition >= CrossHairX0 - (CrosshairSize + CrosshairInsideAreaPadding) &&
				MouseXPosition <= CrossHairX0 + (CrosshairSize + CrosshairInsideAreaPadding)) {

				// Check whether the mouse cursor is inside the crosshair area - the X position
				if (MouseYPosition >= CrossHairY0 - (CrosshairSize + CrosshairInsideAreaPadding) &&
					MouseYPosition <= CrossHairY0 + (CrosshairSize + CrosshairInsideAreaPadding)) {

					// Update the cursor position status
					IsInsideStatus = true;

				}

			}

			// Return the cursor position status
			return IsInsideStatus;

		}

		CrosshairSizableSides RMH_OpenGL_GetSellectedCrosshairSizableSide(GLdouble MouseXPosition, GLdouble MouseYPosition, unsigned short CrosshairTag) {

			// This routine reads and returns which side of the crosshair the mouse is positioned at.

			// Read the temporary array data and sort the kernel array
			CrosshairSizableSides ReturnedSide = CrosshairSizableSides::Default;

			// Check whether the mouse cursor is inside the crosshair area - the X position
			if (MouseXPosition >= CrosshairX0[CrosshairTag] - (CrosshairSize + CrosshairInsideAreaPadding) &&
				MouseXPosition <= CrosshairX0[CrosshairTag] + (CrosshairSize + CrosshairInsideAreaPadding)) {

				// Check whether the mouse cursor is inside the crosshair area - the X position
				if (MouseYPosition >= CrosshairY0[CrosshairTag] - (CrosshairSize + CrosshairInsideAreaPadding) &&
					MouseYPosition <= CrosshairY0[CrosshairTag] + (CrosshairSize + CrosshairInsideAreaPadding)) {

					// Update the returned selected line enum value
					ReturnedSide = CrosshairSizableSides::Crosshair;

				}

			}
			
			// Return the crosshair enum value
			return ReturnedSide;

		}

		System::Windows::Forms::Cursor^ RMH_OpenGL_GetCrosshairCursor(CrosshairSizableSides CrosshairSide) {

			// This routine returns the relevant associated cursor for a given crosshair position 

			// Selection of the crosshair side
			switch (CrosshairSide) {

				// Return the relevant cursor
				case CrosshairSizableSides::Crosshair: return Cursors::SizeAll;
				default: return Cursors::Default;

			}

		}

		GLvoid RMH_OpenGL_ChangeCrosshairCursor(GLdouble MouseXPosition, GLdouble MouseYPosition, unsigned short CrosshairTag) {

			// This routine updates the cursor of the overlaid panel, depending on which crosshair side the mouse touches

			// Update the cursor of the overlaid panel
			OverlayPanel->Cursor = RMH_OpenGL_GetCrosshairCursor(RMH_OpenGL_GetSellectedCrosshairSizableSide(MouseXPosition, MouseYPosition, CrosshairTag));

		}

		GLvoid RMH_OpenGL_HandleCrosshairMouseDownEvents(GLdouble MouseXPosition, GLdouble MouseYPosition) {

			// This routine handles events and states when a position-adjustable crosshair is clicked

			// Reset the selected crosshair index value
			SelectedCrosshairTagIndex = 0;

			// If a rendered rectangle is not selected
			if (RectangleMoveFlag == false && LineMoveFlag == false) {

				// Loop through all active crosshairs 
				for (unsigned short i = 0; i < NmbOfActiveCrosshairs; i++) {

					// Check whether the mouse cursor is inside the area of the active crosshair
					// The highest-priority crosshair is always selected first in the layering 
					if (RMH_OpenGL_IsCursorInsideCrosshair(MouseXPosition, MouseYPosition,
						CrosshairX0[CrosshairOrderIndex[i]], CrosshairY0[CrosshairOrderIndex[i]])) {

						// Update the move flag of the crosshair
						CrosshairMoveFlag = true;

						// Store the tag index of the selected rectangle
						SelectedCrosshairTagIndex = CrosshairOrderIndex[i];

						// Break the for loop
						break;

					}

				}

			}

			// Update the "old state" number of rendered crosshairs variable
			OldNmbOfActiveCrosshairs = NmbOfActiveCrosshairs;

			// Read the current rectangle coordinates/positions on a new click
			ClickCrosshairXPositionOffset = MouseXPosition - CrosshairX0[SelectedCrosshairTagIndex];
			ClickCrosshairYPositionOffset = MouseYPosition - CrosshairY0[SelectedCrosshairTagIndex];

		}

		GLvoid RMH_OpenGL_HandleCrosshairMouseMoveEvents(GLdouble MouseXPosition, GLdouble MouseYPosition) {

			// This routine handles events and states when a clicked position-adjustable crosshair is to move

			// Add the rectangle click offset to the mouse position
			MouseXPosition = MouseXPosition - ClickCrosshairXPositionOffset;
			MouseYPosition = MouseYPosition - ClickCrosshairYPositionOffset;

			// Check the number of active crosshairs and the selected index
			if (NmbOfActiveCrosshairs > 0 && SelectedCrosshairTagIndex != 0) {

				// Update the cursor type of the overlaid panel
				RMH_OpenGL_ChangeCrosshairCursor(MouseXPosition + ClickCrosshairXPositionOffset, MouseYPosition + ClickCrosshairYPositionOffset, SelectedCrosshairTagIndex);

				// If the panel has not yet been clicked
				if (OverlayPanelIsClick == false) {

					// Do not continue
					return;

				}

				// Should the whole rectangle move
				if (CrosshairMoveFlag == true) {

					// Update the rectangle X and Y coordinates
					CrosshairX0[SelectedCrosshairTagIndex] = MouseXPosition;
					CrosshairY0[SelectedCrosshairTagIndex] = MouseYPosition;

				}

			}

			// Limit the position of the crosshair to the texture area
			if (CrosshairX0[SelectedCrosshairTagIndex] <= 1.0) CrosshairX0[SelectedCrosshairTagIndex] = 1.0;
			if (CrosshairY0[SelectedCrosshairTagIndex] <= 1.0) CrosshairY0[SelectedCrosshairTagIndex] = 1.0;
			if (CrosshairX0[SelectedCrosshairTagIndex] > ImageDataPixelWidth) CrosshairX0[SelectedCrosshairTagIndex] = (ImageDataPixelWidth) + 1.0;
			if (CrosshairY0[SelectedCrosshairTagIndex] > ImageDataPixelHeight) CrosshairY0[SelectedCrosshairTagIndex] = (ImageDataPixelHeight) + 1.0;

		}

		CrosshairWLabelPosition RMH_OpenGL_RenderMovableCrossHairWithLabel(unsigned short OrderPriority, System::String^ LabelString, GLubyte CrosshairSelectedColorR, GLubyte CrosshairSelectedColorG, GLubyte CrosshairSelectedColorB, GLubyte CrosshairPassiveColorR, GLubyte CrosshairPassiveColorG, GLubyte CrosshairPassiveColorB) {

			// This routine renders a position-adjustable crosshair, with the associated label

			// Read the temporary array data and sort the kernel array
			CrosshairWLabelPosition RenderedCrossWLabelPosition;

			// Check for the maximum tag order value
			if (OrderPriority < 1 || OrderPriority > _MaxNumberOfMovableCrosshairs) {

				// Write status message in the terminal
				cout << "Order Must Be Higher Then 0 And Less That Max" << endl;

			}
			else {

				// Check the live view rotation setting
				if (LiveViewRotationDegrees == 0) {

					// Read the crosshair position parameters for returning - compensate for live view rotation
					RenderedCrossWLabelPosition.CrosshairX0Pos = CrosshairX0[OrderPriority];
					RenderedCrossWLabelPosition.CrosshairY0Pos = CrosshairY0[OrderPriority];

				}
				if (LiveViewRotationDegrees == 90) {

					// Read the crosshair position parameters for returning - compensate for live view rotation
					RenderedCrossWLabelPosition.CrosshairX0Pos = (ImageDataPixelHeight - CrosshairY0[OrderPriority]) * ImageDataPixelAspectRatio;
					RenderedCrossWLabelPosition.CrosshairY0Pos = CrosshairX0[OrderPriority] / ImageDataPixelAspectRatio;

				}
				if (LiveViewRotationDegrees == 180) {

					// Read the crosshair position parameters for returning - compensate for live view rotation
					RenderedCrossWLabelPosition.CrosshairX0Pos = ImageDataPixelWidth - CrosshairX0[OrderPriority];
					RenderedCrossWLabelPosition.CrosshairY0Pos = ImageDataPixelHeight - CrosshairY0[OrderPriority];
					
				}
				if (LiveViewRotationDegrees == 270) {

					// Read the crosshair position parameters for returning - compensate for live view rotation
					RenderedCrossWLabelPosition.CrosshairX0Pos = CrosshairY0[OrderPriority] * ImageDataPixelAspectRatio;
					RenderedCrossWLabelPosition.CrosshairY0Pos = (ImageDataPixelWidth - CrosshairX0[OrderPriority]) / ImageDataPixelAspectRatio;

				}

				// Store the rendering order of the crosshair with label
				CrosshairOrderIndex[RenderedCrosshairCounter] = OrderPriority;

				// Is this rendered rectangle the last selected rectangle
				if (OrderPriority == SelectedCrosshairTagIndex) {

					// Render the crosshair with label on the texture
					RMH_OpenGL_RenderCrossHairWithLabel(RenderedCrossWLabelPosition.CrosshairX0Pos, RenderedCrossWLabelPosition.CrosshairY0Pos, true, LabelString, CrosshairSelectedColorR, CrosshairSelectedColorG, CrosshairSelectedColorB);

				}
				else {

					// Render the crosshair with label on the texture
					RMH_OpenGL_RenderCrossHairWithLabel(RenderedCrossWLabelPosition.CrosshairX0Pos, RenderedCrossWLabelPosition.CrosshairY0Pos, true, LabelString, CrosshairPassiveColorR, CrosshairPassiveColorG, CrosshairPassiveColorB);

				}

				// Increment the crosshair rendering counter variable
				RenderedCrosshairCounter = RenderedCrosshairCounter + 1;

			}

			// Return the position of the rectangle
			return RenderedCrossWLabelPosition;

		}

		// ------------------------- Texture Mouse Cursor Label Tracking Handling Routines ------------------------ //

		GLvoid RMH_OpenGL_EnableMouseCursorTrackingWLabel(bool EnableFlag) {

			// This routine enables or disables mouse cursor label tracking

			// Update the global variable
			CursorTrackingEnableFlag = EnableFlag;

		}

		GLvoid RMH_OpenGL_UpdateCursorTrackingPosition(GLdouble MouseXPosition, GLdouble MouseYPosition) {

			// This routine handles the update of the cursor label position, if the feature is enabled

			// Read the temporary array data and sort the kernel array
			GLfloat QuadrantXOffset = 0.0;
			GLfloat QuadrantYOffset = 0.0;

			// Is mouse cursor tracking enabled
			if (CursorTrackingEnableFlag == true) {

				// Read the mouse cursor X and Y position
				CursorTrackXPos = MouseXPosition;
				CursorTrackYPos = MouseYPosition;

				// Check whether the position is in quadrant 1
				if (MouseXPosition >= ((GLfloat)ImageDataPixelWidth * 0.5) && MouseYPosition <= ((GLfloat)ImageDataPixelHeight * 0.5)) {
					// Update the quadrant offset values
					QuadrantXOffset = MouseLabelQuadrant1LabelXOffset;
					QuadrantYOffset = MouseLabelQuadrant1LabelYOffset;
				}

				// Check whether the position is in quadrant 2
				if (MouseXPosition <= ((GLfloat)ImageDataPixelWidth * 0.5) && MouseYPosition <= ((GLfloat)ImageDataPixelHeight * 0.5)) {
					// Update the quadrant offset values
					QuadrantXOffset = MovableLineQuadrant2LabelXOffset;
					QuadrantYOffset = MovableLineQuadrant2LabelYOffset;
				}

				// Check whether the position is in quadrant 3
				if (MouseXPosition <= ((GLfloat)ImageDataPixelWidth * 0.5) && MouseYPosition >= ((GLfloat)ImageDataPixelHeight * 0.5)) {
					// Update the quadrant offset values
					QuadrantXOffset = MovableLineQuadrant3LabelXOffset;
					QuadrantYOffset = MovableLineQuadrant3LabelYOffset;
				}

				// Check whether the position is in quadrant 4
				if (MouseXPosition >= ((GLfloat)ImageDataPixelWidth * 0.5) && MouseYPosition >= ((GLfloat)ImageDataPixelHeight * 0.5)) {
					// Update the quadrant offset values
					QuadrantXOffset = MouseLabelQuadrant4LabelXOffset;
					QuadrantYOffset = MouseLabelQuadrant4LabelYOffset;
				}

				// Check the selected aspect ratio setting
				if (LocalAspectRatioFlag == true) {

					// If horizontal aspect ratio compensation is needed
					if (LiveViewPosX0 <= 0.0) {

						// Calculate the crosshair Y coordinate when scaling the texture window - fixed aspect ratio mode
						MouseYPosition = ((GLfloat)MouseYPosition * CurrentPanelHeightFixedAspect * TotalTextureScalableHeight);
						MouseYPosition = MouseYPosition + AspectRatioHeightOffSet;
						
						// Calculate the label X coordinate when scaling the texture window - auto aspect ratio mode
						MouseXPosition = ((GLfloat)MouseXPosition * CurrentTexturePanelWidth * TotalTextureScalableWidth);

					}
					else {

						// Calculate the crosshair Y coordinate when scaling the texture window
						MouseYPosition = ((GLfloat)MouseYPosition * CurrentTexturePanelHeight * TotalTextureScalableHeight);

						// Calculate the label X coordinate when scaling the texture window - fixed aspect ratio mode
						MouseXPosition = ((GLfloat)MouseXPosition * CurrentPanelWidthFixedAspect * TotalTextureScalableWidth);
						MouseXPosition = MouseXPosition + AspectRatioWidthOffSet;

					}

				}
				else {

					// Calculate the crosshair Y coordinate when scaling the texture window
					MouseYPosition = ((GLfloat)MouseYPosition * CurrentTexturePanelHeight * TotalTextureScalableHeight);

					// Calculate the label X coordinate when scaling the texture window - auto aspect ratio mode
					MouseXPosition = ((GLfloat)MouseXPosition * CurrentTexturePanelWidth * TotalTextureScalableWidth);

				}

				// Update the global position variables
				CursorTrackTextureXPos = MouseXPosition + QuadrantXOffset;
				CursorTrackTextureYPos = MouseYPosition + QuadrantYOffset;

			}

		}

		MouseCursorPosition RMH_OpenGL_RenderMouseCursorLabel(System::String^ DisplayString, GLubyte LabelColorR, GLubyte LabelColorG, GLubyte LabelColorB) {

			// This routine renders a label just above the mouse cursor position

			// Read the temporary array data and sort the kernel array
			MouseCursorPosition NewCursorPos;

			// Should a background be rendered for the label
			if (EnableLabelBackgroundFlag == true) {

				// Enable OpenGL 1D texture
				glEnable(GL_TEXTURE_1D);
				glEnable(GL_BLEND);

				// Set the color of the label background
				glColor4ub(LabelBackgroundColorR, LabelBackgroundColorG, LabelBackgroundColorB, CommonLabelBackgroundAlpha);

				// Render the rectangle on the texture
				glBegin(GL_QUADS);

				// Render the positions of the label rectangle
				glVertex2f(CursorTrackTextureXPos - MouseLabelBackgroundXOffset, CursorTrackTextureYPos - MouseLabelBackgroundYOffset);
				glVertex2f(CursorTrackTextureXPos - MouseLabelBackgroundXOffset + MouseLabelBackgroundWidth, CursorTrackTextureYPos - MouseLabelBackgroundYOffset);
				glVertex2f(CursorTrackTextureXPos - MouseLabelBackgroundXOffset + MouseLabelBackgroundWidth, CursorTrackTextureYPos - MouseLabelBackgroundYOffset + MouseLabelBackgroundHeight);
				glVertex2f(CursorTrackTextureXPos - MouseLabelBackgroundXOffset, CursorTrackTextureYPos - MouseLabelBackgroundYOffset + MouseLabelBackgroundHeight);

				// End of configuration
				glEnd();
				// Disable the 1D texture
				glDisable(GL_TEXTURE_1D);
				glDisable(GL_BLEND);

			}

			// Is mouse cursor tracking enabled
			if (CursorTrackingEnableFlag == true) {

				// Render a label just above the mouse cursor position
				RMH_OpenGL_RenderStringOnTexture(CursorTrackTextureXPos, CursorTrackTextureYPos, RMH_Conversion_SystemStringToStdString(DisplayString) + ": X: " + RMH_Conversion_IntToStdString(CursorTrackXPos) + " Y: " + RMH_Conversion_IntToStdString(CursorTrackYPos), LabelColorR, LabelColorG, LabelColorB);

			}

			// Check the live view rotation setting
			if (LiveViewRotationDegrees == 0) {

				// Read the mouse cursor X and Y position
				NewCursorPos.CursorXPos = CursorTrackXPos;
				NewCursorPos.CursorYPos = CursorTrackYPos;

			}
			if (LiveViewRotationDegrees == 90) {

				// Read the mouse cursor X and Y position
				NewCursorPos.CursorXPos = ImageDataPixelWidth - CursorTrackYPos * ImageDataPixelAspectRatio;
				NewCursorPos.CursorYPos = CursorTrackXPos / ImageDataPixelAspectRatio;

			}
			if (LiveViewRotationDegrees == 180) {

				// Read the mouse cursor X and Y position
				NewCursorPos.CursorXPos = ImageDataPixelWidth - CursorTrackXPos;
				NewCursorPos.CursorYPos = ImageDataPixelHeight - CursorTrackYPos;

			}
			if (LiveViewRotationDegrees == 270) {

				// Read the mouse cursor X and Y position
				NewCursorPos.CursorXPos = CursorTrackYPos * ImageDataPixelAspectRatio;
				NewCursorPos.CursorYPos = ImageDataPixelHeight - CursorTrackXPos / ImageDataPixelAspectRatio;

			}

			// Return the mouse cursor position
			return NewCursorPos;

		}

		// ---------------------------- Rectangle Rendering And Handling Routines ----------------------------- //

		bool RMH_OpenGL_IsCursorInsideRectangle(GLdouble MouseXPosition, GLdouble MouseYPosition, GLdouble RectX0, GLdouble RectY0, GLdouble RectWidth, GLdouble RectHeight) {

			// This routine checks whether the mouse cursor is within the rectangle area

			// Read the temporary array data and sort the kernel array
			bool IsInsideStatus = false;

			// Check whether the mouse cursor is inside the area of the rectangle - width
			if (MouseXPosition >= RectX0 - CursorChangeOffset &&
				MouseXPosition <= RectX0 + RectWidth + CursorChangeOffset) {

				// Check whether the mouse cursor is inside the area of the rectangle - height
				if (MouseYPosition >= RectY0 - CursorChangeOffset &&
					MouseYPosition <= RectY0 + RectHeight + CursorChangeOffset) {

					// Update the cursor position status
					IsInsideStatus = true;

				}

			}

			// Return the cursor position status
			return IsInsideStatus;

		}

		RectangelSizableSides RMH_OpenGL_GetSellectedRectangelSizableSide(GLdouble MouseXPosition, GLdouble MouseYPosition, unsigned short RectTag) {

			// This routine reads and returns which side of the rectangle the mouse is positioned at.

			// Read the temporary array data and sort the kernel array
			RectangelSizableSides ReturnedSide = RectangelSizableSides::None;

			// Check whether the top rectangle line is selected
			if (MouseYPosition >= RectY0[RectTag] - CursorChangeOffset &&
				MouseYPosition <= RectY0[RectTag] + CursorChangeOffset &&
				MouseXPosition >= RectX0[RectTag] &&
				MouseXPosition <= RectX0[RectTag] + RectWidth[RectTag]) {

				// Update the returned selected line enum value
				ReturnedSide = RectangelSizableSides::TopLine;

			}

			// Check whether the bottom rectangle line is selected
			if (MouseYPosition >= (RectY0[RectTag] + RectHeight[RectTag]) - CursorChangeOffset &&
				MouseYPosition <= (RectY0[RectTag] + RectHeight[RectTag]) + CursorChangeOffset &&
				MouseXPosition + RectHeight[RectTag] >= RectX0[RectTag] + RectHeight[RectTag] &&
				MouseXPosition + RectHeight[RectTag] <= RectX0[RectTag] + RectHeight[RectTag] + RectWidth[RectTag]) {

				// Update the returned selected line enum value
				ReturnedSide = RectangelSizableSides::BottomLine;

			}

			// Check whether the left rectangle line is selected
			if (MouseXPosition >= RectX0[RectTag] - CursorChangeOffset &&
				MouseXPosition <= RectX0[RectTag] + CursorChangeOffset &&
				MouseYPosition >= RectY0[RectTag] &&
				MouseYPosition <= RectY0[RectTag] + RectHeight[RectTag]) {

				// Update the returned selected line enum value
				ReturnedSide = RectangelSizableSides::LeftLine;

			}

			// Check whether the right rectangle line is selected
			if (MouseXPosition >= (RectX0[RectTag] + RectWidth[RectTag]) - CursorChangeOffset &&
				MouseXPosition <= (RectX0[RectTag] + RectWidth[RectTag]) + CursorChangeOffset &&
				MouseYPosition + RectWidth[RectTag] >= RectY0[RectTag] + RectWidth[RectTag] &&
				MouseYPosition + RectWidth[RectTag] <= RectY0[RectTag] + RectHeight[RectTag] + RectWidth[RectTag]) {

				// Update the returned selected line enum value
				ReturnedSide = RectangelSizableSides::RightLine;

			}

			// Return the line enum value
			return ReturnedSide;

		}

		System::Windows::Forms::Cursor^ RMH_OpenGL_GetRectangelCursor(RectangelSizableSides RectangelSide) {

			// This routine returns the relevant associated cursor for a given rectangle side 

			// Selection of the rectangle side
			switch (RectangelSide) {

				// Return the relevant cursor
				case RectangelSizableSides::TopLine: return Cursors::SizeNS;
				case RectangelSizableSides::BottomLine: return Cursors::SizeNS;
				case RectangelSizableSides::LeftLine: return Cursors::SizeWE;
				case RectangelSizableSides::RightLine: return Cursors::SizeWE;
				default: return Cursors::Default;

			}

		}

		GLvoid RMH_OpenGL_ChangeRectangelCursor(GLdouble MouseXPosition, GLdouble MouseYPosition, unsigned short RectTag) {

			// This routine updates the cursor of the overlaid panel, depending on which rectangle side the mouse touches

			// Update the cursor of the overlaid panel
			OverlayPanel->Cursor = RMH_OpenGL_GetRectangelCursor(RMH_OpenGL_GetSellectedRectangelSizableSide(MouseXPosition, MouseYPosition, RectTag));

		}

		GLvoid RMH_OpenGL_HandleRectangleMouseDownEvents(GLdouble MouseXPosition, GLdouble MouseYPosition) {

			// This routine handles events and states when a position-adjustable rectangle is clicked

			// Reset the selected rectangle index value
			SelectedRectTagIndex = 0;

			// If a rendered crosshair is not selected
			if (CrosshairMoveFlag == false && LineMoveFlag == false) {

				// Loop through all active rectangles
				for (unsigned short i = 0; i < NmbOfActiveRects; i++) {

					// Check whether the mouse cursor is inside the area of the active rectangle 
					// The highest-priority rectangle is always selected first in the layering 
					if (RMH_OpenGL_IsCursorInsideRectangle(MouseXPosition, MouseYPosition, RectX0[RectOrderIndex[i]], RectY0[RectOrderIndex[i]], RectWidth[RectOrderIndex[i]], RectHeight[RectOrderIndex[i]])) {

						// Update the rectangle panel move flag
						RectangleMoveFlag = true;

						// Store the tag index of the selected rectangle
						SelectedRectTagIndex = RectOrderIndex[i];

						// Break the for loop
						break;

					}

				}
			}

			// Update the "old state" number of rendered rectangles variable
			OldNmbOfActiveRects = NmbOfActiveRects;

			// Reset the selected rectangle side enum
			SelectedRectSide = RectangelSizableSides::None;
			// Read which side of the rectangle has been selected
			SelectedRectSide = RMH_OpenGL_GetSellectedRectangelSizableSide(MouseXPosition, MouseYPosition, SelectedRectTagIndex);

			// Read the current rectangle coordinates/positions on a new click
			ClickRectXPosition = RectX0[SelectedRectTagIndex];
			ClickRectYPosition = RectY0[SelectedRectTagIndex];
			ClickRectWidthPosition = RectWidth[SelectedRectTagIndex];
			ClickRectHeightPosition = RectHeight[SelectedRectTagIndex];
			ClickRectXPositionOffset = MouseXPosition - RectX0[SelectedRectTagIndex];
			ClickRectYPositionOffset = MouseYPosition - RectY0[SelectedRectTagIndex];

		}

		GLvoid RMH_OpenGL_HandleRectangleMouseMoveEvents(GLdouble MouseXPosition, GLdouble MouseYPosition) {

			// This routine handles events and states when a clicked rectangle is to move

			// Add the rectangle click offset to the mouse position
			MouseXPosition = MouseXPosition - ClickRectXPositionOffset;
			MouseYPosition = MouseYPosition - ClickRectYPositionOffset;

			// Update the cursor if no active object is in the mouse focus
			if (OldNmbOfActiveRects != NmbOfActiveRects) {

				// Update the cursor type of the overlaid panel
				RMH_OpenGL_ChangeRectangelCursor(MouseXPosition + ClickRectXPositionOffset, MouseYPosition + ClickRectYPositionOffset, 0);

				// Update the "old state" number of rendered rectangles variable
				OldNmbOfActiveRects = NmbOfActiveRects;

				// Reset the selected rectangle index value
				SelectedRectTagIndex = 0;

			}

			// Check the number of active rectangles and the selected index
			if (NmbOfActiveRects > 0 && SelectedRectTagIndex != 0) {

				// Update the cursor type of the overlaid panel
				RMH_OpenGL_ChangeRectangelCursor(MouseXPosition + ClickRectXPositionOffset, MouseYPosition + ClickRectYPositionOffset, SelectedRectTagIndex);

				// If the panel has not yet been clicked
				if (OverlayPanelIsClick == false) {

					// Do not continue
					return;

				}

				// Which rectangle side position should be updated
				switch (SelectedRectSide) {

					// -------------------------------------------------------------------------------------- //

					// Update the top line position
					case RectangelSizableSides::TopLine:

						// Update the Y position of the rectangle
						RectY0[SelectedRectTagIndex] = MouseYPosition;
						// Update the height value of the rectangle from the new Y position
						RectHeight[SelectedRectTagIndex] = ClickRectHeightPosition + (ClickRectYPosition - RectY0[SelectedRectTagIndex]);

						// Should the rectangle be set to a fixed aspect ratio
						if (RectFixedAspectRatioFlags[SelectedRectTagIndex] == true) {

							// Update the width value of the rectangle from the new X position
							RectWidth[SelectedRectTagIndex] = RectHeight[SelectedRectTagIndex] * ImageDataPixelAspectRatio;

							// Limit the size to the minimum rectangle size
							if (RectWidth[SelectedRectTagIndex] <= MinimumRectWidth) {

								// Set the rectangle height to the minimum height
								RectWidth[SelectedRectTagIndex] = MinimumRectWidth;

							}

						}
		
						// Limit the size to the minimum rectangle size
						if (RectY0[SelectedRectTagIndex] >= (RectY0[SelectedRectTagIndex] + RectHeight[SelectedRectTagIndex]) - MinimumRectHeight) {

							// Update the Y position of the rectangle at the minimum size
							RectY0[SelectedRectTagIndex] = (RectY0[SelectedRectTagIndex] + RectHeight[SelectedRectTagIndex]) - MinimumRectHeight;
							// Update the height value of the rectangle from the minimum Y position
							RectHeight[SelectedRectTagIndex] = ClickRectHeightPosition + (ClickRectYPosition - RectY0[SelectedRectTagIndex]);

						}

					break;

					// -------------------------------------------------------------------------------------- //

					// Update the bottom line position
					case RectangelSizableSides::BottomLine:

						// Update the height of the rectangle 
						RectHeight[SelectedRectTagIndex] = ClickRectHeightPosition - (ClickRectYPosition - MouseYPosition);

						// Should the rectangle be set to a fixed aspect ratio
						if (RectFixedAspectRatioFlags[SelectedRectTagIndex] == true) {

							// Update the height of the rectangle 
							RectWidth[SelectedRectTagIndex] = RectHeight[SelectedRectTagIndex] * ImageDataPixelAspectRatio;

							// Limit the size to the minimum rectangle size
							if (RectWidth[SelectedRectTagIndex] <= MinimumRectWidth) {

								// Set the rectangle height to the minimum height
								RectWidth[SelectedRectTagIndex] = MinimumRectWidth;

							}

						}
						
						// Limit the size to the minimum rectangle size
						if (RectHeight[SelectedRectTagIndex] <= MinimumRectHeight) {

							// Set the rectangle height to the minimum height
							RectHeight[SelectedRectTagIndex] = MinimumRectHeight;

						}

					break;

					// -------------------------------------------------------------------------------------- //

					// Update the left line position
					case RectangelSizableSides::LeftLine:

						// Update the X position of the rectangle
						RectX0[SelectedRectTagIndex] = MouseXPosition;
						// Update the width value of the rectangle from the new X position
						RectWidth[SelectedRectTagIndex] = ClickRectWidthPosition + (ClickRectXPosition - RectX0[SelectedRectTagIndex]);

						// Should the rectangle be set to a fixed aspect ratio
						if (RectFixedAspectRatioFlags[SelectedRectTagIndex] == true) {

							// Update the width of the rectangle 
							RectHeight[SelectedRectTagIndex] = RectWidth[SelectedRectTagIndex] * ImageDataPixelAspectRatioReciprok;

							// Limit the size to the minimum rectangle size
							if (RectHeight[SelectedRectTagIndex] <= MinimumRectHeight) {

								// Set the rectangle height to the minimum height
								RectHeight[SelectedRectTagIndex] = MinimumRectHeight;

							}

						}
		
						// Limit the size to the minimum rectangle size
						if (RectX0[SelectedRectTagIndex] >= (RectX0[SelectedRectTagIndex] + RectWidth[SelectedRectTagIndex]) - MinimumRectWidth) {

							// Update the X position of the rectangle at the minimum size
							RectX0[SelectedRectTagIndex] = (RectX0[SelectedRectTagIndex] + RectWidth[SelectedRectTagIndex]) - MinimumRectWidth;
							// Update the width value of the rectangle from the minimum X position
							RectWidth[SelectedRectTagIndex] = ClickRectWidthPosition + (ClickRectXPosition - RectX0[SelectedRectTagIndex]);

						}

					break;

					// -------------------------------------------------------------------------------------- //

					// Update the right line position
					case RectangelSizableSides::RightLine:

						// Update the width of the rectangle 
						RectWidth[SelectedRectTagIndex] = ClickRectWidthPosition - (ClickRectXPosition - MouseXPosition);

						// Should the rectangle be set to a fixed aspect ratio
						if (RectFixedAspectRatioFlags[SelectedRectTagIndex] == true) {

							// Update the width of the rectangle 
							RectHeight[SelectedRectTagIndex] = RectWidth[SelectedRectTagIndex] * ImageDataPixelAspectRatioReciprok;

							// Limit the size to the minimum rectangle size
							if (RectHeight[SelectedRectTagIndex] <= MinimumRectHeight) {

								// Set the rectangle height to the minimum height
								RectHeight[SelectedRectTagIndex] = MinimumRectHeight;

							}

						}
		
						// Limit the size to the minimum rectangle size
						if (RectWidth[SelectedRectTagIndex] <= MinimumRectWidth) {

							// Set the rectangle width to the minimum width
							RectWidth[SelectedRectTagIndex] = MinimumRectWidth;

						}
				
					break;

					// -------------------------------------------------------------------------------------- //

					// Update the whole rectangle position
					default:

						// Should the whole rectangle move
						if (RectangleMoveFlag == true) {

							// Update the rectangle X and Y coordinates
							RectX0[SelectedRectTagIndex] = MouseXPosition;
							RectY0[SelectedRectTagIndex] = MouseYPosition;

						}

					break;

					// -------------------------------------------------------------------------------------- //

				}

				// Limit the position of the rectangle to the texture area
				if (RectX0[SelectedRectTagIndex] <= ROIRectangleLiveViewBorderPixelPadding) RectX0[SelectedRectTagIndex] = ROIRectangleLiveViewBorderPixelPadding;
				if (RectY0[SelectedRectTagIndex] <= ROIRectangleLiveViewBorderPixelPadding) RectY0[SelectedRectTagIndex] = ROIRectangleLiveViewBorderPixelPadding;
				if (RectWidth[SelectedRectTagIndex] <= ROIRectangleLiveViewBorderPixelPadding) RectWidth[SelectedRectTagIndex] = ROIRectangleLiveViewBorderPixelPadding;
				if (RectHeight[SelectedRectTagIndex] <= ROIRectangleLiveViewBorderPixelPadding) RectHeight[SelectedRectTagIndex] = ROIRectangleLiveViewBorderPixelPadding;
				if (RectWidth[SelectedRectTagIndex] >= ImageDataPixelWidth - ROIRectangleLiveViewBorderPixelPadding) RectWidth[SelectedRectTagIndex] = ImageDataPixelWidth - ROIRectangleLiveViewBorderPixelPadding;
				if (RectHeight[SelectedRectTagIndex] >= ImageDataPixelHeight - ROIRectangleLiveViewBorderPixelPadding) RectHeight[SelectedRectTagIndex] = ImageDataPixelHeight - ROIRectangleLiveViewBorderPixelPadding;
				if (RectX0[SelectedRectTagIndex] + RectWidth[SelectedRectTagIndex] >= ImageDataPixelWidth) RectX0[SelectedRectTagIndex] = (ImageDataPixelWidth - ROIRectangleLiveViewBorderPixelPadding) - RectWidth[SelectedRectTagIndex];
				if (RectY0[SelectedRectTagIndex] + RectHeight[SelectedRectTagIndex] >= ImageDataPixelHeight) RectY0[SelectedRectTagIndex] = (ImageDataPixelHeight - ROIRectangleLiveViewBorderPixelPadding) - RectHeight[SelectedRectTagIndex];

			}

		}

		RectangelPosition RMH_OpenGL_RenderMovableRectangle(unsigned short OrderPriority, std::string RectangleTitle, GLubyte SelectedColorR, GLubyte SelectedColorG, GLubyte SelectedColorB,GLubyte PassiveColorR, GLubyte PassiveColorG, GLubyte PassiveColorB, bool FixedAspectRatioFlag) {

			// This routine renders a position-adjustable rectangle
		
			// Read the temporary array data and sort the kernel array
			RectangelPosition RenderedRectPosition;

			// Check for the maximum tag order value
			if (OrderPriority < 1 || OrderPriority > _MaxNumberOfMovableRectangles) {

				// Write status message in the terminal
				cout << "Order Must Be Higher Then 0 And Less Than Max" << endl;

			}
			else {

				// Read the position parameters of the rectangle for returning - before rendering
				RenderedRectPosition.RectangleX0Pos = RectX0[OrderPriority];
				RenderedRectPosition.RectangleY0Pos = RectY0[OrderPriority];
				RenderedRectPosition.RectangleWidth = RectWidth[OrderPriority];
				RenderedRectPosition.RectangleHeight = RectHeight[OrderPriority];

				// Store the rendering order of the rectangle
				RectOrderIndex[RenderedRectanglesCounter] = OrderPriority;
				// Store the fixed aspect ratio enable flag of the rectangle
				RectFixedAspectRatioFlags[OrderPriority] = FixedAspectRatioFlag;

				// Check the selected aspect ratio setting
				if (LocalAspectRatioFlag == true) {

					// If horizontal aspect ratio compensation is needed
					if (LiveViewPosX0 <= 0.0) {

						// Calculate the rectangle coordinates when scaling the texture window
						RenderedRectPosition.RectangleX0Pos = RenderedRectPosition.RectangleX0Pos * CurrentTexturePanelWidth * TotalTextureScalableWidth;
						RenderedRectPosition.RectangleWidth = RenderedRectPosition.RectangleX0Pos + (RenderedRectPosition.RectangleWidth * CurrentTexturePanelWidth * TotalTextureScalableWidth);

						// Calculate the common rectangle coordinates when scaling the texture window - fixed aspect ratio mode
						RenderedRectPosition.RectangleY0Pos = RenderedRectPosition.RectangleY0Pos * CurrentPanelHeightFixedAspect * TotalTextureScalableHeight;
						RenderedRectPosition.RectangleHeight = RenderedRectPosition.RectangleY0Pos + (RenderedRectPosition.RectangleHeight * CurrentPanelHeightFixedAspect * TotalTextureScalableHeight);
						RenderedRectPosition.RectangleY0Pos = RenderedRectPosition.RectangleY0Pos + AspectRatioHeightOffSet;
						RenderedRectPosition.RectangleHeight = RenderedRectPosition.RectangleHeight + AspectRatioHeightOffSet;

					}
					else {

						// Calculate the common rectangle coordinates when scaling the texture window
						RenderedRectPosition.RectangleY0Pos = RenderedRectPosition.RectangleY0Pos * CurrentTexturePanelHeight * TotalTextureScalableHeight;
						RenderedRectPosition.RectangleHeight = RenderedRectPosition.RectangleY0Pos + (RenderedRectPosition.RectangleHeight * CurrentTexturePanelHeight * TotalTextureScalableHeight);

						// Calculate the rectangle coordinates when scaling the texture window - fixed aspect ratio mode
						RenderedRectPosition.RectangleX0Pos = RenderedRectPosition.RectangleX0Pos * CurrentPanelWidthFixedAspect * TotalTextureScalableWidth;
						RenderedRectPosition.RectangleWidth = RenderedRectPosition.RectangleX0Pos + (RenderedRectPosition.RectangleWidth * CurrentPanelWidthFixedAspect * TotalTextureScalableWidth);
						RenderedRectPosition.RectangleX0Pos = RenderedRectPosition.RectangleX0Pos + AspectRatioWidthOffSet;
						RenderedRectPosition.RectangleWidth = RenderedRectPosition.RectangleWidth + AspectRatioWidthOffSet;

					}

				}
				else {

					// Calculate the common rectangle coordinates when scaling the texture window
					RenderedRectPosition.RectangleY0Pos = RenderedRectPosition.RectangleY0Pos * CurrentTexturePanelHeight * TotalTextureScalableHeight;
					RenderedRectPosition.RectangleHeight = RenderedRectPosition.RectangleY0Pos + (RenderedRectPosition.RectangleHeight * CurrentTexturePanelHeight * TotalTextureScalableHeight);

					// Calculate the rectangle coordinates when scaling the texture window - auto aspect ratio mode
					RenderedRectPosition.RectangleX0Pos = RenderedRectPosition.RectangleX0Pos * CurrentTexturePanelWidth * TotalTextureScalableWidth;
					RenderedRectPosition.RectangleWidth = RenderedRectPosition.RectangleX0Pos + (RenderedRectPosition.RectangleWidth * CurrentTexturePanelWidth * TotalTextureScalableWidth);

				}

				// Enable OpenGL 1D texture
				glEnable(GL_TEXTURE_1D);

				// Is this rendered rectangle the last selected rectangle
				if (OrderPriority == SelectedRectTagIndex) {

					// Update the color of the rectangle to the "selected" color
					glColor3ub(SelectedColorR, SelectedColorG, SelectedColorB);

					// Display and update the rectangle ID text and text color
					RMH_OpenGL_RenderStringOnTexture(RenderedRectPosition.RectangleX0Pos + ROIIdentifierLabelXPixelOffset, RenderedRectPosition.RectangleY0Pos + ROIIdentifierLabelYPixelOffset, RectangleTitle, SelectedColorR, SelectedColorG, SelectedColorB);

				}
				else {

					// Update the color of the rectangle to the "default" color
					glColor3ub(PassiveColorR, PassiveColorG, PassiveColorB);

					// Display and update the rectangle ID text and text color
					RMH_OpenGL_RenderStringOnTexture(RenderedRectPosition.RectangleX0Pos + ROIIdentifierLabelXPixelOffset, RenderedRectPosition.RectangleY0Pos + ROIIdentifierLabelYPixelOffset, RectangleTitle, PassiveColorR, PassiveColorG, PassiveColorB);

				}

				// Set the rectangle line thickness
				glLineWidth(DefaultRectLineWidth);

				// Render the line on the texture
				glBegin(GL_LINES);

				// Top rectangle lines
				glVertex2f(RenderedRectPosition.RectangleX0Pos, RenderedRectPosition.RectangleY0Pos);
				glVertex2f(RenderedRectPosition.RectangleWidth, RenderedRectPosition.RectangleY0Pos);
				// Right rectangle lines
				glVertex2f(RenderedRectPosition.RectangleWidth, RenderedRectPosition.RectangleY0Pos);
				glVertex2f(RenderedRectPosition.RectangleWidth, RenderedRectPosition.RectangleHeight);
				// Bottom rectangle lines
				glVertex2f(RenderedRectPosition.RectangleWidth, RenderedRectPosition.RectangleHeight);
				glVertex2f(RenderedRectPosition.RectangleX0Pos, RenderedRectPosition.RectangleHeight);
				// Left rectangle lines
				glVertex2f(RenderedRectPosition.RectangleX0Pos, RenderedRectPosition.RectangleHeight);
				glVertex2f(RenderedRectPosition.RectangleX0Pos, RenderedRectPosition.RectangleY0Pos);

				// End of configuration
				glEnd();
				// Disable the 1D texture
				glDisable(GL_TEXTURE_1D);
		
				// Check the live view rotation setting
				if (LiveViewRotationDegrees == 0) {

					// Update the position coordinate values
					RenderedRectPosition.RectangleX0Pos = RectX0[OrderPriority];
					RenderedRectPosition.RectangleY0Pos = RectY0[OrderPriority];
					RenderedRectPosition.RectangleWidth = RectWidth[OrderPriority];
					RenderedRectPosition.RectangleHeight = RectHeight[OrderPriority];

				}
				if (LiveViewRotationDegrees == 90) {

					// Update the position coordinate values
					RenderedRectPosition.RectangleX0Pos = ImageDataPixelWidth - ((RectY0[OrderPriority] + RectHeight[OrderPriority]) * ImageDataPixelAspectRatio);
					RenderedRectPosition.RectangleY0Pos = RectX0[OrderPriority] / ImageDataPixelAspectRatio;
					RenderedRectPosition.RectangleWidth = RectHeight[OrderPriority] * ImageDataPixelAspectRatio;
					RenderedRectPosition.RectangleHeight = RectWidth[OrderPriority] / ImageDataPixelAspectRatio;

				}
				if (LiveViewRotationDegrees == 180) {

					// Update the position coordinate values
					RenderedRectPosition.RectangleX0Pos = (ImageDataPixelWidth - RectX0[OrderPriority]) - RectWidth[OrderPriority];
					RenderedRectPosition.RectangleY0Pos = (ImageDataPixelHeight - RectY0[OrderPriority]) - RectHeight[OrderPriority];
					RenderedRectPosition.RectangleWidth = RectWidth[OrderPriority];
					RenderedRectPosition.RectangleHeight = RectHeight[OrderPriority];

				}
				if (LiveViewRotationDegrees == 270) {
					
					// Update the position coordinate values
					RenderedRectPosition.RectangleX0Pos = RectY0[OrderPriority] * ImageDataPixelAspectRatio;
					RenderedRectPosition.RectangleY0Pos = ImageDataPixelHeight - (RectX0[OrderPriority] + RectWidth[OrderPriority]) / ImageDataPixelAspectRatio;
					RenderedRectPosition.RectangleWidth = RectHeight[OrderPriority] * ImageDataPixelAspectRatio;
					RenderedRectPosition.RectangleHeight = RectWidth[OrderPriority] / ImageDataPixelAspectRatio;

				}

				// Round the position parameters up to the nearest whole pixel integer value
				RenderedRectPosition.RectangleHeight = RMH_Math_Round(RenderedRectPosition.RectangleHeight);
				RenderedRectPosition.RectangleWidth = RMH_Math_Round(RenderedRectPosition.RectangleWidth);
				RenderedRectPosition.RectangleX0Pos = RMH_Math_Round(RenderedRectPosition.RectangleX0Pos);
				RenderedRectPosition.RectangleY0Pos = RMH_Math_Round(RenderedRectPosition.RectangleY0Pos);
				
				// Increment the rectangle rendering counter variable
				RenderedRectanglesCounter = RenderedRectanglesCounter + 1;

			}

			// Return the position of the rectangle
			return RenderedRectPosition;

		}

		// ----------------------------- Image Rendering And Handling Routines ------------------------------- //

		GLvoid RMH_OpenGL_RotateLiveViewCCW() {

			// This routine sets the rotation value of the live view image from 0 to 360 degrees - counter clockwise.

			// Increment the live view image rotation by 90 degrees
			LiveViewRotationDegrees = LiveViewRotationDegrees + 90.0;

			// If the live view image rotation is over 270 degrees
			if (LiveViewRotationDegrees > 270.0) {

				// Reset the live view image rotation
				LiveViewRotationDegrees = 0.0;

			}

			// Update the "live view rotation has changed" flag
			LiveViewRotationChangedFlag = true;

		}

		GLvoid RMH_OpenGL_RotateLiveViewCW() {

			// This routine sets the rotation value of the live view image from 0 to 360 degrees - clockwise.

			// Increment the live view image rotation by 90 degrees
			LiveViewRotationDegrees = LiveViewRotationDegrees - 90.0;

			// If the live view image rotation is equal to 0 degrees
			if (LiveViewRotationDegrees < 0.0) {

				// Reset the live view image rotation
				LiveViewRotationDegrees = 270.0;

			}

			// Update the "live view rotation has changed" flag
			LiveViewRotationChangedFlag = true;

		}

		bool RMH_LiveView_HasRotationChanged() {

			// This routine returns a status flag indicating whether the live view rotation has changed

			// Has the live view rotation changed
			if (LiveViewRotationChangedFlag == true) {

				// Reset the "live view rotation has changed" flag
				LiveViewRotationChangedFlag = false;

				// Return the status
				return true;

			}
			else {

				// Return the status
				return false;

			}

		}

		GLdouble RMH_LiveView_GetRotation() {

			// This routine reads and returns the current live view rotation in degrees
			 
			// Return the current live view rotation in degrees
			return LiveViewRotationDegrees;

		}

		GLdouble RMH_LiveView_GetNativeImageWidth() {

			// This routine returns the native pixel width of the live view image
			return ImageDataPixelWidth;

		}

		GLdouble RMH_LiveView_GetNativeImageHeight() {

			// This routine returns the native pixel height of the live view image
			return ImageDataPixelHeight;

		}

		GLdouble RMH_LiveView_GetNativeImageAspectRatio() {

			// This routine returns the native pixel height of the live view image
			return ImageDataPixelAspectRatio;

		}

		GLvoid RMH_LiveView_EnableScrollWheelRotation(bool EnableFlag) {

			// This routine is used to enable or disable live view image rotation using the mouse scroll wheel

			// Enable or disable live view image rotation using the mouse scroll wheel
			LiveViewMouseScrollWheelRotationEnablFlag = EnableFlag;

		}

		GLvoid RMH_LiveViewStream_UltraResolutionMode(bool EnableExtremeResolutionFlag) {

			// This routine sets and enables/disables the "ultra resolution" mode of the live view stream

			// Enable the ultra resolution feature
			if (EnableExtremeResolutionFlag == true) {

				// Set the live view frame offset value of the ultra resolution mode
				UltraResolutionFrameOffsetValue = 1.0;

			}
			else {

				// Set the live view frame offset value of the ultra resolution mode
				UltraResolutionFrameOffsetValue = 0.5;

			}

		}

		GLvoid RMH_OpenGL_RenderImageTexture(GLdouble TexturePanelWidth, GLdouble TexturePanelHeight, GLdouble FrameWidth, GLdouble FrameHeight, bool FixedAspectRatio) {

			// This routine renders the configured texture in a given area of the total allocated texture

			// Reset the live view stream image coordinates
			LiveViewPosX0 = 0.0;
			LiveViewPosY0 = 0.0;
			LiveViewPosX1 = 0.0;
			LiveViewPosY1 = 0.0;

			// Reset the aspect ratio offset parameter
			AspectRatioWidthOffSet = 0.0;
			// Update the local class aspect ratio status flag
			LocalAspectRatioFlag = FixedAspectRatio;

			// Read the current pixel height and width of the texture panel 
			CurrentTexturePanelHeight = TexturePanelHeight;
			CurrentTexturePanelWidth = TexturePanelWidth;

			// Update the pixel height, width and aspect ratio of the image data
			ImageDataPixelWidth = FrameWidth;
			ImageDataPixelHeight = FrameHeight;
			ImageDataPixelAspectRatio = ImageDataPixelWidth / ImageDataPixelHeight;
			ImageDataPixelAspectRatioReciprok = 1.0 / ImageDataPixelAspectRatio;

			// Calculate the Y1 position of the image to fill the texture window
			LiveViewPosY1 = (FrameHeight * CurrentTexturePanelHeight * TotalTextureScalableHeight);

			// Check the selected aspect ratio setting
			if (FixedAspectRatio == true) {

				// Check the live view rotation setting
				if (LiveViewRotationDegrees == 90 || LiveViewRotationDegrees == 270) {

					// Calculate the aspect-ratio-compensated pixel width and height of the texture panel - for 90 and 270 degree rotation
					CurrentPanelWidthFixedAspect = (FrameHeight * CurrentTexturePanelHeight) / FrameWidth;
					CurrentPanelHeightFixedAspect = (FrameWidth * CurrentTexturePanelWidth) / FrameHeight;

				}
				else {

					// Calculate the aspect-ratio-compensated pixel width and height of the texture panel 
					CurrentPanelWidthFixedAspect = (FrameWidth * CurrentTexturePanelHeight) / FrameHeight;
					CurrentPanelHeightFixedAspect = (FrameHeight * CurrentTexturePanelWidth) / FrameWidth;

				}

				// Scale the aspect ratio width/height offset to the actual frame data aspect ratio offset - divide by 2 for the combined right and left margin image offset
				AspectRatioWidthOffSet = (FrameWidth * (CurrentTexturePanelWidth - CurrentPanelWidthFixedAspect) * TotalTextureScalableWidth) * 0.5;
				AspectRatioHeightOffSet = (FrameHeight * (CurrentTexturePanelHeight - CurrentPanelHeightFixedAspect) * TotalTextureScalableHeight) * 0.5;

				// Calculate the X1 position of the image to fill the texture window in fixed aspect ratio mode - add the X1 aspect ratio margin on the right side of the texture window
				LiveViewPosX1 = (FrameWidth * CurrentPanelWidthFixedAspect * TotalTextureScalableWidth) + AspectRatioWidthOffSet;

				// Add the X0 aspect ratio margin on the left side of the texture window
				LiveViewPosX0 = AspectRatioWidthOffSet;

				// Compensate for fixed aspect ratio in the horizontal direction
				if (LiveViewPosX0 <= 0.0) {

					// Reset the X0 position
					LiveViewPosX0 = 0.0;
					// Remove the aspect ratio margin on the right side of the texture window
					LiveViewPosX1 = LiveViewPosX1 + AspectRatioWidthOffSet;

					// Add the Y1 aspect ratio margin at the bottom of the texture window
					LiveViewPosY1 = LiveViewPosY1 - AspectRatioHeightOffSet;
					// Add the Y0 aspect ratio margin at the top of the texture window
					LiveViewPosY0 = AspectRatioHeightOffSet;

				}

			}
			else {

				// Calculate the X1 position of the image to fill the texture window
				LiveViewPosX1 = (FrameWidth * CurrentTexturePanelWidth * TotalTextureScalableWidth);

			}

			// Clear the texture color and bit buffers
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Enable OpenGL 2D texture
			glEnable(GL_TEXTURE_2D);
			// Bind the texture as a 2D texture
			glBindTexture(GL_TEXTURE_2D, ImageTecture[0]);

			// Rotate the texture to match the correct image orientation
			glTranslatef(0.0f, FrameHeight, 0.0f);
			glRotatef(180.0f, 1.0f, 0.0f, 0.0f);

			// Begin rendering
			glBegin(GL_QUADS);

			// Check the live view rotation setting
			if (LiveViewRotationDegrees == 0) {

				// Update the rendered texture coordinates - 0 degrees rotation
				glTexCoord2f(0.0, 0.0);
				glVertex2f(LiveViewPosX0, LiveViewPosY0);
				glTexCoord2f(UltraResolutionFrameOffsetValue, 0.0);
				glVertex2f(LiveViewPosX1, LiveViewPosY0);
				glTexCoord2f(UltraResolutionFrameOffsetValue, UltraResolutionFrameOffsetValue);
				glVertex2f(LiveViewPosX1, LiveViewPosY1);
				glTexCoord2f(0.0, UltraResolutionFrameOffsetValue);
				glVertex2f(LiveViewPosX0, LiveViewPosY1);

			}
			else if (LiveViewRotationDegrees == 90) {

				// Update the rendered texture coordinates - 90 degrees rotation
				glTexCoord2f(0.0, 0.0);
				glVertex2f(LiveViewPosX0, LiveViewPosY1);
				glTexCoord2f(UltraResolutionFrameOffsetValue, 0.0);
				glVertex2f(LiveViewPosX0, LiveViewPosY0);
				glTexCoord2f(UltraResolutionFrameOffsetValue, UltraResolutionFrameOffsetValue);
				glVertex2f(LiveViewPosX1, LiveViewPosY0);
				glTexCoord2f(0.0, UltraResolutionFrameOffsetValue);
				glVertex2f(LiveViewPosX1, LiveViewPosY1);
		
			}
			else if (LiveViewRotationDegrees == 180) {
				
				// Update the rendered texture coordinates - 180 degrees rotation
				glTexCoord2f(0.0, 0.0);
				glVertex2f(LiveViewPosX1, LiveViewPosY1);
				glTexCoord2f(UltraResolutionFrameOffsetValue, 0.0);
				glVertex2f(LiveViewPosX0, LiveViewPosY1);
				glTexCoord2f(UltraResolutionFrameOffsetValue, UltraResolutionFrameOffsetValue);
				glVertex2f(LiveViewPosX0, LiveViewPosY0);
				glTexCoord2f(0.0, UltraResolutionFrameOffsetValue);
				glVertex2f(LiveViewPosX1, LiveViewPosY0);

			}
			else {

				// Update the rendered texture coordinates - 270 degrees rotation
				glTexCoord2f(0.0, 0.0);
				glVertex2f(LiveViewPosX1, LiveViewPosY0);
				glTexCoord2f(UltraResolutionFrameOffsetValue, 0.0);
				glVertex2f(LiveViewPosX1, LiveViewPosY1);
				glTexCoord2f(UltraResolutionFrameOffsetValue, UltraResolutionFrameOffsetValue);
				glVertex2f(LiveViewPosX0, LiveViewPosY1);
				glTexCoord2f(0.0, UltraResolutionFrameOffsetValue);
				glVertex2f(LiveViewPosX0, LiveViewPosY0);

			}

			// End of configuration
			glEnd();
			// Disable 2D texture
			glDisable(GL_TEXTURE_2D);

		}
		
		GLvoid RMH_OpenGL_RenderGrayscale16BitImageData(unsigned int TexturePanelWidth, unsigned int TexturePanelHeight, unsigned short* FrameData, unsigned int FrameWidth, unsigned int FrameHeight, bool FixedAspectRatio) {

			// This routine handles the OpenGL rendering of the image data to the texture handler object
			
			// Make the associated render context the current render context
			RMH_OpenGL_MakeRenderContextCurrent();

			// Write the image data to the texture
			RMH_OpenGL_WriteImageDataToTexture(FrameData, FrameWidth, FrameHeight);
			// Configure the resolution and field of view (FOV) of the texture
			RMH_OpenGL_UpdateTextureFieldOfView(FrameWidth, FrameHeight);
			// Render the texture image data
			RMH_OpenGL_RenderImageTexture(TexturePanelWidth, TexturePanelHeight, FrameWidth, FrameHeight, FixedAspectRatio);

		}

		GLvoid RMH_OpenGL_RenderGrayscaleUltraResolutionImageData(unsigned int TexturePanelWidth, unsigned int TexturePanelHeight, unsigned short* FrameData, unsigned int NativeFrameWidth, unsigned int NativeFrameHeight, unsigned int UltraFrameWidth, unsigned int UltraFrameHeight, bool FixedAspectRatio) {

			// This routine handles the OpenGL rendering of the ultra resolution image data to the texture handler object

			// Make the associated render context the current render context
			RMH_OpenGL_MakeRenderContextCurrent();

			// Write the image data to the texture
			RMH_OpenGL_WriteImageDataToTexture(FrameData, UltraFrameWidth, UltraFrameHeight);
			// Configure the resolution and field of view (FOV) of the texture
			RMH_OpenGL_UpdateTextureFieldOfView(NativeFrameWidth, NativeFrameHeight);
			// Render the texture image data
			RMH_OpenGL_RenderImageTexture(TexturePanelWidth, TexturePanelHeight, NativeFrameWidth, NativeFrameHeight, FixedAspectRatio);

		}

		// ----------------------- Texture Panel Interaction Cursor Event Callback Routines ------------------------ //

		GLvoid RMH_OpenGL_UpdateRenderedObjectsZOrder(unsigned short ZOrden) {

			// This routine sets the object rendering Z-order for position-adjustable rectangles and crosshairs

			/*
			 *  Associated macros ->
			 * 
			 *  // Rendered Object Z-Order Setting Macros
			 *  #define _ZOrden_CrosshairsInFront	      0
			 *  #define _ZOrden_RectanglesInFront	      1
			 *  #define _ZOrden_LinesInFront			  2
			 * 
			 */

			// Update the rendering Z-order
			MovableObjectsRenderZOrder = ZOrden;

		}

		GLvoid TexturePanel_MouseDown(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {

			// Read the mouse cursor position
			GLdouble MouseXPosition = RMH_OpenGL_TranslateOverlayPanelMouseXPosToTextureXPos(e);
			GLdouble MouseYPosition = RMH_OpenGL_TranslateOverlayPanelMouseYPosToTextureYPos(e);

			// Update the overlay panel click flag
			OverlayPanelIsClick = true;

			// Selection of the rendering order (Z-order)
			switch (MovableObjectsRenderZOrder) {

				// Crosshairs are in front
				case _ZOrden_CrosshairsInFront:

					// Handle the events when a position-adjustable crosshair is clicked
					RMH_OpenGL_HandleCrosshairMouseDownEvents(MouseXPosition, MouseYPosition);

					// Handle the events when a position-adjustable rectangle is clicked
					RMH_OpenGL_HandleRectangleMouseDownEvents(MouseXPosition, MouseYPosition);

					// Handle the events when a position-adjustable line is clicked
					RMH_OpenGL_HandleLineMouseDownEvents(MouseXPosition, MouseYPosition);

				break;

				// Rectangles are in front
				case _ZOrden_RectanglesInFront:

					// Handle the events when a position-adjustable rectangle is clicked
					RMH_OpenGL_HandleRectangleMouseDownEvents(MouseXPosition, MouseYPosition);

					// Handle the events when a position-adjustable crosshair is clicked
					RMH_OpenGL_HandleCrosshairMouseDownEvents(MouseXPosition, MouseYPosition);

					// Handle the events when a position-adjustable line is clicked
					RMH_OpenGL_HandleLineMouseDownEvents(MouseXPosition, MouseYPosition);

				break;

				// Lines are in front
				case _ZOrden_LinesInFront:

					// Handle the events when a position-adjustable line is clicked
					RMH_OpenGL_HandleLineMouseDownEvents(MouseXPosition, MouseYPosition);

					// Handle the events when a position-adjustable rectangle is clicked
					RMH_OpenGL_HandleRectangleMouseDownEvents(MouseXPosition, MouseYPosition);

					// Handle the events when a position-adjustable crosshair is clicked
					RMH_OpenGL_HandleCrosshairMouseDownEvents(MouseXPosition, MouseYPosition);

				break;

			}

		}

		GLvoid TexturePanel_MouseUp(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {

			// Update the overlay panel click flag
			OverlayPanelIsClick = false;

			// Reset the rectangle move flag
			RectangleMoveFlag = false;

			// Reset the crosshair move flag
			CrosshairMoveFlag = false;

			// Reset the lines move flag
			LineMoveFlag = false;

		}

		GLvoid TexturePanel_MouseMove(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {

			// Read the mouse cursor position
			GLdouble MouseXPosition = RMH_OpenGL_TranslateOverlayPanelMouseXPosToTextureXPos(e);
			GLdouble MouseYPosition = RMH_OpenGL_TranslateOverlayPanelMouseYPosToTextureYPos(e);

			// Reset the overlay panel cursor
			OverlayPanel->Cursor = Cursors::Default;

			// Update the cursor label position, if the feature is enabled
			RMH_OpenGL_UpdateCursorTrackingPosition(MouseXPosition, MouseYPosition);

			// Selection of the rendering order (Z-order)
			switch (MovableObjectsRenderZOrder) {

				// Crosshairs are in front
				case _ZOrden_CrosshairsInFront:

					// Handle the events when a position-adjustable crosshair is to move
					RMH_OpenGL_HandleCrosshairMouseMoveEvents(MouseXPosition, MouseYPosition);

					// Handle the events when a position-adjustable rectangle is to move
					RMH_OpenGL_HandleRectangleMouseMoveEvents(MouseXPosition, MouseYPosition);

					// Handle the events when a position-adjustable line is to move
					RMH_OpenGL_HandleLineMouseMoveEvents(MouseXPosition, MouseYPosition);

				break;

				// Rectangles are in front
				case _ZOrden_RectanglesInFront:

					// Handle the events when a position-adjustable rectangle is to move
					RMH_OpenGL_HandleRectangleMouseMoveEvents(MouseXPosition, MouseYPosition);

					// Handle the events when a position-adjustable crosshair is to move
					RMH_OpenGL_HandleCrosshairMouseMoveEvents(MouseXPosition, MouseYPosition);

					// Handle the events when a position-adjustable line is to move
					RMH_OpenGL_HandleLineMouseMoveEvents(MouseXPosition, MouseYPosition);

				break;

				// Lines are in front
				case _ZOrden_LinesInFront:

					// Handle the events when a position-adjustable line is to move
					RMH_OpenGL_HandleLineMouseMoveEvents(MouseXPosition, MouseYPosition);

					// Handle the events when a position-adjustable rectangle is to move
					RMH_OpenGL_HandleRectangleMouseMoveEvents(MouseXPosition, MouseYPosition);

					// Handle the events when a position-adjustable crosshair is to move
					RMH_OpenGL_HandleCrosshairMouseMoveEvents(MouseXPosition, MouseYPosition);

				break;

			}

		}

		GLvoid TexturePanel_MouseWheel(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {

			// Check whether mouse scroll wheel live view rotation is enabled
			if (LiveViewMouseScrollWheelRotationEnablFlag == true) {

				// Check the rotation polarity of the mouse wheel
				if (e->Delta < 0.0) {

					// Rotate the live view image 90 degrees clockwise
					RMH_OpenGL_RotateLiveViewCW();

				}
				else {

					// Rotate the live view image 90 degrees counterclockwise
					RMH_OpenGL_RotateLiveViewCCW();

				}

			}

		}

		// -------------------------------- OpenGL Rendering Endpoint Routines -------------------------------- //
		
		GLvoid RMH_OpenGL_SwapOpenGLBuffers(GLvoid) {

			// This routine swaps the front/back buffers

			// Swap the buffers
			SwapBuffers(m_hDC);

		}

		GLvoid RMH_OpenGL_RenderingFinishedMark(GLvoid) {

			// This routine marks the end of an OpenGL rendering sequence
			// and must always be called last, when all object renderings have been executed

			// Read the total number of rectangles rendered
			NmbOfActiveRects = RenderedRectanglesCounter;
			// Reset the rectangle rendering counter variable
			RenderedRectanglesCounter = 0;

			// Read the total number of crosshairs rendered
			NmbOfActiveCrosshairs = RenderedCrosshairCounter;
			// Reset the crosshair rendering counter variable
			RenderedCrosshairCounter = 0;

			// Read the total number of lines rendered
			NmbOfActiveLines = RenderedLineCounter;
			// Reset the line rendering counter variable
			RenderedLineCounter = 0;

			// Swap the texture buffers
			RMH_OpenGL_SwapOpenGLBuffers();

			// Reset the render context
			//RMH_OpenGL_MakeRenderContextNULL();

		}
		
		// --------------------------------------------------------------------------------------------------------- //

	private:

		// ------------------------- Additional OpenGL Handling And Setup Routines ------------------------- //

		~RMHOpenGLWF(GLvoid) {

			// Delete the OpenGL context
			DeleteOpenGL();

			// Destroy the OpenGL handler object
			this->DestroyHandle();

			// Garbage Collect managed data
			System::GC::Collect();

		}

		GLvoid DeleteOpenGL(GLvoid)	{

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

		bool RMH_OpenGL_SetTexturePixelFormat(HDC hdc) {

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

		GLvoid RMH_OpenGL_ResizeOpenGLWinformsScene(unsigned int TotalTextureWidth, unsigned int TotalTextureHeight) {

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
			gluPerspective(60.0f, (GLfloat)TotalTextureWidth / (GLfloat)TotalTextureHeight, 0.1, 500.0); 
			// Select the "model view" matrix
			glMatrixMode(GL_MODELVIEW);
			// Reset the "model view" matrix
			glLoadIdentity();

		}

		GLvoid RMH_OpenGL_BuildFont(GLvoid) {

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

		bool RMH_OpenGL_Init(GLvoid) {

			// This routine initializes OpenGL in WinForms C++/CLR

			// Enable "flat shader" mode
			glShadeModel(GL_FLAT);
			// Default background color
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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

		// --------------------------------------------------------------------------------------------------------- //

	};

}