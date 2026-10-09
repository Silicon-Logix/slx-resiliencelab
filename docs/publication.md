<!-- Copyright (c) 2026 Marco Pezzullo / Silicon LogiX. License: see LICENSE. -->
# Distribution repository review

The development checkout contains proprietary implementation and must not be uploaded as a whole. Prepare the source distribution using `scripts/export-public.ps1`; publish only the reviewed distribution checkout. It has no inherited development Git history. The compiled core SDK is evaluation-licensed; its source is excluded.

Review gates:

- Run `python tools/audit-release.py --distribution` on the prepared checkout before initializing Git: headers/attribution, resource paths, language markers, machine paths, credentials, draft markers and forbidden directories.
- Build with `SLX_BUILD_PRIVATE_CORE=OFF` using the approved kit; run the desktop smoke test and a campaign against the imported DLL.
- Run the packaged smoke script with the development kit excluded from PATH; inspect English desktop/About/API screenshots.
- Preserve LICENSE, third-party notices and exact corresponding runtime sources in the release package. Keep dependency binaries replaceable.
- Put portable ZIP/runtime source archives in release downloads; avoid tracking development build products or asset authoring files in source Git history.

The repository workflow runs source/distribution audits with `--allow-git` to accept the public checkout's own Git metadata. Private source, tests, workspace and build directories remain forbidden. Initialize a new public history after the strict export audit; never copy internal Git metadata. The workflow does not claim to compile proprietary binaries on an unmatched hosted runner. Core SDK compatibility, software checks and absent physical board validation are documented. No public remote or upload is created by preparation scripts.
