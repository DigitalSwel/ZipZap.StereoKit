# ZipZap.StereoKit local package

Shared fork for Reveal and GargsVR. Upstream source remains in the `StereoKit` namespace and assembly; our NuGet package ID is **ZipZap.StereoKit**.

See [ZipZap extension boundaries and upstream updates](ZIPZAP_EXTENSIONS.md) for the optional native/managed extension build, compatibility boundary, and integration workflow. Package builds explicitly enable the extensions on both native platforms and in the managed assembly. The original package adoption/consumer validation recorded below does not automatically apply to a newer integration or package.

## Latest upstream integration (package 7)

- Current DefenderVRS/Engine package: **`ZipZap.StereoKit/0.4.0-zipzap.7.g8c7fd2d263ff`**, source merge **`8c7fd2d263ff15194139e22e635ea431a54c285b`**. It combines our fork develop with all fetched upstream commits through **`c2c6ec14`** (PR #1433).
- Package SHA256: **`7c00861bbb9741108ff5ccfb8bd1454e8973041d2e5840feac7edefe6a1398a0`**. The existing ignored local `bin/nuget-feed/` remains the distribution mechanism; no registry/GitHub package release was introduced.
- Includes the newer develop renderer/shader generation (`sk_renderer/v2026.10.27`, SVSL compiler), openxr_loader_nano, glTF image-sharing and asset-synchronization fixes, plus our ASTC6 compatibility/extensions and Model.Copy metadata ownership fix.
- Windows/Android package and consumer Release builds passed. Enabled/core-only CTest: 2/2 and 1/1; ASTC6 parser: 1741 checks; managed API: 11 checks in each mode; model suite: 144 checks in each mode. Non-audio native suites and 1196 consumer/editor/browser regressions passed.
- Actual package HLSL/SVSL/glTF/model-copy/offscreen smoke exits 0. Actual game range (140 frames) and prefab (240 frames) both exit 0 with no swapchain-acquisition errors. Hardware-WPF authoring and both placement paths pass. Windows/APK native libraries and the new APK loader match package provenance.
- Standalone audio tests still reproduce a pre-existing `0xC0000005` on both old and new develop; physical Quest/Steam Frame and Linux ARM64 qualification remain unverified. See [active integration plan](plans/UPSTREAM_DEVELOP_SYNC.md).
- On another machine, provision this exact immutable `.nupkg` and verify its hash, or build a new revision from the pinned source and update consumer pins together. Do not rebuild a moving develop under this existing version or assume Git distributes the local feed.

## Previous DefenderVRS metadata backport (package 6)

- Consumer package: **`ZipZap.StereoKit/0.4.0-zipzap.6.g5863048d039a`**, source **`5863048d039a9a194549040d88e964182ae30504`** on `fix/model-copy-metadata-astc6`.
- Package SHA256: `f95436fb2693d64462166c1f49bbd92e6c45831d2a75181aff6cf5f523681cd0`; provisioned in the existing ignored `bin/nuget-feed/`. The user chose to retain this local-feed workflow, not add a registry or GitHub package release.
- Fixes independent ownership of copied model-node metadata. The equivalent develop fix is `7247a55f010f6177f483a8b70ecb3ca31847a895`; both source branches are covered by the user's push approval.
- The package intentionally backports only that fix onto `cf6986582d77`, preserving `sk_renderer/v2026.10.12` and the existing shader generation. Do **not** rebuild this consumer baseline from current develop, which contains newer integrations.
- Windows x64 / Android arm64 package builds and provenance checks passed. Model/asset suites passed on develop and the backport; **144 model checks, 0 failures**. The original metadata-bearing locomotive copy probe now exits 0, and Windows/Quest output hashes match package payloads.
- Full desktop acceptance remains blocked by the known swapchain/shutdown failure, also reproduced with the unchanged package 3. Physical Quest qualification remains open. That checkpoint's blocker passed the package-7 desktop recheck; see the [completed ownership record](plans/completed/MODEL_COPY_METADATA_OWNERSHIP.md).
- Git push publishes source, not the ignored feed. On another machine, provision the exact revision-6 `.nupkg` and verify its SHA256. Alternatively, build a new immutable package revision from the pinned backport source and update all consumer pins together; do not silently replace revision 6 with rebuilt bytes.

## Previous local shader compiler integration (package 5)

- Integrated upstream PR #1430 commit `e3ab8e716fc8909a338e98440ba3e8068336ca66`; tested merge/source commit **`f3718d82966d789f9cab28e61324691712f8716e`**, merged locally into `develop`.
- New local package: **`ZipZap.StereoKit/0.4.0-zipzap.5.gf3718d82966d`**, in `bin/nuget-feed/`.
- Package SHA256: `d4df7c2cfbc6160b9b6cd0ef5f3be506c7ba327f3a1afebe8595cbb62ec81db5`.
- `sk_renderer/v2026.10.27` uses SVSL for all shaders. HLSL and SVSL remain supported inputs; `SK_USE_SVSL` / `SKUseSVSL` no longer select a backend. glslang is forced OFF even in previously configured caches.
- Windows shader compiler shrank from **5,568,000 to 643,072 bytes**. Native ASTC6 extensions, compatibility APIs, and shader-cache invalidation remain intact.
- Windows x64 / Android arm64 Release builds and package payload/provenance checks passed. Extension-enabled/core-only native suites passed, CPU parser passed 1741 checks, and managed API tests passed 11 checks in each mode.
- All 54 tracked HLSL shaders compiled in Debug and Release. HLSL/SVSL asset and generated-C# builds, incremental no-op, compiler/target/include invalidation, and clean/rebuild passed.
- An isolated consumer restored only this package, verified native payload identity, and rendered generated HLSL/SVSL PBR materials offscreen for 16 frames with screenshot and clean shutdown.
- No consumer pins, APKs, device installs, or remote branches changed. Physical Quest visuals/performance and the known Windows minimize/restore defect were not requalified.
- Completed scope and results: [SVSL integration plan](plans/completed/SVSL_SHADER_COMPILER_INTEGRATION.md).

## Previous local extension integration

- Integrated upstream PR #1429 head `2b6ef4b2084ee9054d1bbe2f170f323c6545034f`; upstream merge commit `60375acb` and isolated extension source commit `73380854999375d817ebb0963fa8aa3e92d933ae`.
- New local package: **`ZipZap.StereoKit/0.4.0-zipzap.4.g733808549993`**, in `bin/nuget-feed/`.
- Package SHA256: `bdeb31fef773ad8408fd8b583abb1af830b2add5177ccf910f99c01fd5965809`.
- Windows x64 / Android arm64 Release builds and package payload/provenance inspection passed. Native core-only and extension-enabled tests passed; managed API tests passed 11 checks in each mode; CPU parser passed 1741 checks.
- An isolated Windows offscreen probe using the extracted package passed API checks, P/Invoke preflight, unsupported native ASTC6 rejection, both UASTC fixture loads, and shutdown.
- Consumer pins, APKs, and device installs were not changed. Physical Quest ASTC6 upload/visual acceptance and the earlier minimize/restore lifecycle gap were not requalified.
- Completed scope and results: [upstream/extension integration plan](plans/completed/UPSTREAM_EXTENSIONS_INTEGRATION.md).

## Original package baseline and supported build

- Source baseline: `b8cc3af2619c012ef47579f424756508879f77b1` on `develop`.
- Current local consumer pin: **`ZipZap.StereoKit/0.4.0-zipzap.2.gb8cc3af2619c`**.
- Package SHA256: `13d463b10179fd85542a3b71f960f8ae41a6ab4a94d67a541ba11da3498665f8`.
- First build host: Windows x64, Visual Studio Community 2026 / MSVC 14.51, bundled CMake 4.3.1 and Ninja.
- Managed SDK: 9.0.313 from `global.json`; managed target remains `net8.0`.
- Android NDK: installed r26d (`26.3.11579264`). Upstream native preset uses API 32 and requires installed SDK platform `android-32` for its Java library. Consumer target frameworks are not changed.
- JDK 21 supplies `javac` and `jar`.
- Package runtimes: **win-x64 and android-arm64 only**. Shader compiler host: **Windows x64 only**, with HLSL/glslang support retained.
- No Linux/macOS/Windows ARM64/Android x64 support is claimed by this package.

The checkout should be adjacent to consumers, for example:

```text
D:/_repos/
  ZipZap.StereoKit/
  Reveal/
  GargsVR/
```

## Build and validate package

From this checkout, using Windows PowerShell:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Build-ZipZapPackage.ps1 `
  -NdkPath D:/Android/android-sdk/ndk/26.3.11579264 `
  -JavaHome 'C:/Program Files/Java/jdk-21' -Revision 3
```

The script discovers installed Visual Studio CMake/Ninja, builds both native runtimes and the Windows shader compiler, then packs/inspects the package. It does not install or upgrade toolchains, publish a registry package, or change user environment variables permanently.

`-PackOnly` is intended for an already-built, verified source checkout, not for mixing binaries from another revision. The current feed already contains revisions 1 and 2; `-Revision 3` creates the next immutable revision at this source SHA. Existing feed versions are never overwritten. Revision 1 is superseded by revision 2's shader-cache fix; consumers must pin revision 2, not restore revision 1 from an existing cache. Runtime/native source must match a committed HEAD. Packaging edits may remain uncommitted during initial local setup.

Outputs:

- Native build/intermediates: upstream ignored `bin/` layout.
- CPM source cache: upstream ignored `.deps_cache/`.
- Staged package/build provenance: `bin/zipzap-package/`.
- Validated local feed: **`bin/nuget-feed/`**.
- Build logs: **`.tmp/stereokit-package/`**, ignored and never committed.

Version format: `<native-major>.<minor>.<patch>-zipzap.<revision>.g<12-character-source-SHA>`. The package records the full source commit and SHA256 hashes of both native libraries, the Android OpenXR loader/Java support, and the shader compiler in `zipzap/provenance.json`. Package metadata also records the repository and full commit.

## Packaging-only changes

- NuGet automatic imports use `build/$(PackageId).props` and `.targets`; this makes the custom package ID import the same upstream asset/shader targets correctly.
- Android x64 native item is conditional on its file existing, since this package intentionally ships arm64 only.
- Packaging validates the required two-platform payload, includes the upstream MIT license and build provenance, and rejects unintended runtime payloads in a partial build.
- Embedded shader code generation tracks the compiler, imported target and standard include headers as MSBuild inputs. Compilation is forced when those dependencies are stale; generated C# is registered for supported clean/rebuild. This prevents old cached water bytecode surviving a package upgrade.
- No native or managed runtime source patch, shader math change, or consumer framework upgrade is introduced by this setup.

## Consumer adoption

Consumers use a repository-scoped `NuGet.Config` pointing at `../ZipZap.StereoKit/bin/nuget-feed`. `ZipZap.StereoKit` is mapped only to this local feed; other packages continue to use nuget.org. No global NuGet source or credentials are modified.

All relevant Reveal/GargsVR host, presentation, validation/probe and ZipZap.EngineLib references must use the same exact custom version. Merely creating the fork or package does not switch consumers. Do not keep an official `StereoKit` dependency alongside the custom package: both contain the same managed assembly/native library identities.

Before adopting a new version, restore/build both Windows and Quest hosts, run applicable CPU tests, inspect resolved assets, and perform isolated native smoke checks. Native tests are not built by the package script; headset visuals/performance and device install are separate validations. No headset data reset or production install is part of packaging.

## Original consumer validation and limits

- Full native build/pack script completed and inspected revision 2. Both consumer Windows Debug/Release and Quest Release builds passed; Gargs native probe/Quest validation hosts also built.
- Both consumer/engine dependency graphs resolve exactly revision 2 with no official `StereoKit` dependency. Restored payload hashes, both Windows output DLLs and both signed Quest APK native runtime/OpenXR entries match package provenance.
- Gargs CPU: parity **1470/1470**, environment **136/136**. Engine StereoKit **24/24**, engine VFX **81/81**. Reveal logic **719/720**: unchanged environment XML contains four objects while its unchanged content test expects three; no content/test patch was made.
- Isolated Gargs Windows 120-frame smoke: exit 0, VFX initialized, clean shutdown and screenshot. Native-only probe close without minimize: exit 0.
- **Windows minimize → restore → close still fails with `0xC0000005` (`-1073741819`), reproduced in both Gargs and the native-only probe without game/engine resources. Latest source has not resolved this lifecycle defect.** Native debugging/fixes are outside this setup's approval.
- Quest APK build is not headset startup, visual, lifecycle, comfort or performance validation. No device install/reset was performed. Reveal's Windows runtime was not launched; Windows binary identity and builds were verified.

Source commit/push is separately approved on fork `develop` and engine/consumer `main`. Both consumer engine gitlinks use shared-main commit `2849ad3238ddd0c7897460bb7d7bf7eca82f5130`; Reveal includes the two already-published diorama/VFX-hit-response changes. Reveal builds passed again; logic remains 719/720 with the same known content mismatch, while engine environment 162/162 and VFX core 230/230 passed.

Git publication does **not** distribute the ignored local feed. For another machine, provision the exact pinned revision-2 `.nupkg` in `bin/nuget-feed/` and verify the recorded package SHA256, or build a new immutable package at committed fork HEAD and update all consumer pins together. A new committed HEAD gives a new SHA-suffixed package version; building there does not automatically recreate revision 2's old baseline suffix. Release/runtime acceptance is incomplete; Gargs tracks remaining checks in `plans/STEREOKIT_FORK_PACKAGE.md`.

## Future updates

Our `develop` is the default fork branch. Future fixes and upstream merges should be tested on separate branches before merging into our development branch and releasing a new package revision. Preserve an exact source SHA for each release; never let a moving branch silently change an existing package version. Commit/push, hosted feed publication and CI automation require separate authorization.
