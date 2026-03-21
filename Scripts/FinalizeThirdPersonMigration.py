import unreal

TARGET_ANIM_BP = "/Game/Characters/Player/Animations/ThirdPerson_AnimBP"
SOURCE_ANIM_BP = "/Game/Mannequin/Animations/ThirdPerson_AnimBP"


def load_required_asset(asset_path):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"Failed to load asset: {asset_path}")
    return asset


def compile_and_save_target_anim_bp():
    anim_blueprint = load_required_asset(TARGET_ANIM_BP)
    unreal.BlueprintEditorLibrary.compile_blueprint(anim_blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(anim_blueprint)


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


def main():
    compile_and_save_target_anim_bp()
    delete_asset_if_exists(SOURCE_ANIM_BP)
    compile_and_save_target_anim_bp()
    unreal.log("Finalized ThirdPerson migration.")


if __name__ == "__main__":
    main()
