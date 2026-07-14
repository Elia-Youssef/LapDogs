// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ShibRunClientTarget : TargetRules
{
    public ShibRunClientTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Client;
        CppStandard = CppStandardVersion.Cpp20;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        bUseLoggingInShipping = true;
        ExtraModuleNames.Add("ShibRun");
    }
}
