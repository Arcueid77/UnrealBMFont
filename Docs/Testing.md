# Testing

[简体中文](Testing.zh-CN.md)

## Automation coverage

The `UnrealBMFont.*` suite covers:

- Text descriptors with BOM, quoted names, arbitrary field order, multiple pages, and supplementary Unicode code points.
- Compact XML descriptors with the declaration and document on one line.
- Generated binary v3 blocks, signed offsets, and kerning.
- Invalid metrics and atlas validation.
- Configurable descriptor, text/XML shape, record, page-name, atlas-dimension, and total multi-page pixel limits.
- Truncated binary prefixes, impossible block sizes, fixed-seed malformed byte corpora, diagnostic flooding, and XML `DOCTYPE` rejection.
- Kerning, tracking, glyph offsets, wrapping, explicit lines, tabs, fallback, missing glyphs, and line-height semantics.
- A real generated PNG plus `.fnt` import through `UBMFontFactory`.
- The packaged multilingual Showcase fixture, including representative Latin, Hiragana, Chinese, and fallback code points.
- UI texture defaults, source path tracking, transactional multi-page reimport/rollback, and preservation of user-edited texture filtering.
- Slate desired-size calculation and invalidation after text changes.
- Layout-scoped brush creation against a generated 20,000-glyph asset.
- Asset metric notifications, Slate reconstruction, packed material recovery and raw Slate weak-reference garbage collection.
- Resource-only brush invalidation in existing rich-text runs and the schema GUID in saved packages.
- Rich text run measurement through `UBMFontRichTextBlock`, including kerning, fallback, tagged and empty runs, runtime font assignment, inherited foreground colors, shadow bounds, and screenshot coverage for parent-clipped ellipsis painting.
- Packed-channel mapping, packed import/reimport through `UBMFontFactory`, packaged material dependency on real save, and render-resource separation by page and glyph channel.

## Screenshot render tests

The `UnrealBMFont.Render.*` tests draw `SBMFontText` off-screen through `FWidgetRenderer` and compare pixels against ground-truth PNGs in `Samples/GroundTruth`. They require a real RHI: under `-NullRHI` they report an informational skip and pass, so the default headless run stays green.

Run them on a GPU:

```powershell
& "C:\Path\To\Engine\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "D:\Projects\BMFontHost\BMFontHost.uproject" `
  -Unattended -NoSplash -NoSound -NoP4 -NoPause -stdout -FullStdOutLogOutput `
  -ExecCmds="Automation RunTests UnrealBMFont.Render" `
  -TestExit="Automation Test Queue Empty" `
  -ReportExportPath="$env:TEMP\UnrealBMFont-RenderReports"
```

Ground truth lives in the repository and is treated as source. Regenerate it deliberately after an intended rendering change, on the reference GPU/driver, and review the PNG diff:

```powershell
# Same command as above, plus:
  -UpdateBMFontGroundTruth
```

Comparison allows a small per-channel delta and a small mismatching-pixel ratio so minor driver or sampler differences do not flake; a structural rendering change still fails. `UnrealBMFont.Render.PackedAtlas` exercises overlapping glyph rectangles stored in different channels, and `UnrealBMFont.Render.RichTextEllipsis` verifies the custom rich-text paint path honors overflow policy.

## Run against a host project

The host project must have the current plugin installed and enabled.

```powershell
pwsh ./Scripts/TestPlugin.ps1 `
  -EngineRoot "C:\Program Files\Epic Games\UE_5.8" `
  -Project "D:\Projects\BMFontHost\BMFontHost.uproject" `
  -ReportDirectory "$env:TEMP\UnrealBMFont-Reports"
```

The script runs `UnrealEditor-Cmd` with `NullRHI`, waits for the automation queue to empty, and fails if the process or JSON report reports a failure.

## Package verification

```powershell
pwsh ./Scripts/BuildPlugin.ps1 `
  -EngineRoot "C:\Program Files\Epic Games\UE_5.8" `
  -OutputDirectory "$env:TEMP\UnrealBMFont-Package" `
  -TargetPlatforms Win64
```

