# Official vendor baseline

The source baseline is the product-specific archive linked as the lower-controller
demo by [Waveshare's product wiki](https://www.waveshare.net/wiki/2-Axis_Pan-Tilt_Camera_Module).
It is independent of the related `waveshareteam/ugv_base_general` GitHub project.

- File page: <https://www.waveshare.net/wiki/文件:PT_esp32.zip>
- Download: <https://www.waveshare.net/w/upload/f/f9/PT_esp32.zip>
- Directory inside archive: `pan_tilt_base_v0.9/`
- Archive SHA-256: `bcd6f0472f3ea43931a716781a8c23b4a7688c0dc06511eefbc15a8aef395aad`
- Archive size: 62159 bytes; 26 source/data files, 257349 uncompressed bytes.
- File-page upload date: 2024-04-17. The archive itself has no Git commit ID.

The import preserves every source/data file byte-for-byte, including line endings
and final newlines. `docs/vendor-manifest.sha256` records the SHA-256 of each file
at this baseline; it describes the import, not subsequent project modifications.
The first migration commit removes the previous community additions from the
firmware tree. Existing Git history is retained for reference and recovery.
`README.md`, `AGENTS.md` and `docs/` are project documentation outside the archive.

## Future imports

Download a new official archive into a separate directory, record its URL and
checksum, inspect its file list, and compare it with this baseline. Commit the
vendor import separately from project changes. Do not mix runtime configuration
or credentials into an import, and do not assume the source defaults match a
previously flashed board's persistent settings. Build and review before deployment.

## Known upstream finding

The archived `esp_now_ctrl.h` is byte-identical to the community snapshot at
`tinkerfuroc/tk25_hardware` commit `6ee8022cc28fa4de073617c3cbea2e04f681f768`.
It also matches `waveshareteam/ugv_base_general` commit
`d308df91b333a513f45a238bef9ba9a0a5edf64c` after normalizing CRLF to LF.
ESP-NOW callbacks mutate shared JSON and write Serial outside the main loop;
the concurrency finding is source-reviewed and has not been reproduced on hardware.
This import does not fix or disable ESP-NOW.
