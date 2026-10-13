# Public API and asset compatibility

[简体中文](API.zh-CN.md)

## 1.x stability contract

The supported surface is the exported declarations under `Source/UnrealBMFont/Public`:
`UBMFontAsset`, `UBMFontText`, `UBMFontRichTextBlock`, `SBMFontText`, parser/layout data
and settings, and `BMFontRendering`. Private Slate runs, editor implementation details,
test hooks and build-script internals are not a stable extension API.

Within 1.x, existing serialized property names and Blueprint class/function paths are
preserved. Breaking public API changes require a major version or a documented
deprecation with a supported replacement. C++ ABI stability across Unreal builds is
not promised: rebuild binaries for the exact engine build and target configuration.

## Mutation and ownership

- Use `SetFontData` to replace asset data and `SetPackedRenderMaterial` to change its
  material. These rebuild derived caches and notify consumers. The public reflected
  fields remain available for serialization and inspection; direct C++ assignment is
  not a supported cache-coherent mutation path.
- Use widget setters after construction. Writing reflected C++ properties directly
  does not automatically synchronize an existing Slate widget.
- Access/mutation of UObjects and widget caches is game-thread only. The standalone
  parser and layout overload accepting immutable `FBMFontData` may run without UObjects
  when the caller owns the input and prevents concurrent modification.
- `FindGlyph`/`FindPage` return borrowed pointers. They expire on any data replacement,
  reimport or property edit. Use copied data when it must outlive a change.
- UMG owns its font through a reflected hard reference. Raw Slate callers must keep
  their font alive using a GC-visible reference; `SBMFontText` deliberately stores a
  weak reference and is not an asset owner.
- `OnChanged()` is a native notification. Metrics changes invalidate layout; render
  resource changes invalidate paint. Remove subscriptions when no longer needed, or
  use weak bindings. A callback must not recursively mutate the same asset.

## Packed material contract

Custom materials must be UI-domain materials using `FontAtlas` (texture),
`ChannelWeights` (vector) and `ChannelBias` (scalar). Channel extraction produces
coverage; Slate vertex color supplies text tint. The asset owns cached dynamic
instances. Do not retain or edit those instances across a resource revision.

Loading is attempted during asset lifecycle and retried once on a render-resource
request. An unresolved asset polls only the already-loaded soft object at 4 Hz, never
performing synchronous IO from that poll. Recovery advances the resource revision and
notifies consumers on a later tick, avoiding reentrant Rich Text layout during paint.
External loaders can call `RefreshRenderResources()` for immediate adoption.

## Serialized schema

`FBMFontCustomVersion::GUID` identifies the schema; version 0 records the initial 1.0
layout. Unversioned beta assets retain the same reflected properties and require no
destructive conversion. Future layout changes must add a schema version, migration
code where necessary, and tests loading older fixtures. Never reuse a version number
for a different serialized meaning.

Plugin schema compatibility does not override Unreal package compatibility: an asset
saved by a newer engine is not promised to open in an older engine. Import the original
`.fnt` and atlas files in the destination engine instead of copying newer `.uasset`s.
