<!-- Copyright (c) 2026 Marco Pezzullo / Silicon LogiX. License: see LICENSE. -->
# Version 0.4.0 validation

Verified on Windows x64 with Qt 6.12.0 and MinGW GCC 13.1.0. Core version 0.2.0, facade ABI 1, customer C ABI 1. The tests cover the deterministic core, message arbitration, desktop/report integration, input handling, live API and QML startup. Native captures exercise OpenGL; mouse tests use offscreen software rendering for display independence.

| Preset / seed 2407 | Observed result |
|---|---|
| Stuck sensor at 1.60 s | S1 isolated at 2.12 s; final error 0.137%; no stops/takeovers |
| Primary stopped at 2.00 s | Watchdog at 2.24 s; backup granted at 2.58 s; one takeover; final error 0.158% |
| S1/S2 disconnected at 2.00 s | Expired readings at 2.40 s; stop applied at 2.42 s; position held at 51.48% |

Times follow original message timestamp and fixed virtual steps. Heartbeat timeout begins at the previous heartbeat, rather than injection time. The tests exercise conflicting/absent/reordered readings, sensor recovery, lost bus traffic, ownership/epoch/sequence fencing, command timeout, measured progress, travel limits, customer DLL execution/rejection and replay.

Version 0.4 adds manual live controls and JSON-line serial/local TCP integration. Tests send fragmented and pipelined requests through real sockets, compare API state with the GUI session, reject invalid grouped faults atomically, check truth-free measurement reads, freeze all three sensors during motion and at rest, recover readings, replay manual actions and verify disconnect/quorum stop. Desktop input tests click New live bench, enter 80% and freeze all sensors through the visible controls. A nonexistent serial port produces a real connection failure. Physical USB boards are not tested in this environment.

A native live capture uses TCP to request 80% and freeze all three sensors. At 1.72 virtual seconds the API and desktop show actual position 26.84%, fused measurement 0%, motor drive 0 and `no_measured_progress`. Exact positions in manual wall-clock interactions depend on action arrival times; the saved timeline makes that interaction replayable. [Executed live bench](../assets/screenshots/live-bench.png).

Campaign tests use actual executed control, all 600 preset samples, plugin hash and complete reports. A jammed actuator produces FAIL and CI exit code 1. Desktop pacing tests cover delayed callbacks, catch-up bounds, pause exclusion, rate changes and identical batch results. Normal/cutaway geometry keeps an independent rotor; batching reduces the cutaway from 1004 to 15 draw calls without removing triangles.

The native render/readback diagnostic is not an interactive FPS guarantee. Desktop timing uses a monotonic clock and reports lag, without guaranteed deadlines. Portable tests exclude the Qt development kit from PATH. Distribution source builds exclude proprietary implementation. Screenshots and published validation documentation use English.

Limits: simplified kinematics, ideal gateway, controllers in one process, customer DLL crashes/stalls not isolated, no mechanical/fluid qualification, no safety certification and no hardware validation. Identical floating-point replay is verified in the same kit; other platforms/compilers can differ slightly.
