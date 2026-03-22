import unreal

SOURCE_ASSET_PATHS = [
    "/Game/Characters/Player/Animations/ThirdPerson_IdleRun_2D",
    "/Game/Characters/Player/Animations/ThirdPerson_AnimBP",
]
SOURCE_MESH_PATH = "/Game/Characters/Player/Character/Mesh/SK_Mannequin.SK_Mannequin"
TARGET_MESH_PATH = "/Game/Characters/Zombie/Mesh/Zombie_Running.Zombie_Running"
TEMP_DIR = "/Game/TempZombieAnimTest"


def cleanup():
    if unreal.EditorAssetLibrary.does_directory_exist(TEMP_DIR):
        unreal.EditorAssetLibrary.delete_directory(TEMP_DIR)


def main():
    cleanup()
    unreal.EditorAssetLibrary.make_directory(TEMP_DIR)

    source_assets = [unreal.EditorAssetLibrary.find_asset_data(path) for path in SOURCE_ASSET_PATHS]
    source_mesh = unreal.EditorAssetLibrary.load_asset(SOURCE_MESH_PATH)
    target_mesh = unreal.EditorAssetLibrary.load_asset(TARGET_MESH_PATH)
    if any(not asset.is_valid() for asset in source_assets) or source_mesh is None or target_mesh is None:
        raise RuntimeError("Failed to load source assets or meshes")

    result = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
        source_assets,
        source_mesh,
        target_mesh,
        None,
        "/Game/Characters/Player/Animations",
        TEMP_DIR,
        "",
        "",
        True,
        True,
    )
    unreal.log(f"retarget_result={result}")

    created_assets = unreal.EditorAssetLibrary.list_assets(TEMP_DIR, recursive=True, include_folder=True)
    unreal.log(f"created_assets={created_assets}")

    cleanup()


if __name__ == "__main__":
    main()
