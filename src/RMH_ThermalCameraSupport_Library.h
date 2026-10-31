
/*
 *  RMH_ThermalCameraSupport_Library.h
 *
 *  Author: Rune Mark Hansen
 *  Date: april 2023
 *
 */

#pragma once

// RMH_ThermalCameraSupport_Library.h
#ifndef RMH_ThermalCameraSupport_Library_H 
#define RMH_ThermalCameraSupport_Library_H

// Included libraries
#include <string>
#include <vector>
#include "RMH_FrameBufferLimits.h"

// ------------------------------ Library Reference Classes ------------------------------- //

// ROI area pixel information class structure
struct ROIAreaPixelInfoFormat {

    // Maximum/minimum pixel parameters
    double MaxValue = 0.0;
    double MinValue = 0.0;
    double AvgValue = 0.0;
    unsigned int ROIMaxPixelWidth = 0;
    unsigned int ROIMaxPixelHeight = 0;
    unsigned int ROIMinPixelWidth = 0;
    unsigned int ROIMinPixelHeight = 0;
    unsigned int ROIAreaNmbOfPixels = 0;

};

// ----------------------------------- Associated Classes ----------------------------------- //

// Associated namespace for the class
namespace ThermalCameraDevice {

    // IR camera device information class 
    class IRCameraDeviceFormat {
    public:

        // Thermal camera identification index and pool variables
        unsigned int SellectedCameraIndex = 0;
        unsigned short ThermalCameraSupportPool = 0;

        // Frame pixel data width/height offset parameters
        unsigned int FrameWidthPixelOffset = 0;
        unsigned int FrameHeightPixelOffset = 0;

        // Camera frame rate variables
        double FrameRate = 0.0;
        double CameraFrameRateSum = 0.0;
        unsigned int CameraFrameRateSumCounter = 0;
        double CameraAverageFrameRate = 0.0;

        // Miscellaneous variables & objects
        std::string StatusMessage = "NAN";
        bool ConnectedFlag = false;  
        bool isStreaming = false;
        unsigned long NumbOfCapturedFrames = 0;
        unsigned char CurrentIRTempRangeFlag = 1; // '1' - LowRange, '2' - High Range 

        // Supported thermal camera pool 2 specific variables
        double ObjectEnvirTempCorrectionFactor = 0.0;
        double ObjectEnvirTempCorrectionOffset = 0.0;

        // Constant variables & objects
        std::string CameraDeviceName = "NAN";
        std::string CameraSystemDevicePath = "NAN";
        unsigned char IRCameraDeviceIndex = 0;
        double fpa_off = 0.0;
        double fpa_div = 0.0;
        double CalValue0Offset = 0.0;
        double CalValue0Fpamul = 0.0;
        unsigned int MetaData1Index = 0;
        unsigned int MetaData2Index = 0;
        unsigned int MetaData3Index = 0;
        unsigned int FrameWidth = 0;
        unsigned int FrameHeight = 0;
        unsigned int FrameMetadataSize = 0;

        // Static metadata variables & objects
        unsigned short Tmax_X = 0;            // Frame max temp X coordinate
        unsigned short Tmax_Y = 0;            // Frame max temp Y coordinate
        unsigned short Tmax_Tmp_Raw = 0;      // Frame max temp raw data
        unsigned short Tmin_X = 0;            // Frame min temp X coordinate
        unsigned short Tmin_Y = 0;            // Frame min temp Y coordinate
        unsigned short Tmin_Tmp_Raw = 0;      // Frame min temp raw data
        double Tavg_Tmp_Raw = 0;              // Frame sensor average value
        unsigned short Center_Tmp_Raw = 0;    // Frame center temp raw data
        unsigned short FrameFPARawWord = 0;   // Pool 3: raw detector temp word saved from the frame before the max/min/center data overwrites it
        unsigned short temp_fpa_Raw = 0;      // IR camera detector temp raw data
        unsigned short temp_shutter_Raw = 0;  // IR camera shutter temp raw data
        unsigned short temp_core_Raw = 0;     // IR camera core temp raw data
        double temp_fpa = 0.0;                // IR camera detector temp (calculated)
        double temp_shutter = 0.0;            // IR camera shutter temp (calculated)
        double temp_core = 0.0;               // IR camera core temp (calculated)

        // Static calibration variables
        float CalValue0 = 0.0;
        float CalValue1 = 0.0;
        float CalValue2 = 0.0;
        float CalValue3 = 0.0;
        float CalValue4 = 0.0;
        float CalValue5 = 0.0;
        float CalValue6 = 0.0;                // Pool 3: additive temperature correction (deg C) stored in the camera calibration data

        // Internal camera configuration parameter variables
        float TemperatureCorrectionSetting = 0.0;
        float ReflectedTemperatureSetting = 0.0;
        float AmbientTemperatureSetting = 0.0;
        float HumiditySetting = 0.0;
        float EmissivitySetting = 0.0;
        float DistanceSetting = 0.0;

        // Camera temperature look-up table array
        double TemperatureLookUpTabel[16384];
        
    };

}

// -------------------------------- Camera Initialization, Configuration & Handling Routines -------------------------------- //

