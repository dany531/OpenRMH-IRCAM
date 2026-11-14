/*
 *  RMH_SupportedIRCameras_Resources.h
 *
 *  Author: Rune Mark Hansen
 *  Date: November 2022
 *
 */

#pragma once

 // Included libraries
#include <string>
#include <vector>
#include <algorithm>

// RMH_SupportedIRCameras_Resources.h
#ifndef RMH_SupportedIRCameras_Resources_H 
#define RMH_SupportedIRCameras_Resources_H

// -------------------- Common Supported IR Camera Reference Macros --------------------- //

// Aspect ratio constant for InfiRay & HTI thermal cameras
#define _FixedThermalCameraFrame_AspectRatio_Pool_1      1.33333333
#define _FixedThermalCameraFrame_AspectRatio_Pool_2      1.33333333
#define _FixedThermalCameraFrame_AspectRatio_Pool_3      1.33333333
#define _FixedThermalCameraFrame_AspectRatio_Pool_4      1.33333333
#define _FixedThermalCameraFrame_AspectRatio_Pool_6      1.33333333

// Camera pool timing constants reference macros
#define _ThermalCameraShutter_CloseTimeMs                500
#define _ThermalCameraShutter_OpenTimeMs                 850
#define _ThermalCameraShutter_CALOpenTimeMs              1000

// Time [ms] the plot / data logging values stay frozen after a shutter calibration has finished
#define _CalibrationFreezeSettleTimeMs                   300
#define _ThermalCameraPool1_RangeSwitchReadyTimeMs       500
#define _ThermalCameraPool3_ReadyTimeMs                  4000
#define _ThermalCameraPool3_RangeSwitchReadyTimeMs       5000
#define _ThermalCameraPool4_RangeSwitchReadyTimeMs       5000

// IR camera temperature calculation parameter reference macros
#define _IRThermalCameraDefault_TemperatureCorrectionValue      0.0
#define _IRThermalCameraDefault_AmbientTemperatureValue         25.1    
#define _IRThermalCameraDefault_ReflectedTemperatureValue       25.1        
#define _IRThermalCameraDefault_SurroundingHumidityValue        0.45       
#define _IRThermalCameraDefault_ObjectEmissivityValue           0.98     
#define _IRThermalCameraDefault_ObjectDistanceValue             1.0  

// Supported thermal camera pool reference macros
#define _SupportedThermalCameras_Pool_1                   1
#define _SupportedThermalCameras_Pool_2                   2
#define _SupportedThermalCameras_Pool_3                   3
#define _SupportedThermalCameras_Pool_4                   4
#define _SupportedThermalCameras_Pool_5                   5
#define _SupportedThermalCameras_Pool_6                   6

// Thermal camera temperature range reference macros 
#define _ThermalCamera_TemperatureRange_HighRange         1
#define _ThermalCamera_TemperatureRange_LowRange          0

// Supported thermal camera reference index macros
#define _SnapShotAnalysisMode                             0  
#define _RecordingAnalysisMode                            1   
#define _SupportedThermalCamera_InfiRayT2L                2     
#define _SupportedThermalCamera_InfiRayT2LV2              3  
#define _SupportedThermalCamera_InfiRayT2Search           4 
#define _SupportedThermalCamera_InfiRayT2SearchV2         5  
#define _SupportedThermalCamera_InfiRayT2Sp               6   
#define _SupportedThermalCamera_InfiRayT2SpV2             7
#define _SupportedThermalCamera_InfiRayT2Pro              8  
#define _SupportedThermalCamera_InfiRayT2ProV2            9  
#define _SupportedThermalCamera_InfiRayT3Search           10          
#define _SupportedThermalCamera_InfiRayT3S                11    
#define _SupportedThermalCamera_InfiRayT3Pro              12       
#define _SupportedThermalCamera_InfiRayP2                 13   
#define _SupportedThermalCamera_InfiRayP2Pro              14     
#define _SupportedThermalCamera_InfiRayDVDL13             15    
#define _SupportedThermalCamera_InfiRayS0Series           16 
#define _SupportedThermalCamera_InfiRayTiny1C             17
#define _SupportedThermalCamera_ThermalMasterP2           18
#define _SupportedThermalCamera_HTIHT301                  19  
#define _SupportedThermalCamera_UNITUTi260M               20  
#define _SupportedThermalCamera_TOPDONTC001               21  
#define _SupportedThermalCamera_TOPDONTC002               22 
#define _SupportedThermalCamera_Victor328B                23 
#define _SupportedThermalCamera_LODESTARL2                24
#define _SupportedThermalCamera_ThermalMasterP3           25
#define _SupportedThermalCamera_ThermalMasterTHOR001      26

