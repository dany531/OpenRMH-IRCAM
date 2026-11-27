# IRCAM for T2S+ V2 (branch `IRCAM-for-T2Splus_V2_2Dplots_expansion_41`)

A modified version of **OpenRMH-IRCAM** (based on v3.0.0-community.5) for the **InfiRay T2S+ V2 / T2S+_A2** thermal camera
(USB ID `VID_3474&PID_6002`). Personal reference notes, last updated 27 November 2026. Not an official release.

- Fork: https://github.com/dany531/OpenRMH-IRCAM
- Original project: https://github.com/Cyclotronic/OpenRMH-IRCAM
- This branch contains **all** changes made so far: correct temperatures, automatic start-up, the 2D plot fixes and extras, the Data Log window, the confirmation / save questions of data logging, readable log file names, the mouse cursor temperature in the Data Log graph, a warning for sessions with many samples, zooming in both axes, the right-click menu of the Data Log graph a check box per graph line an Export Graph Image button and X-axis range lock in the Data Log graph menu a graph title that is asked for again when exporting, colored Start / Stop Logging button borders and a corrected Data Log menu item, and a working scroll bar for the zoomed graph, a question when the application is closed while data logging is running, frozen plot / data logging values during a camera shutter calibration (no more spikes or dips), a frame around the Data Log graph, the first and last value of both axes written in an exported graph image, a Clear Graph button, an orange Reset Zoom border while the graph is zoomed, lighter gray scroll bars / arrows / Reset Zoom buttons on the zoomed graph, and the thermal camera configuration plus export information (date, time, camera model, calibration, session), and crash recovery of unsaved data logging sessions at start-up (branch 26) listed in an exported graph image, a zoom reset at the start of a new logging session, a fix for swapped Ambient / Reflected temperature when the temperature unit changes, an **Add Notes** button and a table with notes below the exported graph, 20 % bigger text and a higher resolution of the exported image, a **Show Graph Title** check box, round X-axis tick steps, a fixed plot area size so the Data Log graph no longer jumps sideways while logging, and **live data logging settings**: the session duration, the sample interval and the Temperature Plot Data Set Settings (enable check box, source, line color, line width) can be changed while a session runs, and a data set that is enabled during a session is added to the log and to the Data Log graph from that moment on, and (branch 27) **the log file is read in blocks** (a very long log file opens without a memory spike and without the 2 GB limit) and **the oldest part of a long Data Log graph is thinned out with min / max decimation** instead of scrolling off the left side, and (branch 28) **two-cursor measurement in the Data Log graph** (cursors A and B: time difference, temperature difference and slope in °C/min per line, also in the exported image), and (branch 29) **typed cursor times** (enter A and B as exact elapsed times), and (branch 31) **markers in the Data Log graph** (a vertical line with the time and a text, for example when the power was changed or a door was opened; they can be dragged and deleted) and **zoom buttons on the scroll bars** of the zoomed Data Log graph (+ and - for the time axis and for the temperature axis: zoom only one axis, also after a zoom rectangle), and (branch 32) **a larger Data Log graph at the start** (the table below the graph shows the last 3 values instead of about 8; the divider can still be dragged) and **a separator line in the right-click menu** of the Data Log graph (between Temperature At The Cursor and Measure With Two Cursors), and (branch 33) **session files**: a **Session menu** at the top of the window to save the settings of the whole program to a file (Save Session / Save Session As) and to open them again later (Open Session); in branch 34 a session file also holds **the live view measurements** (crosshairs, lines, ROIs), **the logged data with the markers and cursors of the Data Log window** and **the positions of windows that were pulled out**, and (branch 41, new) **the log file inside a session file now also holds the markers, cursors, title and notes of the graph**, opening a session never replaces a different log file, and **Save Log File** has two file types (`.csv`, `.txt`) and a check box **Include markers, cursors, title and notes** (1.50); (branch 40) **Start Logging** asks first when the graph already shows a log (1.49); (branch 39) after a logging session **you choose the file name and the folder** of the saved file, and **Save Log File** can replace the file that the graph shows (for example to add notes later) (1.48); (branch 38) a data logging session that is saved after it was stopped also holds **the markers, cursors, zoom, title, notes and hidden lines of the Data Log graph** in its `.csv` / `.txt` file, the same as **Save Log File** writes, and the **Export Graph** button is now called **Export Graph Image** (1.47); and (branch 37) a log file also holds **the graph title, the notes and the hidden lines**, a session holds **the hidden lines**, and the **Add Marker** and **Clear Graph** buttons are gone (1.46); and (branch 36) the Data Log window has **Open Log File** and **Save Log File** buttons (1.45); and (branch 35) a session file also holds **the Data Set Source of all 10 plot data sets**, **the Maximum / Minimum / Center Temperature (and mouse cursor) tracking buttons** and **the zoom of the Data Log graph**.

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
- **Show Log File** opens the log file in the Windows explorer. **Auto-scroll** keeps the newest row visible.
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
- (The **Clear Graph** button was removed in branch 37, see 1.46.)
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

- **Temperature At The Cursor** - tick to show the temperature and time at the mouse cursor (see 1.8). Since branch 32 a **separator line** (white on the dark menu) divides it from the items below (Measure With Two Cursors, see 1.41).
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

Above the graph (below the Start Logging / Stop Logging / Show Log File / Open Log File / Save Log File / Reset Zoom / Add Notes / Export Graph Image buttons) there is one **check box per
graph line**, with the line's name in the line's color. Un-tick a box to hide that line, tick it to show it again, for example to look at
2 of 10 logged data sets. All lines are ticked when a log is opened or a new session starts. The check boxes replace the legend of the graph.
A hidden line is only hidden from the graph: its data keeps being collected and stays in the table and in the log file. The automatic
Y range and the Y tick / range settings only use the lines that are shown.

### 1.13 Export Graph Image: the graph as an image (new; the button was called Export Graph before branch 38, see 1.47)

The **Export Graph Image** button, next to Start Logging, Stop Logging, Show Log File, Open Log File, Save Log File, Reset Zoom and Add Notes, saves the Data Log graph as an image.
First the **Graph Title** window opens (also when the title is hidden with **Show Graph Title**, see 1.29) (see 1.15). It shows the title that is set now, so you can keep it, edit it or replace it (an empty text removes the title); **Set Title** continues, **Cancel** cancels the export. Then the **Graph Notes** window opens (the same window as the **Add Notes** button, see 1.27) with the same behavior: it shows the notes that are set now, **Set Notes** continues, **Cancel** cancels the export. Then a save window opens (file types **PNG**, **JPEG** and **BMP**; the format follows the extension you type, PNG by default) with the session file
name plus `_graph` as the suggested name (or `DataLogGraph_` and the date and time when there is no session file name).

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
you fix the exact window of the graph before using **Export Graph Image** (1.13).

### 1.15 Graph title (new)

Tick **Show Graph Title** (above the graph, 1.29): the title appears in bold at the **top center above the graph** and is part of the image saved with **Export Graph Image** (1.13), so
you can see later what the graph was about. Until you type one, the title is the text `Title`.

**Edit the title directly on the graph:** double-click the title. A text box opens on it: type the title (up to 200 characters) and press **Enter** (or click somewhere else) to keep it,
**Esc** to throw the change away. A new title is shown (it ticks **Show Graph Title** if needed). An **empty** text hides the title (the check box is unticked and the text goes back to `Title`).

The same window as before (**Graph Title**, with the current title filled in) opens when you press **Export Graph Image** (1.13), **also when the title is hidden**: type a new title to show it, empty the text to
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
color of the axis lines (gray), does not change the layout of the graph and is part of an image saved with **Export Graph Image** (1.13).

### 1.21 Last X and Y value in the exported graph (new)

The labels on the axes only show the tick marks, so when the end of an axis does not fall on a tick (zoomed view, locked range, number of ticks, automatic
range) the maximum duration and the maximum temperature could not be read in an image saved with **Export Graph Image**. Now the strip above the graph, below
the names of the lines, always has a line like:

`Time axis: 0:00:00 to 1:02:37 (h:mm:ss)      Temperature axis: 20.5 to 134.2 °C`

- The values are the **first and last value of each axis as shown at the moment of the export**: zoom, locked X / Y range and automatic range are followed.
- The line is split into two rows (time, temperature) when the graph is too narrow for one row.
- Only a **zoomed** time range shorter than one minute is written with milliseconds (`h:mm:ss.fff`, e.g. `0:12:03.400 to 0:12:41.900`). In every other case (not zoomed, a locked range, a range of 60 s or more) the time is `h:mm:ss`, cut to whole seconds like the axis labels. The temperature has one to three decimals.
- These are the ends of the axes, not the highest sample of a line. With an automatic range the end of the axis can be a little above the highest sample.
- The graph itself and the size of the graph part of the image are not changed; the strip is just one row (or two) taller.

### 1.22 Clear Graph button and zoom indicator (new; the Clear Graph button was removed in branch 37, see 1.46)

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

