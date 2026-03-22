import unreal


def main():
    names = sorted(name for name in dir(unreal) if "retarget" in name.lower())
    unreal.log(f"retarget_api_names={names}")


if __name__ == "__main__":
    main()
