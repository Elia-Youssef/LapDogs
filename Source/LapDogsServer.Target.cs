// Copyright Shiba Inu Games LLC.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]
public class LapDogsServerTarget : TargetRules
{
    public LapDogsServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        CppStandard = CppStandardVersion.Cpp20;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("ShibRun");
    }
}
