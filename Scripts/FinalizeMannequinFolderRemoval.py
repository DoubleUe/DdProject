import unreal

SOURCE_ROOT = "/Game/Mannequin"
TARGET_ROOT = "/Game/Characters/ThirdPerson"
ANIM_BP_PATH = f"{TARGET_ROOT}/Animations/ThirdPerson_AnimBP"


def normalize_asset_path(asset_path):
    leaf = asset_path.rsplit("/", 1)[-1]
    if "." not in leaf:
        return asset_path

    package_path, object_name = asset_path.rsplit(".", 1)
    if package_path.rsplit("/", 1)[-1] == object_name:
        return package_path

    return asset_path


def load_asset(asset_path):
    return unreal.EditorAssetLibrary.load_asset(asset_path)


def list_source_assets():
    assets = unreal.EditorAssetLibrary.list_assets(SOURCE_ROOT, recursive=True, include_folder=False)
    normalized = sorted({normalize_asset_path(asset_path) for asset_path in assets})
    result = []

    for asset_path in normalized:
        if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            continue

        asset = load_asset(asset_path)
        if asset is None:
            raise RuntimeError(f"Failed to load asset: {asset_path}")

        asset_class = asset.get_class().get_name()
        if asset_class == "ObjectRedirector":
            unreal.log(f"Skipping redirector {asset_path}")
            continue

        result.append((asset_path, asset_class))

    return result


def make_target_path(source_asset_path):
    return f"{TARGET_ROOT}{source_asset_path[len(SOURCE_ROOT):]}"


def get_asset_priority(asset_path):
    leaf = asset_path.rsplit("/", 1)[-1]

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


def delete_asset(asset_path):
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        return

    unreal.log(f"Deleting target asset {asset_path}")
    if unreal.EditorAssetLibrary.delete_asset(asset_path):
        return

    asset = load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"Failed to load asset for deletion: {asset_path}")

    if not unreal.EditorAssetLibrary.delete_loaded_asset(asset):
        raise RuntimeError(f"Failed to delete asset: {asset_path}")


def rename_asset(source_asset_path, target_asset_path):
    unreal.log(f"Renaming {source_asset_path} -> {target_asset_path}")
    if not unreal.EditorAssetLibrary.rename_asset(source_asset_path, target_asset_path):
        raise RuntimeError(f"Failed to rename {source_asset_path} to {target_asset_path}")


def save_directory(root_path):
    assets = unreal.EditorAssetLibrary.list_assets(root_path, recursive=True, include_folder=False)
    normalized = sorted({normalize_asset_path(asset_path) for asset_path in assets})
    for asset_path in normalized:
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False)


def compile_anim_bp():
    anim_bp = load_asset(ANIM_BP_PATH)
    if anim_bp is None:
        raise RuntimeError(f"Failed to load anim blueprint: {ANIM_BP_PATH}")

    unreal.BlueprintEditorLibrary.compile_blueprint(anim_bp)
    unreal.EditorAssetLibrary.save_loaded_asset(anim_bp)


def main():
    source_assets = list_source_assets()
    if not source_assets:
        unreal.log_warning("No non-redirector assets remain under /Game/Mannequin")
        return

    moves = []
    for source_asset_path, asset_class in source_assets:
        target_asset_path = make_target_path(source_asset_path)
        moves.append((get_asset_priority(source_asset_path), source_asset_path, target_asset_path, asset_class))

    unreal.log(f"Finalizing mannequin removal for {len(moves)} assets")

    for _, _, target_asset_path, _ in sorted(moves, key=lambda item: (item[0], item[2])):
        delete_asset(target_asset_path)

    for _, source_asset_path, target_asset_path, asset_class in sorted(moves, key=lambda item: (item[0], item[1])):
        if unreal.EditorAssetLibrary.does_asset_exist(target_asset_path):
            raise RuntimeError(f"Target path is still occupied: {target_asset_path}")

        unreal.log(f"Moving {asset_class} {source_asset_path}")
        rename_asset(source_asset_path, target_asset_path)

    save_directory(TARGET_ROOT)
    compile_anim_bp()
    unreal.log("Finished final mannequin move. Run FixupRedirectors next.")


if __name__ == "__main__":
    main()
