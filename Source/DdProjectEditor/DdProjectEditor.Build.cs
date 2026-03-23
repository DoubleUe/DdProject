using UnrealBuildTool;

public class DdProjectEditor : ModuleRules
{
	public DdProjectEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(ModuleDirectory + "/Classes");
		PrivateIncludePaths.Add(ModuleDirectory + "/Classes");

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"InputCore",
			"LevelEditor",
			"Slate",
			"SlateCore",
			"DdProject",
			"UnrealEd",
			"WorkspaceMenuStructure"
		});
	}
}
