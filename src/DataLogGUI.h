#pragma once

// Data Log window: shows, in near real time, the values that the Data Logging feature writes to its CSV file.
// The window is built in code (no designer / resource file). It follows the data logging session settings
// (data sets, interval and duration) because it only displays what the data logging feature collects.

#include "GlobalObjectsAndVariables.h"
#include "RMH_Application_ThermalViewer.h"
#include "RMH_SupportedIRCameras_Resources.h"	// Names of the supported camera models (camera model in an exported graph image)

// Class namespace
namespace IRCAMThermalViewer {

	// Associated namespaces
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	// Maximum number of rows kept in the table (_DataLogView_MaxRows) and of points kept per line in the graph
	// (_DataLogView_MaxPlotPoints) of this window are defined in RMH_Application_ThermalViewer.h, with the way the oldest points of a long
	// session are thinned out (_DataLogView_ThinOldPoints) and the size of the blocks in which the log file is read (_DataLogView_ReadBlockBytes)
	// (the complete data is always in the CSV file)
	// Old rows and points are removed in blocks of this size (removing them one at a time from a long list is slow)
	#define _DataLogView_TrimBlockSize 1000

	// Colors of the scroll bars of the zoomed graph (time axis and temperature axis): the track, the buttons (arrows, Reset Zoom button and the sliding
	// part of the bar) and the lines / symbols drawn on the buttons. Light gray buttons with dark symbols so they stand out against the dark graph.
	// Exported image (Export Graph):
	// - _DataLogExport_ResolutionScale: the image has this many times more pixels in width and in height than the graph on the screen (3 = 3 x 3 = 9 times the
	//   pixels). 1 = the size of the graph on the screen.
	// - _DataLogExport_UseVectorChart: 1 = the graph is taken as a vector drawing (EMF+) and drawn at the high resolution, so lines and text stay sharp.
	//   0 = the graph is taken as a picture of the screen size (it is only stretched at a higher resolution; use it to compare or when 1 does not look right).
	// - _DataLogExport_JpegQuality: quality (0 - 100) of a JPEG image.
	// Graph markers: the longest text of a marker (characters)
	#define _DataLogMarker_MaxTextLength 60

	// Zoom buttons on the scroll bars of the zoomed graph (+ and - for the time axis and for the temperature axis):
	// - _DataLogZoomButton_Factor: one click on + shows this part of the visible range (0.7 = 70 %); one click on - shows 1 / this part (about 143 %). Smaller = bigger steps.
	// - _DataLogZoomButton_MinTimeMilliSec: the smallest time range that + zooms to (milliseconds).
	// - _DataLogZoomButton_MinTemperatureRange: the smallest temperature range that + zooms to.
	#define _DataLogZoomButton_Factor 0.7

	// Plot area of the Data Log graph while it is zoomed (UpdatePlotAreaLayout): the zoom scroll bars and the zoom buttons need room around the plot area.
	// - 1 = the plot area always has those larger margins, so it never changes size when the graph is zoomed, zoomed further or reset (the graph is a little smaller
	//   than before, about 35 pixels narrower and 33 pixels lower, also when it is not zoomed).
	// - 0 = the plot area has the smaller margins while the graph is not zoomed and the larger margins while it is zoomed (it changes size once, when zooming starts and ends).
	#define _DataLogPlotArea_ReserveZoomRoom 1
	#define _DataLogZoomButton_MinTimeMilliSec 10.0
	#define _DataLogZoomButton_MinTemperatureRange 0.01

	#define _DataLogExport_ResolutionScale 3
	#define _DataLogExport_UseVectorChart 1
	#define _DataLogExport_JpegQuality 95

	// 1 = the scroll bars use the colors below, 0 = the scroll bars keep the default colors of the chart control (to compare the two)
	#define _DataLogScrollBar_ApplyColors 1
	#define _DataLogScrollBar_TrackColor  System::Drawing::Color::FromArgb(70, 70, 70)
	#define _DataLogScrollBar_ButtonColor System::Drawing::Color::FromArgb(200, 200, 200)
	#define _DataLogScrollBar_LineColor   System::Drawing::Color::FromArgb(40, 40, 40)

	// The points of one line of the graph. The graph shows these points. When a line has more than _DataLogView_MaxPlotPoints points the oldest ones are
	// thinned out (min / max decimation) or removed, see _DataLogView_ThinOldPoints. The lists are filled while the log file is read (also when the
	// file is long); the chart line is brought up to date from them afterwards (SyncChartSeries of the window).
	ref class LogSeriesBuffer {

	public:

		System::Collections::Generic::List<double>^ XValues;     // Time of the points (days, the unit of the time axis)
		System::Collections::Generic::List<double>^ YValues;     // Value of the points
		int SyncedCount;        // How many points (from the start of the lists) the chart line holds now
		int RemovedFront;       // How many points were removed from the start of the lists that the chart line still holds
		bool NeedsRebuild;      // The chart line must be filled again from the lists (the points were thinned out)

		LogSeriesBuffer() {

			this->XValues = gcnew System::Collections::Generic::List<double>();
			this->YValues = gcnew System::Collections::Generic::List<double>();
			this->SyncedCount = 0;
			this->RemovedFront = 0;
			this->NeedsRebuild = false;

		}

		System::Void Clear() {

			this->XValues->Clear();
			this->YValues->Clear();
			this->SyncedCount = 0;
			this->RemovedFront = 0;
			this->NeedsRebuild = false;

		}

		System::Void Add(double XValue, double YValue) {

			this->XValues->Add(XValue);
			this->YValues->Add(YValue);

		}

		System::Void LimitPoints(int MaxPoints) {

			// Called after points were added: keeps the number of points near MaxPoints. It works in steps (not after every sample).
			int Count = this->YValues->Count;

#if _DataLogView_ThinOldPoints == 1

			if (Count <= MaxPoints) { return; }

			// Min / max decimation of the older half of the points: of every 4 neighbouring points the lowest and the highest are kept (they are real points,
			// with their own time, in time order), so a peak or a dip is never lost. A group with a NaN (gap in the line) also keeps its first NaN, so the gap stays.
			// The first point of the line is always kept. The newer half is not changed. Points that were thinned out before are thinned out again, so the older the data the coarser it is.
			int GroupCount = (Count / 2) / 4;
			if (GroupCount < 1) { return; }
			int OldEnd = GroupCount * 4;

			System::Collections::Generic::List<double>^ NewX = gcnew System::Collections::Generic::List<double>(Count);
			System::Collections::Generic::List<double>^ NewY = gcnew System::Collections::Generic::List<double>(Count);

			for (int g = 0; g < GroupCount; g++) {

				int First = g * 4;
				int MinIndex = -1;
				int MaxIndex = -1;
				int NaNIndex = -1;

				for (int k = First; k < First + 4; k++) {

					double Value = this->YValues[k];

					if (System::Double::IsNaN(Value)) {

						if (NaNIndex < 0) { NaNIndex = k; }
						continue;

					}

					if (MinIndex < 0 || Value < this->YValues[MinIndex]) { MinIndex = k; }
					if (MaxIndex < 0 || Value > this->YValues[MaxIndex]) { MaxIndex = k; }

				}

				int Kept[4];
				int KeptCount = 0;

				if (MinIndex >= 0) { Kept[KeptCount] = MinIndex; KeptCount++; }
				if (MaxIndex >= 0 && MaxIndex != MinIndex) { Kept[KeptCount] = MaxIndex; KeptCount++; }
				if (NaNIndex >= 0) { Kept[KeptCount] = NaNIndex; KeptCount++; }

				// The very first point of the line is always kept, so the left end of the graph stays at the start of the session
				if (g == 0 && MinIndex != First && MaxIndex != First && NaNIndex != First) { Kept[KeptCount] = First; KeptCount++; }

				// Time order (insertion sort of at most four indexes)
				for (int a = 1; a < KeptCount; a++) {

					int Key = Kept[a];
					int b = a - 1;

					while (b >= 0 && Kept[b] > Key) { Kept[b + 1] = Kept[b]; b--; }

					Kept[b + 1] = Key;

				}

				for (int a = 0; a < KeptCount; a++) {

					NewX->Add(this->XValues[Kept[a]]);
					NewY->Add(this->YValues[Kept[a]]);

				}

			}

			for (int k = OldEnd; k < Count; k++) {

				NewX->Add(this->XValues[k]);
				NewY->Add(this->YValues[k]);

			}

			this->XValues = NewX;
			this->YValues = NewY;
			this->NeedsRebuild = true;

			// Safety: never more than MaxPoints (cannot happen with the steps above, a very small MaxPoints could)
			if (this->YValues->Count > MaxPoints) {

				int Surplus = this->YValues->Count - MaxPoints;
				this->XValues->RemoveRange(0, Surplus);
				this->YValues->RemoveRange(0, Surplus);

			}

#else

			// The oldest points are removed in blocks (removing them one at a time from a long list is slow)
			if (Count <= MaxPoints + _DataLogView_TrimBlockSize) { return; }

			int Surplus = Count - MaxPoints;
			this->XValues->RemoveRange(0, Surplus);
			this->YValues->RemoveRange(0, Surplus);

			if (this->NeedsRebuild == false && this->SyncedCount >= Surplus) {

				this->RemovedFront = this->RemovedFront + Surplus;
				this->SyncedCount = this->SyncedCount - Surplus;

			}
			else { this->NeedsRebuild = true; }

#endif

		}

	};

	// One graph line in the two-cursor measurement: its name and color and its value at cursor A and at cursor B (HasA / HasB are false when the line has no
	// value at that time: outside the logged time range or a gap in the line)
	ref class MeasureRow {

	public:

		System::String^ Name;
		System::Drawing::Color LineColor;
		bool HasA;
		bool HasB;
		double ValueA;
		double ValueB;

		MeasureRow() {

			this->Name = "";
			this->LineColor = System::Drawing::Color::White;
			this->HasA = false;
			this->HasB = false;
			this->ValueA = 0.0;
			this->ValueB = 0.0;

		}

	};

	// A marker in the graph: a vertical line at a time, with a text (see the graph markers part of DataLogGUI)
	ref class GraphMarker {

	public:

		double ElapsedMilliSec;                       // Time of the marker (elapsed time of the log session, as in the log file)
		System::String^ Text;                         // Text of the marker, shown below its time
		float PixelX;                                 // Position of the line in the graph at the last drawing (for picking it with the mouse)
		System::Drawing::RectangleF LabelRect;        // Position of the text box at the last drawing
		bool Drawn;                                   // False when the marker is outside the time range that is shown

		GraphMarker() {

			this->ElapsedMilliSec = 0.0;
			this->Text = "";
			this->PixelX = 0.0f;
			this->LabelRect = System::Drawing::RectangleF(0.0f, 0.0f, 0.0f, 0.0f);
			this->Drawn = false;

		}

	};

	public ref class DataLogGUI : public System::Windows::Forms::Form {

	public:

		DataLogGUI(void) {

			this->ViewedFilePath = nullptr;
			this->GraphNotes = "";
			this->ViewedFilePosition = 0;
			this->PartialLine = "";
			this->ViewedSampleRows = 0;
			this->LastElapsedMilliSec = 0.0;
			this->SeriesBuffers = gcnew System::Collections::Generic::List<LogSeriesBuffer^>();
			this->PendingLines = gcnew System::Collections::Generic::Queue<System::String^>();
			this->ViewDecoder = System::Text::Encoding::UTF8->GetDecoder();
			this->ReadBuffer = nullptr;
			this->CharBuffer = nullptr;
			this->LoadingBehind = false;
			this->LoadTargetLength = 0;
			this->ViewHasNewData = false;
			this->ChartStartTime = System::DateTime(2000, 1, 1, 0, 0, 0);   // The time axis of the graph starts at 0:00:00 (elapsed time)
			this->SplitDistanceSet = false;
			this->NoticeShowing = false;
			this->HideDurationNotice = false;
			this->SuppressNoticeOnce = false;
			this->ViewDelimiter = nullptr;
			this->ViewDelimiterFilePath = nullptr;
			this->CursorLabelEnabled = false;
			this->MeasureModeEnabled = false;
			this->MeasureTimeA = System::Double::NaN;
			this->MeasureTimeB = System::Double::NaN;
			this->MeasureDragging = -1;
			this->MeasureRowLabels = gcnew System::Collections::Generic::List<System::Windows::Forms::Label^>();
			this->GraphMarkers = gcnew System::Collections::Generic::List<GraphMarker^>();
			this->MarkerDragging = -1;
			this->MarkerHoverActive = false;
			this->MarkerMenuTimeValid = false;
			this->MarkerMenuTime = 0.0;
			this->MarkerMenuTarget = nullptr;
			this->MarkerFont = nullptr;
			this->MeasurePanelPositioned = false;
			this->MeasurePanelDragging = false;
			this->XTickCount = 0;
			this->YTickCount = 0;
			this->YRangeLocked = false;
			this->YLockedMinimum = 0.0;
			this->YLockedMaximum = 100.0;
			this->YRangeSetByProgram = false;
			this->XRangeLocked = false;
			this->XLockedMinimum = 0.0;
			this->XLockedMaximum = 60.0;
			this->XRangeSetByProgram = false;
			this->XIntervalSetByProgram = false;
			this->XIntervalSeconds = 0.0;
			this->YIntervalSetByProgram = false;

			this->BuildLayout();

		}

		// Crash recovery: shows a session file that was left in the temporary folder by a crash, a power loss or a killed program (see
		// RMH_ThermalViewer_CheckForUnsavedDataLoggingSessions). The window follows the "current session file", so that file is set here. The data delimiter of that
		// file (found in its header line) is only used for showing this file - the delimiter setting of the program is not changed, so the next session is still
		// written with the delimiter that is set. The file stays where it is (it is not saved or deleted by this).
		System::Void OpenRecoveredSession(System::String^ FullPath, System::String^ Delimiter) {

			// The window follows the running session while one is logged
			if (DataLoggingIsRunningFlag == true || FullPath == nullptr || FullPath->Length == 0) { return; }

			GlobalVariables::DataLoggingSessionFolderString = System::IO::Path::GetDirectoryName(FullPath);
			GlobalVariables::DataLoggingSessionFileNameString = System::IO::Path::GetFileName(FullPath);

			this->ViewDelimiter = Delimiter;
			this->ViewDelimiterFilePath = FullPath;

			// The notice about the duration of the next data logging session is not shown when the window opens this time
			this->SuppressNoticeOnce = true;

		}

	protected:

		~DataLogGUI() {}

	private:

		System::Windows::Forms::Label^ StatusLabel;
		System::Windows::Forms::DataGridView^ LogGrid;
		System::Windows::Forms::DataVisualization::Charting::Chart^ LogChart;
		System::Windows::Forms::SplitContainer^ ViewSplit;
		bool SplitDistanceSet;
		System::Windows::Forms::Button^ ResetZoomButton;
		System::Windows::Forms::Button^ ExportGraphButton;
		System::Windows::Forms::Button^ AddNotesButton;
		System::String^ GraphNotes;     // Notes about the logging, written in the table below the graph of an exported image (like the graph title, they stay until changed)
		System::DateTime ChartStartTime;
		System::Windows::Forms::Label^ CursorLabel;     // Shows the temperature at the mouse cursor (switched on / off in the right click menu of the graph)
		bool CursorLabelEnabled;

		// Two-cursor measurement (right click menu: Measure With Two Cursors): cursor A and B are vertical lines in the graph (annotations of the chart, so they
		// are part of an exported image). A panel in the graph shows the time of each cursor, the time difference and, for every line that is shown, its value at A and
		// at B, the temperature difference and the slope (temperature unit per minute). Times are in days (the unit of the time axis), NaN = the cursor is not placed.
		bool MeasureModeEnabled;                     // True: a left click / drag in the graph places or moves a cursor (the zoom rectangle is switched off meanwhile)
		double MeasureTimeA;
		double MeasureTimeB;
		int MeasureDragging;                         // -1 = no cursor is being dragged, 0 = cursor A, 1 = cursor B
		System::Windows::Forms::DataVisualization::Charting::VerticalLineAnnotation^ MeasureLineA;
		System::Windows::Forms::DataVisualization::Charting::VerticalLineAnnotation^ MeasureLineB;
		System::Windows::Forms::FlowLayoutPanel^ MeasurePanel;      // The readout of the measurement (can be moved with the mouse)
		System::Collections::Generic::List<System::Windows::Forms::Label^>^ MeasureRowLabels;
		bool MeasurePanelPositioned;                 // The panel got its first position
		bool MeasurePanelDragging;
		System::Drawing::Point MeasurePanelDragStart;     // Mouse position (screen) when the panel drag started
		System::Drawing::Point MeasurePanelDragOrigin;    // Position of the panel when the drag started
		System::Windows::Forms::ToolStripMenuItem^ MeasureMenuItem;
		System::Windows::Forms::ToolStripMenuItem^ ClearCursorsMenuItem;
		System::Windows::Forms::ToolStripMenuItem^ SetCursorTimesMenuItem;

		// Graph markers (a vertical line with a time and a text; drawn on the chart, can be dragged with the mouse)
		System::Collections::Generic::List<GraphMarker^>^ GraphMarkers;
		int MarkerDragging;                          // -1 = no marker is being dragged, otherwise the index of the marker
		bool MarkerHoverActive;                      // The mouse is on a marker (arrow cursor, zoom rectangle off)
		System::Drawing::Font^ MarkerFont;
		System::Windows::Forms::Button^ AddMarkerButton;
		System::Windows::Forms::ToolStripMenuItem^ AddMarkerMenuItem;
		System::Windows::Forms::ToolStripMenuItem^ DeleteMarkerMenuItem;
		System::Windows::Forms::ToolStripMenuItem^ DeleteAllMarkersMenuItem;
		System::Windows::Forms::ToolStripSeparator^ MarkerSeparator;
		bool MarkerMenuTimeValid;                    // The graph was right clicked inside the plot area, over logged data
		double MarkerMenuTime;                       // Time (days) where the graph was right clicked
		GraphMarker^ MarkerMenuTarget;               // The marker that was right clicked (nullptr = none)

		// Zoom buttons on the scroll bars of the graph (shown only while that axis is zoomed)
		System::Windows::Forms::Button^ YZoomInButton;
		System::Windows::Forms::Button^ YZoomOutButton;
		System::Windows::Forms::Button^ XZoomInButton;
		System::Windows::Forms::Button^ XZoomOutButton;

		// Right click menu of the graph, and its settings
		System::Windows::Forms::ContextMenuStrip^ ChartContextMenu;
		System::Windows::Forms::ToolStripMenuItem^ CursorTempMenuItem;
		System::Windows::Forms::ToolStripMenuItem^ XTicksMenuItem;
		System::Windows::Forms::ToolStripMenuItem^ YTicksMenuItem;
		System::Windows::Forms::ToolStripMenuItem^ YRangeStatusMenuItem;
		System::Windows::Forms::ToolStripMenuItem^ YSetRangeMenuItem;
		System::Windows::Forms::ToolStripMenuItem^ YLockCurrentMenuItem;
		System::Windows::Forms::FlowLayoutPanel^ LineChecksPanel;   // One check box per graph line (shows / hides the line)
		System::Windows::Forms::ToolStripMenuItem^ YUnlockMenuItem;
		int XTickCount;              // Number of ticks of the time axis (0 = automatic)
		int YTickCount;              // Number of ticks of the temperature axis (0 = automatic)
		bool YRangeLocked;           // The temperature axis is locked to a fixed range
		double YLockedMinimum;
		double YLockedMaximum;
		System::Windows::Forms::ToolStripMenuItem^ XRangeStatusMenuItem;
		System::Windows::Forms::ToolStripMenuItem^ XSetRangeMenuItem;
		System::Windows::Forms::ToolStripMenuItem^ XLockCurrentMenuItem;
		System::Windows::Forms::ToolStripMenuItem^ XUnlockMenuItem;
		bool XRangeLocked;           // The time axis is locked to a fixed range
		double XLockedMinimum;       // Locked time range in seconds of elapsed time
		double XLockedMaximum;
		bool XRangeSetByProgram;     // The range of the time axis was set by this code (so it is given back to automatic when not needed)
		bool YRangeSetByProgram;     // The range of the temperature axis was set by this code (so it is given back to automatic when not needed)
		bool XIntervalSetByProgram;
		double XIntervalSeconds;     // The tick interval (in seconds) of the time axis that was written to the axis (when XIntervalSetByProgram is true)
		bool YIntervalSetByProgram;
		bool NoticeShowing;
		bool HideDurationNotice;
		bool SuppressNoticeOnce;                     // True: the notice about the session duration is not shown the next time the window opens (a recovered session file is opened)
		System::String^ ViewDelimiter;               // Data delimiter of the viewed file when it differs from the setting of the program (recovered session file), else nullptr
		System::String^ ViewDelimiterFilePath;       // The file that ViewDelimiter belongs to
		System::Windows::Forms::Button^ StartLoggingButton;
		System::Windows::Forms::Button^ StopLoggingButton;
		System::Windows::Forms::Button^ OpenFolderButton;
		System::Windows::Forms::Button^ ClearViewButton;
		System::Windows::Forms::CheckBox^ AutoScrollCheckBox;
		System::Windows::Forms::CheckBox^ ShowTitleCheckBox;     // Shows / hides the graph title on the graph and in an exported image (the title text is kept)
		System::Windows::Forms::TextBox^ TitleEditBox;           // Text box on the title of the graph for editing the title directly (double-click on the title)
		System::Windows::Forms::ToolTip^ HintToolTip;
		System::Windows::Forms::Timer^ RefreshTimer;

		// State of the data that is shown
		System::String^ ViewedFilePath;
		long long ViewedFilePosition;
		System::String^ PartialLine;
		int ViewedSampleRows;
		double LastElapsedMilliSec;

		// Reading of the log file in blocks (see RefreshLogView, ProcessNewText, ShowPendingData)
		System::Collections::Generic::List<LogSeriesBuffer^>^ SeriesBuffers;     // The points of the graph lines (one per line of the graph)
		System::Collections::Generic::Queue<System::String^>^ PendingLines;     // The newest lines of the file (at most _DataLogView_MaxRows) that are not in the table yet
		System::Text::Decoder^ ViewDecoder;                                     // Keeps a character that is split over two blocks together
		cli::array<System::Byte>^ ReadBuffer;                                   // One block of the file
		cli::array<wchar_t>^ CharBuffer;                                        // The text of one block
		bool LoadingBehind;                                                     // The view has not read the whole file yet (a long file is read over several refreshes)
		long long LoadTargetLength;                                             // File length that is being read up to (for the percentage in the status line)
		bool ViewHasNewData;                                                    // Lines were read since the table and the graph were brought up to date

		// ---------------------------------- Window layout ---------------------------------- //

		System::Void StyleButton(System::Windows::Forms::Button^ ButtonObject, System::String^ ButtonText, int X, int Width) {

			ButtonObject->Text = ButtonText;
			ButtonObject->Location = System::Drawing::Point(X, 38);
			ButtonObject->Size = System::Drawing::Size(Width, 28);
			ButtonObject->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			ButtonObject->BackColor = System::Drawing::Color::FromArgb(60, 60, 60);
			ButtonObject->ForeColor = System::Drawing::Color::White;
			ButtonObject->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(100, 100, 100);

		}

		// Gives a button a coloured border (used for the "Start Logging" - dark green and "Stop Logging" - dark red buttons).
		// The background and the text stay exactly like the other buttons.
		System::Void SetButtonBorderColor(System::Windows::Forms::Button^ ButtonObject, System::Drawing::Color BorderColor) {

			ButtonObject->FlatAppearance->BorderColor = BorderColor;

		}

		System::Void BuildLayout() {

			this->Text = "Data Log";
			this->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);
			this->ForeColor = System::Drawing::Color::White;
			this->ClientSize = System::Drawing::Size(1130, 600);
			this->Font = gcnew System::Drawing::Font("Arial", 9);

			// Top panel with the status text and the buttons
			System::Windows::Forms::Panel^ TopPanel = gcnew System::Windows::Forms::Panel();
			TopPanel->Dock = System::Windows::Forms::DockStyle::Top;
			TopPanel->Height = 74;
			TopPanel->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);

			this->StatusLabel = gcnew System::Windows::Forms::Label();
			this->StatusLabel->AutoSize = false;
			this->StatusLabel->Location = System::Drawing::Point(10, 8);
			this->StatusLabel->Size = System::Drawing::Size(1100, 24);
			this->StatusLabel->Anchor = System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left | System::Windows::Forms::AnchorStyles::Right;
			this->StatusLabel->Font = gcnew System::Drawing::Font("Arial", 10, System::Drawing::FontStyle::Bold);
			this->StatusLabel->ForeColor = System::Drawing::Color::White;
			TopPanel->Controls->Add(this->StatusLabel);

