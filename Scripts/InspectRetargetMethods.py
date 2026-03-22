import unreal

TYPE_NAMES = [
    "IKRetargetBatchOperation",
    "IKRetargetFactory",
    "IKRetargeter",
    "IKRetargeterController",
    "BatchRetargetSettings",
]


def main():
    for type_name in TYPE_NAMES:
        type_object = getattr(unreal, type_name, None)
        unreal.log(f"{type_name} exists={type_object is not None}")
        if type_object is None:
            continue

        unreal.log(f"{type_name} methods={sorted(name for name in dir(type_object) if not name.startswith('_'))}")


if __name__ == "__main__":
    main()