// Reference macros for whether a thermal camera supports a higher temperature range
#define _SupportedThermalCamera_InfiRayT2L_SupportsHighRange                false      
#define _SupportedThermalCamera_InfiRayT2LV2_SupportsHighRange              false  
#define _SupportedThermalCamera_InfiRayT2Search_SupportsHighRange           false 
#define _SupportedThermalCamera_InfiRayT2SearchV2_SupportsHighRange         false 
#define _SupportedThermalCamera_InfiRayT2Sp_SupportsHighRange               true 
#define _SupportedThermalCamera_InfiRayT2SpV2_SupportsHighRange             true  // To be added when I figure out the calculations 
#define _SupportedThermalCamera_InfiRayT2Pro_SupportsHighRange              false 
#define _SupportedThermalCamera_InfiRayT2ProV2_SupportsHighRange            false 
#define _SupportedThermalCamera_InfiRayT3Search_SupportsHighRange           false          
#define _SupportedThermalCamera_InfiRayT3S_SupportsHighRange                true    
#define _SupportedThermalCamera_InfiRayT3Pro_SupportsHighRange              true       
#define _SupportedThermalCamera_InfiRayP2_SupportsHighRange                 true     
#define _SupportedThermalCamera_ThermalMasterP2_SupportsHighRange           true 
#define _SupportedThermalCamera_InfiRayP2Pro_SupportsHighRange              true     
#define _SupportedThermalCamera_InfiRayDVDL13_SupportsHighRange             true  
#define _SupportedThermalCamera_InfiRayS0Series_SupportsHighRange           true  
#define _SupportedThermalCamera_InfiRayTiny1C_SupportsHighRange             true  
#define _SupportedThermalCamera_HTIHT301_SupportsHighRange                  true  
#define _SupportedThermalCamera_UNITUTi260M_SupportsHighRange               true  
#define _SupportedThermalCamera_TOPDONTC001_SupportsHighRange               true   
#define _SupportedThermalCamera_TOPDONTC002_SupportsHighRange               true 
#define _SupportedThermalCamera_Victor328B_SupportsHighRange                true 
#define _SupportedThermalCamera_LODESTARL2_SupportsHighRange                true     
#define _SupportedThermalCamera_ThermalMasterP3_SupportsHighRange           true
#define _SupportedThermalCamera_ThermalMasterTHOR001_SupportsHighRange      false // Unknown - not yet verified against real hardware

// Supported thermal camera frame rate reference macros
#define _SupportedThermalCamera_InfiRayT2L_FrameRate                25.0      
#define _SupportedThermalCamera_InfiRayT2LV2_FrameRate              25.0  
#define _SupportedThermalCamera_InfiRayT2Search_FrameRate           25.0   
#define _SupportedThermalCamera_InfiRayT2SearchV2_FrameRate         25.0  
#define _SupportedThermalCamera_InfiRayT2Sp_FrameRate               25.0 
#define _SupportedThermalCamera_InfiRayT2SpV2_FrameRate             25.0 
#define _SupportedThermalCamera_InfiRayT2Pro_FrameRate              25.0  
#define _SupportedThermalCamera_InfiRayT2ProV2_FrameRate            25.0 
#define _SupportedThermalCamera_InfiRayT3Search_FrameRate           25.0           
#define _SupportedThermalCamera_InfiRayT3S_FrameRate                25.0     
#define _SupportedThermalCamera_InfiRayT3Pro_FrameRate              25.0        
#define _SupportedThermalCamera_InfiRayP2_FrameRate                 25.0      
#define _SupportedThermalCamera_ThermalMasterP2_FrameRate           25.0 
#define _SupportedThermalCamera_InfiRayP2Pro_FrameRate              25.0      
#define _SupportedThermalCamera_InfiRayDVDL13_FrameRate             25.0   
#define _SupportedThermalCamera_InfiRayS0Series_FrameRate           25.0   
#define _SupportedThermalCamera_InfiRayTiny1C_FrameRate             25.0   
#define _SupportedThermalCamera_HTIHT301_FrameRate                  25.0   
#define _SupportedThermalCamera_UNITUTi260M_FrameRate               25.0   
#define _SupportedThermalCamera_TOPDONTC001_FrameRate               25.0     
#define _SupportedThermalCamera_TOPDONTC002_FrameRate               25.0  
#define _SupportedThermalCamera_Victor328B_FrameRate                25.0  
#define _SupportedThermalCamera_LODESTARL2_FrameRate                25.0  
#define _SupportedThermalCamera_ThermalMasterP3_FrameRate           25.0
#define _SupportedThermalCamera_ThermalMasterTHOR001_FrameRate      25.0

