import unreal

ASSET_PATH = "/Game/Characters/Player/Animations/ThirdPerson_AnimBP"
GRAPH_NAMES = [
    "Locomotion",
    "Idle/Run",
    "Jump_Start",
    "Jump_Loop",
    "Jump_End",
    "AnimGraph",
    "EventGraph",
]


def main():
    blueprint = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
    if blueprint is None:
        raise RuntimeError(f"Failed to load {ASSET_PATH}")

    for graph_name in GRAPH_NAMES:
        graph = unreal.BlueprintEditorLibrary.find_graph(blueprint, graph_name)
        unreal.log(f"graph {graph_name}={graph}")


if __name__ == "__main__":
    main()
