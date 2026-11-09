/*
 *  RMH_Application_ThermalViewer.c
 *
 *  Author: Rune Mark Hansen
 *  Date: april 2023
 *
 */

// Included libraries
#include "RMH_Application_ThermalViewer.h"
#include "RMH_MathConversions_Library.h"
#include "RMH_ImageProcessing_Library.h"
#include "RMH_ThermalCameraSupport_Library.h"
#include "RMH_AnalysisMode_Routines.h"
#include "RMH_OpenGL_Winforms.h"
#include "VideoPlayBackTools.h"

// Included resources
#include "GlobalObjectsAndVariables.h"
#include "RMH_SupportedIRCameras_Resources.h"
#include "RMH_2DPlotDataSetSources_Resources.h"
#include "RMH_EmissivityTable_Resources.h"
#include "RMH_TemperatureAlarms_Resources.h"
#include "RMH_FullFrameTempData_Resources.h"
#include "RMH_DataLoggingFeature_Resources.h"
#include "RMH_GeneralTriggerEvent_Resources.h"

// Global namespaces
using namespace System;
using namespace System::Windows::Forms;
using namespace System::Threading;
using namespace std;

// Static formatted color palette arrays
static unsigned short FormattedLiveViewPalette[3][16384];
static unsigned short FormattedDualLiveViewPalette[3][16384];

// ROI enable flag and render order arrays
static unsigned short ActiveROIRenderingOrder[_MaxNumberOfMovableRectangles] = { 0,0,0,0,0,0,0,0,0,0 };
static bool ActiveROIEnableFlags[_MaxNumberOfMovableRectangles] = { false,false,false,false,false,false,false,false,false,false };

// Temp measurement enable flag and render order arrays
static unsigned short ActiveTempMeasRenderingOrder[_MaxNumberOfMovableCrosshairs] = { 0,0,0,0,0,0,0,0,0,0 };
static bool ActiveTempMeasEnableFlags[_MaxNumberOfMovableCrosshairs] = { false,false,false,false,false,false,false,false,false,false };

// Temp measurement enable flag and render order arrays
static unsigned short ActiveTempLineRenderingOrder[_MaxNumberOfMovableLines] = { 0,0,0,0,0 };
static bool ActiveTempLineEnableFlags[_MaxNumberOfMovableLines] = { false,false,false,false,false };

// ----------------- Application Feature Enable Handling Routines ------------------ //

void RMH_Application_DisableMainGUIMenuButtons() {

	// This routine enables or disables a given application feature set.

	// Loop through the whole array of menu buttons
	for (unsigned int i = 0; i < GlobalVariables::MainGUILeftMenuButtons->Length; i++) {

		// Disable the main menu feature buttons
		GlobalVariables::MainGUILeftMenuButtons[i]->Enabled = false;

	}

}

void RMH_Application_EnableApplicationFeatures() {

	// This routine enables application features depending on whether the application is in trial or full feature mode

	// Enable the associated application feature set
	// Was a camera connected correctly, or is Recording/Snapshot Analysis mode selected
	if (IRCamera.ConnectedFlag == true || GlobalVariables::GlobalCameraSourceDropList->SelectedIndex == _SnapShotAnalysisMode || GlobalVariables::GlobalCameraSourceDropList->SelectedIndex == _RecordingAnalysisMode) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Genuine License Check Successful!", _StatusMessageType_Success);

		// Feature set enabled for the associated feature mode
		GlobalVariables::GlobalCursorTempTrackButton->Enabled = true;
		GlobalVariables::GlobalAddTempMeasButton->Enabled = true;
		GlobalVariables::GlobalAddROIMeasButton->Enabled = true;
		GlobalVariables::GlobalAddTempSpecLineButton->Enabled = true;
		GlobalVariables::GlobalShowLineHistButton->Enabled = true;
		GlobalVariables::GlobalImageSharpButton->Enabled = true;
		GlobalVariables::GlobalCalibrateCameraButton->Enabled = true;
		GlobalVariables::GlobalTempRangeButton->Enabled = true;
		GlobalVariables::GlobalFixedAspectRatioButton->Enabled = true;
		GlobalVariables::GlobalRecordingButton->Enabled = true;
		GlobalVariables::GlobalUltraResolutionButton->Enabled = true;
		GlobalVariables::GlobalPeriodicTimerTriggerButton->Enabled = true;
		GlobalVariables::GlobalEnhancedResButton->Enabled = true;

		// Enable the dual color palette ComboBox
		GlobalVariables::GlobalDualColorPaletteComboBox->Enabled = true;

		// Enable the save full-frame temperature data feature
		GlobalVariables::GlobalSaveTempFrameDataButton->Enabled = true;

		// Enable the snapshot feature
		GlobalVariables::GlobalSnapshotButton->Enabled = true;

		// Enable the live view dual color palette feature
		GlobalVariables::GlobalDualColorPaletteButton->Enabled = true;

		// Enable various camera configuration menu features
		GlobalVariables::GlobalAutoCalMenuButton->Enabled = true;
		GlobalVariables::GlobalSnapshotConfigMenuButton->Enabled = true;
		GlobalVariables::GlobalVideoRecordingMenuButton->Enabled = true;
		GlobalVariables::GlobalTempPlotDataSetSettingsMenuButton->Enabled = true;
		GlobalVariables::GlobalDataLoggingSettingsMenuButton->Enabled = true;
		GlobalVariables::GlobalTempAlarmsConfigMenuButton->Enabled = true;
		GlobalVariables::GlobalPeriodicTriggerConfigMenuButton->Enabled = true;

		// Enable the live view stream context menu features
		GlobalVariables::GlobalLiveViewMWRotationStripMenuItem->Enabled = true;
		GlobalVariables::GlobalRotateLiveViewCWStripMenuItem->Enabled = true;
		GlobalVariables::GlobalRotateLiveViewCCWStripMenuItem->Enabled = true;
		GlobalVariables::GlobalSetSharpStdDivToolStripMenuItem->Enabled = true;
		GlobalVariables::GlobalimageSharpeningStrengthToolStripMenuItem->Enabled = true;
		GlobalVariables::GlobalshowUnsharpMaskToolStripMenuItem->Enabled = true;
		GlobalVariables::GlobaldeleteLineToolStripMenuItem->Enabled = true;
		GlobalVariables::GlobaldeleteROIToolStripMenuItem->Enabled = true;
		GlobalVariables::GlobaldeleteTempLabelToolStripMenuItem->Enabled = true;
		GlobalVariables::GlobaldeleteAllToolStripMenuItem2->Enabled = true;
		GlobalVariables::GlobalchangeLineColorsToolStripMenuItem->Enabled = true;
		GlobalVariables::GlobalchangeROIColorsToolStripMenuItem->Enabled = true;
		GlobalVariables::GlobalLiveViewSplitViewToolStripMenuItem->Enabled = true;

		// Enable the live view colorbar context menu features
		GlobalVariables::GlobaltemperatureRangeToolStripMenuItem->Enabled = true;
		GlobalVariables::GlobalenableFullPaletteRangeAdjustmentToolStripMenuItem->Enabled = true;
		GlobalVariables::GlobaladjustDualPaletteRangeToolStripMenuItem->Enabled = true;

		// Loop through the whole array of menu buttons
		for (unsigned int i = 0; i < GlobalVariables::MainGUILeftMenuButtons->Length; i++) {

			// Enable the main menu feature buttons
			GlobalVariables::MainGUILeftMenuButtons[i]->Enabled = true;

		}

	}

}

// ----------------------- Camera Configuration Handling Routines ----------------------- //

void RMH_ThermalViewer_EnableCameraConfigurationControls(bool EnableState) {

	// This routine enables or disables the camera configuration GUI components

	// Enable the camera configuration NumericUpDowns
	for (unsigned int i = 0; i < (unsigned int)GlobalVariables::CameraConfigNumericUpDowns->Length; i++) {

		// Enable the camera configuration NumericUpDowns
		GlobalVariables::CameraConfigNumericUpDowns[i]->Enabled = EnableState;

	}

	// Enable the Read/Set/Recover camera configuration buttons
	GlobalVariables::GlobalReadCameraConfigButton->Enabled = EnableState;
	GlobalVariables::GlobalSetCameraConfigButton->Enabled = EnableState;
	GlobalVariables::GlobalRecoverDefaultCameraSettingsButton->Enabled = EnableState;

}

void RMH_ThermalViewer_ReadAndDisplayCameraConfigParameters() {

	// This routine reads and shows the internal camera configuration parameters read

	// Local variables
	bool ConfigurationValuesOKFlag[6] = { false, false, false, false, false, false };

	// Check whether a camera is connected
	if (IRCamera.ConnectedFlag == true) {

		// Read a single data frame from the thermal camera
		RMH_IRThermalCamera_ReadFrameRaw(&IRCameraFrameData[0], &VideoFrameSize);
		// Format the raw YUY2 data to a 16-bit thermal data array
		RMH_IRThermalCamera_ConvertYUY2To14BitThermalDataArray(&IRCamera, &IRCameraFrameData[0], &FrameThermalDataRaw[0]);
		// Read the frame metadata of the IR camera and calculate the internal IR sensor temperatures
		RMH_IRThermalCamera_ReadCalFrameMetaData(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

		// Read the internal configuration parameters of the camera
		RMH_IRThermalCamera_ReadCameraConfigParameters(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

		// Write the internal camera configuration parameters read to the camera configuration panel
		//RMH_Winforms_NumericUpDown_ChangeNumber(NumericUpDowns[0], IRCamera.TemperatureCorrectionSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor); - Not used
		ConfigurationValuesOKFlag[1] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[1], IRCamera.AmbientTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_AmbientTemperatureValue);
		ConfigurationValuesOKFlag[2] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[2], IRCamera.ReflectedTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_ReflectedTemperatureValue);
		ConfigurationValuesOKFlag[3] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[3], IRCamera.HumiditySetting, 1, 0, _IRThermalCameraDefault_SurroundingHumidityValue);
		ConfigurationValuesOKFlag[4] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[4], IRCamera.EmissivitySetting, 1, 0, _IRThermalCameraDefault_ObjectEmissivityValue);
		ConfigurationValuesOKFlag[5] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[5], IRCamera.DistanceSetting, 1, 0, _IRThermalCameraDefault_ObjectDistanceValue);

		// Check whether the configuration values were out of range
		if (ConfigurationValuesOKFlag[1] == false) {

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Reflected Temperature Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
			// Update the associated configuration value to the default value
			IRCamera.ReflectedTemperatureSetting = _IRThermalCameraDefault_ReflectedTemperatureValue;

		}
		if (ConfigurationValuesOKFlag[2] == false) {

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Ambient Temperature Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
			// Update the associated configuration value to the default value
			IRCamera.AmbientTemperatureSetting = _IRThermalCameraDefault_AmbientTemperatureValue;

		}
		if (ConfigurationValuesOKFlag[3] == false) {

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Humidity Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
			// Update the associated configuration value to the default value
			IRCamera.HumiditySetting = _IRThermalCameraDefault_SurroundingHumidityValue;

		}
		if (ConfigurationValuesOKFlag[4] == false) {

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Emissivity Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
			// Update the associated configuration value to the default value
			IRCamera.EmissivitySetting = _IRThermalCameraDefault_ObjectEmissivityValue;

		}
		if (ConfigurationValuesOKFlag[5] == false) {

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Distance Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
			// Update the associated configuration value to the default value
			IRCamera.DistanceSetting = _IRThermalCameraDefault_ObjectDistanceValue;

		}

		// Write GUI start message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "The Thermal Camera Configuration Has Been Read And Displayed.", _StatusMessageType_Success);

	}
	else {

		// Write GUI start message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Please Connect To A Thermal Camera Before Reading The Configuration!", _StatusMessageType_Normal);

	}

}

void RMH_ThermalViewer_ReadAndDisplayRAWVideoFileCameraConfigParameters() {

	// This routine reads and shows the internal camera configuration parameters read
	// These are for the video RAW file read in "Recording Analysis" mode

	// Write the internal camera configuration parameters read to the camera configuration panel
	RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[0], IRCamera.TemperatureCorrectionSetting, 1, 0, _IRThermalCameraDefault_TemperatureCorrectionValue);
	RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[1], IRCamera.AmbientTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_AmbientTemperatureValue);
	RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[2], IRCamera.ReflectedTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_ReflectedTemperatureValue);
	RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[3], IRCamera.HumiditySetting, 1, 0, _IRThermalCameraDefault_SurroundingHumidityValue);
	RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[4], IRCamera.EmissivitySetting, 1, 0, _IRThermalCameraDefault_ObjectEmissivityValue);
	RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[5], IRCamera.DistanceSetting, 1, 0, _IRThermalCameraDefault_ObjectDistanceValue);

	// Write GUI start message
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "The Thermal Camera Configuration Has Been Read From The Video File And Displayed In The Settings Menu", _StatusMessageType_Success);

}

void RMH_ThermalViewer_ReadAndDisplayRAWSnapShotFileCameraConfigParameters() {

	// This routine reads and shows the internal camera configuration parameters read
	// These are for the snapshot RAW file read in "Snapshot Analysis" mode

	// Write the internal camera configuration parameters read to the camera configuration panel
	RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[0], IRCamera.TemperatureCorrectionSetting, 1, 0, _IRThermalCameraDefault_TemperatureCorrectionValue);
	RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[1], IRCamera.AmbientTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_AmbientTemperatureValue);
	RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[2], IRCamera.ReflectedTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_ReflectedTemperatureValue);
	RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[3], IRCamera.HumiditySetting, 1, 0, _IRThermalCameraDefault_SurroundingHumidityValue);
	RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[4], IRCamera.EmissivitySetting, 1, 0, _IRThermalCameraDefault_ObjectEmissivityValue);
	RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[5], IRCamera.DistanceSetting, 1, 0, _IRThermalCameraDefault_ObjectDistanceValue);

	// Write GUI start message
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "The Thermal Camera Configuration Has Been Read From The SnapShot File And Displayed In The Settings Menu", _StatusMessageType_Success);

}

void RMH_ThermalViewer_SetCameraConfigParameters() {

	// This routine writes the configured configuration parameters to the camera

	// Check and compensate for the temperature unit setting
	switch (TempUnitState) {

		// Temperature unit: Celsius
		case 1:

			// Convert back to Celsius and update the camera configuration parameter
			IRCamera.AmbientTemperatureSetting = RMH_Conversion_SystemDecimalToFloat(GlobalVariables::CameraConfigNumericUpDowns[1]->Value);
			IRCamera.ReflectedTemperatureSetting = RMH_Conversion_SystemDecimalToFloat(GlobalVariables::CameraConfigNumericUpDowns[2]->Value);

		break;

		// Temperature unit: Fahrenheit
		case 2:

			// Convert back to Celsius and update the camera configuration parameter
			IRCamera.AmbientTemperatureSetting = 0.55556f * (RMH_Conversion_SystemDecimalToFloat(GlobalVariables::CameraConfigNumericUpDowns[1]->Value) - 32.0f);
			IRCamera.ReflectedTemperatureSetting = 0.55556f * (RMH_Conversion_SystemDecimalToFloat(GlobalVariables::CameraConfigNumericUpDowns[2]->Value) - 32.0f);

		break;

		// Temperature unit: Kelvin
		case 3:

			// Convert back to Celsius and update the camera configuration parameter
			IRCamera.AmbientTemperatureSetting = RMH_Conversion_SystemDecimalToFloat(GlobalVariables::CameraConfigNumericUpDowns[1]->Value) - 273.15f;
			IRCamera.ReflectedTemperatureSetting = RMH_Conversion_SystemDecimalToFloat(GlobalVariables::CameraConfigNumericUpDowns[2]->Value) - 273.15f;

		break;

	}

	// Update the camera configuration parameter in the "IRCamera" object from the associated GUI NumericUpDowns
	IRCamera.TemperatureCorrectionSetting = RMH_Conversion_SystemDecimalToFloat(GlobalVariables::CameraConfigNumericUpDowns[0]->Value);
	IRCamera.HumiditySetting = RMH_Conversion_SystemDecimalToFloat(GlobalVariables::CameraConfigNumericUpDowns[3]->Value);
	IRCamera.EmissivitySetting = RMH_Conversion_SystemDecimalToFloat(GlobalVariables::CameraConfigNumericUpDowns[4]->Value);
	IRCamera.DistanceSetting = RMH_Conversion_SystemDecimalToFloat(GlobalVariables::CameraConfigNumericUpDowns[5]->Value);

	// Write the configured camera configuration parameters from the "IRcamera" object to the internal memory of the camera.
	RMH_IRThermalCamera_SaveConfigParametersToCamera(&IRCamera, IRCamera.ThermalCameraSupportPool);

	// Write GUI start message
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "New Thermal Camera Configuration Has Been Set.", _StatusMessageType_Success);

	// Generate/update the temperature look-up table
	RMH_IRThermalCamera_GenerateThermoGrapicLookUpTable(&IRCamera, IRCamera.ThermalCameraSupportPool);

}

