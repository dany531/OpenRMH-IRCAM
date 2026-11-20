# Changelog

Tags look like `v<upstream version>-community.<n>`: the first part is the version of IRCAM Thermal Viewer the source came from,
the number counts builds of this repository.

## Unreleased

- **Change (branch `IRCAM-for-T2Splus_V2_2Dplots_expansion_27`, not tested yet, not compiled yet): the oldest part of a long Data Log graph is thinned out (min / max decimation) instead of scrolling off the left side.**
  Before, a line of the graph kept the newest `_DataLogView_MaxPlotPoints` (100000) points and the oldest points were deleted, so in a long session (more than about 27.8 hours at 1 s, 66 minutes at 0.04 s) the start of the
  session disappeared from the graph. Now, when a line has more than 100000 points, the **older half** of its points is thinned out: of every **4 neighbouring points the lowest and the highest are kept** (they are real points with
  their own time, in time order), so a peak or a dip is never lost and the whole session stays on the graph. The newer half is not changed, so at least the newest 50000 samples always have full detail; points that were thinned out
  are thinned out again the next time, so the older the data the coarser it is. The very first point of a line is always kept (the left end of the graph stays at the start of the session) and a group with a `NaN` also keeps its
  first `NaN` (a gap stays a gap). The number of points never exceeds 100000. The graph line is filled again when points were thinned out (a short pause of the window, once per about 25000 new samples). Limits: zooming into an
  old part shows only the kept points (every sample is in the CSV file), and the table is unchanged (newest `_DataLogView_MaxRows` (20000) rows). The new setting `_DataLogView_ThinOldPoints` (default 1) switches this off:
  0 = the old behavior (oldest points removed, scroll off the left side). The texts about this (the warning window for sessions with many samples, the status line after a live change of the duration or the interval and the
  notice of the Data Log window) follow the setting. Simulated outside the program (2 million samples with spikes and gaps): the lowest and highest value of the log are kept, the time stays in order, the left end stays at the
  start, never more than 100000 points. Change in `DataLogGUI.h` (new class `LogSeriesBuffer` with `LimitPoints`; `EnsureSeriesBuffers`, `SyncChartSeries`, `ShowPendingData`, `ClearChartData`, `ResetView`), `RMH_Application_ThermalViewer.h`
  (`_DataLogView_ThinOldPoints`) and `RMH_Application_ThermalViewer.cpp` (warning texts). Note: `RMH_Application_ThermalViewer.cpp` uses an older Windows text encoding; upload it as a file on GitHub (do not copy-paste it).

- **Change (branch `IRCAM-for-T2Splus_V2_2Dplots_expansion_27`, not tested yet, not compiled yet): the log file is read in blocks, so a very long log file opens without a memory spike and without the 2 GB limit.**
  The Data Log window read everything that was new in the file in one piece: one byte array of the whole new part (`(int)` of a 64 bit length: for more than 2 GB this is a wrong or negative size; an error was swallowed, so the
  window stayed empty or showed wrong data), converted to one text (2 bytes per character) and split into one string per line - several times the size of the file in memory, and the window frozen meanwhile. Now the file is read in blocks of
  `_DataLogView_ReadBlockBytes` (512 KB). **At most `_DataLogView_ReadTimeBudgetMs` (200 ms) are used per refresh**; when the file is not finished the window refreshes again after 50 ms (instead of 1000 ms) until it has caught up,
  so the program stays usable while a long file loads, and the status line shows **Reading the log file: NN %**. The table and the graph are drawn **once**, when the view has caught up (not again and again while reading). The values
  go to point lists (`LogSeriesBuffer`, see the entry above) and only the newest `_DataLogView_MaxRows` lines are kept for the table (their rows are made at the end), so a log with millions of lines does not create millions of
  table rows or chart points. A character that is split over two blocks (UTF-8) is kept together with a `Decoder` (this also fixes the same split between two refreshes of a running session). A running session looks the same as
  before. The clear-graph button and a restart of the view reset the new state. Change in `DataLogGUI.h` (`RefreshLogView`, `ProcessNewText`, new `ShowPendingData`, `SyncChartSeries`, `EnsureSeriesBuffers`, `UpdateStatusText`,
  `ClearViewButton_Click`, new members `SeriesBuffers`, `PendingLines`, `ViewDecoder`, `ReadBuffer`, `CharBuffer`, `LoadingBehind`, `LoadTargetLength`, `ViewHasNewData`) and `RMH_Application_ThermalViewer.h`
  (`_DataLogView_ReadBlockBytes`, `_DataLogView_ReadTimeBudgetMs`). The documentation file is now `README_IRCAM-for-T2Splus_V2_2Dplots_expansion_27.md` (the `_26` file is replaced by it).

