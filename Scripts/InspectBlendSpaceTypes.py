import unreal

ASSET_PATH = "/Game/Characters/Player/Animations/ThirdPerson_IdleRun_2D"


def main():
    blend_space = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
    if blend_space is None:
        raise RuntimeError(f"Failed to load {ASSET_PATH}")

    sample_data = blend_space.get_editor_property("sample_data")
    blend_parameters = blend_space.get_editor_property("blend_parameters")

    unreal.log(f"sample_count={len(sample_data)}")
    if sample_data:
        sample = sample_data[0]
        unreal.log(f"sample_type={type(sample).__name__}")
        unreal.log(f"sample_dir={sorted(name for name in dir(sample) if not name.startswith('_'))}")

    unreal.log(f"blend_parameter_count={len(blend_parameters)}")
    if blend_parameters:
        parameter = blend_parameters[0]
        unreal.log(f"parameter_type={type(parameter).__name__}")
        unreal.log(f"parameter_dir={sorted(name for name in dir(parameter) if not name.startswith('_'))}")


if __name__ == "__main__":
    main()
