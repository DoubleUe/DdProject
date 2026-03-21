import unreal

SOURCE_ROOT = "/Game/Mannequin"
TARGET_ROOT = "/Game/Characters/Player"
TARGET_ANIM_BP = f"{TARGET_ROOT}/Animations/ThirdPerson_AnimBP"
SOURCE_ANIM_BP = f"{SOURCE_ROOT}/Animations/ThirdPerson_AnimBP"


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


def consolidate_asset_pair(source_asset_path, target_asset_path):
    target_asset = load_required_asset(target_asset_path)
    source_asset = load_required_asset(source_asset_path)

    unreal.log(f"Consolidating {source_asset_path} -> {target_asset_path}")
    result = unreal.EditorAssetLibrary.consolidate_assets(target_asset, [source_asset])
    if not result:
        raise RuntimeError(f"Failed to consolidate {source_asset_path} into {target_asset_path}")

    save_asset_if_possible(target_asset_path)


def delete_asset_if_exists(asset_path):
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        return

    unreal.log(f"Deleting {asset_path}")
    if unreal.EditorAssetLibrary.delete_asset(asset_path):
        return

    loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if loaded_asset is None:
        raise RuntimeError(f"Failed to delete asset: {asset_path}")

    if not unreal.EditorAssetLibrary.delete_loaded_asset(loaded_asset):
        raise RuntimeError(f"Failed to delete loaded asset: {asset_path}")


def compile_target_anim_bp():
    target_anim_blueprint = load_required_asset(TARGET_ANIM_BP)
    unreal.BlueprintEditorLibrary.compile_blueprint(target_anim_blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(target_anim_blueprint)


def main():
    source_assets = list_assets(SOURCE_ROOT)
    pairs = []

    for source_asset_path in source_assets:
        if source_asset_path == SOURCE_ANIM_BP:
            continue

        target_asset_path = make_target_path(source_asset_path)
        if not unreal.EditorAssetLibrary.does_asset_exist(target_asset_path):
            raise RuntimeError(f"Missing target asset for consolidation: {target_asset_path}")

        pairs.append((get_asset_priority(source_asset_path), source_asset_path, target_asset_path))

    unreal.log(f"Consolidating {len(pairs)} Mannequin dependencies into ThirdPerson")

    for _, source_asset_path, target_asset_path in sorted(pairs, key=lambda item: (item[0], item[1])):
        consolidate_asset_pair(source_asset_path, target_asset_path)

    compile_target_anim_bp()
    delete_asset_if_exists(SOURCE_ANIM_BP)
    compile_target_anim_bp()
    unreal.log("Finished consolidating Mannequin dependencies into ThirdPerson")


if __name__ == "__main__":
    main()