- **New (branch `IRCAM-for-T2Splus_V2_2Dplots_expansion_26`, not tested yet, not compiled yet): crash recovery of unsaved data logging sessions at start-up.** A data logging session is written to a temporary file in
  `%TEMP%\IRCAMThermalViewer_DataLogging` while it runs, and the save / delete question is asked when it ends. After a crash, a power loss or a killed process that question was never asked and the file stayed
  in the temporary folder unnoticed. Now the program looks in that folder when it starts (after the main window is shown, **before** the automatic start-up sequence, so the window does not appear over the
  "please wait" window) and, when it finds session files, shows the **Unsaved Data Logging Sessions Found** window: a list (file name, last written, samples, logged time, size; newest first) and four buttons.
  **Save** moves the selected file(s) to the default save path (never overwrites: `_recovered`, `_recovered2`, ... is added to the name; the temporary file is only deleted when the copy has the same size; on an error
  the file is kept). **Open** shows the file in the Data Log window, opened as a separate window so the automatic start-up does not replace it; the file stays in the temporary folder, so it is offered again at the next
  start-up until it is saved or deleted. **Delete** asks first (default No) and deletes the selected file(s) permanently. **Decide Later** keeps everything. The window closes by itself when no file is left.
  Details: files **without a sample** (only the header line, or empty) are deleted without a question and a status line says so; the **data delimiter** of every file is read from its header line (it is not stored
  elsewhere), so a file written with another delimiter than the current setting is still shown correctly - the delimiter setting of the program is **not** changed by Open; a file that cannot be read is still
  listed (never deleted without a question). **A second instance of the program does not offer the session of the first one:** while a session runs a small lock file `<session file>.lock` is held open
  (`FileOptions::DeleteOnClose`); Windows removes it when the process ends, also after a crash, and a lock file left by a power loss is recognised as not held. It is released in
  `RMH_ThermalViewer_FinishDataLoggingSession` after the save / delete question. If the default save path is the temporary folder itself nothing is offered. The scan never stops the start-up (an error gives a status
  line). Data Log window: the notice about the session duration is not shown when the window is opened for a recovered file. Change in new `CrashRecoveryDialog.h` (`RecoveredSessionInfo`, `CrashRecoveryDialog`),
  `RMH_Application_ThermalViewer.cpp` / `.h` (`RMH_ThermalViewer_CheckForUnsavedDataLoggingSessions`, `RMH_ThermalViewer_GetDataLoggingTempFolder`, `RMH_ThermalViewer_AcquireDataLoggingSessionLock`,
  `RMH_ThermalViewer_ReleaseDataLoggingSessionLock`, `RMH_ThermalViewer_SessionFileIsInUse`, `RMH_ThermalViewer_AskToSaveDataLoggingSession` (the former body of
  `RMH_ThermalViewer_FinishDataLoggingSession`), the start code in `RMH_ThermalViewer_StartDataLogging`), `MainGUI.h` (`MainGUI_Shown`), `DataLogGUI.h` (`OpenRecoveredSession`, `GetViewDelimiter`,
  `SuppressNoticeOnce`) and `GlobalObjectsAndVariables.h` (`GlobalVariables::DataLoggingSessionLockStream`). Note: `RMH_Application_ThermalViewer.cpp` uses an older Windows text encoding; upload it as a file on
  GitHub (do not copy-paste it). The new `CrashRecoveryDialog.h` is UTF-8 with an SPDX header.

- **New (branch `IRCAM-for-T2Splus_V2_2Dplots_expansion_25`, not tested yet): the data logging sample interval can be changed while a session runs.** The new interval is used at once (a new `ValueChanged` event of
  the interval box): the logging thread reads the interval again in every 50 ms step of its wait, so the sample that is being waited for comes when the new interval is over (a shorter interval that is
  already over gives the sample at once). The session timer and the `Time(ms)` column now add the **time that was really waited** per sample (was: the interval; the same when it does not change), and the end of the
  session uses the last wait. The interval is rounded to a whole millisecond when it is read (0.29 s could become 289 ms) and is never below 40 ms. A change of the duration or the interval that makes the session
  have more samples than the Data Log window shows gives a status line (no window). Change in `RMH_Application_ThermalViewer.cpp` / `.h` (`RMH_ThermalViewer_UpdateDataLoggingSampleInterval`,
  `RMH_ThermalViewer_ReadSessionIntervalMilliSec`, `RMH_ThermalViewer_WarnLiveSampleLimit`, `RMH_ThermalViewer_DataLoggingThreadProcess`, `RMH_ThermalViewer_StartDataLogging`), `ThermalCameraGUI.h`
  (`DataLoggingIntervalUpDown_ValueChanged`) and `DataLogGUI.h` (notice text).