void RMH_ThermalViewer_SetCameraConfigUpDownRanges(float TemperatureUnitScaleFactor, float TemperatureUnitOffsetFactor) {

	// This routine sets the maximum and minimum ranges of the NumericUpDowns of the camera configuration panel
	// as a result of a change of the temperature unit setting.

	// Configure the temperature correction max/min NumericUpDown limits
	GlobalVariables::CameraConfigNumericUpDowns[0]->Maximum = RMH_Conversion_FloatToSystemDecimal(_TempCorrectionUpDown_DefaultMaxValue);
	GlobalVariables::CameraConfigNumericUpDowns[0]->Minimum = RMH_Conversion_FloatToSystemDecimal(_TempCorrectionUpDown_DefaultMinValue);

	// Configure the ambient temperature max/min NumericUpDown limits
	GlobalVariables::CameraConfigNumericUpDowns[1]->Maximum = RMH_Conversion_FloatToSystemDecimal((_AmbientTempUpDown_DefaultMaxValue * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor);
	GlobalVariables::CameraConfigNumericUpDowns[1]->Minimum = RMH_Conversion_FloatToSystemDecimal((_AmbientTempUpDown_DefaultMinValue * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor);

	// Configure the reflected temperature max/min NumericUpDown limits
	GlobalVariables::CameraConfigNumericUpDowns[2]->Maximum = RMH_Conversion_FloatToSystemDecimal((_ReflectedTempUpDown_DefaultMaxValue * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor);
	GlobalVariables::CameraConfigNumericUpDowns[2]->Minimum = RMH_Conversion_FloatToSystemDecimal((_ReflectedTempUpDown_DefaultMinValue * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor);

}

void RMH_ThermalViewer_RecoverDefaultCameraTempConfiguration() {

	// This routine sets the default temperature configuration for the connected thermal camera

	// Set/write the default temperature configuration values to the associated UpDowns 
	GlobalVariables::CameraConfigNumericUpDowns[0]->Value = (System::Decimal)_IRThermalCameraDefault_TemperatureCorrectionValue;                                                                // Temperature correction
	GlobalVariables::CameraConfigNumericUpDowns[1]->Value = (System::Decimal)((_IRThermalCameraDefault_AmbientTemperatureValue * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor);    // Ambient temperature
	GlobalVariables::CameraConfigNumericUpDowns[2]->Value = (System::Decimal)((_IRThermalCameraDefault_ReflectedTemperatureValue * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor);  // Reflected temperature
	GlobalVariables::CameraConfigNumericUpDowns[3]->Value = (System::Decimal)_IRThermalCameraDefault_SurroundingHumidityValue;                                                                  // Humidity 
	GlobalVariables::CameraConfigNumericUpDowns[4]->Value = (System::Decimal)_IRThermalCameraDefault_ObjectEmissivityValue;                                                                     // Emissivity
	GlobalVariables::CameraConfigNumericUpDowns[5]->Value = (System::Decimal)_IRThermalCameraDefault_ObjectDistanceValue;                                                                       // Distance

}

// ---------------- 2D Temperature Plot Handling & Configuration Routines ---------------- //

void RMH_ThermalViewer_Set2DPlotDataSetSource(double **Plot2DDataSetSourcePointer, unsigned char DataSource) {

	// This routine sets the data set pointers of the 2D plot to the selected data source

	// Which data source is selected
	switch (DataSource) {

		// Set the data source pointer
		case _2DPlotDataSource_MaximumTemp:			*Plot2DDataSetSourcePointer = &MaximumTemperature;				break;
		case _2DPlotDataSource_MinimumTemp:			*Plot2DDataSetSourcePointer = &MinimumTemperature;				break;
		case _2DPlotDataSource_AverageTemp:			*Plot2DDataSetSourcePointer = &AverageTemperature;				break;
		case _2DPlotDataSource_CenterTemp:			*Plot2DDataSetSourcePointer = &CenterTemperature;				break;
		case _2DPlotDataSource_TempPoint1:			*Plot2DDataSetSourcePointer = &TempMeasurementValues[0];		break;
		case _2DPlotDataSource_TempPoint2:			*Plot2DDataSetSourcePointer = &TempMeasurementValues[1];		break;
		case _2DPlotDataSource_TempPoint3:			*Plot2DDataSetSourcePointer = &TempMeasurementValues[2];		break;
		case _2DPlotDataSource_TempPoint4:			*Plot2DDataSetSourcePointer = &TempMeasurementValues[3];		break;
		case _2DPlotDataSource_TempPoint5:			*Plot2DDataSetSourcePointer = &TempMeasurementValues[4];		break;
		case _2DPlotDataSource_TempPoint6:			*Plot2DDataSetSourcePointer = &TempMeasurementValues[5];		break;
		case _2DPlotDataSource_TempPoint7:			*Plot2DDataSetSourcePointer = &TempMeasurementValues[6];		break;
		case _2DPlotDataSource_TempPoint8:			*Plot2DDataSetSourcePointer = &TempMeasurementValues[7];		break;
		case _2DPlotDataSource_TempPoint9:			*Plot2DDataSetSourcePointer = &TempMeasurementValues[8];		break;
		case _2DPlotDataSource_TempPoint10:			*Plot2DDataSetSourcePointer = &TempMeasurementValues[9];		break;
		case _2DPlotDataSource_Line1MaxTemp:		*Plot2DDataSetSourcePointer = &TempLinesMaxTempValues[0];		break;
		case _2DPlotDataSource_Line1MinTemp:		*Plot2DDataSetSourcePointer = &TempLinesMinTempValues[0];		break;
		case _2DPlotDataSource_Line1AvgTemp:		*Plot2DDataSetSourcePointer = &TempLinesAvgTempValues[0];		break;
		case _2DPlotDataSource_Line2MaxTemp:		*Plot2DDataSetSourcePointer = &TempLinesMaxTempValues[1];		break;
		case _2DPlotDataSource_Line2MinTemp:		*Plot2DDataSetSourcePointer = &TempLinesMinTempValues[1];		break;
		case _2DPlotDataSource_Line2AvgTemp:		*Plot2DDataSetSourcePointer = &TempLinesAvgTempValues[1];		break;
		case _2DPlotDataSource_Line3MaxTemp:		*Plot2DDataSetSourcePointer = &TempLinesMaxTempValues[2];		break;
		case _2DPlotDataSource_Line3MinTemp:		*Plot2DDataSetSourcePointer = &TempLinesMinTempValues[2];		break;
		case _2DPlotDataSource_Line3AvgTemp:		*Plot2DDataSetSourcePointer = &TempLinesAvgTempValues[2];		break;
		case _2DPlotDataSource_Line4MaxTemp:		*Plot2DDataSetSourcePointer = &TempLinesMaxTempValues[3];		break;
		case _2DPlotDataSource_Line4MinTemp:		*Plot2DDataSetSourcePointer = &TempLinesMinTempValues[3];		break;
		case _2DPlotDataSource_Line4AvgTemp:		*Plot2DDataSetSourcePointer = &TempLinesAvgTempValues[3];		break;
		case _2DPlotDataSource_Line5MaxTemp:		*Plot2DDataSetSourcePointer = &TempLinesMaxTempValues[4];		break;
		case _2DPlotDataSource_Line5MinTemp:		*Plot2DDataSetSourcePointer = &TempLinesMinTempValues[4];		break;
		case _2DPlotDataSource_Line5AvgTemp:		*Plot2DDataSetSourcePointer = &TempLinesAvgTempValues[4];		break;
		case _2DPlotDataSource_ROI1MaxTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[0].MaxValue;	break;
		case _2DPlotDataSource_ROI1MinTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[0].MinValue;	break;
		case _2DPlotDataSource_ROI1AvgTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[0].AvgValue;	break;
		case _2DPlotDataSource_ROI2MaxTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[1].MaxValue;	break;
		case _2DPlotDataSource_ROI2MinTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[1].MinValue;	break;
		case _2DPlotDataSource_ROI2AvgTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[1].AvgValue;	break;
		case _2DPlotDataSource_ROI3MaxTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[2].MaxValue;	break;
		case _2DPlotDataSource_ROI3MinTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[2].MinValue;	break;
		case _2DPlotDataSource_ROI3AvgTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[2].AvgValue;	break;
		case _2DPlotDataSource_ROI4MaxTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[3].MaxValue;	break;
		case _2DPlotDataSource_ROI4MinTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[3].MinValue;	break;
		case _2DPlotDataSource_ROI4AvgTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[3].AvgValue;	break;
		case _2DPlotDataSource_ROI5MaxTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[4].MaxValue;	break;
		case _2DPlotDataSource_ROI5MinTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[4].MinValue;	break;
		case _2DPlotDataSource_ROI5AvgTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[4].AvgValue;	break;
		case _2DPlotDataSource_ROI6MaxTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[5].MaxValue;	break;
		case _2DPlotDataSource_ROI6MinTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[5].MinValue;	break;
		case _2DPlotDataSource_ROI6AvgTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[5].AvgValue;	break;
		case _2DPlotDataSource_ROI7MaxTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[6].MaxValue;	break;
		case _2DPlotDataSource_ROI7MinTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[6].MinValue;	break;
		case _2DPlotDataSource_ROI7AvgTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[6].AvgValue;	break;
		case _2DPlotDataSource_ROI8MaxTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[7].MaxValue;	break;
		case _2DPlotDataSource_ROI8MinTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[7].MinValue;	break;
		case _2DPlotDataSource_ROI8AvgTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[7].AvgValue;	break;
		case _2DPlotDataSource_ROI9MaxTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[8].MaxValue;	break;
		case _2DPlotDataSource_ROI9MinTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[8].MinValue;	break;
		case _2DPlotDataSource_ROI9AvgTemp:			*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[8].AvgValue;	break;
		case _2DPlotDataSource_ROI10MaxTemp:		*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[9].MaxValue;	break;
		case _2DPlotDataSource_ROI10MinTemp:		*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[9].MinValue;	break;
		case _2DPlotDataSource_ROI10AvgTemp:		*Plot2DDataSetSourcePointer = &ROIAreaPixelValues[9].AvgValue;	break;
		case _2DPlotDataSource_MousePositionTemp:   *Plot2DDataSetSourcePointer = &CursorTemperature;				break;
		case _2DPlotDataSource_ThermalSensorDrift:  *Plot2DDataSetSourcePointer = &SensorTemperatureCalDrift;		break;

	}

}

void RMH_ThermalViewer_Enable2DPlotDataSet(System::Object^ sender) {

	// This routine enables a 2D plot data set for plotting

	// Cast the sender object as a WinForms CheckBox object
	System::Windows::Forms::CheckBox^ CheckBox = (System::Windows::Forms::CheckBox^)sender;

	// Read the identification tag of the form CheckBox object
	unsigned char CheckBoxTag = Convert::ToInt16(CheckBox->Tag);

	// Check the CheckBox state
	if ((bool)CheckBox->Checked == false) {

		// Reset the data set line data rendering index offset value
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_ResetDataSetLineDataIndexRenderOffset(CheckBoxTag);

	}

	// Enable plotting of a selected data set index
	GlobalVariables::OpenGL2DPlot->RMH_OpenGL_EnablePlotOfDataSetx(CheckBoxTag, (bool)CheckBox->Checked);

}

void RMH_ThermalViewer_Load2DPlotLineColorDataToGlobalArrays() {

	// This routine loads the 2D plot line color data into the global arrays

	// Loop up to and including the maximum number of 2D plot data sets
	for (unsigned int i = 0; i < _2DPlotMaxNumberOfDataSets; i++) {

		// Load the color values of the 2D plot data lines into the global arrays
		Plot2DDataSetLineColorsR[i] = GlobalVariables::Plot2DDataSetColorPanels[i]->BackColor.R;
		Plot2DDataSetLineColorsG[i] = GlobalVariables::Plot2DDataSetColorPanels[i]->BackColor.G;
		Plot2DDataSetLineColorsB[i] = GlobalVariables::Plot2DDataSetColorPanels[i]->BackColor.B;

	}

}

void RMH_ThermalViewer_Load2DPlotSavedSessionLineColorData() {

	// This routine sets the saved session 2D plot line color data

	// Loop up to and including the maximum number of 2D plot data sets
	for (unsigned int i = 0; i < _2DPlotMaxNumberOfDataSets; i++) {

		// Set the 2D plot line color data on the visual panels
		GlobalVariables::Plot2DDataSetColorPanels[i]->BackColor = System::Drawing::Color::FromArgb(255, Plot2DDataSetLineColorsR[i], Plot2DDataSetLineColorsG[i], Plot2DDataSetLineColorsB[i]);

		// Set the plot data set line color
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_SetDataSetLineColor(i, Plot2DDataSetLineColorsR[i], Plot2DDataSetLineColorsG[i], Plot2DDataSetLineColorsB[i]);

	}

}

void RMH_ThermalViewer_Apply2DPlotDataSetSettings() {

	// This routine applies the start-up state of the 2D plot data set settings (data source and enable CheckBox) to the 2D plot
	// The settings controls are created with their CheckBoxes checked, but without raising the CheckBox event, so without this the
	// data sets of the 2D plot stay disabled (the plot does not start) until a setting has been changed manually

	// Loop up to and including the maximum number of 2D plot data sets
	for (unsigned int i = 0; i < _2DPlotMaxNumberOfDataSets; i++) {

		// Set the data source of the data set from the ComboBox selection
		if (GlobalVariables::Plot2DDataSetComboBoxs[i]->SelectedIndex >= 0) {

			RMH_ThermalViewer_Change2DPlotDataSetSource(GlobalVariables::Plot2DDataSetComboBoxs[i]);

		}

		// Enable or disable plotting of the data set from the CheckBox state
		RMH_ThermalViewer_Enable2DPlotDataSet(GlobalVariables::Plot2DDataSetCheckBoxs[i]);

	}

}

void RMH_ThermalViewer_Change2DPlotDataSetAndSettingsPanelColor(System::Object^ sender) {

	// This routine updates and sets the color of a 2D plot data set, as well as the color of the settings panel

	// Local flag variable
	bool ColorDialogAbortFlag = false;
	System::Drawing::Color^ SelectedColor;

	// Cast the sender object as a WinForms Panel object
	System::Windows::Forms::Panel^ PanelObject = (System::Windows::Forms::Panel^)sender;

	// Read the identification tag of the form Panel object
	unsigned char ColorPanelTag = Convert::ToInt16(PanelObject->Tag);

	// Open the color dialog and read the selected color
	SelectedColor = RMH_Winforms_ShowAndReadColorDialog(&ColorDialogAbortFlag);

	// Check the color dialog abort flag
	if (ColorDialogAbortFlag == false) {

		// Set the new selected color of the color panel
		PanelObject->BackColor = System::Drawing::Color::FromArgb(255, SelectedColor->R, SelectedColor->G, SelectedColor->B);

		// Set the plot data set line color
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_SetDataSetLineColor(ColorPanelTag, SelectedColor->R, SelectedColor->G, SelectedColor->B);

	}

}

void RMH_ThermalViewer_Change2DPlotDataSetLineWidth(System::Object^ sender) {

	// This routine sets the line thickness of a 2D plot data set

	// Cast the sender object as a WinForms NumericUpDown object
	System::Windows::Forms::NumericUpDown^ NumericUpDownObject = (System::Windows::Forms::NumericUpDown^)sender;

	// Read the identification tag of the form NumericUpDown object
	unsigned char NumericUpDownTag = Convert::ToInt16(NumericUpDownObject->Tag);

	// Set the line thickness of the plot data set
	GlobalVariables::OpenGL2DPlot->RMH_OpenGL_SetDataSetLineWidth(NumericUpDownTag, (float)NumericUpDownObject->Value);

}

void RMH_ThermalViewer_Change2DPlotDataSetSource(System::Object^ sender) {

	// This routine sets a newly selected 2D plot data set source to the selected ComboBox index

	// Cast the sender object as a WinForms ComboBox object
	System::Windows::Forms::ComboBox^ ComboBox = (System::Windows::Forms::ComboBox^)sender;

	// Read the identification tag of the form ComboBox object
	unsigned char ComboBoxTag = Convert::ToInt16(ComboBox->Tag);

	// Selection of the 2D plot data set from the ComboBox tag ID
	switch (ComboBoxTag) {

		// Set the selected data set
		case _2DPlotDataSet_1:  RMH_ThermalViewer_Set2DPlotDataSetSource(&Plot2DDataSet1SourcePointer, ComboBox->SelectedIndex); break;
		case _2DPlotDataSet_2:  RMH_ThermalViewer_Set2DPlotDataSetSource(&Plot2DDataSet2SourcePointer, ComboBox->SelectedIndex); break;
		case _2DPlotDataSet_3:  RMH_ThermalViewer_Set2DPlotDataSetSource(&Plot2DDataSet3SourcePointer, ComboBox->SelectedIndex); break;
		case _2DPlotDataSet_4:  RMH_ThermalViewer_Set2DPlotDataSetSource(&Plot2DDataSet4SourcePointer, ComboBox->SelectedIndex); break;
		case _2DPlotDataSet_5:  RMH_ThermalViewer_Set2DPlotDataSetSource(&Plot2DDataSet5SourcePointer, ComboBox->SelectedIndex); break;
		case _2DPlotDataSet_6:  RMH_ThermalViewer_Set2DPlotDataSetSource(&Plot2DDataSet6SourcePointer, ComboBox->SelectedIndex); break;
		case _2DPlotDataSet_7:  RMH_ThermalViewer_Set2DPlotDataSetSource(&Plot2DDataSet7SourcePointer, ComboBox->SelectedIndex); break;
		case _2DPlotDataSet_8:  RMH_ThermalViewer_Set2DPlotDataSetSource(&Plot2DDataSet8SourcePointer, ComboBox->SelectedIndex); break;
		case _2DPlotDataSet_9:  RMH_ThermalViewer_Set2DPlotDataSetSource(&Plot2DDataSet9SourcePointer, ComboBox->SelectedIndex); break;
		case _2DPlotDataSet_10: RMH_ThermalViewer_Set2DPlotDataSetSource(&Plot2DDataSet10SourcePointer, ComboBox->SelectedIndex); break;

	}

}

void RMH_ThermalViewer_Update2DPlotLegendLabels() {

	// This routine updates and sets the legend labels of the 2D plot with the name and color of the data set

	// Read the temporary array data and sort the kernel array
	unsigned int DataSetCheckedIndex = 0;

	// Loop up to and including the maximum number of 2D plot data sets
	for (unsigned int i = 0; i < _2DPlotMaxNumberOfDataSets; i++) {

		// Make the 2D plot legend label visible
		GlobalVariables::Plot2DLegendLabels[DataSetCheckedIndex]->Visible = false;

		// Increment the data set index 
		DataSetCheckedIndex = DataSetCheckedIndex + 1;

	}

	// Reset the data set index 
	DataSetCheckedIndex = 0;

	// Loop up to and including the maximum number of 2D plot data sets
	for (unsigned int i = 0; i < _2DPlotMaxNumberOfDataSets; i++) {

		// Check which 2D plot data sets are active
		if (GlobalVariables::Plot2DDataSetCheckBoxs[i]->Checked == true) {

			// Make the 2D plot legend label visible
			GlobalVariables::Plot2DLegendLabels[DataSetCheckedIndex]->Visible = true;

			// Set the 2D plot legend text to the ComboBox data set text
			GlobalVariables::Plot2DLegendLabels[DataSetCheckedIndex]->Text = GlobalVariables::Plot2DDataSetComboBoxs[i]->Text;

			// Update the 2D plot legend text color
			GlobalVariables::Plot2DLegendLabels[DataSetCheckedIndex]->ForeColor = GlobalVariables::Plot2DDataSetColorPanels[i]->BackColor;

			// Increment the data set index 
			DataSetCheckedIndex = DataSetCheckedIndex + 1;

		}
		else {

			// Make the 2D plot legend label invisible
			GlobalVariables::Plot2DLegendLabels[DataSetCheckedIndex]->Visible = false;

			// Reset the 2D plot legend text color
			GlobalVariables::Plot2DLegendLabels[DataSetCheckedIndex]->ForeColor = System::Drawing::Color::FromArgb(255, 60, 60, 60);

		}

	}

}

// --------------------------- Data Logging Handling Routines ---------------------------- //

void RMH_ThermalViewer_SetDataLoggingSourcePointer(double** DataLoggingSourcePointer, unsigned char DataSource) {

	// This routine sets the given data logging data source pointer

	// Which data source is selected
	switch (DataSource) {

		// Set the data source pointer
		case _2DPlotDataSource_MaximumTemp:			*DataLoggingSourcePointer = &MaximumTemperature;				break;
		case _2DPlotDataSource_MinimumTemp:			*DataLoggingSourcePointer = &MinimumTemperature;				break;
		case _2DPlotDataSource_CenterTemp:			*DataLoggingSourcePointer = &CenterTemperature;					break;
		case _2DPlotDataSource_TempPoint1:			*DataLoggingSourcePointer = &TempMeasurementValues[0];			break;
		case _2DPlotDataSource_TempPoint2:			*DataLoggingSourcePointer = &TempMeasurementValues[1];			break;
		case _2DPlotDataSource_TempPoint3:			*DataLoggingSourcePointer = &TempMeasurementValues[2];			break;
		case _2DPlotDataSource_TempPoint4:			*DataLoggingSourcePointer = &TempMeasurementValues[3];			break;
		case _2DPlotDataSource_TempPoint5:			*DataLoggingSourcePointer = &TempMeasurementValues[4];			break;
		case _2DPlotDataSource_TempPoint6:			*DataLoggingSourcePointer = &TempMeasurementValues[5];			break;
		case _2DPlotDataSource_TempPoint7:			*DataLoggingSourcePointer = &TempMeasurementValues[6];			break;
		case _2DPlotDataSource_TempPoint8:			*DataLoggingSourcePointer = &TempMeasurementValues[7];			break;
		case _2DPlotDataSource_TempPoint9:			*DataLoggingSourcePointer = &TempMeasurementValues[8];			break;
		case _2DPlotDataSource_TempPoint10:			*DataLoggingSourcePointer = &TempMeasurementValues[9];			break;
		case _2DPlotDataSource_Line1MaxTemp:		*DataLoggingSourcePointer = &TempLinesMaxTempValues[0];			break;
		case _2DPlotDataSource_Line1MinTemp:		*DataLoggingSourcePointer = &TempLinesMinTempValues[0];			break;
		case _2DPlotDataSource_Line1AvgTemp:		*DataLoggingSourcePointer = &TempLinesAvgTempValues[0];			break;
		case _2DPlotDataSource_Line2MaxTemp:		*DataLoggingSourcePointer = &TempLinesMaxTempValues[1];			break;
		case _2DPlotDataSource_Line2MinTemp:		*DataLoggingSourcePointer = &TempLinesMinTempValues[1];			break;
		case _2DPlotDataSource_Line2AvgTemp:		*DataLoggingSourcePointer = &TempLinesAvgTempValues[1];			break;
		case _2DPlotDataSource_Line3MaxTemp:		*DataLoggingSourcePointer = &TempLinesMaxTempValues[2];			break;
		case _2DPlotDataSource_Line3MinTemp:		*DataLoggingSourcePointer = &TempLinesMinTempValues[2];			break;
		case _2DPlotDataSource_Line3AvgTemp:		*DataLoggingSourcePointer = &TempLinesAvgTempValues[2];			break;
		case _2DPlotDataSource_Line4MaxTemp:		*DataLoggingSourcePointer = &TempLinesMaxTempValues[3];			break;
		case _2DPlotDataSource_Line4MinTemp:		*DataLoggingSourcePointer = &TempLinesMinTempValues[3];			break;
		case _2DPlotDataSource_Line4AvgTemp:		*DataLoggingSourcePointer = &TempLinesAvgTempValues[3];			break;
		case _2DPlotDataSource_Line5MaxTemp:		*DataLoggingSourcePointer = &TempLinesMaxTempValues[4];			break;
		case _2DPlotDataSource_Line5MinTemp:		*DataLoggingSourcePointer = &TempLinesMinTempValues[4];			break;
		case _2DPlotDataSource_Line5AvgTemp:		*DataLoggingSourcePointer = &TempLinesAvgTempValues[4];			break;
		case _2DPlotDataSource_ROI1MaxTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[0].MaxValue;	break;
		case _2DPlotDataSource_ROI1MinTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[0].MinValue;	break;
		case _2DPlotDataSource_ROI1AvgTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[0].AvgValue;	break;
		case _2DPlotDataSource_ROI2MaxTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[1].MaxValue;	break;
		case _2DPlotDataSource_ROI2MinTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[1].MinValue;	break;
		case _2DPlotDataSource_ROI2AvgTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[1].AvgValue;	break;
		case _2DPlotDataSource_ROI3MaxTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[2].MaxValue;	break;
		case _2DPlotDataSource_ROI3MinTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[2].MinValue;	break;
		case _2DPlotDataSource_ROI3AvgTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[2].AvgValue;	break;
		case _2DPlotDataSource_ROI4MaxTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[3].MaxValue;	break;
		case _2DPlotDataSource_ROI4MinTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[3].MinValue;	break;
		case _2DPlotDataSource_ROI4AvgTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[3].AvgValue;	break;
		case _2DPlotDataSource_ROI5MaxTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[4].MaxValue;	break;
		case _2DPlotDataSource_ROI5MinTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[4].MinValue;	break;
		case _2DPlotDataSource_ROI5AvgTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[4].AvgValue;	break;
		case _2DPlotDataSource_ROI6MaxTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[5].MaxValue;	break;
		case _2DPlotDataSource_ROI6MinTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[5].MinValue;	break;
		case _2DPlotDataSource_ROI6AvgTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[5].AvgValue;	break;
		case _2DPlotDataSource_ROI7MaxTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[6].MaxValue;	break;
		case _2DPlotDataSource_ROI7MinTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[6].MinValue;	break;
		case _2DPlotDataSource_ROI7AvgTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[6].AvgValue;	break;
		case _2DPlotDataSource_ROI8MaxTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[7].MaxValue;	break;
		case _2DPlotDataSource_ROI8MinTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[7].MinValue;	break;
		case _2DPlotDataSource_ROI8AvgTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[7].AvgValue;	break;
		case _2DPlotDataSource_ROI9MaxTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[8].MaxValue;	break;
		case _2DPlotDataSource_ROI9MinTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[8].MinValue;	break;
		case _2DPlotDataSource_ROI9AvgTemp:			*DataLoggingSourcePointer = &ROIAreaPixelValues[8].AvgValue;	break;
		case _2DPlotDataSource_ROI10MaxTemp:		*DataLoggingSourcePointer = &ROIAreaPixelValues[9].MaxValue;	break;
		case _2DPlotDataSource_ROI10MinTemp:		*DataLoggingSourcePointer = &ROIAreaPixelValues[9].MinValue;	break;
		case _2DPlotDataSource_ROI10AvgTemp:		*DataLoggingSourcePointer = &ROIAreaPixelValues[9].AvgValue;	break;
		case _2DPlotDataSource_MousePositionTemp:   *DataLoggingSourcePointer = &CursorTemperature;					break;
		case _2DPlotDataSource_ThermalSensorDrift:  *DataLoggingSourcePointer = &SensorTemperatureCalDrift;			break;

	}

}

void RMH_ThermalViewer_ChangeDataLoggingDataSetSource(unsigned char DataSetIndex, unsigned char DataSourceIndex) {

	// This routine sets a data logging pointer to a selected data set source

	// Selection of the data logging data set
	switch (DataSetIndex) {

		// Set the data logging data set to the given data source
		case _2DPlotDataSet_1:  RMH_ThermalViewer_SetDataLoggingSourcePointer(&DataLoggingDataSet1SourcePointer, DataSourceIndex);  break;
		case _2DPlotDataSet_2:  RMH_ThermalViewer_SetDataLoggingSourcePointer(&DataLoggingDataSet2SourcePointer, DataSourceIndex);  break;
		case _2DPlotDataSet_3:  RMH_ThermalViewer_SetDataLoggingSourcePointer(&DataLoggingDataSet3SourcePointer, DataSourceIndex);  break;
		case _2DPlotDataSet_4:  RMH_ThermalViewer_SetDataLoggingSourcePointer(&DataLoggingDataSet4SourcePointer, DataSourceIndex);  break;
		case _2DPlotDataSet_5:  RMH_ThermalViewer_SetDataLoggingSourcePointer(&DataLoggingDataSet5SourcePointer, DataSourceIndex);  break;
		case _2DPlotDataSet_6:  RMH_ThermalViewer_SetDataLoggingSourcePointer(&DataLoggingDataSet6SourcePointer, DataSourceIndex);  break;
		case _2DPlotDataSet_7:  RMH_ThermalViewer_SetDataLoggingSourcePointer(&DataLoggingDataSet7SourcePointer, DataSourceIndex);  break;
		case _2DPlotDataSet_8:  RMH_ThermalViewer_SetDataLoggingSourcePointer(&DataLoggingDataSet8SourcePointer, DataSourceIndex);  break;
		case _2DPlotDataSet_9:  RMH_ThermalViewer_SetDataLoggingSourcePointer(&DataLoggingDataSet9SourcePointer, DataSourceIndex);  break;
		case _2DPlotDataSet_10: RMH_ThermalViewer_SetDataLoggingSourcePointer(&DataLoggingDataSet10SourcePointer, DataSourceIndex); break;

	}

}

double* RMH_ThermalViewer_GetDataLoggingSourcePointerFromIndex(unsigned char DataSourceIndex) {

	// This routine returns a selected data logging data source pointer

	// Read the temporary array data and sort the kernel array
	double* ReturnPointer;

	// Selection of the data source pointer index
	switch (DataSourceIndex) {

		// Set the return pointer to the current data logging source pointer
		case _2DPlotDataSet_1:   ReturnPointer = DataLoggingDataSet1SourcePointer;   break;
		case _2DPlotDataSet_2:   ReturnPointer = DataLoggingDataSet2SourcePointer;   break;
		case _2DPlotDataSet_3:   ReturnPointer = DataLoggingDataSet3SourcePointer;   break;
		case _2DPlotDataSet_4:   ReturnPointer = DataLoggingDataSet4SourcePointer;   break;
		case _2DPlotDataSet_5:   ReturnPointer = DataLoggingDataSet5SourcePointer;   break;
		case _2DPlotDataSet_6:   ReturnPointer = DataLoggingDataSet6SourcePointer;   break;
		case _2DPlotDataSet_7:   ReturnPointer = DataLoggingDataSet7SourcePointer;   break;
		case _2DPlotDataSet_8:   ReturnPointer = DataLoggingDataSet8SourcePointer;   break;
		case _2DPlotDataSet_9:   ReturnPointer = DataLoggingDataSet9SourcePointer;   break;
		case _2DPlotDataSet_10:  ReturnPointer = DataLoggingDataSet10SourcePointer;  break;
	
	}

	// Return the data logging data source pointer
	return ReturnPointer;

}

void RMH_ThermalViewer_UpdateDataLoggingDefaultSaveFilePath(System::Windows::Forms::Label^ DefaultPathString) {

	// This routine updates the file location where the data logging CSV file is saved

	// Read the temporary array data and sort the kernel array
	System::String^ SaveFilePathString;

	// Read the selected default save file path for data logging 
	SaveFilePathString = RMH_Winforms_GetSaveFileDialogDirectory();

	// Check whether a path was selected, or whether the dialog was closed
	if (SaveFilePathString == "None") {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "No New Default Data Logging File Path Was Choosen!", _StatusMessageType_Warning);

	}
	else {

		// Update the default data logging path
		GlobalVariables::LoggingCSVDefaultPath = SaveFilePathString;

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "New Default Data Logging File Path Was Choosen.", _StatusMessageType_Success);

	}

	// Update the default data logging file path string 
	DefaultPathString->Text = "Default Save File Path:  " + GlobalVariables::LoggingCSVDefaultPath;

}

void RMH_ThermalViewer_StartDataLogging() {

	// This routine configures the data logging session and starts data logging

	// Local variables
	unsigned int SelectedSourceIndex = 0;
	unsigned char DataLoggingDataSetIndex = 0;
	unsigned short DataLoggingDurationHoursValue = 0;
	unsigned short DataLoggingDurationMinutesValue = 0;
	unsigned short DataLoggingDurationSedundsValue = 0;
	std::vector<std::string> CSVFileHeaderStrings = { "Sample", "Time(ms)", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A" };

	// Check the data logging is running flag
	if (DataLoggingIsRunningFlag == false) {

		// Reset the data logging duration timer variable
		DataLoggingDurationTimerMilliSec = 0;

		// Reset the number of active data logging sets
		DataLoggingNumberOfActiveSets = 0;

		// Read the duration parameters of the data logging session
		DataLoggingDurationHoursValue = System::Decimal::ToInt16(GlobalVariables::GlobalDataLoggingDurationHourUpDown->Value);
		DataLoggingDurationMinutesValue = System::Decimal::ToInt16(GlobalVariables::GlobalDataLoggingDurationMinuteUpDown->Value);
		DataLoggingDurationSedundsValue = System::Decimal::ToInt16(GlobalVariables::GlobalDataLoggingDurationSecondsUpDown->Value);

		// Calculate the length of the data logging session in milliseconds
		DataLoggingSessionDurationMilliSec = (DataLoggingDurationHoursValue * 3600000) + (DataLoggingDurationMinutesValue * 60000) + (DataLoggingDurationSedundsValue * 1000);

		// Set the execution interval variable of the data logging thread
		DataLoggingIntervalMilliSec = (unsigned long)(System::Decimal::ToDouble(GlobalVariables::GlobalDataLoggingIntervalUpDown->Value) * 1000.0);

		// ----------------------------------- Format Data Logging Plot Data Set Pointers ----------------------------------- //

		// Loop up to and including the maximum number of 2D plot data sets
		for (unsigned int i = 0; i < _2DPlotMaxNumberOfDataSets; i++) {

			// Check whether the data set is active for data logging
			if (GlobalVariables::Plot2DDataSetCheckBoxs[i]->Checked == true) {

				// Read the combobox data source index of the active data set
				SelectedSourceIndex = GlobalVariables::Plot2DDataSetComboBoxs[i]->SelectedIndex;

				// Set the data logging data set to the selected combobox data source
				RMH_ThermalViewer_ChangeDataLoggingDataSetSource(DataLoggingDataSetIndex, SelectedSourceIndex);

				// Store the data set source description in the CSV header string array
				CSVFileHeaderStrings[DataLoggingDataSetIndex + 2] = RMH_Conversion_SystemStringToStdString(GlobalVariables::Plot2DDataSetComboBoxs[i]->Text);

				// Increment the data logging data set index variable
				DataLoggingDataSetIndex = DataLoggingDataSetIndex + 1;

				// Increment the counter variable of the number of active data logging sets
				DataLoggingNumberOfActiveSets = DataLoggingNumberOfActiveSets + 1;

			}

		}

		// -------------------------------- Generate And Format The Data Logging Session CSV File -------------------------------- //

		// Format the data identification string of the file (DataLogSession_HHmmssddMMyyyy)
		System::String^ FileName = System::DateTime::Now.ToString("HHmmssfffddMMyyyy");

		// Format the file name of the data logging session
		GlobalVariables::DataLoggingSessionFileNameString = "DataLogSession_" + FileName + ".txt";

		// Generate the data logging CSV file with the associated file header
		RMH_Winforms_WriteHeaderStringsToCSVFile(RMH_Conversion_SystemStringToStdString(GlobalVariables::LoggingCSVDefaultPath), 
			RMH_Conversion_SystemStringToStdString(GlobalVariables::DataLoggingSessionFileNameString), CSVFileHeaderStrings, DataLoggingNumberOfActiveSets + 2, GlobalVariables::DataLoggingCSVDelimiterString);

		// -------------------------------------------------------------------------------------------------------------------- //

		// Update the data logging is running flag
		DataLoggingIsRunningFlag = true;

		// Start Data Logging Thread Process
		GlobalVariables::GlobalDataLoggingThread->RunWorkerAsync();

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Data Logging Has Started.", _StatusMessageType_Success);

	}
	else {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Data Logging Is Already Running!", _StatusMessageType_Warning);

	}

}

void RMH_ThermalViewer_StopDataLogging() {

	// Check the data logging status
	if (DataLoggingIsRunningFlag == true) {

		// Update the data logging is running flag
		DataLoggingIsRunningFlag = false;

		// Update the 2D plot data logging indicator string with timer - inactive state
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_SetDataLoggingLabelStateAndTimer(DataLoggingIsRunningFlag, DataLoggingDurationTimerMilliSec);

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Data Logging Session Has Stopped.", _StatusMessageType_Warning);

	}
	else {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "No Data Logging Session Is Running...", _StatusMessageType_Normal);

	}
	
}

void RMH_ThermalViewer_UpdateDataLoggingCSVDataDelimiter() {

	// This routine updates which data delimiter is used when saving a data logging CSV file

	// Read the temporary array data and sort the kernel array
	unsigned char DataDelimiterIndex = 0;

	// Read the selected data delimiter from the associated ComboBox
	DataDelimiterIndex = GlobalVariables::GlobalDataLoggingCSVDelimiterCombiBox->SelectedIndex;

	// Store and update the CSV data delimiter read in the global variable
	SelectedDataLoggingCSVDataDelimiterIndex = DataDelimiterIndex;

	// Which delimiter index has been selected
	switch (SelectedDataLoggingCSVDataDelimiterIndex) {

		// Update the associated global delimiter string
		case _DataLoggingDelimiterIndex_Comma:		GlobalVariables::DataLoggingCSVDelimiterString = ",";  break;
		case _DataLoggingDelimiterIndex_Semicolon:	GlobalVariables::DataLoggingCSVDelimiterString = ";";  break;
		case _DataLoggingDelimiterIndex_Colon:		GlobalVariables::DataLoggingCSVDelimiterString = ":";  break;
		case _DataLoggingDelimiterIndex_Space:		GlobalVariables::DataLoggingCSVDelimiterString = " ";  break;
		case _DataLoggingDelimiterIndex_Tab:		GlobalVariables::DataLoggingCSVDelimiterString = "\t"; break;

	}

}

void RMH_ThermalViewer_DataLoggingThreadProcess() {

	// This routine is the associated data logging processor thread

	// Read the temporary array data and sort the kernel array
	unsigned long NumberOfSampels = 0;

	// Execute the thread process loop if data logging is active 
	while (DataLoggingIsRunningFlag == true) {

		// The execution interval of the thread is the data logging interval
		System::Threading::Thread::Sleep(DataLoggingIntervalMilliSec);

		// Update the data logging duration counter variable
		DataLoggingDurationTimerMilliSec = DataLoggingDurationTimerMilliSec + DataLoggingIntervalMilliSec;

		// Update the 2D plot data logging indicator string with timer
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_SetDataLoggingLabelStateAndTimer(DataLoggingIsRunningFlag, DataLoggingDurationTimerMilliSec);

		// Load the data logging source data into the data array
		for (unsigned int i = 0; i < DataLoggingNumberOfActiveSets; i++) {

			// Load the data into the source data array
			DataLoggingSourceDataArray[i] = *RMH_ThermalViewer_GetDataLoggingSourcePointerFromIndex(i);

		}

		// Increment the sample counter variable
		NumberOfSampels = NumberOfSampels + 1;

		// If the data logging session has reached its set end
		if (DataLoggingDurationTimerMilliSec >= DataLoggingSessionDurationMilliSec + DataLoggingIntervalMilliSec) {

			// Update the data logging is running flag
			DataLoggingIsRunningFlag = false;

			// Update the 2D plot data logging indicator string with timer - inactive state
			GlobalVariables::OpenGL2DPlot->RMH_OpenGL_SetDataLoggingLabelStateAndTimer(DataLoggingIsRunningFlag, DataLoggingDurationTimerMilliSec);

			// Break the while loop
			break;

		}

		// Write the data to the generated CSV file
		RMH_Winforms_WriteDataArrayToCSVFile(RMH_Conversion_SystemStringToStdString(GlobalVariables::LoggingCSVDefaultPath),
			RMH_Conversion_SystemStringToStdString(GlobalVariables::DataLoggingSessionFileNameString),
			RMH_Conversion_SystemStringToStdString(NumberOfSampels.ToString()),
			RMH_Conversion_SystemStringToStdString(DataLoggingDurationTimerMilliSec.ToString()),
			&DataLoggingSourceDataArray[0], DataLoggingNumberOfActiveSets, GlobalVariables::DataLoggingCSVDelimiterString);

	}

}

// ------------------------ Temperature Alarm Handling Routines ------------------------ //

void RMH_ThermalViewer_SetTempAlarmDataSourcePointer(double** TempAlarmSourcePointer, unsigned char AlarmDataSource) {

	// This routine sets a selected temperature alarm data source pointer

	// Which alarm data source is selected
	switch (AlarmDataSource) {

		// Set the alarm data source pointer
		case _TempAlarmDataSource_MaximumTemp:			*TempAlarmSourcePointer = &MaximumTemperature;				break;
		case _TempAlarmDataSource_MinimumTemp:			*TempAlarmSourcePointer = &MinimumTemperature;				break;
		case _TempAlarmDataSource_AverageTemp:			*TempAlarmSourcePointer = &AverageTemperature;				break;
		case _TempAlarmDataSource_CenterTemp:			*TempAlarmSourcePointer = &CenterTemperature;				break;
		case _TempAlarmDataSource_TempPoint1:			*TempAlarmSourcePointer = &TempMeasurementValues[0];		break;
		case _TempAlarmDataSource_TempPoint2:			*TempAlarmSourcePointer = &TempMeasurementValues[1];		break;
		case _TempAlarmDataSource_TempPoint3:			*TempAlarmSourcePointer = &TempMeasurementValues[2];		break;
		case _TempAlarmDataSource_TempPoint4:			*TempAlarmSourcePointer = &TempMeasurementValues[3];		break;
		case _TempAlarmDataSource_TempPoint5:			*TempAlarmSourcePointer = &TempMeasurementValues[4];		break;
		case _TempAlarmDataSource_TempPoint6:			*TempAlarmSourcePointer = &TempMeasurementValues[5];		break;
		case _TempAlarmDataSource_TempPoint7:			*TempAlarmSourcePointer = &TempMeasurementValues[6];		break;
		case _TempAlarmDataSource_TempPoint8:			*TempAlarmSourcePointer = &TempMeasurementValues[7];		break;
		case _TempAlarmDataSource_TempPoint9:			*TempAlarmSourcePointer = &TempMeasurementValues[8];		break;
		case _TempAlarmDataSource_TempPoint10:			*TempAlarmSourcePointer = &TempMeasurementValues[9];		break;
		case _TempAlarmDataSource_Line1MaxTemp:			*TempAlarmSourcePointer = &TempLinesMaxTempValues[0];		break;
		case _TempAlarmDataSource_Line1MinTemp:			*TempAlarmSourcePointer = &TempLinesMinTempValues[0];		break;
		case _TempAlarmDataSource_Line2MaxTemp:			*TempAlarmSourcePointer = &TempLinesMaxTempValues[1];		break;
		case _TempAlarmDataSource_Line2MinTemp:			*TempAlarmSourcePointer = &TempLinesMinTempValues[1];		break;
		case _TempAlarmDataSource_Line3MaxTemp:			*TempAlarmSourcePointer = &TempLinesMaxTempValues[2];		break;
		case _TempAlarmDataSource_Line3MinTemp:			*TempAlarmSourcePointer = &TempLinesMinTempValues[2];		break;
		case _TempAlarmDataSource_Line4MaxTemp:			*TempAlarmSourcePointer = &TempLinesMaxTempValues[3];		break;
		case _TempAlarmDataSource_Line4MinTemp:			*TempAlarmSourcePointer = &TempLinesMinTempValues[3];		break;
		case _TempAlarmDataSource_Line5MaxTemp:			*TempAlarmSourcePointer = &TempLinesMaxTempValues[4];		break;
		case _TempAlarmDataSource_Line5MinTemp:			*TempAlarmSourcePointer = &TempLinesMinTempValues[4];		break;
		case _TempAlarmDataSource_ROI1MaxTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[0].MaxValue;	break;
		case _TempAlarmDataSource_ROI1MinTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[0].MinValue;	break;
		case _TempAlarmDataSource_ROI2MaxTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[1].MaxValue;	break;
		case _TempAlarmDataSource_ROI2MinTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[1].MinValue;	break;
		case _TempAlarmDataSource_ROI3MaxTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[2].MaxValue;	break;
		case _TempAlarmDataSource_ROI3MinTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[2].MinValue;	break;
		case _TempAlarmDataSource_ROI4MaxTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[3].MaxValue;	break;
		case _TempAlarmDataSource_ROI4MinTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[3].MinValue;	break;
		case _TempAlarmDataSource_ROI5MaxTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[4].MaxValue;	break;
		case _TempAlarmDataSource_ROI5MinTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[4].MinValue;	break;
		case _TempAlarmDataSource_ROI6MaxTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[5].MaxValue;	break;
		case _TempAlarmDataSource_ROI6MinTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[5].MinValue;	break;
		case _TempAlarmDataSource_ROI7MaxTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[6].MaxValue;	break;
		case _TempAlarmDataSource_ROI7MinTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[6].MinValue;	break;
		case _TempAlarmDataSource_ROI8MaxTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[7].MaxValue;	break;
		case _TempAlarmDataSource_ROI8MinTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[7].MinValue;	break;
		case _TempAlarmDataSource_ROI9MaxTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[8].MaxValue;	break;
		case _TempAlarmDataSource_ROI9MinTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[8].MinValue;	break;
		case _TempAlarmDataSource_ROI10MaxTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[9].MaxValue;	break;
		case _TempAlarmDataSource_ROI10MinTemp:			*TempAlarmSourcePointer = &ROIAreaPixelValues[9].MinValue;	break;
		case _TempAlarmDataSource_MousePositionTemp:    *TempAlarmSourcePointer = &CursorTemperature;				break;

	}

}

void RMH_ThermalViewer_ChangeTemperatureAlarmDataSource(System::Object^ sender) {

	// This routine sets a temperature alarm pointer to a selected data source

	// Cast the sender object as a WinForms ComboBox object
	System::Windows::Forms::ComboBox^ ComboBox = (System::Windows::Forms::ComboBox^)sender;

	// Read the identification tag of the form ComboBox object
	unsigned char ComboBoxTag = Convert::ToInt16(ComboBox->Tag);

	// Selection of the temp alarm data source
	switch (ComboBoxTag) {

		// Set the temperature alarm data to the given data source
		case _TemperatureAlarm_1:  RMH_ThermalViewer_SetTempAlarmDataSourcePointer(&TempAlarm1DataSourcePointer, ComboBox->SelectedIndex);  break;
		case _TemperatureAlarm_2:  RMH_ThermalViewer_SetTempAlarmDataSourcePointer(&TempAlarm2DataSourcePointer, ComboBox->SelectedIndex);  break;
		case _TemperatureAlarm_3:  RMH_ThermalViewer_SetTempAlarmDataSourcePointer(&TempAlarm3DataSourcePointer, ComboBox->SelectedIndex);  break;
		case _TemperatureAlarm_4:  RMH_ThermalViewer_SetTempAlarmDataSourcePointer(&TempAlarm4DataSourcePointer, ComboBox->SelectedIndex);  break;
		case _TemperatureAlarm_5:  RMH_ThermalViewer_SetTempAlarmDataSourcePointer(&TempAlarm5DataSourcePointer, ComboBox->SelectedIndex);  break;

	}

	// Reset the status label string and color of the temperature alarm
	GlobalVariables::TempAlarmsStatusLabels[ComboBoxTag]->Text = "Normal";
	GlobalVariables::TempAlarmsStatusLabels[ComboBoxTag]->ForeColor = System::Drawing::Color::White;

	// Reset the trigger status of the temperature alarm
	AlarmsTriggerStatusArray[ComboBoxTag] = false;

}

void RMH_ThermalViewer_ChangeTempAlarmConfigType(System::Object^ sender) {

	// This routine sets the type of the temperature alarm

	// Cast the sender object as a WinForms ComboBox object
	System::Windows::Forms::ComboBox^ ComboBox = (System::Windows::Forms::ComboBox^)sender;

	// Read the identification tag of the form ComboBox object
	unsigned char ComboBoxTag = Convert::ToInt16(ComboBox->Tag);

	// Set the temperature alarm trigger type
	TempAlarmsConfigType[ComboBoxTag] = ComboBox->SelectedIndex;

	// Reset the status label string and color of the temperature alarm
	GlobalVariables::TempAlarmsStatusLabels[ComboBoxTag]->Text = "Normal";
	GlobalVariables::TempAlarmsStatusLabels[ComboBoxTag]->ForeColor = System::Drawing::Color::White;

	// Reset the trigger status of the temperature alarm
	AlarmsTriggerStatusArray[ComboBoxTag] = false;

}

void RMH_ThermalViewer_ChangeTempAlarmLowTempSetPoint(System::Object^ sender) {

	// This routine sets the low temperature value of a selected temperature alarm

	// Cast the sender object as a WinForms UpDown object
	System::Windows::Forms::NumericUpDown^ SenderUpDown = (System::Windows::Forms::NumericUpDown^)sender;

	// Read the identification tag of the form UpDown object
	unsigned char SenderUpDownTag = Convert::ToInt16(SenderUpDown->Tag);

	// Write the configured temperature alarm low value to the global array
	TempAlarmsLowTempValues[SenderUpDownTag] = (float)SenderUpDown->Value;

	// Reset the status label string and color of the temperature alarm
	GlobalVariables::TempAlarmsStatusLabels[SenderUpDownTag]->Text = "Normal";
	GlobalVariables::TempAlarmsStatusLabels[SenderUpDownTag]->ForeColor = System::Drawing::Color::White;

	// Reset the trigger status of the temperature alarm
	AlarmsTriggerStatusArray[SenderUpDownTag] = false;

}

void RMH_ThermalViewer_ChangeTempAlarmHighTempSetPoint(System::Object^ sender) {

	// This routine sets the high temperature value of a selected temperature alarm

	// Cast the sender object as a WinForms UpDown object
	System::Windows::Forms::NumericUpDown^ SenderUpDown = (System::Windows::Forms::NumericUpDown^)sender;

	// Read the identification tag of the form UpDown object
	unsigned char SenderUpDownTag = Convert::ToInt16(SenderUpDown->Tag);

	// Write the configured temperature alarm high value to the global array
	TempAlarmsHighTempValues[SenderUpDownTag] = (float)SenderUpDown->Value;

	// Reset the status label string and color of the temperature alarm
	GlobalVariables::TempAlarmsStatusLabels[SenderUpDownTag]->Text = "Normal";
	GlobalVariables::TempAlarmsStatusLabels[SenderUpDownTag]->ForeColor = System::Drawing::Color::White;

	// Reset the trigger status of the temperature alarm
	AlarmsTriggerStatusArray[SenderUpDownTag] = false;

}

void RMH_ThermalViewer_ChangeTempAlarmTriggerAction(System::Object^ sender) {

	// This routine sets the trigger action of the selected temperature alarm

	// Cast the sender object as a WinForms ComboBox object
	System::Windows::Forms::ComboBox^ ComboBox = (System::Windows::Forms::ComboBox^)sender;

	// Read the identification tag of the form ComboBox object
	unsigned char ComboBoxTag = Convert::ToInt16(ComboBox->Tag);

	// Write the configured temperature alarm high value to the global array
	TempAlarmsTriggerAction[ComboBoxTag] = ComboBox->SelectedIndex;

	// Reset the status label string and color of the temperature alarm
	GlobalVariables::TempAlarmsStatusLabels[ComboBoxTag]->Text = "Normal";
	GlobalVariables::TempAlarmsStatusLabels[ComboBoxTag]->ForeColor = System::Drawing::Color::White;

	// Reset the trigger status of the temperature alarm
	AlarmsTriggerStatusArray[ComboBoxTag] = false;

}

void RMH_ThermalViewer_EnableTemperatureAlarm(System::Object^ sender) {

	// This routine enables or disables a selected temperature alarm

	// Cast the sender object as a WinForms CheckBox object
	System::Windows::Forms::CheckBox^ CheckBox = (System::Windows::Forms::CheckBox^)sender;

	// Read the identification tag of the form CheckBox object
	unsigned char CheckBoxTag = Convert::ToInt16(CheckBox->Tag);

	// Update the enabled state of the temperature alarm
	EnabledTempAlarmsArray[CheckBoxTag] = (bool)CheckBox->Checked;

	// Reset the status label string and color of the temperature alarm
	GlobalVariables::TempAlarmsStatusLabels[CheckBoxTag]->Text = "Normal";
	GlobalVariables::TempAlarmsStatusLabels[CheckBoxTag]->ForeColor = System::Drawing::Color::White;

	// Reset the trigger status of the temperature alarm
	AlarmsTriggerStatusArray[CheckBoxTag] = false;

}

void RMH_ThermalViewer_UpdateTempAlarmsStatusLabels() {

	// This routine updates the status label of the temperature alarm depending on the triggered state

	// Loop up to and including the maximum number of active temperature alarms
	for (unsigned int i = 0; i < _MaxNumberOfConfigurableTempAlarms; i++) {

		// Is the selected temperature alarm enabled
		if (EnabledTempAlarmsArray[i] == true) {

			// Check whether the selected temperature alarm has been triggered
			if (AlarmsTriggerStatusArray[i] == true) {

				// Update the status label string and color of the temperature alarm
				GlobalVariables::TempAlarmsStatusLabels[i]->Text = "Triggered!";
				GlobalVariables::TempAlarmsStatusLabels[i]->ForeColor = System::Drawing::Color::Red;

			}
			else {

				// Update the status label string and color of the temperature alarm
				GlobalVariables::TempAlarmsStatusLabels[i]->Text = "Normal";
				GlobalVariables::TempAlarmsStatusLabels[i]->ForeColor = System::Drawing::Color::White;

			}

		}

	}

}

void RMH_ThermalViewer_ReadTemperatureAlarmStatus(unsigned char TemperatureAlarmIndex, double TemperatureAlarmSourcePointer) {

	// This routine checks whether an active temperature alarm has been triggered

	// Is the selected temperature alarm enabled
	if (EnabledTempAlarmsArray[TemperatureAlarmIndex] == true) {

		// Check the type of the temperature alarm
		switch (TempAlarmsConfigType[TemperatureAlarmIndex]) {

			// The temperature alarm is a "Trigger Above Temp" type alarm
			case _TempAlarmType_Above: 

				// Check whether the source data of the temperature alarm is higher than the high set point of the alarm
				if (TemperatureAlarmSourcePointer >= TempAlarmsHighTempValues[TemperatureAlarmIndex]) {

					// Update the trigger status of the temperature alarm
					AlarmsTriggerStatusArray[TemperatureAlarmIndex] = true;

				}
				else {

					// Update the trigger status of the temperature alarm
					AlarmsTriggerStatusArray[TemperatureAlarmIndex] = false;

				}
				
			break;

			// The temperature alarm is a "Trigger Below Temp" type alarm
			case _TempAlarmType_Below: 

				// Check whether the source data of the temperature alarm is lower than the low set point of the alarm
				if (TemperatureAlarmSourcePointer <= TempAlarmsLowTempValues[TemperatureAlarmIndex]) {

					// Update the trigger status of the temperature alarm
					AlarmsTriggerStatusArray[TemperatureAlarmIndex] = true;

				}
				else {

					// Update the trigger status of the temperature alarm
					AlarmsTriggerStatusArray[TemperatureAlarmIndex] = false;

				}
				
			break;

			// The temperature alarm is a "Temp Window Trigger" type alarm
			case _TempAlarmType_Window:

				// Check whether the source data of the temperature alarm is within the set temperature window
				if (TemperatureAlarmSourcePointer > TempAlarmsLowTempValues[TemperatureAlarmIndex] && TemperatureAlarmSourcePointer < TempAlarmsHighTempValues[TemperatureAlarmIndex]) {

					// Update the trigger status of the temperature alarm
					AlarmsTriggerStatusArray[TemperatureAlarmIndex] = false;

				}
				else {

					// Update the trigger status of the temperature alarm
					AlarmsTriggerStatusArray[TemperatureAlarmIndex] = true;

				}
	
			break;

		}

	}

}

void RMH_ThermalViewer_MonitorEnabledTempAlarmsStatus() {

	// This routine checks the triggered state of the active temperature alarms

	// Read and update the status of the temperature alarms
	RMH_ThermalViewer_ReadTemperatureAlarmStatus(_TemperatureAlarm_1, *TempAlarm1DataSourcePointer);
	RMH_ThermalViewer_ReadTemperatureAlarmStatus(_TemperatureAlarm_2, *TempAlarm2DataSourcePointer);
	RMH_ThermalViewer_ReadTemperatureAlarmStatus(_TemperatureAlarm_3, *TempAlarm3DataSourcePointer);
	RMH_ThermalViewer_ReadTemperatureAlarmStatus(_TemperatureAlarm_4, *TempAlarm4DataSourcePointer);
	RMH_ThermalViewer_ReadTemperatureAlarmStatus(_TemperatureAlarm_5, *TempAlarm5DataSourcePointer);

}

void RMH_ThermalViewer_OpdateTempAlarmTriggerSoundTimer(System::Object^ sender) {

	// This routine enables or disables the warning sound timer of the temperature alarms

	// Cast the sender object as a WinForms CheckBox object
	System::Windows::Forms::CheckBox^ CheckBox = (System::Windows::Forms::CheckBox^)sender;

	// Check whether the warning sound of the alarms should be enabled
	if (CheckBox->Checked == true) {

		// Update the active state of the warning sound timer of the temperature alarms
		TempAlarmTriggerSoundFlag = true;

		// Enable the warning sound timer of the temperature alarms
		GlobalVariables::GlobalAlarmSoundTimer->Enabled = true;

	}
	else {

		// Update the active state of the warning sound timer of the temperature alarms
		TempAlarmTriggerSoundFlag = false;

		// Disable the warning sound timer of the temperature alarms
		GlobalVariables::GlobalAlarmSoundTimer->Enabled = false;

	}
	
}

void RMH_ThermalViewer_AlarmSoundTimerTickEventHandler() {

	// This routine handles the trigger events of the warning sound timer of the temperature alarms 

	// Is the warning sound of the temperature alarms enabled
	if (TempAlarmTriggerSoundFlag == true) {

		// Check whether any alarms have been triggered
		if (AlarmsTriggerStatusArray[_TemperatureAlarm_1] == true ||
			AlarmsTriggerStatusArray[_TemperatureAlarm_2] == true ||
			AlarmsTriggerStatusArray[_TemperatureAlarm_3] == true ||
			AlarmsTriggerStatusArray[_TemperatureAlarm_4] == true ||
			AlarmsTriggerStatusArray[_TemperatureAlarm_5] == true) {

			// Play the alarm warning sound
			System::Media::SystemSounds::Hand->Play();

		}

	}

}

void RMH_ThermalViewer_EnableAlarmTriggerEvents(System::Object^ sender) {

	// This routine enables the trigger events of the temperature alarms

	// Cast the sender object as a WinForms CheckBox object
	System::Windows::Forms::CheckBox^ CheckBox = (System::Windows::Forms::CheckBox^)sender;

	// Update the enable flag of the trigger events of the temperature alarms
	TempAlarmsTriggerEventsEnableFlag = (bool)CheckBox->Checked;

	// Check the state of the checkbox
	if (TempAlarmsTriggerEventsEnableFlag == true) {

		// Configure the trigger timer interval 
		GlobalVariables::GlobalAlarmTriggerEventTimer->Interval = (unsigned int)(GlobalVariables::GlobalAlarmTriggerEventsIntervalUpDown->Value * 1000);

		// Enable the trigger event timer of the temperature alarms
		GlobalVariables::GlobalAlarmTriggerEventTimer->Enabled = true;

	}
	else {

		// Disable the trigger event timer of the temperature alarms
		GlobalVariables::GlobalAlarmTriggerEventTimer->Enabled = false;

	}

}

void RMH_ThermalViewer_UpdateAlarmsTriggerEventResetButtonsBorderColor(unsigned char TemperatureAlarmIndex, bool TriggerEventExecutedFlag) {

	// This routine updates the border color of the "Trigger event has been executed" status buttons

	// Selection of the temperature alarm index
	switch (TemperatureAlarmIndex) {

		// Update the border color of the temperature alarm 1 button
		case _TemperatureAlarm_1:  

			// Has the trigger event of the temp alarm been executed
			if (TriggerEventExecutedFlag == true) {

				// Update the button border color
				GlobalVariables::GlobalAlarm1TriggerEventResetButton->FlatAppearance->BorderColor = System::Drawing::Color::Red;

			}
			else {

				// Update the button border color
				GlobalVariables::GlobalAlarm1TriggerEventResetButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

			}
				
		break;

		// Update the border color of the temperature alarm 2 button
		case _TemperatureAlarm_2:

			// Has the trigger event of the temp alarm been executed
			if (TriggerEventExecutedFlag == true) {

				// Update the button border color
				GlobalVariables::GlobalAlarm2TriggerEventResetButton->FlatAppearance->BorderColor = System::Drawing::Color::Red;

			}
			else {

				// Update the button border color
				GlobalVariables::GlobalAlarm2TriggerEventResetButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

			}

		break;

		// Update the border color of the temperature alarm 3 button
		case _TemperatureAlarm_3:

			// Has the trigger event of the temp alarm been executed
			if (TriggerEventExecutedFlag == true) {

				// Update the button border color
				GlobalVariables::GlobalAlarm3TriggerEventResetButton->FlatAppearance->BorderColor = System::Drawing::Color::Red;

			}
			else {

				// Update the button border color
				GlobalVariables::GlobalAlarm3TriggerEventResetButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

			}

		break;

		// Update the border color of the temperature alarm 4 button
		case _TemperatureAlarm_4:

			// Has the trigger event of the temp alarm been executed
			if (TriggerEventExecutedFlag == true) {

				// Update the button border color
				GlobalVariables::GlobalAlarm4TriggerEventResetButton->FlatAppearance->BorderColor = System::Drawing::Color::Red;

			}
			else {

				// Update the button border color
				GlobalVariables::GlobalAlarm4TriggerEventResetButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

			}

		break;

		// Update the border color of the temperature alarm 5 button
		case _TemperatureAlarm_5:

			// Has the trigger event of the temp alarm been executed
			if (TriggerEventExecutedFlag == true) {

				// Update the button border color
				GlobalVariables::GlobalAlarm5TriggerEventResetButton->FlatAppearance->BorderColor = System::Drawing::Color::Red;

			}
			else {

				// Update the button border color
				GlobalVariables::GlobalAlarm5TriggerEventResetButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

			}

		break;

	}

}

void RMH_ThermalViewer_ResetAlarmTriggerEventExecutedFlag(System::Object^ sender) {

	// This routine resets the "Trigger event has been executed" flag for the alarm tag read

	// Cast the sender object as a WinForms Button object
	System::Windows::Forms::Button^ TriggerResetButton = (System::Windows::Forms::Button^)sender;

	// Read the identification tag of the pressed button
	unsigned int TriggerEventResetButtonTag = Convert::ToInt32(TriggerResetButton->Tag);

	// Reset the "Trigger event has been executed" flag for the associated temperature alarm
	TriggerEventExecutedFlag[TriggerEventResetButtonTag] = false;

	// Update the border color of the associated trigger event reset button
	RMH_ThermalViewer_UpdateAlarmsTriggerEventResetButtonsBorderColor(TriggerEventResetButtonTag, TriggerEventExecutedFlag[TriggerEventResetButtonTag]);

}

void RMH_ThermalViewer_HandleTempAlarmTriggerActionEvent(unsigned char TemperatureAlarmIndex) {

	// This routine handles the trigger action event of a triggered alarm

	// Check whether the selected temperature alarm is enabled
	if (EnabledTempAlarmsArray[TemperatureAlarmIndex] == true) {

		// Check whether the temperature alarm has been triggered
		if (AlarmsTriggerStatusArray[TemperatureAlarmIndex] == true) {

			// Execute the trigger event only if it has not already been triggered
			// or if the trigger event "has been executed" flag is 'false'
			if (TriggerEventExecutedFlag[TemperatureAlarmIndex] == false) {

				// Check the alarm trigger action configuration
				switch (TempAlarmsTriggerAction[TemperatureAlarmIndex]) {

					// -------------------------------------------------------------------------- //

					// Execute the alarm trigger action
					case _TempAlarmTriggerAction_None: break;

					// -------------------------------------------------------------------------- //

					case _TempAlarmTriggerAction_StartDataLogging:

						// Start temperature data logging session
						RMH_ThermalViewer_StartDataLogging();

					break;

					// -------------------------------------------------------------------------- //

					case _TempAlarmTriggerAction_StopDataLogging:

						// Stop temperature data logging session
						RMH_ThermalViewer_StopDataLogging();

					break;

					// -------------------------------------------------------------------------- //

					case _TempAlarmTriggerAction_StartVideoRecording:

						// Update the video recording flag - start recording
						VideoRecordingStartedFlag = true;
						// Start video recording
						RMH_ThermalViewer_StartStopVideoRecording();

					break;

					// -------------------------------------------------------------------------- //

					case _TempAlarmTriggerAction_StopVideoRecording:

						// Update the video recording flag - stop recording
						VideoRecordingStartedFlag = false;
						// Stop video recording
						RMH_ThermalViewer_StartStopVideoRecording();

					break;

					// -------------------------------------------------------------------------- //

					case _TempAlarmTriggerAction_SaveSnapshot:

						// Save a live view snapshot
						RMH_ThermalViewer_SaveLiveViewSnapshot();

					break;

					// -------------------------------------------------------------------------- //

					case _TempAlarmTriggerAction_SaveFullFrameTempData:

						// Generate and save a full-frame temperature data CSV file
						RMH_ThermalViewer_SaveFullFrameTemperatureDataToCSVFile();

					break;

					// -------------------------------------------------------------------------- //

				}

				// Update the alarm trigger event "has been executed" flag
				TriggerEventExecutedFlag[TemperatureAlarmIndex] = true;

				// Update the border color of the associated trigger event reset button
				RMH_ThermalViewer_UpdateAlarmsTriggerEventResetButtonsBorderColor(TemperatureAlarmIndex, TriggerEventExecutedFlag[TemperatureAlarmIndex]);

			}

		}

	}

}

void RMH_ThermalViewer_AlarmTriggerEventTimerTickEventHandler() {

	// This routine handles the trigger events of the trigger event timer of the temperature alarms 

	// Execute the trigger events of the active temperature alarms
	RMH_ThermalViewer_HandleTempAlarmTriggerActionEvent(_TemperatureAlarm_1);
	RMH_ThermalViewer_HandleTempAlarmTriggerActionEvent(_TemperatureAlarm_2);
	RMH_ThermalViewer_HandleTempAlarmTriggerActionEvent(_TemperatureAlarm_3);
	RMH_ThermalViewer_HandleTempAlarmTriggerActionEvent(_TemperatureAlarm_4);
	RMH_ThermalViewer_HandleTempAlarmTriggerActionEvent(_TemperatureAlarm_5);

}

// ------------------- General And Periodic Trigger Handling Routines -------------------- //

void RMH_ThermalViewer_TogglePeriodicTriggerTimer() {

	// This routine enables or disables the periodic trigger timer

	// Toggle the enable flag of the periodic trigger timer
	PeriodicTriggerTimerEnableFlag = !PeriodicTriggerTimerEnableFlag;

	// If the live view stream is in STOP mode
	if (LiveViewRunStopFlag == false) {

		// Reset the enable flag of the periodic trigger timer
		PeriodicTriggerTimerEnableFlag = false;

	}

	// Handle the new state of the enable flag
	if (PeriodicTriggerTimerEnableFlag == true) {

		// Enable the periodic trigger timer
		GlobalVariables::GlobalPeriodicTriggerTimer->Enabled = true;

		// Set the execution interval of the trigger timer
		GlobalVariables::GlobalPeriodicTriggerTimer->Interval = 1000;

		// Update the button border color
		GlobalVariables::GlobalPeriodicTimerTriggerButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Disable the periodic trigger timer
		GlobalVariables::GlobalPeriodicTriggerTimer->Enabled = false;

		// Reset the button border color
		GlobalVariables::GlobalPeriodicTimerTriggerButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	}

}

void RMH_ThermalViewer_EnableDisableSelectedPeriodicTriggerEvent(System::Object^ sender) {

	// This routine enables or disables the selected periodic trigger event 

	// Cast the sender object as a WinForms CheckBox object
	System::Windows::Forms::CheckBox^ CheckBoxTag = (System::Windows::Forms::CheckBox^)sender;

	// Read the identification tag of the selected CheckBox
	unsigned int TriggerEventResetButtonTag = Convert::ToInt32(CheckBoxTag->Tag);

	// Enable or disable the selected periodic trigger event
	PeriodicTriggerEventEnableFlags[TriggerEventResetButtonTag] = GlobalVariables::GlobalPeriodicEventnEnableCheckBox[TriggerEventResetButtonTag]->Checked;

	// If the periodic trigger timer has been enabled
	if (GlobalVariables::GlobalPeriodicEventnEnableCheckBox[TriggerEventResetButtonTag]->Checked == true) {

		// Disable the associated configuration GUI elements
		GlobalVariables::GlobalPeriodicEventnComboBox[TriggerEventResetButtonTag]->Enabled = false;
		GlobalVariables::GlobalPeriodicEventIntervalnUpDown[TriggerEventResetButtonTag]->Enabled = false;
		GlobalVariables::GlobalPeriodicEventnDisableCheckBox[TriggerEventResetButtonTag]->Enabled = false;

	}
	else {

		// Enable the associated configuration GUI elements
		GlobalVariables::GlobalPeriodicEventnComboBox[TriggerEventResetButtonTag]->Enabled = true;
		GlobalVariables::GlobalPeriodicEventIntervalnUpDown[TriggerEventResetButtonTag]->Enabled = true;
		GlobalVariables::GlobalPeriodicEventnDisableCheckBox[TriggerEventResetButtonTag]->Enabled = true;

	}

}

void RMH_ThermalViewer_DisablePeriodicTriggerEventIndex(unsigned int PeriodicTriggerEvent) {

	// This routine disables the selected periodic trigger event 

	/*
	
		Calculate the normalized Gaussian kernel value

		// Periodic Trigger Event Number Macros
		#define _PeriodicTriggerEvent_1                               0
		#define _PeriodicTriggerEvent_2                               1
		#define _PeriodicTriggerEvent_3                               2
		#define _PeriodicTriggerEvent_4                               3
		#define _PeriodicTriggerEvent_5                               4
	
	*/

	// Disable the selected periodic trigger event
	PeriodicTriggerEventEnableFlags[PeriodicTriggerEvent] = false;
	GlobalVariables::GlobalPeriodicEventnEnableCheckBox[PeriodicTriggerEvent]->Checked = false;

}

void RMH_ThermalViewer_ExecuteTriggerEventIndex(unsigned short TriggerEventFunction) {

	// This routine executes the selected trigger event function from the given event function index

	// Check which periodic trigger event function has been set
	switch (GlobalVariables::GlobalPeriodicEventnComboBox[TriggerEventFunction]->SelectedIndex) {

		// -------------------------------------------------------------------------- //

		// Execute the associated periodic trigger event function
		case _TriggerEventFunction_None: 
			
			// Disable the selected periodic trigger event 
			RMH_ThermalViewer_DisablePeriodicTriggerEventIndex(TriggerEventFunction);

		break;

		// Execute the associated periodic trigger event function
		case _TriggerEventFunction_StartDataLogging:

			// Start temperature data logging session
			RMH_ThermalViewer_StartDataLogging();

		break;

		// Execute the associated periodic trigger event function
		case _TriggerEventFunction_StopDataLogging:

			// Stop temperature data logging session
			RMH_ThermalViewer_StopDataLogging();

		break;

		// Execute the associated periodic trigger event function
		case _TriggerEventFunction_StartVideoRecording:

			// Update the video recording flag - start recording
			VideoRecordingStartedFlag = true;
			// Start video recording
			RMH_ThermalViewer_StartStopVideoRecording();

		break;

		// Execute the associated periodic trigger event function
		case _TriggerEventFunction_StopVideoRecording:

			// Update the video recording flag - stop recording
			VideoRecordingStartedFlag = false;
			// Stop video recording
			RMH_ThermalViewer_StartStopVideoRecording();

		break;

		// Execute the associated periodic trigger event function
		case _TriggerEventFunction_SaveSnapshot:

			// Save a live view snapshot
			RMH_ThermalViewer_SaveLiveViewSnapshot();

		break;

		// Execute the associated periodic trigger event function
		case _TriggerEventFunction_SaveFullFrameTempData:

			// Generate and save a full-frame temperature data CSV file
			RMH_ThermalViewer_SaveFullFrameTemperatureDataToCSVFile();

		break;

		// -------------------------------------------------------------------------- //

	}

}

void RMH_ThermalViewer_ExecuteSelectedPeriodicTriggerEvent(unsigned int PeriodicTriggerEvent) {

	// This routine executes the configured periodic trigger event, if enabled.

	/*

		Calculate the normalized Gaussian kernel value

		// Periodic Trigger Event Number Macros
		#define _PeriodicTriggerEvent_1                               0
		#define _PeriodicTriggerEvent_2                               1
		#define _PeriodicTriggerEvent_3                               2
		#define _PeriodicTriggerEvent_4                               3
		#define _PeriodicTriggerEvent_5                               4

	*/

	// Check whether the selected periodic trigger event is enabled
	if (PeriodicTriggerEventEnableFlags[PeriodicTriggerEvent] == true) {

		// Increment the timeout counter variable of the selected periodic trigger event
		PeriodicEventTriggerCounter[PeriodicTriggerEvent] = PeriodicEventTriggerCounter[PeriodicTriggerEvent] + 1;

		// Check whether the selected periodic trigger event has reached its set interval value
		if (PeriodicEventTriggerCounter[PeriodicTriggerEvent] >= GlobalVariables::GlobalPeriodicEventIntervalnUpDown[PeriodicTriggerEvent]->Value) {

			// Reset the timeout counter variable of the selected periodic trigger event
			PeriodicEventTriggerCounter[PeriodicTriggerEvent] = 0;

			// Execute the configured periodic trigger event function
			RMH_ThermalViewer_ExecuteTriggerEventIndex(PeriodicTriggerEvent);

			// Check whether the trigger event execution should be reset after the first execution
			if (GlobalVariables::GlobalPeriodicEventnDisableCheckBox[PeriodicTriggerEvent]->Checked == true) {

				// Disable the selected periodic trigger event 
				RMH_ThermalViewer_DisablePeriodicTriggerEventIndex(PeriodicTriggerEvent);

			}

		}

	}
	else {

		// Reset the timeout counter variable of the selected periodic trigger event
		PeriodicEventTriggerCounter[PeriodicTriggerEvent] = 0;

	}

}

void RMH_ThermalViewer_PeriodicTriggerEventTimerTickEventHandler() {

	// This routine handles the events of the periodic trigger event timer 

	// Execute all enabled periodic trigger event functions
	RMH_ThermalViewer_ExecuteSelectedPeriodicTriggerEvent(_PeriodicTriggerEvent_1);
	RMH_ThermalViewer_ExecuteSelectedPeriodicTriggerEvent(_PeriodicTriggerEvent_2);
	RMH_ThermalViewer_ExecuteSelectedPeriodicTriggerEvent(_PeriodicTriggerEvent_3);
	RMH_ThermalViewer_ExecuteSelectedPeriodicTriggerEvent(_PeriodicTriggerEvent_4);
	RMH_ThermalViewer_ExecuteSelectedPeriodicTriggerEvent(_PeriodicTriggerEvent_5);
	
	// Check whether all trigger events are disabled
	if (PeriodicTriggerEventEnableFlags[_PeriodicTriggerEvent_1] == false &&
		PeriodicTriggerEventEnableFlags[_PeriodicTriggerEvent_2] == false &&
		PeriodicTriggerEventEnableFlags[_PeriodicTriggerEvent_3] == false &&
		PeriodicTriggerEventEnableFlags[_PeriodicTriggerEvent_4] == false &&
		PeriodicTriggerEventEnableFlags[_PeriodicTriggerEvent_5] == false) {

		// Disable the periodic trigger events
		PeriodicTriggerTimerEnableFlag = true;
		RMH_ThermalViewer_TogglePeriodicTriggerTimer();

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "No Periodic Trigger Events Are Enabled...", _StatusMessageType_Warning);

	}

}

// ---------------------- Emissivity Table Screen Handling Routines ----------------------- //

void RMH_ThermalViewer_LoadEmissivisyTableValueToThermalCamera(System::Windows::Forms::DataGridViewCellEventArgs^ e) {

	// This routine loads the selected emissivity table value into the thermal camera and updates the associated GUI elements

	// Read the temporary array data and sort the kernel array
	float DataGridViewRowIndex;
	bool SetNewEmissivityConfigFlag = true;

	// Read which row cell has been clicked 
	DataGridViewRowIndex = e->RowIndex;

	// Check for the minimum emissivity table index 
	if (DataGridViewRowIndex < 0.0) {

		// Set to the lowest allowed emissivity table index 
		DataGridViewRowIndex = 0;

		// Update the new emissivity config flag
		SetNewEmissivityConfigFlag = false;

	}

	// Check for the maximum emissivity table index 
	if (DataGridViewRowIndex >= _EmissivityTableNumberOfElements) {

		// Set to the highest allowed emissivity table index 
		DataGridViewRowIndex = _EmissivityTableNumberOfElements - 1;

		// Update the new emissivity config flag
		SetNewEmissivityConfigFlag = false;

	}

	// Should a new emissivity configuration be written to the camera
	if (SetNewEmissivityConfigFlag == true) {

		// Write the selected emissivity value to the "Thermal Camera Configuration" menu UpDown
		GlobalVariables::CameraConfigNumericUpDowns[4]->Value = (System::Decimal)MaterialEmissivityValues[(unsigned int)DataGridViewRowIndex];

		// Write/set the configured camera configuration parameters to the camera memory
		RMH_ThermalViewer_SetCameraConfigParameters();

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Material Sellected: " + EmissivityMaterialNames[(unsigned int)DataGridViewRowIndex], _StatusMessageType_Normal);
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Material Emissivity: " + RMH_Conversion_FloatToStdString(MaterialEmissivityValues[(unsigned int)DataGridViewRowIndex], 2), _StatusMessageType_Normal);
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "New Emissivity Value Has Been Set!", _StatusMessageType_Success);

	}

}

// --------------------- Camera Auto Calibration Handling Routines ---------------------- //

void RMH_ThermalViewer_ToggleCameraAutoShutterCalibrationTimer() {

	// This routine enables or disables the auto calibration feature timer

	// Toggle the auto calibration enable flag
	AutoShutterCalEnableFlag = !AutoShutterCalEnableFlag;

	// Should automatic shutter calibration be enabled or disabled
	if (AutoShutterCalEnableFlag == true) {

		// Update the button border color
		GlobalVariables::GlobalAutoShutterCalButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

		// Configure the timer period
		GlobalVariables::GlobalAutoCalTimer->Interval = ((unsigned int)GlobalVariables::GlobalAutoCalPeriodUpDown->Value) * 1000;

		// Enable the auto calibration timer
		GlobalVariables::GlobalAutoCalTimer->Enabled = true;

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Automatic Shutter Calibration Is Enabled", _StatusMessageType_Success);

	}
	else {

		// Reset the button border color
		GlobalVariables::GlobalAutoShutterCalButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

		// Disable the auto calibration timer
		GlobalVariables::GlobalAutoCalTimer->Enabled = false;

	}

	// Update the button graphic
	GlobalVariables::GlobalAutoShutterCalButton->Refresh();

}

bool RMH_ThermalViewer_AutoStartSelectAndConnectCamera() {

	// This routine is part of the automatic start-up sequence - it selects the configured thermal camera in the camera
	// ComboBox and connects to it (the same as pressing the "Connect" button, without the information dialog)
	// The routine returns true when the camera is connected

	// Find the position of the configured camera in the camera selection ComboBox
	int CameraPosition = RMH_FindCameraDisplayPosition(_AutoStart_CameraIndex);

	// Was the camera found in the list
	if (CameraPosition < 0) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Auto Start: The Configured Thermal Camera Is Not In The Camera List.", _StatusMessageType_Warning);

		return false;

	}

	// Is the camera not already selected
	if (GlobalVariables::GlobalCameraSourceDropList->SelectedIndex != CameraPosition) {

		// Select the camera in the camera selection ComboBox
		GlobalVariables::GlobalCameraSourceDropList->SelectedIndex = CameraPosition;

	}

	// Write GUI status message
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Auto Start: Connecting To The Thermal Camera...", _StatusMessageType_Normal);

	// Connect to the selected thermal camera
	RMH_IRThermalCamera_ConnectToThermalCameraOrAnalysisMode();

	// Connecting clears the status message area - show the custom build message again
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, _CustomBuild_StartupMessage, _StatusMessageType_Success);

	// Return whether the camera is connected
	return (IRCamera.ConnectedFlag == true);

}

