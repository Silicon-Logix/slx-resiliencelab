<!-- Copyright (c) 2026 Marco Pezzullo / Silicon LogiX. License: see LICENSE. -->
# SDK and customer control

`core/include/slx/resilience.h` exposes `slx::Session`: load scenario, step, run, request a target, inject faults, retrieve samples/events/results, and compare controls. The Qt Core facade remains ABI 1. Core version 0.2.0 adds English diagnostics and controller stop reasons. The approved SDK requires Windows x64, Qt 6.12.0 and MinGW GCC 13.1.0. Do not use its `.dll.a` with MSVC; obtain a matching binary SDK.

Distribution checkouts contain desktop sources, public headers and the approved DLL/import library. Configure `SLX_BUILD_PRIVATE_CORE=OFF`; `SLX_CORE_BINARY` can select a compatible import library. Keep the corresponding DLL alongside the program.

## Customer C ABI v1

`customer_api.h` is a C header independent of Qt. Export `slx_customer_controller_v1()` returning a versioned `SlxCustomerApi` with `create`, `step`, `destroy`. Primary and backup receive separate contexts. `step` receives virtual time, fixed step, received target/readings, measurement ages, trusted mask and fused measurement. It receives no actual position or fault configuration. Return normalized motor drive from -1 to +1. Fusion, quorum stops, transport and arbitration remain in SLX.

Try **API and boards -> Try example DLL -> Run all 3 tests**. Modify `examples/customer-controller/controller.cpp` to integrate an algorithm. In the portable package, compile with MinGW x64:

```powershell
g++ -std=c++17 -O2 -shared -Isdk examples/customer-controller/controller.cpp -o MyController.dll
```

In a source checkout use `-Icore/include`. Load the DLL, run tests and export evidence. The architecture must match; additional DLL runtimes must be available. The host validates ABI/structure sizes and finite output, saturates commands and fences control ownership. Client errors cause a latched stop. The DLL must return quickly and must not propagate exceptions across the ABI. Crashes, stalls and memory corruption are not isolated from the desktop process.

The live serial/TCP API is a separate interface for bench supervision and measurement reads, described in [bench-api.md](bench-api.md). Reading measurements on a board does not automatically move execution of the motor algorithm onto that board. Full firmware control requires a specific integration.