// -------------------- Supported IR Camera Pool 1 Reference Macros --------------------- //

// Specific supported thermal camera pool 1 configuration macros
#define _SupporteredeThermalCameraPool1_FrameWidthPixelOffset       0  
#define _SupporteredeThermalCameraPool1_FrameHeightPixelOffset      0 

// IR camera temperature range macros - for supported camera pool 1
#define _IRCameraPool1_TemperatureRange_HighRangeREG           0x8021
#define _IRCameraPool1_TemperatureRange_LowRangeREG            0x8020

// Internal IR camera configuration parameter addresses - for supported camera pool 1
#define _IRCameraPool1_ConfigParameterAddress_TempCorrREG      0x0000     
#define _IRCameraPool1_ConfigParameterAddress_ReflTempREG      0x0004      
#define _IRCameraPool1_ConfigParameterAddress_AmbTempREG       0x0008        
#define _IRCameraPool1_ConfigParameterAddress_HumidityREG      0x000C       
#define _IRCameraPool1_ConfigParameterAddress_EmissivityREG    0x0010         
#define _IRCameraPool1_ConfigParameterAddress_DistanceREG      0x0014  

// IR camera calibration register command - for supported camera pool 1
#define _IRCameraPool1_NUCCalibrationCommand                   0x8000

// -------------------- Supported IR Camera Pool 2 Reference Macros --------------------- //

// Specific supported thermal camera pool 2 configuration macros
#define _SupporteredeThermalCameraPool2_SensorWidthWithThermalData       256  
#define _SupporteredeThermalCameraPool2_SensorHeightWithThermalData      384  
#define _SupporteredeThermalCameraPool2_FrameWidthPixelOffset            0  
#define _SupporteredeThermalCameraPool2_FrameHeightPixelOffset           192  

// -------------------- Supported IR Camera Pool 3 Reference Macros --------------------- //

// Specific supported thermal camera pool 3 configuration macros
#define _SupporteredeThermalCameraPool3_FrameWidthPixelOffset       0  
#define _SupporteredeThermalCameraPool3_FrameHeightPixelOffset      0 

// IR camera temperature range macros - for supported camera pool 3
#define _IRCameraPool3_TemperatureRange_HighRangeREG           0x8021
#define _IRCameraPool3_TemperatureRange_LowRangeREG            0x8020

// Internal IR camera configuration parameter addresses - for supported camera pool 3
#define _IRCameraPool3_ConfigParameterAddress_TempCorrREG      0x0000     
#define _IRCameraPool3_ConfigParameterAddress_ReflTempREG      0x0004      
#define _IRCameraPool3_ConfigParameterAddress_AmbTempREG       0x0008        
#define _IRCameraPool3_ConfigParameterAddress_HumidityREG      0x000C       
#define _IRCameraPool3_ConfigParameterAddress_EmissivityREG    0x0010         
#define _IRCameraPool3_ConfigParameterAddress_DistanceREG      0x0014  

// IR camera calibration register command - for supported camera pool 3
#define _IRCameraPool3_NUCCalibrationCommand                   0x8000

