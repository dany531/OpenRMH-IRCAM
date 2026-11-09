# IRCAM for T2S+ V2 (branch `IRCAM-for-T2Splus_V2_2Dplots_expansion_04`)

A modified version of **OpenRMH-IRCAM** (based on v3.0.0-community.5) for the **InfiRay T2S+ V2 / T2S+_A2** thermal camera
(USB ID `VID_3474&PID_6002`). Personal reference notes, last updated 10 November 2026. Not an official release.

- Fork: https://github.com/dany531/OpenRMH-IRCAM
- Original project: https://github.com/Cyclotronic/OpenRMH-IRCAM
- This branch contains **all** changes made so far: correct temperatures, automatic start-up, the 2D plot fixes and extras, and the Data Log window.

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
- **Show Log File** opens the CSV file in the Windows explorer (the file is saved on the Desktop). **Clear View** empties the table
  (the file is not changed). **Auto-scroll** keeps the newest row visible.
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
| `RMH_Application_ThermalViewer.cpp` | Start-up routines (select camera and connect, enable periodic calibration) and applying the plot data set settings |
| `RMH_Application_ThermalViewer.h` | Start-up settings and declarations |
| `MainGUI.h` | Start-up timer (switch to the stream window, press Q), the three information windows, applying the plot settings, and the Data Log menu item |
| `DataLogGUI.h` (new) | The Data Log window (table that follows the data logging file) |
| `GlobalObjectsAndVariables.h/.cpp` | Flags for the Data Log window |
| `RMH_OpenGL_2DPlot.h` | OpenGL window created at the panel size and resized with it; right-click menu on the plot; locked Y-axis range |
| `TempMeasGUI.h` | New Y-axis range items (set, lock, unlock) in the plot's right-click menu |
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
**Actions -> build -> Run workflow -> choose `IRCAM-for-T2Splus_V2_2Dplots_expansion_04` -> Run workflow**, then download the build from the run
page. If a build fails, search the log for `error C` (ignore the `warning` lines) to find the real error.

Fallbacks: Build #7 (commit `7b355e8`, branch `fix-t2s-v2-temperature`) and the branches `t2s-v2-calibration-diagnostic`,
`IRCAM-for-T2S-V2`, `IRCAM-for-T2Splus_V2_2Dplots_fix` and `IRCAM-for-T2Splus_V2_2Dplots_expansion_01`, `_02` and `_03`.

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
