<!-- Copyright (c) 2026 Marco Pezzullo / Silicon LogiX. License: see LICENSE. -->
# Live bench API

The desktop, customer DLL and API observe one simulation session. Commands change the requested opening; the executed control computes motor drive. Neither transport writes the plant position directly. `read` is an operator view containing actual position. `measurements` is a controller-facing allowlist containing received measurements, their ages and host diagnostics, with no plant truth or injected fault configuration.

## Connections

Use **API and boards** to connect a selected USB serial port or enable the local API. Serial settings are 115200 baud by default, 8 data bits, no parity, one stop bit, no flow control. The local API listens only on TCP `127.0.0.1:47400`, for PC automation. Both transports exchange UTF-8 JSON objects terminated by a newline. Maximum request size: 16 KiB; at most 32 frames per incoming batch. Keep board polling around 5–10 Hz at 115200 baud and one request outstanding. This is a request/reply protocol with no unsolicited telemetry.

CLI: `slx-resiliencelab.exe --live --api`, or `slx-resiliencelab.exe --live --serial COM3 --baud 115200`. `--api-port` changes the local port. Port 0 selects an available port. Connections are disabled at normal startup and are not stored in scenarios. No access token or workstation path is exported.

## Request contract

```json
{"id":1,"op":"read"}
{"id":2,"op":"measurements","controller":"main"}
{"id":3,"op":"target","value":80}
{"id":4,"op":"fault","kind":"sensor_stuck","sensors":[0,1,2]}
{"id":5,"op":"fault","kind":"sensor_restore","sensors":[0,1,2]}
```

`id` is echoed. Replies contain `schema: "slx.bench.v1"`, `ok`, and either `data` or `error`. `read` returns the same snapshot shown in the GUI, with execution clock and running state. `measurements` defaults to the current owner; `controller` may be `main` or `backup`. It reports setpoint received by that controller, three readings with value/age/status/trust, fused measurement, quorum, ownership, heartbeat availability, calculated motor command, stopped state and diagnostic. Null means unavailable. A stopped primary's ages continue increasing; its last control decision remains cached until execution resumes, while `controller_alive` reflects heartbeat timeout.

Additional operations: `live` starts a new nominal five-minute session at 1×, preserving the selected DLL/configuration but clearing the fault timeline; `run` resumes; `pause` freezes virtual time; `step` advances a paused session; `reset` returns to its beginning. Target/fault actions are acknowledged as scheduled and applied at the next simulation step, including while paused. Fault values follow the scenario contract: `communication` is loss probability 0–1, `sensor_drift` is percentage points per second. Sensor indices are 0, 1 and 2. Invalid grouped requests apply nothing. Concurrent GUI/API target actions execute in arrival order.

## Three frozen sensors

Start the live bench, request 80%, and freeze all three sensors during motion. Readings continue arriving with increasing timestamps but identical frozen values. They can remain fresh and agreeing, so quorum alone does not reveal this fault. The controller's progress monitor compares commanded motion with changes in received measurements; after its observation interval it latches `no_measured_progress` and requests a stop. The gateway applies zero motor drive. It cannot infer whether the cause is common sensor failure or a jammed actuator from these measurements alone.

If all sensors freeze while the valve is stationary at its target, there is no motion evidence and no false diagnosis. Disconnecting them instead causes `insufficient_measurements` after measurement timeout. Restore sensors and change the target to release the progress stop, or reset. True position belongs to the operator view; the controller cannot use it to diagnose the fault.

## Board integration and examples

The USB link is implemented; firmware must implement this JSON contract. A board can request measurements, read diagnostics and command the bench setpoint/faults. The motor algorithm still executes in the SLX host or a customer DLL: this transport does not yet replace that algorithm with commands from physical firmware. Full hardware-in-the-loop control, electrical signals, firmware heartbeat and guaranteed cycle timing remain customer-specific integrations.

Run `python examples/bench-client/client.py read` over local TCP. The example also supports `--serial COM3`; that optional Python mode needs `pyserial` on the client PC. The desktop itself has no Python dependency. The board example `examples/board-client/board-client.ino` polls measurements over USB and can request an 80% opening; adapt the firmware to your board. Software transport tests run without a board. No physical board validation is claimed.
