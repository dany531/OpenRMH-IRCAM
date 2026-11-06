/*
 *  RMH_Application_ThermalViewer.h
 *
 *  Author: Rune Mark Hansen
 *  Date: May 2023
 *
 */

#pragma once

// RMH_Application_ThermalViewer.h
#ifndef RMH_Application_ThermalViewer_H 
#define RMH_Application_ThermalViewer_H

// ---------------- Custom build identification (title bar and start-up status message) ---------------- //
#define _CustomBuild_TitleSuffix                  " - CUSTOM BUILD FOR THE INFIRAY T2S+ V2 (T2S+_A2)"
#define _CustomBuild_StartupMessage               "Custom IRCAM Version - Built Specifically For The InfiRay T2S+ V2 (T2S+_A2) Thermal Camera."

// ---------------- Automatic start-up sequence (selected camera, calibration, periodic calibration) ---------------- //
// When enabled, the application selects the camera below, connects to it, calibrates it and enables periodic shutter
// calibration automatically after the main window has opened. Set _AutoStart_Enabled to 0 to disable all of it.
#define _AutoStart_Enabled                        1
#define _AutoStart_ShowPopupWindows              1       // Show the start-up information, ready and camera-not-connected windows
#define _AutoStart_CameraIndex                    _SupportedThermalCamera_InfiRayT2SpV2   // "InfiRay T2S+ V2"
#define _AutoStart_ConnectDelayMs                 1500    // Delay after the main window opens before connecting
#define _AutoStart_StreamWindowDelayMs            4000    // Delay after connecting before switching to the stream window (let the video start)
#define _AutoStart_CalibrateDelayMs               1500    // Delay after switching to the stream window before pressing the Calibrate Camera button (hotkey Q)
#define _AutoStart_PeriodicCalibrationPeriodSec   180     // "Auto Calibration Period [Sec]" for periodic shutter calibration

// Camera configuration panel NumericUpDowns "start-up" max/min limits
#define _TempCorrectionUpDown_DefaultMaxValue     1000.0
#define _TempCorrectionUpDown_DefaultMinValue    -1000.0
#define _AmbientTempUpDown_DefaultMaxValue        85.0
#define _AmbientTempUpDown_DefaultMinValue       -20.0
#define _ReflectedTempUpDown_DefaultMaxValue      85.0
#define _ReflectedTempUpDown_DefaultMinValue     -20.0
#define _DistanceUpDown_DefaultMaxValue           100
#define _DistanceUpDown_DefaultMinValue           1

// ----------------- Application Feature Enable Handling Routines ------------------ //

void RMH_Application_DisableMainGUIMenuButtons();
void RMH_Application_EnableApplicationFeatures();

// ----------------------- Camera Configuration Handling Routines ----------------------- //

void RMH_ThermalViewer_EnableCameraConfigurationControls(bool EnableState);
void RMH_ThermalViewer_ReadAndDisplayCameraConfigParameters();
void RMH_ThermalViewer_ReadAndDisplayRAWVideoFileCameraConfigParameters();
void RMH_ThermalViewer_ReadAndDisplayRAWSnapShotFileCameraConfigParameters();
void RMH_ThermalViewer_SetCameraConfigParameters();
void RMH_ThermalViewer_SetCameraConfigUpDownRanges(float TemperatureUnitScaleFactor, float TemperatureUnitOffsetFactor);
void RMH_ThermalViewer_RecoverDefaultCameraTempConfiguration();

// ---------------- 2D Temperature Plot Handling & Configuration Routines ---------------- //

void RMH_ThermalViewer_Set2DPlotDataSetSource(double** Plot2DDataSetSourcePointer, unsigned char DataSource);
void RMH_ThermalViewer_Enable2DPlotDataSet(System::Object^ sender);
void RMH_ThermalViewer_Load2DPlotLineColorDataToGlobalArrays();
void RMH_ThermalViewer_Load2DPlotSavedSessionLineColorData();
void RMH_ThermalViewer_Change2DPlotDataSetAndSettingsPanelColor(System::Object^ sender);
void RMH_ThermalViewer_Change2DPlotDataSetLineWidth(System::Object^ sender);
void RMH_ThermalViewer_Change2DPlotDataSetSource(System::Object^ sender);
void RMH_ThermalViewer_Update2DPlotLegendLabels();

// --------------------------- Data Logging Handling Routines ---------------------------- //

void RMH_ThermalViewer_SetDataLoggingSourcePointer(double** DataLoggingSourcePointer, unsigned char DataSource);
void RMH_ThermalViewer_ChangeDataLoggingDataSetSource(unsigned char DataSetIndex, unsigned char DataSourceIndex);
double* RMH_ThermalViewer_GetDataLoggingSourcePointerFromIndex(unsigned char DataSourceIndex);
void RMH_ThermalViewer_UpdateDataLoggingDefaultSaveFilePath(System::Windows::Forms::Label^ DefaultPathString);
void RMH_ThermalViewer_StartDataLogging();
void RMH_ThermalViewer_StopDataLogging();
void RMH_ThermalViewer_UpdateDataLoggingCSVDataDelimiter();
void RMH_ThermalViewer_DataLoggingThreadProcess();

