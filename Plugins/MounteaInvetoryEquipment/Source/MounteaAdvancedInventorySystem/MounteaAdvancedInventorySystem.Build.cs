// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MounteaAdvancedInventorySystem : ModuleRules
{
    public MounteaAdvancedInventorySystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(new string[] { });
        PrivateIncludePaths.AddRange(new string[] { });

        // --- Public deps (kept from your file) ---
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "GameplayTags",
            "CommonUI",
            "GameplayAbilities",
            "GameplayTasks"
        });

        // --- Private deps (kept, plus typical runtime deps) ---
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "CoreUObject",
            "Engine",

            "Slate",
            "SlateCore",
            "UMG",

            "DeveloperSettings",
            "GameplayTags",

            "Projects",
            "NetCore",

            "InputCore",
            "EnhancedInput"
        });

        // --- Editor-only deps (for FAppStyle / ToolMenus / editor utilities) ---
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "AppFramework",     // FAppStyle
                "ToolMenus",        // menus/commands
                "EditorSubsystem",  // editor utilities
                "EditorStyle"       // sometimes referenced by plugins
            });

            // In case any headers are pulled via include paths from UnrealEd
            PublicIncludePathModuleNames.AddRange(new string[]
            {
                "UnrealEd"
            });
        }

        DynamicallyLoadedModuleNames.AddRange(new string[] { });
    }
}
