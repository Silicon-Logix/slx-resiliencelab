<!-- Copyright (c) 2026 Marco Pezzullo / Silicon LogiX. License: see LICENSE. -->
# Valve asset integration

The original SLX ResilienceLab Valve v0.1 asset package remains the geometry reference. Runtime uses `SLX_Valve_closed.glb`, `SLX_Valve_cutaway_closed.glb` and `asset_manifest.json`. The normal and cutaway variants are static; default animation is disabled.

`VALVE_ROTOR` is the moving hierarchy. Opening 0-100% maps to 0-90 degrees around the axis/pivot declared in the manifest. The renderer validates units, hierarchy, rotation mapping and pivot; it applies the rotor transform using actual plant position. Camera motion never changes simulated position.

Geometry, manifest, CAD, Blender scene and previews retain their original bytes; source/document attribution and current package hashes are normalized. Application-specific integration and binary asset attribution live in the project documentation. `python tools/validate-valve.py` verifies manifest/geometry compatibility; screenshots cover normal and cutaway runtime views. The asset is illustrative and conveys no mechanical, fluid or safety qualification.
