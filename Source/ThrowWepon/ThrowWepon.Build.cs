// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ThrowWepon : ModuleRules
{
	public ThrowWepon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
            "OnlineSubsystem", "OnlineSubsystemSteam"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"ThrowWepon",
			"ThrowWepon/Variant_Platforming",
			"ThrowWepon/Variant_Platforming/Animation",
			"ThrowWepon/Variant_Combat",
			"ThrowWepon/Variant_Combat/AI",
			"ThrowWepon/Variant_Combat/Animation",
			"ThrowWepon/Variant_Combat/Gameplay",
			"ThrowWepon/Variant_Combat/Interfaces",
			"ThrowWepon/Variant_Combat/UI",
			"ThrowWepon/Variant_SideScrolling",
			"ThrowWepon/Variant_SideScrolling/AI",
			"ThrowWepon/Variant_SideScrolling/Gameplay",
			"ThrowWepon/Variant_SideScrolling/Interfaces",
			"ThrowWepon/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
