import unreal

TEMP_DIR = "/Game/TempZombieAnimTest"
SOURCE_BLENDSPACE = "/Game/Characters/Player/Animations/ThirdPerson_IdleRun_2D"
SOURCE_ANIM_BP = "/Game/Characters/Player/Animations/ThirdPerson_AnimBP"
TARGET_BLENDSPACE = f"{TEMP_DIR}/ThirdPerson_IdleRun_2D"
TARGET_ANIM_BP = f"{TEMP_DIR}/ThirdPerson_AnimBP"


def cleanup():
    if unreal.EditorAssetLibrary.does_directory_exist(TEMP_DIR):
        unreal.EditorAssetLibrary.delete_directory(TEMP_DIR)


def main():
    cleanup()
    unreal.EditorAssetLibrary.make_directory(TEMP_DIR)

    if not unreal.EditorAssetLibrary.duplicate_asset(SOURCE_BLENDSPACE, TARGET_BLENDSPACE):
        raise RuntimeError("Failed to duplicate blendspace")
    if not unreal.EditorAssetLibrary.duplicate_asset(SOURCE_ANIM_BP, TARGET_ANIM_BP):
        raise RuntimeError("Failed to duplicate anim blueprint")

    referencers = unreal.EditorAssetLibrary.find_package_referencers_for_asset(TARGET_BLENDSPACE, False)
    unreal.log(f"duplicate_blendspace_referencers={referencers}")

    cleanup()


if __name__ == "__main__":
    main()
