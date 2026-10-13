# Release checklist

[简体中文](ReleaseChecklist.zh-CN.md)

## Scope and metadata

- Choose the version, update `UnrealBMFont.uplugin`, and replace the changelog's `[Unreleased]` heading with `[version] - YYYY-MM-DD`, using the actual publication date only when cutting the release.
- Confirm `IsBetaVersion` matches the release maturity.
- Set repository-specific documentation and support URLs only after their final public URLs exist.
- Verify every new source, fixture, image, and document has known provenance and compatible licensing.

## Verification

- Complete every required gate below. Keep local acceptance records, review reports and generated evidence outside the repository.
- Run `Scripts/TestRelease.ps1` on the exact clean release commit for every claimed engine build; at least one complete build is required. Require `Completed=true` and `ReleaseReady=true` in its evidence.
- State the exact engine build and Win64 support scope. Additional UE minors and Launcher distributions need independent validation before they are claimed.
- Review branding/material/ground-truth changes. Regenerate only after an intended change; never update reference PNGs just to make a failing comparison pass.
- Run `Scripts/BuildPlugin.ps1` for every claimed target platform.
- Confirm UnrealEditor Development, UnrealGame Development, and UnrealGame Shipping all succeed without warnings introduced by the plugin.
- Run `Scripts/TestPlugin.ps1`; archive `index.json` and the editor log.
- Repeat the full `UnrealBMFont.*` suite on a GPU (no `-NullRHI`), including the editor preview/thumbnail tests, and archive its report.
- Run `Scripts/TestPackagedRuntime.ps1` with both default configurations and archive its `Evidence` directory.
- Import `Samples/Minimal/Minimal.fnt` in a clean project and visually inspect `AB`, fallback, wrap, tint, shadow, and reimport.
- Open the asset inspector on an imported font and verify the summary, atlas overlay, and glyph table.
- Confirm both packaged configurations report real GPU success, all four pixel cases and their PNG evidence, with no force-cooked plugin content directory.
- Open the generated showcase project, press Play and inspect the multilingual rows and plain/packed/multi-page rendering.
- Load and resave representative beta assets in a disposable copy before accepting the upgrade policy.

## Package audit

- Extract the package into a clean directory.
- Confirm Runtime has no dependency on UnrealEd or AssetDefinition.
- Confirm the test module is absent from UnrealGame targets.
- Confirm the packaged descriptor has `Installed=true` and `EnabledByDefault=false`.
- Confirm the package contains `LICENSE`, the informational Chinese license translation, both language versions of the notices and all documentation, `Content/M_BMFontPacked.uasset`, the showcase template and exactly its two generation helpers.
- Confirm `BuildPlugin.ps1` reports that its automated release-file and personal/source-path audit passed.
- Scan for credentials, private URLs, stale vendor names, and generated host-project files.

## Publish

- Tag the exact tested commit.
- Require independent review acceptance and final release-workflow evidence; a green metadata-only CI run is not release approval.
- Attach immutable checksums for release archives.
- Publish the compatibility matrix and known limitations with the release notes.
- Do not claim a platform, engine version, cook, or runtime result that was not exercised for this release.
