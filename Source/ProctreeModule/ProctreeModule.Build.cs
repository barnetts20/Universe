// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ProctreeModule : ModuleRules
{
	public ProctreeModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore", 
            "RealtimeMeshComponent",
            "RealtimeMeshExt",
            "RenderCore",
            "RHI"
            //"PhysX",
            //"PhysXCooking" 
        });

        //Enable Fast Noise Lib
        CMakeTarget.add(Target, this, "FastNoise", Path.Combine(this.ModuleDirectory, "../ThirdParty/FastNoise2"), "-DFASTNOISE2_NOISETOOL=OFF", true);
        
        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
