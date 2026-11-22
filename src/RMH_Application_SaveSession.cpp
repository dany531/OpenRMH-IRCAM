
/*
 *  RMH_Application_SaveSession.h
 *
 *  Author: Rune Mark Hansen
 *  Date: December 2022
 *
 */

// Included libraries
#include "RMH_Application_SaveSession.h"
#include "RMH_MathConversions_Library.h"
#include "RMH_Winforms_Library.h"
#include <algorithm>
#include <iostream>
#include <vector>

// Included resources
#include "GlobalObjectsAndVariables.h"
#include "RMH_Application_Information.h"
#include "RMH_2DPlotDataSetSources_Resources.h"

// Saved application session data file line length
#define _SavedSessionCSVFileLineLength          79 + 1 + 1 // Index number + top header + bottom header 

// ------------ Routines For Handling Saved Application Session Parameters ------------- //

std::vector<std::string> RMH_Application_BuildSessionConfigLines() {

	// This routine returns the application session configuration as a list of text lines (the layout of the saved session CSV file).
	// It is used by the automatic "last session" file and by the session files (Session menu: Save / Save As)

	std::vector<std::string> SavedSessionDescription = {

		// File start header
		"- IRCAM Thermal Viewer: Application Configuration Data -",

		// ----------------------- Selected Thermal Camera ------------------------ //

		// Selected thermal camera index - row index 1
		RMH_Conversion_IntToStdString(SelectedThermalCameraIndex),

		// ---------------------------- Color Palette ---------------------------- //

		// Selected color palette index - row index 2
		RMH_Conversion_IntToStdString(SelectedColorPaletteIndex),

		// -------------------- Temperature Correction Value --------------------- //

		// Save the set temperature correction value - row index 3
		RMH_Conversion_FloatToStdString(SavedTempCorrectionSetting, 5),

		// ---------------------- Aspect Ratio Setting ----------------------- //

		// Save the aspect ratio setting of the session - row index 4
		RMH_Conversion_SystemStringToStdString(FixedLiveViewAspectRatio.ToString()),

		// ------------------------- Dual Color Palette -------------------------- //

		// Selected color palette index - row index 5
		RMH_Conversion_IntToStdString(SelectedDualColorPaletteIndex),

		// ---------------------- Enhanced Resolution Flag ----------------------- //

		// Save the enhanced resolution setting of the session - row index 6
		RMH_Conversion_SystemStringToStdString(EnhancedResEnableFlag.ToString()),

		// ------------------------- SnapShot Save Path -------------------------- //

		// Save the snapshot file path string of the session - row index 7
		RMH_Conversion_SystemStringToStdString(GlobalVariables::SnapShotDefaultPath),

		// ------------------------ Data Logging Save Path ----------------------- //

		// Save the data logging file path string of the session - row index 8
		RMH_Conversion_SystemStringToStdString(GlobalVariables::LoggingCSVDefaultPath),

		// ------------------ Temperature 2D Plot Settings ------------------- //

		// Save the line colors of the 2D plot data sets
		RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsR[_2DPlotDataSet_1].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsG[_2DPlotDataSet_1].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsB[_2DPlotDataSet_1].ToString()),    // Index 9 - 11
		RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsR[_2DPlotDataSet_2].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsG[_2DPlotDataSet_2].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsB[_2DPlotDataSet_2].ToString()),    // Index 12 - 14
		RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsR[_2DPlotDataSet_3].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsG[_2DPlotDataSet_3].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsB[_2DPlotDataSet_3].ToString()),    // Index 15 - 17
		RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsR[_2DPlotDataSet_4].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsG[_2DPlotDataSet_4].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsB[_2DPlotDataSet_4].ToString()),    // Index 18 - 20
		RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsR[_2DPlotDataSet_5].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsG[_2DPlotDataSet_5].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsB[_2DPlotDataSet_5].ToString()),    // Index 21 - 23
		RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsR[_2DPlotDataSet_6].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsG[_2DPlotDataSet_6].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsB[_2DPlotDataSet_6].ToString()),    // Index 24 - 26
		RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsR[_2DPlotDataSet_7].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsG[_2DPlotDataSet_7].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsB[_2DPlotDataSet_7].ToString()),    // Index 27 - 29
		RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsR[_2DPlotDataSet_8].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsG[_2DPlotDataSet_8].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsB[_2DPlotDataSet_8].ToString()),    // Index 30 - 32
		RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsR[_2DPlotDataSet_9].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsG[_2DPlotDataSet_9].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsB[_2DPlotDataSet_9].ToString()),    // Index 33 - 35
		RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsR[_2DPlotDataSet_10].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsG[_2DPlotDataSet_10].ToString()), RMH_Conversion_SystemStringToStdString(Plot2DDataSetLineColorsB[_2DPlotDataSet_10].ToString()), // Index 36 - 38

		// ------------------- Live View Color Settings -------------------- //

		// Save the live view label background setting
		RMH_Conversion_SystemStringToStdString(EnableLabelBackgroundFlag.ToString()),		// Index 39

		// Save the live view label, background, crosshair etc. color data
		RMH_Conversion_SystemStringToStdString(CommonLabelColorR.ToString()),				// Index 40
		RMH_Conversion_SystemStringToStdString(CommonLabelColorG.ToString()),				// Index 41
		RMH_Conversion_SystemStringToStdString(CommonLabelColorB.ToString()),				// Index 42
		RMH_Conversion_SystemStringToStdString(CommonLabelBackgroundColorR.ToString()),		// Index 43
		RMH_Conversion_SystemStringToStdString(CommonLabelBackgroundColorG.ToString()),		// Index 44
		RMH_Conversion_SystemStringToStdString(CommonLabelBackgroundColorB.ToString()),		// Index 45
		RMH_Conversion_SystemStringToStdString(MaxCrosshairColorR.ToString()),				// Index 46
		RMH_Conversion_SystemStringToStdString(MaxCrosshairColorG.ToString()),				// Index 47
		RMH_Conversion_SystemStringToStdString(MaxCrosshairColorB.ToString()),				// Index 48
		RMH_Conversion_SystemStringToStdString(MinCrosshairColorR.ToString()),				// Index 49
		RMH_Conversion_SystemStringToStdString(MinCrosshairColorG.ToString()),				// Index 50
		RMH_Conversion_SystemStringToStdString(MinCrosshairColorB.ToString()),				// Index 51
		RMH_Conversion_SystemStringToStdString(CenterCrosshairColorR.ToString()),			// Index 52
		RMH_Conversion_SystemStringToStdString(CenterCrosshairColorG.ToString()),			// Index 53
		RMH_Conversion_SystemStringToStdString(CenterCrosshairColorB.ToString()),			// Index 54
		RMH_Conversion_SystemStringToStdString(ROISelectedColorR.ToString()),				// Index 55
		RMH_Conversion_SystemStringToStdString(ROISelectedColorG.ToString()),				// Index 56
		RMH_Conversion_SystemStringToStdString(ROISelectedColorB.ToString()),				// Index 57
		RMH_Conversion_SystemStringToStdString(ROIPassiveColorR.ToString()),				// Index 58
		RMH_Conversion_SystemStringToStdString(ROIPassiveColorG.ToString()),				// Index 59
		RMH_Conversion_SystemStringToStdString(ROIPassiveColorB.ToString()),				// Index 60
		RMH_Conversion_SystemStringToStdString(TempMeasCrosshairSelectedColorR.ToString()), // Index 61
		RMH_Conversion_SystemStringToStdString(TempMeasCrosshairSelectedColorG.ToString()), // Index 62
		RMH_Conversion_SystemStringToStdString(TempMeasCrosshairSelectedColorB.ToString()), // Index 63
		RMH_Conversion_SystemStringToStdString(TempMeasCrosshairPassiveColorR.ToString()),  // Index 64
		RMH_Conversion_SystemStringToStdString(TempMeasCrosshairPassiveColorG.ToString()),  // Index 65
		RMH_Conversion_SystemStringToStdString(TempMeasCrosshairPassiveColorB.ToString()),  // Index 66
		RMH_Conversion_SystemStringToStdString(TempLinesSelectedColorR.ToString()),			// Index 67
		RMH_Conversion_SystemStringToStdString(TempLinesSelectedColorG.ToString()),			// Index 68
		RMH_Conversion_SystemStringToStdString(TempLinesSelectedColorB.ToString()),			// Index 69
		RMH_Conversion_SystemStringToStdString(TempLinesPassiveColorR.ToString()),			// Index 70
		RMH_Conversion_SystemStringToStdString(TempLinesPassiveColorG.ToString()),			// Index 71
		RMH_Conversion_SystemStringToStdString(TempLinesPassiveColorB.ToString()),			// Index 72

		// ----------------------- ColorBar Settings ------------------------ //

		// Save the "full palette" range adjustment setting of the ColorBar
		RMH_Conversion_SystemStringToStdString(AdaptFullColorBarPaletteRangeFlag.ToString()), // Index 73

		// --------------------- Video Recording Save Path ---------------------- //

		// Save the snapshot file path string of the session - row index 74
		RMH_Conversion_SystemStringToStdString(GlobalVariables::RecordingDefaultPath), // Index 74

		// ------------------ Full Frame Data CSV Settings ------------------ //

		// Save the full frame CSV data delimiter index value of the session - row index 75
		RMH_Conversion_SystemStringToStdString(SelectedFullFrameTempCSVDataDelimiterIndex.ToString()), // Index 75

		// ------------------- Camera Info Pop-Up "Do Not Show" Flag ------------------- //

		// Save the camera info pop-up "do not show" flag of the session - row index 76
		RMH_Conversion_SystemStringToStdString(PopUpDialogDontShowFlag.ToString()),

		// --------------------- Data Logging Settings --------------------- //

		// Save the data logging CSV data delimiter index value of the session - row index 77
		RMH_Conversion_SystemStringToStdString(SelectedDataLoggingCSVDataDelimiterIndex.ToString()), // Index 77

		// ------------------- Miscellaneous Settings Parameters ------------------- //

		// Miscellaneous application settings parameters
		RMH_Conversion_SystemStringToStdString(UltraResolutionEnableFlag.ToString()),									// Index 78
		RMH_Conversion_IntToStdString((unsigned int)GlobalVariables::GlobalRecordingFrameRateNumericUpDown->Value),		// Index 79

		// ----------------------------------------------------------------------- //

		// File end header
		"- End Of Session Configuration - "
	};

	return SavedSessionDescription;

}

