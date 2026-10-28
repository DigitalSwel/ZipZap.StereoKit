# SVSL shader compiler integration (StereoKit PR #1430)

## Approved scope

- Integrate pinned upstream PR #1430 commit `e3ab8e716fc8909a338e98440ba3e8068336ca66` from fork baseline `fd9c42da` on `integration/svsl-pr-1430`, preserving upstream ancestry.
- Retain ZipZap ASTC6 extensions, optional extension builds, managed compatibility, and shader-cache invalidation fixes.
- Switch all shader builds to SVSL and `sk_renderer/v2026.10.27`; remove obsolete backend switches and force glslang OFF even in existing CMake caches.
- Validate native Windows x64 / Android arm64 builds, enabled/core-only extension tests, managed compatibility, and shader compilation/code generation.
- Create and inspect a new immutable local Windows x64 / Android arm64 NuGet package from committed source, then merge locally into `develop` after validation.
- No push, hosted publication, consumer dependency changes, or device installation. Host checks do not qualify Quest visuals/performance or the known Windows minimize/restore defect.

## Execution

1. [x] Merge pinned upstream PR, preserving fork-specific changes; record integration independently.
2. [x] Review backend/dependency configuration and update current build documentation without rewriting historical validation.
3. [x] Build/test Windows extension-enabled and core-only configurations; test CPU parser and managed compatibility in both modes.
4. [x] Compile representative HLSL/SVSL assets and generated C# through imported MSBuild targets; verify shader-cache invalidation remains intact.
5. [x] Build Windows x64 / Android arm64 Release and inspect new local package payload, provenance, and compiler size.
6. [x] Review final changes, merge validated branch locally into `develop`, and move completed plan into `plans/completed/`.

## Artifacts and constraints

- All agent-created scripts, logs, extracted package probes, and temporary reports: `.tmp/svsl-pr-1430/` only.
- Tool-managed build/cache/package output retains ignored `bin/` and `.deps_cache/` contracts. Verification-only CMake configurations use `SK_DISTRIBUTE=OFF`.
- Existing NuGet versions must never be overwritten. Package provenance must identify committed runtime/build source.
- Blocked, partial, or unverified scope remains active here.

## Results

- Upstream integration/source commit: `f3718d82966d789f9cab28e61324691712f8716e`, with original upstream PR commit as its second parent. `develop` fast-forwarded to this validated merge.
- Integration changes exactly the four upstream PR files. The one conflict in `SKShaders.targets` retains ZipZap's forced stale-code generation (`-f`), dependency inputs, and generated-file clean registration while removing the obsolete backend argument.
- Windows x64 and Android arm64 Release/Java builds passed. Both package caches force `SKSHADERC_ENABLE_GLSLANG=OFF`; Windows solution contains neither glslang nor SPIRV-Tools-opt. All 57 configure preset inheritance references resolve.
- Windows compiler: 643,072 bytes, down from 5,568,000 bytes. No shader performance benchmark or platform-independent SDK-size reduction is claimed.
- Extension-enabled CTest: 2/2 passed; core-only: 1/1 passed. Asset, SH, texture compression, spatial, texture, and future suites: zero failures in each mode. Core-only project compiles no extension sources.
- CPU parser: 1741 checks, zero failures, exact 640-byte ASTC None/Zstd equality. Managed API tests: 11 checks passed in each mode.
- All 54 tracked HLSL shaders produced nonempty Debug and Release binaries: 108 compilations without `-svsl`. Native built-in and sample shader header builds passed.
- HLSL/SVSL assets and generated C# compile in Debug/Release. Incremental no-op, compiler/target/include invalidation, and clean/rebuild passed; temporary fixture imports match the integrated source.
- Immutable local package: `bin/nuget-feed/ZipZap.StereoKit.0.4.0-zipzap.5.gf3718d82966d.nupkg`, 11,899,574 bytes.
- Package SHA256: `d4df7c2cfbc6160b9b6cd0ef5f3be506c7ba327f3a1afebe8595cbb62ec81db5`. Payload/provenance hashes, packaged import/source equality, and isolated restored native output identity passed.
- Isolated package consumer resolves only the custom package; HLSL/SVSL asset loads, generated PBR materials, 16 offscreen frames, screenshot, extension P/Invoke preflight, and clean shutdown passed. Host reports ASTC6 sampling unsupported.
- Final code review and whitespace checks passed; no blocking findings. Detailed logs, commands, probes, screenshot, and review remain in `.tmp/svsl-pr-1430/`.
- Consumer dependencies and remote branches remain unchanged. No physical Quest/device shader visual or performance qualification, native ASTC6 GPU upload acceptance, or Windows minimize/restore lifecycle fix is claimed.
