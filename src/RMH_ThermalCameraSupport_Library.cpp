
/*
 *  RMH_ThermalCameraSupport_Library.c
 *
 *  Author: Rune Mark Hansen
 *  Date: November 2022
 *
 */

// Included libraries
#include <string>
#include <math.h>
#include <iostream>
#include <cstdlib> 
#include "uvc_camera.h"
#include "p3_winusb_camera.h"
#include "GlobalObjectsAndVariables.h"
#include "RMH_MathConversions_Library.h"
#include "RMH_ThermalCameraSupport_Library.h"
#include "RMH_SupportedIRCameras_Resources.h"
#include "RMH_Application_ThermalViewer.h"

// Associated namespaces
using namespace DirectShowCamera;
using namespace ThermalCameraDevice;
using namespace System::Diagnostics;

// Global objects and variables
UVCCamera IRThermalCamera = UVCCamera();
std::vector<CameraDevice> IRThermalCameraDeivceList;
// Pool 5 - Thermal Master P3, which has no DirectShow/UVC interface at all and is only reachable over WinUSB
P3WinUsbCamera P3Camera;

// ----------------------- USB Communication, Read/Write, Configuration & Handling Routines ------------------------ //

// ------------------------------------------------------------------------------------------------------------------------------- //
//																																   //
//									Routines & Functions Supporting the Following Cameras (Pool 1) ->							   //
//																																   //
//									- InfiRay T2L																				   //
//									- InfiRay T2-Search																			   //
//									- InfiRay T2S+																				   //
//									- InfiRay T2Pro																				   //
//									- InfiRay T3-Search																			   //
//									- InfiRay T3S																				   //
//									- InfiRay T3Pro																				   //
//									- InfiRay DV-DL13	                                                                           //
//									- InfiRay S0 Series																			   //
//									- HTI HT-301																				   //
//																																   //
//									Routines & Functions Supporting the Following Cameras (Pool 2) ->							   //
//																																   //
//									- InfiRay Tiny1-C																			   //
//									- InfiRay P2																				   //
//									- InfiRay P2Pro + Thermal Master P2Pro                                                         //
//									- UNI-T UTi260M                                                                                //  
//									- TOPDON TC001                                                                                 //
//                                  - TOPDON TC002																				   //
//                                  - Victor 328B																				   //
//                                  - LODESTAR L2																				   //
//																																   //
//									Routines & Functions Supporting the Following Cameras (Pool 3) ->							   //
//																																   //
//									- InfiRay T2L V2	    																	   //
//									- InfiRay T2-Search V2																		   //
//									- InfiRay T2S+ V2																			   //
//									- InfiRay T2Pro V2																			   //
//																																   //
//									Routines & Functions Supporting the Following Cameras (Pool 3) ->							   //
//																																   //
//									- Thermal Master P2																			   //
//																																   //
// ------------------------------------------------------------------------------------------------------------------------------- //

// -------------------------------- Camera Initialization, Configuration & Handling Routines -------------------------------- //

double RMH_IRThermalCamera_ReadCameraFPS() {

	// This routine reads and returns the frame rate of the connected thermal camera 

	// Read and return the camera frame rate
	if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_5) { return P3Camera.getFPS(); }
	return IRThermalCamera.getFPS();

}

void RMH_IRThermalCamera_StartCapturing() {

	// This routine starts the video capture of the camera

	// Pool 5 (Thermal Master P3) is not a DirectShow device - it has its own WinUSB backend
	if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_5) {

		if (P3Camera.isOpened() == true) {

			if (P3Camera.startCapture() == false) {

				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "P3 startCapture() failed: " + P3Camera.getLastError(), _StatusMessageType_Error);

			}

		}
		return;

	}

	// Check that the camera is open
	if (IRThermalCamera.isOpened() == true) {

		// Start video capture
		IRThermalCamera.startCapture();

		// Pool 6 (THOR001): DirectShowCamera::start() (called from within startCapture() above) always calls
		// updateGrabberFilterVideoFormat(), which resets the grabber callback's expected sample size back to
		// the format's bogus declared size (614400) - undoing forceExpectedFrameBufferSize() set during
		// RMH_IRThermalCamera_OpenIRCameraDevice(). This function is the last thing that (re)starts capture
		// before frames are actually read, so the override is (re)applied here every time, right after it.
		if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_6) {

			IRThermalCamera.forceExpectedFrameBufferSize(_SupporteredeThermalCameraPool6_RawFrameSizeBytes);

		}

	}

}

void RMH_IRThermalCamera_StopCapturing() {

	// This routine stops the video capture of the camera

	// Pool 5 (Thermal Master P3) is not a DirectShow device - it has its own WinUSB backend
	if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_5) {

		if (P3Camera.isOpened() == true) { P3Camera.stopCapture(); }
		return;

	}

	// Check that the camera is open
	if (IRThermalCamera.isOpened() == true) {

		// Stop video capture
		IRThermalCamera.stopCapture();

	}

}

void RMH_IRThermalCamera_CloseIRCameraDevice() {

	// This routine stops and closes the DirectShow webcam device
	// which also disables video streaming

	// Pool 5 (Thermal Master P3) is not a DirectShow device - it has its own WinUSB backend
	if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_5) {

		P3Camera.stopCapture();
		P3Camera.close();
		return;

	}

	// Close the video capture device
	IRThermalCamera.stopCapture();
	IRThermalCamera.close();

}

bool RMH_IRThermalCamera_CheckForCameraDisconnection() {

	// This routine checks whether the camera connection was lost

	// Pool 5 (Thermal Master P3) is not a DirectShow device - it has its own WinUSB backend
	if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_5) { return P3Camera.checkDisconnection(); }

	// Return the connection status
	return IRThermalCamera.checkDisconnection();

}

void RMH_IRThermalCamera_OpenIRCameraDevice(unsigned char IRCameraDeviceIndex, unsigned short SupportedCameraPool) {

	// This routine opens the DirectShow webcam device with the given index - which also enables video streaming

	// Which supported camera pool is selected
	switch (SupportedCameraPool) {

		// Supported camera pool 1
		case _SupportedThermalCameras_Pool_1:

			// Open the selected video capture device - supported camera pool 1
			IRThermalCamera.open(IRThermalCameraDeivceList[IRCameraDeviceIndex]);

			// Configure the camera to deliver RAW data
			IRThermalCamera.setZoom(0x8004);

		break;

		// Supported camera pool 2
		case _SupportedThermalCameras_Pool_2:

			// Open the selected video capture device - supported camera pool 2
			IRThermalCamera.open(IRThermalCameraDeivceList[IRCameraDeviceIndex], _SupporteredeThermalCameraPool2_SensorWidthWithThermalData, _SupporteredeThermalCameraPool2_SensorHeightWithThermalData);

		break;

		// Supported camera pool 3
		case _SupportedThermalCameras_Pool_3:

			// Open the selected video capture device - supported camera pool 3
			IRThermalCamera.open(IRThermalCameraDeivceList[IRCameraDeviceIndex]);

			// Configure the camera to deliver RAW data
			//IRThermalCamera.setZoom(0x8081); // Not needed
			IRThermalCamera.setZoom(0x8005);
			//IRThermalCamera.setZoom(0x8004); // Not needed

		break;

		// Supported camera pool 4
		case _SupportedThermalCameras_Pool_4:

			// Open the selected video capture device - supported camera pool 4
			IRThermalCamera.open(IRThermalCameraDeivceList[IRCameraDeviceIndex], _SupporteredeThermalCameraPool4_SensorWidthWithThermalData, _SupporteredeThermalCameraPool4_SensorHeightWithThermalData);

		break;

		// Supported camera pool 6
		case _SupportedThermalCameras_Pool_6:

			// Open the selected video capture device with its default format first - Pool 6's raw data
			// is delivered on a format the camera's own descriptor mislabels as "H264" (see
			// RMH_SupportedIRCameras_Resources.h), which the normal open(device, width, height) overload
			// would refuse (it only accepts RGB/monochrome-convertible subtypes), so the video format has
			// to be looked up and selected explicitly here instead.
			IRThermalCamera.open(IRThermalCameraDeivceList[IRCameraDeviceIndex]);

			{
				std::vector<DirectShowVideoFormat> Pool6VideoFormats = IRThermalCamera.getSupportDirectShowVideoFormats();

				for (unsigned int i = 0; i < Pool6VideoFormats.size(); i++) {

					if (Pool6VideoFormats[i].getVideoType() == MEDIASUBTYPE_H264) {

						IRThermalCamera.setDirectShowVideoFormat(&Pool6VideoFormats[i]);
						break;

					}

				}
			}

			// Start video capture explicitly - Pool 6 does not go through a vendor "start stream" step like
			// Pool 5 (P3), but the DirectShow graph still needs Run() called before samples flow.
			IRThermalCamera.startCapture();

			// The camera interleaves a real, variable-size H.264 preview stream with the constant-size raw
			// data on the same pin, so the grabber callback's automatic "5 identical sizes in a row" sample
			// size detection never settles (confirmed via SampleCB diagnostic logging against real hardware:
			// sizes alternated between 98314 and ~13000-14000 every sample, never repeating). Force the
			// expected size directly to the raw sample's real, confirmed size instead - this also has the
			// side effect of making the grabber callback silently ignore the interleaved preview samples,
			// since they will never match this fixed size. This MUST happen after startCapture() - start()
			// internally calls updateGrabberFilterVideoFormat(), which would otherwise overwrite this back
			// to the format's bogus declared size.
			IRThermalCamera.forceExpectedFrameBufferSize(_SupporteredeThermalCameraPool6_RawFrameSizeBytes);

		break;

	}

	// Configure the camera to the default temperature range
	RMH_IRThermalCamera_SetIRCameraTemperatureRange(_ThermalCamera_TemperatureRange_LowRange, SupportedCameraPool);

	// Calibrate the thermal camera
	RMH_IRThermalCamera_CalibrateIRCamera(SupportedCameraPool);

}

void RMH_IRThermalCamera_CalibrateIRCamera(unsigned short SupportedCameraPool) {

	// This routine performs an IR camera shutter calibration

	// Which supported camera pool is selected
	switch (SupportedCameraPool) {

		// Supported camera pool 1
		case _SupportedThermalCameras_Pool_1:

			// Perform an IR camera calibration
			IRThermalCamera.setZoom(_IRCameraPool1_NUCCalibrationCommand);

		break;

		// Supported camera pool 2
		case _SupportedThermalCameras_Pool_2:


		break;

		// Supported camera pool 3
		case _SupportedThermalCameras_Pool_3:

			// Perform an IR camera calibration
			IRThermalCamera.setZoom(_IRCameraPool3_NUCCalibrationCommand);

		break;

		// Supported camera pool 4
		case _SupportedThermalCameras_Pool_4:


		break;

		// Supported camera pool 5
		case _SupportedThermalCameras_Pool_5:

			// Trigger the camera's own shutter (NUC) calibration
			if (P3Camera.triggerShutterCalibration() == false) {

				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "P3 triggerShutterCalibration() failed: " + P3Camera.getLastError(), _StatusMessageType_Error);

			}

		break;

	}

}

void RMH_IRThermalCamera_SetIRCameraTemperatureRange(unsigned int TemperatureRange, unsigned short SupportedCameraPool) {

	// This routine configures the temperature range of the IR camera

	/*
	 *  Associated macros ->
	 *
	 *  // Thermal Camera Temperature Range Reference Macros 
	 *  #define _ThermalCamera_TemperatureRange_HighRange           1
	 *  #define _ThermalCamera_TemperatureRange_LowRange            0
	 *
	 */

	 // Which supported camera pool is selected
	switch (SupportedCameraPool) {

		// Supported camera pool 1
		case _SupportedThermalCameras_Pool_1:

			// Which temperature range should be set
			switch (TemperatureRange) {

				// This routine configures the temperature range of the IR camera
				case _ThermalCamera_TemperatureRange_HighRange: IRThermalCamera.setZoom(_IRCameraPool1_TemperatureRange_HighRangeREG); break;
				case _ThermalCamera_TemperatureRange_LowRange:  IRThermalCamera.setZoom(_IRCameraPool1_TemperatureRange_LowRangeREG);  break;

			}

		break;

		// Supported camera pool 2
		case _SupportedThermalCameras_Pool_2:

			// Which temperature range should be set
			switch (TemperatureRange) {

				// This routine configures the temperature range of the IR camera
				case _ThermalCamera_TemperatureRange_HighRange: break;
				case _ThermalCamera_TemperatureRange_LowRange:  break;

			}

		break;

		// Supported camera pool 3
		case _SupportedThermalCameras_Pool_3:

			// Which temperature range should be set
			switch (TemperatureRange) {

				// This routine configures the temperature range of the IR camera
				case _ThermalCamera_TemperatureRange_HighRange: IRThermalCamera.setZoom(_IRCameraPool3_TemperatureRange_HighRangeREG); break;
				case _ThermalCamera_TemperatureRange_LowRange:  IRThermalCamera.setZoom(_IRCameraPool3_TemperatureRange_LowRangeREG);  break;

			}

		break;

		// Supported camera pool 4
		case _SupportedThermalCameras_Pool_4:

			// Which temperature range should be set
			switch (TemperatureRange) {

				// This routine configures the temperature range of the IR camera
				case _ThermalCamera_TemperatureRange_HighRange: break;
				case _ThermalCamera_TemperatureRange_LowRange:  break;

			}

		break;

		// Supported camera pool 5
		case _SupportedThermalCameras_Pool_5:

			// Which temperature range should be set - switch the camera's own hardware range,
			// see P3WinUsbCamera::setTemperatureRange()
			switch (TemperatureRange) {

				case _ThermalCamera_TemperatureRange_HighRange:

					if (P3Camera.setTemperatureRange(true) == false) {

						RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "P3 setTemperatureRange(high) failed: " + P3Camera.getLastError(), _StatusMessageType_Error);

					}

				break;

				case _ThermalCamera_TemperatureRange_LowRange:

					if (P3Camera.setTemperatureRange(false) == false) {

						RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "P3 setTemperatureRange(low) failed: " + P3Camera.getLastError(), _StatusMessageType_Error);

					}

				break;

			}

		break;

	}

}

// CHANGED 18-11-2025 !!!!!!!
void RMH_IRThermalCamera_InitIRCameraConstants(ThermalCameraDevice::IRCameraDeviceFormat* CameraStatus, unsigned short SupportedCameraPool) {

	// This routine sets the calibration, metadata and frame data constants of the connected IR camera.
	// These depend on the physical resolution of the IR sensor and the supported pool

	// Local variables - frame height minus metadata
	unsigned int FrameHeightMinusMeta = CameraStatus->FrameHeight - CameraStatus->FrameMetadataSize;

	// Which supported camera pool is selected
	switch (SupportedCameraPool) {

		// ------------------------------------ Supported Camera Pool 1 ------------------------------------ //

		case _SupportedThermalCameras_Pool_1:

			// Set the default zero calibration parameters
			CameraStatus->CalValue0Offset = 390.0;
			CameraStatus->CalValue0Fpamul = 7.05;

			// Calculate the length to the start of metadata 1
			CameraStatus->MetaData1Index = CameraStatus->FrameWidth * FrameHeightMinusMeta;

			// Check the pixel width of the IR sensor
			switch (CameraStatus->FrameWidth) {

				// For 640x IR sensors
				case 640:

					// Set the detector temperature calibration constants
					CameraStatus->fpa_off = 6867;
					CameraStatus->fpa_div = 33.8;
					// Calculate the length to the start of metadata 2
					CameraStatus->MetaData2Index = CameraStatus->MetaData1Index + CameraStatus->FrameWidth * 3;

				break;

				// For 384x IR sensors
				case 384:

					// Set the detector temperature calibration constants
					CameraStatus->fpa_off = 7800;
					CameraStatus->fpa_div = 36.0;
					// Calculate the length to the start of metadata 2
					CameraStatus->MetaData2Index = CameraStatus->MetaData1Index + CameraStatus->FrameWidth * 3;

				break;

				// For 256x IR sensors
				case 256:

					// Set the detector temperature calibration constants
					CameraStatus->fpa_off = 8617;
					CameraStatus->fpa_div = 37.682;
					// Calculate the length to the start of metadata 2
					CameraStatus->MetaData2Index = CameraStatus->MetaData1Index + CameraStatus->FrameWidth;
					// Update the zero calibration parameters
					CameraStatus->CalValue0Offset = 170.0;
					CameraStatus->CalValue0Fpamul = 0.0;

				break;

				// For 240x IR sensors
				case 240:

					// Set the detector temperature calibration constants
					CameraStatus->fpa_off = 7800;
					CameraStatus->fpa_div = 36.0;
					// Calculate the length to the start of metadata 2
					CameraStatus->MetaData2Index = CameraStatus->MetaData1Index + CameraStatus->FrameWidth;

				break;

			}

			// Calculate the matrix index for metadata 1 and metadata 2
			CameraStatus->MetaData1Index = CameraStatus->MetaData1Index - 1;
			CameraStatus->MetaData2Index = CameraStatus->MetaData2Index - 1;

		break;

		// ------------------------------------ Supported Camera Pool 2 ------------------------------------ //

		case _SupportedThermalCameras_Pool_2:

			// Set metadata index 1 for the max/min/center frame data
			CameraStatus->MetaData1Index = CameraStatus->FrameWidth * (CameraStatus->FrameHeight - CameraStatus->FrameMetadataSize);

		break;

		// ------------------------------------ Supported Camera Pool 3 ------------------------------------ //

		case _SupportedThermalCameras_Pool_3:

			// Set the default zero calibration parameters
			CameraStatus->CalValue0Offset = 390.0;
			CameraStatus->CalValue0Fpamul = 7.05;

			// Calculate the length to the start of metadata 1
			CameraStatus->MetaData1Index = CameraStatus->FrameWidth * FrameHeightMinusMeta;

			// Check the pixel width of the IR sensor
			switch (CameraStatus->FrameWidth) {

				// For 640x IR sensors
				case 640:

					// Set the detector temperature calibration constants
					CameraStatus->fpa_off = 6867;
					CameraStatus->fpa_div = 33.8;
					// Calculate the length to the start of metadata 2
					CameraStatus->MetaData2Index = CameraStatus->MetaData1Index + CameraStatus->FrameWidth * 3;

				break;

				// For 384x IR sensors
				case 384:

					// Set the detector temperature calibration constants
					CameraStatus->fpa_off = 7800;
					CameraStatus->fpa_div = 36.0;
					// Calculate the length to the start of metadata 2
					CameraStatus->MetaData2Index = CameraStatus->MetaData1Index + CameraStatus->FrameWidth * 3;

				break;

				// For 256x IR sensors
				case 256:

					// Set the detector temperature calibration constants
					CameraStatus->fpa_off = 8617; 
					CameraStatus->fpa_div = 13.9; // 37.682
					// Calculate the length to the start of metadata 2
					CameraStatus->MetaData2Index = CameraStatus->MetaData1Index + CameraStatus->FrameWidth;
					// Update the zero calibration parameters
					//CameraStatus->CalValue0Offset = 170.0;
					//CameraStatus->CalValue0Fpamul = 0.0;

				break;

				// For 240x IR sensors
				case 240:

					// Set the detector temperature calibration constants
					CameraStatus->fpa_off = 7800;
					CameraStatus->fpa_div = 36.0;
					// Calculate the length to the start of metadata 2
					CameraStatus->MetaData2Index = CameraStatus->MetaData1Index + CameraStatus->FrameWidth;

				break;

			}

			// Calculate the matrix index for metadata 1 and metadata 2
			CameraStatus->MetaData1Index = CameraStatus->MetaData1Index - 1;
			CameraStatus->MetaData2Index = CameraStatus->MetaData2Index - 1;

			// Set metadata index 3 for the max/min/center frame data
			CameraStatus->MetaData3Index = CameraStatus->FrameWidth * (CameraStatus->FrameHeight - CameraStatus->FrameMetadataSize);

		break;

		// ------------------------------------ Supported Camera Pool 4 (and Pool 5/6 - identical frame layout) ------------------------------------ //

		case _SupportedThermalCameras_Pool_4: case _SupportedThermalCameras_Pool_5: case _SupportedThermalCameras_Pool_6:

			// Set metadata index 1
			CameraStatus->MetaData1Index = CameraStatus->FrameWidth * (CameraStatus->FrameHeight - CameraStatus->FrameMetadataSize);
			// Set metadata index 2
			CameraStatus->MetaData2Index = CameraStatus->MetaData1Index + CameraStatus->FrameWidth;

		break;

		// ---------------------------------------------------------------------------------------------------- //

	}

}

