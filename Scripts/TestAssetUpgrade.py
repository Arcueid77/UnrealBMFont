# Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

"""Run twice in a disposable project: BMFONT_UPGRADE_PHASE=resave, then verify.

Copy the original beta asset and its dependencies at their original /Game paths;
never point this test at the original project. BMFONT_UPGRADE_ASSET is the object path.
"""

import json
import os
from pathlib import Path

import unreal


asset_path = os.environ["BMFONT_UPGRADE_ASSET"]
phase = os.environ["BMFONT_UPGRADE_PHASE"]
if phase not in ("resave", "verify"):
    raise RuntimeError("Expected resave or verify phase")
font = unreal.load_asset(asset_path)
if not isinstance(font, unreal.BMFontAsset):
    raise RuntimeError("Legacy font could not be loaded: " + asset_path)
data = font.get_editor_property("font_data")
glyphs = data.get_editor_property("glyphs")
info = data.get_editor_property("info")
common = data.get_editor_property("common")
spacing = info.get_editor_property("spacing")
snapshot = {
    "asset": asset_path,
    "lineHeight": font.get_line_height(),
    "pages": font.get_page_count(),
    "codepoints": sorted(glyphs.keys()),
    "glyphs": {str(key): {field: glyph.get_editor_property(field) for field in
                          ("codepoint", "x", "y", "width", "height", "x_offset", "y_offset", "x_advance", "page", "channel")}
               for key, glyph in glyphs.items()},
    "textures": [{"id": page.get_editor_property("id"), "file": page.get_editor_property("file"),
                  "texture": page.get_editor_property("texture").get_path_name()}
                 for page in data.get_editor_property("pages")],
    "info": {field: info.get_editor_property(field) for field in
             ("face", "size", "bold", "italic", "unicode", "smooth", "fixed_height", "character_set",
              "character_set_id", "stretch_height", "supersampling", "padding_up", "padding_right",
              "padding_down", "padding_left", "outline")},
    "spacing": [spacing.x, spacing.y],
    "common": {field: common.get_editor_property(field) for field in
               ("line_height", "base", "scale_width", "scale_height", "page_count", "packed")},
    "channels": {field: str(common.get_editor_property(field)) for field in
                 ("alpha_channel", "red_channel", "green_channel", "blue_channel")},
    "kerning": [{field: pair.get_editor_property(field) for field in ("first", "second", "amount")}
                for pair in data.get_editor_property("kerning_pairs")],
    "format": str(data.get_editor_property("descriptor_format")),
}
if not snapshot["codepoints"] or not snapshot["textures"]:
    raise RuntimeError("Legacy font is unexpectedly empty")
evidence = Path(unreal.Paths.project_saved_dir()) / "BMFontUpgrade.json"
if phase == "resave":
    evidence.parent.mkdir(parents=True, exist_ok=True)
    evidence.write_text(json.dumps(snapshot, sort_keys=True, indent=2), encoding="utf-8")
    if not unreal.EditorAssetLibrary.save_loaded_asset(font, only_if_is_dirty=False):
        raise RuntimeError("Resaving the copied legacy asset failed")
else:
    expected = json.loads(evidence.read_text(encoding="utf-8"))
    if snapshot != expected:
        raise RuntimeError("Metrics or page references changed across save and a fresh-process reload")
unreal.log("BMFONT_UPGRADE_{}=PASS".format(phase.upper()))
