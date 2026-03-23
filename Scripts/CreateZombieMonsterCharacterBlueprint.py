import unreal

BLUEPRINT_DIR = "/Game/Characters/Zombie"
BLUEPRINT_NAME = "BP_ZombieMonsterCharacter"
BLUEPRINT_PATH = f"{BLUEPRINT_DIR}/{BLUEPRINT_NAME}"
PARENT_CLASS_PATH = "/Script/DdProject.MonsterCharacter"


def create_blueprint():
    if not unreal.EditorAssetLibrary.does_directory_exist(BLUEPRINT_DIR):
        unreal.EditorAssetLibrary.make_directory(BLUEPRINT_DIR)

    if unreal.EditorAssetLibrary.does_asset_exist(BLUEPRINT_PATH):
        blueprint = unreal.EditorAssetLibrary.load_asset(BLUEPRINT_PATH)
        if blueprint is None:
            raise RuntimeError("Failed to load zombie monster blueprint")
    else:
        parent_class = unreal.load_class(None, PARENT_CLASS_PATH)
        if parent_class is None:
            raise RuntimeError(f"Failed to load parent class: {PARENT_CLASS_PATH}")

        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", parent_class)

        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        blueprint = asset_tools.create_asset(
            BLUEPRINT_NAME,
            BLUEPRINT_DIR,
            unreal.Blueprint,
            factory,
        )
        if blueprint is None:
            raise RuntimeError("Failed to create zombie monster blueprint")

    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
    unreal.log(f"Created or updated zombie monster blueprint: {BLUEPRINT_PATH}")


if __name__ == "__main__":
    create_blueprint()
