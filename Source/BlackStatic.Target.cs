using UnrealBuildTool;

public class BlackStaticTarget : TargetRules
{
    public BlackStaticTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("BlackStatic");
    }
}

