// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Pool : ModuleRules
{
	public Pool(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "OnlineSubsystem", "OnlineSubsystemSteam" });

		PrivateDependencyModuleNames.AddRange(new string[] { "OnlineSubsystem" });
    }
}
