# Upstream integration and isolated ZipZap extensions

## Approved scope

- Integrate StereoKit PR #1429 head `2b6ef4b2084ee9054d1bbe2f170f323c6545034f` into the fork baseline `cf6986582d771c1b73d569e3860692f3f6824c53` on a separate integration branch.
- Keep upstream as the owner of texture loading, materials, rendering, and task lifecycle.
- Keep native ASTC6 KTX2 parsing, bounded validation, and concrete format preflight as an isolated ZipZap addition, reached through minimal internal loader hooks.
- Preserve existing public fork entry points and transparent loading through `Tex.FromFile` / `Tex.FromMemory`.
- Keep one managed assembly/native runtime and one versioned `ZipZap.StereoKit` package.
- Add an extension build switch and validate both core-only and extension-enabled configurations.
- User approved local merge/refactor commits, final local merge into `develop` after validation, and a new local Windows x64 / Android arm64 package. No push, remote publication, consumer changes, or device installation.

## Execution

1. [x] Integrate pinned upstream PR and resolve the three known conflicts while preserving ASTC6 and public format preflight.
2. [x] Record the upstream integration separately from extension isolation.
3. [x] Isolate extension implementation, native/managed API declarations, build wiring, compatibility additions, and tests without a generic plugin framework.
4. [x] Add focused coverage for dispatch, invalid recognized inputs, extension-disabled behavior, and retained public APIs.
5. [ ] Validate core-only and enabled native builds, CPU tests, available native loader smoke tests, managed build, and Android native build.
6. [ ] Review the final patch, commit the refactor, and build/inspect a new immutable local package from committed source.
7. [ ] After required checks pass, merge locally into `develop` and move this plan into `plans/completed/`.

## Validation and artifacts

- Agent-created scripts, logs, probes, and temporary reports: `.tmp/upstream-extensions/` only.
- Required build/cache/package output retains the repository's ignored `bin/` and `.deps_cache/` contracts.
- Native ASTC6 upload requires a capable GPU. Unsupported GPUs must reject native ASTC6 explicitly and still load portable UASTC.
- No physical Quest visual, lifecycle, or performance acceptance is implied by host builds.
- Package provenance must match the committed runtime source; existing feed versions must never be overwritten.
- Blocked, failed, or unverified scope keeps this plan active.

## Results

- Upstream merge commit: `60375acb`.
- Extension-enabled Windows static native build and CTest: 2/2 passed. CPU parser: 1741 checks, exact 640-byte None/Zstd equality. Loader suite: zero failures.
- Core-only Windows static native build and CTest: 1/1 passed. Generated native project contains zero `StereoKitC/zipzap` compile items.
- Asset, spherical harmonics, texture compression, spatial, texture, and future native suites passed with zero total failures in both build modes.
- Managed API/compile compatibility tests passed in both modes: 11 checks each; core-only assembly omits extension methods.
- Integration portability fix: upstream compression tests used identifiers named `small`, which collide with a Windows SDK macro. Renamed only those test-local identifiers to permit MSVC validation.
- Host reports no ASTC6 sampled/linear support. Unsupported-format rejection and portable UASTC loads passed; native ASTC6 GPU upload/visual validation remains a device qualification gap, not a host validation claim.
- Android/release package validation and final local develop merge are pending.
- Verification logs: `.tmp/upstream-extensions/` (ignored; not committed).
