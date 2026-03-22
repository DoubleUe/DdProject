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

    anim_graph = unreal.BlueprintEditorLibrary.find_graph(blueprint, "AnimGraph")
    if anim_graph is None:
        raise RuntimeError("Failed to load AnimGraph")

    machine_nodes = anim_graph.get_graph_nodes_of_class(unreal.AnimGraphNode_StateMachine)
    if not machine_nodes:
        raise RuntimeError("No state machine nodes found")

    machine_node = machine_nodes[0]
    unreal.log(f"node_dir={sorted(name for name in dir(machine_node) if not name.startswith('_'))}")

    for property_name in [
        "bound_graph",
        "editor_state_machine_graph",
        "state_machine_graph",
        "node",
        "tag",
    ]:
        value = try_get_editor_property(machine_node, property_name)
        unreal.log(f"{property_name}={value}")

    runtime_node = try_get_editor_property(machine_node, "node")
    if runtime_node is not None:
        unreal.log(f"runtime_dir={sorted(name for name in dir(runtime_node) if not name.startswith('_'))}")


if __name__ == "__main__":
    main()
