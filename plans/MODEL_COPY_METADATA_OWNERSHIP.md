# Model.Copy node-metadata ownership

## Approval and scope

User approved a native `Model.Copy()` fix, regression coverage, a local source commit and a new immutable local NuGet package. Fix current `develop`, then backport only this fix to the consumer's existing `cf6986582d77` baseline. The initial approval excluded push, registry publication, other-repository commits and device installation. The user subsequently approved committing/pushing all affected repositories and both StereoKit source branches, while explicitly retaining the existing local NuGet-feed workflow. No registry/GitHub package release or device installation is requested. DefenderVRS adoption remains on the existing renderer/shader generation, not the newer develop integrations.

## Confirmed defect

`model_copy` copies the node array and duplicates node names, but retains aliased `dictionary_t<char*> info` storage and values. Both models free that storage in `model_node_info_clear`. A GLB containing node extras reproduces Windows heap corruption `0xC0000374` in both released local package 3 and current local package 5. Original model use, independently rebuilt static visuals, and a scratch derivative without node metadata exit cleanly. Source assets remain unchanged.

## Implementation

- Preserve the public C/C# ABI, node graph, animations and shared mesh/material references.
- Give each copied node its own info dictionary and strings using existing metadata ownership APIs.
- Add a permanent native `-modeltest` suite covering metadata values/counts, empty nodes/models, mutation isolation, copy-of-copy and both destruction orders.
- Commit only this scoped source/test change locally on develop; backport it in an isolated worktree under `.tmp/model-copy-metadata/`.
- Build/package the backport with matching Windows x64 and Android arm64 binaries, committed-source provenance and a new package version. Preserve existing package versions.

## Validation

- Native model/asset tests on develop and the backport; build warnings reviewed.
- Windows/Android package build and payload/provenance checks.
- Minimal managed `Model.Copy()` GLB probe using the original train model, metadata intact, with clean shutdown.
- Consumer package-pin consistency, Windows/Quest builds, relevant tests and native saved-prefab rendering/clean shutdown.
- Physical Quest rendering/performance and unrelated minimize/restore issues are outside this fix's acceptance; do not claim they passed.

Scratch and logs: repository-root `.tmp/model-copy-metadata/`; compiler/package caches retain their existing ignored native locations. DefenderVRS reproduction/evidence remains in that repository's `.tmp/zipzap-editor-prefabs/`.

## Status

Active — scoped fix is implemented, locally committed, packaged and adopted. Native ownership/package checks pass; integrated desktop acceptance remains blocked by the pre-existing graphics/shutdown failure described below. Keep this plan active until its required consumer acceptance passes.

### Source and package provenance

- Develop commit: `7247a55f`, `Fix independent ownership of copied model node metadata`.
- Stable backport: `5863048d039a9a194549040d88e964182ae30504`, branch `fix/model-copy-metadata-astc6`, parent `cf6986582d77`. Isolated worktree: `.tmp/model-copy-metadata/stable/`.
- Each commit changes only `model.cpp`, the new native model tests, test runner registration and CMake registration. No public ABI, shader generation or renderer upgrade.
- Package: `ZipZap.StereoKit/0.4.0-zipzap.6.g5863048d039a`, SHA256 `f95436fb2693d64462166c1f49bbd92e6c45831d2a75181aff6cf5f523681cd0`. Matching Windows x64/Android arm64 builds, committed-source provenance and payload checks passed.
- Exact immutable package copied to the main checkout's existing ignored `bin/nuget-feed/`, without overwriting previous versions. DefenderVRS's three direct references and EngineLib's shared version property adopted it. EngineLib and DefenderVRS integration commits are now published; the follow-up approval also covers publishing `develop` and `fix/model-copy-metadata-astc6`. The package remains in the existing local feed, with no registry/GitHub package release or headset installation.

### Verification completed

- Develop `SKTests -modeltest -assettest`: **144 model checks, 0 failures; asset suite passed; 0 total failures**.
- Stable backport full native suite: same result (`backport-full-tests.log`). Separate public-API harness linked to the actual packaged DLL also passed all **144** model checks (`backport-model-tests.log`).
- Existing compiler/toolchain warnings remain; no new ownership-test compilation errors. Full native test build uses a separate intermediate directory, not the distributable package cache.
- Managed probe using the original locomotive GLB, node metadata intact, `Model.Copy()`, forced collection and shutdown: **exit 0**. No source-model edits or production static-visual workaround.
- Consumer Windows/Quest/Editor Release builds and affected C# suites passed (139 / 172 / 48 / 81 / 54 / 625); browser suite: 77. Windows DLL and signed Quest APK native hashes match package provenance.
- Original package-3 native DLL comparison was built in an isolated consumer output; production source pins stayed on package 6.

### Remaining acceptance blocker

The fixed package renders the full train prefab, but the longer Windows game smoke reaches its frame budget and then times out with `[skr] Failed to acquire swapchain image: 0x1`. An unchanged-range 140-frame comparison using the exact original package-3 DLL reproduces the same failure. This is the already-known renderer/lifecycle problem, outside the requested metadata-only delta; do not report clean integrated shutdown or broaden the native patch silently.

The current desktop also produced white hardware-WPF shell captures. UI automation works; a scratch-only software-WPF launcher renders correctly and passes standalone prefab authoring plus both environment-placement paths. No shipped WPF fallback was added. Hardware UI acceptance needs a healthy-desktop rerun. Physical Quest rendering/performance remains unverified.

Detailed consumer evidence/status: `D:/_repos/DefenderVRS/plans/ZIPZAP_EDITOR_PREFABS.md` and that repository's `.tmp/zipzap-editor-prefabs/`. Native source/package/test logs remain under `.tmp/model-copy-metadata/`.
