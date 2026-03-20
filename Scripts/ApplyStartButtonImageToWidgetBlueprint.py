import unreal

WIDGET_BP_PATH = "/Game/UI/WBP_TitleScreen"
START_BUTTON_TEXTURE_PATH = "/Game/UI/Resource/StartButton.StartButton"


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


def apply_start_button_texture(start_button, texture):
    style = get_editor_property_any(start_button, "widget_style", "WidgetStyle")

    for brush_property in (("normal", "Normal"), ("hovered", "Hovered"), ("pressed", "Pressed"), ("disabled", "Disabled")):
        brush = get_editor_property_any(style, *brush_property)
        configure_brush(brush, texture)
        set_editor_property_any(style, brush, *brush_property)

    set_editor_property_any(start_button, style, "widget_style", "WidgetStyle")
    start_button.set_background_color(unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    start_button.set_color_and_opacity(unreal.LinearColor(1.0, 1.0, 1.0, 1.0))


def remove_widget_if_exists(widget_bp, widget_name):
    widget = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_bp, widget_name)
    if widget is None:
        return

    parent = widget.get_parent()
    if parent is not None:
        parent.remove_child(widget)
    else:
        widget.remove_from_parent()


def main():
    widget_bp = unreal.EditorAssetLibrary.load_asset(WIDGET_BP_PATH)
    if widget_bp is None:
        raise RuntimeError(f"Failed to load widget blueprint: {WIDGET_BP_PATH}")

    texture = unreal.EditorAssetLibrary.load_asset(START_BUTTON_TEXTURE_PATH)
    if texture is None:
        raise RuntimeError(f"Failed to load start button texture: {START_BUTTON_TEXTURE_PATH}")

    start_button = unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_bp, "StartButton")
    if start_button is None:
        raise RuntimeError("Failed to find StartButton in WBP_TitleScreen")

    remove_widget_if_exists(widget_bp, "StartButtonLabel")
    apply_start_button_texture(start_button, texture)

    unreal.BlueprintEditorLibrary.remove_unused_variables(widget_bp)
    unreal.BlueprintEditorLibrary.compile_blueprint(widget_bp)
    unreal.EditorAssetLibrary.save_loaded_asset(widget_bp)
    unreal.log("Applied StartButton texture and removed StartButtonLabel in /Game/UI/WBP_TitleScreen")


if __name__ == "__main__":
    main()
