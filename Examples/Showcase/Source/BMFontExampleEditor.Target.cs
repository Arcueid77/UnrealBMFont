// Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

using UnrealBuildTool;

public class BMFontExampleEditorTarget : TargetRules
{
	public BMFontExampleEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("BMFontExample");
	}
}
