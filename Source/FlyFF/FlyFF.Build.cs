// Copyright Epic Games, Inc. All Rights Reserved.

using EpicGames.Core;
using System.Collections.Generic;
using System.IO;
using UnrealBuildTool;

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
            "AIModule",
            "Networking", 
            "Sockets",
            "HTTP"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        // ---- PostgreSQL (libpq) ----
        // Try several candidate locations and pick the first that contains libpq-fe.h
        string[] candidates =
        {
            Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "ThirdParty", "Postgres")),      // <project>/Source/ThirdParty/Postgres
            Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "Postgres")),// <project>/ThirdParty/Postgres (project root)
            Path.GetFullPath(Path.Combine(ModuleDirectory, "ThirdParty", "Postgres")),            // <project>/Source/FlyFF/ThirdParty/Postgres
        };

        string ThirdPartyRoot = null;
        string IncludeDirFound = null;

        foreach (var cand in candidates)
        {
            string inc = Path.Combine(cand, "include");
            string hdr = Path.Combine(inc, "libpq-fe.h");
            if (File.Exists(hdr))
            {
                ThirdPartyRoot = cand;
                IncludeDirFound = inc;
                break;
            }
        }

        if (ThirdPartyRoot == null)
        {
            // Helpful error that shows exactly what we checked
            throw new BuildException(
                "Postgres include folder not found.\n" +
                "Looked in:\n" +
                $"  {candidates[0]}\\include\\libpq-fe.h\n" +
                $"  {candidates[1]}\\include\\libpq-fe.h\n" +
                $"  {candidates[2]}\\include\\libpq-fe.h\n" +
                "Fix: Put headers & libs in one of those locations, or adjust Build.cs."
            );
        }

        Log.TraceInformation($"[FlyFF] Using Postgres ThirdParty at: {ThirdPartyRoot}");
        PublicIncludePaths.Add(IncludeDirFound);

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // Accept either .../lib/Win64 or .../lib (some zips use just lib/)
            string lib64 = Path.Combine(ThirdPartyRoot, "lib", "Win64");
            string libGeneric = Path.Combine(ThirdPartyRoot, "lib");

            string libDir = Directory.Exists(lib64) ? lib64 :
                            Directory.Exists(libGeneric) ? libGeneric : null;

            if (libDir == null)
            {
                throw new BuildException(
                    "Postgres lib folder not found.\n" +
                    $"Tried:\n  {lib64}\n  {libGeneric}\n" +
                    "Make sure libpq.lib is copied there."
                );
            }

            string LibpqLib = Path.Combine(libDir, "libpq.lib");
            string LibpqDll = Path.Combine(libDir, "libpq.dll");

            // Common OpenSSL + zlib names for PG 16/17. Adjust if your filenames differ.
            string SslDll = Path.Combine(libDir, "libssl-3-x64.dll");
            string CryptoDll = Path.Combine(libDir, "libcrypto-3-x64.dll");
            string ZlibDll = Path.Combine(libDir, "zlib1.dll");

            // Optional dependencies (some builds ship these)
            string IconvDll = Path.Combine(libDir, "libiconv-2.dll");
            string IntlDll = Path.Combine(libDir, "libintl-9.dll");

            // Validate required files exist; if your ssl/crypto dlls have different names,
            // change them here to match your bin folder.
            if (!File.Exists(LibpqLib))
                throw new BuildException($"Missing libpq.lib at: {LibpqLib}");
            if (!File.Exists(LibpqDll))
                throw new BuildException($"Missing libpq.dll at: {LibpqDll}");

            // If your bin doesn�t have these exact names, comment them out or rename to match.
            if (!File.Exists(SslDll)) throw new BuildException($"Missing OpenSSL dll at: {SslDll} (rename in Build.cs if your version differs)");
            if (!File.Exists(CryptoDll)) throw new BuildException($"Missing OpenSSL dll at: {CryptoDll} (rename in Build.cs if your version differs)");
            if (!File.Exists(ZlibDll)) throw new BuildException($"Missing zlib dll at: {ZlibDll}");

            PublicAdditionalLibraries.Add(LibpqLib);

            string OutDir = "$(TargetOutputDir)";
            RuntimeDependencies.Add($"{OutDir}/libpq.dll", LibpqDll);
            RuntimeDependencies.Add($"{OutDir}/libssl-3-x64.dll", SslDll, StagedFileType.NonUFS);
            RuntimeDependencies.Add($"{OutDir}/libcrypto-3-x64.dll", CryptoDll, StagedFileType.NonUFS);
            RuntimeDependencies.Add($"{OutDir}/zlib1.dll", ZlibDll, StagedFileType.NonUFS);

            if (File.Exists(IconvDll)) RuntimeDependencies.Add($"{OutDir}/libiconv-2.dll", IconvDll, StagedFileType.NonUFS);
            if (File.Exists(IntlDll)) RuntimeDependencies.Add($"{OutDir}/libintl-9.dll", IntlDll, StagedFileType.NonUFS);
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PublicAdditionalLibraries.Add("pq");
            PublicSystemLibraries.AddRange(new[] { "ssl", "crypto", "z" });
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicAdditionalLibraries.Add("pq");
            PublicSystemLibraries.AddRange(new[] { "ssl", "crypto", "z" });
        }
    }
}