double RMH_IRThermalCamera_ReadCameraFPS();
void RMH_IRThermalCamera_StartCapturing();
void RMH_IRThermalCamera_StopCapturing();
void RMH_IRThermalCamera_CloseIRCameraDevice();
bool RMH_IRThermalCamera_CheckForCameraDisconnection();
void RMH_IRThermalCamera_OpenIRCameraDevice(unsigned char IRCameraDeviceIndex, unsigned short SupportedCameraPool);
void RMH_IRThermalCamera_CalibrateIRCamera(unsigned short SupportedCameraPool);
void RMH_IRThermalCamera_SetIRCameraTemperatureRange(unsigned int TemperatureRange, unsigned short SupportedCameraPool);
void RMH_IRThermalCamera_InitIRCameraConstants(ThermalCameraDevice::IRCameraDeviceFormat* CameraStatus, unsigned short SupportedCameraPool);
ThermalCameraDevice::IRCameraDeviceFormat RMH_IRThermalCamera_ConnectToThermalCamera(System::Windows::Forms::ComboBox^ CameraSourceComboBox);

// ---------------------------------- Raw Image Data To Raw Thermal Data Conversion Routine ---------------------------------- //

double RMH_IRThermalCamera_ConvertYUY2To14BitThermalDataArray(ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned char* YUY2in, unsigned short* ThermalDataRaw);
void RMH_IRThermalCamera_Convert14BitThermalDataArrayToYUY2(unsigned short* ThermalData, unsigned char* YUY2Out, unsigned int FrameWidth, unsigned int FrameHeight);

// --------------------------------- Thermal Camera Region Of Interest (ROI) Handling Routine --------------------------------- //

ROIAreaPixelInfoFormat RMH_IRThermalCamera_ReadROIAreaPixelInfoInsideFrameArea(ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short* ThermalData, unsigned int FrameWidth, unsigned int AreaX0Pos, unsigned int AreaY0Pos, unsigned int AreaWidth, unsigned int AreaHeight, bool ReturnROIPixels, unsigned short* ROIAreaRawPixelValues, unsigned short SupportedCameraPool);

// --------------------------------- Thermal Camera Pool-Specific Image Processing Routine -------------------------------- //

void RMH_IRThermalCamera_LinearAutomaticGainControlTemp(unsigned short* ThermalData, unsigned short* GainGrayscale, unsigned int FrameWidth, unsigned int FrameHeight, double MaxOutPixelVal, double MinOutPixelVal, double MaxInPixelVal, double MinInPixelVal, float TempUnitScaleFactor, float TempUnitOffsetFactor);

// -------------------------------------------- Camera Frame Data Handling Routines ------------------------------------------- //

bool RMH_IRThermalCamera_ReadFrameRaw(unsigned char* ImageData, unsigned int* ImageSize);
void RMH_IRThermalCamera_ReadCalFrameMetaData(unsigned short* ThermalData, ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short SupportedCameraPool);
void RMH_IRThermalCamera_ReadCalibrationParameters(unsigned short* ThermalData, ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short SupportedCameraPool);

// --------------------------------------- Camera Configuration Data Handling Routines --------------------------------------- //

void RMH_IRThermalCamera_ReadCameraConfigParameters(unsigned short* ThermalData, ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short SupportedCameraPool);
void RMH_IRThermalCamera_WriteCameraConfigParameter(unsigned int ParameterAddress, float ParameterValue, unsigned short SupportedCameraPool);
void RMH_IRThermalCamera_SaveConfigParametersToCamera(ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short SupportedCameraPool);

// --------------------------------------------- Thermodynamic Calculation Routines --------------------------------------------- //

double RMH_IRThermalCamera_CalAtmosphericWaterVaporContribution(double Humidity, double AmbientTemp);
double RMH_IRThermalCamera_CalAtmosphericWaterVaporAttenuation(double Omega, unsigned short DistanceMeters);

// --------------------------------------------- Thermographic Calculation Routines ---------------------------------------------- //

void RMH_IRThermalCamera_GenerateThermoGrapicLookUpTable(ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short SupportedCameraPool);
double RMH_IRThermalCamera_ReadPixelTemperature(ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short PixelValue, unsigned short SupportedCameraPool);
double RMH_IRThermalCamera_ReadFramePixelTemperature(ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short* ThermalData, unsigned short PixelWidth, unsigned short PixelHeight, unsigned short SupportedCameraPool);

// ------------------------------- Recording/Snapshot Analysis Mode Camera Pool-Specific Routines ------------------------------- //

void RMH_IRThermalCamera_WriteDataToVideoRecordingFilesSequence(unsigned short SupportedCameraPool);
bool RMH_IRThermalCamera_ConvertCapturedRawImageDataToSnapshotPNG(unsigned short SupportedCameraPool);

// --------------------------------- Thermal Camera Pool-Specific Handling/Control Routines --------------------------------- //

void RMH_IRThermalCamera_AutoShutterCalTimerCallbackHandler();
void RMH_IRThermalCamera_TempDriftBasedCalTimerCallbackHandler();
void RMH_IRThermalCamera_CalibrateThermalCamera();
void RMH_IRThermalCamera_ChangeThermalCameraTemperatureRange();
void RMH_IRThermalCamera_ThermalCameraInitialConnectionEvents_Pool1();
void RMH_IRThermalCamera_ThermalCameraInitialConnectionEvents_Pool2();
void RMH_IRThermalCamera_ThermalCameraInitialConnectionEvents_Pool3();
void RMH_IRThermalCamera_ConnectToThermalCameraOrAnalysisMode();

// ------------------------------------------------------------------------------------------------------------------------------- //

#endif /* RMH_ThermalCameraSupport_Library_H */
