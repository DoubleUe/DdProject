import unreal

ASSET_PATH = "/Game/Characters/Player/Animations/ThirdPerson_AnimBP"


def try_get_editor_property(obj, name):
    try:
        return obj.get_editor_property(name)
    except Exception:
        return None


def log_state_nodes(graph, prefix):
    nodes = graph.get_graph_nodes_of_class(unreal.AnimStateNode)
    unreal.log(f"{prefix} state_count={len(nodes)}")
    for index, node in enumerate(nodes):
        unreal.log(f"{prefix} state[{index}] class={node.get_class().get_name()} name={node.get_name()}")
        bound_graph = try_get_editor_property(node, "bound_graph")
        if bound_graph is not None:
            unreal.log(f"{prefix} state[{index}] bound_graph={bound_graph}")
            graph_nodes = bound_graph.get_graph_nodes_of_class(unreal.AnimGraphNode_Base)
            unreal.log(f"{prefix} state[{index}] bound_node_count={len(graph_nodes)}")
            for graph_node_index, graph_node in enumerate(graph_nodes):
                unreal.log(
                    f"{prefix} state[{index}] node[{graph_node_index}] class={graph_node.get_class().get_name()} name={graph_node.get_name()}"
                )
                runtime_node = try_get_editor_property(graph_node, "node")
                if runtime_node is not None:
                    unreal.log(
                        f"{prefix} state[{index}] node[{graph_node_index}] runtime_class={type(runtime_node).__name__}"
                    )
                    for property_name in ["blend_space", "sequence"]:
                        value = try_get_editor_property(runtime_node, property_name)
                        if value is not None:
                            unreal.log(
                                f"{prefix} state[{index}] node[{graph_node_index}] runtime.{property_name}={value}"
                            )


def main():
    blueprint = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
    if blueprint is None:
        raise RuntimeError(f"Failed to load {ASSET_PATH}")

    anim_graph = unreal.BlueprintEditorLibrary.find_graph(blueprint, "AnimGraph")
    if anim_graph is None:
        raise RuntimeError("Failed to load AnimGraph")

    machine_nodes = anim_graph.get_graph_nodes_of_class(unreal.AnimGraphNode_StateMachine)
    unreal.log(f"machine_count={len(machine_nodes)}")
    for index, machine_node in enumerate(machine_nodes):
        unreal.log(f"machine[{index}] name={machine_node.get_name()}")
        editor_state_machine_graph = try_get_editor_property(machine_node, "editor_state_machine_graph")
        if editor_state_machine_graph is None:
            editor_state_machine_graph = try_get_editor_property(machine_node, "bound_graph")
        unreal.log(f"machine[{index}] graph={editor_state_machine_graph}")
        if editor_state_machine_graph is not None:
            log_state_nodes(editor_state_machine_graph, f"machine[{index}]")


if __name__ == "__main__":
    main()
