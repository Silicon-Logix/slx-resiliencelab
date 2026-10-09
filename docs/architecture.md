<!-- Copyright (c) 2026 Marco Pezzullo / Silicon LogiX. License: see LICENSE. -->
# Architecture

The proprietary compiled core contains simulation, control, communication, faults and recording. Public Qt Core facade ABI 1 exposes a session without implementation definitions. The desktop/integration layers link only approved binaries in a distribution checkout.

- Plant: true position, velocity, travel limits and actuator jam. Target, truth and readings are distinct.
- Sensors: three noisy position channels with stuck/drift/disconnection and recovery.
- Controllers: separate primary/backup C++ instances. Each consumes delivered measurements and setpoints, forms a trustworthy subset and computes motor drive. No access to plant truth or injected faults.
- Bus: actual queued measurements, setpoints, heartbeats, claims, grants and commands, with sequence, timestamp, virtual delay/jitter and seeded loss.
- Gateway: one command owner, epochs and sequence/TTL fencing; watchdog and stop application.
- Recorder: every fixed-step sample, events, detections, metrics and bus trace.

`src/labbridge.*` connects the session to QML and records manual actions. `simulationpacer.*` synchronizes fixed virtual steps with active monotonic PC time. `benchapi.*` provides shared JSON-lines framing for QSerialPort and loopback QTcpServer; writes enter the normal session target/fault path. Controller-facing reads use an allowlist excluding truth. `campaign.*` evaluates full runs and exports evidence; `report.*` produces standalone HTML/JSON/CSV.

`valveview.*` uses QQuickFramebufferObject and an original limited GLB importer with OpenGL 3.3. Meshes with equal material/transform are batched without merging fixed geometry into the independent rotor. QML Canvas plots are refreshed at up to 10 Hz while recording preserves every control step. Qt Quick 3D/Charts/Graphs are not linked.

Customer C ABI v1 replaces the motor algorithm; two contexts remain under host fusion, heartbeat and arbitration. Controllers run in the same process. The gateway is idealized and is not a simulated redundant network arbiter. USB bench supervision is implemented; full physical firmware motor control is a separate integration.