bool RMH_ThermalViewer_AutoStartEnablePeriodicCalibration() {

	// This routine is part of the automatic start-up sequence - it enables periodic shutter calibration with the configured period
	// (the camera itself is calibrated before this by pressing the "Calibrate Camera" button of the stream window)
	// The routine returns true when the sequence was executed

	// Is no thermal camera connected
	if (IRCamera.ConnectedFlag == false) { return false; }

	// Set the automatic shutter calibration period
	GlobalVariables::GlobalAutoCalPeriodUpDown->Value = System::Decimal(_AutoStart_PeriodicCalibrationPeriodSec);

	// Is periodic shutter calibration not already enabled
	if (AutoShutterCalEnableFlag == false) {

		// Enable periodic shutter calibration (the same as pressing the "Press The Button To Enable periodic Shutter Calibration" button)
		RMH_ThermalViewer_ToggleCameraAutoShutterCalibrationTimer();

	}

	return true;

}

void RMH_ThermalViewer_ToggleCameraDriftBasedCalibrationTimer() {

	// This routine enables or disables the temperature-drift-based calibration feature 

	// Toggle the temperature-drift-based calibration enable flag
	DriftBasedCalEnableFlag = !DriftBasedCalEnableFlag;

	// Should temperature-drift-based calibration be enabled or disabled
	if (DriftBasedCalEnableFlag == true) {

		// Update the button border color
		GlobalVariables::GlobalSensorDriftCalButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

		// Configure the timer period
		GlobalVariables::GlobalDriftCalTimer->Interval = 2000;

		// Enable the auto calibration timer
		GlobalVariables::GlobalDriftCalTimer->Enabled = true;

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Temperature Drift Base Calibration Is Enabled", _StatusMessageType_Success);

	}
	else {

		// Reset the button border color
		GlobalVariables::GlobalSensorDriftCalButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

		// Disable the auto calibration timer
		GlobalVariables::GlobalDriftCalTimer->Enabled = false;

	}

	// Update the button graphic
	GlobalVariables::GlobalAutoShutterCalButton->Refresh();

}

