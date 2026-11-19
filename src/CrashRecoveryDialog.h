// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Cyclotronic

#pragma once

// Crash recovery of data logging sessions.
//
// A data logging session is written to a temporary file in %TEMP%\IRCAMThermalViewer_DataLogging while it runs. When the session ends normally the
// user is asked whether it is saved or deleted, so no file is left behind. After a crash, a power loss or a killed process that question is never
// asked and the file stays in the temporary folder.
//
// At start-up the program looks in that folder (RMH_ThermalViewer_CheckForUnsavedDataLoggingSessions) and shows this window when it finds session
// files: the user can Save a file (to the default save path), Open it in the Data Log window, Delete it, or Decide Later.
//
// This file holds:
//   - RecoveredSessionInfo: what is known about one leftover session file (read from the file: delimiter, number of samples, logged time)
//   - CrashRecoveryDialog:  the window (built in code, no designer / resource file)

#include "GlobalObjectsAndVariables.h"
#include "RMH_Winforms_Library.h"
#include "RMH_MathConversions_Library.h"

// Class namespace
namespace IRCAMThermalViewer {

	// Associated namespaces
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	// ---------------------------------- One leftover session file ---------------------------------- //

	public ref class RecoveredSessionInfo {

	public:

		System::String^ FileName;
		System::String^ FullPath;
		System::String^ Delimiter;				// Data delimiter of the file (found in its header line)
		bool DelimiterKnown;					// False: the header line was not recognised, the delimiter of the program settings is used
		bool ReadFailed;						// True: the file could not be read (it is still offered to the user, never deleted without a question)
		long long SampleCount;					// Number of sample rows (the header line and the "#column=name" note lines are not counted)
		unsigned long long LastTimeMilliSec;	// Time(ms) of the last sample row
		long long SizeBytes;
		System::DateTime LastWriteTime;

		RecoveredSessionInfo() {

			this->FileName = "";
			this->FullPath = "";
			this->Delimiter = ",";
			this->DelimiterKnown = false;
			this->ReadFailed = false;
			this->SampleCount = 0;
			this->LastTimeMilliSec = 0;
			this->SizeBytes = 0;
			this->LastWriteTime = System::DateTime::MinValue;

		}

		// Reads a session file (it may be longer than 100 000 rows, so it is read line by line) and returns what is known about it.
		// The data delimiter is not stored anywhere else, so it is taken from the header line ("Sample<delimiter>Time(ms)<delimiter>...").
		static RecoveredSessionInfo^ Analyse(System::String^ SessionFilePath, System::String^ FallbackDelimiter) {

			RecoveredSessionInfo^ Info = gcnew RecoveredSessionInfo();
			Info->FullPath = SessionFilePath;
			Info->FileName = System::IO::Path::GetFileName(SessionFilePath);
			Info->Delimiter = FallbackDelimiter;

			try {

				System::IO::FileInfo^ FileInformation = gcnew System::IO::FileInfo(SessionFilePath);
				Info->SizeBytes = FileInformation->Length;
				Info->LastWriteTime = FileInformation->LastWriteTime;

				// FileShare ReadWrite: the file is not locked against other programs (a second instance of this program can have it open too)
				System::IO::FileStream^ Stream = gcnew System::IO::FileStream(SessionFilePath, System::IO::FileMode::Open, System::IO::FileAccess::Read, System::IO::FileShare::ReadWrite);

				try {

					System::IO::StreamReader^ Reader = gcnew System::IO::StreamReader(Stream, System::Text::Encoding::UTF8);

					// The first line is the header line
					System::String^ Line = Reader->ReadLine();

					if (Line != nullptr) {

						cli::array<System::String^>^ Candidates = gcnew cli::array<System::String^>{ ",", ";", ":", " ", "\t" };

						for (int c = 0; c < Candidates->Length; c++) {

							if (Line->StartsWith("Sample" + Candidates[c] + "Time(ms)", System::StringComparison::Ordinal)) {

								Info->Delimiter = Candidates[c];
								Info->DelimiterKnown = true;
								break;

							}

						}

					}

					// The rest of the lines: sample rows (note lines start with '#')
					System::String^ LastDataLine = nullptr;

					while (Line != nullptr && (Line = Reader->ReadLine()) != nullptr) {

						if (Line->Length == 0 || Line[0] == L'#') { continue; }

						Info->SampleCount = Info->SampleCount + 1;
						LastDataLine = Line;

					}

					// The second value of the last sample row is its time in milliseconds
					if (LastDataLine != nullptr) {

						cli::array<System::String^>^ Parts = LastDataLine->Split(gcnew cli::array<System::String^>{ Info->Delimiter }, System::StringSplitOptions::None);

						if (Parts->Length >= 2) {

							unsigned long long ParsedTime = 0;

							if (System::UInt64::TryParse(Parts[1]->Trim(), System::Globalization::NumberStyles::Integer, System::Globalization::CultureInfo::InvariantCulture, ParsedTime)) {

								Info->LastTimeMilliSec = ParsedTime;

							}

						}

					}

				}
				finally {

					Stream->Close();

				}

			}
			catch (System::Exception^) {

				Info->ReadFailed = true;

			}

			return Info;

		}

	};

