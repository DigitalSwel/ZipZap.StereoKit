# ZipZap extensions on an upstream StereoKit core

## Ownership

StereoKit owns the texture upload/asset scheduler, KTX2 Basis transcode path, rendering, materials, and shutdown. ZipZap adds a bounded native ASTC6 sRGB KTX2 decoder and a concrete format-sampling query. The addition still loads transparently through normal `Tex.FromFile` / `Tex.FromMemory`; it does not create another runtime or a general plugin system.

- `StereoKitC/zipzap/`: private ASTC6 parser/loader adapter and the extension's native API.
- `StereoKit/ZipZap/`: managed preflight entry point, P/Invoke declaration, released-fork compatibility overloads, and build selection.
- `cmake/ZipZapExtensions.cmake`: optional native sources and first-party extension test targets.
- `tools/tests/ktx2_astc6_tests.cpp`, `astc6_loader_tests.cpp`, `ZipZapApiTests/`, and `fixtures/astc6/`: permanent extension coverage.
- `tools/Build-ZipZapPackage.ps1`: one immutable package containing the matching managed assembly, Windows/Android native libraries, and provenance.

The upstream loader contains only compile-time adapter calls and an opaque extension handle. ASTC6 format decisions, GPU checks, parsing, size limits, and Zstd validation stay in the extension. Recognized but invalid or unsupported native content fails; it must never fall through to another decoder. Non-ASTC6 data uses the upstream path unchanged.

The adapter owns its opaque handle. The upstream loader owns the decoded output allocation and source-byte lifetime. Cancelling before decode must release the handle without freeing or retaining the caller's output.

## Deliberate upstream seams

- `texture_compression.cpp`: conditional info/begin/step/end dispatch only; no native ASTC6 parsing or format policy.
- `stereokit.h`: one compatibility include for the separately declared extension API.
- `Tex.cs`: `partial` class declaration only.
- `NativeEnums.cs`: obsolete aliases for the released `Uncompressed`, `Quality`, and `Small` enum names, retaining their bit values.
- Root CMake/project files import the extension build modules. Native test runner retains its `-astc6test` entry point.

The old flags-first `SetColors`, `SetMemory(..., blocking, ...)`, `FromData`, and three-argument cubemap reflection binary entry points remain in the managed compatibility file. New code should use upstream names and format-first overloads. The compatibility format parameter is named `legacyDataFormat` to avoid ambiguous overload resolution for fully named upstream calls. For old mixed calls that pass flags positionally and `dataFormat:` by name, move the format before flags or rename that argument to `legacyDataFormat:`.

This is not compatibility with arbitrary old C ABIs. Upstream changed native function signatures; native and managed binaries must always come from the same source revision/package.

## Build selection

This fork enables extensions by default. Use matching native and managed settings:

```text
Native:  -DSK_ZIPZAP_EXTENSIONS=ON   Managed: -p:ZipZapExtensions=true
Native:  -DSK_ZIPZAP_EXTENSIONS=OFF  Managed: -p:ZipZapExtensions=false
```

Disabled native builds omit extension sources and loader branches. Disabled managed builds omit `StereoKit/ZipZap/*.cs`; the upstream APIs remain. The native sampling-query declaration is available through the compatibility header, but its implementation requires an enabled native build. Do not call extension APIs against a core-only library.

Package builds explicitly enable both sides, including when an existing CMake cache previously had another value. `-PackOnly` requires extension-enabled native caches. Core-only validation must use `SK_DISTRIBUTE=OFF` so it cannot replace distributable package inputs.

## Validation

- `SKKtx2Tests`: CPU parser/range/DFD/Zstd framing and exact None/Zstd output checks.
- `SKTests -astc6test`: public asynchronous load, info/decode agreement, invalid recognized input, cooperative decode/cancel, and UASTC fallback. Run with native extensions both ON and OFF.
- `dotnet run --project tools/tests/ZipZapApiTests -p:ZipZapExtensions=true` and again with `false`: API selection, enum values, and compile-time compatibility calls without starting graphics.
- Run relevant upstream native suites, managed build, Windows/Android release builds, and package payload inspection.

Unsupported desktop GPUs must reject native ASTC6 explicitly while still loading UASTC. They cannot qualify ASTC6 upload/visuals on a physical Quest. Host builds are not headset visual, lifecycle, or performance acceptance.

## Upstream workflow

1. Track upstream refs without adding ZipZap commits to those refs.
2. Create an integration branch from our `develop`; merge a pinned upstream revision, retaining Git ancestry.
3. Resolve integration changes first and record the merge independently.
4. Make extension adaptations/refactors in a separate commit. Keep implementation out of the upstream files above.
5. Validate enabled and core-only configurations before merging locally into our `develop`.
6. Build a new immutable local package from committed source. Update consumers only under separate approval.

No package version is overwritten. Git commits do not distribute the ignored local feed. Push, hosted publication, consumer updates, and device installation require separate approval.