void RMH_Application_SaveLastSessionConfigToFile() {

	// This routine generates and saves the latest application session configuration in the "Documents" folder in Windows
	// values and setup in a CSV file at the executable file location. 
	// This can then be read at start-up to set the last active application configuration

	// Read the MyDocuments Windows path 
	System::String^ Path = System::Environment::GetFolderPath(System::Environment::SpecialFolder::MyDocuments);
	// Convert the path System::String to std::string
	std::string StdPathString = RMH_Conversion_SystemStringToStdString(Path);
	// Replace the character "\" with the character "\" in the path string
	std::replace(StdPathString.begin(), StdPathString.end(), '\\', '/');

	// ------------- Data To Be Saved In The Application Preset CSV File -------------- //

	// String with application setup data 
	std::vector<std::string> SavedSessionDescription = RMH_Application_BuildSessionConfigLines();

	// ------------------------------------------------------------------------------- //

	// Generate and save the application data in the preset CSV file
	RMH_Winforms_GenerateAndWriteCSVFile(StdPathString, Application_PresetFileName, SavedSessionDescription);

}

void RMH_Application_ApplySessionConfigLines(const std::vector<std::string>& Lines, bool LiveApply) {

	// This routine sets the application session configuration from a list of text lines (see RMH_Application_BuildSessionConfigLines).
	// LiveApply = false: used at start-up (all values are set before the forms are created).
	// LiveApply = true: used when a session file is opened while the program runs. The values that are set by a control of a form (camera,
	// color palettes, temperature correction, delimiters, recording frame rate) or by a button (aspect ratio, enhanced / ultra resolution,
	// full palette range) are not set here, because the session file restores those controls; only the values without a control are set

	 // ----------------------- Selected Thermal Camera ------------------------ //

	 // Read the saved camera source item index
	if (LiveApply == false) SelectedThermalCameraIndex = RMH_Conversion_StdStringToInt(Lines[1]);

	// ---------------------------- Color Palette ---------------------------- //

	// Read the saved color palette item index
	if (LiveApply == false) SelectedColorPaletteIndex = RMH_Conversion_StdStringToInt(Lines[2]);

	// -------------------- Temperature Correction Value --------------------- //

	// Set the saved temperature correction value to the camera config UpDown
	if (LiveApply == false) SavedTempCorrectionSetting = RMH_Conversion_StdStringToFloat(Lines[3]);

	// ---------------------- Aspect Ratio Setting ----------------------- //

	// Set the saved aspect ratio setting
	if (LiveApply == false) FixedLiveViewAspectRatio = RMH_Conversion_StdStringToBoolean(Lines[4]);

	// ------------------------- Dual Color Palette -------------------------- //

	// Read the saved dual color palette item index
	if (LiveApply == false) SelectedDualColorPaletteIndex = RMH_Conversion_StdStringToInt(Lines[5]);

	// ---------------------- Enhanced Resolution Flag ----------------------- //

	// Read the saved enhanced resolution mode setting
	if (LiveApply == false) EnhancedResEnableFlag = RMH_Conversion_StdStringToBoolean(Lines[6]);

	// ------------------------- SnapShot Save Path -------------------------- //

	// Read the saved snapshot file path string
	GlobalVariables::SnapShotDefaultPath = RMH_Conversion_StdStringToSystemString(Lines[7]);

	// ------------------------ Data Logging Save Path ----------------------- //

	// Read the saved data logging file path string
	GlobalVariables::LoggingCSVDefaultPath = RMH_Conversion_StdStringToSystemString(Lines[8]);

	// ------------------ Temperature 2D Plot Settings ------------------- //

	// Read the saved 2D plot line color data
	for (unsigned int i = 0, j = 0; i < _2DPlotMaxNumberOfDataSets; i++, j += 3) {

		// Write the saved 2D plot line color data to the global arrays
		Plot2DDataSetLineColorsR[i] = RMH_Conversion_StdStringToInt(Lines[9 + j]);
		Plot2DDataSetLineColorsG[i] = RMH_Conversion_StdStringToInt(Lines[10 + j]);
		Plot2DDataSetLineColorsB[i] = RMH_Conversion_StdStringToInt(Lines[11 + j]);

	}

	// ------------------- Live View Color Settings -------------------- //

	// Read the live view label background setting
	EnableLabelBackgroundFlag = RMH_Conversion_StdStringToBoolean(Lines[39]);

	// Read the live view label, background, crosshair etc. color data
	CommonLabelColorR = RMH_Conversion_StdStringToInt(Lines[40]);
	CommonLabelColorG = RMH_Conversion_StdStringToInt(Lines[41]);
	CommonLabelColorB = RMH_Conversion_StdStringToInt(Lines[42]);
	CommonLabelBackgroundColorR = RMH_Conversion_StdStringToInt(Lines[43]);
	CommonLabelBackgroundColorG = RMH_Conversion_StdStringToInt(Lines[44]);
	CommonLabelBackgroundColorB = RMH_Conversion_StdStringToInt(Lines[45]);
	MaxCrosshairColorR = RMH_Conversion_StdStringToInt(Lines[46]);
	MaxCrosshairColorG = RMH_Conversion_StdStringToInt(Lines[47]);
	MaxCrosshairColorB = RMH_Conversion_StdStringToInt(Lines[48]);
	MinCrosshairColorR = RMH_Conversion_StdStringToInt(Lines[49]);
	MinCrosshairColorG = RMH_Conversion_StdStringToInt(Lines[50]);
	MinCrosshairColorB = RMH_Conversion_StdStringToInt(Lines[51]);
	CenterCrosshairColorR = RMH_Conversion_StdStringToInt(Lines[52]);
	CenterCrosshairColorG = RMH_Conversion_StdStringToInt(Lines[53]);
	CenterCrosshairColorB = RMH_Conversion_StdStringToInt(Lines[54]);
	ROISelectedColorR = RMH_Conversion_StdStringToInt(Lines[55]);
	ROISelectedColorG = RMH_Conversion_StdStringToInt(Lines[56]);
	ROISelectedColorB = RMH_Conversion_StdStringToInt(Lines[57]);
	ROIPassiveColorR = RMH_Conversion_StdStringToInt(Lines[58]);
	ROIPassiveColorG = RMH_Conversion_StdStringToInt(Lines[59]);
	ROIPassiveColorB = RMH_Conversion_StdStringToInt(Lines[60]);
	TempMeasCrosshairSelectedColorR = RMH_Conversion_StdStringToInt(Lines[61]);
	TempMeasCrosshairSelectedColorG = RMH_Conversion_StdStringToInt(Lines[62]);
	TempMeasCrosshairSelectedColorB = RMH_Conversion_StdStringToInt(Lines[63]);
	TempMeasCrosshairPassiveColorR = RMH_Conversion_StdStringToInt(Lines[64]);
	TempMeasCrosshairPassiveColorG = RMH_Conversion_StdStringToInt(Lines[65]);
	TempMeasCrosshairPassiveColorB = RMH_Conversion_StdStringToInt(Lines[66]);
    TempLinesSelectedColorR = RMH_Conversion_StdStringToInt(Lines[67]);
	TempLinesSelectedColorG = RMH_Conversion_StdStringToInt(Lines[68]);
	TempLinesSelectedColorB = RMH_Conversion_StdStringToInt(Lines[69]);
	TempLinesPassiveColorR = RMH_Conversion_StdStringToInt(Lines[70]);
	TempLinesPassiveColorG = RMH_Conversion_StdStringToInt(Lines[71]);
	TempLinesPassiveColorB = RMH_Conversion_StdStringToInt(Lines[72]);

	// ----------------------- ColorBar Settings ------------------------ //

	// Read the "full palette" range adjustment setting of the ColorBar
	if (LiveApply == false) AdaptFullColorBarPaletteRangeFlag = RMH_Conversion_StdStringToBoolean(Lines[73]);

	// --------------------- Video Recording Save Path ---------------------- //

	// Read the saved snapshot file path string
	GlobalVariables::RecordingDefaultPath = RMH_Conversion_StdStringToSystemString(Lines[74]);

	// ------------------ Full Frame Data CSV Settings ------------------ //

	// Read the saved full frame CSV data delimiter index value
	if (LiveApply == false) SelectedFullFrameTempCSVDataDelimiterIndex = RMH_Conversion_StdStringToInt(Lines[75]);

	// ------------------- Camera Info Pop-Up "Do Not Show" Flag ------------------- //

	// Read the saved camera info pop-up "do not show" flag
	PopUpDialogDontShowFlag = RMH_Conversion_StdStringToBoolean(Lines[76]);

	// --------------------- Data Logging Settings --------------------- //

	// Read the saved data logging CSV data delimiter index value
	if (LiveApply == false) SelectedDataLoggingCSVDataDelimiterIndex = RMH_Conversion_StdStringToInt(Lines[77]);

	// ------------------- Miscellaneous Settings Parameters ------------------- //

	// Miscellaneous application settings parameters
	if (LiveApply == false) UltraResolutionEnableFlag = RMH_Conversion_StdStringToBoolean(Lines[78]);
	if (LiveApply == false) RecordingFrameRateSetValue = RMH_Conversion_StdStringToInt(Lines[79]);

	// ----------------------------------------------------------------------- //


}