ThermalCameraDevice::IRCameraDeviceFormat RMH_IRThermalCamera_ConnectToThermalCamera(System::Windows::Forms::ComboBox^ CameraSourceComboBox) {

	// This routine reads the available connected camera devices 
	// and connects to the camera device with the given input name selected in the camera device combobox.
	// The routine returns a "ThermalCameraDevice::IRCameraDeviceFormat" class
	// 
	// Local objects and variables
	unsigned int NumberOfCameraDeviceNames = 0;
	std::vector<std::string> CameraDeviceNamesPointer;
	ThermalCameraDevice::IRCameraDeviceFormat CameraStatus;
	
	// Read the associated ComboBox item's camera identity - translate its position in the sorted ComboBox
	// (see SupportedCamerasDisplayOrder in RMH_SupportedIRCameras_Resources.h) back to the camera's stable
	// macro index, since the two are no longer the same number once the list is displayed alphabetically.
	// RMH_GetCameraIndexAtDisplayPosition() bounds-checks this - CameraSourceComboBox->SelectedIndex is -1
	// if nothing is selected (for example a saved session referencing a since-removed camera).
	CameraStatus.SellectedCameraIndex = RMH_GetCameraIndexAtDisplayPosition(CameraSourceComboBox->SelectedIndex);

	// Check and update the associated supported camera pool and the associated camera frame rate parameter
	switch (CameraStatus.SellectedCameraIndex) {

		// Update the camera pool variable
		case _SupportedThermalCamera_InfiRayT2L:        CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_1; CameraDeviceNamesPointer = InfiRayT2LDeviceNames;			CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayT2L_FrameRate; break;
		case _SupportedThermalCamera_InfiRayT2LV2:      CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_3; CameraDeviceNamesPointer = InfiRayT2LV2DeviceNames;		CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayT2LV2_FrameRate; break;
		case _SupportedThermalCamera_InfiRayT2Search:   CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_1; CameraDeviceNamesPointer = InfiRayT2SearchDeviceNames;		CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayT2Search_FrameRate; break;
		case _SupportedThermalCamera_InfiRayT2SearchV2: CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_3; CameraDeviceNamesPointer = InfiRayT2SearchV2DeviceNames;	CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayT2SearchV2_FrameRate; break;
		case _SupportedThermalCamera_InfiRayT2Sp:       CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_1; CameraDeviceNamesPointer = InfiRayT2SpDeviceNames;			CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayT2Sp_FrameRate; break;
		case _SupportedThermalCamera_InfiRayT2SpV2:     CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_3; CameraDeviceNamesPointer = InfiRayT2SpV2DeviceNames;		CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayT2SpV2_FrameRate; break;
		case _SupportedThermalCamera_InfiRayT2Pro:      CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_1; CameraDeviceNamesPointer = InfiRayT2ProDeviceNames;		CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayT2Pro_FrameRate; break;
		case _SupportedThermalCamera_InfiRayT2ProV2:    CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_3; CameraDeviceNamesPointer = InfiRayT2ProV2DeviceNames;		CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayT2ProV2_FrameRate; break;
		case _SupportedThermalCamera_InfiRayT3Search:   CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_1; CameraDeviceNamesPointer = InfiRayT3SearchDeviceNames;		CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayT3Search_FrameRate; break;
		case _SupportedThermalCamera_InfiRayT3S:	    CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_1; CameraDeviceNamesPointer = InfiRayT3SDeviceNames;			CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayT3S_FrameRate; break;
		case _SupportedThermalCamera_InfiRayT3Pro:      CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_1; CameraDeviceNamesPointer = InfiRayT3ProDeviceNames;		CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayT3Pro_FrameRate; break;
		case _SupportedThermalCamera_InfiRayP2:         CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_2; CameraDeviceNamesPointer = InfiRayP2DeviceNames;			CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayP2_FrameRate; break;
		case _SupportedThermalCamera_ThermalMasterP2:   CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_4; CameraDeviceNamesPointer = ThermalMasterP2DeviceNames;		CameraStatus.FrameRate = _SupportedThermalCamera_ThermalMasterP2_FrameRate; break;
		case _SupportedThermalCamera_InfiRayP2Pro:      CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_2; CameraDeviceNamesPointer = InfiRayP2ProDeviceNames;		CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayP2Pro_FrameRate; break;
		case _SupportedThermalCamera_InfiRayDVDL13:     CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_1; CameraDeviceNamesPointer = InfiRayDVDL13DeviceNames;		CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayDVDL13_FrameRate; break;
		case _SupportedThermalCamera_InfiRayS0Series:   CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_1; CameraDeviceNamesPointer = InfiRayS0SeriesDeviceNames;		CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayS0Series_FrameRate; break;
		case _SupportedThermalCamera_InfiRayTiny1C:     CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_2; CameraDeviceNamesPointer = InfiRayTiny1CDeviceNames;		CameraStatus.FrameRate = _SupportedThermalCamera_InfiRayTiny1C_FrameRate; break;
		case _SupportedThermalCamera_HTIHT301:          CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_1; CameraDeviceNamesPointer = HTIHT301DeviceNames;			CameraStatus.FrameRate = _SupportedThermalCamera_HTIHT301_FrameRate; break;
		case _SupportedThermalCamera_UNITUTi260M:       CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_2; CameraDeviceNamesPointer = UNITUTi260MDeviceNames;			CameraStatus.FrameRate = _SupportedThermalCamera_UNITUTi260M_FrameRate; break;
		case _SupportedThermalCamera_TOPDONTC001:       CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_2; CameraDeviceNamesPointer = TOPDONTC001DeviceNames;			CameraStatus.FrameRate = _SupportedThermalCamera_TOPDONTC001_FrameRate; break;
		case _SupportedThermalCamera_TOPDONTC002:       CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_2; CameraDeviceNamesPointer = TOPDONTC002DeviceNames;			CameraStatus.FrameRate = _SupportedThermalCamera_TOPDONTC002_FrameRate; break;
		case _SupportedThermalCamera_Victor328B:        CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_2; CameraDeviceNamesPointer = Victor328BDeviceNames;			CameraStatus.FrameRate = _SupportedThermalCamera_Victor328B_FrameRate; break;	
		case _SupportedThermalCamera_LODESTARL2:        CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_2; CameraDeviceNamesPointer = LODESTARL2DeviceNames;			CameraStatus.FrameRate = _SupportedThermalCamera_LODESTARL2_FrameRate; break;
		case _SupportedThermalCamera_ThermalMasterP3:   CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_5; CameraDeviceNamesPointer = ThermalMasterP3DeviceNames;		CameraStatus.FrameRate = _SupportedThermalCamera_ThermalMasterP3_FrameRate; break;
		case _SupportedThermalCamera_ThermalMasterTHOR001: CameraStatus.ThermalCameraSupportPool = _SupportedThermalCameras_Pool_6; CameraDeviceNamesPointer = ThermalMasterTHOR001DeviceNames;	CameraStatus.FrameRate = _SupportedThermalCamera_ThermalMasterTHOR001_FrameRate; break;

	}

	// Pool 5 (Thermal Master P3) has no DirectShow/UVC interface at all - it is only reachable over WinUSB with its
	// own vendor protocol (see p3_winusb_camera.h). Bypass the DirectShow enumeration/name-matching below entirely.
	if (CameraStatus.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_5) {

		// Stop video capture and close whatever camera was previously open
		RMH_IRThermalCamera_CloseIRCameraDevice();
		// Reset the camera "isStreaming" status flag
		CameraStatus.isStreaming = false;

		// Open the WinUSB camera device
		if (P3Camera.open()) {

			// Update the camera connected status flag
			CameraStatus.ConnectedFlag = true;
			// Update the camera device name and index - there is no DirectShow enumeration for this camera
			CameraStatus.CameraDeviceName = "P3";
			CameraStatus.IRCameraDeviceIndex = 0;
			CameraStatus.CameraSystemDevicePath = "WinUSB";

			// Update the camera device info - frame width and height
			CameraStatus.FrameWidth = P3Camera.getWidth();
			CameraStatus.FrameHeight = P3Camera.getHeight();

			// Update the camera device info - metadata size (same frame layout as Pool 4)
			CameraStatus.FrameMetadataSize = round((float)CameraStatus.FrameHeight - (float)CameraStatus.FrameWidth / (float)_FixedThermalCameraFrame_AspectRatio_Pool_4);

			// Read the operating constants of the IR camera - relative to the supported pool
			RMH_IRThermalCamera_InitIRCameraConstants(&CameraStatus, CameraStatus.ThermalCameraSupportPool);

			// Update the camera status message
			CameraStatus.StatusMessage = "Thermal Camera Is Connected And Ready.";

		}
		else {

			// Update the camera connected status flag
			CameraStatus.ConnectedFlag = false;
			// Update the camera status message
			CameraStatus.StatusMessage = "Error: Could Not Connect To The Selected Thermal Camera! (" + P3Camera.getLastError() + ")";
			// Reset the camera device name from the class object
			CameraStatus.CameraDeviceName = "NAN";
			CameraStatus.CameraSystemDevicePath = "NAN";
			CameraStatus.IRCameraDeviceIndex = 0;
			CameraStatus.FrameWidth = 0;
			CameraStatus.FrameHeight = 0;

		}

		// Return the camera status
		return CameraStatus;

	}

	// Read the number of camera device names in the associated string array
	NumberOfCameraDeviceNames = CameraDeviceNamesPointer.size();

	// Update the camera connect flag
	CameraStatus.ConnectedFlag = false;

	// Read the available connected cameras
	IRThermalCameraDeivceList = IRThermalCamera.getCameras();

	// Are no cameras found or active
	if (IRThermalCameraDeivceList.size() == 0) {

		// Update the camera status message
		CameraStatus.StatusMessage = "Error: No Available Thermal Camera Devices Detected!";
		// Update the camera connected status flag
		CameraStatus.ConnectedFlag = false;

	}
	else {

		// Loop through all available cameras
		for (unsigned int i = 0; i < IRThermalCameraDeivceList.size(); i++) {

			// Read the device names of the active cameras from the array
			CameraStatus.CameraDeviceName = IRThermalCameraDeivceList[i].getFriendlyName();

			// Loop through the number of camera device names in the associated string array
			for (unsigned int j = 0; j < NumberOfCameraDeviceNames; j++) {

				// Does the device name read match the input device name?
				if (CameraStatus.CameraDeviceName.c_str() == CameraDeviceNamesPointer[j]) {

					// Update the camera connect flag
					CameraStatus.ConnectedFlag = true;
					// Save the camera index value 
					CameraStatus.IRCameraDeviceIndex = i;
					// Break the inner for loop
					break;

				}
				else {

					// Update the camera connect flag
					CameraStatus.ConnectedFlag = false;

				}

			}

			// Was the inner for loop broken?
			if (CameraStatus.ConnectedFlag == true) {

				// Break the outer for loop
				break;

			}

		}

		// If the selected camera was found in the device list array -> connect to the camera 
		if (CameraStatus.ConnectedFlag == true) {

			// Stop video capture and close the camera
			RMH_IRThermalCamera_CloseIRCameraDevice();
			// Reset the camera "isStreaming" status flag
			CameraStatus.isStreaming = false;

			// Open the selected camera for video streaming
			RMH_IRThermalCamera_OpenIRCameraDevice(CameraStatus.IRCameraDeviceIndex, CameraStatus.ThermalCameraSupportPool);

			// Update the camera device info - frame width
			CameraStatus.FrameWidth = IRThermalCamera.getWidth();
			// Update the camera device info - frame height
			CameraStatus.FrameHeight = IRThermalCamera.getHeight();

			// Pool 6 (Thermal Master THOR001): the selected DirectShow format's declared resolution (640 x 480,
			// from its VideoStreaming frame descriptor) does not match the true raw pixel layout actually sent
			// over the wire (256 x 192 - see RMH_SupportedIRCameras_Resources.h), so the reported values are
			// overridden with the real, known sensor dimensions.
			if (CameraStatus.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_6) {

				CameraStatus.FrameWidth = _SupporteredeThermalCameraPool6_SensorWidthWithThermalData;
				CameraStatus.FrameHeight = _SupporteredeThermalCameraPool6_SensorHeightWithThermalData;

				// Skip the raw sample's fixed 10 byte ("ff 00" x5) header - see RMH_SupportedIRCameras_Resources.h
				CameraStatus.FrameWidthPixelOffset = _SupporteredeThermalCameraPool6_FrameWidthPixelOffset;
				CameraStatus.FrameHeightPixelOffset = _SupporteredeThermalCameraPool6_FrameHeightPixelOffset;

			}

			// Refuse a video format that does not fit the frame buffers (for example an ordinary webcam matched by the generic name "USB Camera")
			if (RMH_FrameBuffer_IsFrameSizeSupported(CameraStatus.FrameWidth, CameraStatus.FrameHeight) == false) {

				// Stop video capture and close the camera
				RMH_IRThermalCamera_CloseIRCameraDevice();
				// Reset the camera "isStreaming" status flag
				CameraStatus.isStreaming = false;
				// Update the camera connected status flag
				CameraStatus.ConnectedFlag = false;
				// Update the camera status message
				CameraStatus.StatusMessage = "Error: The Selected Camera's Video Format Is Not Supported (" + std::to_string(CameraStatus.FrameWidth) + " x " + std::to_string(CameraStatus.FrameHeight) + ")!";

				// Return the camera status
				return CameraStatus;

			}

			// Read the supported camera pool associated with the thermal camera 
			switch (CameraStatus.ThermalCameraSupportPool) {

				// Update the camera device info - metadata size
				case _SupportedThermalCameras_Pool_1: CameraStatus.FrameMetadataSize = round((float)CameraStatus.FrameHeight - (float)CameraStatus.FrameWidth / (float)_FixedThermalCameraFrame_AspectRatio_Pool_1); break;
				case _SupportedThermalCameras_Pool_2: CameraStatus.FrameMetadataSize = round((float)CameraStatus.FrameHeight - (float)CameraStatus.FrameWidth / (float)_FixedThermalCameraFrame_AspectRatio_Pool_2); break;
				case _SupportedThermalCameras_Pool_3: CameraStatus.FrameMetadataSize = round((float)CameraStatus.FrameHeight - (float)CameraStatus.FrameWidth / (float)_FixedThermalCameraFrame_AspectRatio_Pool_3); break;
				case _SupportedThermalCameras_Pool_4: CameraStatus.FrameMetadataSize = round((float)CameraStatus.FrameHeight - (float)CameraStatus.FrameWidth / (float)_FixedThermalCameraFrame_AspectRatio_Pool_4); break;
				case _SupportedThermalCameras_Pool_6: CameraStatus.FrameMetadataSize = round((float)CameraStatus.FrameHeight - (float)CameraStatus.FrameWidth / (float)_FixedThermalCameraFrame_AspectRatio_Pool_6); break;

			}

			// Read the camera system device path
			CameraStatus.CameraSystemDevicePath = IRThermalCameraDeivceList[CameraStatus.IRCameraDeviceIndex].getDevicePath();

			// Read the operating constants of the IR camera - relative to the supported pool
			RMH_IRThermalCamera_InitIRCameraConstants(&CameraStatus, CameraStatus.ThermalCameraSupportPool);

			// Update the camera status message
			CameraStatus.StatusMessage = "Thermal Camera Is Connected And Ready.";

		}
		else {

			// Stop video capture and close the camera
			RMH_IRThermalCamera_CloseIRCameraDevice();
			// Reset the camera "isStreaming" status flag
			CameraStatus.isStreaming = false;

			// Update the camera status message
			CameraStatus.StatusMessage = "Error: Could Not Connect To The Selected Thermal Camera!";

			// Reset the camera device name from the class object
			CameraStatus.CameraDeviceName = "NAN";
			// Reset the system device path of the camera
			CameraStatus.CameraSystemDevicePath = "NAN";
			// Reset the camera index value 
			CameraStatus.IRCameraDeviceIndex = 0;
			// Reset the camera device info - frame width
			CameraStatus.FrameWidth = 0;
			// Reset the camera device info - frame height
			CameraStatus.FrameHeight = 0;
			// Reset the camera device info - metadata size
			CameraStatus.FrameMetadataSize = 0;

			// Reset the operating constants of the IR camera
			CameraStatus.fpa_off = 0.0;
			CameraStatus.fpa_div = 0.0;
			CameraStatus.CalValue0Offset = 0.0;
			CameraStatus.CalValue0Fpamul = 0.0;
			CameraStatus.MetaData1Index = 0;
			CameraStatus.MetaData2Index = 0;

		}

	}

	// Return the camera status
	return CameraStatus;

}

// ---------------------------------- Raw Image Data To Raw Thermal Data Conversion Routine ---------------------------------- //

double RMH_IRThermalCamera_ConvertYUY2To14BitThermalDataArray(ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned char* YUY2in, unsigned short* ThermalDataRaw) {

	// This routine converts the raw 24-bit YUY2 camera data to a 14-bit raw thermographic data array containing the raw thermal camera sensor intensity values
	// Maximum, minimum and center data are appended to the end of the thermal data array if the selected thermal camera pool requires it.
	// The format of the appended maximum, minimum and center data (at the end of the thermal data array) is: [Max_X Max_Y Max_Raw Min_X Min_Y Min_Raw Center_Raw].
	// These have the following index values: (FrameHeightOffset * FrameWidth) + n -> n = 0 - 6
	// The routine returns the average value of the converted thermal data as a 32-bit double.

	// Read the temporary array data and sort the kernel array
	unsigned short PixelXCoordinate = 0;
	unsigned short PixelYCoordinate = 0;
	unsigned short CenterPixelValue = 0;
	unsigned int CenterPixelIndex = 0;
	unsigned short MaximumPixelValue = 0;
	unsigned short MaximumPixelXCoord = 0;
	unsigned short MaximumPixelYCoord = 0;
	unsigned short MinimumPixelValue = 65535;
	unsigned short MinimumPixelXCoord = 0;
	unsigned short MinimumPixelYCoord = 0;
	unsigned short Pixel16BitValue[4];
	bool AddMaxMinCenterDataToThermalDataArray = false;
	unsigned int AddMaxMinCenterStopIndex = 0;
	unsigned int FrameDataArrayOffset = (IRCamera->FrameHeightPixelOffset * IRCamera->FrameWidth) + IRCamera->FrameWidthPixelOffset;
	bool PerformNonUniformityCorrection = false;
	unsigned long long int ThermalDataAverageValue = 0;
	unsigned long FrameSizeMinusMeta = IRCamera->FrameWidth * (IRCamera->FrameHeight - IRCamera->FrameMetadataSize);

	// Refuse frame geometry that would read or write outside the frame buffers (the values can come from a file)
	if (RMH_FrameBuffer_IsFrameSizeSupported(IRCamera->FrameWidth, IRCamera->FrameHeight) == false || IRCamera->FrameMetadataSize >= IRCamera->FrameHeight ||
		((unsigned long long)IRCamera->FrameHeightPixelOffset * IRCamera->FrameWidth + IRCamera->FrameWidthPixelOffset + (unsigned long long)IRCamera->FrameWidth * IRCamera->FrameHeight) * 2 + 8 > MaximumFrameDataArraySize) {

		// Return: no data converted
		return 0.0;

	}

	// Check which thermal camera pool has been selected
	switch (IRCamera->ThermalCameraSupportPool) {

		// Supported camera pool 1
		case _SupportedThermalCameras_Pool_1:

			// Convert all data bytes
			IRCamera->FrameHeight = IRCamera->FrameHeight;

			// Max/min/center data must NOT be appended to the end of the thermal frame data array
			// since they are already part of the frame metadata
			AddMaxMinCenterDataToThermalDataArray = false;

			// Set the stop index for the max/min/center data
			AddMaxMinCenterStopIndex = IRCamera->FrameWidth * IRCamera->FrameHeight;

			// Do not perform non-uniformity correction of the image data
			PerformNonUniformityCorrection = false;

		break;

		// Supported camera pools 2, 4, 5 and 6
		case _SupportedThermalCameras_Pool_2: case _SupportedThermalCameras_Pool_4: case _SupportedThermalCameras_Pool_5: case _SupportedThermalCameras_Pool_6:

			// Convert only the metadata area
			IRCamera->FrameHeight = IRCamera->FrameHeight;
					
			// Calculate the array index value of the center pixel
			CenterPixelIndex = ((IRCamera->FrameHeight - IRCamera->FrameMetadataSize) * 0.5) * IRCamera->FrameWidth + (IRCamera->FrameWidth * 0.5);

			// Max/min/center data must be appended to the end of the thermal frame data array
			AddMaxMinCenterDataToThermalDataArray = true;

			// Set the stop index for the max/min/center data
			AddMaxMinCenterStopIndex = IRCamera->FrameWidth * (IRCamera->FrameHeight - IRCamera->FrameMetadataSize);

		break;

		// Supported camera pool 3
		case _SupportedThermalCameras_Pool_3: 

			// Convert all data bytes
			IRCamera->FrameHeight = IRCamera->FrameHeight;

			// Calculate the array index value of the center pixel
			CenterPixelIndex = ((IRCamera->FrameHeight - IRCamera->FrameMetadataSize) * 0.5) * IRCamera->FrameWidth + (IRCamera->FrameWidth * 0.5);

			// Max/min/center data must be appended to the end of the thermal frame data array
			AddMaxMinCenterDataToThermalDataArray = true;

			// Set the stop index for the max/min/center data
			AddMaxMinCenterStopIndex = IRCamera->FrameWidth * (IRCamera->FrameHeight - IRCamera->FrameMetadataSize);

			// Perform non-uniformity correction of the image data
			PerformNonUniformityCorrection = true;

		break;

	}

	// Loop through all YUY2 band pixels - FrameDataArrayOffset * 2 -> (2 x 8-bit)
	for (unsigned int i = 0, j = FrameDataArrayOffset * 2; i < (IRCamera->FrameWidth * IRCamera->FrameHeight); i += 4, j += 8) {
		
		// Read and convert the camera band data to 16-bit pixel data (14-bit full scale) - YUY2 high and low byte to a combined 16-bit integer
		Pixel16BitValue[0] = ((unsigned short)(*(YUY2in + (j + 1))) << 8) | ((unsigned short)*(YUY2in + (j + 0)));
		Pixel16BitValue[1] = ((unsigned short)(*(YUY2in + (j + 3))) << 8) | ((unsigned short)*(YUY2in + (j + 2)));
		Pixel16BitValue[2] = ((unsigned short)(*(YUY2in + (j + 5))) << 8) | ((unsigned short)*(YUY2in + (j + 4)));
		Pixel16BitValue[3] = ((unsigned short)(*(YUY2in + (j + 7))) << 8) | ((unsigned short)*(YUY2in + (j + 6)));
	
		// Do not limit the 16-bit pixel values in the metadata area
		if (i < FrameSizeMinusMeta) {

			// Should the image data be non-uniformity corrected
			if (PerformNonUniformityCorrection == true) {

				// Perform non-uniformity correction of the image data by subtracting the non-uniformity map data.
				Pixel16BitValue[0] = (unsigned short)((double)(Pixel16BitValue[0]) - ImageCMOSNonUniformityMapData[i + 0]);
				Pixel16BitValue[1] = (unsigned short)((double)(Pixel16BitValue[1]) - ImageCMOSNonUniformityMapData[i + 1]);
				Pixel16BitValue[2] = (unsigned short)((double)(Pixel16BitValue[2]) - ImageCMOSNonUniformityMapData[i + 2]);
				Pixel16BitValue[3] = (unsigned short)((double)(Pixel16BitValue[3]) - ImageCMOSNonUniformityMapData[i + 3]);

				// Limit the maximum pixel values to 14-bit full scale 
				if (Pixel16BitValue[0] > _ImageProcessing_ImageResolution_14Bit) { Pixel16BitValue[0] = _ImageProcessing_ImageResolution_14Bit; }
				if (Pixel16BitValue[1] > _ImageProcessing_ImageResolution_14Bit) { Pixel16BitValue[1] = _ImageProcessing_ImageResolution_14Bit; }
				if (Pixel16BitValue[2] > _ImageProcessing_ImageResolution_14Bit) { Pixel16BitValue[2] = _ImageProcessing_ImageResolution_14Bit; }
				if (Pixel16BitValue[3] > _ImageProcessing_ImageResolution_14Bit) { Pixel16BitValue[3] = _ImageProcessing_ImageResolution_14Bit; }

			}

			// Accumulate the converted values into the total sum
			ThermalDataAverageValue = ThermalDataAverageValue + (Pixel16BitValue[0] + Pixel16BitValue[1] + Pixel16BitValue[2] + Pixel16BitValue[3]);

		}

		// Should max/min/center data be appended to the end of the thermal frame data array
		if (AddMaxMinCenterDataToThermalDataArray == true) {

			// Loop over each band pixel value read
			for (unsigned int n = 0; n < 4; n++) {

				// Check the stop index for the max/min/center values
				if (i < AddMaxMinCenterStopIndex) {

					// Check for the maximum pixel value
					if (Pixel16BitValue[n] > MaximumPixelValue) {

						// Update the maximum pixel value
						MaximumPixelValue = Pixel16BitValue[n];

						// Read the X/Y coordinates of the maximum value
						MaximumPixelXCoord = PixelXCoordinate;
						MaximumPixelYCoord = PixelYCoordinate;

					}

					// Check for the minimum pixel value
					if (Pixel16BitValue[n] < MinimumPixelValue) {

						// Update the minimum pixel value
						MinimumPixelValue = Pixel16BitValue[n];

						// Read the X/Y coordinates of the minimum value
						MinimumPixelXCoord = PixelXCoordinate;
						MinimumPixelYCoord = PixelYCoordinate;

					}

					// Increment the pixel X coordinate variable 
					PixelXCoordinate = PixelXCoordinate + 1;

					// If the X coordinate variable is greater than or equal to the frame width
					if (PixelXCoordinate >= IRCamera->FrameWidth) {

						// Reset the pixel X coordinate
						PixelXCoordinate = 0;

						// Increment the pixel Y coordinate variable 
						PixelYCoordinate = PixelYCoordinate + 1;

						// If the Y coordinate variable is greater than or equal to the frame height
						if (PixelYCoordinate >= IRCamera->FrameHeight) {

							// Reset the pixel Y coordinate
							PixelYCoordinate = 0;

						}

					}

				}

			}

		}

		// Loop through all baseline band pixels
		*(ThermalDataRaw + (i + 0)) = Pixel16BitValue[0];
		*(ThermalDataRaw + (i + 1)) = Pixel16BitValue[1];
		*(ThermalDataRaw + (i + 2)) = Pixel16BitValue[2];
		*(ThermalDataRaw + (i + 3)) = Pixel16BitValue[3];

	}

	// Should max/min/center data be appended to the end of the thermal frame data array
	if (AddMaxMinCenterDataToThermalDataArray == true) {

		// Append the max/min/center data to the end of the thermal frame data array
		*(ThermalDataRaw + (IRCamera->FrameWidth * (IRCamera->FrameHeight - IRCamera->FrameMetadataSize)) + 0) = MaximumPixelXCoord;
		*(ThermalDataRaw + (IRCamera->FrameWidth * (IRCamera->FrameHeight - IRCamera->FrameMetadataSize)) + 1) = MaximumPixelYCoord;
		*(ThermalDataRaw + (IRCamera->FrameWidth * (IRCamera->FrameHeight - IRCamera->FrameMetadataSize)) + 2) = MaximumPixelValue;
		*(ThermalDataRaw + (IRCamera->FrameWidth * (IRCamera->FrameHeight - IRCamera->FrameMetadataSize)) + 3) = MinimumPixelXCoord;
		*(ThermalDataRaw + (IRCamera->FrameWidth * (IRCamera->FrameHeight - IRCamera->FrameMetadataSize)) + 4) = MinimumPixelYCoord;
		*(ThermalDataRaw + (IRCamera->FrameWidth * (IRCamera->FrameHeight - IRCamera->FrameMetadataSize)) + 5) = MinimumPixelValue;
		*(ThermalDataRaw + (IRCamera->FrameWidth * (IRCamera->FrameHeight - IRCamera->FrameMetadataSize)) + 6) = *(ThermalDataRaw + CenterPixelIndex);

	}

	// Return the average value of the thermal frame data
	return (double)ThermalDataAverageValue / (double)FrameSizeMinusMeta;

}

