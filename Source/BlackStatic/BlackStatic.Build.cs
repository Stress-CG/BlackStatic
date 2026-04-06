using UnrealBuildTool;

public class BlackStatic : ModuleRules
{
    public BlackStatic(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "EnhancedInput",
                "Json",
                "JsonUtilities",
                "Slate",
                "SlateCore",
                "UMG"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "AssetRegistry",
                "Projects"
            }
        );

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(
                new[]
                {
                    "UnrealEd"
                }
            );
        }
    }
}
