import unreal

SOURCE_ANIM_BLUEPRINT_TEMPLATE_PATH = "/Game/Characters/Player/Animations/ThirdPerson_AnimBP"

TARGET_DIRECTORY = "/Game/Characters/Zombie/Animation"
TARGET_BLENDSPACE_NAME = "Zombie_IdleRun_1D"
TARGET_BLENDSPACE_PATH = f"{TARGET_DIRECTORY}/{TARGET_BLENDSPACE_NAME}"
TARGET_ANIM_BLUEPRINT_PATH = f"{TARGET_DIRECTORY}/Zombie_AnimBP"

ZOMBIE_SKELETON_PATH = "/Game/Characters/Zombie/Mesh/Zombie_Running_Skeleton.Zombie_Running_Skeleton"
ZOMBIE_MESH_PATH = "/Game/Characters/Zombie/Mesh/Zombie_Running.Zombie_Running"
ZOMBIE_IDLE_PATH = "/Game/Characters/Zombie/Animation/Zombie_Idle.Zombie_Idle"
ZOMBIE_RUN_PATH = "/Game/Characters/Zombie/Animation/Zombie_Running_Anim.Zombie_Running_Anim"

PLAYER_BLENDSPACE_PATH = "/Game/Characters/Player/Animations/ThirdPerson_IdleRun_2D"


def load_required_asset(asset_path):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if asset is None:
        raise RuntimeError(f"Failed to load asset: {asset_path}")
    return asset


def ensure_directory(directory_path):
    if not unreal.EditorAssetLibrary.does_directory_exist(directory_path):
        unreal.EditorAssetLibrary.make_directory(directory_path)


def delete_asset_if_exists(asset_path):
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.delete_asset(asset_path)


def create_or_update_blend_space(asset_tools, zombie_skeleton, zombie_mesh, zombie_idle, zombie_run):
    blend_space = None
    if unreal.EditorAssetLibrary.does_asset_exist(TARGET_BLENDSPACE_PATH):
        existing_asset = unreal.EditorAssetLibrary.load_asset(TARGET_BLENDSPACE_PATH)
        existing_skeleton = existing_asset.get_editor_property("skeleton") if existing_asset else None
        if isinstance(existing_asset, unreal.BlendSpace1D) and existing_skeleton == zombie_skeleton:
            blend_space = existing_asset
        else:
            delete_asset_if_exists(TARGET_BLENDSPACE_PATH)

    if blend_space is None:
        factory = unreal.BlendSpaceFactory1D()
        factory.set_editor_property("target_skeleton", zombie_skeleton)
        factory.set_editor_property("preview_skeletal_mesh", zombie_mesh)
        blend_space = asset_tools.create_asset(
            TARGET_BLENDSPACE_NAME,
            TARGET_DIRECTORY,
            unreal.BlendSpace1D,
            factory,
        )
        if blend_space is None:
            raise RuntimeError("Failed to create zombie BlendSpace1D")

    blend_parameters = list(blend_space.get_editor_property("blend_parameters"))
    blend_parameters[0].set_editor_property("display_name", "Speed")
    blend_parameters[0].set_editor_property("min", 0.0)
    blend_parameters[0].set_editor_property("max", 300.0)
    blend_parameters[0].set_editor_property("grid_num", 4)
    blend_space.set_editor_property("blend_parameters", blend_parameters)

    idle_sample = unreal.BlendSample()
    idle_sample.set_editor_property("animation", zombie_idle)
    idle_sample.set_editor_property("sample_value", unreal.Vector(0.0, 0.0, 0.0))
    idle_sample.set_editor_property("rate_scale", 1.0)

    run_sample = unreal.BlendSample()
    run_sample.set_editor_property("animation", zombie_run)
    run_sample.set_editor_property("sample_value", unreal.Vector(300.0, 0.0, 0.0))
    run_sample.set_editor_property("rate_scale", 1.0)

    blend_space.set_editor_property("sample_data", [idle_sample, run_sample])
    unreal.EditorAssetLibrary.save_loaded_asset(blend_space)
    return blend_space