- **Change (branch `IRCAM-for-T2Splus_V2_2Dplots_expansion_24`, not tested yet): live data logging settings extended; this replaces the behavior described in the entry below for the plot data sets and for a too short duration.**
  **Duration:** a new duration that is not longer than the time already logged is **no longer accepted**: the session is **not** ended, a warning window explains it, the Hours / Minutes / Seconds boxes are set back to
  the duration of the running session and the session keeps running (a longer duration is still used at once, counted from the start of the session). **Plot data sets:** a data set that is **enabled during a session and was not
  part of it is now added as a new column**, logged from the next sample on (a note line `#<column>=<name>` is written in the file; the rows before it are shorter) and **the new line and table column appear in the Data Log
  window** from that moment. Unchecking a data set that is part of the session still logs `NaN` in its column (gap in the graph), and a changed source is still used from the next sample (note line renames the column).
  **Line color and line width** changed in the plot data set settings during a session now also change the **line in the Data Log graph** (and its check box color); the lines of the Data Log graph use the color and line width of
  their plot data set (before: a fixed color list by column number, line width 1; the same with the default settings). The logging thread reads the number of columns once per sample. Change in
  `RMH_Application_ThermalViewer.cpp` (`RMH_ThermalViewer_UpdateDataLoggingSessionDuration`, `RMH_ThermalViewer_ApplyLiveDataLoggingDataSetChange`, `RMH_ThermalViewer_DataLoggingThreadProcess`,
  `RMH_ThermalViewer_Change2DPlotDataSetAndSettingsPanelColor`, `RMH_ThermalViewer_Change2DPlotDataSetLineWidth`, `RMH_ThermalViewer_StartDataLogging`), `GlobalObjectsAndVariables.h/.cpp`
  (`DataLoggingStyleChangedFlag`, `Plot2DDataSetLineWidthUpDowns`), `ThermalCameraGUI.h` (new array of the line width boxes) and `DataLogGUI.h` (`AddLogColumn`, `GetColumnColor`, `GetColumnLineWidth`,
  `ApplyLiveStyleChanges`, `ApplySourceChangeNote`; the creation of the columns of the header line moved into `AddLogColumn`).

- **New: the data logging duration and the Temperature Plot Data Set Settings can be changed while a session runs** (branch `IRCAM-for-T2Splus_V2_2Dplots_expansion_23`, not tested yet).
  Before, these settings were read once when **Start Logging** was pressed. **Duration:** a new Hours / Minutes / Seconds value is used at once and is the total length of the session,
  counted from the start (1 minute changed to 1 hour after 20 seconds keeps logging until 1 hour after the start); a value shorter than the time already logged ends the session with the
  next sample; a status line tells the change. The duration is now calculated in `unsigned long` (the hours box goes up to 719 and the milliseconds overflowed a signed 32 bit integer).
  The sample interval is still read at the start. **Plot data sets:** a changed **source** of a data set that is part of the session is logged from the next sample on (the last value of the
  old source is not reused), a `#<column>=<name>` note line is written in the file and the Data Log window renames the table header, graph legend and line check box; an **unchecked** data
  set keeps its column and logs `NaN` (a gap in the graph) until it is checked again; a data set that was not part of the session at the start is not added (status message, logged from the
  next session) because the columns of the session file are fixed. The notice of the Data Log window about the duration is updated. Change in `RMH_Application_ThermalViewer.cpp` / `.h`
  (`RMH_ThermalViewer_ReadSessionDurationMilliSec`, `RMH_ThermalViewer_UpdateDataLoggingSessionDuration`, `RMH_ThermalViewer_ApplyLiveDataLoggingDataSetChange`,
  `RMH_ThermalViewer_StartDataLogging`, `RMH_ThermalViewer_DataLoggingThreadProcess`), `ThermalCameraGUI.h` (`DataLoggingDurationUpDown_ValueChanged` and the data set callbacks),
  `GlobalObjectsAndVariables.h/.cpp` (`DataLoggingColumnDataSetIndex`, `DataLoggingColumnEnabled`, `DataLoggingColumnSourceChangedFlag`, `DataLoggingColumnSourceNames`) and `DataLogGUI.h`
  (`ApplySourceChangeNote`, `ProcessNewText`).

- **Bug fix: the Data Log graph jumped horizontally (the right edge moved) while logging, with X-Axis Number Of Ticks on a fixed number and also on Auto.** The graph used the
  automatic layout of the chart, which makes the plot area (the part inside the axes) smaller or larger whenever the width of an axis label changes, for example a time label on the
  right end of the time axis that comes and goes while the log grows. The round tick steps (next entry) were not enough. Now the plot area has a **fixed size**, set in pixels from each side
  of the graph (left 75, right 40, top 12, bottom 55, plus about 30 at the top while the graph title is shown) and converted to percentages for the chart, so a label can no longer move
  it. It is only written when it changes (window resize, graph title shown / hidden), not at every refresh. **While the graph is zoomed the automatic layout is used** (the zoom scroll
  bars need room; the plot area does not change by itself then). The margins are constants at the top of `UpdatePlotAreaLayout`: increase one if an axis title or label is cut off.
  Reported as working; not tested on every window size. Change in `DataLogGUI.h` (new `UpdatePlotAreaLayout`, `LogChart_SizeChanged`; called from `ApplyAxisSettings`,
  `UpdateGraphTitleVisibility` and `BuildChart`).

