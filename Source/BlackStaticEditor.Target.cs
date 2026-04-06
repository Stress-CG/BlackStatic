using UnrealBuildTool;

public class BlackStaticEditorTarget : TargetRules
{
    public BlackStaticEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("BlackStatic");
    }
}

