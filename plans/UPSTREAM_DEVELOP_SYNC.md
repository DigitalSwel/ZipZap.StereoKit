# Upstream develop sync and latest consumer package

## Approved scope

User explicitly requested all latest upstream changes in our fork's develop, then selected **fork + new package + EngineLib/DefenderVRS adoption**. Commit and push the resulting source changes. Preserve the existing local NuGet-feed distribution workflow; no registry/GitHub package release or device installation. This supersedes the earlier metadata-only consumer-baseline restriction for this integration.

## Pinned starting points

- Fork develop/origin: `8108391d` (includes Model.Copy metadata ownership fix `7247a55f`).
- Fetched upstream develop: `c2c6ec14`, merge of PR #1433 / Steam Frame support. Fork has 10 unique commits; upstream has 12 unique commits.
- Existing consumer package: `0.4.0-zipzap.6.g5863048d039a` from the isolated ASTC6 backport. Do not overwrite it.
- Next unused local package revision at inspection: 7.

## Implementation

1. Create an isolated integration branch/worktree from fork develop under ignored `.tmp/upstream-develop-sync/`.
2. Merge pinned upstream preserving Git ancestry; retain our ASTC6 extension seams, released managed compatibility and Model.Copy fix. Commit the merge separately from any adaptations.
3. Inspect changed loader/build/distribution contracts, especially openxr_loader_nano, renderer/shader generation and asset synchronization.
4. Validate native extension-enabled and core-only builds/suites, CPU ASTC6 parser and managed compatibility/API tests; Windows/Android Release builds and immutable package provenance.
5. Adopt one new exact package consistently in EngineLib and all three DefenderVRS consumers. Rebuild/retest, check native Windows/APK payload identity, run original metadata-bearing model-copy and game rendering/lifecycle probes.
6. Update integration/package/consumer documentation, commit and push validated branches/repositories. Preserve unrelated changes.

## Acceptance and limitations

- All fetched upstream commits must be ancestors of our final develop, and remote source revisions must match pushed commits.
- Public extension contracts and model metadata ownership must survive the merge. Native and managed binaries must use matching source/package.
- No latest-package adoption until both platforms build and relevant tests pass; do not conceal remaining graphics/device blockers.
- Existing desktop swapchain/shutdown failure and hardware-WPF composition failure are recorded in the metadata plan; recheck after the upgrade. Physical Quest validation is not implied by packaging or desktop rendering.
- Keep this plan active if required consumer acceptance is blocked/unverified.

## Status

Active — upstream merge, package creation, consumer adoption and desktop acceptance passed. The pre-existing native audio-suite access violation and physical-device qualification remain open; do not claim all native suites or headset acceptance passed.

### Source and package

- Clean ancestry-preserving merge: `8c7fd2d263ff15194139e22e635ea431a54c285b`, from fork `8108391d` and upstream `c2c6ec14`. No merge conflicts or extension adaptation patch was needed. Validated integration branch was fast-forwarded into develop.
- All 12 fetched upstream-only commits are included; `upstream/develop` is an ancestor of our develop. ASTC6 seams, managed compatibility and independent Model.Copy metadata ownership remain present.
- Package **`0.4.0-zipzap.7.g8c7fd2d263ff`**, SHA256 **`7c00861bbb9741108ff5ccfb8bd1454e8973041d2e5840feac7edefe6a1398a0`**, built from the committed merge and copied without overwriting prior versions into the existing local feed.
- Consumers now use the newer develop renderer/shader generation (`sk_renderer/v2026.10.27`, SVSL-based shader compiler) and upstream openxr_loader_nano. The category/path `standard` remains the package's Android loader-selection contract; its contents are the new loader, not stale Khronos bytes.

### Validation completed

- Windows/Android native package builds and full package payload/provenance inspection passed.
- Native extension-enabled CTest: **2/2 passed**; core-only CTest: **1/1 passed**. CPU ASTC6 parser: **1741 checks, 0 failures**.
- In both enabled/core-only builds, asset/model/future/SH/spatial/texture-compression/texture suites passed with **0 total failures**; model ownership: **144 checks, 0 failures** in each.
- Managed compatibility/API tests: **11 passed** with extensions enabled and **11 passed** disabled, using isolated build outputs.
- Actual package probe compiled HLSL/SVSL shader assets and generated material wrappers, passed the upstream glTF image-sharing regression, copied the original metadata-bearing locomotive, rendered 30 offscreen frames and exited **0** after collection and shutdown.
- EngineLib's shared property and all three DefenderVRS package references consistently use package 7. Windows game, signed Quest APK and ZipZap.Editor Release builds passed. Windows/APK StereoKit native payloads and the APK's new OpenXR loader match package hashes.
- Consumer C# suites: **139 / 172 / 48 / 81 / 54 / 625 passed**; browser: **77 passed** (1196 total).
- Actual Windows range: **140 frames, exit 0**; full three-source prefab: **240 frames, exit 0**, with **0 swapchain-acquisition errors** in either run. Screenshot: consumer `.tmp/zipzap-editor-prefabs/game-prefab-latest.png`.
- Actual hardware-WPF standalone authoring and both environment placement paths passed, including save/undo and clean close. Complete shell/viewport captured without the earlier white-composition problem; no shipped software-rendering fallback was added. The environment automation needed foreground restoration before opening its context menu; this was scratch-only verification, not a product-code change.

### Remaining limitations

The standalone native `-audiotest` crashes with Windows `0xC0000005` in both latest extension configurations. A direct run against the pre-merge develop binary reproduces the same access violation, so this is a confirmed baseline test failure, not silently reported as passing or folded into this integration. Logs: `audio-comparison-results.log`, `audio-latest.log`, `audio-previous-develop.log`.

No physical Quest rendering/performance, Steam Frame hardware/runtime, Linux ARM64 or headset loader qualification was performed. Packaging and desktop acceptance do not establish those results. Distribution remains the user-approved existing local feed; source pushes do not transfer ignored `.nupkg` files.

Scratch/logs: native `.tmp/upstream-develop-sync/`; consumer `.tmp/zipzap-editor-prefabs/`. Build caches retain their native ignored locations.
