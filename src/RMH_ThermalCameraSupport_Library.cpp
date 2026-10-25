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
