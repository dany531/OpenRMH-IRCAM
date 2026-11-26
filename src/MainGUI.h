#pragma once

// Included libraries
#include "GlobalObjectsAndVariables.h"
#include "RMH_ImageProcessing_Library.h"
#include "RMH_Application_ThermalViewer.h"
#include "RMH_Application_SaveSession.h"
#include "SessionFile.h"
#include "RMH_MathConversions_Library.h"
#include "RMH_Winforms_Library.h"
#include <iostream>

// Included application resources
#include "RMH_Application_Information.h"

// Included form headers
#include "LiveViewStream.h"
#include "WelcomeScreen.h"
#include "ThermalCameraGUI.h"
#include "SurfacePlotGUI.h"
#include "TempMeasGUI.h"
#include "EmissivityTableGUI.h"
#include "DataLogGUI.h"
#include "VideoPlayBackTools.h"
#include "PopUpDialog.h"
#include "UserGuideViewerGUI.h"

// Class namespace
namespace IRCAMThermalViewer {

	// Associated namespaces
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Threading;
	using namespace std;

	// Summary for Form - MainGUI
	public ref class MainGUI : public System::Windows::Forms::Form {

	public:

		// ------------------------------ Local Form Reference Structure ------------------------------ //

		// Local reference structure
		ref struct ManagedLocals {

			// Live view stream form static objects and variables
			static IRCAMThermalViewer::LiveViewStream^ LiveViewStreamForm;

			// Welcome screen form static objects and variables
			static IRCAMThermalViewer::WelcomeScreen^ WelcomeScreenForm;

			// User guide form static objects and variables
			static IRCAMThermalViewer::UserGuideViewerGUI^ UserGuideViewerGUIForm;

			// Thermal camera form static objects and variables
			static IRCAMThermalViewer::ThermalCameraGUI^ ThermalCameraGUIForm;

			// Surface plot form static objects and variables
			static IRCAMThermalViewer::SurfacePlotGUI^ SurfacePlotGUIForm;

			// Temperature measurement plot form static objects and variables
			static IRCAMThermalViewer::TempMeasGUI^ TempMeasurementsGUIForm;

			// Emissivity table GUI form static objects and variables
			static IRCAMThermalViewer::EmissivityTableGUI^ EmissivityTableGUIForm;

			// Data log GUI form static object
			static IRCAMThermalViewer::DataLogGUI^ DataLogGUIForm;

			// Video playback tools GUI form static objects and variables
			static IRCAMThermalViewer::VideoPlayBackTools^ PlayBackControlsPanelForm;

		};

		// -------------------------------------------------------------------------------------------- //

	public:

		// ------------------------------------ Class Constructor ------------------------------------ //

		MainGUI(void) {

			// Init GUI components and objects
			InitializeComponent();
			// Format arrays of WinForms components for global use
			InitializeComponentArrays();
			// Add the "Data Log" item to the left menu (between "Plots" and "Emissivity")
			AddDataLogMenuItem();
			// Add the "Session" menu (Open Session / Save Session / Save Session As) at the top of the window
			BuildSessionMenu();
			// Set global objects from this form for global use
			InitializeGlobalFormsObjects();

			// Enable dark mode for the application title bar
			RMH_Winforms_EnableTitleBarDarkMode(this->Handle);

			// Set the saved application configuration at application start-up
			RMH_Application_SetSavedSessionConfigToApplication(this->GUIInfoTextArea);

			// Initialize the live view screen form
			ManagedLocals::LiveViewStreamForm = gcnew IRCAMThermalViewer::LiveViewStream();
			// Initialize the welcome screen form
			ManagedLocals::WelcomeScreenForm = gcnew IRCAMThermalViewer::WelcomeScreen();
			// The user guide GUI form (embedded PDF viewer, Adobe Reader only) is not created: the user guide button
			// opens the manual in the system's default PDF viewer instead
			// Initialize the thermal camera GUI form
			ManagedLocals::ThermalCameraGUIForm = gcnew IRCAMThermalViewer::ThermalCameraGUI();
			// Initialize the surface plot GUI form
			ManagedLocals::SurfacePlotGUIForm = gcnew IRCAMThermalViewer::SurfacePlotGUI();
			// Initialize the temperature measurement plot GUI form
			ManagedLocals::TempMeasurementsGUIForm = gcnew IRCAMThermalViewer::TempMeasGUI();
			// Initialize the emissivity table GUI form
			ManagedLocals::EmissivityTableGUIForm = gcnew IRCAMThermalViewer::EmissivityTableGUI();
			ManagedLocals::DataLogGUIForm = gcnew IRCAMThermalViewer::DataLogGUI();

			// The data logging (RMH_Application_ThermalViewer.cpp) does not know the Data Log window: when a stopped session is saved, it calls this routine to write the
			// markers, cursors, zoom, title, notes and hidden lines of the graph into the saved log file (the same "#graph." lines as Save Log File)
			GlobalVariables::DataLoggingGraphStateWriter = gcnew System::Action<System::String^>(ManagedLocals::DataLogGUIForm, &IRCAMThermalViewer::DataLogGUI::AppendGraphStateToLogFile);

			// Set the 2D plot line color data of the saved session
			RMH_ThermalViewer_Load2DPlotSavedSessionLineColorData();

			// Apply the start-up state of the 2D plot data set settings (data source and enable CheckBox) to the 2D plot
			RMH_ThermalViewer_Apply2DPlotDataSetSettings();

			// Handle docking of the welcome form at start-up
			HandleFormsOpeningDockingAndUndocking(ManagedLocals::WelcomeScreenForm, this->MainViewTopPanel, &isWelcomeScreenFormOpen, &isWelcomeScreenFormDocked, &isWelcomeScreenFormUndocked, _FormDockingState_DockForm);

			// Update the title bar string of the form
			RMH_Winforms_ChangeFormTitleBarText(this, ApplicationInformationString + _CustomBuild_TitleSuffix);

		}

		// ---------------------------- Miscellaneous Class-Specific Methods ----------------------------- //

		void InitializeComponentArrays(void) {

			// This routine formats arrays of WinForms components for global use

			// Array of the menu buttons of the main GUI
			GlobalVariables::MainGUILeftMenuButtons = gcnew cli::array<System::Windows::Forms::Button^>(8) {
				this->LiveViewMenuButton,
				this->SurfacePlotMenuButton,
				this->TempMeasMenuButton,
				this->EmissivityMenuButton,
				this->LiveViewUndockButton,
				this->SurfacePlotUndockButton,
				this->TempMeasUndockButton,
				this->EmissivityUndockButton
			};

		}

		void InitializeGlobalFormsObjects() {

			// This routine sets global objects from this form
			// so that these can be accessed from other forms

			// Initialize global objects to the associated form objects
			GlobalVariables::GlobalGUIInfoTextArea = this->GUIInfoTextArea;
			GlobalVariables::GlobalVideoStreamThread = this->VideoStreamThread;
			GlobalVariables::GlobalMainGUIUpdateTimer = this->MainGUIUpdateTimer;
			GlobalVariables::GlobalTempMeasMenuButton = this->TempMeasMenuButton;
			GlobalVariables::GlobalSurfacePlotMenuButton = this->SurfacePlotMenuButton;
			GlobalVariables::GlobalSecondaryProcessingThread = this->SecondaryProcessingThread;

		}

		void HandleFormsOpeningDockingAndUndocking(System::Windows::Forms::Form^ FormObject, System::Windows::Forms::Panel^ ParentPanel, bool* FormOpenedFlag, bool* FormDockedFlag, bool* FormUndockedFlag, unsigned short FormState) {

			// This routine handles docking and undocking of the different forms

			// Should a form be docked to the parent panel
			if (FormState == _FormDockingState_DockForm) {

				// Check the state of the previous associated form object
				if (isThermalCameraFormOpen == true && isThermalCameraFormDocked == true && FormObject != ManagedLocals::ThermalCameraGUIForm) {

					// Remove the associated form as a "control" from the given "parent" panel
					ParentPanel->Controls->Clear();

					// Close the associated form object
					RMH_Winforms_CloseForm(ManagedLocals::ThermalCameraGUIForm, &isThermalCameraFormOpen, &isThermalCameraFormDocked, &isThermalCameraFormUndocked);

				}
				if (isLiveViewStreamFormOpen == true && isLiveViewStreamFormDocked == true && FormObject != ManagedLocals::LiveViewStreamForm) {

					// Remove the associated form as a "control" from the given "parent" panel
					ParentPanel->Controls->Clear();

					// Close the associated form object
					RMH_Winforms_CloseForm(ManagedLocals::LiveViewStreamForm, &isLiveViewStreamFormOpen, &isLiveViewStreamFormDocked, &isLiveViewStreamFormUndocked);

				}
				if (isSurfacePlotFormOpen == true && isSurfacePlotFormDocked == true && FormObject != ManagedLocals::SurfacePlotGUIForm) {

					// Remove the associated form as a "control" from the given "parent" panel
					ParentPanel->Controls->Clear();

					// Close the associated form object
					RMH_Winforms_CloseForm(ManagedLocals::SurfacePlotGUIForm, &isSurfacePlotFormOpen, &isSurfacePlotFormDocked, &isSurfacePlotFormUndocked);

				}
				if (isTempMeasurementsFormOpen == true && isTempMeasurementsFormDocked == true && FormObject != ManagedLocals::TempMeasurementsGUIForm) {

					// Remove the associated form as a "control" from the given "parent" panel
					ParentPanel->Controls->Clear();

					// Close the associated form object
					RMH_Winforms_CloseForm(ManagedLocals::TempMeasurementsGUIForm, &isTempMeasurementsFormOpen, &isTempMeasurementsFormDocked, &isTempMeasurementsFormUndocked);

				}
				if (isEmissivityTableFormOpen == true && isEmissivityTableFormDocked == true && FormObject != ManagedLocals::EmissivityTableGUIForm) {

					// Remove the associated form as a "control" from the given "parent" panel
					ParentPanel->Controls->Clear();

					// Close the associated form object
					RMH_Winforms_CloseForm(ManagedLocals::EmissivityTableGUIForm, &isEmissivityTableFormOpen, &isEmissivityTableFormDocked, &isEmissivityTableFormUndocked);

				}
				if (isDataLogFormOpen == true && isDataLogFormDocked == true && FormObject != ManagedLocals::DataLogGUIForm) {

					// Remove the associated form as a "control" from the given "parent" panel
					ParentPanel->Controls->Clear();

					// Close the associated form object
					RMH_Winforms_CloseForm(ManagedLocals::DataLogGUIForm, &isDataLogFormOpen, &isDataLogFormDocked, &isDataLogFormUndocked);

				}
				if (isWelcomeScreenFormOpen == true && isWelcomeScreenFormDocked == true && FormObject != ManagedLocals::WelcomeScreenForm) {

					// Remove the associated form as a "control" from the given "parent" panel
					ParentPanel->Controls->Clear();

					// Close the associated form object
					RMH_Winforms_CloseForm(ManagedLocals::WelcomeScreenForm, &isWelcomeScreenFormOpen, &isWelcomeScreenFormDocked, &isWelcomeScreenFormUndocked);

				}
				if (isUserGuideFormOpen == true && isUserGuideFormDocked == true && FormObject != ManagedLocals::UserGuideViewerGUIForm) {

					// Remove the associated form as a "control" from the given "parent" panel
					ParentPanel->Controls->Clear();

					// Close the associated form object
					RMH_Winforms_CloseForm(ManagedLocals::UserGuideViewerGUIForm, &isUserGuideFormOpen, &isUserGuideFormDocked, &isUserGuideFormUndocked);

				}

			}

			// ---------------------------------------------------------------- Docking/Undocking Procedure ---------------------------------------------------------------- //

			// Should a form be docked to the parent panel
			if (FormState == _FormDockingState_DockForm) {

				// If the selected form is already open
				if (*FormOpenedFlag == true) {

					// Close the selected form before docking
					RMH_Winforms_CloseForm(FormObject, FormOpenedFlag, FormDockedFlag, FormUndockedFlag);

				}

				// Open and dock the form in the main panel of the main GUI
				RMH_Winforms_OpenAndDockFormInParentPanel(FormObject, ParentPanel, FormOpenedFlag, FormDockedFlag, FormUndockedFlag);

			}

			// Should a form be undocked from the parent panel
			if (FormState == _FormDockingState_UndockForm) {

				// If the main view panel is the parent of the selected form object
				if (FormObject->Parent == ParentPanel) {

					// Reset the parent of the form 
					FormObject->Parent = nullptr;
					// Remove the form as a "control" from the given "parent" panel
					ParentPanel->Controls->Clear();

				}

				// Check whether the form is open, but not docked
				if (*FormOpenedFlag == true && *FormDockedFlag == false) {

					// Close the form
					RMH_Winforms_CloseForm(FormObject, FormOpenedFlag, FormDockedFlag, FormUndockedFlag);

					// Undock the form and open it in a separate window
					RMH_Winforms_OpenFormInSeperateWindow(FormObject, FormOpenedFlag, FormDockedFlag, FormUndockedFlag);

				}

				// Check whether the form is open and docked
				if (*FormOpenedFlag == true && *FormDockedFlag == true) {

					// Close the form
					RMH_Winforms_CloseForm(FormObject, FormOpenedFlag, FormDockedFlag, FormUndockedFlag);

					// Undock the form from the parent panel and open it in a separate window
					RMH_Winforms_UndockFormFromParentPanel(FormObject, ParentPanel, FormOpenedFlag, FormDockedFlag, FormUndockedFlag, System::Windows::Forms::FormBorderStyle::Sizable);

				}

				// Check whether the form is not open and not docked
				if (*FormOpenedFlag == false && *FormDockedFlag == false) {

					// Undock the form and open it in a separate window
					RMH_Winforms_OpenFormInSeperateWindow(FormObject, FormOpenedFlag, FormDockedFlag, FormUndockedFlag);

				}

			}

			// ------------------------------------------------------------------------------------------------------------------------------------------------------------- //

			// If no forms are docked in the view panel of the main GUI
			if (isThermalCameraFormDocked == false &&
				isLiveViewStreamFormDocked == false &&
				isSurfacePlotFormDocked == false &&
				isTempMeasurementsFormDocked == false &&
				isDataLogFormDocked == false &&
				isEmissivityTableFormDocked == false) {

				// Open and dock the welcome screen form in the main panel of the main GUI
				RMH_Winforms_OpenAndDockFormInParentPanel(ManagedLocals::WelcomeScreenForm, ParentPanel, &isWelcomeScreenFormOpen, &isWelcomeScreenFormDocked, &isWelcomeScreenFormUndocked);

			}

		}

		// -------------------------------------------------------------------------------------------- //

		protected:

			/// <summary>
			/// Clean up any resources being used.
			/// </summary>
			~MainGUI() {

				if (components) {

					// Delete all form components
					delete components;

				}

			}

		public:

			/// <summary>
			/// Required designer variable.
			/// </summary>
			private: System::ComponentModel::IContainer^ components;
			private: System::Windows::Forms::ColorDialog^ GlobalColorDialog;
			private: System::Windows::Forms::ToolTip^ GlobalInfoToolTip;
			private: System::Windows::Forms::Panel^ LeftGUIPanel;
			private: System::Windows::Forms::Button^ ThermalCAMMenuButton;
			private: System::Windows::Forms::Panel^ TopLeftGUIPanel;
			private: System::Windows::Forms::Button^ AboutMenuButton;
			private: System::Windows::Forms::Button^ EmissivityMenuButton;
			private: System::Windows::Forms::Button^ TempMeasMenuButton;
			private: System::Windows::Forms::Button^ SurfacePlotMenuButton;
			private: System::Windows::Forms::Button^ LiveViewMenuButton;
			private: System::Windows::Forms::Panel^ MainViewTopPanel;
			private: System::Windows::Forms::Timer^ MainGUIUpdateTimer;
			private: System::Windows::Forms::RichTextBox^ GUIInfoTextArea;
			private: System::Windows::Forms::Panel^ MainMenuButtonsPanel;
			private: System::Windows::Forms::TableLayoutPanel^ tableLayoutPanel1;
			private: System::Windows::Forms::TableLayoutPanel^ tableLayoutPanel2;
			private: System::Windows::Forms::Button^ ThermalCAMUndockButton;
			private: System::Windows::Forms::TableLayoutPanel^ tableLayoutPanel6;
			private: System::Windows::Forms::Button^ EmissivityUndockButton;
			private: System::Windows::Forms::TableLayoutPanel^ tableLayoutPanel5;
			private: System::Windows::Forms::Button^ TempMeasUndockButton;
			private: System::Windows::Forms::TableLayoutPanel^ tableLayoutPanel4;
			private: System::Windows::Forms::Button^ SurfacePlotUndockButton;
			private: System::Windows::Forms::TableLayoutPanel^ tableLayoutPanel3;
			private: System::Windows::Forms::SplitContainer^ splitContainer1;
			private: System::Windows::Forms::Button^ UserGuideButton;
			private: System::ComponentModel::BackgroundWorker^ SecondaryProcessingThread;
			private: System::ComponentModel::BackgroundWorker^ VideoStreamThread;
			private: System::Windows::Forms::Button^ LiveViewUndockButton;

	#pragma region Windows Form Designer generated code

			/// <summary>
			/// Required method for Designer support - do not modify
			/// the contents of this method with the code editor.
			/// </summary>
			void InitializeComponent(void) {
				this->components = (gcnew System::ComponentModel::Container());
				System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(MainGUI::typeid));
				this->splitContainer1 = (gcnew System::Windows::Forms::SplitContainer());
				this->MainViewTopPanel = (gcnew System::Windows::Forms::Panel());
				this->GUIInfoTextArea = (gcnew System::Windows::Forms::RichTextBox());
				this->VideoStreamThread = (gcnew System::ComponentModel::BackgroundWorker());
				this->GlobalColorDialog = (gcnew System::Windows::Forms::ColorDialog());
				this->GlobalInfoToolTip = (gcnew System::Windows::Forms::ToolTip(this->components));
				this->MainGUIUpdateTimer = (gcnew System::Windows::Forms::Timer(this->components));
				this->LeftGUIPanel = (gcnew System::Windows::Forms::Panel());
				this->UserGuideButton = (gcnew System::Windows::Forms::Button());
				this->AboutMenuButton = (gcnew System::Windows::Forms::Button());
				this->MainMenuButtonsPanel = (gcnew System::Windows::Forms::Panel());
				this->tableLayoutPanel1 = (gcnew System::Windows::Forms::TableLayoutPanel());
				this->tableLayoutPanel6 = (gcnew System::Windows::Forms::TableLayoutPanel());
				this->EmissivityUndockButton = (gcnew System::Windows::Forms::Button());
				this->EmissivityMenuButton = (gcnew System::Windows::Forms::Button());
				this->tableLayoutPanel2 = (gcnew System::Windows::Forms::TableLayoutPanel());
				this->ThermalCAMUndockButton = (gcnew System::Windows::Forms::Button());
				this->ThermalCAMMenuButton = (gcnew System::Windows::Forms::Button());
				this->tableLayoutPanel5 = (gcnew System::Windows::Forms::TableLayoutPanel());
				this->TempMeasUndockButton = (gcnew System::Windows::Forms::Button());
				this->TempMeasMenuButton = (gcnew System::Windows::Forms::Button());
				this->TopLeftGUIPanel = (gcnew System::Windows::Forms::Panel());
				this->tableLayoutPanel4 = (gcnew System::Windows::Forms::TableLayoutPanel());
				this->SurfacePlotUndockButton = (gcnew System::Windows::Forms::Button());
				this->SurfacePlotMenuButton = (gcnew System::Windows::Forms::Button());
				this->tableLayoutPanel3 = (gcnew System::Windows::Forms::TableLayoutPanel());
				this->LiveViewMenuButton = (gcnew System::Windows::Forms::Button());
				this->LiveViewUndockButton = (gcnew System::Windows::Forms::Button());
				this->SecondaryProcessingThread = (gcnew System::ComponentModel::BackgroundWorker());
				(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->splitContainer1))->BeginInit();
				this->splitContainer1->Panel1->SuspendLayout();
				this->splitContainer1->Panel2->SuspendLayout();
				this->splitContainer1->SuspendLayout();
				this->LeftGUIPanel->SuspendLayout();
				this->MainMenuButtonsPanel->SuspendLayout();
				this->tableLayoutPanel1->SuspendLayout();
				this->tableLayoutPanel6->SuspendLayout();
				this->tableLayoutPanel2->SuspendLayout();
				this->tableLayoutPanel5->SuspendLayout();
				this->tableLayoutPanel4->SuspendLayout();
				this->tableLayoutPanel3->SuspendLayout();
				this->SuspendLayout();
				// 
				// splitContainer1
				// 
				resources->ApplyResources(this->splitContainer1, L"splitContainer1");
				this->splitContainer1->Name = L"splitContainer1";
				// 
				// splitContainer1.Panel1
				// 
				this->splitContainer1->Panel1->Controls->Add(this->MainViewTopPanel);
				// 
				// splitContainer1.Panel2
				// 
				this->splitContainer1->Panel2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(35)),
					static_cast<System::Int32>(static_cast<System::Byte>(35)), static_cast<System::Int32>(static_cast<System::Byte>(35)));
				this->splitContainer1->Panel2->Controls->Add(this->GUIInfoTextArea);
				resources->ApplyResources(this->splitContainer1->Panel2, L"splitContainer1.Panel2");
				// 
				// MainViewTopPanel
				// 
				this->MainViewTopPanel->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(35)), static_cast<System::Int32>(static_cast<System::Byte>(35)),
					static_cast<System::Int32>(static_cast<System::Byte>(35)));
				resources->ApplyResources(this->MainViewTopPanel, L"MainViewTopPanel");
				this->MainViewTopPanel->Name = L"MainViewTopPanel";
				// 
				// GUIInfoTextArea
				// 
				this->GUIInfoTextArea->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(35)), static_cast<System::Int32>(static_cast<System::Byte>(35)),
					static_cast<System::Int32>(static_cast<System::Byte>(35)));
				this->GUIInfoTextArea->BorderStyle = System::Windows::Forms::BorderStyle::None;
				this->GUIInfoTextArea->DetectUrls = false;
				resources->ApplyResources(this->GUIInfoTextArea, L"GUIInfoTextArea");
				this->GUIInfoTextArea->ForeColor = System::Drawing::Color::White;
				this->GUIInfoTextArea->HideSelection = false;
				this->GUIInfoTextArea->Name = L"GUIInfoTextArea";
				// 
				// VideoStreamThread
				// 
				this->VideoStreamThread->WorkerReportsProgress = true;
				this->VideoStreamThread->WorkerSupportsCancellation = true;
				this->VideoStreamThread->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &MainGUI::VideoStreamThread_DoWork);
				// 
				// GlobalColorDialog
				// 
				this->GlobalColorDialog->FullOpen = true;
				// 
				// GlobalInfoToolTip
				// 
				this->GlobalInfoToolTip->AutomaticDelay = 1000;
				this->GlobalInfoToolTip->AutoPopDelay = 5000;
				this->GlobalInfoToolTip->InitialDelay = 500;
				this->GlobalInfoToolTip->ReshowDelay = 200;
				this->GlobalInfoToolTip->ToolTipTitle = L"Information:";
				this->GlobalInfoToolTip->UseAnimation = false;
				this->GlobalInfoToolTip->UseFading = false;
				// 
				// MainGUIUpdateTimer
				// 
				this->MainGUIUpdateTimer->Interval = 5;
				this->MainGUIUpdateTimer->Tick += gcnew System::EventHandler(this, &MainGUI::MainGUIUpdateTimer_Tick);
				// 
				// LeftGUIPanel
				// 
				this->LeftGUIPanel->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(32)), static_cast<System::Int32>(static_cast<System::Byte>(32)),
					static_cast<System::Int32>(static_cast<System::Byte>(32)));
				this->LeftGUIPanel->Controls->Add(this->UserGuideButton);
				this->LeftGUIPanel->Controls->Add(this->AboutMenuButton);
				this->LeftGUIPanel->Controls->Add(this->MainMenuButtonsPanel);
				resources->ApplyResources(this->LeftGUIPanel, L"LeftGUIPanel");
				this->LeftGUIPanel->Name = L"LeftGUIPanel";
				// 
				// UserGuideButton
				// 
				resources->ApplyResources(this->UserGuideButton, L"UserGuideButton");
				this->UserGuideButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(35)),
					static_cast<System::Int32>(static_cast<System::Byte>(35)), static_cast<System::Int32>(static_cast<System::Byte>(35)));
				this->UserGuideButton->ForeColor = System::Drawing::Color::White;
				this->UserGuideButton->Name = L"UserGuideButton";
				this->UserGuideButton->UseVisualStyleBackColor = true;
				this->UserGuideButton->Click += gcnew System::EventHandler(this, &MainGUI::UserGuideButton_Click);
				// 
				// AboutMenuButton
				// 
				resources->ApplyResources(this->AboutMenuButton, L"AboutMenuButton");
				this->AboutMenuButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(35)),
					static_cast<System::Int32>(static_cast<System::Byte>(35)), static_cast<System::Int32>(static_cast<System::Byte>(35)));
				this->AboutMenuButton->ForeColor = System::Drawing::Color::White;
				this->AboutMenuButton->Name = L"AboutMenuButton";
				this->AboutMenuButton->UseVisualStyleBackColor = true;
				this->AboutMenuButton->Click += gcnew System::EventHandler(this, &MainGUI::AboutMenuButton_Click);
				// 
				// MainMenuButtonsPanel
				// 
				this->MainMenuButtonsPanel->Controls->Add(this->tableLayoutPanel1);
				resources->ApplyResources(this->MainMenuButtonsPanel, L"MainMenuButtonsPanel");
				this->MainMenuButtonsPanel->Name = L"MainMenuButtonsPanel";
				// 
				// tableLayoutPanel1
				// 
				resources->ApplyResources(this->tableLayoutPanel1, L"tableLayoutPanel1");
				this->tableLayoutPanel1->Controls->Add(this->tableLayoutPanel6, 0, 5);
				this->tableLayoutPanel1->Controls->Add(this->tableLayoutPanel2, 0, 1);
				this->tableLayoutPanel1->Controls->Add(this->tableLayoutPanel5, 0, 4);
				this->tableLayoutPanel1->Controls->Add(this->TopLeftGUIPanel, 0, 0);
				this->tableLayoutPanel1->Controls->Add(this->tableLayoutPanel4, 0, 3);
				this->tableLayoutPanel1->Controls->Add(this->tableLayoutPanel3, 0, 2);
				this->tableLayoutPanel1->Name = L"tableLayoutPanel1";
				// 
				// tableLayoutPanel6
				// 
				resources->ApplyResources(this->tableLayoutPanel6, L"tableLayoutPanel6");
				this->tableLayoutPanel6->Controls->Add(this->EmissivityUndockButton, 1, 0);
				this->tableLayoutPanel6->Controls->Add(this->EmissivityMenuButton, 0, 0);
				this->tableLayoutPanel6->Name = L"tableLayoutPanel6";
				// 
				// EmissivityUndockButton
				// 
				resources->ApplyResources(this->EmissivityUndockButton, L"EmissivityUndockButton");
				this->EmissivityUndockButton->BackColor = System::Drawing::Color::Transparent;
				this->EmissivityUndockButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(35)),
					static_cast<System::Int32>(static_cast<System::Byte>(35)), static_cast<System::Int32>(static_cast<System::Byte>(35)));
				this->EmissivityUndockButton->FlatAppearance->MouseDownBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
					static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
				this->EmissivityUndockButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(50)),
					static_cast<System::Int32>(static_cast<System::Byte>(50)), static_cast<System::Int32>(static_cast<System::Byte>(50)));
				this->EmissivityUndockButton->ForeColor = System::Drawing::Color::White;
				this->EmissivityUndockButton->Name = L"EmissivityUndockButton";
				this->EmissivityUndockButton->UseVisualStyleBackColor = false;
				this->EmissivityUndockButton->Click += gcnew System::EventHandler(this, &MainGUI::EmissivityUndockButton_Click);
				// 
				// EmissivityMenuButton
				// 
				resources->ApplyResources(this->EmissivityMenuButton, L"EmissivityMenuButton");
				this->EmissivityMenuButton->BackColor = System::Drawing::Color::Transparent;
				this->EmissivityMenuButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(35)),
					static_cast<System::Int32>(static_cast<System::Byte>(35)), static_cast<System::Int32>(static_cast<System::Byte>(35)));
				this->EmissivityMenuButton->FlatAppearance->MouseDownBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
					static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
				this->EmissivityMenuButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(50)),
					static_cast<System::Int32>(static_cast<System::Byte>(50)), static_cast<System::Int32>(static_cast<System::Byte>(50)));
				this->EmissivityMenuButton->ForeColor = System::Drawing::Color::White;
				this->EmissivityMenuButton->Name = L"EmissivityMenuButton";
				this->EmissivityMenuButton->UseVisualStyleBackColor = false;
				this->EmissivityMenuButton->Click += gcnew System::EventHandler(this, &MainGUI::EmissivityMenuButton_Click);
				// 
				// tableLayoutPanel2
				// 
				resources->ApplyResources(this->tableLayoutPanel2, L"tableLayoutPanel2");
				this->tableLayoutPanel2->Controls->Add(this->ThermalCAMUndockButton, 1, 0);
				this->tableLayoutPanel2->Controls->Add(this->ThermalCAMMenuButton, 0, 0);
				this->tableLayoutPanel2->Name = L"tableLayoutPanel2";
				// 
				// ThermalCAMUndockButton
				// 
				resources->ApplyResources(this->ThermalCAMUndockButton, L"ThermalCAMUndockButton");
				this->ThermalCAMUndockButton->BackColor = System::Drawing::Color::Transparent;
				this->ThermalCAMUndockButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(35)),
					static_cast<System::Int32>(static_cast<System::Byte>(35)), static_cast<System::Int32>(static_cast<System::Byte>(35)));
				this->ThermalCAMUndockButton->FlatAppearance->MouseDownBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
					static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
				this->ThermalCAMUndockButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(50)),
					static_cast<System::Int32>(static_cast<System::Byte>(50)), static_cast<System::Int32>(static_cast<System::Byte>(50)));
				this->ThermalCAMUndockButton->ForeColor = System::Drawing::Color::White;
				this->ThermalCAMUndockButton->Name = L"ThermalCAMUndockButton";
				this->ThermalCAMUndockButton->UseVisualStyleBackColor = false;
				this->ThermalCAMUndockButton->Click += gcnew System::EventHandler(this, &MainGUI::ThermalCAMUndockButton_Click);
				// 
				// ThermalCAMMenuButton
				// 
				resources->ApplyResources(this->ThermalCAMMenuButton, L"ThermalCAMMenuButton");
				this->ThermalCAMMenuButton->BackColor = System::Drawing::Color::Transparent;
				this->ThermalCAMMenuButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(35)),
					static_cast<System::Int32>(static_cast<System::Byte>(35)), static_cast<System::Int32>(static_cast<System::Byte>(35)));
				this->ThermalCAMMenuButton->FlatAppearance->MouseDownBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
					static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
				this->ThermalCAMMenuButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(50)),
					static_cast<System::Int32>(static_cast<System::Byte>(50)), static_cast<System::Int32>(static_cast<System::Byte>(50)));
				this->ThermalCAMMenuButton->ForeColor = System::Drawing::Color::White;
				this->ThermalCAMMenuButton->Name = L"ThermalCAMMenuButton";
				this->ThermalCAMMenuButton->UseVisualStyleBackColor = false;
				this->ThermalCAMMenuButton->Click += gcnew System::EventHandler(this, &MainGUI::ThermalCAMMenuButton_Click);
				// 
				// tableLayoutPanel5
				// 
				resources->ApplyResources(this->tableLayoutPanel5, L"tableLayoutPanel5");
				this->tableLayoutPanel5->Controls->Add(this->TempMeasUndockButton, 1, 0);
				this->tableLayoutPanel5->Controls->Add(this->TempMeasMenuButton, 0, 0);
				this->tableLayoutPanel5->Name = L"tableLayoutPanel5";
				// 
				// TempMeasUndockButton
				// 
				resources->ApplyResources(this->TempMeasUndockButton, L"TempMeasUndockButton");
				this->TempMeasUndockButton->BackColor = System::Drawing::Color::Transparent;
				this->TempMeasUndockButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(35)),
					static_cast<System::Int32>(static_cast<System::Byte>(35)), static_cast<System::Int32>(static_cast<System::Byte>(35)));
				this->TempMeasUndockButton->FlatAppearance->MouseDownBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
					static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
				this->TempMeasUndockButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(50)),
					static_cast<System::Int32>(static_cast<System::Byte>(50)), static_cast<System::Int32>(static_cast<System::Byte>(50)));
				this->TempMeasUndockButton->ForeColor = System::Drawing::Color::White;
				this->TempMeasUndockButton->Name = L"TempMeasUndockButton";
				this->TempMeasUndockButton->UseVisualStyleBackColor = false;
				this->TempMeasUndockButton->Click += gcnew System::EventHandler(this, &MainGUI::TempMeasUndockButton_Click);
				// 
				// TempMeasMenuButton
				// 
				resources->ApplyResources(this->TempMeasMenuButton, L"TempMeasMenuButton");
				this->TempMeasMenuButton->BackColor = System::Drawing::Color::Transparent;
				this->TempMeasMenuButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(35)),
					static_cast<System::Int32>(static_cast<System::Byte>(35)), static_cast<System::Int32>(static_cast<System::Byte>(35)));
				this->TempMeasMenuButton->FlatAppearance->MouseDownBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
					static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
				this->TempMeasMenuButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(50)),
					static_cast<System::Int32>(static_cast<System::Byte>(50)), static_cast<System::Int32>(static_cast<System::Byte>(50)));
				this->TempMeasMenuButton->ForeColor = System::Drawing::Color::White;
				this->TempMeasMenuButton->Name = L"TempMeasMenuButton";
				this->TempMeasMenuButton->UseVisualStyleBackColor = false;
				this->TempMeasMenuButton->Click += gcnew System::EventHandler(this, &MainGUI::TempMeasMenuButton_Click);
				// 
				// TopLeftGUIPanel
				// 
				this->TopLeftGUIPanel->BackColor = System::Drawing::Color::Transparent;
				resources->ApplyResources(this->TopLeftGUIPanel, L"TopLeftGUIPanel");
				this->TopLeftGUIPanel->Name = L"TopLeftGUIPanel";
				// 
				// tableLayoutPanel4
				// 
				resources->ApplyResources(this->tableLayoutPanel4, L"tableLayoutPanel4");
				this->tableLayoutPanel4->Controls->Add(this->SurfacePlotUndockButton, 1, 0);
				this->tableLayoutPanel4->Controls->Add(this->SurfacePlotMenuButton, 0, 0);
				this->tableLayoutPanel4->Name = L"tableLayoutPanel4";
				// 
				// SurfacePlotUndockButton
				// 
				resources->ApplyResources(this->SurfacePlotUndockButton, L"SurfacePlotUndockButton");
				this->SurfacePlotUndockButton->BackColor = System::Drawing::Color::Transparent;
				this->SurfacePlotUndockButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(35)),
					static_cast<System::Int32>(static_cast<System::Byte>(35)), static_cast<System::Int32>(static_cast<System::Byte>(35)));
				this->SurfacePlotUndockButton->FlatAppearance->MouseDownBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
					static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
				this->SurfacePlotUndockButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(50)),
					static_cast<System::Int32>(static_cast<System::Byte>(50)), static_cast<System::Int32>(static_cast<System::Byte>(50)));
				this->SurfacePlotUndockButton->ForeColor = System::Drawing::Color::White;
				this->SurfacePlotUndockButton->Name = L"SurfacePlotUndockButton";
				this->SurfacePlotUndockButton->UseVisualStyleBackColor = false;
				this->SurfacePlotUndockButton->Click += gcnew System::EventHandler(this, &MainGUI::SurfacePlotUndockButton_Click);
				// 
				// SurfacePlotMenuButton
				// 
				resources->ApplyResources(this->SurfacePlotMenuButton, L"SurfacePlotMenuButton");
				this->SurfacePlotMenuButton->BackColor = System::Drawing::Color::Transparent;
				this->SurfacePlotMenuButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(35)),
					static_cast<System::Int32>(static_cast<System::Byte>(35)), static_cast<System::Int32>(static_cast<System::Byte>(35)));
				this->SurfacePlotMenuButton->FlatAppearance->MouseDownBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
					static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
				this->SurfacePlotMenuButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(50)),
					static_cast<System::Int32>(static_cast<System::Byte>(50)), static_cast<System::Int32>(static_cast<System::Byte>(50)));
				this->SurfacePlotMenuButton->ForeColor = System::Drawing::Color::White;
				this->SurfacePlotMenuButton->Name = L"SurfacePlotMenuButton";
				this->SurfacePlotMenuButton->UseVisualStyleBackColor = false;
				this->SurfacePlotMenuButton->Click += gcnew System::EventHandler(this, &MainGUI::SurfacePlotMenuButton_Click);
				// 
				// tableLayoutPanel3
				// 
				resources->ApplyResources(this->tableLayoutPanel3, L"tableLayoutPanel3");
				this->tableLayoutPanel3->Controls->Add(this->LiveViewMenuButton, 0, 0);
				this->tableLayoutPanel3->Controls->Add(this->LiveViewUndockButton, 1, 0);
				this->tableLayoutPanel3->Name = L"tableLayoutPanel3";
				// 
				// LiveViewMenuButton
				// 
				resources->ApplyResources(this->LiveViewMenuButton, L"LiveViewMenuButton");
				this->LiveViewMenuButton->BackColor = System::Drawing::Color::Transparent;
				this->LiveViewMenuButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(35)),
					static_cast<System::Int32>(static_cast<System::Byte>(35)), static_cast<System::Int32>(static_cast<System::Byte>(35)));
				this->LiveViewMenuButton->FlatAppearance->MouseDownBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
					static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
				this->LiveViewMenuButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(50)),
					static_cast<System::Int32>(static_cast<System::Byte>(50)), static_cast<System::Int32>(static_cast<System::Byte>(50)));
				this->LiveViewMenuButton->ForeColor = System::Drawing::Color::White;
				this->LiveViewMenuButton->Name = L"LiveViewMenuButton";
				this->LiveViewMenuButton->UseVisualStyleBackColor = false;
				this->LiveViewMenuButton->Click += gcnew System::EventHandler(this, &MainGUI::LiveViewMenuButton_Click);
				// 
				// LiveViewUndockButton
				// 
				resources->ApplyResources(this->LiveViewUndockButton, L"LiveViewUndockButton");
				this->LiveViewUndockButton->BackColor = System::Drawing::Color::Transparent;
				this->LiveViewUndockButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(35)),
					static_cast<System::Int32>(static_cast<System::Byte>(35)), static_cast<System::Int32>(static_cast<System::Byte>(35)));
				this->LiveViewUndockButton->FlatAppearance->MouseDownBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)),
					static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)));
				this->LiveViewUndockButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(50)),
					static_cast<System::Int32>(static_cast<System::Byte>(50)), static_cast<System::Int32>(static_cast<System::Byte>(50)));
				this->LiveViewUndockButton->ForeColor = System::Drawing::Color::White;
				this->LiveViewUndockButton->Name = L"LiveViewUndockButton";
				this->LiveViewUndockButton->UseVisualStyleBackColor = false;
				this->LiveViewUndockButton->Click += gcnew System::EventHandler(this, &MainGUI::LiveViewUndockButton_Click);
				// 
				// SecondaryProcessingThread
				// 
				this->SecondaryProcessingThread->WorkerReportsProgress = true;
				this->SecondaryProcessingThread->WorkerSupportsCancellation = true;
				this->SecondaryProcessingThread->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &MainGUI::SecondaryProcessingThread_DoWork);
				// 
				// MainGUI
				// 
				resources->ApplyResources(this, L"$this");
				this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
				this->AutoValidate = System::Windows::Forms::AutoValidate::EnablePreventFocusChange;
				this->BackColor = System::Drawing::Color::Black;
				this->Controls->Add(this->splitContainer1);
				this->Controls->Add(this->LeftGUIPanel);
				this->KeyPreview = true;
				this->Name = L"MainGUI";
				this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
				this->WindowState = System::Windows::Forms::FormWindowState::Maximized;
				this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &MainGUI::MainGUI_FormClosing);
				this->Shown += gcnew System::EventHandler(this, &MainGUI::MainGUI_Shown);
				this->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &MainGUI::MainGUI_KeyPress);
				this->splitContainer1->Panel1->ResumeLayout(false);
				this->splitContainer1->Panel2->ResumeLayout(false);
				(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->splitContainer1))->EndInit();
				this->splitContainer1->ResumeLayout(false);
				this->LeftGUIPanel->ResumeLayout(false);
				this->MainMenuButtonsPanel->ResumeLayout(false);
				this->tableLayoutPanel1->ResumeLayout(false);
				this->tableLayoutPanel6->ResumeLayout(false);
				this->tableLayoutPanel2->ResumeLayout(false);
				this->tableLayoutPanel5->ResumeLayout(false);
				this->tableLayoutPanel4->ResumeLayout(false);
				this->tableLayoutPanel3->ResumeLayout(false);
				this->ResumeLayout(false);

			}

	#pragma endregion

		// ------------------ Main GUI Start-Up And Shutdown Callback Routines ------------------ //

		// Main GUI start-up callback routine -> 
		// Automatic start-up sequence timer, stage and information window
		private: System::Windows::Forms::Timer^ StartupAutomationTimer;
		private: int StartupAutomationStage;
		private: System::Windows::Forms::Form^ StartupWaitWindow;

		// Creates a start-up information window (headline, text, optional progress bar or OK button) ->
		private: System::Windows::Forms::Form^ CreateStartupInfoWindow(System::String^ Headline, System::String^ Body, System::Drawing::Color HeadlineColor, bool ShowProgressBar, bool ShowOkButton) {

			System::Windows::Forms::Form^ InfoWindow = gcnew System::Windows::Forms::Form();
			InfoWindow->Text = "IRCAM for T2S+ V2";
			InfoWindow->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			InfoWindow->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			InfoWindow->ControlBox = false;
			InfoWindow->MinimizeBox = false;
			InfoWindow->MaximizeBox = false;
			InfoWindow->ShowInTaskbar = false;
			InfoWindow->BackColor = System::Drawing::Color::White;
			InfoWindow->ClientSize = System::Drawing::Size(540, 250);

			// Headline
			System::Windows::Forms::Label^ HeadlineLabel = gcnew System::Windows::Forms::Label();
			HeadlineLabel->AutoSize = false;
			HeadlineLabel->Location = System::Drawing::Point(20, 18);
			HeadlineLabel->Size = System::Drawing::Size(500, 40);
			HeadlineLabel->Font = gcnew System::Drawing::Font("Segoe UI", 14.0f, System::Drawing::FontStyle::Bold);
			HeadlineLabel->ForeColor = HeadlineColor;
			HeadlineLabel->Text = Headline;
			InfoWindow->Controls->Add(HeadlineLabel);

			// Body text
			System::Windows::Forms::Label^ BodyLabel = gcnew System::Windows::Forms::Label();
			BodyLabel->AutoSize = false;
			BodyLabel->Location = System::Drawing::Point(20, 66);
			BodyLabel->Size = System::Drawing::Size(500, 120);
			BodyLabel->Font = gcnew System::Drawing::Font("Segoe UI", 10.0f);
			BodyLabel->ForeColor = System::Drawing::Color::Black;
			BodyLabel->Text = Body;
			InfoWindow->Controls->Add(BodyLabel);

			// Progress bar
			if (ShowProgressBar == true) {
				System::Windows::Forms::ProgressBar^ WaitBar = gcnew System::Windows::Forms::ProgressBar();
				WaitBar->Style = System::Windows::Forms::ProgressBarStyle::Marquee;
				WaitBar->MarqueeAnimationSpeed = 30;
				WaitBar->Location = System::Drawing::Point(20, 205);
				WaitBar->Size = System::Drawing::Size(500, 20);
				InfoWindow->Controls->Add(WaitBar);
			}

			// OK button
			if (ShowOkButton == true) {
				System::Windows::Forms::Button^ OkButton = gcnew System::Windows::Forms::Button();
				OkButton->Text = "OK";
				OkButton->DialogResult = System::Windows::Forms::DialogResult::OK;
				OkButton->Location = System::Drawing::Point(210, 200);
				OkButton->Size = System::Drawing::Size(120, 34);
				InfoWindow->Controls->Add(OkButton);
				InfoWindow->AcceptButton = OkButton;
			}

			return InfoWindow;

		}

		// Centers a start-up window on the main window ->
		private: System::Void CenterStartupWindowOnMainWindow(System::Windows::Forms::Form^ InfoWindow) {

			// Is the main window minimized - center the window on the screen instead
			if (this->WindowState == System::Windows::Forms::FormWindowState::Minimized) {
				InfoWindow->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
				return;
			}

			// Position the window in the center of the main window
			System::Drawing::Rectangle MainWindowBounds = this->Bounds;
			InfoWindow->StartPosition = System::Windows::Forms::FormStartPosition::Manual;
			InfoWindow->Location = System::Drawing::Point(
				MainWindowBounds.Left + (MainWindowBounds.Width - InfoWindow->Width) / 2,
				MainWindowBounds.Top + (MainWindowBounds.Height - InfoWindow->Height) / 2);

		}

		// Closes the start-up "please wait" window ->
		private: System::Void CloseStartupWaitWindow() {

			if (this->StartupWaitWindow != nullptr) {
				this->StartupWaitWindow->Close();
				this->StartupWaitWindow = nullptr;
			}

		}

		// Shows the "camera not connected" warning window ->
		private: System::Void ShowStartupCameraNotConnectedWindow() {

#if _AutoStart_ShowPopupWindows
			System::Windows::Forms::Form^ WarningWindow = this->CreateStartupInfoWindow(
				"Camera Not Connected",
				"The InfiRay T2S+ V2 thermal camera was not found.\n\n1. Close this application.\n2. Connect the camera to a USB port.\n3. Wait a few seconds, then open the application again.",
				System::Drawing::Color::FromArgb(190, 0, 0), false, true);
			WarningWindow->ShowDialog(this);
#endif

		}

		// Automatic start-up sequence timer callback routine ->
		private: System::Void StartupAutomationTimer_Tick(System::Object^ sender, System::EventArgs^ e) {

			// Stop the timer - each stage of the sequence runs once
			this->StartupAutomationTimer->Stop();

			// Stage 0: select the camera and connect to it
			if (this->StartupAutomationStage == 0) {

				// Keep the "please wait" window centered on the main window (the main window may have changed size since it was opened)
				if (this->StartupWaitWindow != nullptr) { this->CenterStartupWindowOnMainWindow(this->StartupWaitWindow); }

				// Was the connection successful
				if (RMH_ThermalViewer_AutoStartSelectAndConnectCamera() == true) {

					// Continue with switching to the stream window after a short delay
					this->StartupAutomationStage = 1;
					this->StartupAutomationTimer->Interval = _AutoStart_StreamWindowDelayMs;
					this->StartupAutomationTimer->Start();

				}
				else {

					// The camera is not connected - close the "please wait" window and warn the user
					this->CloseStartupWaitWindow();
					this->ShowStartupCameraNotConnectedWindow();

				}

			}
			// Stage 1: switch to the stream window
			else if (this->StartupAutomationStage == 1) {

				// Is the camera still connected
				if (IRCamera.ConnectedFlag == true) {

					// Is the stream window not already open and docked in the main view
					if (!(isLiveViewStreamFormOpen == true && isLiveViewStreamFormDocked == true)) {

						// Switch to the stream window (the same as pressing the "Live View" menu button)
						this->LiveViewMenuButton_Click(nullptr, nullptr);

					}

					// Continue with the calibration after a short delay
					this->StartupAutomationStage = 2;
					this->StartupAutomationTimer->Interval = _AutoStart_CalibrateDelayMs;
					this->StartupAutomationTimer->Start();

				}
				else {

					// The camera was lost during the start-up sequence
					this->CloseStartupWaitWindow();
					this->ShowStartupCameraNotConnectedWindow();

				}

			}
			// Stage 2: press the "Calibrate Camera" button (hotkey Q) of the stream window and enable periodic shutter calibration
			else if (this->StartupAutomationStage == 2) {

				bool StartupSequenceDone = false;

				// Is the camera still connected
				if (IRCamera.ConnectedFlag == true) {

					// Write GUI status message
					RMH_Winforms_RichTextBox_WriteLine(this->GUIInfoTextArea, "Auto Start: Calibrating The Thermal Camera...", _StatusMessageType_Normal);

					// Press the "Calibrate Camera" button of the stream window - hotkey "Q"
					ManagedLocals::LiveViewStreamForm->HandleLiveViewButtonsHotKeyFunctions(gcnew System::Windows::Forms::KeyPressEventArgs(L'Q'));

					// Enable periodic shutter calibration
					StartupSequenceDone = RMH_ThermalViewer_AutoStartEnablePeriodicCalibration();

				}

				// Close the "please wait" window
				this->CloseStartupWaitWindow();

				if (StartupSequenceDone == true) {

#if _AutoStart_ShowPopupWindows
					// Everything is done - show the "ready to use" window
					System::Windows::Forms::Form^ ReadyWindow = this->CreateStartupInfoWindow(
						"READY TO USE",
						System::String::Format("The thermal camera is connected and calibrated.\nAutomatic shutter calibration is enabled (every {0} seconds).\n\nYou can start measuring now.", _AutoStart_PeriodicCalibrationPeriodSec),
						System::Drawing::Color::FromArgb(0, 130, 40), false, true);
					ReadyWindow->ShowDialog(this);
#endif

				}
				else {

					// The camera was lost during the start-up sequence
					this->ShowStartupCameraNotConnectedWindow();

				}

			}

		}

		private: System::Void MainGUI_Shown(System::Object^ sender, System::EventArgs^ e) {

			// This routine is the start-up function of the application GUI
			// which takes the relevant GUI components as input arguments

			// Write GUI start message
			RMH_Winforms_RichTextBox_WriteLine(this->GUIInfoTextArea, _CustomBuild_StartupMessage, _StatusMessageType_Success);

			RMH_Winforms_RichTextBox_WriteLine(this->GUIInfoTextArea, "Sellect A Thermal Camera Or Mode, In The Settings Menu, & Press The 'Connect' or 'Open File' Button.", _StatusMessageType_Normal);

			// Crash recovery: look for data logging session files that were left in the temporary folder by a crash, a power loss or a killed process.
			// The window (save / open / delete / decide later) is shown before the automatic start-up sequence below, so that it does not appear over the "please wait" window.
			try {

				System::String^ RecoveredFilePath = nullptr;
				System::String^ RecoveredDelimiter = nullptr;

				if (RMH_ThermalViewer_CheckForUnsavedDataLoggingSessions(this, RecoveredFilePath, RecoveredDelimiter) == true) {

					// Open: show the file in the Data Log window. The window is opened as a separate window (undocked), so the automatic start-up sequence
					// (which switches the main view to the stream window) does not replace it.
					ManagedLocals::DataLogGUIForm->OpenRecoveredSession(RecoveredFilePath, RecoveredDelimiter);

					HandleFormsOpeningDockingAndUndocking(ManagedLocals::DataLogGUIForm, this->MainViewTopPanel, &isDataLogFormOpen, &isDataLogFormDocked, &isDataLogFormUndocked, _FormDockingState_UndockForm);

				}

			}
			catch (System::Exception^) {}

			// Start the automatic start-up sequence (select camera, connect, calibrate, enable periodic calibration)
#if _AutoStart_Enabled
#if _AutoStart_ShowPopupWindows
			// Show the "custom version - please wait" window while the camera initialises
			this->StartupWaitWindow = this->CreateStartupInfoWindow(
				"Custom Version For The InfiRay T2S+ V2",
				"This is a modified version of OpenRMH-IRCAM for the InfiRay T2S+ V2 (T2S+_A2) thermal camera.\n\nPlease wait while the camera initialises and calibrates.\nDo not unplug the camera. This window closes automatically.",
				System::Drawing::Color::FromArgb(0, 90, 160), true, false);
			this->CenterStartupWindowOnMainWindow(this->StartupWaitWindow);
			this->StartupWaitWindow->Show(this);
#endif
			this->StartupAutomationStage = 0;
			this->StartupAutomationTimer = gcnew System::Windows::Forms::Timer();
			this->StartupAutomationTimer->Interval = _AutoStart_ConnectDelayMs;
			this->StartupAutomationTimer->Tick += gcnew System::EventHandler(this, &MainGUI::StartupAutomationTimer_Tick);
			this->StartupAutomationTimer->Start();
#endif

		}

		// Main GUI shutdown callback routine ->
		private: System::Void MainGUI_FormClosing(System::Object^ sender, System::Windows::Forms::FormClosingEventArgs^ e) {

			// This routine is the shutdown routine of the application GUI

			// Data logging is still running - ask whether the application is really closed
			if (DataLoggingIsRunningFlag == true) {

				if (e->CloseReason == System::Windows::Forms::CloseReason::WindowsShutDown) {

					// Windows is shutting down - no question is asked, the session is only stopped
					RMH_ThermalViewer_StopDataLogging();

				}
				else {

					System::Windows::Forms::DialogResult CloseAnswer = System::Windows::Forms::MessageBox::Show(
						"Data logging is still running.\n\nIf you close the application now, the data logging session is stopped. " +
						"You are then asked whether the session is saved or deleted.\n\nAre you sure you want to close the application?",
						"Data Logging Is Running", System::Windows::Forms::MessageBoxButtons::YesNo, System::Windows::Forms::MessageBoxIcon::Warning, System::Windows::Forms::MessageBoxDefaultButton::Button2);

					// No - the application stays open and the logging continues
					if (CloseAnswer != System::Windows::Forms::DialogResult::Yes) { e->Cancel = true; return; }

					// Yes - stop the session and wait (up to 3 seconds) for the logging thread to finish. Its completed event asks whether the session is saved
					// (the file in the temporary folder is then saved or deleted, so no temporary file is left behind)
					RMH_ThermalViewer_StopDataLogging();

					int WaitedMilliSec = 0;
					while (GlobalVariables::GlobalDataLoggingThread != nullptr && GlobalVariables::GlobalDataLoggingThread->IsBusy == true && WaitedMilliSec < 3000) {

						System::Windows::Forms::Application::DoEvents();
						System::Threading::Thread::Sleep(25);
						WaitedMilliSec = WaitedMilliSec + 25;

					}

					// Safety: ask about the session file if the completed event did not do it (this does nothing when the file is already saved or deleted)
					RMH_ThermalViewer_FinishDataLoggingSession();

				}

			}

			// Update the thermal camera form session parameters into global variables
			ManagedLocals::ThermalCameraGUIForm->SaveFormSessionSettings();

			// Save the current application session parameters for the next session
			RMH_Application_SaveLastSessionConfigToFile();

		}

		// -------------------- Main GUI Keyboard Key-Press Event Callback Routine -------------------- //
		
		// Main GUI Key-Press Event Callback Routine ->
		private: System::Void MainGUI_KeyPress(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e) {

			// Check whether the live view stream, the 3D surface plot or the temp plot is in view
			if (isLiveViewStreamFormOpen == true || isTempMeasurementsFormOpen == true  || isSurfacePlotFormOpen == true ) {

				// Handle the hotkeys of the live view function buttons 
				ManagedLocals::LiveViewStreamForm->HandleLiveViewButtonsHotKeyFunctions(e);

			}

		}

		// --------------------------- GUI Menu/Sub-Menu Callback Routines ---------------------------- //

		// Thermal camera menu button callback ->
		private: System::Void ThermalCAMMenuButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Handle docking of the form GUI in the main view panel of the main GUI
			HandleFormsOpeningDockingAndUndocking(ManagedLocals::ThermalCameraGUIForm, this->MainViewTopPanel, &isThermalCameraFormOpen, &isThermalCameraFormDocked, &isThermalCameraFormUndocked, _FormDockingState_DockForm);

		}

		// Thermal camera undock button callback ->
		private: System::Void ThermalCAMUndockButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Handle undocking of the form GUI
			HandleFormsOpeningDockingAndUndocking(ManagedLocals::ThermalCameraGUIForm, this->MainViewTopPanel, &isThermalCameraFormOpen, &isThermalCameraFormDocked, &isThermalCameraFormUndocked, _FormDockingState_UndockForm);

		}

	    // Live view menu button callback ->
		private: System::Void LiveViewMenuButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Handle docking of the form GUI in the main view panel of the main GUI
			HandleFormsOpeningDockingAndUndocking(ManagedLocals::LiveViewStreamForm, this->MainViewTopPanel, &isLiveViewStreamFormOpen, &isLiveViewStreamFormDocked, &isLiveViewStreamFormUndocked, _FormDockingState_DockForm);

			// Garbage collect the application
			GC::Collect();

		}

		// Live view undock button callback ->
		private: System::Void LiveViewUndockButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Handle undocking of the form GUI
			HandleFormsOpeningDockingAndUndocking(ManagedLocals::LiveViewStreamForm, this->MainViewTopPanel, &isLiveViewStreamFormOpen, &isLiveViewStreamFormDocked, &isLiveViewStreamFormUndocked, _FormDockingState_UndockForm);
			
			// Garbage collect the application
			GC::Collect();

		}

	    // Surface plot menu button callback ->
		private: System::Void SurfacePlotMenuButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Handle docking of the form GUI in the main view panel of the main GUI
			HandleFormsOpeningDockingAndUndocking(ManagedLocals::SurfacePlotGUIForm, this->MainViewTopPanel, &isSurfacePlotFormOpen, &isSurfacePlotFormDocked, &isSurfacePlotFormUndocked, _FormDockingState_DockForm);

			// Garbage collect the application
			GC::Collect();

		}

		// Surface plot undock button callback ->
		private: System::Void SurfacePlotUndockButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Handle undocking of the form GUI
			HandleFormsOpeningDockingAndUndocking(ManagedLocals::SurfacePlotGUIForm, this->MainViewTopPanel, &isSurfacePlotFormOpen, &isSurfacePlotFormDocked, &isSurfacePlotFormUndocked, _FormDockingState_UndockForm);

			// Garbage collect the application
			GC::Collect();

		}

	    // Temperature measurements menu button callback ->
		private: System::Void TempMeasMenuButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Handle docking of the form GUI in the main view panel of the main GUI
			HandleFormsOpeningDockingAndUndocking(ManagedLocals::TempMeasurementsGUIForm, this->MainViewTopPanel, &isTempMeasurementsFormOpen, &isTempMeasurementsFormDocked, &isTempMeasurementsFormUndocked, _FormDockingState_DockForm);
			
		}

		// Temperature measurements undock button callback ->
		private: System::Void TempMeasUndockButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Handle undocking of the form GUI
			HandleFormsOpeningDockingAndUndocking(ManagedLocals::TempMeasurementsGUIForm, this->MainViewTopPanel, &isTempMeasurementsFormOpen, &isTempMeasurementsFormDocked, &isTempMeasurementsFormUndocked, _FormDockingState_UndockForm);

		}

		// Emissivity table menu button callback ->
		private: System::Void EmissivityMenuButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Handle docking of the form GUI in the main view panel of the main GUI
			HandleFormsOpeningDockingAndUndocking(ManagedLocals::EmissivityTableGUIForm, this->MainViewTopPanel, &isEmissivityTableFormOpen, &isEmissivityTableFormDocked, &isEmissivityTableFormUndocked, _FormDockingState_DockForm);

		}

		// Emissivity table undock button callback ->
		private: System::Void EmissivityUndockButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Handle undocking of the form GUI
			HandleFormsOpeningDockingAndUndocking(ManagedLocals::EmissivityTableGUIForm, this->MainViewTopPanel, &isEmissivityTableFormOpen, &isEmissivityTableFormDocked, &isEmissivityTableFormUndocked, _FormDockingState_UndockForm);

		}

		// ---------------------------- Data Log menu item ---------------------------- //

		private: System::Windows::Forms::TableLayoutPanel^ DataLogMenuPanel;
		private: System::Windows::Forms::Button^ DataLogMenuButton;

		// Session menu (menu bar at the top of the window) and the file of the current session
		private: System::Windows::Forms::MenuStrip^ SessionMenuStrip;
		private: System::String^ CurrentSessionPath;
		private: System::String^ SessionSaveNote;
		private: System::Windows::Forms::Button^ DataLogUndockButton;


		// ------------------------------------ Session Files (Session Menu) ------------------------------------ //

		// A session file holds the settings of the whole program: the application configuration (colors, paths, ...), the value of every setting control of the
		// forms (thermal camera, live view tools, temperature plots, 3D plot, emissivity table), the Data Log graph settings and the window / page that was shown.
		// See SessionFile.h for the file layout. Measurements that are placed on the live view (crosshairs, ROIs, lines), the logged data and the video / snapshot
		// files are not part of a session file.

		private: System::Void BuildSessionMenu() {

			SessionMenuColors^ MenuColors = gcnew SessionMenuColors();
			System::Windows::Forms::ToolStripProfessionalRenderer^ MenuRenderer = gcnew System::Windows::Forms::ToolStripProfessionalRenderer(MenuColors);
			MenuRenderer->RoundedEdges = false;

			this->SessionMenuStrip = gcnew System::Windows::Forms::MenuStrip();
			this->SessionMenuStrip->Name = "SessionMenuStrip";
			this->SessionMenuStrip->Dock = System::Windows::Forms::DockStyle::Top;
			this->SessionMenuStrip->GripStyle = System::Windows::Forms::ToolStripGripStyle::Hidden;
			this->SessionMenuStrip->Renderer = MenuRenderer;
			this->SessionMenuStrip->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);
			this->SessionMenuStrip->ForeColor = System::Drawing::Color::White;

			System::Windows::Forms::ToolStripMenuItem^ SessionItem = gcnew System::Windows::Forms::ToolStripMenuItem("Session");
			System::Windows::Forms::ToolStripMenuItem^ OpenItem = gcnew System::Windows::Forms::ToolStripMenuItem("Open Session...");
			System::Windows::Forms::ToolStripMenuItem^ SaveItem = gcnew System::Windows::Forms::ToolStripMenuItem("Save Session");
			System::Windows::Forms::ToolStripMenuItem^ SaveAsItem = gcnew System::Windows::Forms::ToolStripMenuItem("Save Session As...");
			System::Windows::Forms::ToolStripMenuItem^ FolderItem = gcnew System::Windows::Forms::ToolStripMenuItem("Open Sessions Folder");

			OpenItem->ShortcutKeys = (System::Windows::Forms::Keys)((int)System::Windows::Forms::Keys::Control | (int)System::Windows::Forms::Keys::O);
			SaveItem->ShortcutKeys = (System::Windows::Forms::Keys)((int)System::Windows::Forms::Keys::Control | (int)System::Windows::Forms::Keys::S);
			SaveAsItem->ShortcutKeys = (System::Windows::Forms::Keys)((int)System::Windows::Forms::Keys::Control | (int)System::Windows::Forms::Keys::Shift | (int)System::Windows::Forms::Keys::S);

			OpenItem->Click += gcnew System::EventHandler(this, &MainGUI::SessionOpenMenuItem_Click);
			SaveItem->Click += gcnew System::EventHandler(this, &MainGUI::SessionSaveMenuItem_Click);
			SaveAsItem->Click += gcnew System::EventHandler(this, &MainGUI::SessionSaveAsMenuItem_Click);
			FolderItem->Click += gcnew System::EventHandler(this, &MainGUI::SessionFolderMenuItem_Click);

			System::Windows::Forms::ToolStripSeparator^ SessionSeparator = gcnew System::Windows::Forms::ToolStripSeparator();
			SessionSeparator->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);
			SessionSeparator->ForeColor = System::Drawing::Color::White;

			cli::array<System::Windows::Forms::ToolStripItem^>^ DropDownItems = gcnew cli::array<System::Windows::Forms::ToolStripItem^>(5) { OpenItem, SaveItem, SaveAsItem, SessionSeparator, FolderItem };

			for (int i = 0; i < DropDownItems->Length; i++) {
				DropDownItems[i]->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);
				DropDownItems[i]->ForeColor = System::Drawing::Color::White;
				SessionItem->DropDownItems->Add(DropDownItems[i]);
			}

			SessionItem->BackColor = System::Drawing::Color::FromArgb(35, 35, 35);
			SessionItem->ForeColor = System::Drawing::Color::White;
			this->SessionMenuStrip->Items->Add(SessionItem);

			// Added last, so the menu bar is docked first (at the very top of the window)
			this->MainMenuStrip = this->SessionMenuStrip;
			this->Controls->Add(this->SessionMenuStrip);

		}

		private: System::String^ GetSessionFolder() {

			// Default folder of the session files: "IRCAM Thermal Viewer Sessions" in the Documents folder (created when it does not exist)
			System::String^ Folder = System::IO::Path::Combine(System::Environment::GetFolderPath(System::Environment::SpecialFolder::MyDocuments), "IRCAM Thermal Viewer Sessions");

			try { System::IO::Directory::CreateDirectory(Folder); }
			catch (System::Exception^) { Folder = System::Environment::GetFolderPath(System::Environment::SpecialFolder::MyDocuments); }

			return Folder;

		}

		private: System::Void WriteSessionStatus(System::String^ Text, unsigned int MessageType) {

			RMH_Winforms_RichTextBox_WriteLine(this->GUIInfoTextArea, RMH_Conversion_SystemStringToStdString(Text), MessageType);

		}

		private: System::Void UpdateSessionTitleBar() {

			// The name of the session file is shown in the title bar
			std::string Title = ApplicationInformationString + _CustomBuild_TitleSuffix;

			if (this->CurrentSessionPath != nullptr) {
				Title = Title + std::string(" - Session: ") + RMH_Conversion_SystemStringToStdString(System::IO::Path::GetFileNameWithoutExtension(this->CurrentSessionPath));
			}

			RMH_Winforms_ChangeFormTitleBarText(this, Title);

		}

		private: System::String^ GetShownPageName() {

			// The page (left menu item) that is shown in the main window now, "" when none of the pages that a session file brings back is shown
			if (isLiveViewStreamFormOpen == true && isLiveViewStreamFormUndocked == false) { return "LiveView"; }
			if (isThermalCameraFormOpen == true && isThermalCameraFormUndocked == false) { return "ThermalCamera"; }
			if (isTempMeasurementsFormOpen == true && isTempMeasurementsFormUndocked == false) { return "TempMeas"; }
			if (isSurfacePlotFormOpen == true && isSurfacePlotFormUndocked == false) { return "SurfacePlot"; }
			if (isDataLogFormOpen == true && isDataLogFormUndocked == false) { return "DataLog"; }
			if (isEmissivityTableFormOpen == true && isEmissivityTableFormUndocked == false) { return "Emissivity"; }
			return "";

		}

		private: System::Windows::Forms::Button^ GetPageMenuButton(System::String^ PageName) {

			if (PageName == "LiveView") { return this->LiveViewMenuButton; }
			if (PageName == "ThermalCamera") { return this->ThermalCAMMenuButton; }
			if (PageName == "TempMeas") { return this->TempMeasMenuButton; }
			if (PageName == "SurfacePlot") { return this->SurfacePlotMenuButton; }
			if (PageName == "DataLog") { return this->DataLogMenuButton; }
			if (PageName == "Emissivity") { return this->EmissivityMenuButton; }
			return nullptr;

		}

		// The six pages of the main window that can be pulled out into a window of their own (names as in the session file, see GetShownPageName)
		private: cli::array<System::String^>^ GetSessionPageNames() { return gcnew cli::array<System::String^>{ "LiveView", "ThermalCamera", "TempMeas", "SurfacePlot", "DataLog", "Emissivity" }; }

		private: System::Windows::Forms::Form^ GetPageFormForSession(System::String^ PageName) {

			if (PageName == "LiveView") { return ManagedLocals::LiveViewStreamForm; }
			if (PageName == "ThermalCamera") { return ManagedLocals::ThermalCameraGUIForm; }
			if (PageName == "TempMeas") { return ManagedLocals::TempMeasurementsGUIForm; }
			if (PageName == "SurfacePlot") { return ManagedLocals::SurfacePlotGUIForm; }
			if (PageName == "DataLog") { return ManagedLocals::DataLogGUIForm; }
			if (PageName == "Emissivity") { return ManagedLocals::EmissivityTableGUIForm; }
			return nullptr;

		}

		// True when the page is shown in a window of its own (undocked)
		private: bool IsPageUndockedForSession(System::String^ PageName) {

			if (PageName == "LiveView") { return (isLiveViewStreamFormOpen == true && isLiveViewStreamFormUndocked == true); }
			if (PageName == "ThermalCamera") { return (isThermalCameraFormOpen == true && isThermalCameraFormUndocked == true); }
			if (PageName == "TempMeas") { return (isTempMeasurementsFormOpen == true && isTempMeasurementsFormUndocked == true); }
			if (PageName == "SurfacePlot") { return (isSurfacePlotFormOpen == true && isSurfacePlotFormUndocked == true); }
			if (PageName == "DataLog") { return (isDataLogFormOpen == true && isDataLogFormUndocked == true); }
			if (PageName == "Emissivity") { return (isEmissivityTableFormOpen == true && isEmissivityTableFormUndocked == true); }
			return false;

		}

		// Pulls the page out into a window of its own (the same as its Undock button)
		private: System::Void UndockPageForSession(System::String^ PageName) {

			if (PageName == "LiveView") { this->LiveViewUndockButton_Click(nullptr, nullptr); }
			else if (PageName == "ThermalCamera") { this->ThermalCAMUndockButton_Click(nullptr, nullptr); }
			else if (PageName == "TempMeas") { this->TempMeasUndockButton_Click(nullptr, nullptr); }
			else if (PageName == "SurfacePlot") { this->SurfacePlotUndockButton_Click(nullptr, nullptr); }
			else if (PageName == "DataLog") { this->DataLogUndockButton_Click(nullptr, nullptr); }
			else if (PageName == "Emissivity") { this->EmissivityUndockButton_Click(nullptr, nullptr); }

		}

		// True when the rectangle is on one of the connected screens (a session file can come from a computer with other monitors)
		private: bool IsRectangleOnScreenForSession(System::Drawing::Rectangle WantedBounds) {

			for (int i = 0; i < System::Windows::Forms::Screen::AllScreens->Length; i++) {
				if (System::Windows::Forms::Screen::AllScreens[i]->WorkingArea.IntersectsWith(WantedBounds) == true) { return true; }
			}
			return false;

		}

		private: bool TextToRectangleForSession(System::String^ Text, System::Drawing::Rectangle% Result) {

			cli::array<System::String^>^ Parts = Text->Split(',');
			int X = 0, Y = 0, W = 0, H = 0;

			if (Parts->Length == 4 && SessionFile::TextToInt(Parts[0], X) && SessionFile::TextToInt(Parts[1], Y) && SessionFile::TextToInt(Parts[2], W) && SessionFile::TextToInt(Parts[3], H) && W > 200 && H > 200) {
				Result = System::Drawing::Rectangle(X, Y, W, H);
				return true;
			}

			return false;

		}

		private: System::Windows::Forms::Form^ GetLiveViewToolsFormForSession() {

			// The live view tools form is created inside the live view form. It is found through one of its own controls
			if (GlobalVariables::GlobalColorPaletteComboBox == nullptr) { return nullptr; }
			return GlobalVariables::GlobalColorPaletteComboBox->FindForm();

		}

		// Saves the session to a file. Returns an empty text when it worked, else the error text
		private: System::String^ SaveSessionToFile(System::String^ FilePath) {

			try {

				System::Collections::Generic::SortedDictionary<System::String^, System::String^>^ Values = gcnew System::Collections::Generic::SortedDictionary<System::String^, System::String^>();

				// Update the session parameters that live in the thermal camera form into the global variables
				ManagedLocals::ThermalCameraGUIForm->SaveFormSessionSettings();

				Values["meta.format"] = SessionFile::FormatVersion;
				Values["meta.program"] = RMH_Conversion_StdStringToSystemString(ApplicationInformationString);
				Values["meta.saved"] = System::DateTime::Now.ToString("yyyy-MM-dd HH:mm:ss", System::Globalization::CultureInfo::InvariantCulture);

				// Application configuration
				std::vector<std::string> ConfigLines = RMH_Application_BuildSessionConfigLines();
				for (int i = 0; i < (int)ConfigLines.size(); i++) {
					Values["app." + i.ToString("000", System::Globalization::CultureInfo::InvariantCulture)] = RMH_Conversion_StdStringToSystemString(ConfigLines[i]);
				}

				// Window and page
				Values["ui.WindowState"] = this->WindowState.ToString();
				Values["ui.Bounds"] = this->RestoreBounds.X.ToString() + "," + this->RestoreBounds.Y.ToString() + "," + this->RestoreBounds.Width.ToString() + "," + this->RestoreBounds.Height.ToString();
				Values["ui.Page"] = this->GetShownPageName();

				// Value controls of the forms
				SessionFile::CollectControls(ManagedLocals::ThermalCameraGUIForm, "ThermalCameraGUI", Values);
				SessionFile::CollectControls(ManagedLocals::TempMeasurementsGUIForm, "TempMeasGUI", Values);
				SessionFile::CollectControls(ManagedLocals::SurfacePlotGUIForm, "SurfacePlotGUI", Values);
				SessionFile::CollectControls(ManagedLocals::EmissivityTableGUIForm, "EmissivityTableGUI", Values);
				SessionFile::CollectControls(ManagedLocals::LiveViewStreamForm, "LiveViewStream", Values);
				SessionFile::CollectControls(this->GetLiveViewToolsFormForSession(), "LiveViewTools", Values);

				// Data Log graph settings, markers and cursors
				ManagedLocals::DataLogGUIForm->GetSessionSettings(Values);

				// Logged data: the log file that the Data Log window shows is stored inside the session file (compressed). A file that is bigger than
				// SessionFile::MaxEmbeddedFileBytes is only referred to by its path.
				this->SessionSaveNote = "";
				System::String^ LogFilePath = ManagedLocals::DataLogGUIForm->GetSessionLogFilePath();

				if (LogFilePath != nullptr) {

					long long LogFileLength = (gcnew System::IO::FileInfo(LogFilePath))->Length;

					if (LogFileLength <= SessionFile::MaxEmbeddedFileBytes) {
						Values["datalog.file.delimiter"] = ManagedLocals::DataLogGUIForm->GetSessionLogDelimiter();
						Values["datalog.file.gz"] = SessionFile::CompressToBase64(SessionFile::ReadSharedFile(LogFilePath));
					}
					else {
						Values["datalog.file.path"] = LogFilePath;
						this->SessionSaveNote = " The logged data is too big to be stored in the session file: only the path of its file was saved.";
					}

				}

				// Window layout: the pages that are pulled out into a window of their own, with the position and size of the window
				cli::array<System::String^>^ PageNames = this->GetSessionPageNames();

				for (int i = 0; i < PageNames->Length; i++) {

					System::Windows::Forms::Form^ PageForm = this->GetPageFormForSession(PageNames[i]);
					bool Undocked = this->IsPageUndockedForSession(PageNames[i]);

					Values["ui.win." + PageNames[i] + ".Undocked"] = SessionFile::BoolText(Undocked);

					if (Undocked == true && PageForm != nullptr) {
						System::Drawing::Rectangle WindowBounds = (PageForm->WindowState == System::Windows::Forms::FormWindowState::Normal) ? PageForm->Bounds : PageForm->RestoreBounds;
						Values["ui.win." + PageNames[i] + ".Bounds"] = WindowBounds.X.ToString() + "," + WindowBounds.Y.ToString() + "," + WindowBounds.Width.ToString() + "," + WindowBounds.Height.ToString();
					}

				}

				// Live view measurements: temperature crosshairs, temperature lines and ROIs
				System::Collections::Generic::List<System::String^>^ LiveViewEntries = RMH_ThermalViewer_GetLiveViewMeasurementsForSession();
				Values["lv.count"] = LiveViewEntries->Count.ToString(System::Globalization::CultureInfo::InvariantCulture);
				for (int i = 0; i < LiveViewEntries->Count; i++) { Values["lv." + i.ToString("000", System::Globalization::CultureInfo::InvariantCulture)] = LiveViewEntries[i]; }

				// Data Set Source of every plot data set (Temperature Plot Data Set Settings) and the tracking buttons of the live view (maximum, minimum, center, mouse cursor)
				System::Collections::Generic::List<System::String^>^ SourceEntries = RMH_ThermalViewer_GetPlotDataSetSourcesForSession();
				Values["plot.ds.count"] = SourceEntries->Count.ToString(System::Globalization::CultureInfo::InvariantCulture);
				for (int i = 0; i < SourceEntries->Count; i++) { Values["plot.ds." + i.ToString("000", System::Globalization::CultureInfo::InvariantCulture)] = SourceEntries[i]; }

				System::Collections::Generic::List<System::String^>^ TrackEntries = RMH_ThermalViewer_GetTemperatureTrackingForSession();
				Values["track.count"] = TrackEntries->Count.ToString(System::Globalization::CultureInfo::InvariantCulture);
				for (int i = 0; i < TrackEntries->Count; i++) { Values["track." + i.ToString("000", System::Globalization::CultureInfo::InvariantCulture)] = TrackEntries[i]; }

				return SessionFile::WriteFile(FilePath, Values);

			}
			catch (System::Exception^ Ex) { return Ex->Message; }

		}

		// Opens a session file and sets the program to it
		private: System::Void OpenSessionFromFile(System::String^ FilePath) {

			// The settings that run a data logging session cannot be changed while it logs
			if (DataLoggingIsRunningFlag == true) {
				System::Windows::Forms::MessageBox::Show(this, "Data logging is running. Stop the data logging before a session is opened.", "Open Session", System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Information);
				return;
			}

			System::Collections::Generic::Dictionary<System::String^, System::String^>^ Values = gcnew System::Collections::Generic::Dictionary<System::String^, System::String^>();
			System::String^ ErrorText = SessionFile::ReadFile(FilePath, Values);

			if (ErrorText->Length > 0) {
				System::Windows::Forms::MessageBox::Show(this, "The session file could not be opened:\n" + ErrorText, "Open Session", System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Warning);
				this->WriteSessionStatus("Session File Could Not Be Opened!", _StatusMessageType_Error);
				return;
			}

			System::Windows::Forms::Cursor::Current = System::Windows::Forms::Cursors::WaitCursor;

			int SetCount = 0;
			int SkippedCount = 0;

			try {

				// Application configuration (only when the file has the same number of lines as this program version writes)
				std::vector<std::string> ConfigLines;
				for (int i = 0; i < 1000; i++) {
					System::String^ Line = nullptr;
					if (Values->TryGetValue("app." + i.ToString("000", System::Globalization::CultureInfo::InvariantCulture), Line) == false) { break; }
					ConfigLines.push_back(RMH_Conversion_SystemStringToStdString(Line));
				}

				std::vector<std::string> CurrentLines = RMH_Application_BuildSessionConfigLines();

				if (ConfigLines.size() == CurrentLines.size()) {

					RMH_Application_ApplySessionConfigLines(ConfigLines, true);

					// The 2D plot line colors are shown by the color panels of the thermal camera form
					RMH_ThermalViewer_Load2DPlotSavedSessionLineColorData();

					// Settings that a button of the live view tools switches (the button is clicked only when its setting is different)
					if (RMH_Conversion_StdStringToBoolean(ConfigLines[4]) != FixedLiveViewAspectRatio && GlobalVariables::GlobalFixedAspectRatioButton != nullptr && GlobalVariables::GlobalFixedAspectRatioButton->Enabled == true) { GlobalVariables::GlobalFixedAspectRatioButton->PerformClick(); }
					if (RMH_Conversion_StdStringToBoolean(ConfigLines[6]) != EnhancedResEnableFlag && GlobalVariables::GlobalEnhancedResButton != nullptr && GlobalVariables::GlobalEnhancedResButton->Enabled == true) { GlobalVariables::GlobalEnhancedResButton->PerformClick(); }
					if (RMH_Conversion_StdStringToBoolean(ConfigLines[78]) != UltraResolutionEnableFlag && GlobalVariables::GlobalUltraResolutionButton != nullptr && GlobalVariables::GlobalUltraResolutionButton->Enabled == true) { GlobalVariables::GlobalUltraResolutionButton->PerformClick(); }

				}
				else {
					this->WriteSessionStatus("Session File: The Application Configuration Has Another Layout And Was Not Set!", _StatusMessageType_Warning);
				}

				// Value controls of the forms (the live view tools first, they hold the color palettes)
				SetCount += SessionFile::ApplyControls(this->GetLiveViewToolsFormForSession(), "LiveViewTools", Values, SkippedCount);
				SetCount += SessionFile::ApplyControls(ManagedLocals::LiveViewStreamForm, "LiveViewStream", Values, SkippedCount);
				SetCount += SessionFile::ApplyControls(ManagedLocals::ThermalCameraGUIForm, "ThermalCameraGUI", Values, SkippedCount);
				SetCount += SessionFile::ApplyControls(ManagedLocals::TempMeasurementsGUIForm, "TempMeasGUI", Values, SkippedCount);
				SetCount += SessionFile::ApplyControls(ManagedLocals::SurfacePlotGUIForm, "SurfacePlotGUI", Values, SkippedCount);
				SetCount += SessionFile::ApplyControls(ManagedLocals::EmissivityTableGUIForm, "EmissivityTableGUI", Values, SkippedCount);

				// Logged data: the log file of the session is written next to the session file and opened in the Data Log window
				System::String^ OpenedLogFilePath = nullptr;
				System::String^ PackedLogData = nullptr;
				System::String^ LogDelimiter = nullptr;
				Values->TryGetValue("datalog.file.delimiter", LogDelimiter);

				if (Values->TryGetValue("datalog.file.gz", PackedLogData) == true) {

					try {

						System::String^ LogTarget = System::IO::Path::Combine(System::IO::Path::GetDirectoryName(FilePath), System::IO::Path::GetFileNameWithoutExtension(FilePath) + "_DataLog.csv");
						System::IO::File::WriteAllBytes(LogTarget, SessionFile::DecompressFromBase64(PackedLogData));
						OpenedLogFilePath = LogTarget;

					}
					catch (System::Exception^ LogEx) { this->WriteSessionStatus("Session File: The Logged Data Could Not Be Restored (" + LogEx->Message + ")", _StatusMessageType_Warning); }

				}
				else if (Values->TryGetValue("datalog.file.path", PackedLogData) == true && System::IO::File::Exists(PackedLogData) == true) { OpenedLogFilePath = PackedLogData; }

				if (OpenedLogFilePath != nullptr) {
					if (LogDelimiter == nullptr || LogDelimiter->Length == 0) { LogDelimiter = GlobalVariables::DataLoggingCSVDelimiterString; }
					ManagedLocals::DataLogGUIForm->OpenSessionLogFile(OpenedLogFilePath, LogDelimiter);
				}
				else {

					// The session has no logged data (it was saved while the graph was empty, or its log file is not there any more): the graph is shown empty, not with the data that
					// was shown before the session was opened
					ManagedLocals::DataLogGUIForm->ShowEmptyGraphForSession();

					if (Values->ContainsKey("datalog.file.path") == true) { this->WriteSessionStatus("Session File: The Log File Of The Session Was Not Found (" + Values["datalog.file.path"] + "), The Graph Is Empty!", _StatusMessageType_Warning); }

				}

				// Data Log graph settings, markers and cursors
				ManagedLocals::DataLogGUIForm->ApplySessionSettings(Values, OpenedLogFilePath);

				// Live view measurements (only when the session file has them)
				System::String^ LiveViewCountText = nullptr;

				if (Values->TryGetValue("lv.count", LiveViewCountText) == true) {

					System::Collections::Generic::List<System::String^>^ LiveViewEntries = gcnew System::Collections::Generic::List<System::String^>();

					for (int i = 0; i < 1000; i++) {
						System::String^ Entry = nullptr;
						if (Values->TryGetValue("lv." + i.ToString("000", System::Globalization::CultureInfo::InvariantCulture), Entry) == false) { break; }
						LiveViewEntries->Add(Entry);
					}

					SetCount += RMH_ThermalViewer_SetLiveViewMeasurementsFromSession(LiveViewEntries);

				}

				// Data Set Source of every plot data set (set again after the controls: a source that depends on the measurements of the live view is then set last)
				if (Values->ContainsKey("plot.ds.count") == true) {

					System::Collections::Generic::List<System::String^>^ SourceEntries = gcnew System::Collections::Generic::List<System::String^>();

					for (int i = 0; i < 1000; i++) {
						System::String^ Entry = nullptr;
						if (Values->TryGetValue("plot.ds." + i.ToString("000", System::Globalization::CultureInfo::InvariantCulture), Entry) == false) { break; }
						SourceEntries->Add(Entry);
					}

					RMH_ThermalViewer_SetPlotDataSetSourcesFromSession(SourceEntries);

				}

				// Tracking buttons of the live view: maximum, minimum, center and mouse cursor temperature
				if (Values->ContainsKey("track.count") == true) {

					System::Collections::Generic::List<System::String^>^ TrackEntries = gcnew System::Collections::Generic::List<System::String^>();

					for (int i = 0; i < 100; i++) {
						System::String^ Entry = nullptr;
						if (Values->TryGetValue("track." + i.ToString("000", System::Globalization::CultureInfo::InvariantCulture), Entry) == false) { break; }
						TrackEntries->Add(Entry);
					}

					SetCount += RMH_ThermalViewer_SetTemperatureTrackingFromSession(TrackEntries);

				}

				// Windows that were pulled out of the main window: pulled out again (when the page is available) and put where they were. A window that is
				// pulled out now but was docked in the session file is left as it is.
				cli::array<System::String^>^ PageNames = this->GetSessionPageNames();

				for (int i = 0; i < PageNames->Length; i++) {

					System::String^ UndockedText = nullptr;
					if (Values->TryGetValue("ui.win." + PageNames[i] + ".Undocked", UndockedText) == false || SessionFile::TextToBool(UndockedText) == false) { continue; }

					System::Windows::Forms::Button^ PageButton = this->GetPageMenuButton(PageNames[i]);

					if (this->IsPageUndockedForSession(PageNames[i]) == false && PageButton != nullptr && PageButton->Enabled == true) { this->UndockPageForSession(PageNames[i]); }

					System::Windows::Forms::Form^ PageForm = this->GetPageFormForSession(PageNames[i]);
					System::String^ WindowBoundsText = nullptr;
					System::Drawing::Rectangle WindowBounds;

					if (PageForm != nullptr && this->IsPageUndockedForSession(PageNames[i]) == true && Values->TryGetValue("ui.win." + PageNames[i] + ".Bounds", WindowBoundsText) == true &&
						this->TextToRectangleForSession(WindowBoundsText, WindowBounds) == true && this->IsRectangleOnScreenForSession(WindowBounds) == true) {

						PageForm->WindowState = System::Windows::Forms::FormWindowState::Normal;
						PageForm->StartPosition = System::Windows::Forms::FormStartPosition::Manual;
						PageForm->Bounds = WindowBounds;

					}

				}

				// Window: size and position (only a normal window; the maximized state is brought back)
				System::String^ WindowStateText = nullptr;
				System::String^ BoundsText = nullptr;

				if (Values->TryGetValue("ui.WindowState", WindowStateText) == true) {

					if (WindowStateText == "Maximized") { this->WindowState = System::Windows::Forms::FormWindowState::Maximized; }
					else if (WindowStateText == "Normal") {

						this->WindowState = System::Windows::Forms::FormWindowState::Normal;

						if (Values->TryGetValue("ui.Bounds", BoundsText) == true) {

							cli::array<System::String^>^ Parts = BoundsText->Split(',');
							int X = 0, Y = 0, W = 0, H = 0;

							if (Parts->Length == 4 && SessionFile::TextToInt(Parts[0], X) && SessionFile::TextToInt(Parts[1], Y) && SessionFile::TextToInt(Parts[2], W) && SessionFile::TextToInt(Parts[3], H) && W > 200 && H > 200) {

								// Only a window that is on a connected screen is brought back (the monitors can differ from the computer of the session file)
								System::Drawing::Rectangle WantedBounds = System::Drawing::Rectangle(X, Y, W, H);
								bool OnScreen = false;
								for (int i = 0; i < System::Windows::Forms::Screen::AllScreens->Length; i++) {
									if (System::Windows::Forms::Screen::AllScreens[i]->WorkingArea.IntersectsWith(WantedBounds) == true) { OnScreen = true; }
								}
								if (OnScreen == true) { this->Bounds = WantedBounds; }

							}

						}

					}

				}

				// Page: the page of the session file is shown (only when it is available and not shown already)
				System::String^ PageName = nullptr;

				if (Values->TryGetValue("ui.Page", PageName) == true && PageName->Length > 0 && PageName != this->GetShownPageName()) {

					System::Windows::Forms::Button^ PageButton = this->GetPageMenuButton(PageName);
					if (PageButton != nullptr && PageButton->Enabled == true) { PageButton->PerformClick(); }

				}

				System::Windows::Forms::Cursor::Current = System::Windows::Forms::Cursors::Default;

				this->CurrentSessionPath = FilePath;
				this->UpdateSessionTitleBar();

				System::String^ Summary = "Session Opened: " + System::IO::Path::GetFileName(FilePath) + " (" + SetCount.ToString() + " Settings Set";
				if (SkippedCount > 0) { Summary = Summary + ", " + SkippedCount.ToString() + " Not Available Now"; }
				Summary = Summary + ").";
				this->WriteSessionStatus(Summary, _StatusMessageType_Success);

			}
			catch (System::Exception^ Ex) {

				System::Windows::Forms::Cursor::Current = System::Windows::Forms::Cursors::Default;
				System::Windows::Forms::MessageBox::Show(this, "The session could not be set completely:\n" + Ex->Message, "Open Session", System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Warning);
				this->WriteSessionStatus("Session Could Not Be Set Completely!", _StatusMessageType_Error);

			}

		}

		private: System::Void SaveSessionAndReport(System::String^ FilePath) {

			System::String^ ErrorText = this->SaveSessionToFile(FilePath);

			if (ErrorText->Length > 0) {
				System::Windows::Forms::MessageBox::Show(this, "The session could not be saved:\n" + ErrorText, "Save Session", System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Warning);
				this->WriteSessionStatus("Session Could Not Be Saved!", _StatusMessageType_Error);
				return;
			}

			this->CurrentSessionPath = FilePath;
			this->UpdateSessionTitleBar();
			this->WriteSessionStatus("Session Saved: " + System::IO::Path::GetFileName(FilePath) + this->SessionSaveNote, _StatusMessageType_Success);

		}

		private: System::Void SessionOpenMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			System::Windows::Forms::OpenFileDialog^ OpenDialog = gcnew System::Windows::Forms::OpenFileDialog();
			OpenDialog->Title = "Open Session";
			OpenDialog->Filter = "IRCAM session (*.ircamsession)|*.ircamsession|All files (*.*)|*.*";
			OpenDialog->InitialDirectory = (this->CurrentSessionPath != nullptr) ? System::IO::Path::GetDirectoryName(this->CurrentSessionPath) : this->GetSessionFolder();

			if (OpenDialog->ShowDialog(this) == System::Windows::Forms::DialogResult::OK) { this->OpenSessionFromFile(OpenDialog->FileName); }

		}

		private: System::Void SessionSaveAsMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			System::Windows::Forms::SaveFileDialog^ SaveDialog = gcnew System::Windows::Forms::SaveFileDialog();
			SaveDialog->Title = "Save Session As";
			SaveDialog->Filter = "IRCAM session (*.ircamsession)|*.ircamsession";
			SaveDialog->DefaultExt = "ircamsession";
			SaveDialog->AddExtension = true;
			SaveDialog->OverwritePrompt = true;

			if (this->CurrentSessionPath != nullptr) {
				SaveDialog->InitialDirectory = System::IO::Path::GetDirectoryName(this->CurrentSessionPath);
				SaveDialog->FileName = System::IO::Path::GetFileName(this->CurrentSessionPath);
			}
			else {
				SaveDialog->InitialDirectory = this->GetSessionFolder();
				SaveDialog->FileName = "Session_" + System::DateTime::Now.ToString("ddMMMyyyy_HH-mm-ss", System::Globalization::CultureInfo::InvariantCulture);
			}

			if (SaveDialog->ShowDialog(this) == System::Windows::Forms::DialogResult::OK) { this->SaveSessionAndReport(SaveDialog->FileName); }

		}

		private: System::Void SessionSaveMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			// Without a session file the first save asks for a name
			if (this->CurrentSessionPath == nullptr) { this->SessionSaveAsMenuItem_Click(sender, e); return; }

			this->SaveSessionAndReport(this->CurrentSessionPath);

		}

		private: System::Void SessionFolderMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

			try { System::Diagnostics::Process::Start("explorer.exe", "\"" + ((this->CurrentSessionPath != nullptr) ? System::IO::Path::GetDirectoryName(this->CurrentSessionPath) : this->GetSessionFolder()) + "\""); }
			catch (System::Exception^) {}

		}

		private: System::Void AddDataLogMenuItem() {

			// This routine adds a "Data Log" item (menu button and undock button) to the left menu, between "Plots" and "Emissivity"
			// The look of the item is copied from the "Plots" item

			// Panel for the new item - same layout as the panel of the "Plots" item
			this->DataLogMenuPanel = gcnew System::Windows::Forms::TableLayoutPanel();
			this->DataLogMenuPanel->Dock = System::Windows::Forms::DockStyle::Fill;
			this->DataLogMenuPanel->Margin = System::Windows::Forms::Padding(0);
			this->DataLogMenuPanel->ColumnCount = 2;
			this->DataLogMenuPanel->RowCount = 1;
			// The column widths are copied from the "Plots" item panel as they are NOW (after the automatic scaling of the main GUI)
			// Fixed values do not get scaled for controls that are created in code, which made the button too narrow and the text wrap over the icon
			for (int i = 0; i < this->tableLayoutPanel5->ColumnStyles->Count; i++) {
				System::Windows::Forms::ColumnStyle^ SourceStyle = this->tableLayoutPanel5->ColumnStyles[i];
				this->DataLogMenuPanel->ColumnStyles->Add(gcnew System::Windows::Forms::ColumnStyle(SourceStyle->SizeType, SourceStyle->Width));
			}
			this->DataLogMenuPanel->RowStyles->Add(gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 100.0f));

			// Icon of the menu button - a small table, drawn in code
			System::Drawing::Bitmap^ MenuIcon = gcnew System::Drawing::Bitmap(32, 32);
			System::Drawing::Graphics^ IconGraphics = System::Drawing::Graphics::FromImage(MenuIcon);
			IconGraphics->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;
			IconGraphics->Clear(System::Drawing::Color::Transparent);
			System::Drawing::SolidBrush^ IconHeaderBrush = gcnew System::Drawing::SolidBrush(System::Drawing::Color::FromArgb(0, 150, 220));
			System::Drawing::Pen^ IconPen = gcnew System::Drawing::Pen(System::Drawing::Color::FromArgb(235, 235, 235), 2.0f);
			IconGraphics->FillRectangle(IconHeaderBrush, 4, 5, 25, 6);
			IconGraphics->DrawRectangle(IconPen, 3, 4, 26, 24);
			IconGraphics->DrawLine(IconPen, 3, 11, 29, 11);
			IconGraphics->DrawLine(IconPen, 3, 18, 29, 18);
			IconGraphics->DrawLine(IconPen, 12, 4, 12, 28);
			IconGraphics->DrawLine(IconPen, 21, 4, 21, 28);
			delete IconPen;
			delete IconHeaderBrush;
			delete IconGraphics;

			// Menu button - copy of the look of the "Plots" menu button
			this->DataLogMenuButton = gcnew System::Windows::Forms::Button();
			this->DataLogMenuButton->Dock = System::Windows::Forms::DockStyle::Fill;
			this->DataLogMenuButton->Margin = System::Windows::Forms::Padding(0);
			this->DataLogMenuButton->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->DataLogMenuButton->Font = (System::Drawing::Font^)this->TempMeasMenuButton->Font->Clone();
			this->DataLogMenuButton->ForeColor = System::Drawing::Color::White;
			this->DataLogMenuButton->BackColor = System::Drawing::Color::Transparent;
			this->DataLogMenuButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(35, 35, 35);
			this->DataLogMenuButton->FlatAppearance->MouseDownBackColor = System::Drawing::Color::FromArgb(70, 70, 70);
			this->DataLogMenuButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(50, 50, 50);
			this->DataLogMenuButton->Image = MenuIcon;
			this->DataLogMenuButton->ImageAlign = System::Drawing::ContentAlignment::TopCenter;
			this->DataLogMenuButton->TextAlign = System::Drawing::ContentAlignment::BottomCenter;
			this->DataLogMenuButton->TextImageRelation = this->TempMeasMenuButton->TextImageRelation;
			this->DataLogMenuButton->Padding = this->TempMeasMenuButton->Padding;
			this->DataLogMenuButton->UseCompatibleTextRendering = this->TempMeasMenuButton->UseCompatibleTextRendering;
			this->DataLogMenuButton->AutoEllipsis = false;
			// A non-breaking space is used so that the text can never wrap to two lines and end up over the image
			this->DataLogMenuButton->Text = L"Data\u00A0Log";
			this->DataLogMenuButton->Name = "DataLogMenuButton";
			this->DataLogMenuButton->UseVisualStyleBackColor = false;
			this->DataLogMenuButton->Enabled = false;
			this->DataLogMenuButton->Click += gcnew System::EventHandler(this, &MainGUI::DataLogMenuButton_Click);

			// Undock button - copy of the look of the "Plots" undock button
			this->DataLogUndockButton = gcnew System::Windows::Forms::Button();
			this->DataLogUndockButton->Dock = System::Windows::Forms::DockStyle::Fill;
			this->DataLogUndockButton->Margin = System::Windows::Forms::Padding(0);
			this->DataLogUndockButton->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->DataLogUndockButton->Font = (System::Drawing::Font^)this->TempMeasUndockButton->Font->Clone();
			this->DataLogUndockButton->ForeColor = System::Drawing::Color::White;
			this->DataLogUndockButton->BackColor = System::Drawing::Color::Transparent;
			this->DataLogUndockButton->FlatAppearance->BorderColor = System::Drawing::Color::FromArgb(35, 35, 35);
			this->DataLogUndockButton->FlatAppearance->MouseDownBackColor = System::Drawing::Color::FromArgb(70, 70, 70);
			this->DataLogUndockButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(50, 50, 50);
			if (this->TempMeasUndockButton->Image != nullptr) {
				this->DataLogUndockButton->Image = (System::Drawing::Image^)this->TempMeasUndockButton->Image->Clone();
			}
			this->DataLogUndockButton->TextAlign = System::Drawing::ContentAlignment::TopLeft;
			this->DataLogUndockButton->Name = "DataLogUndockButton";
			this->DataLogUndockButton->UseVisualStyleBackColor = false;
			this->DataLogUndockButton->Enabled = false;
			this->DataLogUndockButton->Click += gcnew System::EventHandler(this, &MainGUI::DataLogUndockButton_Click);

			this->DataLogMenuPanel->Controls->Add(this->DataLogMenuButton, 0, 0);
			this->DataLogMenuPanel->Controls->Add(this->DataLogUndockButton, 1, 0);

			// Insert a row for the new item above the "Emissivity" row of the left menu table
			this->tableLayoutPanel1->SuspendLayout();
			this->tableLayoutPanel1->RowCount = this->tableLayoutPanel1->RowCount + 1;
			System::Windows::Forms::RowStyle^ TemplateRowStyle = this->tableLayoutPanel1->RowStyles[this->tableLayoutPanel1->RowStyles->Count - 1];
			this->tableLayoutPanel1->RowStyles->Add(gcnew System::Windows::Forms::RowStyle(TemplateRowStyle->SizeType, TemplateRowStyle->Height));
			this->tableLayoutPanel1->SetRow(this->tableLayoutPanel6, 6);
			this->tableLayoutPanel1->Controls->Add(this->DataLogMenuPanel, 0, 5);
			this->tableLayoutPanel1->ResumeLayout(true);

			// Add the new buttons to the array of left menu buttons (they are enabled and disabled together with the others)
			cli::array<System::Windows::Forms::Button^>^ OldMenuButtons = GlobalVariables::MainGUILeftMenuButtons;
			cli::array<System::Windows::Forms::Button^>^ NewMenuButtons = gcnew cli::array<System::Windows::Forms::Button^>(OldMenuButtons->Length + 2);

			for (int i = 0; i < OldMenuButtons->Length; i++) { NewMenuButtons[i] = OldMenuButtons[i]; }

			NewMenuButtons[OldMenuButtons->Length] = this->DataLogMenuButton;
			NewMenuButtons[OldMenuButtons->Length + 1] = this->DataLogUndockButton;
			GlobalVariables::MainGUILeftMenuButtons = NewMenuButtons;

		}

		// Data log menu button callback ->
		private: System::Void DataLogMenuButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Handle docking of the form GUI in the main view panel of the main GUI
			HandleFormsOpeningDockingAndUndocking(ManagedLocals::DataLogGUIForm, this->MainViewTopPanel, &isDataLogFormOpen, &isDataLogFormDocked, &isDataLogFormUndocked, _FormDockingState_DockForm);

		}

		// Data log undock button callback ->
		private: System::Void DataLogUndockButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Handle undocking of the form GUI
			HandleFormsOpeningDockingAndUndocking(ManagedLocals::DataLogGUIForm, this->MainViewTopPanel, &isDataLogFormOpen, &isDataLogFormDocked, &isDataLogFormUndocked, _FormDockingState_UndockForm);

		}

		// About information menu button callback ->
		private: System::Void AboutMenuButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Handle docking of the form GUI in the main view panel of the main GUI
			HandleFormsOpeningDockingAndUndocking(ManagedLocals::WelcomeScreenForm, this->MainViewTopPanel, &isWelcomeScreenFormOpen, &isWelcomeScreenFormDocked, &isWelcomeScreenFormUndocked, _FormDockingState_DockForm);

		}

		// User guide menu button callback ->
		private: System::Void UserGuideButton_Click(System::Object^ sender, System::EventArgs^ e) {

			// Open the user manual in the system's default PDF viewer
			if (RMH_Winforms_OpenPDFWithDefaultViewer("IRCAMSoftwareManual.pdf") == false) {

				// Write GUI status message - the user manual could not be opened
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "Error: The User Manual (IRCAMSoftwareManual.pdf In The Program Folder) Could Not Be Opened - The File Is Missing Or No PDF Viewer Is Installed.", _StatusMessageType_Error);
				RMH_Winforms_RichTextBox_WriteLine(GlobalVariables::GlobalGUIInfoTextArea, "The User Manual Is Also Available At: https://github.com/Cyclotronic/OpenRMH-IRCAM/tree/main/docs/manual", _StatusMessageType_Normal);

			}

		}

		// --------------------------- Video Stream Thread Callback Routines -------------------------- //
			   
		// Video Stream Process Thread Callback Routine ->
		private: System::Void VideoStreamThread_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {
					
			// Primary thread process
			while (IRCamera.ConnectedFlag) {

				// Check the thread data ready flag
				if (ThreadDataReadyFlag == false) {

					// Execute the thermal camera image processing sequence 
					RMH_ThermalViewer_ImageProcessingSequence();

					// Update the thread data ready flag
					ThreadDataReadyFlag = true;

				}

				// Check whether the camera has been disconnected
				if (IRCamera.ConnectedFlag == false) {

					// Write the GUI start message in the terminal
					cout << "Video Processing Thread Stopped\n" << endl;

					// Break the thread loop
					break;

				}

				// Limit CPU usage and let the thread wait
				System::Threading::Thread::Sleep(1);

			}
			
		}

		// Secondary processing thread callback routine ->
		private: System::Void SecondaryProcessingThread_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {

			// Primary thread process
			while (IRCamera.ConnectedFlag) {

				// Handling of the secondary processing thread
				RMH_ThermalViewer_SecondaryProcessingSequence();

				// Check whether the camera has been disconnected
				if (IRCamera.ConnectedFlag == false) {

					// Write the GUI start message in the terminal
					cout << "Secondary Processing Thread Stopped\n" << endl;

					// Break the thread loop
					break;

				}

				// Limit CPU usage and let the thread wait
				System::Threading::Thread::Sleep(1);

			}

		}

		// Main GUI Update Timer Callback ->
		private: System::Void MainGUIUpdateTimer_Tick(System::Object^ sender, System::EventArgs^ e) {

			// This routine is a timer callback used to update GUI elements

			// Is processed data from the thread ready
			if (ThreadDataReadyFlag == true) {

				// Is the live view form GUI open
				if (isLiveViewStreamFormOpen == true) {

					// Update the live view stream menu
					RMH_ThermalViewer_UpdateLiveView(GlobalVariables::GlobalLiveViewStreamPanel->Width,
						GlobalVariables::GlobalLiveViewStreamPanel->Height, FixedLiveViewAspectRatio,
						GlobalVariables::GlobalColorBarMainPanel->Width,
						GlobalVariables::GlobalColorBarMainPanel->Height,
						GlobalVariables::GlobalLiveViewHistogramPanel->Width,
						GlobalVariables::GlobalLiveViewHistogramPanel->Height);

				}

				// Has video recording been started
				if (VideoRecordingStartedFlag == true) {

					// Handle writing the relevant data to video files, if video recording has started
					RMH_ThermalViewer_WriteDataToVideoRecordingFilesSequence();

				}

				// Should the surface plot form GUI be shown in the associated panel
				if (isSurfacePlotFormOpen == true) {

					// Update the surface plot form
					RMH_ThermalViewer_UpdateSurfacePlotMenuScreen(ManagedLocals::SurfacePlotGUIForm->SurfacePlotPanel->Width,
						ManagedLocals::SurfacePlotGUIForm->SurfacePlotPanel->Height);

				}

				// Should the 2D plot form GUI be shown in the associated panel
				if (isTempMeasurementsFormOpen == true) {

					// Update the 2D plot form
					RMH_ThermalViewer_Update2DPlotMenuScreen(ManagedLocals::TempMeasurementsGUIForm->Temp2DPlotPanel->Width, 
						ManagedLocals::TempMeasurementsGUIForm->Temp2DPlotPanel->Height);

				}

				// Update the status labels of the temperature alarms only if the menu is in view and the camera config form is in view
				if (TempAlarmsConfigMenuIsOpen == true && isThermalCameraFormOpen == true) {

					// Update the status labels of the active temperature alarms in the sub menu
					RMH_ThermalViewer_UpdateTemperatureAlarmsSubMenuStatusLabels();

				}

				// Update the data labels of the live view statistics window - if the window is open
				RMH_ThermalViewer_UpdateAndFormatLiveViewStatisticsLabels();

				// Reset the thread data ready flag
				ThreadDataReadyFlag = false;

			}

			// --------------------- Open, Dock And Close Form GUIs Handling ---------------------- //

			// If the live view tools panel has been closed
			if (isLiveViewToolsFormOpen == false) {

				// Dock the tools form back into the live view form
				ManagedLocals::LiveViewStreamForm->DockLiveViewToolsFormInLiveViewToolsPanel();

			}

			// Should the video playback controls form be opened and is the form already closed
			if (OpenVideoPlayBackControlsFormFlag == true && VideoPlaybackControlsFormIsOpenFlag == false) {

				// Allocate the video playback controls form to memory
				ManagedLocals::PlayBackControlsPanelForm = gcnew IRCAMThermalViewer::VideoPlayBackTools();

				// Open/show the video playback controls form 
				ManagedLocals::PlayBackControlsPanelForm->Show();

				// Reset the "open video playback" form flag
				OpenVideoPlayBackControlsFormFlag = false;

			}

			// Should the video playback controls form be closed and is the form already open
			if (CloseVideoPlayBackControlsFormFlag == true && VideoPlaybackControlsFormIsOpenFlag == true) {

				// Open/show the video playback controls form 
				ManagedLocals::PlayBackControlsPanelForm->Close();

				// Reset the "close video playback" form flag
				CloseVideoPlayBackControlsFormFlag = false;

			}

			// ----------------- Handling Of Loss Of The USB Connection To The Camera ------------------ //

			// Handle events on loss of the connection to the camera
			RMH_ThermalViewer_HandleCameraDisconnectedEvents(true);

			// Was the USB connection to the camera lost
			if (IRCamera.ConnectedFlag == false) {

				// Disable the menu buttons of the main GUI
				RMH_Application_DisableMainGUIMenuButtons();

				// Handle docking of the form GUI in the main view panel of the main GUI
				HandleFormsOpeningDockingAndUndocking(ManagedLocals::ThermalCameraGUIForm, this->MainViewTopPanel, &isThermalCameraFormOpen, &isThermalCameraFormDocked, &isThermalCameraFormUndocked, _FormDockingState_DockForm);

			}

			// ------------------------------------------------------------------------------------ //

		}

	    // -------------------------------------------------------------------------------------------- //

};
}
