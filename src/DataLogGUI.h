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

	// Maximum number of rows kept in the table, and of points kept per line in the graph, of this window
	// (the complete data is always in the CSV file)
	#define _DataLogView_MaxRows 20000
	#define _DataLogView_MaxPlotPoints 100000
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
		System::DateTime ChartStartTime;
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
			this->StyleButton(this->StartLoggingButton, "Start Logging", 10, 130);
			this->StartLoggingButton->Click += gcnew System::EventHandler(this, &DataLogGUI::StartLoggingButton_Click);
			TopPanel->Controls->Add(this->StartLoggingButton);

			this->StopLoggingButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->StopLoggingButton, "Stop Logging", 150, 130);
			this->StopLoggingButton->Click += gcnew System::EventHandler(this, &DataLogGUI::StopLoggingButton_Click);
			TopPanel->Controls->Add(this->StopLoggingButton);

			this->OpenFolderButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->OpenFolderButton, "Show Log File", 290, 130);
			this->OpenFolderButton->Click += gcnew System::EventHandler(this, &DataLogGUI::OpenFolderButton_Click);
			TopPanel->Controls->Add(this->OpenFolderButton);

			this->ClearViewButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->ClearViewButton, "Clear View", 430, 110);
			this->ClearViewButton->Click += gcnew System::EventHandler(this, &DataLogGUI::ClearViewButton_Click);
			TopPanel->Controls->Add(this->ClearViewButton);

			this->ResetZoomButton = gcnew System::Windows::Forms::Button();
			this->StyleButton(this->ResetZoomButton, "Reset Zoom", 550, 110);
			this->ResetZoomButton->Click += gcnew System::EventHandler(this, &DataLogGUI::ResetZoomButton_Click);
			TopPanel->Controls->Add(this->ResetZoomButton);

			this->AutoScrollCheckBox = gcnew System::Windows::Forms::CheckBox();
			this->AutoScrollCheckBox->Text = "Auto-scroll table to newest";
			this->AutoScrollCheckBox->Checked = true;
			this->AutoScrollCheckBox->AutoSize = true;
			this->AutoScrollCheckBox->Location = System::Drawing::Point(675, 43);
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

			this->LogChart->ChartAreas->Add(MainArea);

			System::Windows::Forms::DataVisualization::Charting::Legend^ SeriesLegend = gcnew System::Windows::Forms::DataVisualization::Charting::Legend("Legend");
			SeriesLegend->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);
			SeriesLegend->ForeColor = System::Drawing::Color::White;
			SeriesLegend->Docking = System::Windows::Forms::DataVisualization::Charting::Docking::Top;
			this->LogChart->Legends->Add(SeriesLegend);

			// The labels of the time axis show the elapsed time (also when the log is longer than 24 hours)
			this->LogChart->FormatNumber += gcnew System::EventHandler<System::Windows::Forms::DataVisualization::Charting::FormatNumberEventArgs^>(this, &DataLogGUI::LogChart_FormatNumber);

			this->UpdateAxisTitles();

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

		System::Void ResetZoomButton_Click(System::Object^ sender, System::EventArgs^ e) {

			this->LogChart->ChartAreas[0]->AxisX->ScaleView->ZoomReset(0);
			this->LogChart->ChartAreas[0]->AxisY->ScaleView->ZoomReset(0);

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
						LineSeries->Legend = "Legend";
						LineSeries->LegendText = HeaderText;
						LineSeries->Color = this->GetSeriesColor(c - 2);
						LineSeries->BorderWidth = 1;
						LineSeries->ToolTip = "#LEGENDTEXT: #VALY{F2}  (#VALX{HH:mm:ss})";
						this->LogChart->Series->Add(LineSeries);

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

				System::String^ FullPath = System::IO::Path::Combine(GlobalVariables::LoggingCSVDefaultPath, FileName);

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

			if (DataLoggingIsRunningFlag == true) { RMH_ThermalViewer_StopDataLogging(); }

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