void RMH_IRThermalCamera_Convert14BitThermalDataArrayToYUY2(unsigned short* ThermalData, unsigned char* YUY2Out, unsigned int FrameWidth, unsigned int FrameHeight) {

	// This routine converts a given 16-bit (14-bit full scale) thermographic data array to a YUY2 array.
	// The routine is used as part of the NUC data compensation in "Recording analysis mode", when saving video data that is NUC corrected.

	// Loop through all thermographic data array pixels
	for (unsigned int i = 0, j = 0; i < (FrameWidth * FrameHeight); i += 4, j += 8) {

		// Convert the thermographic data array to a YUY2 array
		*(YUY2Out + (j + 0)) = (unsigned char)(ThermalData[i + 0] & 0x00FF);
		*(YUY2Out + (j + 1)) = (unsigned char)((ThermalData[i + 0] & 0xFF00) >> 8);
		*(YUY2Out + (j + 2)) = (unsigned char)(ThermalData[i + 1] & 0x00FF);
		*(YUY2Out + (j + 3)) = (unsigned char)((ThermalData[i + 1] & 0xFF00) >> 8);
		*(YUY2Out + (j + 4)) = (unsigned char)(ThermalData[i + 2] & 0x00FF);
		*(YUY2Out + (j + 5)) = (unsigned char)((ThermalData[i + 2] & 0xFF00) >> 8);
		*(YUY2Out + (j + 6)) = (unsigned char)(ThermalData[i + 3] & 0x00FF);
		*(YUY2Out + (j + 7)) = (unsigned char)((ThermalData[i + 3] & 0xFF00) >> 8);

	}

}

// --------------------------------- Thermal Camera Pool-Specific Image Processing Routine -------------------------------- //

void RMH_IRThermalCamera_LinearAutomaticGainControlTemp(unsigned short* ThermalData, unsigned short* GainGrayscale, unsigned int FrameWidth, unsigned int FrameHeight, double MaxOutPixelVal, double MinOutPixelVal, double MaxInPixelVal, double MinInPixelVal, float TempUnitScaleFactor, float TempUnitOffsetFactor) {

	// Write the background palette RGB values to the output array pointer
	// Write the background palette RGB values to the output array pointer
	// My algorithm is linearized as: y = a * x + b

	// // Image Resolution Format Reference Macros
	register double PixelValue1 = 0.0;
	register double PixelValue2 = 0.0;
	register double PixelValue3 = 0.0;
	register double PixelValue4 = 0.0;
	
	// Calculate the linear scaling factor (a parameter)
	double LinearScaleFactor = ((double)MinOutPixelVal - (double)MaxOutPixelVal) / ((double)MinInPixelVal - (double)MaxInPixelVal);
	// Calculate the linear offset scaling (b parameter)
	double OffsetScale = -LinearScaleFactor * (double)MaxInPixelVal + (double)MaxOutPixelVal;

	// The AGC image data is then passed on to the pointer array.
	for (unsigned int i = 0; i < (FrameWidth * FrameHeight); i += 4) {

		// Which supported camera pool is selected (applies to both pool 1 and 3)
		if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_1 || IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_3) {

			// Calculate the pixel temperature from the raw pixel data
			PixelValue1 = *(ThermalData + i);
			PixelValue2 = *(ThermalData + i + 1);
			PixelValue3 = *(ThermalData + i + 2);
			PixelValue4 = *(ThermalData + i + 3);

			// Read the pixel temperature from the look-up table
			PixelValue1 = IRCamera.TemperatureLookUpTabel[*(ThermalData + i) & 0x3FFF];
			PixelValue2 = IRCamera.TemperatureLookUpTabel[*(ThermalData + i + 1) & 0x3FFF];
			PixelValue3 = IRCamera.TemperatureLookUpTabel[*(ThermalData + i + 2) & 0x3FFF];
			PixelValue4 = IRCamera.TemperatureLookUpTabel[*(ThermalData + i + 3) & 0x3FFF];

		}
		else if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_2 || IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_4 || IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_5) {

			// Calculate the pixel temperature from the raw pixel data
			PixelValue1 = (*(ThermalData + i) * 0.015625) - 273.15;
			PixelValue2 = (*(ThermalData + i + 1) * 0.015625) - 273.15;
			PixelValue3 = (*(ThermalData + i + 2) * 0.015625) - 273.15;
			PixelValue4 = (*(ThermalData + i + 3) * 0.015625) - 273.15;

			// Compensate for the environmental contribution to the temperature calculations
			PixelValue1 = PixelValue1 * IRCamera.ObjectEnvirTempCorrectionFactor + IRCamera.ObjectEnvirTempCorrectionOffset;
			PixelValue2 = PixelValue2 * IRCamera.ObjectEnvirTempCorrectionFactor + IRCamera.ObjectEnvirTempCorrectionOffset;
			PixelValue3 = PixelValue3 * IRCamera.ObjectEnvirTempCorrectionFactor + IRCamera.ObjectEnvirTempCorrectionOffset;
			PixelValue4 = PixelValue4 * IRCamera.ObjectEnvirTempCorrectionFactor + IRCamera.ObjectEnvirTempCorrectionOffset;

			// Compensate for the temperature correction
			PixelValue1 = PixelValue1 + IRCamera.TemperatureCorrectionSetting;
			PixelValue2 = PixelValue2 + IRCamera.TemperatureCorrectionSetting;
			PixelValue3 = PixelValue3 + IRCamera.TemperatureCorrectionSetting;
			PixelValue4 = PixelValue4 + IRCamera.TemperatureCorrectionSetting;

		}
		else if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_6) {

			// The raw pixel value is temperature in deci-Kelvin (Kelvin x10) - confirmed against real
			// hardware: a scene reading 23.1 C on the vendor's own app produced raw values of ~2965-3021,
			// matching (23.1 + 273.15) x 10 = 2962.5 almost exactly. This is a different encoding from pool
			// 2/4/5's Kelvin x64, and no environmental correction factor is applied (not verified for this
			// camera, and the direct conversion already matches the reference reading closely).
			PixelValue1 = (*(ThermalData + i) / 10.0) - 273.15;
			PixelValue2 = (*(ThermalData + i + 1) / 10.0) - 273.15;
			PixelValue3 = (*(ThermalData + i + 2) / 10.0) - 273.15;
			PixelValue4 = (*(ThermalData + i + 3) / 10.0) - 273.15;

			// Compensate for the temperature correction
			PixelValue1 = PixelValue1 + IRCamera.TemperatureCorrectionSetting;
			PixelValue2 = PixelValue2 + IRCamera.TemperatureCorrectionSetting;
			PixelValue3 = PixelValue3 + IRCamera.TemperatureCorrectionSetting;
			PixelValue4 = PixelValue4 + IRCamera.TemperatureCorrectionSetting;

		}

		// Compensate for the temperature unit
		PixelValue1 = (PixelValue1 * TempUnitScaleFactor) + TempUnitOffsetFactor;
		PixelValue2 = (PixelValue2 * TempUnitScaleFactor) + TempUnitOffsetFactor;
		PixelValue3 = (PixelValue3 * TempUnitScaleFactor) + TempUnitOffsetFactor;
		PixelValue4 = (PixelValue4 * TempUnitScaleFactor) + TempUnitOffsetFactor;

		// Limit the pixel values - done to avoid pixel overflow
		if (PixelValue1 >= MaxInPixelVal) { PixelValue1 = MaxInPixelVal; }
		if (PixelValue1 <= MinInPixelVal) { PixelValue1 = MinInPixelVal; }
		if (PixelValue2 >= MaxInPixelVal) { PixelValue2 = MaxInPixelVal; }
		if (PixelValue2 <= MinInPixelVal) { PixelValue2 = MinInPixelVal; }
		if (PixelValue3 >= MaxInPixelVal) { PixelValue3 = MaxInPixelVal; }
		if (PixelValue3 <= MinInPixelVal) { PixelValue3 = MinInPixelVal; }
		if (PixelValue4 >= MaxInPixelVal) { PixelValue4 = MaxInPixelVal; }
		if (PixelValue4 <= MinInPixelVal) { PixelValue4 = MinInPixelVal; }

		// Local variables - stored in CPU registers
		*(GainGrayscale + i) = (unsigned short)(LinearScaleFactor * PixelValue1 + OffsetScale);
		*(GainGrayscale + i + 1) = (unsigned short)(LinearScaleFactor * PixelValue2 + OffsetScale);
		*(GainGrayscale + i + 2) = (unsigned short)(LinearScaleFactor * PixelValue3 + OffsetScale);
		*(GainGrayscale + i + 3) = (unsigned short)(LinearScaleFactor * PixelValue4 + OffsetScale);

	}

}

// --------------------------------- Thermal Camera Region Of Interest (ROI) Handling Routine --------------------------------- //

ROIAreaPixelInfoFormat RMH_IRThermalCamera_ReadROIAreaPixelInfoInsideFrameArea(ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short* ThermalData, unsigned int FrameWidth, unsigned int AreaX0Pos, unsigned int AreaY0Pos, unsigned int AreaWidth, unsigned int AreaHeight, bool ReturnROIPixels, unsigned short* ROIAreaRawPixelValues, unsigned short SupportedCameraPool) {

	// This routine reads the pixel information values of the ROI area within a given frame data area.
	// It also reads the frame height and width coordinates of the max/min temperatures.
	// All other raw pixel values within the area are likewise read and returned to the pointer array.

	// Read the temporary array data and sort the kernel array
	register double PixelTempValue = 0.0;
	register double PixelAvgTempValue = 0.0;
	register unsigned short PixelValue = 0;
	register unsigned int AreaStartIndex = 0;
	register unsigned int AreaStopIndex = 0;
	register unsigned int RawPixlDataIndex = 0;
	register unsigned int ROINumberOfPixels = 0;
	register ROIAreaPixelInfoFormat ROIPixelData;

	// Initialize the start values for max/min
	ROIPixelData.MaxValue = -10000;
	ROIPixelData.MinValue = 10000;

	// Read the number of pixels in the ROI area
	ROIPixelData.ROIAreaNmbOfPixels = AreaWidth * AreaHeight;

	// Calculate the start and stop index values for the active area
	AreaStartIndex = (AreaY0Pos * FrameWidth) + AreaX0Pos;
	AreaStopIndex = AreaStartIndex + AreaWidth;

	// Reset the ROI average temperature value
	PixelAvgTempValue = 0.0;
	// Reset the ROI pixel count variable
	ROINumberOfPixels = 0;

	// Loop through all pixel rows of the area
	for (unsigned int i = 0; i < AreaHeight; i++) {

		// Find the maximum and minimum pixel value for each frame pixel row
		for (unsigned int j = AreaStartIndex, k = 0; j < AreaStopIndex; j++, k++) {

			// Increment the number of ROI pixels
			ROINumberOfPixels = ROINumberOfPixels + 1;

			// Read the frame pixel temperature data
			PixelValue = *(ThermalData + j);

			// Should the raw pixel values of the ROI area be returned to the pointer array
			if (ReturnROIPixels == true) {

				// Store the raw pixel values of the ROI in the array in output format. 
				*(ROIAreaRawPixelValues + RawPixlDataIndex) = PixelValue;

				// Increment the raw pixel data index
				RawPixlDataIndex = RawPixlDataIndex + 1;

			}

			// Which supported camera pool is selected (applies to both pool 1 and 3)
			if (SupportedCameraPool == _SupportedThermalCameras_Pool_1 || SupportedCameraPool == _SupportedThermalCameras_Pool_3) {

				// Read the pixel temperature from the look-up table
				PixelTempValue = IRCamera->TemperatureLookUpTabel[PixelValue & 0x3FFF];

			}
			// Applies to both pool 2 and 4
			else if (SupportedCameraPool == _SupportedThermalCameras_Pool_2 || SupportedCameraPool == _SupportedThermalCameras_Pool_4 || SupportedCameraPool == _SupportedThermalCameras_Pool_5) {

				// Calculate the pixel temperature from the raw pixel data
				PixelTempValue = (PixelValue * 0.015625) - 273.15;
				// Compensate for the environmental contribution to the temperature calculations
				PixelTempValue = PixelTempValue * IRCamera->ObjectEnvirTempCorrectionFactor + IRCamera->ObjectEnvirTempCorrectionOffset;
				// Compensate for the temperature correction 
				PixelTempValue = PixelTempValue + IRCamera->TemperatureCorrectionSetting;

			}

			// Accumulate the sum of the temperatures of all ROI pixel values
			PixelAvgTempValue = PixelAvgTempValue + PixelTempValue;

			// Check for the maximum pixel value
			if (PixelTempValue > ROIPixelData.MaxValue) {

				// Update the maximum pixel value
				ROIPixelData.MaxValue = PixelTempValue;

				// Store the frame W/H coordinates of the maximum temperature
				ROIPixelData.ROIMaxPixelWidth = (AreaX0Pos + k);
				ROIPixelData.ROIMaxPixelHeight = (AreaY0Pos + i);

			}

			// Check for the minimum pixel value
			if (PixelTempValue < ROIPixelData.MinValue) {

				// Update the minimum pixel value
				ROIPixelData.MinValue = PixelTempValue;

				// Store the frame W/H coordinates of the minimum temperature
				ROIPixelData.ROIMinPixelWidth = (AreaX0Pos + k);
				ROIPixelData.ROIMinPixelHeight = (AreaY0Pos + i);

			}

		}

		// Increment to the next active area data row
		AreaStartIndex = AreaStartIndex + FrameWidth;
		AreaStopIndex = AreaStopIndex + FrameWidth;

	}

	// Calculate the average temperature of the ROI area
	ROIPixelData.AvgValue = PixelAvgTempValue / (double)ROINumberOfPixels;

	// Return the maximum and minimum pixel format
	return ROIPixelData;

}

// -------------------------------------------- Camera Frame Data Handling Routines ------------------------------------------- //

bool RMH_IRThermalCamera_ReadFrameRaw(unsigned char* ImageData, unsigned int* ImageSize) {

	// This routine reads and returns a raw data frame from the camera
	// ImageData must be one of the global frame buffers (MaximumFrameDataArraySize bytes); a larger frame is refused

	// Pool 5 (Thermal Master P3) is not a DirectShow device - it has its own WinUSB backend
	if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_5) { return P3Camera.getFrame(ImageData, (int*)ImageSize, true, MaximumFrameDataArraySize); }

	// Return the camera data frame
	return IRThermalCamera.getFrame(ImageData, (int*)ImageSize, true, MaximumFrameDataArraySize);

}

void RMH_IRThermalCamera_ReadCalFrameMetaData(unsigned short* ThermalData, ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short SupportedCameraPool) {

	// This routine reads the frame metadata of the IR camera and calculates the internal IR sensor temperatures

	// Refuse metadata positions outside the frame buffers (the frame geometry can come from a file)
	if (IRCamera->FrameMetadataSize >= IRCamera->FrameHeight || IRCamera->MetaData1Index + 16 >= MaximumFrameDataArraySize || IRCamera->MetaData2Index + 16 >= MaximumFrameDataArraySize) {

		// Return: no metadata read
		return;

	}

	// Which supported camera pool is selected
	switch (SupportedCameraPool) {

		// Supported camera pool 1
		case _SupportedThermalCameras_Pool_1:

			// Read and calculate the IR camera detector temperature
			IRCamera->temp_fpa_Raw = (double)(*(ThermalData + (IRCamera->MetaData1Index + 2)));
			IRCamera->temp_fpa = 20.0 - ((double)(IRCamera->temp_fpa_Raw - IRCamera->fpa_off)) / IRCamera->fpa_div;

			// Read and calculate the IR camera shutter temperature
			IRCamera->temp_shutter_Raw = *(ThermalData + (IRCamera->MetaData2Index + 2));
			IRCamera->temp_shutter = (double)(IRCamera->temp_shutter_Raw) / 10.0 - 273.15;

			// Read and calculate the IR camera core temperature
			IRCamera->temp_core_Raw = *(ThermalData + (IRCamera->MetaData2Index + 3));
			IRCamera->temp_core = (double)(IRCamera->temp_core_Raw) / 10.0 - 273.15;

			// Read the raw temperature and coordinate data of the max, min and center points
			IRCamera->Tmax_X =         *(ThermalData + (IRCamera->MetaData1Index + 3));
			IRCamera->Tmax_Y =         *(ThermalData + (IRCamera->MetaData1Index + 4));
			IRCamera->Tmax_Tmp_Raw =   *(ThermalData + (IRCamera->MetaData1Index + 5));
			IRCamera->Tmin_X =         *(ThermalData + (IRCamera->MetaData1Index + 6));
			IRCamera->Tmin_Y =         *(ThermalData + (IRCamera->MetaData1Index + 7));
			IRCamera->Tmin_Tmp_Raw =   *(ThermalData + (IRCamera->MetaData1Index + 8));
			IRCamera->Center_Tmp_Raw = *(ThermalData + (IRCamera->MetaData1Index + 13));

		break;

		// Supported camera pool 2
		case _SupportedThermalCameras_Pool_2:

			// Read and calculate the IR camera detector temperature - not supported yet! (Must be 1!!)
			IRCamera->temp_fpa_Raw = 1;
			IRCamera->temp_fpa = 1;

			// Read and calculate the IR camera shutter temperature - not supported yet! (Must be 1!!)
			IRCamera->temp_shutter_Raw = 1;
			IRCamera->temp_shutter = 1;

			// Read and calculate the IR camera core temperature - not supported yet! (Must be 1!!)
			IRCamera->temp_core_Raw = 1;
			IRCamera->temp_core = 1;

			// Read the raw temperature and coordinate data of the max, min and center points
			IRCamera->Tmax_X =         *(ThermalData + (IRCamera->MetaData1Index + 0));
			IRCamera->Tmax_Y =         *(ThermalData + (IRCamera->MetaData1Index + 1));
			IRCamera->Tmax_Tmp_Raw =   *(ThermalData + (IRCamera->MetaData1Index + 2));
			IRCamera->Tmin_X =         *(ThermalData + (IRCamera->MetaData1Index + 3));
			IRCamera->Tmin_Y =         *(ThermalData + (IRCamera->MetaData1Index + 4));
			IRCamera->Tmin_Tmp_Raw =   *(ThermalData + (IRCamera->MetaData1Index + 5));
			IRCamera->Center_Tmp_Raw = *(ThermalData + (IRCamera->MetaData1Index + 6));

		break;

	// Supported camera pool 3
case _SupportedThermalCameras_Pool_3:

    // Read the raw IR camera detector temperature
    IRCamera->temp_fpa_Raw = (double)(*(ThermalData + (IRCamera->MetaData1Index + 2)));

    // T2S+ V2 may report an invalid zero detector-temperature value.
    // Use a provisional fallback for testing when this occurs.
    if (IRCamera->temp_fpa_Raw == 0.0)
    {
        IRCamera->temp_fpa = 18.0;
    }
    else
    {
        IRCamera->temp_fpa = 20.0 -
            ((double)(IRCamera->temp_fpa_Raw - IRCamera->fpa_off)) /
            IRCamera->fpa_div;
    }

    // Read and calculate the IR camera shutter temperature
    IRCamera->temp_shutter_Raw = *(ThermalData + (IRCamera->MetaData2Index + 2));
    IRCamera->temp_shutter = ((((double)(IRCamera->temp_shutter_Raw) * 0.625) + 2731.5) / 10.0) - 273.15;

    // Read and calculate the IR camera core temperature - V2 does not report the core temperature!
    IRCamera->temp_core_Raw = *(ThermalData + (IRCamera->MetaData2Index + 3));
    IRCamera->temp_core = ((((double)(IRCamera->temp_core_Raw) * 0.625) + 2731.5) / 10.0) - 273.15;

    // Read the raw temperature and coordinate data of the max, min and center points
    IRCamera->Tmax_X =         *(ThermalData + (IRCamera->MetaData3Index + 0));
    IRCamera->Tmax_Y =         *(ThermalData + (IRCamera->MetaData3Index + 1));
    IRCamera->Tmax_Tmp_Raw =   *(ThermalData + (IRCamera->MetaData3Index + 2));
    IRCamera->Tmin_X =         *(ThermalData + (IRCamera->MetaData3Index + 3));
    IRCamera->Tmin_Y =         *(ThermalData + (IRCamera->MetaData3Index + 4));
    IRCamera->Tmin_Tmp_Raw =   *(ThermalData + (IRCamera->MetaData3Index + 5));
    IRCamera->Center_Tmp_Raw = *(ThermalData + (IRCamera->MetaData3Index + 6));

    break;

		// Supported camera pools 4, 5 and 6
		case _SupportedThermalCameras_Pool_4: case _SupportedThermalCameras_Pool_5: case _SupportedThermalCameras_Pool_6:

			// Read and calculate the IR camera detector temperature - not supported yet! (Must be 1!!)
			IRCamera->temp_fpa_Raw = 1;
			IRCamera->temp_fpa = 1;

			// Read and calculate the IR camera shutter temperature - not supported yet! (Must be 1!!)
			IRCamera->temp_shutter_Raw = 1;
			IRCamera->temp_shutter = 1;

			// Read and calculate the IR camera core temperature - not supported yet! (Must be 1!!)
			IRCamera->temp_core_Raw = 1;
			IRCamera->temp_core = 1;

			// Read the raw temperature and coordinate data of the max, min and center points
			IRCamera->Tmax_X = *(ThermalData + (IRCamera->MetaData1Index + 0));
			IRCamera->Tmax_Y = *(ThermalData + (IRCamera->MetaData1Index + 1));
			IRCamera->Tmax_Tmp_Raw = *(ThermalData + (IRCamera->MetaData1Index + 2));
			IRCamera->Tmin_X = *(ThermalData + (IRCamera->MetaData1Index + 3));
			IRCamera->Tmin_Y = *(ThermalData + (IRCamera->MetaData1Index + 4));
			IRCamera->Tmin_Tmp_Raw = *(ThermalData + (IRCamera->MetaData1Index + 5));
			IRCamera->Center_Tmp_Raw = *(ThermalData + (IRCamera->MetaData1Index + 6));

		break;

	}

}

