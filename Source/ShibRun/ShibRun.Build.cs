// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ShibRun : ModuleRules
{
	public ShibRun(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"NetCore",
			"CoreOnline",
			"Engine",
			"InputCore",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"OnlineSubsystemBlueprints",
			"Networking",
			"EnhancedInput",
			"GameplayTags",
			"AbilitySystem",
			"UMG",
			"Slate",
			"SlateCore",
			"GameplayTasks",
			"Niagara",
			"ShibMultiplayer",
			"ShibUiNavigation",
			"MoviePlayer",
			"DeveloperSettings",
			"ShibAsyncLoadingScreen",
			"ShibAPIs",
			"Matchmaking",
			"MediaAssets"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "NavigationSystem", "AIModule", "Slate", "SlateCore" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
