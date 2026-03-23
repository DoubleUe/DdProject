import unreal

WIDGET_BP_PATH = "/Game/UI/WBP_ScreenFade"
WIDGET_BP_NAME = "WBP_ScreenFade"
WIDGET_BP_DIR = "/Game/UI"
PARENT_CLASS_PATH = "/Script/DdProject.ScreenFadeWidget"


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


def create_widget_blueprint():
    if unreal.EditorAssetLibrary.does_asset_exist(WIDGET_BP_PATH):
        unreal.EditorAssetLibrary.delete_asset(WIDGET_BP_PATH)

    if not unreal.EditorAssetLibrary.does_directory_exist(WIDGET_BP_DIR):
        unreal.EditorAssetLibrary.make_directory(WIDGET_BP_DIR)

    parent_class = unreal.load_class(None, PARENT_CLASS_PATH)
    if parent_class is None:
        raise RuntimeError(f"Failed to load widget parent class: {PARENT_CLASS_PATH}")

    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    widget_bp = asset_tools.create_asset(
        WIDGET_BP_NAME,
        WIDGET_BP_DIR,
        unreal.WidgetBlueprint,
        factory,
    )
    if widget_bp is None:
        raise RuntimeError("Failed to create screen fade widget blueprint asset")

    return widget_bp


def build_widget_tree(widget_bp):
    root_canvas_name = find_root_canvas_name(widget_bp)
    if root_canvas_name is None:
        root = unreal.EditorUtilityLibrary.add_source_widget(
            widget_bp, unreal.CanvasPanel.static_class(), "RootCanvas", ""
        )
        if root is None:
            raise RuntimeError("Failed to create RootCanvas")
        root_canvas_name = root.get_name()

    fade_image = unreal.EditorUtilityLibrary.add_source_widget(
        widget_bp, unreal.Image.static_class(), "FadeImage", root_canvas_name
    )
    if fade_image is None:
        raise RuntimeError("Failed to create FadeImage")

    fade_image.set_color_and_opacity(unreal.LinearColor(0.0, 0.0, 0.0, 1.0))

    fade_slot = get_canvas_slot(fade_image)
    fade_slot.set_anchors(make_anchors(0.0, 0.0, 1.0, 1.0))
    fade_slot.set_offsets(unreal.Margin(0.0, 0.0, 0.0, 0.0))

    unreal.BlueprintEditorLibrary.compile_blueprint(widget_bp)
    unreal.EditorAssetLibrary.save_loaded_asset(widget_bp)


def main():
    widget_bp = create_widget_blueprint()
    build_widget_tree(widget_bp)
    unreal.log(f"Created screen fade widget blueprint: {WIDGET_BP_PATH}")


if __name__ == "__main__":
    main()
