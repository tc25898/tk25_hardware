# Waveshare pan-tilt firmware with Host Wi-Fi provisioning

Product-specific ESP32 firmware for the Waveshare 2-Axis Pan-Tilt Camera Module.
The firmware directory is imported from Waveshare's official `PT_esp32.zip`,
which contains `pan_tilt_base_v0.9`, with the Host Wi-Fi provisioning feature
from [PR #1](https://github.com/tc25898/tk25_hardware/pull/1) reapplied on top.

See [vendor source and import procedure](docs/vendor-source.md) for provenance,
the archive checksum and the per-file baseline manifest. Repository instructions
in `AGENTS.md` are project-maintained, not part of the vendor archive.

Open `pan_tilt_base_v0.9/pan_tilt_base_v0.9.ino` in Arduino IDE to build the
firmware. Use the [official product guide](https://www.waveshare.com/wiki/2-Axis_Pan-Tilt_Camera_Module)
for assembly, board selection and servo configuration.

## Project extension

The ESP webpage sends Host Wi-Fi credentials over UART and displays successive
Host result messages. It uses `T:-1001`, a flat request and a string `result`
reply; the Host performs saving/application. ESP does not reboot the Host or
restart its own network. See [protocol and setup](docs/host-wifi.md).

## Migration and behavior

See [migration scope and validation](docs/migration.md). This baseline retains
the official defaults: `moduleType = 0`, and `pan`/`tilt` plus IMU/voltage fields
in gimbal feedback when gimbal mode is selected. The previous community
high-frequency control changes, forced feedback and `X`/`Y`-only telemetry are
not included. Host code relying on those community behaviors must be reviewed
before deploying this version. Existing board filesystem settings and boot
missions are not replaced by copying this repository.

This repository contains ESP32 firmware, not a Linux disk image. Compilation
does not validate board startup, calibration or actuator behavior. No flashing
or hardware operation is part of this migration.