- **Bug fix: with X-Axis Number Of Ticks other than Auto, the Data Log graph changed its size and the last time value appeared and disappeared at random moments.** The tick interval was
  the range divided by the number of ticks (a different odd value at every refresh), written to the axis together with the range, so the chart laid the axis out again every second.
  Now the interval is a round time step (1, 2, 3, 5, 10, 15, 20, 30 s, the same in minutes, 1, 2, 3, 6, 12 h, whole days; milliseconds for a deep zoom), the smallest one that gives at
  most N intervals over the range (locked X range, zoomed view, or the log from 0:00:00 to the newest point). It is only written to the axis when it changes, and the time axis range is
  left automatic. **The number of ticks is now a maximum** (the result is between about N / 2 and N, at round times), and the ticks are at round times. **Auto**, the zoom, a locked X
  range and the Y ticks are unchanged. This bug was in earlier versions. Change in `DataLogGUI.h` (`ApplyAxisSettings`, new `GetRoundTickSeconds`, `GetDataEndTime`, `XIntervalSeconds`).

- Data Log window: **new check box Show Graph Title**, before **Auto-scroll table to newest**, **not ticked** by default. Ticked shows the title above the graph and in the
  exported image (`Title` until a title is typed); not ticked hides it (the text is kept). **The title is edited directly: double-click it on the graph**, type, **Enter** (or click
  elsewhere) keeps, **Esc** cancels; a new title ticks the check box, an empty text hides the title (unticks it, text back to `Title`). **Export Graph** opens the Graph Title window also
  when the title is hidden (new title shows it, empty text hides it, same text keeps it as it is). **The right-click menu item Set Graph Title... is removed.** The separate Data Log
  window is 1130 px wide (was 1000) and Auto-scroll moved right. Change in `DataLogGUI.h` (`ShowTitleCheckBox`, `UpdateGraphTitleVisibility`, `ApplyGraphTitleText`,
  `LogChart_MouseDoubleClick`, `BeginTitleEdit`, `EndTitleEdit`, `TitleEditBox`, `ShowGraphTitleWindow`, `BuildLayout`, `BuildChart`).

- Data Log window: the factor of the bigger text in the exported image is now fixed (1.2, constant `TextScale` in `ExportGraphButton_Click`); the macro `_DataLogExport_TextScale`
  is removed. Change in `DataLogGUI.h`.

- Data Log window: **higher resolution of the exported image**. The image has 3 times more pixels in width and height. The graph is taken as a vector drawing (EMF+) and
  drawn at the high resolution (falls back to the stretched picture when that fails); the strip and the table are drawn at the high resolution too. JPEG images are saved
  with quality 95. Macros at the top of `DataLogGUI.h`: `_DataLogExport_ResolutionScale` (3), `_DataLogExport_UseVectorChart` (1; 0 = picture of the screen size),
  `_DataLogExport_JpegQuality` (95). Change in `DataLogGUI.h` (`ExportGraphButton_Click`).

- Data Log window: **Add Notes button and notes in the exported image**. New **Add Notes** button between **Reset Zoom** and **Export Graph** opens the **Graph Notes**
  window (multi-line text, up to 2000 characters, **Set Notes** / **Cancel**, empty text removes the notes). **Export Graph** now asks for the notes after the graph
  title, with the same behavior as the title (shows the notes that are set now to keep, edit or empty them; **Cancel** cancels the export). The notes are written
  in the exported image in a **Notes** cell on the right of **Other Information**, wrapped to the width of the cell; without notes there is no Notes cell. Like the
  title, the notes stay until changed (not saved with the log file or between program runs). To make room, **Export Graph** is narrower (115 px), **Auto-scroll
  table to newest** moved right and the separate Data Log window is 1000 px wide (was 900). Change in `DataLogGUI.h` (`AddNotesButton_Click`,
  `ShowGraphNotesWindow`, `GraphNotes`, `BuildLayout`, `ExportGraphButton_Click`).

- Data Log window: **the information below the exported graph is now a table**. Cells from left to right: **Thermal Camera Configuration** (as before),
  **Export Information** (Date, Time, Logged Time, Session Duration Setting, Sample Interval, Samples In Data Log), **Other Information** (Log File, Camera Model,
  Camera Temperature Range, Auto Calibration, Drift-Based Calibration, Temperature Unit) and **Notes**. Each cell has a heading band and borders, label on the left
  and value next to it, and the cells in a row have the same height. Cells that do not fit the width of a narrow image go to a next row (notes: at least 220 px or a
  full row). The Log File line shows `none` when there is no session file. Change in `DataLogGUI.h` (`ExportGraphButton_Click`, `GetExportInfoLines` now fills
  two lists, new `SplitInfoLine`).

