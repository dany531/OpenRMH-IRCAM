# IRCAM for T2S+ V2 (branch `IRCAM-for-T2Splus_V2_2Dplots_expansion_29`)

A modified version of **OpenRMH-IRCAM** (based on v3.0.0-community.5) for the **InfiRay T2S+ V2 / T2S+_A2** thermal camera
(USB ID `VID_3474&PID_6002`). Personal reference notes, last updated 21 November 2026. Not an official release.

- Fork: https://github.com/dany531/OpenRMH-IRCAM
- Original project: https://github.com/Cyclotronic/OpenRMH-IRCAM
- This branch contains **all** changes made so far: correct temperatures, automatic start-up, the 2D plot fixes and extras, the Data Log window, the confirmation / save questions of data logging, readable log file names, the mouse cursor temperature in the Data Log graph, a warning for sessions with many samples, zooming in both axes, the right-click menu of the Data Log graph a check box per graph line an Export Graph button and X-axis range lock in the Data Log graph menu a graph title that is asked for again when exporting, colored Start / Stop Logging button borders and a corrected Data Log menu item, and a working scroll bar for the zoomed graph, a question when the application is closed while data logging is running, frozen plot / data logging values during a camera shutter calibration (no more spikes or dips), a frame around the Data Log graph, the first and last value of both axes written in an exported graph image, a Clear Graph button, an orange Reset Zoom border while the graph is zoomed, lighter gray scroll bars / arrows / Reset Zoom buttons on the zoomed graph, and the thermal camera configuration plus export information (date, time, camera model, calibration, session), and crash recovery of unsaved data logging sessions at start-up (branch 26) listed in an exported graph image, a zoom reset at the start of a new logging session, a fix for swapped Ambient / Reflected temperature when the temperature unit changes, an **Add Notes** button and a table with notes below the exported graph, 20 % bigger text and a higher resolution of the exported image, a **Show Graph Title** check box, round X-axis tick steps, a fixed plot area size so the Data Log graph no longer jumps sideways while logging, and **live data logging settings**: the session duration, the sample interval and the Temperature Plot Data Set Settings (enable check box, source, line color, line width) can be changed while a session runs, and a data set that is enabled during a session is added to the log and to the Data Log graph from that moment on, and (branch 27) **the log file is read in blocks** (a very long log file opens without a memory spike and without the 2 GB limit) and **the oldest part of a long Data Log graph is thinned out with min / max decimation** instead of scrolling off the left side, and (branch 28) **two-cursor measurement in the Data Log graph** (cursors A and B: time difference, temperature difference and slope in °C/min per line, also in the exported image), and (new in this branch) **typed cursor times** (enter A and B as exact elapsed times).

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
- **Show Log File** opens the log file in the Windows explorer. **Clear Graph** empties the table and graph (the file is not changed). **Auto-scroll** keeps the newest row visible.
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
- **Clear Graph** asks **"Are you sure you want to clear the graph and the table?"**. **Yes** clears; **No** closes the window and nothing changes.
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
- Note: if the program crashes during a session, the temporary file stays in the temporary folder. It is offered for save / open / delete at the next start-up (see 1.34).

### 1.7 Readable log file names (new)

A session file is named **`DataLogSession_ddMMMyyyy_HH-mm-ss.txt`**, for example `DataLogSession_12Nov2026_09-57-39.txt`:
12 = day, Nov = month (Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec - always in English), 2026 = year,
09 = hour, 57 = minutes, 39 = seconds. (Before: `DataLogSession_` followed by a run of digits.) The file is still a text file with
the delimiter you chose in the settings; change `.txt` to `.csv` in `RMH_ThermalViewer_StartDataLogging` if you prefer that extension.

### 1.8 Mouse cursor temperature in the Data Log graph (new, changed in an earlier branch)

