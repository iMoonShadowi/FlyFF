using UnrealBuildTool;

public class FlyFF : ModuleRules
{
    public FlyFF(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Public modules used by game + server
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

        // Add private modules here if/when needed
        PrivateDependencyModuleNames.AddRange(new string[] { });

        // Keep include paths tidy for your Public/Private layout
        PublicIncludePaths.AddRange(new string[] { "FlyFF/Public" });
        PrivateIncludePaths.AddRange(new string[] { "FlyFF/Private" });
    }
}
