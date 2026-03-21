import unreal

TARGET_SKELETON_PATH = "/Game/Characters/Player/Character/Mesh/SK_Mannequin_Skeleton"
ANIMATION_ASSET_PATHS = [
    "/Game/Characters/Player/Animations/ThirdPersonIdle",
    "/Game/Characters/Player/Animations/ThirdPersonRun",
    "/Game/Characters/Player/Animations/ThirdPersonWalk",
    "/Game/Characters/Player/Animations/ThirdPersonJump_Start",
    "/Game/Characters/Player/Animations/ThirdPersonJump_Loop",
    "/Game/Characters/Player/Animations/ThirdPersonJump_End",
    "/Game/Characters/Player/Animations/ThirdPerson_IdleRun_2D",
]
ANIM_BP_PATH = "/Game/Characters/Player/Animations/ThirdPerson_AnimBP"


def load_required_asset(asset_path):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"Failed to load {asset_path}")
    return asset


def save_loaded_asset(asset):
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset):
        raise RuntimeError(f"Failed to save {asset.get_path_name()}")


def main():
    target_skeleton = load_required_asset(TARGET_SKELETON_PATH)

    for asset_path in ANIMATION_ASSET_PATHS:
        asset = load_required_asset(asset_path)
        asset.set_editor_property("skeleton", target_skeleton)
        save_loaded_asset(asset)
        unreal.log(f"Rebound skeleton for {asset_path}")

    anim_bp = load_required_asset(ANIM_BP_PATH)
    anim_bp.set_editor_property("target_skeleton", target_skeleton)
    unreal.BlueprintEditorLibrary.compile_blueprint(anim_bp)
    save_loaded_asset(anim_bp)
    unreal.log(f"Rebound target skeleton for {ANIM_BP_PATH}")


if __name__ == "__main__":
    main()