// -------------------- Supported IR Camera Pool 4 Reference Macros --------------------- //

// Specific supported thermal camera pool 4 configuration macros
#define _SupporteredeThermalCameraPool4_SensorWidthWithThermalData       256  
#define _SupporteredeThermalCameraPool4_SensorHeightWithThermalData      386 
#define _SupporteredeThermalCameraPool4_FrameWidthPixelOffset            0  
#define _SupporteredeThermalCameraPool4_FrameHeightPixelOffset           194  

// -------------------- Supported IR Camera Pool 6 Reference Macros --------------------- //

// Thermal Master THOR001 (Pool 6): a standard UVC device (usbvideo.sys), unlike Pool 5 (P3). The video
// streaming interface declares a "H264" format, but the bytes actually delivered over that pipe are raw
// 16 bit sensor values - a single 256 x 192 block, with no second (display/pseudocolor) block like Pool
// 2/4's sensors, so there is no FrameHeightPixelOffset to skip. See RMH_ThermalCameraSupport_Library.cpp
// for the DirectShow video format selection (must explicitly pick the MEDIASUBTYPE_H264-labeled format).
#define _SupporteredeThermalCameraPool6_SensorWidthWithThermalData       256  
#define _SupporteredeThermalCameraPool6_SensorHeightWithThermalData      192  
// The raw sample is a fixed 10-byte header (5x "ff 00", confirmed against real hardware) immediately
// followed by 256 x 192 raw 16 bit pixel values with no other padding, so skipping 5 pixels (10 bytes)
// lines up exactly with the start of real pixel data and exactly consumes the rest of the buffer.
#define _SupporteredeThermalCameraPool6_FrameWidthPixelOffset            5  
#define _SupporteredeThermalCameraPool6_FrameHeightPixelOffset           0  

// The camera interleaves two differently-sized sample streams on the same pin (a real, variable-size H.264
// preview alongside the constant-size raw data), so DirectShow's own automatic sample-size detection never
// settles (see forceExpectedFrameBufferSize() in ds_camera.h/.cpp). This is the raw sample's exact size, in
// bytes, confirmed against real hardware.
#define _SupporteredeThermalCameraPool6_RawFrameSizeBytes                98314

// ------------------- Supported IR Camera Device Names & Manufacturers -------------------- //

// Supported thermal camera model names ->
static std::vector<std::string> SupportedCamerasModelNames = { "Snapshot Analysis Mode",
                                                               "Recording Analysis Mode", 
                                                               "InfiRay T2L",
                                                               "InfiRay T2L V2",
                                                               "InfiRay T2-Search",
                                                               "InfiRay T2-Search V2",
                                                               "InfiRay T2S+", 
                                                               "InfiRay T2S+ V2",
                                                               "InfiRay T2Pro And T2SPro", 
                                                               "InfiRay T2Pro And T2SPro V2",
                                                               "InfiRay T3-Search", 
                                                               "InfiRay T3S", 
                                                               "InfiRay T3Pro", 
                                                               "InfiRay P2",
                                                               "InfiRay Or Thermal Master P2Pro",
                                                               "InfiRay DV-DL13", 
                                                               "InfiRay S0 Series", 
                                                               "InfiRay Tiny1-C",
                                                               "Thermal Master P2",
                                                               "HTI HT-301",
                                                               "UNI-T UTi260M",
                                                               "TOPDON TC001 or TS001", 
                                                               "TOPDON TC002 or TC003",
                                                               "Victor 328B", 
                                                               "LODESTAR L2",
                                                               "Thermal Master P3",
                                                               "Thermal Master THOR001"};

