# Runnable showcase and packaged rendering probe

[简体中文](README.zh-CN.md)

This is a source template, not an already generated project. From the repository,
build the plugin package and run:

```powershell
./Scripts/CreateExampleProject.ps1 -EngineRoot '<UE root>' -PluginPackage '<BuildPlugin output>' -OutputDirectory '<new external directory>'
```

The command builds the example, imports the committed fixtures, sets nearest-neighbor
filtering, saves all assets, and creates `Content/BMFont/ShowcaseMap.umap`. Open the
generated `BMFontExample.uproject` and press Play. It demonstrates digits, uppercase
Latin, hiragana, Chinese numerals, fallback, packed channels and multiple atlas pages.

The map's actor owns hard references to the imported assets. The project cooks only
the map and its dependencies; it never force-cooks the plugin content directory.

`Scripts/TestPackagedRuntime.ps1` generates this same project, packages Development
and Shipping, and launches GPU validation with `-BMFontValidate`. Four probes compare
every pixel against independent masks for the original Minimal/Packed artwork.
Screenshots and machine-readable results are saved even on pixel mismatch. NullRHI
is explicitly rejected by the rendering probe. Normal Play does not exit automatically.

Generated assets, binaries and engine-specific project files belong outside this
repository. Never copy the generated project back under the plugin root.