- Data Log window: **text in the exported image is 20 % bigger than the axis values**. All text apart from the values on the axes (line names, first / last axis
  values, table, title of the graph, titles of the axes) has the font of the axis values (for example `20.0`, `0:00:00`, read from the graph itself) at 1.2 times the
  size; the axis values keep their size. The title and axis titles on the screen are only made bigger while the image is made.
  Change in `DataLogGUI.h` (`ExportGraphButton_Click`).

- Data Log window graph: **thermal camera configuration list in an exported image**. **Export Graph** now adds a list **below the graph** with the heading
  **Thermal Camera Configuration:** and one line per setting of the Thermal Camera Configuration panel (Settings): **Temperature Correction**,
  **Ambient Temperature**, **Reflected Temperature**, **Surrounding Humidity** (%), **Object Surface Emissivity** and **Surface Distance** (m), for example
  `Ambient Temperature: 23.0 °C`. The temperatures are in the temperature unit of the program, like the panel. The values are the ones **in use by the
  camera at the moment of the export** (the ones set with the Set button of the panel, not text typed in the panel and not yet set), not the ones at the time
  the data was logged. The image gets taller by the height of the list; the graph and the strip above it are not changed. Change in `DataLogGUI.h`
  (`ExportGraphButton_Click`, new `GetCameraConfigLines`).

- **Bug fix: Ambient and Reflected temperature swapped when the temperature unit is changed.** In the Thermal Camera Configuration panel, changing the temperature
  unit (°C / °F / K) wrote the Reflected temperature into the Ambient box and the Ambient temperature into the Reflected box. It was only visible when the two
  values differ (the defaults are equal), and pressing **Set** afterwards would have written the swapped values to the camera. The two boxes now get their own
  values (the same assignment that the read-from-camera and default-recover routines already used). Change in `RMH_Application_ThermalViewer.cpp` (the
  temperature unit change routine, "Update the parameter values of the camera configuration NumericUpDowns"). Note: this file uses an older Windows text
  encoding; upload it as a file on GitHub (do not copy-paste it).

- Data Log window: **a new data logging session starts with the zoom reset**. When the window switches to the file of a new session (Start Logging, hotkey, plot
  menu, alarm or trigger event), both axes go back to the whole view and the border of the Reset Zoom button returns to normal. Before, a zoomed view of the
  previous session stayed, and a time range that the new session had not reached yet showed an empty graph. A locked X / Y range is not a zoom and stays.
  Change in `DataLogGUI.h` (`ResetView`).

- Data Log window graph: **"Export Information" list next to the camera configuration list in an exported image**. On the right of the **Thermal Camera
  Configuration:** list the image now has a second list, **Export Information:**, for later reference: **Date** and **Time** of the export, **Camera Model**
  (for example `InfiRay T2S+ V2`), **Camera Temperature Range** (Low / High), **Auto Calibration** (Off, or On with its period in seconds),
  **Drift-Based Calibration** (Off, or On with its threshold), **Log File** (name of the session file, when there is one), **Sample Interval**, **Session
  Duration Setting**, **Samples In Data Log**, **Logged Time** (elapsed time of the last sample in the graph) and **Temperature Unit**. When the image is too
  narrow for two columns, the second list is written below the first. All values are the ones at the moment of the export (settings are not stored in the log
  file). Change in `DataLogGUI.h` (`ExportGraphButton_Click`, new `GetExportInfoLines`, new include of `RMH_SupportedIRCameras_Resources.h` for the model names).

- Data Log window graph: new macro `_DataLogScrollBar_ApplyColors` at the top of `DataLogGUI.h`. 1 (default) = the lighter gray scroll bar colors below,
  0 = the scroll bars keep the default colors of the chart control. Only meant to compare the two when checking the scroll bar behavior while logging.

- Data Log window graph: **scroll bars, arrows and Reset Zoom buttons are lighter gray** on both axes (time axis and temperature axis), so they are easier to
  see when the graph is zoomed. Track: dark gray (70 / 70 / 70), buttons - arrows, Reset Zoom button and the sliding part - light gray (200 / 200 / 200), symbols
  on the buttons dark (40 / 40 / 40). The three colors are the macros `_DataLogScrollBar_TrackColor`, `_DataLogScrollBar_ButtonColor` and
  `_DataLogScrollBar_LineColor` at the top of `DataLogGUI.h`. Change in `DataLogGUI.h` (`BuildChart`, new `StyleAxisScrollBar`). Note: the Reset Zoom button
  meant here is the small button at the end of the scroll bar, not the **Reset Zoom** button above the graph.

- Data Log window: the **Clear View** button is now called **Clear Graph**. Its question window has the title "Clear Graph" and asks **"Are you sure
  you want to clear the graph and the table?"** (the button still empties both the table and the graph, as before; only data logged from now on is
  shown, the file is not changed). Change in `DataLogGUI.h` (`BuildLayout`, `ClearViewButton_Click`). Older entries in this file still use the old name.

