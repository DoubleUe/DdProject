import unreal

WIDGET_BP_PATH = "/Game/UI/WBP_ResultPopup"
WIDGET_BP_NAME = "WBP_ResultPopup"
WIDGET_BP_DIR = "/Game/UI"
PARENT_CLASS_PATH = "/Script/DdProject.ResultPopupWidget"


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
        raise RuntimeError("Failed to create result popup widget blueprint asset")

    return widget_bp


def get_canvas_slot(widget):
    return widget.get_editor_property("slot")


def make_anchors(min_x, min_y, max_x, max_y):
    anchors = unreal.Anchors()
    anchors.minimum = unreal.Vector2D(min_x, min_y)
    anchors.maximum = unreal.Vector2D(max_x, max_y)
    return anchors


def build_widget_tree(widget_bp):
    root = unreal.EditorUtilityLibrary.add_source_widget(
        widget_bp, unreal.CanvasPanel.static_class(), "RootCanvas", ""
    )
    if root is None:
        raise RuntimeError("Failed to create RootCanvas")

    success_image = unreal.EditorUtilityLibrary.add_source_widget(
        widget_bp, unreal.Image.static_class(), "SuccessImage", "RootCanvas"
    )
    if success_image is None:
        raise RuntimeError("Failed to create SuccessImage")
    success_slot = get_canvas_slot(success_image)
    success_slot.set_anchors(make_anchors(0.5, 1.0 / 3.0, 0.5, 1.0 / 3.0))
    success_slot.set_alignment(unreal.Vector2D(0.5, 0.5))
    success_slot.set_offsets(unreal.Margin(0.0, 0.0, 420.0, 140.0))

    confirm_button = unreal.EditorUtilityLibrary.add_source_widget(
        widget_bp, unreal.Button.static_class(), "ConfirmButton", "RootCanvas"
    )
    if confirm_button is None:
        raise RuntimeError("Failed to create ConfirmButton")
    button_slot = get_canvas_slot(confirm_button)
    button_slot.set_anchors(make_anchors(0.5, 2.0 / 3.0, 0.5, 2.0 / 3.0))
    button_slot.set_alignment(unreal.Vector2D(0.5, 0.5))
    button_slot.set_offsets(unreal.Margin(0.0, 0.0, 280.0, 72.0))

    confirm_text = unreal.EditorUtilityLibrary.add_source_widget(
        widget_bp, unreal.TextBlock.static_class(), "ConfirmButtonText", "ConfirmButton"
    )
    if confirm_text is None:
        raise RuntimeError("Failed to create ConfirmButtonText")
    confirm_text.set_text(unreal.Text("확인"))
    confirm_text.set_color_and_opacity(unreal.SlateColor(specified_color=unreal.LinearColor(0.0, 0.0, 0.0, 1.0)))

    unreal.BlueprintEditorLibrary.compile_blueprint(widget_bp)
    unreal.EditorAssetLibrary.save_loaded_asset(widget_bp)


def main():
    widget_bp = create_widget_blueprint()
    build_widget_tree(widget_bp)
    unreal.log(f"Created widget blueprint: {WIDGET_BP_PATH}")


if __name__ == "__main__":
    main()