**Export Graph Image** (1.13) adds a **table below the graph** for later reference. From left to right it has these cells (heading on a darker band, then the lines
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

The **Add Notes** button, between **Reset Zoom** and **Export Graph Image**, opens the **Graph Notes** window. Type notes about the logging (up to 2000 characters, **Enter**
starts a new line) and press **Set Notes**; **Cancel** keeps the notes as they were. An empty text removes the notes.

- The notes are written in the **Notes** cell of the table below the graph in an exported image, on the right of **Other Information** (1.24). They wrap to the
  width of the cell. Without notes there is no Notes cell.
- It works like the **Graph Title** (1.15): when you press **Export Graph Image**, the **Graph Notes** window opens after the **Graph Title** window. It shows the notes that
  are set now, so you can keep, edit or empty them; **Set Notes** continues with the save window, **Cancel** cancels the export.
- Like the title, the notes **stay** when you use Clear Graph or start a new session, until you change them or close the program (they are not saved with the log
  file and not saved between program runs). Check or empty them before the next export.
- The buttons moved to make room: **Export Graph Image** is now a little narrower and **Auto-scroll table to newest** is further right; the Data Log window is wider when it is opened as a separate window (1130 pixels, see also 1.29). When it is docked in the main window its width follows the main window.

### 1.28 Text size in the exported image (new)

In the exported image all text **apart from the values on the axes** (for example `20.0` and `0:00:00`) is **20 % bigger** than those values: the names of the lines, the first / last
axis values, the table, the title of the graph and the titles of the axes (**Time (h:mm:ss)** and **Temperature**). The size is read from the graph itself, so it follows the axes. The values on
the axes keep their size. The rows of the table and the colored squares in front of the line names follow the text size. The factor is fixed (1.2, the constant `TextScale` in `ExportGraphButton_Click`). The title and the axis titles on the screen keep their
size: they are only made bigger while the image is made.

### 1.29 Show Graph Title check box (new)

A new check box **Show Graph Title** sits before **Auto-scroll table to newest**, at the top right of the Data Log window. It is **not ticked** by default.

- **Ticked:** the title is shown above the graph and in an exported image. When no title was typed yet, it shows `Title`. Double-click the title to edit it (1.15).
- **Not ticked:** the title is hidden on the graph and in the exported image, but the **text is kept**.
- **Export Graph Image** opens the **Graph Title** window **in both cases** (1.13, 1.15). Typing a new title there shows it (ticks the check box), an empty text hides it (unticks it), and the same text keeps it as it is.
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
- **While the graph is zoomed, the automatic layout is used**, because the zoom scroll bars need room. The plot area does not change by itself during a zoom. (**Changed in branch 31, see 1.40:** the fixed layout is now used while zoomed too, with larger margins, because the automatic layout changed the plot area at every zoom step.)
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

### 1.38 Typed cursor times (new, branch 29)

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

### 1.39 Markers in the Data Log graph (new, branch 31)

**What it is for.** To write down in the graph *when* something happened in a test (power changed, door opened, sample moved), so that a jump in a line can be explained later and the note is in the exported image.

**How to use it.**

- **Add Marker** button (top bar of the Data Log window; **removed in branch 37**, use the right-click menu below, see 1.46): adds a marker at the **current time of the data logging** (the time of the newest logged sample) and opens a small window for its text. The data logging must be running; if it is not, a message says so. The marker is placed **when you press the button**, not when the text window is closed, so typing the text does not move it. **Cancel** removes the marker again.
- **Right-click the graph -> Add Marker Here...**: adds a marker at the time where you right-clicked (inside the plot area, over logged data). This works for a log that has stopped, and for a time in the past.
- The text can be empty (the marker then shows only its time) and has at most 60 characters (`_DataLogMarker_MaxTextLength`). There are no ready-made texts.
- **Drag a marker with the mouse** (its line or its text box, hold the left button), like the cursors of the two-cursor measurement (1.37). The mouse cursor changes to a horizontal arrow while it is on a marker, and the zoom rectangle is switched off there, so a drag moves the marker and does not zoom. A marker stays inside the logged time range. It works with and without **Measure With Two Cursors**.
- **Right-click on a marker -> Delete Marker: <time> <text>** deletes that marker (the item is only shown when the mouse is on a marker). **Delete All Markers** deletes all of them (asks first).

**What it looks like.** A vertical orange line over the whole plot area, with a dark box at the top: the **time** (`h:mm:ss`, like the time axis) on the first line and the **text below the time**. The box is right of the line, or left of it near the right edge. Boxes that would cover each other are put in rows below each other. The markers are drawn on the chart itself, so they follow zoom, scroll and resizing, and they are **part of an exported image** (Export Graph Image).

**What happens to them.** Markers belong to the data of the graph: the start of a new logging session and opening another log file remove them (**Clear Graph** was removed in branch 37). Branch 36 saves them in the log file with Save Log File (1.45) and branch 35 in the IRCAM session (1.43).

How it works (for later changes): the class `GraphMarker` (time in elapsed milliseconds, text, and the pixel position of the last drawing), the list `GraphMarkers`, `LogChart_PostPaint` / `DrawMarkers` (drawing after the plot area), `FindMarkerAt` (picking), `UpdateMarkerHover` (arrow cursor and zoom rectangle off), the drag part of `LogChart_MouseDown` / `LogChart_MouseMove` / `LogChart_MouseUp`, `AddMarkerAt`, `ShowMarkerTextWindow`, `UpdateMarkerMenuItems` (the items of the right-click menu depend on the mouse position), `ClearGraphMarkers` (called from `ClearChartData` and `ResetView`).

---

### 1.40 Zoom buttons on the scroll bars of the Data Log graph (new, branch 31)

**What it is for.** The zoom rectangle (1.10) always zooms **both** axes at once, and once the graph is zoomed it is not possible to zoom only the temperature axis or only the time axis further. Small **+** and **-** buttons now zoom **one axis at a time**.

**Where they are.** They **only appear while the graph is zoomed** (after a zoom rectangle), and only for the axis that is zoomed. They sit **at the end of the scroll bar** of that axis, at the end of the scroll bar, beyond the arrow (the chart draws the scroll bars itself, so they are small separate buttons laid over the graph, 13 x 13 pixels, in the colors of the scroll bar buttons; the **+** and **-** are drawn by the program, not as button text). They are beyond the end of the bar, so the scroll bar thumb never goes under them:

- **Time axis (X)**: the scroll bar below the graph. **-** and **+** are just right of the **right arrow**.
- **Temperature axis (Y)**: the scroll bar left of the graph. **+** and **-** are next to each other just **above the top end of the bar** (above its Reset Zoom button), in the top margin of the graph, so they never come near the labels of the time axis.

The chart always draws a scroll bar over the whole length of its axis, so it cannot be made shorter to hold the buttons: they are in the margins around the plot area, at the ends of the bars (the margins are kept free by the fixed layout below).

When the graph shows its whole range (not zoomed, or after **Reset Zoom**, or when **-** has reached the whole range) the buttons are gone and the graph looks and is sized exactly as before. Hover over a button for its tooltip (it says which axis).

**How to use it.**

- **+** zooms in on that axis only: the visible range becomes **70 %** of what is shown now (`_DataLogZoomButton_Factor`), around the middle of the shown range. **-** zooms out on that axis only: the visible range becomes about 143 % (1 / 0.7). Click again for another step.
- It works after a zoom rectangle and in any order, so the zoom can be tuned one axis at a time. The shown range stays inside the whole range of the axis. When **-** reaches the whole range, that axis is no longer zoomed (its scroll bar and its buttons disappear). If only one axis is zoomed, only its buttons are shown; to zoom the other axis too, drag a zoom rectangle again.
- The smallest range that **+** goes to is 10 ms on the time axis and 0.01 on the temperature axis (`_DataLogZoomButton_MinTimeMilliSec`, `_DataLogZoomButton_MinTemperatureRange`).
- The scroll bars, **Reset Zoom** (both axes back to the full view) and the orange Reset Zoom border work as before; the tick steps, scroll steps and the plot area are updated after every click, as after a zoom rectangle. Markers and cursors stay at their time. The buttons are not part of an exported image.

**The graph no longer changes size when you zoom with the buttons.** Before, the chart used its automatic layout while the graph was zoomed, and that layout changes the plot area whenever a label gets another digit (the temperature labels do when you zoom in) or a scroll bar appears or disappears, so the graph jumped at some clicks. Now the fixed plot area of 1.32 is also used while zoomed, with larger margins for the scroll bars and the buttons: **left 110, right 40, top 28, bottom 72** pixels instead of 75, 40, 12, 55. With `_DataLogPlotArea_ReserveZoomRoom` = 1 (the default) these margins are used all the time, so the plot area **never changes size** (it is about 35 pixels narrower and 33 pixels lower than in branch 29, also when the graph is not zoomed). With 0 the smaller margins are used while the graph is not zoomed, and the plot area changes size once when zooming starts and once when the zoom ends. The exported image uses the same plot area.

How it works (for later changes): `BuildZoomButtons` (the four buttons, children of the chart), `StyleZoomButton`, `ZoomButton_Paint` (draws the + or -), `UpdateZoomButtons` (called from `LogChart_PostPaint` at every drawing of the plot area: shows a pair only while its axis is zoomed and puts it at the end of the scroll bar, from the plot area rectangle of `GetPlotAreaRect` and the scroll bar thickness `ScrollBar->Size`, 14), `SetZoomButtonPlace`, `ZoomAxisByButton` (one click: new range from the shown range, `ScaleView->Zoom(start, end)` or `ZoomReset(0)`), `GetAxisFullRange` (the whole range of an axis; if the axis has not calculated it yet, the range of the logged data is used). If the buttons are not exactly on the bar on another PC (for example with a large Windows text scale), change the positions in `UpdateZoomButtons`.

The program itself (C++/CLI) was **not compiled and not tested**. Check first: after a zoom rectangle the buttons appear at the ends of both scroll bars (time axis: right of the right arrow; temperature axis: next to each other above the top of its bar), with a visible + or -, and the scroll bar thumb does not go under them; **+** on one axis zooms only that axis and the other range stays; **-** steps back and the buttons of that axis disappear at the full view; Reset Zoom removes them; the buttons follow when the window is resized; the markers and the cursors stay at the same time while zooming; the graph does not jump while logging runs.

---

### 1.41 Larger graph at the start, separator line in the right-click menu (new, branch 32)

**Smaller table at the start.** When the Data Log window is shown for the first time, the table below the graph was 220 pixels high (about 8 rows), so the graph was small. The table now starts with its column header plus **3 rows** (the 3 newest values), and the graph gets all the rest of the height. The divider between the graph and the table can still be **dragged with the mouse** (drag the top part of the table window) to show more or fewer rows, exactly as before. The number of rows is the setting `_DataLogView_DefaultVisibleRows` (3) at the top of `DataLogGUI.h`; the height is calculated from the real header and row heights of the table, so it also fits with another Windows text scaling. It is only the starting size: a size that you dragged is kept while the window stays open, and the table keeps its height when the main window is resized (the graph takes the change, as before).

**Separator line in the right-click menu.** The right-click menu of the Data Log graph has a thin **white separator line between "Temperature At The Cursor" and "Measure With Two Cursors"**, so the mouse cursor label is set apart from the two-cursor measurement items. The line is colored white on the dark menu background (`CursorSeparator` in `BuildChartContextMenu`). The other items and their order are unchanged.

Change in `DataLogGUI.h` only (`_DataLogView_DefaultVisibleRows`, `DataLogGUI_VisibleChanged`, `BuildChartContextMenu`). The program itself (C++/CLI) was **not compiled and not tested**. Check first: at the first opening of the Data Log window the table shows the header and 3 rows and the graph is clearly larger; the divider drags up and down; the right-click menu shows the line between the first two items.

---

### 1.42 Session files: save and open the settings of the whole program (new, branch 33; extended in branches 34 and 35, see 1.43 and 1.44; log files, see 1.45 and 1.46)

At the top of the main window there is a new **Session** menu bar:

| Menu item | Shortcut | What it does |
|---|---|---|
| Open Session... | Ctrl+O | Opens a session file and sets the program to it |
| Save Session | Ctrl+S | Saves to the current session file (asks for a name the first time) |
| Save Session As... | Ctrl+Shift+S | Saves to a new session file |
| Open Sessions Folder | - | Opens the folder `Documents\IRCAM Thermal Viewer Sessions` in Windows Explorer |

A session file is a plain text file with the extension `.ircamsession` (UTF-8, one `key=value` line per setting, so it can also be read, compared and kept in version control). The name of the current session file is shown in the title bar of the program. The existing automatic **last session** file (`IRCAMApplicationPreset.txt` in Documents, saved when the program closes and read at start-up) works exactly as before and is not changed by session files.

**What a session file contains**

- The **application configuration**: the same values as the automatic last session file (palettes, temperature correction, aspect ratio, enhanced / ultra resolution, save paths for snapshots, recordings and data logging, the colors of the 2D plot lines and of the live view labels, crosshairs, ROIs and lines, CSV delimiters, pop-up flag, recording frame rate).
- The **value of every setting control** of the forms: the thermal camera page (emissivity, ambient and reflected temperature, camera settings, auto calibration, periodic triggers, temperature alarms, data logging and 2D plot settings, ...), the live view tools (palette lists, ...), the temperature plots page, the 3D surface plot page and the emissivity table page - all check boxes, number boxes, drop-down lists, track bars and text boxes that have a name.
- The **Data Log graph settings**: graph title and whether it is shown, notes, number of ticks of both axes, locked X and Y ranges, the mouse cursor label and auto scroll.
- The **window**: maximized or normal window (with size and position, only brought back when it is on a connected screen) and the **page** that was shown (Live View, Thermal Camera, Plots, 3D Plot, Data Log or Emissivity).

**What opening a session does**

Opening a session sets the controls one by one, so the **normal event of each control runs**, exactly as when you change the setting by hand: the setting is applied in the program, not only shown. The status line reports how many settings were set. A control that is **disabled at that moment** (for example camera settings while no camera is connected) is skipped and counted as "not available now"; open the session again after connecting the camera to set them. The camera that is selected in the camera list is **never** changed by a session file, and no camera is connected or disconnected. Buttons that switch a mode (fixed aspect ratio, enhanced resolution, ultra resolution) are clicked only when the mode is different and the button is available. A session cannot be opened while **data logging is running**.

**Not part of a session file (known limits)** - see 1.43 and 1.44 for what branches 34 and 35 added: the full palette range option of the color bar and settings that exist only as menu check marks without a control, the zoom window, the dual color palette rectangle, and video, snapshot and CSV files other than the Data Log file.

**Technical notes.** New header-only file `SessionFile.h` (file reading / writing, escaping, the walk through the controls of a form, dark colors of the menu). `RMH_Application_SaveSession.cpp` was split into `RMH_Application_BuildSessionConfigLines` and `RMH_Application_ApplySessionConfigLines` (the automatic last session file uses the same two routines). `MainGUI.h` has the menu and the save / open routines (`BuildSessionMenu`, `SaveSessionToFile`, `OpenSessionFromFile`), `DataLogGUI.h` has `GetSessionSettings` / `ApplySessionSettings`. The file format has a version number (`IRCAM-SESSION-FILE 1`); a file from a newer program version is refused with a message. A file is written to a temporary file first, so a failed save never destroys a good session file. The program (C++/CLI) was **not compiled and not tested**. Check first: the Session menu is visible and dark; Save Session As writes a file; changing a few settings and then opening the file brings them back; the status line shows the count; opening while a data logging session runs is refused.

---

### 1.43 Live view measurements, logged data, markers, cursors and window positions in session files (new, branch 34)

A session file (1.42) now also holds the three things that were left out in branch 33.

**Live view measurements.** The active **temperature crosshairs (TM1 ... TM10)**, **temperature lines (1 ... 5)** and **ROIs (1 ... 10)** are saved with their position and size (in image pixels), each with its **number**, so TM3 comes back as TM3 and ROI 2 as ROI 2 also when TM1 and ROI 1 were deleted. Opening a session first **removes all measurements that are on the live view**, then creates the saved ones with the same code as the Add buttons (so the right-click menus, the button border colors, the histogram sources and the counters are right) and puts them in place. Positions are kept inside the image (a session from a camera with another image size cannot place a measurement outside it). The labels that you typed for the temperature measurements are not changed by this. The measurements are only set when the live view exists; a session file from branch 33 has none and then leaves the live view as it is.

**Logged data.** The log file that the Data Log window shows (the running session or the session that was logged last) is stored **inside the session file** (compressed with GZip and written as Base64, so a log of many thousands of samples makes a file of a few hundred kilobytes). A log file bigger than 50 MB (`MaxEmbeddedFileBytes` in `SessionFile.h`) is only referred to by its path, and the status line says so. Opening a session writes the log to a CSV file **next to the session file** (`<session name>_DataLog.csv`) and shows it in the Data Log window, so the graph and the table are back, also on another computer. Saving works while data logging runs (the file is read without stopping the logging). Opening is still refused while data logging runs.

**Markers and cursors.** The **graph markers** (time and text) and the **two measurement cursors** (with the Measure With Two Cursors mode) of the Data Log window are saved. The window clears its graph when it loads a log file, so the markers and cursors are set **when the log file has been read into the graph** (the Data Log window reads the file when it is shown; open the Data Log page once to see them). A session file without a log file sets them at once.

**Windows that were pulled out.** For each of the six pages (Live View, Thermal Camera, Plots, 3D Plot, Data Log, Emissivity) the session file records whether the page was in a window of its own and the **position and size of that window**. Opening a session pulls such a page out again (the same as its Undock button, only when its menu button is available) and puts the window where it was; a position that is not on a connected screen is ignored. A page that is in its own window now but was docked in the session file is **left as it is** (a session never closes a window). The docked page that was shown is brought back after that, as before.

**Technical notes.** `RMH_OpenGL_Winforms.h`: new `RMH_OpenGL_GetMovableObjectPosition` / `RMH_OpenGL_SetMovableObjectPosition` (the positions are kept in arrays inside the OpenGL class). `RMH_Application_ThermalViewer.cpp` / `.h`: new `RMH_ThermalViewer_GetLiveViewMeasurementsForSession` / `RMH_ThermalViewer_SetLiveViewMeasurementsFromSession` (entries `TM;slot;x;y`, `LINE;slot;x0;y0;x1;y1`, `ROI;slot;x;y;w;h`; unwanted slots between wanted ones are added and deleted again through the normal Delete routines, so the slot numbers are kept). `DataLogGUI.h`: markers and cursors in `GetSessionSettings` / `ApplySessionSettings(Values, LogFilePath)`, new `GetSessionLogFilePath`, `GetSessionLogDelimiter`, `OpenSessionLogFile`, `TryApplyPendingSessionState` (called after every refresh of the log view). `SessionFile.h`: `CompressToBase64`, `DecompressFromBase64`, `ReadSharedFile`, `MaxEmbeddedFileBytes`. `MainGUI.h`: the save / open routines (keys `lv.*`, `datalog.file.*`, `ui.win.<Page>.*`). The file format version is still 1: files of branch 33 open in branch 34 (the new keys are simply missing), and files of branch 34 open in branch 33 (the new keys are ignored). The program (C++/CLI) was **not compiled and not tested**. Check first: add two crosshairs, a line and an ROI, move them, Save Session As, delete them and Open the session - they are back at the same places and numbers; log some data, add markers and cursors, Save, close the program, start it, Open the session, open the Data Log page - graph, markers and cursors are back; pull out the Data Log page, move its window, Save, dock it, Open - it is a window again at the saved place.

### 1.44 Session files: Data Set Sources, tracking buttons and the zoom of the Data Log graph (new, branch 35)

Two things were missing in the session files (1.42, 1.43): the **Maximum / Minimum / Center Temperature** settings and the **zoom** of the Data Log graph.

**Data Set Source of all plot data sets.** The **Data Set Source** drop-down list of each of the 10 data sets (Temperature Plot Data Set Settings: *Maximum Temp, Minimum Temp, Average Temp, Center Temp, Temp Point 1-10, Line 1-5 Max / Min / Avg, ROI 1-10 Max / Min / Avg, Mouse Position Temp, Thermal Sensor Drift*) is saved with its **slot, list position and name** (`plot.ds.NNN` = `slot;index;name`). Opening a session shows the saved source in the drop-down list and **always sets the data source of the plot** (and so of the data logging and the plot legend), also when the list already showed that source. The source is looked up by its **name** first (so a session still opens right if the list of sources has another order in another program version), then by its list position. The values of the other controls of that menu (enable check box, line width) were already saved with the controls (1.42), the line colors with the application configuration. The sources are set **after** the live view measurements, so a data set that uses Temp Point 3 or ROI 2 is connected to the measurement that was just restored.

**Maximum / Minimum / Center Temperature tracking.** The tracking buttons of the live view tools (**Track Maximum Temperature** hotkey 1, **Track Minimum Temperature** hotkey 2, **Track Center Temperature** hotkey 3) and the **mouse cursor temperature** button were only switches with no entry in the session file. Their on / off state is now saved (`track.NNN` = `MAX;1`, `MIN;0`, `CENTER;1`, `CURSOR;0`). Opening a session switches a button only when its state is different (the same routine as a click on the button, so the green border and the tracking follow).

**Zoom of the Data Log graph.** Before, the graph always opened with the whole view (the zoom was removed on purpose). Now the zoomed time range (saved as **elapsed seconds** since the start of the log) and the zoomed temperature range (saved as **temperature values**) are stored with `datalog.ZoomX`, `datalog.ZoomXMin` / `Max` and `datalog.ZoomY`, `datalog.ZoomYMin` / `Max`. Each axis is saved on its own, so a graph that is zoomed on one axis only (for example with the **+** / **-** buttons, 1.40) comes back that way. The zoom is set **when the log file of the session has been read into the graph** (the same moment as the markers and cursors, 1.43), because the whole range of the axes is not known before. The scroll bars, the **+ / -** buttons and the **orange Reset Zoom border** are there again after opening, and **Reset Zoom** works as usual. The time axis uses the range of the logged data (the automatic range of the axis is not ready right after loading; a first test showed the time zoom lost for this reason), the axes are recalculated first, and a zoom that did not take is tried again at the next refreshes (`_SessionZoomMaxTries`, 20). A saved zoom that lies outside the data, or that is (almost) the whole range, is left out; a zoom that reaches over the end of the data is moved inside it (the size is kept).

**The graph is always stored (also after answering No to "Save Data Logging Session").** A session file stores the log file that the Data Log window follows. Before, answering **No** to the question after a logging session deleted that file, although the graph still showed the data, so a session saved afterwards held no data and the graph was empty when it was opened. Now **No** moves the data to a hidden graph cache (`%TEMP%\IRCAMThermalViewer_DataLogging\GraphCache`; not offered by the crash recovery at start-up) and the window follows the file there. Saving an IRCAM session then stores it as usual (compressed in the session file, or only the path when it is bigger than `SessionFile::MaxEmbeddedFileBytes`). The cache is emptied by the next unsaved logging session and at start-up. The question says what **No** does now.

**Compatibility.** The file format stays version 1. Files of branches 33 and 34 open in branch 35 (the missing keys are simply not set: the drop-down lists, tracking buttons and zoom stay as they are), and a file of branch 35 opens in branch 34 (unknown keys are ignored).

**Technical notes.** `RMH_Application_ThermalViewer.cpp` / `.h`: new `RMH_ThermalViewer_GetPlotDataSetSourcesForSession` / `RMH_ThermalViewer_SetPlotDataSetSourcesFromSession` and `RMH_ThermalViewer_GetTemperatureTrackingForSession` / `RMH_ThermalViewer_SetTemperatureTrackingFromSession`. `MainGUI.h`: the save and open routines write / read the keys `plot.ds.*` and `track.*`. `DataLogGUI.h`: `GetSessionSettings` writes the zoom keys, `ApplySessionSettings` reads them into the new `SessionStateZoom...` fields, new `ApplySessionZoom` is called from `TryApplyPendingSessionState` (which now also runs for a file that has a zoom but no markers). `SessionFile.h`: only the description of the key groups.

---

### 1.45 Open Log File and Save Log File in the Data Log window (new, branch 36; the Save window and the check box changed in branch 41, see 1.50)

Two new buttons in the Data Log window, next to **Show Log File** (which only shows the file in Windows Explorer). The other buttons of the row moved to the right to make room.

**Save Log File** saves the log file that the graph shows (the running session, a saved file, an opened file, or the data of a session that was not saved, 1.44) to a file of your choice, as `.csv` or `.txt`. The Save window has two file types (`.csv` and `.txt`); what the file holds is set with the check box **Include markers, cursors, title and notes** under the button (ticked at the start, branch 41, see 1.50):
- **Ticked** (the default): the data, followed by `#graph.` lines with the **markers** (time and text), the **cursors A and B**, the **measure mode** and the **zoom** of the graph.
- **Not ticked** ('data only'): the plain data, as written by the data logging.

The `#graph.` lines (`#graph.marker=<ms>|<text>`, `#graph.cursorA=<ms>`, `#graph.cursorB=<ms>`, `#graph.measure=0/1`, `#graph.zoomX=<min s>|<max s>`, `#graph.zoomY=<min>|<max>`) start with `#`, like the other note lines of a log file, so they are no samples: the number of samples, the graph and the table are the same with or without them, and older program versions ignore them. In Excel they show as text lines under the data (untick the check box for a clean sheet). Times are elapsed times (the same as the log file), so the graph can be shown again at any time. `#graph.` lines of an earlier save are not copied twice. The file is copied line by line (also when it is very long, and also while a data logging session still writes to it); the file name must not be the file that the graph shows.

**Open Log File** loads a `.csv` or `.txt` log file in the graph and the table, with the markers, cursors, measure mode and zoom that the file has in its `#graph.` lines (set when the file has been read, like the markers and cursors of a session, 1.43). The data delimiter is found in the header line. A file that has no header line of a log file (`Sample, Time(ms), ...`) or no samples is refused with a message. It is not possible while a data logging session runs. When the graph shows the data of a session that was **not saved** (answer No, 1.44), the program asks first whether to save it (Yes: the Save dialog opens, No: the data is lost, Cancel: nothing happens). The opened file is then the log file of the window, so **Save Session** (1.42) stores it, with the markers, cursors and zoom, as for any other log.

**Together with a session file.** A session file keeps its own markers, cursors and zoom (1.43, 1.44); when an opened session has them, those win over the `#graph.` lines of the log file inside it.

**Also in the log file (branch 37):** the graph title, whether it is shown, the notes of **Add Notes** and the hidden lines, see 1.46. **Not saved in the log file:** the line colors (they follow the Temperature Plot Data Set Settings, so they can differ on another PC), the axis lock settings and the number of ticks. They are part of the IRCAM session (1.42). **Automatic after a logging session (branch 38):** when you answer **Yes** to "Save Data Logging Session", the saved file gets the same `#graph.` lines, see 1.47.

**Technical notes.** `DataLogGUI.h`: new buttons `OpenLogButton` / `SaveLogButton`, `SaveLogFileWithDialog`, `BuildGraphStateLines`, `ReadGraphStateLine` (called from `ProcessNewText`), `ApplyFileState` (called from the refresh timer when the file has been read, before the session state), `ClearFileState`; the zoom is set with `ApplySessionZoom` (1.44). `RMH_Application_ThermalViewer.cpp` / `.h`: `RMH_ThermalViewer_FileIsInDataLoggingGraphCache`. Uses `RecoveredSessionInfo::Analyse` (`CrashRecoveryDialog.h`) for the delimiter and the number of samples.

---

### 1.46 Title, notes and hidden lines in log files; hidden lines in sessions; Add Marker and Clear Graph removed (new, branch 37)

**Why.** A log file that is saved with **Save Log File** (1.45) should open months later, or on another PC, looking the way you left it, without the matching IRCAM session. Three things that were only in the session are now in the log file as well.

**In a log file saved with the check box ticked** (more `#graph.` lines after the data, see 1.45):
- `#graph.title=<text>` and `#graph.showTitle=0/1`: the **title** of the graph and whether it is shown.
- `#graph.notes=<text>`: the text of **Add Notes** (a new line in the text is written as `\n`, a backslash as `\\`, so the text stays on one line).
- `#graph.hidden=<list>`: the lines that were **hidden** with their check boxes, as line numbers (`#graph.hidden=2,5`; empty: all lines shown).

**Open Log File** sets them when the file has been read (an empty title is not used; a file without these lines leaves the current title and notes as they are). As for the markers, cursors and zoom, the values of an opened IRCAM session win. The **hidden lines are set before the zoom**, because the temperature range follows the lines that are shown.

**Hidden lines in IRCAM sessions (a gap that was found).** The session did not store which lines were hidden: they were all shown again after opening it. The session now stores them too (`datalog.HiddenLines`, set when the log file has been read, before the zoom). A session of an older branch has no such value and shows all lines, as before.

**Add Marker button removed.** The button added a marker at the time of the newest logged sample, for a running data logging session only. Markers are added with **right-click on the graph -> Add Marker Here...** (also for a stopped log and for any time). To mark a moment of a running session, right-click the graph at its right end (the newest samples). Markers can still be dragged and deleted as before.

**Clear Graph button removed.** It only hid the earlier data in the window (the file stayed complete), which was confusing: a session or a **Save Log File** afterwards still held the whole file, and the graph looked as if the data was gone. To look at a part of the data, zoom the graph (+ / - buttons, zoom rectangle); to start with another log, use **Open Log File** or start a new data logging session. The remaining buttons moved left.

**Technical notes.** `DataLogGUI.h`: removed `AddMarkerButton`, `AddMarkerButton_Click`, `ClearViewButton`, `ClearViewButton_Click` (`AddMarkerAt` and the right-click menu stay, `ClearChartData` is still used by the view restart); new `EscapeGraphText` / `UnescapeGraphText`, `GetHiddenLinesText`, `ParseHiddenLinesText`, `ApplyHiddenLines`; changed `BuildGraphStateLines`, `ReadGraphStateLine`, `ApplyFileState`, `GetSessionSettings`, `ApplySessionSettings`, `TryApplyPendingSessionState`.

---

### 1.47 A saved logging session keeps the markers, cursors and zoom of the graph; Export Graph is now Export Graph Image (new, in this branch)

**Why.** Only **Save Log File** (1.45) wrote the `#graph.` lines. When a data logging session was stopped and the question **Save Data Logging Session** was answered **Yes**, the file in the save folder held the data only. The markers, cursors, zoom, title, notes and hidden lines of the graph were gone as soon as another log file was opened (or the program was restarted), unless **Save Log File** was used as well.

**Now.** Answer **Yes** and the session file is moved to the file you choose (branch 39: 1.48; before it was the save folder with the name of the session file), and then gets the **same `#graph.` lines that Save Log File writes with 'markers, cursors and zoom'** (1.45, 1.46): `#graph.title`, `#graph.showTitle`, `#graph.notes`, `#graph.hidden`, `#graph.marker=...` (one per marker), `#graph.cursorA`, `#graph.cursorB`, `#graph.measure` and `#graph.zoomX` / `#graph.zoomY` (only for a zoomed axis). The lines describe the graph **at the moment the question is answered**, so markers you added while the data logging ran, and markers, cursors and zoom you set after it stopped (before pressing Yes), are in the file. The graph shows the saved file right afterwards (the window follows the file to the save folder) and sets the same state again from the lines, so nothing looks different. Later, **Open Log File** (or **Open Session** with the file) shows the graph as it was, so you can switch between log files without losing the markers and cursors. It makes no difference how the session stopped: Stop Logging, the **N** hotkey, an alarm or trigger, the end of the duration, or closing the program (the same question is asked).

**Same as Save Log File.** The saved file is the same as one written by **Save Log File** with 'markers, cursors and zoom', so you do not need to use that button afterwards. As before, the `#graph.` lines are no samples: the number of samples, the table and the graph are the same with or without them, older program versions ignore them, and in Excel they show as text lines under the data (**Save Log File** with the check box unticked gives a clean copy). **Not in the file:** the line colors, the axis locks and the number of ticks (1.45).

**If the lines cannot be written** (for example when another program locks the file) the session is still saved; the status text shows the warning `Data Logging Session Was Saved, But The Graph Markers, Cursors And Zoom Could Not Be Added To The File: ...` and you can use **Save Log File** with a new file name.

**Answer No (also fixed).** Nothing is saved in the save folder, the data stays in the graph cache for the graph and for an IRCAM session (1.44). Before, the markers and cursors were lost here: the graph starts again when it follows the file to the cache folder, and the file had no `#graph.` lines. Now the cached file gets the same `#graph.` lines, so the graph shows the markers, cursors, zoom, title, notes and hidden lines again when it has read the file, and an IRCAM session that stores the file holds them too. Use **Save Log File** later to save the data with them. A running session is not changed: the markers and cursors are not written to the temporary file while it runs.

**Export Graph Image.** The button **Export Graph** is now named **Export Graph Image**, so it is clear that it saves a picture and not data. Only the text changed, the function is the same (1.13). The button is 7 pixels wider (122 instead of 115) so the longer text fits; the **Show Graph Title** check box next to it did not move. The titles of the windows that the button opens (the Save dialog and its messages) still say **Export Graph**.

**Technical notes.** `DataLogGUI.h`: new public `AppendGraphStateToLogFile(FilePath)` (builds the lines with `BuildGraphStateLines`, adds a new line first when the last line of the file has none, appends the lines as UTF-8 without BOM, with `\r\n` like Save Log File; it does not read the file); the text and width of `ExportGraphButton`. `GlobalObjectsAndVariables.h`: new `GlobalVariables::DataLoggingGraphStateWriter` (`System::Action<System::String^>^`), because the data logging code in `RMH_Application_ThermalViewer.cpp` does not know the Data Log window. `MainGUI.h`: sets the writer to `DataLogGUIForm->AppendGraphStateToLogFile` when the Data Log window is created. `RMH_Application_ThermalViewer.cpp`: in both branches of `RMH_ThermalViewer_AskToSaveDataLoggingSession` (Yes: the saved path, No: the file in the graph cache) call the writer after the copy and before the temporary file is deleted; an error only gives a status warning.

---

### 1.48 Choose the file of a saved logging session; Save Log File can replace the file that the graph shows (new, in this branch)

**Choose the file.** After a logging session stops, the question **Save Data Logging Session** now says that you choose the file name and the folder in the next window. **Yes** opens a Save window (it starts in the save folder with the name of the session file, `.csv` or `.txt`); the session file is moved to the file you choose, wherever it is (any folder, any drive, also a new name), gets the `#graph.` lines (1.47) and the status text shows `Data Logging Session Was Saved: <path>`. The save folder in the settings is only the folder where the window starts, it is not changed. If you cancel the Save window the program asks **Do you want to choose a file?**: Yes opens the window again, No handles the session as answered **No** (the data stays in the graph cache, nothing is lost, 1.44, 1.47). A file that already exists asks for confirmation before it is replaced. The graph follows the file to the new name and folder, and the question is not asked a second time (also not when the program is closed). **No** is not changed.

**Notes after saving.** The notes (**Add Notes**), the title, the markers, the cursors and the zoom are written into the file at the moment you answer the question (**Yes**), and by **Save Log File**; the saved file is a copy of that moment, so notes that are added **after** the session was saved are not in the file until it is saved again. Before, **Save Log File** refused the file that the graph shows ("Choose another file name"), so the only way was a copy with a new name. Now you can choose the **same file** in the **Save Log File** window: the new file is written next to it (`<name>.saving`) and replaces the old one when it is complete, so a failure leaves the old file as it was. Afterwards the graph reads the file again and shows the same state from the new `#graph.` lines. With the check box unticked ('data only') the `#graph.` lines are removed from the file, and the graph shows no markers after it has read it again. It is **not** possible (the same message as before) while the data logging runs, and for the temporary copy of a session that was not saved (answer No): save that with a new name.

**Title and notes belong to the file (also in this branch).** The graph kept its title and notes when another log file was opened, and a file without `#graph.` lines left them as they were. So a file could show a title and notes that were only left over from the file that was shown before (they were not in the file, and they were saved into the next file by **Save Log File**): after a restart the title and the notes were suddenly gone, and it looked like they were saved only sometimes. Now **Open Log File** sets the title to its default (`Title`) and the notes to empty first, and then the title and notes of the file (when it has them) are set when it has been read. A new logging session still keeps the title and notes that are set. **Save Log File** also waits until the graph has read the file ("The graph is still reading the log file. Wait a moment ..."): saving before the title, notes, markers and cursors of the file were set wrote the ones of the graph as it was and lost the ones of the file. After a file has been read, the status text says what the file had: `Graph State Read From The Log File: Title, Notes (42 Characters), 3 Markers, Cursors, Zoom` (or `No Notes`, `Nothing`), so it is clear whether a file holds the notes. To check a file yourself: open it in a text editor, the last lines start with `#graph.title=` and `#graph.notes=`.

**A session saved with an empty graph opens with an empty graph (also in this branch).** A session file holds the logged data of the Data Log window only when the window showed a log file. When it showed none (nothing was logged or opened yet), the session had no data, and opening it did not touch the graph: it kept the file that was shown before (another session, or a log file opened with **Open Log File**), so the session did not open as it was saved. Now opening a session without logged data **shows the graph empty**: no table, no lines, no markers, no cursors, no zoom; then the title, notes and other settings of the session are set. It is the same when the session has only the path of a big log file that is not there any more (a warning in the status text says so). While a data logging runs the graph is not changed (it follows the session that logs). The data of a logging session that was not saved (answer No) is replaced as it already was when a session with logged data is opened.

**Technical notes.** `RMH_Application_ThermalViewer.cpp`: `RMH_ThermalViewer_AskToSaveDataLoggingSession` shows a `SaveFileDialog` (loop until a file is chosen or the user gives up), sets `DataLoggingSessionFolderString` / `DataLoggingSessionFileNameString` to the chosen file, and returns early when the session folder is not the temporary folder any more. `MainGUI.h` (session open) calls the new `DataLogGUI::ShowEmptyGraphForSession()` when the session has no log file; `DataLogGUI.h`: `OpenLogFileWithDialog` resets the title and notes before the file is shown, `SaveLogFileWithDialog` waits while the file is read (`LoadingBehind`, `FileStatePending`, `ViewedFilePath`), `ApplyFileState` writes the status line; `SaveLogFileWithDialog` accepts the shown file (writes `<name>.saving`, copies it over the file, deletes it, sets `ViewedFilePath = nullptr` so the view starts again), the tooltip of **Save Log File**.

---

### 1.49 Start Logging asks before it replaces the log on the graph (new, in this branch)

**Why.** A new data logging session replaces what the Data Log window shows: the table, the lines, the markers and the cursors of the log that was there. Before, **Start Logging** did that without asking, also when the graph showed a session that was not saved (answer **No**), whose data was then lost.

**Now.** When the Data Log window already shows a log (a session that was logged, or a log file that was opened with **Open Log File** or came with a session), pressing **Start Logging** (the button of the main window, the button of the Data Log window, or the hotkey **M**) shows the question **The graph already shows a logged session: `<file name>`. ... Are you sure you want to start a new data logging session?** (**No** is the default button).
- A log that is saved in a file: the text says that the new session replaces it in the graph and that **the file itself is not changed or deleted** (the file is only no longer shown; open it again with **Open Log File**).
- A session that was not saved (answer **No**, the data is only in the graph cache, 1.44): the text says that it is not saved and that **its data is lost**, and to use **Save Log File** first.
- **Yes** starts the session as before (the question about the number of samples comes after it when the session is long). **No** does not start it, the graph stays as it is, and the status text says `Data Logging Was Not Started.`

**Not asked** when the graph is empty (nothing logged or opened yet, or after a session without data was opened, 1.48), when a session is already running, and for an **automatic start** (a temperature alarm or a trigger event starts the data logging): nobody is there to answer, and the question would stop the automatic start.

**Export Graph Image (fixed in this branch).** The button text of branch 38 was not always seen: the button was a fixed 122 pixels wide, but the text is wider with a Windows display scaling of more than 100%, and a button text that does not fit is wrapped on a second line that is cut off, so the button still looked like **Export Graph**. The width is now taken from the size of the text (at least 122 pixels, the text plus 24 pixels), and the **Show Graph Title** and **Auto-scroll table to newest** check boxes next to it move to the right by the same amount. Only the layout of the top row changed; the function of the button is the same.

**Technical notes.** `DataLogGUI.h` `BuildLayout`: `ExportButtonWidth` from `TextRenderer::MeasureText`, `ShowTitleX` and `AutoScrollX` follow it. `GlobalObjectsAndVariables.h`: new `GlobalVariables::DataLoggingGraphLogFileGetter` (`System::Func<System::String^>^`). `MainGUI.h`: sets it to `DataLogGUIForm->GetSessionLogFilePath`. `RMH_Application_ThermalViewer.cpp`: `RMH_ThermalViewer_StartDataLogging` asks (only when `AskUserAboutSampleLimit` is true, which is the interactive start) before the sample limit question, and uses `RMH_ThermalViewer_FileIsInDataLoggingGraphCache` to tell a saved file from an unsaved session.

---

### 1.50 The log file inside a session file keeps the graph state; a session never replaces a different log file; Save Log File has two file types and a check box (new, in this branch)

**Why (session files).** A session file stores the log file of the Data Log window inside it (compressed). Opening the session writes that log out again as `<session name>_DataLog.csv` next to the session file, and the graph shows that file; it is a normal log file that you can open again later with **Open Log File** (for example when you switch between several `.csv` files). The copy inside the session was the file exactly as it was on disk, so it held the markers, cursors, title, notes and hidden lines only when an earlier save (**Save Log File**, or **Yes** after a session stopped, 1.47) had written them into it. The session file keeps its own copy of the markers and cursors (`datalog.marker.*`, `datalog.CursorA` / `B`, which win when the session is opened, 1.45), so the graph looked right when the session was opened, but the restored `_DataLog.csv` itself had no markers: opened again later with **Open Log File** it showed none.

**Now.** **Save Session** builds the log file inside the session from the data of the file plus the `#graph.` lines of the graph as it is at that moment (title, notes, hidden lines, markers, cursors, zoom: the same lines that **Save Log File** writes with the check box ticked). The restored `_DataLog.csv` is therefore a complete log file, and **Open Log File** shows it as you left it.
- The data lines are copied as bytes (nothing is decoded), so they stay exactly as they were; `#graph.` lines that the file already had are replaced by the new ones (never doubled). The log file on disk is not changed.
- The session file still keeps its own `datalog.*` keys: they hold more than the log file does (locked axis ranges, number of ticks, line settings, 1.42) and they are the only copy when the log file is too big to be stored in the session (more than 50 MB: only its path is stored and the file is not rewritten).
- While the graph is still reading the file (an opened long file) its title, notes, markers and cursors are not set yet, and the state of the graph would replace the ones of the file: then the file is stored as it is on disk (as before). While a data logging session runs only the complete lines of the file are stored.

**A session never replaces a different log file.** Opening a session used to write `<session name>_DataLog.csv` over a file of that name without asking, so a newer version of that file (for example saved again with **Save Log File** with more notes) was lost. Now:
- no file of that name: it is written;
- a file of that name with the same content: it is used as it is (opening the same session again does not make copies);
- a file of that name with other content: it is **not** replaced; the data is written to `<session name>_DataLog_2.csv` (then `_3`, `_4` ...) and the graph shows that file; the status text says `Session File: A Different File Named ..._DataLog.csv Already Exists And Was Not Replaced - The Logged Data Is In ..._DataLog_2.csv`.

**Save Log File: two file types and a check box.** The Save window had four entries (`.csv` and `.txt`, each 'with markers, cursors and zoom' or 'data only'), which mixed the file extension and the content. It now has two, **Log file (\*.csv)** and **Log file (\*.txt)** (it starts with the type of the file that the graph shows), and the content is set with the check box **Include markers, cursors, title and notes** in the Data Log window, in a second row under the **Save Log File** button (the toolbar is 24 pixels higher for it). It is **ticked at every start of the program**; it is not stored in a session file.
- **Ticked:** the file gets the `#graph.` lines (markers, cursors, measure mode, zoom, title, notes, hidden lines, 1.45, 1.46).
- **Not ticked:** the plain data only, for Excel, MATLAB, Python and other programs that do not expect the `#graph.` lines. Saving the shown file under its own name (1.48) with the check box unticked removes the `#graph.` lines from it.
- The check box is beside the button and not in the Save window because the standard Windows Save window cannot hold a control of its own without native Windows code (which cannot be tested in this project's build). A tooltip on the check box explains it.
- **Save Data Logging Session** (the question after a session stops, 1.47, 1.48) is not changed: that file always gets the `#graph.` lines, whatever the check box says, so a stopped session can always be shown again as you left it.

**Not changed.** The question about notes that are added after saving still holds (1.48): the markers, cursors, title and notes are written into a file when it is saved. Changes made in the graph after that are not in the file until **Save Log File** is used again; **Open Log File** and **Start Logging** do not ask about such changes yet (see 7).

**Technical notes.** `DataLogGUI.h`: new public `GetLogFileBytesForSession(FilePath)` (reads the file with `SessionFile::ReadSharedFile`, copies the lines that do not start with `#graph.` as bytes, appends `BuildGraphStateLines()` as UTF-8 with `\r\n`; returns the raw bytes while `LoadingBehind` / `FileStatePending` or when the viewed file is not this file); new member `IncludeGraphStateCheckBox` (created in `BuildLayout`, `TopPanel->Height` 74 -> 98); `SaveLogFileWithDialog`: filter `Log file (*.csv)|*.csv|Log file (*.txt)|*.txt`, `FilterIndex` 1 / 2, `WithGraphState` from the check box. `MainGUI.h`: the session save calls `GetLogFileBytesForSession` instead of `SessionFile::ReadSharedFile`; the session open compares an existing `<session>_DataLog.csv` with the stored data and uses `_DataLog_2.csv` ... `_DataLog_999.csv` when it is different.

---

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
| `DataLogGUI.h` (new) | The Data Log window (table and graph that follow the data logging file); light gray scroll bars, arrows and Reset Zoom buttons on both axes (`StyleAxisScrollBar`, `_DataLogScrollBar_...` macros); table below the exported graph (`GetCameraConfigLines`, `GetExportInfoLines`, `SplitInfoLine`), Add Notes button and Graph Notes window (`ShowGraphNotesWindow`, `GraphNotes`), Show Graph Title check box (`ShowTitleCheckBox`, `UpdateGraphTitleVisibility`), text size and high resolution of the exported image (`_DataLogExport_...` macros, direct title editing `LogChart_MouseDoubleClick` / `TitleEditBox`); zoom reset when a new session starts (`ResetView`); frame around the plot area (secondary axes with only the line shown); first / last axis values in the strip of an exported image (`GetAxisRangeTexts`, `FormatAxisTime`); Clear Graph button; orange Reset Zoom border while zoomed (`UpdateZoomIndicator`); Yes / No questions for Stop Logging and Clear Graph; follows the session file to the saved location; mouse cursor temperature label in the graph (right-click menu); note about many samples in the opening notice; zoom rectangle on both axes; right-click menu of the graph (cursor temperature, number of ticks, Y-axis range set / lock at current / unlock); check box per graph line; Export Graph Image button (graph saved as image); X-axis range items (set / lock at current / unlock); graph title; dark green / dark red Start / Stop Logging button borders; scroll steps of the zoomed graph (`UpdateScrollSteps`); round X-axis tick steps (`GetRoundTickSeconds`, `GetDataEndTime`); fixed plot area size (`UpdatePlotAreaLayout`, `LogChart_SizeChanged`) |
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
| `DataLogGUI.h` (also, branch 29) | Typed cursor times (1.38): `SetCursorTimesMenuItem_Click`, `TryParseElapsedTime`, `RemoveMeasureCursor`; `IsPointInPlotArea` now tests by position (ends of the shown axis ranges) and `UpdateCursorLabel` uses it (label also on a cursor line); new item `Set Cursor Times...` in the right-click menu (`SetCursorTimesMenuItem`) |
| `DataLogGUI.h` (also, branch 31) | Markers (1.39): new class `GraphMarker`, `GraphMarkers`, `LogChart_PostPaint`, `DrawMarkers`, `FindMarkerAt`, `UpdateMarkerHover`, `AddMarkerAt`, `ShowMarkerTextWindow`, `AddMarkerButton_Click`, `CreateMarkerMenuItems`, `UpdateMarkerMenuItems`, `ClearGraphMarkers`; marker part in `LogChart_MouseDown` / `LogChart_MouseMove` / `LogChart_MouseUp` / `LogChart_MouseLeave`; new items in the right-click menu (`Add Marker Here...`, `Delete Marker`, `Delete All Markers`); **Add Marker** button in the top bar (Export Graph Image and the two check boxes moved to the right); `_DataLogMarker_MaxTextLength` |
| `DataLogGUI.h` (also, branch 31) | Zoom buttons (1.40): `BuildZoomButtons`, `StyleZoomButton`, `UpdateZoomButtons` (called from `LogChart_PostPaint`), `SetZoomButtonPlace`, `ZoomAxisByButton`, `GetAxisFullRange`, the four click handlers (`XZoomInButton_Click` ...); `UpdatePlotAreaLayout` (fixed layout also while zoomed, larger margins); `_DataLogZoomButton_Factor`, `_DataLogZoomButton_MinTimeMilliSec`, `_DataLogZoomButton_MinTemperatureRange`, `_DataLogPlotArea_ReserveZoomRoom` |
| `RMH_OpenGL_Winforms.h` (also, branch 34) | Position access for session files (1.43): `RMH_OpenGL_GetMovableObjectPosition` / `RMH_OpenGL_SetMovableObjectPosition` |
| `RMH_Application_ThermalViewer.cpp` / `.h` (also, branch 34) | Live view measurements for session files (1.43): `RMH_ThermalViewer_GetLiveViewMeasurementsForSession` / `RMH_ThermalViewer_SetLiveViewMeasurementsFromSession` |
| `SessionFile.h` (new, branch 33; branch 34: compression of the log file) | Session files (1.42): file reading / writing, escaping, the walk through the controls of a form, dark menu colors |
| `RMH_Application_SaveSession.cpp` / `.h` (also, branch 33) | Split into `RMH_Application_BuildSessionConfigLines` and `RMH_Application_ApplySessionConfigLines` (1.42) |
| `MainGUI.h` (also, branches 33 and 34) | Session menu bar and the save / open routines (1.42) |
| `DataLogGUI.h` (also, branches 32, 33 and 34; 33: `GetSessionSettings` / `ApplySessionSettings`; 34: markers, cursors and the log file in a session) | Larger graph at the start and separator line (1.41): new setting `_DataLogView_DefaultVisibleRows` (3), table height in `DataLogGUI_VisibleChanged`, `CursorSeparator` in `BuildChartContextMenu` |
| `RMH_Application_ThermalViewer.cpp` / `.h` (also, branch 35) | Data Set Sources and tracking buttons for session files (1.44): `RMH_ThermalViewer_GetPlotDataSetSourcesForSession` / `...SetPlotDataSetSourcesFromSession`, `RMH_ThermalViewer_GetTemperatureTrackingForSession` / `...SetTemperatureTrackingFromSession` |
| `DataLogGUI.h` (also, branch 37) | Title, notes and hidden lines in log files (`#graph.title`, `#graph.showTitle`, `#graph.notes`, `#graph.hidden`) and hidden lines in sessions (`datalog.HiddenLines`); Add Marker and Clear Graph buttons removed (1.46) |
| `DataLogGUI.h` (also, branch 36) | Open Log File / Save Log File buttons, `#graph.` lines for markers, cursors and zoom in a log file (1.45) |
| `RMH_Application_ThermalViewer.cpp` (also, branch 36) | `RMH_ThermalViewer_FileIsInDataLoggingGraphCache` (1.45) |
| `RMH_Application_ThermalViewer.cpp` (also, branch 35, graph cache) | `RMH_ThermalViewer_GetDataLoggingGraphCacheFolder` / `RMH_ThermalViewer_ClearDataLoggingGraphCache`, the No branch of `RMH_ThermalViewer_AskToSaveDataLoggingSession` keeps the data for the session (1.44) |
| `MainGUI.h` (also, branch 35) | Keys `plot.ds.*` and `track.*` written and read by the session save / open routines (1.44) |
| `DataLogGUI.h` (also, branch 35) | Zoom of the graph in a session file (1.44): zoom keys in `GetSessionSettings` / `ApplySessionSettings`, new `ApplySessionZoom`, `SessionStateZoom...` fields |
| `SessionFile.h` (also, branch 35) | Description of the new key groups (comment only) |
| `DataLogGUI.h` (also, branches 38 and 39) | Branch 39: **Save Log File** can replace the file that the graph shows (1.48). Branch 38: | `AppendGraphStateToLogFile` (the saved logging session gets the `#graph.` lines, 1.47); the **Export Graph** button is named **Export Graph Image** (text, width 115 -> 122) |
| `DataLogGUI.h` (also, branch 40) | The width of the **Export Graph Image** button follows the text size (display scaling), the two check boxes next to it move with it (1.49) |
| `GlobalObjectsAndVariables.h` (branches 38 and 40) | Branch 40: new `GlobalVariables::DataLoggingGraphLogFileGetter` (1.49). Branch 38: | New `GlobalVariables::DataLoggingGraphStateWriter` (1.47) |
| `MainGUI.h` (also, branches 38, 39 and 40) | Branch 40: sets `DataLoggingGraphLogFileGetter` (1.49). Branches 38 and 39: | Sets `DataLoggingGraphStateWriter` when the Data Log window is created (1.47). Branch 39: a session without logged data opens with an empty graph (1.48) |
| `RMH_Application_ThermalViewer.cpp` (also, branches 38, 39 and 40) | Branch 40: **Start Logging** asks before it replaces the log on the graph (1.49). Branches 38 and 39: | Branch 39: **Save Data Logging Session** opens a Save window (1.48). Branch 38: | The Yes branch of `RMH_ThermalViewer_AskToSaveDataLoggingSession` calls the writer after the file was copied to the save folder; the No branch does the same for the graph cache file (1.47) |
| `DataLogGUI.h` (also, branch 41) | The log file inside a session file keeps the graph state (`GetLogFileBytesForSession`, 1.50); **Save Log File**: two file types and the check box `IncludeGraphStateCheckBox` ("Include markers, cursors, title and notes", ticked at the start), `SaveLogFileWithDialog` follows it; the top panel is 98 pixels high (second row for the check box) |
| `MainGUI.h` (also, branch 41) | Session save stores `GetLogFileBytesForSession` instead of the raw file; session open never replaces a different `<session>_DataLog.csv` (same content: used as it is; other content: `_DataLog_2.csv`, `_3.csv` ... and a warning in the status text) (1.50) |
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

**Data Log graph: table size, markers and zoom buttons** (top of `src/DataLogGUI.h`, near `_DataLogExport_ResolutionScale`):

| Setting | Default | Meaning |
|---|---|---|
| `SessionFile::MaxEmbeddedFileBytes` | 52428800 (50 MB) | Largest Data Log file that is stored inside a session file; a bigger one is only referred to by its path (1.43) |
| Session file extension | `.ircamsession` | Default folder: `Documents\IRCAM Thermal Viewer Sessions` (1.42) |
| `_DataLogView_DefaultVisibleRows` | 3 | Number of table rows visible when the Data Log window is first shown (1.41); the graph gets the rest. The divider can be dragged afterwards |
| `_DataLogMarker_MaxTextLength` | 60 | Most characters in the text of a marker (1.39) |
| `_DataLogZoomButton_Factor` | 0.7 | One click on X+ / Y+ shows this part of the visible range; X- / Y- shows 1 / this part (1.40). Smaller = bigger steps |
| `_DataLogZoomButton_MinTimeMilliSec` | 10.0 | Smallest time range that X+ zooms to (milliseconds) |
| `_DataLogZoomButton_MinTemperatureRange` | 0.01 | Smallest temperature range that Y+ zooms to |
| `_DataLogPlotArea_ReserveZoomRoom` | 1 | 1 = the plot area always has the larger margins that the zoom scroll bars and buttons need, so it never changes size when zooming (the graph is a little smaller); 0 = smaller margins while the graph is not zoomed (the plot area changes size when zooming starts and ends) (1.40) |

**Calibration freeze** (top of `src/RMH_SupportedIRCameras_Resources.h`):

| Setting | Default | Meaning |
|---|---|---|
| `_CalibrationFreezeSettleTimeMs` | 300 | Time in ms that the plot and data logging values stay frozen after a shutter calibration has finished. Raise it if a small bump is still visible right after the shutter reopens. |

---

## 5. Building it

The GitHub Actions build only starts automatically for pushes to `main`. For this branch:
**Actions -> build -> Run workflow -> choose `IRCAM-for-T2Splus_V2_2Dplots_expansion_41` -> Run workflow**, then download the build from the run
page. If a build fails, search the log for `error C` (ignore the `warning` lines) to find the real error.

Fallbacks: Build #7 (commit `7b355e8`, branch `fix-t2s-v2-temperature`) and the branches `t2s-v2-calibration-diagnostic`,
`IRCAM-for-T2S-V2`, `IRCAM-for-T2Splus_V2_2Dplots_fix` and `IRCAM-for-T2Splus_V2_2Dplots_expansion_01`, `_02`, `_03`, `_04`, `_05`, `_06`, `_07`, `_08`, `_10`, `_12`, `_13`, `_15`, `_16`, `_17`, `_18`, `_19`, `_20`, `_21`, `_22`, `_23` and `_29` (the last branch before markers and zoom buttons).

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
| Data Log window: table starts with 3 rows, graph larger, divider still draggable (1.41) | Not tested yet, not compiled yet |
| Data Log graph: separator line between Temperature At The Cursor and Measure With Two Cursors (1.41) | Not tested yet, not compiled yet |
| Session menu: visible and dark, Save / Save As / Open work, status line count (1.42) | Not tested yet, not compiled yet |
| Session file: settings come back after changing them and opening the file; camera selection unchanged (1.42) | Not tested yet, not compiled yet |
| Session file: opening is refused while data logging runs; a damaged or foreign file gives a message (1.42) | Not tested yet, not compiled yet |
| Session file: live view crosshairs, lines and ROIs come back with their numbers and positions (1.43) | Not tested yet, not compiled yet |
| Session file: logged data, markers and cursors come back in the Data Log window (also after a restart of the program) (1.43) | Not tested yet, not compiled yet |
| Session file: a saved log bigger than 50 MB is only referred to by its path, status line says so (1.43) | Not tested yet, not compiled yet |
| Session file: pulled-out windows are pulled out again at the saved position; an off-screen position is ignored (1.43) | Not tested yet, not compiled yet |
| Session file of branch 33 opens in branch 34 without errors (1.43) | Not tested yet, not compiled yet |
| Data Log graph: Set X-Axis Range And Lock (valid and invalid times), Lock X-Axis At Current Range, Unlock X-Axis; zoom and ticks inside a locked range | Not tested yet |
| Data Log graph: tick Show Graph Title: bold `Title` at the top center; double-click it, type, Enter keeps it, Esc cancels, empty text hides it; the title is in the exported image | Not tested yet |
| Data Log graph: Export Graph Image first shows the title window with the current title (also when the title is hidden); Set Title continues, Cancel cancels the export | Not tested yet |
| Data Log graph: Export Graph Image saves PNG / JPEG / BMP with the shown lines named above the graph | Not tested yet |
| Data Log graph: left-drag rectangle zooms both X and Y; Reset Zoom works | Not tested yet |
| Warning window for more than 100000 samples: Start starts, Cancel does not start; no window for alarm / trigger starts | Not tested yet |
| Calibration freeze: no spikes / dips in the plot and in the data logging file during periodic (auto) calibration, Calibrate Camera button and drift-based calibration | Not tested yet |
| Calibration freeze: no bump right after the shutter reopens with the default 300 ms settle time | Not tested yet |
| Calibration freeze: build compiles (MSVC, /clr) | Not tested yet |
| Data Log graph: frame on all four sides of the plot area (also while zoomed and in the exported image); layout of the graph unchanged | Not tested yet |
| Export Graph Image: line with the first and last value of the time and temperature axis in the strip (automatic range, zoomed, locked X / Y range, number of ticks) | Not tested yet |
| Export Graph Image: the axis line is split into two rows for a narrow graph; milliseconds shown only for a zoomed range below one minute (not for a short unzoomed log) | Not tested yet |
| Frame and Export Graph Image change: build compiles (MSVC, /clr) | Not tested yet |
| Data Log graph: Clear Graph button (new name, new question text) clears table and graph after Yes | Not tested yet |
| Data Log graph: Reset Zoom border turns orange when zoomed (drag rectangle, one axis or both) and returns to gray after Reset Zoom / locking or unlocking a range | Not tested yet |
| Data Log graph: scroll bars, arrows and small Reset Zoom buttons are light gray and clearly visible on both axes when zoomed; arrows are visible on the buttons | Not tested yet |
| Export Graph Image: list "Thermal Camera Configuration" below the graph with all 6 settings; values follow the panel (after Set) and the temperature unit (°C, °F, K) | Not tested yet |
| Export Graph Image: image with the list saves correctly as PNG, JPEG and BMP; graph and strip above unchanged | Not tested yet |
| Export Graph Image: Export Information list (date, time, camera model, temperature range, auto / drift calibration, log file, interval, duration, samples, logged time, unit) on the right of the configuration list; below it for a narrow image | Not tested yet |
| Zoomed graph while logging: the view stays on the zoomed part (axis labels do not change); the thumb of the scroll bar moves left and gets smaller as the log grows (the bar shows the position in the whole log) | Not tested yet |
| Start a new logging session while the graph is zoomed: both axes reset, Reset Zoom border back to gray; a locked range stays | Not tested yet |
| Change the temperature unit with different Ambient and Reflected values: each box keeps its own value (then Set, then Read) | Not tested yet |
| Add Notes button sits between Reset Zoom and Export Graph Image; Auto-scroll check box and Export Graph Image are fully visible (window opened separately and docked) | Not tested yet |
| Add Notes: window shows the current notes; Set Notes keeps them, Cancel does not change them, empty text removes them; Enter starts a new line | Not tested yet |
| Export Graph Image: Graph Title window, then Graph Notes window (Cancel in either cancels the export), then the save window | Not tested yet |
| Exported image: table with 3 cells (no notes) or 4 cells (with notes) in the order Configuration, Export Information, Other Information, Notes; borders line up | Not tested yet |
| Exported image: long notes (several lines, 2000 characters) wrap inside the Notes cell; a narrow graph moves cells to a next row | Not tested yet |
| Exported image: line names, axis ranges, table, graph title and axis titles are about 20 % bigger than the axis values (20.0, 0:00:00); the axis values and the graph on the screen are unchanged after the export | Not tested yet |
| Show Graph Title check box (before Auto-scroll): not ticked at the start; ticked shows `Title`; unticked hides it, text kept; the right-click menu has no Set Graph Title...; Export Graph Image asks for the title when it is hidden (new title shows it, empty hides it, same text keeps it) | Not tested yet |
| Exported image resolution: 3 x the pixels; lines and text sharp when zoomed in a viewer (PNG, JPEG, BMP); compare `_DataLogExport_UseVectorChart` 1 and 0; the graph looks the same as on the screen (colors, lines, frame, zoom, locked range) | Not tested yet |
| Export Graph Image on a big window / long log: no out-of-memory or very slow export at 3 x resolution | Not tested yet |
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
| Decimation: the pause when the lines are filled again is short; zoom and a locked X / Y range stay while it happens; Export Graph Image of a thinned graph works | Not tested yet |
| `_DataLogView_ThinOldPoints` 0: the oldest points scroll off the left side as before | Not tested yet |
| Block reading and decimation: build compiles (MSVC, /clr) | Not tested yet |
| Markers (1.39): Add Marker button while logging (line at the newest sample, time above the text); Cancel removes it; Add Marker Here... on a running and on a stopped log; empty text shows only the time | Not tested yet |
| Markers: drag by the line and by the text (with and without Measure With Two Cursors; no zoom rectangle appears); a marker cannot leave the logged time range; right-click on a marker offers Delete Marker, elsewhere it does not; Delete All Markers asks first | Not tested yet |
| Markers: stay at their time while zoomed / scrolled / the window is resized; several close markers are put in rows; Clear Graph and a new session remove them; markers are in an exported image | Not tested yet |
| Zoom buttons (1.40): after a zoom rectangle the + / - buttons appear on the scroll bars (time axis: right of the right arrow; temperature axis: above the top of its bar), with a visible + or -; none are shown when the graph is not zoomed | Not tested yet |
| Zoom buttons: + / - zoom only their own axis, also after a zoom rectangle and in any order; - ends at the full view and the scroll bar and the buttons of that axis disappear; Reset Zoom removes them; they follow a window resize | Not tested yet |
| Zoom buttons: the plot area does not change size at any click of + / - (also when labels get more digits and when the last - ends the zoom); the buttons do not cover any label; while logging runs the zoomed view stays; with a locked X / Y range and with a number of ticks set the buttons still work and the ticks follow; markers and cursors stay at their time; the graph does not jump | Not tested yet |
| Markers and zoom buttons: build compiles (MSVC, /clr) | Not tested yet |
| Session (1.44): change several Data Set Source drop-down lists (also Maximum Temp, Minimum Temp, Center Temp, a Temp Point, a Line, an ROI, Mouse Position Temp), Save Session, change them again, Open Session: all 10 lists show the saved sources, the plot legend and the plot lines follow, a new data logging session logs the saved sources | Not tested yet |
| Session (1.44): switch Track Maximum / Minimum / Center Temperature and the mouse cursor temperature on and off in different combinations (buttons and hotkeys 1, 2, 3), save, switch them, open: the green borders and the tracking follow the file; a button that is in the right state is not toggled | Not tested yet |
| Session (1.44): zoom the Data Log graph with a zoom rectangle, save, reopen the session (or the program): the same time and temperature range is shown, scroll bars and + / - buttons are there, the Reset Zoom border is orange, Reset Zoom works | Not tested yet |
| Session (1.44): zoom only the time axis (or only the temperature axis) with the + / - buttons, save, open: only that axis is zoomed; a graph that is not zoomed opens not zoomed; a session of branch 34 opens without errors | Not tested yet |
| Session (1.44): a session with a zoom and a locked X / Y range, and one with a zoom and markers / cursors, open correctly (zoom after the markers and cursors are set); a session without a log file and with a zoom does not give an error | Not tested yet |
| Session (1.44): start data logging, stop it, answer No to 'Save Data Logging Session', save the IRCAM session, restart the program, open the session: the graph and the table show the logged data (also zoom, markers, cursors); the file is not in the save folder; no 'Unsaved Data Logging Sessions' question at start-up | Not tested yet |
| Session (1.44): build compiles (MSVC, /clr) | Not tested yet |
| Log files (1.46): set a graph title (shown), write notes with several lines and a backslash, hide two lines, Save Log File with markers..., change title / notes / unhide, Open Log File: title, notes and hidden lines come back; the temperature range fits the shown lines | Not tested yet |
| Log files (1.46): a log file saved with the check box unticked ('data only') or by an older branch keeps the current title and notes when opened | Not tested yet |
| Session (1.46): hide lines, Save Session, restart, open the session: the same lines are hidden (also with zoom); a session of branch 35 shows all lines | Not tested yet |
| Data Log toolbar (1.46): Add Marker and Clear Graph are gone, the other buttons and the two check boxes are in one row without overlap; right-click -> Add Marker Here... still works while logging and for a stopped log | Not tested yet |
| Build compiles (MSVC, /clr) after the buttons were removed (no left-over reference to AddMarkerButton / ClearViewButton) | Not tested yet |
| Log files (1.45): add markers and set cursors A and B, zoom the graph, Save Log File with the check box ticked; Clear Graph, Open Log File: the same graph, markers, cursors and zoom come back, the sample count is the same | Not tested yet |
| Log files (1.45): untick the check box, save, open: the data without markers, cursors and zoom; the saved files open in Excel (the `#graph.` lines show as text under the data) | Not tested yet |
| Log files (1.45): Save Log File while a data logging session runs (file is complete up to the last line), and after answering No to the save question; the same file name as the shown file is refused | Not tested yet |
| Log files (1.45): Open Log File with a file that is no log file (a text file, an empty file, a file with only the header line): a message, the graph is not changed; Open is refused while logging runs; with unsaved data (answer No) the question Yes / No / Cancel works | Not tested yet |
| Log files (1.45): open a log file, Save Session, restart, open the session: the log file, markers, cursors and zoom are there (the session values win); a log file of an older version (no `#graph.` lines) opens as before | Not tested yet |
| Log files (1.45): the toolbar of the Data Log window shows all buttons without overlap at 1920 px and at a smaller window width; build compiles (MSVC, /clr) | Not tested yet |
| Saved session (1.47): start data logging, add two markers (right-click -> Add Marker Here...) while it runs, stop it, set cursors A and B and zoom, answer Yes: the status text says the session was saved; the `.csv` / `.txt` in the save folder ends with `#graph.` lines (title, showTitle, notes, hidden, two markers, cursorA, cursorB, measure, zoomX / zoomY); the graph looks the same as before the answer | Not tested yet |
| Saved session (1.47): open another log file, then Open Log File with the saved session file: the same markers (text and time), cursors, measure mode, zoom, title, notes and hidden lines; the number of samples is the same as in the table before | Not tested yet |
| Saved session (1.47): the session was stopped with the N hotkey, by an alarm / trigger stop, at the end of the duration and by closing the program (answer Yes): the saved file has the `#graph.` lines in each case | Not tested yet |
| Saved session (1.47): add markers, set cursors A and B and zoom, stop, answer No: nothing in the save folder, the graph still shows the data **with the same markers, cursors, zoom, title, notes and hidden lines**; Save Session and open it again: the same; Save Log File afterwards has the `#graph.` lines | Not tested yet |
| Saved session (1.47): a session with no marker and no cursor and no zoom: the file only gets title / showTitle / notes / hidden lines, and opens normally | Not tested yet |
| Saved session (1.47): the saved file in Excel and in a text editor: the `#graph.` lines are on their own lines under the data (not glued to the last sample line); the session also saved with the delimiter set to `;` or tab | Not tested yet |
| Saved session (1.47): file that is locked by another program when answering Yes: the session is saved, the status text shows the warning, no crash | Not tested yet |
| Export Graph Image (1.47): the button text is fully visible (also at a high Windows display scaling), does not touch **Show Graph Title**; the click works as before (Graph Title window, Graph Notes window, Save dialog, image) | Not tested yet |
| Build compiles (MSVC, /clr) with `DataLoggingGraphStateWriter` (the `gcnew System::Action<System::String^>(...)` in `MainGUI.h`, the public `AppendGraphStateToLogFile`) | Not tested yet |
| Choose the file (1.48): stop a session, Yes: the Save window starts in the save folder with the name of the session file; choose another folder and another name: the file is there with the `#graph.` lines, nothing is in the save folder, the temporary file is gone, the graph follows the file (the markers stay), the status text names the path | Not tested yet |
| Choose the file (1.48): Yes, cancel the Save window, Yes (choose again), then cancel and No: the data stays in the graph (no markers lost), the file is not in the save folder, Save Log File works with a new name | Not tested yet |
| Choose the file (1.48): after saving to a chosen folder, close the program: the question is not asked a second time; start the program: no crash recovery question for it; an existing file name asks before it is replaced | Not tested yet |
| Notes (1.48): add notes before stopping, stop, Yes: the file has a `#graph.notes=` line, Open Log File (also after restart) and Add Notes shows the notes | Not tested yet |
| Notes (1.48): open the saved file, Add Notes, then Save Log File with the same name (with markers, cursors and zoom): no error, the file has the new `#graph.notes=` line and no doubled `#graph.` lines, the same number of samples, the graph shows the same markers and cursors again; no `.saving` file is left | Not tested yet |
| Save Log File (1.48): the same name while the data logging runs, and for the data of a session that was answered No: the message "cannot be replaced now"; a copy with a new name still works | Not tested yet |
| Title and notes (1.48): file A with a title and notes, file B with other ones; open A, B, A: each shows its own title and notes (Add Notes window); open a file without `#graph.` lines after A: the title is `Title` and the notes are empty; the status text names what each file had | Not tested yet |
| Title and notes (1.48): open a long log file and press Save Log File at once: the message "still reading" (no file is written); after the data is shown it saves with the title and notes of the file | Not tested yet |
| Session with an empty graph (1.48): start the program, save session_1 (nothing logged); open a log file or another session with data; open session_1: the graph is empty (no table, no lines, no markers) with the title and notes of session_1, the status text says that no data logging session exists yet; Start Logging afterwards works as on a fresh start | Not tested yet |
| Session with an empty graph (1.48): open a session with data after session_1: the data and markers are shown as before | Not tested yet |
| Start Logging (1.49): the graph shows a saved log file (Open Log File): Start Logging (main window button, Data Log window button and hotkey M) asks; No: nothing starts, the status text says `Data Logging Was Not Started.`, the graph is unchanged; Yes: the session starts and the file on disk is unchanged | Not tested yet |
| Start Logging (1.49): after a session that was answered No: the question says it is not saved and the data is lost; Yes starts, No keeps the graph and Save Log File still works | Not tested yet |
| Start Logging (1.49): empty graph (fresh start, or after a session without data was opened): no question; a long session: the question about the number of samples still comes (after the first one) | Not tested yet |
| Start Logging (1.49): a temperature alarm / trigger event that starts the data logging while the graph shows a log: no question, the session starts | Not tested yet |
| Export Graph Image (1.49): at a Windows display scaling of 100%, 125% and 150%: the full text **Export Graph Image** is seen on one line, the button does not touch **Show Graph Title**, which does not touch **Auto-scroll table to newest**; the click works as before | Not tested yet |
| Session (1.50): add two markers, set cursors A and B, a title and notes (do not use Save Log File), Save Session; open the session: the `<session>_DataLog.csv` next to it ends with `#graph.` lines (title, showTitle, notes, hidden, markers, cursors); open another log file, then **Open Log File** with `<session>_DataLog.csv`: the same markers, cursors, title and notes are shown | Not tested yet |
| Session (1.50): a log file that already has `#graph.` lines, change one marker, Save Session, open: the `_DataLog.csv` has one set of `#graph.` lines (none doubled), the same number of samples, and the data lines are identical to the original file (file compare; also a header line with a non-ASCII character) | Not tested yet |
| Session (1.50): Save Session at once after opening a long log file (the graph is still reading): the session is saved, the file inside is the file as it is on disk (the title and notes of the file are not replaced by the default ones); Save Session while a data logging session runs: works, the `_DataLog.csv` has only complete lines | Not tested yet |
| Session (1.50): open the same session twice: the `_DataLog.csv` is used again, no `_DataLog_2.csv`; save the shown `_DataLog.csv` again with Save Log File (more notes) under the same name, open the session: that file is **not** replaced, the data is in `_DataLog_2.csv` (the graph shows it), the status text names both files | Not tested yet |
| Session (1.50): a log file bigger than 50 MB: only its path is stored, the file on disk is not changed, the markers and cursors of the session keys still come back | Not tested yet |
| Save Log File (1.50): the Save window has two file types, `.csv` and `.txt` (it starts with `.txt` for a `.txt` file); the check box is ticked after the program starts; ticked: the file has `#graph.` lines; not ticked: none, and the file opens in Excel with only the data | Not tested yet |
| Save Log File (1.50): the same file name as the shown file with the check box unticked: the `#graph.` lines are gone from the file and the graph shows no markers after it has read it again; with it ticked again the markers come back after Save Log File | Not tested yet |
| Data Log toolbar (1.50): the check box is under the **Save Log File** button, the full text is seen (also at 125% and 150% Windows display scaling), nothing overlaps, the table and the graph fill the rest of the window, the tooltip of the check box shows | Not tested yet |
| Saved session (1.50): untick the check box, stop a data logging session and answer Yes: the saved file still has the `#graph.` lines (the check box only applies to Save Log File) | Not tested yet |
| Build compiles (MSVC, /clr) with `GetLogFileBytesForSession`, `IncludeGraphStateCheckBox` and the file name loop with `break` in the session open routine of `MainGUI.h` | Not tested yet |

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
- Markers (1.39): they are not saved in the log file (they are gone when the program closes or the graph is cleared), so a marker that matters should also be noted with **Add Notes** before exporting. The **Add Marker** button uses the time of the newest *logged* sample as it is read by the window (it refreshes about once a second), so the line can be up to about a second behind the real moment: drag it if that matters. With very many markers close together the boxes can overlap (the rows only go down to the bottom of the plot area). The text has only the printable characters of the font; the box does not wrap a long text. Markers exist in the Data Log graph only, not in the live 2D plot (Plots window).
- Zoom buttons (1.40): the zoom is around the middle of the shown range (not around the mouse); the step is a fixed factor (`_DataLogZoomButton_Factor`); there is no mouse-wheel zoom and no key shortcut. The chart draws its scroll bars itself, so the buttons are separate small buttons laid over the bar at a computed place (next to the arrows); they are put in the margins beyond the end of the bar (about 30 pixels right of the plot area for the time axis, the top margin above the plot area for the temperature axis; the time axis ones are moved left when there is no room at the right), and the place can be a few pixels off with another Windows text scale. The chart cannot shorten a scroll bar, so the buttons stay outside the plot frame. The larger margins make the graph about 35 pixels narrower and 33 pixels lower than in branch 29 (`_DataLogPlotArea_ReserveZoomRoom`); with a very large Windows text scale a label can still be cut off (increase the zoom margins in `UpdatePlotAreaLayout`). They show only for an axis that is zoomed, so the other axis cannot be zoomed with them until a zoom rectangle is dragged.
- Session files (1.44): the saved temperature zoom is in the temperature unit of the program at the time of saving; when the unit is changed before opening, the temperature range is not converted (a zoom outside the data is left out). The time zoom is in elapsed seconds, so it needs the same log data (a session without logged data has nothing to zoom, the zoom is then left out). The tracking buttons are set also when no camera is connected.
- Log files and sessions (1.50): the check box of **Save Log File** is ticked at every start of the program (it is not stored). The markers, cursors, title and notes are written into a file when it is saved; changes made in the graph after that are not in the file until **Save Log File** is used again, and **Open Log File** and **Start Logging** do not ask about such changes (only about the data of a session that was not saved). The check box is beside the **Save Log File** button, not in the Save window. A log file bigger than 50 MB is not stored in a session (only its path), so its markers are only in the session keys.
- The frame around the Data Log graph (1.20) is always on; to remove it, delete the `AxisX2` / `AxisY2` lines in `BuildChart` (`DataLogGUI.h`).

---

## 8. Sources used

The InfiRay Android APK (XthermNew 6.6.230906), the analysis of the InfiRay Windows viewer, IR-Py-Thermal, and
`dzaczek/t2s-thermal-macos` (which note that the V2 shutter sensor drifts).
