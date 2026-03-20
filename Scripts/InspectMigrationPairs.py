import unreal

PAIRS = [
    (
        "/Game/Mannequin/Animations/ThirdPersonIdle",
        "/Game/Characters/ThirdPerson/Animations/ThirdPersonIdle",
    ),
    (
        "/Game/Mannequin/Animations/ThirdPersonJump_End",
        "/Game/Characters/ThirdPerson/Animations/ThirdPersonJump_End",
    ),
    (
        "/Game/Mannequin/Animations/ThirdPersonJump_Loop",
        "/Game/Characters/ThirdPerson/Animations/ThirdPersonJump_Loop",
    ),
    (
        "/Game/Mannequin/Animations/ThirdPersonJump_Start",
        "/Game/Characters/ThirdPerson/Animations/ThirdPersonJump_Start",
    ),
    (
        "/Game/Mannequin/Animations/ThirdPersonRun",
        "/Game/Characters/ThirdPerson/Animations/ThirdPersonRun",
    ),
    (
        "/Game/Mannequin/Animations/ThirdPersonWalk",
        "/Game/Characters/ThirdPerson/Animations/ThirdPersonWalk",
    ),
    (
        "/Game/Mannequin/Animations/ThirdPerson_IdleRun_2D",
        "/Game/Characters/ThirdPerson/Animations/ThirdPerson_IdleRun_2D",
    ),
    (
        "/Game/Mannequin/Character/Materials/MaterialLayers/T_ML_Aluminum01",
        "/Game/Characters/ThirdPerson/Character/Materials/MaterialLayers/T_ML_Aluminum01",
    ),
    (
        "/Game/Mannequin/Character/Materials/MaterialLayers/T_ML_Aluminum01_N",
        "/Game/Characters/ThirdPerson/Character/Materials/MaterialLayers/T_ML_Aluminum01_N",
    ),
    (
        "/Game/Mannequin/Character/Materials/MaterialLayers/T_ML_Rubber_Blue_01_D",
        "/Game/Characters/ThirdPerson/Character/Materials/MaterialLayers/T_ML_Rubber_Blue_01_D",
    ),
    (
        "/Game/Mannequin/Character/Materials/MaterialLayers/T_ML_Rubber_Blue_01_N",
        "/Game/Characters/ThirdPerson/Character/Materials/MaterialLayers/T_ML_Rubber_Blue_01_N",
    ),
    (
        "/Game/Mannequin/Character/Mesh/SK_Mannequin_Skeleton",
        "/Game/Characters/ThirdPerson/Character/Mesh/SK_Mannequin_Skeleton",
    ),
]


def log_asset(path):
    exists = unreal.EditorAssetLibrary.does_asset_exist(path)
    unreal.log(f"{path} exists={exists}")
    if not exists:
        return

    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset is None:
        unreal.log_warning(f"Failed to load: {path}")
        return

    unreal.log(f"{path} class={asset.get_class().get_name()} path={asset.get_path_name()}")
    referencers = unreal.EditorAssetLibrary.find_package_referencers_for_asset(path, False)
    unreal.log(f"{path} referencers={referencers}")


def main():
    for source_path, target_path in PAIRS:
        unreal.log("-----")
        log_asset(source_path)
        log_asset(target_path)


if __name__ == "__main__":
    main()
