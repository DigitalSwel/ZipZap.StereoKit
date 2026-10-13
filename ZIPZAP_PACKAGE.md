# ZipZap.StereoKit local package

Shared fork for Reveal and GargsVR. Upstream source remains in the `StereoKit` namespace and assembly; our NuGet package ID is **ZipZap.StereoKit**.

## Baseline and supported build

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

## Current validation and limits

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