- Data Log window: the **border of the Reset Zoom button turns orange** (dark orange, RGB 255 / 140 / 0) while the graph is zoomed in (time axis and / or
  temperature axis) and goes back to the normal gray border of the other buttons when the zoom is reset (Reset Zoom button, or when a range is
  locked / unlocked). Only the border color changes. Change in `DataLogGUI.h` (new `UpdateZoomIndicator`, called from `LogChart_AxisViewChanged`,
  `ResetZoomButton_Click` and the 1 s refresh timer as a safety net).

- Data Log window graph: **frame around the graph**. The plot area now always has a border on all four sides, like **Show Plot Border Box** in the
  2D plot of the Plots window (there is no right-click item to show / hide it: it is always shown). Before, only the left (Y) and bottom (X) axis
  lines were drawn. The top and right sides are the secondary axes with only their line switched on (same gray as the axis lines; no labels, tick
  marks or grid lines), so the layout of the graph does not change. The frame is part of an image saved with **Export Graph**. Change in `DataLogGUI.h`
  (`BuildChart`).

- Data Log window graph: **Export Graph always shows the last (maximum) value of the time axis and of the temperature axis**. Before, the labels on
  the axes only show the tick marks, so when the end of an axis did not fall on a tick (zoomed view, locked range, number of ticks, automatic range) the
  maximum duration and the maximum temperature could not be read in the saved image. Now the strip above the graph (where the names of the lines are)
  gets a line with the first and last value of both axes as shown at the moment of the export, for example
  `Time axis: 0:00:00 to 1:02:37 (h:mm:ss)      Temperature axis: 20.5 to 134.2 °C`. The line is split in two rows when the graph is too narrow for one
  row. The values follow what the graph shows: zoom, locked X / Y range and automatic range. Only a **zoomed** time range shorter than one minute is
  written with milliseconds (`h:mm:ss.fff`); in every other case (not zoomed, locked range, 60 s or more) the time is `h:mm:ss`; the temperature has one to three decimals. The axes themselves and the image size of the graph are not changed.
  Change in `DataLogGUI.h` (`ExportGraphButton_Click`, new `GetAxisRangeTexts` and `FormatAxisTime`). Note: the values are the ends of the axes (the
  visible range), not the highest sample of a line; with an automatic range the axis end can be a little above the highest sample.

- Spikes / dips in the **2D plot and in the data logging file during a camera shutter calibration** are removed. Cause: while the shutter
  calibration runs (shutter closed, non-uniformity map and temperature look-up table regenerated, shutter reopened) the video thread kept
  processing frames, so the plot and the data logging thread picked up the values of the closed shutter and of the first frames after it
  reopened (maximum, minimum, center, points, lines and ROI values collapsing towards the shutter temperature). Now the **last good value of every
  data set is held (frozen)** for the whole calibration plus a settle time of 300 ms. This applies to the 2D plot feed (all 10 data sets) and to the
  data logging thread (the CSV file), and to every calibration: periodic (auto) calibration, temperature-drift-based calibration, the Calibrate
  Camera button and the calibration after a temperature range switch. The frozen samples are written as a repeated last value; the CSV format is
  not changed (no extra column, so the Data Log window and the MATLAB examples keep working). The settle time is `_CalibrationFreezeSettleTimeMs`
  in `RMH_SupportedIRCameras_Resources.h`. Changes in `GlobalObjectsAndVariables.h/.cpp` (`CalibrationActiveFlag`, `CalibrationSettleUntilTick`,
  `RMH_IsCalibrationFreezeActive`), `RMH_ThermalCameraSupport_Library.cpp` (`RMH_IRThermalCamera_CalibrateThermalCamera` sets and clears the freeze),
  `RMH_Application_ThermalViewer.cpp` (new `RMH_FreezeValueDuringCalibration`, used in `RMH_ThermalViewer_ImageProcessingSequence` and
  `RMH_ThermalViewer_DataLoggingThreadProcess`). Not changed: the calibration code and the video thread still use the same frame buffers without a
  lock, so a single outlier frame right after a calibration is still possible in theory (see README section 7).

- Closing the application while **data logging is running** now asks first. A window "Data Logging Is Running" says that the session is stopped
  if the application is closed and asks **"Are you sure you want to close the application?"** (the default button is **No**).
  - **No**: the application stays open and the logging continues.
  - **Yes**: the session is stopped, the program waits (up to 3 seconds) for the logging thread to finish, and the usual question "Do you want
    to save this data logging session?" is asked. **Yes** there saves the file to the default save path, **No** deletes it. After this the
    application closes.
  - When Windows itself is shutting down no question is asked; the session is only stopped.
  Change in `MainGUI.h` (`MainGUI_FormClosing`).

