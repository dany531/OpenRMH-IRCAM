# Changelog

Tags look like `v<upstream version>-community.<n>`: the first part is the version of IRCAM Thermal Viewer the source came from,
the number counts builds of this repository.

## Unreleased

- Data Log window graph zoom: dragging a rectangle with the left mouse button now zooms both the time (X) and the temperature (Y) axis
  to the area of the rectangle. Before, only the temperature axis zoomed: the time axis is a date / time axis, and the selection
  snapped to a large time unit. The selection cursors of both axes are now set to a fine step (X: 1 ms, Y: 0.001) and the smallest
  zoomed view is also small (X: 1 ms, Y: 0.001), so the rectangle is used as drawn. Reset Zoom is unchanged. A click without
  dragging still switches the cursor temperature label on or off. Change in `DataLogGUI.h` (`BuildChart`).

- Data logging: warning for sessions with many samples. When Start Logging is pressed (Data Log window button, the plot's right-click
  menu or hotkey M) and the session is set to more than 100000 samples (duration divided by sample interval), a window explains the
  situation: the Data Log window shows at most the newest 100000 samples in the graph and the newest 20000 rows in the table, the oldest
  samples scroll off the left side of the graph and out of the table, and every sample is saved in the log file. **Start** starts
  logging, **Cancel** does not start, so the interval or duration can be changed first. A start by a temperature alarm or a trigger event
  shows no window (it would block an automatic action), only a status message. The notice that is shown when the Data Log window is opened
  has the same note for such a session. The limits (`_DataLogView_MaxRows`, `_DataLogView_MaxPlotPoints`) moved to
  `RMH_Application_ThermalViewer.h`; `RMH_ThermalViewer_StartDataLogging` has a new parameter `AskUserAboutSampleLimit` (default true).
  Changes in `RMH_Application_ThermalViewer.cpp/.h` and `DataLogGUI.h`. No limit or default value was changed.

- Data logging file names: a session file is now named `DataLogSession_ddMMMyyyy_HH-mm-ss.txt`, for example
  `DataLogSession_12Nov2026_09-57-39.txt` (12 = day, Nov = month abbreviation in English, 2026 = year, 09 = hour, 57 = minutes,
  39 = seconds). The old name was a run of digits (`HHmmssfffddMMyyyy`). The English month abbreviation is used on every PC
  (invariant culture). Change in `RMH_Application_ThermalViewer.cpp` (`RMH_ThermalViewer_StartDataLogging`).