// ------------------------ Temperature Alarm Handling Routines ------------------------ //

void RMH_ThermalViewer_SetTempAlarmDataSourcePointer(double** TempAlarmSourcePointer, unsigned char AlarmDataSource);
void RMH_ThermalViewer_ChangeTemperatureAlarmDataSource(System::Object^ sender);
void RMH_ThermalViewer_ChangeTempAlarmConfigType(System::Object^ sender);
void RMH_ThermalViewer_ChangeTempAlarmLowTempSetPoint(System::Object^ sender);
void RMH_ThermalViewer_ChangeTempAlarmHighTempSetPoint(System::Object^ sender);
void RMH_ThermalViewer_ChangeTempAlarmTriggerAction(System::Object^ sender);
void RMH_ThermalViewer_EnableTemperatureAlarm(System::Object^ sender);
void RMH_ThermalViewer_UpdateTempAlarmsStatusLabels();
void RMH_ThermalViewer_ReadTemperatureAlarmStatus(unsigned char TemperatureAlarmIndex, double TemperatureAlarmSourcePointer);
void RMH_ThermalViewer_MonitorEnabledTempAlarmsStatus();
void RMH_ThermalViewer_OpdateTempAlarmTriggerSoundTimer(System::Object^ sender);
void RMH_ThermalViewer_AlarmSoundTimerTickEventHandler();
void RMH_ThermalViewer_EnableAlarmTriggerEvents(System::Object^ sender);
void RMH_ThermalViewer_UpdateAlarmsTriggerEventResetButtonsBorderColor(unsigned char TemperatureAlarmIndex, bool TriggerEventExecutedFlag);
void RMH_ThermalViewer_ResetAlarmTriggerEventExecutedFlag(System::Object^ sender);
void RMH_ThermalViewer_HandleTempAlarmTriggerActionEvent(unsigned char TemperatureAlarmIndex);
void RMH_ThermalViewer_AlarmTriggerEventTimerTickEventHandler();

// ------------------- General And Periodic Trigger Handling Routines -------------------- //

void RMH_ThermalViewer_TogglePeriodicTriggerTimer();
void RMH_ThermalViewer_EnableDisableSelectedPeriodicTriggerEvent(System::Object^ sender);
void RMH_ThermalViewer_DisablePeriodicTriggerEventIndex(unsigned int PeriodicTriggerEvent);
void RMH_ThermalViewer_ExecuteTriggerEventIndex(unsigned short TriggerEventFunction);
void RMH_ThermalViewer_ExecuteSelectedPeriodicTriggerEvent(unsigned int PeriodicTriggerEvent);
void RMH_ThermalViewer_PeriodicTriggerEventTimerTickEventHandler();

// ---------------------- Emissivity Table Screen Handling Routines ----------------------- //

void RMH_ThermalViewer_LoadEmissivisyTableValueToThermalCamera(System::Windows::Forms::DataGridViewCellEventArgs^ e);

// --------------------- Camera Auto Calibration Handling Routines ---------------------- //

void RMH_ThermalViewer_ToggleCameraAutoShutterCalibrationTimer();

// Automatic start-up sequence routines
bool RMH_ThermalViewer_AutoStartSelectAndConnectCamera();
bool RMH_ThermalViewer_AutoStartEnablePeriodicCalibration();
void RMH_ThermalViewer_ToggleCameraDriftBasedCalibrationTimer();
void RMH_ThermalViewer_ReadThermalCameraInternalTemps();

// ------------------------ Camera Disconnect Handling Routines ------------------------- //

void RMH_ThermalViewer_HandleSellectedDeviceOrModeChange();
void RMH_ThermalViewer_HandleCameraDisconnectedEvents(bool ShowStatusMEssageFlag);

// --------------------- Camera Temperature Unit Handling Routines ---------------------- //

void RMH_ThermalViewer_ChangeTemperatureUnit(System::Object^ sender);

// ----------------------- Temperature Tracking Handling Routines ------------------------ //

void RMH_ThermalViewer_ReadMaxMinCentTemperatures();
void RMH_ThermalViewer_FormatMaximumTemperatureLabel();
void RMH_ThermalViewer_FormatMinimumTemperatureLabel();
void RMH_ThermalViewer_FormatCenterTemperatureLabel();
void RMH_ThermalViewer_ReadAndFormatMouseCursorTempAndLabel();
void RMH_ThermalViewer_ReadAndFormatROITempAndLabels();
void RMH_ThermalViewer_ReadAndFormatTempMeasurementsAndLabels();
void RMH_ThermalViewer_ReadAndFormatLinesMaxMinAvgTempsAndLabels();
void RMH_ThermalViewer_ToggleMaximumTempTracking();
void RMH_ThermalViewer_ToggleMinimumTempTracking();
void RMH_ThermalViewer_ToggleCenterTempTracking();
void RMH_ThermalViewer_ToggleMouseCursorTempTracking();

// --------------------- Live View Statistics Data Handling Routines ---------------------- //

