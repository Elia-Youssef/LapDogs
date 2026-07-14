// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]
public class ShibRunServerTarget : TargetRules
{
    public ShibRunServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        CppStandard = CppStandardVersion.Cpp20;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        bUseLoggingInShipping = true;
        ExtraModuleNames.Add("ShibRun");
    }
}
