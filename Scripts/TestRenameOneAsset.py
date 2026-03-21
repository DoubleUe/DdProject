import unreal

SOURCE = "/Game/Mannequin/Character/Textures/T_UE4_Mannequin_Mobile_M"
TARGET = "/Game/Characters/Player/Character/Textures/T_UE4_Mannequin_Mobile_M"


def log_state(label):
    for path in (SOURCE, TARGET):
        exists = unreal.EditorAssetLibrary.does_asset_exist(path)
        unreal.log(f"{label} {path} exists={exists}")
        if exists:
            asset = unreal.EditorAssetLibrary.load_asset(path)
            if asset is not None:
                unreal.log(f"{label} {path} class={asset.get_class().get_name()} path={asset.get_path_name()}")


def main():
    log_state("before")
    result = unreal.EditorAssetLibrary.rename_asset(SOURCE, TARGET)
    unreal.log(f"rename_result={result}")
    log_state("after")


if __name__ == "__main__":
    main()
