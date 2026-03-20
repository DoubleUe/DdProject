import unreal

LEVEL_PATH = "/Game/Maps/TitleLevel"


def ensure_folder(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def main() -> None:
    ensure_folder("/Game/Maps")

    if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH):
        unreal.log(f"{LEVEL_PATH} already exists")
        unreal.EditorLevelLibrary.load_level(LEVEL_PATH)
        return

    created = unreal.EditorLevelLibrary.new_level(LEVEL_PATH)
    if not created:
        raise RuntimeError(f"Failed to create level: {LEVEL_PATH}")

    unreal.EditorLevelLibrary.load_level(LEVEL_PATH)
    unreal.log(f"Created level: {LEVEL_PATH}")


if __name__ == "__main__":
    main()