Right-click in the Data Log graph and tick **Temperature At The Cursor** (see 1.11). A label follows the mouse and shows the
**temperature at the cursor** (in the program's temperature unit) and the **elapsed time** at that position, for example
`Temperature: 25.43 C   (0:01:23)`. Outside the plot area it shows `Temperature: N/A`. The label hides when the mouse leaves the graph.
Untick the item to switch it off. Before that change this was switched on and off with a left click; the left click now does nothing.

### 1.9 Warning for sessions with many samples (new)

The Data Log window keeps at most **100000 points per line in the graph** and shows the **newest 20000 rows in the table**. The log file
always holds every sample. When the session is set to more samples than the graph keeps in full detail (duration divided by sample interval, for
example more than about 27.7 hours at 1.00 s), pressing Start Logging shows a window that explains this:

- Since branch 27 the **oldest part of the graph is thinned out** (of every few neighbouring points the lowest and the highest are kept, so peaks and dips stay visible, see 1.36) and the oldest rows leave the table; the newest are added on the right. (With `_DataLogView_ThinOldPoints` set to 0 the oldest samples scroll off the **left** side of the graph, as before branch 27.)
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
- **X-Axis Number Of Ticks** - Auto, 5, 10, 20, 25 or 50 ticks on the time axis. This is the **maximum** number of intervals: the step is rounded up to a round time (see 1.31).
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

Above the graph (below the Start Logging / Stop Logging / Show Log File / Clear Graph / Reset Zoom / Add Notes / Export Graph buttons) there is one **check box per
graph line**, with the line's name in the line's color. Un-tick a box to hide that line, tick it to show it again, for example to look at
2 of 10 logged data sets. All lines are ticked when a log is opened or a new session starts. The check boxes replace the legend of the graph.
A hidden line is only hidden from the graph: its data keeps being collected and stays in the table and in the log file. The automatic
Y range and the Y tick / range settings only use the lines that are shown.

### 1.13 Export Graph as an image (new)

The **Export Graph** button, next to Start Logging, Stop Logging, Show Log File, Clear Graph, Reset Zoom and Add Notes, saves the Data Log graph as an image.
First the **Graph Title** window opens (also when the title is hidden with **Show Graph Title**, see 1.29) (see 1.15). It shows the title that is set now, so you can keep it, edit it or replace it (an empty text removes the title); **Set Title** continues, **Cancel** cancels the export. Then the **Graph Notes** window opens (the same window as the **Add Notes** button, see 1.27) with the same behavior: it shows the notes that are set now, **Set Notes** continues, **Cancel** cancels the export. Then a save window opens (file types **PNG**, **JPEG** and **BMP**; the format follows the extension you type, PNG by default) with the session file
name plus `_graph_` and the **time of the export** (`HH-mm-ss`) as the suggested name, for example `DataLogSession_21Nov2026_12-57-55_graph_12-59-55` (changed in branch 29: before, the name did not change between exports and the save window asked to overwrite; the date and time in the middle are those of the session start). When there is no session file name it is `DataLogGraph_` and the date and time.

- The image is the graph **as it is shown now**: zoom, number of ticks and locked Y range are included; lines you hid with the check boxes (1.12) are not.
- It has the size of the graph on the screen. A strip above the graph names the lines that are shown, each name in the color of its line.
- The strip also has a line with the first and last value of the time axis and of the temperature axis (see 1.21).
- Below the graph the image has a table with the thermal camera configuration, the export information, other information and the notes (see 1.24 and 1.27).
- All text apart from the values on the axes (line names, axis ranges, table, title of the graph, titles of the axes) is 20 % bigger than those values (see 1.28).
- The image has 3 times more pixels in width and height than the graph on the screen, so lines and text stay sharp (see 1.30).
- The image only has the title when **Show Graph Title** (1.29) is ticked; the **Graph Title** window opens in both cases.
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

Tick **Show Graph Title** (above the graph, 1.29): the title appears in bold at the **top center above the graph** and is part of the image saved with **Export Graph** (1.13), so
you can see later what the graph was about. Until you type one, the title is the text `Title`.

**Edit the title directly on the graph:** double-click the title. A text box opens on it: type the title (up to 200 characters) and press **Enter** (or click somewhere else) to keep it,
**Esc** to throw the change away. A new title is shown (it ticks **Show Graph Title** if needed). An **empty** text hides the title (the check box is unticked and the text goes back to `Title`).

The same window as before (**Graph Title**, with the current title filled in) opens when you press **Export Graph** (1.13), **also when the title is hidden**: type a new title to show it, empty the text to
hide it, or press **Set Title** with the text as it is to keep the title as it is (shown or hidden). **Cancel** cancels the export. The right-click menu item **Set Graph Title...** is removed, because the
title is edited directly. The title stays when you use Clear Graph or start a new session, but it is not saved: it is `Title` and hidden again when the program is restarted.

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
can still be left in that folder after a crash, a power loss or a killed process; it holds the samples written so far. It is offered for save / open / delete
at the next start-up (see 1.34).

### 1.19 No spikes or dips during a shutter calibration (new)

When the camera does its shutter calibration (the periodic one every 180 s, the temperature-drift-based one, the **Calibrate Camera** button, or the
one after a temperature range switch) the plot and the data logging file used to show spikes or dips. During the calibration the shutter is closed
(the camera sees a uniform surface, so maximum, minimum and center all move towards the shutter temperature), the non-uniformity map and the
temperature look-up table are regenerated, and the first frames after the shutter reopens are not reliable yet.

Now the **last good value of every data set is frozen** from the start of the calibration until **300 ms after it has finished**:

- The **2D plot** (Plots window) shows a flat line for that time instead of a spike or dip.
- The **data logging file** (and with it the Data Log window) gets the repeated last good value for the samples that fall in that time. With a 1 s
  logging interval this is about one or two samples per calibration.
- The CSV format is **not changed**: there is no extra column, so the Data Log window and the MATLAB examples keep working.

The frozen samples are repeated values, not measurements. If you need to know which samples were affected, look at the calibration messages in the
information area, or compare with the calibration period (180 s by default). The settle time is `_CalibrationFreezeSettleTimeMs` (see section 4).

### 1.20 Frame around the Data Log graph (new)

The plot area of the Data Log graph now always has a **border on all four sides**, like **Show Plot Border Box** in the 2D plot of the Plots window.
Before, only the left (Y) and bottom (X) axis lines were drawn. There is no right-click item to show or hide it: it is always shown. The frame has the
color of the axis lines (gray), does not change the layout of the graph and is part of an image saved with **Export Graph** (1.13).

### 1.21 Last X and Y value in the exported graph (new)

The labels on the axes only show the tick marks, so when the end of an axis does not fall on a tick (zoomed view, locked range, number of ticks, automatic
range) the maximum duration and the maximum temperature could not be read in an image saved with **Export Graph**. Now the strip above the graph, below
the names of the lines, always has a line like:

`Time axis: 0:00:00 to 1:02:37 (h:mm:ss)      Temperature axis: 20.5 to 134.2 °C`

- The values are the **first and last value of each axis as shown at the moment of the export**: zoom, locked X / Y range and automatic range are followed.
- The line is split into two rows (time, temperature) when the graph is too narrow for one row.
- Only a **zoomed** time range shorter than one minute is written with milliseconds (`h:mm:ss.fff`, e.g. `0:12:03.400 to 0:12:41.900`). In every other case (not zoomed, a locked range, a range of 60 s or more) the time is `h:mm:ss`, cut to whole seconds like the axis labels. The temperature has one to three decimals.
- These are the ends of the axes, not the highest sample of a line. With an automatic range the end of the axis can be a little above the highest sample.
- The graph itself and the size of the graph part of the image are not changed; the strip is just one row (or two) taller.

### 1.22 Clear Graph button and zoom indicator (new)

- The **Clear View** button is now called **Clear Graph**. It still empties the table **and** the graph (only data logged from now on is shown; the file is
  not changed), so its question reads "Are you sure you want to clear the graph and the table?".
- The **border of the Reset Zoom button turns orange** while the graph is zoomed in (time axis and / or temperature axis), so you can see at a glance that
  you are looking at a part of the graph. After **Reset Zoom** (or when a range is locked / unlocked, which also resets the zoom) the border goes back to the
  normal gray of the other buttons. Only the border changes, not the text or the background. The colors are in `UpdateZoomIndicator` in `DataLogGUI.h`
  (`FromArgb(255, 140, 0)` orange, `FromArgb(100, 100, 100)` normal).

### 1.23 Lighter scroll bars on the zoomed graph (new)

When the Data Log graph is zoomed, a scroll bar shows under the time axis and beside the temperature axis, with an arrow at each end and a small **Reset Zoom
button** next to the arrows. These were hard to see on the dark graph. On **both axes** they are now light gray:

| Part | Color (R / G / B) |
|---|---|
| Track of the bar | dark gray, 70 / 70 / 70 |
| Buttons: the two arrows, the Reset Zoom button and the sliding part | light gray, 200 / 200 / 200 |
| Symbols and outline on the buttons | dark, 40 / 40 / 40 |

The three colors are the macros `_DataLogScrollBar_TrackColor`, `_DataLogScrollBar_ButtonColor` and `_DataLogScrollBar_LineColor` at the top of `DataLogGUI.h`;
change the numbers there if you want a different gray. `_DataLogScrollBar_ApplyColors` (1 or 0) switches the colors on or off, to compare with the default chart colors. How the scroll bars work (1.17) is not changed. The Reset Zoom button at the end of the scroll bar is not the
**Reset Zoom** button above the graph (1.10), but does the same for its axis.

### 1.24 Table with the camera configuration and information in the exported image (new)

**Export Graph** (1.13) adds a **table below the graph** for later reference. From left to right it has these cells (heading on a darker band, then the lines
with the label on the left and the value next to it):

| Thermal Camera Configuration: | Export Information: | Other Information: | Notes: |
|---|---|---|---|
| Temperature Correction: 0.0 °C | Date: 17 Nov 2026 | Log File: DataLogSession_17Nov2026_14-05-12.txt | Your notes, wrapped in the rest of the width of the image |
| Ambient Temperature: 23.0 °C | Time: 14:32:07 | Camera Model: InfiRay T2S+ V2 | |
| Reflected Temperature: 23.0 °C | Logged Time: 0:27:14 | Camera Temperature Range: Low | |
| Surrounding Humidity: 50 % | Session Duration Setting: 1:00:00 | Auto Calibration: On, every 180 s | |
| Object Surface Emissivity: 0.95 | Sample Interval: 1 s | Drift-Based Calibration: Off | |
| Surface Distance: 1 m | Samples In Data Log: 1635 | Temperature Unit: °C | |

- **Thermal Camera Configuration:** the six settings of the Thermal Camera Configuration panel (Settings): Temperature Correction, Ambient Temperature,
  Reflected Temperature, Surrounding Humidity, Object Surface Emissivity and Surface Distance. Temperatures use the temperature unit of the program (°C, °F or K).
- **Export Information:** **Date** and **Time** of the export (not of the measurement; the log file name holds the start of the session), **Logged Time**
  (elapsed time of the last sample in the graph), **Session Duration Setting** and **Sample Interval** (the data logging session settings) and **Samples In
  Data Log** (the number of rows read into the Data Log window).
- **Other Information:** **Log File** (`none` when there is no session file name), **Camera Model** (the camera selected in the program), **Camera
  Temperature Range** (Low / High), **Auto Calibration** (Off, or On with the period in seconds; the periodic shutter calibration), **Drift-Based Calibration**
  (Off, or On with the threshold in the program's temperature unit; the calibration triggered by the sensor temperature drift) and **Temperature Unit**.
- **Notes:** the notes of the **Add Notes** window (1.27). The cell is only in the table when there are notes.
- All values are the ones **at the moment of the export**, they are **not saved with the log file**. The camera configuration values are the ones in use by the
  camera (set with the **Set** button of the panel; text typed in the panel but not set yet is not used). If you export an older log later, or changed the
  settings after logging, the table shows the settings of that moment, not of the logging session. **Sample Interval** and **Session Duration Setting** have
  the same limit. Export while the settings are still the ones used during the measurement.
- When the image is too narrow for all cells in one row, the cells that do not fit go to a next row; the notes get at least 220 pixels, or a full row.
- The image is taller by the height of the table. The graph itself and the strip above it (line names, axis ranges) are not changed.

### 1.25 Zoom reset for a new logging session (new)

When a new data logging session starts, the Data Log graph automatically goes back to the **whole view** on both axes (like **Reset Zoom**), and the border of the
Reset Zoom button goes back to gray. This applies however the session was started (Start Logging, hotkey **M**, the plot's right-click menu, an alarm or trigger
event). Before, the zoomed range of the previous session stayed, so a zoomed time range that the new session had not reached yet showed an empty graph.
A **locked** X or Y range (1.11, 1.14) is not a zoom and is kept; unlock it from the right-click menu if the new session needs a different window.

### 1.26 Fix: Ambient / Reflected temperature swapped on a unit change (bug fix)

Changing the temperature unit (°C / °F / K) used to write the Reflected temperature into the **Ambient** box and the Ambient temperature into the **Reflected** box of
the Thermal Camera Configuration panel. It was only visible when the two values differ, and **Set** afterwards would have written the swapped values to the camera.
Each box now gets its own value. This bug comes from the original code, not from this branch.

### 1.27 Add Notes button and notes in the exported image (new)

The **Add Notes** button, between **Reset Zoom** and **Export Graph**, opens the **Graph Notes** window. Type notes about the logging (up to 2000 characters, **Enter**
starts a new line) and press **Set Notes**; **Cancel** keeps the notes as they were. An empty text removes the notes.

- The notes are written in the **Notes** cell of the table below the graph in an exported image, on the right of **Other Information** (1.24). They wrap to the
  width of the cell. Without notes there is no Notes cell.
- It works like the **Graph Title** (1.15): when you press **Export Graph**, the **Graph Notes** window opens after the **Graph Title** window. It shows the notes that
  are set now, so you can keep, edit or empty them; **Set Notes** continues with the save window, **Cancel** cancels the export.
- Like the title, the notes **stay** when you use Clear Graph or start a new session, until you change them or close the program (they are not saved with the log
  file and not saved between program runs). Check or empty them before the next export.
- The buttons moved to make room: **Export Graph** is now a little narrower and **Auto-scroll table to newest** is further right; the Data Log window is wider when it is opened as a separate window (1130 pixels, see also 1.29). When it is docked in the main window its width follows the main window.

### 1.28 Text size in the exported image (new)

In the exported image all text **apart from the values on the axes** (for example `20.0` and `0:00:00`) is **20 % bigger** than those values: the names of the lines, the first / last
axis values, the table, the title of the graph and the titles of the axes (**Time (h:mm:ss)** and **Temperature**). The size is read from the graph itself, so it follows the axes. The values on
the axes keep their size. The rows of the table and the colored squares in front of the line names follow the text size. The factor is fixed (1.2, the constant `TextScale` in `ExportGraphButton_Click`). The title and the axis titles on the screen keep their
size: they are only made bigger while the image is made.

### 1.29 Show Graph Title check box (new)

A new check box **Show Graph Title** sits before **Auto-scroll table to newest**, at the top right of the Data Log window. It is **not ticked** by default.

- **Ticked:** the title is shown above the graph and in an exported image. When no title was typed yet, it shows `Title`. Double-click the title to edit it (1.15).
- **Not ticked:** the title is hidden on the graph and in the exported image, but the **text is kept**.
- **Export Graph** opens the **Graph Title** window **in both cases** (1.13, 1.15). Typing a new title there shows it (ticks the check box), an empty text hides it (unticks it), and the same text keeps it as it is.
- Hover the check box to see a hint. The check box is not saved between program runs: it is not ticked when the program starts.

### 1.30 Higher resolution of the exported image (new)

The exported image has **3 times more pixels in width and height** (9 times more in total) than the graph on the screen, for example 3000 x 2100 pixels instead of 1000 x 700. The graph is taken
from the chart control as a **vector drawing (EMF+)** and drawn at the high resolution, so the lines and the text of the graph stay sharp instead of being a stretched picture. The text and the
table around the graph are drawn at the high resolution too. If the vector drawing cannot be made, the picture of the graph is used (stretched, so less sharp). **JPEG** images are saved with quality 95
(the framework default is 75, which blurs text). The file is bigger: a PNG of a typical graph is about 1 to 3 MB, a BMP is about 30 MB or more.

The settings are macros at the top of `DataLogGUI.h`:

| Macro | Default | Meaning |
|---|---|---|
| `_DataLogExport_ResolutionScale` | 3 | Times more pixels in width and height (1 = the size of the graph on the screen, 4 = even sharper and bigger files) |
| `_DataLogExport_UseVectorChart` | 1 | 1 = the graph as vector drawing (sharp); 0 = the graph as a picture of the screen size (to compare, or when 1 does not look right) |
| `_DataLogExport_JpegQuality` | 95 | JPEG quality, 0 to 100 |

### 1.31 Fix: X-axis number of ticks made the graph resize and the last time value come and go (bug fix, round tick steps)

With **X-Axis Number Of Ticks** set to anything other than **Auto**, the Data Log graph changed its size and the last time value on the axis appeared and disappeared at seemingly random
moments while logging. This was in earlier versions too. The tick interval was the range divided by the number of ticks, a different odd value at every refresh (every second), written to the
axis together with the range, so the chart laid out the axis again every second and small differences decided whether the last label fitted.

Now the interval is a **round time step** that is only written to the axis when it changes, and nothing else of the time axis is set (the range is automatic, like with **Auto**):

- The step is the **smallest round step that gives at most N intervals** over the range: 1, 2, 3, 5, 10, 15, 20, 30 seconds, then the same in minutes, then 1, 2, 3, 6, 12 hours and whole days (for a deep zoom
  also 1 to 500 milliseconds). The ticks are at round times (for example every 10 minutes: 0:10:00, 0:20:00, ...), not at odd times.
- So **N is a maximum**: with 10 ticks and a 1 hour log the step is 10 minutes (6 intervals); the number you get is between about N / 2 and N. A higher N gives smaller steps.
- While logging the step stays the same until the log has grown into the next step (for example from 10 to 15 minutes), so the axis only changes then, like with **Auto**.
- The range for the step is the locked X range (1.14), the zoomed view, or the whole log (from 0:00:00 to the newest point). Zooming gives a smaller step.
- **Auto** is unchanged. The temperature ticks (Y) are not changed.

This did not stop the graph from jumping sideways while logging (also with **Auto**); that is fixed in 1.32.

### 1.32 Fix: the Data Log graph jumped horizontally while logging (bug fix, fixed plot area)

While logging, the Data Log graph changed its width at the right edge, as soon as a new last X value appeared. It happened with **X-Axis Number Of Ticks** on a fixed number and also on **Auto**,
so the round tick steps of 1.31 were not the (only) cause. The chart was laying out its plot area (the part inside the axes) automatically, and that layout changes size whenever the width of an
axis label changes, for example a time label on the right end of the time axis that comes and goes while the log grows.

Now the plot area has a **fixed size**:

- It is set in pixels from each side of the graph and converted to percentages for the chart: **left 75, right 40, top 12, bottom 55** pixels (room for the temperature title and labels, half of the last
  time label, and the time labels and title). While the **graph title** is shown, about 30 pixels are taken off the height.
- It is only written when it changes (the window is resized, the graph title is shown or hidden), not at every refresh, so the chart is not laid out again every second.
- **While the graph is zoomed, the automatic layout is used**, because the zoom scroll bars need room. The plot area does not change by itself during a zoom.
- The exported image uses the same fixed plot area (percentages), so it only gets a little more empty space around the plot.
- Tick settings, the Y range, the X range, the zoom and the right-click menu are not changed.

If an axis title or a label is cut off, or there is too much empty space, change the margin constants (`LeftMarginPixels`, `RightMarginPixels`, `TopMarginPixels`, `BottomMarginPixels`,
`TitleHeightPixels`) at the top of `UpdatePlotAreaLayout` in `DataLogGUI.h`.

### 1.33 Live data logging settings: duration and plot data sets can be changed while logging (new, changed in branches 23 - 25)

Before, the settings were read once when **Start Logging** was pressed; changing them during a session had no effect. Now they are applied to the running session.

**Duration (Data Logging Session Settings)**

- Change the Duration Hours / Minutes / Seconds while a session runs and the new value is used at once. It is the **total length of the session, counted from the moment logging was started**:
  1 minute changed to 1 hour after 20 seconds keeps logging until 1 hour after the start (not 1 hour after the change). A status line tells the new duration.
- **The new duration must be longer than the time already logged.** If it is not (for example 10 s when 30 s are logged), the session is **not** ended: a warning window explains it, the duration boxes are
  set back to the duration of the running session, and the session keeps running. (Because every box is checked when you change it, going to a much shorter duration may need a certain order: for example from 1 h 0 min 0 s
  to 0 h 5 min 0 s, change the minutes first and then the hours. To end the session earlier, press **Stop Logging**.)
- The duration is calculated as `unsigned long` (the hours box goes up to 719, which overflowed a signed 32 bit integer in milliseconds).
**Sample interval (Data Logging Session Settings)**

- Change the Logging Sample Interval while a session runs and it is used at once. The logging thread reads the interval again in every 50 ms step of its wait: a longer interval makes the sample that is
  being waited for come later, a shorter one makes it come as soon as the new interval is over, counted from the previous sample (if that time has already passed, the sample is taken at once).
- The time of a sample (`Time(ms)` column and the session timer) is now the **time that was really waited** (before: the interval, added once per sample). It is the same with an unchanged interval, and stays correct when the interval changes.
- The interval is rounded to a whole millisecond when it is read (0.29 s is 290 ms; before it could become 289 ms) and never goes below 40 ms.
- A status line tells the new interval. When the duration or the interval is changed so that the session would have more samples than the Data Log window shows (1.9), a status line says so (no window); all samples are still saved in the file.
- The end of the session uses the time of the last wait: with a changing interval the last sample can be up to one (last) interval later than the duration, as with a fixed interval that does not divide the duration.


**Temperature Plot Data Set Settings**

| Change while logging | What happens |
|---|---|
| **Check box checked** for a data set that is **not** part of the session | The data set is **added to the session as a new column**, logged from the next sample on, and the **new line is shown in the Data Log graph** and table from the moment it was enabled (the rows before have no value in it). The new column is the next one in the file; it uses the data source selected at that moment. |
| **Check box unchecked** (data set that is part of the session) | The column stays in the file and **`NaN` is logged** in it (a gap in the graph) until the check box is checked again. |
| **Source ComboBox** changed (data set that is part of the session) | The column is logged from the **new source from the next sample**. The last value of the old source is not reused. A note line `#<column number>=<new name>` is written in the file, and the Data Log window renames the table header, graph legend and line check box of that column. |
| **Line color / line width** changed | The 2D plot changes at once (as before) and now the **line in the Data Log graph** (and its check box color) follows it at its next refresh (about 1 s). |

The lines of the Data Log graph now use the **color and the line width of the plot data set** that feeds the column (before, a fixed color list by column number, line width 1). With the default settings this looks the same.

Notes on the file:

- The header line of the file is not changed, so it keeps the **names from the start of the session**. A `#<column number>=<name>` note line is written when a column is **added** (the next column number) or **renamed** (an existing column number); it shows where that happened. Programs that read the CSV
  (for example Excel) see the note line as one row, and the rows before a new column are shorter (they have no value in it).
- The table header of the Data Log window shows only the newest name of a column; the earlier rows of that column are not relabeled.
- At most 10 columns (the 10 plot data sets). A data set that is unchecked from the start and never enabled is not in the file.

How it works (for later changes): the GUI thread sets `DataLoggingSessionDurationMilliSec`, `DataLoggingColumnEnabled[]`, the data logging source pointers and, last, `DataLoggingNumberOfActiveSets`; the logging thread reads the number of columns once per sample, `DataLoggingIntervalMilliSec` in every 50 ms step of its wait, and writes the note line first when `DataLoggingColumnSourceChangedFlag[]` is set. The mapping column -> plot data set (`DataLoggingColumnDataSetIndex[]`) and the column names are filled in `RMH_ThermalViewer_StartDataLogging` and in `RMH_ThermalViewer_ApplyLiveDataLoggingDataSetChange`. The duration is handled in `RMH_ThermalViewer_UpdateDataLoggingSessionDuration` and the interval in `RMH_ThermalViewer_UpdateDataLoggingSampleInterval` (both call `RMH_ThermalViewer_WarnLiveSampleLimit`).
The Data Log window adds a column in `AddLogColumn` (also used for the header line), follows color / width in `ApplyLiveStyleChanges` (flag `DataLoggingStyleChangedFlag[]`, set in `RMH_ThermalViewer_Change2DPlotDataSetAndSettingsPanelColor` and `RMH_ThermalViewer_Change2DPlotDataSetLineWidth`) and handles the note lines in `ApplySourceChangeNote`.

### 1.34 Crash recovery of unsaved data logging sessions (new, branch 26)

A session is written to a temporary file in `%TEMP%\IRCAMThermalViewer_DataLogging` while it runs. When it ends normally you are asked whether to save it. After a crash, a power loss or a killed process that
question is never asked, so the file used to stay in the folder unnoticed. Now IRCAM looks in that folder **every time it starts** (after the main window is shown, before the automatic
start-up sequence) and, when it finds session files, shows the **Unsaved Data Logging Sessions Found** window.

The window lists the files, newest first: **Session File**, **Last Written**, **Samples**, **Logged Time**, **Size**. Select one or more files and press:

| Button | What happens |
|---|---|
| **Save** | The selected file(s) are moved to the default save path (Settings). An existing file is never overwritten: `_recovered` (then `_recovered2`, ...) is added to the name. The temporary file is deleted only when the copy has the same size; on an error a window is shown and the file is kept. |
| **Open** | (One file.) The window closes and the file is shown in the Data Log window, opened as a **separate window** so the automatic start-up does not replace it. The file **stays** in the temporary folder, so you are asked again at the next start-up until you save or delete it (use Save or Delete the next time, or **Open Folder** in the Data Log window to copy it by hand). |
| **Delete** | Asks first (default **No**), then deletes the selected file(s) permanently. |
| **Decide Later** | Keeps all files; you are asked again at the next start-up. |

The window closes by itself when no file is left. Every action also writes a status line.

Details:

- A file **without any sample** (only the header line, or empty) is deleted without a question; a status line says how many.
- The **data delimiter** (comma, semicolon, colon, space, tab) is not stored anywhere, so it is read from the header line of each file. A file written with another delimiter than the current
  setting is shown correctly, and **Open does not change the delimiter setting** (the next session is written with the delimiter you set).
- A file that cannot be read is still listed (`?` in the Samples column) and is never deleted without your answer.
- **Two instances of IRCAM:** while a session runs, a small lock file `<session file name>.lock` is kept open next to the temporary file. A second IRCAM that starts meanwhile sees it and does **not**
  offer the running session. Windows removes the lock file when the program ends, also after a crash, and a lock file left by a power loss is recognised as not held.
- If the default save path is the temporary folder itself, nothing is offered (those files are saved sessions).
- The check never stops the start-up: if it fails, a status line says so.
- The Data Log window does not show the notice about the session duration when it is opened for a recovered file.

How it works (for later changes): `RMH_ThermalViewer_CheckForUnsavedDataLoggingSessions` (called in `MainGUI_Shown`) scans the folder; `RecoveredSessionInfo::Analyse` reads a file line by line;
`CrashRecoveryDialog` is the window; `DataLogGUI::OpenRecoveredSession` sets the file and delimiter for the Data Log window (`GetViewDelimiter`); the lock file is created in
`RMH_ThermalViewer_StartDataLogging` and released in `RMH_ThermalViewer_FinishDataLoggingSession` (after the save / delete question).

### 1.35 The log file is read in blocks (new, branch 27)

**What was wrong.** The Data Log window read everything that was new in the log file **in one piece**: one array with the whole new part of the file, then one text (2 bytes per character), then one text per line. That
needs several times the size of the file in memory and freezes the window until it is done. The size of the array was `(int)` of a 64 bit file length, so for a file of more than 2 GB the size was wrong or negative - the error
was swallowed and the window stayed empty or showed wrong data. A session that was opened from the crash recovery (1.34) or a very long session made this visible.

**What it does now.**

- The file is read in **blocks of 512 KB** (`_DataLogView_ReadBlockBytes`). Memory use no longer depends on the size of the file.
- At most **200 ms** (`_DataLogView_ReadTimeBudgetMs`) are used per refresh. If the file is not finished, the window refreshes again after **50 ms** (normally 1000 ms) until it has caught up, so the program stays
  usable while a long file loads. The status line shows **Reading the log file: NN %**.
- The table and the graph are drawn **once**, when the window has caught up with the file - not again and again while it reads. Only the newest 20000 lines are kept for the table (their rows are made at the end), and the
  values go to point lists (see 1.36), so a file with millions of lines does not create millions of table rows or chart points.
- A UTF-8 character that is split over two blocks (or over two refreshes of a running session) is put together again (`Decoder`).
- A running session looks the same as before: the new lines of the last second are read and drawn each second.

How it works (for later changes): `RefreshLogView` reads the blocks (`ReadBuffer` / `CharBuffer`, `ViewDecoder`) and sets `LoadingBehind`; `ProcessNewText` reads the lines of one block (values into `SeriesBuffers`, the newest lines
into `PendingLines`); `ShowPendingData` makes the table rows and calls `SyncChartSeries` for each line; `ResetView` and the Clear Graph button reset all of it.

### 1.36 Min / max decimation of the oldest graph points (new, branch 27)

**Before.** Each line of the graph kept the newest 100000 points; older points were deleted, so in a long session the start of the session was no longer on the graph (it stayed in the CSV file).

**Now.** When a line has more than 100000 points, its **older half** is thinned out: of every **4 neighbouring points the lowest and the highest are kept** (real points, with their own time, in time order). Example: the
four points 20.1, 20.2, 35.0, 20.1 become 20.1 and 35.0, so a short peak is still visible - a plain "every 2nd point" would have lost it. The newer half is not changed, so **at least the newest 50000 samples always have full
detail**. Points that were thinned out are thinned out again the next time, so the older the data, the coarser it is. The whole session stays on the graph, the number of points never goes above 100000.

Details:

- The very first point of a line is always kept, so the left end of the graph stays at the start of the session.
- A group of 4 that contains a `NaN` (a gap in the line) also keeps its first `NaN`, so a gap stays a gap.
- Because the lowest and highest values are kept, the automatic temperature axis still shows the real lowest and highest temperature of the whole session.
- When points were thinned out, the line is filled again (a short pause of the window, about once per 25000 new samples; with 100000 points per line and several lines it can take a moment).
- **Limits:** zooming into an old part shows only the points that were kept (the tooltip / cursor temperature too); every sample is in the CSV file. The table is not thinned: it shows the newest 20000 rows.
- The warning window for sessions with many samples (1.9), the status line after a live change of the duration / interval and the notice of the Data Log window say this.
- `_DataLogView_ThinOldPoints` (`RMH_Application_ThermalViewer.h`): 1 = thinning (default), 0 = the old behavior (the oldest points are removed and scroll off the left side).

Checked outside the program with a copy of the algorithm (2 million samples with spikes and gaps, added in blocks of 20000): the lowest and highest value of the whole log were kept, the time stayed in order, the left end
stayed at the start of the session, there were never more than 100000 points and the gaps were kept. The program itself (C++/CLI) was **not compiled and not tested**.

How it works (for later changes): class `LogSeriesBuffer` (top of `DataLogGUI.h`) holds the points of one line; `LimitPoints` does the thinning (or the removal when `_DataLogView_ThinOldPoints` is 0); `SyncChartSeries` copies
the points to the chart line (appends new points, or fills the line again after a thinning).

### 1.37 Two-cursor measurement in the Data Log graph (new, branch 28)

**What it is for.** Reading a heating or cooling curve: how long did the temperature take from A to B, how many degrees did it change, and how fast (slope in °C/min)? Before, only the single mouse
cursor temperature (1.8) was available, so the two readings had to be subtracted by hand.

**How to use it.**

1. Right-click the graph -> **Measure With Two Cursors** (check item). The mouse pointer becomes a cross and a small panel in the graph says *Click in the graph to place cursor A*.
2. **Click** in the plot area: the first click places **cursor A** (yellow dashed vertical line), the second click **cursor B** (cyan dashed line). After that a click moves the **nearer** cursor.
   Keep the left button down and move the mouse to fine-tune a cursor, or type exact times (1.38). A cursor stays inside the logged time range of the lines that are shown.
3. The panel shows: the time of A and B (h:mm:ss.fff), **Δt (B - A)** in seconds (and h:mm:ss from one minute on) and, **for every line that is shown** (in the color of the line): the value at A and at B,
   **ΔT = value(B) - value(A)** and the **slope = ΔT / Δt in temperature unit per minute** (for example `+2.340 °C/min`; negative while cooling). The panel can be **moved with the mouse** (drag it).
4. **Measure With Two Cursors** switched off again: the cursors and the panel **stay** (so you can zoom with the rectangle, which is switched off while cursors are placed, and measure again). **Clear Cursors** removes them.

Details:

- The value of a line at a cursor is the sample itself or the **linear interpolation** between the two neighbouring samples. It shows `N/A` outside the logged time range of the line, in a gap (`NaN`), and the slope shows `N/A` when Δt is 0.
- Hiding a line (check box above the graph) removes it from the panel at once; new samples or a thinning of old points (1.36) update the values.
- The cursors are annotations of the chart: they follow zoom and axis changes and are **part of an exported image (1.13)**. The **readout panel is also in the exported image**, on the graph, exactly as in the Data Log window:
  same position in the graph (so move it before exporting if it covers something), same size, dark background, border, text rows, fonts and colors. It is drawn as vector text, so it stays sharp at the export
  resolution. Nothing is drawn while no cursor is placed. The **Other Information** table is not changed by the measurement.
- Cursors and panel are removed when the view is restarted (new session) and with **Clear Graph**. The mouse cursor temperature (1.8) can be used at the same time and also works while the mouse is on a cursor line (branch 29: before, it showed N/A there).
- While **Measure With Two Cursors** is on, dragging a rectangle does not zoom (switch it off to zoom); the scroll bars of a zoomed graph keep working.
- Only the time axis position is set by the mouse; the temperatures come from the data, not from the height of the click.

The program itself (C++/CLI) was **not compiled and not tested**. Things to check first: the cursor lines appear at the clicked time on the date / time axis, the lines stay in place while zooming, the exported image shows them, and the values
in the panel agree with the table of the Data Log window.

How it works (for later changes): `BuildMeasureObjects` creates the two `VerticalLineAnnotation` objects (`MeasureLineA`, `MeasureLineB`, axis bound to the time axis, infinite) and the panel (`MeasurePanel`);
`LogChart_MouseDown` / `LogChart_MouseMove` / `LogChart_MouseUp` place and drag; `GetLineValueAt` interpolates in `SeriesBuffers`; `CollectMeasureRows`, `BuildMeasureTexts` (panel) and `DrawMeasurePanelOnExport` (export: draws the panel into the image from the live panel and its rows) make the texts;
`UpdateMeasurement` fills the panel. The helper class `MeasureRow` is at the top of `DataLogGUI.h`.

### 1.38 Typed cursor times (new, in this branch)

**What it is for.** Placing a cursor with the mouse is only as exact as the pixel. To measure between two known moments (for example *heater on at 0:02:00* and *1:45:30 later*) the times of cursor A and B can be typed.

**How to use it.** Right-click the graph -> **Set Cursor Times...**. A window with two boxes opens:

- **Cursor A** and **Cursor B**: the elapsed time since the start of the log, **the same time as in the panel and on the time axis**. The boxes start with the current cursor times (empty for a cursor that is not placed).
- Accepted forms: `h:mm:ss`, `h:mm:ss.fff`, `m:ss`, `m:ss.f` or only seconds (`83.5`). A decimal point or comma, and an `s` at the end, are allowed (`1:23,5`, `83.5 s`). No sign.
- **Set** places the cursors and the panel shows the new values at once (Δt, ΔT, slope per line, as in 1.37). An **empty box removes that cursor**; both boxes empty removes both.
- A time that is not a time, or **outside the logged time range** of the lines that are shown (the range is written in the window), gets a message and the window opens again with your text, so nothing typed is lost. A time up to 1 ms beyond the first or last sample is accepted and moved to it. **Cancel** changes nothing.
- It works with **Measure With Two Cursors** on or off; a typed cursor can then still be moved with the mouse (mode on). Message "no data in the graph yet" when the graph is empty.

How it works (for later changes): `SetCursorTimesMenuItem_Click` (the window and the checks), `TryParseElapsedTime` (the text -> seconds), `RemoveMeasureCursor` (one cursor); the cursors are placed with the same `SetMeasureCursor` as for the mouse. The elapsed time is counted from `ChartStartTime`, like `FormatMeasureTime`.

The program itself (C++/CLI) was **not compiled and not tested**. Check first: a typed time puts the line at the same place as the same time read from the panel after a mouse click, and a wrong text keeps the window open.

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
| `RMH_ThermalCameraSupport_Library.cpp` | Temperature fixes (detector value saved, 6000 flat-field level, offsets, shutter handling, calibration floats); freeze of the plot / logging values set and cleared in `RMH_IRThermalCamera_CalibrateThermalCamera` |
| `RMH_ThermalCameraSupport_Library.h` | New data members (`FrameFPARawWord`, `CalValue6`) |
| `RMH_Application_ThermalViewer.cpp` | Start-up routines (select camera and connect, enable periodic calibration), applying the plot data set settings, temporary data log file, save / discard question (`RMH_ThermalViewer_FinishDataLoggingSession`), 50 ms stop check of the logging thread, readable log file name, warning window for sessions with many samples; `RMH_FreezeValueDuringCalibration` (holds the last good value during a camera calibration) used by the 2D plot feed and the data logging thread |
| `RMH_Application_ThermalViewer.h` | Start-up settings and declarations (incl. `RMH_ThermalViewer_FinishDataLoggingSession`); Data Log window limits (`_DataLogView_MaxRows`, `_DataLogView_MaxPlotPoints`); `AskUserAboutSampleLimit` parameter of `RMH_ThermalViewer_StartDataLogging` |
| `MainGUI.h` | Question when closing the application during data logging (`MainGUI_FormClosing`); start-up timer (switch to the stream window, press Q), the three information windows, applying the plot settings, and the Data Log menu item (column widths, padding and text copied from the Plots item so the text shows under the image) |
| `DataLogGUI.h` (new) | The Data Log window (table and graph that follow the data logging file); light gray scroll bars, arrows and Reset Zoom buttons on both axes (`StyleAxisScrollBar`, `_DataLogScrollBar_...` macros); table below the exported graph (`GetCameraConfigLines`, `GetExportInfoLines`, `SplitInfoLine`), Add Notes button and Graph Notes window (`ShowGraphNotesWindow`, `GraphNotes`), Show Graph Title check box (`ShowTitleCheckBox`, `UpdateGraphTitleVisibility`), text size and high resolution of the exported image (`_DataLogExport_...` macros, direct title editing `LogChart_MouseDoubleClick` / `TitleEditBox`); zoom reset when a new session starts (`ResetView`); frame around the plot area (secondary axes with only the line shown); first / last axis values in the strip of an exported image (`GetAxisRangeTexts`, `FormatAxisTime`); Clear Graph button; orange Reset Zoom border while zoomed (`UpdateZoomIndicator`); Yes / No questions for Stop Logging and Clear Graph; follows the session file to the saved location; mouse cursor temperature label in the graph (right-click menu); note about many samples in the opening notice; zoom rectangle on both axes; right-click menu of the graph (cursor temperature, number of ticks, Y-axis range set / lock at current / unlock); check box per graph line; Export Graph button (graph saved as image); X-axis range items (set / lock at current / unlock); graph title; dark green / dark red Start / Stop Logging button borders; scroll steps of the zoomed graph (`UpdateScrollSteps`); round X-axis tick steps (`GetRoundTickSeconds`, `GetDataEndTime`); fixed plot area size (`UpdatePlotAreaLayout`, `LogChart_SizeChanged`) |
| `RMH_Application_ThermalViewer.cpp` (also, earlier branch) | Fix: Ambient / Reflected temperature boxes of the configuration panel were swapped when the temperature unit changed (see 1.26) |
| `RMH_Application_ThermalViewer.cpp` / `.h` (also, branches 23 - 25) | Live data logging settings (1.33): `RMH_ThermalViewer_ReadSessionDurationMilliSec`, `RMH_ThermalViewer_UpdateDataLoggingSessionDuration` (warning window, duration set back), `RMH_ThermalViewer_UpdateDataLoggingSampleInterval`, `RMH_ThermalViewer_ReadSessionIntervalMilliSec`, `RMH_ThermalViewer_WarnLiveSampleLimit`, `RMH_ThermalViewer_ApplyLiveDataLoggingDataSetChange` (new column / pause / source), column mapping in `RMH_ThermalViewer_StartDataLogging`, one read of the number of columns and the note lines in `RMH_ThermalViewer_DataLoggingThreadProcess`; style flags in `RMH_ThermalViewer_Change2DPlotDataSetAndSettingsPanelColor` / `RMH_ThermalViewer_Change2DPlotDataSetLineWidth` |
| `ThermalCameraGUI.h` | Live data logging settings (1.33): `ValueChanged` events of the three duration boxes (`DataLoggingDurationUpDown_ValueChanged`) and of the interval box (`DataLoggingIntervalUpDown_ValueChanged`), calls in the data set check box / source ComboBox callbacks, new array `Plot2DDataSetLineWidthUpDowns` |
| `DataLogGUI.h` (also, branches 23 - 25) | Columns added while logging (`AddLogColumn`, `ApplySourceChangeNote`, `ProcessNewText`); line color / width of the plot data set (`GetColumnColor`, `GetColumnLineWidth`, `ApplyLiveStyleChanges`); notice text about the duration and interval (1.33) |
| `CrashRecoveryDialog.h` (new, branch 26) | Crash recovery (1.34): `RecoveredSessionInfo` (reads a leftover session file: delimiter, samples, logged time) and `CrashRecoveryDialog` (Save / Open / Delete / Decide Later window, built in code) |
| `RMH_Application_ThermalViewer.cpp` / `.h` (also, branch 26) | Crash recovery (1.34): `RMH_ThermalViewer_CheckForUnsavedDataLoggingSessions`, temporary folder helper, session lock file (`..._AcquireDataLoggingSessionLock`, `..._ReleaseDataLoggingSessionLock`, `..._SessionFileIsInUse`); `RMH_ThermalViewer_FinishDataLoggingSession` now asks (`..._AskToSaveDataLoggingSession`) and then releases the lock |
| `MainGUI.h` (also, branch 26) | Crash recovery (1.34): the check and opening of a recovered file in `MainGUI_Shown`, before the automatic start-up |
| `DataLogGUI.h` (also, branch 26) | Crash recovery (1.34): `OpenRecoveredSession`, `GetViewDelimiter` (delimiter of the viewed file), no duration notice for a recovered file |
| `GlobalObjectsAndVariables.h` (also, branch 26) | `GlobalVariables::DataLoggingSessionLockStream` |
| `GlobalObjectsAndVariables.h/.cpp` | Live logging state (`DataLoggingColumnDataSetIndex`, `DataLoggingColumnEnabled`, `DataLoggingColumnSourceChangedFlag`, `DataLoggingStyleChangedFlag`, `GlobalVariables::DataLoggingColumnSourceNames`, `GlobalVariables::Plot2DDataSetLineWidthUpDowns`); flags for the Data Log window; `DataLoggingSessionFolderString` (folder of the current session file); calibration freeze (`CalibrationActiveFlag`, `CalibrationSettleUntilTick`, `RMH_IsCalibrationFreezeActive`) |
| `RMH_SupportedIRCameras_Resources.h` | New setting `_CalibrationFreezeSettleTimeMs` |
| `RMH_OpenGL_2DPlot.h` | OpenGL window created at the panel size and resized with it; right-click menu on the plot; locked Y-axis range |
| `TempMeasGUI.h` | New Y-axis range items (set, lock, unlock) in the plot's right-click menu; data logging thread `RunWorkerCompleted` event (asks save / discard) |
| `DataLogGUI.h` (also, branch 27) | Log file read in blocks and min / max decimation (1.35, 1.36): new class `LogSeriesBuffer` (`LimitPoints`), `EnsureSeriesBuffers`, `SyncChartSeries`, `ShowPendingData`, rewritten `RefreshLogView` and `ProcessNewText`, loading percentage in `UpdateStatusText`, reset in `ResetView` / `ClearChartData` / `ClearViewButton_Click`, notice text |
| `RMH_Application_ThermalViewer.h` (also, branch 27) | New settings `_DataLogView_ThinOldPoints`, `_DataLogView_ReadBlockBytes`, `_DataLogView_ReadTimeBudgetMs` |
| `RMH_Application_ThermalViewer.cpp` (also, branch 27) | Texts of the warning window for sessions with many samples and of the status line after a live change of the duration / interval follow `_DataLogView_ThinOldPoints` |
| `DataLogGUI.h` (also, branch 28) | Two-cursor measurement (1.37): new class `MeasureRow`, `BuildMeasureObjects`, `LogChart_MouseDown` / `LogChart_MouseUp` (and the drag part of `LogChart_MouseMove`), `GetLineValueAt`, `GetMeasureTimeAtMouse`, `CollectMeasureRows`, `BuildMeasureTexts`, `DrawMeasurePanelOnExport`, `UpdateMeasurement`, `ApplyMeasureMode`, the two new items of the right-click menu, hooks in `LineCheck_CheckedChanged`, `ShowPendingData`, `ClearChartData`, `ResetView`, and in `ExportGraphButton_Click` (the panel is drawn on the exported graph) |
| `DataLogGUI.h` (also, branch 29) | Typed cursor times (1.38): `ExportGraphButton_Click` (suggested file name with the export time), `SetCursorTimesMenuItem_Click`, `TryParseElapsedTime`, `RemoveMeasureCursor`; `IsPointInPlotArea` now tests by position (ends of the shown axis ranges) and `UpdateCursorLabel` uses it (label also on a cursor line); new item `Set Cursor Times...` in the right-click menu (`SetCursorTimesMenuItem`) |
| `CHANGELOG.md`, this README | Notes (the README file is renamed for each branch) |

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

**Data Log window** (top of `src/RMH_Application_ThermalViewer.h`, near `_DataLogView_MaxRows`):

| Setting | Default | Meaning |
|---|---|---|
| `_DataLogView_MaxRows` | 20000 | Rows kept in the table (the newest) |
| `_DataLogView_MaxPlotPoints` | 100000 | Points kept per graph line |
| `_DataLogView_ThinOldPoints` | 1 | 1 = the oldest points are thinned out (min / max, 1.36), the whole session stays on the graph; 0 = the oldest points are removed (old behavior) |
| `_DataLogView_ReadBlockBytes` | 524288 | Size of one block when the log file is read (1.35) |
| `_DataLogView_ReadTimeBudgetMs` | 200 | Most time (ms) used per refresh for reading the file; the rest is read 50 ms later |

**Calibration freeze** (top of `src/RMH_SupportedIRCameras_Resources.h`):

| Setting | Default | Meaning |
|---|---|---|
| `_CalibrationFreezeSettleTimeMs` | 300 | Time in ms that the plot and data logging values stay frozen after a shutter calibration has finished. Raise it if a small bump is still visible right after the shutter reopens. |

---

## 5. Building it

The GitHub Actions build only starts automatically for pushes to `main`. For this branch:
**Actions -> build -> Run workflow -> choose `IRCAM-for-T2Splus_V2_2Dplots_expansion_29` -> Run workflow**, then download the build from the run
page. If a build fails, search the log for `error C` (ignore the `warning` lines) to find the real error.

Fallbacks: Build #7 (commit `7b355e8`, branch `fix-t2s-v2-temperature`) and the branches `t2s-v2-calibration-diagnostic`,
`IRCAM-for-T2S-V2`, `IRCAM-for-T2Splus_V2_2Dplots_fix` and `IRCAM-for-T2Splus_V2_2Dplots_expansion_01`, `_02`, `_03`, `_04`, `_05`, `_06`, `_07`, `_08`, `_10`, `_12`, `_13`, `_15`, `_16`, `_17`, `_18`, `_19`, `_20`, `_21`, `_22` and `_23`.

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
| Data Log graph: no horizontal jump while logging (fixed plot area, 1.32) | Seems to work (reported after the change); not tested on every window size or with the graph title shown / zoomed |
| Stop Logging / Clear Graph confirmation windows (Yes / No) | Not tested yet |
| Temporary data log file: save (Yes) and discard (No) when a session ends | Not tested yet |
| Log file name format `DataLogSession_12Nov2026_09-57-39.txt` | Not tested yet |
| Data Log graph: right-click menu item Temperature At The Cursor shows / hides the temperature at the mouse cursor; left click does nothing | Not tested yet |
| Data Log graph: X / Y number of ticks (Auto, fixed numbers), also while zoomed | Not tested yet |
| Data Log graph: Set Y-Axis Range And Lock, Lock Y-Axis At Current Range, Unlock Y-Axis | Not tested yet |
| Data Log graph: check box per line shows / hides the line; all ticked at the start | Not tested yet |
| Data Log graph: Set X-Axis Range And Lock (valid and invalid times), Lock X-Axis At Current Range, Unlock X-Axis; zoom and ticks inside a locked range | Not tested yet |
| Data Log graph: tick Show Graph Title: bold `Title` at the top center; double-click it, type, Enter keeps it, Esc cancels, empty text hides it; the title is in the exported image | Not tested yet |
| Data Log graph: Export Graph first shows the title window with the current title (also when the title is hidden); Set Title continues, Cancel cancels the export | Not tested yet |
| Data Log graph: Export Graph saves PNG / JPEG / BMP with the shown lines named above the graph | Not tested yet |
| Data Log graph: left-drag rectangle zooms both X and Y; Reset Zoom works | Not tested yet |
| Warning window for more than 100000 samples: Start starts, Cancel does not start; no window for alarm / trigger starts | Not tested yet |
| Calibration freeze: no spikes / dips in the plot and in the data logging file during periodic (auto) calibration, Calibrate Camera button and drift-based calibration | Not tested yet |
| Calibration freeze: no bump right after the shutter reopens with the default 300 ms settle time | Not tested yet |
| Calibration freeze: build compiles (MSVC, /clr) | Not tested yet |
| Data Log graph: frame on all four sides of the plot area (also while zoomed and in the exported image); layout of the graph unchanged | Not tested yet |
| Export Graph: line with the first and last value of the time and temperature axis in the strip (automatic range, zoomed, locked X / Y range, number of ticks) | Not tested yet |
| Export Graph: the axis line is split into two rows for a narrow graph; milliseconds shown only for a zoomed range below one minute (not for a short unzoomed log) | Not tested yet |
| Frame and Export Graph change: build compiles (MSVC, /clr) | Not tested yet |
| Data Log graph: Clear Graph button (new name, new question text) clears table and graph after Yes | Not tested yet |
| Data Log graph: Reset Zoom border turns orange when zoomed (drag rectangle, one axis or both) and returns to gray after Reset Zoom / locking or unlocking a range | Not tested yet |
| Data Log graph: scroll bars, arrows and small Reset Zoom buttons are light gray and clearly visible on both axes when zoomed; arrows are visible on the buttons | Not tested yet |
| Export Graph: list "Thermal Camera Configuration" below the graph with all 6 settings; values follow the panel (after Set) and the temperature unit (°C, °F, K) | Not tested yet |
| Export Graph: image with the list saves correctly as PNG, JPEG and BMP; graph and strip above unchanged | Not tested yet |
| Export Graph: Export Information list (date, time, camera model, temperature range, auto / drift calibration, log file, interval, duration, samples, logged time, unit) on the right of the configuration list; below it for a narrow image | Not tested yet |
| Zoomed graph while logging: the view stays on the zoomed part (axis labels do not change); the thumb of the scroll bar moves left and gets smaller as the log grows (the bar shows the position in the whole log) | Not tested yet |
| Start a new logging session while the graph is zoomed: both axes reset, Reset Zoom border back to gray; a locked range stays | Not tested yet |
| Change the temperature unit with different Ambient and Reflected values: each box keeps its own value (then Set, then Read) | Not tested yet |
| Add Notes button sits between Reset Zoom and Export Graph; Auto-scroll check box and Export Graph are fully visible (window opened separately and docked) | Not tested yet |
| Add Notes: window shows the current notes; Set Notes keeps them, Cancel does not change them, empty text removes them; Enter starts a new line | Not tested yet |
| Export Graph: Graph Title window, then Graph Notes window (Cancel in either cancels the export), then the save window | Not tested yet |
| Exported image: table with 3 cells (no notes) or 4 cells (with notes) in the order Configuration, Export Information, Other Information, Notes; borders line up | Not tested yet |
| Exported image: long notes (several lines, 2000 characters) wrap inside the Notes cell; a narrow graph moves cells to a next row | Not tested yet |
| Exported image: line names, axis ranges, table, graph title and axis titles are about 20 % bigger than the axis values (20.0, 0:00:00); the axis values and the graph on the screen are unchanged after the export | Not tested yet |
| Show Graph Title check box (before Auto-scroll): not ticked at the start; ticked shows `Title`; unticked hides it, text kept; the right-click menu has no Set Graph Title...; Export Graph asks for the title when it is hidden (new title shows it, empty hides it, same text keeps it) | Not tested yet |
| Exported image resolution: 3 x the pixels; lines and text sharp when zoomed in a viewer (PNG, JPEG, BMP); compare `_DataLogExport_UseVectorChart` 1 and 0; the graph looks the same as on the screen (colors, lines, frame, zoom, locked range) | Not tested yet |
| Export Graph on a big window / long log: no out-of-memory or very slow export at 3 x resolution | Not tested yet |
| X-Axis Number Of Ticks 5 / 10 / 20 / 25 / 50 while logging (not zoomed): the graph does not change its size and the last time value stays; ticks at round times (e.g. every 10 minutes); the step only changes when the log grows into the next step; also zoomed, with a locked X range, and back to Auto | Not tested yet |
| Duration changed while logging: 1 min -> 1 h after 20 s (logging continues to 1 h after the start) | Reported as working (branch `_23`) |
| Sample interval changed while logging: 1 s -> 5 s and 5 s -> 0.5 s (new interval used from the next sample; `Time(ms)` in the file matches the real time, compare with a stopwatch); a shorter interval that is already over gives the sample at once | Not tested yet |
| Duration shorter than the logged time (e.g. 10 s after 30 s): warning window, duration boxes set back, session keeps running; change in a good order (minutes first, then hours); change while not logging (used at the next start) | Not tested yet |
| Plot data set enabled during a session (not in the session at the start): new column in the file from that moment, new line and table column in the Data Log window, `#` note line in the file; several data sets added one after the other; up to 10 | Not tested yet |
| Check box unchecked / checked again during a session (NaN gap, then values again); source ComboBox changed during a session (new source from the next sample, name changed in the Data Log window) | Not tested yet |
| Line color / line width changed during a session: the line in the Data Log graph follows within about 1 s; default colors look the same as before | Not tested yet |
| Live logging settings (1.33): build compiles (MSVC, /clr) | Not tested yet |
| Scroll bar colors, table, notes, text size, zoom reset and unit fix: build compiles (MSVC, /clr) | Not tested yet |
| Crash recovery (1.34): start a session, kill IRCAM in Task Manager, start again: the window lists the file with the right samples / logged time; Save, Open, Delete and Decide Later each work; window closes when the list is empty | Not tested yet |
| Crash recovery: a file written with a semicolon / tab delimiter opens correctly with the comma setting, and the next session still uses the setting | Not tested yet |
| Crash recovery: two instances at the same time - the second does not offer the running session of the first; after the first is killed, a third start offers it | Not tested yet |
| Crash recovery: header-only file is deleted silently; Save with an existing name gives `_recovered`; several files, newest first; Open of a file keeps it for the next start-up | Not tested yet |
| Crash recovery: build compiles (MSVC, /clr) | Not tested yet |
| Reading in blocks (1.35): open a long log file (about 1 million lines / 50 MB or more, e.g. a recovered session): the program stays usable, the status line shows Reading the log file: NN %, then the table and the graph appear once; memory stays low | Not tested yet |
| Reading in blocks: a log file of more than 2 GB opens (no empty window, no error) | Not tested yet |
| Reading in blocks: a running session looks the same as before (new rows every second, auto-scroll, zoomed view stays) | Not tested yet |
| Reading in blocks: a file name / header with non-ASCII characters (UTF-8) is shown correctly; Clear Graph during a running session, and the start of a new session, work | Not tested yet |
| Min / max decimation (1.36): a session with more than 100000 samples per line (e.g. 0.04 s interval, more than 66 minutes, or a long recovered log): the whole session stays on the graph, a short spike at the start is still visible, the highest and lowest value of the graph match the CSV file | Not tested yet |
| Decimation: the left end of the graph is at 0:00:00; a gap (NaN, unchecked data set) is still a gap in the old part; the newest part keeps full detail | Not tested yet |
| Decimation: the pause when the lines are filled again is short; zoom and a locked X / Y range stay while it happens; Export Graph of a thinned graph works | Not tested yet |
| `_DataLogView_ThinOldPoints` 0: the oldest points scroll off the left side as before | Not tested yet |
| Block reading and decimation: build compiles (MSVC, /clr) | Not tested yet |

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
- The calibration freeze (1.19) holds a repeated last value; it is not an interpolation and the CSV has no column that marks the frozen samples.
- The calibration code (UI thread) and the video thread still use the same frame buffers and the same look-up table without a lock. The freeze hides
  the values, but a single outlier frame right after a calibration is still possible in theory. The proper fix would be a lock around the calibration
  and the frame processing.
- The shutter calibration still blocks the user interface (`Sleep`) for about 0.5 - 1.4 s, depending on the camera type.
- The exported image shows the last X / Y value as text in the strip above the graph (1.21), not as an extra label on the axes. The values are the ends of
  the axes, not the highest sample of a line.
- The plot area of the Data Log graph has fixed margins (1.32). A very long axis label (for example a temperature with many digits, or a big font scale in Windows) can be cut off; increase the margin constants in `UpdatePlotAreaLayout`. The graph window must be at least about 215 px wide and 130 px high for the fixed layout; smaller than that it uses the automatic layout.
- Live data logging settings (1.33): the header line of the file keeps the names from the start (the `#` note lines show added / renamed columns), the rows before a new column are shorter, and `NaN` is written with the culture of the PC. The warning for sessions with many samples is a status line only when the duration or interval is changed while logging (the window with the question is only shown at the start). A duration change is checked box by box, so a big step down may have to be done in a certain order. With a changing interval the time between two samples is the real time waited, but it does not include the (very short) time used to read and write the sample.
- Crash recovery (1.34): **Open** only shows the file; it stays in the temporary folder until saved or deleted. A power loss can cut the last line of the file (it is kept as it is). The lock file only protects against another instance of IRCAM, not against other programs.
- Min / max decimation (1.36): the points that were thinned out cannot be brought back in the window (zoom shows only the kept points); the CSV file has every sample. The lines are filled again after each thinning, which can pause the window briefly with 100000 points on several lines. The table still shows only the newest rows.
- Reading in blocks (1.35): while a very long file is read the table and graph stay as they were (or empty) until the window has caught up; a file that grows faster than it can be read keeps the status at Reading the log file. The block reading and the decimation were checked outside the program only (algorithm copy); the C++/CLI code was not compiled.
- Two-cursor measurement (1.37): the temperatures are taken from the data (interpolated), not from the height of the click; the cursors are placed with the mouse only (no typed time, no keyboard step); in the exported image the panel text can differ from the window by about a pixel (GDI+ text instead of the window's text drawing) and the panel is always at its window position; a cursor cannot be placed outside the logged time range; zoom with the rectangle is off while the mode is on; after a thinning (1.36) the value of an old part is interpolated between the points that were kept. The program was **not compiled and not tested**.
- Typed cursor times (1.38): the time is the elapsed time since the start of the log (no clock time, no negative time); only whole `h` and `m` parts before the seconds; a time outside the logged time range is refused, not clamped (apart from 1 ms); no step or keyboard input.
- The frame around the Data Log graph (1.20) is always on; to remove it, delete the `AxisX2` / `AxisY2` lines in `BuildChart` (`DataLogGUI.h`).

---

## 8. Sources used

The InfiRay Android APK (XthermNew 6.6.230906), the analysis of the InfiRay Windows viewer, IR-Py-Thermal, and
`dzaczek/t2s-thermal-macos` (which note that the V2 shutter sensor drifts).
