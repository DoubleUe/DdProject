import unreal

KEYWORDS = [
    "retarget",
    "skeleton",
    "anim",
]


def main():
    names = sorted(
        name
        for name in dir(unreal)
        if any(keyword in name.lower() for keyword in KEYWORDS)
    )
    unreal.log(f"api_names={names}")


if __name__ == "__main__":
    main()