- Data Log window graph: a left click in the graph switches a mouse cursor label on or off, like in the Plots window. While on, the
  label follows the mouse and shows the temperature at the cursor position (in the program's temperature unit) and the elapsed time,
  for example "Temperature: 25.43 C   (0:01:23)", or "Temperature: N/A" when the cursor is outside the plot area. Dragging a rectangle
  (zoom) is not treated as a click. Change in `DataLogGUI.h`.

- Data logging: confirmation windows and temporary session file.
  - Data Log window: **Stop Logging** and **Clear View** now ask "Are you sure ...?" (Yes / No). "No" closes the window and does
    nothing (logging keeps running, the view is not cleared). Both windows default to "No".
  - A data logging session is now written to a temporary file (`%TEMP%\IRCAMThermalViewer_DataLogging`) instead of directly to the
    default save path. When the session ends, a window asks "Do you want to save this data logging session?". **Yes**: the file is
    copied to the default save path and the temporary file is deleted (if saving fails, the error is shown and the temporary file is
    kept). **No**: the temporary file is deleted. This stops unwanted files piling up on the Desktop.
  - The save question appears however the session ends: Stop Logging button, hotkey N, the plot's right-click menu, a temperature
    alarm or trigger event, or the end of the session duration. It is shown by the data logging thread's `RunWorkerCompleted` event
    (`RMH_ThermalViewer_FinishDataLoggingSession`), after the last sample was written.
  - The data logging thread now waits for the sample interval in 50 ms slices, so a stop request is handled at once (and the save
    question appears quickly) instead of after up to one full interval. No extra sample is written after a stop.
  - Start Logging while the previous session is still waiting for its save / discard answer shows a warning instead of starting.
  - The Data Log window follows the session file to its new place after it was saved.
  - New global `GlobalVariables::DataLoggingSessionFolderString` (folder that holds the session file now).
    Changes in `DataLogGUI.h`, `TempMeasGUI.h`, `RMH_Application_ThermalViewer.cpp/.h`, `GlobalObjectsAndVariables.h`.

- Custom build identification: the main window title bar ends with "CUSTOM BUILD FOR THE INFIRAY T2S+ V2 (T2S+_A2)" and a start-up
  status message says this version is built specifically for that camera (`_CustomBuild_TitleSuffix`, `_CustomBuild_StartupMessage`).

- Data Log window: a new item "Data Log" in the left menu (between "Plots" and "Emissivity"), with its own pop-out button, shows the
  values collected by the Data Logging feature in a table, in near real time (the CSV file written by Data Logging is read once per
  second). It follows the data logging session settings (data sets, interval, duration). The window has Start Logging / Stop Logging
  buttons (the same as the data logging menu), a status line (running or stopped, file, samples, elapsed time of the session,
  interval), Show Log File, Clear View and an auto-scroll option. At most 20000 rows are kept in the table; the complete data is
  always in the CSV file. The window also draws the values as a graph (temperature against time) above the table; drag a
  rectangle in the graph to zoom and use Reset Zoom to go back. The time axis shows the elapsed time of the log session from
  0:00:00 (h:mm:ss, also beyond 24 hours), and the axis titles show the units (the temperature unit follows the program setting).
  At most 100000 points per line are kept in the graph and 20000 rows in the table (old ones are removed in blocks).
  When the window is opened, a notice shows the current data logging session duration and sample interval and where to change them.
  New file `DataLogGUI.h`; changes in `MainGUI.h`, `GlobalObjectsAndVariables.h/.cpp`.
- 2D plot Y-axis range: the plot's right-click menu has new items to set a fixed Y-axis minimum and maximum and lock it ("Set Y-Axis
  Range And Lock...", "Lock Y-Axis At Current Range", "Unlock Y-Axis (Auto Range)"). Data outside a locked range is limited to the
  edges of the plot area. Changes are in `RMH_OpenGL_2DPlot.h` and `TempMeasGUI.h`.
- 2D temperature plot (`RMH_OpenGL_2DPlot.h`): the OpenGL window was created at four times the size of the panel, which some older
  graphics drivers (found on an Intel HD Graphics 2000, driver 9.17.10.4459) could not handle - black plot background, no text, garbled
  lines. The window is now created at the panel size and follows it when it is resized. A right-click on the plot's overlay opens
  the plot's context menu.
- 2D temperature plot start: the data set check boxes of the plot settings start checked without raising their event, so no data set was
  enabled in the plot and it did not start until a setting was changed. The start-up state (data source and enable check box of each
  data set) is now applied to the plot when the program starts (`RMH_ThermalViewer_Apply2DPlotDataSetSettings`).
- Automatic start-up sequence (set `_AutoStart_Enabled` to 0 in `RMH_Application_ThermalViewer.h` to turn it off): after the main
  window opens, IRCAM selects the InfiRay T2S+ V2, connects, switches to the stream window, presses its Calibrate Camera button
  (hotkey Q), sets the auto calibration period to 180 s and enables periodic shutter calibration. While this runs a "custom version - please wait" window is shown; a "READY TO USE" window
  is shown when it is finished, and a "Camera Not Connected" warning window is shown if the camera is not found
  (`_AutoStart_ShowPopupWindows`, also in `RMH_Application_ThermalViewer.h`).

- Pool 3 (InfiRay T2S+ V2) temperatures: two errors made temperatures read far too high.
  1. The detector (FPA) raw value was overwritten by the max/min/center data written to the start of the metadata row, so the
     detector temperature was garbage. The value is now saved before it is overwritten.
  2. The non-uniformity correction put a pixel that sees the shutter at the mean of the baseline frame. The manufacturer's software
     (and the camera's calibration data) expect a fixed level of 6000, so the map is now re-referenced to 6000 for Pool 3
     (`_Pool3_NonUniformityCorrectionLevel`). Press Calibrate after starting the camera.
  Also aligned with the manufacturer's software: shutter temperature fine correction, 256-sensor gray offset of 170, FPA divisor
  37.682, calibration floats read low word first, distance term limited to 20. The shutter register is used again; a fixed
  value can be selected with `_Pool3_UseFixedShutterTemperature` as a fallback.
  A signal gain setting (`_Pool3_PixelGain`, default 1.00 = none) approximates the manufacturer's per-pixel gain map. To tune it: new gain = old gain x
  (T_reference - T_shutter) / (T_ircam - T_shutter) for a hot target, using the shutter temperature shown in the statistics window.

## v3.0.0-community.5 - 2026-09-30

- Two new cameras, contributed by Krzysztof Boksa ([#2](https://github.com/Cyclotronic/OpenRMH-IRCAM/pull/2)). Both
  were tested by the contributor on real hardware; neither has been tested by this project.
  - **Thermal Master P3** (pool 5): the camera has no USB Video Class interface, so it is opened directly through WinUSB
    (`src/p3_winusb_camera.h`/`.cpp`) using the vendor's own protocol, reverse engineered from a USB capture of the vendor
    app. The Calibrate button triggers the camera's shutter, and the Temperature Range button switches the camera between
    its two ranges.
  - **Thermal Master THOR001** (pool 6): a UVC camera that declares an H.264 format but delivers raw 256 x 192 sensor
    values. Temperature is decoded as deci-Kelvin, unlike the other pools. Its range is not yet known.
- The camera list is now shown alphabetically. Each camera keeps a fixed index, which is what a saved session stores, so
  existing sessions still select the same camera; new cameras are appended to the list of indexes and sort themselves in.
- A saved session that names an unknown camera now falls back to Snapshot Analysis mode instead of indexing outside the
  list.
- The camera-adding guide in `docs/DEVELOPER-GUIDE.md` now covers cameras that are not DirectShow devices.

## v3.0.0-community.4 - 2026-09-27

- User manual: the author's manual, converted from the Word original in his source release to Markdown
  (`docs/manual/`) and updated for 3.0.0 as a community edition. The sections on buying and activating a licence and
  the proprietary Terms of Use (which applied to the earlier commercial release and contradicted the MIT licence) are
  removed; the camera list, MATLAB examples and several settings are updated. GitHub Actions builds the PDF
  (`scripts/build-manual.py`, pinned pandoc + Typst). It ships in the installer and the zip as
  `IRCAMSoftwareManual.pdf`, and each release attaches it.
- The User Guide button opens the manual in the system's default PDF viewer. The embedded viewer it replaces worked
  only with Adobe Reader installed. If the PDF is missing or no viewer is installed, the status panel says so.
- Input validation: a frame from a file or a camera is now checked against the size of the frame buffers before it is copied
  or indexed (`src/RMH_FrameBufferLimits.h`). Previously an oversized or corrupt snapshot PNG, recording AVI, or camera
  video format could write past the end of the global frame buffers.
  - Snapshot PNGs must be 24-bit RGB and fit the buffers; recordings must have frames that fit and a readable first frame.
  - Metadata stored in snapshot and recording files (camera pool, metadata rows, pixel offsets) is checked before use;
    an invalid file is reported as corrupt instead of being loaded.
  - A camera whose video format is larger than the buffers is refused at connect with an error message.
  - The DirectShow layer takes an optional destination size and refuses to copy a larger frame.
- Fixed the centre-pixel index overflowing for 640-pixel-wide sensors (pool 3).
- Saved-session values are range-checked: an out-of-range list index is ignored and the recording frame rate is clamped.
- Verified on hardware with a Thermal Master P2 Pro: camera connect, snapshot, recording, saving and loading a session,
  the user manual button, and rejecting a file that isn't a valid snapshot or recording.

## v3.0.0-community.3 - 2026-09-27

- Windows installer (`OpenRMH-IRCAM-<version>-setup.exe`, Inno Setup) built alongside the portable zip. It installs the
  Microsoft Visual C++ Redistributable (x64) if needed, as the original author's installer did.
- No change to the program's behaviour.

## v3.0.0-community.2 - 2026-09-27

- Source comments translated from Danish to English (about 7,000 comment lines in 52 files). Code is unchanged: for every file
  the source with comments removed is identical to the original import. See `NOTICE.md`.
- New documentation: `docs/DEVELOPER-GUIDE.md` (how a frame flows from camera to screen, how to add a camera model) and
  `CONTRIBUTING.md`. `docs/ARCHITECTURE.md` updated.
- No change to the program's behaviour.

## v3.0.0-community.1 - 2026-09-26

- First public build: IRCAM Thermal Viewer 3.0.0 (September 2026 source release by Rune Mark Hansen), MIT licensed.
- Source-only repository with a scripted dependency fetch (`scripts/fetch-deps.ps1`), a build script, and a GitHub Actions build.
- Removed the unused TurboActivate link dependency; dependency and icon paths made portable.
- Verified with a Thermal Master P2 Pro (list entry "InfiRay Or Thermal Master P2Pro").
