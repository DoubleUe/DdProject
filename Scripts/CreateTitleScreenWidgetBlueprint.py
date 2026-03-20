import unreal

WIDGET_BP_PATH = "/Game/UI/WBP_TitleScreen"
WIDGET_BP_NAME = "WBP_TitleScreen"
WIDGET_BP_DIR = "/Game/UI"
PARENT_CLASS_PATH = "/Script/TestProject.TitleScreenWidget"
START_BUTTON_TEXTURE_PATH = "/Game/UI/Resource/StartButton.StartButton"


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
        raise RuntimeError("Failed to create widget blueprint asset")

    return widget_bp


def find_root_canvas_name(widget_bp):
    candidates = ["RootCanvas"] + [f"CanvasPanel_{index}" for index in range(10)]
    for candidate in candidates:
        widget = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_bp, candidate)
        if widget is not None:
            return candidate
    return None


def get_canvas_slot(widget):
    return widget.get_editor_property("slot")


def get_editor_property_any(obj, *names):
    for name in names:
        try:
            return obj.get_editor_property(name)
        except Exception:
            continue
    raise RuntimeError(f"Failed to read properties {names} from {obj}")


def set_editor_property_any(obj, value, *names):
    for name in names:
        try:
            obj.set_editor_property(name, value)
            return
        except Exception:
            continue
    raise RuntimeError(f"Failed to set properties {names} on {obj}")


def make_anchors(min_x, min_y, max_x, max_y):
    anchors = unreal.Anchors()
    anchors.minimum = unreal.Vector2D(min_x, min_y)
    anchors.maximum = unreal.Vector2D(max_x, max_y)
    return anchors


def configure_brush(brush, texture):
    try:
        brush.set_resource_object(texture)
    except Exception:
        set_editor_property_any(brush, texture, "resource_object", "ResourceObject")
    set_editor_property_any(brush, unreal.Margin(0.0, 0.0, 0.0, 0.0), "margin", "Margin")

    try:
        set_editor_property_any(brush, unreal.SlateBrushDrawType.IMAGE, "draw_as", "DrawAs")
    except Exception:
        pass


def apply_start_button_texture(start_button):
    texture = unreal.EditorAssetLibrary.load_asset(START_BUTTON_TEXTURE_PATH)
    if texture is None:
        unreal.log_warning(f"Start button texture not found: {START_BUTTON_TEXTURE_PATH}")
        return

    style = get_editor_property_any(start_button, "widget_style", "WidgetStyle")

    for brush_property in (("normal", "Normal"), ("hovered", "Hovered"), ("pressed", "Pressed"), ("disabled", "Disabled")):
        brush = get_editor_property_any(style, *brush_property)
        configure_brush(brush, texture)
        set_editor_property_any(style, brush, *brush_property)

    set_editor_property_any(start_button, style, "widget_style", "WidgetStyle")
    start_button.set_background_color(unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    start_button.set_color_and_opacity(unreal.LinearColor(1.0, 1.0, 1.0, 1.0))


def build_widget_tree(widget_bp):
    root_canvas_name = find_root_canvas_name(widget_bp)
    if root_canvas_name is None:
        root = unreal.EditorUtilityLibrary.add_source_widget(
            widget_bp, unreal.CanvasPanel.static_class(), "RootCanvas", ""
        )
        if root is None:
            raise RuntimeError("Failed to create root canvas panel")
        root_canvas_name = root.get_name()

    background = unreal.EditorUtilityLibrary.add_source_widget(
        widget_bp, unreal.Image.static_class(), "BackgroundImage", root_canvas_name
    )
    if background is None:
        raise RuntimeError("Failed to create BackgroundImage")
    bg_slot = get_canvas_slot(background)
    bg_slot.set_anchors(make_anchors(0.0, 0.0, 1.0, 1.0))
    bg_slot.set_offsets(unreal.Margin(0.0, 0.0, 0.0, 0.0))

    game_logo_image = unreal.EditorUtilityLibrary.add_source_widget(
        widget_bp, unreal.Image.static_class(), "GameLogoImage", root_canvas_name
    )
    if game_logo_image is None:
        raise RuntimeError("Failed to create GameLogoImage")
    logo_slot = get_canvas_slot(game_logo_image)
    logo_slot.set_anchors(make_anchors(0.5, 0.42, 0.5, 0.42))
    logo_slot.set_alignment(unreal.Vector2D(0.5, 0.5))
    logo_slot.set_offsets(unreal.Margin(0.0, 0.0, 640.0, 220.0))

    start_button = unreal.EditorUtilityLibrary.add_source_widget(
        widget_bp, unreal.Button.static_class(), "StartButton", root_canvas_name
    )
    if start_button is None:
        raise RuntimeError("Failed to create StartButton")
    start_button.set_background_color(unreal.LinearColor(1.0, 1.0, 1.0, 0.0))
    button_slot = get_canvas_slot(start_button)
    button_slot.set_anchors(make_anchors(0.5, 0.75, 0.5, 0.75))
    button_slot.set_alignment(unreal.Vector2D(0.5, 0.5))
    button_slot.set_offsets(unreal.Margin(0.0, 0.0, 360.0, 96.0))
    apply_start_button_texture(start_button)

    unreal.BlueprintEditorLibrary.compile_blueprint(widget_bp)
    unreal.EditorAssetLibrary.save_loaded_asset(widget_bp)


def main():
    widget_bp = create_widget_blueprint()
    build_widget_tree(widget_bp)
    unreal.log(f"Created widget blueprint: {WIDGET_BP_PATH}")


if __name__ == "__main__":
    main()
