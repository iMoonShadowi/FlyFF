// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;
using System.IO;

public class FlyFF : ModuleRules
{
    public FlyFF(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "Json",
            "JsonUtilities",
            "GameplayTags",
            "NavigationSystem",
            "AIModule"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        //
        // ---- PostgreSQL (libpq) wiring ----
        //
        string ThirdPartyRoot = Path.Combine(ModuleDirectory, "..", "ThirdParty", "Postgres");
        string PqInclude = Path.Combine(ThirdPartyRoot, "include");
        PublicIncludePaths.Add(PqInclude);

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string PqLibWin = Path.Combine(ThirdPartyRoot, "lib", "Win64");

            // Link libpq
            PublicAdditionalLibraries.Add(Path.Combine(PqLibWin, "libpq.lib"));

            // Make sure required DLLs are staged next to your binary
            // (If your libpq build depends on extra DLLs like libiconv/libintl, stage them too.)
            string OutputDir = "$(TargetOutputDir)";
            RuntimeDependencies.Add($"{OutputDir}/libpq.dll", Path.Combine(PqLibWin, "libpq.dll"));
            RuntimeDependencies.Add($"{OutputDir}/libssl-3-x64.dll", Path.Combine(PqLibWin, "libssl-3-x64.dll"));
            RuntimeDependencies.Add($"{OutputDir}/libcrypto-3-x64.dll", Path.Combine(PqLibWin, "libcrypto-3-x64.dll"));
            RuntimeDependencies.Add($"{OutputDir}/zlib1.dll", Path.Combine(PqLibWin, "zlib1.dll"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            // Prefer system libraries on Linux server images:
            //   apt-get install -y libpq5 libpq-dev libssl3 zlib1g
            // If you ship custom static libs instead, replace with full paths similar to Windows above.
            PublicAdditionalLibraries.Add("pq");
            PublicSystemLibraries.AddRange(new string[] { "ssl", "crypto", "z" });
        }
        // (Add macOS here if you target it)
    }
}
