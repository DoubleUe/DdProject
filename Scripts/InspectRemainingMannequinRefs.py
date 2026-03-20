import unreal

PATHS = [
    "/Game/Mannequin/Animations/ThirdPersonIdle",
    "/Game/Mannequin/Character/Materials/MaterialLayers/T_ML_Aluminum01",
    "/Game/Mannequin/Character/Mesh/SK_Mannequin",
    "/Game/Mannequin/Character/Mesh/SK_Mannequin_Skeleton",
]


def main():
    for path in PATHS:
        exists = unreal.EditorAssetLibrary.does_asset_exist(path)
        unreal.log(f"{path} exists={exists}")
        if not exists:
            continue

        asset = unreal.EditorAssetLibrary.load_asset(path)
        if asset is None:
            unreal.log_warning(f"Failed to load: {path}")
            continue

        unreal.log(f"{path} class={asset.get_class().get_name()} path={asset.get_path_name()}")
        referencers = unreal.EditorAssetLibrary.find_package_referencers_for_asset(path, False)
        unreal.log(f"{path} referencers={referencers}")


if __name__ == "__main__":
    main()
