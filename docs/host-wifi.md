# Host Wi-Fi provisioning through the ESP webpage

The embedded ESP page has a **Host Wi-Fi configuration** section beneath the
controls. It sends an SSID and WPA-PSK password to Host and displays the latest
UART reply. This is separate from ESP's existing access-point/station settings.
"Host" names the 上位机 role, independently of its processor, OS or application.

This PR takes over the owner-supplied patch
`63118190276bb3dfdedf371bcca7fdbf9d265505`, based on repository main `6ee8022`.
The corresponding Pico live-apply implementation is
[backyard-defense PR #20](https://github.com/tc25898/backyard-defense/pull/20).

## Protocol and behavior

The browser POSTs JSON to `/host/wifi`. ESP validates required fields and WPA
credential bounds, then writes a flat request and newline to the existing
115200 UART. `CMD_HOST_WIFI_CONFIGURE` in `json_cmd.h` defines command `-1001`:

```json
{"T":-1001,"ssid":"example-network","password":"example-passphrase"}
```

HTTP 202 means ESP sent the request to UART; it does not prove receipt, saving
or connection. The page clears its password field and shows
"Request sent. Host has not replied yet." There is no Host reply deadline,
readiness handshake, automatic resend, reboot or network restart.

Every complete UART object with numeric `T:-1001` and string `result` replaces
the latest result. Management input is consumed before command echo and vendor
dispatch, so replies cannot become echoed requests. Other vendor IDs retain
their dispatch. Host owns the operation and the meaning of its result text.
For example, Pico PR #20 sends these as separate UART messages:

```json
{"T":-1001,"result":"Configuration saved; applying Wi-Fi"}
{"T":-1001,"result":"Wi-Fi connected"}
```

Errors are also plain result strings. ESP adds no success interpretation,
prefix, reboot reminder or fields. It accepts late and empty replies too.

The page polls GET `/host/wifi/status` once per second, with one poll in flight.
The endpoint returns `state` (`idle`, `sent`, or `reply`) and string `result`.
All browsers observe the same RAM-only latest result; reads do not consume it.
A new valid submission clears it; ESP restart resets status. Polling continues
after a reply, so later progress/final results can replace it. Fast transitions
may be skipped by polling: this is latest-state display, not message history.
Results are rendered verbatim with `textContent`, never as HTML. Status responses
are not cached. An in-flight status response from before a submission cannot
overwrite that submission's browser feedback.

No version/request ID or correlation guarantee is introduced. Submit one update
at a time. The page permits another submission; delayed messages cannot reliably
be associated with overlapping or repeated requests. HTTP failure or a missing
Host reply leaves the outcome unknown; the page never automatically replays it.

SSID is 1–32 UTF-8 bytes without NUL. Passwords are 8–63 printable ASCII bytes,
or a 64-digit hex PSK. Browser and firmware both validate these constraints.
Credentials use the POST body, not URL parameters, and are neither persisted
nor copied into retained status/logging by this code. The Host must likewise
avoid echoing credentials in its result. Extra JSON fields are ignored.

## Integration and keyboard handling

Command IDs remain in `json_cmd.h`; `UART_FRAME_BYTES` belongs in `ugv_config.h`
with the UART settings. `host_wifi_ctrl.h` owns Host request/status handling,
following the existing sibling-header layout. HTTP uses synchronous `WebServer`;
the browser uses `XMLHttpRequest` and interval polling like the existing page.
The form reuses the page's classes and adds only layout/message-wrapping CSS.
Host status is separate from `jsonInfoHttp`, which normal command replies clear.

The entire Host provisioning form, including SEND and nested elements, is
excluded from document-level command shortcuts. Enter/Space retains the button's
normal form activation, without also submitting the raw JSON command or
starting movement. Input/select/textarea/content-editable elements elsewhere
remain excluded too. A control key pressed before focus moved into the form
still releases its already-held direction on key-up. Ordinary typing with no
held direction remains ignored. Heartbeat, actuator modules, boot
initialization, command IDs and motion limits are unchanged by this PR.

## Bounds and resource review

UART frames are limited to 1024 bytes including newline. Oversized/literal-NUL
frames are discarded through the next newline; later frames remain usable.
Reception consumes at most 1024 bytes per main-loop iteration before HTTP,
control and heartbeat work resumes. ArduinoJson parses the mutable buffer and
checks required fields, with no handwritten JSON validator or field whitelist.
The existing 256-byte JSON pool still bounds object complexity.

The fixed receive and latest-result buffers reserve 2048 bytes plus counters
and flags, replacing the previous unbounded receive String. Parsed strings
borrow the receive buffer; only the latest result is copied before reuse.
There is no history queue, worker, per-browser state or long-lived credential
copy. HTTP uses transient Strings and small local JSON documents. The HTTP
body bound is checked after WebServer collects it, so it is not an allocation
bound for the HTTP parser itself. UART TX uses the existing synchronous Serial
writer but never waits for a Host reply. These are source-derived bounds, not
measured target free-heap or timing improvements.

## Validation and handoff

The takeover retains the supplied patch's toolchain/library pins. Current
validation uses Arduino CLI 1.2.2, ESP32 core 2.0.17 and generic validation FQBN
`esp32:esp32:esp32`; this does not identify the delivered board/core. Libraries:
ArduinoJson 6.21.5, SCServo 1.0.2, Adafruit SSD1306 2.5.17, Adafruit GFX 1.13.0,
Adafruit BusIO 1.17.4, Adafruit Unified Sensor 1.1.15, Adafruit ICM20X 2.0.7,
INA219_WE 1.2.7, ESP32Encoder 0.12.0, PID_v2 2.0.1 and SimpleKalmanFilter 0.2.0.
INA219_WE 1.4.1 is incompatible with existing battery enum names; use 1.2.7
instead of changing unrelated battery code. Build tools also need the bundled
esptool 4.5.1's host dependencies. No source Python tooling was added.

- Temporary Node VM checks of the actual embedded JavaScript passed request
  fields/password clearing, repeated progress/final and late replies, no replay,
  verbatim text, one poll in flight, UTF-8/password bounds, typing isolation and
  control-key release after focus changes. This mocks DOM/HTTP, not a physical
  browser/device or a visual layout test.
- A temporary native C++ exercise with ArduinoJson 6.21.5, the actual management
  header and extracted UART receiver passed request serialization, no echo,
  repeated/late/empty replies, credential bounds, exact 1024-byte framing,
  per-loop RX budget, oversized/NUL recovery and vendor dispatch. It exercises
  software paths without a physical UART or actuator. No new test suite added.
- The complete sketch compiled, linked and generated its binary with the pins
  above. Compiler report: 1,001,669 bytes program storage (76%) and 51,776 bytes
  static global storage (15%). These are build results, not measured free heap,
  heartbeat latency or delivered-board compatibility. No LTO flags or actuator
  modules were changed. The bundled esptool's host dependencies were installed
  only in the temporary build workspace; no Python application/tool was added.
- Source blobs for the management header, HTTP handlers, command ID, sketch
  include, UART receiver and shared frame limit exactly match the supplied patch.
  The webpage differs by excluding the entire form from command shortcuts and
  preserving the held-key release described above;
  documentation now records takeover validation and the two-result Pico peer.
  Whitespace checks passed with CRLF treated as line endings; existing file
  endings, including the mixed-ending config header, are preserved. Temporary
  validation sources, tools, libraries and binaries remain outside this repo.

### PR #1 review follow-up

Addressed the [SEND-button shortcut finding](https://github.com/tc25898/tk25_hardware/pull/1#issuecomment-6453356255).
The temporary Node VM check first reproduced Enter on the focused SEND button
leaking a raw `/js` request with a prefilled actuator command. The guard now
uses `closest('#host-wifi-form')` as well as the existing editable-element
checks, covering the form itself, buttons and all descendants. No default
button action is cancelled, and the held-direction key-up exception is retained.

The extended check passes Enter/Space/WASD/arrows on BUTTON/SPAN/FORM/DIV/LABEL
inside the form with no control requests, Host-only Enter submission, and all
eight movement-key releases after focus enters SEND. Existing provisioning,
password clearing and repeated/late result checks also pass. No new test suite,
UART/heartbeat code change or hardware operation. Full-sketch build and
CRLF-aware whitespace checks passed again. The compiler-reported program
storage grows by 64 bytes from the takeover build, with unchanged static global
storage; there are no new buffers, queues or workers. These are build results,
not measured runtime memory or latency.

With the pinned dependencies installed:

```sh
arduino-cli compile --fqbn esp32:esp32:esp32 pan_tilt_base_v0.9
```

Next: PR review, then separately authorize installing on the delivered ESP and
validate two same-T replies with Pico over real UART. Hardware boot, real
Wi-Fi switching/persistence, heartbeat timing and actuator behavior remain
unverified. No flash, deployment or hardware operation is part of this PR.
