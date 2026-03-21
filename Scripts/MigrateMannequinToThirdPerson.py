import unreal

SOURCE_ROOT = "/Game/Mannequin"
TARGET_ROOT = "/Game/Characters/Player"
ANIM_BP_PATH = f"{TARGET_ROOT}/Animations/ThirdPerson_AnimBP"


def normalize_asset_path(asset_path):
    leaf = asset_path.rsplit("/", 1)[-1]
    if "." not in leaf:
        return asset_path

    package_path, object_name = asset_path.rsplit(".", 1)
    if package_path.rsplit("/", 1)[-1] == object_name:
        return package_path

    return asset_path


def list_assets(root_path):
    assets = unreal.EditorAssetLibrary.list_assets(root_path, recursive=True, include_folder=False)
    normalized = {normalize_asset_path(asset_path) for asset_path in assets}
    return sorted(
        asset_path
        for asset_path in normalized
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path)
    )


def make_target_path(source_asset_path):
    if not source_asset_path.startswith(f"{SOURCE_ROOT}/"):
        raise RuntimeError(f"Unexpected source asset path: {source_asset_path}")

    return f"{TARGET_ROOT}{source_asset_path[len(SOURCE_ROOT):]}"


def get_asset_priority(asset_path):
    leaf = asset_path.rsplit("/", 1)[-1]

    if leaf == "ThirdPerson_AnimBP":
        return 100
    if "/Textures/" in asset_path:
        return 10
    if "/MaterialLayers/" in asset_path:
        return 20
    if "/Materials/" in asset_path:
        return 30
    if leaf.endswith("_Skeleton"):
        return 40
    if leaf.endswith("_PhysicsAsset"):
        return 50
    if "/Mesh/" in asset_path:
        return 60
    if "/Animations/" in asset_path:
        return 70

    return 80


def load_required_asset(asset_path):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"Failed to load asset: {asset_path}")
    return asset


def save_asset_if_possible(asset_path):
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False)


def delete_existing_targets():
    source_assets = list_assets(SOURCE_ROOT)
    targets = [(get_asset_priority(source_asset_path), make_target_path(source_asset_path)) for source_asset_path in source_assets]

    for _, target_asset_path in sorted(targets, key=lambda item: (item[0], item[1])):
        if not unreal.EditorAssetLibrary.does_asset_exist(target_asset_path):
            continue

        unreal.log(f"Deleting existing target asset {target_asset_path}")
        result = unreal.EditorAssetLibrary.delete_asset(target_asset_path)
        if result:
            continue

        loaded_target_asset = unreal.EditorAssetLibrary.load_asset(target_asset_path)
        if loaded_target_asset is None:
            raise RuntimeError(f"Failed to delete target asset: {target_asset_path}")

        result = unreal.EditorAssetLibrary.delete_loaded_asset(loaded_target_asset)
        if not result:
            raise RuntimeError(f"Failed to delete loaded target asset: {target_asset_path}")


def move_mannequin_assets_to_third_person():
    source_assets = list_assets(SOURCE_ROOT)
    moves = [(get_asset_priority(source_asset_path), source_asset_path, make_target_path(source_asset_path)) for source_asset_path in source_assets]

    for _, source_asset_path, target_asset_path in sorted(moves, key=lambda item: (item[0], item[1])):
        if unreal.EditorAssetLibrary.does_asset_exist(target_asset_path):
            raise RuntimeError(f"Target path is still occupied: {target_asset_path}")

        unreal.log(f"Renaming {source_asset_path} -> {target_asset_path}")
        result = unreal.EditorAssetLibrary.rename_asset(source_asset_path, target_asset_path)
        if not result:
            raise RuntimeError(f"Failed to rename {source_asset_path} to {target_asset_path}")


def resave_third_person_assets():
    for asset_path in list_assets(TARGET_ROOT):
        unreal.log(f"Saving {asset_path}")
        save_asset_if_possible(asset_path)


def compile_anim_blueprint():
    anim_blueprint = load_required_asset(ANIM_BP_PATH)
    unreal.BlueprintEditorLibrary.compile_blueprint(anim_blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(anim_blueprint)


def main():
    if not unreal.EditorAssetLibrary.does_directory_exist(SOURCE_ROOT):
        unreal.log_warning(f"{SOURCE_ROOT} does not exist. Nothing to migrate.")
        return

    if not unreal.EditorAssetLibrary.does_directory_exist(TARGET_ROOT):
        raise RuntimeError(f"Missing target directory: {TARGET_ROOT}")

    source_assets = list_assets(SOURCE_ROOT)
    if not source_assets:
        unreal.log_warning(f"No assets found under {SOURCE_ROOT}")
        return

    unreal.log(f"Preparing to migrate {len(source_assets)} assets from {SOURCE_ROOT} to {TARGET_ROOT}")

    delete_existing_targets()
    move_mannequin_assets_to_third_person()
    resave_third_person_assets()
    compile_anim_blueprint()

    unreal.log("Mannequin migration finished. Run ResavePackages -FixupRedirects next to remove redirectors.")


if __name__ == "__main__":
    main()
