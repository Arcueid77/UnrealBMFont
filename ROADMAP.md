# Roadmap

[简体中文](ROADMAP.zh-CN.md)

The roadmap records direction, not a compatibility promise.

## Delivered in 0.3

- Bound parser work and imported atlas declarations with explicit, configurable limits.
- Keep plain-widget brush creation proportional to the glyphs in the current layout and retain a large-glyph-set regression baseline.
- Automate Development and Shipping cook, package, launch, and deterministic runtime verification.
- Make GPU screenshot execution an explicit self-hosted CI capability.

## Delivered in 1.0

- Repair live asset/resource cache invalidation, including late material recovery.
- Verify imported assets through save, reload, dependency cooking and packaged GPU rendering.
- Share a runnable showcase with the automated package probes.
- Document the public API, serialized schema and non-destructive beta upgrade path.
- Require validation of the exact commit for every claimed engine build and independent review before publication.
- Keep local acceptance records outside the repository; state verified compatibility and limitations in the release notes.

## After 1.0

- Validate a second Unreal minor and an official Launcher installation before expanding the verified engine matrix; publish upgrade notes for any API differences.
- Validate Linux and macOS builds and runtime rendering before claiming those platforms.
- Add optional import policies for texture filtering and texture asset placement.
- Add a continuously running coverage-guided parser fuzz target when the build infrastructure can retain and minimize corpora.

## 1.0 criteria

- Public API and serialized asset versioning policy.
- Verified UE 5.8.0 source-build / Win64 compatibility, with exact engine build details in the release evidence.
- Required validation gates for Editor, Development, Shipping, automation, cook, and packaged runtime smoke tests on every release commit, run in CI or through the same local release script with retained evidence.
- Documented upgrade guidance for every breaking beta change.

Bidirectional text and OpenType shaping are not planned for the core renderer: those jobs require a shaping engine and source font data, which BMFont does not contain.
