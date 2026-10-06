// Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

using UnrealBuildTool;

public class BMFontExampleTarget : TargetRules
{
	public BMFontExampleTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("BMFontExample");
	}
}