def create_or_update_anim_blueprint(zombie_skeleton, target_blend_space):
    if not unreal.EditorAssetLibrary.does_asset_exist(TARGET_ANIM_BLUEPRINT_PATH):
        if not unreal.EditorAssetLibrary.duplicate_asset(
            SOURCE_ANIM_BLUEPRINT_TEMPLATE_PATH,
            TARGET_ANIM_BLUEPRINT_PATH,
        ):
            raise RuntimeError("Failed to create zombie animation blueprint template")

    anim_blueprint = load_required_asset(TARGET_ANIM_BLUEPRINT_PATH)
    anim_blueprint.set_editor_property("target_skeleton", zombie_skeleton)

    idle_run_graph = unreal.BlueprintEditorLibrary.find_graph(anim_blueprint, "Idle/Run")
    if idle_run_graph is None:
        raise RuntimeError("Failed to find Idle/Run graph in zombie animation blueprint")

    blend_space_nodes = idle_run_graph.get_graph_nodes_of_class(unreal.AnimGraphNode_BlendSpacePlayer)
    if len(blend_space_nodes) != 1:
        raise RuntimeError(f"Expected exactly one BlendSpace player node, found {len(blend_space_nodes)}")

    blend_space_node = blend_space_nodes[0]
    runtime_node = blend_space_node.get_editor_property("node")
    runtime_node.set_editor_property("blend_space", target_blend_space)
    blend_space_node.set_editor_property("node", runtime_node)

    unreal.BlueprintEditorLibrary.compile_blueprint(anim_blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(anim_blueprint)
    return anim_blueprint


def verify_assets():
    expected_blend_space_path = load_required_asset(TARGET_BLENDSPACE_PATH).get_path_name()
    zombie_anim_blueprint = load_required_asset(TARGET_ANIM_BLUEPRINT_PATH)
    idle_run_graph = unreal.BlueprintEditorLibrary.find_graph(zombie_anim_blueprint, "Idle/Run")
    if idle_run_graph is None:
        raise RuntimeError("Failed to reload Idle/Run graph for verification")

    blend_space_nodes = idle_run_graph.get_graph_nodes_of_class(unreal.AnimGraphNode_BlendSpacePlayer)
    if len(blend_space_nodes) != 1:
        raise RuntimeError("Failed to reload BlendSpace player node for verification")

    runtime_node = blend_space_nodes[0].get_editor_property("node")
    current_blend_space = runtime_node.get_editor_property("blend_space")
    current_blend_space_path = current_blend_space.get_path_name() if current_blend_space else None
    if current_blend_space_path != expected_blend_space_path:
        raise RuntimeError(
            f"Zombie AnimBP blend space mismatch. Expected {expected_blend_space_path}, got {current_blend_space_path}"
        )

    referencers = unreal.EditorAssetLibrary.find_package_referencers_for_asset(PLAYER_BLENDSPACE_PATH, False)
    if referencers and TARGET_ANIM_BLUEPRINT_PATH in referencers:
        raise RuntimeError("Zombie AnimBP still references the player blend space")


def main():
    ensure_directory(TARGET_DIRECTORY)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    zombie_skeleton = load_required_asset(ZOMBIE_SKELETON_PATH)
    zombie_mesh = load_required_asset(ZOMBIE_MESH_PATH)
    zombie_idle = load_required_asset(ZOMBIE_IDLE_PATH)
    zombie_run = load_required_asset(ZOMBIE_RUN_PATH)

    target_blend_space = create_or_update_blend_space(
        asset_tools,
        zombie_skeleton,
        zombie_mesh,
        zombie_idle,
        zombie_run,
    )
    create_or_update_anim_blueprint(zombie_skeleton, target_blend_space)
    verify_assets()

    unreal.log(f"Created or updated zombie blend space: {TARGET_BLENDSPACE_PATH}")
    unreal.log(f"Created or updated zombie anim blueprint: {TARGET_ANIM_BLUEPRINT_PATH}")


if __name__ == "__main__":
    main()
