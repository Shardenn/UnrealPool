// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Pool : ModuleRules
{
	public Pool(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "OnlineSubsystem" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        PrivateDependencyModuleNames.Add("OnlineSubsystem");
    }
}
