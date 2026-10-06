# Compatibility

The table below records the **0.3.0 reference baseline**. Candidate evidence and
blockers are tracked separately in [ReleaseReadiness.md](ReleaseReadiness.md).

On 2026-09-26, `1.0.0-rc.1` passed local UE 5.8.0 source-build / Win64 compilation,
40 headless tests, 8 GPU-suite tests, and real imported-asset pixel probes in packaged
Development and Shipping. This is development-tree evidence, not approval for a
release or support for a second engine version. Manual acceptance and final-commit
validation remain open.

| Area | Verified | Not yet claimed |
| --- | --- | --- |
| Unreal Engine | 5.8 source build | 5.7 and earlier; future minors |
| Host platform | Windows 11, Win64 | Linux, macOS |
| Targets | UnrealEditor Development, UnrealGame Development, UnrealGame Shipping | Consoles and mobile |
| Runtime smoke | Headless Editor automation plus automated packaged Development and Shipping launches with NullRHI, widget construction, and packed-material load markers | Interactive packaged screenshot comparison |
| Widget rendering | GPU screenshot tests against committed ground truth, reference GPU/driver | Cross-vendor GPU variance beyond the comparison tolerance |
| Descriptor formats | Text, compact XML, binary v3 | Binary v1/v2 |
| Page images | PNG | Every format accepted by TextureFactory |
| Packed atlases | Glyph coverage extraction via the bundled UI material | Separate outline-channel compositing |

The Runtime module uses platform-neutral Engine, Slate, SlateCore, UMG, and XmlParser APIs, but portability is a hypothesis until the target is compiled and exercised in CI.

The 1.0 release scope is UE 5.8.0 source build / Win64. Final validation requires at
least one complete engine build and tests every supplied build. Additional UE minors
and Launcher distributions are follow-up compatibility work; their absence does not
block this narrowly scoped release. No minimum earlier than 5.8 is established by
the existing evidence.

Precompiled archives are specific to the engine build and target configurations
recorded with the release. The verified source-build binaries are not claimed to
work with all UE 5.8 builds or the Launcher distribution. Rebuild the plugin in the
destination engine when its build does not match.

Source distributions intentionally omit `EngineVersion` from the plugin descriptor so developers can attempt newer compatible engines. That omission is not a promise of forward compatibility. Releases should state the exact tested engine CL/version and regenerate packaged binaries per engine version.
