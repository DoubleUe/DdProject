import unreal

ASSET_PATH = "/Game/Characters/Player/Animations/ThirdPerson_IdleRun_2D"


def log_object(label, obj):
    unreal.log(f"{label} type={type(obj)} value={obj}")
    unreal.log(f"{label} dir={sorted(name for name in dir(obj) if not name.startswith('_'))}")
    if hasattr(obj, "get_editor_property"):
        for property_name in ["animation", "sample_value", "rate_scale", "grid_num", "display_name", "min", "max"]:
            try:
                unreal.log(f"{label} {property_name}={obj.get_editor_property(property_name)}")
            except Exception:
                pass


def main():
    asset = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
    if asset is None:
        raise RuntimeError(f"Failed to load {ASSET_PATH}")

    sample_data = asset.get_editor_property("sample_data")
    blend_parameters = asset.get_editor_property("blend_parameters")

    if sample_data:
        log_object("sample_data[0]", sample_data[0])
    if blend_parameters:
        log_object("blend_parameters[0]", blend_parameters[0])


if __name__ == "__main__":
    main()
