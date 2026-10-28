# Migration to the official product source

Prepared on 2026-10-28 on `migrate/official-pan-tilt`, based on merged main
`8ceadeede07a4e5b78360b2417dcb9dc739adbe3`.

## Scope and history

The fork has one pull request: [PR #1](https://github.com/tc25898/tk25_hardware/pull/1),
merged as `8ceadee`. Its feature commit `8b12bf4` adds Host Wi-Fi provisioning;
`85ac5da` fixes keyboard isolation for the entire provisioning form. The PR's
initial implementation also preserves release of movement keys held before
focus enters the form. All of those changes are retained.

The migration has two layers:

1. Import the 26 official ZIP files byte-for-byte, removing the prior community
   firmware changes and community-specific READMEs/test script/summary. Preserve
   existing Git history and `AGENTS.md`; add source provenance and a checksum
   manifest outside the firmware directory.
2. Apply only PR #1's firmware changes and its protocol documentation. Replace
   the old root README with documentation for this vendor-based project.

No history rewrite, new firmware features, servo/heartbeat fixes or ESP-NOW
changes are part of the migration. The repository name and GitHub fork
relationship remain unchanged; the owner plans to rename it separately.

## Exact firmware difference from the ZIP

Six existing files change and one is added:

| File | Project change |
|---|---|
| `host_wifi_ctrl.h` (new) | Credential validation, request serialization, retained latest Host reply |
| `http_server.h` | Host POST/status endpoints and the PR's existing invalid-JSON/MIME correction |
| `json_cmd.h` | Management command `T:-1001` |
| `pan_tilt_base_v0.9.ino` | Include the management header before UART dispatch |
| `uart_ctrl.h` | Bounded receiver and management consume-before-echo handling |
| `ugv_config.h` | Shared 1024-byte frame bound |
| `web_page.h` | Provisioning form, polling, whole-form shortcut guard and held-key release |

All other 20 official source/data files are unchanged. The two unchanged vendor
data JSON files contain public demo settings, not the owner's network settings;
no private configuration is imported. Source file line endings/final newlines
are retained. `docs/vendor-manifest.sha256` identifies all 26 original files at
the import commit; six entries intentionally differ after the project layer.

## Changes relative to the old community-based main

These are restored vendor behaviors, not missing portions of PR #1:

| Area | Previous community main | Official baseline retained here |
|---|---|---|
| Default `moduleType` | `2` (gimbal) | `0` (no optional module) |
| Gimbal feedback fields | `T`, `X`, `Y` only | IMU/voltage fields plus `pan` and `tilt` when in gimbal mode |
| Servo feedback collection | 20 ms gate in `moduleType_Gimbal()` | Every call to `moduleType_Gimbal()` |
| Explicit base feedback query | Bypasses interval with `baseInfoFeedback(true)` | Ordinary interval-limited `baseInfoFeedback()` |
| Manual gimbal stop | Community pending-stop logic | Vendor torque/delay/feedback sequence |
| Wheel-speed shell mapping | Community `fabs`/comparison correction | Original vendor comparisons |

Review host expectations before adopting this version; in particular, a consumer
expecting `X`/`Y` feedback is not wire-compatible with `pan`/`tilt`. Existing boot
missions/persisted board settings can override or supplement source defaults.
Vendor setup contains servo initialization and boot-mission execution; this
import does not establish no-motion boot or hardware readiness.

If selected community behavior is needed, retain it as an explicit, separately
reviewed project change rather than silently mixing it into the vendor baseline.

## Validation

Both complete sketches compiled, linked and generated binaries with Arduino CLI
1.2.2, ESP32 core 2.0.17 and generic validation FQBN `esp32:esp32:esp32`, using
the dependency versions listed in [Host Wi-Fi notes](host-wifi.md). This generic
target is not identification of the delivered board. The vendor documentation
describes core 2.0.11; that version was not installed or validated in this run.

| Build | Program storage | Static global storage |
|---|---|---|
| Untouched official ZIP | 991,933 bytes | 49,748 bytes |
| Official ZIP + PR #1 | 1,001,305 bytes | 51,752 bytes |
| Increment from project layer | 9,372 bytes | 2,004 bytes |

These are compiler reports, not measured runtime heap, latency or whole-board
headroom. Two fixed 1024-byte RX/result buffers replace the unbounded receiver
String. Transient HTTP/serialization allocations remain; no worker, queue,
extra history, camera pipeline or media service is introduced.

The original temporary Node VM exercise of the actual embedded JavaScript
passed provisioning, repeated/late results, verbatim text, credential bounds,
no replay, whole-form Enter/Space/WASD/arrow isolation and release of all eight
held direction-key aliases after focus changes. The original temporary native
C++ exercise passed the actual management header and identical extracted UART
receiver: serialization, echo suppression, repeated/late/empty replies, framing,
RX budget, overflow/NUL recovery, vendor dispatch and credential bounds. These
are host simulations, not browser-layout or physical-UART tests; no new test
suite or Python tooling was added to the repository.

All 26 import files passed checksum verification before the project layer.
Source comparison confirms only the six modifications and one added header
listed above, with unchanged vendor motion, feedback and ESP-NOW modules.
Whitespace checks treat CRLF as line endings. The untouched vendor import has
pre-existing space-before-tab indentation in its old receiver, which is preserved
at the baseline; the final PR #1 receiver replaces those lines.

No flashing, deployment, device configuration change, Wi-Fi switching or hardware
operation occurred. Next: review the migration scope, merge when accepted, and
rename the repository separately. Physical integration needs the usual separately
authorized board validation.
