import unreal

ASSET_PATH = "/Game/Characters/Player/Animations/ThirdPerson_AnimBP"


def try_get_editor_property(obj, name):
    try:
        return obj.get_editor_property(name)
    except Exception:
        return None


def main():
    blueprint = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
    if blueprint is None:
        raise RuntimeError(f"Failed to load {ASSET_PATH}")

    for graph_name in ["AnimGraph", "EventGraph"]:
        graph = unreal.BlueprintEditorLibrary.find_graph(blueprint, graph_name)
        unreal.log(f"graph {graph_name}={graph}")
        if graph is None:
            continue

        nodes = try_get_editor_property(graph, "nodes") or []
        unreal.log(f"graph {graph_name} node_count={len(nodes)}")
        for index, node in enumerate(nodes):
            unreal.log(f"node[{index}] class={node.get_class().get_name()} name={node.get_name()}")
            for property_name in ["node", "bound_graph", "blend_space", "sequence", "state_machine_graph"]:
                value = try_get_editor_property(node, property_name)
                if value is not None:
                    unreal.log(f"node[{index}] {property_name}={value}")

            runtime_node = try_get_editor_property(node, "node")
            if runtime_node is None:
                continue

            for property_name in ["blend_space", "sequence", "x", "y"]:
                value = try_get_editor_property(runtime_node, property_name)
                if value is not None:
                    unreal.log(f"node[{index}] runtime.{property_name}={value}")


if __name__ == "__main__":
    main()