void RMH_ThermalViewer_ReadThermalCameraInternalTemps() {

	// This routine reads and shows the internal detector, core and shutter temperatures of the camera

	// Read the temporary array data and sort the kernel array
	float CameraDetectorTemp = (IRCamera.temp_fpa * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;
	float CameraCoreTemp = (IRCamera.temp_core * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;
	float CameraShutterTemp = (IRCamera.temp_shutter * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;
	
	// Update the internal temperature labels of the camera
	GlobalVariables::GlobalCameraDetectorTempLabel->Text = "Camera Detector: " + CameraDetectorTemp.ToString("F3") + GlobalVariables::DefaultTempUnitString;
	GlobalVariables::GlobalCameraCoreTempLabel->Text = "Camera Core: " + CameraCoreTemp.ToString("F3") + GlobalVariables::DefaultTempUnitString;
	GlobalVariables::GlobalCameraShutterTempLabel->Text = "Camera Shutter: " + CameraShutterTemp.ToString("F3") + GlobalVariables::DefaultTempUnitString;

}

// ---------------- Camera Disconnect Or Mode Change Handling Routines ---------------- //

void RMH_ThermalViewer_HandleSellectedDeviceOrModeChange() {

	// This routine handles the actions when the camera source ComboBox item changes

	// ----------------------------------- Reset Temperature Range States ----------------------------------- //

	// Reset the thermal camera high-range flag
	ThermalCameraHighRangeFlag = false;

	// Update the temperature range button border color
	GlobalVariables::GlobalTempRangeButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	// Update the IR camera device temperature range variable
	IRCamera.CurrentIRTempRangeFlag = 1;

	// -------------------------------------------------------------------------------------------------------- //

	// Disable the camera configuration GUI components
	RMH_ThermalViewer_EnableCameraConfigurationControls(false);

	// Reset the auto calibration enable flag - false after execution of the associated routine
	AutoShutterCalEnableFlag = true;
	// Disable the automatic shutter calibration feature timer
	RMH_ThermalViewer_ToggleCameraAutoShutterCalibrationTimer();
	// Disable the auto shutter calibration button in the settings menu
	GlobalVariables::GlobalAutoShutterCalButton->Enabled = false;

	// Disable the camera disconnect button
	GlobalVariables::GlobalDisconnectButton->Enabled = false;
	// Disable the calibration button in the live view tools panel
	GlobalVariables::GlobalCalibrateCameraButton->Enabled = false;
	// Disable the temperature-drift-based calibration button in the settings menu
	GlobalVariables::GlobalSensorDriftCalButton->Enabled = false;
	// Disable the temperature range button in the live view tools panel
	GlobalVariables::GlobalTempRangeButton->Enabled = false;
	// Disable the recording button in the live view tools panel
	GlobalVariables::GlobalRecordingButton->Enabled = false;

	// Check whether the video playback form is open
	if (VideoPlaybackControlsFormIsOpenFlag == true) {

		// Disable the play, forward and back buttons
		GlobalVariables::VideoPlaybackForwardStepButton->Enabled = false;
		GlobalVariables::VideoPlaybackBackwardStepButton->Enabled = false;
		GlobalVariables::VideoPlaybackPlayStopButton->Enabled = false;

	}

	// Loop through the whole array of menu buttons of the main form GUI
	for (unsigned int i = 0; i < GlobalVariables::MainGUILeftMenuButtons->Length; i++) {

		// Disable the left menu buttons of the main GUI form - except the "Settings" menu button
		GlobalVariables::MainGUILeftMenuButtons[i]->Enabled = false;

	}

	// Disable the GUI update timer
	GlobalVariables::GlobalMainGUIUpdateTimer->Enabled = false;
	GlobalVariables::GlobalMainGUIUpdateTimer->Stop();

	// Check whether a camera was connected
	if (IRCamera.ConnectedFlag == true) {
		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Has Been Disconnected And The Video Stream Has Stopped!", _StatusMessageType_Normal);
	}

	// Reset the camera "isStreaming" status flag
	IRCamera.isStreaming = false;
	// Update the camera connect status flag
	IRCamera.ConnectedFlag = false;

	// Only if "Recording Analysis" mode is not active
	if (InRecordingAnalysisModeFlag == false && InSnapShotAnalysisModeFlag == false) {

		// Stop the camera video capture
		RMH_IRThermalCamera_StopCapturing();
		// Stop video capture and close the camera - if a camera is active
		RMH_IRThermalCamera_CloseIRCameraDevice();

	}

	// Is "Snapshot Analysis" mode selected
	if (GlobalVariables::GlobalCameraSourceDropList->SelectedIndex == _SnapShotAnalysisMode) { 

		// Update the connect button label text
		GlobalVariables::GlobalConnectButton->Text = L"Click To\r\nBrowse And Open\r\nSnapShot File";
		// Update the connect button border color 
		GlobalVariables::GlobalConnectButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	}
	else if (GlobalVariables::GlobalCameraSourceDropList->SelectedIndex == _RecordingAnalysisMode) { // Is "Recording Analysis" mode selected

		// Update the connect button label text
		GlobalVariables::GlobalConnectButton->Text = L"Click To\r\nBrowse And Open\r\nVideo File";
		// Update the connect button border color 
		GlobalVariables::GlobalConnectButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	}
	else {

		// Reset the connect button label text
		GlobalVariables::GlobalConnectButton->Text = L"Connect";
		// Update the connect button border color 
		GlobalVariables::GlobalConnectButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	}

	// Close the "Recording Analysis" mode video file, if it is open
	RMH_VideoFileReading_CloseRecordingAnalysisModeFile();

	// If "Recording Analysis" mode is active
	if (InRecordingAnalysisModeFlag == true) {

		// Close the video playback controls panel form
		CloseVideoPlayBackControlsFormFlag = true;

	}

	// Reset the "in Recording Analysis mode" flag
	InRecordingAnalysisModeFlag = false;
	// Reset the "in Snapshot Analysis mode" flag
	InSnapShotAnalysisModeFlag = false;

}

void RMH_ThermalViewer_HandleCameraDisconnectedEvents(bool ShowStatusMEssageFlag) {

	// This routine handles the events when the camera connection is lost during video streaming

	// Must not be done in "Recording Analysis" mode and "Snapshot Analysis" mode
	if (InRecordingAnalysisModeFlag == false && InSnapShotAnalysisModeFlag == false) {

		// Check whether the camera connection was lost
		if (RMH_IRThermalCamera_CheckForCameraDisconnection()) {

			// Write GUI status message
			if (ShowStatusMEssageFlag == true) { RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Connection To The Thermal Camera Was Lost!", _StatusMessageType_Error); }

			// Handle the GUI state on lost camera connection or "mode" change
			RMH_ThermalViewer_HandleSellectedDeviceOrModeChange();

		}

	}

}

// --------------------- Camera Temperature Unit Handling Routines ---------------------- //

void RMH_ThermalViewer_ChangeTemperatureUnit(System::Object^ sender) {

	// This routine handles the temperature unit buttons, as a nested callback for all three buttons.
	// The routine also handles the events and actions when the temperature unit is changed, for all temperature measurements.

	// Cast the sender object as a WinForms Button object
	System::Windows::Forms::Button^ PressedTempUnitButton = (System::Windows::Forms::Button^)sender;

	// Read the identification tag of the pressed button
	unsigned int ButtonTag = Convert::ToInt32(PressedTempUnitButton->Tag);

	// Check that the newly selected temperature unit is not the current one
	if (TempUnitState != ButtonTag) {

		// Which button has been pressed - read the button tag
		switch (ButtonTag) {

			// Celsius button
			case 1:

				// Update the button border color
				GlobalVariables::TempUnitButtons[0]->FlatAppearance->BorderColor = System::Drawing::Color::Lime;
				GlobalVariables::TempUnitButtons[1]->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);
				GlobalVariables::TempUnitButtons[2]->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

				// Update the scaling factor of the temperature unit from Celsius
				TemperatureUnitScaleFactor = 1.0;
				// Update the offset value of the temperature unit from Celsius
				TemperatureUnitOffsetFactor = 0.0;

				// Update the temp unit status old state
				TempUnitOldstate = TempUnitState;
				// Update the temperature unit status value
				TempUnitState = 1;

				// Update the default temperature unit string
				GlobalVariables::DefaultTempUnitString = "°C";

				// Write GUI start message
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Temperature Measurement Unit Changed To: Celsius.", _StatusMessageType_Normal);

			break;

			// Fahrenheit button
			case 2:

				// Update the button border color
				GlobalVariables::TempUnitButtons[0]->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);
				GlobalVariables::TempUnitButtons[1]->FlatAppearance->BorderColor = System::Drawing::Color::Lime;
				GlobalVariables::TempUnitButtons[2]->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

				// Update the scaling factor of the temperature unit from Celsius
				TemperatureUnitScaleFactor = 1.8;
				// Update the offset value of the temperature unit from Celsius
				TemperatureUnitOffsetFactor = 32.0;

				// Update the temp unit status old state
				TempUnitOldstate = TempUnitState;
				// Update the temperature unit status value
				TempUnitState = 2;

				// Update the default temperature unit string
				GlobalVariables::DefaultTempUnitString = "°F";

				// Write GUI start message
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Temperature Measurement Unit Changed To: Fahrenheit.", _StatusMessageType_Normal);

			break;

			// Kelvin button
			case 3:

				// Update the button border color
				GlobalVariables::TempUnitButtons[0]->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);
				GlobalVariables::TempUnitButtons[1]->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);
				GlobalVariables::TempUnitButtons[2]->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

				// Update the scaling factor of the temperature unit from Celsius
				TemperatureUnitScaleFactor = 1.0;
				// Update the offset value of the temperature unit from Celsius
				TemperatureUnitOffsetFactor = 273.15;

				// Update the temp unit status old state
				TempUnitOldstate = TempUnitState;
				// Update the temperature unit status value
				TempUnitState = 3;

				// Update the default temperature unit string
				GlobalVariables::DefaultTempUnitString = "K";

				// Write GUI start message
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Temperature Measurement Unit Changed To: Kelvin.", _StatusMessageType_Normal);

			break;

		}

		// --------------- Update GUI Elements To The Selected Temperature Unit --------------- //

		// ----- Camera Configuration Panel ----->

		// Update the maximum and minimum limits of the NumericUpDowns of the camera configuration panel
		RMH_ThermalViewer_SetCameraConfigUpDownRanges(TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor);

		// Update the parameter values of the camera configuration NumericUpDowns
		RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[1], IRCamera.ReflectedTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_ReflectedTemperatureValue);
		RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[2], IRCamera.AmbientTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_AmbientTemperatureValue);

		// Update the temperature unit string of the labels of the camera configuration panel
		GlobalVariables::CameraConfigLabels[0]->Text = "Temperature Correction [" + GlobalVariables::DefaultTempUnitString + "]:";
		GlobalVariables::CameraConfigLabels[1]->Text = "Ambient Temperature [" + GlobalVariables::DefaultTempUnitString + "]:";
		GlobalVariables::CameraConfigLabels[2]->Text = "Reflected Temperature [" + GlobalVariables::DefaultTempUnitString + "]:";

		// ----- Video PlayBack Controls Panel ----->

		// Update only if the video playback form is open
		if (VideoPlaybackControlsFormIsOpenFlag == true) {

			// Update the temperature unit string of the video playback controls labels
			GlobalVariables::VideoPlaybackTempCorrectionLabel->Text = RMH_Conversion_FloatToSystemString(IRCamera.TemperatureCorrectionSetting) + " " + GlobalVariables::DefaultTempUnitString;
			GlobalVariables::VideoPlaybackAmbientTempLabel->Text = RMH_Conversion_FloatToSystemString(IRCamera.AmbientTemperatureSetting * TemperatureUnitScaleFactor + TemperatureUnitOffsetFactor) + " " + GlobalVariables::DefaultTempUnitString;
			GlobalVariables::VideoPlaybackReflectedTempLabel->Text = RMH_Conversion_FloatToSystemString(IRCamera.ReflectedTemperatureSetting * TemperatureUnitScaleFactor + TemperatureUnitOffsetFactor) + " " + GlobalVariables::DefaultTempUnitString;

		}

		// ------ Live View Statistics Window ------->
		
		// Reset the maximum peak value
		MaxPeakTemperature = 0;
		// Reset the minimum peak value
		MinPeakTemperature = 2000.0;

		// Update the maximum drift temperature label in the calibration settings menu
		GlobalVariables::GlobalMaxTempDriftSetPountLabel->Text = "Maximum Drift Temperature [" + GlobalVariables::DefaultTempUnitString + "]:";

		// --------------------------------------------------------------------------------- //

	}

}

// ----------------------- Temperature Tracking Handling Routines ------------------------ //

