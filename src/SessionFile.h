
/*
 *  SessionFile.h
 *
 *  Helper routines for the session files (Session menu: Open Session / Save Session / Save Session As).
 *  Added in branch IRCAM-for-T2Splus_V2_2Dplots_expansion_33.
 *
 *  A session file is a plain text file (UTF-8, extension .ircamsession). The first line is the file header, then every
 *  line is "key=value". Line breaks, tabs and backslashes inside a value are written as \n, \t and \\ .
 *
 *  Key groups:
 *    meta.*          information about the file (format version, program version, time of saving)
 *    app.NNN         the application configuration lines (see RMH_Application_BuildSessionConfigLines)
 *    ui.*            the window and the page that was shown
 *    FormName.Ctrl   the value of a control of a form (check box, number box, drop-down list, text box, track bar)
 *    datalog.*       the Data Log graph settings, markers, cursors and the logged data (datalog.file.gz = GZip + Base64 of the log CSV file)
 *    lv.*            the temperature crosshairs, lines and ROIs of the live view
 *
 *  The routines here do not know the forms of the program: the main window gives them the forms to walk through.
 *
 */

#pragma once

#ifndef SessionFile_H
#define SessionFile_H

// ------------------------------ Dark Colors For The Session Menu ------------------------------ //

// Colors of the session menu (same dark look as the right-click menus of the program)
#define _SessionMenuColorProperty(PropertyName, R, G, B) \
	virtual property System::Drawing::Color PropertyName { System::Drawing::Color get() override { return System::Drawing::Color::FromArgb(R, G, B); } }

public ref class SessionMenuColors : public System::Windows::Forms::ProfessionalColorTable {

public:

	_SessionMenuColorProperty(MenuItemSelected, 70, 70, 70);
	_SessionMenuColorProperty(MenuItemBorder, 110, 110, 110);
	_SessionMenuColorProperty(MenuBorder, 90, 90, 90);
	_SessionMenuColorProperty(MenuItemSelectedGradientBegin, 70, 70, 70);
	_SessionMenuColorProperty(MenuItemSelectedGradientEnd, 70, 70, 70);
	_SessionMenuColorProperty(MenuItemPressedGradientBegin, 55, 55, 55);
	_SessionMenuColorProperty(MenuItemPressedGradientMiddle, 55, 55, 55);
	_SessionMenuColorProperty(MenuItemPressedGradientEnd, 55, 55, 55);
	_SessionMenuColorProperty(ToolStripDropDownBackground, 35, 35, 35);
	_SessionMenuColorProperty(ImageMarginGradientBegin, 35, 35, 35);
	_SessionMenuColorProperty(ImageMarginGradientMiddle, 35, 35, 35);
	_SessionMenuColorProperty(ImageMarginGradientEnd, 35, 35, 35);
	_SessionMenuColorProperty(MenuStripGradientBegin, 35, 35, 35);
	_SessionMenuColorProperty(MenuStripGradientEnd, 35, 35, 35);
	_SessionMenuColorProperty(SeparatorDark, 120, 120, 120);
	_SessionMenuColorProperty(SeparatorLight, 35, 35, 35);

};

// ------------------------------ Session File Routines ------------------------------ //

