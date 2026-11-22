
/*
 *  RMH_Application_SaveSession.h
 *
 *  Author: Rune Mark Hansen
 *  Date: December 2022
 *
 */

#pragma once

// RMH_Application_SaveSession.h
#ifndef RMH_Application_SaveSession_H 
#define RMH_Application_SaveSession_H

#include <string>
#include <vector>

// ------------ Routines For Handling Saved Application Session Parameters ------------- //

std::vector<std::string> RMH_Application_BuildSessionConfigLines();
void RMH_Application_ApplySessionConfigLines(const std::vector<std::string>& Lines, bool LiveApply);
void RMH_Application_SaveLastSessionConfigToFile();
void RMH_Application_SetSavedSessionConfigToApplication(System::Windows::Forms::RichTextBox^ GUIInfoTextArea);

// ------------------------------------------------------------------------------------------ //

#endif /* RMH_Application_SaveSession_H */
