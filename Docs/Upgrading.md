# Upgrading to 1.0

[简体中文](Upgrading.zh-CN.md)

1. Back up the project and preserve the source `.fnt` and atlas images.
2. Close the Editor and replace the plugin with a package built for the destination
   engine, or rebuild from source. Do not mix editor DLLs and precompiled game libraries
   from different releases or engine builds.
3. Open a copy of the project. Existing BMFont asset and widget property names are
   retained; no class redirects or bulk rename operation is required.
4. Save representative assets, then verify plain/Rich Text rendering, wrapping,
   packed channels, multi-page fonts and reimport in the copied project.
5. Package Development and Shipping. Confirm the font, page textures and packed
   material are cooked through asset references. The runnable example demonstrates
   this without `DirectoriesToAlwaysCook`.

For repeatable upgrade evidence, copy a beta asset and its dependencies into a
disposable generated project at their original `/Game` paths. Run
`Scripts/TestAssetUpgrade.py` in two separate Editor processes with
`BMFONT_UPGRADE_ASSET` set to its object path: first with
`BMFONT_UPGRADE_PHASE=resave`, then `verify`. The test compares every glyph metric,
the codepoint set, font metadata, spacing, channel mapping, kerning and page texture
references across a fresh reload.
It never authorizes modifying the original project.

## C++ callers

Use `SetFontData` and `SetPackedRenderMaterial` instead of assigning those fields after
widgets have been created. Asset changes now notify existing widgets automatically.
Custom subscribers can use `OnChanged()`; they must respect its game-thread and
non-reentrant contract. See [API.md](API.md).

## Scripts and evidence

`TestPackagedRuntime.ps1` now requires a GPU and checks real imported assets and pixels.
Its evidence is `Evidence/<Configuration>/result.json` plus PNGs, not the old text-only
smoke marker. The example is generated outside the repository and remains available
for manual inspection after testing. `TestPlugin.ps1` keeps NullRHI as its default;
release validation also runs the separate strict GPU suite.

## Engine changes

Check [Compatibility.md](Compatibility.md) before selecting an engine. Supporting one
engine does not imply supporting every minor between it and another tested engine.
Launcher binaries and source-built engines require separate validation. If moving to
an older engine, regenerate imported assets and the packed material with that engine;
do not assume newer-engine `.uasset` files are backward compatible.