			this->StartLoggingButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->StartLoggingButton, "Start Logging", 10, 120);
			this->SetButtonBorderColor(this->StartLoggingButton, System::Drawing::Color::DarkGreen);	// dark green border
			this->StartLoggingButton->Click += gcnew System::EventHandler(this, &DataLogGUI::StartLoggingButton_Click);
			TopPanel->Controls->Add(this->StartLoggingButton);

			this->StopLoggingButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->StopLoggingButton, "Stop Logging", 135, 120);
			this->SetButtonBorderColor(this->StopLoggingButton, System::Drawing::Color::DarkRed);	// dark red border
			this->StopLoggingButton->Click += gcnew System::EventHandler(this, &DataLogGUI::StopLoggingButton_Click);
			TopPanel->Controls->Add(this->StopLoggingButton);

			this->OpenFolderButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->OpenFolderButton, "Show Log File", 260, 120);
			this->OpenFolderButton->Click += gcnew System::EventHandler(this, &DataLogGUI::OpenFolderButton_Click);
			TopPanel->Controls->Add(this->OpenFolderButton);

			this->ClearViewButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->ClearViewButton, "Clear Graph", 385, 100);
			this->ClearViewButton->Click += gcnew System::EventHandler(this, &DataLogGUI::ClearViewButton_Click);
			TopPanel->Controls->Add(this->ClearViewButton);

			this->ResetZoomButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->ResetZoomButton, "Reset Zoom", 490, 100);
			this->ResetZoomButton->Click += gcnew System::EventHandler(this, &DataLogGUI::ResetZoomButton_Click);
			TopPanel->Controls->Add(this->ResetZoomButton);

			this->AddNotesButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->AddNotesButton, "Add Notes", 595, 95);
			this->AddNotesButton->Click += gcnew System::EventHandler(this, &DataLogGUI::AddNotesButton_Click);
			TopPanel->Controls->Add(this->AddNotesButton);

			this->AddMarkerButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->AddMarkerButton, "Add Marker", 695, 105);
			this->AddMarkerButton->Click += gcnew System::EventHandler(this, &DataLogGUI::AddMarkerButton_Click);
			TopPanel->Controls->Add(this->AddMarkerButton);

			this->ExportGraphButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->ExportGraphButton, "Export Graph", 805, 115);
			this->ExportGraphButton->Click += gcnew System::EventHandler(this, &DataLogGUI::ExportGraphButton_Click);
			TopPanel->Controls->Add(this->ExportGraphButton);

			this->ShowTitleCheckBox = gcnew System::Windows::Forms::CheckBox();
			this->ShowTitleCheckBox->Text = "Show Graph Title";
			this->ShowTitleCheckBox->Checked = false;
			this->ShowTitleCheckBox->AutoSize = true;
			this->ShowTitleCheckBox->Location = System::Drawing::Point(928, 43);
			this->ShowTitleCheckBox->ForeColor = System::Drawing::Color::White;
			this->ShowTitleCheckBox->CheckedChanged += gcnew System::EventHandler(this, &DataLogGUI::ShowTitleCheckBox_CheckedChanged);
			TopPanel->Controls->Add(this->ShowTitleCheckBox);

			this->HintToolTip = gcnew System::Windows::Forms::ToolTip();
			this->HintToolTip->SetToolTip(this->AddMarkerButton, "Adds a marker (a vertical line with the time and a text) at the current time of the data logging. To add a marker at another time, right-click the graph. Markers can be dragged with the mouse.");
			this->HintToolTip->SetToolTip(this->ShowTitleCheckBox, "Shows the title above the graph and in an exported image. Double-click the title on the graph to edit it.");

			this->AutoScrollCheckBox = gcnew System::Windows::Forms::CheckBox();
			this->AutoScrollCheckBox->Text = "Auto-scroll table to newest";
			this->AutoScrollCheckBox->Checked = true;
			this->AutoScrollCheckBox->AutoSize = true;
			this->AutoScrollCheckBox->Location = System::Drawing::Point(1060, 43);
			this->AutoScrollCheckBox->ForeColor = System::Drawing::Color::White;
			TopPanel->Controls->Add(this->AutoScrollCheckBox);

			// Table of the logged values
			this->LogGrid = gcnew System::Windows::Forms::DataGridView();
			this->LogGrid->Dock = System::Windows::Forms::DockStyle::Fill;
			this->LogGrid->ReadOnly = true;
			this->LogGrid->AllowUserToAddRows = false;
			this->LogGrid->AllowUserToDeleteRows = false;
			this->LogGrid->AllowUserToResizeRows = false;
			this->LogGrid->RowHeadersVisible = false;
			this->LogGrid->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->LogGrid->BackgroundColor = System::Drawing::Color::FromArgb(35, 35, 35);
			this->LogGrid->GridColor = System::Drawing::Color::FromArgb(70, 70, 70);
			this->LogGrid->EnableHeadersVisualStyles = false;
			this->LogGrid->ColumnHeadersDefaultCellStyle->BackColor = System::Drawing::Color::FromArgb(60, 60, 60);
			this->LogGrid->ColumnHeadersDefaultCellStyle->ForeColor = System::Drawing::Color::White;
			this->LogGrid->ColumnHeadersDefaultCellStyle->SelectionBackColor = System::Drawing::Color::FromArgb(60, 60, 60);
			this->LogGrid->DefaultCellStyle->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);
			this->LogGrid->DefaultCellStyle->ForeColor = System::Drawing::Color::White;
			this->LogGrid->DefaultCellStyle->SelectionBackColor = System::Drawing::Color::FromArgb(0, 90, 160);
			this->LogGrid->DefaultCellStyle->SelectionForeColor = System::Drawing::Color::White;
			this->LogGrid->AutoSizeColumnsMode = System::Windows::Forms::DataGridViewAutoSizeColumnsMode::Fill;
			this->LogGrid->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;

			// Graph of the logged values (temperature against time)
			this->BuildChart();

			// The graph is shown above the table
			this->ViewSplit = gcnew System::Windows::Forms::SplitContainer();
			ViewSplit->Dock = System::Windows::Forms::DockStyle::Fill;
			ViewSplit->Orientation = System::Windows::Forms::Orientation::Horizontal;
			ViewSplit->BackColor = System::Drawing::Color::FromArgb(60, 60, 60);
			ViewSplit->FixedPanel = System::Windows::Forms::FixedPanel::Panel2;
			ViewSplit->Panel1->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);
			ViewSplit->Panel2->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);
			ViewSplit->Panel1->Controls->Add(this->LogChart);

			// Zoom buttons on the scroll bars of the zoomed graph (small buttons on the graph, shown only while an axis is zoomed)
			this->BuildZoomButtons();

			// Check boxes above the graph - one per graph line, in the color of the line (replaces the legend of the graph)
			this->LineChecksPanel = gcnew System::Windows::Forms::FlowLayoutPanel();
			this->LineChecksPanel->Dock = System::Windows::Forms::DockStyle::Top;
			this->LineChecksPanel->AutoSize = true;
			this->LineChecksPanel->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
			this->LineChecksPanel->WrapContents = true;
			this->LineChecksPanel->FlowDirection = System::Windows::Forms::FlowDirection::LeftToRight;
			this->LineChecksPanel->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);
			this->LineChecksPanel->Padding = System::Windows::Forms::Padding(6, 2, 6, 2);
			ViewSplit->Panel1->Controls->Add(this->LineChecksPanel);
			ViewSplit->Panel2->Controls->Add(this->LogGrid);

			// The split container is added first, so that the top panel is docked before it
			this->Controls->Add(ViewSplit);
			this->Controls->Add(TopPanel);

			// Refresh timer - only runs while the window is visible
			this->RefreshTimer = gcnew System::Windows::Forms::Timer();
			this->RefreshTimer->Interval = 1000;
			this->RefreshTimer->Tick += gcnew System::EventHandler(this, &DataLogGUI::RefreshTimer_Tick);

			this->VisibleChanged += gcnew System::EventHandler(this, &DataLogGUI::DataLogGUI_VisibleChanged);
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &DataLogGUI::DataLogGUI_FormClosing);

			this->UpdateStatusText();

		}

		// ---------------------------------- Graph ---------------------------------- //

		System::Drawing::Color GetSeriesColor(int SeriesIndex) {

			// Line colors of the graph - the same order of colors as the 2D plot
			switch (SeriesIndex % 10) {

				case 0: return System::Drawing::Color::FromArgb(255, 64, 64);
				case 1: return System::Drawing::Color::FromArgb(90, 130, 255);
				case 2: return System::Drawing::Color::FromArgb(60, 210, 60);
				case 3: return System::Drawing::Color::FromArgb(255, 230, 0);
				case 4: return System::Drawing::Color::FromArgb(255, 150, 40);
				case 5: return System::Drawing::Color::FromArgb(255, 60, 255);
				case 6: return System::Drawing::Color::FromArgb(0, 220, 255);
				case 7: return System::Drawing::Color::FromArgb(235, 235, 235);
				case 8: return System::Drawing::Color::FromArgb(150, 150, 150);
				default: return System::Drawing::Color::FromArgb(150, 255, 100);

			}

		}

		System::Void BuildChart() {

			this->LogChart = gcnew System::Windows::Forms::DataVisualization::Charting::Chart();
			this->LogChart->Dock = System::Windows::Forms::DockStyle::Fill;
			this->LogChart->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);

			System::Windows::Forms::DataVisualization::Charting::ChartArea^ MainArea = gcnew System::Windows::Forms::DataVisualization::Charting::ChartArea("Main");
			MainArea->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);
			MainArea->AxisX->LineColor = System::Drawing::Color::FromArgb(150, 150, 150);
			MainArea->AxisY->LineColor = System::Drawing::Color::FromArgb(150, 150, 150);
			MainArea->AxisX->MajorGrid->LineColor = System::Drawing::Color::FromArgb(70, 70, 70);
			MainArea->AxisY->MajorGrid->LineColor = System::Drawing::Color::FromArgb(70, 70, 70);
			MainArea->AxisX->LabelStyle->ForeColor = System::Drawing::Color::White;
			MainArea->AxisY->LabelStyle->ForeColor = System::Drawing::Color::White;
			MainArea->AxisX->LabelStyle->Format = "HH:mm:ss";
			MainArea->AxisY->LabelStyle->Format = "0.0";
			MainArea->AxisX->Title = "Time (h:mm:ss)";
			MainArea->AxisY->Title = "Temperature";
			MainArea->AxisX->TitleForeColor = System::Drawing::Color::White;
			MainArea->AxisY->TitleForeColor = System::Drawing::Color::White;
			MainArea->AxisY->IsStartedFromZero = false;

			// Frame (border box) around the whole plot area, always shown - like "Show Plot Border Box" of the 2D plot in the Plots window. The left and bottom sides
			// are the X and Y axis lines; the top and right sides are the secondary axes with only their line switched on (no labels, tick marks or grid lines),
			// so the layout of the graph does not change. The frame has the color of the axis lines and is part of an exported image.
			MainArea->AxisX2->Enabled = System::Windows::Forms::DataVisualization::Charting::AxisEnabled::True;
			MainArea->AxisX2->LineColor = System::Drawing::Color::FromArgb(150, 150, 150);
			MainArea->AxisX2->LineWidth = 1;
			MainArea->AxisX2->LabelStyle->Enabled = false;
			MainArea->AxisX2->MajorTickMark->Enabled = false;
			MainArea->AxisX2->MinorTickMark->Enabled = false;
			MainArea->AxisX2->MajorGrid->Enabled = false;
			MainArea->AxisX2->MinorGrid->Enabled = false;
			MainArea->AxisY2->Enabled = System::Windows::Forms::DataVisualization::Charting::AxisEnabled::True;
			MainArea->AxisY2->LineColor = System::Drawing::Color::FromArgb(150, 150, 150);
			MainArea->AxisY2->LineWidth = 1;
			MainArea->AxisY2->LabelStyle->Enabled = false;
			MainArea->AxisY2->MajorTickMark->Enabled = false;
			MainArea->AxisY2->MinorTickMark->Enabled = false;
			MainArea->AxisY2->MajorGrid->Enabled = false;
			MainArea->AxisY2->MinorGrid->Enabled = false;

			// Zoom by dragging a rectangle in the graph (Reset Zoom button to go back)
			MainArea->CursorX->IsUserSelectionEnabled = true;
			MainArea->CursorY->IsUserSelectionEnabled = true;
			MainArea->AxisX->ScaleView->Zoomable = true;
			MainArea->AxisY->ScaleView->Zoomable = true;

			// The zoom rectangle is not snapped to whole units, and the zoomed view may be small, on both axes. The time axis is a date / time axis:
			// without this the selection snaps to a large time unit (so the time range hardly zooms) and only the temperature axis zooms freely.
			MainArea->CursorX->IntervalType = System::Windows::Forms::DataVisualization::Charting::DateTimeIntervalType::Milliseconds;
			MainArea->CursorX->Interval = 1;
			MainArea->CursorY->Interval = 0.001;
			MainArea->AxisX->ScaleView->MinSizeType = System::Windows::Forms::DataVisualization::Charting::DateTimeIntervalType::Milliseconds;
			MainArea->AxisX->ScaleView->MinSize = 1;
			MainArea->AxisY->ScaleView->MinSize = 0.001;

			// Scroll bars of the zoomed view (they show when the graph is zoomed). The time axis is a date / time axis, so its scroll steps must be set in
			// milliseconds: with the default step (a whole unit) the arrows jumped to the beginning or the end and dragging the bar did not move the view.
			// The temperature axis gets a fine step as well (its default step is 1 whole unit). The size of an arrow step is set in LogChart_AxisViewChanged.
			MainArea->AxisX->ScaleView->SmallScrollMinSizeType = System::Windows::Forms::DataVisualization::Charting::DateTimeIntervalType::Milliseconds;
			MainArea->AxisX->ScaleView->SmallScrollMinSize = 1;
			MainArea->AxisX->ScaleView->SmallScrollSizeType = System::Windows::Forms::DataVisualization::Charting::DateTimeIntervalType::Milliseconds;
			MainArea->AxisX->ScaleView->SmallScrollSize = 100;
			MainArea->AxisY->ScaleView->SmallScrollMinSize = 0.001;
			MainArea->AxisY->ScaleView->SmallScrollSize = 0.1;

			// Lighter gray scroll bars, arrows and Reset Zoom buttons on both axes (more visible against the dark graph)
			this->StyleAxisScrollBar(MainArea->AxisX);
			this->StyleAxisScrollBar(MainArea->AxisY);

			this->LogChart->ChartAreas->Add(MainArea);

			// Title of the graph (top center, above the graph) - empty and hidden until a title is set in the right-click menu. It is part of an exported image.
			System::Windows::Forms::DataVisualization::Charting::Title^ GraphTitle = gcnew System::Windows::Forms::DataVisualization::Charting::Title();
			GraphTitle->Name = "GraphTitle";
			GraphTitle->Text = "Title";		// Shown when the Show Graph Title check box is ticked and no other title is set
			GraphTitle->Docking = System::Windows::Forms::DataVisualization::Charting::Docking::Top;
			GraphTitle->Alignment = System::Drawing::ContentAlignment::TopCenter;
			GraphTitle->Font = gcnew System::Drawing::Font("Arial", 12, System::Drawing::FontStyle::Bold);
			GraphTitle->ForeColor = System::Drawing::Color::White;
			GraphTitle->Visible = false;
			this->LogChart->Titles->Add(GraphTitle);

			// The graph has no legend of its own: the check boxes above the graph name the lines and show / hide them

			// The labels of the time axis show the elapsed time (also when the log is longer than 24 hours)
			this->LogChart->FormatNumber += gcnew System::EventHandler<System::Windows::Forms::DataVisualization::Charting::FormatNumberEventArgs^>(this, &DataLogGUI::LogChart_FormatNumber);

			// The plot area has a fixed size (see UpdatePlotAreaLayout), so it is set again when the graph is resized
			this->LogChart->SizeChanged += gcnew System::EventHandler(this, &DataLogGUI::LogChart_SizeChanged);

			// Mouse cursor temperature label - switched on or off in the right click menu of the graph
			this->CursorLabel = gcnew System::Windows::Forms::Label();
			this->CursorLabel->AutoSize = true;
			this->CursorLabel->BackColor = System::Drawing::Color::FromArgb(20, 20, 20);
			this->CursorLabel->ForeColor = System::Drawing::Color::White;
			this->CursorLabel->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->CursorLabel->Padding = System::Windows::Forms::Padding(2);
			this->CursorLabel->Visible = false;
			this->LogChart->Controls->Add(this->CursorLabel);

			this->LogChart->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &DataLogGUI::LogChart_MouseMove);
			this->LogChart->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &DataLogGUI::LogChart_MouseDown);
			this->LogChart->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &DataLogGUI::LogChart_MouseUp);
			this->LogChart->PostPaint += gcnew System::EventHandler<System::Windows::Forms::DataVisualization::Charting::ChartPaintEventArgs^>(this, &DataLogGUI::LogChart_PostPaint);
			this->BuildMeasureObjects();
			this->LogChart->MouseDoubleClick += gcnew System::Windows::Forms::MouseEventHandler(this, &DataLogGUI::LogChart_MouseDoubleClick);

			// Text box for editing the title directly on the graph (hidden until the title is double-clicked)
			this->TitleEditBox = gcnew System::Windows::Forms::TextBox();
			this->TitleEditBox->Visible = false;
			this->TitleEditBox->MaxLength = 200;
			this->TitleEditBox->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->TitleEditBox->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			this->TitleEditBox->Font = gcnew System::Drawing::Font("Arial", 12, System::Drawing::FontStyle::Bold);
			this->TitleEditBox->BackColor = System::Drawing::Color::FromArgb(60, 60, 60);
			this->TitleEditBox->ForeColor = System::Drawing::Color::White;
			this->TitleEditBox->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &DataLogGUI::TitleEditBox_KeyDown);
			this->TitleEditBox->Leave += gcnew System::EventHandler(this, &DataLogGUI::TitleEditBox_Leave);
			this->LogChart->Controls->Add(this->TitleEditBox);
			this->LogChart->MouseLeave += gcnew System::EventHandler(this, &DataLogGUI::LogChart_MouseLeave);

			// Update the tick intervals when the graph is zoomed
			this->LogChart->AxisViewChanged += gcnew System::EventHandler<System::Windows::Forms::DataVisualization::Charting::ViewEventArgs^>(this, &DataLogGUI::LogChart_AxisViewChanged);

			// Right click menu of the graph
			this->BuildChartContextMenu();

			this->UpdateAxisTitles();

		}

		// ------------------------- Mouse cursor temperature ------------------------- //

		System::Void UpdateCursorLabel(int MouseX, int MouseY) {

			if (this->CursorLabelEnabled == false) { this->CursorLabel->Visible = false; return; }

			System::String^ LabelString = gcnew System::String("Temperature: N/A ") + GlobalVariables::DefaultTempUnitString;

			try {

				// Only inside the plot area of the graph (not on the axes, the legend or outside the graph). A cursor line of the two-cursor measurement counts as plot area
				// (it is an annotation of the chart), so the label also works while the mouse is on a cursor line.
				if (this->IsPointInPlotArea(MouseX, MouseY) == true) {

					double TemperatureValue = this->LogChart->ChartAreas[0]->AxisY->PixelPositionToValue((double)MouseY);
					double TimeValue = this->LogChart->ChartAreas[0]->AxisX->PixelPositionToValue((double)MouseX);
					double ElapsedMilliSec = (TimeValue - this->ChartStartTime.ToOADate()) * 86400000.0;
					if (ElapsedMilliSec < 0.0) { ElapsedMilliSec = 0.0; }

					LabelString = gcnew System::String("Temperature: ") + TemperatureValue.ToString("F2") + gcnew System::String(" ") + GlobalVariables::DefaultTempUnitString +
						gcnew System::String("   (") + this->FormatDuration(ElapsedMilliSec) + gcnew System::String(")");

				}

			}
			catch (System::Exception^) {}

			this->CursorLabel->Text = LabelString;

			// The label is placed beside the mouse cursor (on the other side near the edge of the graph)
			int LabelX = MouseX + 12;
			int LabelY = MouseY - 10;
			if (LabelX + this->CursorLabel->Width > this->LogChart->Width) { LabelX = MouseX - 12 - this->CursorLabel->Width; }
			if (LabelX < 0) { LabelX = 0; }
			if (LabelY < 0) { LabelY = 0; }

			this->CursorLabel->Location = System::Drawing::Point(LabelX, LabelY);
			this->CursorLabel->Visible = true;
			this->CursorLabel->BringToFront();

		}

		System::Void LogChart_MouseMove(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {

			// A marker that is being dragged follows the mouse (it stays inside the logged time range); without a drag the mouse cursor shows when it is on a marker
			if (this->MarkerDragging >= 0) {

				if ((e->Button & System::Windows::Forms::MouseButtons::Left) == System::Windows::Forms::MouseButtons::Left && this->MarkerDragging < this->GraphMarkers->Count) {

					double MarkerTime = 0.0;

					if (this->GetMeasureTimeAtMouse(e->X, MarkerTime) == true) {

						double MarkerMilliSec = (MarkerTime - this->ChartStartTime.ToOADate()) * 86400000.0;
						if (MarkerMilliSec < 0.0) { MarkerMilliSec = 0.0; }

						this->GraphMarkers[this->MarkerDragging]->ElapsedMilliSec = MarkerMilliSec;
						this->LogChart->Invalidate();

					}

				}
				else { this->MarkerDragging = -1; }

			}
			else { this->UpdateMarkerHover(e->X, e->Y); }

			// A measurement cursor that is being dragged follows the mouse (also outside the plot area: it stays inside the logged time range)
			if (this->MeasureDragging >= 0 && (e->Button & System::Windows::Forms::MouseButtons::Left) == System::Windows::Forms::MouseButtons::Left) {

				double DragTime = 0.0;
				if (this->GetMeasureTimeAtMouse(e->X, DragTime) == true) { this->SetMeasureCursor(this->MeasureDragging, DragTime); }

			}

			if (this->CursorLabelEnabled == true) { this->UpdateCursorLabel(e->X, e->Y); }

		}

		System::Void LogChart_MouseLeave(System::Object^ sender, System::EventArgs^ e) {

			// The label is hidden when the mouse leaves the graph (it shows again when the mouse comes back)
			this->CursorLabel->Visible = false;
			this->UpdateMarkerHover(-1, -1);

		}

		// ------------------------- Zoom buttons on the scroll bars ------------------------- //

		// The zoom rectangle always zooms both axes at once. The buttons zoom one axis only (X = time axis, Y = temperature axis), in steps, also when the graph is
		// zoomed already: + shows a smaller part of the axis, - a larger part (back to the whole range at the end, which also hides the scroll bar of that axis and
		// the buttons of that axis). The buttons only exist on the scroll bar of a zoomed axis. The zoom is around the middle of what is shown now.
		// The scroll bars and the Reset Zoom button work as before.

		System::Void StyleZoomButton(System::Windows::Forms::Button^ ZoomButton, System::String^ ButtonText, System::String^ HintText) {

			// A small button in the colors of the scroll bar buttons (_DataLogScrollBar_... at the top of this file). Its + or - is drawn by ZoomButton_Paint (the text
			// of a button this small is not drawn reliably), the symbol is kept in its Tag.
			ZoomButton->Text = "";
			ZoomButton->Tag = ButtonText;
			ZoomButton->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &DataLogGUI::ZoomButton_Paint);
			ZoomButton->Size = System::Drawing::Size(13, 13);
			ZoomButton->Padding = System::Windows::Forms::Padding(0);
			ZoomButton->Margin = System::Windows::Forms::Padding(0);
			ZoomButton->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			ZoomButton->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			ZoomButton->BackColor = _DataLogScrollBar_ButtonColor;
			ZoomButton->ForeColor = _DataLogScrollBar_LineColor;
			ZoomButton->FlatAppearance->BorderColor = _DataLogScrollBar_LineColor;
			ZoomButton->FlatAppearance->BorderSize = 1;
			ZoomButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(235, 235, 235);
			ZoomButton->TabStop = false;
			ZoomButton->Visible = false;
			this->HintToolTip->SetToolTip(ZoomButton, HintText);

		}

		System::Void ZoomButton_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {

			// Draws the + (two lines) or the - (one line) in the middle of the button
			System::Windows::Forms::Button^ ZoomButton = dynamic_cast<System::Windows::Forms::Button^>(sender);
			if (ZoomButton == nullptr) { return; }

			System::String^ Symbol = dynamic_cast<System::String^>(ZoomButton->Tag);
			if (Symbol == nullptr) { return; }

			float CenterX = (float)ZoomButton->ClientSize.Width / 2.0f;
			float CenterY = (float)ZoomButton->ClientSize.Height / 2.0f;

			System::Drawing::Pen^ SymbolPen = gcnew System::Drawing::Pen(_DataLogScrollBar_LineColor, 2.0f);

			e->Graphics->DrawLine(SymbolPen, CenterX - 3.0f, CenterY, CenterX + 3.0f, CenterY);
			if (Symbol->Equals("+")) { e->Graphics->DrawLine(SymbolPen, CenterX, CenterY - 3.0f, CenterX, CenterY + 3.0f); }

			delete SymbolPen;

		}

		System::Void BuildZoomButtons() {

			// Four small buttons on the graph, at the ends of the scroll bars of the zoomed axes (see UpdateZoomButtons): at the time axis scroll bar (below the graph) - and +
			// right of the right arrow, at the temperature axis scroll bar (left of the graph) + and - above its top end. They are beyond the ends of the bars, like the
			// Reset Zoom button at the other end, so the scroll bar thumb never goes under them.
			this->XZoomOutButton = gcnew System::Windows::Forms::Button();
			this->StyleZoomButton(this->XZoomOutButton, "-", "Zoom out the time axis (X): shows a longer time");
			this->XZoomOutButton->Click += gcnew System::EventHandler(this, &DataLogGUI::XZoomOutButton_Click);

			this->XZoomInButton = gcnew System::Windows::Forms::Button();
			this->StyleZoomButton(this->XZoomInButton, "+", "Zoom in the time axis (X): shows a shorter time");
			this->XZoomInButton->Click += gcnew System::EventHandler(this, &DataLogGUI::XZoomInButton_Click);

			this->YZoomInButton = gcnew System::Windows::Forms::Button();
			this->StyleZoomButton(this->YZoomInButton, "+", "Zoom in the temperature axis (Y): shows a smaller temperature range");
			this->YZoomInButton->Click += gcnew System::EventHandler(this, &DataLogGUI::YZoomInButton_Click);

			this->YZoomOutButton = gcnew System::Windows::Forms::Button();
			this->StyleZoomButton(this->YZoomOutButton, "-", "Zoom out the temperature axis (Y): shows a larger temperature range");
			this->YZoomOutButton->Click += gcnew System::EventHandler(this, &DataLogGUI::YZoomOutButton_Click);

			this->LogChart->Controls->Add(this->XZoomOutButton);
			this->LogChart->Controls->Add(this->XZoomInButton);
			this->LogChart->Controls->Add(this->YZoomInButton);
			this->LogChart->Controls->Add(this->YZoomOutButton);

		}

		System::Void SetZoomButtonPlace(System::Windows::Forms::Button^ ZoomButton, int ButtonLeft, int ButtonTop, bool ShowButton) {

			// Only written when it changes (this is called at every drawing of the graph)
			if (ZoomButton->Left != ButtonLeft || ZoomButton->Top != ButtonTop) { ZoomButton->Location = System::Drawing::Point(ButtonLeft, ButtonTop); }
			if (ZoomButton->Visible != ShowButton) { ZoomButton->Visible = ShowButton; }
			if (ShowButton == true) { ZoomButton->BringToFront(); }

		}

		System::Void UpdateZoomButtons() {

			// Shows the buttons of an axis only while that axis is zoomed, at the end of its scroll bar. The scroll bars are drawn by the chart: they are about 14 pixels
			// thick and lie against the plot area - the time axis one below it, with the Reset Zoom button and the left arrow at its left end and the right arrow at its
			// right end, the temperature axis one left of it, with the Reset Zoom button and the up arrow at its top end and the down arrow at its bottom end. The buttons are
			// put beyond the end of the bar (the time axis ones right of the right arrow, the temperature axis ones above the top end), in the margins that
			// UpdatePlotAreaLayout keeps free, so they never cover the track, the labels or the plot area.
			if (this->XZoomInButton == nullptr || this->XZoomOutButton == nullptr || this->YZoomInButton == nullptr || this->YZoomOutButton == nullptr) { return; }

			try {

				System::Windows::Forms::DataVisualization::Charting::ChartArea^ Area = this->LogChart->ChartAreas[0];

				System::Drawing::RectangleF PlotRect;
				double XMinimum = 0.0;
				double XMaximum = 0.0;

				bool Ready = this->GetPlotAreaRect(PlotRect, XMinimum, XMaximum);
				bool ShowX = (Ready == true && Area->AxisX->ScaleView->IsZoomed == true);
				bool ShowY = (Ready == true && Area->AxisY->ScaleView->IsZoomed == true);

				if (Ready == false) {

					if (this->XZoomInButton->Visible == true) { this->XZoomInButton->Visible = false; }
					if (this->XZoomOutButton->Visible == true) { this->XZoomOutButton->Visible = false; }
					if (this->YZoomInButton->Visible == true) { this->YZoomInButton->Visible = false; }
					if (this->YZoomOutButton->Visible == true) { this->YZoomOutButton->Visible = false; }
					return;

				}

				int BarSize = (int)System::Math::Round(Area->AxisX->ScrollBar->Size);
				if (BarSize < 10 || BarSize > 40) { BarSize = 14; }

				int ButtonSize = this->XZoomInButton->Width;

				// Time axis: the bar is below the plot area; - and + are right of its right arrow (moved left when the graph has no room there)
				int XButtonTop = (int)System::Math::Round(PlotRect.Bottom) + (BarSize - ButtonSize) / 2;
				int XOutLeft = (int)System::Math::Round(PlotRect.Right) + 3;
				int XInLeft = XOutLeft + ButtonSize + 1;

				if (XInLeft + ButtonSize > this->LogChart->ClientSize.Width) {

					XInLeft = this->LogChart->ClientSize.Width - ButtonSize;
					XOutLeft = XInLeft - 1 - ButtonSize;

				}

				// Temperature axis: the bar is left of the plot area; + and - are next to each other above the top of the bar (above its Reset Zoom button), in the top
				// margin of the plot area (so they never cover the labels of the time axis)
				int YButtonTop = (int)System::Math::Round(PlotRect.Top) - ButtonSize - 2;
				int YInLeft = (int)System::Math::Round(PlotRect.Left) - BarSize;
				int YOutLeft = YInLeft + ButtonSize + 1;

				this->SetZoomButtonPlace(this->XZoomOutButton, XOutLeft, XButtonTop, ShowX);
				this->SetZoomButtonPlace(this->XZoomInButton, XInLeft, XButtonTop, ShowX);
				this->SetZoomButtonPlace(this->YZoomInButton, YInLeft, YButtonTop, ShowY);
				this->SetZoomButtonPlace(this->YZoomOutButton, YOutLeft, YButtonTop, ShowY);

			}
			catch (System::Exception^) {}

		}

		System::Void XZoomInButton_Click(System::Object^ sender, System::EventArgs^ e) { this->ZoomAxisByButton(true, true); }
		System::Void XZoomOutButton_Click(System::Object^ sender, System::EventArgs^ e) { this->ZoomAxisByButton(true, false); }
		System::Void YZoomInButton_Click(System::Object^ sender, System::EventArgs^ e) { this->ZoomAxisByButton(false, true); }
		System::Void YZoomOutButton_Click(System::Object^ sender, System::EventArgs^ e) { this->ZoomAxisByButton(false, false); }

		bool GetAxisFullRange(bool IsTimeAxis, double% RangeMinimum, double% RangeMaximum) {

			// The whole range of an axis (the time axis in days): the range of the axis itself, or - while the axis has not calculated it yet - the range of the logged data
			// of the lines that are shown. False when there is no data.
			try {

				System::Windows::Forms::DataVisualization::Charting::Axis^ TheAxis = (IsTimeAxis == true) ? this->LogChart->ChartAreas[0]->AxisX : this->LogChart->ChartAreas[0]->AxisY;

				double AxisMinimum = TheAxis->Minimum;
				double AxisMaximum = TheAxis->Maximum;

				if (!System::Double::IsNaN(AxisMinimum) && !System::Double::IsNaN(AxisMaximum) && AxisMaximum > AxisMinimum) {

					RangeMinimum = AxisMinimum;
					RangeMaximum = AxisMaximum;
					return true;

				}

				if (IsTimeAxis == true) {

					double FirstTime = 0.0;
					double LastTime = 0.0;

					if (this->GetMeasureTimeLimits(FirstTime, LastTime) == true && LastTime > FirstTime) {

						RangeMinimum = FirstTime;
						RangeMaximum = LastTime;
						return true;

					}

					return false;

				}

				bool Found = false;
				double LowestValue = 0.0;
				double HighestValue = 0.0;

				for (int i = 0; i < this->SeriesBuffers->Count && i < this->LogChart->Series->Count; i++) {

					if (this->LogChart->Series[i]->Enabled == false) { continue; }

					for (int j = 0; j < this->SeriesBuffers[i]->YValues->Count; j++) {

						double Value = this->SeriesBuffers[i]->YValues[j];
						if (System::Double::IsNaN(Value)) { continue; }

						if (Found == false || Value < LowestValue) { LowestValue = Value; }
						if (Found == false || Value > HighestValue) { HighestValue = Value; }
						Found = true;

					}

				}

				if (Found == true && HighestValue > LowestValue) {

					RangeMinimum = LowestValue;
					RangeMaximum = HighestValue;
					return true;

				}

			}
			catch (System::Exception^) {}

			return false;

		}

		System::Void ZoomAxisByButton(bool IsTimeAxis, bool ZoomIn) {

			// One click of a zoom button: the visible range of one axis becomes smaller (zoom in) or larger (zoom out), around its middle, and stays inside the whole range
			try {

				System::Windows::Forms::DataVisualization::Charting::Axis^ TheAxis = (IsTimeAxis == true) ? this->LogChart->ChartAreas[0]->AxisX : this->LogChart->ChartAreas[0]->AxisY;

				double FullMinimum = 0.0;
				double FullMaximum = 0.0;

				if (this->GetAxisFullRange(IsTimeAxis, FullMinimum, FullMaximum) == false) { return; }

				double FullSize = FullMaximum - FullMinimum;
				bool Zoomed = (TheAxis->ScaleView->IsZoomed == true);

				// Nothing to zoom out when the whole range is shown already
				if (ZoomIn == false && Zoomed == false) { return; }

				double ViewMinimum = (Zoomed == true) ? TheAxis->ScaleView->ViewMinimum : FullMinimum;
				double ViewMaximum = (Zoomed == true) ? TheAxis->ScaleView->ViewMaximum : FullMaximum;
				double ViewSize = ViewMaximum - ViewMinimum;

				if (System::Double::IsNaN(ViewSize) || ViewSize <= 0.0) { return; }

				double SmallestSize = (IsTimeAxis == true) ? (_DataLogZoomButton_MinTimeMilliSec / 86400000.0) : _DataLogZoomButton_MinTemperatureRange;

				if (ZoomIn == true && ViewSize <= SmallestSize * 1.001) { return; }

				double NewSize = (ZoomIn == true) ? (ViewSize * _DataLogZoomButton_Factor) : (ViewSize / _DataLogZoomButton_Factor);
				if (NewSize < SmallestSize) { NewSize = SmallestSize; }

				if (ZoomIn == false && NewSize >= FullSize * 0.999) {

					// The whole range again
					TheAxis->ScaleView->ZoomReset(0);

				}
				else {

					double Middle = ViewMinimum + (ViewSize / 2.0);
					double NewMinimum = Middle - (NewSize / 2.0);
					double NewMaximum = NewMinimum + NewSize;

					if (NewMinimum < FullMinimum) { NewMinimum = FullMinimum; NewMaximum = NewMinimum + NewSize; }
					if (NewMaximum > FullMaximum) { NewMaximum = FullMaximum; NewMinimum = NewMaximum - NewSize; }

					TheAxis->ScaleView->Zoom(NewMinimum, NewMaximum);

				}

				// The same updates as after a zoom rectangle: scroll steps, ticks, plot area and the orange border of Reset Zoom
				this->UpdateScrollSteps();
				this->ApplyAxisSettings();
				this->UpdateZoomIndicator();
				this->LogChart->Invalidate();

			}
			catch (System::Exception^) {}

		}

		// ------------------------- Graph markers (a note at a time in the graph) ------------------------- //

		// A marker is a vertical line in the graph with its time (h:mm:ss) and, below the time, a text. It is added at the time of the newest logged sample (Add Marker
		// button, while logging) or at the time where the graph is right clicked (Add Marker Here), and it can be dragged with the mouse (like the cursors of the two-cursor
		// measurement), deleted one by one (right click on the marker) or all at once. The markers are drawn on the chart itself (PostPaint), so they follow zoom and
		// resizing, and they are part of an exported image. They belong to the data of the graph: Clear Graph and a new log session remove them (they are not
		// saved in the log file).

		System::Void CreateMarkerMenuItems() {

			this->AddMarkerMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->AddMarkerMenuItem, "Add Marker Here...");
			this->AddMarkerMenuItem->Click += gcnew System::EventHandler(this, &DataLogGUI::AddMarkerMenuItem_Click);

			this->DeleteMarkerMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->DeleteMarkerMenuItem, "Delete Marker");
			this->DeleteMarkerMenuItem->Visible = false;
			this->DeleteMarkerMenuItem->Click += gcnew System::EventHandler(this, &DataLogGUI::DeleteMarkerMenuItem_Click);

			this->DeleteAllMarkersMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->DeleteAllMarkersMenuItem, "Delete All Markers");
			this->DeleteAllMarkersMenuItem->Click += gcnew System::EventHandler(this, &DataLogGUI::DeleteAllMarkersMenuItem_Click);

			this->MarkerSeparator = gcnew System::Windows::Forms::ToolStripSeparator();

		}

		System::String^ MarkerDisplayText(GraphMarker^ Marker) {

			// The time of a marker, followed by its text (for the menu)
			System::String^ DisplayText = this->FormatAxisTime(Marker->ElapsedMilliSec, false);
			if (Marker->Text != nullptr && Marker->Text->Length > 0) { DisplayText = DisplayText + gcnew System::String("  ") + Marker->Text; }

			return DisplayText;

		}

		System::Void UpdateMarkerMenuItems() {

			// Called when the right click menu opens: where the mouse is decides what the marker items do
			System::Drawing::Point MousePoint = this->LogChart->PointToClient(System::Windows::Forms::Control::MousePosition);

			this->MarkerMenuTimeValid = false;
			this->MarkerMenuTarget = nullptr;

			double TimeValue = 0.0;

			if (this->IsPointInPlotArea(MousePoint.X, MousePoint.Y) == true && this->GetMeasureTimeAtMouse(MousePoint.X, TimeValue) == true) {

				this->MarkerMenuTime = TimeValue;
				this->MarkerMenuTimeValid = true;

			}

			this->AddMarkerMenuItem->Enabled = this->MarkerMenuTimeValid;

			// The item that deletes one marker is only shown when the mouse is on a marker (its line or its text)
			int Hit = this->FindMarkerAt(MousePoint.X, MousePoint.Y);

			if (Hit >= 0) {

				this->MarkerMenuTarget = this->GraphMarkers[Hit];
				this->DeleteMarkerMenuItem->Text = gcnew System::String("Delete Marker: ") + this->MarkerDisplayText(this->MarkerMenuTarget);
				this->DeleteMarkerMenuItem->Visible = true;

			}
			else { this->DeleteMarkerMenuItem->Visible = false; }

			this->DeleteAllMarkersMenuItem->Enabled = (this->GraphMarkers->Count > 0);

		}

		System::Void ClearGraphMarkers() {

			// Removes all markers (nothing to do before the graph objects are built)
			if (this->GraphMarkers == nullptr) { return; }

			this->GraphMarkers->Clear();
			this->MarkerDragging = -1;
			this->MarkerMenuTarget = nullptr;

			if (this->LogChart != nullptr) { this->LogChart->Invalidate(); }

		}

		bool ShowMarkerTextWindow(System::String^% MarkerText) {

			// The window for the text of a marker. Returns true when it was closed with Add Marker (the text can be empty: the marker then shows only its time).
			System::Windows::Forms::Form^ MarkerWindow = gcnew System::Windows::Forms::Form();
			MarkerWindow->Text = "Add Marker";
			MarkerWindow->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			MarkerWindow->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			MarkerWindow->MinimizeBox = false;
			MarkerWindow->MaximizeBox = false;
			MarkerWindow->ShowInTaskbar = false;
			MarkerWindow->BackColor = System::Drawing::Color::White;
			MarkerWindow->ForeColor = System::Drawing::Color::Black;
			MarkerWindow->ClientSize = System::Drawing::Size(420, 130);

			System::Windows::Forms::Label^ MarkerLabel = gcnew System::Windows::Forms::Label();
			MarkerLabel->Text = "Marker text (shown below the time of the marker):";
			MarkerLabel->AutoSize = false;
			MarkerLabel->Location = System::Drawing::Point(20, 16);
			MarkerLabel->Size = System::Drawing::Size(380, 22);
			MarkerWindow->Controls->Add(MarkerLabel);

			System::Windows::Forms::TextBox^ MarkerTextBox = gcnew System::Windows::Forms::TextBox();
			MarkerTextBox->MaxLength = _DataLogMarker_MaxTextLength;
			MarkerTextBox->Text = MarkerText;
			MarkerTextBox->Location = System::Drawing::Point(20, 42);
			MarkerTextBox->Size = System::Drawing::Size(380, 24);
			MarkerWindow->Controls->Add(MarkerTextBox);

			System::Windows::Forms::Button^ MarkerOkButton = gcnew System::Windows::Forms::Button();
			MarkerOkButton->Text = "Add Marker";
			MarkerOkButton->DialogResult = System::Windows::Forms::DialogResult::OK;
			MarkerOkButton->Location = System::Drawing::Point(95, 84);
			MarkerOkButton->Size = System::Drawing::Size(110, 30);
			MarkerWindow->Controls->Add(MarkerOkButton);
			MarkerWindow->AcceptButton = MarkerOkButton;

			System::Windows::Forms::Button^ MarkerCancelButton = gcnew System::Windows::Forms::Button();
			MarkerCancelButton->Text = "Cancel";
			MarkerCancelButton->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			MarkerCancelButton->Location = System::Drawing::Point(215, 84);
			MarkerCancelButton->Size = System::Drawing::Size(110, 30);
			MarkerWindow->Controls->Add(MarkerCancelButton);
			MarkerWindow->CancelButton = MarkerCancelButton;

			MarkerWindow->ActiveControl = MarkerTextBox;

			if (MarkerWindow->ShowDialog(this) == System::Windows::Forms::DialogResult::OK) {

				MarkerText = MarkerTextBox->Text->Trim();
				return true;

			}

			return false;

		}

		System::Void AddMarkerAt(double ElapsedMilliSec) {

			// The marker is placed at once (at the time of the click, not at the time the text window is closed) and gets its text in the window. Cancel removes it again.
			GraphMarker^ NewMarker = gcnew GraphMarker();
			NewMarker->ElapsedMilliSec = ElapsedMilliSec;
			NewMarker->Text = "";

			this->GraphMarkers->Add(NewMarker);
			this->LogChart->Invalidate();

			System::String^ NewText = "";

			if (this->ShowMarkerTextWindow(NewText) == true) { NewMarker->Text = NewText; }
			else { this->GraphMarkers->Remove(NewMarker); }

			this->LogChart->Invalidate();

		}

		System::Void AddMarkerButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// A marker at the current time: the time of the newest logged sample
			if (DataLoggingIsRunningFlag == false) {

				System::Windows::Forms::MessageBox::Show(this, "The Add Marker button adds a marker at the current time of the data logging, so data logging must be running.\r\n\r\nTo add a marker at another time (also for a log that has stopped), right-click the graph at that time and choose Add Marker Here.",
					"Add Marker", System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Information);
				return;

			}

			// Bring the graph up to date first, so that the newest logged sample is known
			this->RefreshLogView();

			if (this->ViewedSampleRows == 0) { return; }

			this->AddMarkerAt(this->LastElapsedMilliSec);

		}

		System::Void AddMarkerMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			// A marker at the time where the graph was right clicked
			if (this->MarkerMenuTimeValid == false) { return; }

			double ElapsedMilliSec = (this->MarkerMenuTime - this->ChartStartTime.ToOADate()) * 86400000.0;
			if (ElapsedMilliSec < 0.0) { ElapsedMilliSec = 0.0; }

			this->AddMarkerAt(ElapsedMilliSec);

		}

		System::Void DeleteMarkerMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			// Deletes the marker that was right clicked
			if (this->MarkerMenuTarget == nullptr) { return; }

			this->GraphMarkers->Remove(this->MarkerMenuTarget);
			this->MarkerMenuTarget = nullptr;
			this->LogChart->Invalidate();

		}

		System::Void DeleteAllMarkersMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			if (this->GraphMarkers->Count == 0) { return; }

			System::Windows::Forms::DialogResult Answer = System::Windows::Forms::MessageBox::Show(this,
				"Are you sure you want to delete all markers?", "Delete All Markers",
				System::Windows::Forms::MessageBoxButtons::YesNo, System::Windows::Forms::MessageBoxIcon::Question, System::Windows::Forms::MessageBoxDefaultButton::Button2);

			if (Answer != System::Windows::Forms::DialogResult::Yes) { return; }

			this->ClearGraphMarkers();

		}

		bool GetPlotAreaRect(System::Drawing::RectangleF% PlotRect, double% XMinimum, double% XMaximum) {

			// The plot area of the graph in pixels, and the time range (days) that is shown. The same test as IsPointInPlotArea. False when the axes are not ready.
			try {

				System::Windows::Forms::DataVisualization::Charting::Axis^ AxisX = this->LogChart->ChartAreas[0]->AxisX;
				System::Windows::Forms::DataVisualization::Charting::Axis^ AxisY = this->LogChart->ChartAreas[0]->AxisY;

				double XMin = (AxisX->ScaleView->IsZoomed == true) ? AxisX->ScaleView->ViewMinimum : AxisX->Minimum;
				double XMax = (AxisX->ScaleView->IsZoomed == true) ? AxisX->ScaleView->ViewMaximum : AxisX->Maximum;
				double YMin = (AxisY->ScaleView->IsZoomed == true) ? AxisY->ScaleView->ViewMinimum : AxisY->Minimum;
				double YMax = (AxisY->ScaleView->IsZoomed == true) ? AxisY->ScaleView->ViewMaximum : AxisY->Maximum;

				if (System::Double::IsNaN(XMin) || System::Double::IsNaN(XMax) || System::Double::IsNaN(YMin) || System::Double::IsNaN(YMax)) { return false; }

				double PixelA = AxisX->ValueToPixelPosition(XMin);
				double PixelB = AxisX->ValueToPixelPosition(XMax);
				double PixelC = AxisY->ValueToPixelPosition(YMin);
				double PixelD = AxisY->ValueToPixelPosition(YMax);

				if (System::Double::IsNaN(PixelA) || System::Double::IsNaN(PixelB) || System::Double::IsNaN(PixelC) || System::Double::IsNaN(PixelD)) { return false; }

				PlotRect = System::Drawing::RectangleF((float)System::Math::Min(PixelA, PixelB), (float)System::Math::Min(PixelC, PixelD),
					(float)System::Math::Abs(PixelB - PixelA), (float)System::Math::Abs(PixelD - PixelC));
				XMinimum = XMin;
				XMaximum = XMax;

				return true;

			}
			catch (System::Exception^) {}

			return false;

		}

		System::Void LogChart_PostPaint(System::Object^ sender, System::Windows::Forms::DataVisualization::Charting::ChartPaintEventArgs^ e) {

			if (dynamic_cast<System::Windows::Forms::DataVisualization::Charting::ChartArea^>(e->ChartElement) == nullptr) { return; }

			// The zoom buttons follow the plot area and the zoom (the positions are known after the plot area is drawn)
			this->UpdateZoomButtons();

			// The markers are drawn after the plot area (so they are also in an exported image)
			if (this->GraphMarkers->Count == 0) { return; }

			try { this->DrawMarkers(e->ChartGraphics->Graphics); }
			catch (System::Exception^) {}

		}

		System::Void DrawMarkers(System::Drawing::Graphics^ Gfx) {

			for (int i = 0; i < this->GraphMarkers->Count; i++) { this->GraphMarkers[i]->Drawn = false; }

			System::Drawing::RectangleF PlotRect;
			double XMinimum = 0.0;
			double XMaximum = 0.0;

			if (this->GetPlotAreaRect(PlotRect, XMinimum, XMaximum) == false) { return; }
			if (PlotRect.Width < 4.0f || PlotRect.Height < 4.0f) { return; }

			System::Windows::Forms::DataVisualization::Charting::Axis^ AxisX = this->LogChart->ChartAreas[0]->AxisX;

			// The markers in the order of their time, so that the texts that would be on top of each other are put in rows below each other
			System::Collections::Generic::List<GraphMarker^>^ Ordered = gcnew System::Collections::Generic::List<GraphMarker^>();

			for (int i = 0; i < this->GraphMarkers->Count; i++) {

				int Position = Ordered->Count;
				while (Position > 0 && Ordered[Position - 1]->ElapsedMilliSec > this->GraphMarkers[i]->ElapsedMilliSec) { Position--; }
				Ordered->Insert(Position, this->GraphMarkers[i]);

			}

			if (this->MarkerFont == nullptr) { this->MarkerFont = gcnew System::Drawing::Font("Arial", 9.0f, System::Drawing::FontStyle::Bold); }

			System::Drawing::Color MarkerColor = System::Drawing::Color::FromArgb(255, 150, 0);
			System::Drawing::Pen^ LinePen = gcnew System::Drawing::Pen(MarkerColor, 2.0f);
			System::Drawing::Pen^ BorderPen = gcnew System::Drawing::Pen(MarkerColor, 1.0f);
			System::Drawing::SolidBrush^ BackBrush = gcnew System::Drawing::SolidBrush(System::Drawing::Color::FromArgb(215, 20, 20, 20));
			System::Drawing::SolidBrush^ TimeBrush = gcnew System::Drawing::SolidBrush(MarkerColor);
			System::Drawing::SolidBrush^ TextBrush = gcnew System::Drawing::SolidBrush(System::Drawing::Color::White);
			System::Collections::Generic::List<System::Drawing::RectangleF>^ PlacedBoxes = gcnew System::Collections::Generic::List<System::Drawing::RectangleF>();

			System::Drawing::Drawing2D::GraphicsState^ SavedState = Gfx->Save();

			try {

				Gfx->SetClip(PlotRect);

				for (int i = 0; i < Ordered->Count; i++) {

					GraphMarker^ Marker = Ordered[i];

					double TimeValue = this->ChartStartTime.AddMilliseconds(Marker->ElapsedMilliSec).ToOADate();
					if (TimeValue < XMinimum || TimeValue > XMaximum) { continue; }

					float LineX = (float)AxisX->ValueToPixelPosition(TimeValue);
					if (System::Single::IsNaN(LineX)) { continue; }

					Gfx->DrawLine(LinePen, LineX, PlotRect.Top, LineX, PlotRect.Bottom);

					// The time on the first line, the text below it
					System::String^ TimeLine = this->FormatAxisTime(Marker->ElapsedMilliSec, false);
					System::String^ TextLine = (Marker->Text != nullptr) ? Marker->Text : gcnew System::String("");

					System::Drawing::SizeF TimeSize = Gfx->MeasureString(TimeLine, this->MarkerFont);
					System::Drawing::SizeF TextSize = System::Drawing::SizeF(0.0f, 0.0f);
					if (TextLine->Length > 0) { TextSize = Gfx->MeasureString(TextLine, this->MarkerFont); }

					float BoxWidth = System::Math::Max(TimeSize.Width, TextSize.Width) + 8.0f;
					float BoxHeight = TimeSize.Height + TextSize.Height + 4.0f;

					// Right of the line, or left of it when it would leave the plot area
					float BoxX = LineX + 4.0f;
					if (BoxX + BoxWidth > PlotRect.Right - 1.0f) { BoxX = LineX - 4.0f - BoxWidth; }
					if (BoxX < PlotRect.Left + 1.0f) { BoxX = PlotRect.Left + 1.0f; }

					// The first row where the box does not cover another one
					float BoxY = PlotRect.Top + 3.0f;
					bool Moved = true;

					for (int Guard = 0; Moved == true && Guard < 40; Guard++) {

						Moved = false;
						System::Drawing::RectangleF Candidate = System::Drawing::RectangleF(BoxX, BoxY, BoxWidth, BoxHeight);

						for (int p = 0; p < PlacedBoxes->Count; p++) {

							System::Drawing::RectangleF Other = PlacedBoxes[p];

							if (Candidate.IntersectsWith(Other)) { BoxY = Other.Bottom + 2.0f; Moved = true; break; }

						}

					}

					if (BoxY + BoxHeight > PlotRect.Bottom) { BoxY = PlotRect.Top + 3.0f; }

					System::Drawing::RectangleF Box = System::Drawing::RectangleF(BoxX, BoxY, BoxWidth, BoxHeight);
					PlacedBoxes->Add(Box);

					Gfx->FillRectangle(BackBrush, Box);
					Gfx->DrawRectangle(BorderPen, Box.X, Box.Y, Box.Width, Box.Height);
					Gfx->DrawString(TimeLine, this->MarkerFont, TimeBrush, BoxX + 4.0f, BoxY + 2.0f);
					if (TextLine->Length > 0) { Gfx->DrawString(TextLine, this->MarkerFont, TextBrush, BoxX + 4.0f, BoxY + 2.0f + TimeSize.Height); }

					// Where the marker is on screen (for picking it with the mouse)
					Marker->PixelX = LineX;
					Marker->LabelRect = Box;
					Marker->Drawn = true;

				}

			}
			finally {

				Gfx->Restore(SavedState);
				delete LinePen;
				delete BorderPen;
				delete BackBrush;
				delete TimeBrush;
				delete TextBrush;

			}

		}

		int FindMarkerAt(int MouseX, int MouseY) {

			// The marker whose line (a few pixels wide) or text is at the mouse position, or -1. Uses the positions of the last drawing.
			if (this->GraphMarkers->Count == 0) { return -1; }
			if (this->IsPointInPlotArea(MouseX, MouseY) == false) { return -1; }

			for (int i = this->GraphMarkers->Count - 1; i >= 0; i--) {

				GraphMarker^ Marker = this->GraphMarkers[i];
				if (Marker->Drawn == false) { continue; }

				if (Marker->LabelRect.Contains((float)MouseX, (float)MouseY)) { return i; }
				if (System::Math::Abs((float)MouseX - Marker->PixelX) <= 5.0f) { return i; }

			}

			return -1;

		}

		System::Void UpdateMarkerHover(int MouseX, int MouseY) {

			// While the mouse is on a marker (or a marker is dragged) the cursor is a horizontal arrow and the zoom rectangle is switched off, so a drag moves the marker
			// and does not zoom. It comes back when the mouse leaves the marker (unless the two-cursor measurement has it switched off).
			bool Over = (this->MarkerDragging >= 0) || (this->FindMarkerAt(MouseX, MouseY) >= 0);

			if (Over == this->MarkerHoverActive) { return; }

			this->MarkerHoverActive = Over;

			System::Windows::Forms::DataVisualization::Charting::ChartArea^ Area = this->LogChart->ChartAreas[0];
			Area->CursorX->IsUserSelectionEnabled = (Over == false) && (this->MeasureModeEnabled == false);
			Area->CursorY->IsUserSelectionEnabled = (Over == false) && (this->MeasureModeEnabled == false);

			if (Over == true) { this->LogChart->Cursor = System::Windows::Forms::Cursors::SizeWE; }
			else if (this->MeasureModeEnabled == true) { this->LogChart->Cursor = System::Windows::Forms::Cursors::Cross; }
			else { this->LogChart->Cursor = System::Windows::Forms::Cursors::Default; }

		}

		// ------------------------- Two-cursor measurement ------------------------- //

		System::Void BuildMeasureObjects() {

			// The two cursor lines (annotations of the chart, hidden until placed) and the readout panel
			System::Drawing::Color ColorA = System::Drawing::Color::FromArgb(255, 220, 60);
			System::Drawing::Color ColorB = System::Drawing::Color::FromArgb(60, 220, 255);

			for (int k = 0; k < 2; k++) {

				System::Windows::Forms::DataVisualization::Charting::VerticalLineAnnotation^ Line = gcnew System::Windows::Forms::DataVisualization::Charting::VerticalLineAnnotation();
				Line->Name = (k == 0) ? gcnew System::String("MeasureCursorA") : gcnew System::String("MeasureCursorB");
				Line->AxisX = this->LogChart->ChartAreas[0]->AxisX;
				Line->IsInfinitive = true;
				Line->ClipToChartArea = this->LogChart->ChartAreas[0]->Name;
				Line->LineColor = (k == 0) ? ColorA : ColorB;
				Line->LineWidth = 2;
				Line->LineDashStyle = System::Windows::Forms::DataVisualization::Charting::ChartDashStyle::Dash;
				Line->AllowMoving = false;
				Line->AllowSelecting = false;
				Line->Y = 0;
				Line->Height = 100;
				Line->X = 0;
				Line->Visible = false;
				this->LogChart->Annotations->Add(Line);

				if (k == 0) { this->MeasureLineA = Line; }
				else { this->MeasureLineB = Line; }

			}

			this->MeasurePanel = gcnew System::Windows::Forms::FlowLayoutPanel();
			this->MeasurePanel->FlowDirection = System::Windows::Forms::FlowDirection::TopDown;
			this->MeasurePanel->WrapContents = false;
			this->MeasurePanel->AutoSize = true;
			this->MeasurePanel->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
			this->MeasurePanel->BackColor = System::Drawing::Color::FromArgb(20, 20, 20);
			this->MeasurePanel->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->MeasurePanel->Padding = System::Windows::Forms::Padding(4);
			this->MeasurePanel->Cursor = System::Windows::Forms::Cursors::SizeAll;
			this->MeasurePanel->Visible = false;
			this->MeasurePanel->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &DataLogGUI::MeasurePanel_MouseDown);
			this->MeasurePanel->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &DataLogGUI::MeasurePanel_MouseMove);
			this->MeasurePanel->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &DataLogGUI::MeasurePanel_MouseUp);
			this->LogChart->Controls->Add(this->MeasurePanel);

		}

		System::Void MeasurePanel_MouseDown(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {

			// The panel is moved by dragging it (the sender is the panel or one of its text rows; the screen position of the mouse is used)
			if (e->Button != System::Windows::Forms::MouseButtons::Left) { return; }

			this->MeasurePanelDragging = true;
			this->MeasurePanelDragStart = System::Windows::Forms::Control::MousePosition;
			this->MeasurePanelDragOrigin = this->MeasurePanel->Location;

		}

		System::Void MeasurePanel_MouseMove(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {

			if (this->MeasurePanelDragging == false) { return; }

			System::Drawing::Point Now = System::Windows::Forms::Control::MousePosition;
			this->MeasurePanel->Location = System::Drawing::Point(this->MeasurePanelDragOrigin.X + (Now.X - this->MeasurePanelDragStart.X),
				this->MeasurePanelDragOrigin.Y + (Now.Y - this->MeasurePanelDragStart.Y));
			this->KeepMeasurePanelInside();

		}

		System::Void MeasurePanel_MouseUp(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {

			this->MeasurePanelDragging = false;

		}

		System::Void KeepMeasurePanelInside() {

			// The panel stays inside the graph
			int X = this->MeasurePanel->Left;
			int Y = this->MeasurePanel->Top;

			if (X + this->MeasurePanel->Width > this->LogChart->ClientSize.Width) { X = this->LogChart->ClientSize.Width - this->MeasurePanel->Width; }
			if (Y + this->MeasurePanel->Height > this->LogChart->ClientSize.Height) { Y = this->LogChart->ClientSize.Height - this->MeasurePanel->Height; }
			if (X < 0) { X = 0; }
			if (Y < 0) { Y = 0; }

			if (X != this->MeasurePanel->Left || Y != this->MeasurePanel->Top) { this->MeasurePanel->Location = System::Drawing::Point(X, Y); }

		}

		bool IsPointInPlotArea(int MouseX, int MouseY) {

			// True when the point is inside the plot area of the graph (not on the axes, the scroll bars or outside the graph). A cursor line of the two-cursor
			// measurement counts as plot area. The test is by position (the pixel positions of the ends of the shown axis ranges), not by the chart's hit test:
			// over an annotation (the cursor lines) the hit test reports an annotation without a chart area, so a hit test alone treated the lines as outside the graph.
			try {

				System::Windows::Forms::DataVisualization::Charting::Axis^ AxisX = this->LogChart->ChartAreas[0]->AxisX;
				System::Windows::Forms::DataVisualization::Charting::Axis^ AxisY = this->LogChart->ChartAreas[0]->AxisY;

				double XMinimum = (AxisX->ScaleView->IsZoomed == true) ? AxisX->ScaleView->ViewMinimum : AxisX->Minimum;
				double XMaximum = (AxisX->ScaleView->IsZoomed == true) ? AxisX->ScaleView->ViewMaximum : AxisX->Maximum;
				double YMinimum = (AxisY->ScaleView->IsZoomed == true) ? AxisY->ScaleView->ViewMinimum : AxisY->Minimum;
				double YMaximum = (AxisY->ScaleView->IsZoomed == true) ? AxisY->ScaleView->ViewMaximum : AxisY->Maximum;

				if (!System::Double::IsNaN(XMinimum) && !System::Double::IsNaN(XMaximum) && !System::Double::IsNaN(YMinimum) && !System::Double::IsNaN(YMaximum)) {

					double PixelA = AxisX->ValueToPixelPosition(XMinimum);
					double PixelB = AxisX->ValueToPixelPosition(XMaximum);
					double PixelC = AxisY->ValueToPixelPosition(YMinimum);
					double PixelD = AxisY->ValueToPixelPosition(YMaximum);

					if (!System::Double::IsNaN(PixelA) && !System::Double::IsNaN(PixelB) && !System::Double::IsNaN(PixelC) && !System::Double::IsNaN(PixelD)) {

						return ((double)MouseX >= System::Math::Min(PixelA, PixelB) && (double)MouseX <= System::Math::Max(PixelA, PixelB) &&
							(double)MouseY >= System::Math::Min(PixelC, PixelD) && (double)MouseY <= System::Math::Max(PixelC, PixelD));

					}

				}

			}
			catch (System::Exception^) {}

			// Fallback (axes not ready): the hit test of the chart
			try {

				System::Windows::Forms::DataVisualization::Charting::HitTestResult^ Hit = this->LogChart->HitTest(MouseX, MouseY);
				System::Windows::Forms::DataVisualization::Charting::ChartElementType Element = Hit->ChartElementType;

				if (Element == System::Windows::Forms::DataVisualization::Charting::ChartElementType::Annotation) { return true; }

				return (Hit->ChartArea != nullptr && (Element == System::Windows::Forms::DataVisualization::Charting::ChartElementType::PlottingArea ||
					Element == System::Windows::Forms::DataVisualization::Charting::ChartElementType::DataPoint ||
					Element == System::Windows::Forms::DataVisualization::Charting::ChartElementType::Gridlines ||
					Element == System::Windows::Forms::DataVisualization::Charting::ChartElementType::StripLines));

			}
			catch (System::Exception^) {}

			return false;

		}

		bool GetMeasureTimeLimits(double% FirstTime, double% LastTime) {

			// The logged time range (in days) of the lines that are shown: a cursor stays inside it, so every line has a value at the cursor (false when there is no data)
			bool Found = false;

			for (int i = 0; i < this->SeriesBuffers->Count && i < this->LogChart->Series->Count; i++) {

				if (this->LogChart->Series[i]->Enabled == false) { continue; }

				int Count = this->SeriesBuffers[i]->XValues->Count;
				if (Count == 0) { continue; }

				double LineFirst = this->SeriesBuffers[i]->XValues[0];
				double LineLast = this->SeriesBuffers[i]->XValues[Count - 1];

				if (Found == false || LineFirst < FirstTime) { FirstTime = LineFirst; }
				if (Found == false || LineLast > LastTime) { LastTime = LineLast; }
				Found = true;

			}

			return Found;

		}

		bool GetMeasureTimeAtMouse(int MouseX, double% TimeValue) {

			// The time (in days) at the horizontal mouse position, limited to the logged time range. False when there is nothing to measure yet.
			double FirstTime = 0.0;
			double LastTime = 0.0;

			if (this->GetMeasureTimeLimits(FirstTime, LastTime) == false) { return false; }

			try {

				double Value = this->LogChart->ChartAreas[0]->AxisX->PixelPositionToValue((double)MouseX);

				if (System::Double::IsNaN(Value)) { return false; }
				if (Value < FirstTime) { Value = FirstTime; }
				if (Value > LastTime) { Value = LastTime; }

				TimeValue = Value;
				return true;

			}
			catch (System::Exception^) {}

			return false;

		}

		bool GetLineValueAt(int SeriesIndex, double TimeValue, double% Value) {

			// The value of a line at a time (days): the sample itself, or linear interpolation between the two neighbouring samples. False outside the logged time range
			// of the line and in a gap (a NaN sample next to the time).
			if (SeriesIndex < 0 || SeriesIndex >= this->SeriesBuffers->Count) { return false; }

			LogSeriesBuffer^ Buffer = this->SeriesBuffers[SeriesIndex];
			int Count = Buffer->XValues->Count;

			if (Count == 0 || Buffer->YValues->Count != Count) { return false; }
			if (TimeValue < Buffer->XValues[0] || TimeValue > Buffer->XValues[Count - 1]) { return false; }

			int Index = Buffer->XValues->BinarySearch(TimeValue);

			if (Index >= 0) {

				double Exact = Buffer->YValues[Index];
				if (System::Double::IsNaN(Exact)) { return false; }

				Value = Exact;
				return true;

			}

			Index = ~Index;     // The index of the first sample that is later than the time

			if (Index <= 0 || Index >= Count) { return false; }

			double X0 = Buffer->XValues[Index - 1];
			double X1 = Buffer->XValues[Index];
			double Y0 = Buffer->YValues[Index - 1];
			double Y1 = Buffer->YValues[Index];

			if (System::Double::IsNaN(Y0) || System::Double::IsNaN(Y1)) { return false; }

			if (X1 - X0 <= 0.0) { Value = Y0; }
			else { Value = Y0 + ((Y1 - Y0) * ((TimeValue - X0) / (X1 - X0))); }

			return true;

		}

		System::Void LogChart_MouseDown(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {

			// A left click on a marker (its line or its text) picks it: keep the button down and move the mouse to drag it. This works with and without the two-cursor measurement.
			if (e->Button == System::Windows::Forms::MouseButtons::Left) {

				int MarkerHit = this->FindMarkerAt(e->X, e->Y);

				if (MarkerHit >= 0) {

					this->MarkerDragging = MarkerHit;
					this->UpdateMarkerHover(e->X, e->Y);
					return;

				}

			}

			// With Measure With Two Cursors on, a left click in the plot area places a cursor: the first click A, the second B; after that the nearer cursor moves.
			// Keep the button down and move the mouse to fine-tune the position.
			if (this->MeasureModeEnabled == false || e->Button != System::Windows::Forms::MouseButtons::Left) { return; }
			if (this->IsPointInPlotArea(e->X, e->Y) == false) { return; }

			double TimeValue = 0.0;
			if (this->GetMeasureTimeAtMouse(e->X, TimeValue) == false) { return; }

			int Index = 0;

			if (System::Double::IsNaN(this->MeasureTimeA)) { Index = 0; }
			else if (System::Double::IsNaN(this->MeasureTimeB)) { Index = 1; }
			else if (System::Math::Abs(TimeValue - this->MeasureTimeA) <= System::Math::Abs(TimeValue - this->MeasureTimeB)) { Index = 0; }
			else { Index = 1; }

			this->MeasureDragging = Index;
			this->SetMeasureCursor(Index, TimeValue);

		}

		System::Void LogChart_MouseUp(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {

			this->MeasureDragging = -1;

			// A dragged marker is dropped (the arrow cursor stays while the mouse is still on it)
			if (this->MarkerDragging >= 0) { this->MarkerDragging = -1; this->UpdateMarkerHover(e->X, e->Y); }

		}

		System::Void SetMeasureCursor(int Index, double TimeValue) {

			// Places cursor A (0) or B (1) at a time (days) and updates the readout
			if (Index == 0) { this->MeasureTimeA = TimeValue; this->MeasureLineA->X = TimeValue; this->MeasureLineA->Visible = true; }
			else { this->MeasureTimeB = TimeValue; this->MeasureLineB->X = TimeValue; this->MeasureLineB->Visible = true; }

			this->UpdateMeasurement();

		}

		System::Void ClearMeasureCursors() {

			// Removes both cursors and the readout (nothing to do before the graph objects are built)
			if (this->MeasureLineA == nullptr || this->MeasureLineB == nullptr || this->MeasurePanel == nullptr) { return; }

			this->MeasureTimeA = System::Double::NaN;
			this->MeasureTimeB = System::Double::NaN;
			this->MeasureDragging = -1;
			this->MeasureLineA->Visible = false;
			this->MeasureLineB->Visible = false;
			this->UpdateMeasurement();

		}

		System::Void ApplyMeasureMode() {

			// While cursors are placed with the mouse the zoom rectangle is switched off (a click that moves a little would zoom in). Switching the mode off
			// brings the zoom back and keeps the cursors and the readout until Clear Cursors.
			System::Windows::Forms::DataVisualization::Charting::ChartArea^ Area = this->LogChart->ChartAreas[0];

			Area->CursorX->IsUserSelectionEnabled = !this->MeasureModeEnabled;
			Area->CursorY->IsUserSelectionEnabled = !this->MeasureModeEnabled;

			if (this->MeasureModeEnabled == true) { this->LogChart->Cursor = System::Windows::Forms::Cursors::Cross; }
			else { this->LogChart->Cursor = System::Windows::Forms::Cursors::Default; this->MeasureDragging = -1; }

			this->UpdateMeasurement();

		}

		System::String^ FormatMeasureTime(double TimeValue) {

			// Elapsed time of a cursor, h:mm:ss.fff
			double ElapsedMilliSec = (TimeValue - this->ChartStartTime.ToOADate()) * 86400000.0;
			return this->FormatAxisTime(ElapsedMilliSec, true);

		}

		System::String^ FormatMeasureDeltaTime(double DeltaSeconds) {

			// Time difference B - A: signed seconds, and h:mm:ss in brackets from one minute on
			System::String^ Text = DeltaSeconds.ToString("+0.###;-0.###;0") + gcnew System::String(" s");

			if (System::Math::Abs(DeltaSeconds) >= 60.0) { Text = Text + gcnew System::String(" (") + this->FormatAxisTime(System::Math::Abs(DeltaSeconds) * 1000.0, false) + gcnew System::String(")"); }

			return Text;

		}

		System::Collections::Generic::List<MeasureRow^>^ CollectMeasureRows() {

			// The lines that are shown with their value at cursor A and at cursor B
			System::Collections::Generic::List<MeasureRow^>^ Rows = gcnew System::Collections::Generic::List<MeasureRow^>();

			bool ASet = !System::Double::IsNaN(this->MeasureTimeA);
			bool BSet = !System::Double::IsNaN(this->MeasureTimeB);

			for (int i = 0; i < this->LogChart->Series->Count; i++) {

				System::Windows::Forms::DataVisualization::Charting::Series^ LineSeries = this->LogChart->Series[i];
				if (LineSeries->Enabled == false) { continue; }

				MeasureRow^ Row = gcnew MeasureRow();
				Row->Name = LineSeries->LegendText;
				Row->LineColor = LineSeries->Color;

				double Value = 0.0;
				if (ASet == true && this->GetLineValueAt(i, this->MeasureTimeA, Value) == true) { Row->HasA = true; Row->ValueA = Value; }
				if (BSet == true && this->GetLineValueAt(i, this->MeasureTimeB, Value) == true) { Row->HasB = true; Row->ValueB = Value; }

				Rows->Add(Row);

			}

			return Rows;

		}

		System::Void BuildMeasureTexts(System::Collections::Generic::List<System::String^>^ Texts, System::Collections::Generic::List<System::Drawing::Color>^ Colors) {

			// The rows of the readout panel. Slope = (value at B - value at A) / (time at B - time at A), in the temperature unit per minute.
			System::String^ Unit = GlobalVariables::DefaultTempUnitString;
			System::Drawing::Color ColorA = System::Drawing::Color::FromArgb(255, 220, 60);
			System::Drawing::Color ColorB = System::Drawing::Color::FromArgb(60, 220, 255);
			System::Drawing::Color ColorText = System::Drawing::Color::White;
			System::Drawing::Color ColorHint = System::Drawing::Color::FromArgb(170, 170, 170);

			bool ASet = !System::Double::IsNaN(this->MeasureTimeA);
			bool BSet = !System::Double::IsNaN(this->MeasureTimeB);

			if (ASet == true) { Texts->Add(gcnew System::String("A:  ") + this->FormatMeasureTime(this->MeasureTimeA)); Colors->Add(ColorA); }
			if (BSet == true) { Texts->Add(gcnew System::String("B:  ") + this->FormatMeasureTime(this->MeasureTimeB)); Colors->Add(ColorB); }

			double DeltaSeconds = 0.0;
			bool BothSet = (ASet == true && BSet == true);

			if (BothSet == true) {

				DeltaSeconds = (this->MeasureTimeB - this->MeasureTimeA) * 86400.0;
				Texts->Add(gcnew System::String(L"\u0394t (B - A):  ") + this->FormatMeasureDeltaTime(DeltaSeconds)); Colors->Add(ColorText);

			}

			System::Collections::Generic::List<MeasureRow^>^ Rows = this->CollectMeasureRows();

			for (int r = 0; r < Rows->Count; r++) {

				MeasureRow^ Row = Rows[r];

				System::String^ ValueText = Row->Name + gcnew System::String(":");
				double RowValueA = Row->ValueA;
				double RowValueB = Row->ValueB;
				if (ASet == true) { ValueText = ValueText + gcnew System::String("  A = ") + ((Row->HasA == true) ? RowValueA.ToString("F2") : gcnew System::String("N/A")); }
				if (BSet == true) { ValueText = ValueText + gcnew System::String("  B = ") + ((Row->HasB == true) ? RowValueB.ToString("F2") : gcnew System::String("N/A")); }
				ValueText = ValueText + gcnew System::String(" ") + Unit;

				Texts->Add(ValueText); Colors->Add(Row->LineColor);

				if (BothSet == true) {

					System::String^ DeltaText = gcnew System::String(L"      \u0394T = N/A");

					if (Row->HasA == true && Row->HasB == true) {

						double DeltaTemperature = Row->ValueB - Row->ValueA;
						DeltaText = gcnew System::String(L"      \u0394T = ") + DeltaTemperature.ToString("+0.00;-0.00;0.00") + gcnew System::String(" ") + Unit;

						if (System::Math::Abs(DeltaSeconds) > 0.0000001) {

							double SlopePerMinute = DeltaTemperature / (DeltaSeconds / 60.0);
							DeltaText = DeltaText + gcnew System::String("    Slope = ") + SlopePerMinute.ToString("+0.000;-0.000;0.000") + gcnew System::String(" ") + Unit + gcnew System::String("/min");

						}
						else { DeltaText = DeltaText + gcnew System::String("    Slope = N/A"); }

					}

					Texts->Add(DeltaText); Colors->Add(Row->LineColor);

				}

			}

			if (Rows->Count == 0) { Texts->Add("No line is shown"); Colors->Add(ColorHint); }

			if (BothSet == false && this->MeasureModeEnabled == true) {

				Texts->Add(ASet == true ? gcnew System::String("Click in the graph to place cursor B") : gcnew System::String("Click in the graph to place cursor A"));
				Colors->Add(ColorHint);

			}

		}

		System::Void DrawMeasurePanelOnExport(System::Drawing::Graphics^ OutputGraphics, float GraphTop) {

			// Draws the readout panel of the two-cursor measurement into the exported image exactly as it is in the Data Log window: the same position in the graph,
			// size, background, border, text rows, fonts and colors (taken from the panel and its rows). GraphTop = the top of the graph in the image. Nothing is drawn
			// while no cursor is placed (the hint text of the mode is not data).
			if (this->MeasurePanel == nullptr || this->MeasurePanel->Visible == false) { return; }
			if (System::Double::IsNaN(this->MeasureTimeA) && System::Double::IsNaN(this->MeasureTimeB)) { return; }

			float PanelX = (float)this->MeasurePanel->Left;
			float PanelY = GraphTop + (float)this->MeasurePanel->Top;
			float PanelW = (float)this->MeasurePanel->Width;
			float PanelH = (float)this->MeasurePanel->Height;

			// Border thickness of the panel (the part of its size that is not client area)
			float BorderX = ((float)this->MeasurePanel->Width - (float)this->MeasurePanel->ClientSize.Width) / 2.0f;
			float BorderY = ((float)this->MeasurePanel->Height - (float)this->MeasurePanel->ClientSize.Height) / 2.0f;

			System::Drawing::SolidBrush^ BackBrush = gcnew System::Drawing::SolidBrush(this->MeasurePanel->BackColor);
			System::Drawing::Pen^ BorderPen = gcnew System::Drawing::Pen(System::Drawing::Color::FromArgb(110, 110, 110), 1.0f);

			OutputGraphics->FillRectangle(BackBrush, PanelX, PanelY, PanelW, PanelH);
			OutputGraphics->DrawRectangle(BorderPen, PanelX, PanelY, PanelW - 1.0f, PanelH - 1.0f);

			// No extra left margin: the text starts where the label of the window starts
			System::Drawing::StringFormat^ TextFormat = gcnew System::Drawing::StringFormat(System::Drawing::StringFormat::GenericTypographic);

			for (int r = 0; r < this->MeasureRowLabels->Count; r++) {

				System::Windows::Forms::Label^ RowLabel = this->MeasureRowLabels[r];
				System::Drawing::SolidBrush^ TextBrush = gcnew System::Drawing::SolidBrush(RowLabel->ForeColor);

				OutputGraphics->DrawString(RowLabel->Text, RowLabel->Font, TextBrush, PanelX + BorderX + (float)RowLabel->Left, PanelY + BorderY + (float)RowLabel->Top, TextFormat);

				delete TextBrush;

			}

			delete TextFormat;
			delete BorderPen;
			delete BackBrush;

		}

		System::Void UpdateMeasurement() {

			// Brings the readout panel up to date (cursor positions, shown lines). It is hidden while no cursor is placed.
			bool ASet = !System::Double::IsNaN(this->MeasureTimeA);
			bool BSet = !System::Double::IsNaN(this->MeasureTimeB);

			if (ASet == false && BSet == false && this->MeasureModeEnabled == false) {

				this->MeasurePanel->Visible = false;
				return;

			}

			System::Collections::Generic::List<System::String^>^ Texts = gcnew System::Collections::Generic::List<System::String^>();
			System::Collections::Generic::List<System::Drawing::Color>^ Colors = gcnew System::Collections::Generic::List<System::Drawing::Color>();
			this->BuildMeasureTexts(Texts, Colors);

			// Mode on but no cursor yet: only the hint
			if (ASet == false && BSet == false) {

				Texts->Clear();
				Colors->Clear();
				Texts->Add("Click in the graph to place cursor A");
				Colors->Add(System::Drawing::Color::FromArgb(170, 170, 170));

			}

			this->MeasurePanel->SuspendLayout();

			// One text row per label; labels are reused
			while (this->MeasureRowLabels->Count < Texts->Count) {

				System::Windows::Forms::Label^ RowLabel = gcnew System::Windows::Forms::Label();
				RowLabel->AutoSize = true;
				RowLabel->Margin = System::Windows::Forms::Padding(0, 1, 0, 1);
				RowLabel->Cursor = System::Windows::Forms::Cursors::SizeAll;
				RowLabel->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &DataLogGUI::MeasurePanel_MouseDown);
				RowLabel->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &DataLogGUI::MeasurePanel_MouseMove);
				RowLabel->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &DataLogGUI::MeasurePanel_MouseUp);
				RowLabel->ContextMenuStrip = this->ChartContextMenu;
				this->MeasurePanel->Controls->Add(RowLabel);
				this->MeasureRowLabels->Add(RowLabel);

			}

			while (this->MeasureRowLabels->Count > Texts->Count) {

				System::Windows::Forms::Label^ ExtraLabel = this->MeasureRowLabels[this->MeasureRowLabels->Count - 1];
				this->MeasureRowLabels->RemoveAt(this->MeasureRowLabels->Count - 1);
				this->MeasurePanel->Controls->Remove(ExtraLabel);

			}

			for (int r = 0; r < Texts->Count; r++) {

				if (this->MeasureRowLabels[r]->Text != Texts[r]) { this->MeasureRowLabels[r]->Text = Texts[r]; }
				this->MeasureRowLabels[r]->ForeColor = Colors[r];

			}

			this->MeasurePanel->ResumeLayout(true);

			if (this->MeasurePanelPositioned == false) {

				// First position: in the top left corner of the plot area
				this->MeasurePanel->Location = System::Drawing::Point(85, 20);
				this->MeasurePanelPositioned = true;

			}

			this->MeasurePanel->Visible = true;
			this->MeasurePanel->BringToFront();
			this->KeepMeasurePanelInside();

		}

		System::Void MeasureMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			// Switches the placing of the two cursors on or off
			this->MeasureModeEnabled = this->MeasureMenuItem->Checked;
			this->ApplyMeasureMode();

		}

		bool TryParseElapsedTime(System::String^ Text, double% Seconds) {

			// An elapsed time typed by the user, in seconds: h:mm:ss, h:mm:ss.fff, m:ss, m:ss.f or only seconds (83.5); a decimal point or comma; an "s" at the end is allowed.
			// The parts before the last one are whole numbers; no sign.
			System::String^ Clean = Text->Trim();

			if (Clean->EndsWith("s", System::StringComparison::OrdinalIgnoreCase)) { Clean = Clean->Substring(0, Clean->Length - 1)->Trim(); }

			Clean = Clean->Replace(L',', L'.');

			if (Clean->Length == 0) { return false; }

			cli::array<System::String^>^ Parts = Clean->Split(L':');

			if (Parts->Length > 3) { return false; }

			double Total = 0.0;

			for (int i = 0; i < Parts->Length; i++) {

				System::String^ Part = Parts[i]->Trim();
				double Number = 0.0;
				System::Globalization::NumberStyles Style = (i == Parts->Length - 1) ? System::Globalization::NumberStyles::AllowDecimalPoint : System::Globalization::NumberStyles::None;

				if (Part->Length == 0) { return false; }
				if (System::Double::TryParse(Part, Style, System::Globalization::CultureInfo::InvariantCulture, Number) == false) { return false; }

				Total = (Total * 60.0) + Number;

			}

			Seconds = Total;
			return true;

		}

		System::Void RemoveMeasureCursor(int Index) {

			// Removes cursor A (0) or B (1) only
			if (Index == 0) { this->MeasureTimeA = System::Double::NaN; this->MeasureLineA->Visible = false; }
			else { this->MeasureTimeB = System::Double::NaN; this->MeasureLineB->Visible = false; }

			this->UpdateMeasurement();

		}

		System::Void SetCursorTimesMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			// A window to type the elapsed time of cursor A and cursor B (the same time as shown in the panel and on the time axis)
			double FirstTime = 0.0;
			double LastTime = 0.0;

			if (this->GetMeasureTimeLimits(FirstTime, LastTime) == false) {

				System::Windows::Forms::MessageBox::Show(this, "There is no data in the graph yet, so no cursor can be placed.", "Set Cursor Times", System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Information);
				return;

			}

			double StartDay = this->ChartStartTime.ToOADate();
			System::String^ RangeText = this->FormatMeasureTime(FirstTime) + gcnew System::String(" to ") + this->FormatMeasureTime(LastTime);

			System::Windows::Forms::Form^ TimesWindow = gcnew System::Windows::Forms::Form();
			TimesWindow->Text = "Set Cursor Times";
			TimesWindow->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			TimesWindow->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			TimesWindow->MinimizeBox = false;
			TimesWindow->MaximizeBox = false;
			TimesWindow->ShowInTaskbar = false;
			TimesWindow->BackColor = System::Drawing::Color::White;
			TimesWindow->ForeColor = System::Drawing::Color::Black;
			TimesWindow->ClientSize = System::Drawing::Size(400, 250);

			System::Windows::Forms::Label^ ALabel = gcnew System::Windows::Forms::Label();
			ALabel->Text = "Cursor A:";
			ALabel->AutoSize = false;
			ALabel->Location = System::Drawing::Point(20, 24);
			ALabel->Size = System::Drawing::Size(100, 24);
			TimesWindow->Controls->Add(ALabel);

			System::Windows::Forms::TextBox^ ABox = gcnew System::Windows::Forms::TextBox();
			ABox->Location = System::Drawing::Point(130, 22);
			ABox->Size = System::Drawing::Size(240, 24);
			if (!System::Double::IsNaN(this->MeasureTimeA)) { ABox->Text = this->FormatMeasureTime(this->MeasureTimeA); }
			TimesWindow->Controls->Add(ABox);

			System::Windows::Forms::Label^ BLabel = gcnew System::Windows::Forms::Label();
			BLabel->Text = "Cursor B:";
			BLabel->AutoSize = false;
			BLabel->Location = System::Drawing::Point(20, 64);
			BLabel->Size = System::Drawing::Size(100, 24);
			TimesWindow->Controls->Add(BLabel);

			System::Windows::Forms::TextBox^ BBox = gcnew System::Windows::Forms::TextBox();
			BBox->Location = System::Drawing::Point(130, 62);
			BBox->Size = System::Drawing::Size(240, 24);
			if (!System::Double::IsNaN(this->MeasureTimeB)) { BBox->Text = this->FormatMeasureTime(this->MeasureTimeB); }
			TimesWindow->Controls->Add(BBox);

			System::Windows::Forms::Label^ NoteLabel = gcnew System::Windows::Forms::Label();
			NoteLabel->Text = gcnew System::String("Elapsed time since the start of the log, as on the time axis: h:mm:ss, h:mm:ss.fff, m:ss or seconds (for example 1:23.5 or 83.5).\r\n\r\nThe logged time range is ") +
				RangeText + gcnew System::String(". An empty box removes that cursor.");
			NoteLabel->AutoSize = false;
			NoteLabel->Location = System::Drawing::Point(20, 104);
			NoteLabel->Size = System::Drawing::Size(360, 90);
			TimesWindow->Controls->Add(NoteLabel);

			System::Windows::Forms::Button^ TimesOkButton = gcnew System::Windows::Forms::Button();
			TimesOkButton->Text = "Set";
			TimesOkButton->DialogResult = System::Windows::Forms::DialogResult::OK;
			TimesOkButton->Location = System::Drawing::Point(90, 204);
			TimesOkButton->Size = System::Drawing::Size(110, 30);
			TimesWindow->Controls->Add(TimesOkButton);
			TimesWindow->AcceptButton = TimesOkButton;

			System::Windows::Forms::Button^ TimesCancelButton = gcnew System::Windows::Forms::Button();
			TimesCancelButton->Text = "Cancel";
			TimesCancelButton->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			TimesCancelButton->Location = System::Drawing::Point(210, 204);
			TimesCancelButton->Size = System::Drawing::Size(110, 30);
			TimesWindow->Controls->Add(TimesCancelButton);
			TimesWindow->CancelButton = TimesCancelButton;

			// The window is shown again with the typed text until both boxes are correct (or Cancel)
			while (TimesWindow->ShowDialog(this) == System::Windows::Forms::DialogResult::OK) {

				// New limits: the log can have grown while the window was open
				if (this->GetMeasureTimeLimits(FirstTime, LastTime) == false) { return; }

				cli::array<System::Windows::Forms::TextBox^>^ Boxes = gcnew cli::array<System::Windows::Forms::TextBox^>(2);
				Boxes[0] = ABox;
				Boxes[1] = BBox;

				cli::array<double>^ NewTimes = gcnew cli::array<double>(2);
				bool InputIsCorrect = true;
				const double ToleranceDays = 0.001 / 86400.0;     // 1 ms: a time shown with milliseconds can be a little beyond the first / last sample

				for (int k = 0; k < 2 && InputIsCorrect == true; k++) {

					System::String^ CursorName = (k == 0) ? gcnew System::String("A") : gcnew System::String("B");
					NewTimes[k] = System::Double::NaN;

					if (Boxes[k]->Text->Trim()->Length == 0) { continue; }

					double TypedSeconds = 0.0;

					if (this->TryParseElapsedTime(Boxes[k]->Text, TypedSeconds) == false) {

						System::Windows::Forms::MessageBox::Show(this, gcnew System::String("Cursor ") + CursorName + gcnew System::String(": \"") + Boxes[k]->Text + gcnew System::String("\" is not a time. Use h:mm:ss, h:mm:ss.fff, m:ss or seconds."),
							"Set Cursor Times", System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Warning);
						InputIsCorrect = false;
						continue;

					}

					double TypedDay = StartDay + (TypedSeconds / 86400.0);

					if (TypedDay < FirstTime - ToleranceDays || TypedDay > LastTime + ToleranceDays) {

						System::Windows::Forms::MessageBox::Show(this, gcnew System::String("Cursor ") + CursorName + gcnew System::String(": the time is outside the logged time range (") + this->FormatMeasureTime(FirstTime) +
							gcnew System::String(" to ") + this->FormatMeasureTime(LastTime) + gcnew System::String(") of the lines that are shown."), "Set Cursor Times",
							System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Warning);
						InputIsCorrect = false;
						continue;

					}

					NewTimes[k] = System::Math::Min(LastTime, System::Math::Max(FirstTime, TypedDay));

				}

				if (InputIsCorrect == false) { continue; }

				for (int k = 0; k < 2; k++) {

					if (System::Double::IsNaN(NewTimes[k])) { this->RemoveMeasureCursor(k); }
					else { this->SetMeasureCursor(k, NewTimes[k]); }

				}

				break;

			}

			delete TimesWindow;

		}

		System::Void ClearCursorsMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			this->ClearMeasureCursors();

		}

		System::Void StyleAxisScrollBar(System::Windows::Forms::DataVisualization::Charting::Axis^ AxisObject) {

			// Colors of the scroll bar that shows under / beside a zoomed axis: the bar track, the buttons (the two arrows, the Reset Zoom button and the
			// sliding part) and the symbols on the buttons. The colors are set at the top of this file (_DataLogScrollBar_...).
#if _DataLogScrollBar_ApplyColors
			System::Windows::Forms::DataVisualization::Charting::AxisScrollBar^ Bar = AxisObject->ScrollBar;

			Bar->ButtonStyle = System::Windows::Forms::DataVisualization::Charting::ScrollBarButtonStyles::All;	// arrows and the Reset Zoom button
			Bar->BackColor = _DataLogScrollBar_TrackColor;
			Bar->ButtonColor = _DataLogScrollBar_ButtonColor;
			Bar->LineColor = _DataLogScrollBar_LineColor;
#endif

		}

		System::Void UpdateZoomIndicator() {

			// The border of the Reset Zoom button shows whether the graph is zoomed in: orange while the time axis and / or the temperature axis is zoomed,
			// the normal button border color (like the other buttons) when the graph shows its whole range
			try {

				System::Windows::Forms::DataVisualization::Charting::ChartArea^ Area = this->LogChart->ChartAreas[0];
				bool Zoomed = (Area->AxisX->ScaleView->IsZoomed == true || Area->AxisY->ScaleView->IsZoomed == true);

				System::Drawing::Color WantedColor = System::Drawing::Color::FromArgb(100, 100, 100);
				if (Zoomed == true) { WantedColor = System::Drawing::Color::FromArgb(255, 140, 0); }

				System::Drawing::Color CurrentColor = this->ResetZoomButton->FlatAppearance->BorderColor;
				if (CurrentColor.ToArgb() != WantedColor.ToArgb()) { this->SetButtonBorderColor(this->ResetZoomButton, WantedColor); }

			}
			catch (System::Exception^) {}

		}

		System::Void UpdateScrollSteps() {

			// An arrow of a scroll bar moves the zoomed view by a tenth of the visible range (at least the smallest step)
			try {

				System::Windows::Forms::DataVisualization::Charting::ChartArea^ Area = this->LogChart->ChartAreas[0];

				if (Area->AxisX->ScaleView->IsZoomed == true) {

					// The time axis is in days - the step is set in milliseconds
					double ViewMilliSec = (Area->AxisX->ScaleView->ViewMaximum - Area->AxisX->ScaleView->ViewMinimum) * 86400000.0;
					if (!System::Double::IsNaN(ViewMilliSec) && ViewMilliSec > 0.0) { Area->AxisX->ScaleView->SmallScrollSize = System::Math::Max(ViewMilliSec / 10.0, 1.0); }

				}

				if (Area->AxisY->ScaleView->IsZoomed == true) {

					double ViewRange = Area->AxisY->ScaleView->ViewMaximum - Area->AxisY->ScaleView->ViewMinimum;
					if (!System::Double::IsNaN(ViewRange) && ViewRange > 0.0) { Area->AxisY->ScaleView->SmallScrollSize = System::Math::Max(ViewRange / 10.0, 0.001); }

				}

			}
			catch (System::Exception^) {}

		}

		System::Void LogChart_AxisViewChanged(System::Object^ sender, System::Windows::Forms::DataVisualization::Charting::ViewEventArgs^ e) {

			// The graph was zoomed, scrolled or the zoom was reset - the scroll steps and the ticks follow the visible range
			this->UpdateScrollSteps();
			this->ApplyAxisSettings();
			this->UpdateZoomIndicator();

		}

		// ------------------------- Right click menu of the graph ------------------------- //

		System::Void StyleMenuItem(System::Windows::Forms::ToolStripMenuItem^ MenuItem, System::String^ MenuText) {

			// Gives an item of the right click menu the same look as the context menus of the other windows
			MenuItem->Text = MenuText;
			MenuItem->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);
			MenuItem->ForeColor = System::Drawing::Color::White;
			MenuItem->Font = gcnew System::Drawing::Font("Arial", 9);

		}

		System::Void AddTickItems(System::Windows::Forms::ToolStripMenuItem^ ParentItem, cli::array<int>^ TickCounts) {

			// Auto first, then the fixed numbers of ticks (the Tag of an item is its number of ticks, 0 is automatic)
			System::Windows::Forms::ToolStripMenuItem^ AutoItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(AutoItem, "Auto");
			AutoItem->Tag = "0";
			AutoItem->Click += gcnew System::EventHandler(this, &DataLogGUI::TickMenuItem_Click);
			ParentItem->DropDownItems->Add(AutoItem);

			for (int i = 0; i < TickCounts->Length; i++) {

				System::Windows::Forms::ToolStripMenuItem^ TickItem = gcnew System::Windows::Forms::ToolStripMenuItem();
				this->StyleMenuItem(TickItem, System::Convert::ToString(TickCounts[i]) + gcnew System::String(" Ticks"));
				TickItem->Tag = System::Convert::ToString(TickCounts[i]);
				TickItem->Click += gcnew System::EventHandler(this, &DataLogGUI::TickMenuItem_Click);
				ParentItem->DropDownItems->Add(TickItem);

			}

		}

		System::Void BuildChartContextMenu() {

			// The same menu structure as the right click menu of the Plots window, with the mouse cursor temperature added at the top
			this->ChartContextMenu = gcnew System::Windows::Forms::ContextMenuStrip();
			this->ChartContextMenu->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);
			this->ChartContextMenu->ForeColor = System::Drawing::Color::White;
			this->ChartContextMenu->ShowImageMargin = false;
			this->ChartContextMenu->ShowCheckMargin = true;

			this->CursorTempMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->CursorTempMenuItem, "Temperature At The Cursor");
			this->CursorTempMenuItem->CheckOnClick = true;
			this->CursorTempMenuItem->Click += gcnew System::EventHandler(this, &DataLogGUI::CursorTempMenuItem_Click);

			this->MeasureMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->MeasureMenuItem, "Measure With Two Cursors");
			this->MeasureMenuItem->CheckOnClick = true;
			this->MeasureMenuItem->Click += gcnew System::EventHandler(this, &DataLogGUI::MeasureMenuItem_Click);

			this->SetCursorTimesMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->SetCursorTimesMenuItem, "Set Cursor Times...");
			this->SetCursorTimesMenuItem->Click += gcnew System::EventHandler(this, &DataLogGUI::SetCursorTimesMenuItem_Click);

			this->ClearCursorsMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->ClearCursorsMenuItem, "Clear Cursors");
			this->ClearCursorsMenuItem->Click += gcnew System::EventHandler(this, &DataLogGUI::ClearCursorsMenuItem_Click);

			this->CreateMarkerMenuItems();

			this->XTicksMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->XTicksMenuItem, "X-Axis Number Of Ticks");
			this->AddTickItems(this->XTicksMenuItem, gcnew cli::array<int>{ 5, 10, 20, 25, 50 });

			this->YTicksMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->YTicksMenuItem, "Y-Axis Number Of Ticks");
			this->AddTickItems(this->YTicksMenuItem, gcnew cli::array<int>{ 5, 10, 15, 20, 25, 30 });

			this->XRangeStatusMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->XRangeStatusMenuItem, "X-Axis Range: Auto");
			this->XRangeStatusMenuItem->Enabled = false;

			this->XSetRangeMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->XSetRangeMenuItem, "Set X-Axis Range And Lock...");
			this->XSetRangeMenuItem->Click += gcnew System::EventHandler(this, &DataLogGUI::XSetRangeMenuItem_Click);

			this->XLockCurrentMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->XLockCurrentMenuItem, "Lock X-Axis At Current Range");
			this->XLockCurrentMenuItem->Click += gcnew System::EventHandler(this, &DataLogGUI::XLockCurrentMenuItem_Click);

			this->XUnlockMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->XUnlockMenuItem, "Unlock X-Axis (Auto Range)");
			this->XUnlockMenuItem->Click += gcnew System::EventHandler(this, &DataLogGUI::XUnlockMenuItem_Click);

			this->YRangeStatusMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->YRangeStatusMenuItem, "Y-Axis Range: Auto");
			this->YRangeStatusMenuItem->Enabled = false;

			this->YSetRangeMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->YSetRangeMenuItem, "Set Y-Axis Range And Lock...");
			this->YSetRangeMenuItem->Click += gcnew System::EventHandler(this, &DataLogGUI::YSetRangeMenuItem_Click);

			this->YLockCurrentMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->YLockCurrentMenuItem, "Lock Y-Axis At Current Range");
			this->YLockCurrentMenuItem->Click += gcnew System::EventHandler(this, &DataLogGUI::YLockCurrentMenuItem_Click);

			this->YUnlockMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->YUnlockMenuItem, "Unlock Y-Axis (Auto Range)");
			this->YUnlockMenuItem->Click += gcnew System::EventHandler(this, &DataLogGUI::YUnlockMenuItem_Click);

			System::Windows::Forms::ToolStripSeparator^ Separator1 = gcnew System::Windows::Forms::ToolStripSeparator();
			System::Windows::Forms::ToolStripSeparator^ Separator2 = gcnew System::Windows::Forms::ToolStripSeparator();

			this->ChartContextMenu->Items->Add(this->CursorTempMenuItem);
			this->ChartContextMenu->Items->Add(this->MeasureMenuItem);
			this->ChartContextMenu->Items->Add(this->SetCursorTimesMenuItem);
			this->ChartContextMenu->Items->Add(this->ClearCursorsMenuItem);
			this->ChartContextMenu->Items->Add(Separator1);
			this->ChartContextMenu->Items->Add(this->AddMarkerMenuItem);
			this->ChartContextMenu->Items->Add(this->DeleteMarkerMenuItem);
			this->ChartContextMenu->Items->Add(this->DeleteAllMarkersMenuItem);
			this->ChartContextMenu->Items->Add(this->MarkerSeparator);
			this->ChartContextMenu->Items->Add(this->XTicksMenuItem);
			this->ChartContextMenu->Items->Add(this->YTicksMenuItem);
			this->ChartContextMenu->Items->Add(Separator2);
			this->ChartContextMenu->Items->Add(this->XRangeStatusMenuItem);
			this->ChartContextMenu->Items->Add(this->XSetRangeMenuItem);
			this->ChartContextMenu->Items->Add(this->XLockCurrentMenuItem);
			this->ChartContextMenu->Items->Add(this->XUnlockMenuItem);
			this->ChartContextMenu->Items->Add(gcnew System::Windows::Forms::ToolStripSeparator());
			this->ChartContextMenu->Items->Add(this->YRangeStatusMenuItem);
			this->ChartContextMenu->Items->Add(this->YSetRangeMenuItem);
			this->ChartContextMenu->Items->Add(this->YLockCurrentMenuItem);
			this->ChartContextMenu->Items->Add(this->YUnlockMenuItem);

			this->ChartContextMenu->Opening += gcnew System::ComponentModel::CancelEventHandler(this, &DataLogGUI::ChartContextMenu_Opening);

			this->LogChart->ContextMenuStrip = this->ChartContextMenu;
			this->CursorLabel->ContextMenuStrip = this->ChartContextMenu;
			this->MeasurePanel->ContextMenuStrip = this->ChartContextMenu;

		}

		System::Void CheckTickItems(System::Windows::Forms::ToolStripMenuItem^ ParentItem, int TickCount) {

			// Marks the item of the number of ticks that is in use
			for (int i = 0; i < ParentItem->DropDownItems->Count; i++) {

				System::Windows::Forms::ToolStripMenuItem^ TickItem = dynamic_cast<System::Windows::Forms::ToolStripMenuItem^>(ParentItem->DropDownItems[i]);
				if (TickItem != nullptr) { TickItem->Checked = (System::Convert::ToInt32(TickItem->Tag) == TickCount); }

			}

		}

		System::Void ChartContextMenu_Opening(System::Object^ sender, System::ComponentModel::CancelEventArgs^ e) {

			// Updates the items before the menu is shown
			this->UpdateMarkerMenuItems();
			this->CursorTempMenuItem->Checked = this->CursorLabelEnabled;
			this->MeasureMenuItem->Checked = this->MeasureModeEnabled;
			this->ClearCursorsMenuItem->Enabled = (!System::Double::IsNaN(this->MeasureTimeA) || !System::Double::IsNaN(this->MeasureTimeB));
			this->CheckTickItems(this->XTicksMenuItem, this->XTickCount);
			this->CheckTickItems(this->YTicksMenuItem, this->YTickCount);

			if (this->YRangeLocked == true) {

				double LockedMinimumValue = System::Math::Round(this->YLockedMinimum, 1);
				double LockedMaximumValue = System::Math::Round(this->YLockedMaximum, 1);
				this->YRangeStatusMenuItem->Text = gcnew System::String("Y-Axis Range: Locked ") + LockedMinimumValue.ToString("F1") +
					gcnew System::String(" To ") + LockedMaximumValue.ToString("F1");

			}
			else { this->YRangeStatusMenuItem->Text = "Y-Axis Range: Auto"; }

			if (this->XRangeLocked == true) {

				this->XRangeStatusMenuItem->Text = gcnew System::String("X-Axis Range: Locked ") + this->FormatRangeTime(this->XLockedMinimum) +
					gcnew System::String(" To ") + this->FormatRangeTime(this->XLockedMaximum);

			}
			else { this->XRangeStatusMenuItem->Text = "X-Axis Range: Auto"; }

			this->XLockCurrentMenuItem->Enabled = !this->XRangeLocked;
			this->XUnlockMenuItem->Enabled = this->XRangeLocked;

			this->YLockCurrentMenuItem->Enabled = !this->YRangeLocked;
			this->YUnlockMenuItem->Enabled = this->YRangeLocked;

		}

		System::Void ShowTitleCheckBox_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {

			this->UpdateGraphTitleVisibility();

		}

		System::Void UpdateGraphTitleVisibility() {

			// The title is shown when the Show Graph Title check box is ticked. A title without text gets the text "Title" (the text shown until a title is typed).
			System::Windows::Forms::DataVisualization::Charting::Title^ GraphTitle = this->LogChart->Titles->FindByName("GraphTitle");
			if (GraphTitle == nullptr) { return; }

			if (GraphTitle->Text->Length == 0) { GraphTitle->Text = "Title"; }
			GraphTitle->Visible = this->ShowTitleCheckBox->Checked;

			// The title takes room above the plot area
			this->UpdatePlotAreaLayout();

		}

		System::Void ApplyGraphTitleText(System::String^ NewTitle) {

			// A new title (typed in the graph or in the Graph Title window) is shown: it ticks the Show Graph Title check box. An empty text hides the title
			// (the check box is unticked and the text goes back to "Title"). A title that was not changed leaves the check box as it is.
			System::Windows::Forms::DataVisualization::Charting::Title^ GraphTitle = this->LogChart->Titles->FindByName("GraphTitle");
			if (GraphTitle == nullptr) { return; }

			NewTitle = NewTitle->Trim();

			if (NewTitle->Length == 0) {

				GraphTitle->Text = "Title";
				this->ShowTitleCheckBox->Checked = false;

			}
			else {

				bool TitleChanged = !NewTitle->Equals(GraphTitle->Text);
				GraphTitle->Text = NewTitle;
				if (TitleChanged == true) { this->ShowTitleCheckBox->Checked = true; }

			}

			this->UpdateGraphTitleVisibility();

		}

		System::Void LogChart_MouseDoubleClick(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {

			// A double-click on the title of the graph edits the title directly on the graph
			System::Windows::Forms::DataVisualization::Charting::HitTestResult^ HitResult = this->LogChart->HitTest(e->X, e->Y);

			if (HitResult != nullptr && HitResult->ChartElementType == System::Windows::Forms::DataVisualization::Charting::ChartElementType::Title) { this->BeginTitleEdit(); }

		}

		System::Void BeginTitleEdit() {

			System::Windows::Forms::DataVisualization::Charting::Title^ GraphTitle = this->LogChart->Titles->FindByName("GraphTitle");
			if (GraphTitle == nullptr || GraphTitle->Visible == false) { return; }

			// The text box covers the title: at the top center of the graph, 60 % of the width of the graph (at least 240 pixels)
			int BoxWidth = System::Math::Max(240, (this->LogChart->Width * 6) / 10);
			BoxWidth = System::Math::Min(BoxWidth, this->LogChart->Width - 20);

			this->TitleEditBox->Text = GraphTitle->Text;
			this->TitleEditBox->Width = BoxWidth;
			this->TitleEditBox->Location = System::Drawing::Point((this->LogChart->Width - BoxWidth) / 2, 4);
			this->TitleEditBox->Visible = true;
			this->TitleEditBox->BringToFront();
			this->TitleEditBox->Focus();
			this->TitleEditBox->SelectAll();

		}

		System::Void EndTitleEdit(bool KeepText) {

			// Enter or leaving the text box keeps the typed title, Escape throws it away
			if (this->TitleEditBox->Visible == false) { return; }

			System::String^ TypedTitle = this->TitleEditBox->Text;
			this->TitleEditBox->Visible = false;

			if (KeepText == true) { this->ApplyGraphTitleText(TypedTitle); }

		}

		System::Void TitleEditBox_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e) {

			if (e->KeyCode == System::Windows::Forms::Keys::Enter) { e->SuppressKeyPress = true; this->EndTitleEdit(true); }
			else if (e->KeyCode == System::Windows::Forms::Keys::Escape) { e->SuppressKeyPress = true; this->EndTitleEdit(false); }

		}

		System::Void TitleEditBox_Leave(System::Object^ sender, System::EventArgs^ e) {

			this->EndTitleEdit(true);

		}

		bool ShowGraphTitleWindow() {

			// Shows a window for entering the title of the graph (it starts with the title that is set now). It opens when the graph is exported, also when the
			// title is hidden. A new title is shown, an empty text hides the title (see ApplyGraphTitleText). Returns true when the window was closed with
			// Set Title, false when it was cancelled.
			System::Windows::Forms::DataVisualization::Charting::Title^ GraphTitle = this->LogChart->Titles->FindByName("GraphTitle");
			if (GraphTitle == nullptr) { return false; }

			System::Windows::Forms::Form^ TitleWindow = gcnew System::Windows::Forms::Form();
			TitleWindow->Text = "Graph Title";
			TitleWindow->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			TitleWindow->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			TitleWindow->MinimizeBox = false;
			TitleWindow->MaximizeBox = false;
			TitleWindow->ShowInTaskbar = false;
			TitleWindow->BackColor = System::Drawing::Color::White;
			TitleWindow->ForeColor = System::Drawing::Color::Black;
			TitleWindow->ClientSize = System::Drawing::Size(460, 150);

			System::Windows::Forms::Label^ TitleLabel = gcnew System::Windows::Forms::Label();
			TitleLabel->Text = "Title (shown at the top center of the graph and in an exported image):";
			TitleLabel->AutoSize = false;
			TitleLabel->Location = System::Drawing::Point(20, 16);
			TitleLabel->Size = System::Drawing::Size(420, 22);
			TitleWindow->Controls->Add(TitleLabel);

			System::Windows::Forms::TextBox^ TitleBox = gcnew System::Windows::Forms::TextBox();
			TitleBox->MaxLength = 200;
			TitleBox->Text = GraphTitle->Text;
			TitleBox->Location = System::Drawing::Point(20, 42);
			TitleBox->Size = System::Drawing::Size(420, 24);
			TitleWindow->Controls->Add(TitleBox);

			System::Windows::Forms::Label^ NoteLabel = gcnew System::Windows::Forms::Label();
			if (this->ShowTitleCheckBox->Checked == true) { NoteLabel->Text = "Leave the text empty to hide the title."; }
			else { NoteLabel->Text = "The title is hidden now (Show Graph Title). A new title shows it."; }
			NoteLabel->AutoSize = false;
			NoteLabel->Location = System::Drawing::Point(20, 74);
			NoteLabel->Size = System::Drawing::Size(420, 22);
			TitleWindow->Controls->Add(NoteLabel);

			System::Windows::Forms::Button^ TitleOkButton = gcnew System::Windows::Forms::Button();
			TitleOkButton->Text = "Set Title";
			TitleOkButton->DialogResult = System::Windows::Forms::DialogResult::OK;
			TitleOkButton->Location = System::Drawing::Point(115, 106);
			TitleOkButton->Size = System::Drawing::Size(110, 30);
			TitleWindow->Controls->Add(TitleOkButton);
			TitleWindow->AcceptButton = TitleOkButton;

			System::Windows::Forms::Button^ TitleCancelButton = gcnew System::Windows::Forms::Button();
			TitleCancelButton->Text = "Cancel";
			TitleCancelButton->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			TitleCancelButton->Location = System::Drawing::Point(235, 106);
			TitleCancelButton->Size = System::Drawing::Size(110, 30);
			TitleWindow->Controls->Add(TitleCancelButton);
			TitleWindow->CancelButton = TitleCancelButton;

			if (TitleWindow->ShowDialog(this) == System::Windows::Forms::DialogResult::OK) {

				this->ApplyGraphTitleText(TitleBox->Text);
				return true;

			}

			return false;

		}

		System::Void AddNotesButton_Click(System::Object^ sender, System::EventArgs^ e) {

			this->ShowGraphNotesWindow();

		}

		bool ShowGraphNotesWindow() {

			// Shows a window for entering notes about the logging (it starts with the notes that are set now). The notes are written in the table below the graph
			// in an exported image. An empty text removes the notes. Returns true when the window was closed with Set Notes, false when it was cancelled.
			System::Windows::Forms::Form^ NotesWindow = gcnew System::Windows::Forms::Form();
			NotesWindow->Text = "Graph Notes";
			NotesWindow->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			NotesWindow->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			NotesWindow->MinimizeBox = false;
			NotesWindow->MaximizeBox = false;
			NotesWindow->ShowInTaskbar = false;
			NotesWindow->BackColor = System::Drawing::Color::White;
			NotesWindow->ForeColor = System::Drawing::Color::Black;
			NotesWindow->ClientSize = System::Drawing::Size(520, 290);

			System::Windows::Forms::Label^ NotesLabel = gcnew System::Windows::Forms::Label();
			NotesLabel->Text = "Notes about the logging (shown in the table below the graph in an exported image):";
			NotesLabel->AutoSize = false;
			NotesLabel->Location = System::Drawing::Point(20, 16);
			NotesLabel->Size = System::Drawing::Size(480, 22);
			NotesWindow->Controls->Add(NotesLabel);

			System::Windows::Forms::TextBox^ NotesBox = gcnew System::Windows::Forms::TextBox();
			NotesBox->Multiline = true;
			NotesBox->AcceptsReturn = true;
			NotesBox->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			NotesBox->MaxLength = 2000;
			NotesBox->Text = this->GraphNotes;
			NotesBox->Location = System::Drawing::Point(20, 42);
			NotesBox->Size = System::Drawing::Size(480, 160);
			NotesWindow->Controls->Add(NotesBox);

			System::Windows::Forms::Label^ NotesInfoLabel = gcnew System::Windows::Forms::Label();
			NotesInfoLabel->Text = "Leave the text empty to remove the notes. Enter starts a new line.";
			NotesInfoLabel->AutoSize = false;
			NotesInfoLabel->Location = System::Drawing::Point(20, 208);
			NotesInfoLabel->Size = System::Drawing::Size(480, 22);
			NotesWindow->Controls->Add(NotesInfoLabel);

			System::Windows::Forms::Button^ NotesOkButton = gcnew System::Windows::Forms::Button();
			NotesOkButton->Text = "Set Notes";
			NotesOkButton->DialogResult = System::Windows::Forms::DialogResult::OK;
			NotesOkButton->Location = System::Drawing::Point(140, 242);
			NotesOkButton->Size = System::Drawing::Size(110, 30);
			NotesWindow->Controls->Add(NotesOkButton);

			System::Windows::Forms::Button^ NotesCancelButton = gcnew System::Windows::Forms::Button();
			NotesCancelButton->Text = "Cancel";
			NotesCancelButton->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			NotesCancelButton->Location = System::Drawing::Point(270, 242);
			NotesCancelButton->Size = System::Drawing::Size(110, 30);
			NotesWindow->Controls->Add(NotesCancelButton);
			NotesWindow->CancelButton = NotesCancelButton;

			if (NotesWindow->ShowDialog(this) == System::Windows::Forms::DialogResult::OK) {

				this->GraphNotes = NotesBox->Text->Trim();
				return true;

			}

			return false;

		}

		System::Void CursorTempMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			// Switches the temperature (and time) at the mouse cursor on or off
			this->CursorLabelEnabled = this->CursorTempMenuItem->Checked;
			if (this->CursorLabelEnabled == false) { this->CursorLabel->Visible = false; }

		}

		System::Void TickMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			System::Windows::Forms::ToolStripMenuItem^ TickItem = (System::Windows::Forms::ToolStripMenuItem^)sender;
			int NewTickCount = System::Convert::ToInt32(TickItem->Tag);

			if (TickItem->OwnerItem == this->XTicksMenuItem) { this->XTickCount = NewTickCount; }
			else { this->YTickCount = NewTickCount; }

			this->ApplyAxisSettings();

		}

		System::String^ FormatRangeTime(double Seconds) {

			// Elapsed time as h:mm:ss, with one decimal (h:mm:ss.f) when it is not a whole number of seconds
			if (Seconds < 0.0) { Seconds = 0.0; }
			long long Tenths = (long long)System::Math::Round(Seconds * 10.0);
			long long WholeSeconds = Tenths / 10;
			int Fraction = (int)(Tenths % 10);
			int Minutes = (int)((WholeSeconds / 60) % 60);
			int SecondsPart = (int)(WholeSeconds % 60);

			System::String^ TimeText = System::Convert::ToString(WholeSeconds / 3600) + gcnew System::String(":") + Minutes.ToString("00") +
				gcnew System::String(":") + SecondsPart.ToString("00");
			if (Fraction > 0) { TimeText = TimeText + gcnew System::String(".") + System::Convert::ToString(Fraction); }

			return TimeText;

		}

		bool ParseRangeTime(System::String^ TimeText, double% Seconds) {

			// Reads h:mm:ss, m:ss or seconds (the last part may have decimals, with a point or a comma)
			if (TimeText == nullptr) { return false; }
			cli::array<System::String^>^ Parts = TimeText->Trim()->Split(gcnew cli::array<wchar_t>{ L':' });
			if (Parts->Length < 1 || Parts->Length > 3) { return false; }

			double Total = 0.0;

			for (int i = 0; i < Parts->Length; i++) {

				System::String^ Part = Parts[i]->Trim()->Replace(L',', L'.');
				double PartValue = 0.0;

				if (Part->Length == 0) { return false; }
				if (!System::Double::TryParse(Part, System::Globalization::NumberStyles::Float, System::Globalization::CultureInfo::InvariantCulture, PartValue)) { return false; }
				if (PartValue < 0.0) { return false; }
				if (i < Parts->Length - 1 && PartValue != System::Math::Floor(PartValue)) { return false; }

				Total = Total * 60.0 + PartValue;

			}

			Seconds = Total;
			return true;

		}

		bool GetXViewRange(double% ViewMinimumSec, double% ViewMaximumSec) {

			// The time range that is shown now, in seconds of elapsed time (the zoomed range, or the range of the whole axis)
			try {

				this->LogChart->ChartAreas[0]->RecalculateAxesScale();
				System::Windows::Forms::DataVisualization::Charting::Axis^ AxisX = this->LogChart->ChartAreas[0]->AxisX;

				double ViewMinimum = AxisX->Minimum;
				double ViewMaximum = AxisX->Maximum;
				if (AxisX->ScaleView->IsZoomed == true) { ViewMinimum = AxisX->ScaleView->ViewMinimum; ViewMaximum = AxisX->ScaleView->ViewMaximum; }

				if (System::Double::IsNaN(ViewMinimum) || System::Double::IsNaN(ViewMaximum) || ViewMaximum <= ViewMinimum) { return false; }

				double StartDay = this->ChartStartTime.ToOADate();
				ViewMinimumSec = (ViewMinimum - StartDay) * 86400.0;
				ViewMaximumSec = (ViewMaximum - StartDay) * 86400.0;
				if (ViewMinimumSec < 0.0) { ViewMinimumSec = 0.0; }

				return (ViewMaximumSec > ViewMinimumSec);

			}
			catch (System::Exception^) { return false; }

		}

		System::Void XLockCurrentMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			// Locks the time axis at the range that is shown now (rounded outwards to 0.1 second)
			double CurrentMinimum = 0.0;
			double CurrentMaximum = 0.0;

			if (this->GetXViewRange(CurrentMinimum, CurrentMaximum)) {

				this->XLockedMinimum = System::Math::Floor(CurrentMinimum * 10.0) / 10.0;
				this->XLockedMaximum = System::Math::Ceiling(CurrentMaximum * 10.0) / 10.0;
				this->XRangeLocked = true;

				// The locked range is the range that is shown, so the zoom of this axis is not needed any more
				this->LogChart->ChartAreas[0]->AxisX->ScaleView->ZoomReset(0);
				this->ApplyAxisSettings();

			}

		}

		System::Void XUnlockMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			// The time axis follows the data again
			this->XRangeLocked = false;
			this->ApplyAxisSettings();

		}

		System::Void XSetRangeMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			// Shows a window for entering a time axis start and end (h:mm:ss), and locks the axis to that range
			double StartMinimum = 0.0;
			double StartMaximum = 60.0;

			if (this->XRangeLocked == true) {

				StartMinimum = this->XLockedMinimum;
				StartMaximum = this->XLockedMaximum;

			}
			else {

				double CurrentMinimum = 0.0;
				double CurrentMaximum = 0.0;

				if (this->GetXViewRange(CurrentMinimum, CurrentMaximum)) {

					StartMinimum = System::Math::Floor(CurrentMinimum * 10.0) / 10.0;
					StartMaximum = System::Math::Ceiling(CurrentMaximum * 10.0) / 10.0;

				}

			}

			System::String^ MinimumText = this->FormatRangeTime(StartMinimum);
			System::String^ MaximumText = this->FormatRangeTime(StartMaximum);

			// The window is shown again (with the entered text) while the text is not a valid time
			while (true) {

				System::Windows::Forms::Form^ RangeWindow = gcnew System::Windows::Forms::Form();
				RangeWindow->Text = "X-Axis Range";
				RangeWindow->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
				RangeWindow->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
				RangeWindow->MinimizeBox = false;
				RangeWindow->MaximizeBox = false;
				RangeWindow->ShowInTaskbar = false;
				RangeWindow->BackColor = System::Drawing::Color::White;
				RangeWindow->ForeColor = System::Drawing::Color::Black;
				RangeWindow->ClientSize = System::Drawing::Size(380, 210);

				System::Windows::Forms::Label^ MinimumLabel = gcnew System::Windows::Forms::Label();
				MinimumLabel->Text = "Start (h:mm:ss):";
				MinimumLabel->AutoSize = false;
				MinimumLabel->Location = System::Drawing::Point(20, 24);
				MinimumLabel->Size = System::Drawing::Size(120, 24);
				RangeWindow->Controls->Add(MinimumLabel);

				System::Windows::Forms::TextBox^ MinimumBox = gcnew System::Windows::Forms::TextBox();
				MinimumBox->Text = MinimumText;
				MinimumBox->Location = System::Drawing::Point(150, 22);
				MinimumBox->Size = System::Drawing::Size(190, 24);
				RangeWindow->Controls->Add(MinimumBox);

				System::Windows::Forms::Label^ MaximumLabel = gcnew System::Windows::Forms::Label();
				MaximumLabel->Text = "End (h:mm:ss):";
				MaximumLabel->AutoSize = false;
				MaximumLabel->Location = System::Drawing::Point(20, 64);
				MaximumLabel->Size = System::Drawing::Size(120, 24);
				RangeWindow->Controls->Add(MaximumLabel);

				System::Windows::Forms::TextBox^ MaximumBox = gcnew System::Windows::Forms::TextBox();
				MaximumBox->Text = MaximumText;
				MaximumBox->Location = System::Drawing::Point(150, 62);
				MaximumBox->Size = System::Drawing::Size(190, 24);
				RangeWindow->Controls->Add(MaximumBox);

				System::Windows::Forms::Label^ NoteLabel = gcnew System::Windows::Forms::Label();
				NoteLabel->Text = "Elapsed time of the log session, for example 0:05:30 or 1:00:00.5 (also m:ss or seconds). The range stays locked until you choose Unlock X-Axis in the graph menu.";
				NoteLabel->AutoSize = false;
				NoteLabel->Location = System::Drawing::Point(20, 100);
				NoteLabel->Size = System::Drawing::Size(340, 60);
				RangeWindow->Controls->Add(NoteLabel);

				System::Windows::Forms::Button^ RangeOkButton = gcnew System::Windows::Forms::Button();
				RangeOkButton->Text = "Lock Range";
				RangeOkButton->DialogResult = System::Windows::Forms::DialogResult::OK;
				RangeOkButton->Location = System::Drawing::Point(80, 164);
				RangeOkButton->Size = System::Drawing::Size(110, 30);
				RangeWindow->Controls->Add(RangeOkButton);
				RangeWindow->AcceptButton = RangeOkButton;

				System::Windows::Forms::Button^ RangeCancelButton = gcnew System::Windows::Forms::Button();
				RangeCancelButton->Text = "Cancel";
				RangeCancelButton->DialogResult = System::Windows::Forms::DialogResult::Cancel;
				RangeCancelButton->Location = System::Drawing::Point(200, 164);
				RangeCancelButton->Size = System::Drawing::Size(110, 30);
				RangeWindow->Controls->Add(RangeCancelButton);
				RangeWindow->CancelButton = RangeCancelButton;

				if (RangeWindow->ShowDialog(this) != System::Windows::Forms::DialogResult::OK) { return; }

				MinimumText = MinimumBox->Text;
				MaximumText = MaximumBox->Text;

				double NewMinimum = 0.0;
				double NewMaximum = 0.0;

				if (this->ParseRangeTime(MinimumText, NewMinimum) && this->ParseRangeTime(MaximumText, NewMaximum)) {

					// An end that is not later than the start is corrected
					if (NewMaximum <= NewMinimum + 0.1) { NewMaximum = NewMinimum + 1.0; }

					this->XLockedMinimum = NewMinimum;
					this->XLockedMaximum = NewMaximum;
					this->XRangeLocked = true;

					// A locked range is a fresh view - the zoom is reset so the whole locked range is visible
					this->LogChart->ChartAreas[0]->AxisX->ScaleView->ZoomReset(0);
					this->ApplyAxisSettings();
					return;

				}

				System::Windows::Forms::MessageBox::Show(this, "The start or the end is not a valid time. Use h:mm:ss, for example 0:05:30.", "X-Axis Range",
					System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Warning);

			}

		}

		bool GetYViewRange(double% ViewMinimum, double% ViewMaximum) {

			// The temperature range that is shown now (the zoomed range, or the range of the whole axis). False when there is no valid range yet.
			try {

				this->LogChart->ChartAreas[0]->RecalculateAxesScale();
				System::Windows::Forms::DataVisualization::Charting::Axis^ AxisY = this->LogChart->ChartAreas[0]->AxisY;

				if (AxisY->ScaleView->IsZoomed == true) { ViewMinimum = AxisY->ScaleView->ViewMinimum; ViewMaximum = AxisY->ScaleView->ViewMaximum; }
				else { ViewMinimum = AxisY->Minimum; ViewMaximum = AxisY->Maximum; }

				return (!System::Double::IsNaN(ViewMinimum) && !System::Double::IsNaN(ViewMaximum) && ViewMaximum > ViewMinimum);

			}
			catch (System::Exception^) { return false; }

		}

		System::Void YLockCurrentMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			// Locks the temperature axis at the range that is shown now (rounded outwards to one decimal)
			double CurrentMinimum = 0.0;
			double CurrentMaximum = 0.0;

			if (this->GetYViewRange(CurrentMinimum, CurrentMaximum)) {

				this->YLockedMinimum = System::Math::Floor(CurrentMinimum * 10.0) / 10.0;
				this->YLockedMaximum = System::Math::Ceiling(CurrentMaximum * 10.0) / 10.0;
				this->YRangeLocked = true;

				// The locked range is the range that is shown, so the zoom of this axis is not needed any more
				this->LogChart->ChartAreas[0]->AxisY->ScaleView->ZoomReset(0);
				this->ApplyAxisSettings();

			}

		}

		System::Void LineCheck_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {

			// Shows or hides the graph line of the check box (the data of a hidden line keeps being collected)
			System::Windows::Forms::CheckBox^ LineCheck = (System::Windows::Forms::CheckBox^)sender;
			int SeriesIndex = System::Convert::ToInt32(LineCheck->Tag);

			if (SeriesIndex >= 0 && SeriesIndex < this->LogChart->Series->Count) { this->LogChart->Series[SeriesIndex]->Enabled = LineCheck->Checked; }

			// The measurement shows the lines that are shown
			this->UpdateMeasurement();

			// The ticks and the automatic range follow the lines that are shown
			this->ApplyAxisSettings();

		}

		System::Void YUnlockMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			// The temperature axis follows the data again
			this->YRangeLocked = false;
			this->ApplyAxisSettings();

		}

		System::Void YSetRangeMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			// Shows a window for entering a temperature axis minimum and maximum, and locks the axis to that range
			double StartMinimum = 0.0;
			double StartMaximum = 100.0;

			if (this->YRangeLocked == true) {

				StartMinimum = this->YLockedMinimum;
				StartMaximum = this->YLockedMaximum;

			}
			else {

				double CurrentMinimum = 0.0;
				double CurrentMaximum = 0.0;

				if (this->GetYViewRange(CurrentMinimum, CurrentMaximum)) {

					StartMinimum = System::Math::Floor(CurrentMinimum * 10.0) / 10.0;
					StartMaximum = System::Math::Ceiling(CurrentMaximum * 10.0) / 10.0;

				}

			}

			System::Windows::Forms::Form^ RangeWindow = gcnew System::Windows::Forms::Form();
			RangeWindow->Text = "Y-Axis Range";
			RangeWindow->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			RangeWindow->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			RangeWindow->MinimizeBox = false;
			RangeWindow->MaximizeBox = false;
			RangeWindow->ShowInTaskbar = false;
			RangeWindow->BackColor = System::Drawing::Color::White;
			RangeWindow->ForeColor = System::Drawing::Color::Black;
			RangeWindow->ClientSize = System::Drawing::Size(380, 200);

			System::Windows::Forms::Label^ MinimumLabel = gcnew System::Windows::Forms::Label();
			MinimumLabel->Text = "Minimum:";
			MinimumLabel->AutoSize = false;
			MinimumLabel->Location = System::Drawing::Point(20, 24);
			MinimumLabel->Size = System::Drawing::Size(110, 24);
			RangeWindow->Controls->Add(MinimumLabel);

			System::Windows::Forms::NumericUpDown^ MinimumBox = gcnew System::Windows::Forms::NumericUpDown();
			MinimumBox->DecimalPlaces = 1;
			MinimumBox->Minimum = System::Decimal(-1000);
			MinimumBox->Maximum = System::Decimal(10000);
			MinimumBox->Increment = System::Decimal(1);
			MinimumBox->Value = System::Decimal(System::Math::Max(-1000.0, System::Math::Min(10000.0, StartMinimum)));
			MinimumBox->Location = System::Drawing::Point(140, 22);
			MinimumBox->Size = System::Drawing::Size(200, 24);
			RangeWindow->Controls->Add(MinimumBox);

			System::Windows::Forms::Label^ MaximumLabel = gcnew System::Windows::Forms::Label();
			MaximumLabel->Text = "Maximum:";
			MaximumLabel->AutoSize = false;
			MaximumLabel->Location = System::Drawing::Point(20, 64);
			MaximumLabel->Size = System::Drawing::Size(110, 24);
			RangeWindow->Controls->Add(MaximumLabel);

			System::Windows::Forms::NumericUpDown^ MaximumBox = gcnew System::Windows::Forms::NumericUpDown();
			MaximumBox->DecimalPlaces = 1;
			MaximumBox->Minimum = System::Decimal(-1000);
			MaximumBox->Maximum = System::Decimal(10000);
			MaximumBox->Increment = System::Decimal(1);
			MaximumBox->Value = System::Decimal(System::Math::Max(-1000.0, System::Math::Min(10000.0, StartMaximum)));
			MaximumBox->Location = System::Drawing::Point(140, 62);
			MaximumBox->Size = System::Drawing::Size(200, 24);
			RangeWindow->Controls->Add(MaximumBox);

			System::Windows::Forms::Label^ NoteLabel = gcnew System::Windows::Forms::Label();
			NoteLabel->Text = "The values are in the temperature unit shown on the graph. The range stays locked until you choose Unlock Y-Axis in the graph menu.";
			NoteLabel->AutoSize = false;
			NoteLabel->Location = System::Drawing::Point(20, 100);
			NoteLabel->Size = System::Drawing::Size(340, 50);
			RangeWindow->Controls->Add(NoteLabel);

			System::Windows::Forms::Button^ RangeOkButton = gcnew System::Windows::Forms::Button();
			RangeOkButton->Text = "Lock Range";
			RangeOkButton->DialogResult = System::Windows::Forms::DialogResult::OK;
			RangeOkButton->Location = System::Drawing::Point(80, 154);
			RangeOkButton->Size = System::Drawing::Size(110, 30);
			RangeWindow->Controls->Add(RangeOkButton);
			RangeWindow->AcceptButton = RangeOkButton;

			System::Windows::Forms::Button^ RangeCancelButton = gcnew System::Windows::Forms::Button();
			RangeCancelButton->Text = "Cancel";
			RangeCancelButton->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			RangeCancelButton->Location = System::Drawing::Point(200, 154);
			RangeCancelButton->Size = System::Drawing::Size(110, 30);
			RangeWindow->Controls->Add(RangeCancelButton);
			RangeWindow->CancelButton = RangeCancelButton;

			if (RangeWindow->ShowDialog(this) == System::Windows::Forms::DialogResult::OK) {

				double NewMinimum = System::Decimal::ToDouble(MinimumBox->Value);
				double NewMaximum = System::Decimal::ToDouble(MaximumBox->Value);

				// A maximum that is not higher than the minimum is corrected
				if (NewMaximum <= NewMinimum + 0.05) { NewMaximum = NewMinimum + 1.0; }

				this->YLockedMinimum = NewMinimum;
				this->YLockedMaximum = NewMaximum;
				this->YRangeLocked = true;

				// A locked range is a fresh view - the zoom is reset so the whole locked range is visible
				this->LogChart->ChartAreas[0]->AxisY->ScaleView->ZoomReset(0);
				this->ApplyAxisSettings();

			}

		}

		bool GetDataRange(double% DataMinimum, double% DataMaximum) {

			// The lowest and highest temperature of all lines of the graph (false when there is no data)
			bool Found = false;

			for (int i = 0; i < this->LogChart->Series->Count; i++) {

				System::Windows::Forms::DataVisualization::Charting::Series^ LineSeries = this->LogChart->Series[i];
				if (LineSeries->Enabled == false || LineSeries->Points->Count == 0) { continue; }

				double SeriesMinimum = LineSeries->Points->FindMinByValue("Y1")->YValues[0];
				double SeriesMaximum = LineSeries->Points->FindMaxByValue("Y1")->YValues[0];

				if (Found == false || SeriesMinimum < DataMinimum) { DataMinimum = SeriesMinimum; }
				if (Found == false || SeriesMaximum > DataMaximum) { DataMaximum = SeriesMaximum; }
				Found = true;

			}

			return Found;

		}

		double GetRoundTickSeconds(double MinimumSeconds) {

			// The smallest round time step (in seconds) that is at least MinimumSeconds: 1, 2, 3, 5, 10, 15, 20, 30 seconds, the same in minutes, 1, 2, 3, 6, 12 hours,
			// whole days - and, for a zoom into less than a few seconds, steps down to a millisecond. So with a number of ticks N the axis has at most N intervals.
			cli::array<double>^ Steps = gcnew cli::array<double>{ 0.001, 0.002, 0.005, 0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1.0, 2.0, 3.0, 5.0, 10.0, 15.0, 20.0, 30.0,
				60.0, 120.0, 180.0, 300.0, 600.0, 900.0, 1200.0, 1800.0, 3600.0, 7200.0, 10800.0, 21600.0, 43200.0, 86400.0 };

			for (int i = 0; i < Steps->Length; i++) {

				if (Steps[i] >= MinimumSeconds) { return Steps[i]; }

			}

			return System::Math::Ceiling(MinimumSeconds / 86400.0) * 86400.0;

		}

		bool GetDataEndTime(double% DataEnd) {

			// The time (in days, the unit of the time axis) of the newest point of the graph. Returns false when the graph has no points.
			bool Found = false;

			for (int i = 0; i < this->LogChart->Series->Count; i++) {

				System::Windows::Forms::DataVisualization::Charting::Series^ LineSeries = this->LogChart->Series[i];
				if (LineSeries->Points->Count == 0) { continue; }

				double LastTime = LineSeries->Points[LineSeries->Points->Count - 1]->XValue;
				if (Found == false || LastTime > DataEnd) { DataEnd = LastTime; }
				Found = true;

			}

			return Found;

		}

		System::Void LogChart_SizeChanged(System::Object^ sender, System::EventArgs^ e) {

			this->UpdatePlotAreaLayout();

		}

		System::Void UpdatePlotAreaLayout() {

			// Gives the plot area (the part inside the axes) a fixed size, in pixels from each side of the graph, instead of the automatic layout of the chart.
			// With the automatic layout the chart makes the plot area smaller or larger whenever the width of a label changes (a time label on the right end of the
			// time axis that comes and goes while the log grows, a temperature label that gets another digit): the graph then jumps horizontally at every such change.
			// While the graph is zoomed the same fixed layout is used, with larger margins: the zoom scroll bars (14 pixels thick, against the plot area) and the zoom buttons
			// need room, and a temperature label has more digits when the graph is zoomed in. With the automatic layout the plot area changed size at every such change.
			// See _DataLogPlotArea_ReserveZoomRoom (at the top of this file). Tune the margins below if a label or an axis title is cut off.
			const double LeftMarginPixels = 75.0;		// Temperature axis title and labels
			const double RightMarginPixels = 40.0;		// Room for half of the last time label, and for the zoom buttons right of the time axis scroll bar
			const double TopMarginPixels = 12.0;
			const double BottomMarginPixels = 55.0;		// Time axis labels and title
			const double ZoomLeftMarginPixels = 110.0;	// With the temperature axis scroll bar and labels with more digits
			const double ZoomTopMarginPixels = 28.0;	// Room for the zoom buttons of the temperature axis, above its scroll bar
			const double ZoomBottomMarginPixels = 72.0;	// With the time axis scroll bar
			const double TitleHeightPixels = 30.0;		// Height that the graph title takes when it is shown

			try {

				System::Windows::Forms::DataVisualization::Charting::ChartArea^ Area = this->LogChart->ChartAreas[0];
				bool Zoomed = (Area->AxisX->ScaleView->IsZoomed == true || Area->AxisY->ScaleView->IsZoomed == true);

				double ChartWidth = (double)this->LogChart->ClientSize.Width;
				double ChartHeight = (double)this->LogChart->ClientSize.Height;

				if (this->LogChart->Titles->Count > 0 && this->LogChart->Titles[0]->Visible == true) { ChartHeight = ChartHeight - TitleHeightPixels; }

				bool ReserveZoomRoom = (_DataLogPlotArea_ReserveZoomRoom == 1 || Zoomed == true);

				double UseLeft = (ReserveZoomRoom == true) ? ZoomLeftMarginPixels : LeftMarginPixels;
				double UseTop = (ReserveZoomRoom == true) ? ZoomTopMarginPixels : TopMarginPixels;
				double UseBottom = (ReserveZoomRoom == true) ? ZoomBottomMarginPixels : BottomMarginPixels;

				// A graph too small for fixed margins gets the automatic layout of the chart
				if (ChartWidth < UseLeft + RightMarginPixels + 100.0 || ChartHeight < UseTop + UseBottom + 60.0) {

					if (Area->InnerPlotPosition->Auto == false) { Area->InnerPlotPosition->Auto = true; }
					return;

				}

				float WantedX = (float)(UseLeft / ChartWidth * 100.0);
				float WantedY = (float)(UseTop / ChartHeight * 100.0);
				float WantedWidth = (float)((ChartWidth - UseLeft - RightMarginPixels) / ChartWidth * 100.0);
				float WantedHeight = (float)((ChartHeight - UseTop - UseBottom) / ChartHeight * 100.0);

				// Only written when it changes, so the chart is not laid out again at every refresh
				System::Windows::Forms::DataVisualization::Charting::ElementPosition^ Inner = Area->InnerPlotPosition;

				if (Inner->Auto == true || System::Math::Abs(Inner->X - WantedX) > 0.05f || System::Math::Abs(Inner->Y - WantedY) > 0.05f ||
					System::Math::Abs(Inner->Width - WantedWidth) > 0.05f || System::Math::Abs(Inner->Height - WantedHeight) > 0.05f) {

					Area->InnerPlotPosition = gcnew System::Windows::Forms::DataVisualization::Charting::ElementPosition(WantedX, WantedY, WantedWidth, WantedHeight);

				}

			}
			catch (System::Exception^) {}

		}

		System::Void ApplyAxisSettings() {

			// The size of the plot area is fixed, whatever the tick settings are
			this->UpdatePlotAreaLayout();

			// Applies the number of ticks and the locked temperature range to the axes of the graph. With everything on automatic nothing is changed
			// (and the automatic scaling of the graph is given back after a setting is switched to automatic).
			if (this->XTickCount == 0 && this->YTickCount == 0 && this->YRangeLocked == false && this->XRangeLocked == false &&
				this->XRangeSetByProgram == false && this->YRangeSetByProgram == false && this->XIntervalSetByProgram == false && this->YIntervalSetByProgram == false) { return; }

			try {

				System::Windows::Forms::DataVisualization::Charting::ChartArea^ Area = this->LogChart->ChartAreas[0];
				System::Windows::Forms::DataVisualization::Charting::Axis^ AxisX = Area->AxisX;
				System::Windows::Forms::DataVisualization::Charting::Axis^ AxisY = Area->AxisY;

				// ---- Time axis range ----
				if (this->XRangeLocked == true) {

					double StartDay = this->ChartStartTime.ToOADate();
					double LockedStartDay = StartDay + this->XLockedMinimum / 86400.0;
					double LockedEndDay = StartDay + this->XLockedMaximum / 86400.0;

					if (AxisX->Minimum != LockedStartDay || AxisX->Maximum != LockedEndDay) {

						AxisX->Minimum = LockedStartDay;
						AxisX->Maximum = LockedEndDay;

					}

					this->XRangeSetByProgram = true;

				}
				else if (this->XRangeSetByProgram == true) {

					// Back to automatic
					AxisX->Minimum = System::Double::NaN;
					AxisX->Maximum = System::Double::NaN;
					this->XRangeSetByProgram = false;

				}

				// ---- Temperature axis range ----
				bool HasRange = false;
				double RangeMinimum = 0.0;
				double RangeMaximum = 1.0;

				if (this->YRangeLocked == true) {

					RangeMinimum = this->YLockedMinimum;
					RangeMaximum = this->YLockedMaximum;
					HasRange = true;

				}
				else if (this->YTickCount > 0 && AxisY->ScaleView->IsZoomed == false) {

					// The ticks are spread evenly over the range of the data (with a small margin, rounded to one decimal)
					double DataMinimum = 0.0;
					double DataMaximum = 0.0;

					if (this->GetDataRange(DataMinimum, DataMaximum)) {

						double Margin = System::Math::Max((DataMaximum - DataMinimum) * 0.05, 0.1);
						RangeMinimum = System::Math::Floor((DataMinimum - Margin) * 10.0) / 10.0;
						RangeMaximum = System::Math::Ceiling((DataMaximum + Margin) * 10.0) / 10.0;
						HasRange = true;

					}

				}

				if (HasRange == true) {

					if (AxisY->Minimum != RangeMinimum || AxisY->Maximum != RangeMaximum) {

						AxisY->Minimum = RangeMinimum;
						AxisY->Maximum = RangeMaximum;

					}

					this->YRangeSetByProgram = true;

				}
				else if (this->YRangeSetByProgram == true && this->YTickCount == 0) {

					// Back to automatic
					AxisY->Minimum = System::Double::NaN;
					AxisY->Maximum = System::Double::NaN;
					this->YRangeSetByProgram = false;

				}

				// ---- Temperature axis ticks ----
				if (this->YTickCount > 0) {

					double ViewMinimum = 0.0;
					double ViewMaximum = 0.0;

					if (AxisY->ScaleView->IsZoomed == true) { ViewMinimum = AxisY->ScaleView->ViewMinimum; ViewMaximum = AxisY->ScaleView->ViewMaximum; }
					else if (HasRange == true) { ViewMinimum = RangeMinimum; ViewMaximum = RangeMaximum; }

					if (ViewMaximum > ViewMinimum) {

						AxisY->Interval = (ViewMaximum - ViewMinimum) / (double)this->YTickCount;
						this->YIntervalSetByProgram = true;

					}

				}
				else if (this->YIntervalSetByProgram == true) {

					AxisY->Interval = 0;
					this->YIntervalSetByProgram = false;

				}

				// ---- Time axis ticks ----
				if (this->XTickCount > 0) {

					// The range that the ticks are for: the locked range, the zoomed view, or the data (from 0:00:00 to the newest point). It is not read from the axis:
					// the end of the axis is rounded by the chart to a multiple of the tick interval, so a range read from the axis depends on the interval.
					double RangeSeconds = 0.0;

					if (this->XRangeLocked == true) { RangeSeconds = this->XLockedMaximum - this->XLockedMinimum; }
					else if (AxisX->ScaleView->IsZoomed == true) { RangeSeconds = (AxisX->ScaleView->ViewMaximum - AxisX->ScaleView->ViewMinimum) * 86400.0; }
					else {

						double EndDay = 0.0;
						if (this->GetDataEndTime(EndDay) == true) { RangeSeconds = (EndDay - this->ChartStartTime.ToOADate()) * 86400.0; }

					}

					if (!System::Double::IsNaN(RangeSeconds) && RangeSeconds > 0.0) {

						// The tick interval is a round time step (see GetRoundTickSeconds), so it only changes when the range grows into the next step, and it is only
						// written to the axis when it changes. Writing a new, odd interval and range to the axis at every refresh made the graph re-layout every second.
						double WantedSeconds = this->GetRoundTickSeconds(RangeSeconds / (double)this->XTickCount);

						if (this->XIntervalSetByProgram == false || WantedSeconds != this->XIntervalSeconds) {

							AxisX->IntervalType = System::Windows::Forms::DataVisualization::Charting::DateTimeIntervalType::Seconds;
							AxisX->Interval = WantedSeconds;
							this->XIntervalSeconds = WantedSeconds;
							this->XIntervalSetByProgram = true;

						}

					}

				}
				else if (this->XIntervalSetByProgram == true) {

					AxisX->Interval = 0;
					AxisX->IntervalType = System::Windows::Forms::DataVisualization::Charting::DateTimeIntervalType::Auto;
					this->XIntervalSetByProgram = false;

				}

			}
			catch (System::Exception^) {}

		}

		System::Void LogChart_FormatNumber(System::Object^ sender, System::Windows::Forms::DataVisualization::Charting::FormatNumberEventArgs^ e) {

			// Only the labels of the (date and time) time axis
			if (e->ElementType == System::Windows::Forms::DataVisualization::Charting::ChartElementType::AxisLabels &&
				e->ValueType == System::Windows::Forms::DataVisualization::Charting::ChartValueType::DateTime) {

				double ElapsedMilliSec = (e->Value - this->ChartStartTime.ToOADate()) * 86400000.0;
				if (ElapsedMilliSec < 0.0) { ElapsedMilliSec = 0.0; }

				e->LocalizedValue = this->FormatDuration(ElapsedMilliSec);

			}

		}

		System::Void UpdateAxisTitles() {

			// The axis titles with the units - the temperature unit follows the temperature unit setting of the program
			this->LogChart->ChartAreas[0]->AxisY->Title = gcnew System::String("Temperature (") + GlobalVariables::DefaultTempUnitString + gcnew System::String(")");
			this->LogChart->ChartAreas[0]->AxisX->Title = "Time (h:mm:ss)";

		}

		System::Void ClearChartData() {

			// Removes the points from all lines of the graph
			for (int i = 0; i < this->LogChart->Series->Count; i++) { this->LogChart->Series[i]->Points->Clear(); }
			for (int i = 0; i < this->SeriesBuffers->Count; i++) { this->SeriesBuffers[i]->Clear(); }

			// The cursors and the markers belonged to the data that was removed
			this->ClearMeasureCursors();
			this->ClearGraphMarkers();

		}

		System::Void ExportGraphButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Saves the graph as an image (PNG, JPEG or BMP) - the graph as it is shown now (zoom, ticks, Y range), without the hidden lines,
			// with the names of the lines that are shown (in the color of the lines) in a strip above the graph, then a table below the graph (thermal camera configuration, export information, other information and the notes)
			if (this->LogChart->Series->Count == 0) {

				System::Windows::Forms::MessageBox::Show(this, "There is no graph to export yet. Start a data logging session first.", "Export Graph",
					System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Information);
				return;

			}

			// First the window for the title of the graph (it shows the title that is set now - keep it or change it). Cancel cancels the export.
			// The window opens also when the title is hidden (Show Graph Title not ticked): a new title shows it, an empty text hides it, the same text keeps it as it is.
			if (this->ShowGraphTitleWindow() == false) { return; }

			// Then the window for the notes (it shows the notes that are set now - keep them, change them or empty the text). Cancel cancels the export.
			if (this->ShowGraphNotesWindow() == false) { return; }

			// Default file name: the name of the session file + _graph, or the date and time
			System::String^ DefaultName;
			System::String^ SessionFileName = GlobalVariables::DataLoggingSessionFileNameString;

			if (SessionFileName != nullptr && SessionFileName->Length > 0) { DefaultName = System::IO::Path::GetFileNameWithoutExtension(SessionFileName) + gcnew System::String("_graph"); }
			else {

				System::DateTime NowTime = System::DateTime::Now;
				DefaultName = gcnew System::String("DataLogGraph_") + NowTime.ToString("ddMMMyyyy_HH-mm-ss", System::Globalization::CultureInfo::InvariantCulture);

			}

			System::Windows::Forms::SaveFileDialog^ SaveDialog = gcnew System::Windows::Forms::SaveFileDialog();
			SaveDialog->Title = "Export Graph";
			SaveDialog->Filter = "PNG image (*.png)|*.png|JPEG image (*.jpg)|*.jpg|Bitmap image (*.bmp)|*.bmp";
			SaveDialog->FilterIndex = 1;
			SaveDialog->AddExtension = true;
			SaveDialog->OverwritePrompt = true;
			SaveDialog->FileName = DefaultName;

			if (GlobalVariables::LoggingCSVDefaultPath != nullptr && System::IO::Directory::Exists(GlobalVariables::LoggingCSVDefaultPath)) { SaveDialog->InitialDirectory = GlobalVariables::LoggingCSVDefaultPath; }

			if (SaveDialog->ShowDialog(this) != System::Windows::Forms::DialogResult::OK) { return; }

			// The image format follows the extension of the file name (PNG when it is something else)
			System::String^ Extension = System::IO::Path::GetExtension(SaveDialog->FileName)->ToLowerInvariant();
			System::Drawing::Imaging::ImageFormat^ SaveFormat = System::Drawing::Imaging::ImageFormat::Png;
			if (Extension == ".jpg" || Extension == ".jpeg") { SaveFormat = System::Drawing::Imaging::ImageFormat::Jpeg; }
			else if (Extension == ".bmp") { SaveFormat = System::Drawing::Imaging::ImageFormat::Bmp; }

			System::IO::MemoryStream^ ChartStream = nullptr;
			System::Drawing::Bitmap^ ChartImage = nullptr;
			System::Drawing::Bitmap^ OutputImage = nullptr;
			System::IO::MemoryStream^ VectorStream = nullptr;
			System::Drawing::Imaging::Metafile^ VectorImage = nullptr;

			// The title of the graph and the titles of the axes are made bigger for the image (and put back in the finally part below)
			System::Windows::Forms::DataVisualization::Charting::Title^ TitleObject = this->LogChart->Titles->FindByName("GraphTitle");
			System::Drawing::Font^ SavedTitleFont = nullptr;
			System::Drawing::Font^ SavedXTitleFont = nullptr;
			System::Drawing::Font^ SavedYTitleFont = nullptr;

			// All the text of the image apart from the values on the axes is 20 % bigger than those values
			const float TextScale = 1.2f;

			try {

				if (TitleObject != nullptr) {

					SavedTitleFont = TitleObject->Font;
					TitleObject->Font = gcnew System::Drawing::Font(SavedTitleFont->FontFamily, SavedTitleFont->Size * TextScale, SavedTitleFont->Style, SavedTitleFont->Unit);

				}

				SavedXTitleFont = this->LogChart->ChartAreas[0]->AxisX->TitleFont;
				this->LogChart->ChartAreas[0]->AxisX->TitleFont = gcnew System::Drawing::Font(SavedXTitleFont->FontFamily, SavedXTitleFont->Size * TextScale, SavedXTitleFont->Style, SavedXTitleFont->Unit);
				SavedYTitleFont = this->LogChart->ChartAreas[0]->AxisY->TitleFont;
				this->LogChart->ChartAreas[0]->AxisY->TitleFont = gcnew System::Drawing::Font(SavedYTitleFont->FontFamily, SavedYTitleFont->Size * TextScale, SavedYTitleFont->Style, SavedYTitleFont->Unit);

				ChartStream = gcnew System::IO::MemoryStream();
				this->LogChart->SaveImage(ChartStream, System::Windows::Forms::DataVisualization::Charting::ChartImageFormat::Png);
				ChartImage = gcnew System::Drawing::Bitmap(ChartStream);

#if _DataLogExport_UseVectorChart
				// The same graph as a vector drawing (EMF+): it can be drawn at any resolution and stays sharp. When it is not possible the picture is used.
				try {

					VectorStream = gcnew System::IO::MemoryStream();
					this->LogChart->SaveImage(VectorStream, System::Windows::Forms::DataVisualization::Charting::ChartImageFormat::EmfPlus);
					VectorStream->Position = 0;
					VectorImage = gcnew System::Drawing::Imaging::Metafile(VectorStream);

				}
				catch (System::Exception^) {

					if (VectorImage != nullptr) { delete VectorImage; VectorImage = nullptr; }

				}
#endif

				// The lines that are shown
				System::Collections::Generic::List<System::String^>^ LineNames = gcnew System::Collections::Generic::List<System::String^>();
				System::Collections::Generic::List<System::Drawing::Color>^ LineColors = gcnew System::Collections::Generic::List<System::Drawing::Color>();

				for (int i = 0; i < this->LogChart->Series->Count; i++) {

					if (this->LogChart->Series[i]->Enabled == true) {

						LineNames->Add(this->LogChart->Series[i]->LegendText);
						LineColors->Add(this->LogChart->Series[i]->Color);

					}

				}

				// All the text around the graph (names of the lines, axis ranges, table) has the font of the values on the axes (for example 20.0 and 0:00:00),
				// 20 % bigger. The title of the graph and the titles of the axes are bigger by the same factor.
				// The values on the axes are not changed.
				System::Drawing::Font^ AxisFont = this->LogChart->ChartAreas[0]->AxisY->LabelStyle->Font;
				System::Drawing::Font^ NameFont = gcnew System::Drawing::Font(AxisFont->FontFamily, AxisFont->Size * TextScale, System::Drawing::FontStyle::Regular, AxisFont->Unit);
				float TextHeight = NameFont->GetHeight(96.0f);
				int RowHeight = (int)System::Math::Ceiling((double)TextHeight) + 7;
				int SquareSize = (int)(TextHeight * 0.7f);
				const float EdgeMargin = 10.0f;
				const float EntryGap = 18.0f;

				// First pass: how many rows of names fit in the width of the image
				System::Drawing::Graphics^ MeasureGraphics = System::Drawing::Graphics::FromImage(ChartImage);
				int Rows = 0;
				float PositionX = EdgeMargin;
				System::Collections::Generic::List<float>^ EntryWidths = gcnew System::Collections::Generic::List<float>();

				for (int i = 0; i < LineNames->Count; i++) {

					float EntryWidth = (float)SquareSize + 6.0f + MeasureGraphics->MeasureString(LineNames[i], NameFont).Width + EntryGap;
					EntryWidths->Add(EntryWidth);
					if (Rows == 0 || (PositionX + EntryWidth > (float)ChartImage->Width - EdgeMargin && PositionX > EdgeMargin)) { Rows++; PositionX = EdgeMargin; }
					PositionX = PositionX + EntryWidth;

				}

				// The first and last value of both axes as shown now (the labels on the axes only show the ticks, so the last value is often missing there).
				// They are written in the strip above the graph, in one line, or in two lines when the graph is too narrow for one.
				System::String^ TimeRangeText = nullptr;
				System::String^ TemperatureRangeText = nullptr;
				this->GetAxisRangeTexts(TimeRangeText, TemperatureRangeText);

				System::Collections::Generic::List<System::String^>^ RangeLines = gcnew System::Collections::Generic::List<System::String^>();

				if (TimeRangeText->Length > 0 && TemperatureRangeText->Length > 0) {

					System::String^ CombinedText = TimeRangeText + gcnew System::String("      ") + TemperatureRangeText;

					if (MeasureGraphics->MeasureString(CombinedText, NameFont).Width <= (float)ChartImage->Width - 2.0f * EdgeMargin) { RangeLines->Add(CombinedText); }
					else { RangeLines->Add(TimeRangeText); RangeLines->Add(TemperatureRangeText); }

				}
				else if (TimeRangeText->Length > 0) { RangeLines->Add(TimeRangeText); }
				else if (TemperatureRangeText->Length > 0) { RangeLines->Add(TemperatureRangeText); }

				delete MeasureGraphics;

				int StripHeight = 0;
				if (Rows + RangeLines->Count > 0) { StripHeight = (Rows + RangeLines->Count) * RowHeight + 8; }

				// Table below the graph, in this order from left to right: Thermal Camera Configuration, Export Information, Other Information and (when there are
				// notes) Notes. A cell has a heading and the lines of its list (label on the left, value next to it). The notes cell takes the rest of the width; when
				// the image is too narrow for all cells, the cells that do not fit go to a next row.
				System::Collections::Generic::List<System::String^>^ ConfigLines = gcnew System::Collections::Generic::List<System::String^>();
				System::Collections::Generic::List<System::String^>^ ExportLines = gcnew System::Collections::Generic::List<System::String^>();
				System::Collections::Generic::List<System::String^>^ OtherLines = gcnew System::Collections::Generic::List<System::String^>();
				this->GetCameraConfigLines(ConfigLines);
				this->GetExportInfoLines(ExportLines, OtherLines);

				System::String^ NotesText = (this->GraphNotes != nullptr) ? this->GraphNotes->Trim() : gcnew System::String("");
				int CellCount = 3;
				if (NotesText->Length > 0) { CellCount = 4; }

				cli::array<System::String^>^ CellHeadings = gcnew cli::array<System::String^>{ "Thermal Camera Configuration:", "Export Information:", "Other Information:", "Notes:" };
				cli::array<System::Collections::Generic::List<System::String^>^>^ CellLists = gcnew cli::array<System::Collections::Generic::List<System::String^>^>{ ConfigLines, ExportLines, OtherLines, nullptr };

				System::Drawing::Font^ HeadingFont = gcnew System::Drawing::Font(NameFont, System::Drawing::FontStyle::Bold);
				System::Drawing::Graphics^ FooterMeasure = System::Drawing::Graphics::FromImage(ChartImage);

				const float CellPad = 8.0f;
				const float LabelGap = 12.0f;
				const float MinNotesWidth = 220.0f;
				const float TableTop = 8.0f;
				float TableRight = (float)ChartImage->Width - EdgeMargin;

				cli::array<float>^ CellX = gcnew cli::array<float>(CellCount);
				cli::array<float>^ CellY = gcnew cli::array<float>(CellCount);
				cli::array<float>^ CellW = gcnew cli::array<float>(CellCount);
				cli::array<float>^ CellH = gcnew cli::array<float>(CellCount);
				cli::array<float>^ CellLabelW = gcnew cli::array<float>(CellCount);

				float CursorX = EdgeMargin;
				float RowTop = 0.0f;
				float RowBottom = 0.0f;

				for (int c = 0; c < CellCount; c++) {

					float CellWidth = 0.0f;
					float CellHeight = 0.0f;
					CellLabelW[c] = 0.0f;

					if (c < 3) {

						// Width: the widest of the heading and the (label + value) lines
						float ValueWidth = 0.0f;

						for (int i = 0; i < CellLists[c]->Count; i++) {

							System::String^ LabelText = nullptr;
							System::String^ ValueText = nullptr;
							this->SplitInfoLine(CellLists[c][i], LabelText, ValueText);
							CellLabelW[c] = System::Math::Max(CellLabelW[c], FooterMeasure->MeasureString(LabelText, NameFont).Width);
							ValueWidth = System::Math::Max(ValueWidth, FooterMeasure->MeasureString(ValueText, NameFont).Width);

						}

						CellWidth = System::Math::Max(FooterMeasure->MeasureString(CellHeadings[c], HeadingFont).Width, CellLabelW[c] + LabelGap + ValueWidth) + (2.0f * CellPad);

						// The cell does not fit in the rest of the row: next row
						if (CursorX + CellWidth > TableRight && CursorX > EdgeMargin) { RowTop = RowBottom; CursorX = EdgeMargin; }

						CellHeight = (float)((CellLists[c]->Count + 1) * RowHeight) + 8.0f;

					}
					else {

						// The notes take the rest of the width of the row (a full row when less than the minimum width is left)
						if (TableRight - CursorX < MinNotesWidth && CursorX > EdgeMargin) { RowTop = RowBottom; CursorX = EdgeMargin; }

						CellWidth = TableRight - CursorX;
						float NotesTextHeight = FooterMeasure->MeasureString(NotesText, NameFont, (int)(CellWidth - (2.0f * CellPad))).Height;
						CellHeight = (float)RowHeight + NotesTextHeight + 12.0f;

					}

					CellX[c] = CursorX;
					CellY[c] = RowTop;
					CellW[c] = CellWidth;
					CellH[c] = CellHeight;

					CursorX = CursorX + CellWidth;
					RowBottom = System::Math::Max(RowBottom, RowTop + CellHeight);

				}

				delete FooterMeasure;

				// The cells in the same row get the height of the highest one, so the borders line up
				for (int c = 0; c < CellCount; c++) {

					for (int d = 0; d < CellCount; d++) {

						if (CellY[d] == CellY[c]) { CellH[c] = System::Math::Max(CellH[c], CellH[d]); }

					}

				}

				int FooterHeight = (int)System::Math::Ceiling((double)(TableTop + RowBottom)) + 10;

				// The image has _DataLogExport_ResolutionScale times more pixels in width and height. Everything below is drawn in the units of the graph on the
				// screen: the scale transform makes the text, lines and the table sharp at the higher resolution.
				const int ImageScale = _DataLogExport_ResolutionScale;
				OutputImage = gcnew System::Drawing::Bitmap(ChartImage->Width * ImageScale, (ChartImage->Height + StripHeight + FooterHeight) * ImageScale);
				System::Drawing::Graphics^ OutputGraphics = System::Drawing::Graphics::FromImage(OutputImage);
				OutputGraphics->Clear(System::Drawing::Color::FromArgb(35, 35, 35));
				OutputGraphics->ScaleTransform((float)ImageScale, (float)ImageScale);
				OutputGraphics->TextRenderingHint = System::Drawing::Text::TextRenderingHint::AntiAliasGridFit;
				OutputGraphics->InterpolationMode = System::Drawing::Drawing2D::InterpolationMode::HighQualityBicubic;
				OutputGraphics->PixelOffsetMode = System::Drawing::Drawing2D::PixelOffsetMode::Half;

				// Second pass: draw the names above the graph
				int RowIndex = -1;
				PositionX = EdgeMargin;

				for (int i = 0; i < LineNames->Count; i++) {

					float EntryWidth = EntryWidths[i];
					if (RowIndex < 0 || (PositionX + EntryWidth > (float)ChartImage->Width - EdgeMargin && PositionX > EdgeMargin)) { RowIndex++; PositionX = EdgeMargin; }

					float PositionY = 4.0f + (float)(RowIndex * RowHeight);
					System::Drawing::SolidBrush^ LineBrush = gcnew System::Drawing::SolidBrush(LineColors[i]);
					OutputGraphics->FillRectangle(LineBrush, PositionX, PositionY + ((TextHeight - (float)SquareSize) / 2.0f), (float)SquareSize, (float)SquareSize);
					OutputGraphics->DrawString(LineNames[i], NameFont, LineBrush, PositionX + (float)SquareSize + 6.0f, PositionY);
					delete LineBrush;

					PositionX = PositionX + EntryWidth;

				}

				// The axis ranges, below the names of the lines
				System::Drawing::SolidBrush^ RangeBrush = gcnew System::Drawing::SolidBrush(System::Drawing::Color::FromArgb(220, 220, 220));

				for (int i = 0; i < RangeLines->Count; i++) {

					OutputGraphics->DrawString(RangeLines[i], NameFont, RangeBrush, EdgeMargin, 4.0f + (float)((Rows + i) * RowHeight));

				}

				delete RangeBrush;

				if (VectorImage != nullptr) { OutputGraphics->DrawImage(VectorImage, 0, StripHeight, ChartImage->Width, ChartImage->Height); }
				else { OutputGraphics->DrawImage(ChartImage, 0, StripHeight, ChartImage->Width, ChartImage->Height); }

				// The readout of the two-cursor measurement, on the graph, as in the window (the cursor lines are already part of the graph)
				this->DrawMeasurePanelOnExport(OutputGraphics, (float)StripHeight);

				// The table below the graph
				System::Drawing::SolidBrush^ ConfigBrush = gcnew System::Drawing::SolidBrush(System::Drawing::Color::FromArgb(220, 220, 220));
				System::Drawing::SolidBrush^ HeadingBackBrush = gcnew System::Drawing::SolidBrush(System::Drawing::Color::FromArgb(60, 60, 60));
				System::Drawing::Pen^ TablePen = gcnew System::Drawing::Pen(System::Drawing::Color::FromArgb(120, 120, 120), 1.0f);
				float TableOriginY = (float)(StripHeight + ChartImage->Height) + TableTop;
				float TextOffsetY = ((float)RowHeight - TextHeight) / 2.0f;

				for (int c = 0; c < CellCount; c++) {

					float X = CellX[c];
					float Y = TableOriginY + CellY[c];

					OutputGraphics->FillRectangle(HeadingBackBrush, X, Y, CellW[c], (float)RowHeight);
					OutputGraphics->DrawRectangle(TablePen, X, Y, CellW[c], CellH[c]);
					OutputGraphics->DrawLine(TablePen, X, Y + (float)RowHeight, X + CellW[c], Y + (float)RowHeight);
					OutputGraphics->DrawString(CellHeadings[c], HeadingFont, ConfigBrush, X + CellPad, Y + TextOffsetY);

					if (c < 3) {

						for (int i = 0; i < CellLists[c]->Count; i++) {

							System::String^ LabelText = nullptr;
							System::String^ ValueText = nullptr;
							this->SplitInfoLine(CellLists[c][i], LabelText, ValueText);

							float LineY = Y + (float)((i + 1) * RowHeight) + 4.0f + TextOffsetY;
							OutputGraphics->DrawString(LabelText, NameFont, ConfigBrush, X + CellPad, LineY);
							OutputGraphics->DrawString(ValueText, NameFont, ConfigBrush, X + CellPad + CellLabelW[c] + LabelGap, LineY);

						}

					}
					else {

						System::Drawing::RectangleF NotesRectangle = System::Drawing::RectangleF(X + CellPad, Y + (float)RowHeight + 6.0f, CellW[c] - (2.0f * CellPad), CellH[c] - (float)RowHeight - 6.0f);
						OutputGraphics->DrawString(NotesText, NameFont, ConfigBrush, NotesRectangle);

					}

				}

				delete TablePen;
				delete HeadingBackBrush;
				delete ConfigBrush;
				delete HeadingFont;
				delete OutputGraphics;
				delete NameFont;

				if (SaveFormat == System::Drawing::Imaging::ImageFormat::Jpeg) {

					// JPEG with a high quality (the default quality of the framework makes the text blurry)
					System::Drawing::Imaging::ImageCodecInfo^ JpegCodec = nullptr;
					cli::array<System::Drawing::Imaging::ImageCodecInfo^>^ Codecs = System::Drawing::Imaging::ImageCodecInfo::GetImageEncoders();

					for (int i = 0; i < Codecs->Length; i++) {

						if (Codecs[i]->FormatID == System::Drawing::Imaging::ImageFormat::Jpeg->Guid) { JpegCodec = Codecs[i]; }

					}

					if (JpegCodec != nullptr) {

						System::Drawing::Imaging::EncoderParameters^ JpegParameters = gcnew System::Drawing::Imaging::EncoderParameters(1);
						JpegParameters->Param[0] = gcnew System::Drawing::Imaging::EncoderParameter(System::Drawing::Imaging::Encoder::Quality, (System::Int64)_DataLogExport_JpegQuality);
						OutputImage->Save(SaveDialog->FileName, JpegCodec, JpegParameters);

					}
					else { OutputImage->Save(SaveDialog->FileName, SaveFormat); }

				}
				else { OutputImage->Save(SaveDialog->FileName, SaveFormat); }

			}
			catch (System::Exception^ ExportError) {

				System::Windows::Forms::MessageBox::Show(this, gcnew System::String("The graph could not be saved: ") + ExportError->Message, "Export Graph",
					System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Error);

			}
			finally {

				// Put the titles back to their size on the screen
				if (TitleObject != nullptr && SavedTitleFont != nullptr) { TitleObject->Font = SavedTitleFont; }
				if (SavedXTitleFont != nullptr) { this->LogChart->ChartAreas[0]->AxisX->TitleFont = SavedXTitleFont; }
				if (SavedYTitleFont != nullptr) { this->LogChart->ChartAreas[0]->AxisY->TitleFont = SavedYTitleFont; }

				if (OutputImage != nullptr) { delete OutputImage; }
				if (VectorImage != nullptr) { delete VectorImage; }
				if (VectorStream != nullptr) { delete VectorStream; }
				if (ChartImage != nullptr) { delete ChartImage; }
				if (ChartStream != nullptr) { delete ChartStream; }

			}

		}

		System::Void GetCameraConfigLines(System::Collections::Generic::List<System::String^>^ Lines) {

			// The thermal camera configuration (the values of the "Thermal Camera Configuration" panel in Settings that are in use by the camera now), one text
			// per line, for the list in an exported graph image. Temperatures are shown in the temperature unit of the program, like the panel does.
			System::String^ Unit = GlobalVariables::DefaultTempUnitString;

			double TemperatureCorrection = (double)IRCamera.TemperatureCorrectionSetting;
			double AmbientTemperature = ((double)IRCamera.AmbientTemperatureSetting * (double)TemperatureUnitScaleFactor) + (double)TemperatureUnitOffsetFactor;
			double ReflectedTemperature = ((double)IRCamera.ReflectedTemperatureSetting * (double)TemperatureUnitScaleFactor) + (double)TemperatureUnitOffsetFactor;
			double Humidity = (double)IRCamera.HumiditySetting;
			double Emissivity = (double)IRCamera.EmissivitySetting;
			double Distance = (double)IRCamera.DistanceSetting;

			Lines->Add(gcnew System::String("Temperature Correction: ") + TemperatureCorrection.ToString("0.0#") + gcnew System::String(" ") + Unit);
			Lines->Add(gcnew System::String("Ambient Temperature: ") + AmbientTemperature.ToString("0.0#") + gcnew System::String(" ") + Unit);
			Lines->Add(gcnew System::String("Reflected Temperature: ") + ReflectedTemperature.ToString("0.0#") + gcnew System::String(" ") + Unit);
			Lines->Add(gcnew System::String("Surrounding Humidity: ") + Humidity.ToString("0.0#") + gcnew System::String(" %"));
			Lines->Add(gcnew System::String("Object Surface Emissivity: ") + Emissivity.ToString("0.00#"));
			Lines->Add(gcnew System::String("Surface Distance: ") + Distance.ToString("0.0#") + gcnew System::String(" m"));

		}

		System::Void SplitInfoLine(System::String^ Line, System::String^% LabelText, System::String^% ValueText) {

			// Splits a "Label: value" line of the tables of an exported image at the first ": " (the label keeps the colon)
			int Position = Line->IndexOf(": ");

			if (Position < 0) { LabelText = Line; ValueText = System::String::Empty; }
			else { LabelText = Line->Substring(0, Position + 1); ValueText = Line->Substring(Position + 2); }

		}

		System::Void GetExportInfoLines(System::Collections::Generic::List<System::String^>^ ExportLines, System::Collections::Generic::List<System::String^>^ OtherLines) {

			// Information for later reference, written in two tables below the graph in an exported image. All values are the ones at the moment of the export.
			// ExportLines: date and time of the export, the logged time and the settings / size of the data logging session.
			// OtherLines: log file, camera model and temperature range, calibration settings and the temperature unit.
			System::DateTime NowTime = System::DateTime::Now;
			ExportLines->Add(gcnew System::String("Date: ") + NowTime.ToString("dd MMM yyyy", System::Globalization::CultureInfo::InvariantCulture));
			ExportLines->Add(gcnew System::String("Time: ") + NowTime.ToString("HH:mm:ss", System::Globalization::CultureInfo::InvariantCulture));
			ExportLines->Add(gcnew System::String("Logged Time: ") + this->FormatDuration(this->LastElapsedMilliSec));
			ExportLines->Add(gcnew System::String("Session Duration Setting: ") + this->FormatDuration((double)DataLoggingSessionDurationMilliSec));

			double IntervalSeconds = (double)DataLoggingIntervalMilliSec / 1000.0;
			ExportLines->Add(gcnew System::String("Sample Interval: ") + IntervalSeconds.ToString("0.0##") + gcnew System::String(" s"));
			ExportLines->Add(gcnew System::String("Samples In Data Log: ") + System::Convert::ToString(this->ViewedSampleRows));

			// Log file
			System::String^ SessionFileName = GlobalVariables::DataLoggingSessionFileNameString;
			if (SessionFileName != nullptr && SessionFileName->Length > 0) { OtherLines->Add(gcnew System::String("Log File: ") + System::IO::Path::GetFileName(SessionFileName)); }
			else { OtherLines->Add(gcnew System::String("Log File: none")); }

			// Camera model (the index is the camera macro index, which is also the index in the list of model names)
			System::String^ ModelName = gcnew System::String("Unknown");
			if ((size_t)IRCamera.SellectedCameraIndex < SupportedCamerasModelNames.size()) { ModelName = gcnew System::String(SupportedCamerasModelNames[IRCamera.SellectedCameraIndex].c_str()); }
			OtherLines->Add(gcnew System::String("Camera Model: ") + ModelName);

			if (IRCamera.CurrentIRTempRangeFlag == 2) { OtherLines->Add(gcnew System::String("Camera Temperature Range: High")); }
			else { OtherLines->Add(gcnew System::String("Camera Temperature Range: Low")); }

			// Periodic (auto) shutter calibration and the temperature-drift-based calibration
			if (AutoShutterCalEnableFlag == true) {

				double CalibrationPeriod = 0.0;
				if (GlobalVariables::GlobalAutoCalPeriodUpDown != nullptr) { CalibrationPeriod = System::Decimal::ToDouble(GlobalVariables::GlobalAutoCalPeriodUpDown->Value); }
				OtherLines->Add(gcnew System::String("Auto Calibration: On, every ") + CalibrationPeriod.ToString("0.#") + gcnew System::String(" s"));

			}
			else { OtherLines->Add(gcnew System::String("Auto Calibration: Off")); }

			if (DriftBasedCalEnableFlag == true) {

				double DriftThreshold = (double)TempDriftCalibrationSetValue;
				OtherLines->Add(gcnew System::String("Drift-Based Calibration: On, at ") + DriftThreshold.ToString("0.0##") + gcnew System::String(" ") + GlobalVariables::DefaultTempUnitString);

			}
			else { OtherLines->Add(gcnew System::String("Drift-Based Calibration: Off")); }

			OtherLines->Add(gcnew System::String("Temperature Unit: ") + GlobalVariables::DefaultTempUnitString);

		}

		System::Void GetAxisRangeTexts(System::String^% TimeText, System::String^% TemperatureText) {

			// Text with the first and last value of the time axis and of the temperature axis as they are shown now (zoom, locked range or automatic range).
			// An empty text is returned for an axis whose range is not known.
			TimeText = gcnew System::String("");
			TemperatureText = gcnew System::String("");

			try {

				System::Windows::Forms::DataVisualization::Charting::ChartArea^ Area = this->LogChart->ChartAreas[0];
				System::Windows::Forms::DataVisualization::Charting::Axis^ AxisX = Area->AxisX;
				System::Windows::Forms::DataVisualization::Charting::Axis^ AxisY = Area->AxisY;

				bool XZoomed = AxisX->ScaleView->IsZoomed;
				bool YZoomed = AxisY->ScaleView->IsZoomed;

				// Not while zoomed: recalculating the scale while the view is zoomed would interfere with it (and with the scroll bars)
				if (XZoomed == false && YZoomed == false) { Area->RecalculateAxesScale(); }

				double XMinimum = AxisX->Minimum;
				double XMaximum = AxisX->Maximum;
				double YMinimum = AxisY->Minimum;
				double YMaximum = AxisY->Maximum;

				if (XZoomed == true) { XMinimum = AxisX->ScaleView->ViewMinimum; XMaximum = AxisX->ScaleView->ViewMaximum; }
				if (YZoomed == true) { YMinimum = AxisY->ScaleView->ViewMinimum; YMaximum = AxisY->ScaleView->ViewMaximum; }

				// Time axis (values in days) -> elapsed time of the log session
				if (!System::Double::IsNaN(XMinimum) && !System::Double::IsNaN(XMaximum) && XMaximum > XMinimum) {

					double StartDay = this->ChartStartTime.ToOADate();
					bool WithMilliSeconds = (XZoomed == true && (XMaximum - XMinimum) * 86400.0 < 60.0);	// only a zoomed range of less than a minute needs the milliseconds

					TimeText = gcnew System::String("Time axis: ") + this->FormatAxisTime((XMinimum - StartDay) * 86400000.0, WithMilliSeconds) + gcnew System::String(" to ") +
						this->FormatAxisTime((XMaximum - StartDay) * 86400000.0, WithMilliSeconds) + gcnew System::String(" (h:mm:ss)");

				}

				// Temperature axis
				if (!System::Double::IsNaN(YMinimum) && !System::Double::IsNaN(YMaximum) && YMaximum > YMinimum) {

					TemperatureText = gcnew System::String("Temperature axis: ") + YMinimum.ToString("0.0##") + gcnew System::String(" to ") + YMaximum.ToString("0.0##") +
						gcnew System::String(" ") + GlobalVariables::DefaultTempUnitString;

				}

			}
			catch (System::Exception^) {}

		}

		System::String^ FormatAxisTime(double MilliSeconds, bool WithMilliSeconds) {

			// Elapsed time as h:mm:ss (rounded to the nearest millisecond first, so a value like 59.9996 s is not shown as 59 s); h:mm:ss.fff when asked
			if (MilliSeconds < 0.0) { MilliSeconds = 0.0; }

			System::TimeSpan Duration = System::TimeSpan::FromMilliseconds(System::Math::Round(MilliSeconds));
			System::String^ Text = System::Convert::ToString((int)Duration.TotalHours) + gcnew System::String(":") + Duration.Minutes.ToString("00") + gcnew System::String(":") + Duration.Seconds.ToString("00");

			if (WithMilliSeconds == true) { Text = Text + gcnew System::String(".") + Duration.Milliseconds.ToString("000"); }

			return Text;

		}

		System::Void ResetZoomButton_Click(System::Object^ sender, System::EventArgs^ e) {

			this->LogChart->ChartAreas[0]->AxisX->ScaleView->ZoomReset(0);
			this->LogChart->ChartAreas[0]->AxisY->ScaleView->ZoomReset(0);
			this->ApplyAxisSettings();
			this->UpdateZoomIndicator();

		}

		bool ParseNumber(System::String^ NumberText, double% NumberValue) {

			// Reads a number from the log file - in the regional format of the computer, or in the invariant format
			if (System::Double::TryParse(NumberText, System::Globalization::NumberStyles::Float, System::Globalization::CultureInfo::CurrentCulture, NumberValue)) { return true; }
			return System::Double::TryParse(NumberText, System::Globalization::NumberStyles::Float, System::Globalization::CultureInfo::InvariantCulture, NumberValue);

		}

		// ---------------------------------- Helpers ---------------------------------- //

		System::String^ FormatDuration(double MilliSeconds) {

			System::TimeSpan Duration = System::TimeSpan::FromMilliseconds(MilliSeconds);
			System::String^ Hours = System::Convert::ToString((int)Duration.TotalHours);
			System::String^ Minutes = Duration.Minutes.ToString("00");
			System::String^ Seconds = Duration.Seconds.ToString("00");
			return Hours + gcnew System::String(":") + Minutes + gcnew System::String(":") + Seconds;

		}

		System::Void UpdateStatusText() {

			System::Text::StringBuilder^ StatusBuilder = gcnew System::Text::StringBuilder();
			System::String^ FileName = GlobalVariables::DataLoggingSessionFileNameString;

			if (FileName == nullptr || FileName->Length == 0) {

				StatusBuilder->Append("No data logging session yet. Set the session in Settings (data logging session settings), then press Start Logging.");

			}
			else {

				if (DataLoggingIsRunningFlag == true) { StatusBuilder->Append("Data logging: RUNNING"); }
				else { StatusBuilder->Append("Data logging: stopped"); }

				StatusBuilder->Append("   |   File: ");
				StatusBuilder->Append(FileName);
				StatusBuilder->Append("   |   Samples: ");
				StatusBuilder->Append(System::Convert::ToString(this->ViewedSampleRows));
				StatusBuilder->Append("   |   Elapsed: ");
				StatusBuilder->Append(this->FormatDuration(this->LastElapsedMilliSec));

				if (DataLoggingIsRunningFlag == true && DataLoggingSessionDurationMilliSec > 0) {

					StatusBuilder->Append(" of ");
					StatusBuilder->Append(this->FormatDuration((double)DataLoggingSessionDurationMilliSec));

				}

				// A long file is read over several refreshes of the window
				if (this->LoadingBehind == true && this->LoadTargetLength > 0) {

					StatusBuilder->Append("   |   Reading the log file: ");
					StatusBuilder->Append(System::Convert::ToString((int)((this->ViewedFilePosition * 100) / this->LoadTargetLength)));
					StatusBuilder->Append(" %");

				}

				if (DataLoggingIntervalMilliSec > 0) {

					StatusBuilder->Append("   |   Interval: ");
					StatusBuilder->Append(System::Convert::ToString((double)DataLoggingIntervalMilliSec / 1000.0));
					StatusBuilder->Append(" s");

				}

			}

			this->StatusLabel->Text = StatusBuilder->ToString();

		}

		System::Void ResetView() {

			// Clears the table and starts reading the file from its beginning
			this->LogGrid->Rows->Clear();
			this->LogGrid->Columns->Clear();
			this->LogChart->Series->Clear();
			this->LineChecksPanel->Controls->Clear();
			this->ViewedFilePosition = 0;
			this->PartialLine = "";
			this->ViewedSampleRows = 0;
			this->LastElapsedMilliSec = 0.0;
			this->SeriesBuffers->Clear();
			this->ClearMeasureCursors();
			this->ClearGraphMarkers();
			this->PendingLines->Clear();
			this->ViewDecoder->Reset();
			this->LoadingBehind = false;
			this->ViewHasNewData = false;

			// A new session (or a restart of the view) starts with the whole graph: a zoomed view of the previous session would show a time range that the new
			// data has not reached yet (an empty graph). A locked X / Y range is not zoom and stays.
			this->LogChart->ChartAreas[0]->AxisX->ScaleView->ZoomReset(0);
			this->LogChart->ChartAreas[0]->AxisY->ScaleView->ZoomReset(0);
			this->ApplyAxisSettings();
			this->UpdateZoomIndicator();

		}

		// ---------------------------------- Reading the log file ---------------------------------- //

		System::Drawing::Color GetColumnColor(int Column) {

			// Line color of a log column: the color of the plot data set that feeds the column (Temperature Plot Data Set Settings), so the graph and the 2D plot have the same colors.
			// The colors of the standard order are used when the column has no plot data set.
			int SetColumn = Column - 2;

			if (SetColumn >= 0 && SetColumn < (int)DataLoggingNumberOfActiveSets && GlobalVariables::Plot2DDataSetColorPanels != nullptr) {

				int DataSet = DataLoggingColumnDataSetIndex[SetColumn];

				if (DataSet < GlobalVariables::Plot2DDataSetColorPanels->Length) {

					System::Drawing::Color PanelColor = GlobalVariables::Plot2DDataSetColorPanels[DataSet]->BackColor;
					return System::Drawing::Color::FromArgb(255, PanelColor.R, PanelColor.G, PanelColor.B);

				}

			}

			return this->GetSeriesColor(SetColumn);

		}

		int GetColumnLineWidth(int Column) {

			// Line width of a log column: the line width of the plot data set that feeds the column (1 when there is none)
			int SetColumn = Column - 2;

			if (SetColumn >= 0 && SetColumn < (int)DataLoggingNumberOfActiveSets && GlobalVariables::Plot2DDataSetLineWidthUpDowns != nullptr) {

				int DataSet = DataLoggingColumnDataSetIndex[SetColumn];

				if (DataSet < GlobalVariables::Plot2DDataSetLineWidthUpDowns->Length) {

					int Width = (int)System::Math::Round(System::Decimal::ToDouble(GlobalVariables::Plot2DDataSetLineWidthUpDowns[DataSet]->Value));
					if (Width < 1) { Width = 1; }
					return Width;

				}

			}

			return 1;

		}

		System::Void AddLogColumn(int c, System::String^ HeaderText) {

			// Adds a column to the table and, for a data column, a line to the graph and its check box. Used for the header line of the file and for a data set
			// that is enabled while the session runs (note line #column=name).
			this->LogGrid->Columns->Add(System::Convert::ToString(c), HeaderText);
			this->LogGrid->Columns[c]->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;

			// The data columns (after the sample number and the time) are the lines of the graph
			if (c >= 2) {

				System::Windows::Forms::DataVisualization::Charting::Series^ LineSeries = gcnew System::Windows::Forms::DataVisualization::Charting::Series(gcnew System::String("S") + System::Convert::ToString(c));
				LineSeries->ChartType = System::Windows::Forms::DataVisualization::Charting::SeriesChartType::FastLine;
				LineSeries->XValueType = System::Windows::Forms::DataVisualization::Charting::ChartValueType::DateTime;
				LineSeries->YValueType = System::Windows::Forms::DataVisualization::Charting::ChartValueType::Double;
				LineSeries->ChartArea = "Main";
				LineSeries->LegendText = HeaderText;
				LineSeries->Color = this->GetColumnColor(c);
				LineSeries->BorderWidth = this->GetColumnLineWidth(c);
				LineSeries->ToolTip = "#LEGENDTEXT: #VALY{F2}  (#VALX{HH:mm:ss})";
				this->LogChart->Series->Add(LineSeries);

				// Check box of the line (all lines are shown at first)
				System::Windows::Forms::CheckBox^ LineCheck = gcnew System::Windows::Forms::CheckBox();
				LineCheck->Text = HeaderText;
				LineCheck->AutoSize = true;
				LineCheck->Checked = true;
				LineCheck->ForeColor = this->GetColumnColor(c);
				LineCheck->Tag = System::Convert::ToString(c - 2);
				LineCheck->Margin = System::Windows::Forms::Padding(4, 2, 10, 2);
				LineCheck->CheckedChanged += gcnew System::EventHandler(this, &DataLogGUI::LineCheck_CheckedChanged);
				this->LineChecksPanel->Controls->Add(LineCheck);

			}

		}

		System::Void ApplyLiveStyleChanges() {

			// The line color / line width of a plot data set was changed while the session runs - the line of that data set in the graph follows it
			for (int DataSet = 0; DataSet < _2DPlotMaxNumberOfDataSets; DataSet++) {

				if (DataLoggingStyleChangedFlag[DataSet] == false) { continue; }

				int Column = -1;
				for (int c = 0; c < (int)DataLoggingNumberOfActiveSets; c++) {

					if (DataLoggingColumnDataSetIndex[c] == DataSet) { Column = c + 2; break; }

				}

				// Not a column of the session (a column that is added later reads the color and width when it is created)
				if (Column < 0) { DataLoggingStyleChangedFlag[DataSet] = false; continue; }

				// The column is not in the window yet - the next refresh tries again
				if ((Column - 2) >= this->LogChart->Series->Count || (Column - 2) >= this->LineChecksPanel->Controls->Count) { continue; }

				this->LogChart->Series[Column - 2]->Color = this->GetColumnColor(Column);
				this->LogChart->Series[Column - 2]->BorderWidth = this->GetColumnLineWidth(Column);
				this->LineChecksPanel->Controls[Column - 2]->ForeColor = this->GetColumnColor(Column);

				DataLoggingStyleChangedFlag[DataSet] = false;

			}

		}

		System::Void ApplySourceChangeNote(System::String^ Line) {

			// Note line of the session file: #<column number>=<name>
			//   - the column exists: the source of the column was changed while the session runs - the column gets the new name (table header, graph line, check box)
			//   - the column is the next new one: a plot data set was enabled while the session runs - the column is added (the rows before have no value in it)
			int EqualsPos = Line->IndexOf(L'=');
			if (EqualsPos < 2) { return; }

			int Column = 0;
			if (System::Int32::TryParse(Line->Substring(1, EqualsPos - 1)->Trim(), Column) == false) { return; }

			System::String^ NewName = Line->Substring(EqualsPos + 1)->Trim();
			if (Column < 2) { return; }

			if (Column == this->LogGrid->Columns->Count) { this->AddLogColumn(Column, NewName); return; }
			if (Column > this->LogGrid->Columns->Count) { return; }

			this->LogGrid->Columns[Column]->HeaderText = NewName;
			if ((Column - 2) < this->LogChart->Series->Count) { this->LogChart->Series[Column - 2]->LegendText = NewName; }
			if ((Column - 2) < this->LineChecksPanel->Controls->Count) { this->LineChecksPanel->Controls[Column - 2]->Text = NewName; }

		}

		System::String^ GetViewDelimiter() {

			// The data delimiter of the viewed file: the delimiter found in a recovered session file, else the delimiter setting of the program
			if (this->ViewDelimiter != nullptr && this->ViewDelimiterFilePath != nullptr && this->ViewedFilePath != nullptr &&
				this->ViewedFilePath->Equals(this->ViewDelimiterFilePath, System::StringComparison::OrdinalIgnoreCase)) {

				return this->ViewDelimiter;

			}

			return GlobalVariables::DataLoggingCSVDelimiterString;

		}

		System::Void EnsureSeriesBuffers() {

			// One list of points per line of the graph (a line is added when the header line is read, and when a data set is added while the session runs)
			while (this->SeriesBuffers->Count < this->LogChart->Series->Count) { this->SeriesBuffers->Add(gcnew LogSeriesBuffer()); }

		}

		System::Void ProcessNewText(System::String^ NewText) {

			// Reads the complete lines of the newly read text: the values go to the point lists of the graph lines (SeriesBuffers), the newest lines are kept in
			// PendingLines for the table. Nothing is drawn here - ShowPendingData does that when the view has caught up with the file, so a long file is not drawn
			// again and again while it is read.
			System::String^ AllText = this->PartialLine + NewText;
			cli::array<System::String^>^ Lines = AllText->Split(gcnew cli::array<wchar_t>{ L'\n' });

			// The last element is an unfinished line (or empty) - keep it for the next read
			this->PartialLine = Lines[Lines->Length - 1];

			int CompleteLines = Lines->Length - 1;
			if (CompleteLines < 1) { return; }

			cli::array<System::String^>^ Delimiters = gcnew cli::array<System::String^>{ this->GetViewDelimiter() };
			cli::array<wchar_t>^ TrimChars = gcnew cli::array<wchar_t>{ L'\r' };

			int FirstIndex = 0;

			// The first line of a log file is the header line - create the columns of the table and the lines of the graph
			if (this->LogGrid->Columns->Count == 0) {

				cli::array<System::String^>^ HeaderParts = Lines[0]->TrimEnd(TrimChars)->Split(Delimiters, System::StringSplitOptions::None);

				for (int c = 0; c < HeaderParts->Length; c++) {

					System::String^ HeaderText = HeaderParts[c]->Trim();
					if (c == 1) { HeaderText = "Elapsed (h:mm:ss)"; }
					this->AddLogColumn(c, HeaderText);

				}

				FirstIndex = 1;

			}

			for (int i = FirstIndex; i < CompleteLines; i++) {

				System::String^ Line = Lines[i]->TrimEnd(TrimChars);
				if (Line->Length == 0) { continue; }

				// A note line (#column=name): the source of a column was changed while the session runs - show the new name, it is no sample
				if (Line->StartsWith("#")) { this->ApplySourceChangeNote(Line); continue; }

				cli::array<System::String^>^ Parts = Line->Split(Delimiters, System::StringSplitOptions::None);

				this->ViewedSampleRows++;
				this->EnsureSeriesBuffers();

				// The time of the sample (milliseconds in the file)
				double MilliSeconds = 0.0;
				bool HasTime = false;

				if (Parts->Length > 1) { HasTime = this->ParseNumber(Parts[1]->Trim(), MilliSeconds); }
				if (HasTime == true) { this->LastElapsedMilliSec = MilliSeconds; }

				// Add the values to the point lists of the graph lines
				if (HasTime == true) {

					double PointTime = this->ChartStartTime.AddMilliseconds(MilliSeconds).ToOADate();

					for (int c = 2; c < Parts->Length && (c - 2) < this->SeriesBuffers->Count; c++) {

						double PointValue = 0.0;
						if (this->ParseNumber(Parts[c]->Trim(), PointValue)) { this->SeriesBuffers[c - 2]->Add(PointTime, PointValue); }

					}

				}

				// Keep the line for the table - only the newest _DataLogView_MaxRows lines are kept (the row is made when the table is brought up to date)
				this->PendingLines->Enqueue(Line);
				if (this->PendingLines->Count > _DataLogView_MaxRows) { this->PendingLines->Dequeue(); }

			}

			// Keep the number of points of the graph lines limited (thin out, or remove, the oldest)
			for (int s = 0; s < this->SeriesBuffers->Count; s++) { this->SeriesBuffers[s]->LimitPoints(_DataLogView_MaxPlotPoints); }

			this->ViewHasNewData = true;

		}

		System::Void SyncChartSeries(int SeriesIndex) {

			// Brings one line of the graph up to date with its point list: new points are added at the end; when the list was thinned out the line is filled again
			if (SeriesIndex >= this->SeriesBuffers->Count || SeriesIndex >= this->LogChart->Series->Count) { return; }

			LogSeriesBuffer^ Buffer = this->SeriesBuffers[SeriesIndex];
			System::Windows::Forms::DataVisualization::Charting::Series^ LineSeries = this->LogChart->Series[SeriesIndex];
			int Count = Buffer->YValues->Count;

			if (Buffer->NeedsRebuild == true) {

				LineSeries->Points->Clear();

				for (int i = 0; i < Count; i++) { LineSeries->Points->AddXY(Buffer->XValues[i], Buffer->YValues[i]); }

				Buffer->NeedsRebuild = false;
				Buffer->RemovedFront = 0;
				Buffer->SyncedCount = Count;
				return;

			}

			// Oldest points that were removed from the list (_DataLogView_ThinOldPoints = 0)
			while (Buffer->RemovedFront > 0 && LineSeries->Points->Count > 0) {

				LineSeries->Points->RemoveAt(0);
				Buffer->RemovedFront--;

			}

			Buffer->RemovedFront = 0;

			for (int i = Buffer->SyncedCount; i < Count; i++) { LineSeries->Points->AddXY(Buffer->XValues[i], Buffer->YValues[i]); }

			Buffer->SyncedCount = Count;

		}

		System::Void ShowPendingData() {

			// Adds the collected newest lines to the table and brings the lines of the graph up to date (called when the view has caught up with the file)
			if (this->ViewHasNewData == false) { return; }
			this->ViewHasNewData = false;

			if (this->PendingLines->Count > 0) {

				cli::array<System::String^>^ Delimiters = gcnew cli::array<System::String^>{ this->GetViewDelimiter() };

				this->LogGrid->SuspendLayout();

				while (this->PendingLines->Count > 0) {

					System::String^ Line = this->PendingLines->Dequeue();
					cli::array<System::String^>^ Parts = Line->Split(Delimiters, System::StringSplitOptions::None);

					double MilliSeconds = 0.0;
					bool HasTime = false;

					if (Parts->Length > 1) { HasTime = this->ParseNumber(Parts[1]->Trim(), MilliSeconds); }

					cli::array<System::Object^>^ RowValues = gcnew cli::array<System::Object^>(this->LogGrid->Columns->Count);

					for (int c = 0; c < RowValues->Length; c++) {

						if (c < Parts->Length) { RowValues[c] = Parts[c]->Trim(); }
						else { RowValues[c] = ""; }

					}

					// Show the time column as hours:minutes:seconds
					if (HasTime == true && RowValues->Length > 1) { RowValues[1] = this->FormatDuration(MilliSeconds); }

					this->LogGrid->Rows->Add(RowValues);

				}

				// Keep the number of rows in the table limited
				if (this->LogGrid->Rows->Count > _DataLogView_MaxRows + _DataLogView_TrimBlockSize) {

					while (this->LogGrid->Rows->Count > _DataLogView_MaxRows) { this->LogGrid->Rows->RemoveAt(0); }

				}

				this->LogGrid->ResumeLayout();

			}

			for (int s = 0; s < this->SeriesBuffers->Count; s++) { this->SyncChartSeries(s); }

			// New samples (or thinned out points) can change the values at the cursors
			if (!System::Double::IsNaN(this->MeasureTimeA) || !System::Double::IsNaN(this->MeasureTimeB)) { this->UpdateMeasurement(); }

			// Time labels of the graph - with the day when the log is longer than about a day
			if (this->LastElapsedMilliSec > 20.0 * 3600000.0) { this->LogChart->ChartAreas[0]->AxisX->LabelStyle->Format = "dd HH:mm"; }
			else { this->LogChart->ChartAreas[0]->AxisX->LabelStyle->Format = "HH:mm:ss"; }

			// Scroll the table to the newest row
			if (this->AutoScrollCheckBox->Checked == true && this->LogGrid->Rows->Count > 0) {

				this->LogGrid->FirstDisplayedScrollingRowIndex = this->LogGrid->Rows->Count - 1;

			}

		}

		System::Void RefreshLogView() {

			System::String^ FileName = GlobalVariables::DataLoggingSessionFileNameString;

			// Not behind unless the reading below finds that the file is longer than what it could read now
			this->LoadingBehind = false;

			if (FileName != nullptr && FileName->Length > 0) {

				System::String^ FolderPath = (GlobalVariables::DataLoggingSessionFolderString != nullptr) ? GlobalVariables::DataLoggingSessionFolderString : GlobalVariables::LoggingCSVDefaultPath;
				System::String^ FullPath = System::IO::Path::Combine(FolderPath, FileName);

				// A new data logging session has its own file - start a new view
				if (this->ViewedFilePath == nullptr || !this->ViewedFilePath->Equals(FullPath)) {

					this->ResetView();
					this->ViewedFilePath = FullPath;

				}

				if (System::IO::File::Exists(FullPath)) {

					try {

						System::IO::FileStream^ Stream = gcnew System::IO::FileStream(FullPath, System::IO::FileMode::Open, System::IO::FileAccess::Read, System::IO::FileShare::ReadWrite);

						try {

							// The file is shorter than expected - start again from the beginning
							if (Stream->Length < this->ViewedFilePosition) { this->ResetView(); }

							// The file is read in blocks (never the whole file at once: a very long log file would need several times its size in memory, and more than 2 GB
							// does not fit in one array). At most _DataLogView_ReadTimeBudgetMs milliseconds are used per refresh - the rest is read at the next one.
							long long FileLength = Stream->Length;
							this->LoadTargetLength = FileLength;

							if (FileLength > this->ViewedFilePosition) {

								if (this->ReadBuffer == nullptr) {

									this->ReadBuffer = gcnew cli::array<System::Byte>(_DataLogView_ReadBlockBytes);
									this->CharBuffer = gcnew cli::array<wchar_t>(System::Text::Encoding::UTF8->GetMaxCharCount(_DataLogView_ReadBlockBytes));

								}

								Stream->Seek(this->ViewedFilePosition, System::IO::SeekOrigin::Begin);
								System::Diagnostics::Stopwatch^ Watch = System::Diagnostics::Stopwatch::StartNew();

								while (this->ViewedFilePosition < FileLength) {

									long long Left = FileLength - this->ViewedFilePosition;
									int ToRead = (Left > (long long)_DataLogView_ReadBlockBytes) ? _DataLogView_ReadBlockBytes : (int)Left;

									int BytesRead = Stream->Read(this->ReadBuffer, 0, ToRead);
									if (BytesRead <= 0) { break; }

									this->ViewedFilePosition = this->ViewedFilePosition + BytesRead;

									int CharCount = this->ViewDecoder->GetChars(this->ReadBuffer, 0, BytesRead, this->CharBuffer, 0);
									this->ProcessNewText(gcnew System::String(this->CharBuffer, 0, CharCount));

									if (Watch->ElapsedMilliseconds >= _DataLogView_ReadTimeBudgetMs) { break; }

								}

							}

							this->LoadingBehind = (this->ViewedFilePosition < FileLength);

						}
						finally {

							Stream->Close();

						}

					}
					catch (System::Exception^) { this->LoadingBehind = false; }

				}

			}

			// While the file is still being read, the table and the graph are not drawn (that is done once, when the view has caught up) and the window is
			// refreshed again soon, so a long file is read in a few seconds without blocking the program
			if (this->LoadingBehind == false) {

				this->ShowPendingData();
				this->ApplyLiveStyleChanges();
				this->UpdateAxisTitles();
				this->ApplyAxisSettings();

			}

			this->UpdateStatusText();

			int WantedInterval = (this->LoadingBehind == true) ? 50 : 1000;
			if (this->RefreshTimer->Interval != WantedInterval) { this->RefreshTimer->Interval = WantedInterval; }

		}

		// ---------------------------------- Notice when the window is opened ---------------------------------- //

		System::Void ShowDurationNotice() {

			// Shows a window with the current data logging session duration and where it can be changed
			if (this->Visible == false || this->HideDurationNotice == true) { this->NoticeShowing = false; return; }

			try {

				int Hours = (int)System::Decimal::ToDouble(GlobalVariables::GlobalDataLoggingDurationHourUpDown->Value);
				int Minutes = (int)System::Decimal::ToDouble(GlobalVariables::GlobalDataLoggingDurationMinuteUpDown->Value);
				int Seconds = (int)System::Decimal::ToDouble(GlobalVariables::GlobalDataLoggingDurationSecondsUpDown->Value);
				double IntervalSec = System::Decimal::ToDouble(GlobalVariables::GlobalDataLoggingIntervalUpDown->Value);
				double TotalSec = (double)Hours * 3600.0 + (double)Minutes * 60.0 + (double)Seconds;

				System::Text::StringBuilder^ NoticeText = gcnew System::Text::StringBuilder();
				NoticeText->Append("The current data logging session is set to ");
				NoticeText->Append(System::Convert::ToString(Hours));
				NoticeText->Append(" hours, ");
				NoticeText->Append(System::Convert::ToString(Minutes));
				NoticeText->Append(" minutes and ");
				NoticeText->Append(System::Convert::ToString(Seconds));
				NoticeText->Append(" seconds, with one sample every ");
				NoticeText->Append(System::Convert::ToString(IntervalSec));
				NoticeText->Append(" seconds");

				if (IntervalSec > 0.0) {

					NoticeText->Append(" (about ");
					NoticeText->Append(System::Convert::ToString((int)(TotalSec / IntervalSec)));
					NoticeText->Append(" samples)");

				}

				NoticeText->Append(".");

				// A session with more samples than the graph keeps in full detail - the oldest part is thinned out (or scrolls off the screen), all samples stay in the file
				if (IntervalSec > 0.0 && TotalSec / IntervalSec > (double)_DataLogView_MaxPlotPoints) {

					NoticeText->Append(System::Environment::NewLine);
					NoticeText->Append(System::Environment::NewLine);
#if _DataLogView_ThinOldPoints == 1
					NoticeText->Append("Note: this is more than the ");
					NoticeText->Append(System::Convert::ToString(_DataLogView_MaxPlotPoints));
					NoticeText->Append(" points per line that the graph keeps. The oldest part of the graph is thinned out (of every few neighbouring points the lowest and the highest are kept, so peaks and dips stay visible), so the whole session stays on the graph. The table shows the newest ");
					NoticeText->Append(System::Convert::ToString(_DataLogView_MaxRows));
					NoticeText->Append(" rows. Every sample is saved in the log file.");
#else
					NoticeText->Append("Note: this is more than the newest ");
					NoticeText->Append(System::Convert::ToString(_DataLogView_MaxPlotPoints));
					NoticeText->Append(" samples that the graph shows (the table shows the newest ");
					NoticeText->Append(System::Convert::ToString(_DataLogView_MaxRows));
					NoticeText->Append("). The oldest samples scroll off the left side of the graph, but every sample is saved in the log file.");
#endif

				}

				NoticeText->Append(System::Environment::NewLine);
				NoticeText->Append(System::Environment::NewLine);
				NoticeText->Append("A running session stops by itself when this duration is over.");
				NoticeText->Append(System::Environment::NewLine);
				NoticeText->Append(System::Environment::NewLine);
				NoticeText->Append("To change the duration, open Settings, go to 'Data Logging Session Settings', and change the values. While a session runs, a new duration is used at once - it is the total length of the session, counted from the moment the session was started, and it must be longer than the time already logged. The sample interval can be changed while a session runs as well and is used from the next sample.");

				System::Windows::Forms::Form^ NoticeWindow = gcnew System::Windows::Forms::Form();
				NoticeWindow->Text = "Data Log";
				NoticeWindow->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
				NoticeWindow->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
				NoticeWindow->MinimizeBox = false;
				NoticeWindow->MaximizeBox = false;
				NoticeWindow->ShowInTaskbar = false;
				NoticeWindow->BackColor = System::Drawing::Color::White;
				NoticeWindow->ClientSize = System::Drawing::Size(520, 290);

				System::Windows::Forms::Label^ NoticeLabel = gcnew System::Windows::Forms::Label();
				NoticeLabel->AutoSize = false;
				NoticeLabel->Location = System::Drawing::Point(20, 18);
				NoticeLabel->Size = System::Drawing::Size(480, 190);
				NoticeLabel->Font = gcnew System::Drawing::Font("Segoe UI", 10);
				NoticeLabel->ForeColor = System::Drawing::Color::Black;
				NoticeLabel->Text = NoticeText->ToString();
				NoticeWindow->Controls->Add(NoticeLabel);

				System::Windows::Forms::CheckBox^ NoticeCheckBox = gcnew System::Windows::Forms::CheckBox();
				NoticeCheckBox->Text = "Do not show this message again (until the program is restarted)";
				NoticeCheckBox->AutoSize = true;
				NoticeCheckBox->Location = System::Drawing::Point(20, 220);
				NoticeCheckBox->ForeColor = System::Drawing::Color::Black;
				NoticeWindow->Controls->Add(NoticeCheckBox);

				System::Windows::Forms::Button^ NoticeOkButton = gcnew System::Windows::Forms::Button();
				NoticeOkButton->Text = "OK";
				NoticeOkButton->DialogResult = System::Windows::Forms::DialogResult::OK;
				NoticeOkButton->Location = System::Drawing::Point(205, 248);
				NoticeOkButton->Size = System::Drawing::Size(110, 30);
				NoticeWindow->Controls->Add(NoticeOkButton);
				NoticeWindow->AcceptButton = NoticeOkButton;

				NoticeWindow->ShowDialog(this);

				if (NoticeCheckBox->Checked == true) { this->HideDurationNotice = true; }

			}
			catch (System::Exception^) {}

			this->NoticeShowing = false;

		}

		// ---------------------------------- Event callbacks ---------------------------------- //

		System::Void RefreshTimer_Tick(System::Object^ sender, System::EventArgs^ e) {

			this->RefreshLogView();

			// Safety net: the zoom indicator also follows zoom resets that come from other routines (range lock / unlock)
			this->UpdateZoomIndicator();

		}

		System::Void DataLogGUI_VisibleChanged(System::Object^ sender, System::EventArgs^ e) {

			// Only refresh while the window is visible
			this->RefreshTimer->Enabled = this->Visible;

			// The first time the window is shown - the table gets a height of about 220 pixels, the graph gets the rest
			if (this->Visible == true && this->SplitDistanceSet == false) {

				this->SplitDistanceSet = true;

				try {

					int GraphHeight = this->ViewSplit->Height - 220 - this->ViewSplit->SplitterWidth;
					if (GraphHeight > 120) { this->ViewSplit->SplitterDistance = GraphHeight; }

				}
				catch (System::Exception^) {}

			}

			if (this->Visible == true) { this->RefreshLogView(); }

			// A recovered session file was opened (crash recovery): the notice about the duration of the next data logging session is not shown this time
			if (this->Visible == true && this->SuppressNoticeOnce == true) { this->SuppressNoticeOnce = false; }

			// Show the notice about the data logging session duration when the window is opened (after the window is shown)
			else if (this->Visible == true && this->HideDurationNotice == false && this->NoticeShowing == false) {

				this->NoticeShowing = true;

				try { this->BeginInvoke(gcnew System::Action(this, &DataLogGUI::ShowDurationNotice)); }
				catch (System::Exception^) { this->NoticeShowing = false; }

			}

		}

		System::Void DataLogGUI_FormClosing(System::Object^ sender, System::Windows::Forms::FormClosingEventArgs^ e) {

			// Update the associated form flags
			isDataLogFormOpen = false;
			isDataLogFormDocked = false;
			isDataLogFormUndocked = false;

			// When the form is closed - hide the form
			this->Hide();
			// Disable "disposing" of the form object
			e->Cancel = true;

		}

		System::Void StartLoggingButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Start data logging with the data logging session settings (the same as the data logging start in the plot menu)
			if (DataLoggingIsRunningFlag == false) { RMH_ThermalViewer_StartDataLogging(); }

			this->RefreshLogView();

		}

		System::Void StopLoggingButton_Click(System::Object^ sender, System::EventArgs^ e) {

			if (DataLoggingIsRunningFlag == true) {

				System::Windows::Forms::DialogResult Answer = System::Windows::Forms::MessageBox::Show(this,
					"Are you sure you want to stop logging?", "Stop Logging",
					System::Windows::Forms::MessageBoxButtons::YesNo, System::Windows::Forms::MessageBoxIcon::Question, System::Windows::Forms::MessageBoxDefaultButton::Button2);

				// "No": close the window and keep logging
				if (Answer != System::Windows::Forms::DialogResult::Yes) { return; }

				// The save / discard question follows when the logging thread has finished
				RMH_ThermalViewer_StopDataLogging();

			}

			this->RefreshLogView();

		}

		System::Void OpenFolderButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Shows the log file in the Windows file explorer
			try {

				if (this->ViewedFilePath != nullptr && System::IO::File::Exists(this->ViewedFilePath)) {

					System::Diagnostics::Process::Start("explorer.exe", gcnew System::String("/select,\"") + this->ViewedFilePath + gcnew System::String("\""));

				}
				else {

					System::Diagnostics::Process::Start("explorer.exe", GlobalVariables::LoggingCSVDefaultPath);

				}

			}
			catch (System::Exception^) {}

		}

		System::Void ClearViewButton_Click(System::Object^ sender, System::EventArgs^ e) {

			System::Windows::Forms::DialogResult Answer = System::Windows::Forms::MessageBox::Show(this,
				"Are you sure you want to clear the graph and the table?", "Clear Graph",
				System::Windows::Forms::MessageBoxButtons::YesNo, System::Windows::Forms::MessageBoxIcon::Question, System::Windows::Forms::MessageBoxDefaultButton::Button2);

			// "No": close the window and keep the view
			if (Answer != System::Windows::Forms::DialogResult::Yes) { return; }

			// Clears the table and the graph - only data that is logged from now on is shown (the file is not changed)
			this->LogGrid->Rows->Clear();
			this->ClearChartData();
			this->PendingLines->Clear();
			this->ViewDecoder->Reset();
			this->LoadingBehind = false;
			this->ViewHasNewData = false;
			this->ViewedSampleRows = 0;
			this->PartialLine = "";

			try {

				if (this->ViewedFilePath != nullptr) {

					System::IO::FileInfo^ Info = gcnew System::IO::FileInfo(this->ViewedFilePath);
					if (Info->Exists) { this->ViewedFilePosition = Info->Length; }

				}

			}
			catch (System::Exception^) {}

			this->UpdateStatusText();

		}

	};

}