void RMH_IRThermalCamera_ReadCalibrationParameters(unsigned short* ThermalData, ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short SupportedCameraPool) {

	// This routine reads the internal calibration parameters of the IR camera
	// which are used to calculate the temperature look-up table

	// Read the temporary array data and sort the kernel array
	unsigned int MetaCal16BitSizeOffset = 2;

	// Which supported camera pool is selected
	switch (SupportedCameraPool) {

		// Supported camera pool 1
		case _SupportedThermalCameras_Pool_1:

			// Read the internal calibration parameters of the IR camera
			IRCamera->CalValue0 = *(ThermalData + (IRCamera->MetaData2Index + 1));
			IRCamera->CalValue1 = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 3 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 4 + MetaCal16BitSizeOffset)));
			IRCamera->CalValue2 = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 5 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 6 + MetaCal16BitSizeOffset)));
			IRCamera->CalValue3 = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 7 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 8 + MetaCal16BitSizeOffset)));
			IRCamera->CalValue4 = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 9 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 10 + MetaCal16BitSizeOffset)));
			IRCamera->CalValue5 = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 11 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 12 + MetaCal16BitSizeOffset)));

		break;

		// Supported camera pool 2
		case _SupportedThermalCameras_Pool_2:

			// Read the internal calibration parameters of the IR camera
			IRCamera->CalValue0 = 0.0;
			IRCamera->CalValue1 = 0.0;
			IRCamera->CalValue2 = 0.0;
			IRCamera->CalValue3 = 0.0;
			IRCamera->CalValue4 = 0.0;
			IRCamera->CalValue5 = 0.0;

		break;

		// Supported camera pool 3
		case _SupportedThermalCameras_Pool_3:

			// Read the internal calibration parameters of the IR camera
			IRCamera->CalValue0 = *(ThermalData + (IRCamera->MetaData2Index + 1));
			IRCamera->CalValue1 = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 3 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 4 + MetaCal16BitSizeOffset)));
			IRCamera->CalValue2 = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 5 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 6 + MetaCal16BitSizeOffset)));
			IRCamera->CalValue3 = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 7 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 8 + MetaCal16BitSizeOffset)));
			IRCamera->CalValue4 = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 9 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 10 + MetaCal16BitSizeOffset)));
			IRCamera->CalValue5 = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 11 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 12 + MetaCal16BitSizeOffset)));

		break;

		// Supported camera pools 4 and 5
		case _SupportedThermalCameras_Pool_4: case _SupportedThermalCameras_Pool_5:

			// Read the internal calibration parameters of the IR camera
			IRCamera->CalValue0 = 0.0;
			IRCamera->CalValue1 = 0.0;
			IRCamera->CalValue2 = 0.0;
			IRCamera->CalValue3 = 0.0;
			IRCamera->CalValue4 = 0.0;
			IRCamera->CalValue5 = 0.0;

		break;

	}

}

// --------------------------------------- Camera Configuration Data Handling Routines --------------------------------------- //

void RMH_IRThermalCamera_ReadCameraConfigParameters(unsigned short* ThermalData, ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short SupportedCameraPool) {

	// This routine reads the internal configuration parameters of the camera

	// Read the temporary array data and sort the kernel array
	unsigned int MetaCal16BitSizeOffset = 2;

	// Which supported camera pool is selected
	switch (SupportedCameraPool) {

		// Supported camera pool 1
		case _SupportedThermalCameras_Pool_1:

			// Read the internal configuration parameters of the camera
			//IRCamera->TemperatureCorrectionSetting = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 127 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 128 + MetaCal16BitSizeOffset)));
			IRCamera->ReflectedTemperatureSetting = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 129 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 130 + MetaCal16BitSizeOffset)));
			IRCamera->AmbientTemperatureSetting = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 131 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 132 + MetaCal16BitSizeOffset)));
			IRCamera->HumiditySetting = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 133 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 134 + MetaCal16BitSizeOffset)));
			IRCamera->EmissivitySetting = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 135 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 136 + MetaCal16BitSizeOffset)));
			IRCamera->DistanceSetting = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 137 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 138 + MetaCal16BitSizeOffset)));

		break;

		// Supported camera pool 2
		case _SupportedThermalCameras_Pool_2:

			// Read the internal configuration parameters of the camera - from the associated GUI up/down controls
			IRCamera->TemperatureCorrectionSetting = _IRThermalCameraDefault_TemperatureCorrectionValue;
			IRCamera->AmbientTemperatureSetting = _IRThermalCameraDefault_AmbientTemperatureValue;
			IRCamera->ReflectedTemperatureSetting = _IRThermalCameraDefault_ReflectedTemperatureValue;
			IRCamera->HumiditySetting = _IRThermalCameraDefault_SurroundingHumidityValue;
			IRCamera->EmissivitySetting = _IRThermalCameraDefault_ObjectEmissivityValue;
			IRCamera->DistanceSetting = _IRThermalCameraDefault_ObjectDistanceValue;

		break;

		// Supported camera pool 3
		case _SupportedThermalCameras_Pool_3:

			// Read the internal configuration parameters of the camera
			//IRCamera->TemperatureCorrectionSetting = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 127 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 128 + MetaCal16BitSizeOffset)));
			IRCamera->ReflectedTemperatureSetting = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 129 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 130 + MetaCal16BitSizeOffset)));
			IRCamera->AmbientTemperatureSetting = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 131 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 132 + MetaCal16BitSizeOffset)));
			IRCamera->HumiditySetting = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 133 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 134 + MetaCal16BitSizeOffset)));
			IRCamera->EmissivitySetting = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 135 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 136 + MetaCal16BitSizeOffset)));
			IRCamera->DistanceSetting = RMH_Conversion_uint16x2ToSinglePrecisionFloat(*(ThermalData + (IRCamera->MetaData2Index + 137 + MetaCal16BitSizeOffset)), *(ThermalData + (IRCamera->MetaData2Index + 138 + MetaCal16BitSizeOffset))) + 1.0;

		break;

		// Supported camera pools 4 and 5
		case _SupportedThermalCameras_Pool_4: case _SupportedThermalCameras_Pool_5:

			// Read the internal configuration parameters of the camera - from the associated GUI up/down controls
			IRCamera->TemperatureCorrectionSetting = _IRThermalCameraDefault_TemperatureCorrectionValue;
			IRCamera->AmbientTemperatureSetting = _IRThermalCameraDefault_AmbientTemperatureValue;
			IRCamera->ReflectedTemperatureSetting = _IRThermalCameraDefault_ReflectedTemperatureValue;
			IRCamera->HumiditySetting = _IRThermalCameraDefault_SurroundingHumidityValue;
			IRCamera->EmissivitySetting = _IRThermalCameraDefault_ObjectEmissivityValue;
			IRCamera->DistanceSetting = _IRThermalCameraDefault_ObjectDistanceValue;

		break;

		// Supported camera pool 6
		case _SupportedThermalCameras_Pool_6:

			// The camera has no emissivity/atmospheric correction settings at all - not in its raw frame
			// data, and not exposed anywhere in the vendor's own app either (confirmed: there is no such
			// setting in "TM THOR"). The raw pixel value is used as-is (see
			// RMH_IRThermalCamera_LinearAutomaticGainControlTemp()), so these are just fixed defaults for
			// the configuration panel to display - they play no part in the pool 6 temperature calculation.
			IRCamera->TemperatureCorrectionSetting = _IRThermalCameraDefault_TemperatureCorrectionValue;
			IRCamera->AmbientTemperatureSetting = _IRThermalCameraDefault_AmbientTemperatureValue;
			IRCamera->ReflectedTemperatureSetting = _IRThermalCameraDefault_ReflectedTemperatureValue;
			IRCamera->HumiditySetting = _IRThermalCameraDefault_SurroundingHumidityValue;
			IRCamera->EmissivitySetting = _IRThermalCameraDefault_ObjectEmissivityValue;
			IRCamera->DistanceSetting = _IRThermalCameraDefault_ObjectDistanceValue;

		break;

	}

}

void RMH_IRThermalCamera_WriteCameraConfigParameter(unsigned int ParameterAddress, float ParameterValue, unsigned short SupportedCameraPool) {

	// This routine writes the given camera configuration parameter to the camera.

	/*
	 *  Associated macros ->  
	 * 
	 *   // Internal IR Camera Configuration Parameter Addresses - For Supported Camera Pool x
	 *   #define _IRCameraPoolx_ConfigParameterAddress_TempCorr      0x0000
	 *   #define _IRCameraPoolx_ConfigParameterAddress_ReflTemp      0x0004
	 *   #define _IRCameraPoolx_ConfigParameterAddress_AmbTemp       0x0008
	 *   #define _IRCameraPoolx_ConfigParameterAddress_Humidity      0x000C
	 *   #define _IRCameraPoolx_ConfigParameterAddress_Emissivity    0x0010
	 *   #define _IRCameraPoolx_ConfigParameterAddress_Distance      0x0014
	 * 
	 */

	// Read the temporary array data and sort the kernel array
	unsigned char* DataPointer = (unsigned char*)&ParameterValue;

	// Which supported camera pool is selected
	switch (SupportedCameraPool) {

		// Supported camera pool 1
		case _SupportedThermalCameras_Pool_1:

			// Write the float parameter to the internal camera address of the parameter
			IRThermalCamera.setZoom((((ParameterAddress + 0) & 0x7F) << 8) | DataPointer[0]);
			IRThermalCamera.setZoom((((ParameterAddress + 1) & 0x7F) << 8) | DataPointer[1]);
			IRThermalCamera.setZoom((((ParameterAddress + 2) & 0x7F) << 8) | DataPointer[2]);
			IRThermalCamera.setZoom((((ParameterAddress + 3) & 0x7F) << 8) | DataPointer[3]);
			
		break;

		// Supported camera pool 2
		case _SupportedThermalCameras_Pool_2:


			
		break;

		// Supported camera pool 3
		case _SupportedThermalCameras_Pool_3:

			// Write the float parameter to the internal camera address of the parameter
			IRThermalCamera.setZoom((((ParameterAddress + 0) & 0x7F) << 8) | DataPointer[0]);
			IRThermalCamera.setZoom((((ParameterAddress + 1) & 0x7F) << 8) | DataPointer[1]);
			IRThermalCamera.setZoom((((ParameterAddress + 2) & 0x7F) << 8) | DataPointer[2]);
			IRThermalCamera.setZoom((((ParameterAddress + 3) & 0x7F) << 8) | DataPointer[3]);

		break;

		// Supported camera pool 4
		case _SupportedThermalCameras_Pool_4:



		break;

	}

}

void RMH_IRThermalCamera_SaveConfigParametersToCamera(ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short SupportedCameraPool) {

	// This routine writes the configured camera configuration parameters from the "IRcamera" object
	// to the internal memory of the camera.

	// Which supported camera pool is selected
	switch (SupportedCameraPool) {

		// Supported camera pools 1 and 3
		case _SupportedThermalCameras_Pool_1: case _SupportedThermalCameras_Pool_3:

			// Write the configuration parameters to the internal memory of the camera
			//RMH_IRThermalCamera_WrtreCameraConfigParameter(_IRCamera_ConfigParameterAddress_TempCorrREG, IRCamera->TemperatureCorrectionSetting);
			RMH_IRThermalCamera_WriteCameraConfigParameter(_IRCameraPool1_ConfigParameterAddress_ReflTempREG, IRCamera->ReflectedTemperatureSetting, SupportedCameraPool);
			RMH_IRThermalCamera_WriteCameraConfigParameter(_IRCameraPool1_ConfigParameterAddress_AmbTempREG, IRCamera->AmbientTemperatureSetting, SupportedCameraPool);
			RMH_IRThermalCamera_WriteCameraConfigParameter(_IRCameraPool1_ConfigParameterAddress_HumidityREG, IRCamera->HumiditySetting, SupportedCameraPool);
			RMH_IRThermalCamera_WriteCameraConfigParameter(_IRCameraPool1_ConfigParameterAddress_EmissivityREG, IRCamera->EmissivitySetting, SupportedCameraPool);
			RMH_IRThermalCamera_WriteCameraConfigParameter(_IRCameraPool1_ConfigParameterAddress_DistanceREG, IRCamera->DistanceSetting, SupportedCameraPool);
		
		break;

		// Supported camera pool 2
		case _SupportedThermalCameras_Pool_2:



		break;

		// Supported camera pool 4
		case _SupportedThermalCameras_Pool_4:



		break;

	}

}

// --------------------------------------------- Thermodynamic Calculation Routines --------------------------------------------- //

double RMH_IRThermalCamera_CalAtmosphericWaterVaporContribution(double Humidity, double AmbientTemp) {

	// This routine calculates the reflected contribution from atmospheric water vapor
	// The routine returns omega as a double

	// Locally defined constants
	double Omega = 0.0;

	// Coefficients for the amount of water vapor in the atmosphere
	double h1 = 1.5587;
	double h2 = 0.069390;
	double h3 = -0.000278160;
	double h4 = 0.68455e-6;

	// Calculate the reflected contribution from atmospheric water vapor
	Omega = Humidity * exp(pow(AmbientTemp, 3) * h4 + pow(AmbientTemp, 2) * h3 + AmbientTemp * h2 + h1);

	// Return the calculated omega
	return Omega;

}

double RMH_IRThermalCamera_CalAtmosphericWaterVaporAttenuation(double Omega, unsigned short DistanceMeters) {

	// This routine calculates the attenuation caused by atmospheric water vapor
	// The routine returns the atmospheric wavelength transmission "Tau" as a double

	// Read the temporary array data and sort the kernel array
	double Tau = 0.0;

	// Atmospheric attenuation constant
	double Katm = 1.9;

	// Atmospheric water vapor attenuation constants
	double a1 = 0.0066;
	double a2 = 0.0126;

	// Water vapor attenuation constants (at the Earth's surface)
	double b1 = -0.0023;
	double b2 = -0.0067;

	// Calculate the contribution from the atmospheric wavelength transmission
	Tau = Katm * exp(-sqrt((double)DistanceMeters) * (a1 + b1 * sqrt(Omega))) + (1 - Katm) * exp(-sqrt((double)DistanceMeters) * (a2 + b2 * sqrt(Omega)));

	// Return the calculated tau
	return Tau;

}

// --------------------------------------------- Thermographic Calculation Routines ---------------------------------------------- //

// CHANGED 18-11-2025 !!!!!!!
void RMH_IRThermalCamera_GenerateThermoGrapicLookUpTable(ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short SupportedCameraPool) {

	// This routine calculates and generates a temperature look-up table
	// which is used to convert pixel data to actual temperature

	// Read the temporary array data and sort the kernel array
	double n = 0.0;
	double Tau = 0.0;
	double wtot, ttot;
	double Omega = 0.0;
	double ObjectTemp = 0;
	double NumeratorPart = 0.0;
	double Sigma = 0.0000000567;
	double DenominatorPart = 0.0;
	double LookUpTableOffset = 0.0;
	double CalValue0Correction = 0.0;
	double SqrtComplexNegative = 0.0;
	double EmissivityTauSigma = 0.0;
	double EmissivitySigma = 0.0;
	double TSensorContribution = 0.0;
	double TReflectContribution = 0.0;
	double TAmbientContribution = 0.0;
	double TSensCoreContribution = 0.0;
	double CalValue_A, CalValue_B, CalValue_C, CalValue_D;
	
	// Which supported camera pool is selected
	switch (SupportedCameraPool) {

		// Supported camera pools 1 and 3
		case _SupportedThermalCameras_Pool_1: case _SupportedThermalCameras_Pool_3:

			// ------------------------------------------------- Thermographic Look-Up Table Calculations Pool 1 ------------------------------------------------- // 

			// Calculate the reflected contribution from atmospheric water vapor
			Omega = RMH_IRThermalCamera_CalAtmosphericWaterVaporContribution(IRCamera->HumiditySetting, IRCamera->AmbientTemperatureSetting);
			// The function calculates the attenuation factor from atmospheric water vapor
			Tau = RMH_IRThermalCamera_CalAtmosphericWaterVaporAttenuation(Omega, IRCamera->DistanceSetting);

			// Split the numerator and denominator of the object temperature equation
			NumeratorPart = (1.0 - IRCamera->EmissivitySetting) * Tau * pow((IRCamera->ReflectedTemperatureSetting + 273.15), 4) + (1.0 - Tau) * pow((IRCamera->AmbientTemperatureSetting + 273.15), 4);
			DenominatorPart = IRCamera->EmissivitySetting * Tau;

			// Calculate the compensated camera calibration values
			CalValue_A = IRCamera->CalValue2 / (IRCamera->CalValue1 + IRCamera->CalValue1);
			CalValue_B = IRCamera->CalValue2 * IRCamera->CalValue2 / (IRCamera->CalValue1 * IRCamera->CalValue1 * 4.0);
			CalValue_C = IRCamera->CalValue1 * pow(IRCamera->temp_shutter, 2) + IRCamera->temp_shutter * IRCamera->CalValue2;
			CalValue_D = IRCamera->CalValue3 * pow(IRCamera->temp_fpa, 2) + IRCamera->CalValue4 * IRCamera->temp_fpa + IRCamera->CalValue5;

			// If the temperature range of the IR camera is low range
			if (IRCamera->CurrentIRTempRangeFlag == 1) {

				// Calculate the correction to the calibration value
				CalValue0Correction = IRCamera->CalValue0Offset - IRCamera->temp_fpa * IRCamera->CalValue0Fpamul;

				// If the correction is lower than '0'
				if (CalValue0Correction < 0.0) {
					// Reset the correction value
					CalValue0Correction = 0.0;
				}

			}
			else {

				// Reset the correction value
				CalValue0Correction = 0.0;

			}

			// Calculate the offset value of the look-up table
			LookUpTableOffset = IRCamera->CalValue0 - CalValue0Correction;

			// Generate a look-up table with the length of the bit width of the IR sensor - 14-bit 
			for (unsigned int i = 0; i < 16384; i++) {

				// Calculate the square root parameter  
				SqrtComplexNegative = (((double)i - LookUpTableOffset) * CalValue_D + CalValue_C) / IRCamera->CalValue1 + CalValue_B;
					
				// Check whether the value is negative
				if (SqrtComplexNegative < 0) {
					// Convert the square root argument to a positive value
					n = sqrt(-SqrtComplexNegative);
				}
				else {
					// Calculate the square root of the positive value
					n = sqrt(SqrtComplexNegative);
				}
				
				// Formulate a 2nd-order polynomial for the object temperature calculation
				wtot = pow((n - CalValue_A + 273.15), 4);
				ttot = pow(((wtot - NumeratorPart) / DenominatorPart), 0.25) - 273.15;

				// Calculate the object temperature
				ObjectTemp = ttot + (IRCamera->DistanceSetting * 0.85 - 1.125) * (ttot - IRCamera->AmbientTemperatureSetting) / 100.0 + IRCamera->TemperatureCorrectionSetting;

				// Write the data to the temperature look-up table
				IRCamera->TemperatureLookUpTabel[i] = ObjectTemp;

			}

			// -------------------------------------------------------------------------------------------------------------------------------------------------- // 

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "New Temperature Look-Up Tabel Has Been Generated.", _StatusMessageType_Normal);
			
		break;

		// Supported camera pools 2, 4, 5 and 6
		case _SupportedThermalCameras_Pool_2: case _SupportedThermalCameras_Pool_4: case _SupportedThermalCameras_Pool_5: case _SupportedThermalCameras_Pool_6:

			// Calculate the reflected contribution from atmospheric water vapor
			Omega = RMH_IRThermalCamera_CalAtmosphericWaterVaporContribution(IRCamera->HumiditySetting, IRCamera->AmbientTemperatureSetting);
			// The function calculates the attenuation factor from atmospheric water vapor
			Tau = RMH_IRThermalCamera_CalAtmosphericWaterVaporAttenuation(Omega, IRCamera->DistanceSetting);

			// Calculate the environmental contribution to the temperature calculations - compensate for external temperatures
			TReflectContribution = (1.0 - IRCamera->EmissivitySetting) * Tau * Sigma * IRCamera->ReflectedTemperatureSetting;
			TAmbientContribution = (1.0 - Tau) * Sigma * IRCamera->AmbientTemperatureSetting;
			TSensCoreContribution = (1.0 - IRCamera->temp_shutter / IRCamera->temp_core) * Sigma * IRCamera->temp_shutter;
			EmissivityTauSigma = 1.0 / (IRCamera->EmissivitySetting * Tau * Sigma);
			EmissivitySigma = Sigma * IRCamera->EmissivitySetting;

			// Calculate the scaling factor and offset of the environmental contribution 
			IRCamera->ObjectEnvirTempCorrectionOffset = -(TReflectContribution * EmissivityTauSigma) - (TAmbientContribution * EmissivityTauSigma) - (TSensCoreContribution * EmissivityTauSigma);
			IRCamera->ObjectEnvirTempCorrectionFactor = EmissivityTauSigma * EmissivitySigma;

		break;

	}

}

