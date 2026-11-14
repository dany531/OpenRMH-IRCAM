# IRCAM for T2S+ V2 (branch `IRCAM-for-T2Splus_V2_2Dplots_expansion_17`)

A modified version of **OpenRMH-IRCAM** (based on v3.0.0-community.5) for the **InfiRay T2S+ V2 / T2S+_A2** thermal camera
(USB ID `VID_3474&PID_6002`). Personal reference notes, last updated 14 November 2026. Not an official release.

- Fork: https://github.com/dany531/OpenRMH-IRCAM
- Original project: https://github.com/Cyclotronic/OpenRMH-IRCAM
- This branch contains **all** changes made so far: correct temperatures, automatic start-up, the 2D plot fixes and extras, the Data Log window, the confirmation / save questions of data logging, readable log file names, the mouse cursor temperature in the Data Log graph, a warning for sessions with many samples, zooming in both axes, the right-click menu of the Data Log graph a check box per graph line an Export Graph button and X-axis range lock in the Data Log graph menu a graph title that is asked for again when exporting, colored Start / Stop Logging button borders and a corrected Data Log menu item, and a working scroll bar for the zoomed graph, and a question when the application is closed while data logging is running.

---

## 1. What this version fixes

### 1.1 Wrong temperatures (two bugs in IRCAM, not the camera)

The original IRCAM read this camera far too hot (for example 34 °C on a tape that was really about 21.8 °C).

1. **The camera's detector (FPA) value was overwritten.** IRCAM wrote the max/min/center data over the first words of the
   metadata row, which destroyed the raw detector temperature (this gave the 246 °C "camera detector" reading). The value is now
   saved before it is overwritten.
2. **The flat-field correction used the wrong gray level.** IRCAM put a pixel that sees the shutter at the mean of the baseline
   frame. The manufacturer's software (and the camera's calibration data) use a fixed level of **6000**. The correction map is now
   re-referenced to 6000 for this camera type.

Other changes, aligned with the manufacturer's software: FPA divisor 37.682, 256-pixel gray offset 170, shutter temperature from
the register plus its signed fine-correction byte, calibration floats read low word first, distance term limited to 20.
All of this applies to this camera type only ("Pool 3"); other cameras are unaffected.

The older workaround (forcing `temp_fpa` and `temp_shutter` to 18 °C, "Build #7", commit `7b355e8`) only hid these bugs.

### 1.2 Automatic start-up

After the main window opens, the program does the following without any clicks:

1. Shows a **"Custom Version For The InfiRay T2S+ V2 - please wait"** window, centered on the main window.
2. Selects **InfiRay T2S+ V2** in the camera list and connects (like the Connect button, without the TOPDON information dialog).
3. About 4 seconds later, **switches to the stream window** (like the Live View menu button).
4. About 1.5 seconds later, presses the stream window's **Calibrate Camera button (hotkey Q)**.
5. Sets **Auto Calibration Period [Sec] to 180** and enables **periodic shutter calibration**.
6. Closes the wait window and shows a **READY TO USE** window. Click OK and start measuring.

If the camera is **not found**, a **"Camera Not Connected"** window explains what to do: close the application, connect the camera
to a USB port, wait a few seconds, and open the application again. There is no automatic retry, so plug the camera in before
starting the program.

### 1.3 2D temperature plots (Plots window)

Two separate problems were fixed:

1. **Black background, no axis text, garbled lines (old graphics hardware).** The plot's OpenGL window was created at four times
   the size of its panel. An old Intel HD Graphics 2000 (Windows 10, driver 9.17.10.4459) could not handle that. The window is now
   created at the panel's size and is resized to follow the panel. A right-click on the plot's top layer opens the plot's menu.
2. **The plot did not start until a setting was changed.** The data set check boxes of the plot settings start checked, but
   without raising their event, so no data set was enabled in the plot. The start-up state (data source and enable check box of
   each data set) is now applied to the plot when the program starts.

### 1.4 Y-axis range of the 2D plot (new)

Right-click inside the Plots window. The menu now has (below "Y-Axis Number Of Ticks"):

- a status line: **Y-Axis Range: Auto** or **Y-Axis Range: Locked (min to max)**,
- **Set Y-Axis Range And Lock...** - opens a small window where you type a minimum and a maximum (decimal values),
- **Lock Y-Axis At Current Range** - freezes the range that is shown now (rounded outwards to one decimal),
- **Unlock Y-Axis (Auto Range)** - the range follows the data again.

