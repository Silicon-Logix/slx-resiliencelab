<!-- Copyright (c) 2026 Marco Pezzullo / Silicon LogiX. License: see LICENSE. -->
# Demonstration guide

Open the portable desktop and choose a guided test in **Getting started**. Request 80% opening and inspect actual position, three received readings, fused control measurement and authorized controller. Drag the valve to rotate the camera, use the wheel to zoom, and switch between normal/cutaway views. The rotor follows actual simulated position, rather than a decorative animation.

The stuck sensor test freezes S1 at 1.6 s. Persistent disagreement isolates it and control continues with S2/S3. Primary loss at 2 s causes a command watchdog stop followed by backup authorization after heartbeat timeout. Losing S1/S2 at 2 s removes sufficient trustworthy measurements and stops the motor. Guidance uses events and final state; adding an actuator jam can produce a failed result.

**Pause** freezes virtual time, **Step** executes one fixed step, **Reset** replays the same seed and timeline. The presets use 20 ms steps. Select 1x PC realtime, 0.5x or 4x; the footer displays active PC time and lag. These modes do not guarantee hard realtime deadlines.

## Manual live bench

In **Laboratory**, press **New live bench** for a nominal five-minute session with no scheduled faults. Enter requested opening or use Close/Open. Freeze, disconnect or restore one sensor or all three. Use the general fault selector for drift, primary loss, communication loss and actuator jam. Commands/faults enter the next simulation step and remain in the saved timeline. At the five-minute limit, export the run and explicitly start another session.

Three frozen readings can remain fresh and agreeing. Request motion to observe the progress monitor: if the received position does not change despite motor drive, control latches a stop. It cannot distinguish a shared sensor failure from a jammed actuator using these measurements alone. At rest there is no evidence to detect all agreeing frozen sensors. Restore readings and change the target, or reset, to release the progress stop.

## API and evidence

Use **API and boards** to connect USB serial or enable local TCP. The board needs firmware using the JSON-lines contract. **Read via API** opens an actual local TCP client and displays returned position. [Contract and examples](bench-api.md). The serial API controls the bench setpoint; full physical firmware motor control remains an integration project.

**Try example DLL** loads executed C++ code. **Run all 3 tests** evaluates actual runs with PASS/FAIL and exports a campaign. **Scenarios** edits thresholds, timeouts, transport, seed and fault times. Invalid configuration preserves the existing valid scenario. JSON sensor indices are 0/1/2. Drift is percentage points/second; communication is additional loss probability 0-1.

**Comparison** repeats the same experiment with selected control A and SLX alternative B. If A is a customer DLL, B uses SLX control. Export creates HTML/JSON/CSV with both runs. A paused export is partial evidence. About, license, dependency notices and API documentation work offline.
