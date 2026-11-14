#pragma once

// Data Log window: shows, in near real time, the values that the Data Logging feature writes to its CSV file.
// The window is built in code (no designer / resource file). It follows the data logging session settings
// (data sets, interval and duration) because it only displays what the data logging feature collects.

#include "GlobalObjectsAndVariables.h"
#include "RMH_Application_ThermalViewer.h"

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
	// (_DataLogView_MaxPlotPoints) of this window are defined in RMH_Application_ThermalViewer.h
	// (the complete data is always in the CSV file)
	// Old rows and points are removed in blocks of this size (removing them one at a time from a long list is slow)
	#define _DataLogView_TrimBlockSize 1000

	public ref class DataLogGUI : public System::Windows::Forms::Form {

	public:

		DataLogGUI(void) {

			this->ViewedFilePath = nullptr;
			this->ViewedFilePosition = 0;
			this->PartialLine = "";
			this->ViewedSampleRows = 0;
			this->LastElapsedMilliSec = 0.0;
			this->ChartStartTime = System::DateTime(2000, 1, 1, 0, 0, 0);   // The time axis of the graph starts at 0:00:00 (elapsed time)
			this->SplitDistanceSet = false;
			this->NoticeShowing = false;
			this->HideDurationNotice = false;
			this->CursorLabelEnabled = false;
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
			this->YIntervalSetByProgram = false;

			this->BuildLayout();

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
		System::DateTime ChartStartTime;
		System::Windows::Forms::Label^ CursorLabel;     // Shows the temperature at the mouse cursor (switched on / off in the right click menu of the graph)
		bool CursorLabelEnabled;

		// Right click menu of the graph, and its settings
		System::Windows::Forms::ContextMenuStrip^ ChartContextMenu;
		System::Windows::Forms::ToolStripMenuItem^ CursorTempMenuItem;
		System::Windows::Forms::ToolStripMenuItem^ GraphTitleMenuItem;
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
		bool YIntervalSetByProgram;
		bool NoticeShowing;
		bool HideDurationNotice;
		System::Windows::Forms::Button^ StartLoggingButton;
		System::Windows::Forms::Button^ StopLoggingButton;
		System::Windows::Forms::Button^ OpenFolderButton;
		System::Windows::Forms::Button^ ClearViewButton;
		System::Windows::Forms::CheckBox^ AutoScrollCheckBox;
		System::Windows::Forms::Timer^ RefreshTimer;

		// State of the data that is shown
		System::String^ ViewedFilePath;
		long long ViewedFilePosition;
		System::String^ PartialLine;
		int ViewedSampleRows;
		double LastElapsedMilliSec;

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

		// Same as StyleButton, but the text and the outline (border) of the button get the given color
		// (used for Start Logging = green and Stop Logging = red)
		System::Void StyleColoredButton(System::Windows::Forms::Button^ ButtonObject, System::String^ ButtonText, int X, int Width, System::Drawing::Color AccentColor) {

			this->StyleButton(ButtonObject, ButtonText, X, Width);
			ButtonObject->ForeColor = AccentColor;
			ButtonObject->FlatAppearance->BorderColor = AccentColor;
			ButtonObject->FlatAppearance->BorderSize = 2;

		}

		System::Void BuildLayout() {

			this->Text = "Data Log";
			this->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);
			this->ForeColor = System::Drawing::Color::White;
			this->ClientSize = System::Drawing::Size(900, 600);
			this->Font = gcnew System::Drawing::Font("Arial", 9);

			// Top panel with the status text and the buttons
			System::Windows::Forms::Panel^ TopPanel = gcnew System::Windows::Forms::Panel();
			TopPanel->Dock = System::Windows::Forms::DockStyle::Top;
			TopPanel->Height = 74;
			TopPanel->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);

			this->StatusLabel = gcnew System::Windows::Forms::Label();
			this->StatusLabel->AutoSize = false;
			this->StatusLabel->Location = System::Drawing::Point(10, 8);
			this->StatusLabel->Size = System::Drawing::Size(870, 24);
			this->StatusLabel->Anchor = System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left | System::Windows::Forms::AnchorStyles::Right;
			this->StatusLabel->Font = gcnew System::Drawing::Font("Arial", 10, System::Drawing::FontStyle::Bold);
			this->StatusLabel->ForeColor = System::Drawing::Color::White;
			TopPanel->Controls->Add(this->StatusLabel);

			this->StartLoggingButton = gcnew System::Windows::Forms::Button();
			this->StyleColoredButton(this->StartLoggingButton, "Start Logging", 10, 120, System::Drawing::Color::FromArgb(0, 200, 70));
			this->StartLoggingButton->Click += gcnew System::EventHandler(this, &DataLogGUI::StartLoggingButton_Click);
			TopPanel->Controls->Add(this->StartLoggingButton);

			this->StopLoggingButton = gcnew System::Windows::Forms::Button();
			this->StyleColoredButton(this->StopLoggingButton, "Stop Logging", 135, 120, System::Drawing::Color::FromArgb(230, 40, 40));
			this->StopLoggingButton->Click += gcnew System::EventHandler(this, &DataLogGUI::StopLoggingButton_Click);
			TopPanel->Controls->Add(this->StopLoggingButton);

			this->OpenFolderButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->OpenFolderButton, "Show Log File", 260, 120);
			this->OpenFolderButton->Click += gcnew System::EventHandler(this, &DataLogGUI::OpenFolderButton_Click);
			TopPanel->Controls->Add(this->OpenFolderButton);

			this->ClearViewButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->ClearViewButton, "Clear View", 385, 100);
			this->ClearViewButton->Click += gcnew System::EventHandler(this, &DataLogGUI::ClearViewButton_Click);
			TopPanel->Controls->Add(this->ClearViewButton);

			this->ResetZoomButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->ResetZoomButton, "Reset Zoom", 490, 100);
			this->ResetZoomButton->Click += gcnew System::EventHandler(this, &DataLogGUI::ResetZoomButton_Click);
			TopPanel->Controls->Add(this->ResetZoomButton);

			this->ExportGraphButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->ExportGraphButton, "Export Graph", 595, 120);
			this->ExportGraphButton->Click += gcnew System::EventHandler(this, &DataLogGUI::ExportGraphButton_Click);
			TopPanel->Controls->Add(this->ExportGraphButton);

			this->AutoScrollCheckBox = gcnew System::Windows::Forms::CheckBox();
			this->AutoScrollCheckBox->Text = "Auto-scroll table to newest";
			this->AutoScrollCheckBox->Checked = true;
			this->AutoScrollCheckBox->AutoSize = true;
			this->AutoScrollCheckBox->Location = System::Drawing::Point(725, 43);
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

			this->LogChart->ChartAreas->Add(MainArea);

			// Title of the graph (top center, above the graph) - empty and hidden until a title is set in the right-click menu. It is part of an exported image.
			System::Windows::Forms::DataVisualization::Charting::Title^ GraphTitle = gcnew System::Windows::Forms::DataVisualization::Charting::Title();
			GraphTitle->Name = "GraphTitle";
			GraphTitle->Text = "";
			GraphTitle->Docking = System::Windows::Forms::DataVisualization::Charting::Docking::Top;
			GraphTitle->Alignment = System::Drawing::ContentAlignment::TopCenter;
			GraphTitle->Font = gcnew System::Drawing::Font("Arial", 12, System::Drawing::FontStyle::Bold);
			GraphTitle->ForeColor = System::Drawing::Color::White;
			GraphTitle->Visible = false;
			this->LogChart->Titles->Add(GraphTitle);

			// The graph has no legend of its own: the check boxes above the graph name the lines and show / hide them

			// The labels of the time axis show the elapsed time (also when the log is longer than 24 hours)
			this->LogChart->FormatNumber += gcnew System::EventHandler<System::Windows::Forms::DataVisualization::Charting::FormatNumberEventArgs^>(this, &DataLogGUI::LogChart_FormatNumber);

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

				System::Windows::Forms::DataVisualization::Charting::HitTestResult^ Hit = this->LogChart->HitTest(MouseX, MouseY);
				System::Windows::Forms::DataVisualization::Charting::ChartElementType Element = Hit->ChartElementType;

				// Only inside the plot area of the graph (not on the axes, the legend or outside the graph)
				if (Hit->ChartArea != nullptr && (Element == System::Windows::Forms::DataVisualization::Charting::ChartElementType::PlottingArea ||
					Element == System::Windows::Forms::DataVisualization::Charting::ChartElementType::DataPoint ||
					Element == System::Windows::Forms::DataVisualization::Charting::ChartElementType::Gridlines ||
					Element == System::Windows::Forms::DataVisualization::Charting::ChartElementType::StripLines)) {

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

			if (this->CursorLabelEnabled == true) { this->UpdateCursorLabel(e->X, e->Y); }

		}

		System::Void LogChart_MouseLeave(System::Object^ sender, System::EventArgs^ e) {

			// The label is hidden when the mouse leaves the graph (it shows again when the mouse comes back)
			this->CursorLabel->Visible = false;

		}

		System::Void LogChart_AxisViewChanged(System::Object^ sender, System::Windows::Forms::DataVisualization::Charting::ViewEventArgs^ e) {

			// The graph was zoomed or the zoom was reset - the ticks follow the visible range
			this->ApplyAxisSettings();

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

			this->GraphTitleMenuItem = gcnew System::Windows::Forms::ToolStripMenuItem();
			this->StyleMenuItem(this->GraphTitleMenuItem, "Set Graph Title...");
			this->GraphTitleMenuItem->Click += gcnew System::EventHandler(this, &DataLogGUI::GraphTitleMenuItem_Click);

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
			this->ChartContextMenu->Items->Add(this->GraphTitleMenuItem);
			this->ChartContextMenu->Items->Add(Separator1);
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
			this->CursorTempMenuItem->Checked = this->CursorLabelEnabled;
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

		System::Void GraphTitleMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			this->ShowGraphTitleWindow();

		}

		bool ShowGraphTitleWindow() {

			// Shows a window for entering the title of the graph (it starts with the title that is set now). An empty text removes the title.
			// Returns true when the window was closed with Set Title, false when it was cancelled.
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
			NoteLabel->Text = "Leave the text empty to remove the title.";
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

				System::String^ NewTitle = TitleBox->Text->Trim();
				GraphTitle->Text = NewTitle;
				GraphTitle->Visible = (NewTitle->Length > 0);
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

		System::Void ApplyAxisSettings() {

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

					Area->RecalculateAxesScale();

					double ViewMinimum = AxisX->Minimum;
					double ViewMaximum = AxisX->Maximum;

					if (AxisX->ScaleView->IsZoomed == true) { ViewMinimum = AxisX->ScaleView->ViewMinimum; ViewMaximum = AxisX->ScaleView->ViewMaximum; }

					// The time axis is in days (date / time values) - the ticks are set in seconds
					double RangeSeconds = (ViewMaximum - ViewMinimum) * 86400.0;

					if (!System::Double::IsNaN(RangeSeconds) && RangeSeconds > 0.0) {

						AxisX->IntervalType = System::Windows::Forms::DataVisualization::Charting::DateTimeIntervalType::Seconds;
						AxisX->Interval = System::Math::Max(RangeSeconds / (double)this->XTickCount, 0.001);
						this->XIntervalSetByProgram = true;

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

		}

		System::Void ExportGraphButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Saves the graph as an image (PNG, JPEG or BMP) - the graph as it is shown now (zoom, ticks, Y range), without the hidden lines,
			// with the names of the lines that are shown (in the color of the lines) in a strip above the graph
			if (this->LogChart->Series->Count == 0) {

				System::Windows::Forms::MessageBox::Show(this, "There is no graph to export yet. Start a data logging session first.", "Export Graph",
					System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Information);
				return;

			}

			// First the window for the title of the graph (it shows the title that is set now - keep it or change it). Cancel cancels the export.
			if (this->ShowGraphTitleWindow() == false) { return; }

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

			try {

				ChartStream = gcnew System::IO::MemoryStream();
				this->LogChart->SaveImage(ChartStream, System::Windows::Forms::DataVisualization::Charting::ChartImageFormat::Png);
				ChartImage = gcnew System::Drawing::Bitmap(ChartStream);

				// The lines that are shown
				System::Collections::Generic::List<System::String^>^ LineNames = gcnew System::Collections::Generic::List<System::String^>();
				System::Collections::Generic::List<System::Drawing::Color>^ LineColors = gcnew System::Collections::Generic::List<System::Drawing::Color>();

				for (int i = 0; i < this->LogChart->Series->Count; i++) {

					if (this->LogChart->Series[i]->Enabled == true) {

						LineNames->Add(this->LogChart->Series[i]->LegendText);
						LineColors->Add(this->LogChart->Series[i]->Color);

					}

				}

				System::Drawing::Font^ NameFont = gcnew System::Drawing::Font("Arial", 10);
				const int SquareSize = 10;
				const int RowHeight = 22;
				const float EdgeMargin = 10.0f;
				const float EntryGap = 18.0f;

				// First pass: how many rows of names fit in the width of the image
				System::Drawing::Graphics^ MeasureGraphics = System::Drawing::Graphics::FromImage(ChartImage);
				int Rows = 0;
				float PositionX = EdgeMargin;

				for (int i = 0; i < LineNames->Count; i++) {

					float EntryWidth = (float)SquareSize + 6.0f + MeasureGraphics->MeasureString(LineNames[i], NameFont).Width + EntryGap;
					if (Rows == 0 || (PositionX + EntryWidth > (float)ChartImage->Width - EdgeMargin && PositionX > EdgeMargin)) { Rows++; PositionX = EdgeMargin; }
					PositionX = PositionX + EntryWidth;

				}

				delete MeasureGraphics;

				int StripHeight = 0;
				if (Rows > 0) { StripHeight = Rows * RowHeight + 8; }

				OutputImage = gcnew System::Drawing::Bitmap(ChartImage->Width, ChartImage->Height + StripHeight);
				System::Drawing::Graphics^ OutputGraphics = System::Drawing::Graphics::FromImage(OutputImage);
				OutputGraphics->Clear(System::Drawing::Color::FromArgb(35, 35, 35));
				OutputGraphics->TextRenderingHint = System::Drawing::Text::TextRenderingHint::AntiAliasGridFit;

				// Second pass: draw the names above the graph
				int RowIndex = -1;
				PositionX = EdgeMargin;

				for (int i = 0; i < LineNames->Count; i++) {

					float EntryWidth = (float)SquareSize + 6.0f + OutputGraphics->MeasureString(LineNames[i], NameFont).Width + EntryGap;
					if (RowIndex < 0 || (PositionX + EntryWidth > (float)ChartImage->Width - EdgeMargin && PositionX > EdgeMargin)) { RowIndex++; PositionX = EdgeMargin; }

					float PositionY = 4.0f + (float)(RowIndex * RowHeight);
					System::Drawing::SolidBrush^ LineBrush = gcnew System::Drawing::SolidBrush(LineColors[i]);
					OutputGraphics->FillRectangle(LineBrush, PositionX, PositionY + 5.0f, (float)SquareSize, (float)SquareSize);
					OutputGraphics->DrawString(LineNames[i], NameFont, LineBrush, PositionX + (float)SquareSize + 6.0f, PositionY);
					delete LineBrush;

					PositionX = PositionX + EntryWidth;

				}

				OutputGraphics->DrawImage(ChartImage, 0, StripHeight, ChartImage->Width, ChartImage->Height);
				delete OutputGraphics;
				delete NameFont;

				OutputImage->Save(SaveDialog->FileName, SaveFormat);

			}
			catch (System::Exception^ ExportError) {

				System::Windows::Forms::MessageBox::Show(this, gcnew System::String("The graph could not be saved: ") + ExportError->Message, "Export Graph",
					System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Error);

			}
			finally {

				if (OutputImage != nullptr) { delete OutputImage; }
				if (ChartImage != nullptr) { delete ChartImage; }
				if (ChartStream != nullptr) { delete ChartStream; }

			}

		}

		System::Void ResetZoomButton_Click(System::Object^ sender, System::EventArgs^ e) {

			this->LogChart->ChartAreas[0]->AxisX->ScaleView->ZoomReset(0);
			this->LogChart->ChartAreas[0]->AxisY->ScaleView->ZoomReset(0);
			this->ApplyAxisSettings();

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

		}

		// ---------------------------------- Reading the log file ---------------------------------- //

		System::Void ProcessNewText(System::String^ NewText) {

			// Adds the complete lines of the newly read text to the table and to the graph
			System::String^ AllText = this->PartialLine + NewText;
			cli::array<System::String^>^ Lines = AllText->Split(gcnew cli::array<wchar_t>{ L'\n' });

			// The last element is an unfinished line (or empty) - keep it for the next read
			this->PartialLine = Lines[Lines->Length - 1];

			int CompleteLines = Lines->Length - 1;
			if (CompleteLines < 1) { return; }

			cli::array<System::String^>^ Delimiters = gcnew cli::array<System::String^>{ GlobalVariables::DataLoggingCSVDelimiterString };

			this->LogGrid->SuspendLayout();

			int FirstIndex = 0;

			// The first line of a log file is the header line - create the columns of the table and the lines of the graph
			if (this->LogGrid->Columns->Count == 0) {

				cli::array<System::String^>^ HeaderParts = Lines[0]->TrimEnd(gcnew cli::array<wchar_t>{ L'\r' })->Split(Delimiters, System::StringSplitOptions::None);

				for (int c = 0; c < HeaderParts->Length; c++) {

					System::String^ HeaderText = HeaderParts[c]->Trim();
					if (c == 1) { HeaderText = "Elapsed (h:mm:ss)"; }
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
						LineSeries->Color = this->GetSeriesColor(c - 2);
						LineSeries->BorderWidth = 1;
						LineSeries->ToolTip = "#LEGENDTEXT: #VALY{F2}  (#VALX{HH:mm:ss})";
						this->LogChart->Series->Add(LineSeries);

						// Check box of the line (all lines are shown at first)
						System::Windows::Forms::CheckBox^ LineCheck = gcnew System::Windows::Forms::CheckBox();
						LineCheck->Text = HeaderText;
						LineCheck->AutoSize = true;
						LineCheck->Checked = true;
						LineCheck->ForeColor = this->GetSeriesColor(c - 2);
						LineCheck->Tag = System::Convert::ToString(c - 2);
						LineCheck->Margin = System::Windows::Forms::Padding(4, 2, 10, 2);
						LineCheck->CheckedChanged += gcnew System::EventHandler(this, &DataLogGUI::LineCheck_CheckedChanged);
						this->LineChecksPanel->Controls->Add(LineCheck);

					}

				}

				FirstIndex = 1;

			}

			// Rows that would be removed again right away (a long log file that is opened) are skipped
			int SkipTableUntil = FirstIndex;
			if (CompleteLines - FirstIndex > _DataLogView_MaxRows) { SkipTableUntil = CompleteLines - _DataLogView_MaxRows; }
			int SkipPlotUntil = FirstIndex;
			if (CompleteLines - FirstIndex > _DataLogView_MaxPlotPoints) { SkipPlotUntil = CompleteLines - _DataLogView_MaxPlotPoints; }

			for (int i = FirstIndex; i < CompleteLines; i++) {

				System::String^ Line = Lines[i]->TrimEnd(gcnew cli::array<wchar_t>{ L'\r' });
				if (Line->Length == 0) { continue; }

				cli::array<System::String^>^ Parts = Line->Split(Delimiters, System::StringSplitOptions::None);

				this->ViewedSampleRows++;

				// The time of the sample (milliseconds in the file)
				double MilliSeconds = 0.0;
				bool HasTime = false;

				if (Parts->Length > 1) { HasTime = this->ParseNumber(Parts[1]->Trim(), MilliSeconds); }
				if (HasTime == true) { this->LastElapsedMilliSec = MilliSeconds; }

				// Add the values to the lines of the graph
				if (HasTime == true && i >= SkipPlotUntil) {

					double PointTime = this->ChartStartTime.AddMilliseconds(MilliSeconds).ToOADate();

					for (int c = 2; c < Parts->Length && (c - 2) < this->LogChart->Series->Count; c++) {

						double PointValue = 0.0;
						if (this->ParseNumber(Parts[c]->Trim(), PointValue)) { this->LogChart->Series[c - 2]->Points->AddXY(PointTime, PointValue); }

					}

				}

				// Add the row to the table
				if (i >= SkipTableUntil) {

					cli::array<System::Object^>^ RowValues = gcnew cli::array<System::Object^>(this->LogGrid->Columns->Count);

					for (int c = 0; c < RowValues->Length; c++) {

						if (c < Parts->Length) { RowValues[c] = Parts[c]->Trim(); }
						else { RowValues[c] = ""; }

					}

					// Show the time column as hours:minutes:seconds
					if (HasTime == true && RowValues->Length > 1) { RowValues[1] = this->FormatDuration(MilliSeconds); }

					this->LogGrid->Rows->Add(RowValues);

				}

			}

			// Keep the number of rows in the table and of points in the graph limited
			if (this->LogGrid->Rows->Count > _DataLogView_MaxRows + _DataLogView_TrimBlockSize) {

				while (this->LogGrid->Rows->Count > _DataLogView_MaxRows) { this->LogGrid->Rows->RemoveAt(0); }

			}

			for (int i = 0; i < this->LogChart->Series->Count; i++) {

				if (this->LogChart->Series[i]->Points->Count > _DataLogView_MaxPlotPoints + _DataLogView_TrimBlockSize) {

					while (this->LogChart->Series[i]->Points->Count > _DataLogView_MaxPlotPoints) { this->LogChart->Series[i]->Points->RemoveAt(0); }

				}

			}

			this->LogGrid->ResumeLayout();

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

							long long Remaining = Stream->Length - this->ViewedFilePosition;

							if (Remaining > 0) {

								Stream->Seek(this->ViewedFilePosition, System::IO::SeekOrigin::Begin);
								cli::array<System::Byte>^ Buffer = gcnew cli::array<System::Byte>((int)Remaining);
								int BytesRead = Stream->Read(Buffer, 0, (int)Remaining);
								this->ViewedFilePosition = this->ViewedFilePosition + BytesRead;

								this->ProcessNewText(System::Text::Encoding::UTF8->GetString(Buffer, 0, BytesRead));

							}

						}
						finally {

							Stream->Close();

						}

					}
					catch (System::Exception^) {}

				}

			}

			this->UpdateAxisTitles();
			this->ApplyAxisSettings();
			this->UpdateStatusText();

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

				// A session with more samples than the graph keeps - the oldest samples scroll off the screen (they stay in the file)
				if (IntervalSec > 0.0 && TotalSec / IntervalSec > (double)_DataLogView_MaxPlotPoints) {

					NoticeText->Append(System::Environment::NewLine);
					NoticeText->Append(System::Environment::NewLine);
					NoticeText->Append("Note: this is more than the newest ");
					NoticeText->Append(System::Convert::ToString(_DataLogView_MaxPlotPoints));
					NoticeText->Append(" samples that the graph shows (the table shows the newest ");
					NoticeText->Append(System::Convert::ToString(_DataLogView_MaxRows));
					NoticeText->Append("). The oldest samples scroll off the left side of the graph, but every sample is saved in the log file.");

				}

				NoticeText->Append(System::Environment::NewLine);
				NoticeText->Append(System::Environment::NewLine);
				NoticeText->Append("A running session stops by itself when this duration is over.");
				NoticeText->Append(System::Environment::NewLine);
				NoticeText->Append(System::Environment::NewLine);
				NoticeText->Append("To change the duration or the sample interval, open Settings, go to 'Data Logging Session Settings', and change the values. The new values are used the next time you press Start Logging.");

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

			// Show the notice about the data logging session duration when the window is opened (after the window is shown)
			if (this->Visible == true && this->HideDurationNotice == false && this->NoticeShowing == false) {

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
				"Are you sure you want to clear the view?", "Clear View",
				System::Windows::Forms::MessageBoxButtons::YesNo, System::Windows::Forms::MessageBoxIcon::Question, System::Windows::Forms::MessageBoxDefaultButton::Button2);

			// "No": close the window and keep the view
			if (Answer != System::Windows::Forms::DialogResult::Yes) { return; }

			// Clears the table and the graph - only data that is logged from now on is shown (the file is not changed)
			this->LogGrid->Rows->Clear();
			this->ClearChartData();
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
