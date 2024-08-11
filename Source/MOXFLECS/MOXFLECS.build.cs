// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MOXFLECS : ModuleRules
{
    public MOXFLECS(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivatePCHHeaderFile = "Public/MOXFLECS.h";
		CppStandard = CppStandardVersion.Cpp17;

        PublicDependencyModuleNames.AddRange(new string[] {
             "Core",
             "CoreUObject",
             "Engine",
             "DeveloperSettings",
        });

        PublicIncludePaths.AddRange(new string[] { "FlecsLibrary/Public" });
        PrivateIncludePaths.AddRange(new string[] { "FlecsLibrary/Private" });

        AppendStringToPublicDefinition("flecs_EXPORTS", "0");
    }

}