void RMH_ThermalViewer_CalLiveViewStatisticsData();
void RMH_ThermalViewer_UpdateAndFormatLiveViewStatisticsLabels();

// ------------------ Fixed Temperature Label Tracking Handling Routines ------------------ //

void RMH_ThermalViewer_UpdateTempMeasurementsRenderingOrder();
void RMH_ThermalViewer_AddTemperatureMeasurementToLiveView();
void RMH_ThermalViewer_DeleteTemperatureMeasurementFromLiveView(System::Object^ sender);
void RMH_ThermalViewer_DeleteAllTemperatureMeasurementFromLiveView();

// --------------------- ROI Temperature Tracking Handling Routines ---------------------- //

void RMH_ThermalViewer_UpdateROIRenderingOrder();
void RMH_ThermalViewer_AddRegionOfInterestBoxToLiveView();
void RMH_ThermalViewer_DeleteRegionOfInterestBoxFromLiveView(System::Object^ sender);
void RMH_ThermalViewer_DeleteAllRegionOfInterestBoxFromLiveView();

// --------------------- Temperature Line Tracking Handling Routines --------------------- //

void RMH_ThermalViewer_UpdateTempLinesRenderingOrder();
void RMH_ThermalViewer_AddTemperatureLineToLiveView();
void RMH_ThermalViewer_DeleteTemperatureLineFromLiveView(System::Object^ sender);
void RMH_ThermalViewer_DeleteAllTemperatureLinesFromLiveView();

// ------------------------ Live View Histogram Handling Routines ------------------------ //

void RMH_ThermalViewer_EnableLiveViewHistogram();
void RMH_ThermalViewer_ChangeHistoramDataSource(System::Object^ sender);

// -------------- Save Full-Frame Temperature Data To CSV Handling Routines --------------- //

void RMH_ThermalViewer_UpdateFullFrameTemperatureCSVDataDelimiter();
void RMH_ThermalViewer_SaveFullFrameTemperatureDataToCSVFile();

// ----------- Live View Stream Video Recording And Snapshot Handling Routines ------------ //

void RMH_ThermalViewer_UpdateSnapshotDefaultSaveFilePath(System::Windows::Forms::Label^ DefaultPathString);
void RMH_ThermalViewer_UpdateVideoRecordingDefaultSaveFilePath(System::Windows::Forms::Label^ DefaultPathString);
void RMH_ThermalViewer_IncludeColorBarInSnapshot();
void RMH_ThermalViewer_ToggleSavinfOfRawSensorDataSnapshot();
void RMH_ThermalViewer_SaveLiveViewSnapshot();
void RMH_ThermalViewer_SaveSurfacePlotSnapshot();
void RMH_ThermalViewer_ConfigDefaultCapturingProgram(System::Object^ sender);
void RMH_ThermalViewer_OpenDefaultVideoCapturingApp();
void RMH_ThermalViewer_ToggleRecordingOfRAWDataForPostAnalysis();
void RMH_ThermalViewer_StartStopVideoRecording();

// --------------------- Live View Stream Run/Stop Handling Routines --------------------- //

void RMH_ThermalViewer_ToggleLiveViewStreamRunStop();
void RMH_ThermalViewer_TriggerLiveViewSingleFrameCapture();

// ------------------ Thermal Camera Image Processing Loop Routines ------------------- //

void RMH_ThermalViewer_FrameCrabberCallback();
void RMH_ThermalViewer_ImageProcessingSequence();
void RMH_ThermalViewer_SecondaryProcessingSequence();
void RMH_ThermalViewer_ToggleEnhancedLiveViewResolution();
void RMH_ThermalViewer_ToggleLiveViewImageSharpening();
void RMH_ThermalViewer_ToggleLiveViewUltraResolution();

// --------------- Live View Aspect Ratio Button And Event Handling Routines ---------------- //

void RMH_ThermalViewer_UpdateAspectRatioButtonBorderColor();

// ------------------ Surface Plot Menu Update And Handling Routines ----------------- //

void RMH_ThermalViewer_UpdateSurfacePlotMenuScreen(unsigned int SurfacePlotPanelWidth, unsigned int SurfacePlotPanelHeight);

// ----------------------- 2D Plot Update And Handling Routines ---------------------- //

void RMH_ThermalViewer_Update2DPlotMenuScreen(unsigned int PlotPanelWidth, unsigned int PlotPanelHeight);

// ----------------- Temperature Alarm Update And Handling Routines ----------------- //

void RMH_ThermalViewer_UpdateTemperatureAlarmsSubMenuStatusLabels();

// ------------------- Live View Menu Update And Handling Routines ------------------- //

void RMH_ThermalViewer_WriteDataToVideoRecordingFilesSequence();
void RMH_ThermalViewer_UpdateLiveView(unsigned int LiveViewPanelWidth, unsigned int LiveViewPanelHeight, bool FixedAspectRatio, unsigned int ColorBarPanelWidth, unsigned int ColorBarPanelHeight, unsigned int HistogramPanelWidth, unsigned int HistogramPanelHeight);

// ------------------------------------------------------------------------------------------ //

#endif /* RMH_Application_ThermalViewer_H */
