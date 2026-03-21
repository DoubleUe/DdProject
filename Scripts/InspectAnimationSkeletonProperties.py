import unreal

ASSET_PATHS = [
    "/Game/Characters/Player/Animations/ThirdPersonIdle",
    "/Game/Characters/Player/Animations/ThirdPerson_IdleRun_2D",
    "/Game/Characters/Player/Animations/ThirdPerson_AnimBP",
]

PROPERTY_NAMES = [
    "skeleton",
    "target_skeleton",
]


def main():
    for asset_path in ASSET_PATHS:
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if asset is None:
            raise RuntimeError(f"Failed to load {asset_path}")

        unreal.log(f"{asset_path} class={asset.get_class().get_name()}")
        unreal.log(f"{asset_path} has_get_editor_property={hasattr(asset, 'get_editor_property')}")
        unreal.log(f"{asset_path} has_set_editor_property={hasattr(asset, 'set_editor_property')}")

        if not hasattr(asset, "get_editor_property"):
            continue

        for property_name in PROPERTY_NAMES:
            try:
                value = asset.get_editor_property(property_name)
                unreal.log(f"{asset_path} {property_name}={value}")
            except Exception as exc:
                unreal.log_warning(f"{asset_path} {property_name} unavailable: {exc}")


if __name__ == "__main__":
    main()