	// ---------------------------------- The crash recovery window ---------------------------------- //

	public ref class CrashRecoveryDialog : public System::Windows::Forms::Form {

	public:

		// The session file that the user chose to open in the Data Log window (nullptr when no file was chosen)
		RecoveredSessionInfo^ SessionToOpen;

		CrashRecoveryDialog(System::Collections::Generic::List<RecoveredSessionInfo^>^ Sessions, System::String^ SaveFolderPath) {

			this->SessionToOpen = nullptr;
			this->SavePath = SaveFolderPath;

			this->BuildLayout(Sessions);

			// Enable dark mode for the title bar of the window
			RMH_Winforms_EnableTitleBarDarkMode(this->Handle);

		}

	protected:

		~CrashRecoveryDialog() {}

	private:

		System::Windows::Forms::ListView^ SessionList;
		System::Windows::Forms::Label^ IntroLabel;
		System::Windows::Forms::Button^ SaveButton;
		System::Windows::Forms::Button^ OpenButton;
		System::Windows::Forms::Button^ DeleteButton;
		System::Windows::Forms::Button^ LaterButton;
		System::String^ SavePath;			// Default save path of the data logging sessions (Settings)

		// ---------------------------------- Window layout ---------------------------------- //

		System::Windows::Forms::Button^ MakeButton(System::String^ ButtonText, int H) {

			System::Windows::Forms::Button^ ButtonObject = gcnew System::Windows::Forms::Button();
			ButtonObject->Text = ButtonText;
			ButtonObject->Size = System::Drawing::Size(H * 9, H * 2 + H / 2);
			ButtonObject->Margin = System::Windows::Forms::Padding(H / 4, H / 4, H / 4, 0);
			ButtonObject->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			ButtonObject->BackColor = System::Drawing::Color::FromArgb(60, 60, 60);
			ButtonObject->ForeColor = System::Drawing::Color::White;
			ButtonObject->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(100, 100, 100);

			return ButtonObject;

		}

		System::String^ FormatLoggedTime(unsigned long long MilliSec) {

			System::TimeSpan Logged = System::TimeSpan::FromMilliseconds((double)MilliSec);

			return System::Convert::ToString((int)Logged.TotalHours) + ":" + Logged.Minutes.ToString("00") + ":" + Logged.Seconds.ToString("00");

		}

		System::Void AddSessionToList(RecoveredSessionInfo^ Session) {

			long long Samples = Session->SampleCount;
			long long SizeKiloBytes = (Session->SizeBytes + 1023) / 1024;
			System::DateTime Written = Session->LastWriteTime;

			cli::array<System::String^>^ Cells = gcnew cli::array<System::String^>(5);
			Cells[0] = Session->FileName;
			Cells[1] = Written.ToString("yyyy-MM-dd HH:mm:ss", System::Globalization::CultureInfo::InvariantCulture);
			Cells[2] = (Session->ReadFailed == true) ? gcnew System::String("?") : Samples.ToString("N0");
			Cells[3] = (Session->ReadFailed == true) ? gcnew System::String("?") : this->FormatLoggedTime(Session->LastTimeMilliSec);
			Cells[4] = SizeKiloBytes.ToString("N0") + " KB";

			System::Windows::Forms::ListViewItem^ Item = gcnew System::Windows::Forms::ListViewItem(Cells);
			Item->Tag = Session;
			this->SessionList->Items->Add(Item);

		}

		System::Void BuildLayout(System::Collections::Generic::List<RecoveredSessionInfo^>^ Sessions) {

			this->Text = "Unsaved Data Logging Sessions Found";
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->ShowInTaskbar = false;
			this->ShowIcon = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);
			this->ForeColor = System::Drawing::Color::White;
			this->Font = gcnew System::Drawing::Font(L"Segoe UI", 9.0f);

			// All sizes follow the height of the text, so the window also fits a screen with a larger text scale
			int H = this->Font->Height;
			this->ClientSize = System::Drawing::Size(H * 52, H * 28);

			System::Windows::Forms::TableLayoutPanel^ Layout = gcnew System::Windows::Forms::TableLayoutPanel();
			Layout->Dock = System::Windows::Forms::DockStyle::Fill;
			Layout->ColumnCount = 1;
			Layout->RowCount = 4;
			Layout->Padding = System::Windows::Forms::Padding(H / 2);
			Layout->ColumnStyles->Add(gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 100.0f));
			Layout->RowStyles->Add(gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute, (float)(H * 4)));
			Layout->RowStyles->Add(gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 100.0f));
			Layout->RowStyles->Add(gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute, (float)(H * 6)));
			Layout->RowStyles->Add(gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute, (float)(H * 3 + H / 2)));

			// Row 0: what was found
			this->IntroLabel = gcnew System::Windows::Forms::Label();
			this->IntroLabel->Dock = System::Windows::Forms::DockStyle::Fill;
			this->IntroLabel->AutoSize = false;
			this->IntroLabel->TextAlign = System::Drawing::ContentAlignment::TopLeft;
			Layout->Controls->Add(this->IntroLabel, 0, 0);

			// Row 1: the list of session files
			this->SessionList = gcnew System::Windows::Forms::ListView();
			this->SessionList->Dock = System::Windows::Forms::DockStyle::Fill;
			this->SessionList->View = System::Windows::Forms::View::Details;
			this->SessionList->FullRowSelect = true;
			this->SessionList->MultiSelect = true;
			this->SessionList->HideSelection = false;
			this->SessionList->BackColor = System::Drawing::Color::FromArgb(45, 45, 45);
			this->SessionList->ForeColor = System::Drawing::Color::White;
			this->SessionList->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->SessionList->Columns->Add("Session File", H * 17);
			this->SessionList->Columns->Add("Last Written", H * 11);
			this->SessionList->Columns->Add("Samples", H * 6, System::Windows::Forms::HorizontalAlignment::Right);
			this->SessionList->Columns->Add("Logged Time", H * 7, System::Windows::Forms::HorizontalAlignment::Right);
			this->SessionList->Columns->Add("Size", H * 6, System::Windows::Forms::HorizontalAlignment::Right);
			this->SessionList->SelectedIndexChanged += gcnew System::EventHandler(this, &CrashRecoveryDialog::SessionList_SelectedIndexChanged);
			Layout->Controls->Add(this->SessionList, 0, 1);

			// Row 2: what the buttons do
			System::Windows::Forms::Label^ NoteLabel = gcnew System::Windows::Forms::Label();
			NoteLabel->Dock = System::Windows::Forms::DockStyle::Fill;
			NoteLabel->AutoSize = false;
			NoteLabel->TextAlign = System::Drawing::ContentAlignment::TopLeft;
			NoteLabel->ForeColor = System::Drawing::Color::FromArgb(200, 200, 200);
			NoteLabel->Text =
				"Save: the file is moved to the default save path (" + this->SavePath + "), under a new name if that name is already used there.\r\n" +
				"Open: shows the file in the Data Log window. It stays in the temporary folder until you save or delete it, so you are asked again at the next start-up.\r\n" +
				"Delete: the file is deleted permanently.\r\n" +
				"Decide Later: all files are kept and you are asked again at the next start-up.";
			Layout->Controls->Add(NoteLabel, 0, 2);

			// Row 3: the buttons (the first button that is added is the one on the right)
			System::Windows::Forms::FlowLayoutPanel^ ButtonPanel = gcnew System::Windows::Forms::FlowLayoutPanel();
			ButtonPanel->Dock = System::Windows::Forms::DockStyle::Fill;
			ButtonPanel->FlowDirection = System::Windows::Forms::FlowDirection::RightToLeft;
			ButtonPanel->WrapContents = false;

			this->LaterButton = this->MakeButton("Decide Later", H);
			this->DeleteButton = this->MakeButton("Delete", H);
			this->OpenButton = this->MakeButton("Open", H);
			this->SaveButton = this->MakeButton("Save", H);

			this->LaterButton->Click += gcnew System::EventHandler(this, &CrashRecoveryDialog::LaterButton_Click);
			this->DeleteButton->Click += gcnew System::EventHandler(this, &CrashRecoveryDialog::DeleteButton_Click);
			this->OpenButton->Click += gcnew System::EventHandler(this, &CrashRecoveryDialog::OpenButton_Click);
			this->SaveButton->Click += gcnew System::EventHandler(this, &CrashRecoveryDialog::SaveButton_Click);

			ButtonPanel->Controls->Add(this->LaterButton);
			ButtonPanel->Controls->Add(this->DeleteButton);
			ButtonPanel->Controls->Add(this->OpenButton);
			ButtonPanel->Controls->Add(this->SaveButton);
			Layout->Controls->Add(ButtonPanel, 0, 3);

			this->Controls->Add(Layout);

			// Fill the list (newest file first, see the scan routine)
			for (int i = 0; i < Sessions->Count; i++) { this->AddSessionToList(Sessions[i]); }

			this->UpdateIntroText();

			if (this->SessionList->Items->Count > 0) { this->SessionList->Items[0]->Selected = true; }

			this->UpdateButtonStates();

		}

		System::Void UpdateIntroText() {

			int Count = this->SessionList->Items->Count;

			this->IntroLabel->Text =
				"Unsaved data logging session files were found (" + Count.ToString() + "). They are left over from a data logging session that was running when "
				"the program was closed unexpectedly (a crash, a power loss or a killed process). Each file holds the samples that were written before that.\r\n"
				"Select a file and choose what to do with it:";

		}

		System::Void UpdateButtonStates() {

			int Selected = this->SessionList->SelectedItems->Count;

			this->SaveButton->Enabled = (Selected > 0);
			this->DeleteButton->Enabled = (Selected > 0);
			this->OpenButton->Enabled = (Selected == 1);

		}

		cli::array<System::Windows::Forms::ListViewItem^>^ GetSelectedItems() {

			cli::array<System::Windows::Forms::ListViewItem^>^ Items = gcnew cli::array<System::Windows::Forms::ListViewItem^>(this->SessionList->SelectedItems->Count);
			this->SessionList->SelectedItems->CopyTo(Items, 0);

			return Items;

		}

		// Called after files were saved or deleted: the window closes when no file is left
		System::Void AfterListChange() {

			if (this->SessionList->Items->Count == 0) {

				this->DialogResult = System::Windows::Forms::DialogResult::Cancel;
				return;

			}

			this->UpdateIntroText();

			if (this->SessionList->SelectedItems->Count == 0) { this->SessionList->Items[0]->Selected = true; }

			this->UpdateButtonStates();

		}

		// ---------------------------------- Actions ---------------------------------- //

		System::Void DeleteLockFile(System::String^ SessionFilePath) {

			// The lock file of a session that is not running any more (see RMH_ThermalViewer_StartDataLogging) is of no use
			try { System::IO::File::Delete(SessionFilePath + ".lock"); }
			catch (System::Exception^) {}

		}

		// Moves one session file to the default save path. Returns true when it was saved (the temporary file is then deleted).
		bool SaveSession(RecoveredSessionInfo^ Session) {

			try {

				System::IO::Directory::CreateDirectory(this->SavePath);

				// An existing file is never overwritten: the name gets "_recovered" (and a number) added
				System::String^ BaseName = System::IO::Path::GetFileNameWithoutExtension(Session->FileName);
				System::String^ Extension = System::IO::Path::GetExtension(Session->FileName);
				System::String^ TargetPath = System::IO::Path::Combine(this->SavePath, Session->FileName);
				int Counter = 1;

				while (System::IO::File::Exists(TargetPath)) {

					System::String^ Suffix = "_recovered";
					if (Counter > 1) { Suffix = Suffix + Counter.ToString(); }

					TargetPath = System::IO::Path::Combine(this->SavePath, BaseName + Suffix + Extension);
					Counter = Counter + 1;

				}

				// Copy and delete (works across drives). The temporary file is only deleted when the copy has the same size.
				System::IO::File::Copy(Session->FullPath, TargetPath, false);

				long long SavedLength = (gcnew System::IO::FileInfo(TargetPath))->Length;
				long long TempLength = (gcnew System::IO::FileInfo(Session->FullPath))->Length;

				if (SavedLength != TempLength) {

					throw gcnew System::IO::IOException("The saved file is not as long as the temporary file.");

				}

				System::IO::File::Delete(Session->FullPath);
				this->DeleteLockFile(Session->FullPath);

				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Recovered Data Logging Session Was Saved: " + RMH_Conversion_SystemStringToStdString(TargetPath), _StatusMessageType_Success);

				return true;

			}
			catch (System::Exception^ Ex) {

				// The temporary file is kept so no data is lost
				System::Windows::Forms::MessageBox::Show(this, "The data logging session could not be saved:\n" + Ex->Message + "\n\nThe data is kept in the temporary file:\n" + Session->FullPath,
					"Save Data Logging Session", System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Error);

				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Recovered Data Logging Session Could Not Be Saved!", _StatusMessageType_Warning);

				return false;

			}

		}

		System::Void SessionList_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {

			this->UpdateButtonStates();

		}

		System::Void SaveButton_Click(System::Object^ sender, System::EventArgs^ e) {

			cli::array<System::Windows::Forms::ListViewItem^>^ Items = this->GetSelectedItems();

			for (int i = 0; i < Items->Length; i++) {

				if (this->SaveSession((RecoveredSessionInfo^)Items[i]->Tag) == true) { this->SessionList->Items->Remove(Items[i]); }

			}

			this->AfterListChange();

		}

		System::Void OpenButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// One file at a time: the window closes and the file is shown in the Data Log window by the caller
			if (this->SessionList->SelectedItems->Count != 1) { return; }

			this->SessionToOpen = (RecoveredSessionInfo^)this->SessionList->SelectedItems[0]->Tag;
			this->DialogResult = System::Windows::Forms::DialogResult::OK;

		}

		System::Void DeleteButton_Click(System::Object^ sender, System::EventArgs^ e) {

			cli::array<System::Windows::Forms::ListViewItem^>^ Items = this->GetSelectedItems();
			if (Items->Length == 0) { return; }

			int NumberOfItems = Items->Length;
			System::String^ Question;

			if (NumberOfItems == 1) { Question = "Delete this data logging session file permanently?\n\n" + ((RecoveredSessionInfo^)Items[0]->Tag)->FileName; }
			else { Question = "Delete the " + NumberOfItems.ToString() + " selected data logging session files permanently?"; }

			// Default button is No: the data cannot be recovered after this
			System::Windows::Forms::DialogResult Answer = System::Windows::Forms::MessageBox::Show(this, Question + "\n\nThe data in the file is lost.", "Delete Data Logging Session",
				System::Windows::Forms::MessageBoxButtons::YesNo, System::Windows::Forms::MessageBoxIcon::Warning, System::Windows::Forms::MessageBoxDefaultButton::Button2);

			if (Answer != System::Windows::Forms::DialogResult::Yes) { return; }

			for (int i = 0; i < Items->Length; i++) {

				RecoveredSessionInfo^ Session = (RecoveredSessionInfo^)Items[i]->Tag;

				try {

					System::IO::File::Delete(Session->FullPath);
					this->DeleteLockFile(Session->FullPath);
					this->SessionList->Items->Remove(Items[i]);

					RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Unsaved Data Logging Session Was Deleted: " + RMH_Conversion_SystemStringToStdString(Session->FileName), _StatusMessageType_Normal);

				}
				catch (System::Exception^ Ex) {

					System::Windows::Forms::MessageBox::Show(this, "The file could not be deleted:\n" + Ex->Message + "\n\n" + Session->FullPath, "Delete Data Logging Session",
						System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Error);

				}

			}

			this->AfterListChange();

		}

		System::Void LaterButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// All files are kept
			this->DialogResult = System::Windows::Forms::DialogResult::Cancel;

		}

	};

}