The wrapper first creates a clean source staging copy outside the repository, excluding local engine trees, build products, reports, and previous packages. Its internal host and package live under a shared neutral build root (override with `-BuildRoot`), so compiler paths do not expose a developer profile or source checkout. This matters because UE 5.8's `BuildPlugin` command copies the complete input plugin to its temporary host before applying `FilterPlugin.ini`.

Both release scripts wait for an already-running AutomationTool process instead of failing the release when another Unreal build temporarily owns the engine-wide UAT mutex.

Before copying to `OutputDirectory`, the wrapper verifies that the packaged descriptor is both installed and opt-in, checks the required license/readme/compatibility files, strips PDBs by default, and scans every packaged file for the user-profile and source-checkout paths. Raw `BuildPlugin` removes `EnabledByDefault`; publishing that uncorrected descriptor from a project-local package can prevent a content-only project from generating a plugin-aware game target.

Do not describe a static source scan as a successful runtime test. A release checklist should retain the BuildPlugin log, automation `index.json`, cook log, and packaged runtime evidence.

## Packaged runtime verification

After `BuildPlugin.ps1` succeeds, run:

```powershell
pwsh ./Scripts/TestPackagedRuntime.ps1 `
  -EngineRoot "C:\Program Files\Epic Games\UE_5.8" `
  -PluginPackage "$env:TEMP\UnrealBMFont-Package"
```

The script generates the [runnable showcase](../Examples/Showcase/README.md) outside the repository, imports and saves real fixtures, and cooks the showcase map's asset dependencies. It does not force-cook the plugin directory. Development and Shipping launch on a real GPU and compare plain, packed, multi-page and Rich Text pixels against independently specified fixture masks. Each configuration writes four probe PNGs, a full `Showcase.png`, and `result.json`; missing cases, mismatched pixels, missing assets and NullRHI all fail. An explicit `-WorkDirectory` must not already exist, preventing stale evidence from satisfying a new run. Keep the complete `Evidence` directory, including the import/build/cook logs.

The default headless automation still skips GPU screenshot cases. Run a second pass with `-DisableNullRHI -TestFilter 'UnrealBMFont.Render' -Strict` to require actual rendering. `-Strict` also rejects warning results; every mode rejects incomplete reports.

## Final release validation

`Scripts/TestRelease.ps1` runs package audits, both packaged GPU configurations, full automation and GPU automation for each supplied engine. Final mode requires a clean commit and at least one complete Win64 engine build; a source build is sufficient. Every supplied build must pass. It records each engine's version and installed-build marker, checks that the source commit and input fingerprint stay unchanged, and writes `release-evidence.json`. Development runs with `-CandidateOnly`, prerelease versions and beta descriptors explicitly remain non-release evidence even when `AutomatedGatesPassed` is true. See [Release checklist](ReleaseChecklist.md). Keep generated evidence and local acceptance records outside the repository; automated success alone does not complete interactive acceptance or authorize publication.

`Scripts/TestScriptGuards.ps1` exercises output-tree, stale-directory and empty-configuration rejection without starting a build. Supply the reference engine and a built plugin package. `Scripts/TestAssetUpgrade.py` checks a copied beta asset across two Editor processes; see [Upgrading.md](Upgrading.md).

## GitHub Actions

The dependency-free repository validation job runs on every push and pull request. The development UE 5.8 job is opt-in; pull requests never execute on its persistent self-hosted runner. Its green metadata result is not final release approval.

To enable the Unreal job, register a Windows x64 self-hosted runner with the `ue-5.8` label, then set these repository variables:

- `UE_CI_ENABLED=true`
- `UE_ROOT` to the Unreal Engine 5.8 installation or source-tree root on that runner

The runner now needs a usable GPU for packaged pixel validation. `UE_GPU_CI_ENABLED=true` adds the Editor screenshot suite to the development job. The separate manually dispatched `Release validation` workflow requires at least one complete engine build and every GPU/package gate; it has no optional GPU skip and never publishes a release automatically.
