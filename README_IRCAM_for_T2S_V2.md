# IRCAM for T2S+ V2

A modified version of **OpenRMH-IRCAM** (based on v3.0.0-community.5) for the **InfiRay T2S+ V2 / T2S+_A2** thermal camera.
Personal reference notes, written 5 November 2026. Not an official release.

Branch: `IRCAM-for-T2S-V2` (fork: https://github.com/dany531/OpenRMH-IRCAM). Original project: https://github.com/Cyclotronic/OpenRMH-IRCAM

---

## 1. What this version does

### Correct temperatures for the T2S+ V2
The original IRCAM read this camera far too hot (for example 34 °C on a tape that was really about 21.8 °C). Two bugs in IRCAM caused it:

1. **The camera's detector (FPA) value was overwritten.** IRCAM wrote the max/min/center data over the first words of the
   metadata row, which destroyed the raw detector temperature (this gave the 246 °C "camera detector" reading). The value is
   now saved before it is overwritten.
2. **The flat-field correction used the wrong gray level.** IRCAM put a pixel that sees the shutter at the mean of the baseline
   frame; the manufacturer's software (and the camera's calibration data) use a fixed level of **6000**. The correction map is
   now re-referenced to 6000 for this camera type.

Other changes, aligned with the manufacturer's software: FPA divisor 37.682, 256-pixel gray offset 170, shutter temperature
from the register plus its signed fine-correction byte, calibration floats read low word first, distance term limited to 20.
All of this applies to this camera type only ("Pool 3"); other cameras are unaffected.

The older workaround (forcing `temp_fpa` and `temp_shutter` to 18 °C, "Build #7", commit `7b355e8`) only hid these bugs.

### Automatic start-up
After the main window opens, the program does the following without any clicks:

1. Shows a **"Custom Version For The InfiRay T2S+ V2 - please wait"** window while the camera initialises.
2. Selects **InfiRay T2S+ V2** in the camera list and connects (like the Connect button, without the TOPDON information dialog).
3. About 4 seconds later, **calibrates** the camera (like the Calibrate button).
4. Sets **Auto Calibration Period [Sec] to 180** and enables **periodic shutter calibration**
   (status message: "Automatic Shutter Calibration Is Enabled").
5. Closes the wait window and shows a **READY TO USE** window. Click OK and start measuring.

If the camera is **not found**, the wait window closes and a **"Camera Not Connected"** window explains what to do:
close the application, connect the camera to a USB port, wait a few seconds, open the application again.
(There is no automatic retry. Plug the camera in before starting the program.)

---

## 2. How to use it

1. Plug in the camera, then start IRCAM.
2. Wait for the READY TO USE window. Let the camera warm up for about **10 minutes** before trusting readings,
   because this camera drifts a little while it heats up. The 180 s periodic calibration helps.
3. For shiny parts (solder, bare metal, chip lids) put a dot of **matte black paint or Kapton tape** on the spot you measure;
   shiny surfaces read much too low.
4. Small constant errors can be corrected with IRCAM's **Temperature correction** setting (no rebuild needed).

---

## 3. Files changed (all in `src/`)

| File | What changed |
|---|---|
| `RMH_ThermalCameraSupport_Library.cpp` | Temperature fixes (detector value saved, 6000 flat-field level, offsets, shutter handling, calibration floats) |
| `RMH_ThermalCameraSupport_Library.h` | New data members (`FrameFPARawWord`, `CalValue6`) |
| `RMH_Application_ThermalViewer.cpp` | Automatic start-up routines (select camera, connect, calibrate, enable periodic calibration) |
| `RMH_Application_ThermalViewer.h` | Start-up settings and declarations |
| `MainGUI.h` | Start-up timer and the three information windows |
| `CHANGELOG.md` | Description of the changes |

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
| `_AutoStart_CalibrateDelayMs` | 4000 | Wait after connecting before the first calibration (raise to ~6000 if the first calibration looks off) |
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
**Actions -> build -> Run workflow -> choose `IRCAM-for-T2S-V2` -> Run workflow**, then download the build from the run page.
If a build fails, the red error text in the log shows the cause.

Keep these as fallbacks: Build #7 (commit `7b355e8`, branch `fix-t2s-v2-temperature`) and branch `t2s-v2-calibration-diagnostic`.

---

## 6. Test results (informal)

| Test | Result |
|---|---|
| Tape on a thermometer | Thermometer 21.8 °C, InfiRay app 21.0 °C, this version very close |
| Hot tea | Windows software 59 °C, IRCAM about 56 °C |
| Hot water, 80-82 °C | IRCAM about 1-3 °C below the Windows software |
| Soldering iron, up to 150 °C | IRCAM about 5 °C higher, but the iron was still heating and programs were switched between readings, so not conclusive |

---

## 7. Known limits

- Good enough for PCB and hardware checks (roughly +/- 2-3 °C up to about 100 °C). Not for work needing better than about 1 °C.
- Above about 120 °C (outside the low range) is **untested**; the high range is untested for this camera.
- The manufacturer's software also applies a per-pixel gain map read from the camera. It is not copied, so a small scale
  difference at higher temperatures is possible.
- "Camera core" shows the same value as "Camera shutter" (the V2 has no separate core sensor).
- The distance for this camera type may show 2 m instead of 1 m; the effect is small (about 0.4 °C at 60 °C).
- Recordings made with the old code lost the detector value. `tests/tools/ircam_raw.py` still has the old logic for this camera.
- The start-up windows and sequence have not been tested on every system; if something misbehaves, set
  `_AutoStart_Enabled` to 0 and rebuild.

---

## 8. Sources used

The InfiRay Android APK (XthermNew 6.6.230906), the analysis of the InfiRay Windows viewer, IR-Py-Thermal, and
`dzaczek/t2s-thermal-macos` (which note that the V2 shutter sensor drifts).