double RMH_IRThermalCamera_ReadPixelTemperature(ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short PixelValue, unsigned short SupportedCameraPool) {

	// This routine returns the pixel temperature from the generated temperature look-up table or from the pool-specific calculation

	// Read the temporary array data and sort the kernel array
	double PixelTemperature = 0.0;

	// Which supported camera pool is selected
	switch (SupportedCameraPool) {

		// Supported camera pools 1 and 3
		case _SupportedThermalCameras_Pool_1: case _SupportedThermalCameras_Pool_3:

			// Read the pixel temperature from the look-up table
			PixelTemperature = IRCamera->TemperatureLookUpTabel[PixelValue & 0x3FFF];

		break;

		// Supported camera pools 2, 4 and 5
		case _SupportedThermalCameras_Pool_2: case _SupportedThermalCameras_Pool_4: case _SupportedThermalCameras_Pool_5:

			// Calculate the pixel temperature from the raw pixel data
			PixelTemperature = ((double)PixelValue * 0.015625) - 273.15;
			// Compensate for the environmental contribution to the temperature calculations
			PixelTemperature = PixelTemperature * IRCamera->ObjectEnvirTempCorrectionFactor + IRCamera->ObjectEnvirTempCorrectionOffset;
			// Compensate for the temperature correction 
			PixelTemperature = PixelTemperature + IRCamera->TemperatureCorrectionSetting;

		break;

		// Supported camera pool 6
		case _SupportedThermalCameras_Pool_6:

			// The raw pixel value is temperature in deci-Kelvin (Kelvin x10) - see
			// RMH_IRThermalCamera_LinearAutomaticGainControlTemp() for how this was confirmed.
			PixelTemperature = ((double)PixelValue / 10.0) - 273.15;
			// Compensate for the temperature correction
			PixelTemperature = PixelTemperature + IRCamera->TemperatureCorrectionSetting;

		break;

	}

	// Return the pixel temperature
	return PixelTemperature;

}

double RMH_IRThermalCamera_ReadFramePixelTemperature(ThermalCameraDevice::IRCameraDeviceFormat* IRCamera, unsigned short* ThermalData, unsigned short PixelWidth, unsigned short PixelHeight, unsigned short SupportedCameraPool) {

	// This routine reads and returns the data value of a pixel from the thermal frame data array
	// The input pixel positions are in matrix form as the position in the matrix: Width x Height

	// Read the temporary array data and sort the kernel array
	double PixelTemperature = 0.0;
	unsigned int PixelData = 0;
	unsigned int ArrayIndex = 0;

	// Convert the matrix index to an array index
	ArrayIndex = (PixelHeight * IRCamera->FrameWidth) + PixelWidth;

	// Which supported camera pool is selected
	switch (SupportedCameraPool) {

		// Supported camera pools 1 and 3
		case _SupportedThermalCameras_Pool_1: case _SupportedThermalCameras_Pool_3:

			// Prevent array index overflow
			if (ArrayIndex > (IRCamera->FrameWidth * IRCamera->FrameHeight) - 1) { ArrayIndex = 0; }

			// Read the pixel data from the thermal frame data
			PixelData = *(ThermalData + ArrayIndex);

			// Read the pixel temperature
			PixelTemperature = IRCamera->TemperatureLookUpTabel[PixelData & 0x3FFF];

		break;

		// Supported camera pools 2, 4 and 5
		case _SupportedThermalCameras_Pool_2: case _SupportedThermalCameras_Pool_4: case _SupportedThermalCameras_Pool_5:

			// Prevent array index overflow
			if (ArrayIndex > (IRCamera->FrameWidth * (IRCamera->FrameHeight - IRCamera->FrameMetadataSize)) - 1) { ArrayIndex = 0; }

			// Read the pixel data from the thermal frame data
			PixelData = *(ThermalData + ArrayIndex);

			// Calculate the pixel temperature from the raw pixel data
			PixelTemperature = ((double)PixelData * 0.015625) - 273.15;
			// Compensate for the environmental contribution to the temperature calculations
			PixelTemperature = PixelTemperature * IRCamera->ObjectEnvirTempCorrectionFactor + IRCamera->ObjectEnvirTempCorrectionOffset;
			// Compensate for the temperature correction
			PixelTemperature = PixelTemperature + IRCamera->TemperatureCorrectionSetting;

		break;

		// Supported camera pool 6
		case _SupportedThermalCameras_Pool_6:

			// Prevent array index overflow
			if (ArrayIndex > (IRCamera->FrameWidth * (IRCamera->FrameHeight - IRCamera->FrameMetadataSize)) - 1) { ArrayIndex = 0; }

			// Read the pixel data from the thermal frame data
			PixelData = *(ThermalData + ArrayIndex);

			// The raw pixel value is temperature in deci-Kelvin (Kelvin x10) - see
			// RMH_IRThermalCamera_LinearAutomaticGainControlTemp() for how this was confirmed.
			PixelTemperature = ((double)PixelData / 10.0) - 273.15;
			// Compensate for the temperature correction
			PixelTemperature = PixelTemperature + IRCamera->TemperatureCorrectionSetting;

		break;

	}

	// Return the pixel temperature 
	return PixelTemperature;

}

// ------------------------------- Recording/Snapshot Analysis Mode Camera Pool-Specific Routines ------------------------------- //

void RMH_IRThermalCamera_WriteDataToVideoRecordingFilesSequence(unsigned short SupportedCameraPool) {

	// This routine handles writing the selected camera pool data to video files, if video recording has started and is ready

	// If video recording has started, the video files are ready for writing
	if (VideoRecordingStartedFlag == true && VideoFilesReadyFlag == true) {

		// Should the RAW unprocessed camera data be saved 
		if (SaveRAWDataRecordingFlag == true) {

			// Which supported camera pool is selected
			switch (SupportedCameraPool) {

				// Supported camera pools 1, 2, 4, 5 and 6
				case _SupportedThermalCameras_Pool_1: case _SupportedThermalCameras_Pool_2: case _SupportedThermalCameras_Pool_4: case _SupportedThermalCameras_Pool_5: case _SupportedThermalCameras_Pool_6:

					// Write the RAW camera data frames to the video file
					RMH_VideoFileRecording_WriteDataToFile(_VideoFileWriteObject_RecordingAnalysisModeFile, IRCamera.FrameWidth, IRCamera.FrameHeight, IRCameraFrameData);

				break;

				// Supported camera pool 3
				case _SupportedThermalCameras_Pool_3:

					// Convert the thermographic data array to a YUY2 array - with NUC-corrected data
					RMH_IRThermalCamera_Convert14BitThermalDataArrayToYUY2(&FrameThermalDataRaw[0], &FrameThermalData3Band[0], IRCamera.FrameWidth, IRCamera.FrameHeight);

					// Write the NUC-corrected RAW camera data frames to the video file
					RMH_VideoFileRecording_WriteDataToFile(_VideoFileWriteObject_RecordingAnalysisModeFile, IRCamera.FrameWidth, IRCamera.FrameHeight, FrameThermalData3Band);
				
				break;

			}

		}

		// If live view ultra resolution mode is enabled
		if (UltraResolutionEnableFlag == true) {

			// Write the processed camera data frames to the video file
			RMH_VideoFileRecording_WriteDataToFile16Bit(_VideoFileWriteObject_LiveViewStreamFile, IRCamera.FrameWidth * UltraResolutionScaleFactor, (IRCamera.FrameHeight - IRCamera.FrameMetadataSize) * UltraResolutionScaleFactor, PrecessedUltraResolutionImage);

		}
		else {

			// Write the processed camera data frames to the video file
			RMH_VideoFileRecording_WriteDataToFile16Bit(_VideoFileWriteObject_LiveViewStreamFile, IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize, ProcessedThermalImage);

		}

	}

}

bool RMH_IRThermalCamera_ConvertCapturedRawImageDataToSnapshotPNG(unsigned short SupportedCameraPool) {

	// This routine handles the conversion of the selected camera pool RAW data to snapshot PNG data.
	// The routine returns the snapshot file status

	// Read the temporary array data and sort the kernel array
	bool SnapshotStatus = false;

	// Which supported camera pool is selected
	switch (SupportedCameraPool) {

		// Supported camera pools 1, 2, 4 and 6 (Thermal Master THOR001) - pool 6's FrameWidth/FrameHeight (256 x
		// 192) are already the true displayed image size (unlike pool 5's 256 x 386 full-buffer size), and its
		// raw data starts only FrameWidthPixelOffset (5) pixels into the buffer, so the metadata row this writes
		// at (FrameHeight - 1) falls past the real _SupporteredeThermalCameraPool6_RawFrameSizeBytes (98314)
		// altogether - it lands in the buffer's unused padding tail, not over any real pixel data, so pool 6
		// needs no re-encoding step of its own: saving &IRCameraFrameData[0] directly works like pools 1/2/4.
		case _SupportedThermalCameras_Pool_1: case _SupportedThermalCameras_Pool_2: case _SupportedThermalCameras_Pool_4: case _SupportedThermalCameras_Pool_6:

			// Generate and write extra RAW metadata to the frame data array 
			RMH_AnalysisMode_AddIDAndMetaDataToFrameArray(IRCamera.FrameWidth, IRCamera.FrameHeight, &IRCameraFrameData[0],
				IRCamera.ThermalCameraSupportPool, IRCamera.FrameMetadataSize, IRCamera.FrameWidthPixelOffset, IRCamera.FrameHeightPixelOffset,
				IRCamera.TemperatureCorrectionSetting, IRCamera.AmbientTemperatureSetting, IRCamera.ReflectedTemperatureSetting, IRCamera.HumiditySetting, IRCamera.EmissivitySetting, IRCamera.DistanceSetting);

			// Save a raw sensor data snapshot from the connected thermal camera
			SnapshotStatus = RMH_Winforms_SaveRawImageDataAsSnapShotPNG(GlobalVariables::SnapShotDefaultPath, IRCamera.FrameWidth, IRCamera.FrameHeight, &IRCameraFrameData[0]);

		break;

		// Supported camera pool 5 (Thermal Master P3) - RMH_AnalysisMode_AddIDAndMetaDataToFrameArray() and
		// RMH_Winforms_SaveRawImageDataAsSnapShotPNG() both treat the frame buffer as 3 bytes/pixel, which
		// matches what pool 1/2/4 actually get from DirectShow - but P3's own WinUSB backend (p3_winusb_camera.cpp)
		// delivers tightly-packed 2-byte/pixel raw samples straight off the wire, with no such conversion.
		// Saving &IRCameraFrameData[0] directly at IRCamera.FrameWidth x IRCamera.FrameHeight (as the pool
		// 1/2/4 case above does) therefore reads the 256 x 386 raw buffer with the wrong stride and produces
		// a corrupt PNG. Re-encode the real, already-offset-corrected 256 x 192 image (the exact pixels the
		// live view displays, after applying FrameWidthPixelOffset/FrameHeightPixelOffset) into its own
		// 3-bytes/pixel buffer first instead - R = low byte, G = high byte, B unused. The counterpart of this
		// (de-interleaving back to 2 bytes/pixel on load) is in RMH_IRThermalCamera_ConnectToThermalCamera()'s
		// Snapshot Analysis Mode handling, right before RMH_IRThermalCamera_ConvertYUY2To14BitThermalDataArray().
		case _SupportedThermalCameras_Pool_5: {

			unsigned int SnapshotWidth = IRCamera.FrameWidth;
			unsigned int SnapshotImageHeight = IRCamera.FrameHeight - IRCamera.FrameMetadataSize;
			// An extra pixel row is appended on top of the real, offset-corrected image for the ID/metadata
			// string, instead of overwriting the image's own last row (as the pool 1/2/4 case above does) -
			// unlike that shared path, which relies on the sacrificed row already being outside the real
			// image (true for pools 1-4's own camera-embedded metadata rows), pool 5's saved buffer here IS
			// exactly the real image with nothing spare, so every one of its rows is kept intact.
			unsigned int SnapshotHeight = SnapshotImageHeight + 1;
			unsigned int FrameDataArrayOffset = (IRCamera.FrameHeightPixelOffset * IRCamera.FrameWidth) + IRCamera.FrameWidthPixelOffset;
			unsigned char* Pool5SnapshotBuffer = new unsigned char[SnapshotWidth * SnapshotHeight * 3];

			for (unsigned int i = 0; i < SnapshotWidth * SnapshotImageHeight; i++) {

				Pool5SnapshotBuffer[i * 3 + 0] = IRCameraFrameData[(FrameDataArrayOffset + i) * 2 + 0];
				Pool5SnapshotBuffer[i * 3 + 1] = IRCameraFrameData[(FrameDataArrayOffset + i) * 2 + 1];
				Pool5SnapshotBuffer[i * 3 + 2] = 0;

			}

			// Generate and write extra RAW metadata to the frame data array - the saved buffer already IS the
			// real, offset-corrected image, so the pixel offsets recorded for it are both zero, but the
			// metadata size is 1: the loader must exclude this extra row from its min/max scan, or the
			// ID-string/float bytes would be read as a wildly out-of-range "pixel" and blow out the whole
			// colour range. RMH_IRThermalCamera_ConnectToThermalCameraOrAnalysisMode()'s pool 5 de-interleave
			// step (before RMH_IRThermalCamera_ConvertYUY2To14BitThermalDataArray()) already processes
			// IRCamera.FrameWidth x IRCamera.FrameHeight pixels generically, so it needs no change for the
			// extra row - it reads back as the file's own height (SnapshotHeight), the same way it is written.
			RMH_AnalysisMode_AddIDAndMetaDataToFrameArray(SnapshotWidth, SnapshotHeight, Pool5SnapshotBuffer,
				IRCamera.ThermalCameraSupportPool, 1, 0, 0,
				IRCamera.TemperatureCorrectionSetting, IRCamera.AmbientTemperatureSetting, IRCamera.ReflectedTemperatureSetting, IRCamera.HumiditySetting, IRCamera.EmissivitySetting, IRCamera.DistanceSetting);

			// Save a raw sensor data snapshot from the connected thermal camera
			SnapshotStatus = RMH_Winforms_SaveRawImageDataAsSnapShotPNG(GlobalVariables::SnapShotDefaultPath, SnapshotWidth, SnapshotHeight, Pool5SnapshotBuffer);

			delete[] Pool5SnapshotBuffer;

		}
		break;

		// Supported camera pool 3
		case _SupportedThermalCameras_Pool_3:

			// Generate and write extra RAW metadata to the frame data array - for pool 3 cameras with NUC correction
			RMH_AnalysisMode_AddIDAndMetaDataToFrameArray(IRCamera.FrameWidth, IRCamera.FrameHeight, &FrameThermalData3Band[0],
				IRCamera.ThermalCameraSupportPool, IRCamera.FrameMetadataSize, IRCamera.FrameWidthPixelOffset, IRCamera.FrameHeightPixelOffset,
				IRCamera.TemperatureCorrectionSetting, IRCamera.AmbientTemperatureSetting, IRCamera.ReflectedTemperatureSetting, IRCamera.HumiditySetting, IRCamera.EmissivitySetting, IRCamera.DistanceSetting);

			// Convert the thermographic data array to a YUY2 array - with NUC-corrected data
			RMH_IRThermalCamera_Convert14BitThermalDataArrayToYUY2(&FrameThermalDataRaw[0], &FrameThermalData3Band[0], IRCamera.FrameWidth, IRCamera.FrameHeight);

			// Save a raw sensor data snapshot from the connected thermal camera - with NUC-corrected data
			SnapshotStatus = RMH_Winforms_SaveRawImageDataAsSnapShotPNG(GlobalVariables::SnapShotDefaultPath, IRCamera.FrameWidth, IRCamera.FrameHeight, &FrameThermalData3Band[0]);

		break;

	}

	// Return the snapshot status
	return SnapshotStatus;

}

// --------------------------------- Thermal Camera Pool-Specific Handling/Control Routines --------------------------------- //

void RMH_IRThermalCamera_AutoShutterCalTimerCallbackHandler() {

	// This routine handles the events when the auto calibration timer callback executes  

	// If automatic shutter calibration is enabled
	if (AutoShutterCalEnableFlag == true) {

		// Calibrate the thermal camera
		RMH_IRThermalCamera_CalibrateThermalCamera();

	}

}

void RMH_IRThermalCamera_TempDriftBasedCalTimerCallbackHandler() {

	// This routine handles the events when the temperature-drift-based calibration timer callback executes  

	// Check whether the current drift of the thermal camera is greater than or equal to the setpoint value
	if (RMH_Math_absDouble(SensorTemperatureCalDrift * TemperatureUnitScaleFactor) >= TempDriftCalibrationSetValue) {

		// Calibrate the thermal camera
		RMH_IRThermalCamera_CalibrateThermalCamera();

	}

}

