import unreal

WIDGET_BP_PATH = "/Game/UI/WBP_TitleScreen"


def find_root_canvas_name(widget_bp):
    candidates = ["RootCanvas"] + [f"CanvasPanel_{index}" for index in range(10)]
    for candidate in candidates:
        widget = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_bp, candidate)
        if widget is not None:
            return candidate
    return None


def get_canvas_slot(widget):
    return widget.get_editor_property("slot")


def make_anchors(min_x, min_y, max_x, max_y):
    anchors = unreal.Anchors()
    anchors.minimum = unreal.Vector2D(min_x, min_y)
    anchors.maximum = unreal.Vector2D(max_x, max_y)
    return anchors


def main():
    widget_bp = unreal.EditorAssetLibrary.load_asset(WIDGET_BP_PATH)
    if widget_bp is None:
        raise RuntimeError(f"Failed to load widget blueprint: {WIDGET_BP_PATH}")

    existing_logo = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_bp, "GameLogoImage")
    if existing_logo is not None:
        unreal.log("GameLogoImage already exists. Nothing to add.")
        unreal.BlueprintEditorLibrary.compile_blueprint(widget_bp)
        unreal.EditorAssetLibrary.save_loaded_asset(widget_bp)
        return

    root_canvas_name = find_root_canvas_name(widget_bp)
    if root_canvas_name is None:
        raise RuntimeError("Failed to find root canvas panel in WBP_TitleScreen")

    game_logo_image = unreal.EditorUtilityLibrary.add_source_widget(
        widget_bp, unreal.Image.static_class(), "GameLogoImage", root_canvas_name
    )
    if game_logo_image is None:
        raise RuntimeError("Failed to create GameLogoImage")

    logo_slot = get_canvas_slot(game_logo_image)
    logo_slot.set_anchors(make_anchors(0.5, 0.42, 0.5, 0.42))
    logo_slot.set_alignment(unreal.Vector2D(0.5, 0.5))
    logo_slot.set_offsets(unreal.Margin(0.0, 0.0, 640.0, 220.0))

    unreal.BlueprintEditorLibrary.compile_blueprint(widget_bp)
    unreal.EditorAssetLibrary.save_loaded_asset(widget_bp)
    unreal.log("Added GameLogoImage to /Game/UI/WBP_TitleScreen")


if __name__ == "__main__":
    main()
