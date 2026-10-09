<!-- Copyright (c) 2026 Marco Pezzullo / Silicon LogiX. License: see LICENSE. -->
# Dependencies and distribution

The approved kit is Windows x64 / Qt 6.12.0 / MinGW GCC 13.1.0. Qt Core, Gui, QML, Quick, Quick Controls 2, Network, Serial Port, OpenGL and SVG are dynamically linked. Replaceable runtime DLLs, license texts, SPDX notices and corresponding sources accompany the portable package.

| Component | Terms | Distribution choice |
|---|---|---|
| SLX code and original assets | Silicon LogiX Evaluation License 1.0 | Compiled proprietary core; separately distributable integration source |
| Used Qt modules | LGPLv3 or commercial Qt | Dynamic, replaceable libraries; notices and corresponding source supplied |
| GCC runtime | GPLv3 with GCC Runtime Library Exception | Preserve runtime exception and corresponding toolchain sources |
| MinGW-w64 / winpthreads | Runtime-specific terms | Preserve license texts, build/patch provenance and sources |
| OpenGL | System graphics driver API | No third-party rendering engine bundled |
| Optional Python serial client | pyserial BSD license | Not bundled into the desktop; example requirements only |

Qt Quick 3D, Charts, Graphs and HTTP Server are not used. The renderer/importer and plots are original SLX code. Serial uses Qt Serial Port under its LGPL option. Network uses Qt Network's TCP API. [Qt licensing](https://doc.qt.io/qt-6/licensing.html), [LGPL obligations](https://www.qt.io/development/open-source-lgpl-obligations), [Serial Port license](https://doc.qt.io/qt-6/qtserialport-index.html#licenses).

`tools/prepare-runtime-sources.py` verifies pinned archives and runtime hashes. Qt Serial Port is built without modifications from its verified 6.12.0 source with the approved kit; build instructions and binary hash are included. Local workstation paths in generated SPDX build metadata are normalized to `/project`; attribution and legal terms are retained. The SLX license permits LGPL replacement and required reverse engineering. Third-party legal texts are retained unchanged.

`scripts/export-public.ps1` prepares a distribution source checkout using an allowlist. It contains integration sources, public headers, examples, docs, runtime assets and approved compiled core SDK; proprietary source/tests, development logs, Git history and workspace are excluded. The SDK facade requires the documented matching Qt/compiler kit. Builds of this checkout import the core binary.

GitHub-visible source remains subject to the evaluation license. Runtime source archives belong in the portable release package, rather than source Git history. [Publication checks](publication.md) describe repository review and evidence. Publication is a separate release action.