void RMH_IRThermalCamera_CalibrateThermalCamera() {

	// This routine calibrates the connected thermal camera
	// and handles the events during camera calibration

	// Which thermal camera pool is connected
	switch (IRCamera.ThermalCameraSupportPool) {

		// Supported camera pool 1
		case _SupportedThermalCameras_Pool_1:

			// Update the calibration button border color
			GlobalVariables::GlobalCalibrateCameraButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;
			GlobalVariables::GlobalCalibrateCameraButton->Refresh();

			// Calibrate the thermal camera
			RMH_IRThermalCamera_CalibrateIRCamera(IRCamera.ThermalCameraSupportPool);

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Is Calibrating...", _StatusMessageType_Normal);

			// Wait for the calibration to finish
			System::Threading::Thread::Sleep(_ThermalCameraShutter_CloseTimeMs);

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Calibrating Finished.", _StatusMessageType_Success);

			// Update the calibration button border color
			GlobalVariables::GlobalCalibrateCameraButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

			// Read a single frame of data from the thermal camera
			RMH_IRThermalCamera_ReadFrameRaw(&IRCameraFrameData[0], &VideoFrameSize);
			// Format the raw YUY2 data to a 16-bit thermal data array
			RMH_IRThermalCamera_ConvertYUY2To14BitThermalDataArray(&IRCamera, &IRCameraFrameData[0], &FrameThermalDataRaw[0]);
			// Read the frame metadata of the IR camera and calculate the internal IR sensor temperatures
			RMH_IRThermalCamera_ReadCalFrameMetaData(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);
			// Read the internal calibration parameters of the IR camera
			RMH_IRThermalCamera_ReadCalibrationParameters(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

			// Generate/update the temperature look-up table
			RMH_IRThermalCamera_GenerateThermoGrapicLookUpTable(&IRCamera, IRCamera.ThermalCameraSupportPool);
			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Generated New Temperature Look-Up Tabel.", _StatusMessageType_Normal);

		break;

		// Supported camera pools 2 and 4
		case _SupportedThermalCameras_Pool_2: case _SupportedThermalCameras_Pool_4:

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "This Function Is Not Supported For The Camera In This Version Of IRCAM Thermal Viewer", _StatusMessageType_Warning);

		break;

		// Supported camera pool 5 (Thermal Master P3) - the camera performs its own shutter (NUC)
		// calibration internally; there is no look-up table to regenerate on this side (pool 5
		// calculates temperature directly from the raw pixel value, like pool 4)
		case _SupportedThermalCameras_Pool_5:

			// Update the calibration button border color
			GlobalVariables::GlobalCalibrateCameraButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;
			GlobalVariables::GlobalCalibrateCameraButton->Refresh();

			// Trigger the camera's shutter calibration
			RMH_IRThermalCamera_CalibrateIRCamera(IRCamera.ThermalCameraSupportPool);

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Is Calibrating...", _StatusMessageType_Normal);

			// Wait for the calibration shutter to close and reopen
			System::Threading::Thread::Sleep(_ThermalCameraShutter_CloseTimeMs + _ThermalCameraShutter_OpenTimeMs);

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Calibrating Finished.", _StatusMessageType_Success);

			// Update the calibration button border color
			GlobalVariables::GlobalCalibrateCameraButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

		break;

		// Supported camera pool 3
		case _SupportedThermalCameras_Pool_3:

			// Update the calibration button border color
			GlobalVariables::GlobalCalibrateCameraButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;
			GlobalVariables::GlobalCalibrateCameraButton->Refresh();

			// Calibrate the thermal camera
			RMH_IRThermalCamera_CalibrateIRCamera(IRCamera.ThermalCameraSupportPool);

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Shutter Is Closed...", _StatusMessageType_Normal);

			// Wait for the calibration shutter to close
			System::Threading::Thread::Sleep(_ThermalCameraShutter_CloseTimeMs);

			// Read a single frame with the camera shutter closed - CMOS baseline measurement
			RMH_IRThermalCamera_ReadFrameRaw(&IRCameraFrameData[0], &VideoFrameSize);

			// --------------------------------------- Non-Uniformity Correction --------------------------------------- //

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Performing Non-Uniformity Correction & Calibration...", _StatusMessageType_Normal);

			// Convert the CMOS baseline measurement to a 16-bit data array
			RMH_ImageNonUniformityCorrection_ConvertBaselineImageTo16Bit(&IRCameraFrameData[0], &ClosedShutterCMOSBaselineData[0], IRCamera.FrameWidth, IRCamera.FrameHeight);

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Calculating Non-Uniformity Mapping...", _StatusMessageType_Normal);

			// Calculate the mean value of the CMOS baseline measurement, together with the non-uniformity mapping of the baseline data
			ImageCMOSBaselineMeanValue = RMH_ImageNonUniformityCorrection_CalNonUniformityMap(&ClosedShutterCMOSBaselineData[0], IRCamera.FrameWidth, IRCamera.FrameHeight, IRCamera.FrameMetadataSize, &ImageCMOSNonUniformityMapData[0]);

			// --------------------------------------------------------------------------------------------------------- //

			// Wait for the calibration shutter to open again
			System::Threading::Thread::Sleep(_ThermalCameraShutter_OpenTimeMs);

			// Read a single frame of data from the thermal camera
			RMH_IRThermalCamera_ReadFrameRaw(&IRCameraFrameData[0], &VideoFrameSize);
			// Format the raw YUY2 data to a 16-bit thermal data array
			RMH_IRThermalCamera_ConvertYUY2To14BitThermalDataArray(&IRCamera, &IRCameraFrameData[0], &FrameThermalDataRaw[0]);
			// Read the frame metadata of the IR camera and calculate the internal IR sensor temperatures
			RMH_IRThermalCamera_ReadCalFrameMetaData(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);
			// Read the internal calibration parameters of the IR camera
			RMH_IRThermalCamera_ReadCalibrationParameters(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

			// Generate/update the temperature look-up table
			RMH_IRThermalCamera_GenerateThermoGrapicLookUpTable(&IRCamera, IRCamera.ThermalCameraSupportPool);

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Non-Uniformity Correction & Calibrating Finished.", _StatusMessageType_Normal);

			// Update the calibration button border color
			GlobalVariables::GlobalCalibrateCameraButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

		break;

	}

	// Store the last calibration sensor detector temperature
	CurrentCalDetectorTemperature = IRCamera.temp_fpa;

}

void RMH_IRThermalCamera_ChangeThermalCameraTemperatureRange() {

	// This routine switches the temperature range of the thermal camera

	// Read the temporary array data and sort the kernel array
	bool SupportsHighTemperatureRangeFlag = false;

	// Check whether the connected camera supports a higher temperature range
	switch (IRCamera.SellectedCameraIndex) {

		// Update the camera pool variable
		case _SupportedThermalCamera_InfiRayT2L:        SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayT2L_SupportsHighRange; break;
		case _SupportedThermalCamera_InfiRayT2LV2:      SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayT2LV2_SupportsHighRange; break;
		case _SupportedThermalCamera_InfiRayT2Search:   SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayT2Search_SupportsHighRange; break;
		case _SupportedThermalCamera_InfiRayT2SearchV2: SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayT2SearchV2_SupportsHighRange; break;
		case _SupportedThermalCamera_InfiRayT2Sp:       SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayT2Sp_SupportsHighRange; break;
		case _SupportedThermalCamera_InfiRayT2SpV2:     SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayT2SpV2_SupportsHighRange; break;
		case _SupportedThermalCamera_InfiRayT2Pro:      SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayT2Pro_SupportsHighRange; break;
		case _SupportedThermalCamera_InfiRayT2ProV2:    SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayT2ProV2_SupportsHighRange; break;
		case _SupportedThermalCamera_InfiRayT3Search:   SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayT3Search_SupportsHighRange; break;
		case _SupportedThermalCamera_InfiRayT3S:	    SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayT3S_SupportsHighRange; break;
		case _SupportedThermalCamera_InfiRayT3Pro:      SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayT3Pro_SupportsHighRange; break;
		case _SupportedThermalCamera_InfiRayP2:         SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayP2_SupportsHighRange; break;
		case _SupportedThermalCamera_ThermalMasterP2:   SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_ThermalMasterP2_SupportsHighRange; break;
		case _SupportedThermalCamera_InfiRayP2Pro:      SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayP2Pro_SupportsHighRange; break;
		case _SupportedThermalCamera_InfiRayDVDL13:     SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayDVDL13_SupportsHighRange; break;
		case _SupportedThermalCamera_InfiRayS0Series:   SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayS0Series_SupportsHighRange; break;
		case _SupportedThermalCamera_InfiRayTiny1C:     SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_InfiRayTiny1C_SupportsHighRange; break;
		case _SupportedThermalCamera_HTIHT301:          SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_HTIHT301_SupportsHighRange; break;
		case _SupportedThermalCamera_UNITUTi260M:       SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_UNITUTi260M_SupportsHighRange; break;
		case _SupportedThermalCamera_TOPDONTC001:       SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_TOPDONTC001_SupportsHighRange; break;
		case _SupportedThermalCamera_TOPDONTC002:       SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_TOPDONTC002_SupportsHighRange; break;
		case _SupportedThermalCamera_Victor328B:        SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_Victor328B_SupportsHighRange; break;
		case _SupportedThermalCamera_LODESTARL2:        SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_LODESTARL2_SupportsHighRange; break;
		case _SupportedThermalCamera_ThermalMasterP3:   SupportsHighTemperatureRangeFlag = _SupportedThermalCamera_ThermalMasterP3_SupportsHighRange; break;

	}

	// If the connected thermal camera supports a higher temperature range
	if (SupportsHighTemperatureRangeFlag == true) {

		// Which thermal camera pool is connected
		switch (IRCamera.ThermalCameraSupportPool) {

			// Supported camera pool 1
			case _SupportedThermalCameras_Pool_1:

				// Toggle the temperature range flag
				ThermalCameraHighRangeFlag = !ThermalCameraHighRangeFlag;

				// Write GUI status message
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Changing Thermal Camera Temperature Range... Please Wait...", _StatusMessageType_Normal);

				// Handle the state of the temperature range flag
				if (ThermalCameraHighRangeFlag == true) {

					// Configure the thermal camera to its highest temperature range
					RMH_IRThermalCamera_SetIRCameraTemperatureRange(_ThermalCamera_TemperatureRange_HighRange, IRCamera.ThermalCameraSupportPool);

					// Update the temperature range button border color
					GlobalVariables::GlobalTempRangeButton->FlatAppearance->BorderColor = System::Drawing::Color::Yellow;

					// Update the temperature range button graphic
					GlobalVariables::GlobalTempRangeButton->Update();
					// Wait for the temperature range to switch correctly
					System::Threading::Thread::Sleep(_ThermalCameraPool1_RangeSwitchReadyTimeMs);

					// Update the IR camera device temperature range variable
					IRCamera.CurrentIRTempRangeFlag = 2;

					// Update the temperature range button border color
					GlobalVariables::GlobalTempRangeButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

				}
				else {

					// Configure the thermal camera to its lowest temperature range
					RMH_IRThermalCamera_SetIRCameraTemperatureRange(_ThermalCamera_TemperatureRange_LowRange, IRCamera.ThermalCameraSupportPool);

					// Update the temperature range button border color
					GlobalVariables::GlobalTempRangeButton->FlatAppearance->BorderColor = System::Drawing::Color::Yellow;

					// Update the temperature range button graphic
					GlobalVariables::GlobalTempRangeButton->Update();
					// Wait for the temperature range to switch correctly
					System::Threading::Thread::Sleep(_ThermalCameraPool1_RangeSwitchReadyTimeMs);

					// Update the IR camera device temperature range variable
					IRCamera.CurrentIRTempRangeFlag = 1;

					// Update the temperature range button border color
					GlobalVariables::GlobalTempRangeButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

				}

				// Write GUI status message
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Temperature Range Was Changed", _StatusMessageType_Normal);

				// Perform a thermal camera calibration
				RMH_IRThermalCamera_CalibrateThermalCamera();

			break;

			// Supported camera pools 2 and 4
			case _SupportedThermalCameras_Pool_2: case _SupportedThermalCameras_Pool_4:

				// Write GUI status message
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "This Function Is Not Supported For The Camera In This Version Of IRCAM Thermal Viewer", _StatusMessageType_Warning);

			break;

			// Supported camera pool 5 (Thermal Master P3) - the camera itself has a genuine hardware
			// high/low range, unlike pool 2/4 - see P3WinUsbCamera::setTemperatureRange()
			case _SupportedThermalCameras_Pool_5:

				// Toggle the temperature range flag
				ThermalCameraHighRangeFlag = !ThermalCameraHighRangeFlag;

				// Write GUI status message
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Changing Thermal Camera Temperature Range... Please Wait...", _StatusMessageType_Normal);

				// Handle the state of the temperature range flag
				if (ThermalCameraHighRangeFlag == true) {

					// Configure the thermal camera to its highest temperature range
					RMH_IRThermalCamera_SetIRCameraTemperatureRange(_ThermalCamera_TemperatureRange_HighRange, IRCamera.ThermalCameraSupportPool);

					// Update the temperature range button border color
					GlobalVariables::GlobalTempRangeButton->FlatAppearance->BorderColor = System::Drawing::Color::Yellow;

					// Update the temperature range button graphic
					GlobalVariables::GlobalTempRangeButton->Update();
					// Wait for the temperature range to switch correctly
					System::Threading::Thread::Sleep(_ThermalCameraPool4_RangeSwitchReadyTimeMs);

					// Update the IR camera device temperature range variable
					IRCamera.CurrentIRTempRangeFlag = 2;

					// Update the temperature range button border color
					GlobalVariables::GlobalTempRangeButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

				}
				else {

					// Configure the thermal camera to its lowest temperature range
					RMH_IRThermalCamera_SetIRCameraTemperatureRange(_ThermalCamera_TemperatureRange_LowRange, IRCamera.ThermalCameraSupportPool);

					// Update the temperature range button border color
					GlobalVariables::GlobalTempRangeButton->FlatAppearance->BorderColor = System::Drawing::Color::Yellow;

					// Update the temperature range button graphic
					GlobalVariables::GlobalTempRangeButton->Update();
					// Wait for the temperature range to switch correctly
					System::Threading::Thread::Sleep(_ThermalCameraPool4_RangeSwitchReadyTimeMs);

					// Update the IR camera device temperature range variable
					IRCamera.CurrentIRTempRangeFlag = 1;

					// Update the temperature range button border color
					GlobalVariables::GlobalTempRangeButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

				}

				// Write GUI status message
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Temperature Range Was Changed", _StatusMessageType_Normal);

				// Perform a thermal camera calibration - switching range needs a fresh shutter (NUC)
				// calibration for the new gain/range, exactly like the vendor app's own range menu does
				RMH_IRThermalCamera_CalibrateThermalCamera();

			break;

			// Supported camera pool 3
			case _SupportedThermalCameras_Pool_3:

				// Toggle the temperature range flag
				ThermalCameraHighRangeFlag = !ThermalCameraHighRangeFlag;

				// Write GUI status message
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Changing Thermal Camera Temperature Range... Please Wait...", _StatusMessageType_Normal);

				// Handle the state of the temperature range flag
				if (ThermalCameraHighRangeFlag == true) {

					// Configure the thermal camera to its highest temperature range
					RMH_IRThermalCamera_SetIRCameraTemperatureRange(_ThermalCamera_TemperatureRange_HighRange, IRCamera.ThermalCameraSupportPool);

					// Update the temperature range button border color
					GlobalVariables::GlobalTempRangeButton->FlatAppearance->BorderColor = System::Drawing::Color::Yellow;

					// Update the temperature range button graphic
					GlobalVariables::GlobalTempRangeButton->Update();
					// Wait for the temperature range to switch correctly
					System::Threading::Thread::Sleep(_ThermalCameraPool3_RangeSwitchReadyTimeMs);

					// Update the IR camera device temperature range variable
					IRCamera.CurrentIRTempRangeFlag = 2;

					// Update the temperature range button border color
					GlobalVariables::GlobalTempRangeButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

				}
				else {

					// Configure the thermal camera to its lowest temperature range
					RMH_IRThermalCamera_SetIRCameraTemperatureRange(_ThermalCamera_TemperatureRange_LowRange, IRCamera.ThermalCameraSupportPool);

					// Update the temperature range button border color
					GlobalVariables::GlobalTempRangeButton->FlatAppearance->BorderColor = System::Drawing::Color::Yellow;

					// Update the temperature range button graphic
					GlobalVariables::GlobalTempRangeButton->Update();
					// Wait for the temperature range to switch correctly
					System::Threading::Thread::Sleep(_ThermalCameraPool3_RangeSwitchReadyTimeMs);

					// Update the IR camera device temperature range variable
					IRCamera.CurrentIRTempRangeFlag = 1;

					// Update the temperature range button border color
					GlobalVariables::GlobalTempRangeButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(255, 40, 40, 40);

				}

				// Write GUI status message
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Temperature Range Was Changed", _StatusMessageType_Normal);

				// Perform a thermal camera calibration
				RMH_IRThermalCamera_CalibrateThermalCamera();

			break;

		}

	}
	else { // If the connected thermal camera does NOT support a higher temperature range

		// Update the IR camera device temperature range variable
		IRCamera.CurrentIRTempRangeFlag = 1;

		// Reset the temperature range flag
		ThermalCameraHighRangeFlag = false;

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "The Connected Thermal Camera Does Not Support A Higher Temperature Range.", _StatusMessageType_Warning);

	}

}

void RMH_IRThermalCamera_ThermalCameraInitialConnectionEvents_Pool1() {

	// This routine handles the events, sequences and actions to be performed after connecting to a pool 1 thermal camera 

	// Local variables
	bool ConfigurationValuesOKFlag[6] = { false, false, false, false, false, false };

	// Set the frame pixel offset values of the thermal camera
	IRCamera.FrameWidthPixelOffset = _SupporteredeThermalCameraPool1_FrameWidthPixelOffset;
	IRCamera.FrameHeightPixelOffset = _SupporteredeThermalCameraPool1_FrameHeightPixelOffset;

	// Start the camera video capture
	RMH_IRThermalCamera_StartCapturing();

	// Reset the number of camera video frames read
	IRCamera.NumbOfCapturedFrames = 0;

	// Read camera frame data until the video feed is ready 
	// The loop ends when the camera reports valid internal temperature data
	while (1) {

		// Wait before reading each data frame
		System::Threading::Thread::Sleep(125);

		// Increment the number of camera video frames read
		IRCamera.NumbOfCapturedFrames = IRCamera.NumbOfCapturedFrames + 1;

		// Read a single frame of data from the thermal camera
		RMH_IRThermalCamera_ReadFrameRaw(&IRCameraFrameData[0], &VideoFrameSize);
		// Format the raw YUY2 data to a 16-bit thermal data array
		RMH_IRThermalCamera_ConvertYUY2To14BitThermalDataArray(&IRCamera, &IRCameraFrameData[0], &FrameThermalDataRaw[0]);
		// Read the frame metadata of the IR camera and calculate the internal IR sensor temperatures
		RMH_IRThermalCamera_ReadCalFrameMetaData(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

		// If the internal temperature data is other than 0
		if (IRCamera.temp_fpa_Raw != 0 && IRCamera.temp_shutter_Raw != 0) {

			// Check whether the core temperature is available
			if (IRCamera.temp_core_Raw == 0) {

				// Write GUI status message
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Does Not Support Core Temperature Measurements.", _StatusMessageType_Normal);

			}

			// Break the while loop
			break;

		}

		// Timeout handling - if the internal temperature data has not been read properly
		if (IRCamera.NumbOfCapturedFrames >= 25) {

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Initial Frame Meta Data Read Sequence Failed!", _StatusMessageType_Error);

			// Reset the number of camera video frames read
			IRCamera.NumbOfCapturedFrames = 0;

			// Set the camera connect error flag
			CameraConnectErrorFlag = true;

			// Break the while loop
			break;

		}

	}

	// Stop the camera video capture
	RMH_IRThermalCamera_StopCapturing();

	// Store the last calibration sensor detector temperature
	CurrentCalDetectorTemperature = IRCamera.temp_fpa;

	// Write GUI status message
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Reading The Thermal Camera Internal Calibration...", _StatusMessageType_Normal);
	// Read the internal calibration parameters of the IR camera
	RMH_IRThermalCamera_ReadCalibrationParameters(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

	// Write GUI status message
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Reading The Thermal Camera Temperature Configuration...", _StatusMessageType_Normal);
	// Read the internal configuration parameters of the camera
	RMH_IRThermalCamera_ReadCameraConfigParameters(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

	// ---------------------------------------------- Write Camera Temperature Configuration ---------------------------------------------- //

	// Write GUI status message - configuration parameters
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Device Pool: Pool " + RMH_Conversion_IntToStdString(IRCamera.ThermalCameraSupportPool), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Temperature Correction: " + RMH_Conversion_FloatToStdString(IRCamera.TemperatureCorrectionSetting, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Reflected Temperature: " + RMH_Conversion_FloatToStdString(IRCamera.ReflectedTemperatureSetting * TemperatureUnitScaleFactor + TemperatureUnitOffsetFactor, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Ambient Temperature " + RMH_Conversion_FloatToStdString(IRCamera.AmbientTemperatureSetting * TemperatureUnitScaleFactor + TemperatureUnitOffsetFactor, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Humidity: " + RMH_Conversion_FloatToStdString(IRCamera.HumiditySetting, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Emissivity: " + RMH_Conversion_FloatToStdString(IRCamera.EmissivitySetting, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Distance: " + RMH_Conversion_FloatToStdString(IRCamera.DistanceSetting, 5), _StatusMessageType_Normal);

	// ----------------------------------------------------------------------------------------------------------------------------------- //

	// Update the saved temperature correction value from the external file
	IRCamera.TemperatureCorrectionSetting = SavedTempCorrectionSetting;

	// Write the internal camera configuration parameters read to the camera configuration panel
	ConfigurationValuesOKFlag[0] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[0], IRCamera.TemperatureCorrectionSetting, 1, 0, _IRThermalCameraDefault_TemperatureCorrectionValue);
	ConfigurationValuesOKFlag[1] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[1], IRCamera.AmbientTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_AmbientTemperatureValue);
	ConfigurationValuesOKFlag[2] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[2], IRCamera.ReflectedTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_ReflectedTemperatureValue);
	ConfigurationValuesOKFlag[3] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[3], IRCamera.HumiditySetting, 1, 0, _IRThermalCameraDefault_SurroundingHumidityValue);
	ConfigurationValuesOKFlag[4] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[4], IRCamera.EmissivitySetting, 1, 0, _IRThermalCameraDefault_ObjectEmissivityValue);
	ConfigurationValuesOKFlag[5] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[5], IRCamera.DistanceSetting, 1, 0, _IRThermalCameraDefault_ObjectDistanceValue);

	// Check whether the configuration values were out of range
	if (ConfigurationValuesOKFlag[0] == false) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Temperature Correction Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
		// Update the associated configuration value to the default value
		IRCamera.TemperatureCorrectionSetting = _IRThermalCameraDefault_TemperatureCorrectionValue;

	}
	if (ConfigurationValuesOKFlag[1] == false) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Ambient Temperature Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
		// Update the associated configuration value to the default value
		IRCamera.AmbientTemperatureSetting = _IRThermalCameraDefault_AmbientTemperatureValue;

	}
	if (ConfigurationValuesOKFlag[2] == false) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Reflected Temperature Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
		// Update the associated configuration value to the default value
		IRCamera.ReflectedTemperatureSetting = _IRThermalCameraDefault_ReflectedTemperatureValue;

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

	// Write GUI status message
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Generating Temperature Look-Up Tabel...", _StatusMessageType_Normal);

	// Generate the temperature look-up table
	RMH_IRThermalCamera_GenerateThermoGrapicLookUpTable(&IRCamera, IRCamera.ThermalCameraSupportPool);

}

void RMH_IRThermalCamera_ThermalCameraInitialConnectionEvents_Pool2() {

	// This routine handles the events, sequences and actions to be performed after connecting to a pool 2 thermal camera 

	// Local variables
	bool ConfigurationValuesOKFlag[6] = { false, false, false, false, false, false };

	// Set the frame pixel offset values of the thermal camera
	IRCamera.FrameWidthPixelOffset = _SupporteredeThermalCameraPool2_FrameWidthPixelOffset;
	IRCamera.FrameHeightPixelOffset = _SupporteredeThermalCameraPool2_FrameHeightPixelOffset;

	// Write GUI status message
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Reading The Thermal Camera Temperature Configuration...", _StatusMessageType_Normal);
	// Read the internal configuration parameters of the camera
	RMH_IRThermalCamera_ReadCameraConfigParameters(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

	// ---------------------------------------------- Write Camera Temperature Configuration ---------------------------------------------- //

	// Write GUI status message - configuration parameters
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Device Pool: Pool " + RMH_Conversion_IntToStdString(IRCamera.ThermalCameraSupportPool), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Temperature Correction: " + RMH_Conversion_FloatToStdString(IRCamera.TemperatureCorrectionSetting, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Reflected Temperature: " + RMH_Conversion_FloatToStdString(IRCamera.ReflectedTemperatureSetting * TemperatureUnitScaleFactor + TemperatureUnitOffsetFactor, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Ambient Temperature " + RMH_Conversion_FloatToStdString(IRCamera.AmbientTemperatureSetting * TemperatureUnitScaleFactor + TemperatureUnitOffsetFactor, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Humidity: " + RMH_Conversion_FloatToStdString(IRCamera.HumiditySetting, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Emissivity: " + RMH_Conversion_FloatToStdString(IRCamera.EmissivitySetting, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Distance: " + RMH_Conversion_FloatToStdString(IRCamera.DistanceSetting, 5), _StatusMessageType_Normal);

	// ----------------------------------------------------------------------------------------------------------------------------------- //

	// Write the internal camera configuration parameters read to the camera configuration panel
	ConfigurationValuesOKFlag[0] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[0], IRCamera.TemperatureCorrectionSetting, 1, 0, _IRThermalCameraDefault_TemperatureCorrectionValue);
	ConfigurationValuesOKFlag[1] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[1], IRCamera.AmbientTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_AmbientTemperatureValue);
	ConfigurationValuesOKFlag[2] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[2], IRCamera.ReflectedTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_ReflectedTemperatureValue);
	ConfigurationValuesOKFlag[3] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[3], IRCamera.HumiditySetting, 1, 0, _IRThermalCameraDefault_SurroundingHumidityValue);
	ConfigurationValuesOKFlag[4] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[4], IRCamera.EmissivitySetting, 1, 0, _IRThermalCameraDefault_ObjectEmissivityValue);
	ConfigurationValuesOKFlag[5] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[5], IRCamera.DistanceSetting, 1, 0, _IRThermalCameraDefault_ObjectDistanceValue);

	// Check whether the configuration values were out of range
	if (ConfigurationValuesOKFlag[0] == false) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Temperature Correction Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
		// Update the associated configuration value to the default value
		IRCamera.TemperatureCorrectionSetting = _IRThermalCameraDefault_TemperatureCorrectionValue;

	}
	if (ConfigurationValuesOKFlag[1] == false) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Ambient Temperature Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
		// Update the associated configuration value to the default value
		IRCamera.AmbientTemperatureSetting = _IRThermalCameraDefault_AmbientTemperatureValue;

	}
	if (ConfigurationValuesOKFlag[2] == false) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Reflected Temperature Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
		// Update the associated configuration value to the default value
		IRCamera.ReflectedTemperatureSetting = _IRThermalCameraDefault_ReflectedTemperatureValue;

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

	// Read the frame metadata of the IR camera and calculate the internal IR sensor temperatures
	RMH_IRThermalCamera_ReadCalFrameMetaData(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

	// Generate the temperature look-up table
	RMH_IRThermalCamera_GenerateThermoGrapicLookUpTable(&IRCamera, IRCamera.ThermalCameraSupportPool);

}

