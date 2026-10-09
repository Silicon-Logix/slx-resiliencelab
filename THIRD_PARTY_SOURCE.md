<!-- Copyright (c) 2026 Marco Pezzullo / Silicon LogiX. See LICENSE. -->
# Corresponding runtime sources and replacement

The portable Windows package contains `runtime-sources/manifest.json` and the pinned corresponding source archives for Qt 6.12.0, GCC 13.1.0, MinGW-w64 v11.0.0 and MinGW build scripts. Patch provenance, Qt configuration files and module SPDX documents are included. Runtime hashes are checked during packaging.

To fetch/verify these same sources from the integration repository:

```powershell
python tools/prepare-runtime-sources.py
```

Archives come from the official Qt download archive, GNU GCC archive and the pinned upstream MinGW repositories. Exact URLs and SHA-256 values are in the preparation script and source manifest. No third-party source modifications were made by this application.

The application and proprietary core link Qt dynamically. To replace a Qt library, rebuild that module with matching Windows x64 ABI and compatible configuration, replace the matching DLL/plugins/QML runtime beside the executable, then run `slx-resiliencelab.exe --smoke-test` and the three scenarios. The same Qt and GCC runtime ABI is required by the approved SLX core binary. Third-party rights to modification, replacement and debugging are expressly preserved by section 8 of LICENSE.

The source candidate contains UI/integration code and public core headers; the proprietary core implementation is not included. Use a matching approved core SDK when rebuilding integration code. The runtime source materials accompany the portable bundle regardless of whether SLX implementation sources are supplied.
