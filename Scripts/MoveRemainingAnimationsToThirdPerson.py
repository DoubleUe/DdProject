import unreal

PAIRS = [
    (
        "/Game/Mannequin/Animations/ThirdPersonIdle",
        "/Game/Characters/Player/Animations/ThirdPersonIdle",
    ),
    (
        "/Game/Mannequin/Animations/ThirdPersonRun",
        "/Game/Characters/Player/Animations/ThirdPersonRun",
    ),
    (
        "/Game/Mannequin/Animations/ThirdPersonWalk",
        "/Game/Characters/Player/Animations/ThirdPersonWalk",
    ),
    (
        "/Game/Mannequin/Animations/ThirdPersonJump_Start",
        "/Game/Characters/Player/Animations/ThirdPersonJump_Start",
    ),
    (
        "/Game/Mannequin/Animations/ThirdPersonJump_Loop",
        "/Game/Characters/Player/Animations/ThirdPersonJump_Loop",
    ),
    (
        "/Game/Mannequin/Animations/ThirdPersonJump_End",
        "/Game/Characters/Player/Animations/ThirdPersonJump_End",
    ),
    (
        "/Game/Mannequin/Animations/ThirdPerson_IdleRun_2D",
        "/Game/Characters/Player/Animations/ThirdPerson_IdleRun_2D",
    ),
]

ANIM_BP_PATH = "/Game/Characters/Player/Animations/ThirdPerson_AnimBP"


def rename_asset(source_path, target_path):
    if not unreal.EditorAssetLibrary.does_asset_exist(source_path):
        raise RuntimeError(f"Missing source asset: {source_path}")
    if unreal.EditorAssetLibrary.does_asset_exist(target_path):
        raise RuntimeError(f"Target path is occupied: {target_path}")

    unreal.log(f"Renaming {source_path} -> {target_path}")
    if not unreal.EditorAssetLibrary.rename_asset(source_path, target_path):
        raise RuntimeError(f"Failed to rename {source_path} to {target_path}")


def compile_anim_bp():
    anim_bp = unreal.EditorAssetLibrary.load_asset(ANIM_BP_PATH)
    if anim_bp is None:
        raise RuntimeError(f"Failed to load {ANIM_BP_PATH}")

    unreal.BlueprintEditorLibrary.compile_blueprint(anim_bp)
    unreal.EditorAssetLibrary.save_loaded_asset(anim_bp)


def main():
    for source_path, target_path in PAIRS:
        rename_asset(source_path, target_path)

    compile_anim_bp()
    unreal.log("Moved remaining animation assets to ThirdPerson.")


if __name__ == "__main__":
    main()
