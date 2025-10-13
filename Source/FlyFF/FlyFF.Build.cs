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
        // ---------- PostgreSQL (libpq) ----------
        //
        string ThirdPartyRoot = Path.Combine(ModuleDirectory, "..", "ThirdParty", "Postgres");
        string IncludeDir     = Path.Combine(ThirdPartyRoot, "include");
        if (!Directory.Exists(IncludeDir))
        {
            throw new BuildException($"Postgres include folder not found: {IncludeDir}\n" +
                                     $"Expected 'libpq-fe.h' inside it.");
        }
        PublicIncludePaths.Add(IncludeDir);

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string WinLibDir = Path.Combine(ThirdPartyRoot, "lib", "Win64");
            if (!Directory.Exists(WinLibDir))
            {
                throw new BuildException($"Postgres Win64 lib folder not found: {WinLibDir}");
            }

            string LibpqLib  = Path.Combine(WinLibDir, "libpq.lib");
            string LibpqDll  = Path.Combine(WinLibDir, "libpq.dll");

            // Common OpenSSL + zlib DLL names for libpq 16/17 builds (adjust if your filenames differ)
            string SslDll    = Path.Combine(WinLibDir, "libssl-3-x64.dll");
            string CryptoDll = Path.Combine(WinLibDir, "libcrypto-3-x64.dll");
            string ZlibDll   = Path.Combine(WinLibDir, "zlib1.dll");

            // Some libpq builds also need these (stage if present)
            string IconvDll  = Path.Combine(WinLibDir, "libiconv-2.dll");
            string IntlDll   = Path.Combine(WinLibDir, "libintl-9.dll");

            // Validate required files
            string[] mustExist = new[] { LibpqLib, LibpqDll, SslDll, CryptoDll, ZlibDll };
            foreach (var p in mustExist)
            {
                if (!File.Exists(p))
                    throw new BuildException($"Missing Postgres binary: {p}");
            }

            // Link libpq
            PublicAdditionalLibraries.Add(LibpqLib);

            // Stage runtime DLLs next to the built binary
            string OutDir = "$(TargetOutputDir)";
            RuntimeDependencies.Add($"{OutDir}/libpq.dll",           LibpqDll);
            RuntimeDependencies.Add($"{OutDir}/libssl-3-x64.dll",    SslDll,    StagedFileType.NonUFS);
            RuntimeDependencies.Add($"{OutDir}/libcrypto-3-x64.dll", CryptoDll, StagedFileType.NonUFS);
            RuntimeDependencies.Add($"{OutDir}/zlib1.dll",           ZlibDll,   StagedFileType.NonUFS);

            if (File.Exists(IconvDll)) RuntimeDependencies.Add($"{OutDir}/libiconv-2.dll", IconvDll, StagedFileType.NonUFS);
            if (File.Exists(IntlDll))  RuntimeDependencies.Add($"{OutDir}/libintl-9.dll",  IntlDll,  StagedFileType.NonUFS);
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            // Prefer system libraries on your server image (e.g., Debian/Ubuntu):
            //   apt-get install -y libpq5 libpq-dev libssl3 zlib1g
            PublicAdditionalLibraries.Add("pq");
            PublicSystemLibraries.AddRange(new string[] { "ssl", "crypto", "z" });
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            // If you ever target macOS, rely on system-installed libs (brew install libpq openssl zlib)
            // You may need to ensure RPaths or bundle dylibs depending on your packaging.
            PublicAdditionalLibraries.Add("pq");
            PublicSystemLibraries.AddRange(new string[] { "ssl", "crypto", "z" });
        }
    }
}
