// Copyright Shiba Inu Games LLC.

using UnrealBuildTool;
using System.Collections.Generic;

public class LapDogsTarget : TargetRules
{
    public LapDogsTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Client;
        CppStandard = CppStandardVersion.Cpp20;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("ShibRun");
    }
}
