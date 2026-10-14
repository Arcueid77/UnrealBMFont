# Changelog

[简体中文](CHANGELOG.zh-CN.md)

All notable changes to Unreal BMFont are documented here. The project follows [Semantic Versioning](https://semver.org/) and the structure of [Keep a Changelog](https://keepachangelog.com/).

## [Unreleased]

## [1.0.0] - Unreleased

### Added

- Native asset-change notifications for live Slate/UMG consumers and resource-only cache revisions.
- Serialized BMFont schema identity and documented public API and upgrade policy.
- Runnable showcase template with plain, packed, multi-page and multilingual imported assets.
- Packaged Development/Shipping GPU pixel probes using map-owned font references and normal dependency cooking.
- Release validation for every claimed engine build, with source fingerprints and archived evidence; the initial scope is UE 5.8.0 source build / Win64.

### Changed

- Completed English and Simplified Chinese documentation, including sample guides and contribution forms; local acceptance records and review reports remain outside the repository.
- Final release validation repeats the complete automation suite with a real GPU, including editor atlas-preview and thumbnail tests.

### Fixed

- Existing brushes recover after a previously unavailable packed material becomes available.
- Asset metric changes invalidate existing plain and Rich Text layouts, including cached widget trees.
- Automation rejects incomplete results; strict GPU validation rejects NullRHI skips.
- The showcase explicitly enables the Editor-only Content Browser asset source and engine asset definitions.
- Font inspector summary rows are attached to the panel, and atlas previews use the theme background tint so white glyphs remain visible.
- Packed atlas previews and Content Browser thumbnails show the page's glyph coverage channels, including RGB glyphs with zero atlas alpha.
- Imported fonts expose source-file registry metadata and enable the standard Content Browser reimport actions.

## [0.3.0] - 2026-09-03

### Added

- Configurable `FBMFontParserLimits` for descriptor size, text/XML shape, pages, glyphs, kerning pairs, page names, per-page atlas dimensions, and total multi-page atlas pixels.
- Deterministic malformed-input coverage for truncated binary descriptors, impossible block sizes, fixed-seed byte corpora, diagnostic flooding, and XML `DOCTYPE` rejection.
- A large-glyph-set regression test proving the plain Slate widget creates brushes only for glyphs used by its current layout.
- `TestPackagedRuntime.ps1`, which builds a clean C++ host, cooks and packages Development/Shipping targets, launches each package, and verifies both `UBMFontText` and the bundled packed-channel material through a deterministic marker.
- Optional GPU screenshot execution in the self-hosted Unreal CI job through `UE_GPU_CI_ENABLED=true`.

### Changed

- Plain-text brush caches now follow the current layout revision instead of eagerly creating one brush for every glyph in the asset.
- Packed-channel assets resolve their base material during asset lifecycle updates, keeping synchronous material loading out of the normal first-paint path.
- Release packages keep licensing, documentation, samples, and runtime content while excluding repository-only scripts.
- Release scripts wait for the engine-wide AutomationTool mutex so concurrent Unreal automation does not fail a local release run at startup.
- Repository validation derives plugin version checks from the descriptor instead of a hard-coded release number.

### Fixed

- Atlas rectangle and UV calculations use overflow-safe arithmetic and return a zero UV rectangle for invalid public API input.
- The importer rejects oversized descriptor files before loading them into memory, and binary page file-name limits are enforced before UTF-8 conversion.
- Parser diagnostics are capped so malformed descriptors cannot grow an unbounded message list without turning warning-only descriptors into import failures or hiding suppressed errors.
- Parser page-count limits no longer reject valid non-contiguous page IDs.
- Packed materials get one bounded render-time retry after a transient lifecycle load failure.

### Security

- XML descriptors containing a `DOCTYPE` declaration or exceeding configured element/attribute limits are rejected before DOM materialization.
- Default parser limits bound CPU and memory growth from untrusted descriptors while remaining overrideable for explicit C++ use cases.

## [0.2.0] - 2026-08-24

### Added

- Channel-aware rendering for packed-channel (`packed=1`) atlases: glyphs sharing a page can select independent `char.chnl` masks through cached instances of the bundled `M_BMFontPacked` UI material. The asset's **Packed Render Material** property overrides the material per asset.
- Packed descriptor import with sRGB-disabled page textures and a warning when reimport preserves an sRGB-enabled texture.
- Transactional multi-page reimport that stages every image before commit and rolls back destination textures on failure.
- `UBMFontRichTextBlock`, a rich text adapter that renders tagged (and optionally plain) runs with a BMFont asset. Kerning resolves within each run, empty tags stay empty, and ellipsis overflow policies are rendered with asset glyphs.
- Editor thumbnails for BMFont assets, drawn from the first atlas page.
- A read-only font/atlas inspector (double-click an asset) with a descriptor summary, an atlas preview with glyph rectangles and click-to-select, and a glyph table.
- Screenshot-based render tests (`UnrealBMFont.Render.*`) that draw widgets off-screen and compare against committed ground truth, including per-glyph packed channels and rich-text ellipsis. They skip cleanly under NullRHI; ground truth regenerates with `-UpdateBMFontGroundTruth`.
- `TestPlugin.ps1 -DisableNullRHI` for GPU runs.

### Changed

- The plugin descriptor now allows content (`CanContainContent`) to ship `M_BMFontPacked`.
- Rich-text brush caches are scoped to glyphs used by each run and refreshed by asset revision.
- Packed assets persist the bundled render-material dependency even when the override was cleared before saving.
- Rich-text foreground colors now honor inherited Slate style colors; clipping, ellipsis, and shadow bounds match the effective paint region.
- The asset inspector refreshes its summary, pages, atlas, and glyph rows after successful reimport.

## [0.1.0] - 2026-08-05

### Added

- Initial Unreal BMFont plugin release for importing and rendering AngelCode BMFont assets.
- Runtime parser for AngelCode BMFont text, XML, and binary v3 descriptors, with consistent duplicate-kerning handling (the last record wins and emits a warning) in all three formats.
- Serializable font asset model with multiple pages, Unicode glyphs, and kerning.
- Plain-text Slate and UMG bitmap-font widgets.
- Wrapping, justification, margins, line-height behavior, spacing, tint, shadow, fallback glyphs, and pixel snapping.
- Deterministic layout that reuses the asset's kerning lookup, keeps trailing-whitespace metrics consistent with and without wrapping, and never reads past a bounded string view.
- Per-line paint culling for off-screen lines and correct volatility tracking for bound tint attributes.
- Editor importer, page texture import, source tracking, and reimport that validates every atlas image before touching existing textures.
- Modern Asset Definition integration.
- Parser, layout, import, and reimport automation tests.
- Opt-in code-plugin packaging support for C++ and content-only projects.
- Build/test scripts that stage a clean plugin source copy and keep default outputs outside the repository, preventing local engine caches and old artifacts from being copied into UAT hosts.
- Original Minimal and multilingual Showcase sample fixtures; Showcase covers digits, uppercase Latin, the `U+3041`–`U+3096` Hiragana range, Japanese `ー`, Chinese numerals, and fallback rendering.
- Bilingual documentation, CI template, and project governance files.
