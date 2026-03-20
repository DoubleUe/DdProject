import unreal

WIDGET_BP_PATH = "/Game/UI/WBP_TitleScreen"


def main():
    widget_bp = unreal.EditorAssetLibrary.load_asset(WIDGET_BP_PATH)
    if widget_bp is None:
        raise RuntimeError(f"Failed to load widget blueprint: {WIDGET_BP_PATH}")

    title_image = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_bp, "TitleImage")
    if title_image is None:
        unreal.log("TitleImage does not exist. Nothing to remove.")
        unreal.EditorAssetLibrary.save_loaded_asset(widget_bp)
        return

    root_widget = title_image.get_parent()
    if root_widget is not None:
        removed = root_widget.remove_child(title_image)
        if not removed:
            raise RuntimeError("Failed to remove TitleImage from parent widget")
    else:
        title_image.remove_from_parent()

    widget_bp.modify()
    unreal.BlueprintEditorLibrary.compile_blueprint(widget_bp)
    unreal.EditorAssetLibrary.save_loaded_asset(widget_bp)
    unreal.log("Removed TitleImage from /Game/UI/WBP_TitleScreen")


if __name__ == "__main__":
    main()
