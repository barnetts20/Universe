// Copyright TriAxis Games, L.L.C. All Rights Reserved.

using System;
using UnrealBuildTool;

public class RealtimeMeshExt : ModuleRules
{
    public RealtimeMeshExt(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
#if UE_5_1_OR_LATER
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
#endif

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "RealtimeMeshComponent"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "GeometryCore",
                "GeometryFramework",
                "RenderCore",
                "RHI",
            }
        );


        PrivateIncludePaths.AddRange(
            new string[]
            {
                System.IO.Path.Combine(PluginDirectory, "Source", "ThirdParty")
            }
        );
    }
}

public class Paths
{
}