void RMH_IRThermalCamera_ThermalCameraInitialConnectionEvents_Pool3() {

	// This routine handles the events, sequences and actions to be performed after connecting to a pool 3 thermal camera 

	// Local variables
	bool ConfigurationValuesOKFlag[6] = { false, false, false, false, false, false };
	unsigned int CenterPixelIndex = ((IRCamera.FrameHeight - IRCamera.FrameMetadataSize) * 0.5) * IRCamera.FrameWidth + (IRCamera.FrameWidth * 0.5);

	// Set the frame pixel offset values of the thermal camera
	IRCamera.FrameWidthPixelOffset = _SupporteredeThermalCameraPool3_FrameWidthPixelOffset;
	IRCamera.FrameHeightPixelOffset = _SupporteredeThermalCameraPool3_FrameHeightPixelOffset;

	// Start the camera video capture
	RMH_IRThermalCamera_StartCapturing();

	// Wait for the thermal camera to be ready
	System::Threading::Thread::Sleep(_ThermalCameraPool3_ReadyTimeMs);

	// Perform a thermal camera calibration
	RMH_IRThermalCamera_CalibrateThermalCamera();

	// Stop the camera video capture
	RMH_IRThermalCamera_StopCapturing();

	// Reset the number of video frames read variable
	IRCamera.NumbOfCapturedFrames = 0;

	// Write GUI status message
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Reading The Thermal Camera Internal Calibration...", _StatusMessageType_Normal);
	// Read the internal calibration parameters of the IR camera
	RMH_IRThermalCamera_ReadCalibrationParameters(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

	// Write GUI status message
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Reading The Thermal Camera Temperature Configuration...", _StatusMessageType_Normal);
	// Read the internal configuration parameters of the camera
	RMH_IRThermalCamera_ReadCameraConfigParameters(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

	// ---------------------------------------------- Write Camera Temperature Configuration ---------------------------------------------- //

	// Write GUI status message - configuration parameters
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Device Pool: Pool " + RMH_Conversion_IntToStdString(IRCamera.ThermalCameraSupportPool), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Temperature Correction: " + RMH_Conversion_FloatToStdString(IRCamera.TemperatureCorrectionSetting, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Reflected Temperature: " + RMH_Conversion_FloatToStdString(IRCamera.ReflectedTemperatureSetting * TemperatureUnitScaleFactor + TemperatureUnitOffsetFactor, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Ambient Temperature " + RMH_Conversion_FloatToStdString(IRCamera.AmbientTemperatureSetting * TemperatureUnitScaleFactor + TemperatureUnitOffsetFactor, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Humidity: " + RMH_Conversion_FloatToStdString(IRCamera.HumiditySetting, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Emissivity: " + RMH_Conversion_FloatToStdString(IRCamera.EmissivitySetting, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Distance: " + RMH_Conversion_FloatToStdString(IRCamera.DistanceSetting, 5), _StatusMessageType_Normal);

	// ----------------------------------------------------------------------------------------------------------------------------------- //

	// Update the saved temperature correction value from the external file
	IRCamera.TemperatureCorrectionSetting = SavedTempCorrectionSetting;

	// Write the internal camera configuration parameters read to the camera configuration panel
	ConfigurationValuesOKFlag[0] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[0], IRCamera.TemperatureCorrectionSetting, 1, 0, _IRThermalCameraDefault_TemperatureCorrectionValue);
	ConfigurationValuesOKFlag[1] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[1], IRCamera.AmbientTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_AmbientTemperatureValue);
	ConfigurationValuesOKFlag[2] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[2], IRCamera.ReflectedTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_ReflectedTemperatureValue);
	ConfigurationValuesOKFlag[3] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[3], IRCamera.HumiditySetting, 1, 0, _IRThermalCameraDefault_SurroundingHumidityValue);
	ConfigurationValuesOKFlag[4] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[4], IRCamera.EmissivitySetting, 1, 0, _IRThermalCameraDefault_ObjectEmissivityValue);
	ConfigurationValuesOKFlag[5] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[5], IRCamera.DistanceSetting, 1, 0, _IRThermalCameraDefault_ObjectDistanceValue);

	// Check whether the configuration values were out of range
	if (ConfigurationValuesOKFlag[0] == false) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Temperature Correction Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
		// Update the associated configuration value to the default value
		IRCamera.TemperatureCorrectionSetting = _IRThermalCameraDefault_TemperatureCorrectionValue;

	}
	if (ConfigurationValuesOKFlag[1] == false) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Ambient Temperature Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
		// Update the associated configuration value to the default value
		IRCamera.AmbientTemperatureSetting = _IRThermalCameraDefault_AmbientTemperatureValue;

	}
	if (ConfigurationValuesOKFlag[2] == false) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Reflected Temperature Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
		// Update the associated configuration value to the default value
		IRCamera.ReflectedTemperatureSetting = _IRThermalCameraDefault_ReflectedTemperatureValue;

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

	// Write GUI status message
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Generating Temperature Look-Up Tabel...", _StatusMessageType_Normal);

	// Generate the temperature look-up table
	RMH_IRThermalCamera_GenerateThermoGrapicLookUpTable(&IRCamera, IRCamera.ThermalCameraSupportPool);

}