void RMH_ThermalViewer_ReadMaxMinCentTemperatures() {

	// This routine reads the maximum, minimum and center temperatures 

	// Read the maximum, minimum and center temperatures and compensate for the selected temperature unit
	MaximumTemperature = (RMH_IRThermalCamera_ReadPixelTemperature(&IRCamera, IRCamera.Tmax_Tmp_Raw, IRCamera.ThermalCameraSupportPool) * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;
	MinimumTemperature = (RMH_IRThermalCamera_ReadPixelTemperature(&IRCamera, IRCamera.Tmin_Tmp_Raw, IRCamera.ThermalCameraSupportPool) * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;
	CenterTemperature = (RMH_IRThermalCamera_ReadPixelTemperature(&IRCamera, IRCamera.Center_Tmp_Raw, IRCamera.ThermalCameraSupportPool) * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;

	// Calculate/convert the raw average thermal frame data value to an actual temperature
	AverageTemperature = (RMH_IRThermalCamera_ReadPixelTemperature(&IRCamera, IRCamera.Tavg_Tmp_Raw, IRCamera.ThermalCameraSupportPool) * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;

}

void RMH_ThermalViewer_FormatMaximumTemperatureLabel() {

	// This routine formats an associated label for rendering on the live view

	// Format the maximum temperature label for live view rendering
	GlobalVariables::MaximumTempLabel = "Max: " + MaximumTemperature.ToString(GlobalVariables::TemperaturePrecision) + " " + GlobalVariables::DefaultTempUnitString;

}

void RMH_ThermalViewer_FormatMinimumTemperatureLabel() {

	// This routine formats an associated label for rendering on the live view

	// Format the minimum temperature label for live view rendering
	GlobalVariables::MinimumTempLabel = "Min: " + MinimumTemperature.ToString(GlobalVariables::TemperaturePrecision) + " " + GlobalVariables::DefaultTempUnitString;

}

void RMH_ThermalViewer_FormatCenterTemperatureLabel() {

	// This routine formats an associated label for rendering on the live view

	// Format the center temperature label for live view rendering
	GlobalVariables::CenterTempLabel = "Center: " + CenterTemperature.ToString(GlobalVariables::TemperaturePrecision) + " " + GlobalVariables::DefaultTempUnitString;

}

void RMH_ThermalViewer_ReadAndFormatMouseCursorTempAndLabel() {

	// This routine reads and formats an associated label for the mouse cursor label rendering on the live view

	// Read the mouse cursor temperature and compensate for the selected temperature unit
	CursorTemperature = (RMH_IRThermalCamera_ReadFramePixelTemperature(&IRCamera, &FrameThermalDataRaw[0], LiveViewCursorTrackPos.CursorXPos, LiveViewCursorTrackPos.CursorYPos, IRCamera.ThermalCameraSupportPool) * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;

	// Format the mouse cursor temperature label for live view rendering
	GlobalVariables::MouseCursorTempLabel = "Temp: " + CursorTemperature.ToString(GlobalVariables::TemperaturePrecision) + " " + GlobalVariables::DefaultTempUnitString;

}

void RMH_ThermalViewer_ReadAndFormatROITempAndLabels() {

	// This routine reads the active ROI maximum and minimum temperatures and formats the associated label strings for rendering

	// Read the temporary array data and sort the kernel array
	unsigned short Renderindex = 0;

	// Read the temperature and format the labels for all active ROIs
	for (unsigned int i = 0; i < NumOfActiveLiveViewROIs; i++) {

		// Read the render order index
		Renderindex = ActiveROIRenderingOrder[i];

		// Read the active ROI maximum and minimum temperature - as well as the raw pixel values of the ROI area
		ROIAreaPixelValues[Renderindex] = RMH_IRThermalCamera_ReadROIAreaPixelInfoInsideFrameArea(&IRCamera,
			&FrameThermalDataRaw[0], IRCamera.FrameWidth,
			ROIRectanglePositions[Renderindex].RectangleX0Pos,
			ROIRectanglePositions[Renderindex].RectangleY0Pos,
			ROIRectanglePositions[Renderindex].RectangleWidth,
			ROIRectanglePositions[Renderindex].RectangleHeight,
			ROIxReturnAreaRawPixelValsFlags[Renderindex], &ROIxAreaRawPixelValues[0], 
			IRCamera.ThermalCameraSupportPool);

		// Compensate for the selected temperature unit
		ROIAreaPixelValues[Renderindex].MaxValue = (ROIAreaPixelValues[Renderindex].MaxValue * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;
		ROIAreaPixelValues[Renderindex].MinValue = (ROIAreaPixelValues[Renderindex].MinValue * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;
		ROIAreaPixelValues[Renderindex].AvgValue = (ROIAreaPixelValues[Renderindex].AvgValue * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;

		// Format the ROI maximum and minimum temperature label for live view rendering
		GlobalVariables::ROIMaxTempLabels[Renderindex] = "Max: " + ROIAreaPixelValues[Renderindex].MaxValue.ToString(GlobalVariables::TemperaturePrecision) + " " + GlobalVariables::DefaultTempUnitString;
		GlobalVariables::ROIMinTempLabels[Renderindex] = "Min: " + ROIAreaPixelValues[Renderindex].MinValue.ToString(GlobalVariables::TemperaturePrecision) + " " + GlobalVariables::DefaultTempUnitString;

	}

	// Is live view split view enabled
	if (LiveViewSplitViewEnableFlag == true) {

		// Read the zoom ROI maximum and minimum temperature - as well as the raw pixel values of the ROI area
		ZoomROIAreaPixelValues = RMH_IRThermalCamera_ReadROIAreaPixelInfoInsideFrameArea(&IRCamera,
			&FrameThermalDataRaw[0], IRCamera.FrameWidth,
			ZoomROIRectanglePositions.RectangleX0Pos,
			ZoomROIRectanglePositions.RectangleY0Pos,
			ZoomROIRectanglePositions.RectangleWidth,
			ZoomROIRectanglePositions.RectangleHeight,
			true, &ZoomROIxAreaRawPixelValues[0],
			IRCamera.ThermalCameraSupportPool);

		// Compensate for the selected temperature unit
		ZoomROIAreaPixelValues.MaxValue = (ZoomROIAreaPixelValues.MaxValue * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;
		ZoomROIAreaPixelValues.MinValue = (ZoomROIAreaPixelValues.MinValue * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;
		ZoomROIAreaPixelValues.AvgValue = (ZoomROIAreaPixelValues.AvgValue * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;

		// Format the zoom ROI maximum and minimum temperature label for live view rendering
		GlobalVariables::ZoomROIMaxTempLabels = "Max: " + ZoomROIAreaPixelValues.MaxValue.ToString(GlobalVariables::TemperaturePrecision) + " " + GlobalVariables::DefaultTempUnitString;
		GlobalVariables::ZoomROIMinTempLabels = "Min: " + ZoomROIAreaPixelValues.MinValue.ToString(GlobalVariables::TemperaturePrecision) + " " + GlobalVariables::DefaultTempUnitString;

	}

}

void RMH_ThermalViewer_ReadAndFormatTempMeasurementsAndLabels() {

	// This routine reads the active ROI maximum and minimum temperatures and formats the associated label strings for rendering

	// Read the temporary array data and sort the kernel array
	unsigned short Renderindex = 0;

	// Read the temperatures and format the labels for all active temperature measurements
	for (unsigned int i = 0; i < NumOfActiveLiveViewTempMeas; i++) {

		// Read the render order index
		Renderindex = ActiveTempMeasRenderingOrder[i];

		// Read the temperature of the active temperature measurement
		TempMeasurementValues[Renderindex] = RMH_IRThermalCamera_ReadFramePixelTemperature(&IRCamera, &FrameThermalDataRaw[0],
			TempMeasPositions[Renderindex].CrosshairX0Pos, TempMeasPositions[Renderindex].CrosshairY0Pos, IRCamera.ThermalCameraSupportPool);

		// Compensate for the selected temperature unit
		TempMeasurementValues[Renderindex] = (TempMeasurementValues[Renderindex] * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;

		// Format the temperature measurement label string for live view rendering
		GlobalVariables::TempMeasurementsLabels[Renderindex] = TempMeasurementValues[Renderindex].ToString(GlobalVariables::TemperaturePrecision) + " " + GlobalVariables::DefaultTempUnitString;

	}

}

void RMH_ThermalViewer_ReadAndFormatLinesMaxMinAvgTempsAndLabels() {

	// This routine reads the maximum, minimum and average temperatures of the active lines and formats the associated label strings for rendering

	// Read the temporary array data and sort the kernel array
	double LineTempValue = 0.0;
	unsigned short Renderindex = 0;
	double LineMaximumTemperature = 0.0;
	double LineMinimumTemperature = 0.0;
	double LineAverageTemperature = 0.0;
	unsigned short TempLinesPositionsXCordinates = 0;
	unsigned short TempLinesPositionsYCordinates = 0;

	// Read the native height, width and aspect ratio parameters of the live view images
	LiveViewNativeImageWidth = GlobalVariables::OpenGLRender->RMH_LiveView_GetNativeImageWidth();
	LiveViewNativeImageHeight = GlobalVariables::OpenGLRender->RMH_LiveView_GetNativeImageHeight();
	LiveViewNativeImageAspectRatio = GlobalVariables::OpenGLRender->RMH_LiveView_GetNativeImageAspectRatio();

	// Read the temperatures and format the labels for all active temperature lines
	for (unsigned int i = 0; i < NumOfActiveLiveViewLines; i++) {

		// Read the render order index
		Renderindex = ActiveTempLineRenderingOrder[i];

		// Set the maximum and minimum values to the absolute max/min
		LineMaximumTemperature = -10000;
		LineMinimumTemperature = 10000;

		// Average temperature value of the line
		LineAverageTemperature = 0.0;

		// Read the maximum and minimum temperatures of the temperature line
		for (unsigned int j = 0; j < TempLinesPositions[Renderindex].LinePixelLength; j++) {

			// Check the live view rotation setting
			if (GlobalVariables::OpenGLRender->RMH_LiveView_GetRotation() == 0) {

				// Read the X/Y position coordinates of the temperature lines
				TempLinesPositionsXCordinates = TempLinesPositions[Renderindex].LineXCordinates[j];
				TempLinesPositionsYCordinates = TempLinesPositions[Renderindex].LineYCordinates[j];

			}
			if (GlobalVariables::OpenGLRender->RMH_LiveView_GetRotation() == 90) {

				// Read the X/Y position coordinates of the temperature lines
				TempLinesPositionsXCordinates = LiveViewNativeImageWidth - TempLinesPositions[Renderindex].LineYCordinates[j] * LiveViewNativeImageAspectRatio;
				TempLinesPositionsYCordinates = TempLinesPositions[Renderindex].LineXCordinates[j] / LiveViewNativeImageAspectRatio;
	
			}
			if (GlobalVariables::OpenGLRender->RMH_LiveView_GetRotation() == 180) {

				// Read the X/Y position coordinates of the temperature lines
				TempLinesPositionsXCordinates = LiveViewNativeImageWidth - TempLinesPositions[Renderindex].LineXCordinates[j];
				TempLinesPositionsYCordinates = LiveViewNativeImageHeight - TempLinesPositions[Renderindex].LineYCordinates[j];
	
			}
			if (GlobalVariables::OpenGLRender->RMH_LiveView_GetRotation() == 270) {

				// Read the X/Y position coordinates of the temperature lines
				TempLinesPositionsXCordinates = TempLinesPositions[Renderindex].LineYCordinates[j] * LiveViewNativeImageAspectRatio;
				TempLinesPositionsYCordinates = LiveViewNativeImageHeight - TempLinesPositions[Renderindex].LineXCordinates[j] / LiveViewNativeImageAspectRatio;
	
			}

			// Read the temperature value from the X/Y coordinates of the line
			LineTempValue = RMH_IRThermalCamera_ReadFramePixelTemperature(&IRCamera, &FrameThermalDataRaw[0], TempLinesPositionsXCordinates, TempLinesPositionsYCordinates, IRCamera.ThermalCameraSupportPool);

			// Store the temperature values of the line
			TempLinesTemperatureValues[Renderindex][j] = (LineTempValue * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;

			// Check for the maximum temperature
			if (LineTempValue > LineMaximumTemperature) {

				// Update the maximum temperature value
				LineMaximumTemperature = LineTempValue;

				// Store the frame X/Y coordinates of the maximum temperature
				TempLinesMaxTempValueXCoordinate[Renderindex] = TempLinesPositions[Renderindex].LineXCordinates[j];
				TempLinesMaxTempValueYCoordinate[Renderindex] = TempLinesPositions[Renderindex].LineYCordinates[j];

			}

			// Check for the maximum temperature
			if (LineTempValue < LineMinimumTemperature) {

				// Update the minimum temperature value
				LineMinimumTemperature = LineTempValue;

				// Store the frame X/Y coordinates of the minimum temperature
				TempLinesMinTempValueXCoordinate[Renderindex] = TempLinesPositions[Renderindex].LineXCordinates[j];
				TempLinesMinTempValueYCoordinate[Renderindex] = TempLinesPositions[Renderindex].LineYCordinates[j];

			}

			// Accumulate the sum of the temperatures of all line pixel values
			LineAverageTemperature = LineAverageTemperature + LineTempValue;

		}

		// Calculate the average line temperature
		LineAverageTemperature = LineAverageTemperature / (double)(TempLinesPositions[Renderindex].LinePixelLength);

		// Compensate for the selected temperature unit
		LineMaximumTemperature = (LineMaximumTemperature * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;
		LineMinimumTemperature = (LineMinimumTemperature * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;
		LineAverageTemperature = (LineAverageTemperature * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;

		// Store the maximum, minimum and average temperatures read in the global array
		TempLinesMaxTempValues[Renderindex] = LineMaximumTemperature;
		TempLinesMinTempValues[Renderindex] = LineMinimumTemperature;
		TempLinesAvgTempValues[Renderindex] = LineAverageTemperature;

		// Format the temperature measurement label string for live view rendering
		GlobalVariables::TempLinesMaxLabels[Renderindex] = "Max: " + TempLinesMaxTempValues[Renderindex].ToString(GlobalVariables::TemperaturePrecision) + " " + GlobalVariables::DefaultTempUnitString;
		GlobalVariables::TempLinesMinLabels[Renderindex] = "Min: " + TempLinesMinTempValues[Renderindex].ToString(GlobalVariables::TemperaturePrecision) + " " + GlobalVariables::DefaultTempUnitString;

	}

}

void RMH_ThermalViewer_ToggleMaximumTempTracking() {

	// This routine enables or disables live view maximum temperature tracking

	// Toggle the max temp tracking enable flag
	MaxTempTrackingEnableFlag = !MaxTempTrackingEnableFlag;

	// Should the dual color palette be enabled or disabled
	if (MaxTempTrackingEnableFlag == true) {

		// Update the button border color
		GlobalVariables::GlobalMaxTempTrackButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Reset the button border color
		GlobalVariables::GlobalMaxTempTrackButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	}

	// Update the button graphic
	GlobalVariables::GlobalMaxTempTrackButton->Refresh();

}

void RMH_ThermalViewer_ToggleMinimumTempTracking() {

	// This routine enables or disables live view minimum temperature tracking

	// Toggle the min temp tracking enable flag
	MinTempTrackingEnableFlag = !MinTempTrackingEnableFlag;

	// Should the dual color palette be enabled or disabled
	if (MinTempTrackingEnableFlag == true) {

		// Update the button border color
		GlobalVariables::GlobalMinTempTrackButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Reset the button border color
		GlobalVariables::GlobalMinTempTrackButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	}

	// Update the button graphic
	GlobalVariables::GlobalMinTempTrackButton->Refresh();

}

void RMH_ThermalViewer_ToggleCenterTempTracking() {

	// This routine enables or disables live view center temperature tracking

	// Toggle the center temp tracking enable flag
	CenterTempTrackingEnableFlag = !CenterTempTrackingEnableFlag;

	// Should the dual color palette be enabled or disabled
	if (CenterTempTrackingEnableFlag == true) {

		// Update the button border color
		GlobalVariables::GlobalCenterTempTrackButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Reset the button border color
		GlobalVariables::GlobalCenterTempTrackButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	}

	// Update the button graphic
	GlobalVariables::GlobalCenterTempTrackButton->Refresh();

}

void RMH_ThermalViewer_ToggleMouseCursorTempTracking() {
	
	// This routine enables or disables mouse cursor temperature tracking

	// Toggle the center temp tracking enable flag
	CursorTempTrackEnableFlag = !CursorTempTrackEnableFlag;

	// Should the dual color palette be enabled or disabled
	if (CursorTempTrackEnableFlag == true) {

		// Update the button border color
		GlobalVariables::GlobalCursorTempTrackButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Reset the button border color
		GlobalVariables::GlobalCursorTempTrackButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	}

	// Enable or disable mouse cursor temperature tracking
	GlobalVariables::OpenGLRender->RMH_OpenGL_EnableMouseCursorTrackingWLabel(CursorTempTrackEnableFlag);

	// Update the button graphic
	GlobalVariables::GlobalCursorTempTrackButton->Refresh();

}

// --------------------- Live View Statistics Data Handling Routines ---------------------- //

void RMH_ThermalViewer_CalLiveViewStatisticsData() {

	// This routine calculates the data of the live view statistics window.

	// Calculate the temperature drift of the thermal camera since the last calibration
	SensorTemperatureCalDrift = CurrentCalDetectorTemperature - IRCamera.temp_fpa;

	// Check whether the live view statistics window is open
	if (LiveViewStatisticsWindowIsShownFlag == true) {

		// Calculate the live view "span" (max - min) temperature
		TemperatureSpan = MaximumTemperature - MinimumTemperature;
		// Calculate how much of the current temperature range of the thermal camera is used (0 - 14-bit = 0% - 100%)
		ThermalCameraRangeUsage = ((double)IRCamera.Tmax_Tmp_Raw / 16383.0) * 100.0;

		// Calculate the sensor drift error of the thermal camera (Tdrift / (Min - Max))
		SensorDriftError = (SensorTemperatureCalDrift / (MinimumTemperature - MaximumTemperature)) * 100.0;

		// Has the maximum temperature become higher
		if (MaximumTemperature > MaxPeakTemperature) {

			// Store the latest highest temperature measurement
			MaxPeakTemperature = MaximumTemperature;

		}

		// Has the minimum temperature become lower
		if (MinimumTemperature < MinPeakTemperature) {

			// Store the latest lowest temperature measurement
			MinPeakTemperature = MinimumTemperature;

		}

	}

}

void RMH_ThermalViewer_UpdateAndFormatLiveViewStatisticsLabels() {

	// This routine calculates the live view statistics data, and updates and formats the statistics labels

	// Check whether the live view statistics window is open
	if (LiveViewStatisticsWindowIsShownFlag == true) {

		// Accumulate the sum of the camera frame rate read
		IRCamera.CameraFrameRateSum = IRCamera.CameraFrameRateSum + RMH_IRThermalCamera_ReadCameraFPS();

		// Increment the camera frame rate counter variable
		IRCamera.CameraFrameRateSumCounter = IRCamera.CameraFrameRateSumCounter + 1;

		// If the camera frame rate sum read has accumulated enough measurements
		if (IRCamera.CameraFrameRateSumCounter >= (unsigned int)IRCamera.FrameRate) {

			// Calculate the average frame rate of the camera
			IRCamera.CameraAverageFrameRate = IRCamera.CameraFrameRateSum / 20;

			// Reset the camera frame rate sum read
			IRCamera.CameraFrameRateSum = 0;
			// Reset the camera frame rate counter variable
			IRCamera.CameraFrameRateSumCounter = 0;

			// Update the FPS label of the live view statistics window
			GlobalVariables::GlobalFrameRateLabel->Text = IRCamera.CameraAverageFrameRate.ToString("F2") + " FPS";

		}

		// Update the "Captured frames" label of the live view statistics window
		GlobalVariables::GlobalNumberOfFramesLabel->Text = IRCamera.NumbOfCapturedFrames.ToString();
		// Update the "Temperature span" label of the live view statistics window
		GlobalVariables::GlobalSpanLabel->Text = TemperatureSpan.ToString(GlobalVariables::TemperaturePrecision) + " " + GlobalVariables::DefaultTempUnitString;;
		// Update the "Average temperature" label of the live view statistics window
		GlobalVariables::GlobalAverageLabel->Text = AverageTemperature.ToString(GlobalVariables::TemperaturePrecision) + " " + GlobalVariables::DefaultTempUnitString;
		// Update the "Range Usage" label of the live view statistics window
		GlobalVariables::GlobalRangeUsageLabel->Text = ThermalCameraRangeUsage.ToString("F2") + "%";
		// Update the "Drift" label of the live view statistics window
		GlobalVariables::GlobalDriftLabel->Text = (SensorTemperatureCalDrift * TemperatureUnitScaleFactor).ToString("F5") + " " + GlobalVariables::DefaultTempUnitString;
		// Update the "Drift Error" label of the live view statistics window
		GlobalVariables::GlobalDriftErrorLabel->Text = SensorDriftError.ToString("F5") + "%";
		// Update the "Max Peak" label of the live view statistics window
		GlobalVariables::GlobalMaxPeakLabel->Text = MaxPeakTemperature.ToString(GlobalVariables::TemperaturePrecision) + " " + GlobalVariables::DefaultTempUnitString;
		// Update the "Min Peak" label of the live view statistics window
		GlobalVariables::GlobalMinPeakLabel->Text = MinPeakTemperature.ToString(GlobalVariables::TemperaturePrecision) + " " + GlobalVariables::DefaultTempUnitString;

	}

}

// ------------------ Fixed Temperature Label Tracking Handling Routines ------------------ //

void RMH_ThermalViewer_UpdateTempMeasurementsRenderingOrder() {

	// This routine stores the positions of the enabled temperature measurement labels in an array
	// which defines the later rendering order

	// Read the temporary array data and sort the kernel array
	unsigned int ActiveTempMeasOrderIndex = 0;

	// Loop up to and including the maximum allowed number of temperature measurement labels
	for (unsigned int i = 0; i < _MaxNumberOfMovableCrosshairs; i++) {

		// If the active temperature measurement label read is enabled
		if (ActiveTempMeasEnableFlags[i] == true) {

			// Store the indices of the enabled temp measurement labels in the rendering order array
			ActiveTempMeasRenderingOrder[ActiveTempMeasOrderIndex] = i;

			// Increment the local order index counter variable
			ActiveTempMeasOrderIndex = ActiveTempMeasOrderIndex + 1;

		}

	}

}

void RMH_ThermalViewer_AddTemperatureMeasurementToLiveView() {

	// This routine enables a temperature measurement label for the live view stream

	// Read the temporary array data and sort the kernel array
	unsigned int ActiveTempMeasEnableIndex = 0;

	// If the maximum number of active temperature measurements has been reached
	if (NumOfActiveLiveViewTempMeas < _MaxNumberOfMovableCrosshairs) {

		// Loop up to and including the maximum allowed number of crosshair labels
		for (unsigned int i = 0; i < _MaxNumberOfMovableCrosshairs; i++) {

			// Which temperature measurements are not active
			if (ActiveTempMeasEnableFlags[i] == false) {

				// Read the index position of the most recently deleted temperature measurement
				ActiveTempMeasEnableIndex = i;

				// Update the ROI enable flag array position
				ActiveTempMeasEnableFlags[ActiveTempMeasEnableIndex] = true;

				// Break the for loop
				break;

			}

		}

		// Update the rendering order of the temperature measurements
		RMH_ThermalViewer_UpdateTempMeasurementsRenderingOrder();

		// Enable the associated ROI sub context menu drop-down list item 
		GlobalVariables::GlobaldeleteTempLabelToolStripMenuItem->DropDownItems[ActiveTempMeasEnableIndex]->Enabled = true;

		// Increment the number of active temperature measurements
		NumOfActiveLiveViewTempMeas = NumOfActiveLiveViewTempMeas + 1;

		// Update the button border color
		GlobalVariables::GlobalAddTempMeasButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Update the button border color
		GlobalVariables::GlobalAddTempMeasButton->FlatAppearance->BorderColor = System::Drawing::Color::Magenta;

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Maximum Number Of Temperature Measurements Reached!", _StatusMessageType_Warning);

	}

}

void RMH_ThermalViewer_DeleteTemperatureMeasurementFromLiveView(System::Object^ sender) {

	// This routine removes a temperature measurement from the rendering list of the live view stream

	// Cast the sender object as a WinForms ToolStrip object
	System::Windows::Forms::ToolStripMenuItem^ TempMeasIndex = (System::Windows::Forms::ToolStripMenuItem^)sender;

	// Read the sub context menu identification tag
	unsigned int TempMeasIndexTag = Convert::ToInt32(TempMeasIndex->Tag);

	// Check whether there are temperature measurements to delete
	if (NumOfActiveLiveViewTempMeas > 0) {

		// Disable the associated temperature measurement sub context menu drop-down list item 
		GlobalVariables::GlobaldeleteTempLabelToolStripMenuItem->DropDownItems[TempMeasIndexTag]->Enabled = false;

		// Decrement the number of active temperature measurements
		NumOfActiveLiveViewTempMeas = NumOfActiveLiveViewTempMeas - 1;

		// If the last temperature measurement has been deleted
		if (NumOfActiveLiveViewTempMeas <= 0) {
			// Reset the button border color
			GlobalVariables::GlobalAddTempMeasButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);
		}
		else {
			// Update the button border color
			GlobalVariables::GlobalAddTempMeasButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;
		}

		// Reset the enable flag of the associated temperature measurement tag
		ActiveTempMeasEnableFlags[TempMeasIndexTag] = false;

		// Update the rendering order of the temperature measurements
		RMH_ThermalViewer_UpdateTempMeasurementsRenderingOrder();

	}

}

void RMH_ThermalViewer_DeleteAllTemperatureMeasurementFromLiveView() {

	// This routine deletes all active temperature measurements from the live view stream

	// Loop up to and including the maximum allowed number of temperature measurements
	for (unsigned int i = 0; i < _MaxNumberOfMovableCrosshairs; i++) {

		// Check the active temperature measurements
		if (ActiveTempMeasEnableFlags[i] == true) {

			// Disable the associated temperature measurement sub context menu drop-down list item 
			GlobalVariables::GlobaldeleteTempLabelToolStripMenuItem->DropDownItems[i]->Enabled = false;

			// Decrement the number of active temperature measurements
			NumOfActiveLiveViewTempMeas = NumOfActiveLiveViewTempMeas - 1;

			// Reset the enable flag of the associated temperature measurement tag
			ActiveTempMeasEnableFlags[i] = false;

		}

	}

	// Reset the button border color
	GlobalVariables::GlobalAddTempMeasButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	// Update the rendering order of the temperature measurements
	RMH_ThermalViewer_UpdateTempMeasurementsRenderingOrder();

}

// --------------------- ROI Temperature Tracking Handling Routines ---------------------- //

void RMH_ThermalViewer_UpdateROIRenderingOrder() {

	// This routine stores the positions of the enabled ROIs in an array
    // which defines the later rendering order
	
	// Read the temporary array data and sort the kernel array
	unsigned int ActiveROIOrderIndex = 0;

	// Loop up to and including the maximum allowed number of ROI rectangles
	for (unsigned int i = 0; i < _MaxNumberOfMovableRectangles; i++) {

		// If the active ROI flag read is enabled
		if (ActiveROIEnableFlags[i] == true) {

			// Store the indices of the enabled ROIs in the ROI rendering order array
			ActiveROIRenderingOrder[ActiveROIOrderIndex] = i;

			// Increment the local order index counter variable
			ActiveROIOrderIndex = ActiveROIOrderIndex + 1;

		}

	}

}

void RMH_ThermalViewer_AddRegionOfInterestBoxToLiveView() {

	// This routine enables an ROI for rendering on the live view stream

	// Read the temporary array data and sort the kernel array
	unsigned int ActiveROIEnableIndex = 0;

	// If the maximum number of active ROIs (minus the color palette ROI) has been reached - 2 = (color palette ROI + zoom ROI)
	if (NumOfActiveLiveViewROIs < _MaxNumberOfMovableRectangles - _NumberOfNonMainLiveViewROIs) {

		// Loop up to and including the maximum allowed number of ROI rectangles
		for (unsigned int i = 0; i < _MaxNumberOfMovableRectangles; i++) {

			// Which ROIs are not active
			if (ActiveROIEnableFlags[i] == false) {

				// Read the index position of the most recently deleted ROI
				ActiveROIEnableIndex = i;

				// Update the ROI enable flag array position
				ActiveROIEnableFlags[ActiveROIEnableIndex] = true;

				// Break the for loop
				break;

			}

		}

		// Update the rendering order of the ROIs
		RMH_ThermalViewer_UpdateROIRenderingOrder();

		// Enable the associated ROI sub context menu drop-down list item 
		GlobalVariables::GlobaldeleteROIToolStripMenuItem->DropDownItems[ActiveROIEnableIndex]->Enabled = true;
		GlobalVariables::GlobalhistogramSourceToolStripMenuItem->DropDownItems[ActiveROIEnableIndex + 5]->Enabled = true;

		// Increment the number of active ROIs
		NumOfActiveLiveViewROIs = NumOfActiveLiveViewROIs + 1;

		// Update the button border color
		GlobalVariables::GlobalAddROIMeasButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Update the button border color
		GlobalVariables::GlobalAddROIMeasButton->FlatAppearance->BorderColor = System::Drawing::Color::Magenta;

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Maximum Number Of ROI Reached!", _StatusMessageType_Warning);

	}

}

void RMH_ThermalViewer_DeleteRegionOfInterestBoxFromLiveView(System::Object^ sender) {

	// This routine removes an ROI from the rendering list of the live view stream

	// Cast the sender object as a WinForms ToolStrip object
	System::Windows::Forms::ToolStripMenuItem^ ROIIndex = (System::Windows::Forms::ToolStripMenuItem^)sender;

	// Read the sub context menu identification tag
	unsigned int ROIIndexTag = Convert::ToInt32(ROIIndex->Tag);

	// Check whether there are ROIs to delete
	if (NumOfActiveLiveViewROIs > 0) {

		// Disable the associated ROI sub context menu drop-down list item 
		GlobalVariables::GlobaldeleteROIToolStripMenuItem->DropDownItems[ROIIndexTag]->Enabled = false;
		GlobalVariables::GlobalhistogramSourceToolStripMenuItem->DropDownItems[ROIIndexTag + 5]->Enabled = false;

		// Decrement the number of active ROIs
		NumOfActiveLiveViewROIs = NumOfActiveLiveViewROIs - 1;

		// If the last ROI has been deleted
		if (NumOfActiveLiveViewROIs <= 0) {
			// Reset the button border color
			GlobalVariables::GlobalAddROIMeasButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);
		}
		else {
			// Update the button border color
			GlobalVariables::GlobalAddROIMeasButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;
		}

		// Reset the enable flag of the associated ROI tag
		ActiveROIEnableFlags[ROIIndexTag] = false;

		// Update the rendering order of the ROIs
		RMH_ThermalViewer_UpdateROIRenderingOrder();

	}

}

void RMH_ThermalViewer_DeleteAllRegionOfInterestBoxFromLiveView() {

	// This routine deletes all active ROIs from the live view stream

	// Loop up to and including the maximum allowed number of ROI rectangles
	for (unsigned int i = 0; i < _MaxNumberOfMovableRectangles; i++) {

		// Check the active ROIs
		if (ActiveROIEnableFlags[i] == true) {

			// Disable the associated ROI sub context menu drop-down list item 
			GlobalVariables::GlobaldeleteROIToolStripMenuItem->DropDownItems[i]->Enabled = false;
			GlobalVariables::GlobalhistogramSourceToolStripMenuItem->DropDownItems[i + 5]->Enabled = false;

			// Decrement the number of active ROIs
			NumOfActiveLiveViewROIs = NumOfActiveLiveViewROIs - 1;

			// Reset the enable flag of the associated ROI tag
			ActiveROIEnableFlags[i] = false;

		}

	}

	// Reset the button border color
	GlobalVariables::GlobalAddROIMeasButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	// Update the rendering order of the ROIs
	RMH_ThermalViewer_UpdateROIRenderingOrder();

}

// --------------------- Temperature Line Tracking Handling Routines --------------------- //

void RMH_ThermalViewer_UpdateTempLinesRenderingOrder() {

	// This routine stores the positions of the enabled temperature lines in an array
	// which defines the later rendering order

	// Read the temporary array data and sort the kernel array
	unsigned int ActiveLinesOrderIndex = 0;

	// Loop up to and including the maximum allowed number of lines
	for (unsigned int i = 0; i < _MaxNumberOfMovableLines; i++) {

		// If the active line read is enabled
		if (ActiveTempLineEnableFlags[i] == true) {

			// Store the indices of the enabled lines in the rendering order array
			ActiveTempLineRenderingOrder[ActiveLinesOrderIndex] = i;

			// Increment the local order index counter variable
			ActiveLinesOrderIndex = ActiveLinesOrderIndex + 1;

		}

	}

}

void RMH_ThermalViewer_AddTemperatureLineToLiveView() {

	// This routine enables a temperature line for rendering on the live view stream

	// Read the temporary array data and sort the kernel array
	unsigned int ActiveLineEnableIndex = 0;

	// If the maximum number of active temperature lines has been reached
	if (NumOfActiveLiveViewLines < _MaxNumberOfMovableLines) {

		// Loop up to and including the maximum allowed number of lines
		for (unsigned int i = 0; i < _MaxNumberOfMovableLines; i++) {

			// Which lines are not active
			if (ActiveTempLineEnableFlags[i] == false) {

				// Read the index position of the most recently deleted line
				ActiveLineEnableIndex = i;

				// Update the ROI enable flag array position
				ActiveTempLineEnableFlags[ActiveLineEnableIndex] = true;

				// Break the for loop
				break;

			}

		}

		// Update the rendering order of the lines
		RMH_ThermalViewer_UpdateTempLinesRenderingOrder();

		// Enable the associated label sub context menu drop-down list item 
		GlobalVariables::GlobaldeleteLineToolStripMenuItem->DropDownItems[ActiveLineEnableIndex]->Enabled = true;
		GlobalVariables::GlobalhistogramSourceToolStripMenuItem->DropDownItems[ActiveLineEnableIndex]->Enabled = true;

		// Increment the number of active lines
		NumOfActiveLiveViewLines = NumOfActiveLiveViewLines + 1;

		// Update the button border color
		GlobalVariables::GlobalAddTempSpecLineButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Update the button border color
		GlobalVariables::GlobalAddTempSpecLineButton->FlatAppearance->BorderColor = System::Drawing::Color::Magenta;

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Maximum Number Of Lines Reached!", _StatusMessageType_Warning);

	}

}

void RMH_ThermalViewer_DeleteTemperatureLineFromLiveView(System::Object^ sender) {

	// This routine removes a temperature line from the rendering list of the live view stream

	// Cast the sender object as a WinForms ToolStrip object
	System::Windows::Forms::ToolStripMenuItem^ LineIndex = (System::Windows::Forms::ToolStripMenuItem^)sender;

	// Read the sub context menu identification tag
	unsigned int LineIndexTag = Convert::ToInt32(LineIndex->Tag);

	// Check whether there are lines that can be deleted
	if (NumOfActiveLiveViewLines > 0) {

		// Disable the associated line sub context menu drop-down list item 
		GlobalVariables::GlobaldeleteLineToolStripMenuItem->DropDownItems[LineIndexTag]->Enabled = false;
		GlobalVariables::GlobalhistogramSourceToolStripMenuItem->DropDownItems[LineIndexTag]->Enabled = false;

		// Decrement the number of active lines
		NumOfActiveLiveViewLines = NumOfActiveLiveViewLines - 1;

		// If the last line has been deleted
		if (NumOfActiveLiveViewLines <= 0) {
			// Reset the button border color
			GlobalVariables::GlobalAddTempSpecLineButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);
		}
		else {
			// Update the button border color
			GlobalVariables::GlobalAddTempSpecLineButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;
		}

		// Reset the enable flag of the associated line tag
		ActiveTempLineEnableFlags[LineIndexTag] = false;

		// Update the rendering order of the lines
		RMH_ThermalViewer_UpdateTempLinesRenderingOrder();

	}

}

void RMH_ThermalViewer_DeleteAllTemperatureLinesFromLiveView() {

	// This routine deletes all active temperature lines from the live view stream

	// Loop up to and including the maximum allowed number of temperature lines
	for (unsigned int i = 0; i < _MaxNumberOfMovableLines; i++) {

		// Check the active temperature lines
		if (ActiveTempLineEnableFlags[i] == true) {

			// Disable the associated temperature line sub context menu drop-down list item 
			GlobalVariables::GlobaldeleteLineToolStripMenuItem->DropDownItems[i]->Enabled = false;
			GlobalVariables::GlobalhistogramSourceToolStripMenuItem->DropDownItems[i]->Enabled = false;

			// Decrement the number of active temperature lines
			NumOfActiveLiveViewLines = NumOfActiveLiveViewLines - 1;

			// Reset the enable flag of the associated line tag
			ActiveTempLineEnableFlags[i] = false;

		}

	}

	// Reset the button border color
	GlobalVariables::GlobalAddTempSpecLineButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	// Update the rendering order of the lines
	RMH_ThermalViewer_UpdateTempLinesRenderingOrder();

}

// ------------------------ Live View Histogram Handling Routines ------------------------ //

void RMH_ThermalViewer_EnableLiveViewHistogram() {

	// This routine enables the histogram feature of the live view stream for lines and frame data

	// Toggle live view histogram enable flag
	LiveViewHistogramEnableFlag = !LiveViewHistogramEnableFlag;

	// Should the live view histogram be enabled or disabled
	if (LiveViewHistogramEnableFlag == true) {

		// Update the button border color
		GlobalVariables::GlobalShowLineHistButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

		// Make the histogram panel visible
		GlobalVariables::GlobalLiveViewHistogramPanel->Visible = true;

	}
	else {

		// Reset the button border color
		GlobalVariables::GlobalShowLineHistButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

		// Make the histogram panel invisible
		GlobalVariables::GlobalLiveViewHistogramPanel->Visible = false;

	}

	// Update the button graphic
	GlobalVariables::GlobalShowLineHistButton->Refresh();

}

void RMH_ThermalViewer_ChangeHistoramDataSource(System::Object^ sender) {

	// This routine sets the data source of the histogram 
	// which can be either temperature lines, ROIs or the whole live view image

	// Cast the sender object as a WinForms ToolStrip object
	System::Windows::Forms::ToolStripMenuItem^ MenuIndex = (System::Windows::Forms::ToolStripMenuItem^)sender;

	// Read the sub context menu identification tag
	unsigned int MenuIndexTag = Convert::ToInt32(MenuIndex->Tag);

	// Update the histogram data source tag value
	HistogramDataSourceTag = (unsigned char)MenuIndexTag;

	// Loop up to and including the maximum number of ROIs
	for (unsigned int i = 0; i < _MaxNumberOfMovableRectangles - _NumberOfNonMainLiveViewROIs; i++) {

		// Disable reading of raw pixel data from all active ROIs
		ROIxReturnAreaRawPixelValsFlags[i] = false;

	}

	// If the selected histogram data source is one of the active ROIs
	if (HistogramDataSourceTag >= _MaxNumberOfMovableRectangles - _NumberOfNonMainLiveViewROIs) {

		// Enable reading of raw pixel data for the selected ROI
		ROIxReturnAreaRawPixelValsFlags[HistogramDataSourceTag - (_MaxNumberOfMovableRectangles - _NumberOfNonMainLiveViewROIs)] = true;

	}

}

// -------------- Save Full-Frame Temperature Data To CSV Handling Routines --------------- //

void RMH_ThermalViewer_UpdateFullFrameTemperatureCSVDataDelimiter() {

	// This routine updates which data delimiter is used when saving a full-frame temperature data CSV file

	// Read the temporary array data and sort the kernel array
	unsigned char DataDelimiterIndex = 0;

	// Read the selected data delimiter from the associated ComboBox
	DataDelimiterIndex = GlobalVariables::GlobalFullFrameTempDataCSVDelimiterCombiBox->SelectedIndex;

	// Store and update the CSV data delimiter read in the global variable
	SelectedFullFrameTempCSVDataDelimiterIndex = DataDelimiterIndex;

	// Which delimiter index has been selected
	switch (SelectedFullFrameTempCSVDataDelimiterIndex) {

		// Update the associated global delimiter string
		case _FullFrameTempCSVDataDelimiterIndex_Comma:		GlobalVariables::FullFrameDataCSVDelimiterString = ",";  break;
		case _FullFrameTempCSVDataDelimiterIndex_Semicolon: GlobalVariables::FullFrameDataCSVDelimiterString = ";";  break;
		case _FullFrameTempCSVDataDelimiterIndex_Colon:		GlobalVariables::FullFrameDataCSVDelimiterString = ":";  break;
		case _FullFrameTempCSVDataDelimiterIndex_Space:		GlobalVariables::FullFrameDataCSVDelimiterString = " ";  break;
		case _FullFrameTempCSVDataDelimiterIndex_Tab:		GlobalVariables::FullFrameDataCSVDelimiterString = "\t"; break;

	}

}

void RMH_ThermalViewer_SaveFullFrameTemperatureDataToCSVFile() {

	// This routine creates/saves a CSV file and formats the latest data frame into a full frame of temperature data, which is then written to the CSV file.
	// The path of the saved file is the same as the selected snapshot file path

	// Local variables - frame width and height constants
	unsigned int FrameWidth = IRCamera.FrameWidth;
	unsigned int FrameHeight = IRCamera.FrameHeight - IRCamera.FrameMetadataSize;

	// Format the data identification string of the file (FrameTempData_HHmmssddMMyyyy)
	System::String^ FileName = System::DateTime::Now.ToString("HHmmssfffddMMyyyy");
	// Format the name of the temperature frame data file
	System::String^ FrameTempDataFileNameString = "FrameTempData_" + FileName + ".txt";

	// Perform a live view single frame trigger
	RMH_ThermalViewer_TriggerLiveViewSingleFrameCapture();

	// Loop through all raw thermal data values in the associated frame data array
	for (unsigned long i = 0; i < (FrameWidth * FrameHeight); i += 4) {

		// Convert the raw thermal data to temperature data and store it in the array
		FrameTemperatureData[i + 0] = (RMH_IRThermalCamera_ReadPixelTemperature(&IRCamera, FrameThermalDataRaw[i + 0], IRCamera.ThermalCameraSupportPool) * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;
		FrameTemperatureData[i + 1] = (RMH_IRThermalCamera_ReadPixelTemperature(&IRCamera, FrameThermalDataRaw[i + 1], IRCamera.ThermalCameraSupportPool) * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;
		FrameTemperatureData[i + 2] = (RMH_IRThermalCamera_ReadPixelTemperature(&IRCamera, FrameThermalDataRaw[i + 2], IRCamera.ThermalCameraSupportPool) * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;
		FrameTemperatureData[i + 3] = (RMH_IRThermalCamera_ReadPixelTemperature(&IRCamera, FrameThermalDataRaw[i + 3], IRCamera.ThermalCameraSupportPool) * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor;

	}

	// Generate the temperature frame data CSV file and write the data to the file
	RMH_Winforms_WriteDataArrayMatrixToCSVFile(RMH_Conversion_SystemStringToStdString(GlobalVariables::SnapShotDefaultPath), RMH_Conversion_SystemStringToStdString(FrameTempDataFileNameString), &FrameTemperatureData[0], FrameWidth, FrameHeight, GlobalVariables::FullFrameDataCSVDelimiterString);

	// Write GUI status message
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Full Frame Temperature Data CSV, Has Been Saved To Path.", _StatusMessageType_Success);

	// Restart the live view stream
	LiveViewRunStopFlag = false;
	RMH_ThermalViewer_ToggleLiveViewStreamRunStop();

}

// ----------- Live View Stream Video Recording And Snapshot Handling Routines ------------ //

void RMH_ThermalViewer_UpdateSnapshotDefaultSaveFilePath(System::Windows::Forms::Label^ DefaultPathString) {

	// This routine updates the file location where a live view snapshot is saved

	// Read the temporary array data and sort the kernel array
	System::String^ SaveFilePathString;

	// Read the selected default save file path of the snapshot 
	SaveFilePathString = RMH_Winforms_GetSaveFileDialogDirectory();

	// Check whether a path was selected, or whether the dialog was closed
	if (SaveFilePathString == "None") {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "No New Default Snapshot File Path Was Choosen!", _StatusMessageType_Warning);

	}
	else {

		// Update the default snapshot path
		GlobalVariables::SnapShotDefaultPath = SaveFilePathString;

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "New Default Snapshot File Path Was Choosen.", _StatusMessageType_Success);

	}

	// Update the default snapshot file path string 
	DefaultPathString->Text = "Default Save File Path:  " + GlobalVariables::SnapShotDefaultPath;

}

void RMH_ThermalViewer_UpdateVideoRecordingDefaultSaveFilePath(System::Windows::Forms::Label^ DefaultPathString) {

	// This routine updates the file location where the video recording is saved

	// Read the temporary array data and sort the kernel array
	System::String^ SaveFilePathString;

	// Read the selected default save file path of the video recording 
	SaveFilePathString = RMH_Winforms_GetSaveFileDialogDirectory();

	// Check whether a path was selected, or whether the dialog was closed
	if (SaveFilePathString == "None") {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "No New Default Video Recording File Path Was Choosen!", _StatusMessageType_Warning);

	}
	else {

		// Update the default snapshot path
		GlobalVariables::RecordingDefaultPath = SaveFilePathString;

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "New Default Video Recording File Path Was Choosen.", _StatusMessageType_Success);

	}

	// Update the default snapshot file path string 
	DefaultPathString->Text = "Default Save File Path:  " + GlobalVariables::RecordingDefaultPath;

}

void RMH_ThermalViewer_IncludeColorBarInSnapshot() {

	// This routine handles whether the selected snapshot should include the colorbar or not

	// Toggle the include colorbar in snapshot enable flag
	IncludeColorBarSnapshotFlag = !IncludeColorBarSnapshotFlag;

	// Whether the colorbar is included in the snapshot is enabled or disabled
	if (IncludeColorBarSnapshotFlag == true) {

		// Update the button border color
		GlobalVariables::GlobalIncludeColorbarSnapButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Reset the button border color
		GlobalVariables::GlobalIncludeColorbarSnapButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	}

	// Update the button graphic
	GlobalVariables::GlobalIncludeColorbarSnapButton->Refresh();

}

void RMH_ThermalViewer_ToggleSavinfOfRawSensorDataSnapshot() {

	// This routine updates the GUI elements and control flags when a RAW sensor data snapshot 
	// has been selected to be saved together with the associated live view snapshot.

	// Toggle the save raw sensor data snapshot enable flag
	CaptureRawSensorSnapshotFlag = !CaptureRawSensorSnapshotFlag;

	// Should the raw sensor data be saved
	if (CaptureRawSensorSnapshotFlag == true) {

		// Update the button border color
		GlobalVariables::GlobalSaveRawSensorSnapButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Reset the button border color
		GlobalVariables::GlobalSaveRawSensorSnapButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	}

	// Update the button graphic
	GlobalVariables::GlobalSaveRawSensorSnapButton->Refresh();

}

void RMH_ThermalViewer_SaveLiveViewSnapshot() {

	// This routine saves a snapshot of the live view stream, with or without the colorbar and the tools panel

	// local variables
	bool SnapshotStatus = false;
	bool RawSnapshotStatus = false;

	// Check whether the live view panel is shown in the GUI - or undocked
	if (isLiveViewStreamFormOpen == true) {

		// Should the snapshot include the colorbar and the tools panel
		if (IncludeColorBarSnapshotFlag == true) {
			// Save a snapshot of the live view stream - with the colorbar and the tools panel
			SnapshotStatus = RMH_Winforms_SavePanelSnapShotPNG(GlobalVariables::GlobalStreamAndCBarPanel, GlobalVariables::SnapShotDefaultPath);
		}
		else {
			// Save a snapshot of the live view stream - without the colorbar
			SnapshotStatus = RMH_Winforms_SavePanelSnapShotPNG(GlobalVariables::GlobalLiveViewStreamPanel, GlobalVariables::SnapShotDefaultPath);
		}
	}

	// Should an associated raw sensor data snapshot be saved
	if (CaptureRawSensorSnapshotFlag == true) {

		// Save a raw sensor data snapshot from the connected thermal camera
		RawSnapshotStatus = RMH_IRThermalCamera_ConvertCapturedRawImageDataToSnapshotPNG(IRCamera.ThermalCameraSupportPool);

	}

	// Check whether the snapshot was saved correctly
	if (SnapshotStatus == true) {
		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Snapshot Has Been Saved To Path Location.", _StatusMessageType_Success);
	}
	else {
		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Could Not Save The Snapshot To Path Location!", _StatusMessageType_Error);
	}

	// Should an associated raw sensor data snapshot be saved
	if (CaptureRawSensorSnapshotFlag == true) {

		// Check whether the raw snapshot was saved correctly
		if (RawSnapshotStatus == true) {
			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "RAW Snapshot Has Been Saved To Path Location.", _StatusMessageType_Success);
		}
		else {
			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Could Not Save The RAW Snapshot To Path Location!", _StatusMessageType_Error);
		}

	}

}

void RMH_ThermalViewer_SaveSurfacePlotSnapshot() {

	// This routine saves a snapshot of the 3D surface plot

	// local variables
	bool SnapshotStatus = false;

	// Save a snapshot of the 3D surface plot
	SnapshotStatus = RMH_Winforms_SavePanelSnapShotPNG(GlobalVariables::GlobalSurfacePlotPanel, GlobalVariables::SnapShotDefaultPath);
	
	// Check whether the snapshot was saved correctly
	if (SnapshotStatus == true) {
		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Snapshot Has Been Saved To Path Location.", _StatusMessageType_Success);
	}
	else {
		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Could Not Save The Snapshot To Path Location!", _StatusMessageType_Error);
	}

}

void RMH_ThermalViewer_ConfigDefaultCapturingProgram(System::Object^ sender) {

	// This routine configures the default video capturing program for recording live view video

	// Cast the sender object as a WinForms Button object
	System::Windows::Forms::Button^ SettingsButton = (System::Windows::Forms::Button^)sender;

	// Read the identification tag of the settings button
	unsigned int ButtonTag = Convert::ToInt32(SettingsButton->Tag);

	// Which button has been pressed
	switch (ButtonTag) {

		// Snipping Tool
		case 0: 

			// Update the button border colors
			GlobalVariables::GlobalUseWinSnippingToolButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;
			GlobalVariables::GlobalUseWin11ScreenRecordToolButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

			// Update the default video capturing program for recording live view video
			GlobalVariables::DefaultCapturingAppPackageFamilyNameString = _MicrosoftStore_SnippingTool;


		break;

		// Screen Recorder FOr Windows 11
		case 1: 

			// Update the button border colors
			GlobalVariables::GlobalUseWinSnippingToolButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);
			GlobalVariables::GlobalUseWin11ScreenRecordToolButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

			// Update the default video capturing program for recording live view video
			GlobalVariables::DefaultCapturingAppPackageFamilyNameString = _MicrosoftStore_ScreenRecorderForWindows11;

		break;

	}

	// Update the button graphics
	GlobalVariables::GlobalUseWinSnippingToolButton->Refresh();
	GlobalVariables::GlobalUseWin11ScreenRecordToolButton->Refresh();

}

void RMH_ThermalViewer_OpenDefaultVideoCapturingApp() {

	// This routine opens the selected default video capturing application

	// Local variables
	bool AppProcessErrorStatus = false;

	// Open the selected default video capturing application
	AppProcessErrorStatus = RMH_Winforms_OpenWindowsMicrosoftStoreApp(GlobalVariables::DefaultCapturingAppPackageFamilyNameString);

	// Check whether there were any errors when opening the app
	if (AppProcessErrorStatus == true) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Could Not Open The Selected Capturing Program!", _StatusMessageType_Error);

	}
	else {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Opening Video Capturing Program...", _StatusMessageType_Normal);

	}

}

void RMH_ThermalViewer_ToggleRecordingOfRAWDataForPostAnalysis() {

	// This routine toggles whether a RAW data recording file should be saved

	// Toggle the RAW data recording flag
	SaveRAWDataRecordingFlag = !SaveRAWDataRecordingFlag;

	// Should RAW data recording be enabled or disabled
	if (SaveRAWDataRecordingFlag == true) {

		// Update the button border color
		GlobalVariables::GlobalSaveRawAnalysisRecordingButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Reset the button border color
		GlobalVariables::GlobalSaveRawAnalysisRecordingButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	}

	// Update the button graphic
	GlobalVariables::GlobalSaveRawAnalysisRecordingButton->Refresh();

}

void RMH_ThermalViewer_StartStopVideoRecording() {

	// This routine starts the recording of video data

	// Generate and write extra RAW metadata to the frame data array 
	RMH_AnalysisMode_AddIDAndMetaDataToFrameArray(IRCamera.FrameWidth, IRCamera.FrameHeight, &IRCameraFrameData[0],
		IRCamera.ThermalCameraSupportPool, IRCamera.FrameMetadataSize, IRCamera.FrameWidthPixelOffset, IRCamera.FrameHeightPixelOffset,
		IRCamera.TemperatureCorrectionSetting, IRCamera.AmbientTemperatureSetting, IRCamera.ReflectedTemperatureSetting, IRCamera.HumiditySetting, IRCamera.EmissivitySetting, IRCamera.DistanceSetting);

	// Generate and write extra RAW metadata to the frame data array - for pool 3 cameras with NUC correction
	RMH_AnalysisMode_AddIDAndMetaDataToFrameArray(IRCamera.FrameWidth, IRCamera.FrameHeight, &FrameThermalData3Band[0],
		IRCamera.ThermalCameraSupportPool, IRCamera.FrameMetadataSize, IRCamera.FrameWidthPixelOffset, IRCamera.FrameHeightPixelOffset,
		IRCamera.TemperatureCorrectionSetting, IRCamera.AmbientTemperatureSetting, IRCamera.ReflectedTemperatureSetting, IRCamera.HumiditySetting, IRCamera.EmissivitySetting, IRCamera.DistanceSetting);

	// Check whether video recording should be started
	if (VideoRecordingStartedFlag == true && VideoFilesReadyFlag == false) {

		// Set the global frame rate variable of the recording
		RecordingFrameRateSetValue = (unsigned int)GlobalVariables::GlobalRecordingFrameRateNumericUpDown->Value;

		// Reset the video recording timeout counter variable
		RecordingFrameRateTimeOutCounter = 0;

		// Disable the use of "SaveRawAnalysisRecordingButton" when video recording starts
		GlobalVariables::GlobalSaveRawAnalysisRecordingButton->Enabled = false;

		// Check whether a RAW recording should be saved
		if (SaveRAWDataRecordingFlag == true) {

			// Configure and prepare an AVI video file for recording RAW camera data
			RMH_VideoFileRecording_SetupRecordingAnalysisModeVideoFile(GlobalVariables::RecordingDefaultPath, "RAWRecording_", IRCamera.FrameWidth, IRCamera.FrameHeight, RecordingFrameRateSetValue);

		}

		// If live view ultra resolution mode is enabled
		if (UltraResolutionEnableFlag == true) {

			// Configure and prepare an AVI video file for recording processed camera data
			RMH_VideoFileRecording_SetupLiveViewCaptureVideoFile(GlobalVariables::RecordingDefaultPath, "Recording_", IRCamera.FrameWidth * UltraResolutionScaleFactor, (IRCamera.FrameHeight - IRCamera.FrameMetadataSize) * UltraResolutionScaleFactor, RecordingFrameRateSetValue);

			// Disable the ultra resolution button in the live view tools panel
			GlobalVariables::GlobalUltraResolutionButton->Enabled = false;

		}
		else {

			// Configure and prepare an AVI video file for recording processed camera data
			RMH_VideoFileRecording_SetupLiveViewCaptureVideoFile(GlobalVariables::RecordingDefaultPath, "Recording_", IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, RecordingFrameRateSetValue);

		}

		// Update the border color of the recording tools button
		GlobalVariables::GlobalRecordingButton->FlatAppearance->BorderColor = System::Drawing::Color::Red;

		// Write status message to the GUI status text box
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Video Recording Has Started.", _StatusMessageType_Success);

		// Update the "ready" flag of the video files
		VideoFilesReadyFlag = true;

	}

	// Check whether video recording should be stopped
	if (VideoRecordingStartedFlag == false && VideoFilesReadyFlag == true) {

		// Reset the "ready" flag of the video files
		VideoFilesReadyFlag = false;

		// Check whether a RAW recording should be saved
		if (SaveRAWDataRecordingFlag == true) {

			// Close and save the recording of RAW camera data
			RMH_VideoFileRecording_CloseVideoFileWriting(_VideoFileWriteObject_RecordingAnalysisModeFile);

		}

		// Enable the ultra resolution button in the live view tools panel
		GlobalVariables::GlobalUltraResolutionButton->Enabled = true;

		// Close and save the recording of processed camera data
		RMH_VideoFileRecording_CloseVideoFileWriting(_VideoFileWriteObject_LiveViewStreamFile);

		// Reset the border color of the recording tools button
		GlobalVariables::GlobalRecordingButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

		// Write status message to the GUI status text box
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Video Recording Has Stopped.", _StatusMessageType_Warning);

		// Enable the use of "SaveRawAnalysisRecordingButton" when video recording starts
		GlobalVariables::GlobalSaveRawAnalysisRecordingButton->Enabled = true;

	}

	// Reset the "new recording frame" ready flag
	NewRecordFrameAvailableFlag = false;

}

// --------------------- Live View Stream Run/Stop Handling Routines --------------------- //

void RMH_ThermalViewer_ToggleLiveViewStreamRunStop() {

	// This routine toggles the live view stream run/stop state

	// Toggle the run/stop flag of the live view stream
	LiveViewRunStopFlag = !LiveViewRunStopFlag;

	// Whether the colorbar is included in the snapshot is enabled or disabled
	if (LiveViewRunStopFlag == true) {

		// Update the button border color
		GlobalVariables::GlobalLiveViewRunStopButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Reset the button border color
		GlobalVariables::GlobalLiveViewRunStopButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 255, 0, 0);

	}

	// Update the button graphic
	GlobalVariables::GlobalLiveViewRunStopButton->Refresh();

}

void RMH_ThermalViewer_TriggerLiveViewSingleFrameCapture() {

	// This routine triggers a single live view data frame capture

	// Update the run/stop flag of the live view stream
	LiveViewRunStopFlag = false;

	// Reset the button border color
	GlobalVariables::GlobalLiveViewRunStopButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 255, 255, 0);

	// Update the button graphic
	GlobalVariables::GlobalLiveViewRunStopButton->Refresh();

	// Update the live view single frame trigger flag
	LiveViewSingleFrameTriggerFlag = true;

}

// -------------- Thermal Camera Image Processing And Handling Routines -------------- //

void RMH_ThermalViewer_FrameCrabberCallback() {

	// This routine executes every time a data frame from the connected camera has been received

	// Increment the number of camera video frames read
	IRCamera.NumbOfCapturedFrames = IRCamera.NumbOfCapturedFrames + 1;
	
	// Enable thread data event - not used 
	//GlobalVariables::ThreadDataReadyEvent->Set();

}

void RMH_ThermalViewer_ImageProcessingSequence() {

	// This routine details the camera image processing of the thermal ratiometric data
	// and the implementations of various image processing techniques.
	// The routine is written to be optimized for execution in a separate CPU process thread
	// The handling of the colorbar data is likewise handled in this routine

	// Check the states of the live view run/stop and single trigger flags
	if (LiveViewRunStopFlag == true || LiveViewSingleFrameTriggerFlag == true) {

		// Is the application in "Recording Analysis" mode
		if (InRecordingAnalysisModeFlag == true) {

			// Read the selected raw video frame from the open RAW file
			RMH_VideoFileReading_ReadVideoFileFrame(CurrentPlayBackFrameValue, RecordingAnalysisModeFileInfo.NumberOfFrames, &IRCameraFrameData[0], MaximumFrameDataArraySize);

			// Every frame read is a new frame
			IsCapturedFrameNewFlag = true;

		}
		else {

			// Read raw frame data from the thermal camera
			IsCapturedFrameNewFlag = RMH_IRThermalCamera_ReadFrameRaw(&IRCameraFrameData[0], &VideoFrameSize);

		}

		// Format the raw YUY2 data to a 16-bit thermal data array - read the average value of the data
		IRCamera.Tavg_Tmp_Raw = RMH_IRThermalCamera_ConvertYUY2To14BitThermalDataArray(&IRCamera, &IRCameraFrameData[0], &FrameThermalDataRaw[0]);

		// Check whether the latest frame read is a new data frame
		if (IsCapturedFrameNewFlag == true && NewRecordFrameAvailableFlag == false) {

			// Update the "new recording frame" ready flag
			NewRecordFrameAvailableFlag = true;

		}

		// Update the live view single frame trigger flag
		LiveViewSingleFrameTriggerFlag = false;

	}

	// Read the frame metadata of the IR camera and calculate the internal IR sensor temperatures
	RMH_IRThermalCamera_ReadCalFrameMetaData(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

	// Read the maximum, minimum and center temperatures
	RMH_ThermalViewer_ReadMaxMinCentTemperatures();

	// ---------------------------------- Generation Of The Gaussian Kernel Mask ----------------------------------- //

	// Check whether a new Gaussian kernel mask should be generated
	if (NewGaussianKernelMaskGenerateFlag == true) {

		// Reset the "a new Gaussian kernel mask should be generated" flag
		NewGaussianKernelMaskGenerateFlag = false;

		// Generate a new Gaussian kernel unsharp mask
		RMH_ImageProcessing_GenerateUnsharpKernelMask(_ImageKernelMaskFilter_Size3x3, ImageUnSharpeningSigma, &GlobalGaussian3x3KernelMask[0]);

	}

	// --------------------------------- ColorBar & Image Processing Part 1 --------------------------------- //

	// Check whether the colorbar is in automatic or manual range mode
	if (ColorBarManualRangeFlag == true && ColorBarManualHighRangeFlag == false && ColorBarManualLowRangeFlag == false) {

		// Set the maximum and minimum temperature range values of the colorbar
		GlobalVariables::OpenGLColorBar->RMH_OpenGL_SetColorBarMaxMinRangeTemperature(ColorBarInitialManualRangeMaxTemp, ColorBarInitialManualRangeMinTemp);

		// Read the maximum and minimum range values of the colorbar in manual range mode
		ManualTempRangeSetValues = GlobalVariables::OpenGLColorBar->RMH_OpenGL_ReadColorBarRangeMaxMinValues();

		// Is image sharpening enabled
		if (LiveViewImageSharpeningEnableFlag == true) {

			// Implements linear automatic gain control for the image data - convert to grayscale - manual range
			RMH_IRThermalCamera_LinearAutomaticGainControlTemp(&FrameThermalDataRaw[0], &AGCSharpFrameDataArray[0], IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, _ImageProcessing_ImageResolution_14Bit, 0,
				ManualTempRangeSetValues.ManualMaxRangeTemp, ManualTempRangeSetValues.ManualMinRangeTemp, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor);

			// Perform image sharpening with Gaussian unsharp masking
			RMH_ImageProcessing_2DUnsharpMaskKernelImageSharpening(&AGCSharpFrameDataArray[0], _ImageProcessing_ImageResolution_14Bit, IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, _ImageKernelMaskFilter_Size3x3, &GlobalGaussian3x3KernelMask[0], ImageSharpeningStrength, ShowUnsharpenMaskImageFlag, &AGCFrameDataArray[0]);

		}
		else {

			// Implements linear automatic gain control for the image data - convert to grayscale - manual range
			RMH_IRThermalCamera_LinearAutomaticGainControlTemp(&FrameThermalDataRaw[0], &AGCFrameDataArray[0], IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, _ImageProcessing_ImageResolution_14Bit, 0,
				ManualTempRangeSetValues.ManualMaxRangeTemp, ManualTempRangeSetValues.ManualMinRangeTemp, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor);

		}

	}
	else if (ColorBarManualRangeFlag == false && ColorBarManualHighRangeFlag == true && ColorBarManualLowRangeFlag == false) {

		// Set the maximum and minimum temperature range values of the colorbar
		GlobalVariables::OpenGLColorBar->RMH_OpenGL_SetColorBarMaxMinRangeTemperature(ColorBarInitialManualRangeMaxTemp, MinimumTemperature);

		// Read the maximum and minimum range values of the colorbar in manual range mode
		ManualTempRangeSetValues = GlobalVariables::OpenGLColorBar->RMH_OpenGL_ReadColorBarRangeMaxMinValues();

		// Is image sharpening enabled
		if (LiveViewImageSharpeningEnableFlag == true) {

			// Implements linear automatic gain control for the image data - convert to grayscale - manual range
			RMH_IRThermalCamera_LinearAutomaticGainControlTemp(&FrameThermalDataRaw[0], &AGCSharpFrameDataArray[0], IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, _ImageProcessing_ImageResolution_14Bit, 0,
				ManualTempRangeSetValues.ManualMaxRangeTemp, MinimumTemperature, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor);

			// Perform image sharpening with Gaussian unsharp masking
			RMH_ImageProcessing_2DUnsharpMaskKernelImageSharpening(&AGCSharpFrameDataArray[0], _ImageProcessing_ImageResolution_14Bit, IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, _ImageKernelMaskFilter_Size3x3, &GlobalGaussian3x3KernelMask[0], ImageSharpeningStrength, ShowUnsharpenMaskImageFlag, &AGCFrameDataArray[0]);

		}
		else {

			// Implements linear automatic gain control for the image data - convert to grayscale - manual range
			RMH_IRThermalCamera_LinearAutomaticGainControlTemp(&FrameThermalDataRaw[0], &AGCFrameDataArray[0], IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, _ImageProcessing_ImageResolution_14Bit, 0,
				ManualTempRangeSetValues.ManualMaxRangeTemp, MinimumTemperature, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor);

		}

	}
	else if (ColorBarManualRangeFlag == false && ColorBarManualHighRangeFlag == false && ColorBarManualLowRangeFlag == true) {

		// Set the maximum and minimum temperature range values of the colorbar
		GlobalVariables::OpenGLColorBar->RMH_OpenGL_SetColorBarMaxMinRangeTemperature(MaximumTemperature, ColorBarInitialManualRangeMinTemp);

		// Read the maximum and minimum range values of the colorbar in manual range mode
		ManualTempRangeSetValues = GlobalVariables::OpenGLColorBar->RMH_OpenGL_ReadColorBarRangeMaxMinValues();

		// Is image sharpening enabled
		if (LiveViewImageSharpeningEnableFlag == true) {

			// Implements linear automatic gain control for the image data - convert to grayscale - manual range
			RMH_IRThermalCamera_LinearAutomaticGainControlTemp(&FrameThermalDataRaw[0], &AGCSharpFrameDataArray[0], IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, _ImageProcessing_ImageResolution_14Bit, 0,
				MaximumTemperature, ManualTempRangeSetValues.ManualMinRangeTemp, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor);

			// Perform image sharpening with Gaussian unsharp masking
			RMH_ImageProcessing_2DUnsharpMaskKernelImageSharpening(&AGCSharpFrameDataArray[0], _ImageProcessing_ImageResolution_14Bit, IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, _ImageKernelMaskFilter_Size3x3, &GlobalGaussian3x3KernelMask[0], ImageSharpeningStrength, ShowUnsharpenMaskImageFlag, &AGCFrameDataArray[0]);

		}
		else {

			// Implements linear automatic gain control for the image data - convert to grayscale - manual range
			RMH_IRThermalCamera_LinearAutomaticGainControlTemp(&FrameThermalDataRaw[0], &AGCFrameDataArray[0], IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, _ImageProcessing_ImageResolution_14Bit, 0,
				MaximumTemperature, ManualTempRangeSetValues.ManualMinRangeTemp, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor);

		}

	}
	else {

		// Set the maximum and minimum temperature range values of the colorbar
		GlobalVariables::OpenGLColorBar->RMH_OpenGL_SetColorBarMaxMinRangeTemperature(MaximumTemperature, MinimumTemperature);

		// Is image sharpening enabled
		if (LiveViewImageSharpeningEnableFlag == true) {

			// Implements linear automatic gain control for the image data - convert to grayscale - adaptive range
			RMH_ImageProcessing_LinearAutomaticGainControlRaw(&FrameThermalDataRaw[0], &AGCSharpFrameDataArray[0], IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, _ImageProcessing_ImageResolution_14Bit, 0, IRCamera.Tmax_Tmp_Raw, IRCamera.Tmin_Tmp_Raw);

			// Perform image sharpening with Gaussian unsharp masking
			RMH_ImageProcessing_2DUnsharpMaskKernelImageSharpening(&AGCSharpFrameDataArray[0], _ImageProcessing_ImageResolution_14Bit, IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, _ImageKernelMaskFilter_Size3x3, &GlobalGaussian3x3KernelMask[0], ImageSharpeningStrength, ShowUnsharpenMaskImageFlag, &AGCFrameDataArray[0]);

		}
		else {

			// Implements linear automatic gain control for the image data - convert to grayscale - adaptive range
			RMH_ImageProcessing_LinearAutomaticGainControlRaw(&FrameThermalDataRaw[0], &AGCFrameDataArray[0], IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, _ImageProcessing_ImageResolution_14Bit, 0, IRCamera.Tmax_Tmp_Raw, IRCamera.Tmin_Tmp_Raw);

		}

	}

	// Format the major and minor tick label strings of the colorbar
	GlobalVariables::OpenGLColorBar->RMH_OpenGL_FormatColorBarTickAndTagLabelStrings(GlobalVariables::DefaultTempUnitString);

	// Update the position of the maximum, minimum and center temperature indicator arrows of the colorbar
	GlobalVariables::OpenGLColorBar->RMH_OpenGL_UpdateColorBarMaxMinCenterTempArrowsPos(MaximumTemperature, MinimumTemperature, CenterTemperature);

	// ------------------------------ Color Palette & Image Processing Part 2 ------------------------------- //

	// Read the positions of the colorbar maximum and minimum tags
	ColorBarMaxMinTagPositions = GlobalVariables::OpenGLColorBar->RMH_OpenGL_ReadColorBarTagsPositions();

	// Should the live view color palette range be scaled in the colorbar
	if (LiveViewPaletteRangeScalingEnableFlag == true) {

		// Update the maximum and minimum range positions of the live view color palette
		LiveViewPaletteTagPositions = ColorBarMaxMinTagPositions;

	}

	// Compensate for the inverted live view color palette
	if (InvertLiveViewPaletteFlag == true) {

		// Format the live view color palette within the configured colorbar maximum and minimum tag temperature range
		RMH_ImageProcessing_FormatColorPaletteRangeInsideBackgroundPalette(ColorPalettePtr, InvertLiveViewPaletteFlag, AdaptFullColorBarPaletteRangeFlag, ColorBarBackPalettePtr, InvertColorBarBackgroundPaletteFlag,
			LiveViewPaletteTagPositions.MinimumTagPos - 1, LiveViewPaletteTagPositions.MaximumTagPos, FormattedLiveViewPalette);

	}
	else {

		// Format the live view color palette within the configured colorbar maximum and minimum tag temperature range
		RMH_ImageProcessing_FormatColorPaletteRangeInsideBackgroundPalette(ColorPalettePtr, InvertLiveViewPaletteFlag, AdaptFullColorBarPaletteRangeFlag, ColorBarBackPalettePtr, InvertColorBarBackgroundPaletteFlag,
			_ImageProcessing_ImageResolution_14Bit - LiveViewPaletteTagPositions.MaximumTagPos, _ImageProcessing_ImageResolution_14Bit - (LiveViewPaletteTagPositions.MinimumTagPos - 1), FormattedLiveViewPalette);

	}

	// Write the formatted live view color palette to the colorbar data structure 
	GlobalVariables::OpenGLColorBar->RMH_OpenGL_LoadFirstColorPalettesData(FormattedLiveViewPalette);
	
	// Is the dual color palette enabled
	if (DualColorPaletteEnableFlag == true) {

		// Should the dual live view color palette range be scaled in the colorbar
		if (DualPaletteRangeScalingEnableFlag == true) {

			// Update the maximum and minimum range positions of the dual color palette
			DualLiveViewPaletteTagPositions = ColorBarMaxMinTagPositions;

		}

		// Compensate for the inverted dual live view color palette
		if (InvertLiveViewDualPaletteFlag == true) {

			// Format the live view color palette within the configured colorbar maximum and minimum tag temperature range
			RMH_ImageProcessing_FormatColorPaletteRangeInsideBackgroundPalette(DualColorPalettePtr, InvertLiveViewDualPaletteFlag, AdaptFullColorBarPaletteRangeFlag, ColorBarBackPalettePtr, InvertColorBarBackgroundPaletteFlag,
				DualLiveViewPaletteTagPositions.MinimumTagPos - 1, DualLiveViewPaletteTagPositions.MaximumTagPos, FormattedDualLiveViewPalette);

		}
		else {

			// Format the live view color palette within the configured colorbar maximum and minimum tag temperature range
			RMH_ImageProcessing_FormatColorPaletteRangeInsideBackgroundPalette(DualColorPalettePtr, InvertLiveViewDualPaletteFlag, AdaptFullColorBarPaletteRangeFlag, ColorBarBackPalettePtr, InvertColorBarBackgroundPaletteFlag,
				_ImageProcessing_ImageResolution_14Bit - DualLiveViewPaletteTagPositions.MaximumTagPos, _ImageProcessing_ImageResolution_14Bit - (DualLiveViewPaletteTagPositions.MinimumTagPos - 1), FormattedDualLiveViewPalette);

		}

		// Write the formatted dual live view color palette to the colorbar data structure 
		GlobalVariables::OpenGLColorBar->RMH_OpenGL_LoadSecondColorPalettesData(FormattedDualLiveViewPalette);

		// Map the frame data to the selected color palette format - with dual color palette
		RMH_ImageProcessing_ApplyOverlayedPaletteToGrayScaleImageData(
			&AGCFrameDataArray[0], &ProcessedThermalImage[0],
			IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize,
			FormattedLiveViewPalette, FormattedDualLiveViewPalette,
			InvertLiveViewPaletteFlag, InvertLiveViewDualPaletteFlag,
			DualPaletteRectPosition.RectangleX0Pos, DualPaletteRectPosition.RectangleY0Pos,
			DualPaletteRectPosition.RectangleWidth, DualPaletteRectPosition.RectangleHeight);

	}
	else {

		// Map the frame data to the selected color palette format
		RMH_ImageProcessing_ApplyColorPaletteToGrayscaleImageData(&AGCFrameDataArray[0], IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, FormattedLiveViewPalette, InvertLiveViewPaletteFlag, &ProcessedThermalImage[0]);

	}

	// ----------------------------------------------- Histogram ------------------------------------------------ //

	// Is the live view histogram panel enabled
	if (LiveViewHistogramEnableFlag == true && LiveViewHistogramDataReadyFlag == false) {

		// Which palette should the histogram be rendered with
		if (HistogramDualOrLiveViewPaletteFlag == false) {

			// Should the formatted or the full color palette be shown 
			if (HistogramShowRangedPaletteFlag == true) {

				// Load histogram color palette
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_LoadHistogramColorPalette(FormattedLiveViewPalette);

			}
			else {

				// Load histogram color palette
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_LoadHistogramColorPalette(ColorPalettePtr);

			}

		}
		else {

			// Should the formatted or the full color palette be shown 
			if (HistogramShowRangedPaletteFlag == true) {

				// Load histogram color palette
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_LoadHistogramColorPalette(FormattedDualLiveViewPalette);

			}
			else {

				// Load histogram color palette
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_LoadHistogramColorPalette(DualColorPalettePtr);

			}

		}

		// Check whether the colorbar is in automatic or manual range mode
		if (ColorBarManualRangeFlag == true && ColorBarManualHighRangeFlag == false && ColorBarManualLowRangeFlag == false) {

			// Is the data source of the histogram the live view data
			if (HistogramDataSourceTag == 5) {

				// Format and distribute the frame data into histogram bins - manual range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataTemp(&IRCamera, &FrameThermalDataRaw[0],
					IRCamera.FrameWidth * (IRCamera.FrameHeight - IRCamera.FrameMetadataSize), 
					ManualTempRangeSetValues.ManualMaxRangeTemp, ManualTempRangeSetValues.ManualMinRangeTemp,
					TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, IRCamera.ThermalCameraSupportPool);

			}
			// Is the data source of the histogram the live view zoom data
			else if (HistogramDataSourceTag == 6) {

				// Format and distribute the selected zoom ROI data into histogram bins - manual range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataTemp(&IRCamera, &ZoomROIxAreaRawPixelValues[0],
					ZoomROIAreaPixelValues.ROIAreaNmbOfPixels, ManualTempRangeSetValues.ManualMaxRangeTemp, ManualTempRangeSetValues.ManualMinRangeTemp,
					TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, IRCamera.ThermalCameraSupportPool);

			}
			else if (HistogramDataSourceTag >= _MaxNumberOfMovableRectangles - _NumberOfNonMainLiveViewROIs) {

				// Format and distribute the selected ROI data into histogram bins - manual range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataTemp(&IRCamera, &ROIxAreaRawPixelValues[0],
					ROIAreaPixelValues[HistogramDataSourceTag - (_MaxNumberOfMovableRectangles - _NumberOfNonMainLiveViewROIs)].ROIAreaNmbOfPixels,
					ManualTempRangeSetValues.ManualMaxRangeTemp, ManualTempRangeSetValues.ManualMinRangeTemp,
					TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, IRCamera.ThermalCameraSupportPool);

			}
			else {

				// Format and distribute the selected line data into histogram bins - manual range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataLine(&IRCamera, &FrameThermalDataRaw[0],
					&TempLinesPositions[HistogramDataSourceTag].LineXCordinates[0],
					&TempLinesPositions[HistogramDataSourceTag].LineYCordinates[0],
					TempLinesPositions[HistogramDataSourceTag].LinePixelLength,
					ManualTempRangeSetValues.ManualMaxRangeTemp,
					ManualTempRangeSetValues.ManualMinRangeTemp, 
					TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor);

			}

		}
		else if (ColorBarManualRangeFlag == false && ColorBarManualHighRangeFlag == true && ColorBarManualLowRangeFlag == false) {

			// Is the data source of the histogram the live view data
			if (HistogramDataSourceTag == 5) {

				// Format and distribute the frame data into histogram bins - manual range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataTemp(&IRCamera, &FrameThermalDataRaw[0],
					IRCamera.FrameWidth * (IRCamera.FrameHeight - IRCamera.FrameMetadataSize),
					ManualTempRangeSetValues.ManualMaxRangeTemp, MinimumTemperature,
					TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, IRCamera.ThermalCameraSupportPool);

			}
			// Is the data source of the histogram the live view zoom data
			else if (HistogramDataSourceTag == 6) {

				// Format and distribute the selected zoom ROI data into histogram bins - manual range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataTemp(&IRCamera, &ZoomROIxAreaRawPixelValues[0],
					ZoomROIAreaPixelValues.ROIAreaNmbOfPixels, ManualTempRangeSetValues.ManualMaxRangeTemp, MinimumTemperature,
					TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, IRCamera.ThermalCameraSupportPool);

			}
			else if (HistogramDataSourceTag >= _MaxNumberOfMovableRectangles - _NumberOfNonMainLiveViewROIs) {

				// Format and distribute the selected ROI data into histogram bins - manual range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataTemp(&IRCamera, &ROIxAreaRawPixelValues[0],
					ROIAreaPixelValues[HistogramDataSourceTag - (_MaxNumberOfMovableRectangles - _NumberOfNonMainLiveViewROIs)].ROIAreaNmbOfPixels,
					ManualTempRangeSetValues.ManualMaxRangeTemp, MinimumTemperature,
					TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, IRCamera.ThermalCameraSupportPool);

			}
			else {

				// Format and distribute the selected line data into histogram bins - manual range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataLine(&IRCamera, &FrameThermalDataRaw[0],
					&TempLinesPositions[HistogramDataSourceTag].LineXCordinates[0],
					&TempLinesPositions[HistogramDataSourceTag].LineYCordinates[0],
					TempLinesPositions[HistogramDataSourceTag].LinePixelLength,
					ManualTempRangeSetValues.ManualMaxRangeTemp,
					MinimumTemperature,
					TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor);

			}

		}
		else if (ColorBarManualRangeFlag == false && ColorBarManualHighRangeFlag == false && ColorBarManualLowRangeFlag == true) {

			// Is the data source of the histogram the live view data
			if (HistogramDataSourceTag == 5) {

				// Format and distribute the frame data into histogram bins - manual range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataTemp(&IRCamera, &FrameThermalDataRaw[0],
					IRCamera.FrameWidth * (IRCamera.FrameHeight - IRCamera.FrameMetadataSize),
					MaximumTemperature, ManualTempRangeSetValues.ManualMinRangeTemp,
					TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, IRCamera.ThermalCameraSupportPool);

			}
			// Is the data source of the histogram the live view zoom data
			else if (HistogramDataSourceTag == 6) {

				// Format and distribute the selected zoom ROI data into histogram bins - manual range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataTemp(&IRCamera, &ZoomROIxAreaRawPixelValues[0],
					ZoomROIAreaPixelValues.ROIAreaNmbOfPixels, MaximumTemperature, ManualTempRangeSetValues.ManualMinRangeTemp,
					TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, IRCamera.ThermalCameraSupportPool);

			}
			else if (HistogramDataSourceTag >= _MaxNumberOfMovableRectangles - _NumberOfNonMainLiveViewROIs) {

				// Format and distribute the selected ROI data into histogram bins - manual range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataTemp(&IRCamera, &ROIxAreaRawPixelValues[0],
					ROIAreaPixelValues[HistogramDataSourceTag - (_MaxNumberOfMovableRectangles - _NumberOfNonMainLiveViewROIs)].ROIAreaNmbOfPixels,
					MaximumTemperature, ManualTempRangeSetValues.ManualMinRangeTemp,
					TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, IRCamera.ThermalCameraSupportPool);

			}
			else {

				// Format and distribute the selected line data into histogram bins - manual range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataLine(&IRCamera, &FrameThermalDataRaw[0],
					&TempLinesPositions[HistogramDataSourceTag].LineXCordinates[0],
					&TempLinesPositions[HistogramDataSourceTag].LineYCordinates[0],
					TempLinesPositions[HistogramDataSourceTag].LinePixelLength,
					MaximumTemperature,
					ManualTempRangeSetValues.ManualMinRangeTemp,
					TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor);

			}

		}
		else {

			// Is the data source of the histogram the live view data
			if (HistogramDataSourceTag == 5) {

				// Format and distribute the frame data into histogram bins - auto range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataRaw(&FrameThermalDataRaw[0],
					IRCamera.FrameWidth * (IRCamera.FrameHeight - IRCamera.FrameMetadataSize), IRCamera.Tmax_Tmp_Raw, IRCamera.Tmin_Tmp_Raw);

			}
			// Is the data source of the histogram the live view zoom data
			else if (HistogramDataSourceTag == 6) {

				// Format and distribute the selected zoom ROI data into histogram bins - auto range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataRaw(&ZoomROIxAreaRawPixelValues[0],
					ZoomROIAreaPixelValues.ROIAreaNmbOfPixels, IRCamera.Tmax_Tmp_Raw, IRCamera.Tmin_Tmp_Raw);

			}
			else if (HistogramDataSourceTag >= _MaxNumberOfMovableRectangles - _NumberOfNonMainLiveViewROIs) {

				// Format and distribute the selected ROI data into histogram bins - auto range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataRaw(&ROIxAreaRawPixelValues[0],
					ROIAreaPixelValues[HistogramDataSourceTag - (_MaxNumberOfMovableRectangles - _NumberOfNonMainLiveViewROIs)].ROIAreaNmbOfPixels,
					IRCamera.Tmax_Tmp_Raw, IRCamera.Tmin_Tmp_Raw);

			}
			else {

				// Format and distribute the selected line data into histogram bins - auto range
				GlobalVariables::OpenGLHistogram->RMH_OpenGL_FormatHistogramBinDataLine(&IRCamera, &FrameThermalDataRaw[0],
					&TempLinesPositions[HistogramDataSourceTag].LineXCordinates[0],
					&TempLinesPositions[HistogramDataSourceTag].LineYCordinates[0],
					TempLinesPositions[HistogramDataSourceTag].LinePixelLength,
					(RMH_IRThermalCamera_ReadPixelTemperature(&IRCamera, IRCamera.Tmax_Tmp_Raw, IRCamera.ThermalCameraSupportPool) * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor,
					(RMH_IRThermalCamera_ReadPixelTemperature(&IRCamera, IRCamera.Tmin_Tmp_Raw, IRCamera.ThermalCameraSupportPool) * TemperatureUnitScaleFactor) + TemperatureUnitOffsetFactor,
					TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor);

			}

		}

		// Update the "Histogram data is ready for rendering" flag
		LiveViewHistogramDataReadyFlag = true;

	}

	// ------------------------------------------ Temperature Tracking ------------------------------------------- //

	// Read the active ROI max/min temperatures and format the associated label strings for rendering
	RMH_ThermalViewer_ReadAndFormatROITempAndLabels();

	// Read the temperatures of the active temperature measurements and format the associated label strings for rendering 
	RMH_ThermalViewer_ReadAndFormatTempMeasurementsAndLabels();

	// Read the temperatures of the active temperature lines and format the associated label strings for rendering 
	RMH_ThermalViewer_ReadAndFormatLinesMaxMinAvgTempsAndLabels();

	// Is maximum temperature tracking enabled
	if (MaxTempTrackingEnableFlag == true) {

		// Format the max temperature label for rendering on the live view
		RMH_ThermalViewer_FormatMaximumTemperatureLabel();

	}

	// Is minimum temperature tracking enabled
	if (MinTempTrackingEnableFlag == true) {

		// Format the min temperature label for rendering on the live view
		RMH_ThermalViewer_FormatMinimumTemperatureLabel();

	}

	// Is center temperature tracking enabled
	if (CenterTempTrackingEnableFlag == true) {

		// Format the center temperature label for rendering on the live view
		RMH_ThermalViewer_FormatCenterTemperatureLabel();

	}

	// If mouse cursor temperature tracking is enabled
	if (CursorTempTrackEnableFlag == true) {

		// Read and format the temperature and label for the mouse cursor label rendering on the live view
		RMH_ThermalViewer_ReadAndFormatMouseCursorTempAndLabel();

	}

	// ---------------------------------------------- 2D Plot Data ---------------------------------------------- //

	// Is the temperature plot form visible
	if (isTempMeasurementsFormDocked == true || isTempMeasurementsFormUndocked == true) {

		// Add measurements to the active 2D plot data sets 
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_AddDataPointToPlotDataSet(*Plot2DDataSet1SourcePointer, _2DPlotDataSet_1);
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_AddDataPointToPlotDataSet(*Plot2DDataSet2SourcePointer, _2DPlotDataSet_2);
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_AddDataPointToPlotDataSet(*Plot2DDataSet3SourcePointer, _2DPlotDataSet_3);
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_AddDataPointToPlotDataSet(*Plot2DDataSet4SourcePointer, _2DPlotDataSet_4);
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_AddDataPointToPlotDataSet(*Plot2DDataSet5SourcePointer, _2DPlotDataSet_5);
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_AddDataPointToPlotDataSet(*Plot2DDataSet6SourcePointer, _2DPlotDataSet_6);
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_AddDataPointToPlotDataSet(*Plot2DDataSet7SourcePointer, _2DPlotDataSet_7);
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_AddDataPointToPlotDataSet(*Plot2DDataSet8SourcePointer, _2DPlotDataSet_8);
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_AddDataPointToPlotDataSet(*Plot2DDataSet9SourcePointer, _2DPlotDataSet_9);
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_AddDataPointToPlotDataSet(*Plot2DDataSet10SourcePointer, _2DPlotDataSet_10);

		// Read the maximum and minimum values of all active data sets and set the Y-axis range variables of the plot
		GlobalVariables::OpenGL2DPlot->RMH_OpenGL_ReadDataSetsMaxMinDataRangeValues();

	}

	// ------------------------------------------- Temperature Alarms ------------------------------------------- //

	// Monitor the active temperature alarms and update their statuses
	RMH_ThermalViewer_MonitorEnabledTempAlarmsStatus();

	// ---------------------------------------- Live View Statistics Data ---------------------------------------- //

	// Calculate the data of the live view statistics window
	RMH_ThermalViewer_CalLiveViewStatisticsData();

	// ---------------------------------------------------------------------------------------------------------- //

}

