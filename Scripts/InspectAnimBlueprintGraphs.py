import unreal

ASSET_PATH = "/Game/Characters/Player/Animations/ThirdPerson_AnimBP"


def main():
    asset = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
    if asset is None:
        raise RuntimeError(f"Failed to load {ASSET_PATH}")

    unreal.log(f"class={asset.get_class().get_name()}")
    unreal.log(f"dir={sorted(name for name in dir(asset) if not name.startswith('_'))}")

    for property_name in ["ubergraph_pages", "function_graphs", "delegate_signature_graphs", "macro_graphs", "event_graphs", "animations"]:
        try:
            value = asset.get_editor_property(property_name)
            unreal.log(f"{property_name}={value}")
        except Exception as exc:
            unreal.log_warning(f"{property_name} unavailable: {exc}")


if __name__ == "__main__":
    main()
