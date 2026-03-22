import unreal

ASSET_PATH = "/Game/Characters/Player/Animations/ThirdPerson_AnimBP"
GRAPH_NAME = "Idle/Run"


def try_get_editor_property(obj, name):
    try:
        return obj.get_editor_property(name)
    except Exception:
        return None


def main():
    blueprint = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
    if blueprint is None:
        raise RuntimeError(f"Failed to load {ASSET_PATH}")

    graph = unreal.BlueprintEditorLibrary.find_graph(blueprint, GRAPH_NAME)
    if graph is None:
        raise RuntimeError(f"Failed to load graph {GRAPH_NAME}")

    nodes = graph.get_graph_nodes_of_class(unreal.AnimGraphNode_Base)
    unreal.log(f"node_count={len(nodes)}")
    for index, node in enumerate(nodes):
        unreal.log(f"node[{index}] class={node.get_class().get_name()} name={node.get_name()}")
        runtime_node = try_get_editor_property(node, "node")
        if runtime_node is not None:
            unreal.log(f"node[{index}] runtime_class={type(runtime_node).__name__}")
            unreal.log(f"node[{index}] runtime_dir={sorted(name for name in dir(runtime_node) if not name.startswith('_'))}")
            for property_name in ["blend_space", "sequence", "x", "y"]:
                value = try_get_editor_property(runtime_node, property_name)
                if value is not None:
                    unreal.log(f"node[{index}] runtime.{property_name}={value}")


if __name__ == "__main__":
    main()
