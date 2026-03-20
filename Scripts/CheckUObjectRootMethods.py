import unreal

ASSET_PATH = "/Game/Mannequin/Character/Materials/MaterialLayers/T_ML_Aluminum01"


def main():
    asset = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
    if asset is None:
        raise RuntimeError(f"Failed to load {ASSET_PATH}")

    unreal.log(f"class={asset.get_class().get_name()}")
    unreal.log(f"has_is_rooted={hasattr(asset, 'is_rooted')}")
    unreal.log(f"has_remove_from_root={hasattr(asset, 'remove_from_root')}")
    unreal.log(f"has_clear_flags={hasattr(asset, 'clear_flags')}")
    unreal.log(f"has_set_flags={hasattr(asset, 'set_flags')}")
    if hasattr(asset, "is_rooted"):
        unreal.log(f"is_rooted={asset.is_rooted()}")


if __name__ == "__main__":
    main()
