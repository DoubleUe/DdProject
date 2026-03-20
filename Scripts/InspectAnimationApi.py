import unreal

TYPE_NAMES = [
    "AnimationBlueprintLibrary",
    "AnimationLibrary",
    "IKRigEditorSubsystem",
    "AssetTools",
]

KEYWORDS = [
    "skeleton",
    "retarget",
    "animation",
]


def main():
    for type_name in TYPE_NAMES:
        type_object = getattr(unreal, type_name, None)
        unreal.log(f"{type_name} exists={type_object is not None}")
        if type_object is None:
            continue

        names = sorted(name for name in dir(type_object) if any(keyword in name.lower() for keyword in KEYWORDS))
        unreal.log(f"{type_name} names={names}")


if __name__ == "__main__":
    main()
