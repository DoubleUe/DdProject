import unreal

TYPE_NAMES = [
    "IKRigController",
    "IKRetargeterController",
    "IKRigDefinition",
    "IKRetargeter",
]


def main():
    unreal.log(f"has_new_object={hasattr(unreal, 'new_object')}")
    unreal.log(f"has_get_transient_package={hasattr(unreal, 'get_transient_package')}")

    for type_name in TYPE_NAMES:
        type_object = getattr(unreal, type_name, None)
        unreal.log(f"{type_name} exists={type_object is not None}")
        if type_object is None:
            continue

        unreal.log(f"{type_name} methods={sorted(name for name in dir(type_object) if not name.startswith('_'))}")


if __name__ == "__main__":
    main()