public ref class SessionFile abstract sealed {

public:

	// Header (first line) of a session file, and the version of the file format
	literal System::String^ FileHeader = "IRCAM-SESSION-FILE";
	literal System::String^ FormatVersion = "1";
	literal System::String^ FileExtension = ".ircamsession";

	// ---------------------------- Text Escaping ---------------------------- //

	static System::String^ Escape(System::String^ Text) {

		if (Text == nullptr) { return gcnew System::String(""); }

		System::Text::StringBuilder^ Result = gcnew System::Text::StringBuilder();

		for (int i = 0; i < Text->Length; i++) {

			wchar_t Character = Text[i];

			if (Character == L'\\') { Result->Append(L"\\\\"); }
			else if (Character == L'\n') { Result->Append(L"\\n"); }
			else if (Character == L'\r') { Result->Append(L"\\r"); }
			else if (Character == L'\t') { Result->Append(L"\\t"); }
			else { Result->Append(Character); }

		}

		return Result->ToString();

	}

	static System::String^ Unescape(System::String^ Text) {

		System::Text::StringBuilder^ Result = gcnew System::Text::StringBuilder();

		for (int i = 0; i < Text->Length; i++) {

			wchar_t Character = Text[i];

			if (Character == L'\\' && i + 1 < Text->Length) {

				i++;
				wchar_t Next = Text[i];

				if (Next == L'n') { Result->Append(L'\n'); }
				else if (Next == L'r') { Result->Append(L'\r'); }
				else if (Next == L't') { Result->Append(L'\t'); }
				else { Result->Append(Next); }

			}
			else { Result->Append(Character); }

		}

		return Result->ToString();

	}

	// ---------------------------- Value Conversion ---------------------------- //

	static System::String^ BoolText(bool Value) { return Value ? gcnew System::String("1") : gcnew System::String("0"); }

	static bool TextToBool(System::String^ Text) { return (Text != nullptr && (Text->Trim() == "1" || Text->Trim()->ToLower() == "true")); }

	static System::String^ DoubleText(double Value) { return Value.ToString("R", System::Globalization::CultureInfo::InvariantCulture); }

	static bool TextToDouble(System::String^ Text, double% Value) {

		double Parsed = 0.0;
		if (Text == nullptr) { return false; }
		if (System::Double::TryParse(Text, System::Globalization::NumberStyles::Float, System::Globalization::CultureInfo::InvariantCulture, Parsed) == false) { return false; }
		Value = Parsed;
		return true;

	}

	static bool TextToInt(System::String^ Text, int% Value) {

		int Parsed = 0;
		if (Text == nullptr) { return false; }
		if (System::Int32::TryParse(Text, System::Globalization::NumberStyles::Integer, System::Globalization::CultureInfo::InvariantCulture, Parsed) == false) { return false; }
		Value = Parsed;
		return true;

	}

	// ---------------------------- Controls Of A Form ---------------------------- //

	// Controls that are never saved or set (a session file must not change which camera is selected)
	static bool IsSkippedControl(System::String^ ControlName) {

		return (ControlName == "CameraSourceDropList");

	}

	// Writes the value of every check box, number box, drop-down list, track bar and text box under "Root" into "Values" (key = FormKey.ControlName).
	// Controls without a name are skipped, and forms inside the form (docked forms) are skipped - they are saved with their own key.
	static void CollectControls(System::Windows::Forms::Control^ Root, System::String^ FormKey, System::Collections::Generic::SortedDictionary<System::String^, System::String^>^ Values) {

		if (Root == nullptr) { return; }

		for (int i = 0; i < Root->Controls->Count; i++) {

			System::Windows::Forms::Control^ Child = Root->Controls[i];

			// Nested form: saved with its own key
			if (dynamic_cast<System::Windows::Forms::Form^>(Child) != nullptr) { continue; }

			bool IsValueControl = false;

			if (Child->Name != nullptr && Child->Name->Length > 0 && IsSkippedControl(Child->Name) == false) {

				System::String^ Key = FormKey + "." + Child->Name;

				System::Windows::Forms::CheckBox^ Check = dynamic_cast<System::Windows::Forms::CheckBox^>(Child);
				System::Windows::Forms::RadioButton^ Radio = dynamic_cast<System::Windows::Forms::RadioButton^>(Child);
				System::Windows::Forms::NumericUpDown^ Number = dynamic_cast<System::Windows::Forms::NumericUpDown^>(Child);
				System::Windows::Forms::ComboBox^ Combo = dynamic_cast<System::Windows::Forms::ComboBox^>(Child);
				System::Windows::Forms::TrackBar^ Track = dynamic_cast<System::Windows::Forms::TrackBar^>(Child);
				System::Windows::Forms::TextBox^ Text = dynamic_cast<System::Windows::Forms::TextBox^>(Child);

				if (Check != nullptr) { Values[Key] = BoolText(Check->Checked); IsValueControl = true; }
				else if (Radio != nullptr) { Values[Key] = BoolText(Radio->Checked); IsValueControl = true; }
				else if (Number != nullptr) { Values[Key] = Number->Value.ToString(System::Globalization::CultureInfo::InvariantCulture); IsValueControl = true; }
				else if (Combo != nullptr) { Values[Key] = Combo->SelectedIndex.ToString(System::Globalization::CultureInfo::InvariantCulture); IsValueControl = true; }
				else if (Track != nullptr) { Values[Key] = Track->Value.ToString(System::Globalization::CultureInfo::InvariantCulture); IsValueControl = true; }
				else if (Text != nullptr) { Values[Key] = Text->Text; IsValueControl = true; }

			}

			// A value control (number box, drop-down list, ...) has no controls of its own that are saved
			if (IsValueControl == false && Child->HasChildren == true) { CollectControls(Child, FormKey, Values); }

		}

	}

	// Sets the controls under "Root" to the values of "Values". A control that is disabled at that moment is skipped (for example the camera
	// settings while no camera is connected), also a control that has no value in the file. Setting a value runs the normal event of the control,
	// so the setting is applied in the program like when it is changed by hand. Returns the number of controls that were set; "Skipped" gets
	// the number of controls that had a value in the file but were not set.
	static int ApplyControls(System::Windows::Forms::Control^ Root, System::String^ FormKey, System::Collections::Generic::Dictionary<System::String^, System::String^>^ Values, int% Skipped) {

		int SetCount = 0;

		if (Root == nullptr) { return 0; }

		for (int i = 0; i < Root->Controls->Count; i++) {

			System::Windows::Forms::Control^ Child = Root->Controls[i];

			if (dynamic_cast<System::Windows::Forms::Form^>(Child) != nullptr) { continue; }

			bool IsValueControl = false;

			if (Child->Name != nullptr && Child->Name->Length > 0 && IsSkippedControl(Child->Name) == false) {

				System::String^ Key = FormKey + "." + Child->Name;
				System::String^ Value = nullptr;

				System::Windows::Forms::CheckBox^ Check = dynamic_cast<System::Windows::Forms::CheckBox^>(Child);
				System::Windows::Forms::RadioButton^ Radio = dynamic_cast<System::Windows::Forms::RadioButton^>(Child);
				System::Windows::Forms::NumericUpDown^ Number = dynamic_cast<System::Windows::Forms::NumericUpDown^>(Child);
				System::Windows::Forms::ComboBox^ Combo = dynamic_cast<System::Windows::Forms::ComboBox^>(Child);
				System::Windows::Forms::TrackBar^ Track = dynamic_cast<System::Windows::Forms::TrackBar^>(Child);
				System::Windows::Forms::TextBox^ Text = dynamic_cast<System::Windows::Forms::TextBox^>(Child);

				IsValueControl = (Check != nullptr || Radio != nullptr || Number != nullptr || Combo != nullptr || Track != nullptr || Text != nullptr);

				if (IsValueControl == true && Values->TryGetValue(Key, Value) == true) {

					if (Child->Enabled == false) { Skipped++; }
					else {

						try {

							if (Check != nullptr) {
								bool NewValue = TextToBool(Value);
								if (Check->Checked != NewValue) { Check->Checked = NewValue; }
							}
							else if (Radio != nullptr) {
								bool NewValue = TextToBool(Value);
								if (Radio->Checked != NewValue) { Radio->Checked = NewValue; }
							}
							else if (Number != nullptr) {
								double NewValue = 0.0;
								if (TextToDouble(Value, NewValue) == false) { throw gcnew System::FormatException(); }
								System::Decimal NewDecimal = System::Decimal(NewValue);
								if (NewDecimal < Number->Minimum) { NewDecimal = Number->Minimum; }
								if (NewDecimal > Number->Maximum) { NewDecimal = Number->Maximum; }
								if (Number->Value != NewDecimal) { Number->Value = NewDecimal; }
							}
							else if (Combo != nullptr) {
								int NewIndex = -1;
								if (TextToInt(Value, NewIndex) == false) { throw gcnew System::FormatException(); }
								if (NewIndex >= 0 && NewIndex < Combo->Items->Count && Combo->SelectedIndex != NewIndex) { Combo->SelectedIndex = NewIndex; }
							}
							else if (Track != nullptr) {
								int NewValue = 0;
								if (TextToInt(Value, NewValue) == false) { throw gcnew System::FormatException(); }
								if (NewValue < Track->Minimum) { NewValue = Track->Minimum; }
								if (NewValue > Track->Maximum) { NewValue = Track->Maximum; }
								if (Track->Value != NewValue) { Track->Value = NewValue; }
							}
							else if (Text != nullptr) {
								if (Text->Text != Value) { Text->Text = Value; }
							}

							SetCount++;

						}
						catch (System::Exception^) { Skipped++; }

					}

				}

			}

			if (IsValueControl == false && Child->HasChildren == true) { SetCount += ApplyControls(Child, FormKey, Values, Skipped); }

		}

		return SetCount;

	}

	// ---------------------------- Compressed Binary Data (Logged Data Of A Session) ---------------------------- //

	// The largest log file that is stored inside a session file (bigger files are only referred to by their path)
	literal long long MaxEmbeddedFileBytes = 52428800;

	// Compresses bytes (GZip) and returns them as text (Base64), so a log file can be stored on one line of the session file
	static System::String^ CompressToBase64(cli::array<System::Byte>^ Data) {

		System::IO::MemoryStream^ Output = gcnew System::IO::MemoryStream();
		System::IO::Compression::GZipStream^ Zip = gcnew System::IO::Compression::GZipStream(Output, System::IO::Compression::CompressionMode::Compress, true);
		Zip->Write(Data, 0, Data->Length);
		Zip->Close();

		return System::Convert::ToBase64String(Output->ToArray());

	}

	static cli::array<System::Byte>^ DecompressFromBase64(System::String^ Text) {

		cli::array<System::Byte>^ Packed = System::Convert::FromBase64String(Text);

		System::IO::MemoryStream^ Input = gcnew System::IO::MemoryStream(Packed);
		System::IO::Compression::GZipStream^ Zip = gcnew System::IO::Compression::GZipStream(Input, System::IO::Compression::CompressionMode::Decompress);
		System::IO::MemoryStream^ Output = gcnew System::IO::MemoryStream();

		cli::array<System::Byte>^ Block = gcnew cli::array<System::Byte>(65536);
		int BytesRead = 0;

		while ((BytesRead = Zip->Read(Block, 0, Block->Length)) > 0) { Output->Write(Block, 0, BytesRead); }

		Zip->Close();

		return Output->ToArray();

	}

	// Reads a whole file, also when another part of the program still writes to it (a running data logging session)
	static cli::array<System::Byte>^ ReadSharedFile(System::String^ FilePath) {

		System::IO::FileStream^ Stream = gcnew System::IO::FileStream(FilePath, System::IO::FileMode::Open, System::IO::FileAccess::Read, System::IO::FileShare::ReadWrite);

		try {

			cli::array<System::Byte>^ Data = gcnew cli::array<System::Byte>((int)Stream->Length);
			int Position = 0;

			while (Position < Data->Length) {
				int BytesRead = Stream->Read(Data, Position, Data->Length - Position);
				if (BytesRead <= 0) { break; }
				Position = Position + BytesRead;
			}

			return Data;

		}
		finally { Stream->Close(); }

	}

	// ---------------------------- Session File Reading And Writing ---------------------------- //

	// Writes the values to a session file. Returns an empty text when it worked, else the error text.
	static System::String^ WriteFile(System::String^ FilePath, System::Collections::Generic::SortedDictionary<System::String^, System::String^>^ Values) {

		try {

			System::Text::StringBuilder^ Content = gcnew System::Text::StringBuilder();

			Content->Append(FileHeader + " " + FormatVersion + "\r\n");

			for each (System::Collections::Generic::KeyValuePair<System::String^, System::String^> Pair in Values) {
				Content->Append(Pair.Key + "=" + Escape(Pair.Value) + "\r\n");
			}

			// Written to a temporary file first, so a failed save never destroys a good session file
			System::String^ TempPath = FilePath + ".tmp";
			System::IO::File::WriteAllText(TempPath, Content->ToString(), gcnew System::Text::UTF8Encoding(false));

			if (System::IO::File::Exists(FilePath) == true) { System::IO::File::Delete(FilePath); }
			System::IO::File::Move(TempPath, FilePath);

			return gcnew System::String("");

		}
		catch (System::Exception^ Ex) { return Ex->Message; }

	}

	// Reads a session file into "Values". Returns an empty text when it worked, else the error text.
	static System::String^ ReadFile(System::String^ FilePath, System::Collections::Generic::Dictionary<System::String^, System::String^>^ Values) {

		try {

			cli::array<System::String^>^ Lines = System::IO::File::ReadAllLines(FilePath, System::Text::Encoding::UTF8);

			if (Lines->Length == 0 || Lines[0]->StartsWith(FileHeader) == false) { return gcnew System::String("This is not an IRCAM session file."); }

			// File format version (second word of the header line)
			cli::array<System::String^>^ HeaderWords = Lines[0]->Trim()->Split(' ');
			int Version = 0;
			if (HeaderWords->Length < 2 || TextToInt(HeaderWords[1], Version) == false) { return gcnew System::String("The version of the session file could not be read."); }
			if (Version > System::Int32::Parse(FormatVersion)) { return gcnew System::String("The session file was saved by a newer version of the program (file format " + Version.ToString() + ")."); }

			for (int i = 1; i < Lines->Length; i++) {

				int EqualsPosition = Lines[i]->IndexOf('=');
				if (EqualsPosition <= 0) { continue; }

				Values[Lines[i]->Substring(0, EqualsPosition)] = Unescape(Lines[i]->Substring(EqualsPosition + 1));

			}

			return gcnew System::String("");

		}
		catch (System::Exception^ Ex) { return Ex->Message; }

	}

};

#endif /* SessionFile_H */