- Temporary data log file when the application is closed during logging. Before, the file `DataLogSession_*.txt` stayed in
  `%TEMP%\IRCAMThermalViewer_DataLogging` when the application was closed while logging ran, because the save / delete question was never asked
  (the question is asked when the logging thread completes, and the application had already closed). With the change above the question is asked
  before closing, so the file is saved or deleted. A file can still stay behind in that folder only after a crash, a power loss or when the process is
  killed; it holds the samples written so far and can be opened or deleted by hand. Change in `MainGUI.h` (`MainGUI_FormClosing`).

- Data Log window: the **Start Logging** and **Stop Logging** buttons are exactly like the other buttons again (same background, same text color and
  font). Only the **border** keeps its color: **dark green** for Start Logging and **dark red** for Stop Logging. Change in `DataLogGUI.h`
  (`SetButtonTextColor` is replaced by `SetButtonBorderColor`).

- Data Log window graph: the **scroll bar of the zoomed time (X) axis** works now. After zooming in, the arrows of the bar jumped straight to the
  beginning or the end of the log, and dragging the bar did not move the view. Cause: the time axis is a date / time axis (values in days) and the
  scroll steps had their default size of one whole unit, which is far larger than a zoomed range (the bar snapped to the ends). The scroll steps
  are now set in milliseconds: the smallest step is 1 ms and an arrow moves the view by a tenth of the visible range (updated at every zoom or
  scroll). The scroll bar of the temperature (Y) axis had the same problem (default step of 1 unit) and got a fine step (smallest 0.001, an arrow
  moves a tenth of the visible range). While the graph is zoomed the X-axis tick setting no longer recalculates the axis scale, so it does not
  interfere with scrolling. Change in `DataLogGUI.h` (`BuildChart`, new `UpdateScrollSteps`, `LogChart_AxisViewChanged`, `ApplyAxisSettings`).

- Data Log window: the **Start Logging** and **Stop Logging** buttons have their original dark gray background and hover colours back (the light gray
  background of the previous change is removed). The dark green / dark red text and border colours stay (the text stays bold). Change in
  `DataLogGUI.h` (`SetButtonTextColor`).

- Data Log window: the text of the **Start Logging** button is now **dark green** and the text of the **Stop Logging** button is **dark red**
  (both bold), with a border in the text color. (A light gray background that was added with this change was removed again, see above.) Change in `DataLogGUI.h` (new routine `SetButtonTextColor`,
  used in `BuildLayout`).

- Left menu: the **Data Log** text no longer overlaps its image. Cause: the column widths of the Data Log item were fixed values written
  in code (100 % + 60 px), while the other items (Settings, Stream, 3D Surface, Plots, Emissivity) are scaled by the main GUI, so on this
  screen the Data Log button was too narrow, the text wrapped into two lines ("Data" / "Log") and the first line was drawn over the image.
  The column widths of the Data Log item are now copied from the Plots item as they are at run time, the button takes over the padding and
  text / image relation of the Plots button, and a non-breaking space is used in the text so it can never wrap. "Data Log" now shows under
  its image like the other items. Change in `MainGUI.h` (`AddDataLogMenuItem`).

- Data Log window: **Export Graph** now first opens the same "Graph Title" window as **Set Graph Title...** (it is one shared routine,
  `ShowGraphTitleWindow`). The window shows the title that is set now, so it can be kept as it is, edited or replaced (an empty text removes
  the title). **Set Title** applies the title and continues with the save window; **Cancel** cancels the export. The title that is entered stays
  set for the graph, like with the menu item. Change in `DataLogGUI.h` (`ExportGraphButton_Click`, `GraphTitleMenuItem_Click`).

- Data Log window graph: graph title. A new right-click menu item **Set Graph Title...** (below "Temperature At The Cursor") opens a window
  to type a title (up to 200 characters). The title is shown in bold at the top center of the graph, above the plot area, and is part of an image saved
  with **Export Graph**, so the image shows what the graph is about. An empty text removes the title. The title is kept when the view is cleared or
  a new session starts, but it is not saved: it is empty again when the program is restarted. Change in `DataLogGUI.h` (`BuildChart`,
  `GraphTitleMenuItem_Click`).

- Data Log window graph: X-axis (time) range items in the right-click menu, the same as for the Y axis. Four new items, in their own block
  above the Y-axis block:
  - **X-Axis Range: Auto** (status line; **X-Axis Range: Locked h:mm:ss To h:mm:ss** when locked),
  - **Set X-Axis Range And Lock...** - a window for a start and an end time (elapsed time of the log session, h:mm:ss; m:ss or seconds also
    work, decimals allowed, for example 0:05:30 or 1:00:00.5). An invalid text shows a warning and the window opens again; an end that is not
    later than the start is corrected (start + 1 s),
  - **Lock X-Axis At Current Range** - locks the time range that is shown now (the zoomed range, or the whole range), rounded outwards to 0.1 s.
    Enabled while the axis is not locked,
  - **Unlock X-Axis (Auto Range)** - the time axis follows the data again. Enabled while locked.
  While the X axis is locked, the graph shows only that time range (new data outside it is still collected, but not shown). Drag-zooming
  and the X-axis number of ticks work inside the locked range. The lock is not saved (it resets when the program is restarted).
  Change in `DataLogGUI.h` (`XSetRangeMenuItem_Click`, `XLockCurrentMenuItem_Click`, `XUnlockMenuItem_Click`, `GetXViewRange`,
  `FormatRangeTime`, `ParseRangeTime`, `ApplyAxisSettings`).