void RMH_IRThermalCamera_ThermalCameraInitialConnectionEvents_Pool4() {

	// This routine handles the events, sequences and actions to be performed after connecting to a pool 4 thermal camera 

	// Local variables
	bool ConfigurationValuesOKFlag[6] = { false, false, false, false, false, false };

	// Set the frame pixel offset values of the thermal camera
	IRCamera.FrameWidthPixelOffset = _SupporteredeThermalCameraPool4_FrameWidthPixelOffset;
	IRCamera.FrameHeightPixelOffset = _SupporteredeThermalCameraPool4_FrameHeightPixelOffset;

	// Pool 5 (Thermal Master P3): the first 6 pixels of the row where display starts read as a
	// fixed near-zero value (a small dead/blanking region, not real image content) - skip them.
	// A handful of words at the very end of the frame become unused padding as a result, which is
	// far less noticeable than 6 dead pixels at the start of the image.
	if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_5) {
		IRCamera.FrameWidthPixelOffset = 6;
	}

	// Pool 6 (Thermal Master THOR001): the raw sample starts with a fixed 10 byte (5 pixel) sync
	// header ("ff 00" x5) before the real pixel data - skip it.
	if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_6) {
		IRCamera.FrameWidthPixelOffset = _SupporteredeThermalCameraPool6_FrameWidthPixelOffset;
		IRCamera.FrameHeightPixelOffset = _SupporteredeThermalCameraPool6_FrameHeightPixelOffset;
	}

	// Write GUI status message
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Reading The Thermal Camera Temperature Configuration...", _StatusMessageType_Normal);
	// Read the internal configuration parameters of the camera
	RMH_IRThermalCamera_ReadCameraConfigParameters(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

	// ---------------------------------------------- Write Camera Temperature Configuration ---------------------------------------------- //

	// Write GUI status message - configuration parameters
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Device Pool: Pool " + RMH_Conversion_IntToStdString(IRCamera.ThermalCameraSupportPool), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Temperature Correction: " + RMH_Conversion_FloatToStdString(IRCamera.TemperatureCorrectionSetting, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Reflected Temperature: " + RMH_Conversion_FloatToStdString(IRCamera.ReflectedTemperatureSetting * TemperatureUnitScaleFactor + TemperatureUnitOffsetFactor, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Ambient Temperature " + RMH_Conversion_FloatToStdString(IRCamera.AmbientTemperatureSetting * TemperatureUnitScaleFactor + TemperatureUnitOffsetFactor, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Humidity: " + RMH_Conversion_FloatToStdString(IRCamera.HumiditySetting, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Emissivity: " + RMH_Conversion_FloatToStdString(IRCamera.EmissivitySetting, 5), _StatusMessageType_Normal);
	RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Distance: " + RMH_Conversion_FloatToStdString(IRCamera.DistanceSetting, 5), _StatusMessageType_Normal);

	// ----------------------------------------------------------------------------------------------------------------------------------- //

	// Write the internal camera configuration parameters read to the camera configuration panel
	ConfigurationValuesOKFlag[0] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[0], IRCamera.TemperatureCorrectionSetting, 1, 0, _IRThermalCameraDefault_TemperatureCorrectionValue);
	ConfigurationValuesOKFlag[1] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[1], IRCamera.AmbientTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_AmbientTemperatureValue);
	ConfigurationValuesOKFlag[2] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[2], IRCamera.ReflectedTemperatureSetting, TemperatureUnitScaleFactor, TemperatureUnitOffsetFactor, _IRThermalCameraDefault_ReflectedTemperatureValue);
	ConfigurationValuesOKFlag[3] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[3], IRCamera.HumiditySetting, 1, 0, _IRThermalCameraDefault_SurroundingHumidityValue);
	ConfigurationValuesOKFlag[4] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[4], IRCamera.EmissivitySetting, 1, 0, _IRThermalCameraDefault_ObjectEmissivityValue);
	ConfigurationValuesOKFlag[5] = RMH_Winforms_NumericUpDown_ChangeNumber(GlobalVariables::CameraConfigNumericUpDowns[5], IRCamera.DistanceSetting, 1, 0, _IRThermalCameraDefault_ObjectDistanceValue);

	// Check whether the configuration values were out of range
	if (ConfigurationValuesOKFlag[0] == false) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Temperature Correction Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
		// Update the associated configuration value to the default value
		IRCamera.TemperatureCorrectionSetting = _IRThermalCameraDefault_TemperatureCorrectionValue;

	}
	if (ConfigurationValuesOKFlag[1] == false) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Ambient Temperature Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
		// Update the associated configuration value to the default value
		IRCamera.AmbientTemperatureSetting = _IRThermalCameraDefault_AmbientTemperatureValue;

	}
	if (ConfigurationValuesOKFlag[2] == false) {

		// Write GUI status message
		RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Reflected Temperature Setting Was Out Of Range - Using Default Value.", _StatusMessageType_Error);
		// Update the associated configuration value to the default value
		IRCamera.ReflectedTemperatureSetting = _IRThermalCameraDefault_ReflectedTemperatureValue;

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

	// Read the frame metadata of the IR camera and calculate the internal IR sensor temperatures
	RMH_IRThermalCamera_ReadCalFrameMetaData(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

	// Generate the temperature look-up table
	RMH_IRThermalCamera_GenerateThermoGrapicLookUpTable(&IRCamera, IRCamera.ThermalCameraSupportPool);

}

void RMH_IRThermalCamera_ConnectToThermalCameraOrAnalysisMode() {

	// This routine handles the events when connecting to a selected thermal camera pool
	// or the events if "Recording Analysis" mode is selected

	// Clear the status message area
	GlobalVariables::GlobalGUIInfoTextArea->Clear();

	// Reset the CMOS non-uniformity mapping data array when connecting or changing mode
	RMH_ImageNonUniformityCorrection_ZeroNonUniformityMapArrayData(&ImageCMOSNonUniformityMapData[0], IRCamera.FrameWidth, IRCamera.FrameHeight);

	/*
	// Check whether the application has a genuine license installed
	RMH_Application_HandleOnlinePeriodicLicenseCheck();

	// Check the application feature status
	if (CheckApplicationWindowsStatus == 3567482) {

		// Check whether the trial period of the application has expired
		if (TrialPeriodExpiredFlag == true) {

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Your Trial Period Has Expired!, Please Extend The Period Or Buy A License.", _StatusMessageType_Warning);

			// Do not execute the rest of the routine
			return;

		}

	}
	*/

	// Is "Snapshot Analysis" mode selected
	if (GlobalVariables::GlobalCameraSourceDropList->SelectedIndex == _SnapShotAnalysisMode) {

		// Check whether a snapshot file, or "Snapshot Analysis" mode, is already active and open
		if (InSnapShotAnalysisModeFlag == true || SnapShotAnalysisModeFileInfo.IsFileReady == true) {

			// Handle the events and actions for resetting "Snapshot Analysis" mode
			RMH_ThermalViewer_HandleSellectedDeviceOrModeChange();

		}

		// ----------------------------------------------------------------------------------------------------------------------------------------------- // 
		//													   Read RAW Snapshot File For Analysis													           // 
		// ----------------------------------------------------------------------------------------------------------------------------------------------- // 

		// Open the "Open File" dialog to select the RAW file
		GlobalVariables::SnapShotAnalysisModeRAWFilePath = RMH_Winforms_GetOpenFileDialogDirectory();

		// Read the information parameters of the selected snapshot file 
		SnapShotAnalysisModeFileInfo = RMH_AnalysisMode_ReadAndLoadPNGImage(GlobalVariables::SnapShotAnalysisModeRAWFilePath, &IRCameraFrameData[0], MaximumFrameDataArraySize);

		// Were any errors registered while reading the snapshot file 
		if (SnapShotAnalysisModeFileInfo.FileErrorFlag == false) {

			// Is the snapshot file read a .png file
			if (SnapShotAnalysisModeFileInfo.IsPNGFileFlag == true) {

				// Write GUI status message - the file is a .png file
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "File Extension Is: .png.", _StatusMessageType_Normal);

				// Check the frame width of the snapshot file read
				if (SnapShotAnalysisModeFileInfo.FrameWidth > 0) {

					// Write GUI status message - frame width of the file
					RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "File Frame Width: " + RMH_Conversion_IntToStdString(SnapShotAnalysisModeFileInfo.FrameWidth) + " Pixels", _StatusMessageType_Normal);

					// Check the frame height of the snapshot file read
					if (SnapShotAnalysisModeFileInfo.FrameHeight > 0) {

						// Write GUI status message - frame height of the file
						RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "File Frame Height: " + RMH_Conversion_IntToStdString(SnapShotAnalysisModeFileInfo.FrameHeight) + " Pixels", _StatusMessageType_Normal);

						// Check whether the file is ready 
						if (SnapShotAnalysisModeFileInfo.IsFileReady == true) {

							// ----------------------------------------------------------------------------------------------------------------------------------------------- // 
							//                                            Read The Snapshot File Metadata And Check The RAW ID String                                            // 
							// ----------------------------------------------------------------------------------------------------------------------------------------------- //

							// Configure and set the camera parameters from the video data information read
							IRCamera.FrameWidth = SnapShotAnalysisModeFileInfo.FrameWidth;
							IRCamera.FrameHeight = SnapShotAnalysisModeFileInfo.FrameHeight;
							IRCamera.FrameRate = 1; // Frame Rate Er 1 i SnapShot Analysis Mode

							// Read the identification and metadata of the snapshot file
							SnapShotAnalysisModeFileMetaData = RMH_AnalysisMode_ReadRAWMetaData(IRCamera.FrameWidth, IRCamera.FrameHeight, &IRCameraFrameData[0]);

							// Reset the RAW file ID string match flag
							RAWFileIDStringMatchFlag = false;

							// Check whether the file is a RAW recording from the IRCAM Thermal Viewer software - check the metadata string
							for (unsigned int i = 0; i < _RAWRecordingFileMetaDataIndex_IDStringStop; i++) {

								// Check whether the RAW identification string of the file matches the expected one
								if (SnapShotAnalysisModeFileMetaData.RAWIDCharData[i] == _RAWFileIDData_FileIDString[i]) {

									// Update the RAW file ID string match flag - all characters must match
									RAWFileIDStringMatchFlag = true;

								}
								else {

									// Reset the RAW file ID string match flag - the ID string is not a match
									RAWFileIDStringMatchFlag = false;
									// Break the for loop
									break;

								}

							}

							// ----------------------------------------------------------------------------------------------------------------------------------------------- // 
							//                                    Check Whether The Metadata Of The Opened File Contains The Correct ID String                                        // 
							// ----------------------------------------------------------------------------------------------------------------------------------------------- //

							// Is the metadata of the snapshot file consistent with the frame (the values come from the file)
							if (RAWFileIDStringMatchFlag == true && RMH_AnalysisMode_IsRAWMetaDataValid(&SnapShotAnalysisModeFileMetaData, IRCamera.FrameWidth, IRCamera.FrameHeight) == false) {

								// Write GUI status message - the metadata is invalid
								RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Error: The Snapshot File Metadata Is Invalid Or Corrupt.", _StatusMessageType_Error);
								// Reset the RAW file ID string match flag - the file can not be used
								RAWFileIDStringMatchFlag = false;

							}

							// Was the ID string of the snapshot file a match
							if (RAWFileIDStringMatchFlag == true) {

								// Configure and set the camera parameters from the snapshot metadata read
								IRCamera.FrameMetadataSize = SnapShotAnalysisModeFileMetaData.FileMetaDataSizeID;
								IRCamera.FrameWidthPixelOffset = SnapShotAnalysisModeFileMetaData.FileFrameWidthPixelOffsetID;
								IRCamera.FrameHeightPixelOffset = SnapShotAnalysisModeFileMetaData.FileFrameHeightPixelOffsetID;
								IRCamera.ThermalCameraSupportPool = SnapShotAnalysisModeFileMetaData.CameraPoolID;
								IRCamera.TemperatureCorrectionSetting = SnapShotAnalysisModeFileMetaData.RecordingTempCorrectionSetting;
								IRCamera.AmbientTemperatureSetting = SnapShotAnalysisModeFileMetaData.RecordingAmbientTempSetting;
								IRCamera.ReflectedTemperatureSetting = SnapShotAnalysisModeFileMetaData.RecordingReflectedTempSetting;
								IRCamera.HumiditySetting = SnapShotAnalysisModeFileMetaData.RecordingHumiditySetting;
								IRCamera.EmissivitySetting = SnapShotAnalysisModeFileMetaData.RecordingEmissivitySetting;
								IRCamera.DistanceSetting = SnapShotAnalysisModeFileMetaData.RecordingDistanceSetting;

								// Pool 5 (Thermal Master P3) snapshots are saved as 3 bytes/pixel (R = low byte, G = high
								// byte, B unused - see RMH_IRThermalCamera_ConvertCapturedRawImageDataToSnapshotPNG()'s
								// pool 5 case), since that is what RMH_AnalysisMode_ReadAndLoadPNGImage()/
								// RMH_AnalysisMode_ReadRAWMetaData() above require - but
								// RMH_IRThermalCamera_ConvertYUY2To14BitThermalDataArray() below expects a tightly-packed
								// 2-byte/pixel buffer (P3's actual live wire format). De-interleave in place
								// (R,G,B,R,G,B... -> R,G,R,G...) before calling it - safe in place since the write index
								// never overtakes the read index.
								if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_5) {

									unsigned int Pool5PixelCount = IRCamera.FrameWidth * IRCamera.FrameHeight;
									for (unsigned int i = 0; i < Pool5PixelCount; i++) {

										IRCameraFrameData[i * 2 + 0] = IRCameraFrameData[i * 3 + 0];
										IRCameraFrameData[i * 2 + 1] = IRCameraFrameData[i * 3 + 1];

									}

								}

								// Read the operating constants of the IR camera - relative to the supported pool
								RMH_IRThermalCamera_InitIRCameraConstants(&IRCamera, IRCamera.ThermalCameraSupportPool);

								// Format the raw YUY2 data, from the video file, to a 16-bit thermal data array
								RMH_IRThermalCamera_ConvertYUY2To14BitThermalDataArray(&IRCamera, &IRCameraFrameData[0], &FrameThermalDataRaw[0]);
								// Read the frame metadata of the IR camera and calculate the internal IR sensor temperatures
								RMH_IRThermalCamera_ReadCalFrameMetaData(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

								// Reset the number of video frames read variable
								IRCamera.NumbOfCapturedFrames = 0;

								// Read the calibration parameters of the video file camera data
								RMH_IRThermalCamera_ReadCalibrationParameters(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);
								// Generate the camera temperature look-up table
								RMH_IRThermalCamera_GenerateThermoGrapicLookUpTable(&IRCamera, IRCamera.ThermalCameraSupportPool);

								// Read and show the internal camera configuration parameters read - from the snapshot file metadata
								RMH_ThermalViewer_ReadAndDisplayRAWSnapShotFileCameraConfigParameters();

								// ------------------------------------------------- Update GUI Elements & Components ------------------------------------------------- //

								// Update the connect button label text
								GlobalVariables::GlobalConnectButton->Text = "File is Open\r\nAnd Ready";
								// Update the connect button border color 
								GlobalVariables::GlobalConnectButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

								// ----------------------------------- Initialization Of The OpenGL Rendering Texture For The Live Video Stream ----------------------------------- //

								// Configure the live view stream OpenGL texture rendering resolution 
								GlobalVariables::OpenGLRender->RMH_OpenGL_InitImageTexture(IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize);
								// Configure the live view OpenGL zoom texture rendering resolution 
								GlobalVariables::LiveViewZoomWindowRender->RMH_OpenGL_InitLiveViewZoomWindow(IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize);

								// Pool 5 (Thermal Master P3) sensor is mounted rotated 90 degrees - correct it automatically
								if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_5) {
									while (GlobalVariables::OpenGLRender->RMH_LiveView_GetRotation() != 0.0) { GlobalVariables::OpenGLRender->RMH_OpenGL_RotateLiveViewCCW(); }
								}

								// Toggle/update the live view enhanced image resolution mode
								RMH_ThermalViewer_ToggleEnhancedLiveViewResolution();
								// Toggle/update the live view ultra image resolution mode
								RMH_ThermalViewer_ToggleLiveViewUltraResolution();

								// --------------------------------- Start Reading The Video File, Processing Thread & Main Update Timer -------------------------------- //

								// Update the camera "file is connected" flag
								IRCamera.ConnectedFlag = true;

								// Start the asynchronous thread operation
								GlobalVariables::GlobalVideoStreamThread->RunWorkerAsync();
								GlobalVariables::GlobalSecondaryProcessingThread->RunWorkerAsync();

								// Enable the GUI update timer
								GlobalVariables::GlobalMainGUIUpdateTimer->Enabled = true;
								GlobalVariables::GlobalMainGUIUpdateTimer->Start();

								// Update the camera "isStreaming" status flag
								IRCamera.isStreaming = true;

								// ----------------------------------------------------------------------------------------------------------------------------------------------- // 
								//                                    The File Has Been Checked And Is Ready For Reading. "Snapshot Analysis" Is Active                                     // 
								// ----------------------------------------------------------------------------------------------------------------------------------------------- //

								// Update the "in Snapshot Analysis mode" flag
								InSnapShotAnalysisModeFlag = true;

								// Write GUI status message - the file is ready
								RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "SnapShot Analysis Is Now Ready - Open The Live View Stream To Analyse Your Snapshot.", _StatusMessageType_Success);

								// Enable the application features
								RMH_Application_EnableApplicationFeatures();

							}
							else {

								// Write GUI status message - the video file is not a RAW.avi file
								RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "The Selected File Is Not A RAW.png File. Please Select A Correct RAW.png File!", _StatusMessageType_Error);

							}

						}
						else {

							// Write GUI status message - snapshot file read error
							RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "An Error Occured While Reading The File, Please Try Another File", _StatusMessageType_Error);

							// Reset the "in Snapshot Analysis mode" flag
							InSnapShotAnalysisModeFlag = false;

						}

					}
					else {

						// Write GUI status message - file frame height error
						RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Selected File Error: File Has '0' Pixel Height!", _StatusMessageType_Error);

						// Reset the "in Snapshot Analysis mode" flag
						InSnapShotAnalysisModeFlag = false;

					}

				}
				else {

					// Write GUI status message - file frame width error
					RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Selected File Error: File Has '0' Pixel Width!", _StatusMessageType_Error);

					// Reset the "in Snapshot Analysis mode" flag
					InSnapShotAnalysisModeFlag = false;

				}

			}
			else {

				// Write GUI status message - the file is not a .png file
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Selected File Is Not An .png File!", _StatusMessageType_Error);

				// Reset the "in Snapshot Analysis mode" flag
				InSnapShotAnalysisModeFlag = false;

			}

		}
		else {

			// Write GUI status message - snapshot file read error
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Could Not Read The Selected SnapShot File, Please Check If The Correct File Was Selected!", _StatusMessageType_Error);

			// Reset the "in Snapshot Analysis mode" flag
			InSnapShotAnalysisModeFlag = false;

		}

		// If "Snapshot Analysis" mode has not become active
		if (InSnapShotAnalysisModeFlag == false) {

			// Handle the events and actions for resetting "Snapshot Analysis" mode
			RMH_ThermalViewer_HandleSellectedDeviceOrModeChange();

			// Write GUI status message - the file has not been opened
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Selected SnapShot File Could Not Be Opened, Please Check The Selected File Or Choose Another File.", _StatusMessageType_Error);

		}

	}
	else if (GlobalVariables::GlobalCameraSourceDropList->SelectedIndex == _RecordingAnalysisMode) { // Is "Recording Analysis" mode selected

		// Check whether a video file, or "Recording Analysis" mode, is already active and open
		if (InRecordingAnalysisModeFlag == true || RecordingAnalysisModeFileInfo.IsFileOpenFlag == true) {

			// Handle the events and actions for resetting "Recording Analysis" mode
			RMH_ThermalViewer_HandleSellectedDeviceOrModeChange();

		}

		// ----------------------------------------------------------------------------------------------------------------------------------------------- // 
		//													   Read RAW Video File For Analysis													           // 
		// ----------------------------------------------------------------------------------------------------------------------------------------------- // 

		// Open the "Open File" dialog to select the RAW file
		GlobalVariables::RecordingAnalysisModeRAWFilePath = RMH_Winforms_GetOpenFileDialogDirectory();

		// Read the information parameters of the selected video file 
		RecordingAnalysisModeFileInfo = RMH_VideoFileReading_SetupRecordingAnalysisModeVideoFileReader(GlobalVariables::RecordingAnalysisModeRAWFilePath);

		// ----------------------------------------------------------------------------------------------------------------------------------------------- // 
		//                         Check Whether The Video File Is The Correct RAW File And That The File Was Read Correctly And Is Ready                         // 
		// ----------------------------------------------------------------------------------------------------------------------------------------------- //

		//  Check whether the video file was opened correctly
		if (RecordingAnalysisModeFileInfo.IsFileOpenFlag == true) {

			// Write GUI status message - the file has been opened
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Selected Video File Is Open.", _StatusMessageType_Normal);

			// Check whether the file read is an ".avi" file
			if (RecordingAnalysisModeFileInfo.IsAVIFileFlag == true) {

				// Write GUI status message - the file is an .avi file
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "File Extension Is: .avi.", _StatusMessageType_Normal);

				// Check the frame width of the file read
				if (RecordingAnalysisModeFileInfo.FrameWidth > 0 && RecordingAnalysisModeFileInfo.FrameWidth <= _MaximumSupportedFrameDimension) {

					// Write GUI status message - frame width of the file
					RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "File Frame Width: " + RMH_Conversion_IntToStdString(RecordingAnalysisModeFileInfo.FrameWidth) + " Pixels", _StatusMessageType_Normal);

					// Check the frame height of the file read (the whole frame must fit the frame buffers)
					if (RMH_FrameBuffer_IsFrameSizeSupported(RecordingAnalysisModeFileInfo.FrameWidth, RecordingAnalysisModeFileInfo.FrameHeight) == true) {

						// Write GUI status message - frame height of the file
						RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "File Frame Height: " + RMH_Conversion_IntToStdString(RecordingAnalysisModeFileInfo.FrameHeight) + " Pixels", _StatusMessageType_Normal);

						// Check the number of frames of the file read
						if (RecordingAnalysisModeFileInfo.NumberOfFrames > 0) {

							// Write GUI status message - number of frames in the file
							RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "File Number Of Frames: " + RMH_Conversion_IntToStdString(RecordingAnalysisModeFileInfo.NumberOfFrames) + " Frames", _StatusMessageType_Normal);

							// Check the frame rate of the file read
							if (RecordingAnalysisModeFileInfo.FrameRate > 0) {

								// Write GUI status message - frame rate of the file
								RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "File Frame Rate: " + RMH_Conversion_IntToStdString(RecordingAnalysisModeFileInfo.FrameRate) + " FPS", _StatusMessageType_Normal);

								// Check the duration of the file read in seconds
								if (RecordingAnalysisModeFileInfo.DurationTime > 0.0) {

									// Write GUI status message - duration of the file in seconds
									RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "File Duration: " + RMH_Conversion_FloatToStdString(RecordingAnalysisModeFileInfo.DurationTime, 5) + " Sec", _StatusMessageType_Normal);

									// ----------------------------------------------------------------------------------------------------------------------------------------------- // 
									//                                             Read The Video File Metadata And Check The RAW ID String                                              // 
									// ----------------------------------------------------------------------------------------------------------------------------------------------- //

									// Configure and set the camera parameters from the video data information read
									IRCamera.FrameWidth = RecordingAnalysisModeFileInfo.FrameWidth;
									IRCamera.FrameHeight = RecordingAnalysisModeFileInfo.FrameHeight;
									IRCamera.FrameRate = RecordingAnalysisModeFileInfo.FrameRate;

									// Read the first frame of data from the opened video file
									unsigned int FirstFrameReadStatus = RMH_VideoFileReading_ReadVideoFileFrame(1, RecordingAnalysisModeFileInfo.NumberOfFrames, &IRCameraFrameData[0], MaximumFrameDataArraySize);

									// Read the identification and metadata of the video file
									RecordingAnalysisModeFileMetaData = RMH_AnalysisMode_ReadRAWMetaData(IRCamera.FrameWidth, IRCamera.FrameHeight, &IRCameraFrameData[0]);

									// Reset the RAW file ID string match flag
									RAWFileIDStringMatchFlag = false;

									// Check whether the file is a RAW recording from the IRCAM Thermal Viewer software - check the metadata string
									for (unsigned int i = 0; i < _RAWRecordingFileMetaDataIndex_IDStringStop; i++) {

										// Check whether the RAW identification string of the file matches the expected one
										if (RecordingAnalysisModeFileMetaData.RAWIDCharData[i] == _RAWFileIDData_FileIDString[i]) {

											// Update the RAW file ID string match flag - all characters must match
											RAWFileIDStringMatchFlag = true;

										}
										else {

											// Reset the RAW file ID string match flag - the ID string is not a match
											RAWFileIDStringMatchFlag = false;
											// Break the for loop
											break;

										}

									}

									// ----------------------------------------------------------------------------------------------------------------------------------------------- // 
									//                                    Check Whether The Metadata Of The Opened File Contains The Correct ID String                                        // 
									// ----------------------------------------------------------------------------------------------------------------------------------------------- //

									// Was the first frame read, and is the metadata consistent with the frame (the values come from the file)
									if (RAWFileIDStringMatchFlag == true && (FirstFrameReadStatus != _ReadAVIFile_StatusCode_FrameReadOK || RMH_AnalysisMode_IsRAWMetaDataValid(&RecordingAnalysisModeFileMetaData, IRCamera.FrameWidth, IRCamera.FrameHeight) == false)) {

										// Write GUI status message - the file can not be used
										RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Error: The Video File Frame Or Metadata Is Invalid Or Corrupt.", _StatusMessageType_Error);
										// Reset the RAW file ID string match flag - the file can not be used
										RAWFileIDStringMatchFlag = false;

									}

									// Was the ID string of the video file a match
									if (RAWFileIDStringMatchFlag == true) {

										// ----------------------------------------------------------------------------------------------------------------------------------------------- // 
										//                                                 Configure And Start "Recording Analysis Mode"                                                 // 
										// ----------------------------------------------------------------------------------------------------------------------------------------------- //

										// Configure and set the camera parameters from the video data metadata read
										IRCamera.FrameMetadataSize = RecordingAnalysisModeFileMetaData.FileMetaDataSizeID;
										IRCamera.FrameWidthPixelOffset = RecordingAnalysisModeFileMetaData.FileFrameWidthPixelOffsetID;
										IRCamera.FrameHeightPixelOffset = RecordingAnalysisModeFileMetaData.FileFrameHeightPixelOffsetID;
										IRCamera.ThermalCameraSupportPool = RecordingAnalysisModeFileMetaData.CameraPoolID;
										IRCamera.TemperatureCorrectionSetting = RecordingAnalysisModeFileMetaData.RecordingTempCorrectionSetting;
										IRCamera.AmbientTemperatureSetting = RecordingAnalysisModeFileMetaData.RecordingAmbientTempSetting;
										IRCamera.ReflectedTemperatureSetting = RecordingAnalysisModeFileMetaData.RecordingReflectedTempSetting;
										IRCamera.HumiditySetting = RecordingAnalysisModeFileMetaData.RecordingHumiditySetting;
										IRCamera.EmissivitySetting = RecordingAnalysisModeFileMetaData.RecordingEmissivitySetting;
										IRCamera.DistanceSetting = RecordingAnalysisModeFileMetaData.RecordingDistanceSetting;

										// Read the operating constants of the IR camera - relative to the supported pool
										RMH_IRThermalCamera_InitIRCameraConstants(&IRCamera, IRCamera.ThermalCameraSupportPool);

										// Format the raw YUY2 data, from the video file, to a 16-bit thermal data array
										RMH_IRThermalCamera_ConvertYUY2To14BitThermalDataArray(&IRCamera, &IRCameraFrameData[0], &FrameThermalDataRaw[0]);
										// Read the frame metadata of the IR camera and calculate the internal IR sensor temperatures
										RMH_IRThermalCamera_ReadCalFrameMetaData(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);

										// Reset the number of video frames read variable
										IRCamera.NumbOfCapturedFrames = 0;

										// Read the calibration parameters of the video file camera data
										RMH_IRThermalCamera_ReadCalibrationParameters(&FrameThermalDataRaw[0], &IRCamera, IRCamera.ThermalCameraSupportPool);
										// Generate the camera temperature look-up table
										RMH_IRThermalCamera_GenerateThermoGrapicLookUpTable(&IRCamera, IRCamera.ThermalCameraSupportPool);

										// Read and show the internal camera configuration parameters read - from the video file metadata
										RMH_ThermalViewer_ReadAndDisplayRAWVideoFileCameraConfigParameters();

										// ------------------------------------------------- Update GUI Elements & Components ------------------------------------------------- //

										// Update the connect button label text
										GlobalVariables::GlobalConnectButton->Text = "File is Open\r\nAnd Ready";
										// Update the connect button border color 
										GlobalVariables::GlobalConnectButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

										// ----------------------------------- Initialization Of The OpenGL Rendering Texture For The Live Video Stream ----------------------------------- //

										// Configure the live view stream OpenGL texture rendering resolution 
										GlobalVariables::OpenGLRender->RMH_OpenGL_InitImageTexture(IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize);
										// Configure the live view OpenGL zoom texture rendering resolution 
										GlobalVariables::LiveViewZoomWindowRender->RMH_OpenGL_InitLiveViewZoomWindow(IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize);

										// Pool 5 (Thermal Master P3) sensor is mounted rotated 90 degrees - correct it automatically
										if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_5) {
											while (GlobalVariables::OpenGLRender->RMH_LiveView_GetRotation() != 0.0) { GlobalVariables::OpenGLRender->RMH_OpenGL_RotateLiveViewCCW(); }
										}

										// Toggle/update the live view enhanced image resolution mode
										RMH_ThermalViewer_ToggleEnhancedLiveViewResolution();
										// Toggle/update the live view ultra image resolution mode
										RMH_ThermalViewer_ToggleLiveViewUltraResolution();

										// --------------------------------- Start Reading The Video File, Processing Thread & Main Update Timer -------------------------------- //

										// Update the camera "file is connected" flag
										IRCamera.ConnectedFlag = true;

										// Start the asynchronous thread operation
										GlobalVariables::GlobalVideoStreamThread->RunWorkerAsync();
										GlobalVariables::GlobalSecondaryProcessingThread->RunWorkerAsync();

										// Enable the GUI update timer
										GlobalVariables::GlobalMainGUIUpdateTimer->Enabled = true;
										GlobalVariables::GlobalMainGUIUpdateTimer->Start();

										// Update the camera "isStreaming" status flag
										IRCamera.isStreaming = true;

										// ----------------------------------------------------------------------------------------------------------------------------------------------- // 
										//                                    The File Has Been Checked And Is Ready For Reading. "Recording Analysis" Is Active                                    // 
										// ----------------------------------------------------------------------------------------------------------------------------------------------- //

										// Open the video playback controls panel form
										OpenVideoPlayBackControlsFormFlag = true;

										// Update the "in Recording Analysis mode" flag
										InRecordingAnalysisModeFlag = true;

										// Enable the record button in the live view tools panel
										GlobalVariables::GlobalRecordingButton->Enabled = true;

										// Write GUI status message - the file is ready
										RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Recording Analysis Is Now Ready.", _StatusMessageType_Success);

										// Enable the application features
										RMH_Application_EnableApplicationFeatures();

									}
									else {

										// Write GUI status message - the video file is not a RAW.avi file
										RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "The Selected File Is Not A RAW.avi File. Please Select A Correct RAW.avi File!", _StatusMessageType_Error);

									}

								}
								else {

									// Write GUI status message - file duration error
									RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Selected File Error: Duration Is 0 Sec!", _StatusMessageType_Error);

									// Reset the "in Recording Analysis mode" flag
									InRecordingAnalysisModeFlag = false;

								}

							}
							else {

								// Write GUI status message - frame rate error
								RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Selected File Error: Frame Rate Is '0' FPS!", _StatusMessageType_Error);

								// Reset the "in Recording Analysis mode" flag
								InRecordingAnalysisModeFlag = false;

							}

						}
						else {

							// Write GUI status message - the file has no frames
							RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Selected File Error: File Has '0' Total Frames!", _StatusMessageType_Error);

							// Reset the "in Recording Analysis mode" flag
							InRecordingAnalysisModeFlag = false;

						}

					}
					else {

						// Write GUI status message - file frame height error
						RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Selected File Error: File Has '0' Pixel Height!", _StatusMessageType_Error);

						// Reset the "in Recording Analysis mode" flag
						InRecordingAnalysisModeFlag = false;

					}

				}
				else {

					// Write GUI status message - file frame width error
					RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Selected File Error: File Has '0' Pixel Width!", _StatusMessageType_Error);

					// Reset the "in Recording Analysis mode" flag
					InRecordingAnalysisModeFlag = false;

				}

			}
			else {

				// Write GUI status message - the file is not an .avi file
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Selected File Is Not An .avi File!", _StatusMessageType_Error);

				// Reset the "in Recording Analysis mode" flag
				InRecordingAnalysisModeFlag = false;

			}

		}
		else {

			// Write GUI status message - the file has not been opened
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Selected Video File Could Not Be Opened!", _StatusMessageType_Error);

			// Reset the "in Recording Analysis mode" flag
			InRecordingAnalysisModeFlag = false;

		}

		// If "Recording Analysis" mode has not become active
		if (InRecordingAnalysisModeFlag == false) {

			// Handle the events and actions for resetting "Recording Analysis" mode
			RMH_ThermalViewer_HandleSellectedDeviceOrModeChange();

			// Write GUI status message - the file has not been opened
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Selected Video File Could Not Be Opened, Please Check The Selected File.", _StatusMessageType_Error);

		}

	}
	else {

		// Reset the "in Recording Analysis mode" flag
		InRecordingAnalysisModeFlag = false;

		// Reset the camera connect error flag
		CameraConnectErrorFlag = false;

		// Check whether the thermal camera is already connected and active
		if (IRCamera.ConnectedFlag == true) {

			// Write GUI status message - if a thermal camera is already connected and active
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "A Thermal Camera Is Already Connected & Streaming Video.", _StatusMessageType_Normal);
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Sellect Another Camera Or Press The Disconnect Button To Disconnect The Connected Thermal Camera.", _StatusMessageType_Normal);

		}
		else {

			// ----------------------------------------------------------------------------------------------------------------------------------------------- // 
			//													   Connect To The Selected Thermal Camera Pool													   // 
			// ----------------------------------------------------------------------------------------------------------------------------------------------- // 

			// Reset the number of video frames read variable
			IRCamera.NumbOfCapturedFrames = 0;

			// Write GUI status message
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Connecting To The Sellected Thermal Camera...", _StatusMessageType_Normal);

			// Connect to the selected thermal camera and return the status
			IRCamera = RMH_IRThermalCamera_ConnectToThermalCamera(GlobalVariables::GlobalCameraSourceDropList);

			// Write the camera status message to the GUI status text box
			RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, IRCamera.StatusMessage, _StatusMessageType_Normal);

			// ----------------------------------------------------------------------------------------------------------------------------------------------- // 
			//								  If An Active Connection To The Selected Thermal Camera Was Established										   // 
			// ----------------------------------------------------------------------------------------------------------------------------------------------- //  

			// If an active connection to the selected thermal camera was established
			if (IRCamera.ConnectedFlag == true) {

				// ------------------------------------------------------------------------------------------------------------------------------------------- //
				//                                                     Write Connection Status Messages                                                     //
				// ------------------------------------------------------------------------------------------------------------------------------------------- //

				// Write the camera device name to the GUI status text box
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Device Name: " + IRCamera.CameraDeviceName, _StatusMessageType_Normal);

				// Write the camera device IR sensor frame info to the GUI status text box
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Image Frame Width: " + RMH_Conversion_IntToStdString(IRCamera.FrameWidth) + " Pixels", _StatusMessageType_Normal);
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Image Frame Height: " + RMH_Conversion_IntToStdString(IRCamera.FrameHeight - IRCamera.FrameMetadataSize) + " Pixels", _StatusMessageType_Normal);

				// ------------------------------------------------------------------------------------------------------------------------------------------- //
				//             Handle Events, Sequences & Actions To Be Performed After Connecting To A Selected Pool Of Thermal Cameras              //
				// ------------------------------------------------------------------------------------------------------------------------------------------- //

				// Update the connect button label text
				GlobalVariables::GlobalConnectButton->Text = "Please Wait...";
				// Update the connect button border color 
				GlobalVariables::GlobalConnectButton->FlatAppearance->BorderColor = System::Drawing::Color::Yellow;
				GlobalVariables::GlobalConnectButton->Update();

				// Which camera pool does the connected thermal camera belong to
				switch (IRCamera.ThermalCameraSupportPool) {

					// Supported thermal camera pool 1
					case _SupportedThermalCameras_Pool_1:

						// Handle the events, sequences and actions to be performed after connecting to a pool 1 thermal camera 
						RMH_IRThermalCamera_ThermalCameraInitialConnectionEvents_Pool1();

					break;

					// Supported thermal camera pool 2
					case _SupportedThermalCameras_Pool_2:

						// Handle the events, sequences and actions to be performed after connecting to a pool 2 thermal camera 
						RMH_IRThermalCamera_ThermalCameraInitialConnectionEvents_Pool2();

					break;

					// Supported thermal camera pool 3
					case _SupportedThermalCameras_Pool_3:

						// Handle the events, sequences and actions to be performed after connecting to a pool 3 thermal camera 
						RMH_IRThermalCamera_ThermalCameraInitialConnectionEvents_Pool3();

					break;

					// Supported thermal camera pools 4, 5 and 6
					case _SupportedThermalCameras_Pool_4: case _SupportedThermalCameras_Pool_5: case _SupportedThermalCameras_Pool_6:

						// Handle the events, sequences and actions to be performed after connecting to a pool 4/5/6 thermal camera
						RMH_IRThermalCamera_ThermalCameraInitialConnectionEvents_Pool4();

					break;

				}

				// ------------------------------------------------------------------------------------------------------------------------------------------- //
				//                        Handle Events, Sequences & Actions After The Start-Up Initialization Of The Thermal Camera                         //
				// ------------------------------------------------------------------------------------------------------------------------------------------- //

				// If there were no camera connection errors
				if (CameraConnectErrorFlag == false) {

					// ------------------------------------------------- Update GUI Elements & Components ------------------------------------------------- //

					// Update the connect button label text
					GlobalVariables::GlobalConnectButton->Text = "Connected";
					// Update the connect button border color 
					GlobalVariables::GlobalConnectButton->FlatAppearance->BorderColor = System::Drawing::Color::Lime;

					// Enable the camera configuration GUI components
					RMH_ThermalViewer_EnableCameraConfigurationControls(true);

					// Enable the camera disconnect button
					GlobalVariables::GlobalDisconnectButton->Enabled = true;
					// Enable the auto shutter calibration button in the settings menu
					GlobalVariables::GlobalAutoShutterCalButton->Enabled = true;
					// Enable the temperature-drift-based calibration button in the settings menu
					GlobalVariables::GlobalSensorDriftCalButton->Enabled = true;
					// Enable the calibration button in the live view tools panel
					GlobalVariables::GlobalCalibrateCameraButton->Enabled = true;
					// Enable the temperature range button in the live view tools panel
					GlobalVariables::GlobalTempRangeButton->Enabled = true;
					// Enable the record button in the live view tools panel
					GlobalVariables::GlobalRecordingButton->Enabled = true;

					// ----------------------------------- Initialization Of The OpenGL Rendering Texture For The Live Video Stream ----------------------------------- //

					// Configure the live view stream OpenGL texture rendering resolution 
					GlobalVariables::OpenGLRender->RMH_OpenGL_InitImageTexture(IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize);
					// Configure the live view OpenGL zoom texture rendering resolution 
					GlobalVariables::LiveViewZoomWindowRender->RMH_OpenGL_InitLiveViewZoomWindow(IRCamera.FrameWidth, IRCamera.FrameHeight - IRCamera.FrameMetadataSize);

					// Pool 5 (Thermal Master P3) sensor is mounted rotated 90 degrees - correct it automatically
					if (IRCamera.ThermalCameraSupportPool == _SupportedThermalCameras_Pool_5) {
						while (GlobalVariables::OpenGLRender->RMH_LiveView_GetRotation() != 0.0) { GlobalVariables::OpenGLRender->RMH_OpenGL_RotateLiveViewCCW(); }
					}

					// Toggle/update the live view enhanced image resolution mode
					RMH_ThermalViewer_ToggleEnhancedLiveViewResolution();
					// Toggle/update the live view ultra image resolution mode
					RMH_ThermalViewer_ToggleLiveViewUltraResolution();

					// ------------------------------- Start Camera Video Capturing, Processing Thread & Main Update Timer -------------------------------- //

					// Write GUI status message
					RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Starting The Live View Video Stream...", _StatusMessageType_Normal);

					// Start the camera video capture
					RMH_IRThermalCamera_StartCapturing();

					// Start the asynchronous thread operation
					GlobalVariables::GlobalVideoStreamThread->RunWorkerAsync();
					GlobalVariables::GlobalSecondaryProcessingThread->RunWorkerAsync();

					// Enable the GUI update timer
					GlobalVariables::GlobalMainGUIUpdateTimer->Enabled = true;
					GlobalVariables::GlobalMainGUIUpdateTimer->Start();

					// Update the camera "isStreaming" status flag
					IRCamera.isStreaming = true;

					// ----------------------------------------------------- Write End Status Message ------------------------------------------------------ //

					// Write GUI status message
					RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "The Camera Is Now Streaming Live Video.", _StatusMessageType_Success);

					// Enable the application features
					RMH_Application_EnableApplicationFeatures();

					// --------------------------------------------------------------------------------------------------------------------------------------- //

				}
				else { // If camera connection errors were registered

					// ------------------------------------------------ Reset Of Relevant Control Flags ------------------------------------------------ //

					// Reset the camera connect error flag
					CameraConnectErrorFlag = false;

					// Reset the camera connect flag
					IRCamera.ConnectedFlag = false;

					// ------------------------------------------- Stop Camera Video Capturing & Main Update Timer ------------------------------------------- //

					// Disable the GUI update timer
					GlobalVariables::GlobalMainGUIUpdateTimer->Enabled = false;
					GlobalVariables::GlobalMainGUIUpdateTimer->Stop();

					// Update the connect button border color - indicate connection error
					GlobalVariables::GlobalConnectButton->FlatAppearance->BorderColor = System::Drawing::Color::Red;

					// Write GUI status message
					RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "ERROR: Camera Video Feed Could Not Be Started!", _StatusMessageType_Error);

					// --------------------------------------------------------------------------------------------------------------------------------------- //

				}

				// ------------------------------------------------------------------------------------------------------------------------------------------- //
			}
			else { // If an active connection to the selected thermal camera was NOT established

				// Reset the connect button label text
				GlobalVariables::GlobalConnectButton->Text = "Camera Not Found!";
				// Update the connect button border color 
				GlobalVariables::GlobalConnectButton->FlatAppearance->BorderColor = System::Drawing::Color::Red;

				// Write the camera device name to the GUI status text box
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Thermal Camera Device Could Not Be Found, Or A Connection Error Occured", _StatusMessageType_Error);

			}
		}

	}

}

// ------------------------------------------------------------------------------------------------------------------------------- //




