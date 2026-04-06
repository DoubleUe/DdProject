// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DdProject : ModuleRules
{
	public DdProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(ModuleDirectory + "/Classes");
		PrivateIncludePaths.Add(ModuleDirectory + "/Classes");
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "DeveloperSettings", "AIModule", "NavigationSystem", "GameplayTasks", "MotionTrajectory", "OnlineSubsystem", "OnlineSubsystemUtils", "Sockets", "Json" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");
	}
}
