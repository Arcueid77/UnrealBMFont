# Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

"""Run in the freshly created example's Editor after its C++ module has built."""

from pathlib import Path

import unreal


def main():
    source = Path(unreal.Paths.project_plugins_dir()) / "UnrealBMFont" / "Samples"
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    assets = {}
    for name, folder, descriptor in (
        ("Plain", "Minimal", "Minimal.fnt"),
        ("Packed", "Packed", "Packed.fnt"),
        ("MultiPage", "MultiPage", "MultiPage.fnt"),
        ("Showcase", "Showcase", "Showcase.fnt"),
    ):
        asset_path = "/Game/BMFont/{0}/{0}".format(name)
        if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            task = unreal.AssetImportTask()
            task.set_editor_property("filename", str(source / folder / descriptor))
            task.set_editor_property("destination_path", "/Game/BMFont/" + name)
            task.set_editor_property("destination_name", name)
            task.set_editor_property("automated", True)
            task.set_editor_property("save", True)
            tools.import_asset_tasks([task])
        font = unreal.load_asset(asset_path)
        if not isinstance(font, unreal.BMFontAsset):
            raise RuntimeError("BMFont import failed: " + name)
        for page_id in range(font.get_page_count()):
            texture = font.get_page_texture(page_id)
            if texture is None:
                raise RuntimeError("Missing imported texture: " + name)
            texture.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
            if not unreal.EditorAssetLibrary.save_loaded_asset(texture):
                raise RuntimeError("Texture save failed: " + name)
        if not unreal.EditorAssetLibrary.save_loaded_asset(font):
            raise RuntimeError("Font save failed: " + name)
        assets[name] = font

    # These APIs also work in unattended initialization where the Level Editor's
    # interactive subsystem has not been created yet.
    world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    if world is None:
        raise RuntimeError("Example map creation failed")
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if actors is None:
        raise RuntimeError("Editor actor subsystem is unavailable")
    actor = actors.spawn_actor_from_class(unreal.BMFontShowcaseActor, unreal.Vector())
    if actor is None:
        raise RuntimeError("Example actor creation failed")
    for prop, name in (("plain_font", "Plain"), ("packed_font", "Packed"),
                       ("multi_page_font", "MultiPage"), ("showcase_font", "Showcase")):
        actor.set_editor_property(prop, assets[name])
    if not unreal.EditorLoadingAndSavingUtils.save_map(world, "/Game/BMFont/ShowcaseMap"):
        raise RuntimeError("Example map save failed")
    unreal.log("BMFONT_EXAMPLE_ASSETS=PASS")


main()
