import unreal

GAME_LEVEL_PATH = "/Game/Maps/GameLevel"
TEMPLATE_LEVEL_PATH = "/Engine/Maps/Templates/OpenWorld"


def main():
    if unreal.EditorAssetLibrary.does_asset_exist(GAME_LEVEL_PATH):
        unreal.log(f"{GAME_LEVEL_PATH} already exists")
        return

    if not unreal.EditorAssetLibrary.does_directory_exist("/Game/Maps"):
        unreal.EditorAssetLibrary.make_directory("/Game/Maps")

    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if level_subsystem is None:
        raise RuntimeError("Failed to acquire LevelEditorSubsystem")

    created = level_subsystem.new_level_from_template(GAME_LEVEL_PATH, TEMPLATE_LEVEL_PATH)
    if not created:
        raise RuntimeError(f"Failed to create level from template: {TEMPLATE_LEVEL_PATH}")

    unreal.log(f"Created gameplay level from OpenWorld template: {GAME_LEVEL_PATH}")


if __name__ == "__main__":
    main()
