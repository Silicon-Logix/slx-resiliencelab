<!-- Copyright (c) 2026 Marco Pezzullo / Silicon LogiX. License: see LICENSE. -->
# Integrating a customer's product

ResilienceLab simulates the plant. The operator sees actual simulated position to assess results; control works from received measurements and messages.

## Available software paths

**Software-in-the-loop:** simulated plant and sensors -> received measurements/setpoint -> customer DLL on the PC -> motor command -> simulated plant. C ABI v1 replaces motor-command calculation; fusion, controller heartbeat and arbitration remain in SLX. The DLL is not a microcontroller emulator and does not test firmware peripherals or hardware timing.

**Live USB serial API:** a connected board or PC client can read controller measurements/diagnostics, request an opening, pause/run and inject faults. Operator `read` includes actual position; controller `measurements` excludes plant truth. The desktop and transport use one session. [Protocol](bench-api.md). Firmware must implement the contract. Serial transport is implemented and software-tested; no physical board test is claimed.

## Commissioned firmware control / HIL

The complete HIL flow is simulated plant -> adapter -> physical board running customer firmware -> adapter -> simulated plant. The board receives the signals expected by its firmware and returns motor commands/heartbeat. The valve remains virtual.

This requires agreed board/protocol/signal map, cycle period, timeouts, controller identity, sequence handling and command ownership. UART/USB, CAN/Ethernet and electrical encoder/analog I/O are different integrations. Current bench API commands set the target; they do not replace host motor computation with physical firmware output. This boundary is explicit in the application previews.

The reverse path, a PC controller driving a physical actuator, is a distinct test bench with real I/O and appropriate bench protections; it is absent from this demonstrator.

## Timing, verification and contact

Core integration uses fixed virtual steps. Batch runs preserve replay without waiting for the PC. Desktop 1x follows a monotonic clock and catches up delayed steps with bounded work. Pause excludes inactive time; 0.5x/4x change playback speed. Windows/Qt timers and customer DLLs do not guarantee deadlines. Dedicated realtime execution and physical I/O must be designed and measured for the chosen firmware target.

Reference: [Qt timer accuracy](https://doc.qt.io/qt-6/qtimer.html#accuracy-and-timer-resolution), [monotonic clock](https://doc.qt.io/qt-6/qelapsedtimer.html), [HIL architecture](https://www.ni.com/en/solutions/hardware-in-the-loop-testing.html).

Campaigns execute three demonstration criteria with full evidence and a customer DLL hash. CI exit codes distinguish pass, criterion failure and loading/export failure. Product requirements and extended campaigns are commissioned work. **Custom test bench** saves a technical request locally; [contact Silicon LogiX](https://www.siliconlogix.it/contatti) to discuss the product, board/interface, cycle and failures to test. No automatic transmission or safety certification is implied.