void RMH_ThermalViewer_SecondaryProcessingSequence() {

	// This routine is used as a secondary processing thread

	// Local variables
	unsigned int FrameWidth = IRCamera.FrameWidth; 
	unsigned int FrameHeight = IRCamera.FrameHeight - IRCamera.FrameMetadataSize;

	// If live view ultra resolution mode is enabled
	if (UltraResolutionEnableFlag == true && UltraResolutionImageDataReadyFlag == false && UltraResolutionImageDataReadyZoomFlag == false) {

		// Perform bilinear 2D interpolation of the AGC image data
		RMH_ImageProcessing_2DBilinearInterpolation(&AGCFrameDataArray[0], FrameWidth, FrameHeight, FrameWidth * UltraResolutionScaleFactor, FrameHeight * UltraResolutionScaleFactor, &UltraResolutionImage[0]);

		// Is the dual color palette enabled
		if (DualColorPaletteEnableFlag == true) {

			// Map the frame data to the selected color palette format - with dual color palette
			RMH_ImageProcessing_ApplyOverlayedPaletteToGrayScaleImageData(
				&UltraResolutionImage[0], &PrecessedUltraResolutionImage[0],
				FrameWidth * UltraResolutionScaleFactor, FrameHeight * UltraResolutionScaleFactor,
				FormattedLiveViewPalette, FormattedDualLiveViewPalette,
				InvertLiveViewPaletteFlag, InvertLiveViewDualPaletteFlag,
				DualPaletteRectPosition.RectangleX0Pos * UltraResolutionScaleFactor, DualPaletteRectPosition.RectangleY0Pos * UltraResolutionScaleFactor,
				DualPaletteRectPosition.RectangleWidth * UltraResolutionScaleFactor, DualPaletteRectPosition.RectangleHeight * UltraResolutionScaleFactor);

		}
		else {

			// Map the frame data to the selected color palette format
			RMH_ImageProcessing_ApplyColorPaletteToGrayscaleImageData(&UltraResolutionImage[0], FrameWidth * UltraResolutionScaleFactor, FrameHeight * UltraResolutionScaleFactor, FormattedLiveViewPalette, InvertLiveViewPaletteFlag, &PrecessedUltraResolutionImage[0]);

		}

		// Update the "ultra resolution image" finished processing and ready flag
		UltraResolutionImageDataReadyFlag = true;
		UltraResolutionImageDataReadyZoomFlag = true;

	}

}