// The camera selection ComboBox is not populated from SupportedCamerasModelNames directly - it is populated
// in the order below instead, so that adding a new camera only ever means appending a new macro index and a
// new SupportedCamerasModelNames entry (as above), without renumbering anything or hand-sorting any list.
//
// SupportedCamerasDisplayOrder[i] is the camera macro index shown at ComboBox position i: position 0 and 1
// are always "Snapshot Analysis Mode" and "Recording Analysis Mode" (matching their fixed macro values 0
// and 1), and every other position is one of the real camera macro indices (2 upward), sorted alphabetically
// (case-insensitive) by its SupportedCamerasModelNames entry. Every place that turns a ComboBox selection
// into a camera identity (or vice-versa) must go through this array or RMH_FindCameraDisplayPosition() below
// - see RMH_IRThermalCamera_ConnectToThermalCamera() and ThermalCameraGUI.h's camera ComboBox handling.
inline std::vector<int> RMH_BuildCameraDisplayOrder() {

	std::vector<int> DisplayOrder;
	DisplayOrder.push_back(_SnapShotAnalysisMode);
	DisplayOrder.push_back(_RecordingAnalysisMode);

	std::vector<int> CameraIndices;
	for (int i = 2; i < (int)SupportedCamerasModelNames.size(); i++) { CameraIndices.push_back(i); }

	std::sort(CameraIndices.begin(), CameraIndices.end(), [](int a, int b) {
		std::string NameA = SupportedCamerasModelNames[a];
		std::string NameB = SupportedCamerasModelNames[b];
		std::transform(NameA.begin(), NameA.end(), NameA.begin(), ::tolower);
		std::transform(NameB.begin(), NameB.end(), NameB.begin(), ::tolower);
		return NameA < NameB;
	});

	DisplayOrder.insert(DisplayOrder.end(), CameraIndices.begin(), CameraIndices.end());
	return DisplayOrder;

}

static std::vector<int> SupportedCamerasDisplayOrder = RMH_BuildCameraDisplayOrder();

// Reverse lookup for SupportedCamerasDisplayOrder - given a camera's stable macro index (for example one
// read back from a saved session file), returns the ComboBox position it is currently displayed at, or -1
// if it is not found (an old or hand-edited saved session referencing a since-removed camera).
inline int RMH_FindCameraDisplayPosition(int CameraIndex) {

	for (int i = 0; i < (int)SupportedCamerasDisplayOrder.size(); i++) {

		if (SupportedCamerasDisplayOrder[i] == CameraIndex) { return i; }

	}

	return -1;

}

// The inverse of RMH_FindCameraDisplayPosition(): given a ComboBox position, returns the camera's stable
// macro index at that position, or _SnapShotAnalysisMode (0) if the position is out of range. A ComboBox
// can end up with no selection (SelectedIndex == -1) after RMH_Winforms_CombiBox_SetSellectedItemPosition()
// silently declines to select a saved camera ID that RMH_FindCameraDisplayPosition() could not find (an old
// or hand-edited saved session file) - indexing SupportedCamerasDisplayOrder with that -1 directly would be
// undefined behaviour, so every read of a ComboBox position must go through this function instead.
inline int RMH_GetCameraIndexAtDisplayPosition(int Position) {

	if (Position < 0 || Position >= (int)SupportedCamerasDisplayOrder.size()) { return _SnapShotAnalysisMode; }

	return SupportedCamerasDisplayOrder[Position];

}

// Supported camera device names - InfiRay T2L - supported pool 1
static std::vector<std::string> InfiRayT2LDeviceNames = { "T2L-A4L", "T2L-A6L", "T2L-A8L", "T2L", "T2L-A4L_R", "T2L-A4L_A", "T2L-A4L_C" };

// Supported camera device names - InfiRay T2L V2 - supported pool 3
static std::vector<std::string> InfiRayT2LV2DeviceNames = { "T2L-A4L", "T2L-A6L", "T2L-A8L", "T2L_V2", "T2L", "T2L_A2", "T2L-A4L_A2", "T2L-A4L_V2" };

// Supported camera device names - InfiRay T2-Search - supported pool 1
static std::vector<std::string> InfiRayT2SearchDeviceNames = { "T2-Search", "T2", "T2S"};

// Supported camera device names - InfiRay T2-Search V2 - supported pool 3
static std::vector<std::string> InfiRayT2SearchV2DeviceNames = { "T2_V2", "T2-Search", "T2", "T2S", "T2_A2", "T2S_A2" };

// Supported camera device names - InfiRay T2S+ - supported pool 1  
static std::vector<std::string> InfiRayT2SpDeviceNames = { "T2S+", "T2Sp", "T2S+_A", "T2S+_C", "T2S+_R", "T2SPro" };

