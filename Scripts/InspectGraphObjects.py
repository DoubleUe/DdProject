import unreal

ASSET_PATH = "/Game/Characters/Player/Animations/ThirdPerson_AnimBP"


def main():
    blueprint = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
    if blueprint is None:
        raise RuntimeError(f"Failed to load {ASSET_PATH}")

    graph = unreal.BlueprintEditorLibrary.find_graph(blueprint, "AnimGraph")
    if graph is None:
        raise RuntimeError("Failed to load AnimGraph")

    unreal.log(f"graph_class={graph.get_class().get_name()}")
    unreal.log(f"graph_dir={sorted(name for name in dir(graph) if not name.startswith('_'))}")
    for property_name in ["nodes", "sub_graphs", "schema", "graph_guid"]:
        try:
            unreal.log(f"{property_name}={graph.get_editor_property(property_name)}")
        except Exception as exc:
            unreal.log_warning(f"{property_name} unavailable: {exc}")


if __name__ == "__main__":
    main()
