# Waveshare pan-tilt firmware

Product-specific ESP32 firmware for the Waveshare 2-Axis Pan-Tilt Camera Module.
The firmware directory is imported from Waveshare's official `PT_esp32.zip`,
which contains `pan_tilt_base_v0.9`.

See [vendor source and import procedure](docs/vendor-source.md) for provenance,
the archive checksum and the per-file baseline manifest. Repository instructions
in `AGENTS.md` are project-maintained, not part of the vendor archive.

Open `pan_tilt_base_v0.9/pan_tilt_base_v0.9.ino` in Arduino IDE to build the
firmware. Use the [official product guide](https://www.waveshare.com/wiki/2-Axis_Pan-Tilt_Camera_Module)
for assembly, board selection and servo configuration.

This repository contains ESP32 firmware, not a Linux disk image. Compilation
does not validate board startup, calibration or actuator behavior.