void RMH_ThermalViewer_ToggleEnhancedLiveViewResolution() {

	// This routine enables or disables the live view enhanced image resolution mode

	// Handle the new state of the enable flag
	if (EnhancedResEnableFlag == true) {

		// Update the button border color
		GlobalVariables::GlobalEnhancedResButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Reset the button border color
		GlobalVariables::GlobalEnhancedResButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	}

	// Enable or disable enhanced image resolution mode
	GlobalVariables::OpenGLRender->RMH_OpenGL_EnableTextureLinearInterpolation(EnhancedResEnableFlag);

}

void RMH_ThermalViewer_ToggleLiveViewImageSharpening() {

	// This routine enables or disables the live view image sharpening feature

	// Toggle the live view image sharpening enable flag
	LiveViewImageSharpeningEnableFlag = !LiveViewImageSharpeningEnableFlag;

	// Handle the new state of the enable flag
	if (LiveViewImageSharpeningEnableFlag == true) {

		// Update the button border color
		GlobalVariables::GlobalImageSharpButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Reset the button border color
		GlobalVariables::GlobalImageSharpButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	}

}

void RMH_ThermalViewer_ToggleLiveViewUltraResolution() {

	// This routine enables or disables the live view ultra image resolution mode

	// Handle the state of the enable flag
	if (UltraResolutionEnableFlag == true) {

		// Update the button border color
		GlobalVariables::GlobalUltraResolutionButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Reset the button border color
		GlobalVariables::GlobalUltraResolutionButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	}

	// Enable or disable ultra resolution mode
	GlobalVariables::OpenGLRender->RMH_LiveViewStream_UltraResolutionMode(UltraResolutionEnableFlag);

}

