// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Unforgotten : ModuleRules
{
	public Unforgotten(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Niagara", "NavigationSystem", "AIModule" });
	}
}