// Supported camera device names - InfiRay T2S+ V2 - supported pool 3  
static std::vector<std::string> InfiRayT2SpV2DeviceNames = { "T2S+_V2", "T2S+_A", "T2S+_A2", "T2S+", "T2Sp", "T2SPro" };

// Supported camera device names - InfiRay T2Pro - supported pool 1
static std::vector<std::string> InfiRayT2ProDeviceNames = { "T2Pro", "T2+", "T2p", "T2P", "T2SPro" };

// Supported camera device names - InfiRay T2Pro V2 - supported pool 3
static std::vector<std::string> InfiRayT2ProV2DeviceNames = { "T2Pro_V2", "T2Pro_A1", "T2Pro_A2", "T2Pro", "T2pro", "T2P", "T2SPro"};

// Supported camera device names - InfiRay T3-Search - supported pool 1
static std::vector<std::string> InfiRayT3SearchDeviceNames = { "T3-Search", "T3" };

// Supported camera device names - InfiRay T3S - supported pool 1
static std::vector<std::string> InfiRayT3SDeviceNames = { "Xtherm-T3S", "T3S-A68", "T3S-A13", "T3S", "T3S-A13_A", "T3S-A13_C", "T3S-A13_R" };

// Supported camera device names - InfiRay T3Pro - supported pool 1
static std::vector<std::string> InfiRayT3ProDeviceNames = { "T3Pro-A13", "T3Pro-A68", "T3Pro", "T3Pro-A13_C", "T3Pro-A13_A", "T3Pro-A13_R" };

// Supported camera device names - InfiRay P2 - supported pool 2
static std::vector<std::string> InfiRayP2DeviceNames = { "USB Camera", "Camera" };

// Supported camera device names - Thermal Master P2 - supported pool 4
static std::vector<std::string> ThermalMasterP2DeviceNames = { "Camera" };

// Supported camera device names - InfiRay P2Pro - supported pool 2 
static std::vector<std::string> InfiRayP2ProDeviceNames = { "USB Camera", "Camera" };

// Supported camera device names - InfiRay DV-DL13 - supported pool 1
static std::vector<std::string> InfiRayDVDL13DeviceNames = { "VirtualBox Webcam - DV-DL13", "DV-DL13" };

// Supported camera device names - InfiRay S0 Series - supported pool 1
static std::vector<std::string> InfiRayS0SeriesDeviceNames = { "S0-90W", "S0-40", "S0-68", "S0-90" };

// Supported camera device names - InfiRay Tiny1-C - supported pool 2
static std::vector<std::string> InfiRayTiny1CDeviceNames = { "USB Camera", "Tiny1C" };

// Supported camera device names - HTI HT-301 - supported pool 1
static std::vector<std::string> HTIHT301DeviceNames = { "T3", "T3-317-13", "T3-317-68", "HT-301"};

// Supported camera device names - UNI-T UTi260M - supported pool 2
static std::vector<std::string> UNITUTi260MDeviceNames = { "USB Camera" };

// Supported camera device names - TOPDON TC001 - supported pool 2
static std::vector<std::string> TOPDONTC001DeviceNames = { "USB Camera", "TC001" };

// Supported camera device names - TOPDON TC002 - supported pool 2
static std::vector<std::string> TOPDONTC002DeviceNames = { "USB Camera", "TC002", "TC003"};

// Supported camera device names - Victor 328B - supported pool 2
static std::vector<std::string> Victor328BDeviceNames = { "USB Camera" };

// Supported camera device names - LODESTAR L2 - supported pool 2
static std::vector<std::string> LODESTARL2DeviceNames = { "USB Camera" };

// Supported camera device names - Thermal Master P3 - supported pool 4
static std::vector<std::string> ThermalMasterP3DeviceNames = { "P3" };

// Supported camera device names - Thermal Master THOR001 - supported pool 6
static std::vector<std::string> ThermalMasterTHOR001DeviceNames = { "THOR001" };

// ------------------------------------------------------------------------------------------ //

#endif /* RMH_SupportedIRCameras_Resources_H */