With a locked range, data outside it is drawn along the top or bottom edge of the plot. The values are in the temperature unit shown
on the plot, so if you change the unit, set the range again. The lock is not saved: it resets to Auto when the program is restarted.

### 1.5 Data Log window (new)

A new item **Data Log** in the left menu, between **Plots** and **Emissivity**, with its own pop-out button like the other items.
The window shows the values collected by the **Data Logging** feature, in a table, in near real time (about 1 second behind):

- It follows the **data logging session settings** (data sets, interval, duration) because it displays what Data Logging writes.
- **Start Logging / Stop Logging** buttons do the same as the data logging start and stop in the plot menu.
- The status line shows running or stopped, the log file, the number of samples, the elapsed time of the session and the interval.
- **Show Log File** opens the log file in the Windows explorer. **Clear View** empties the table and graph (the file is not changed). **Auto-scroll** keeps the newest row visible.
- Above the table the window draws the values as a **graph: temperature (Y axis, in the program's temperature unit) against elapsed
  time (X axis, h:mm:ss)**. The time axis starts at **0:00:00** when the log session starts, and also works beyond 24 hours. There is
  one line per logged data set, with the same line colors as the 2D plot, and a legend. Hover a line to see its value and time.
- **Zoom:** drag a rectangle in the graph to zoom in on that part of the time and temperature range. **Reset Zoom** goes back to
  the full view of all the data. While zoomed in, new data keeps being added, but the view stays on the zoomed part. The divider
  between the graph and the table can be dragged.
- When you open the window, a **notice** shows the current data logging session duration and sample interval, and tells you where
  to change them (Settings -> Data Logging Session Settings). It can be switched off until the program is restarted.
- The graph keeps up to **100000 points per line** (about 27.8 hours at one sample per second). The table keeps the last 20000 rows
  (it is only for reading the recent values). The complete data is always in the CSV file.
- The menu item is enabled together with the other items, after a camera is connected.

### 1.6 Confirmation windows and temporary data log file (new)

- **Stop Logging** (Data Log window) asks **"Are you sure you want to stop logging?"**. **Yes** stops; **No** closes the window and
  logging continues.
- **Clear View** asks **"Are you sure you want to clear the view?"**. **Yes** clears; **No** closes the window and nothing changes.
  Both windows have **No** as the default button, so pressing Enter by mistake does nothing.
- **Start Logging** now writes the session to a **temporary file** in `%TEMP%\IRCAMThermalViewer_DataLogging`, not to the default
  save path.
- When the session ends, a window asks **"Do you want to save this data logging session?"**
  - **Yes**: the file is copied to the default save path (Settings) and the temporary file is deleted. The Data Log window then follows
    the saved file. If saving fails, an error window is shown and the temporary file is kept, so no data is lost.
  - **No**: the temporary file is deleted. Nothing is left on the Desktop.
- This question is asked every time a session ends, not only with the Stop Logging button: also the hotkey **N**, the plot's right-click
  menu, a temperature alarm or trigger event that stops logging, and the end of the session duration.
- The logging thread now checks for a stop request every 50 ms, so the questions appear quickly (before: up to one full sample
  interval later), and no extra sample is written after a stop.
- If **Start Logging** is pressed while the previous session is still waiting for the save / discard answer, a warning is shown.
- Note: if the program is closed or crashes during a session, the temporary file stays in the temporary folder.

### 1.7 Readable log file names (new)

A session file is named **`DataLogSession_ddMMMyyyy_HH-mm-ss.txt`**, for example `DataLogSession_12Nov2026_09-57-39.txt`:
12 = day, Nov = month (Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec - always in English), 2026 = year,
09 = hour, 57 = minutes, 39 = seconds. (Before: `DataLogSession_` followed by a run of digits.) The file is still a text file with
the delimiter you chose in the settings; change `.txt` to `.csv` in `RMH_ThermalViewer_StartDataLogging` if you prefer that extension.

### 1.8 Mouse cursor temperature in the Data Log graph (new, changed in this branch)

Right-click in the Data Log graph and tick **Temperature At The Cursor** (see 1.11). A label follows the mouse and shows the
**temperature at the cursor** (in the program's temperature unit) and the **elapsed time** at that position, for example
`Temperature: 25.43 C   (0:01:23)`. Outside the plot area it shows `Temperature: N/A`. The label hides when the mouse leaves the graph.
Untick the item to switch it off. Before this branch this was switched on and off with a left click; the left click now does nothing.

### 1.9 Warning for sessions with many samples (new)

The Data Log window keeps the **newest 100000 points per line in the graph** and the **newest 20000 rows in the table**. The log file
always holds every sample. When the session is set to more samples than the graph keeps (duration divided by sample interval, for
example more than about 27.7 hours at 1.00 s), pressing Start Logging shows a window that explains this:

- The oldest samples scroll off the **left** side of the graph (and out of the table); the newest are added on the right.
- **No data is lost**: every sample is saved in the log file.
- **Start** starts logging. **Cancel** does not start, so you can change the interval or duration (Settings -> Data Logging Session
  Settings) first.

The window is shown when logging is started from the Data Log window, the plot's right-click menu or the hotkey **M**. A start by a
temperature alarm or a trigger event shows only a status message (no window, so the automatic action is not blocked). The notice
shown when the Data Log window is opened has the same note for such a session. The limits and the default values (interval 0.04 s,
duration 1 minute) were not changed.

Reference: at 1 s interval, 100000 samples = about 27.8 hours; the 20000 table rows = about 5.6 hours. At 0.1 s: about 2.8 hours
and 33 minutes. At 10 s: about 11.6 days and 2.3 days.

### 1.10 Zoom in both axes of the Data Log graph (new)

**Hold the left mouse button and drag a rectangle** in the Data Log graph: the graph zooms to the time range (X) **and** the
temperature range (Y) that the rectangle contains. **Reset Zoom** goes back to the full view. While zoomed in, new data keeps being
added but the view stays on the zoomed part. A click without dragging does nothing.
Before this change only the temperature axis zoomed, because the selection on the time axis snapped to a large time unit.

### 1.11 Right-click menu of the Data Log graph (new)

Right-click in the Data Log graph. The menu has the same items as the right-click menu of the Plots window, plus the cursor temperature:

- **Temperature At The Cursor** - tick to show the temperature and time at the mouse cursor (see 1.8).
- **Set Graph Title...** - type a title for the graph (see 1.15).
- **X-Axis Number Of Ticks** - Auto, 5, 10, 20, 25 or 50 ticks on the time axis.
- **Y-Axis Number Of Ticks** - Auto, 5, 10, 15, 20, 25 or 30 ticks on the temperature axis.
- **X-Axis Range: Auto** (status line; shows **Locked h:mm:ss To h:mm:ss** when locked), **Set X-Axis Range And Lock...**, **Lock X-Axis At Current Range** and **Unlock X-Axis (Auto Range)** - the same four items as for the Y axis, for the time axis (see 1.14).
- **Y-Axis Range: Auto** (status line; shows **Locked min To max** when locked).
- **Set Y-Axis Range And Lock...** - type a minimum and maximum (in the temperature unit of the graph); the axis stays at that range.
- **Lock Y-Axis At Current Range** - freezes the range that is shown now (rounded outwards to one decimal). Available while not locked.
- **Unlock Y-Axis (Auto Range)** - the range follows the data again.

The tick spacing is the visible range divided by the chosen number and follows zooming, **Reset Zoom** and new data. With a number of Y ticks
chosen and the range not locked, the Y axis fits the data with a small margin. **Auto** (the default) keeps the normal scaling of the chart.
Hidden lines (1.12) are ignored. The settings are not saved: they reset to Auto when the program is restarted.

### 1.12 Show / hide each graph line (new)

Above the graph (below the Start Logging / Stop Logging / Show Log File / Clear View / Reset Zoom buttons) there is one **check box per
graph line**, with the line's name in the line's color. Un-tick a box to hide that line, tick it to show it again, for example to look at
2 of 10 logged data sets. All lines are ticked when a log is opened or a new session starts. The check boxes replace the legend of the graph.
A hidden line is only hidden from the graph: its data keeps being collected and stays in the table and in the log file. The automatic
Y range and the Y tick / range settings only use the lines that are shown.

### 1.13 Export Graph as an image (new)

The **Export Graph** button, next to Start Logging, Stop Logging, Show Log File, Clear View and Reset Zoom, saves the Data Log graph as an image.
First the **Graph Title** window opens (the same window as **Set Graph Title...**, see 1.15). It shows the title that is set now, so you can keep it, edit it or replace it (an empty text removes the title); **Set Title** continues, **Cancel** cancels the export. Then a save window opens (file types **PNG**, **JPEG** and **BMP**; the format follows the extension you type, PNG by default) with the session file
name plus `_graph` as the suggested name (or `DataLogGraph_` and the date and time when there is no session file name).

- The image is the graph **as it is shown now**: zoom, number of ticks and locked Y range are included; lines you hid with the check boxes (1.12) are not.
- It has the size of the graph on the screen. A strip above the graph names the lines that are shown, each name in the color of its line.
- The mouse cursor temperature label (1.8) is not part of the image.
- If there is no graph yet, or saving fails, a message window says so.

### 1.14 X-axis (time) range: set, lock, unlock (new)

The right-click menu of the Data Log graph has the same range items for the time axis as for the temperature axis, in a block above the Y-axis block:

- **X-Axis Range: Auto** or **X-Axis Range: Locked 0:05:00 To 0:10:00** (status line).
- **Set X-Axis Range And Lock...** - type a **start** and an **end** as elapsed time of the log session: `h:mm:ss` (for example `0:05:30`),
  `m:ss` or plain seconds also work, and decimals are allowed (`1:00:00.5`). If the text is not a valid time a warning is shown and the
  window opens again with what you typed. An end that is not later than the start is corrected to start + 1 second.
- **Lock X-Axis At Current Range** - locks the time range that is shown now (the zoomed range, or the whole range), rounded outwards to 0.1 s.
  Available while the axis is not locked.
- **Unlock X-Axis (Auto Range)** - the time axis follows the data again. Available while locked.

While the X axis is locked the graph shows only that time range: new data outside it is still collected (table, log file) but not drawn.
You can still drag-zoom inside the locked range, **Reset Zoom** goes back to the locked range, and the X-axis number of ticks (1.11) spreads the
ticks over the locked range. The lock is not saved: it resets to Auto when the program is restarted. Together with the Y-axis lock this lets
you fix the exact window of the graph before using **Export Graph** (1.13).

### 1.15 Graph title (new)

Right-click in the Data Log graph and choose **Set Graph Title...**. Type a title (up to 200 characters) and press **Set Title**. The title
appears in bold at the **top center above the graph** and is part of the image saved with **Export Graph** (1.13), so you can see later
what the graph was about. Choose the item again to change the title; an empty text removes it. The same window also opens when you press **Export Graph** (1.13), with the current title filled in. The title stays when you use Clear View or start a new
session, but it is not saved: it is empty again when the program is restarted.

### 1.16 Colored Start / Stop Logging button borders and Data Log menu item fix (new)

- In the Data Log window the **border** of **Start Logging** is **dark green** and the border of **Stop Logging** is **dark red**. The background and
  the text of these two buttons are exactly like the other buttons.
- In the left menu the **Data Log** text is now shown **under its image**, like Settings, Stream, 3D Surface, Plots and Emissivity. Before, the
  button was too narrow (its column widths were fixed numbers that the main GUI did not scale), so the text wrapped into two lines and overlapped the
  image. The item now copies the column widths, padding and layout of the Plots item, and the text cannot wrap.

### 1.17 Scroll bar of the zoomed graph (fixed)

After zooming the Data Log graph (drag a rectangle), a scroll bar shows under the time axis (and beside the temperature axis). Before, its arrows
jumped straight to the beginning or the end of the log and dragging the bar did nothing, because the scroll steps of the date / time axis were
far too large for a zoomed range. Now the **arrows move the view by a tenth of the visible range**, the **bar can be dragged** and clicking in the
bar track moves one view at a time. The temperature axis bar works the same way. The ticks of the axes follow the view while scrolling.

### 1.18 Closing the application while data logging runs (new)

If you close IRCAM while a data logging session is running, a window **"Data Logging Is Running"** asks **"Are you sure you want to close the
application?"** (default **No**). **No** keeps the application open and the logging continues. **Yes** stops the session and then asks the usual
**"Do you want to save this data logging session?"**: **Yes** saves the file to the default save path, **No** deletes it; then the application closes.
(When Windows shuts down no question is asked and the session is only stopped.)

**Temporary file:** the session is written to `%TEMP%\IRCAMThermalViewer_DataLogging` while it runs. Before this change that file stayed there when
the application was closed during logging (no save / delete question was asked). Now it is saved or deleted when you close the application. A file
can still be left in that folder after a crash, a power loss or a killed process; it holds the samples written so far (so it can be useful) and can
be deleted by hand.

---

## 2. How to use it

1. Plug in the camera, then start IRCAM.
2. Wait for the READY TO USE window. Let the camera warm up for about **10 minutes** before trusting readings, because this camera
   drifts a little while it heats up. The 180 s periodic calibration helps; you can still press Calibrate (Q) manually.
3. For shiny parts (solder, bare metal, chip lids) put a dot of **matte black paint or Kapton tape** on the spot you measure;
   shiny surfaces read much too low.
4. Small constant errors can be corrected with IRCAM's **Temperature correction** setting (no rebuild needed).

---

## 3. Files changed (all in `src/`, plus two notes files at the top level)

| File | What changed |
|---|---|
| `RMH_ThermalCameraSupport_Library.cpp` | Temperature fixes (detector value saved, 6000 flat-field level, offsets, shutter handling, calibration floats) |
| `RMH_ThermalCameraSupport_Library.h` | New data members (`FrameFPARawWord`, `CalValue6`) |
| `RMH_Application_ThermalViewer.cpp` | Start-up routines (select camera and connect, enable periodic calibration), applying the plot data set settings, temporary data log file, save / discard question (`RMH_ThermalViewer_FinishDataLoggingSession`), 50 ms stop check of the logging thread, readable log file name, warning window for sessions with many samples |
| `RMH_Application_ThermalViewer.h` | Start-up settings and declarations (incl. `RMH_ThermalViewer_FinishDataLoggingSession`); Data Log window limits (`_DataLogView_MaxRows`, `_DataLogView_MaxPlotPoints`); `AskUserAboutSampleLimit` parameter of `RMH_ThermalViewer_StartDataLogging` |
| `MainGUI.h` | Question when closing the application during data logging (`MainGUI_FormClosing`); start-up timer (switch to the stream window, press Q), the three information windows, applying the plot settings, and the Data Log menu item (column widths, padding and text copied from the Plots item so the text shows under the image) |
| `DataLogGUI.h` (new) | The Data Log window (table and graph that follow the data logging file); Yes / No questions for Stop Logging and Clear View; follows the session file to the saved location; mouse cursor temperature label in the graph (right-click menu); note about many samples in the opening notice; zoom rectangle on both axes; right-click menu of the graph (cursor temperature, number of ticks, Y-axis range set / lock at current / unlock); check box per graph line; Export Graph button (graph saved as image); X-axis range items (set / lock at current / unlock); graph title; dark green / dark red Start / Stop Logging button borders; scroll steps of the zoomed graph (`UpdateScrollSteps`) |
| `GlobalObjectsAndVariables.h/.cpp` | Flags for the Data Log window; `DataLoggingSessionFolderString` (folder of the current session file) |
| `RMH_OpenGL_2DPlot.h` | OpenGL window created at the panel size and resized with it; right-click menu on the plot; locked Y-axis range |
| `TempMeasGUI.h` | New Y-axis range items (set, lock, unlock) in the plot's right-click menu; data logging thread `RunWorkerCompleted` event (asks save / discard) |
| `CHANGELOG.md`, this README | Notes |

Note: `RMH_Application_ThermalViewer.cpp` uses an older Windows text encoding. Upload it as a file on GitHub (do not copy-paste it).

---

## 4. Settings you can change

**Start-up** (top of `src/RMH_Application_ThermalViewer.h`):

| Setting | Default | Meaning |
|---|---|---|
| `_AutoStart_Enabled` | 1 | 0 = no automatic start-up at all (manual start as before) |
| `_AutoStart_ShowPopupWindows` | 1 | 0 = no information windows (the sequence still runs) |
| `_AutoStart_CameraIndex` | InfiRay T2S+ V2 | Camera selected at start-up |
| `_AutoStart_ConnectDelayMs` | 1500 | Wait after the main window opens before connecting |
| `_AutoStart_StreamWindowDelayMs` | 4000 | Wait after connecting before switching to the stream window (raise to ~6000 if the first calibration looks off) |
| `_AutoStart_CalibrateDelayMs` | 1500 | Wait after switching to the stream window before pressing Calibrate Camera (Q) |
| `_AutoStart_PeriodicCalibrationPeriodSec` | 180 | Period of the periodic shutter calibration |

**Temperature** (top of `src/RMH_ThermalCameraSupport_Library.cpp`, near line 440):

| Setting | Default | Meaning |
|---|---|---|
| `_Pool3_NonUniformityCorrectionLevel` | 6000.0 | Gray level of the shutter reference. Leave alone. |
| `_Pool3_PixelGain` | 1.00 | Signal gain, 1.00 = none. Only change with good reference data. |
| `_Pool3_UseFixedShutterTemperature` | 0 | 1 = use a fixed shutter temperature instead of the camera register (fallback) |
| `_Pool3_AssumedShutterTemperatureC` | 18.0 | The fixed value used by the fallback above |

---

## 5. Building it

The GitHub Actions build only starts automatically for pushes to `main`. For this branch:
**Actions -> build -> Run workflow -> choose `IRCAM-for-T2Splus_V2_2Dplots_expansion_17` -> Run workflow**, then download the build from the run
page. If a build fails, search the log for `error C` (ignore the `warning` lines) to find the real error.

Fallbacks: Build #7 (commit `7b355e8`, branch `fix-t2s-v2-temperature`) and the branches `t2s-v2-calibration-diagnostic`,
`IRCAM-for-T2S-V2`, `IRCAM-for-T2Splus_V2_2Dplots_fix` and `IRCAM-for-T2Splus_V2_2Dplots_expansion_01`, `_02`, `_03`, `_04`, `_05`, `_06`, `_07`, `_08`, `_10`, `_12`, `_13`, `_15` and `_16`.

---

## 6. Test results (informal)

| Test | Result |
|---|---|
| Tape on a thermometer | Thermometer 21.8 °C, InfiRay app 21.0 °C, this version very close |
| Hot tea | Windows software 59 °C, IRCAM about 56 °C |
| Hot water, 80-82 °C | IRCAM about 1-3 °C below the Windows software |
| Soldering iron, up to 150 °C | IRCAM about 5 °C higher, but the iron was still heating and programs were switched between readings, so not conclusive |
| Automatic start-up | Works (tested) |
| Plot: OpenGL window size fix | Works on the old Windows 10 PC (Intel HD 2000) and on a Windows 11 laptop |
| Plot: starts without visiting Settings | Not tested yet |
| Plot: Y-axis range set / lock / unlock | Not tested yet |
| Data Log window | Not tested yet |
| Stop Logging / Clear View confirmation windows (Yes / No) | Not tested yet |
| Temporary data log file: save (Yes) and discard (No) when a session ends | Not tested yet |
| Log file name format `DataLogSession_12Nov2026_09-57-39.txt` | Not tested yet |
| Data Log graph: right-click menu item Temperature At The Cursor shows / hides the temperature at the mouse cursor; left click does nothing | Not tested yet |
| Data Log graph: X / Y number of ticks (Auto, fixed numbers), also while zoomed | Not tested yet |
| Data Log graph: Set Y-Axis Range And Lock, Lock Y-Axis At Current Range, Unlock Y-Axis | Not tested yet |
| Data Log graph: check box per line shows / hides the line; all ticked at the start | Not tested yet |
| Data Log graph: Set X-Axis Range And Lock (valid and invalid times), Lock X-Axis At Current Range, Unlock X-Axis; zoom and ticks inside a locked range | Not tested yet |
| Data Log graph: Set Graph Title shows a bold title at the top center; empty text removes it; the title is in the exported image | Not tested yet |
| Data Log graph: Export Graph first shows the title window with the current title; Set Title continues, Cancel cancels the export | Not tested yet |
| Data Log graph: Export Graph saves PNG / JPEG / BMP with the shown lines named above the graph | Not tested yet |
| Data Log graph: left-drag rectangle zooms both X and Y; Reset Zoom works | Not tested yet |
| Warning window for more than 100000 samples: Start starts, Cancel does not start; no window for alarm / trigger starts | Not tested yet |

---

## 7. Known limits

- Good enough for PCB and hardware checks (roughly +/- 2-3 °C up to about 100 °C). Not for work needing better than about 1 °C.
- Above about 120 °C (outside the low range) is **untested**; the high range is untested for this camera.
- The manufacturer's software also applies a per-pixel gain map read from the camera. It is not copied, so a small scale
  difference at higher temperatures is possible.
- "Camera core" shows the same value as "Camera shutter" (the V2 has no separate core sensor).
- The distance for this camera type may show 2 m instead of 1 m; the effect is small (about 0.4 °C at 60 °C).
- Recordings made with the old code lost the detector value. `tests/tools/ircam_raw.py` still has the old logic for this camera.
- If the start-up sequence misbehaves, set `_AutoStart_Enabled` to 0 and rebuild.

---

## 8. Sources used

The InfiRay Android APK (XthermNew 6.6.230906), the analysis of the InfiRay Windows viewer, IR-Py-Thermal, and
`dzaczek/t2s-thermal-macos` (which note that the V2 shutter sensor drifts).