- Data Log window: **Export Graph** button, next to Start Logging / Stop Logging / Show Log File / Clear View / Reset Zoom. It saves the graph
  as an image: a save window (PNG, JPEG or BMP; the format follows the file extension, PNG by default) opens with the session file name + `_graph`
  as the file name. The image shows the graph as it is shown now (zoom, ticks, locked Y range) at the size of the graph on the screen, without
  the hidden lines, and has a strip above the graph with the names of the lines that are shown, in the color of the lines (the graph has
  no legend of its own since the line check boxes were added). The mouse cursor temperature label is not part of the image. A message is shown
  when there is no graph yet or when saving fails. The buttons of the top row were made a little narrower to make room (the window
  layout is otherwise unchanged). Change in `DataLogGUI.h` (`ExportGraphButton_Click`, `BuildLayout`).

- Data Log window graph: check box per graph line. Each line of the graph now has a check box above the graph (below the Start Logging /
  Stop Logging / Show Log File / Clear View / Reset Zoom buttons), with the name of the line in the color of the line. Un-tick a box to
  hide that line from the graph, tick it to show it again. All lines are shown (ticked) when a log is opened or a new session starts.
  This replaces the legend of the graph (the boxes name the lines, the legend is removed). A hidden line is only hidden: its data keeps
  being collected and is still in the table and in the log file. Hidden lines are ignored by the automatic Y-axis range and by the
  Y-axis tick / range settings, so the Y axis fits the lines that are shown. Changes in `DataLogGUI.h` (`LineChecksPanel`,
  `LineCheck_CheckedChanged`, `ProcessNewText`, `ResetView`, `GetDataRange`).

- Data Log window graph: right-click menu item **Lock Y-Axis At Current Range** (like in the Plots window), between "Set Y-Axis Range
  And Lock..." and "Unlock Y-Axis (Auto Range)". It locks the temperature axis at the range that is shown now (the zoomed range, or the
  whole range), rounded outwards to one decimal. It is enabled while the axis is not locked. The "Set Y-Axis Range And Lock..." window
  uses the same shown range as its start values. Change in `DataLogGUI.h` (`YLockCurrentMenuItem_Click`, `GetYViewRange`).

- Data Log window graph: right-click menu. The left click that switched the mouse cursor temperature on or off is removed (a left click
  does nothing now; dragging with the left button still zooms). A right click in the graph opens a menu with the same items as the
  right-click menu of the Plots window, plus the cursor temperature:
  - **Temperature At The Cursor** (check item): shows / hides the label with the temperature and the elapsed time at the mouse cursor.
    The label also hides while the mouse is outside the graph.
  - **X-Axis Number Of Ticks** (Auto, 5, 10, 20, 25, 50) and **Y-Axis Number Of Ticks** (Auto, 5, 10, 15, 20, 25, 30). The tick spacing is
    the visible range divided by the number; it follows zooming, Reset Zoom and new data. With Y ticks set and the range not locked,
    the Y axis fits the data (small margin, rounded to 0.1). Auto (default) leaves the scaling of the chart as before.
  - **Y-Axis Range: Auto / Locked (min To max)** (status line), **Set Y-Axis Range And Lock...** (window for a minimum and maximum)
    and **Unlock Y-Axis (Auto Range)**.
  The lock and the tick settings are not saved (they reset when the program is restarted). Change in `DataLogGUI.h`
  (`BuildChartContextMenu`, `ApplyAxisSettings`).

- Data Log window graph zoom: dragging a rectangle with the left mouse button now zooms both the time (X) and the temperature (Y) axis
  to the area of the rectangle. Before, only the temperature axis zoomed: the time axis is a date / time axis, and the selection
  snapped to a large time unit. The selection cursors of both axes are now set to a fine step (X: 1 ms, Y: 0.001) and the smallest
  zoomed view is also small (X: 1 ms, Y: 0.001), so the rectangle is used as drawn. Reset Zoom is unchanged. A click without
  dragging no longer does anything (the cursor temperature label is in the right-click menu now). Change in `DataLogGUI.h` (`BuildChart`).

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
  (zoom) is not treated as a click. Change in `DataLogGUI.h`. **Replaced in the entry "Data Log window graph: right-click menu" above: the left click no longer does this.**

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
