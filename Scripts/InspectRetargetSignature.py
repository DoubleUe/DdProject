import inspect
import unreal


def main():
    method = unreal.IKRetargetBatchOperation.duplicate_and_retarget
    try:
        signature = inspect.signature(method)
        unreal.log(f"duplicate_and_retarget_signature={signature}")
    except Exception as exc:
        unreal.log_warning(f"signature unavailable: {exc}")

    unreal.log(f"duplicate_and_retarget_doc={getattr(method, '__doc__', None)}")

    settings = unreal.BatchRetargetSettings()
    unreal.log(f"batch_retarget_settings={settings}")
    for property_name in ["source_skeletal_mesh", "target_skeletal_mesh", "retarget_asset", "auto_generate_retargeter"]:
        try:
            unreal.log(f"{property_name}={settings.get_editor_property(property_name)}")
        except Exception as exc:
            unreal.log_warning(f"{property_name} unavailable: {exc}")


if __name__ == "__main__":
    main()