// --------------- Live View Aspect Ratio Button And Event Handling Routines ---------------- //

void RMH_ThermalViewer_UpdateAspectRatioButtonBorderColor() {

	// This routine sets the start state of the live view aspect ratio button

	// Check the current aspect ratio setting
	if (FixedLiveViewAspectRatio == true) {

		// Update the aspect ratio button border color
		GlobalVariables::GlobalFixedAspectRatioButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

	}
	else {

		// Update the aspect ratio button border color
		GlobalVariables::GlobalFixedAspectRatioButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

	}

}

// -------------------- Surface Plot Update And Handling Routines -------------------- //

void RMH_ThermalViewer_UpdateSurfacePlotMenuScreen(unsigned int SurfacePlotPanelWidth, unsigned int SurfacePlotPanelHeight) {

	// This routine updates the surface plot with the latest processed data

	// Render 3. dimensional Surface Plot
	GlobalVariables::OpenGLSurfacePlot->RMH_OpenGL_RenderSurfacePlot(SurfacePlotPanelWidth, SurfacePlotPanelHeight, &ProcessedThermalImage[0], IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, &FrameThermalDataRaw[0], IRCamera.Tmax_Tmp_Raw, IRCamera.Tmin_Tmp_Raw);

}

// ----------------------- 2D Plot Update And Handling Routines ---------------------- //

void RMH_ThermalViewer_Update2DPlotMenuScreen(unsigned int PlotPanelWidth, unsigned int PlotPanelHeight) {

	// This routine updates the 2D plot with the latest processed data

	// Render 2. dimensional X/Y Plot
	GlobalVariables::OpenGL2DPlot->RMH_OpenGL_Render2DPlot(PlotPanelWidth, PlotPanelHeight, GlobalVariables::DefaultTempUnitString);

}

// ----------------- Temperature Alarm Update And Handling Routines ----------------- //

void RMH_ThermalViewer_UpdateTemperatureAlarmsSubMenuStatusLabels() {

	// This routine updates the status labels of the relevant temperature alarms 

	// Update the status labels of the active temperature alarms
	RMH_ThermalViewer_UpdateTempAlarmsStatusLabels();

}

// --------------- Live View Update, Recording And Handling Routines ---------------- //

void RMH_ThermalViewer_WriteDataToVideoRecordingFilesSequence() {

	// This routine handles writing the relevant data to video files, if video recording has started

	// Check whether a new video data frame is ready
	if (NewRecordFrameAvailableFlag == true) {

		// Increment the video recording timeout counter variable
		RecordingFrameRateTimeOutCounter = RecordingFrameRateTimeOutCounter + 1;

		// Check whether the video recording timeout counter variable has reached the set point value
		if (RecordingFrameRateTimeOutCounter >= (IRCamera.FrameRate / RecordingFrameRateSetValue)) {

			// Handle writing the selected camera pool data to video files, if video recording has started and is ready
			RMH_IRThermalCamera_WriteDataToVideoRecordingFilesSequence(IRCamera.ThermalCameraSupportPool);

			// Reset the video recording timeout counter variable
			RecordingFrameRateTimeOutCounter = 0;

		}

		// Reset the "new recording frame" ready flag
		NewRecordFrameAvailableFlag = false;

	}

}

// ZOOM TEMPERATURE LABELS - NOT FINISHED !!!
void RMH_ThermalViewer_UpdateLiveView(unsigned int LiveViewPanelWidth, unsigned int LiveViewPanelHeight, bool FixedAspectRatio, unsigned int ColorBarPanelWidth, unsigned int ColorBarPanelHeight, unsigned int HistogramPanelWidth, unsigned int HistogramPanelHeight) {

	// This routine updates the live view video stream, the colorbar and the live view histogram with the latest processed image data

	// Read the temporary array data and sort the kernel array
	unsigned int i = 0;

	// Render the colorbar with the associated tick lines, labels and additional graphical objects
	GlobalVariables::OpenGLColorBar->RMH_OpenGL_RenderColorBar(ColorBarPanelWidth, ColorBarPanelHeight, DualColorPaletteEnableFlag, NmbOfColorBarTempTicks, ColorBarCenterTrackEnableFlag, ColorBarManualRangeFlag, ColorBarManualHighRangeFlag, ColorBarManualLowRangeFlag);

	// If live view ultra resolution mode is enabled
	if (UltraResolutionEnableFlag == true) {

		// Is the ultra resolution image data finished processing and ready
		if (UltraResolutionImageDataReadyFlag == true) {

			// Render the latest processed ultra resolution image data in the live view texture panel 
			GlobalVariables::OpenGLRender->RMH_OpenGL_RenderGrayscaleUltraResolutionImageData(LiveViewPanelWidth, LiveViewPanelHeight,
				PrecessedUltraResolutionImage, IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize,
				IRCamera.FrameWidth * UltraResolutionScaleFactor, (IRCamera.FrameHeight - IRCamera.FrameMetadataSize) * UltraResolutionScaleFactor, FixedAspectRatio);

			// Reset the "ultra resolution image" finished processing and ready flag
			UltraResolutionImageDataReadyFlag = false;

		}

	}
	else {

		// Render the latest processed image data in the live view texture panel 
		GlobalVariables::OpenGLRender->RMH_OpenGL_RenderGrayscale16BitImageData(LiveViewPanelWidth, LiveViewPanelHeight,
			ProcessedThermalImage, IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, FixedAspectRatio);

	}

	// Is the dual color palette enabled
	if (DualColorPaletteEnableFlag == true) {

		// Render the resizable rectangle and read its position on the texture
		DualPaletteRectPosition = GlobalVariables::OpenGLRender->RMH_OpenGL_RenderMovableRectangle(1, "Palette", 
			ROISelectedColorR, ROISelectedColorG, ROISelectedColorB, ROIPassiveColorR, ROIPassiveColorG, ROIPassiveColorB, false);

	}

	// Render the number of active ROI rectangles
	for (i = 0; i < NumOfActiveLiveViewROIs; i++) {

		// Render the resizable ROI rectangle - with active order and read the ROI position
		ROIRectanglePositions[ActiveROIRenderingOrder[i]] = GlobalVariables::OpenGLRender->RMH_OpenGL_RenderMovableRectangle(
			ActiveROIRenderingOrder[i] + 3, "ROI " + std::to_string(ActiveROIRenderingOrder[i] + 1),
			ROISelectedColorR, ROISelectedColorG, ROISelectedColorB, ROIPassiveColorR, ROIPassiveColorG, ROIPassiveColorB, false);

		// Render the ROI maximum temperature crosshair and label
		GlobalVariables::OpenGLRender->RMH_OpenGL_RenderCrossHairWithLabel(
			ROIAreaPixelValues[ActiveROIRenderingOrder[i]].ROIMaxPixelWidth,
			ROIAreaPixelValues[ActiveROIRenderingOrder[i]].ROIMaxPixelHeight, true,
			GlobalVariables::ROIMaxTempLabels[ActiveROIRenderingOrder[i]],
			MaxCrosshairColorR, MaxCrosshairColorG, MaxCrosshairColorB);

		// Render the ROI minimum temperature crosshair and label
		GlobalVariables::OpenGLRender->RMH_OpenGL_RenderCrossHairWithLabel(
			ROIAreaPixelValues[ActiveROIRenderingOrder[i]].ROIMinPixelWidth,
			ROIAreaPixelValues[ActiveROIRenderingOrder[i]].ROIMinPixelHeight, true,
			GlobalVariables::ROIMinTempLabels[ActiveROIRenderingOrder[i]],
			MinCrosshairColorR, MinCrosshairColorG, MinCrosshairColorB);

	}

	// Render the number of active temperature measurements
	for (i = 0; i < NumOfActiveLiveViewTempMeas; i++) {

		// Render the position-adjustable temperature measurement crosshair with label
		TempMeasPositions[ActiveTempMeasRenderingOrder[i]] = GlobalVariables::OpenGLRender->RMH_OpenGL_RenderMovableCrossHairWithLabel(
			ActiveTempMeasRenderingOrder[i] + 1, 
			"TM" + (ActiveTempMeasRenderingOrder[i] + 1).ToString() + ": " + GlobalVariables::TempMeasurementsLabels[ActiveTempMeasRenderingOrder[i]],
			TempMeasCrosshairSelectedColorR, TempMeasCrosshairSelectedColorG, TempMeasCrosshairSelectedColorB,
			TempMeasCrosshairPassiveColorR, TempMeasCrosshairPassiveColorG, TempMeasCrosshairPassiveColorB);

	}

	// Render the number of active temperature lines
	for (i = 0; i < NumOfActiveLiveViewLines; i++) {

		// Render the position-adjustable temperature lines on the live view stream
		TempLinesPositions[ActiveTempLineRenderingOrder[i]] = GlobalVariables::OpenGLRender->RMH_OpenGL_RenderMovableLine(
			ActiveTempLineRenderingOrder[i] + 1, 2,
			TempLinesSelectedColorR, TempLinesSelectedColorG, TempLinesSelectedColorB, 
			TempLinesPassiveColorR, TempLinesPassiveColorG, TempLinesPassiveColorB);

		// Check the live view rotation setting
		if (GlobalVariables::OpenGLRender->RMH_LiveView_GetRotation() == 0) {

			// Render the line maximum temperature crosshair and label
			GlobalVariables::OpenGLRender->RMH_OpenGL_RenderCrossHairWithLabel(
				TempLinesMaxTempValueXCoordinate[ActiveTempLineRenderingOrder[i]],
				TempLinesMaxTempValueYCoordinate[ActiveTempLineRenderingOrder[i]],
				true, GlobalVariables::TempLinesMaxLabels[ActiveTempLineRenderingOrder[i]],
				MaxCrosshairColorR, MaxCrosshairColorG, MaxCrosshairColorB);

			// Render the line minimum temperature crosshair and label
			GlobalVariables::OpenGLRender->RMH_OpenGL_RenderCrossHairWithLabel(
				TempLinesMinTempValueXCoordinate[ActiveTempLineRenderingOrder[i]],
				TempLinesMinTempValueYCoordinate[ActiveTempLineRenderingOrder[i]],
				true, GlobalVariables::TempLinesMinLabels[ActiveTempLineRenderingOrder[i]],
				MinCrosshairColorR, MinCrosshairColorG, MinCrosshairColorB);

		}
		if (GlobalVariables::OpenGLRender->RMH_LiveView_GetRotation() == 90) {

			// Render the line maximum temperature crosshair and label
			GlobalVariables::OpenGLRender->RMH_OpenGL_RenderCrossHairWithLabel(
				LiveViewNativeImageWidth - ((double)TempLinesMaxTempValueYCoordinate[ActiveTempLineRenderingOrder[i]]) * LiveViewNativeImageAspectRatio,
				(double)TempLinesMaxTempValueXCoordinate[ActiveTempLineRenderingOrder[i]] / LiveViewNativeImageAspectRatio,
				true, GlobalVariables::TempLinesMaxLabels[ActiveTempLineRenderingOrder[i]],
				MaxCrosshairColorR, MaxCrosshairColorG, MaxCrosshairColorB);

			// Render the line minimum temperature crosshair and label
			GlobalVariables::OpenGLRender->RMH_OpenGL_RenderCrossHairWithLabel(
				LiveViewNativeImageWidth - ((double)TempLinesMinTempValueYCoordinate[ActiveTempLineRenderingOrder[i]]) * LiveViewNativeImageAspectRatio,
				(double)TempLinesMinTempValueXCoordinate[ActiveTempLineRenderingOrder[i]] / LiveViewNativeImageAspectRatio,
				true, GlobalVariables::TempLinesMinLabels[ActiveTempLineRenderingOrder[i]],
				MinCrosshairColorR, MinCrosshairColorG, MinCrosshairColorB);

		}
		if (GlobalVariables::OpenGLRender->RMH_LiveView_GetRotation() == 180) {

			// Render the line maximum temperature crosshair and label
			GlobalVariables::OpenGLRender->RMH_OpenGL_RenderCrossHairWithLabel(
				LiveViewNativeImageWidth - (double)TempLinesMaxTempValueXCoordinate[ActiveTempLineRenderingOrder[i]],
				LiveViewNativeImageHeight - (double)TempLinesMaxTempValueYCoordinate[ActiveTempLineRenderingOrder[i]],
				true, GlobalVariables::TempLinesMaxLabels[ActiveTempLineRenderingOrder[i]],
				MaxCrosshairColorR, MaxCrosshairColorG, MaxCrosshairColorB);

			// Render the line minimum temperature crosshair and label
			GlobalVariables::OpenGLRender->RMH_OpenGL_RenderCrossHairWithLabel(
				LiveViewNativeImageWidth - (double)TempLinesMinTempValueXCoordinate[ActiveTempLineRenderingOrder[i]],
				LiveViewNativeImageHeight - (double)TempLinesMinTempValueYCoordinate[ActiveTempLineRenderingOrder[i]],
				true, GlobalVariables::TempLinesMinLabels[ActiveTempLineRenderingOrder[i]],
				MinCrosshairColorR, MinCrosshairColorG, MinCrosshairColorB);

		}
		if (GlobalVariables::OpenGLRender->RMH_LiveView_GetRotation() == 270) {

			// Render the line maximum temperature crosshair and label
			GlobalVariables::OpenGLRender->RMH_OpenGL_RenderCrossHairWithLabel(
				(double)TempLinesMaxTempValueYCoordinate[ActiveTempLineRenderingOrder[i]] * LiveViewNativeImageAspectRatio,
				LiveViewNativeImageHeight - (double)TempLinesMaxTempValueXCoordinate[ActiveTempLineRenderingOrder[i]] / LiveViewNativeImageAspectRatio,
				true, GlobalVariables::TempLinesMaxLabels[ActiveTempLineRenderingOrder[i]],
				MaxCrosshairColorR, MaxCrosshairColorG, MaxCrosshairColorB);

			// Render the line minimum temperature crosshair and label
			GlobalVariables::OpenGLRender->RMH_OpenGL_RenderCrossHairWithLabel(
				(double)TempLinesMinTempValueYCoordinate[ActiveTempLineRenderingOrder[i]] * LiveViewNativeImageAspectRatio,
				LiveViewNativeImageHeight - (double)TempLinesMinTempValueXCoordinate[ActiveTempLineRenderingOrder[i]] / LiveViewNativeImageAspectRatio,
				true, GlobalVariables::TempLinesMinLabels[ActiveTempLineRenderingOrder[i]],
				MinCrosshairColorR, MinCrosshairColorG, MinCrosshairColorB);

		}

	}

	// Is maximum temperature tracking enabled
	if (MaxTempTrackingEnableFlag == true) {

		// Render the maximum temperature crosshair with label on the live view texture
		GlobalVariables::OpenGLRender->RMH_OpenGL_RenderCrossHairWithLabel(
			IRCamera.Tmax_X, IRCamera.Tmax_Y, true, 
			GlobalVariables::MaximumTempLabel, 
			MaxCrosshairColorR, MaxCrosshairColorG, MaxCrosshairColorB);

	}

	// Is minimum temperature tracking enabled
	if (MinTempTrackingEnableFlag == true) {

		// Render the minimum temperature crosshair with label on the live view texture
		GlobalVariables::OpenGLRender->RMH_OpenGL_RenderCrossHairWithLabel(
			IRCamera.Tmin_X, IRCamera.Tmin_Y, true,
			GlobalVariables::MinimumTempLabel, 
			MinCrosshairColorR, MinCrosshairColorG, MinCrosshairColorB);

	}

	// Is center temperature tracking enabled
	if (CenterTempTrackingEnableFlag == true) {

		// Render the minimum temperature crosshair with label on the live view texture
		GlobalVariables::OpenGLRender->RMH_OpenGL_RenderCrossHairCenterLabel(
			IRCamera.FrameWidth * 0.5, ((IRCamera.FrameHeight - IRCamera.FrameMetadataSize) * 0.5),
			GlobalVariables::CenterTempLabel, 
			CenterCrosshairColorR, CenterCrosshairColorG, CenterCrosshairColorB);

	}

	// If mouse cursor temperature tracking is enabled
	if (CursorTempTrackEnableFlag == true) {

		// Render the mouse cursor temperature tracking label
		LiveViewCursorTrackPos = GlobalVariables::OpenGLRender->RMH_OpenGL_RenderMouseCursorLabel(GlobalVariables::MouseCursorTempLabel, CommonLabelColorR, CommonLabelColorG, CommonLabelColorB);

	}

	// Is live view split view enabled
	if (LiveViewSplitViewEnableFlag == true) {

		// Render the resizable rectangle for setting the live view zoom
		ZoomROIRectanglePositions = GlobalVariables::OpenGLRender->RMH_OpenGL_RenderMovableRectangle(2, "Zoom",
			ROISelectedColorR, ROISelectedColorG, ROISelectedColorB, ROIPassiveColorR, ROIPassiveColorG, ROIPassiveColorB, true);

	}
	
	// Mark the end of a live view OpenGL rendering sequence
	GlobalVariables::OpenGLRender->RMH_OpenGL_RenderingFinishedMark();

	// Is the live view histogram panel enabled
	if (LiveViewHistogramEnableFlag == true && LiveViewHistogramDataReadyFlag == true) {

		// Render the live view histogram in the histogram panel
		GlobalVariables::OpenGLHistogram->RMH_OpenGL_RenderHistogram(HistogramPanelWidth, HistogramPanelHeight);

		// Reset the "Histogram data is ready for rendering" flag
		LiveViewHistogramDataReadyFlag = false;

	}

	// Is live view split view enabled
	if (LiveViewSplitViewEnableFlag == true) {

		// If live view ultra resolution mode is enabled
		if (UltraResolutionEnableFlag == true) {

			// Is the ultra resolution image data finished processing and ready
			if (UltraResolutionImageDataReadyZoomFlag == true) {

				// Render the latest processed ultra resolution image data in the live view zoom texture panel 
				GlobalVariables::LiveViewZoomWindowRender->RMH_LiveView_RenderZoomWindowUltraResolution(GlobalVariables::GlobalLiveViewZoomPanel->Width, GlobalVariables::GlobalLiveViewZoomPanel->Height,
					PrecessedUltraResolutionImage, IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize,
					IRCamera.FrameWidth * UltraResolutionScaleFactor, (IRCamera.FrameHeight - IRCamera.FrameMetadataSize) * UltraResolutionScaleFactor, FixedAspectRatio,
					ZoomROIRectanglePositions.RectangleX0Pos, ZoomROIRectanglePositions.RectangleY0Pos, ZoomROIRectanglePositions.RectangleWidth, ZoomROIRectanglePositions.RectangleHeight,
					GlobalVariables::OpenGLRender->RMH_LiveView_GetRotation());

				// Reset the "ultra resolution zoom image" finished processing and ready flag
				UltraResolutionImageDataReadyZoomFlag = false;

			}

		}
		else {

			// Render the live view zoom window
			GlobalVariables::LiveViewZoomWindowRender->RMH_LiveView_RenderZoomWindow(GlobalVariables::GlobalLiveViewZoomPanel->Width, GlobalVariables::GlobalLiveViewZoomPanel->Height, 
				ProcessedThermalImage, IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, FixedAspectRatio,
				ZoomROIRectanglePositions.RectangleX0Pos, ZoomROIRectanglePositions.RectangleY0Pos, ZoomROIRectanglePositions.RectangleWidth, ZoomROIRectanglePositions.RectangleHeight,
				GlobalVariables::OpenGLRender->RMH_LiveView_GetRotation());

		}

		//cout << ZoomROIRectanglePositions.RectangleX0Pos << endl;
		//cout << ZoomROIRectanglePositions.RectangleY0Pos << endl;
		//cout << ZoomROIRectanglePositions.RectangleWidth << endl;
		//cout << ZoomROIRectanglePositions.RectangleHeight << endl;
		//cout << endl;

		// Render the ROI maximum temperature crosshair and label
		//GlobalVariables::LiveViewZoomWindowRender->RMH_LiveView_RenderCrossHairWithLabel(
			//ZoomROIAreaPixelValues.ROIMaxPixelWidth + ZoomROIRectanglePositions.RectangleX0Pos,
			//ZoomROIAreaPixelValues.ROIMaxPixelHeight + ZoomROIRectanglePositions.RectangleX0Pos, true,
			//GlobalVariables::ZoomROIMaxTempLabels,
			//MaxCrosshairColorR, MaxCrosshairColorG, MaxCrosshairColorB);
	
		// Render the ROI minimum temperature crosshair and label
		//GlobalVariables::LiveViewZoomWindowRender->RMH_LiveView_RenderCrossHairWithLabel(
			//ZoomROIAreaPixelValues.ROIMinPixelWidth,
			//ZoomROIAreaPixelValues.ROIMinPixelHeight, true,
			//GlobalVariables::ZoomROIMinTempLabels,
			//MinCrosshairColorR, MinCrosshairColorG, MinCrosshairColorB);



		// Mark the end of a zoom window OpenGL rendering sequence
		GlobalVariables::LiveViewZoomWindowRender->RMH_OpenGL_RenderingFinishedMark();

	}
	else {

		// Reset the "ultra resolution zoom image" finished processing and ready flag
		UltraResolutionImageDataReadyZoomFlag = false;

	}

}

// ------------------------------------------------------------------------------------------ //