void RMH_Application_SetSavedSessionConfigToApplication(System::Windows::Forms::RichTextBox^ GUIInfoTextArea) {

	// This routine reads the saved application session configuration file
	// and sets the relevant application values and objects at start-up 

	// Local file data object
	RMHWinformsLib::FileReadFormat PresetFile;

	// Read the MyDocuments Windows path 
	System::String^ Path = System::Environment::GetFolderPath(System::Environment::SpecialFolder::MyDocuments);
	// Convert the path System::String to std::string
	std::string StdPathString = RMH_Conversion_SystemStringToStdString(Path);
	// Replace the character "\" with the character "\" in the path string
	std::replace(StdPathString.begin(), StdPathString.end(), '\\', '/');

	// Read the data from the application preset file
	PresetFile = RMH_Winforms_ReadLinesFromCSVFile(StdPathString, Application_PresetFileName);

	// Handling of file errors
	try {

		// Was a valid configuration file read 
		if (PresetFile.FileReadSuccess == true) {

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GUIInfoTextArea, "Session Configuration File Found.", _StatusMessageType_Normal);

			// Check that the file is not empty and has the correct length
			if ((PresetFile.FileLineLength != 0 || PresetFile.FileZeroLengthFlag == true) && PresetFile.FileLineLength == _SavedSessionCSVFileLineLength) {

				// Write GUI status message
				RMH_Winforms_RichTextBox_WriteLine(GUIInfoTextArea, "Loading Last Session Configuration...", _StatusMessageType_Normal);

				/*
				 *  Setting of the file configuration data ->
				 */

				RMH_Application_ApplySessionConfigLines(PresetFile.FileStrings, false);

				// Write GUI status message
				RMH_Winforms_RichTextBox_WriteLine(GUIInfoTextArea, "Session Configuration Set.", _StatusMessageType_Normal);

			}
			else {
				// Write GUI status message
				RMH_Winforms_RichTextBox_WriteLine(GUIInfoTextArea, "Session Configuration File Is Empty Or Length Error!", _StatusMessageType_Normal);
			}

		}
		else {
			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GUIInfoTextArea, "No Session Configuration File Found!", _StatusMessageType_Normal);
		}

	}
	catch (System::Exception^ Ex) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GUIInfoTextArea, "Session Configuration File Format Error! ", _StatusMessageType_Error);

	}

}

// ------------------------------------------------------------------------------------------ //
